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

#ifndef KafkaStreamWriter_h
#define KafkaStreamWriter_h

#include "ndds/ndds_c.h"

#include "routingservice/routingservice_adapter.h"
#include "routingservice/routingservice_service.h"

/* Kafka C library */
#include "rdkafka.h"

#include <ctype.h>
#include <time.h>

struct RTI_RS_KafkaStreamWriter {
    rd_kafka_t *rk;        /* rdkafka producer instance handle */
    rd_kafka_conf_t *conf; /* rdkafka configuration object */
    const char *topic;     /* Topic to produce to */
};

int RTI_RS_KafkaStreamWriter_write(
        RTI_RoutingServiceStreamWriter stream_writer,
        const RTI_RoutingServiceSample *sample_list,
        const RTI_RoutingServiceSampleInfo *info_list,
        int count,
        RTI_RoutingServiceEnvironment *env);

static void RTI_RS_KafkaStreamWriter_dr_msg_cb(
        rd_kafka_t *rk,
        const rd_kafka_message_t *rkmessage,
        void *opaque);

#endif /* KafkaStreamWriter_h */
