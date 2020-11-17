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

#include <signal.h>

#include "ShapesAgentDds.h"
#include "json.h"
#include "termcolor.h"

#define RTI_MQTT_LOG_ARGS "ShapesAgent::DDS"

static const char *ShapesAgent_fv_message_format =
        SHAPES_AGENT_WRITER_MESSAGE_FMT;

static DDS_GuardCondition *ShapesAgent_fv_exit_condition = NULL;

static void ShapesAgent_signal_handler(int sig);

static void ShapesAgent_setup_signal_handler()
{
    signal(SIGINT, ShapesAgent_signal_handler);
}


static void ShapesAgent_signal_handler(int sig)
{
    RTI_MQTT_LOG_1("exiting on signal", "%d", sig)

    if (ShapesAgent_fv_exit_condition != NULL) {
        if (DDS_RETCODE_OK
            != DDS_GuardCondition_set_trigger_value(
                    ShapesAgent_fv_exit_condition,
                    DDS_BOOLEAN_TRUE)) {
            RTI_MQTT_ERROR("failed to trigger exit condition")
            return;
        }
    }

    ShapesAgent_setup_signal_handler();
}


void ShapesAgent_finalize(struct ShapesAgent *self)
{
    if (self->cond_exit != NULL) {
        DDS_GuardCondition_set_trigger_value(self->cond_exit, DDS_BOOLEAN_TRUE);
    }

    if (self->thread_sub != NULL) {
        RTI_MQTT_Thread_join(self->thread_sub, NULL);
        self->thread_sub = NULL;
    }

    if (self->thread_pub != NULL) {
        RTI_MQTT_Thread_join(self->thread_pub, NULL);
        self->thread_pub = NULL;
    }

    DDS_ConditionSeq_finalize(&self->active_conditions);
    if (self->cond_exit != NULL) {
        DDS_WaitSet_detach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_exit));
        DDS_GuardCondition_delete(self->cond_exit);
        self->cond_exit = NULL;
    }
    if (self->cond_timeout != NULL) {
        DDS_WaitSet_detach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_timeout));
        DDS_GuardCondition_delete(self->cond_timeout);
        self->cond_timeout = NULL;
    }
    if (self->waitset) {
        DDS_WaitSet_delete(self->waitset);
        self->waitset = NULL;
    }
    if (self->dp != NULL) {
        DDS_DomainParticipant_delete_contained_entities(self->dp);
        DDS_DomainParticipantFactory_delete_participant(
                DDS_TheParticipantFactory,
                self->dp);
        self->dp = NULL;
        self->mqtt_message_writer = NULL;
        self->mqtt_message_reader = NULL;
    }
    if (self->topic_circles != NULL) {
        DDS_String_free(self->topic_circles);
        self->topic_circles = NULL;
    }
    if (self->topic_squares != NULL) {
        DDS_String_free(self->topic_squares);
        self->topic_squares = NULL;
    }
    if (self->topic_triangles != NULL) {
        DDS_String_free(self->topic_triangles);
        self->topic_triangles = NULL;
    }
    if (self->pub_msg != NULL) {
        self->pub_msg->topic = DDS_String_dup("");
        RTI_MQTT_KeyedMessageTypeSupport_delete_data(self->pub_msg);
        self->pub_msg = NULL;
    }
    if (self->shape != NULL) {
        DDS_DynamicData_delete(self->shape);
        self->shape = NULL;
    }
}

