/******************************************************************************/
/* (c) 2021 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
/*                                                                            */
/* RTI grants Licensee a license to use, modify, compile, and create          */
/* derivative works of the software solely for use with RTI Connext DDS.      */
/* Licensee may redistribute copies of the software provided that all such    */
/* copies are subject to this license.                                        */
/* The software is provided "as is", with no warranty of any type, including  */
/* any warranty for fitness for any purpose. RTI is under no obligation to    */
/* maintain or support the software.  RTI shall not be liable for any         */
/* incidental or consequential damages arising out of the use or inability to */
/* use the software.                                                          */
/*                                                                            */
/******************************************************************************/

#include "KafkaConnection.h"

/**
 * @brief Message delivery report callback.
 *
 * This callback is called exactly once per message, indicating if
 * the message was succesfully delivered
 * (rkmessage->err == RD_KAFKA_RESP_ERR_NO_ERROR) or permanently
 * failed delivery (rkmessage->err != RD_KAFKA_RESP_ERR_NO_ERROR).
 *
 * The callback is triggered from rd_kafka_poll() and executes on
 * the application's thread.
 */
static void dr_msg_cb(
        rd_kafka_t *rk,
        const rd_kafka_message_t *rkmessage,
        void *opaque)
{
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);
    if (rkmessage->err) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "Message delivery failed: %s",
                rd_kafka_err2str(rkmessage->err));
    } else {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                "Message delivered (%zd bytes, "
                "partition %d)",
                rkmessage->len,
                rkmessage->partition);
    }

    // The rkmessage is destroyed automatically by librdkafka
}

static uint64_t json_parse_fields(
        const char *json,
        const char **end,
        const char *field1,
        const char *field2)
{
    const char *t = json;
    const char *t2;
    int len1 = (int) strlen(field1);
    int len2 = (int) strlen(field2);

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    while ((t2 = strstr(t, field1))) {
        uint64_t v;

        t = t2;
        t += len1;

        /* Find field */
        if (!(t2 = strstr(t, field2)))
            continue;
        t2 += len2;

        while (isspace((int) *t2))
            t2++;

        v = strtoull(t2, (char **) &t, 10);
        if (t2 == t)
            continue;

        *end = t;
        return v;
    }

    *end = t + strlen(t);
    return 0;
}

static void json_parse_stats(const char *json)
{
    const char *t;
#define MAX_AVGS 100 /* max number of brokers to scan for rtt */
    uint64_t avg_rtt[MAX_AVGS + 1];
    int avg_rtt_i = 0;
    /* Store totals at end of array */
    avg_rtt[MAX_AVGS] = 0;
    /* Extract all broker RTTs */
    t = json;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    while (avg_rtt_i < MAX_AVGS && *t) {
        avg_rtt[avg_rtt_i] = json_parse_fields(t, &t, "\"rtt\":", "\"avg\":");

        /* Skip low RTT values, means no messages are passing */
        if (avg_rtt[avg_rtt_i] < 100 /*0.1ms*/)
            continue;

        avg_rtt[MAX_AVGS] += avg_rtt[avg_rtt_i];
        avg_rtt_i++;
    }

    if (avg_rtt_i > 0)
        avg_rtt[MAX_AVGS] /= avg_rtt_i;
}

static int stats_cb(rd_kafka_t *rk, char *json, size_t json_len, void *opaque)
{
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);
    /* Extract values for our own stats */

    json_parse_stats(json);
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            json);

    return 0;
}

void delete_stream_writer(struct RTI_RS_KafkaStreamWriter *self)
{
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);
    
    /* Free allocated resources */
    if (self->rk != NULL) {
        rd_kafka_destroy(self->rk);
        self->rk = NULL;
    }

    if (self != NULL) {
        free(self);
        self = NULL;
    }
}

