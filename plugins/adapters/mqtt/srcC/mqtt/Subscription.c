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

#include "Subscription.h"
#include "Client.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::Subscription"

static DDS_ReturnCode_t RTI_MQTT_Subscription_initialize(
        struct RTI_MQTT_Subscription *self,
        struct RTI_MQTT_Client *client,
        RTI_MQTT_SubscriptionConfig *config);

static DDS_ReturnCode_t
        RTI_MQTT_Subscription_finalize(struct RTI_MQTT_Subscription *self);


DDS_ReturnCode_t RTI_MQTT_Subscription_new(
        struct RTI_MQTT_Client *client,
        RTI_MQTT_SubscriptionConfig *config,
        struct RTI_MQTT_Subscription **sub_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Subscription *sub = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_Subscription_new)

    *sub_out = NULL;

    sub = (struct RTI_MQTT_Subscription *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_MQTT_Subscription));
    if (sub == NULL) {
        RTI_MQTT_HEAP_ALLOCATE_FAILED(sizeof(struct RTI_MQTT_Subscription))
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Subscription_initialize(sub, client, config)) {
        RTI_MQTT_LOG_SUBSCRIPTION_INIT_FAILED(sub, client, config)
        goto done;
    }

    *sub_out = sub;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (sub != NULL) {
            RTI_MQTT_Heap_free(sub);
        }
    }

    return retval;
}

void RTI_MQTT_Subscription_delete(struct RTI_MQTT_Subscription *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_Subscription_delete)

    if (self == NULL) {
        return;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Subscription_finalize(self)) {
        RTI_MQTT_LOG_SUBSCRIPTION_FINALIZE_FAILED(self)
    }

    RTI_MQTT_Heap_free(self);
}

DDS_ReturnCode_t RTI_MQTT_Subscription_match(
        struct RTI_MQTT_Subscription *self,
        const char *topic_name,
        DDS_Boolean *match_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0, i = 0;
    DDS_Boolean match = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_Subscription_match)

    /* TODO RM Mutex: only used to protect self->data->config */
    RTI_MQTT_Mutex_assert(&self->client->sub_lock);

    seq_len = DDS_StringSeq_get_length(&self->data->config->topic_filters);
    for (i = 0; i < seq_len && !match; i++) {
        const char *topic_filter = *DDS_StringSeq_get_reference(
                &self->data->config->topic_filters,
                i);

        if (DDS_RETCODE_OK
            != RTI_MQTT_TopicFilter_match(topic_filter, topic_name, &match)) {
            RTI_MQTT_LOG_SUBSCRIPTION_TOPIC_MATCH_FAILED(
                    self,
                    topic_filter,
                    topic_name)
            goto done;
        }
    }

    *match_out = match;

    retval = DDS_RETCODE_OK;
