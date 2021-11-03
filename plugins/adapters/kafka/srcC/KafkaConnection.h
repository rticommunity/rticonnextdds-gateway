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

#ifndef KafkaConnection_h
#define KafkaConnection_h

#define ERR_MSG_BUF_SIZE 512

#include "ndds/ndds_c.h"

#include "routingservice/routingservice_adapter.h"
#include "routingservice/routingservice_service.h"

/* Kafka C library */
#include "rdkafka.h"

#include "KafkaStreamReader.h"
#include "KafkaStreamWriter.h"

struct RTI_RS_KafkaConnection {
    struct RTI_RS_KafkaAdapterPlugin *adapter;
    const char *bootstrap_servers; /* Initial list of brokers as a CSV list of
                                      broker host or host:port */
};

RTI_RoutingServiceStreamWriter RTI_RS_KafkaConnection_create_stream_writer(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceSession session,
        const struct RTI_RoutingServiceStreamInfo *stream_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_KafkaConnection_delete_stream_writer(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceStreamWriter stream_writer,
        RTI_RoutingServiceEnvironment *env);

RTI_RoutingServiceStreamReader RTI_RS_KafkaConnection_create_stream_reader(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceSession session,
        const struct RTI_RoutingServiceStreamInfo *stream_info,
        const struct RTI_RoutingServiceProperties *properties,
        const struct RTI_RoutingServiceStreamReaderListener *listener,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_KafkaConnection_delete_stream_reader(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceEnvironment *env);

#endif /* KafkaConnection_h */
