#include "FakeKafka.h"
#include "KafkaAdapter.h"
#include <stdarg.h>

/* Instrument only calls made by adapter/connection code. DDS objects and
 * semaphores remain real; the thread double records lifecycle ordering. */
static struct {
    void *blocks[32];
    int allocation_calls, fail_allocation, allocations;
    int samples, supports, semaphores, semaphore_calls, fail_semaphore;
    int fail_sample, fail_support, fail_thread, fail_initial_give, fail_join;
    struct RTIOsapiSemaphore *fail_give;
    int threads, stops, errors, sensitive_log, undelivered_log;
    int delivery_success_log, delivery_failure_log;
    char error[512];
    void *thread_param;
} hooks;

static void *test_calloc(size_t count, size_t size)
{
    int i;
    void *block;
    hooks.allocation_calls++;
    if (hooks.allocation_calls == hooks.fail_allocation) return NULL;
    block = calloc(count, size);
    CHECK(block != NULL);
    for (i = 0; i < 32; i++) {
        if (!hooks.blocks[i]) {
            hooks.blocks[i] = block;
            hooks.allocations++;
            return block;
        }
    }
    CHECK(0);
    return NULL;
}

static void test_free(void *block)
{
    int i;
    if (!block) return;
    for (i = 0; i < 32; i++) {
        if (hooks.blocks[i] == block) {
            hooks.blocks[i] = NULL;
            hooks.allocations--;
            free(block);
            return;
        }
    }
    CHECK(0); /* Invalid or duplicate free. */
}

static DDS_DynamicData *test_sample_new(const DDS_TypeCode *type,
        const struct DDS_DynamicDataProperty_t *properties)
{
    DDS_DynamicData *sample;
    if (hooks.fail_sample) return NULL;
    sample = DDS_DynamicData_new(type, properties);
    CHECK(sample != NULL);
    hooks.samples++;
    return sample;
}

static void test_sample_delete(DDS_DynamicData *sample)
{
    CHECK(sample != NULL && hooks.samples > 0);
    hooks.samples--;
    DDS_DynamicData_delete(sample);
}

static struct DDS_DynamicDataTypeSupport *test_support_new(const DDS_TypeCode *type,
        const struct DDS_DynamicDataTypeProperty_t *properties)
{
    struct DDS_DynamicDataTypeSupport *support;
    if (hooks.fail_support) return NULL;
    support = DDS_DynamicDataTypeSupport_new(type, properties);
    CHECK(support != NULL);
    hooks.supports++;
    return support;
}

static void test_support_delete(struct DDS_DynamicDataTypeSupport *support)
{
    CHECK(support != NULL && hooks.supports > 0);
    hooks.supports--;
    DDS_DynamicDataTypeSupport_delete(support);
}

static struct RTIOsapiSemaphore *test_semaphore_new(RTIOsapiSemaphoreKind kind,
        const struct RTIOsapiSemaphoreProperty *properties)
{
    struct RTIOsapiSemaphore *sem;
    hooks.semaphore_calls++;
    if (hooks.semaphore_calls == hooks.fail_semaphore) return NULL;
    sem = RTIOsapiSemaphore_new(kind, properties);
    CHECK(sem != NULL);
    hooks.semaphores++;
    return sem;
}

static void test_semaphore_delete(struct RTIOsapiSemaphore *sem)
{
    CHECK(sem != NULL && hooks.semaphores > 0);
    hooks.semaphores--;
    RTIOsapiSemaphore_delete(sem);
}

static RTIOsapiSemaphoreStatus test_semaphore_give(struct RTIOsapiSemaphore *sem)
{
    if (sem == hooks.fail_give) {
        hooks.fail_give = NULL;
        return RTI_OSAPI_SEMAPHORE_STATUS_ERROR;
    }
    if (hooks.fail_initial_give) {
        hooks.fail_initial_give = 0;
        return RTI_OSAPI_SEMAPHORE_STATUS_ERROR;
    }
    return RTIOsapiSemaphore_give(sem);
}

