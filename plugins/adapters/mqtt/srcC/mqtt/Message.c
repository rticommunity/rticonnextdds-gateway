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

#include "Message.h"

#include "rtiadapt_mqtt_types_messagePlugin.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::Message"

DDS_ReturnCode_t
        RTI_MQTT_ReceivedMessage_new(struct RTI_MQTT_ReceivedMessage **msg_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_ReceivedMessage *msg = NULL;

    msg = (struct RTI_MQTT_ReceivedMessage *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_MQTT_ReceivedMessage));
    if (msg == NULL) {
        RTI_MQTT_HEAP_ALLOCATE_FAILED(sizeof(struct RTI_MQTT_ReceivedMessage))
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_ReceivedMessage_initialize(msg)) {
        RTI_MQTT_LOG_RECEIVED_MESSAGE_INIT_FAILED(msg)
        goto done;
    }

    *msg_out = msg;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (msg != NULL) {
            RTI_MQTT_Heap_free(msg);
        }
    }
    return retval;
}

void RTI_MQTT_ReceivedMessage_delete(struct RTI_MQTT_ReceivedMessage *self)
{
    if (self == NULL) {
        return;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_ReceivedMessage_finalize(self)) {
        RTI_MQTT_LOG_RECEIVED_MESSAGE_FINALIZE_FAILED(self)
    }

    RTI_MQTT_Heap_free(self);
}