RTI_RoutingServiceStreamWriter RTI_RS_KafkaConnection_create_stream_writer(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceSession session,
        const struct RTI_RoutingServiceStreamInfo *stream_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_KafkaConnection *kafka_connection =
            (struct RTI_RS_KafkaConnection *) connection;
    struct RTI_RS_KafkaStreamWriter *stream_writer = NULL;
    char errstr[ERR_MSG_BUF_SIZE]; /* librdkafka API error reporting buffer */
    int i = 0;
    rd_kafka_conf_res_t res;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    stream_writer = calloc(1, sizeof(struct RTI_RS_KafkaStreamWriter));
    if (stream_writer == NULL) {
        RTI_RoutingServiceEnvironment_set_error(
                env,
                "RTI_RS_KafkaStreamWriter memory allocation error");
        goto error;
    }

    if (stream_info->stream_name == NULL) {
        RTI_RoutingServiceEnvironment_set_error(env, "stream_name is null");
        goto error;
    }

    /* Set a topic name from <route>/<output>/<property> */
    stream_writer->topic =
            RTI_RoutingServiceProperties_lookup_property(properties, "topic");
    if (stream_writer->topic == NULL) {
        RTI_RoutingServiceEnvironment_set_error(env, "topic name missing");
        goto error;
    }

    stream_writer->conf = rd_kafka_conf_new();

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(
                stream_writer->conf,
                "bootstrap.servers",
                kafka_connection->bootstrap_servers,
                errstr,
                sizeof(errstr))
        != RD_KAFKA_CONF_OK) {
        RTI_RoutingServiceEnvironment_set_error(env, errstr);
        goto error;
    }

    /* Set Kafka configurations from <route>/<output>/<property> */
    for (i = 0; i < properties->count; i++) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_INFO,
                "property name[%d]: %s\n",
                i,
                properties->properties[i].name);
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_INFO,
                "property value[%d]: %s\n",
                i,
                properties->properties[i].value);
        // Skip RTI Connext configurations
        if ((strcmp(properties->properties[i].name, "topic") == 0)
            || (strcmp(properties->properties[i].name,
                       "rti.routing_service.entity.resource_name")
                == 0)) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_INFO,
                    "%s is skipped\n",
                    properties->properties[i].name);
        } else {
            res = rd_kafka_conf_set(
                    stream_writer->conf,
                    properties->properties[i].name,
                    properties->properties[i].value,
                    errstr,
                    sizeof(errstr));
            if (res != RD_KAFKA_CONF_OK) {
                RTI_RoutingServiceEnvironment_set_error(env, errstr);
                goto error;
            }
        }
    }

    /* Set the delivery report callback.
     * This callback will be called once per message to inform
     * the application if delivery succeeded or failed.
     * See dr_msg_cb() above.
     * The callback is only triggered from rd_kafka_poll() and
     * rd_kafka_flush(). */
    rd_kafka_conf_set_dr_msg_cb(stream_writer->conf, dr_msg_cb);

    /* Set the statistics callback. */
    /*
    rd_kafka_conf_set_stats_cb(stream_writer->conf, stats_cb);

    if (rd_kafka_conf_set(stream_writer->conf, "statistics.interval.ms",
                          "1000",
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        RTI_RoutingServiceEnvironment_set_error(env, errstr);
        return NULL;
    }*/

    /*
     * Create producer instance.
     *
     * NOTE: rd_kafka_new() takes ownership of the conf object
     *       and the application must not reference it again after
     *       this call.
     */
    stream_writer->rk = rd_kafka_new(
            RD_KAFKA_PRODUCER,
            stream_writer->conf,
            errstr,
            sizeof(errstr));
    if (!stream_writer->rk) {
        RTI_RoutingServiceEnvironment_set_error(env, errstr);
        goto error;
    }

    return stream_writer;

error:
    delete_stream_writer(stream_writer);
    return NULL;
}

void RTI_RS_KafkaConnection_delete_stream_writer(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceStreamWriter stream_writer,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_KafkaStreamWriter *self =
            (struct RTI_RS_KafkaStreamWriter *) stream_writer;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    /* Wait for final messages to be delivered or fail.
     * rd_kafka_flush() is an abstraction over rd_kafka_poll() which
     * waits for all messages to be delivered. */
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_INFO,
            "Flushing final messages...");
    rd_kafka_flush(self->rk, 10 * 1000 /* wait for max 10 seconds */);

    /* If the output queue is still not empty there is an issue
     * with producing messages to the clusters. */
    if (rd_kafka_outq_len(self->rk) > 0) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "%d message(s) were not delivered",
                rd_kafka_outq_len(self->rk));
    }

    delete_stream_writer(stream_writer);
}

void delete_stream_reader(struct RTI_RS_KafkaStreamReader *self)
{
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);
    
    /* Free allocated resources */
    if (self->rk != NULL) {
        rd_kafka_destroy(self->rk);
    }

    // TODO: Check self->payload is not NULL?
    DDS_OctetSeq_finalize(&self->payload);

    if (self->poll_sem != NULL) {
        RTIOsapiSemaphore_delete(self->poll_sem);
    }

    if (self->read_sem != NULL) {
        RTIOsapiSemaphore_delete(self->read_sem);
    }

    if (self->sample_list != NULL) {
        free(self->sample_list);
        self->sample_list = NULL;
    }

    if (self->info_list != NULL) {
        free(self->info_list);
        self->info_list = NULL;
    }

    if (self != NULL) {
        free(self);
    }
}