static struct RTIOsapiJoinableThread *test_thread_new(const char *name,
        int priority, int options, int stack_size, struct RTIOsapiCpuBitmap *cpu,
        RTIOsapiThreadOnSpawnedMethod entry, void *param)
{
    struct RTI_RS_KafkaStreamReader *reader = param;
    (void) priority; (void) options; (void) stack_size; (void) cpu;
    CHECK(strcmp(name, "KafkaReader") == 0);
    CHECK(entry == RTI_RS_KafkaStreamReader_on_data_availabe_thread);
    CHECK(reader->stop_sem && reader->poll_sem && reader->read_sem);
    if (hooks.fail_thread) return NULL;
    hooks.thread_param = param;
    hooks.threads++;
    return (struct RTIOsapiJoinableThread *) &hooks;
}

static RTIBool test_thread_stop(struct RTIOsapiJoinableThread *thread, RTI_UINT32 timeout)
{
    struct RTI_RS_KafkaStreamReader *reader = hooks.thread_param;
    struct RTINtpTime zero = {0, 0};
    CHECK(thread == (struct RTIOsapiJoinableThread *) &hooks);
    CHECK(timeout == RTI_OSAPI_THREAD_INFINITE_BLOCKING_TIMEOUT);
    CHECK(RTIOsapiSemaphore_take(reader->stop_sem, &zero) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    CHECK(RTIOsapiSemaphore_give(reader->stop_sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    /* A worker waiting for a returned loan must be woken before joining. */
    CHECK(RTIOsapiSemaphore_take(reader->poll_sem, &zero) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    CHECK(hooks.threads == 1 && kafka.close_calls == 0);
    if (hooks.fail_join) {
        hooks.fail_join = 0;
        return RTI_FALSE;
    }
    hooks.threads--;
    hooks.stops++;
    return RTI_TRUE;
}

static void test_set_error(RTI_RoutingServiceEnvironment *env, const char *format, ...)
{
    va_list args;
    (void) env;
    hooks.errors++;
    va_start(args, format);
    vsnprintf(hooks.error, sizeof(hooks.error), format, args);
    va_end(args);
}

static void test_log(int verbosity, const char *format, ...)
{
    char message[1024];
    va_list args;
    (void) verbosity;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    if (strstr(message, "unit-secret")) hooks.sensitive_log++;
    if (strstr(message, "not delivered")) hooks.undelivered_log++;
    if (strstr(message, "Message delivered")) hooks.delivery_success_log++;
    if (strstr(message, "Message delivery failed")) hooks.delivery_failure_log++;
}

#define calloc test_calloc
#define free test_free
#define DDS_DynamicData_new test_sample_new
#define DDS_DynamicData_delete test_sample_delete
#define DDS_DynamicDataTypeSupport_new test_support_new
#define DDS_DynamicDataTypeSupport_delete test_support_delete
#define RTIOsapiSemaphore_new test_semaphore_new
#define RTIOsapiSemaphore_delete test_semaphore_delete
#define RTIOsapiSemaphore_give test_semaphore_give
#define RTIOsapiJoinableThread_new test_thread_new
#define RTIOsapiJoinableThread_stopAndDelete test_thread_stop
#define RTI_RoutingServiceEnvironment_set_error test_set_error
#undef RTI_RoutingServiceLogger_log
#define RTI_RoutingServiceLogger_log test_log

/* Compile the production implementation with local dependency wrappers. This
 * permits allocation-failure testing without replacing the process allocator. */
#include "../srcC/KafkaAdapter.c"
#include "../srcC/KafkaConnection.c"

#undef calloc
#undef free
#undef DDS_DynamicData_new
#undef DDS_DynamicData_delete
#undef DDS_DynamicDataTypeSupport_new
#undef DDS_DynamicDataTypeSupport_delete
#undef RTIOsapiSemaphore_new
#undef RTIOsapiSemaphore_delete
#undef RTIOsapiSemaphore_give
#undef RTIOsapiJoinableThread_new
#undef RTIOsapiJoinableThread_stopAndDelete
#undef RTI_RoutingServiceEnvironment_set_error
#undef RTI_RoutingServiceLogger_log

static void check_no_resources(void)
{
    CHECK(hooks.allocations == 0);
    CHECK(hooks.samples == 0 && hooks.supports == 0);
    CHECK(hooks.semaphores == 0 && hooks.threads == 0);
    CHECK(kafka.configurations == 0 && kafka.clients == 0 && kafka.subscriptions == 0);
}

static void unused_listener(RTI_RoutingServiceStreamReader reader, void *data)
{
    (void) reader; (void) data;
    CHECK(0); /* The lifecycle thread double must not run the consumer. */
}

static void test_adapter(const char *name)
{
    struct RTI_RoutingServiceAdapterPlugin *adapter;
    if (strcmp(name, "adapter_allocation_failure") == 0) hooks.fail_allocation = 1;
    adapter = RTI_RS_Kafka_AdapterPlugin_create(NULL, NULL);
    if (hooks.fail_allocation) {
        CHECK(adapter == NULL && hooks.errors == 1);
    } else {
        CHECK(adapter != NULL && hooks.errors == 0);
        CHECK(adapter->adapter_plugin_delete == RTI_RS_KafkaAdapterPlugin_delete);
        CHECK(adapter->adapter_plugin_create_connection == RTI_RS_KafkaAdapterPlugin_create_connection);
        CHECK(adapter->adapter_plugin_delete_connection == RTI_RS_KafkaAdapterPlugin_delete_connection);
        CHECK(adapter->connection_create_stream_writer == RTI_RS_KafkaConnection_create_stream_writer);
        CHECK(adapter->connection_delete_stream_writer == RTI_RS_KafkaConnection_delete_stream_writer);
        CHECK(adapter->connection_create_stream_reader == RTI_RS_KafkaConnection_create_stream_reader);
        CHECK(adapter->connection_delete_stream_reader == RTI_RS_KafkaConnection_delete_stream_reader);
        CHECK(adapter->stream_writer_write == RTI_RS_KafkaStreamWriter_write);
        CHECK(adapter->stream_reader_read == RTI_RS_KafkaStreamReader_read);
        CHECK(adapter->stream_reader_return_loan == RTI_RS_KafkaStreamReader_return_loan);
        CHECK(adapter->connection_create_session == NULL && adapter->connection_delete_session == NULL);
        RTI_RS_KafkaAdapterPlugin_delete(adapter, NULL);
    }
    check_no_resources();
}

static void test_connection(const char *name)
{
    struct RTI_RoutingServiceNameValue property = {"bootstrap.servers", "host1:9092,host2:9092"};
    struct RTI_RoutingServiceProperties properties = {&property, 1, DDS_BOOLEAN_TRUE};
    struct RTI_RS_KafkaConnection *connection;
    int failure = strcmp(name, "connection_success") != 0;
    if (strcmp(name, "connection_missing_bootstrap") == 0) properties.count = 0;
    if (strcmp(name, "connection_allocation_failure") == 0) hooks.fail_allocation = 1;
    connection = RTI_RS_KafkaAdapterPlugin_create_connection(NULL,
            "service", "group", NULL, NULL, NULL, 0, &properties, NULL);
    if (failure) {
        CHECK(connection == NULL && hooks.errors == 1);
    } else {
        CHECK(connection != NULL && hooks.errors == 0);
        CHECK(strcmp(connection->bootstrap_servers, "host1:9092,host2:9092") == 0);
        RTI_RS_KafkaAdapterPlugin_delete_connection(NULL, connection, NULL);
    }
    check_no_resources();
}

static const char *last_config(const char *name)
{
    int i;
    for (i = kafka.config_count - 1; i >= 0; i--) {
        if (strcmp(kafka.config_names[i], name) == 0) return kafka.config_values[i];
    }
    return NULL;
}

static void test_stream_creation(const char *name, int reader_case)
{
    struct RTI_RS_KafkaConnection connection = {NULL, "host:9092"};
    struct RTI_RoutingServiceStreamInfo info = {0};
    struct RTI_RoutingServiceNameValue values[] = {
        {"topic", "unit-topic"},
        {"rti.routing_service.entity.resource_name", "unit-resource"},
        {"client.id", "unit-client"}};
    struct RTI_RoutingServiceProperties properties = {values, 3, DDS_BOOLEAN_TRUE};
    struct RTI_RoutingServiceStreamReaderListener listener = {0};
    DDS_TypeCode *type = message_type();
    DDS_ExceptionCode_t ex;
    void *stream;
    const char *scenario = name + (reader_case ? strlen("create_reader_") : strlen("create_writer_"));
    int shutdown_failure = strcmp(scenario, "join_failure") == 0
            || strcmp(scenario, "stop_signal_failure") == 0
            || strcmp(scenario, "stop_wakeup_failure") == 0;
    int success = shutdown_failure || strcmp(scenario, "format_text") == 0 || strcmp(scenario, "format_binary") == 0
            || strcmp(scenario, "success") == 0 || strcmp(scenario, "group_override") == 0
            || strcmp(scenario, "secret") == 0 || strcmp(scenario, "pending_loan") == 0
            || strcmp(scenario, "undelivered") == 0 || strcmp(scenario, "close_error") == 0
            || strcmp(scenario, "delivery_success") == 0 || strcmp(scenario, "delivery_failure") == 0;
    listener.on_data_available = unused_listener;
    info.stream_name = "unit-stream";
    info.type_info.type_representation = type;

    if (strncmp(scenario, "allocation_", 11) == 0) hooks.fail_allocation = atoi(scenario + 11);
    else if (strcmp(scenario, "missing_topic") == 0) { values[0].name = "client.id"; }
    else if (strcmp(scenario, "missing_name") == 0) info.stream_name = NULL;
    else if (strcmp(scenario, "conf_allocation") == 0) kafka.fail_conf_new = 1;
    else if (strcmp(scenario, "invalid_bootstrap") == 0) kafka.fail_property = "bootstrap.servers";
    else if (strcmp(scenario, "invalid_group") == 0) kafka.fail_property = "group.id";
    else if (strcmp(scenario, "invalid_property") == 0) kafka.fail_property = "client.id";
    else if (strcmp(scenario, "client_failure") == 0) kafka.fail_new = 1;
    else if (strcmp(scenario, "redirect_failure") == 0) kafka.fail_redirect = 1;
    else if (strcmp(scenario, "subscription_allocation") == 0) kafka.fail_subscription_new = 1;
    else if (strcmp(scenario, "subscription_add") == 0) kafka.fail_subscription_add = 1;
    else if (strcmp(scenario, "subscribe_failure") == 0) kafka.fail_subscribe = 1;
    else if (strcmp(scenario, "support_failure") == 0) hooks.fail_support = 1;
    else if (strcmp(scenario, "sample_failure") == 0) hooks.fail_sample = 1;
    else if (strcmp(scenario, "semaphore_1") == 0) hooks.fail_semaphore = 1;
    else if (strcmp(scenario, "semaphore_2") == 0) hooks.fail_semaphore = 2;
    else if (strcmp(scenario, "semaphore_3") == 0) hooks.fail_semaphore = 3;
    else if (strcmp(scenario, "initial_give") == 0) hooks.fail_initial_give = 1;
    else if (strcmp(scenario, "thread_failure") == 0) hooks.fail_thread = 1;
    else if (strcmp(scenario, "group_override") == 0) { values[2].name = "group.id"; values[2].value = "custom-group"; }
    else if (strcmp(scenario, "secret") == 0) { values[2].name = "sasl.password"; values[2].value = "unit-secret"; }
    else if (strcmp(scenario, "undelivered") == 0) kafka.outq = 2;
    else if (strcmp(scenario, "close_error") == 0) kafka.fail_close = 1;

    if (strncmp(scenario, "format_", 7) == 0) {
        values[2].name = "payload_format";
        values[2].value = strcmp(scenario, "format_empty") == 0 ? ""
                : strcmp(scenario, "format_text") == 0 ? "text"
                : strcmp(scenario, "format_binary") == 0 ? "binary" : "invalid";
    }

    if (reader_case) {
        stream = RTI_RS_KafkaConnection_create_stream_reader(&connection, NULL, &info, &properties, &listener, NULL);
    } else {
        stream = RTI_RS_KafkaConnection_create_stream_writer(&connection, NULL, &info, &properties, NULL);
    }
    if (success) {
        CHECK(stream != NULL && hooks.errors == 0);
        CHECK(kafka.client_type == (reader_case ? RD_KAFKA_CONSUMER : RD_KAFKA_PRODUCER));
        CHECK(strcmp(last_config("bootstrap.servers"), "host:9092") == 0);
        CHECK(last_config("topic") == NULL && last_config("rti.routing_service.entity.resource_name") == NULL);
        if (strcmp(scenario, "success") == 0) CHECK(strcmp(last_config("client.id"), "unit-client") == 0);
        if (strcmp(scenario, "secret") == 0) {
            CHECK(strcmp(last_config("sasl.password"), "unit-secret") == 0);
            CHECK(hooks.sensitive_log == 0);
        }
        if (reader_case) {
            struct RTI_RS_KafkaStreamReader *reader = stream;
            struct RTINtpTime zero = {0, 0};
            CHECK(reader->listener.on_data_available == unused_listener);
            CHECK(reader->info_list[0]->valid_data && reader->info_list[0]->instance_state == DDS_ALIVE_INSTANCE_STATE);
            CHECK(strcmp(last_config("group.id"), strcmp(scenario, "group_override") == 0 ? "custom-group" : "1") == 0);
            CHECK(strcmp(kafka.subscribed_topic, "unit-topic") == 0 && kafka.subscribed_partition == RD_KAFKA_PARTITION_UA);
            CHECK(kafka.redirect_calls == 1 && hooks.semaphore_calls == 3);
            if (strcmp(scenario, "pending_loan") == 0 || shutdown_failure) {
                CHECK(RTIOsapiSemaphore_take(reader->poll_sem, &zero) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
                reader->rkm = calloc(1, sizeof(*reader->rkm));
                CHECK(reader->rkm != NULL);
                reader->rkm->payload = malloc(sizeof(protobuf_payload));
                CHECK(reader->rkm->payload != NULL);
                memcpy(reader->rkm->payload, protobuf_payload, sizeof(protobuf_payload));
                CHECK(DDS_OctetSeq_loan_contiguous(&reader->payload, reader->rkm->payload,
                        sizeof(protobuf_payload), sizeof(protobuf_payload)));
            }
            if (shutdown_failure) {
                void *thread = reader->polling_thread;
                int allocations = hooks.allocations;
                if (strcmp(scenario, "join_failure") == 0) hooks.fail_join = 1;
                else hooks.fail_give = strcmp(scenario, "stop_signal_failure") == 0
                        ? reader->stop_sem : reader->poll_sem;
                RTI_RS_KafkaConnection_delete_stream_reader(&connection, stream, NULL);
                CHECK(hooks.errors == 1 && hooks.error[0]);
                CHECK(reader->polling_thread == thread && hooks.threads == 1);
                CHECK(hooks.allocations == allocations && hooks.samples == 1
                        && hooks.supports == 1 && hooks.semaphores == 3);
                CHECK(kafka.clients == 1 && kafka.destroy_calls == 0
                        && kafka.close_calls == 0 && kafka.destroyed == 0);
                CHECK(reader->rkm != NULL && !DDS_OctetSeq_has_ownership(&reader->payload));
            }
            RTI_RS_KafkaConnection_delete_stream_reader(&connection, stream, NULL);
            CHECK(hooks.stops == 1 && kafka.close_calls == 1);
            if (strcmp(scenario, "pending_loan") == 0 || shutdown_failure) CHECK(kafka.destroyed == 1);
        } else {
            CHECK(kafka.delivery_cb != NULL);
            CHECK(((struct RTI_RS_KafkaStreamWriter *)stream)->text_payload
                    == (strcmp(scenario, "format_binary") != 0));
            CHECK(last_config("payload_format") == NULL);
            if (strncmp(scenario, "delivery_", 9) == 0) {
                rd_kafka_message_t message = {0};
                message.err = strcmp(scenario, "delivery_failure") == 0 ? RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE : RD_KAFKA_RESP_ERR_NO_ERROR;
                kafka.delivery_cb(((struct RTI_RS_KafkaStreamWriter *)stream)->rk, &message, NULL);
                CHECK(hooks.delivery_success_log == (message.err == 0));
                CHECK(hooks.delivery_failure_log == (message.err != 0));
            }
            RTI_RS_KafkaConnection_delete_stream_writer(&connection, stream, NULL);
            CHECK(kafka.flush_calls == 1);
            if (kafka.outq) CHECK(hooks.undelivered_log == 1);
        }
        CHECK(kafka.destroy_calls == 1);
    } else {
        CHECK(stream == NULL && hooks.errors > 0 && hooks.error[0]);
        if (kafka.fail_property) CHECK(strstr(hooks.error, "invalid %s property:") != NULL);
    }
    check_no_resources();
    DDS_TypeCodeFactory_delete_tc(DDS_TypeCodeFactory_get_instance(), type, &ex);
}

int test_lifecycle(const char *name)
{
    if (strncmp(name, "adapter_", 8) == 0) test_adapter(name);
    else if (strncmp(name, "connection_", 11) == 0) test_connection(name);
    else if (strncmp(name, "create_reader_", 14) == 0) test_stream_creation(name, 1);
    else if (strncmp(name, "create_writer_", 14) == 0) test_stream_creation(name, 0);
    else return 0;
    return 1;
}