DDS_ReturnCode_t RTI_MQTT_ReceivedMessage_initialize(
        struct RTI_MQTT_ReceivedMessage *self)
{
    self->message = NULL;
    self->read = DDS_BOOLEAN_FALSE;
    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
        RTI_MQTT_ReceivedMessage_finalize(struct RTI_MQTT_ReceivedMessage *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    /* Data must have already been deleted */
    if (self->message != NULL) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

RTIBool RTI_MQTT_ReceivedMessagePtr_initialize_w_params(
        struct RTI_MQTT_ReceivedMessage **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_MQTT_ReceivedMessagePtr_finalize_w_params(
        struct RTI_MQTT_ReceivedMessage **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_MQTT_ReceivedMessagePtr_copy(
        struct RTI_MQTT_ReceivedMessage **dst,
        const struct RTI_MQTT_ReceivedMessage **src)
{
    *dst = (struct RTI_MQTT_ReceivedMessage *) *src;
    return RTI_TRUE;
}

#define T struct RTI_MQTT_ReceivedMessage *
#define TSeq RTI_MQTT_ReceivedMessagePtrSeq
#define T_initialize_w_params RTI_MQTT_ReceivedMessagePtr_initialize_w_params
#define T_finalize_w_params RTI_MQTT_ReceivedMessagePtr_finalize_w_params
#define T_copy RTI_MQTT_ReceivedMessagePtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T

#define RTI_MQTT_MessageReceiveQueue_log_message_state(q_) \
    {                                                      \
        RTI_MQTT_TRACE_1(                                  \
                "queue.msg_status.RECEIVED =",             \
                "%u",                                      \
                (q_)->msg_status->received_count)          \
        RTI_MQTT_TRACE_1(                                  \
                "queue.msg_status.UNREAD =",               \
                "%u",                                      \
                (q_)->msg_status->unread_count)            \
        RTI_MQTT_TRACE_1(                                  \
                "queue.msg_status.READ =",                 \
                "%u",                                      \
                (q_)->msg_status->read_count)              \
        RTI_MQTT_TRACE_1(                                  \
                "queue.msg_status.LOST =",                 \
                "%u",                                      \
                (q_)->msg_status->lost_count)              \
    }

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_new(
        DDS_UnsignedLong size,
        RTI_MQTT_SubscriptionMessageStatus *msg_status,
        struct RTI_MQTT_MessageReceiveQueue **queue_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_MessageReceiveQueue *queue = NULL;

    queue = (struct RTI_MQTT_MessageReceiveQueue *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_MQTT_MessageReceiveQueue));
    if (queue == NULL) {
        RTI_MQTT_HEAP_ALLOCATE_FAILED(
                sizeof(struct RTI_MQTT_MessageReceiveQueue))
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_MessageReceiveQueue_initialize(queue, size, msg_status)) {
        RTI_MQTT_LOG_MSG_RECV_QUEUE_INIT_FAILED(queue, size, msg_status)
        goto done;
    }

    *queue_out = queue;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (queue != NULL) {
            RTI_MQTT_Heap_free(queue);
        }
    }
    return retval;
}

void RTI_MQTT_MessageReceiveQueue_delete(
        struct RTI_MQTT_MessageReceiveQueue *self)
{
    if (self == NULL) {
        return;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_MessageReceiveQueue_finalize(self)) {
        RTI_MQTT_LOG_MSG_RECV_QUEUE_FINALIZE_FAILED(self)
    }

    RTI_MQTT_Heap_free(self);
}


DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_initialize_circular(
        struct RTI_MQTT_MessageReceiveQueue *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i = 0, initd_msgs = 0;
    DDS_Boolean read_buffer_initd = DDS_BOOLEAN_FALSE;

    if (self->capacity == 0) {
        RTI_MQTT_INTERNAL_ERROR(
                "RTI_MQTT_MessageReceiveQueue_initialize_circular can only be "
                "called with self->capacity > 0")
        goto done;
    }

    if (!RTI_MQTT_DDS_DynamicDataPtrSeq_initialize(&self->read_buffer)) {
        /* TODO Log error */
        goto done;
    }
    read_buffer_initd = DDS_BOOLEAN_TRUE;

    if (!RTI_MQTT_DDS_DynamicDataPtrSeq_set_maximum(
                &self->read_buffer,
                self->capacity)) {
        /* TODO Log error */
        goto done;
    }

    self->read_buffer_loaned = DDS_BOOLEAN_FALSE;

    if (!RTI_MQTT_ReceivedMessagePtrSeq_set_maximum(
                &self->queue,
                self->capacity)) {
        RTI_MQTT_LOG_SET_SEQUENCE_MAX_FAILED(&self->queue, self->capacity)
        goto done;
    }
    if (!RTI_MQTT_ReceivedMessagePtrSeq_set_length(
                &self->queue,
                self->capacity)) {
        RTI_MQTT_LOG_SET_SEQUENCE_LENGTH_FAILED(&self->queue, self->capacity)
        goto done;
    }

    for (i = 0; i < self->capacity; i++) {
        struct RTI_MQTT_ReceivedMessage *msg = NULL, **msg_ref = NULL;

        msg_ref = RTI_MQTT_ReceivedMessagePtrSeq_get_reference(&self->queue, i);

        if (DDS_RETCODE_OK != RTI_MQTT_ReceivedMessage_new(&msg)) {
            RTI_MQTT_LOG_RECEIVED_MESSAGE_CREATE_FAILED()
            goto done;
        }
        *msg_ref = msg;

        initd_msgs += 1;
    }


    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        for (i = 0; i < initd_msgs; i++) {
            struct RTI_MQTT_ReceivedMessage *msg = NULL;

            msg = *RTI_MQTT_ReceivedMessagePtrSeq_get_reference(
                    &self->queue,
                    i);
            if (msg->message != NULL) {
                DDS_DynamicDataTypeSupport_delete_data(
                        self->dyn_data,
                        msg->message);
            }
            RTI_MQTT_ReceivedMessage_delete(msg);
        }
        if (read_buffer_initd) {
            if (!RTI_MQTT_DDS_DynamicDataPtrSeq_finalize(&self->read_buffer)) {
                /* TODO Log error */
            }
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_initialize_unbounded(
        struct RTI_MQTT_MessageReceiveQueue *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i = 0, initd_msgs = 0;

    if (self->capacity > 0) {
        RTI_MQTT_INTERNAL_ERROR(
                "RTI_MQTT_MessageReceiveQueue_initialize_circular can only be "
                "called with self->capacity == 0")
        goto done;
    }

    if (!RTI_MQTT_DDS_DynamicDataPtrSeq_initialize(&self->read_buffer)) {
        /* TODO Log error */
        goto done;
    }

    self->read_buffer_loaned = DDS_BOOLEAN_FALSE;

    retval = DDS_RETCODE_OK;
done:

    return retval;
}


DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_initialize(
        struct RTI_MQTT_MessageReceiveQueue *self,
        DDS_UnsignedLong size,
        RTI_MQTT_SubscriptionMessageStatus *msg_status)
{
    DDS_Boolean queue_initd = DDS_BOOLEAN_FALSE, lock_initd = DDS_BOOLEAN_FALSE;
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i = 0, initd_msgs = 0;

    self->capacity = size;
    self->size = 0;
    self->head = 0;
    self->next = 0;
    self->next_unread = 0;
    self->listener_data_avail = NULL;
    self->listener_data_avail_arg = NULL;
    self->dyn_data = NULL;
    self->msg_status = msg_status;

    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_initialize(&self->lock)) {
        /* TODO Log error */
        goto done;
    }
    lock_initd = DDS_BOOLEAN_TRUE;

    if (!RTI_MQTT_ReceivedMessagePtrSeq_initialize(&self->queue)) {
        RTI_MQTT_LOG_INITIALIZE_SEQUENCE_FAILED(&self->queue)
        goto done;
    }
    queue_initd = DDS_BOOLEAN_TRUE;

    self->dyn_data = DDS_DynamicDataTypeSupport_new(
            RTI_MQTT_Message_get_typecode(),
            &DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
    if (self->dyn_data == NULL) {
        /* TODO Log error */
        goto done;
    }

    if (self->capacity > 0) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_MessageReceiveQueue_initialize_circular(self)) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_INIT_CIRCULAR_FAILED(self)
            goto done;
        }
    } else {
        if (DDS_RETCODE_OK
            != RTI_MQTT_MessageReceiveQueue_initialize_unbounded(self)) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_INIT_UNBOUNDED_FAILED(self)
            goto done;
        }
    }


    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        if (lock_initd) {
            if (DDS_RETCODE_OK != RTI_MQTT_Mutex_finalize(&self->lock)) {
                /* TODO Log error */
            }
            if (queue_initd) {
                if (!RTI_MQTT_ReceivedMessagePtrSeq_finalize(&self->queue)) {
                    RTI_MQTT_LOG_FINALIZE_SEQUENCE_FAILED(&self->queue)
                }
                if (self->dyn_data != NULL) {
                    DDS_DynamicDataTypeSupport_delete(self->dyn_data);
                    self->dyn_data = NULL;
                }
            }
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_finalize(
        struct RTI_MQTT_MessageReceiveQueue *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0, i = 0;

    seq_len = RTI_MQTT_ReceivedMessagePtrSeq_get_length(&self->queue);
    for (i = 0; i < seq_len; i++) {
        struct RTI_MQTT_ReceivedMessage *msg =
                *RTI_MQTT_ReceivedMessagePtrSeq_get_reference(&self->queue, i);
        if (msg->message != NULL) {
            DDS_DynamicDataTypeSupport_delete_data(
                    self->dyn_data,
                    msg->message);
            msg->message = NULL;
        }
        RTI_MQTT_ReceivedMessage_delete(msg);
    }

    if (!RTI_MQTT_ReceivedMessagePtrSeq_finalize(&self->queue)) {
        RTI_MQTT_LOG_FINALIZE_SEQUENCE_FAILED(&self->queue)
    }

    if (!RTI_MQTT_DDS_DynamicDataPtrSeq_finalize(&self->read_buffer)) {
        RTI_MQTT_LOG_FINALIZE_SEQUENCE_FAILED(&self->read_buffer)
    }

    if (self->dyn_data != NULL) {
        DDS_DynamicDataTypeSupport_delete(self->dyn_data);
        self->dyn_data = NULL;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_finalize(&self->lock)) {
        /* TODO Log error */
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

static DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_receive_circular(
        struct RTI_MQTT_MessageReceiveQueue *self,
        DDS_DynamicData *msg,
        DDS_DynamicData **dropped_out,
        DDS_Boolean *lost_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean lost = DDS_BOOLEAN_FALSE;
    DDS_DynamicData *dropped = NULL;
    struct RTI_MQTT_ReceivedMessage *msg_new = NULL;
    DDS_UnsignedLong seq_len = 0, i = 0;

    msg_new = *RTI_MQTT_ReceivedMessagePtrSeq_get_reference(
            &self->queue,
            self->next);


    if (msg_new->message != NULL) {
        dropped = msg_new->message;
        lost = dropped != NULL;
    }
    msg_new->message = msg;

    if (self->size < self->capacity) {
        self->size += 1;
    } else {
        self->head = (self->head += 1) % self->capacity;
    }

    self->next = (self->next + 1) % self->capacity;

    if (lost) {
        self->next_unread = (self->next_unread + 1) % self->capacity;
    }

    if (self->listener_data_avail != NULL) {
        self->listener_data_avail(self, self->listener_data_avail_arg);
    }

    if (lost_out != NULL) {
        *lost_out = lost;
    }
    if (dropped_out != NULL) {
        *dropped_out = dropped;
    }

    retval = DDS_RETCODE_OK;

done:
    return retval;
}

static DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_receive_unbounded(
        struct RTI_MQTT_MessageReceiveQueue *self,
        DDS_DynamicData *msg,
        DDS_DynamicData **dropped_out,
        DDS_Boolean *lost_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean lost = DDS_BOOLEAN_FALSE, seq_len_set = DDS_BOOLEAN_FALSE;
    DDS_DynamicData *dropped = NULL;
    struct RTI_MQTT_ReceivedMessage *msg_new = NULL, **msg_ref = NULL;
    DDS_UnsignedLong i = 0, seq_len = 0;

    seq_len = RTI_MQTT_ReceivedMessagePtrSeq_get_length(&self->queue);

    if (!RTI_MQTT_ReceivedMessagePtrSeq_ensure_length(
                &self->queue,
                seq_len + 1,
                seq_len + 1)) {
        RTI_MQTT_LOG_SET_SEQUENCE_ENSURE_LENGTH_FAILED(
                &self->queue,
                seq_len + 1,
                seq_len + 1)
        goto done;
    }
    seq_len_set = DDS_BOOLEAN_TRUE;

    msg_ref =
            RTI_MQTT_ReceivedMessagePtrSeq_get_reference(&self->queue, seq_len);

    if (DDS_RETCODE_OK != RTI_MQTT_ReceivedMessage_new(msg_ref)) {
        RTI_MQTT_LOG_RECEIVED_MESSAGE_CREATE_FAILED()
        goto done;
    }

    (*msg_ref)->message = msg;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (msg_ref != NULL) {
            if (*msg_ref != NULL) {
                if ((*msg_ref)->message != NULL) {
                    DDS_DynamicDataTypeSupport_delete_data(
                            self->dyn_data,
                            (*msg_ref)->message);
                    (*msg_ref)->message = NULL;
                }

                RTI_MQTT_ReceivedMessage_delete(*msg_ref);
            }
        }
        if (seq_len_set) {
            seq_len = RTI_MQTT_ReceivedMessagePtrSeq_get_length(&self->queue);
            if (!RTI_MQTT_ReceivedMessagePtrSeq_set_length(
                        &self->queue,
                        seq_len - 1)) {
                RTI_MQTT_LOG_SET_SEQUENCE_LENGTH_FAILED(
                        &self->queue,
                        seq_len - 1)
            }
        }
    }


    return retval;
}

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_receive(
        struct RTI_MQTT_MessageReceiveQueue *self,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        const char *topic,
        RTI_MQTT_MessageInfo *msg_info,
        DDS_DynamicData **dropped_out,
        DDS_Boolean *lost_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean lost = DDS_BOOLEAN_FALSE, locked = DDS_BOOLEAN_FALSE;
    DDS_DynamicData *msg = NULL;
    RTI_MQTT_Message msg_static;

    if (dropped_out != NULL) {
        *dropped_out = NULL;
    }
    if (lost_out != NULL) {
        *lost_out = DDS_BOOLEAN_FALSE;
    }

    msg = DDS_DynamicDataTypeSupport_create_data(self->dyn_data);
    if (msg == NULL) {
        /* TODO Log error */
        goto done;
    }
    msg_static.topic = (char *) topic;

    if (!DDS_OctetSeq_initialize(&msg_static.payload.data)) {
        /* TODO Log error */
        goto done;
    }

    if (!DDS_OctetSeq_loan_contiguous(
                &msg_static.payload.data,
                (char *) buffer,
                buffer_len,
                buffer_len)) {
        /* TODO Log error */
        goto done;
    }
    msg_static.info = msg_info;

    if (DDS_RETCODE_OK != RTI_MQTT_Message_to_dynamic_data(&msg_static, msg)) {
        /* TODO Log error */
        goto done;
    }

    RTI_MQTT_Mutex_assert_w_state(&self->lock, &locked);

    if (self->capacity > 0) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_MessageReceiveQueue_receive_circular(
                    self,
                    msg,
                    dropped_out,
                    &lost)) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_RECEIVE_CIRCULAR_FAILED(self)
            goto done;
        }
    } else {
        if (DDS_RETCODE_OK
            != RTI_MQTT_MessageReceiveQueue_receive_unbounded(
                    self,
                    msg,
                    dropped_out,
                    &lost)) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_RECEIVE_UNBOUNDED_FAILED(self)
            goto done;
        }
    }

    /* Update message state */
    self->msg_status->received_count += 1;
    self->msg_status->unread_count += 1;
    if (lost) {
        self->msg_status->lost_count += 1;
        self->msg_status->unread_count -= 1;
    }

    if (lost_out != NULL) {
        *lost_out = lost;
    }

    retval = DDS_RETCODE_OK;
