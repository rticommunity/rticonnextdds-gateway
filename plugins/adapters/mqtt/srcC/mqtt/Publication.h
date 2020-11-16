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

#ifndef Publication_h
#define Publication_h

#include "rtiadapt_mqtt.h"

#include "Infrastructure.h"

struct RTI_MQTT_Publication;

struct RTI_MQTT_PublicationRequestContext {
    struct RTI_MQTT_Publication *pub;
    RTI_MQTT_QosLevel last_write_qos;
    char *topic;
    DDS_UnsignedLong topic_len;
    struct DDS_OctetSeq payload;
};

#define RTI_MQTT_PublicationRequestContext_INITIALIZER  \
    {                                                   \
        NULL, /* pub */                                 \
        RTI_MQTT_QosLevel_UNKNOWN, /* last_write_qos */ \
        NULL, /* topic */                               \
        0, /* topic_len */                              \
        DDS_SEQUENCE_INITIALIZER /* payload */          \
    }

struct RTI_MQTT_Publication {
    RTI_MQTT_PublicationStatus *data;
    struct RTI_MQTT_Client *client;
    struct RTI_MQTT_PendingRequest *req;
    struct RTI_MQTT_PublicationRequestContext req_ctx;
};

#define RTI_MQTT_Publication_INITIALIZER                              \
    {                                                                 \
        NULL, /* data */                                              \
        NULL, /* client */                                            \
        NULL, /* req_publish */                                       \
        RTI_MQTT_PublicationRequestContext_INITIALIZER, /* req_ctx */ \
    }

DDS_ReturnCode_t RTI_MQTT_Publication_new(
        struct RTI_MQTT_Client *client,
        RTI_MQTT_PublicationConfig *config,
        struct RTI_MQTT_Publication **pub_out);

void RTI_MQTT_Publication_delete(struct RTI_MQTT_Publication *self);

DDS_ReturnCode_t RTI_MQTT_Publication_on_write_result(
        struct RTI_MQTT_Publication *self,
        DDS_ReturnCode_t result);

DDS_ReturnCode_t RTI_MQTT_Publication_on_write_delivery_result(
        struct RTI_MQTT_Publication *self,
        DDS_ReturnCode_t result);

DDS_SEQUENCE(RTI_MQTT_PublicationPtrSeq, struct RTI_MQTT_Publication *);

#endif /* Publication_h */