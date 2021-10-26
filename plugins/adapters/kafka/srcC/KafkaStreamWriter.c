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

/* =========================================================================*/
/* RTI Routing Service Kafka Adapter                                        */
/* =========================================================================*/
/* This reads DDS samples and then writes them to Kafka in a format that Kafka
 * supports                      */

#include "KafkaStreamWriter.h"

int RTI_RS_KafkaStreamWriter_write(
        RTI_RoutingServiceStreamWriter stream_writer,
        const RTI_RoutingServiceSample *sample_list,
        const RTI_RoutingServiceSampleInfo *info_list,
        int count,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_KafkaStreamWriter *self =
            (struct RTI_RS_KafkaStreamWriter *) stream_writer;
    struct DDS_DynamicData *sample = NULL;
    struct DDS_SampleInfo *sample_info = NULL;
    struct DDS_OctetSeq buffer_seq = DDS_SEQUENCE_INITIALIZER;
    char *buffer = NULL;

    int i = 0;
    size_t len = 0;
    rd_kafka_resp_err_t err = RD_KAFKA_RESP_ERR_NO_ERROR;
    int retcode;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%d DDS samples requested to send",
            count);

    for (i = 0; i < count; i++) {
        sample = (struct DDS_DynamicData *) sample_list[i];
        /* we are not using sample_info anywhere
        sample_info = (struct DDS_SampleInfo *)info_list[i];

        if (sample_info) {
            sample_info = (struct DDS_SampleInfo *)info_list[i];
        }
        */

        retcode = DDS_DynamicData_get_octet_seq(
                sample,
                &buffer_seq,
                "payload.data",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
        if (retcode != DDS_RETCODE_OK) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Error getting payload data");
            continue;
        }

        buffer = DDS_OctetSeq_get_contiguous_buffer(&buffer_seq);
        if (buffer == NULL) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Error getting contiguous buffer");
            continue;
        }

        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                "Payload data: %s",
                buffer);

        len = strlen(buffer);

        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                "Payload lenth: %d",
                len);

    retry:
        err = rd_kafka_producev(
                self->rk,
                RD_KAFKA_V_TOPIC(self->topic),
                /* Make a copy of the payload. */
                RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
                /* Message value and length */
                RD_KAFKA_V_VALUE(buffer, len),
                /* Per-Message opaque, provided in delivery report callback as msg_opaque. */
                RD_KAFKA_V_OPAQUE(NULL),
                /* End sentinel */
                RD_KAFKA_V_END);
        if (err != RD_KAFKA_RESP_ERR_NO_ERROR) {
            /* Failed to *enqueue* message for producing. */
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Failed to produce to topic %s: %s\n",
                    self->topic,
                    rd_kafka_err2str(err));

            if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
                /* If the internal queue is full, wait for
                 * messages to be delivered and then retry.
                 * The internal queue represents both
                 * messages to be sent and messages that have
                 * been sent or failed, awaiting their
                 * delivery report callback to be called.

                 * The internal queue is limited by the
                 * configuration property
                 * queue.buffering.max.messages */
                RTI_RoutingServiceLogger_log(
                        RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                        "Internal queue is full. Waiting for messages to be "
                        "delivered...");
                rd_kafka_poll(self->rk, 10);  /* block for max 10ms */
                goto retry;
            }
        } else {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                    "Enqueued message (%zd bytes) "
                    "for topic %s\n",
                    len,
                    self->topic);
        }

        /* A producer application should continually serve
         * the delivery report queue by calling rd_kafka_poll()
         * at frequent intervals.
         * Either put the poll call in your main loop, or in a
         * dedicated thread, or call it after every
         * rd_kafka_produce() call.
         * Just make sure that rd_kafka_poll() is still called
         * during periods where you are not producing any messages
         * to make sure previously produced messages have their
         * delivery report callback served (and any other callbacks
         * you register). */
        rd_kafka_poll(self->rk, 0 /*non-blocking*/);
    }

    return count;
}

#undef ROUTER_CURRENT_SUBMODULE