int ShapesAgent_initialize(struct ShapesAgent *self)
{
    int rc = 1;
    const char *type_name = NULL;
    char **str_ref = NULL;
    DDS_Boolean active_conditions_initd = DDS_BOOLEAN_FALSE;
    struct DDS_DomainParticipantFactoryQos dpf_qos =
            DDS_DomainParticipantFactoryQos_INITIALIZER;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;
    DDS_Entity *dp_entity = NULL;
    DDS_InstanceHandle_t dp_handle = DDS_HANDLE_NIL;

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = RTI_TRUE;

    self->dp = NULL;
    self->mqtt_message_writer = NULL;
    self->mqtt_message_reader = NULL;
    self->waitset = NULL;
    self->cond_exit = NULL;
    self->cond_timeout = NULL;
    self->msg_recvd_max = 0;
    self->msg_recvd_tot = 0;
    self->msg_sent_max = 0;
    self->msg_sent_tot = 0;
    self->pub_msg = NULL;
    self->shape = NULL;
    self->thread_pub = NULL;
    self->thread_sub = NULL;
    self->topic_circles = NULL;
    self->topic_squares = NULL;
    self->topic_triangles = NULL;
    if (!DDS_ConditionSeq_initialize(&self->active_conditions)) {
        RTI_MQTT_ERROR("failed to initialize conditions sequence")
        goto done;
    }
    active_conditions_initd = DDS_BOOLEAN_TRUE;

    /* Disable default QoS sources, and configure qos profile library
       file explicitly */
    if (DDS_RETCODE_OK
        != DDS_DomainParticipantFactory_get_qos(
                DDS_TheParticipantFactory,
                &dpf_qos)) {
        RTI_MQTT_ERROR("failed to get qos of DomainParticipantFactory")
        goto done;
    }

    if (!DDS_StringSeq_ensure_length(&dpf_qos.profile.url_profile, 1, 1)) {
        RTI_MQTT_ERROR("failed to set max of DPFQos::profile::url_profile")
        goto done;
    }

    str_ref = DDS_StringSeq_get_reference(&dpf_qos.profile.url_profile, 0);

    *str_ref = DDS_String_dup(SHAPES_AGENT_PROFILES);
    if (*str_ref == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to duplicate string:",
                "%s",
                SHAPES_AGENT_PROFILES)
        goto done;
    }

#if 0
    dpf_qos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
#endif
    dpf_qos.profile.ignore_user_profile = DDS_BOOLEAN_TRUE;
    dpf_qos.profile.ignore_environment_profile = DDS_BOOLEAN_TRUE;
    dpf_qos.profile.ignore_resource_profile = DDS_BOOLEAN_TRUE;

    if (DDS_RETCODE_OK
        != DDS_DomainParticipantFactory_set_qos(
                DDS_TheParticipantFactory,
                &dpf_qos)) {
        RTI_MQTT_ERROR("failed to get qos of DomainParticipantFactory")
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DomainParticipantFactory_register_type_support(
                DDS_TheParticipantFactory,
                RTI_MQTT_MessageTypeSupport_register_type,
                RTI_MQTT_MessageTypeSupport_get_type_name())) {
        RTI_MQTT_ERROR("failed to register type support: Message")
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DomainParticipantFactory_register_type_support(
                DDS_TheParticipantFactory,
                RTI_MQTT_KeyedMessageTypeSupport_register_type,
                RTI_MQTT_KeyedMessageTypeSupport_get_type_name())) {
        RTI_MQTT_ERROR("failed to register type support: KeyedMessage")
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DomainParticipantFactory_register_type_support(
                DDS_TheParticipantFactory,
                ShapeTypeTypeSupport_register_type,
                ShapeTypeTypeSupport_get_type_name())) {
        RTI_MQTT_ERROR("failed to register type support: ShapeType")
        goto done;
    }

    self->dp = DDS_DomainParticipantFactory_create_participant_from_config(
            DDS_TheParticipantFactory,
            SHAPES_AGENT_PARTICIPANT);

    if (self->dp == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to lookup participant",
                "%s",
                SHAPES_AGENT_PARTICIPANT)
        goto done;
    }

    dp_entity = DDS_DomainParticipant_as_entity(self->dp);
    dp_handle = DDS_Entity_get_instance_handle(dp_entity);
    if (DDS_RETCODE_OK
        != DDS_DomainParticipant_ignore_participant(self->dp, &dp_handle)) {
        RTI_MQTT_ERROR("failed to ignore participant")
        goto done;
    }
#if 0
    if (DDS_RETCODE_OK != DDS_Entity_enable(dp_entity))
    {
        RTI_MQTT_ERROR("failed to enable participant");
    }
