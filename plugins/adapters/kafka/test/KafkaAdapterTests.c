#include "FakeKafka.h"
#include "KafkaTestCases.h"

DDS_TypeCode *message_type(void)
{
    DDS_TypeCodeFactory *factory = DDS_TypeCodeFactory_get_instance();
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    struct DDS_StructMemberSeq members = DDS_SEQUENCE_INITIALIZER;
    DDS_TypeCode *sequence, *payload, *message;
    sequence = DDS_TypeCodeFactory_create_sequence_tc(factory, DDS_LENGTH_UNLIMITED,
            DDS_TypeCodeFactory_get_primitive_tc(factory, DDS_TK_OCTET), &ex);
    CHECK(sequence != NULL && ex == DDS_NO_EXCEPTION_CODE);
    payload = DDS_TypeCodeFactory_create_struct_tc(factory, "MessagePayload", &members, &ex);
    CHECK(payload != NULL && ex == DDS_NO_EXCEPTION_CODE);
    DDS_TypeCode_add_member(payload, "data", DDS_TYPECODE_MEMBER_ID_INVALID,
            sequence, DDS_TYPECODE_NONKEY_MEMBER, &ex);
    CHECK(ex == DDS_NO_EXCEPTION_CODE);
    message = DDS_TypeCodeFactory_create_struct_tc(factory, "Message", &members, &ex);
    CHECK(message != NULL && ex == DDS_NO_EXCEPTION_CODE);
    DDS_TypeCode_add_member(message, "payload", DDS_TYPECODE_MEMBER_ID_INVALID,
            payload, DDS_TYPECODE_NONKEY_MEMBER, &ex);
    CHECK(ex == DDS_NO_EXCEPTION_CODE);
    DDS_TypeCodeFactory_delete_tc(factory, sequence, &ex);
    DDS_TypeCodeFactory_delete_tc(factory, payload, &ex);
    DDS_StructMemberSeq_finalize(&members);
    return message;
}

DDS_DynamicData *new_sample(DDS_TypeCode *type,
        const unsigned char *bytes, DDS_Long length)
{
    struct DDS_DynamicDataProperty_t properties = DDS_DynamicDataProperty_t_INITIALIZER;
    struct DDS_OctetSeq sequence = DDS_SEQUENCE_INITIALIZER;
    DDS_DynamicData *sample = DDS_DynamicData_new(type, &properties);
    CHECK(sample != NULL);
    CHECK(DDS_OctetSeq_set_length(&sequence, length));
    if (length > 0) {
        memcpy(DDS_OctetSeq_get_contiguous_buffer(&sequence), bytes, (size_t) length);
    }
    CHECK(DDS_DynamicData_set_octet_seq(sample, "payload.data",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, &sequence) == DDS_RETCODE_OK);
    CHECK(DDS_OctetSeq_finalize(&sequence));
    return sample;
}