done:

    if (locked) {
        RTI_MQTT_MessageReceiveQueue_log_message_state(self);
        RTI_MQTT_Mutex_release_w_state(&self->lock, &locked);
    }

    return retval;
}


static DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_read_circular(
        struct RTI_MQTT_MessageReceiveQueue *self,
        DDS_UnsignedLong max_messages,
        struct DDS_DynamicDataSeq *messages)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i = 0, idx = 0, tot_messages = 0, messages_max = 0,
                     loaned_max = 0;
    DDS_Boolean loan = DDS_BOOLEAN_FALSE;
    struct RTI_MQTT_ReceivedMessage *rcvd_msg = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_MessageReceiveQueue_read_circular)

    messages_max = DDS_DynamicDataSeq_get_maximum(messages);

    if (messages_max > 0) {
        /* We only support passing an empty sequence with
         * max_messages == UNLIMITED */
        if (max_messages == RTI_MQTT_SUBSCRIPTION_READ_LENGTH_UNLIMITED) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_UNEXPECTED_MESSAGE_BUFFER(self)
            goto done;
        }

        if (messages_max < max_messages) {
            /* TODO Log error */
            goto done;
        }

        if (!DDS_DynamicDataSeq_set_length(messages, max_messages)) {
            /* TODO Log error */
            goto done;
        }
    } else {
        /* When passing an empty sequence, on max_messages == UNLIMITED
         * is supported */
        if (max_messages != RTI_MQTT_SUBSCRIPTION_READ_LENGTH_UNLIMITED) {
            /* TODO Log error */
            goto done;
        }

        if (self->read_buffer_loaned) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_ALREADY_IN_PROGRESS(self)
            goto done;
        }
        loan = DDS_BOOLEAN_TRUE;
        max_messages =
                RTI_MQTT_DDS_DynamicDataPtrSeq_get_maximum(&self->read_buffer);
        if (!RTI_MQTT_DDS_DynamicDataPtrSeq_set_length(
                    &self->read_buffer,
                    max_messages)) {
            /* TODO Log error */
            goto done;
        }
        if (!DDS_DynamicDataSeq_set_length(messages, 0)) {
            /* TODO Log error */
            goto done;
        }
    }

    for (i = 0, idx = self->next_unread; i < max_messages && idx != self->next;
         i++, idx = (self->next_unread + i) % self->capacity) {
        struct RTI_MQTT_ReceivedMessage **rcvd_msg_ref =
                RTI_MQTT_ReceivedMessagePtrSeq_get_reference(&self->queue, idx);

        rcvd_msg = *rcvd_msg_ref;

        if (loan) {
            *RTI_MQTT_DDS_DynamicDataPtrSeq_get_reference(
                    &self->read_buffer,
                    i) = rcvd_msg->message;
            rcvd_msg->message = NULL;
        } else {
            if (!DDS_DynamicDataTypeSupport_copy_data(
                        self->dyn_data,
                        DDS_DynamicDataSeq_get_reference(messages, i),
                        rcvd_msg->message)) {
                /* TODO Log error */
                goto done;
            }
            if (!DDS_DynamicDataTypeSupport_delete_data(
                        self->dyn_data,
                        rcvd_msg->message)) {
                /* TODO Log error */
                goto done;
            }
            rcvd_msg->message = NULL;
        }

        tot_messages += 1;
    }

    if (loan) {
        if (tot_messages > 0) {
            if (!DDS_DynamicDataSeq_loan_discontiguous(
                        messages,
                        RTI_MQTT_DDS_DynamicDataPtrSeq_get_contiguous_buffer(
                                &self->read_buffer),
                        tot_messages,
                        tot_messages)) {
                /* TODO Log error */
                goto done;
            }

            self->read_buffer_loaned = DDS_BOOLEAN_TRUE;
        }
    } else {
        if (!DDS_DynamicDataSeq_set_length(messages, tot_messages)) {
            /* TODO Log error */
            goto done;
        }
    }

    self->next_unread = idx;
    self->size -= tot_messages;
    self->head = (self->head + tot_messages) % self->capacity;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (self->read_buffer_loaned) {
            self->msg_status->lost_count += tot_messages;
            if (DDS_RETCODE_OK
                != RTI_MQTT_MessageReceiveQueue_return_loan(self, messages)) {
                /* TODO Log error */
            }
        } else if (loan) {
            for (i = 0; i < tot_messages; i++) {
                DDS_DynamicData **msg_ref =
                        RTI_MQTT_DDS_DynamicDataPtrSeq_get_reference(
                                &self->read_buffer,
                                i);
                if (*msg_ref != NULL) {
                    if (!DDS_DynamicDataTypeSupport_delete_data(
                                self->dyn_data,
                                *msg_ref)) {
                        /* TODO Log error */
                    }
                    *msg_ref = NULL;
                }
            }
        } else if (rcvd_msg != NULL && rcvd_msg->message != NULL) {
            if (!DDS_DynamicDataTypeSupport_delete_data(
                        self->dyn_data,
                        rcvd_msg->message)) {
                /* TODO Log error */
                goto done;
            }
            rcvd_msg->message = NULL;
        }
    }

    return retval;
}


static DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_read_unbounded(
        struct RTI_MQTT_MessageReceiveQueue *self,
        DDS_UnsignedLong max_messages,
        struct DDS_DynamicDataSeq *messages)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0, i = 0, idx = 0, messages_max = 0,
                     tot_messages = 0;
    DDS_Boolean loan = DDS_BOOLEAN_FALSE;
    struct RTI_MQTT_ReceivedMessage *rcvd_msg = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_MessageReceiveQueue_read_unbounded)

    seq_len = RTI_MQTT_ReceivedMessagePtrSeq_get_length(&self->queue);
    messages_max = DDS_DynamicDataSeq_get_maximum(messages);

    if (messages_max > 0) {
        /* We only support passing an empty sequence with
         * max_messages == UNLIMITED */
        if (max_messages == RTI_MQTT_SUBSCRIPTION_READ_LENGTH_UNLIMITED) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_UNEXPECTED_MESSAGE_BUFFER(self)
            goto done;
        }

        /* Check that the user didn't pass a preallocated sequence but didn't
         * specify the correct length in max_messages */
        if (messages_max < max_messages) {
            /* TODO Log error */
            goto done;
        }

        if (max_messages > seq_len) {
            max_messages = seq_len;
        }

        if (!DDS_DynamicDataSeq_set_length(messages, max_messages)) {
            /* TODO Log error */
            goto done;
        }
    } else {
        /* When passing an empty sequence, on max_messages == UNLIMITED
         * is supported */
        if (max_messages != RTI_MQTT_SUBSCRIPTION_READ_LENGTH_UNLIMITED) {
            /* TODO Log error */
            goto done;
        }

        /* If the user already called read, and we loaned them a buffer
         * they should have returned the loan before re-reading */
        if (self->read_buffer_loaned) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_ALREADY_IN_PROGRESS(self)
            goto done;
        }

        if (!RTI_MQTT_DDS_DynamicDataPtrSeq_ensure_length(
                    &self->read_buffer,
                    seq_len,
                    seq_len)) {
            /* TODO Log error */
            goto done;
        }
        if (!RTI_MQTT_DDS_DynamicDataPtrSeq_set_length(
                    &self->read_buffer,
                    seq_len)) {
            /* TODO Log error */
            goto done;
        }
        if (!DDS_DynamicDataSeq_set_length(messages, 0)) {
            /* TODO Log error */
            goto done;
        }
        max_messages = seq_len;
        loan = DDS_BOOLEAN_TRUE;
    }

    for (i = 0; i < max_messages; i++) {
        struct RTI_MQTT_ReceivedMessage **rcvd_msg_ref =
                RTI_MQTT_ReceivedMessagePtrSeq_get_reference(&self->queue, i);

        rcvd_msg = *rcvd_msg_ref;

        if (loan) {
            *RTI_MQTT_DDS_DynamicDataPtrSeq_get_reference(
                    &self->read_buffer,
                    i) = rcvd_msg->message;
            rcvd_msg->message = NULL;
        } else {
            if (!DDS_DynamicDataTypeSupport_copy_data(
                        self->dyn_data,
                        DDS_DynamicDataSeq_get_reference(messages, i),
                        rcvd_msg->message)) {
                /* TODO Log error */
                goto done;
            }
            if (!DDS_DynamicDataTypeSupport_delete_data(
                        self->dyn_data,
                        rcvd_msg->message)) {
                /* TODO Log error */
                goto done;
            }
            rcvd_msg->message = NULL;
        }

        RTI_MQTT_ReceivedMessage_delete(rcvd_msg);
        rcvd_msg = NULL;
        *rcvd_msg_ref = NULL;
    }
    tot_messages = i;

    if (loan) {
        if (i > 0) {
            if (!DDS_DynamicDataSeq_loan_discontiguous(
                        messages,
                        RTI_MQTT_DDS_DynamicDataPtrSeq_get_contiguous_buffer(
                                &self->read_buffer),
                        tot_messages,
                        tot_messages)) {
                /* TODO Log error */
                goto done;
            }

            self->read_buffer_loaned = DDS_BOOLEAN_TRUE;
        }
    } else {
        if (!DDS_DynamicDataSeq_set_length(messages, tot_messages)) {
            /* TODO Log error */
            goto done;
        }
    }

    if (seq_len < tot_messages) {
        RTI_MQTT_Memory_move(
                RTI_MQTT_ReceivedMessagePtrSeq_get_contiguous_buffer(
                        &self->queue),
                RTI_MQTT_ReceivedMessagePtrSeq_get_reference(
                        &self->queue,
                        tot_messages),
                sizeof(struct RTI_MQTT_ReceivedMessage *)
                        * (seq_len - tot_messages));
    }

    if (!RTI_MQTT_ReceivedMessagePtrSeq_set_length(
                &self->queue,
                seq_len - tot_messages)) {
        RTI_MQTT_LOG_SET_SEQUENCE_LENGTH_FAILED(
                &self->queue,
                seq_len - tot_messages)
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        self->msg_status->lost_count += tot_messages;

        if (self->read_buffer_loaned) {
            if (DDS_RETCODE_OK
                != RTI_MQTT_MessageReceiveQueue_return_loan(self, messages)) {
                /* TODO Log error */
            }
        } else if (loan) {
            for (i = 0; i < tot_messages; i++) {
                DDS_DynamicData **msg_ref =
                        RTI_MQTT_DDS_DynamicDataPtrSeq_get_reference(
                                &self->read_buffer,
                                i);
                if (*msg_ref != NULL) {
                    if (!DDS_DynamicDataTypeSupport_delete_data(
                                self->dyn_data,
                                *msg_ref)) {
                        /* TODO Log error */
                    }
                    *msg_ref = NULL;
                }
            }
        } else if (rcvd_msg != NULL) {
            RTI_MQTT_ReceivedMessage_delete(rcvd_msg);
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_read(
        struct RTI_MQTT_MessageReceiveQueue *self,
        DDS_UnsignedLong max_messages,
        struct DDS_DynamicDataSeq *messages)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong messages_len = 0;

    RTI_MQTT_LOG_FN(RTI_MQTT_MessageReceiveQueue_read)

    RTI_MQTT_Mutex_assert(&self->lock);

    if (self->capacity > 0) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_MessageReceiveQueue_read_circular(
                    self,
                    max_messages,
                    messages)) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_CIRCULAR_FAILED(self)
            goto done;
        }
    } else {
        if (DDS_RETCODE_OK
            != RTI_MQTT_MessageReceiveQueue_read_unbounded(
                    self,
                    max_messages,
                    messages)) {
            RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_UNBOUNDED_FAILED(self)
            goto done;
        }
    }

    messages_len = DDS_DynamicDataSeq_get_length(messages);

    /* Update message state */
    self->msg_status->unread_count -= messages_len;
    self->msg_status->read_count += messages_len;

    retval = DDS_RETCODE_OK;
