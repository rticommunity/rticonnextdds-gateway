#ifndef KAFKA_TEST_RDKAFKA_H
#define KAFKA_TEST_RDKAFKA_H

/* Only the librdkafka surface used by the real stream reader and writer.
 * The tests provide these functions, so no Kafka library or broker is needed.
 */
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

typedef struct rd_kafka_s rd_kafka_t;
typedef struct rd_kafka_topic_s rd_kafka_topic_t;
typedef struct rd_kafka_conf_s rd_kafka_conf_t;
typedef enum { RD_KAFKA_PRODUCER, RD_KAFKA_CONSUMER } rd_kafka_type_t;
typedef enum { RD_KAFKA_CONF_UNKNOWN = -2, RD_KAFKA_CONF_INVALID = -1,
    RD_KAFKA_CONF_OK = 0 } rd_kafka_conf_res_t;
typedef enum {
    RD_KAFKA_RESP_ERR__QUEUE_FULL = -184,
    RD_KAFKA_RESP_ERR_NO_ERROR = 0,
    RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE = 10
} rd_kafka_resp_err_t;

typedef struct rd_kafka_message_s {
    rd_kafka_resp_err_t err;
    rd_kafka_topic_t *rkt;
    int32_t partition;
    void *payload;
    size_t len;
    void *key;
    size_t key_len;
    int64_t offset;
    void *_private;
} rd_kafka_message_t;

typedef void (*kafka_test_delivery_cb)(rd_kafka_t *, const rd_kafka_message_t *, void *);
typedef struct rd_kafka_topic_partition_s { char *topic; int32_t partition; }
    rd_kafka_topic_partition_t;
typedef struct rd_kafka_topic_partition_list_s {
    int cnt;
    rd_kafka_topic_partition_t elems[1];
} rd_kafka_topic_partition_list_t;
#define RD_KAFKA_PARTITION_UA -1

rd_kafka_conf_t *rd_kafka_conf_new(void);
void rd_kafka_conf_destroy(rd_kafka_conf_t *conf);
rd_kafka_conf_res_t rd_kafka_conf_set(rd_kafka_conf_t *conf, const char *name,
        const char *value, char *errstr, size_t errstr_size);
void rd_kafka_conf_set_dr_msg_cb(rd_kafka_conf_t *conf, kafka_test_delivery_cb cb);
rd_kafka_t *rd_kafka_new(rd_kafka_type_t type, rd_kafka_conf_t *conf,
        char *errstr, size_t errstr_size);
void rd_kafka_destroy(rd_kafka_t *rk);
rd_kafka_resp_err_t rd_kafka_flush(rd_kafka_t *rk, int timeout_ms);
int rd_kafka_outq_len(rd_kafka_t *rk);
rd_kafka_resp_err_t rd_kafka_poll_set_consumer(rd_kafka_t *rk);
rd_kafka_topic_partition_list_t *rd_kafka_topic_partition_list_new(int size);
rd_kafka_topic_partition_t *rd_kafka_topic_partition_list_add(
        rd_kafka_topic_partition_list_t *list, const char *topic, int32_t partition);
void rd_kafka_topic_partition_list_destroy(rd_kafka_topic_partition_list_t *list);
rd_kafka_resp_err_t rd_kafka_subscribe(rd_kafka_t *rk,
        const rd_kafka_topic_partition_list_t *list);
rd_kafka_resp_err_t rd_kafka_consumer_close(rd_kafka_t *rk);

/* Match the tagged variadic argument convention of rd_kafka_producev(). */
enum kafka_test_vtype { KAFKA_TEST_END, KAFKA_TEST_TOPIC,
    KAFKA_TEST_MSGFLAGS, KAFKA_TEST_VALUE, KAFKA_TEST_OPAQUE };
#define RD_KAFKA_V_TOPIC(topic) KAFKA_TEST_TOPIC, (const char *) (topic)
#define RD_KAFKA_V_MSGFLAGS(flags) KAFKA_TEST_MSGFLAGS, (int) (flags)
#define RD_KAFKA_V_VALUE(value, size) \
    KAFKA_TEST_VALUE, (void *) (value), (size_t) (size)
#define RD_KAFKA_V_OPAQUE(opaque) KAFKA_TEST_OPAQUE, (void *) (opaque)
#define RD_KAFKA_V_END KAFKA_TEST_END
#define RD_KAFKA_MSG_F_COPY 0x2

rd_kafka_resp_err_t rd_kafka_producev(rd_kafka_t *rk, ...);
int rd_kafka_poll(rd_kafka_t *rk, int timeout_ms);
const char *rd_kafka_err2str(rd_kafka_resp_err_t err);
rd_kafka_message_t *rd_kafka_consumer_poll(rd_kafka_t *rk, int timeout_ms);
void rd_kafka_message_destroy(rd_kafka_message_t *message);
const char *rd_kafka_message_errstr(const rd_kafka_message_t *message);
const char *rd_kafka_topic_name(const rd_kafka_topic_t *topic);

#endif