static void test_writer(const char *name)
{
    DDS_ExceptionCode_t ex;
    DDS_TypeCode *type = message_type();
    DDS_TypeCode *invalid_type = NULL;
    struct RTI_RS_KafkaStreamWriter writer = {NULL, "unit-topic", DDS_BOOLEAN_FALSE};
    DDS_DynamicData *samples[3];
    const unsigned char text[] = "hello";
    unsigned char nonterminated[1024];
    const unsigned char *bytes = text;
    DDS_Long length = sizeof(text) - 1;
    size_t expected_length = sizeof(text) - 1;
    int count = 1, expected_count = 1, i, result;

    if (strcmp(name, "writer_binary") == 0) {
        bytes = protobuf_payload;
        length = sizeof(protobuf_payload);
        expected_length = sizeof(protobuf_payload);
    } else if (strcmp(name, "writer_leading_zero") == 0 || strcmp(name, "writer_trailing_zero") == 0
            || strcmp(name, "writer_large") == 0) {
        for (i = 0; i < (int)sizeof(nonterminated); i++) nonterminated[i] = (unsigned char)i;
        length = expected_length = sizeof(nonterminated);
        if (strcmp(name, "writer_leading_zero") == 0) {
            memset(nonterminated, 'x', sizeof(nonterminated));
            nonterminated[0] = 0;
            length = expected_length = 5;
        } else if (strcmp(name, "writer_trailing_zero") == 0) {
            memset(nonterminated, 'x', sizeof(nonterminated));
            nonterminated[sizeof(nonterminated)-1] = 0;
        }
        bytes = nonterminated;
    } else if (strcmp(name, "writer_nonterminated") == 0) {
        memset(nonterminated, 'x', sizeof(nonterminated));
        bytes = nonterminated;
        length = sizeof(nonterminated);
        expected_length = sizeof(nonterminated);
    } else if (strcmp(name, "writer_empty") == 0) {
        length = 0;
        expected_length = 0;
    } else if (strcmp(name, "writer_batch") == 0) {
        count = expected_count = 3;
    } else if (strcmp(name, "writer_queue_full") == 0) {
        kafka.queue_full = 2;
    } else if (strcmp(name, "writer_retry_error") == 0) {
        kafka.queue_full = 2;
        kafka.fail_call = 3;
        expected_count = 0;
    } else if (strcmp(name, "writer_last_failure") == 0) {
        count = 3;
        kafka.fail_call = 3;
        expected_count = 2;
    } else if (strcmp(name, "writer_enqueue_error") == 0) {
        kafka.fail_call = 1;
        expected_count = 0;
    } else if (strcmp(name, "writer_partial_failure") == 0) {
        count = 3;
        kafka.fail_call = 2;
        expected_count = 1;
    } else if (strcmp(name, "writer_missing_later") == 0) {
        count = 3;
        expected_count = 1;
    } else if (strcmp(name, "writer_missing_payload") == 0) {
        expected_count = 0;
    } else if (strcmp(name, "writer_zero_count") == 0) {
        count = expected_count = 0;
    }
    if (strncmp(name, "writer_text_", 12) == 0) {
        static const unsigned char double_nul[] = {'a', 0, 0};
        static const unsigned char embedded[] = {'a', 0, 'b', 0};
        writer.text_payload = DDS_BOOLEAN_TRUE;
        length = sizeof(text);
        if (strcmp(name, "writer_text_nonterminated") == 0) length--;
        else if (strcmp(name, "writer_text_empty") == 0) length = 0;
        else if (strcmp(name, "writer_text_nul_only") == 0) { bytes = text + sizeof(text) - 1; length = 1; }
        else if (strcmp(name, "writer_text_double_nul") == 0) { bytes = double_nul; length = sizeof(double_nul); }
        else if (strcmp(name, "writer_text_embedded_zero") == 0) { bytes = embedded; length = sizeof(embedded); }
        else if (strcmp(name, "writer_text_retry") == 0) kafka.queue_full = 2;
        else if (strcmp(name, "writer_text_batch") == 0) count = expected_count = 3;
        expected_length = length;
        if (length > 0 && bytes[length - 1] == 0) expected_length--;
    } else if (strcmp(name, "writer_binary_terminated") == 0) {
        length = expected_length = sizeof(text);
    }
    for (i = 0; i < count; i++) {
        samples[i] = new_sample(type, bytes, length);
    }
    if (strcmp(name, "writer_missing_payload") == 0 || strcmp(name, "writer_missing_later") == 0) {
        int invalid_index = strcmp(name, "writer_missing_later") == 0 ? 1 : 0;
        struct DDS_DynamicDataProperty_t properties = DDS_DynamicDataProperty_t_INITIALIZER;
        struct DDS_StructMemberSeq members = DDS_SEQUENCE_INITIALIZER;
        DDS_TypeCodeFactory *factory = DDS_TypeCodeFactory_get_instance();
        invalid_type = DDS_TypeCodeFactory_create_struct_tc(
                factory, "InvalidMessage", &members, &ex);
        CHECK(invalid_type != NULL && ex == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCode_add_member(invalid_type, "other", DDS_TYPECODE_MEMBER_ID_INVALID,
                DDS_TypeCodeFactory_get_primitive_tc(factory, DDS_TK_LONG),
                DDS_TYPECODE_NONKEY_MEMBER, &ex);
        CHECK(ex == DDS_NO_EXCEPTION_CODE);
        DDS_StructMemberSeq_finalize(&members);
        DDS_DynamicData_delete(samples[invalid_index]);
        samples[invalid_index] = DDS_DynamicData_new(invalid_type, &properties);
        CHECK(samples[invalid_index] != NULL);
    }
    result = RTI_RS_KafkaStreamWriter_write(&writer,
            (const RTI_RoutingServiceSample *) samples, NULL, count, NULL);
    CHECK(result == expected_count);
    CHECK(kafka.accepted == expected_count);
    if (expected_count > 0) {
        for (i = 0; i < expected_count; i++) {
            CHECK(kafka.lengths[i] == expected_length);
            CHECK(memcmp(kafka.values[i], bytes, expected_length) == 0);
        }
    }
    if (strcmp(name, "writer_queue_full") == 0) {
        CHECK(kafka.produce_calls == 3);
        CHECK(kafka.poll_calls >= 2);
    }
    if (strcmp(name, "writer_retry_error") == 0) CHECK(kafka.produce_calls == 3);
    if (strcmp(name, "writer_repeated") == 0) {
        CHECK(RTI_RS_KafkaStreamWriter_write(&writer,
                (const RTI_RoutingServiceSample *) samples, NULL, count, NULL) == count);
        CHECK(kafka.accepted == 2 && kafka.lengths[1] == expected_length);
        CHECK(memcmp(kafka.values[1], bytes, expected_length) == 0);
    }
    if (strcmp(name, "writer_mixed_batch") == 0) {
        DDS_DynamicData *mixed[] = {new_sample(type, protobuf_payload, 7),
            new_sample(type, text, 0), new_sample(type, text, 1)};
        CHECK(RTI_RS_KafkaStreamWriter_write(&writer,
                (const RTI_RoutingServiceSample *) mixed, NULL, 3, NULL) == 3);
        CHECK(kafka.accepted == 4);
        CHECK(kafka.lengths[1] == 7 && kafka.lengths[2] == 0 && kafka.lengths[3] == 1);
        CHECK(memcmp(kafka.values[1], protobuf_payload, 7) == 0 && kafka.values[3][0] == text[0]);
        for (i = 0; i < 3; i++) DDS_DynamicData_delete(mixed[i]);
    }
    for (i = 0; i < count; i++) {
        DDS_DynamicData_delete(samples[i]);
    }
    DDS_TypeCodeFactory_delete_tc(DDS_TypeCodeFactory_get_instance(), type, &ex);
    if (invalid_type != NULL) {
        DDS_TypeCodeFactory_delete_tc(DDS_TypeCodeFactory_get_instance(), invalid_type, &ex);
    }
}

static void check_read(struct RTI_RS_KafkaStreamReader *reader, int empty)
{
    RTI_RoutingServiceSample *samples = NULL;
    RTI_RoutingServiceSampleInfo *infos = NULL;
    struct DDS_OctetSeq payload = DDS_SEQUENCE_INITIALIZER;
    int count = -1;
    RTI_RS_KafkaStreamReader_read(reader, &samples, &infos, &count, NULL);
    CHECK(count == 1 && samples[0] == reader->sample_list[0] && infos[0] == reader->info_list[0]);
    CHECK(DDS_DynamicData_get_octet_seq(samples[0], &payload, "payload.data",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED) == DDS_RETCODE_OK);
    CHECK(DDS_OctetSeq_get_length(&payload) == (empty ? 0 : sizeof(protobuf_payload)));
    if (!empty) CHECK(memcmp(DDS_OctetSeq_get_contiguous_buffer(&payload),
            protobuf_payload, sizeof(protobuf_payload)) == 0);
    CHECK(DDS_OctetSeq_finalize(&payload));
    RTI_RS_KafkaStreamReader_return_loan(reader, samples, infos, count, NULL);
    CHECK(reader->rkm == NULL);
    CHECK(DDS_OctetSeq_has_ownership(&reader->payload));
}

static void on_data_available(RTI_RoutingServiceStreamReader reader, void *data)
{
    CHECK(reader == kafka.reader);
    kafka.notifications++;
    if (data) check_read(reader, 0); /* Read synchronously from the notification. */
}

static void test_reader(const char *name)
{
    struct RTI_RS_KafkaStreamReader reader = {0};
    struct DDS_SampleInfo info = DDS_SAMPLEINFO_DEFAULT;
    struct DDS_SampleInfo *infos[] = {&info};
    DDS_TypeCode *type = message_type();
    DDS_DynamicData *samples[] = {new_sample(type, protobuf_payload, 0)};
    DDS_ExceptionCode_t ex;
    struct RTINtpTime zero = {0, 0};
    int empty = strcmp(name, "reader_empty") == 0 || strcmp(name, "reader_tombstone") == 0;
    int no_data = strcmp(name, "reader_stopped") == 0 || strcmp(name, "reader_idle") == 0;
    int read_failure = strcmp(name, "reader_invalid_sample") == 0;
    int synchronous = strcmp(name, "reader_synchronous") == 0;
    int errors = strcmp(name, "reader_error_recovery") == 0 ? 1 :
            (strcmp(name, "reader_repeated_errors") == 0 ? 3 : 0);
    int timeouts = strcmp(name, "reader_timeout") == 0 ? 1 :
            (strcmp(name, "reader_repeated_timeouts") == 0 ? 3 : 0);
    int iterations = strcmp(name, "reader_repeated_messages") == 0 ? 8 : 1;
    int i;

    kafka.reader = &reader;
    kafka.consumer_timeouts = timeouts;
    kafka.consumer_errors = errors;
    kafka.empty_message = empty;
    kafka.null_payload = strcmp(name, "reader_tombstone") == 0;
    kafka.stop_without_message = strcmp(name, "reader_idle") == 0;
    reader.sample_list = samples;
    reader.info_list = infos;
    reader.listener.on_data_available = on_data_available;
    reader.listener.listener_data = synchronous ? &reader : NULL;
    reader.poll_sem = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    reader.read_sem = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    reader.stop_sem = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    CHECK(reader.poll_sem != NULL && reader.read_sem != NULL && reader.stop_sem != NULL);
    if (strcmp(name, "reader_stopped") == 0) {
        CHECK(RTIOsapiSemaphore_give(reader.stop_sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    }
    CHECK(DDS_OctetSeq_initialize(&reader.payload));
    CHECK(RTIOsapiSemaphore_give(reader.poll_sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    if (read_failure) {
        struct DDS_DynamicDataProperty_t props = DDS_DynamicDataProperty_t_INITIALIZER;
        DDS_DynamicData_delete(samples[0]);
        /* MessagePayload has a data member but no payload.data path. */
        samples[0] = DDS_DynamicData_new(DDS_TypeCode_member_type(type, 0, &ex), &props);
        CHECK(samples[0] != NULL);
    }
    for (i = 0; i < iterations; i++) {
        RTI_RS_KafkaStreamReader_on_data_availabe_thread(&reader);
        CHECK(kafka.notifications == (no_data ? 0 : i + 1));
        if (!no_data && !read_failure && !synchronous) check_read(&reader, empty);
        if (no_data || read_failure || strcmp(name, "reader_read_twice") == 0) {
            RTI_RoutingServiceSample *loan_samples = (void *)1;
            RTI_RoutingServiceSampleInfo *loan_infos = (void *)1;
            int count = -1;
            RTI_RS_KafkaStreamReader_read(&reader, &loan_samples, &loan_infos, &count, NULL);
            CHECK(count == 0 && loan_samples == NULL && loan_infos == NULL);
            CHECK(reader.rkm == NULL);
        }
    }
    CHECK(kafka.consumer_calls == (strcmp(name, "reader_stopped") == 0 ? 0 :
            errors + timeouts + iterations));
    CHECK(kafka.destroyed == (no_data ? 0 : errors + iterations));
    CHECK(RTIOsapiSemaphore_take(reader.poll_sem, &zero) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    CHECK(DDS_OctetSeq_has_ownership(&reader.payload));
    CHECK(DDS_OctetSeq_finalize(&reader.payload));
    RTIOsapiSemaphore_delete(reader.poll_sem);
    RTIOsapiSemaphore_delete(reader.read_sem);
    RTIOsapiSemaphore_delete(reader.stop_sem);
    DDS_DynamicData_delete(samples[0]);
    DDS_TypeCodeFactory_delete_tc(DDS_TypeCodeFactory_get_instance(), type, &ex);
}

static void shutdown_notification(RTI_RoutingServiceStreamReader reader, void *data)
{
    (void)reader;
    kafka.notifications++;
    CHECK(RTIOsapiSemaphore_give(data) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
}

/* Run the actual worker on an SDK thread. Semaphores coordinate the test so
 * no sleeps or unsynchronized flags determine when shutdown is requested. */
static void test_reader_shutdown(const char *name)
{
    struct RTI_RS_KafkaStreamReader reader = {0};
    struct RTIOsapiJoinableThread *thread;
    struct RTINtpTime wait = {5, 0};
    struct RTIOsapiSemaphore *entered = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    struct RTIOsapiSemaphore *gate = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    int polling = strcmp(name, "reader_shutdown_polling") == 0;
    int pending = strcmp(name, "reader_shutdown_pending_loan") == 0;
    CHECK(entered != NULL && gate != NULL);
    reader.poll_sem = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    reader.read_sem = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    reader.stop_sem = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    CHECK(reader.poll_sem && reader.read_sem && reader.stop_sem);
    CHECK(DDS_OctetSeq_initialize(&reader.payload));
    reader.listener.on_data_available = shutdown_notification;
    reader.listener.listener_data = entered;
    kafka.reader = &reader;
    kafka.keep_running = 1;
    if (polling) {
        kafka.consumer_entered = entered;
        kafka.consumer_gate = gate;
    }
    if (polling || pending) {
        CHECK(RTIOsapiSemaphore_give(reader.poll_sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    }
    thread = RTIOsapiJoinableThread_new("KafkaStopTest",
            RTI_OSAPI_THREAD_PRIORITY_DEFAULT, RTI_OSAPI_THREAD_OPTION_DEFAULT,
            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT, NULL,
            RTI_RS_KafkaStreamReader_on_data_availabe_thread, &reader);
    CHECK(thread != NULL);
    if (polling || pending) {
        CHECK(RTIOsapiSemaphore_take(entered, &wait) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    }
    CHECK(RTIOsapiSemaphore_give(reader.stop_sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    CHECK(RTIOsapiSemaphore_give(reader.poll_sem) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    if (polling) CHECK(RTIOsapiSemaphore_give(gate) == RTI_OSAPI_SEMAPHORE_STATUS_OK);
    CHECK(RTIOsapiJoinableThread_stopAndDelete(thread, 2000));
    CHECK(kafka.consumer_calls == (polling || pending ? 1 : 0));
    CHECK(kafka.notifications == (pending ? 1 : 0));
    if (pending) {
        CHECK(reader.rkm != NULL && !DDS_OctetSeq_has_ownership(&reader.payload));
        CHECK(memcmp(reader.rkm->payload, protobuf_payload, sizeof(protobuf_payload)) == 0);
        RTI_RS_KafkaStreamReader_return_loan(&reader, NULL, NULL, 0, NULL);
        CHECK(kafka.destroyed == 1 && reader.rkm == NULL);
    }
    CHECK(DDS_OctetSeq_finalize(&reader.payload));
    RTIOsapiSemaphore_delete(reader.poll_sem);
    RTIOsapiSemaphore_delete(reader.read_sem);
    RTIOsapiSemaphore_delete(reader.stop_sem);
    RTIOsapiSemaphore_delete(entered);
    RTIOsapiSemaphore_delete(gate);
}

int main(int argc, char **argv)
{
    size_t i;
    CHECK(argc == 2);
    for (i = 0; i < sizeof(kafka_test_cases) / sizeof(kafka_test_cases[0]); i++) {
        if (strcmp(argv[1], kafka_test_cases[i]) == 0) break;
    }
    CHECK(i < sizeof(kafka_test_cases) / sizeof(kafka_test_cases[0]));
    if (strncmp(argv[1], "writer_", 7) == 0) {
        test_writer(argv[1]);
    } else if (strncmp(argv[1], "reader_shutdown_", 16) == 0) {
        test_reader_shutdown(argv[1]);
    } else if (strncmp(argv[1], "reader_", 7) == 0) {
        test_reader(argv[1]);
    } else {
        CHECK(test_lifecycle(argv[1]));
    }
    printf("PASS %s\n", argv[1]);
    return EXIT_SUCCESS;
}