RTI_RoutingServiceStreamReader RTI_RS_KafkaConnection_create_stream_reader(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceSession session,
        const struct RTI_RoutingServiceStreamInfo *stream_info,
        const struct RTI_RoutingServiceProperties *properties,
        const struct RTI_RoutingServiceStreamReaderListener *listener,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_KafkaConnection *kafka_connection =
            (struct RTI_RS_KafkaConnection *) connection;
    struct RTI_RS_KafkaStreamReader *stream_reader = NULL;
    char errstr[ERR_MSG_BUF_SIZE]; /* librdkafka API error reporting buffer */
    int error = 0;
    rd_kafka_topic_partition_list_t *subscription; /* Subscribed topics */
    struct DDS_DynamicDataProperty_t dynamicDataProps =
            DDS_DynamicDataProperty_t_INITIALIZER;
    int i = 0;
    rd_kafka_conf_res_t res;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    stream_reader = calloc(1, sizeof(struct RTI_RS_KafkaStreamReader));
    if (stream_reader == NULL) {
        RTI_RoutingServiceEnvironment_set_error(
                env,
                "RTI_RS_KafkaStreamReader memory allocation error");
        goto error;
    }

    stream_reader->listener = *listener;
    stream_reader->type_code =
            (struct DDS_TypeCode *) stream_info->type_info.type_representation;
    stream_reader->run_thread = RTI_TRUE;

    stream_reader->sample_list = calloc(1, sizeof(DDS_DynamicData *));
    stream_reader->info_list = calloc(1, sizeof(struct DDS_SampleInfo *));

    stream_reader->sample_list[0] =
            DDS_DynamicData_new(stream_reader->type_code, &dynamicDataProps);
    stream_reader->info_list[0] =
            (struct DDS_SampleInfo *) calloc(1, sizeof(struct DDS_SampleInfo));
    if (stream_reader->info_list[0] == NULL) {
        RTI_RoutingServiceEnvironment_set_error(
                env,
                "Failure creating sample info");
        return NULL;
    }

    *(stream_reader->info_list[0]) = DDS_SAMPLEINFO_DEFAULT;
    stream_reader->info_list[0]->instance_handle = DDS_HANDLE_NIL;
    stream_reader->info_list[0]->valid_data = 1;
    stream_reader->info_list[0]->instance_state = DDS_ALIVE_INSTANCE_STATE;

    /*
     * Get the configuration properties in <route>/<input>/<property>
     */

    stream_reader->topic =
            RTI_RoutingServiceProperties_lookup_property(properties, "topic");
    if (stream_reader->topic == NULL) {
        RTI_RoutingServiceEnvironment_set_error(env, "topic missing");
        goto error;
    }

    stream_reader->conf = rd_kafka_conf_new();
    if (!DDS_OctetSeq_initialize(&stream_reader->payload)) {
        RTI_RoutingServiceEnvironment_set_error(env, "DDS_OctetSeq_initialize error");
        goto error;       
    }

    // TODO this 63k should be configurable
    if (!DDS_OctetSeq_ensure_length(&stream_reader->payload, 63000, 63000)) {
        RTI_RoutingServiceEnvironment_set_error(env, "DDS_OctetSeq_ensure_length error");
        goto error;       
    }

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(
                stream_reader->conf,
                "bootstrap.servers",
                kafka_connection->bootstrap_servers,
                errstr,
                sizeof(errstr))
        != RD_KAFKA_CONF_OK) {
        RTI_RoutingServiceEnvironment_set_error(env, errstr);
        goto error;
    }

    /* Set the consumer group id.
     * All consumers sharing the same group id will join the same
     * group, and the subscribed topic' partitions will be assigned
     * according to the partition.assignment.strategy
     * (consumer config property) to the consumers in the group. */
    if (rd_kafka_conf_set(
                stream_reader->conf,
                "group.id",
                "1",
                errstr,
                sizeof(errstr))
        != RD_KAFKA_CONF_OK) {
        RTI_RoutingServiceEnvironment_set_error(env, errstr);
        goto error;
    }

    /* Set Kafka configurations from <route>/<input>/<property> */
    for (i = 0; i < properties->count; i++) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_INFO,
                "property name[%d]: %s\n",
                i,
                properties->properties[i].name);
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_INFO,
                "property value[%d]: %s\n",
                i,
                properties->properties[i].value);
        // Skip RTI Connext configurations
        if ((strcmp(properties->properties[i].name, "topic") == 0)
            || (strcmp(properties->properties[i].name,
                       "rti.routing_service.entity.resource_name")
                == 0)) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_INFO,
                    "%s is skipped\n",
                    properties->properties[i].name);
        } else {
            res = rd_kafka_conf_set(
                    stream_reader->conf,
                    properties->properties[i].name,
                    properties->properties[i].value,
                    errstr,
                    sizeof(errstr));
            if (res != RD_KAFKA_CONF_OK) {
                RTI_RoutingServiceEnvironment_set_error(env, errstr);
                goto error;
            }
        }
    }

    /*
     * Create consumer instance.
     *
     * NOTE: rd_kafka_new() takes ownership of the conf object
     *       and the application must not reference it again after
     *       this call.
     */
    stream_reader->rk = rd_kafka_new(
            RD_KAFKA_CONSUMER,
            stream_reader->conf,
            errstr,
            sizeof(errstr));
    if (!stream_reader->rk) {
        RTI_RoutingServiceEnvironment_set_error(env, errstr);
        goto error;
    }

    /* Redirect all messages from per-partition queues to
     * the main queue so that messages can be consumed with one
     * call from all assigned partitions.
     *
     * The alternative is to poll the main queue (for events)
     * and each partition queue separately, which requires setting
     * up a rebalance callback and keeping track of the assignment:
     * but that is more complex and typically not recommended. */
    rd_kafka_poll_set_consumer(stream_reader->rk);

    subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(
            subscription,
            stream_reader->topic,
            /* the partition is ignored
             * by subscribe() */
            RD_KAFKA_PARTITION_UA);

    /* Subscribe to a topic */
    error = rd_kafka_subscribe(stream_reader->rk, subscription);
    if (error) {
        RTI_RoutingServiceEnvironment_set_error(
                env,
                "Failed to subscribe to %d topic(s): %s",
                subscription->cnt,
                rd_kafka_err2str(error));
        rd_kafka_topic_partition_list_destroy(subscription);
        goto error;
    }

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_INFO,
            "Subscribed to %d topic(s), waiting for rebalance and messages...",
            subscription->cnt);

    rd_kafka_topic_partition_list_destroy(subscription);


    stream_reader->poll_sem =
            RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    if (!stream_reader->poll_sem) {
        RTI_RoutingServiceEnvironment_set_error(env, "Error creating semaphore");
        goto error;
    }

    stream_reader->read_sem =
            RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    if (!stream_reader->read_sem) {
        RTI_RoutingServiceEnvironment_set_error(env, "Error creating semaphore");
        goto error;
    }

    stream_reader->poll_sem =
            RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    if (!stream_reader->poll_sem) {
        RTI_RoutingServiceEnvironment_set_error(env, "Error creating mutex");
        goto error;
    }
    // Initialize semaphore to available
    RTIOsapiSemaphore_give(stream_reader->poll_sem);

    stream_reader->read_sem =
            RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    if (!stream_reader->read_sem) {
        RTI_RoutingServiceEnvironment_set_error(env, "Error creating mutex");
        goto error;
    }

    stream_reader->polling_thread = RTIOsapiJoinableThread_new(
            "KafkaStreamReader_run",
            RTI_OSAPI_THREAD_PRIORITY_DEFAULT,
            RTI_OSAPI_THREAD_OPTION_DEFAULT,
            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
            NULL,
            RTI_RS_KafkaStreamReader_on_data_availabe_thread,
            (void *) stream_reader);

    if (!stream_reader->polling_thread) {
        RTI_RoutingServiceEnvironment_set_error(env, "Error creating polling_thread");
        goto error; 
    }

    return stream_reader;

error:
    delete_stream_reader(stream_reader);
    return NULL;
}

void RTI_RS_KafkaConnection_delete_stream_reader(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_KafkaStreamReader *self =
            (struct RTI_RS_KafkaStreamReader *) stream_reader;

    void *value = NULL;
    struct DDS_Duration_t sleep_period = { 1, 0 };

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s\n",
            __func__);

    self->run_thread = RTI_FALSE;
    if (!RTIOsapiJoinableThread_stopAndDelete(
                self->polling_thread,
                RTI_OSAPI_THREAD_INFINITE_BLOCKING_TIMEOUT)) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "Error deleting "
                "RTI_RS_KafkaStreamReader_on_data_availabe_thread");
    }
    self->polling_thread = NULL;

    /* Close the consumer: commit final offsets and leave the group. */
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_INFO,
            "Closing consumer");
    if (self->rk != NULL) {
        rd_kafka_consumer_close(self->rk);
    }

    /* Free allocated resources */
    delete_stream_reader(stream_reader);
}

#undef ROUTER_CURRENT_SUBMODULE