#endif

    RTI_MQTT_LOG_1(
            "looked up DomainParticipant:",
            "%s",
            SHAPES_AGENT_PARTICIPANT)

    {
        DDS_DataReader *reader = NULL;
        reader = DDS_DomainParticipant_lookup_datareader_by_name(
                self->dp,
                SHAPES_AGENT_READER_MQTT);
        if (reader == NULL) {
            RTI_MQTT_ERROR_1(
                    "failed to lookup reader",
                    "%s",
                    SHAPES_AGENT_READER_MQTT)
            goto done;
        }
        self->mqtt_message_reader =
                RTI_MQTT_KeyedMessageDataReader_narrow(reader);
        if (self->mqtt_message_reader == NULL) {
            RTI_MQTT_ERROR_1(
                    "failed to narrow reader",
                    "%s",
                    SHAPES_AGENT_READER_MQTT)
            goto done;
        }
        RTI_MQTT_LOG_1(
                "looked up DataReader:",
                "%s",
                SHAPES_AGENT_PARTICIPANT "::" SHAPES_AGENT_READER_MQTT)
    }
    {
        DDS_DataWriter *writer = NULL;
        writer = DDS_DomainParticipant_lookup_datawriter_by_name(
                self->dp,
                SHAPES_AGENT_WRITER_MQTT);
        if (writer == NULL) {
            RTI_MQTT_ERROR_1(
                    "failed to lookup writer",
                    "%s",
                    SHAPES_AGENT_WRITER_MQTT)
            goto done;
        }
        self->mqtt_message_writer =
                RTI_MQTT_KeyedMessageDataWriter_narrow(writer);
        if (self->mqtt_message_writer == NULL) {
            RTI_MQTT_ERROR_1(
                    "failed to narrow writer",
                    "%s",
                    SHAPES_AGENT_WRITER_MQTT)
            goto done;
        }
        RTI_MQTT_LOG_1(
                "looked up DataWriter:",
                "%s",
                SHAPES_AGENT_PARTICIPANT "::" SHAPES_AGENT_WRITER_MQTT)
    }

    if (!DDS_ConditionSeq_set_maximum(&self->active_conditions, 2)) {
        RTI_MQTT_ERROR("failed to set max on conditions sequence")
        goto done;
    }

    self->waitset = DDS_WaitSet_new();
    if (self->waitset == NULL) {
        RTI_MQTT_ERROR("failed to create waitset")
        goto done;
    }

    if (ShapesAgent_fv_exit_condition == NULL) {
        ShapesAgent_fv_exit_condition = DDS_GuardCondition_new();
        if (ShapesAgent_fv_exit_condition == NULL) {
            RTI_MQTT_ERROR("failed to create exit condition")
            goto done;
        }
    }
    self->cond_exit = ShapesAgent_fv_exit_condition;

    self->cond_timeout = DDS_GuardCondition_new();
    if (self->cond_timeout == NULL) {
        RTI_MQTT_ERROR("failed to create timeout condition")
        goto done;
    }

    self->topic_circles = DDS_String_dup(SHAPES_AGENT_TOPIC_CIRCLES);
    if (self->topic_circles == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to clone string:",
                "%s",
                SHAPES_AGENT_TOPIC_CIRCLES)
        goto done;
    }
    self->topic_squares = DDS_String_dup(SHAPES_AGENT_TOPIC_SQUARES);
    if (self->topic_squares == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to clone string:",
                "%s",
                SHAPES_AGENT_TOPIC_SQUARES)
        goto done;
    }
    self->topic_triangles = DDS_String_dup(SHAPES_AGENT_TOPIC_TRIANGLES);
    if (self->topic_triangles == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to clone string:",
                "%s",
                SHAPES_AGENT_TOPIC_TRIANGLES)
        goto done;
    }

    {
        DDS_UnsignedLong msg_buf_len =
                RTI_TSFM_String_length(ShapesAgent_fv_message_format) + 20;

        self->pub_msg = RTI_MQTT_KeyedMessageTypeSupport_create_data_w_params(
                &alloc_params);
        if (self->pub_msg == NULL) {
            RTI_MQTT_ERROR("failed to allocate publisher message")
            goto done;
        }

        if (!DDS_OctetSeq_ensure_length(
                    &self->pub_msg->payload.data,
                    msg_buf_len,
                    msg_buf_len)) {
            RTI_MQTT_ERROR("failed to allocate message buffer")
            goto done;
        }

        self->pub_msg->info->qos_level = SHAPES_AGENT_QOS;
        self->pub_msg->info->retained = SHAPES_AGENT_RETAINED;
        DDS_String_free(self->pub_msg->topic);
        self->pub_msg->topic = NULL;
    }

    self->shape = DDS_DynamicData_new(
            ShapeType_get_typecode(),
            &DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    if (self->shape == NULL) {
        RTI_MQTT_ERROR("failed to create shape sample")
        goto done;
    }

    ShapesAgent_setup_signal_handler();

    rc = 0;
done:

    if (DDS_RETCODE_OK != DDS_DomainParticipantFactoryQos_finalize(&dpf_qos)) {
        RTI_MQTT_ERROR("failed to finalize DomainParticipantFactoryQos")
    }

    if (rc != 0) {
        if (active_conditions_initd) {
            ShapesAgent_finalize(self);
        }
    }

    return rc;
}

