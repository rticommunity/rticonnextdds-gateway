/*
 * (c) 2021 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */

/* =========================================================================*/
/* RTI Routing Service Kafka Adapter                                        */
/* =========================================================================*/
/* This reads DDS samples writes them to Kafka in JSON                      */

#include "KafkaStreamReader.h"

/*
* This function will run as a separate thread and 
* notify of data availability in Kafka consumer
*/
void *RTI_RS_KafkaStreamReader_run(void *threadParam)
{
    struct RTI_RS_KafkaStreamReader *self = threadParam;

    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    // printf("RTI_RS_KafkaStreamReader_run. Period: {%d,%u}\n",
    // self->polling_period.sec + self->polling_period.nanosec);

    while (self->run) {
        rd_kafka_message_t *rkm;
        struct DDS_DynamicData *sample = self->sample_list[0];
        struct DDS_SampleInfo *info = NULL;
        DDS_Octet *buffer = NULL;
        struct DDS_OctetSeq buffer_seq;
        DDS_OctetSeq_initialize(&buffer_seq);

        rkm = rd_kafka_consumer_poll(self->rk, 100);
        if (!rkm)
            continue; /* Timeout: no message within 100ms,
                       *  try again. This short timeout allows
                       *  checking for `run` at frequent intervals.
                       */
        /* consumer_poll() will return either a proper message
         * or a consumer error (rkm->err is set). */
        if (rkm->err) {
            /* Consumer errors are generally to be considered
             * informational as the consumer will automatically
             * try to recover from all types of errors. */

            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Consumer error: %s\n",
                    rd_kafka_message_errstr(rkm));

            rd_kafka_message_destroy(rkm);
            continue;
        }

        DDS_OctetSeq_ensure_length(&buffer_seq, (int) rkm->len, 63000);
        buffer = DDS_OctetSeq_get_contiguous_buffer(&buffer_seq);
        strcpy(buffer, (const char *) rkm->payload);

        retcode = DDS_DynamicData_set_octet_seq(
                sample,
                "payload.data",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                &buffer_seq);
        if (retcode != DDS_RETCODE_OK) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Error setting payload data");
            continue;
        }

        /* Proper message. */
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                    "Message on %s [%" PRId32 "] at offset %" PRId64 ":",
               rd_kafka_topic_name(rkm->rkt), rkm->partition,
               rkm->offset);

        /* Print the message value/payload. */
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                    "Value: %.*s",
                    (int) rkm->len,
                    (const char *) rkm->payload);

            self->listener.on_data_available(
                    self,
                    self->listener.listener_data);
            rd_kafka_message_destroy(rkm);
    }

    return NULL;
}

void RTI_RS_KafkaStreamReader_read(
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceSample **sample_list,
        RTI_RoutingServiceSampleInfo **info_list,
        int *count,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_KafkaStreamReader *self =
            (struct RTI_RS_KafkaStreamReader *) stream_reader;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    *sample_list = NULL;
    *info_list = NULL;
    *count = 0;

    *count += 1;
    *sample_list = (RTI_RoutingServiceSample *) self->sample_list;
    *info_list = (RTI_RoutingServiceSampleInfo *) self->info_list;
}

void RTI_RS_KafkaStreamReader_return_loan(
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceSample *sample_list,
        RTI_RoutingServiceSampleInfo *info_list,
        int count,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    /*
     * Nothing to do here since the samples and sample list are
     * only used by the session thread
     */
}

#undef ROUTER_CURRENT_SUBMODULE