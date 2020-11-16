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

#ifndef MessageWriter_h
#define MessageWriter_h

#include "rtiadapt_mqtt.h"

struct RTI_RS_MQTT_MessageWriter {
    RTI_RS_MQTT_MessageWriterConfig *config;
    struct RTI_RS_MQTT_BrokerConnection *connection;
    struct RTI_MQTT_Publication *pub;
};

DDS_ReturnCode_t RTI_RS_MQTT_MessageWriter_new(
        struct RTI_RS_MQTT_BrokerConnection *connection,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_MessageWriter **writer_out);

void RTI_RS_MQTT_MessageWriter_delete(struct RTI_RS_MQTT_MessageWriter *writer);

void RTI_RS_MQTT_MessageWriter_update(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

int RTI_RS_MQTT_MessageWriter_write(
        RTI_RoutingServiceStreamWriter stream_writer,
        const RTI_RoutingServiceSample *sample_list,
        const RTI_RoutingServiceSampleInfo *info_list,
        int count,
        RTI_RoutingServiceEnvironment *env);


RTIBool RTI_RS_MQTT_MessageWriterPtr_initialize_w_params(
        struct RTI_RS_MQTT_MessageWriter **self,
        const struct DDS_TypeAllocationParams_t *allocParams);

RTIBool RTI_RS_MQTT_MessageWriterPtr_finalize_w_params(
        struct RTI_RS_MQTT_MessageWriter **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams);

RTIBool RTI_RS_MQTT_MessageWriterPtr_copy(
        struct RTI_RS_MQTT_MessageWriter **dst,
        const struct RTI_RS_MQTT_MessageWriter **src);

DDS_SEQUENCE(
        RTI_RS_MQTT_MessageWriterPtrSeq,
        struct RTI_RS_MQTT_MessageWriter *);


#endif /* MessageWriter_h */