static DDS_Long random_no(DDS_Long min, DDS_Long max)
{
    DDS_Long rand_no = min + (rand() % (max - min));
    RTI_MQTT_TRACE_3("RANDOM", "[%d,%d] => %d", min, max, rand_no)
    return rand_no;
}

static void ShapesAgent_get_shape_position(
        struct ShapesAgent *self,
        DDS_Long size_min,
        DDS_Long size_max,
        DDS_Long x_min,
        DDS_Long x_max,
        DDS_Long y_min,
        DDS_Long y_max,
        DDS_Long *size,
        DDS_Long *x,
        DDS_Long *y)
{
    *size = random_no(size_min, size_max);
    *x = random_no(x_min + (*size / 2), x_max - (*size / 2));
    *y = random_no(y_min + (*size / 2), y_max - (*size / 2));
}

int ShapesAgent_publish_message(struct ShapesAgent *self)
{
    int rc = 1;
    char *msg_buf = NULL, *msg_topic = NULL, *cur_topic = NULL;
    DDS_UnsignedLong msg_fmt_len = 0, msg_len = 0;
    DDS_Long x = 0, y = 0, size = 0;

    self->msg_sent_tot += 1;

    msg_buf = (char *) DDS_OctetSeq_get_contiguous_buffer(
            &self->pub_msg->payload.data);
    if (msg_buf == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to get message buffer",
                "(%d)",
                self->msg_sent_tot)
    }

    ShapesAgent_get_shape_position(
            self,
            SHAPES_AGENT_SIZE_MIN,
            SHAPES_AGENT_SIZE_MAX,
            SHAPES_AGENT_X_MIN,
            SHAPES_AGENT_X_MAX,
            SHAPES_AGENT_Y_MIN,
            SHAPES_AGENT_Y_MAX,
            &size,
            &x,
            &y);

    sprintf(msg_buf,
            ShapesAgent_fv_message_format,
            SHAPES_AGENT_COLOR,
            x,
            y,
            size);

    msg_len = strlen(msg_buf);
    if (!DDS_OctetSeq_set_length(&self->pub_msg->payload.data, msg_len)) {
        RTI_MQTT_ERROR_1(
                "failed to set payload length for message",
                "(%d)",
                self->msg_sent_tot)
        goto done;
    }

    /* Set MQTT topic to one of the three topics based on the message counter.
     * We "cache" the current string, and set self->pub_msg->topic to a static
     * string to avoid doing memory allocation here. We will restore the value
     * to the original to avoid freeing static memory when the sample is
     * eventually finalized */

    switch (self->msg_sent_tot % 3) {
    case 0:
        self->pub_msg->topic = self->topic_circles;
        break;

    case 1:
        self->pub_msg->topic = self->topic_squares;
        break;

    case 2:
        self->pub_msg->topic = self->topic_triangles;
        break;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_KeyedMessageDataWriter_write(
                self->mqtt_message_writer,
                self->pub_msg,
                &DDS_HANDLE_NIL)) {
        RTI_MQTT_ERROR_1("failed to write message", "(%d)", self->msg_sent_tot)
        goto done;
    }

    rc = 0;
done:

    return rc;
}


