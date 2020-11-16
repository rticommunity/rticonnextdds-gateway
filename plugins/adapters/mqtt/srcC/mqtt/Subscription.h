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

#ifndef Subscription_h
#define Subscription_h

#include "rtiadapt_mqtt.h"

#include "Infrastructure.h"
#include "Message.h"

struct RTI_MQTT_Subscription;

struct RTI_MQTT_SubscriptionRequestContext {
    struct RTI_MQTT_Subscription *sub;
    struct RTI_MQTT_SubscriptionParamsSeq params;
};

#define RTI_MQTT_SubscriptionRequestContext_INITIALIZER \
    {                                                   \
        NULL, /* sub */                                 \
        DDS_SEQUENCE_INITIALIZER /* params */           \
    }

struct RTI_MQTT_Subscription {
    RTI_MQTT_SubscriptionStatus *data;
    struct RTI_MQTT_MessageReceiveQueue *queue;
    struct RTI_MQTT_Client *client;
    RTI_MQTT_Subscription_DataAvailableCallback data_avail_listener;
    void *data_avail_listener_data;
    struct RTI_MQTT_PendingRequest *req_sub;
    struct RTI_MQTT_PendingRequest *req_unsub;
    struct RTI_MQTT_SubscriptionRequestContext req_ctx;
};

#define RTI_MQTT_Subscription_INITIALIZER                             \
    {                                                                 \
        NULL, /* data */                                              \
        NULL, /* queue */                                             \
        NULL, /* client */                                            \
        NULL, /* data_avail_listener */                               \
        NULL, /* data_avail_listener_data */                          \
        NULL, /* req_sub */                                           \
        NULL, /* req_unsub */                                         \
        RTI_MQTT_SubscriptionRequestContext_INITIALIZER /* req_ctx */ \
    }

DDS_ReturnCode_t RTI_MQTT_Subscription_new(
        struct RTI_MQTT_Client *client,
        RTI_MQTT_SubscriptionConfig *config,
        struct RTI_MQTT_Subscription **sub_out);

void RTI_MQTT_Subscription_delete(struct RTI_MQTT_Subscription *self);

DDS_ReturnCode_t RTI_MQTT_Subscription_match(
        struct RTI_MQTT_Subscription *self,
        const char *topic_name,
        DDS_Boolean *match_out);

DDS_ReturnCode_t RTI_MQTT_Subscription_receive(
        struct RTI_MQTT_Subscription *self,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        const char *topic,
        RTI_MQTT_MessageInfo *msg_info,
        DDS_DynamicData **dropped_out,
        DDS_Boolean *lost_out);

DDS_SEQUENCE(RTI_MQTT_SubscriptionPtrSeq, struct RTI_MQTT_Subscription *);

#endif /* Subscription_h */