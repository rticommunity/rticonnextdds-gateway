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

#ifndef Infrastructure_h
#define Infrastructure_h

#include "rtiadapt_mqtt.h"

#if RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_POSIX \
        || RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_WINDOWS

    #define RTI_MQTT_String_length strlen
    #define RTI_MQTT_String_compare strcmp
    #define RTI_MQTT_Memory_compare memcmp
    #define RTI_MQTT_String_to_long strtol
    #define RTI_MQTT_String_find_substring strstr
    #define RTI_MQTT_Heap_allocate malloc

    #if 0
        #define RTI_MQTT_Heap_free(ptr_)                 \
            {                                            \
                RTI_MQTT_LOG_1("FREE", "ptr=%p", (ptr_)) \
                free((ptr_));                            \
            }
    #else
        #define RTI_MQTT_Heap_free free
    #endif

    #define RTI_MQTT_Memory_copy memcpy
    #define RTI_MQTT_Memory_set memset
    #define RTI_MQTT_Memory_zero(ptr_, size_) (memset((ptr_), 0, (size_)))
    #define RTI_MQTT_Memory_move memmove
#endif

#define MQTT_TOPIC_NAME_MAX_LEN 65535

#define RTI_MQTT_String_is_equal(s_, o_) \
    (((s_) == NULL && (o_) == NULL)      \
     || ((s_) != NULL && (o_) != NULL    \
         && RTI_MQTT_String_compare((s_), (o_)) == 0))

DDS_ReturnCode_t RTI_MQTT_Time_normalize(RTI_MQTT_Time *self);

DDS_ReturnCode_t
        RTI_MQTT_Time_to_seconds(RTI_MQTT_Time *self, int *seconds_out);

DDS_ReturnCode_t RTI_MQTT_Time_to_dds_time(
        RTI_MQTT_Time *self,
        struct DDS_Time_t *time_out);

DDS_ReturnCode_t RTI_MQTT_Time_to_dds_duration(
        RTI_MQTT_Time *self,
        struct DDS_Duration_t *dur_out);

#define RTI_MQTT_Time_is_zero(t_) ((t_)->seconds == 0 && (t_)->nanoseconds == 0)

DDS_ReturnCode_t RTI_MQTT_DDS_OctetSeq_to_string(
        struct DDS_OctetSeq *self,
        char **str_out);


DDS_ReturnCode_t RTI_MQTT_TopicFilter_match(
        const char *filter,
        const char *value,
        DDS_Boolean *match_out);

DDS_ReturnCode_t
        RTI_MQTT_QosLevel_to_mqtt_qos(RTI_MQTT_QosLevel level, int *mqtt_out);

DDS_ReturnCode_t RTI_MQTT_QosLevel_from_mqtt_qos(
        int mqtt_qos,
        RTI_MQTT_QosLevel *level_out);

DDS_Boolean RTI_MQTT_QosLevel_is_valid(RTI_MQTT_QosLevel level);

#define RTI_MQTT_QosLevel_is_valid(q_)                               \
    ((q_) == RTI_MQTT_QosLevel_ZERO || (q_) == RTI_MQTT_QosLevel_ONE \
     || (q_) == RTI_MQTT_QosLevel_TWO)

#define RTI_MQTT_QosLevel_as_string(q_)                                 \
    (((q_) == RTI_MQTT_QosLevel_ZERO) ? "0"                             \
                                      : ((q_) == RTI_MQTT_QosLevel_ONE) \
                     ? "1"                                              \
                     : ((q_) == RTI_MQTT_QosLevel_TWO) ? "2" : "Unknown")

#define RTI_MQTT_ClientStateKind_as_string(s_)                               \
    (((s_) == RTI_MQTT_ClientStateKind_CONNECTED)                            \
             ? "CONNECTED"                                                   \
             : ((s_) == RTI_MQTT_ClientStateKind_DISCONNECTED)               \
                     ? "DISCONNECTED"                                        \
                     : ((s_) == RTI_MQTT_ClientStateKind_DISCONNECTING)      \
                             ? "DISCONNECTING"                               \
                             : ((s_) == RTI_MQTT_ClientStateKind_CONNECTING) \
                                     ? "CONNECTING"                          \
                                     : ((s_)                                 \
                                        == RTI_MQTT_ClientStateKind_ERROR)   \
                                             ? "ERROR"                       \
                                             : "unknown")

DDS_ReturnCode_t RTI_MQTT_ClientStatus_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_ClientStatus **state_out);

void RTI_MQTT_ClientStatus_delete(RTI_MQTT_ClientStatus *self);

DDS_ReturnCode_t RTI_MQTT_SubscriptionStatus_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_SubscriptionStatus **state_out);

void RTI_MQTT_SubscriptionStatus_delete(RTI_MQTT_SubscriptionStatus *self);

DDS_ReturnCode_t RTI_MQTT_PublicationStatus_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_PublicationStatus **state_out);

void RTI_MQTT_PublicationStatus_delete(RTI_MQTT_PublicationStatus *self);

RTIBool RTI_MQTT_MessagePtr_initialize_w_params(
        struct RTI_MQTT_Message **self,
        const struct DDS_TypeAllocationParams_t *allocParams);

RTIBool RTI_MQTT_MessagePtr_finalize_w_params(
        struct RTI_MQTT_Message **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams);

RTIBool RTI_MQTT_MessagePtr_copy(
        struct RTI_MQTT_Message **dst,
        const struct RTI_MQTT_Message **src);

DDS_SEQUENCE(RTI_MQTT_MessagePtrSeq, struct RTI_MQTT_Message *);


DDS_SEQUENCE(RTI_MQTT_DDS_DynamicDataPtrSeq, struct DDS_DynamicData *);

struct RTI_MQTT_PendingRequest;

typedef void (*RTI_MQTT_PendingRequest_ResultHandlerFn)(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result);

struct RTI_MQTT_PendingRequest {
    struct RTI_MQTT_Client *client;
    DDS_WaitSet *waitset;
    DDS_GuardCondition *condition;
    void *context;
    DDS_ReturnCode_t result;
    RTI_MQTT_Time timeout;
    RTI_MQTT_PendingRequest_ResultHandlerFn result_handler;
    struct DDS_ConditionSeq cond_seq;
};

#define RTI_MQTT_PendingRequest_INITIALIZER                   \
    {                                                         \
        NULL, /* client */                                    \
        NULL, /* waitset */                                   \
        NULL, /* condition */                                 \
        NULL, /* context */                                   \
        DDS_RETCODE_ERROR, /* result */                       \
        RTI_MQTT_Time_INITIALIZER(0, 0), /* timeout */        \
        NULL, /* result_handler */                            \
        DDS_SEQUENCE_INITIALIZER /* cond_seq */               \
    }

#define RTI_MQTT_PendingRequest_handle_result(r_, res_) \
    if ((r_)->result_handler != NULL) {                 \
        (r_)->result_handler((r_), (res_));             \
    } else {                                            \
        RTI_MQTT_ERROR_2(                               \
                "no result handler for request:",       \
                "req=%p, result=%d",                    \
                (r_),                                   \
                (res_))                                 \
    }

DDS_SEQUENCE(RTI_MQTT_PendingRequestPtrSeq, struct RTI_MQTT_PendingRequest *);

#endif /* Infrastructure_h */
