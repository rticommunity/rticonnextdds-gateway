#include "FakeKafka.h"
#include <stdarg.h>

struct FakeKafka kafka;


const unsigned char protobuf_payload[] = {0x0a, 0x03, 'A', 0, 'B', 0x10, 1};

rd_kafka_resp_err_t rd_kafka_producev(rd_kafka_t *rk, ...)
{
    va_list args;
    int tag, flags = 0;
    const char *topic = NULL;
    void *value = NULL;
    size_t length = 0;
    (void) rk;
    va_start(args, rk);
    while ((tag = va_arg(args, int)) != KAFKA_TEST_END) {
        switch (tag) {
        case KAFKA_TEST_TOPIC: topic = va_arg(args, const char *); break;
        case KAFKA_TEST_MSGFLAGS: flags = va_arg(args, int); break;
        case KAFKA_TEST_VALUE:
            value = va_arg(args, void *);
            length = va_arg(args, size_t);
            break;
        case KAFKA_TEST_OPAQUE: (void) va_arg(args, void *); break;
        default: CHECK(0);
        }
    }
    va_end(args);
    CHECK(topic != NULL && strcmp(topic, "unit-topic") == 0);
    CHECK(flags & RD_KAFKA_MSG_F_COPY);
    kafka.produce_calls++;
    if (kafka.queue_full > 0) {
        kafka.queue_full--;
        return RD_KAFKA_RESP_ERR__QUEUE_FULL;
    }
    if (kafka.produce_calls == kafka.fail_call) {
        return RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE;
    }
    CHECK(value != NULL); /* Empty values must not become Kafka tombstones. */
    CHECK(kafka.accepted < 16 && length <= sizeof(kafka.values[0]));
    kafka.lengths[kafka.accepted] = length;
    if (length > 0) {
        memcpy(kafka.values[kafka.accepted], value, length);
    }
    kafka.accepted++;
    return RD_KAFKA_RESP_ERR_NO_ERROR;
}

int rd_kafka_poll(rd_kafka_t *rk, int timeout_ms)
{
    (void) rk;
    CHECK(timeout_ms == 0 || timeout_ms == 10);
    kafka.poll_calls++;
    return 0;
}

const char *rd_kafka_err2str(rd_kafka_resp_err_t err)
{
    (void) err;
    return "injected Kafka error";
}