void ShapesAgent_print_mqtt_message(
        struct ShapesAgent *self,
        RTI_MQTT_KeyedMessage *msg,
        DDS_Boolean in)
{
    char *msg_payload = NULL, *color = NULL;
    DDS_UnsignedLong msg_payload_len = 0, msg_tot = 0, i = 0, color_len = 0;
    const char *prefix = NULL;

    if (in) {
        msg_tot = self->msg_recvd_tot;
    } else {
        msg_tot = self->msg_sent_tot;
    }

    if (strstr(msg->topic, "/circles") != NULL) {
        if (in) {
            prefix = " recv ( ) ";
        } else {
            prefix = " sent ( ) ";
        }
    } else if (strstr(msg->topic, "/squares") != NULL) {
        if (in) {
            prefix = " recv [ ] ";
        } else {
            prefix = " sent [ ] ";
        }
    } else if (strstr(msg->topic, "/triangles") != NULL) {
        if (in) {
            prefix = " recv / \\ ";
        } else {
            prefix = " sent / \\ ";
        }
    }

    msg_payload_len = DDS_OctetSeq_get_length(&msg->payload.data);
    if (msg_payload_len == 0) {
        msg_payload = "<none>";
        msg_payload_len = RTI_TSFM_String_length(msg_payload);
    } else {
        msg_payload = (char *) DDS_OctetSeq_get_contiguous_buffer(
                &msg->payload.data);

        if (DDS_RETCODE_OK
            != ShapesAgent_deserialize_shape(
                    msg_payload,
                    msg_payload_len,
                    self->shape)) {
            RTI_MQTT_ERROR("failed to deserialize JSON shape")
            goto done;
        }

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_string(
                    self->shape,
                    &color,
                    &color_len,
                    "color",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_MQTT_ERROR("failed to get 'color' field")
            goto done;
        }

        reset_colors(stdout);
        if (RTI_TSFM_String_compare(color, "RED") == 0) {
            background_red(stdout);
        } else if (RTI_TSFM_String_compare(color, "GREEN") == 0) {
            background_green(stdout);
        } else if (RTI_TSFM_String_compare(color, "ORANGE") == 0) {
            background_yellow(stdout);
            text_red(stdout);
        } else if (RTI_TSFM_String_compare(color, "YELLOW") == 0) {
            background_yellow(stdout);
        } else if (RTI_TSFM_String_compare(color, "BLUE") == 0) {
            background_blue(stdout);
        } else if (RTI_TSFM_String_compare(color, "CYAN") == 0) {
            background_cyan(stdout);
        } else if (RTI_TSFM_String_compare(color, "MAGENTA") == 0) {
            background_magenta(stdout);
        } else if (RTI_TSFM_String_compare(color, "PURPLE") == 0) {
            background_magenta(stdout);
            text_cyan(stdout);
        }
    }

    // printf("%s(%d)\n",action,msg_tot);

    if (msg->info == NULL) {
        printf("%s[%d] '%s' [no message info]\n", prefix, msg_tot, msg->topic);
    } else {
        printf("%s[%d] '%s' [%s%s%s]\n",
               prefix,
               msg_tot,
               msg->topic,
               (msg->info->qos_level == RTI_MQTT_QosLevel_ZERO)
                       ? "QOS_0"
                       : (msg->info->qos_level == RTI_MQTT_QosLevel_ONE)
                               ? "QOS_1"
                               : (msg->info->qos_level == RTI_MQTT_QosLevel_TWO)
                                       ? "QOS_2"
                                       : "QOS_UNKNOWN",
               msg->info->retained ? ", RETAINED" : "",
               msg->info->duplicate ? ", DUPLICATE" : "");
    }

    printf("%s[%d] ", prefix, msg_tot);
    for (i = 0; i < msg_payload_len; i++) {
        fputc(msg_payload[i], stdout);
    }
    fputc('\n', stdout);

    reset_colors(stdout);

done:
    if (color != NULL) {
        RTI_TSFM_Heap_free(color);
    }
}


void ShapesAgent_print_shape(
        struct ShapesAgent *self,
        void *entity,
        ShapeType *shape,
        DDS_Boolean in)
{
    ShapeTypeDataWriter *writer = NULL;
    ShapeTypeDataReader *reader = NULL;
    DDS_UnsignedLong msg_tot = 0, i = 0;
    const char *prefix = NULL, *action = NULL, *topic = NULL;

    if (in) {
        prefix = " << ";
        action = "RECEIVED";
        msg_tot = self->msg_recvd_tot;
        reader = (ShapeTypeDataReader *) entity;
        topic = DDS_TopicDescription_get_name(
                DDS_DataReader_get_topicdescription(
                        ShapeTypeDataReader_as_datareader(reader)));
    } else {
        prefix = " >> ";
        action = "PUBLISHED";
        msg_tot = self->msg_sent_tot;
        writer = (ShapeTypeDataWriter *) entity;
        topic = DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(DDS_DataWriter_get_topic(
                        ShapeTypeDataWriter_as_datawriter(writer))));
    }

    printf("%s(%d)\n", action, msg_tot);
    printf("%sSHAPE[%s/%d] color='%s' x=%d y=%d shapesize=%d\n",
           prefix,
           topic,
           msg_tot,
           shape->color,
           shape->x,
           shape->y,
           shape->shapesize);
}