done:
    RTI_MQTT_MessageReceiveQueue_log_message_state(self);
    RTI_MQTT_Mutex_release(&self->lock);

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_return_loan(
        struct RTI_MQTT_MessageReceiveQueue *self,
        struct DDS_DynamicDataSeq *messages)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i = 0, messages_len = 0;

    RTI_MQTT_LOG_FN(RTI_MQTT_MessageReceiveQueue_return_loan)

    RTI_MQTT_Mutex_assert(&self->lock);

    if (!self->read_buffer_loaned) {
        RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_NOT_IN_PROGRESS(self)
        retval = DDS_RETCODE_OK;
        goto done;
    }

    /* TODO precondition check:
        is messages.buffer == self->read_buffer.buffer ? */

    messages_len = DDS_DynamicDataSeq_get_length(messages);
    if (!DDS_DynamicDataSeq_unloan(messages)) {
        /* TODO Log error */
        goto done;
    }
    if (!RTI_MQTT_DDS_DynamicDataPtrSeq_set_length(
                &self->read_buffer,
                messages_len)) {
        /* TODO Log error */
        goto done;
    }

    for (i = 0; i < messages_len; i++) {
        DDS_DynamicData **msg = RTI_MQTT_DDS_DynamicDataPtrSeq_get_reference(
                &self->read_buffer,
                i);

        if (*msg == NULL) {
            /* TODO Log error */
            continue;
        }

        DDS_DynamicDataTypeSupport_delete_data(self->dyn_data, *msg);
        *msg = NULL;
    }

    if (!RTI_MQTT_DDS_DynamicDataPtrSeq_set_length(&self->read_buffer, 0)) {
        /* TODO Log error */
        goto done;
    }

    self->read_buffer_loaned = DDS_BOOLEAN_FALSE;

    retval = DDS_RETCODE_OK;
