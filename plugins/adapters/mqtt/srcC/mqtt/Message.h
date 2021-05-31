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

#ifndef Message_h
#define Message_h

#include "rtiadapt_mqtt.h"

#include "Infrastructure.h"

struct RTI_MQTT_ReceivedMessage {
    DDS_DynamicData *message;
    DDS_Boolean read;
};

DDS_ReturnCode_t
        RTI_MQTT_ReceivedMessage_new(struct RTI_MQTT_ReceivedMessage **msg_out);

void RTI_MQTT_ReceivedMessage_delete(struct RTI_MQTT_ReceivedMessage *self);

DDS_ReturnCode_t RTI_MQTT_ReceivedMessage_initialize(
        struct RTI_MQTT_ReceivedMessage *self);

DDS_ReturnCode_t RTI_MQTT_ReceivedMessage_finalize(
        struct RTI_MQTT_ReceivedMessage *self);

RTIBool RTI_MQTT_ReceivedMessagePtr_initialize_w_params(
        struct RTI_MQTT_ReceivedMessage **self,
        const struct DDS_TypeAllocationParams_t *allocParams);

RTIBool RTI_MQTT_ReceivedMessagePtr_finalize_w_params(
        struct RTI_MQTT_ReceivedMessage **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams);

RTIBool RTI_MQTT_ReceivedMessagePtr_copy(
        struct RTI_MQTT_ReceivedMessage **dst,
        const struct RTI_MQTT_ReceivedMessage **src);

DDS_SEQUENCE(RTI_MQTT_ReceivedMessagePtrSeq, struct RTI_MQTT_ReceivedMessage *);

struct RTI_MQTT_MessageReceiveQueue;

typedef void (*RTI_MQTT_MessageReceiveQueue_OnDataAvailableCallback)(
        struct RTI_MQTT_MessageReceiveQueue *queue,
        void *arg);

struct RTI_MQTT_MessageReceiveQueue {
    RTI_MQTT_Mutex lock;
    DDS_UnsignedLong capacity;
    DDS_UnsignedLong size;
    DDS_UnsignedLong head;
    DDS_UnsignedLong next;
    DDS_UnsignedLong next_unread;
    struct RTI_MQTT_DDS_DynamicDataPtrSeq read_buffer;
    struct DDS_DynamicDataTypeSupport *dyn_data;
    DDS_Boolean read_buffer_loaned;
    struct RTI_MQTT_ReceivedMessagePtrSeq queue;
    RTI_MQTT_MessageReceiveQueue_OnDataAvailableCallback listener_data_avail;
    void *listener_data_avail_arg;
    RTI_MQTT_SubscriptionMessageStatus *msg_status;
};

#define RTI_MQTT_LOG_MESSAGE_QUEUE_STATE(msg_, q_)                           \
    RTI_MQTT_TRACE_8(                                                        \
            (msg_),                                                          \
            "[cap=%u, size=%u, head=%u, next=%u, next_un=%u, queue.len=%u, " \
            "queue.max=%u, loaned=%d]",                                      \
            (q_)->capacity,                                                  \
            (q_)->size,                                                      \
            (q_)->head,                                                      \
            (q_)->next,                                                      \
            (q_)->next_unread,                                               \
            RTI_MQTT_ReceivedMessagePtrSeq_get_length(&(q_)->queue),         \
            RTI_MQTT_ReceivedMessagePtrSeq_get_maximum(&(q_)->queue),        \
            (q_)->read_buffer_loaned)

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_new(
        DDS_UnsignedLong size,
        RTI_MQTT_SubscriptionMessageStatus *msg_status,
        struct RTI_MQTT_MessageReceiveQueue **queue_out);

void RTI_MQTT_MessageReceiveQueue_delete(
        struct RTI_MQTT_MessageReceiveQueue *self);

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_initialize(
        struct RTI_MQTT_MessageReceiveQueue *self,
        DDS_UnsignedLong size,
        RTI_MQTT_SubscriptionMessageStatus *msg_status);

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_finalize(
        struct RTI_MQTT_MessageReceiveQueue *self);

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_receive(
        struct RTI_MQTT_MessageReceiveQueue *self,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        const char *topic,
        RTI_MQTT_MessageInfo *msg_info,
        DDS_DynamicData **dropped_out,
        DDS_Boolean *lost_out);

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_read(
        struct RTI_MQTT_MessageReceiveQueue *self,
        DDS_UnsignedLong max_messages,
        struct DDS_DynamicDataSeq *messages);

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_return_loan(
        struct RTI_MQTT_MessageReceiveQueue *self,
        struct DDS_DynamicDataSeq *messages);

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_set_listener(
        struct RTI_MQTT_MessageReceiveQueue *self,
        RTI_MQTT_MessageReceiveQueue_OnDataAvailableCallback listener,
        void *listener_arg);

#endif /* Message_h */
