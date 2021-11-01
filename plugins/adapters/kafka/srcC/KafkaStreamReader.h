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

#ifndef KafkaStreamReader_h
#define KafkaStreamReader_h
#define CONSUMER_POLL_TIMEOUT 100 /*100 ms*/

#include "ndds/ndds_c.h"

#include "routingservice/routingservice_adapter.h"
#include "routingservice/routingservice_service.h"

/* Kafka C library */
#include "rdkafka.h"

struct RTI_RS_KafkaStreamReader {
    rd_kafka_t *rk;          /* rdkafka consumer instance handle */
    rd_kafka_message_t *rkm; /*rdkafka message object */
    const char *topic;       /* Topic to consume */
    struct RTIOsapiJoinableThread *polling_thread;
    struct RTIOsapiSemaphore *poll_sem;
    struct RTIOsapiSemaphore *read_sem;
    int run_thread;
    struct RTI_RoutingServiceStreamReaderListener listener;
    struct DDS_DynamicData **sample_list;
    struct DDS_SampleInfo **info_list;
    struct DDS_TypeCode *type_code;
    struct DDS_DynamicDataTypeSupport *type_support;
    struct DDS_OctetSeq payload;
};

void RTI_RS_KafkaStreamReader_read(
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceSample **sample_list,
        RTI_RoutingServiceSampleInfo **info_list,
        int *count,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_KafkaStreamReader_return_loan(
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceSample *sample_list,
        RTI_RoutingServiceSampleInfo *info_list,
        int count,
        RTI_RoutingServiceEnvironment *env);

void *RTI_RS_KafkaStreamReader_on_data_availabe_thread(void *thread_params);

#endif /* KafkaStreamReader_h */
