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

#ifndef ShapesShapesAgent_h
#define ShapesShapesAgent_h

#include "ShapeType.h"
#include "ShapeTypeSupport.h"
#include "TransformationPlatform.h"
#include "ndds/ndds_c.h"
#include "rtiadapt_mqtt.h"

#define SHAPE_TYPE_FIELDS 4
#define SHAPE_TYPE_FIELD_COLOR "color"
#define SHAPE_TYPE_FIELD_X "x"
#define SHAPE_TYPE_FIELD_Y "y"
#define SHAPE_TYPE_FIELD_SHAPESIZE "shapesize"

struct ShapesAgent {
    DDS_DomainParticipant *dp;

    RTI_MQTT_KeyedMessageDataWriter *mqtt_message_writer;
    RTI_MQTT_KeyedMessageDataReader *mqtt_message_reader;

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

    RTI_MQTT_KeyedMessage *pub_msg;
    DDS_DynamicData *shape;
    char *topic_squares;
    char *topic_circles;
    char *topic_triangles;
};

#define SHAPES_AGENT_PROFILES "file://etc/shapes_agent_profiles.xml"

#define SHAPES_AGENT_TOPIC_SQUARES "agent/squares"
#define SHAPES_AGENT_TOPIC_CIRCLES "agent/circles"
#define SHAPES_AGENT_TOPIC_TRIANGLES "agent/triangles"
#define SHAPES_AGENT_COLOR "RED"
#define SHAPES_AGENT_SIZE_MIN 30
#define SHAPES_AGENT_SIZE_MAX 60
#define SHAPES_AGENT_X_MIN 0
#define SHAPES_AGENT_X_MAX 240
#define SHAPES_AGENT_Y_MIN 0
#define SHAPES_AGENT_Y_MAX 240
#define SHAPES_AGENT_QOS RTI_MQTT_QosLevel_ZERO
#define SHAPES_AGENT_RETAINED DDS_BOOLEAN_TRUE

#define SHAPES_AGENT_PARTICIPANT "ShapesAgentParticipants::agent"
#define SHAPES_AGENT_WRITER_MQTT "publisher::mqtt_messages"
#define SHAPES_AGENT_READER_MQTT "subscriber::mqtt_messages"

#define SHAPES_AGENT_WRITER_MESSAGE_FMT \
    "{ \"color\" : \"%s\", \"x\" : %d, \"y\" : %d, \"shapesize\" : %d }"
#ifndef SHAPES_AGENT_WRITER_LOOP_SEC
    #define SHAPES_AGENT_WRITER_LOOP_SEC 1
#endif /* SHAPES_AGENT_WRITER_LOOP_SEC */
#define SHAPES_AGENT_WRITER_LOOP_NSEC 0

void ShapesAgent_finalize(struct ShapesAgent *self);

int ShapesAgent_initialize(struct ShapesAgent *self);

int ShapesAgent_main(struct ShapesAgent *self);

void ShapesAgent_print_mqtt_message(
        struct ShapesAgent *self,
        RTI_MQTT_KeyedMessage *msg,
        DDS_Boolean in);

int ShapesAgent_publish_message(struct ShapesAgent *self);

void *ShapesAgent_thread_subscriber(void *arg);

void *ShapesAgent_thread_publisher(void *arg);

DDS_ReturnCode_t ShapesAgent_deserialize_shape(
        const char *json_buffer,
        DDS_UnsignedLong json_buffer_size,
        DDS_DynamicData *sample);

#endif /* ShapesShapesAgent_h */
