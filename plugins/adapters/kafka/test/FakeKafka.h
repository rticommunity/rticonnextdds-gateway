#ifndef KAFKA_TEST_FAKE_KAFKA_H
#define KAFKA_TEST_FAKE_KAFKA_H
#include "KafkaStreamReader.h"
#include "KafkaStreamWriter.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)
struct FakeKafka {
    int produce_calls;
    int accepted;
    int poll_calls;
    int queue_full;
    int fail_call;
    unsigned char values[16][1024];
    size_t lengths[16];
    struct RTI_RS_KafkaStreamReader *reader;
    int consumer_calls;
    int consumer_timeout;
    int consumer_error;
    int destroyed;
    int notifications;
    int empty_message;
    int null_payload;
    int consumer_errors;
    int consumer_timeouts;
    int stop_without_message;
    int keep_running;
    struct RTIOsapiSemaphore *consumer_entered;
    struct RTIOsapiSemaphore *consumer_gate;
    int configurations;
    int clients;
    int subscriptions;
    int new_calls;
    int destroy_calls;
    int close_calls;
    int flush_calls;
    int redirect_calls;
    int outq;
    int fail_conf_new;
    int fail_new;
    int fail_subscription_new;
    int fail_subscription_add;
    int fail_subscribe;
    int fail_redirect;
    int fail_close;
    const char *fail_property;
    char config_names[16][80];
    char config_values[16][128];
    int config_count;
    rd_kafka_type_t client_type;
    kafka_test_delivery_cb delivery_cb;
    char subscribed_topic[128];
    int32_t subscribed_partition;
};
extern struct FakeKafka kafka;
extern const unsigned char protobuf_payload[7];
DDS_TypeCode *message_type(void);
DDS_DynamicData *new_sample(DDS_TypeCode *type, const unsigned char *bytes, DDS_Long length);
int test_lifecycle(const char *name);
#endif
