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

#ifndef Plugin_h
#define Plugin_h

#include "rtiadapt_mqtt.h"

#include "BrokerConnection.h"

struct RTI_RS_MQTT_AdapterPlugin {
    struct RTI_RoutingServiceAdapterPlugin parent;
    struct RTI_RS_MQTT_BrokerConnectionPtrSeq connections;
};

DDS_ReturnCode_t RTI_RS_MQTT_AdapterPlugin_new(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_AdapterPlugin **plugin_out);

void RTI_RS_MQTT_AdapterPlugin_delete(
        struct RTI_RoutingServiceAdapterPlugin *plugin,
        RTI_RoutingServiceEnvironment *env);

RTI_RoutingServiceConnection RTI_RS_MQTT_AdapterPlugin_create_connection(
        struct RTI_RoutingServiceAdapterPlugin *plugin,
        const char *routing_service_name,
        const char *routing_service_group_name,
        const struct RTI_RoutingServiceStreamReaderListener
                *input_stream_discovery_listener,
        const struct RTI_RoutingServiceStreamReaderListener
                *output_stream_discovery_listener,
        const struct RTI_RoutingServiceTypeInfo **edTypes,
        int edTypeCount,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_AdapterPlugin_delete_connection(
        struct RTI_RoutingServiceAdapterPlugin *plugin,
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_AdapterPlugin_update(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

#endif /* Plugin_h */