done:
    RTI_MQTT_MessageReceiveQueue_log_message_state(self);
    RTI_MQTT_Mutex_release(&self->lock);

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_MessageReceiveQueue_set_listener(
        struct RTI_MQTT_MessageReceiveQueue *self,
        RTI_MQTT_MessageReceiveQueue_OnDataAvailableCallback listener,
        void *arg)
{
    self->listener_data_avail = listener;
    self->listener_data_avail_arg = arg;
    return DDS_RETCODE_OK;
}


DDS_ReturnCode_t RTI_MQTT_Message_to_dynamic_data(
        RTI_MQTT_Message *self,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Message_to_dynamic_data)

    if (self->topic != NULL) {
        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_string(
                    sample,
                    "topic",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    self->topic)) {
            /* TODO Log error */
            goto done;
        }
    }

    if (self->info != NULL) {
        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_long(
                    sample,
                    "info.id",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    self->info->id)) {
            /* TODO Log error */
            goto done;
        }

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_long(
                    sample,
                    "info.qos_level",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    self->info->qos_level)) {
            /* TODO Log error */
            goto done;
        }

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_boolean(
                    sample,
                    "info.retained",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    self->info->retained)) {
            /* TODO Log error */
            goto done;
        }

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_boolean(
                    sample,
                    "info.duplicate",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    self->info->duplicate)) {
            /* TODO Log error */
            goto done;
        }
    }

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_octet_seq(
                sample,
                "payload.data",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                &self->payload.data)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Message_from_dynamic_data(
        RTI_MQTT_Message *self,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean allocd_info = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong str_len = 0;
    char *str_val = NULL;

    if (self->info == NULL) {
        self->info = RTI_MQTT_MessageInfoPluginSupport_create_data();
        allocd_info = DDS_BOOLEAN_TRUE;
    }

    if (DDS_RETCODE_OK
        != DDS_DynamicData_get_long(
                sample,
                &self->info->id,
                "info.id",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        /* TODO Log error */
        goto done;
    }

    str_len = 0;
    if (DDS_RETCODE_OK
        != DDS_DynamicData_get_string(
                sample,
                &str_val,
                &str_len,
                "topic",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        /* TODO Log error */
        goto done;
    }
    if (NULL == DDS_String_replace(&self->topic, str_val)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DynamicData_get_long(
                sample,
                (DDS_Long *) &self->info->qos_level,
                "info.qos_level",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DynamicData_get_boolean(
                sample,
                &self->info->retained,
                "info.retained",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DynamicData_get_boolean(
                sample,
                &self->info->duplicate,
                "info.duplicate",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DynamicData_get_octet_seq(
                sample,
                &self->payload.data,
                "payload.data",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:

    if (str_val != NULL) {
        DDS_String_free(str_val);
    }

    if (retval != DDS_RETCODE_OK) {
        if (allocd_info) {
            RTI_MQTT_MessageInfoPluginSupport_destroy_data(self->info);
            self->info = NULL;
        }
    }

    return retval;
}
