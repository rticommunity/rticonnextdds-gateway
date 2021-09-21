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
/* This reads DDS samples and writes them to Kafka in JSON                      */

#include "KafkaStreamReader.h"

/*
* This function will run as a separate thread and
* notify of data availability in Kafka consumer
*/
void *RTI_RS_KafkaStreamReader_run(void *threadParam)
{
    struct RTI_RS_KafkaStreamReader *self = threadParam;

    const struct RTINtpTime BLOCK_TIME = {3, 0};

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    // printf("RTI_RS_KafkaStreamReader_run. Period: {%d,%u}\n",
    // self->polling_period.sec + self->polling_period.nanosec);

    while (self->run) {
	    RTIOsapiSemaphore_take(self->sem, &BLOCK_TIME);
	    self->rkm = rd_kafka_consumer_poll(self->rk, 100);

	    if (self->rkm) {
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

			    rd_kafka_message_destroy(self->rkm);
		            RTIOsapiSemaphore_give(self->sem);
		    } else {

			    self->listener.on_data_available(
					    self,
					    self->listener.listener_data);
		    }

	    }
	    else { // Timeout: no message within 100ms,
		    RTIOsapiSemaphore_give(self->sem);
	    }

	    // Proper message.
	    RTI_RoutingServiceLogger_log(
	    RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
	    "Message on %s [%" PRId32 "] at offset %" PRId64 ":",
	    rd_kafka_topic_name(self->rkm->rkt), self->rkm->partition,
	    self->rkm->offset);

	    // Print the message value/payload.
	    RTI_RoutingServiceLogger_log(
	    RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
	    "Value: %.*s",
	    (int) self->rkm->len,
	    (const char *) self->rkm->payload);

    }
    self->finalized_thread = 1;

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

	struct DDS_DynamicData * sample = NULL;
	struct DDS_SampleInfo  * info   = NULL;
	sample = self->sample_list[0];
	info = self->info_list[0];

	DDS_Octet *buffer = NULL;
	struct DDS_OctetSeq buffer_seq;
	DDS_OctetSeq_initialize(&buffer_seq);
	DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

	DDS_OctetSeq_ensure_length(&buffer_seq, (int) self->rkm->len, 63000);
	buffer = DDS_OctetSeq_get_contiguous_buffer(&buffer_seq);
	strcpy(buffer, (const char *) self->rkm->payload);

	retcode = DDS_DynamicData_set_octet_seq(
			sample,
			"payload.data",
			DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
			&buffer_seq);
	if (retcode != DDS_RETCODE_OK) {
		RTI_RoutingServiceLogger_log(
				RTI_ROUTING_SERVICE_VERBOSITY_EXCEPTION,
				"Error setting payload data");

		rd_kafka_message_destroy(self->rkm);
		RTIOsapiSemaphore_give(self->sem);
		return;
	}

	*count = 1;
	*sample_list = (RTI_RoutingServiceSample *) self->sample_list;
	*info_list = (RTI_RoutingServiceSampleInfo *) self->info_list;

	rd_kafka_message_destroy(self->rkm);
	RTIOsapiSemaphore_give(self->sem);
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