done:
    /* TODO RM Mutex: only used to protect self->data->config */
    RTI_MQTT_Mutex_release(&self->client->sub_lock);
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Subscription_receive(
        struct RTI_MQTT_Subscription *self,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        const char *topic,
        RTI_MQTT_MessageInfo *msg_info,
        DDS_DynamicData **dropped_out,
        DDS_Boolean *lost_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Subscription_receive)

    if (DDS_RETCODE_OK
        != RTI_MQTT_MessageReceiveQueue_receive(
                self->queue,
                buffer,
                buffer_len,
                topic,
                msg_info,
                dropped_out,
                lost_out)) {
        RTI_MQTT_LOG_SUBSCRIPTION_ADD_TO_QUEUE_FAILED(self, buffer)
        goto done;
    }

    if (self->data_avail_listener != NULL) {
        self->data_avail_listener(self->data_avail_listener_data, self);
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Subscription_read(
        struct RTI_MQTT_Subscription *self,
        DDS_UnsignedLong max_messages,
        struct DDS_DynamicDataSeq *messages)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Subscription_read)

    if (DDS_RETCODE_OK
        != RTI_MQTT_MessageReceiveQueue_read(
                self->queue,
                max_messages,
                messages)) {
        RTI_MQTT_LOG_SUBSCRIPTION_READ_FROM_QUEUE_FAILED(self, max_messages)
        goto done;
    }
    RTI_MQTT_TRACE_2(
            "READ result:",
            "%d messages [%p]",
            DDS_DynamicDataSeq_get_length(messages),
            self)

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Subscription_return_loan(
        struct RTI_MQTT_Subscription *self,
        struct DDS_DynamicDataSeq *messages)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Subscription_return_loan)

    if (DDS_RETCODE_OK
        != RTI_MQTT_MessageReceiveQueue_return_loan(self->queue, messages)) {
        RTI_MQTT_LOG_SUBSCRIPTION_RETURN_QUEUE_LOAN_FAILED(self)
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Subscription_set_data_available_listener(
        struct RTI_MQTT_Subscription *self,
        RTI_MQTT_Subscription_DataAvailableCallback listener,
        void *listener_data)
{
    self->data_avail_listener = listener;
    self->data_avail_listener_data = listener_data;
    return DDS_RETCODE_OK;
}


static DDS_ReturnCode_t RTI_MQTT_Subscription_initialize(
        struct RTI_MQTT_Subscription *self,
        struct RTI_MQTT_Client *client,
        RTI_MQTT_SubscriptionConfig *config)
{
    DDS_UnsignedLong tot_conditions = 0;
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Subscription def_self = RTI_MQTT_Subscription_INITIALIZER;

    RTI_MQTT_LOG_FN(RTI_MQTT_Subscription_initialize)

    *self = def_self;

    self->client = client;

    if (DDS_RETCODE_OK
        != RTI_MQTT_SubscriptionStatus_new(DDS_BOOLEAN_TRUE, &self->data)) {
        RTI_MQTT_LOG_CREATE_DATA_FAILED("RTI_MQTT_SubscriptionStatus")
        goto done;
    }

    if (!RTI_MQTT_SubscriptionConfig_copy(self->data->config, config)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_MQTT_SubscriptionConfig",
                config,
                self->data->config)
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_MessageReceiveQueue_new(
                self->data->config->message_queue_size,
                self->data->message_status,
                &self->queue)) {
        RTI_MQTT_LOG_SUBSCRIPTION_CREATE_MSG_QUEUE_FAILED(
                self,
                self->data->config->message_queue_size,
                self->data->message_status)
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval && self != NULL) {
        if (DDS_RETCODE_OK != RTI_MQTT_Subscription_finalize(self)) {
            /* TODO Log error */
        }
    }
    return retval;
}

static DDS_ReturnCode_t
        RTI_MQTT_Subscription_finalize(struct RTI_MQTT_Subscription *self)
{
    struct RTI_MQTT_Subscription def_self = RTI_MQTT_Subscription_INITIALIZER;
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Subscription_finalize)

    if (self->queue != NULL) {
        RTI_MQTT_MessageReceiveQueue_delete(self->queue);
        self->queue = NULL;
    }
    if (self->data != NULL) {
        RTI_MQTT_SubscriptionStatusTypeSupport_delete_data(self->data);
        self->data = NULL;
    }

    if (!RTI_MQTT_SubscriptionParamsSeq_finalize(&self->req_ctx.params)) {
        /* TODO Log error */
        goto done;
    }

    *self = def_self;

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

static RTIBool RTI_MQTT_SubscriptionPtr_initialize_w_params(
        struct RTI_MQTT_Subscription **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

static RTIBool RTI_MQTT_SubscriptionPtr_finalize_w_params(
        struct RTI_MQTT_Subscription **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

static RTIBool RTI_MQTT_SubscriptionPtr_copy(
        struct RTI_MQTT_Subscription **dst,
        const struct RTI_MQTT_Subscription **src)
{
    *dst = (struct RTI_MQTT_Subscription *) *src;
    return RTI_TRUE;
}

#define T struct RTI_MQTT_Subscription *
#define TSeq RTI_MQTT_SubscriptionPtrSeq
#define T_initialize_w_params RTI_MQTT_SubscriptionPtr_initialize_w_params
#define T_finalize_w_params RTI_MQTT_SubscriptionPtr_finalize_w_params
#define T_copy RTI_MQTT_SubscriptionPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T
