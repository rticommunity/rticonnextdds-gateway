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

#ifndef MessageReader_h
#define MessageReader_h

#include "rtiadapt_mqtt.h"

DDS_SEQUENCE(RTI_MQTT_DDS_SampleInfoPtrSeq, struct DDS_SampleInfo *);

struct RTI_RS_MQTT_MessageReader {
    RTI_RS_MQTT_MessageReaderConfig *config;
    struct RTI_RS_MQTT_BrokerConnection *connection;
    const struct RTI_RoutingServiceStreamReaderListener *listener;
    struct RTI_MQTT_Subscription *sub;
    struct DDS_SampleInfoSeq info_seq;
    struct RTI_MQTT_DDS_SampleInfoPtrSeq info_ptr_seq;
};

#define RTI_RS_MQTT_MessageReader_is_discovery(r_) \
    ((r_)->connection->disc_reader_in == (r_)      \
     || (r_)->connection->disc_reader_out == (r_))

DDS_ReturnCode_t RTI_RS_MQTT_MessageReader_new(
        struct RTI_RS_MQTT_BrokerConnection *connection,
        const struct RTI_RoutingServiceStreamReaderListener *listener,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_MessageReader **reader_out);

DDS_ReturnCode_t RTI_RS_MQTT_MessageReader_new_discovery(
        struct RTI_RS_MQTT_BrokerConnection *connection,
        const struct RTI_RoutingServiceStreamReaderListener *listener,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_MessageReader **reader_out);

void RTI_RS_MQTT_MessageReader_delete(struct RTI_RS_MQTT_MessageReader *reader);

void RTI_RS_MQTT_MessageReader_update(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_MessageReader_read(
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceSample **sample_list,
        RTI_RoutingServiceSampleInfo **info_list,
        int *count,
        RTI_RoutingServiceEnvironment *env);

void RTI_RS_MQTT_MessageReader_return_loan(
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceSample *sample_list,
        RTI_RoutingServiceSampleInfo *info_list,
        int count,
        RTI_RoutingServiceEnvironment *env);


RTIBool RTI_RS_MQTT_MessageReaderPtr_initialize_w_params(
        struct RTI_RS_MQTT_MessageReader **self,
        const struct DDS_TypeAllocationParams_t *allocParams);

RTIBool RTI_RS_MQTT_MessageReaderPtr_finalize_w_params(
        struct RTI_RS_MQTT_MessageReader **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams);

RTIBool RTI_RS_MQTT_MessageReaderPtr_copy(
        struct RTI_RS_MQTT_MessageReader **dst,
        const struct RTI_RS_MQTT_MessageReader **src);

DDS_SEQUENCE(
        RTI_RS_MQTT_MessageReaderPtrSeq,
        struct RTI_RS_MQTT_MessageReader *);

RTIBool RTI_MQTT_DDS_SampleInfoPtr_initialize_w_params(
        struct DDS_SampleInfo **self,
        const struct DDS_TypeAllocationParams_t *allocParams);

RTIBool RTI_MQTT_DDS_SampleInfoPtr_finalize_w_params(
        struct DDS_SampleInfo **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams);

RTIBool RTI_MQTT_DDS_SampleInfoPtr_copy(
        struct DDS_SampleInfo **dst,
        const struct DDS_SampleInfo **src);

#endif /* MessageReader_h */
