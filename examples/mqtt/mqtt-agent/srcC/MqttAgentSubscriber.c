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

#define RTI_MQTT_LOG_ARGS "MqttAgent::Subscriber"

void *MqttAgent_thread_subscriber(void *arg)
{
    struct MqttAgent *self = (struct MqttAgent *) arg;
    DDS_WaitSet *w = NULL;
    DDS_StatusCondition *s = NULL;
    DDS_Boolean cond_attached_exit = DDS_BOOLEAN_FALSE,
                cond_attached_reader = DDS_BOOLEAN_FALSE,
                cond_exit = DDS_BOOLEAN_FALSE, cond_reader = DDS_BOOLEAN_FALSE;
    struct DDS_ConditionSeq active_conditions = DDS_SEQUENCE_INITIALIZER;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong cond_seq_len = 0, msg_seq_len = 0, i = 0;
    struct RTI_MQTT_KeyedMessageSeq message_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq info_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_Duration_t wait_timeout = DDS_DURATION_INFINITE;

    w = DDS_WaitSet_new();
    if (w == NULL) {
        RTI_MQTT_ERROR("failed to create reader waitset");
        goto done;
    }

    s = DDS_Entity_get_statuscondition(DDS_DataReader_as_entity(
            RTI_MQTT_KeyedMessageDataReader_as_datareader(self->reader)));
    if (s == NULL) {
        RTI_MQTT_ERROR("failed to get reader condition")
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_StatusCondition_set_enabled_statuses(
                s,
                DDS_DATA_AVAILABLE_STATUS)) {
        RTI_MQTT_ERROR("failed to set status condition statuses")
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_WaitSet_attach_condition(
                w,
                DDS_StatusCondition_as_condition(s))) {
        RTI_MQTT_ERROR("failed to attach condition to reader waitset")
        goto done;
    }
    cond_attached_reader = DDS_BOOLEAN_TRUE;

    if (DDS_RETCODE_OK
        != DDS_WaitSet_attach_condition(
                w,
                DDS_GuardCondition_as_condition(self->cond_exit))) {
        RTI_MQTT_ERROR("failed to attach exit condition to reader waitset")
        goto done;
    }
    cond_attached_exit = DDS_BOOLEAN_TRUE;

    while (!cond_exit) {
        cond_reader = DDS_BOOLEAN_FALSE;
        retcode = DDS_WaitSet_wait(w, &active_conditions, &wait_timeout);

        switch (retcode) {
        case DDS_RETCODE_OK:
            break;
        case DDS_RETCODE_TIMEOUT:
            RTI_MQTT_LOG("reader wait timed out")
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
                cond_reader = DDS_BOOLEAN_TRUE;
            }
        }

        if (cond_reader) {
            retcode = RTI_MQTT_KeyedMessageDataReader_take(
                    self->reader,
                    &message_seq,
                    &info_seq,
                    DDS_LENGTH_UNLIMITED,
                    DDS_ANY_SAMPLE_STATE,
                    DDS_ANY_VIEW_STATE,
                    DDS_ANY_INSTANCE_STATE);

            switch (retcode) {
            case DDS_RETCODE_OK:
                msg_seq_len = RTI_MQTT_KeyedMessageSeq_get_length(&message_seq);
                for (i = 0; i < msg_seq_len; i++) {
                    RTI_MQTT_KeyedMessage *msg =
                            RTI_MQTT_KeyedMessageSeq_get_reference(
                                    &message_seq,
                                    i);
                    struct DDS_SampleInfo *info =
                            DDS_SampleInfoSeq_get_reference(&info_seq, i);

                    if (info->valid_data) {
                        self->msg_recvd_tot += 1;
                        MqttAgent_print_message(
                                self,
                                (RTI_MQTT_Message *) msg,
                                DDS_BOOLEAN_TRUE);
                    }
                }
                if (DDS_RETCODE_OK
                    != RTI_MQTT_KeyedMessageDataReader_return_loan(
                            self->reader,
                            &message_seq,
                            &info_seq)) {
                    /* TODO Log error */
                    goto done;
                }
                break;
            case DDS_RETCODE_NO_DATA:
                /* code */
                RTI_MQTT_LOG("no data")
                break;
            default:
                RTI_MQTT_ERROR_1(
                        "failed to perform reader::take():",
                        "%d",
                        retcode)
                goto done;
            }
        }
    }

done:
    if (cond_attached_reader) {
        if (DDS_RETCODE_OK
            != DDS_WaitSet_attach_condition(
                    w,
                    DDS_StatusCondition_as_condition(s))) {
            RTI_MQTT_ERROR("failed to detach condition from reader waitset")
            goto done;
        }
    }
    if (cond_attached_exit) {
        if (DDS_RETCODE_OK
            != DDS_WaitSet_attach_condition(
                    w,
                    DDS_GuardCondition_as_condition(self->cond_exit))) {
            RTI_MQTT_ERROR(
                    "failed to detach exit condition from reader waitset")
            goto done;
        }
    }
    if (w != NULL) {
        DDS_WaitSet_delete(w);
    }
    return NULL;
}
