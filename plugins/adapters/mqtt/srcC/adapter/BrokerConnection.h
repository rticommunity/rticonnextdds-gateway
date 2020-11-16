/******************************************************************************/
/* (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#ifndef BrokerConnection_h
#define BrokerConnection_h

#include "rtiadapt_mqtt.h"

#include "MessageReader.h"
#include "MessageWriter.h"

struct RTI_RS_MQTT_BrokerConnection {
    RTI_RS_MQTT_BrokerConnectionConfig *config;
    struct RTI_MQTT_Client *client;
    struct RTI_RS_MQTT_MessageReaderPtrSeq readers;
    struct RTI_RS_MQTT_MessageWriterPtrSeq writers;
    const struct RTI_RoutingServiceStreamReaderListener *listener_discovery_in;
    const struct RTI_RoutingServiceStreamReaderListener *listener_discovery_out;
    struct DDS_TypeCode *tc_message;
    struct RTI_RS_MQTT_MessageReader *disc_reader_in;
    struct RTI_RS_MQTT_MessageReader *disc_reader_out;
};

void RTI_RS_MQTT_BrokerConnection_delete(
        struct RTI_RS_MQTT_BrokerConnection *self);

DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnection_new(
        const struct RTI_RoutingServiceStreamReaderListener
                *input_stream_discovery_listener,
        const struct RTI_RoutingServiceStreamReaderListener
                *output_stream_discovery_listener,
        const struct RTI_RoutingServiceTypeInfo **ed_types,
        int ed_types_count,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_BrokerConnection **connection_out);

RTI_RoutingServiceStreamReader
        RTI_RS_MQTT_BrokerConnection_create_stream_reader(
                RTI_RoutingServiceConnection connection,
                RTI_RoutingServiceSession session,
                const struct RTI_RoutingServiceStreamInfo *stream_info,
                const struct RTI_RoutingServiceProperties *properties,
                const struct RTI_RoutingServiceStreamReaderListener *listener,
                RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_BrokerConnection_delete_stream_reader(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceEnvironment *env);

RTI_RoutingServiceStreamWriter
        RTI_RS_MQTT_BrokerConnection_create_stream_writer(
                RTI_RoutingServiceConnection connection,
                RTI_RoutingServiceSession session,
                const struct RTI_RoutingServiceStreamInfo *stream_info,
                const struct RTI_RoutingServiceProperties *properties,
                RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_BrokerConnection_delete_stream_writer(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceStreamWriter stream_writer,
        RTI_RoutingServiceEnvironment *env);

RTI_RoutingServiceStreamReader
        RTI_RS_MQTT_BrokerConnection_get_input_discovery_reader(
                RTI_RoutingServiceConnection connection,
                RTI_RoutingServiceEnvironment *env);

RTI_RoutingServiceStreamReader
        RTI_RS_MQTT_BrokerConnection_get_output_discovery_reader(
                RTI_RoutingServiceConnection connection,
                RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_BrokerConnection_update(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_BrokerConnection_update_session(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

const char *RTI_RS_MQTT_BrokerConnection_to_string(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceEnvironment *env);

RTI_RoutingServiceSession RTI_RS_MQTT_BrokerConnection_create_session(
        RTI_RoutingServiceConnection connection,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_BrokerConnection_delete_session(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceSession session,
        RTI_RoutingServiceEnvironment *env);

RTIBool RTI_RS_MQTT_BrokerConnectionPtr_initialize_w_params(
        struct RTI_RS_MQTT_BrokerConnection **self,
        const struct DDS_TypeAllocationParams_t *allocParams);

RTIBool RTI_RS_MQTT_BrokerConnectionPtr_finalize_w_params(
        struct RTI_RS_MQTT_BrokerConnection **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams);

RTIBool RTI_RS_MQTT_BrokerConnectionPtr_copy(
        struct RTI_RS_MQTT_BrokerConnection **dst,
        const struct RTI_RS_MQTT_BrokerConnection **src);

DDS_SEQUENCE(
        RTI_RS_MQTT_BrokerConnectionPtrSeq,
        struct RTI_RS_MQTT_BrokerConnection *);


#endif /* BrokerConnection_h */