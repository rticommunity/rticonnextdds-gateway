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

#include "MqttAgent.h"

#define RTI_MQTT_LOG_ARGS "MqttAgent"

void MqttAgent_finalize(struct MqttAgent *self)
{
    if (self->cond_exit != NULL) {
        DDS_GuardCondition_set_trigger_value(self->cond_exit, DDS_BOOLEAN_TRUE);
    }

    if (self->thread_sub != NULL) {
        RTI_MQTT_Thread_join(self, self->thread_sub);
    }

    if (self->thread_pub != NULL) {
        RTI_MQTT_Thread_join(self, self->thread_pub);
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
        DDS_DomainParticipantFactory_delete_participant(
                DDS_TheParticipantFactory,
                self->dp);
        self->dp = NULL;
        self->writer = NULL;
        self->reader = NULL;
    }
}

int MqttAgent_initialize(
        struct MqttAgent *self
#if USE_APP_GEN
        ,
        const char *participant_name,
        const char *reader_name,
        const char *writer_name
#endif /* USE_APP_GEN */
)
{
    int rc = 1;
    const char *type_name = NULL;
    char **str_ref = NULL;
    DDS_Boolean active_conditions_initd = DDS_BOOLEAN_FALSE;
    struct DDS_DomainParticipantFactoryQos dpf_qos =
            DDS_DomainParticipantFactoryQos_INITIALIZER;

    self->dp = NULL;
#if !USE_APP_GEN
    self->sub = NULL;
    self->pub = NULL;
    self->topic_in = NULL;
    self->topic_out = NULL;
#endif /* !USE_APP_GEN */
    self->writer = NULL;
    self->reader = NULL;
    self->waitset = NULL;
    self->cond_exit = NULL;
    self->cond_timeout = NULL;
    self->msg_recvd_max = 0;
    self->msg_recvd_tot = 0;
    self->msg_sent_max = 0;
    self->msg_sent_tot = 0;
    self->pub_msg = NULL;
    self->thread_pub = NULL;
    self->thread_sub = NULL;

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

    *str_ref = DDS_String_dup(MQTT_AGENT_PROFILES);
    if (*str_ref == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to duplicate string:",
                "%s",
                MQTT_AGENT_PROFILES)
        goto done;
    }

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

#if USE_APP_GEN
    if (DDS_RETCODE_OK
        != DDS_DomainParticipantFactory_register_type_support(
                DDS_TheParticipantFactory,
                RTI_MQTT_MessageTypeSupport_register_type,
                RTI_MQTT_MessageTypeSupport_get_type_name())) {
        RTI_MQTT_ERROR("failed to register type support")
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DomainParticipantFactory_register_type_support(
                DDS_TheParticipantFactory,
                RTI_MQTT_KeyedMessageTypeSupport_register_type,
                RTI_MQTT_KeyedMessageTypeSupport_get_type_name())) {
        RTI_MQTT_ERROR("failed to register type support")
        goto done;
    }

    self->dp = DDS_DomainParticipantFactory_create_participant_from_config(
            DDS_TheParticipantFactory,
            participant_name);

    if (self->dp == NULL) {
        RTI_MQTT_ERROR_1("failed to lookup participant", "%s", participant_name)
        goto done;
    }

    {
        DDS_DataReader *reader = NULL;
        reader = DDS_DomainParticipant_lookup_datareader_by_name(
                self->dp,
                reader_name);
        if (reader == NULL) {
            RTI_MQTT_ERROR_1("failed to lookup reader", "%s", reader_name)
            goto done;
        }
        self->reader = RTI_MQTT_KeyedMessageDataReader_narrow(reader);
        if (self->reader == NULL) {
            RTI_MQTT_ERROR_1("failed to narrow reader", "%s", reader_name)
            goto done;
        }
    }

    {
        DDS_DataWriter *writer = NULL;
        writer = DDS_DomainParticipant_lookup_datawriter_by_name(
                self->dp,
                writer_name);
        if (writer == NULL) {
            RTI_MQTT_ERROR_1("failed to lookup writer", "%s", writer_name)
            goto done;
        }
        self->writer = RTI_MQTT_MessageDataWriter_narrow(writer);
        if (self->writer == NULL) {
            RTI_MQTT_ERROR_1("failed to narrow writer", "%s", writer_name)
            goto done;
        }
    }
