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
/* This reads DDS samples and writes them to Kafka in JSON */

#include "KafkaStreamReader.h"


int error_cleanup(struct RTI_RS_KafkaStreamReader *stream_reader)
{
    if (stream_reader->rkm != NULL) {
        rd_kafka_message_destroy(stream_reader->rkm);
    }
    if (RTIOsapiSemaphore_give(stream_reader->poll_sem)
        != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "Error giving poll semaphore");
        return -1;
    }
    return 0;
}
/*
 * This function will run as a separate thread and
 * notify of data availability in Kafka consumer
 */
void *RTI_RS_KafkaStreamReader_on_data_availabe_thread(void *thread_params)
{
    struct RTI_RS_KafkaStreamReader *self = thread_params;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    while (self->run_thread) {
        /* Block until the read thread returns loaned sample is done */
        if (RTIOsapiSemaphore_take(self->poll_sem, NULL)
            != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Error taking poll semaphore");
            return NULL;
        }

        self->rkm = rd_kafka_consumer_poll(self->rk, CONSUMER_POLL_TIMEOUT);

        /* Timeout: no message within CONSUMER_POLL_TIMEOUT */
        if (!self->rkm) {
            if (RTIOsapiSemaphore_give(self->poll_sem)
                != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                RTI_RoutingServiceLogger_log(
                        RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                        "Error giving poll semaphore");
                return NULL;
            }
            continue;
        }

        /* consumer_poll() will return either a proper message
         * or a consumer error (rkm->err is set). */
        if (self->rkm->err) {
            /* Consumer errors are generally to be considered
             * informational as the consumer will automatically
             * try to recover from all types of errors. */

            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Consumer error: %s\n",
                    rd_kafka_message_errstr(self->rkm));
            if (!error_cleanup(self)) {
                return NULL;
            }
            continue;
        }

        /* Proper message. */
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                "Message on %s [%" PRId32 "] at offset %" PRId64 ":",
                rd_kafka_topic_name(self->rkm->rkt),
                self->rkm->partition,
                self->rkm->offset);

        /* Print the message value/payload. */
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                "Value: %.*s",
                (int) self->rkm->len,
                (const char *) self->rkm->payload);

        if (!DDS_OctetSeq_initialize(&self->payload)) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Error initialize buffer");
            if (!error_cleanup(self)) {
                return NULL;
            }
            continue;
        }

        // TODO not sure if we need the +1, it depends on how the
        // payload
        // is serialized in kafka and what the 'len' contains
        // KA: Kafka does not serialize data. Need to discuss on
        // this... KA: instead of copying the buffer, loaning the
        // buffer from the kafka message object.
        if (!DDS_OctetSeq_loan_contiguous(
                    &self->payload,
                    (DDS_Octet *) self->rkm->payload,
                    self->rkm->len + 1,
                    self->rkm->len + 1)) {
            if (!error_cleanup(self)) {
                return NULL;
            }
            continue;
        }

        self->listener.on_data_available(self, self->listener.listener_data);

        if (RTIOsapiSemaphore_give(self->read_sem)
            != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Error giving read semaphore");
            return NULL;
        }
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

    struct DDS_DynamicData *sample = NULL;
    struct DDS_SampleInfo *info = NULL;
    sample = self->sample_list[0];
    info = self->info_list[0];

    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    if (RTIOsapiSemaphore_take(self->read_sem, NULL)
        != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "Error taking read semaphore");
        return NULL;
    }

    // TODO the payload.data should be configurable
    retcode = DDS_DynamicData_set_octet_seq(
            sample,
            "payload.data",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            &self->payload);
    if (retcode != DDS_RETCODE_OK) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "Error setting payload data");
        return NULL;
    }

    *count = 1;
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

    struct RTI_RS_KafkaStreamReader *self =
            (struct RTI_RS_KafkaStreamReader *) stream_reader;

    /* Destroy message if it exist */
    if (self->rkm != NULL) {
        rd_kafka_message_destroy(self->rkm);
    }

    RTIOsapiSemaphore_give(self->poll_sem);
    /*
     * Nothing to do here since the samples and sample list are
     * only used by the session thread
     */
}

#undef ROUTER_CURRENT_SUBMODULE
