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

#ifndef MqttAgent_h
#define MqttAgent_h

#include "ndds/ndds_c.h"
#include "rtiadapt_mqtt.h"

#ifndef ENABLE_APP_GEN
    #define USE_APP_GEN 1
#endif /* ENABLE_APP_GEN */

struct MqttAgent {
    DDS_DomainParticipant *dp;
#if !USE_APP_GEN
    DDS_Subscriber *sub;
    DDS_Publisher *pub;
    DDS_Topic *topic_in;
    DDS_Topic *topic_out;
#endif /* !USE_APP_GEN */
    RTI_MQTT_MessageDataWriter *writer;
    RTI_MQTT_KeyedMessageDataReader *reader;
    DDS_WaitSet *waitset;
    DDS_GuardCondition *cond_exit;
    DDS_GuardCondition *cond_timeout;
    struct DDS_ConditionSeq active_conditions;

    DDS_UnsignedLong msg_recvd_tot;
    DDS_UnsignedLong msg_recvd_max;
    DDS_UnsignedLong msg_sent_tot;
    DDS_UnsignedLong msg_sent_max;

    void *thread_sub;
    void *thread_pub;

    RTI_MQTT_Message *pub_msg;
};

#define MQTT_AGENT_DOMAIN 0

#define MQTT_AGENT_PROFILES "file://etc/mqtt_agent_profiles.xml"

#if USE_APP_GEN
    #define MQTT_AGENT_PARTICIPANT "MqttAgent/Participants::agent/def"
    #define MQTT_AGENT_WRITER "pub/def::output"
    #define MQTT_AGENT_READER "sub/def::input"
#else
    #define MQTT_AGENT_QOS_LIBRARY "MqttAgent/Qos"
    #define MQTT_AGENT_PARTICIPANT_PROFILE "agent"
    #define MQTT_AGENT_TOPIC_INPUT "mqtt-in"
    #define MQTT_AGENT_TOPIC_OUTPUT "mqtt-out"
    #define MQTT_AGENT_WRITER_PROFILE "output"
    #define MQTT_AGENT_READER_PROFILE "input"
    #define MQTT_AGENT_TYPE "Message"
#endif /* USE_APP_GEN */

#define MQTT_AGENT_WRITER_TOPIC "foo/bar/baz"
#define MQTT_AGENT_WRITER_QOS RTI_MQTT_QosLevel_ZERO
#define MQTT_AGENT_WRITER_RETAINED DDS_BOOLEAN_FALSE
#define MQTT_AGENT_WRITER_MESSAGE_FMT "a message written from DDS (%d)"
#define MQTT_AGENT_WRITER_LOOP_SEC 1
#define MQTT_AGENT_WRITER_LOOP_NSEC 0

void MqttAgent_finalize(struct MqttAgent *self);

int MqttAgent_initialize(
        struct MqttAgent *self
#if USE_APP_GEN
        ,
        const char *participant_name,
        const char *reader_name,
        const char *writer_name
#endif /* USE_APP_GEN */
);

int MqttAgent_main(struct MqttAgent *self);

void MqttAgent_print_message(
        struct MqttAgent *self,
        RTI_MQTT_Message *msg,
        DDS_Boolean in);

int MqttAgent_publish_message(struct MqttAgent *self);

void *MqttAgent_thread_subscriber(void *arg);

void *MqttAgent_thread_publisher(void *arg);

#endif /* MqttAgent_h */