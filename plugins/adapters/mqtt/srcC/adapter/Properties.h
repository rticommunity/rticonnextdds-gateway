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

#ifndef Properties_h
#define Properties_h

#include "rtiadapt_mqtt.h"

extern const RTI_RS_MQTT_BrokerConnectionConfig
        RTI_RS_MQTT_BrokerConnectionConfig_DEFAULT;
extern const RTI_RS_MQTT_MessageReaderConfig
        RTI_RS_MQTT_MessageReaderConfig_DEFAULT;
extern const RTI_RS_MQTT_MessageWriterConfig
        RTI_RS_MQTT_MessageWriterConfig_DEFAULT;

#define RTI_RS_MQTT_BrokerConnectionConfig_INITIALIZER \
    {                                                  \
        RTI_MQTT_ClientConfig_INITIALIZER /* client */ \
    }

#define RTI_RS_MQTT_MessageReaderConfig_INITIALIZER       \
    {                                                     \
        RTI_MQTT_SubscriptionConfig_INITIALIZER /* sub */ \
    }

#define RTI_RS_MQTT_MessageWriterConfig_INITIALIZER      \
    {                                                    \
        RTI_MQTT_PublicationConfig_INITIALIZER /* pub */ \
    }
DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnectionConfig_default(
        RTI_RS_MQTT_BrokerConnectionConfig **config_out);

DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnectionConfig_new(
        DDS_Boolean allocate_optional,
        RTI_RS_MQTT_BrokerConnectionConfig **config_out);

void RTI_RS_MQTT_BrokerConnectionConfig_delete(
        RTI_RS_MQTT_BrokerConnectionConfig *self);

DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnectionConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RS_MQTT_BrokerConnectionConfig **config_out);

DDS_ReturnCode_t RTI_RS_MQTT_MessageReaderConfig_default(
        RTI_RS_MQTT_MessageReaderConfig **config_out);

DDS_ReturnCode_t RTI_RS_MQTT_MessageReaderConfig_new(
        DDS_Boolean allocate_optional,
        RTI_RS_MQTT_MessageReaderConfig **config_out);

void RTI_RS_MQTT_MessageReaderConfig_delete(
        RTI_RS_MQTT_MessageReaderConfig *self);

DDS_ReturnCode_t RTI_RS_MQTT_MessageReaderConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RS_MQTT_MessageReaderConfig **config_out);

DDS_ReturnCode_t RTI_RS_MQTT_MessageWriterConfig_default(
        RTI_RS_MQTT_MessageWriterConfig **config_out);

DDS_ReturnCode_t RTI_RS_MQTT_MessageWriterConfig_new(
        DDS_Boolean allocate_optional,
        RTI_RS_MQTT_MessageWriterConfig **config_out);

void RTI_RS_MQTT_MessageWriterConfig_delete(
        RTI_RS_MQTT_MessageWriterConfig *self);

DDS_ReturnCode_t RTI_RS_MQTT_MessageWriterConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RS_MQTT_MessageWriterConfig **config_out);

#endif /* Properties_h */