#else
    self->dp = DDS_DomainParticipantFactory_create_participant_with_profile(
            DDS_TheParticipantFactory,
            MQTT_AGENT_DOMAIN,
            MQTT_AGENT_QOS_LIBRARY,
            MQTT_AGENT_PARTICIPANT_PROFILE,
            NULL,
            DDS_STATUS_MASK_NONE);
    if (self->dp == NULL) {
        RTI_MQTT_ERROR_3(
                "failed to create participant:",
                "domain=%d, lib='%s', profile='%s",
                MQTT_AGENT_DOMAIN,
                MQTT_AGENT_QOS_LIBRARY,
                MQTT_AGENT_PARTICIPANT_PROFILE)
        goto done;
    }

    type_name = RTI_MQTT_MessageTypeSupport_get_type_name();
    if (DDS_RETCODE_OK
        != RTI_MQTT_MessageTypeSupport_register_type(self->dp, type_name)) {
        RTI_MQTT_ERROR_1("failed to register type:", "%s", type_name)
        goto done;
    }

    self->topic_in = DDS_DomainParticipant_create_topic(
            self->dp,
            MQTT_AGENT_TOPIC_INPUT,
            type_name,
            &DDS_TOPIC_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);

    if (self->topic_in == NULL) {
        RTI_MQTT_ERROR_2(
                "failed to create input topic:",
                "topic='%s', type='%s'",
                MQTT_AGENT_TOPIC_INPUT,
                type_name)
        goto done;
    }

    self->topic_out = DDS_DomainParticipant_create_topic(
            self->dp,
            MQTT_AGENT_TOPIC_OUTPUT,
            type_name,
            &DDS_TOPIC_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);

    if (self->topic_out == NULL) {
        RTI_MQTT_ERROR_2(
                "failed to create output topic:",
                "topic='%s', type='%s'",
                MQTT_AGENT_TOPIC_OUTPUT,
                type_name)
        goto done;
    }

    self->sub = DDS_DomainParticipant_create_subscriber(
            self->dp,
            &DDS_SUBSCRIBER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);

    if (self->sub == NULL) {
        RTI_MQTT_ERROR("failed to create subscriber")
        goto done;
    }

    self->pub = DDS_DomainParticipant_create_publisher(
            self->dp,
            &DDS_PUBLISHER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);

    if (self->pub == NULL) {
        RTI_MQTT_ERROR("failed to create publisher")
        goto done;
    }

    {
        DDS_DataReader *reader = NULL;
        reader = DDS_Subscriber_create_datareader_with_profile(
                self->sub,
                DDS_Topic_as_topicdescription(self->topic_in),
                MQTT_AGENT_QOS_LIBRARY,
                MQTT_AGENT_READER_PROFILE,
                NULL,
                DDS_STATUS_MASK_NONE);
        if (reader == NULL) {
            RTI_MQTT_ERROR("failed to create input reader")
            goto done;
        }
        self->reader = RTI_MQTT_KeyedMessageDataReader_narrow(reader);
        if (self->reader == NULL) {
            RTI_MQTT_ERROR("failed to narrow input reader")
            goto done;
        }
    }

    {
        DDS_DataWriter *writer = NULL;
        writer = DDS_Publisher_create_datawriter_with_profile(
                self->pub,
                self->topic_out,
                MQTT_AGENT_QOS_LIBRARY,
                MQTT_AGENT_WRITER_PROFILE,
                NULL,
                DDS_STATUS_MASK_NONE);
        if (writer == NULL) {
            RTI_MQTT_ERROR("failed to create output writer")
            goto done;
        }
        self->writer = RTI_MQTT_MessageDataWriter_narrow(writer);
        if (self->writer == NULL) {
            RTI_MQTT_ERROR("failed to narrow output writer")
            goto done;
        }
    }

#endif /* USE_APP_GEN */

    if (!DDS_ConditionSeq_set_maximum(&self->active_conditions, 2)) {
        RTI_MQTT_ERROR("failed to set max on conditions sequence")
        goto done;
    }

    self->waitset = DDS_WaitSet_new();
    if (self->waitset == NULL) {
        RTI_MQTT_ERROR("failed to create waitset")
        goto done;
    }

    self->cond_exit = DDS_GuardCondition_new();
    if (self->cond_exit == NULL) {
        RTI_MQTT_ERROR("failed to create exit condition")
        goto done;
    }

    self->cond_timeout = DDS_GuardCondition_new();
    if (self->cond_timeout == NULL) {
        RTI_MQTT_ERROR("failed to create timeout condition")
        goto done;
    }

    {
        struct DDS_TypeAllocationParams_t alloc_params =
                DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

        alloc_params.allocate_memory = RTI_TRUE;
        alloc_params.allocate_pointers = RTI_TRUE;
        alloc_params.allocate_optional_members = RTI_TRUE;

        self->pub_msg =
                RTI_MQTT_MessageTypeSupport_create_data_w_params(&alloc_params);
        if (self->pub_msg == NULL) {
            RTI_MQTT_ERROR("failed to allocate publisher message")
            goto done;
        }
    }

    rc = 0;
done:

    if (DDS_RETCODE_OK != DDS_DomainParticipantFactoryQos_finalize(&dpf_qos)) {
        RTI_MQTT_ERROR("failed to finalize DomainParticipantFactoryQos")
    }

    if (rc != 0) {
        if (active_conditions_initd) {
            MqttAgent_finalize(self);
        }
    }

    return rc;
}