rd_kafka_message_t *rd_kafka_consumer_poll(rd_kafka_t *rk, int timeout_ms)
{
    rd_kafka_message_t *message;
    (void) rk;
    CHECK(timeout_ms == CONSUMER_POLL_TIMEOUT);
    kafka.consumer_calls++;
    if (kafka.consumer_gate != NULL) {
        CHECK(RTIOsapiSemaphore_give(kafka.consumer_entered) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
        CHECK(RTIOsapiSemaphore_take(kafka.consumer_gate, NULL) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
        return NULL;
    }
    if (kafka.stop_without_message) {
        CHECK(RTIOsapiSemaphore_give(kafka.reader->stop_sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
        return NULL;
    }
    if (kafka.consumer_timeout || kafka.consumer_timeouts > 0) {
        kafka.consumer_timeout = 0;
        if (kafka.consumer_timeouts > 0) kafka.consumer_timeouts--;
        return NULL;
    }
    message = calloc(1, sizeof(*message));
    CHECK(message != NULL);
    if (kafka.consumer_error || kafka.consumer_errors > 0) {
        kafka.consumer_error = 0;
        if (kafka.consumer_errors > 0) kafka.consumer_errors--;
        message->err = RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE;
    } else {
        message->payload = malloc(sizeof(protobuf_payload));
        CHECK(message->payload != NULL);
        memcpy(message->payload, protobuf_payload, sizeof(protobuf_payload));
        message->len = kafka.empty_message ? 0 : sizeof(protobuf_payload);
        if (kafka.null_payload) {
            free(message->payload);
            message->payload = NULL;
        }
        /* Most stream cases stop after one message. Real-thread shutdown
         * cases leave stopping to the controlling thread. */
        if (!kafka.keep_running) {
            CHECK(RTIOsapiSemaphore_give(kafka.reader->stop_sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
        }
    }
    return message;
}

void rd_kafka_message_destroy(rd_kafka_message_t *message)
{
    kafka.destroyed++;
    free(message->payload);
    free(message);
}

const char *rd_kafka_message_errstr(const rd_kafka_message_t *message)
{
    return rd_kafka_err2str(message->err);
}

const char *rd_kafka_topic_name(const rd_kafka_topic_t *topic)
{
    (void) topic;
    return "unit-topic";
}

struct rd_kafka_conf_s { kafka_test_delivery_cb callback; };
struct rd_kafka_s { rd_kafka_conf_t *conf; };

rd_kafka_conf_t *rd_kafka_conf_new(void)
{
    rd_kafka_conf_t *conf;
    if (kafka.fail_conf_new) return NULL;
    conf = calloc(1, sizeof(*conf));
    CHECK(conf != NULL);
    kafka.configurations++;
    return conf;
}

void rd_kafka_conf_destroy(rd_kafka_conf_t *conf)
{
    CHECK(conf != NULL && kafka.configurations > 0);
    kafka.configurations--;
    free(conf);
}

rd_kafka_conf_res_t rd_kafka_conf_set(rd_kafka_conf_t *conf, const char *name,
        const char *value, char *errstr, size_t errstr_size)
{
    int i = kafka.config_count++;
    CHECK(conf != NULL && name != NULL && value != NULL && i < 16);
    snprintf(kafka.config_names[i], sizeof(kafka.config_names[i]), "%s", name);
    snprintf(kafka.config_values[i], sizeof(kafka.config_values[i]), "%s", value);
    if (kafka.fail_property && strcmp(name, kafka.fail_property) == 0) {
        /* Literal percent signs must survive error propagation. */
        snprintf(errstr, errstr_size, "invalid %%s property: %s", name);
        return RD_KAFKA_CONF_INVALID;
    }
    return RD_KAFKA_CONF_OK;
}

void rd_kafka_conf_set_dr_msg_cb(rd_kafka_conf_t *conf, kafka_test_delivery_cb cb)
{
    CHECK(conf != NULL && cb != NULL);
    conf->callback = cb;
    kafka.delivery_cb = cb;
}

rd_kafka_t *rd_kafka_new(rd_kafka_type_t type, rd_kafka_conf_t *conf,
        char *errstr, size_t errstr_size)
{
    rd_kafka_t *client;
    CHECK(conf != NULL);
    kafka.new_calls++;
    kafka.client_type = type;
    if (kafka.fail_new) {
        snprintf(errstr, errstr_size, "injected client creation failure");
        return NULL; /* librdkafka consumes conf only on success. */
    }
    client = calloc(1, sizeof(*client));
    CHECK(client != NULL);
    client->conf = conf;
    kafka.clients++;
    return client;
}

void rd_kafka_destroy(rd_kafka_t *rk)
{
    CHECK(rk != NULL && kafka.clients > 0);
    rd_kafka_conf_destroy(rk->conf);
    kafka.clients--;
    kafka.destroy_calls++;
    free(rk);
}

rd_kafka_resp_err_t rd_kafka_flush(rd_kafka_t *rk, int timeout_ms)
{
    CHECK(rk != NULL && timeout_ms == 10000);
    kafka.flush_calls++;
    return kafka.outq ? RD_KAFKA_RESP_ERR__QUEUE_FULL : RD_KAFKA_RESP_ERR_NO_ERROR;
}

int rd_kafka_outq_len(rd_kafka_t *rk)
{
    CHECK(rk != NULL);
    return kafka.outq;
}

rd_kafka_resp_err_t rd_kafka_poll_set_consumer(rd_kafka_t *rk)
{
    CHECK(rk != NULL);
    kafka.redirect_calls++;
    return kafka.fail_redirect ? RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE : RD_KAFKA_RESP_ERR_NO_ERROR;
}

rd_kafka_topic_partition_list_t *rd_kafka_topic_partition_list_new(int size)
{
    rd_kafka_topic_partition_list_t *list;
    CHECK(size == 1);
    if (kafka.fail_subscription_new) return NULL;
    list = calloc(1, sizeof(*list));
    CHECK(list != NULL);
    kafka.subscriptions++;
    return list;
}

rd_kafka_topic_partition_t *rd_kafka_topic_partition_list_add(
        rd_kafka_topic_partition_list_t *list, const char *topic, int32_t partition)
{
    CHECK(list != NULL && list->cnt == 0);
    if (kafka.fail_subscription_add) return NULL;
    list->elems[0].topic = malloc(strlen(topic) + 1);
    CHECK(list->elems[0].topic != NULL);
    strcpy(list->elems[0].topic, topic);
    list->elems[0].partition = partition;
    list->cnt = 1;
    return &list->elems[0];
}

void rd_kafka_topic_partition_list_destroy(rd_kafka_topic_partition_list_t *list)
{
    CHECK(list != NULL && kafka.subscriptions > 0);
    free(list->elems[0].topic);
    free(list);
    kafka.subscriptions--;
}

rd_kafka_resp_err_t rd_kafka_subscribe(rd_kafka_t *rk,
        const rd_kafka_topic_partition_list_t *list)
{
    CHECK(rk != NULL && list != NULL && list->cnt == 1);
    snprintf(kafka.subscribed_topic, sizeof(kafka.subscribed_topic), "%s", list->elems[0].topic);
    kafka.subscribed_partition = list->elems[0].partition;
    return kafka.fail_subscribe ? RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE : RD_KAFKA_RESP_ERR_NO_ERROR;
}

rd_kafka_resp_err_t rd_kafka_consumer_close(rd_kafka_t *rk)
{
    CHECK(rk != NULL);
    kafka.close_calls++;
    return kafka.fail_close ? RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE : RD_KAFKA_RESP_ERR_NO_ERROR;
}