int ShapesAgent_main(struct ShapesAgent *self)
{
    int rc = 1;
    unsigned long seq_len = 0, i = 0;
    DDS_Boolean cond_attached_exit = DDS_BOOLEAN_FALSE,
                cond_attached_timeout = DDS_BOOLEAN_FALSE,
                cond_triggered_exit = DDS_BOOLEAN_FALSE,
                cond_triggered_timeout = DDS_BOOLEAN_FALSE,
                agent_done = DDS_BOOLEAN_FALSE;
    struct DDS_Duration_t wait_timeout = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    if (DDS_RETCODE_OK
        != DDS_WaitSet_attach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_exit))) {
        RTI_MQTT_ERROR("failed to attach exit condition")
        goto done;
    }
    cond_attached_exit = DDS_BOOLEAN_TRUE;


    if (DDS_RETCODE_OK
        != DDS_WaitSet_attach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_timeout))) {
        RTI_MQTT_ERROR("failed to attach timeout condition")
        goto done;
    }
    cond_attached_timeout = DDS_BOOLEAN_TRUE;

#if 1
    if (DDS_RETCODE_OK
        != RTI_MQTT_Thread_spawn(
                ShapesAgent_thread_publisher,
                self,
                &self->thread_pub)) {
        RTI_MQTT_ERROR("failed to spawn publisher thread")
        goto done;
    }
#endif
#if 1
    if (DDS_RETCODE_OK
        != RTI_MQTT_Thread_spawn(
                ShapesAgent_thread_subscriber,
                self,
                &self->thread_sub)) {
        RTI_MQTT_ERROR("failed to spawn subscriber thread")
        goto done;
    }
#endif
    while (!agent_done) {
        cond_triggered_exit = DDS_BOOLEAN_FALSE;
        cond_triggered_timeout = DDS_BOOLEAN_FALSE;
        retcode = DDS_WaitSet_wait(
                self->waitset,
                &self->active_conditions,
                &wait_timeout);

        switch (retcode) {
        case DDS_RETCODE_OK:
            break;
        case DDS_RETCODE_TIMEOUT:
            RTI_MQTT_LOG("wait timed out")
            break;
        default:
            RTI_MQTT_ERROR_1("wait error", "%d", retcode)
            break;
        }

        seq_len = DDS_ConditionSeq_get_length(&self->active_conditions);
        for (i = 0; i < seq_len; i++) {
            DDS_Condition *cond = *DDS_ConditionSeq_get_reference(
                    &self->active_conditions,
                    i);

            if (cond == DDS_GuardCondition_as_condition(self->cond_exit)) {
                cond_triggered_exit = DDS_BOOLEAN_TRUE;
            } else if (DDS_GuardCondition_as_condition(self->cond_exit)) {
                cond_triggered_timeout = DDS_BOOLEAN_TRUE;
            }
        }

        agent_done = cond_triggered_exit || cond_triggered_timeout;
    }


    rc = 0;
done:

    if (cond_attached_exit) {
        DDS_WaitSet_detach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_exit));
    }
    if (cond_attached_timeout) {
        DDS_WaitSet_detach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_timeout));
    }

    return rc;
}