int MqttAgent_publish_message(struct MqttAgent *self)
{
    int rc = 1;
    const char *msg_fmt = MQTT_AGENT_WRITER_MESSAGE_FMT;
    char *msg_buf = NULL;
    DDS_UnsignedLong msg_fmt_len = 0, msg_len = 0;

    self->msg_sent_tot += 1;

    DDS_String_replace(&self->pub_msg->topic, MQTT_AGENT_WRITER_TOPIC);
    if (self->pub_msg->topic == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to set message topic",
                "(%d)",
                self->msg_sent_tot)
        goto done;
    }
    self->pub_msg->info->qos_level = MQTT_AGENT_WRITER_QOS;
    self->pub_msg->info->retained = MQTT_AGENT_WRITER_RETAINED;

    msg_fmt_len = (DDS_UnsignedLong) strlen(msg_fmt) + 10;
    msg_buf = (char *) malloc(sizeof(char) * (msg_fmt_len + 1));
    if (msg_buf == NULL) {
        RTI_MQTT_ERROR_1(
                "failed to allocate message",
                "(%d)",
                self->msg_sent_tot)
        goto done;
    }
    snprintf(
            msg_buf,
            sizeof(char) * (msg_fmt_len + 1),
            msg_fmt,
            self->msg_sent_tot);

    msg_len = (DDS_UnsignedLong) strlen(msg_buf);
    if (!DDS_OctetSeq_ensure_length(
            &self->pub_msg->payload.data,
            msg_len,
            msg_fmt_len)) {
        RTI_MQTT_ERROR_1(
                "failed to set payload length for message",
                "(%d)",
                self->msg_sent_tot)
        goto done;
    }

    memcpy(DDS_OctetSeq_get_contiguous_buffer(&self->pub_msg->payload.data),
           msg_buf,
           sizeof(char) * msg_fmt_len);

    if (DDS_RETCODE_OK
        != RTI_MQTT_MessageDataWriter_write(
                self->writer,
                self->pub_msg,
                &DDS_HANDLE_NIL)) {
        RTI_MQTT_ERROR_1("failed to write message", "(%d)", self->msg_sent_tot)
        goto done;
    }

    rc = 0;
done:
    if (msg_buf == NULL) {
        free(msg_buf);
    }

    return rc;
}

void MqttAgent_print_message(
        struct MqttAgent *self,
        RTI_MQTT_Message *msg,
        DDS_Boolean in)
{
    char *msg_payload = NULL;
    const char *msg_payload_str = NULL;
    DDS_UnsignedLong msg_payload_len = 0, msg_tot = 0;
    const char *prefix = NULL, *action = NULL;

    if (in) {
        prefix = " << ";
        action = "RECEIVED";
        msg_tot = self->msg_recvd_tot;
    } else {
        prefix = " >> ";
        action = "PUBLISHED";
        msg_tot = self->msg_sent_tot;
    }

    printf("%s(%d)\n", action, msg_tot);

    if (msg->info != NULL) {
        printf("%sMSG[%d] info: "
               "topic='%s' qos='%d', ret=%d, dup=%d, id=%d\n",
               prefix,
               msg_tot,
               msg->topic,
               msg->info->qos_level,
               msg->info->retained,
               msg->info->duplicate,
               msg->info->id);
    }
    msg_payload_len = DDS_OctetSeq_get_length(&msg->payload.data);
    if (msg_payload_len == 0) {
        msg_payload_str = "<none>";
    } else {
        msg_payload = (char *) malloc(sizeof(char) * msg_payload_len + 1);
        if (msg_payload == NULL) {
            RTI_MQTT_ERROR_1(
                    "failed to allocate message buffer:",
                    "len=%d",
                    msg_payload_len + 1)
            goto done;
        }
        memcpy(msg_payload,
               DDS_OctetSeq_get_contiguous_buffer(&msg->payload.data),
               sizeof(char) * msg_payload_len);
        msg_payload[msg_payload_len] = '\0';
        msg_payload_str = msg_payload;
    }

    printf("%sMSG[%d] data: %s\n", prefix, msg_tot, msg_payload_str);

done:
    if (msg_payload != NULL) {
        free(msg_payload);
    }
}

int MqttAgent_main(struct MqttAgent *self)
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

    if (DDS_RETCODE_OK
        != RTI_MQTT_Thread_spawn(
                MqttAgent_thread_publisher,
                self,
                &self->thread_pub)) {
        RTI_MQTT_ERROR("failed to spawn publisher thread")
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Thread_spawn(
                MqttAgent_thread_subscriber,
                self,
                &self->thread_sub)) {
        RTI_MQTT_ERROR("failed to spawn subscriber thread")
        goto done;
    }

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