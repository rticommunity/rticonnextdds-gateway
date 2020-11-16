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

#define RTI_MQTT_LOG_ARGS "MqttAgent::Publisher"

void *MqttAgent_thread_publisher(void *arg)
{
    struct MqttAgent *self = (struct MqttAgent *) arg;
    DDS_WaitSet *w = NULL;
    DDS_StatusCondition *s = NULL;
    DDS_Boolean cond_attached_exit = DDS_BOOLEAN_FALSE,
                cond_attached_writer = DDS_BOOLEAN_FALSE,
                cond_exit = DDS_BOOLEAN_FALSE, cond_writer = DDS_BOOLEAN_FALSE;
    struct DDS_PublicationMatchedStatus status_match =
            DDS_PublicationMatchedStatus_INITIALIZER;
    DDS_StatusMask writer_statuses = DDS_STATUS_MASK_NONE;
    struct DDS_ConditionSeq active_conditions = DDS_SEQUENCE_INITIALIZER;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong cond_seq_len = 0, i = 0;
    struct DDS_Duration_t wait_timeout = DDS_DURATION_ZERO;

    wait_timeout.sec = MQTT_AGENT_WRITER_LOOP_SEC;
    wait_timeout.nanosec = MQTT_AGENT_WRITER_LOOP_NSEC;

    w = DDS_WaitSet_new();
    if (w == NULL) {
        RTI_MQTT_ERROR("failed to create writer waitset");
        goto done;
    }

    s = DDS_Entity_get_statuscondition(DDS_DataWriter_as_entity(
            RTI_MQTT_MessageDataWriter_as_datawriter(self->writer)));
    if (s == NULL) {
        RTI_MQTT_ERROR("failed to get writer condition")
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_StatusCondition_set_enabled_statuses(
                s,
                DDS_PUBLICATION_MATCHED_STATUS)) {
        RTI_MQTT_ERROR("failed to set status condition statuses")
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_WaitSet_attach_condition(
                w,
                DDS_StatusCondition_as_condition(s))) {
        RTI_MQTT_ERROR("failed to attach condition to writer waitset")
        goto done;
    }
    cond_attached_writer = DDS_BOOLEAN_TRUE;

    if (DDS_RETCODE_OK
        != DDS_WaitSet_attach_condition(
                w,
                DDS_GuardCondition_as_condition(self->cond_exit))) {
        RTI_MQTT_ERROR("failed to attach exit condition to writer waitset")
        goto done;
    }
    cond_attached_exit = DDS_BOOLEAN_TRUE;

    while (!cond_exit) {
        cond_writer = DDS_BOOLEAN_FALSE;
        retcode = DDS_WaitSet_wait(w, &active_conditions, &wait_timeout);

        switch (retcode) {
        case DDS_RETCODE_OK:
            break;
        case DDS_RETCODE_TIMEOUT:
            RTI_MQTT_TRACE("writer wait timed out")
            break;
        default:
            RTI_MQTT_ERROR_1("wait error", "%d", retcode)
            break;
        }

        cond_seq_len = DDS_ConditionSeq_get_length(&active_conditions);
        for (i = 0; i < cond_seq_len; i++) {
            DDS_Condition *cond =
                    *DDS_ConditionSeq_get_reference(&active_conditions, i);

            if (cond == DDS_GuardCondition_as_condition(self->cond_exit)) {
                cond_exit = DDS_BOOLEAN_TRUE;
            } else if (cond == DDS_StatusCondition_as_condition(s)) {
                cond_writer = DDS_BOOLEAN_TRUE;
            }
        }

        if (cond_writer) {
            writer_statuses =
                    DDS_Entity_get_status_changes(DDS_DataWriter_as_entity(
                            RTI_MQTT_MessageDataWriter_as_datawriter(
                                    self->writer)));

            if (writer_statuses & DDS_PUBLICATION_MATCHED_STATUS) {
                if (DDS_RETCODE_OK
                    != DDS_DataWriter_get_publication_matched_status(
                            RTI_MQTT_MessageDataWriter_as_datawriter(
                                    self->writer),
                            &status_match)) {
                    RTI_MQTT_ERROR("failed to get publication matched status")
                    goto done;
                }

                RTI_MQTT_LOG_5(
                        "publication MATCHED:",
                        "total=%d, total_change=%d, "
                        "current=%d, current_change=%d, current_peak=%d",
                        status_match.total_count,
                        status_match.total_count_change,
                        status_match.current_count,
                        status_match.current_count_change,
                        status_match.current_count_peak)
            }
        }

        if (status_match.current_count > 0) {
            if (0 != MqttAgent_publish_message(self)) {
                RTI_MQTT_ERROR_1(
                        "failed to publish message",
                        "%d",
                        self->msg_sent_tot)
                goto done;
            }
            MqttAgent_print_message(self, self->pub_msg, DDS_BOOLEAN_FALSE);
        } else {
            RTI_MQTT_LOG("no readers matched yet")
        }
    }

done:
    if (cond_attached_writer) {
        if (DDS_RETCODE_OK
            != DDS_WaitSet_attach_condition(
                    w,
                    DDS_StatusCondition_as_condition(s))) {
            RTI_MQTT_ERROR("failed to detach condition from writer waitset")
            goto done;
        }
    }
    if (cond_attached_exit) {
        if (DDS_RETCODE_OK
            != DDS_WaitSet_attach_condition(
                    w,
                    DDS_GuardCondition_as_condition(self->cond_exit))) {
            RTI_MQTT_ERROR(
                    "failed to detach exit condition from writer waitset")
            goto done;
        }
    }
    if (w != NULL) {
        DDS_WaitSet_delete(w);
    }

    return NULL;
}