DDS_ReturnCode_t ShapesAgent_deserialize_shape(
        const char *json_buffer,
        DDS_UnsignedLong json_buffer_size,
        DDS_DynamicData *shape)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    json_value *value = NULL, *member_value = NULL;
    json_object_entry *member = NULL;
    DDS_UnsignedLong i = 0;
    DDS_Long member_val_l = 0;

    /* json_buffer should not be printed with printf() because there is
     * no guarantee that the string is well terminated (and most likely
     * it won't be, i.e. no 'nul' terminator) */

    value = json_parse(json_buffer, json_buffer_size);
    if (value == NULL) {
        /* TODO Log error */
        RTI_MQTT_ERROR("failed: json_parse")
        goto done;
    }
    if (value->type != json_object) {
        /* TODO Log error */
        RTI_MQTT_ERROR("failed: invalid parsed value")
        goto done;
    }
    /* ShapeType has 4 fields */
    if (value->u.object.length < SHAPE_TYPE_FIELDS) {
        /* TODO Log error */
        RTI_MQTT_ERROR("failed: insufficient fields")
        goto done;
    }

    for (i = 0; i < value->u.object.length; i++) {
        member = &value->u.object.values[i];
        member_value = member->value;

        if (RTI_TSFM_String_compare(member->name, SHAPE_TYPE_FIELD_COLOR)
            == 0) {
            switch (member_value->type) {
            case json_string:

                if (member_value->u.string.length == 0) {
                    /* TODO Log error */
                    RTI_MQTT_ERROR("failed: invalid COLOR string")
                    goto done;
                }

                break;

            default:
                /* TODO Log error */
                RTI_MQTT_ERROR("failed: COLOR field type")
                goto done;
            }

            if (DDS_RETCODE_OK
                != DDS_DynamicData_set_string(
                        shape,
                        SHAPE_TYPE_FIELD_COLOR,
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                        member_value->u.string.ptr)) {
                /* TODO Log error */
                RTI_MQTT_ERROR("failed: set COLOR")
                goto done;
            }
        } else if (
                RTI_TSFM_String_compare(member->name, SHAPE_TYPE_FIELD_X)
                == 0) {
            switch (member_value->type) {
            case json_string:

                member_val_l = RTI_TSFM_String_to_long(
                        member_value->u.string.ptr,
                        NULL,
                        0);
                break;

            case json_integer:

                member_val_l = member_value->u.integer;
                break;

            default:
                /* TODO Log error */
                RTI_MQTT_ERROR("failed: X field type")
                goto done;
            }

            if (DDS_RETCODE_OK
                != DDS_DynamicData_set_long(
                        shape,
                        SHAPE_TYPE_FIELD_X,
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                        member_val_l)) {
                /* TODO Log error */
                RTI_MQTT_ERROR("failed: set X")
                goto done;
            }
        } else if (
                RTI_TSFM_String_compare(member->name, SHAPE_TYPE_FIELD_Y)
                == 0) {
            switch (member_value->type) {
            case json_string:

                member_val_l = RTI_TSFM_String_to_long(
                        member_value->u.string.ptr,
                        NULL,
                        0);
                break;

            case json_integer:

                member_val_l = member_value->u.integer;
                break;

            default:
                /* TODO Log error */
                RTI_MQTT_ERROR("failed: Y field type")
                goto done;
            }

            if (DDS_RETCODE_OK
                != DDS_DynamicData_set_long(
                        shape,
                        SHAPE_TYPE_FIELD_Y,
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                        member_val_l)) {
                /* TODO Log error */
                RTI_MQTT_ERROR("failed: set Y")
                goto done;
            }
        } else if (
                RTI_TSFM_String_compare(
                        member->name,
                        SHAPE_TYPE_FIELD_SHAPESIZE)
                == 0) {
            switch (member_value->type) {
            case json_string:

                member_val_l = RTI_TSFM_String_to_long(
                        member_value->u.string.ptr,
                        NULL,
                        0);
                break;

            case json_integer:

                member_val_l = member_value->u.integer;
                break;

            default:
                /* TODO Log error */
                RTI_MQTT_ERROR("failed: SHAPESIZE field type")
                goto done;
            }


            if (DDS_RETCODE_OK
                != DDS_DynamicData_set_long(
                        shape,
                        SHAPE_TYPE_FIELD_SHAPESIZE,
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                        member_val_l)) {
                /* TODO Log error */
                RTI_MQTT_ERROR("failed: set SHAPESIZE")
                goto done;
            }
        }
    }

#if 0
    DDS_DynamicData_print(shape, stdout, 0);
#endif

    retcode = DDS_RETCODE_OK;

done:
    if (value != NULL) {
        json_value_free(value);
    }

    return retcode;
}