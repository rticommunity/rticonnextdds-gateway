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


int RTI_RS_KafkaStreamReader_error_cleanup(struct RTI_RS_KafkaStreamReader *stream_reader)
{
    if (!DDS_OctetSeq_has_ownership(&stream_reader->payload)) {
        DDS_OctetSeq_unloan(&stream_reader->payload);
    }
    if (stream_reader->rkm != NULL) {
        rd_kafka_message_destroy(stream_reader->rkm);
        stream_reader->rkm = NULL;
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
/* A stop token replaces the unsynchronized flag shared with the deleting
 * thread. Only the worker consumes it, immediately before exiting. */
static DDS_Boolean RTI_RS_KafkaStreamReader_stop_requested(
        struct RTI_RS_KafkaStreamReader *self)
{
    struct RTINtpTime zero = {0, 0};
    RTIOsapiSemaphoreStatus status = RTIOsapiSemaphore_take(self->stop_sem, &zero);
    if (status == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "Error checking stop semaphore");
    }
    return status != RTI_OSAPI_SEMAPHORE_STATUS_TIMEOUT;
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

    while (!RTI_RS_KafkaStreamReader_stop_requested(self)) {
        /* Block until the read thread returns loaned sample is done */
        if (RTIOsapiSemaphore_take(self->poll_sem, NULL)
            != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Error taking poll semaphore");
            return NULL;
        }

        if (RTI_RS_KafkaStreamReader_stop_requested(self)) {
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
            if (RTI_RS_KafkaStreamReader_error_cleanup(self) != 0) {
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

        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
                "Payload length: %zu", self->rkm->len);

        /* Kafka permits empty values and null tombstones. Both map to an
         * empty payload in the adapter's byte-sequence representation. */
        if (self->rkm->len == 0) {
            if (!DDS_OctetSeq_set_length(&self->payload, 0)) {
                if (RTI_RS_KafkaStreamReader_error_cleanup(self) != 0) {
                    return NULL;
                }
                continue;
            }
        } else if (!DDS_OctetSeq_loan_contiguous(
                    &self->payload,
                    (DDS_Octet *) self->rkm->payload,
                    self->rkm->len,
                    self->rkm->len)) {
            if (RTI_RS_KafkaStreamReader_error_cleanup(self) != 0) {
                return NULL;
            }
            continue;
        }

        if (RTIOsapiSemaphore_give(self->read_sem)
            != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
            RTI_RoutingServiceLogger_log(
                    RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                    "Error giving read semaphore");
            return NULL;
        }

        /* Make read() ready before invoking a listener that may read inline. */
        self->listener.on_data_available(self, self->listener.listener_data);
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
    sample = self->sample_list[0];

    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    struct RTINtpTime zero = {0, 0};

    /* Routing Service may read again after draining the notification. */
    if (RTIOsapiSemaphore_take(self->read_sem, &zero)
        != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
        return;
    }

    retcode = DDS_DynamicData_set_octet_seq(
            sample,
            "payload.data",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            &self->payload);
    if (retcode != DDS_RETCODE_OK) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "Error setting payload data");
        RTI_RS_KafkaStreamReader_error_cleanup(self);
        return;
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

    if (!DDS_OctetSeq_has_ownership(&self->payload)) {
        DDS_OctetSeq_unloan(&self->payload);
    }

    /* Destroy the message only after releasing the borrowed payload buffer. */
    if (self->rkm != NULL) {
        rd_kafka_message_destroy(self->rkm);
        self->rkm = NULL;
    }

    if (RTIOsapiSemaphore_give(self->poll_sem)
        != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
        RTI_RoutingServiceLogger_log(
                RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
                "Error giving poll semaphore");
        return;
    }
}

