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

#include "Client.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::Client"

/*****************************************************************************
 *                        Private Functions Declarations
 *****************************************************************************/

static DDS_ReturnCode_t RTI_MQTT_Client_initialize(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_ClientConfig *config);

static DDS_ReturnCode_t RTI_MQTT_Client_finalize(struct RTI_MQTT_Client *self);

static DDS_ReturnCode_t RTI_MQTT_Client_set_state(
        struct RTI_MQTT_Client *self,
        RTI_MQTT_ClientStateKind next_state,
        RTI_MQTT_ClientStateKind *prev_state_out);

static RTI_MQTT_ClientStateKind
        RTI_MQTT_Client_get_state(struct RTI_MQTT_Client *self);

#define RTI_MQTT_Client_get_state(c_) \
    (((c_) != NULL) ? (c_)->data->state : RTI_MQTT_ClientStateKind_ERROR)

static DDS_ReturnCode_t RTI_MQTT_Client_reconnect(struct RTI_MQTT_Client *self);

static DDS_ReturnCode_t
        RTI_MQTT_Client_submit_all_subscriptions(struct RTI_MQTT_Client *self);

static DDS_ReturnCode_t
        RTI_MQTT_Client_cancel_all_subscriptions(struct RTI_MQTT_Client *self);

static DDS_ReturnCode_t RTI_MQTT_Client_submit_subscriptions(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req);

static DDS_ReturnCode_t RTI_MQTT_Client_cancel_subscriptions(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req);

static DDS_ReturnCode_t RTI_MQTT_Client_get_all_subscription_params(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_SubscriptionParamsSeq *params);

static DDS_ReturnCode_t RTI_MQTT_Client_get_subscription_params(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub,
        struct RTI_MQTT_SubscriptionParamsSeq *params);

static void RTI_MQTT_Client_on_connection_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result);

static void RTI_MQTT_Client_on_disconnection_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result);

static void RTI_MQTT_Client_on_write_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result);

static void RTI_MQTT_Client_on_write_delivery_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result);

static void RTI_MQTT_Client_on_subscription_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result);

static void RTI_MQTT_Client_on_unsubscription_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result);

static DDS_ReturnCode_t RTI_MQTT_Client_new_request(
        struct RTI_MQTT_Client *self,
        RTI_MQTT_PendingRequest_ResultHandlerFn result_handler,
        void *context,
        RTI_MQTT_Time *timeout,
        struct RTI_MQTT_PendingRequest **req_out);

static DDS_ReturnCode_t RTI_MQTT_Client_delete_request(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest **req_out);

static void RTI_MQTT_Client_handle_request_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result);

static DDS_ReturnCode_t RTI_MQTT_Client_create_publication_requests(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub);

static DDS_ReturnCode_t RTI_MQTT_Client_create_subscription_requests(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub);

static DDS_ReturnCode_t RTI_MQTT_Client_delete_publication_requests(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub);

static DDS_ReturnCode_t RTI_MQTT_Client_delete_subscription_requests(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub);

static DDS_ReturnCode_t RTI_MQTT_Client_add_subscription(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub);

static DDS_ReturnCode_t RTI_MQTT_Client_remove_subscription(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub);

static DDS_ReturnCode_t RTI_MQTT_Client_add_publication(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub);

static DDS_ReturnCode_t RTI_MQTT_Client_remove_publication(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub);


/*****************************************************************************
 *                      RTI_MQTT_PendingRequestPtrSeq
 *****************************************************************************/

static RTIBool RTI_MQTT_PendingRequestPtr_initialize_w_params(
        struct RTI_MQTT_PendingRequest **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

static RTIBool RTI_MQTT_PendingRequestPtr_finalize_w_params(
        struct RTI_MQTT_PendingRequest **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

static RTIBool RTI_MQTT_PendingRequestPtr_copy(
        struct RTI_MQTT_PendingRequest **dst,
        const struct RTI_MQTT_PendingRequest **src)
{
    *dst = (struct RTI_MQTT_PendingRequest *) *src;
    return RTI_TRUE;
}

#define T struct RTI_MQTT_PendingRequest *
#define TSeq RTI_MQTT_PendingRequestPtrSeq
#define T_initialize_w_params RTI_MQTT_PendingRequestPtr_initialize_w_params
#define T_finalize_w_params RTI_MQTT_PendingRequestPtr_finalize_w_params
#define T_copy RTI_MQTT_PendingRequestPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T

/*****************************************************************************
 *                     RTI_MQTT_PendingRequest Support
 *****************************************************************************/
static void RTI_MQTT_Client_handle_request_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Client *self = req->client;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_handle_request_result)

    req->result = result;

    if (DDS_RETCODE_OK
        != DDS_GuardCondition_set_trigger_value(
                req->condition,
                DDS_BOOLEAN_TRUE)) {
        RTI_MQTT_LOG_CLIENT_SET_GUARD_CONDITION_TRIGGER_FAILED(
                self,
                req->condition,
                DDS_BOOLEAN_TRUE)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        RTI_MQTT_ERROR_3(
                "FAILED to handle request result:",
                "client=%p, req=%p, result=%d",
                self,
                req,
                result)

        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_set_state(
                    self,
                    RTI_MQTT_ClientStateKind_ERROR,
                    NULL)) {
            RTI_MQTT_LOG_CLIENT_SET_ERROR_FAILED(self)
        }
    }
}

static DDS_ReturnCode_t RTI_MQTT_Client_new_request(
        struct RTI_MQTT_Client *self,
        RTI_MQTT_PendingRequest_ResultHandlerFn result_handler,
        void *context,
        RTI_MQTT_Time *timeout,
        struct RTI_MQTT_PendingRequest **req_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_PendingRequest *req = NULL, **req_ref = NULL,
                                   def_req =
                                           RTI_MQTT_PendingRequest_INITIALIZER;
    DDS_Boolean attached = DDS_BOOLEAN_FALSE,
                seq_cond_initd = DDS_BOOLEAN_FALSE, locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_new_request)

    *req_out = NULL;

    req = (struct RTI_MQTT_PendingRequest *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_MQTT_PendingRequest));
    if (req == NULL) {
        /* TODO Log error */
        goto done;
    }

    *req = def_req;

    req->client = self;
    req->context = context;
    req->timeout = *timeout;
    req->result_handler = result_handler;

    req->condition = DDS_GuardCondition_new();
    if (req->condition == NULL) {
        /* TODO Log error */
        goto done;
    }
    req->waitset = DDS_WaitSet_new();
    if (req->waitset == NULL) {
        /* TODO Log error */
        goto done;
    }

    if (!DDS_ConditionSeq_initialize(&req->cond_seq)) {
        /* TODO Log error */
    }
    seq_cond_initd = DDS_BOOLEAN_TRUE;

    if (!DDS_ConditionSeq_set_maximum(&req->cond_seq, 1)) {
        RTI_MQTT_LOG_SET_SEQUENCE_MAX_FAILED(&req->cond_seq, 1)
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_WaitSet_attach_condition(
                req->waitset,
                DDS_GuardCondition_as_condition(req->condition))) {
        /* TODO Log error */
        goto done;
    }
    attached = DDS_BOOLEAN_TRUE;

    *req_out = req;

    retcode = DDS_RETCODE_OK;

done:

    if (retcode != DDS_RETCODE_OK) {
        if (req != NULL) {
            if (attached) {
                if (DDS_RETCODE_OK
                    != DDS_WaitSet_detach_condition(
                            req->waitset,
                            DDS_GuardCondition_as_condition(req->condition))) {
                    /* TODO Log error */
                }
            }
            if (seq_cond_initd) {
                if (!DDS_ConditionSeq_finalize(&req->cond_seq)) {
                    /* TODO Log error */
                }
            }
            if (req->waitset != NULL) {
                DDS_WaitSet_delete(req->waitset);
            }
            if (req->condition != NULL) {
                DDS_GuardCondition_delete(req->condition);
            }

            RTI_MQTT_Heap_free(req);
        }
    }
    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_delete_request(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest **req_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_PendingRequest *req = *req_out;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_delete_request)

    *req_out = NULL;

    if (req == NULL) {
        /* TODO Log error */
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    if (req->waitset != NULL && req->condition != NULL) {
        if (DDS_RETCODE_OK
            != DDS_WaitSet_detach_condition(
                    req->waitset,
                    DDS_GuardCondition_as_condition(req->condition))) {
            /* TODO Log error */
        }
    }

    if (req->waitset != NULL) {
        if (DDS_RETCODE_OK != DDS_WaitSet_delete(req->waitset)) {
            /* TODO Log error */
        }
    }

    if (req->condition != NULL) {
        if (DDS_RETCODE_OK != DDS_GuardCondition_delete(req->condition)) {
            /* TODO Log error */
        }
    }

    if (!DDS_ConditionSeq_finalize(&req->cond_seq)) {
        /* TODO Log error */
    }

    RTI_MQTT_Heap_free(req);

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_wait_for_request(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_Duration_t dds_timeout = DDS_DURATION_INFINITE;
    DDS_UnsignedLong seq_len = 0, i = 0;
    DDS_Boolean triggered = DDS_BOOLEAN_FALSE;
    DDS_Condition *condition = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_wait_for_request)

    if (!RTI_MQTT_Time_is_zero(&req->timeout)) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Time_to_dds_duration(&req->timeout, &dds_timeout)) {
            RTI_MQTT_TIME_TO_DURATION_FAILED(&req->timeout)
            goto done;
        }
    }

    retcode = DDS_WaitSet_wait(req->waitset, &req->cond_seq, &dds_timeout);

    if (retcode != DDS_RETCODE_OK) {
        RTI_MQTT_WAITSET_WAIT_FAILED(req->waitset)
        goto done;
    }
    retcode = DDS_RETCODE_ERROR;

    condition = DDS_GuardCondition_as_condition(req->condition);

    seq_len = DDS_ConditionSeq_get_length(&req->cond_seq);
    for (i = 0; i < seq_len && !triggered; i++) {
        DDS_Condition *active_cond =
                *DDS_ConditionSeq_get_reference(&req->cond_seq, i);

        if (active_cond != condition) {
            continue;
        }

        triggered = DDS_BOOLEAN_TRUE;
    }

    if (!triggered) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_GuardCondition_set_trigger_value(
                req->condition,
                DDS_BOOLEAN_FALSE)) {
        RTI_MQTT_LOG_CLIENT_SET_GUARD_CONDITION_TRIGGER_FAILED(
                self,
                req->condition,
                DDS_BOOLEAN_FALSE)
        goto done;
    }

    retcode = req->result;

done:

    return retcode;
}


static DDS_ReturnCode_t RTI_MQTT_Client_create_publication_requests(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_create_publication_requests)

    RTI_MQTT_Mutex_assert(&self->pub_lock);

    pub->req_ctx.pub = pub;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_new_request(
                self,
                RTI_MQTT_Client_on_write_result,
                &pub->req_ctx,
                &self->data->config->max_reply_timeout,
                &pub->req)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->pub_lock);

    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_create_subscription_requests(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_create_subscription_requests)

    RTI_MQTT_Mutex_assert(&self->sub_lock);

    sub->req_ctx.sub = sub;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_get_subscription_params(
                self,
                sub,
                &sub->req_ctx.params)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_get_all_subscription_params(
                self,
                &self->req_ctx_sub.params)) {
        /* TODO Log error */
        goto done;
    }


    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_new_request(
                self,
                RTI_MQTT_Client_on_subscription_result,
                &sub->req_ctx,
                &self->data->config->max_reply_timeout,
                &sub->req_sub)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_new_request(
                self,
                RTI_MQTT_Client_on_unsubscription_result,
                &sub->req_ctx,
                &self->data->config->max_reply_timeout,
                &sub->req_unsub)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;
done:
    RTI_MQTT_Mutex_release(&self->sub_lock);

    return retcode;
}


static DDS_ReturnCode_t RTI_MQTT_Client_delete_publication_requests(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_delete_publication_requests)

    RTI_MQTT_Mutex_assert(&self->pub_lock);

    if (DDS_RETCODE_OK != RTI_MQTT_Client_delete_request(self, &pub->req)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->pub_lock);
    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_delete_subscription_requests(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_delete_subscription_requests)

    RTI_MQTT_Mutex_assert(&self->sub_lock);

    if (!RTI_MQTT_SubscriptionParamsSeq_set_length(&sub->req_ctx.params, 0)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_get_all_subscription_params(
                self,
                &self->req_ctx_sub.params)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Client_delete_request(self, &sub->req_sub)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_delete_request(self, &sub->req_unsub)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->sub_lock);
    return retcode;
}

/*****************************************************************************
 *                     Non-request Event Handlers
 *****************************************************************************/

DDS_ReturnCode_t
        RTI_MQTT_Client_on_connection_lost(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    DDS_Boolean locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_on_connection_lost)

    RTI_MQTT_ERROR_1("connection LOST", "client=%p", self)

    RTI_MQTT_Mutex_assert_w_state(&self->cfg_lock, &locked) if (
            RTI_MQTT_Client_get_state(self)
            != RTI_MQTT_ClientStateKind_CONNECTED)
    {
        retcode = DDS_RETCODE_OK;
        goto done;
    }
    RTI_MQTT_Mutex_release_w_state(&self->cfg_lock, &locked)

            if (DDS_RETCODE_OK
                != RTI_MQTT_Client_set_state(
                        self,
                        RTI_MQTT_ClientStateKind_DISCONNECTED,
                        NULL))
    {
        /* TODO Log error */
        goto done;
    }

    /* Try to re-establish a connection */
    if (DDS_RETCODE_OK != RTI_MQTT_Client_reconnect(self)) {
        RTI_MQTT_LOG_CLIENT_RECONNECT_FAILED(self)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release_from_state(&self->cfg_lock, &locked)

            if (retcode != DDS_RETCODE_OK)
    {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_set_state(
                    self,
                    RTI_MQTT_ClientStateKind_ERROR,
                    NULL)) {
            RTI_MQTT_LOG_CLIENT_SET_ERROR_FAILED(self)
        }
    }
    return retcode;
}


DDS_ReturnCode_t RTI_MQTT_Client_on_message_arrived(
        struct RTI_MQTT_Client *self,
        const char *topic,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        RTI_MQTT_MessageInfo *msg_info)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    DDS_Boolean match = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong seq_len = 0, i = 0;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_on_message_arrived)

    RTI_MQTT_TRACE_2(
            "message RECEIVED:",
            "topic=%s, size=%u",
            topic,
            buffer_len)

    RTI_MQTT_Mutex_assert(&self->sub_lock);

    seq_len = RTI_MQTT_SubscriptionPtrSeq_get_length(&self->subscriptions);
    for (i = 0; i < seq_len; i++) {
        struct RTI_MQTT_Subscription *sub =
                *RTI_MQTT_SubscriptionPtrSeq_get_reference(
                        &self->subscriptions,
                        i);

        if (DDS_RETCODE_OK != RTI_MQTT_Subscription_match(sub, topic, &match)) {
            RTI_MQTT_LOG_CLIENT_MATCH_SUBSCRIPTION_FAILED(self, sub)
            goto done;
        }

        if (match) {
            RTI_MQTT_TRACE_3(
                    "DELIVER message:",
                    "client=%p sub=%p, topic=%s",
                    self,
                    sub,
                    topic)

            if (DDS_RETCODE_OK
                != RTI_MQTT_Subscription_receive(
                        sub,
                        buffer,
                        buffer_len,
                        topic,
                        msg_info,
                        NULL /* dropped */,
                        NULL /* lost */)) {
                RTI_MQTT_LOG_CLIENT_SUBSCRIPTION_RECEIVE_FAILED(self, sub)
                goto done;
            }
        }
    }

    retcode = DDS_RETCODE_OK;
done:
    RTI_MQTT_Mutex_release(&self->sub_lock);

    if (retcode != DDS_RETCODE_OK) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_set_state(
                    self,
                    RTI_MQTT_ClientStateKind_ERROR,
                    NULL)) {
            RTI_MQTT_LOG_CLIENT_SET_ERROR_FAILED(self)
        }
    }

    return retcode;
}

/*****************************************************************************
 *                        Write Message API
 *****************************************************************************/

DDS_ReturnCode_t RTI_MQTT_Client_write_message(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        const char *topic,
        RTI_MQTT_WriteParams *params)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_write_message)

    RTI_MQTT_Mutex_assert(&self->pub_lock);

    if (DDS_RETCODE_OK
        != RTI_MQTT_ClientMqttApi_write_message(
                self,
                buffer,
                buffer_len,
                topic,
                params,
                pub->req)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    RTI_MQTT_Mutex_release(&self->pub_lock);

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Client_wait_for_write_result(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_wait_for_write_result)

    /* Messages sent with Qos O do not require any acknowledgement from
       the Broker so we don't wait for a reply, but we do notify the
       publication so that it can update its statistics. */
    if (DDS_RETCODE_OK != RTI_MQTT_Client_wait_for_request(self, pub->req)) {
        RTI_MQTT_LOG_CLIENT_WAIT_FAILED(self, "publication")
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

/*****************************************************************************
 *                     RTI_MQTT_PendingRequest Result Handlers
 *****************************************************************************/

static void RTI_MQTT_Client_on_connection_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_Client_on_connection_result)

#if RTI_MQTT_USE_LOG
    if (result != DDS_RETCODE_OK) {
        RTI_MQTT_ERROR_3(
                "connection ERROR",
                "client=%p, req=%p result=%d",
                req->client,
                req,
                result)
    } else {
        RTI_MQTT_LOG_2("connection OK", "client=%p, req=%p", req->client, req)
    }
#endif /* RTI_MQTT_USE_LOG */

    RTI_MQTT_Client_handle_request_result(req, result);
}

static void RTI_MQTT_Client_on_disconnection_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_Client_on_disconnection_result)

#if RTI_MQTT_USE_LOG
    if (result != DDS_RETCODE_OK) {
        RTI_MQTT_ERROR_3(
                "disconnection ERROR",
                "client=%p, req=%p, result=%d",
                req->client,
                req,
                result)
    } else {
        RTI_MQTT_LOG_2(
                "disconnection OK",
                "client=%p, req=%p",
                req->client,
                req)
    }
#endif /* RTI_MQTT_USE_LOG */

    RTI_MQTT_Client_handle_request_result(req, result);
}

static void RTI_MQTT_Client_on_subscription_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_Client_on_subscription_result)
#if RTI_MQTT_USE_LOG
    if (result != DDS_RETCODE_OK) {
        RTI_MQTT_ERROR_3(
                "subscription ERROR",
                "client=%p, req=%p, result=%d",
                req->client,
                req,
                result)
    } else {
        RTI_MQTT_LOG_2("subscription OK", "client=%p, req=%p", req->client, req)
    }
#endif /* RTI_MQTT_USE_LOG */
    RTI_MQTT_Client_handle_request_result(req, result);
}

static void RTI_MQTT_Client_on_unsubscription_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_Client_on_unsubscription_result)
#if RTI_MQTT_USE_LOG
    if (result != DDS_RETCODE_OK) {
        RTI_MQTT_ERROR_3(
                "unsubscription ERROR",
                "client=%p, req=%p, result=%d",
                req->client,
                req,
                result)
    } else {
        RTI_MQTT_LOG_2(
                "unsubscription OK",
                "client=%p, req=%p",
                req->client,
                req)
    }
#endif /* RTI_MQTT_USE_LOG */
    RTI_MQTT_Client_handle_request_result(req, result);
}

static void RTI_MQTT_Client_on_write_delivery_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Client *self = req->client;
    struct RTI_MQTT_PublicationRequestContext *req_ctx =
            (struct RTI_MQTT_PublicationRequestContext *) req->context;
    struct RTI_MQTT_Publication *pub = req_ctx->pub;
    RTI_MQTT_QosLevel pub_qos = req_ctx->last_write_qos;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_on_write_delivery_result)

#if RTI_MQTT_USE_LOG
    if (result != DDS_RETCODE_OK) {
        RTI_MQTT_ERROR_4(
                "write delivery ERROR",
                "client=%p, pub=%p, req=%p, result=%d",
                self,
                pub,
                req,
                result)
    } else {
        RTI_MQTT_TRACE_3(
                "write delivery OK",
                "client=%p, pub=%p, req=%p",
                self,
                pub,
                req)
    }
#endif /* RTI_MQTT_USE_LOG */

    if (pub_qos != RTI_MQTT_QosLevel_TWO) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Publication_on_write_delivery_result(pub, result)) {
        RTI_MQTT_LOG_CLIENT_NOTIFY_DELIVERY_RESULTS_FAILED(self, pub)
        goto done;
    }

    retcode = result;

done:

    RTI_MQTT_Client_handle_request_result(req, retcode);
}

static void RTI_MQTT_Client_on_write_result(
        struct RTI_MQTT_PendingRequest *req,
        DDS_ReturnCode_t result)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Client *self = req->client;
    struct RTI_MQTT_PublicationRequestContext *req_ctx =
            (struct RTI_MQTT_PublicationRequestContext *) req->context;
    struct RTI_MQTT_Publication *pub = req_ctx->pub;
    RTI_MQTT_QosLevel pub_qos = req_ctx->last_write_qos;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_on_write_result)

#if RTI_MQTT_USE_LOG
    if (result != DDS_RETCODE_OK) {
        RTI_MQTT_ERROR_4(
                "write ERROR",
                "client=%p, pub=%p, req=%p, result=%d",
                self,
                pub,
                req,
                result)
    } else {
        RTI_MQTT_TRACE_3(
                "write OK",
                "client=%p, pub=%p, req=%p",
                self,
                pub,
                req)
    }
#endif /* RTI_MQTT_USE_LOG */

    if (DDS_RETCODE_OK != RTI_MQTT_Publication_on_write_result(pub, result)) {
        RTI_MQTT_LOG_CLIENT_NOTIFY_WRITE_RESULTS_FAILED(self, pub)
        goto done;
    }

    retcode = result;

done:
    /* Ideally we would wait for delivery confirmation for Qos 2, but since
       we're not correlating tokens with Publications, we just assume all
       deliveries are completed and sucessful if the Broker accepted them */
    if (pub_qos != RTI_MQTT_QosLevel_TWO) {
        RTI_MQTT_Client_handle_request_result(req, retcode);
    } else {
        RTI_MQTT_Client_on_write_delivery_result(req, retcode);
    }
}

/*****************************************************************************
 *                          Public API Implementation
 *****************************************************************************/

const char *RTI_MQTT_Client_get_id(struct RTI_MQTT_Client *self)
{
    return self->data->config->id;
}

DDS_ReturnCode_t RTI_MQTT_Client_new(
        struct RTI_MQTT_ClientConfig *config,
        struct RTI_MQTT_Client **client_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Client *client = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_new)

    *client_out = NULL;

    client = (struct RTI_MQTT_Client *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_MQTT_Client));
    if (client == NULL) {
        RTI_MQTT_HEAP_ALLOCATE_FAILED(sizeof(struct RTI_MQTT_Client))
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Client_initialize(client, config)) {
        RTI_MQTT_LOG_CLIENT_INIT_FAILED(client, config)
        goto done;
    }

    *client_out = client;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (client != NULL) {
            RTI_MQTT_Heap_free(client);
        }
    }

    return retval;
}

void RTI_MQTT_Client_delete(struct RTI_MQTT_Client *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_Client_delete)

    if (self == NULL) {
        return;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Client_finalize(self)) {
        RTI_MQTT_LOG_CLIENT_FINALIZE_FAILED(self)
    }

    RTI_MQTT_Heap_free(self);
}


DDS_ReturnCode_t RTI_MQTT_Client_connect(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean cond_triggered = DDS_BOOLEAN_FALSE,
                wait_timedout = DDS_BOOLEAN_FALSE, locked = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong tot_servers = 0;
    RTI_MQTT_ClientStateKind client_state = RTI_MQTT_ClientStateKind_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_connect)

    RTI_MQTT_Mutex_assert_w_state(&self->cfg_lock, &locked);

    if (RTI_MQTT_Client_get_state(self) == RTI_MQTT_ClientStateKind_CONNECTED) {
        RTI_MQTT_TRACE_1("client ALREADY connected:", "client=%p", self)
        retval = DDS_RETCODE_OK;
        goto done;
    }

    if (RTI_MQTT_Client_get_state(self) == RTI_MQTT_ClientStateKind_CONNECTING
        || RTI_MQTT_Client_get_state(self)
                == RTI_MQTT_ClientStateKind_DISCONNECTING) {
        RTI_MQTT_ERROR_2(
                "invalid client state for connection:",
                "client=%p, state=%s",
                self,
                RTI_MQTT_ClientStateKind_as_string(
                        RTI_MQTT_Client_get_state(self)))
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_set_state(
                self,
                RTI_MQTT_ClientStateKind_CONNECTING,
                NULL)) {
        /* TODO Log error */
        goto done;
    }
    RTI_MQTT_Mutex_release_w_state(&self->cfg_lock, &locked);

    if (DDS_RETCODE_OK != RTI_MQTT_ClientMqttApi_connect(self)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_wait_for_request(self, self->req_connect)) {
        RTI_MQTT_LOG_CLIENT_WAIT_FAILED(self, "connection")
        goto done;
    }

    /* We always resubmit current subscriptions to make sure they exist
       on the broker. */
    if (DDS_RETCODE_OK != RTI_MQTT_Client_submit_all_subscriptions(self)) {
        RTI_MQTT_LOG_CLIENT_SUBMIT_SUBSCRIPTIONS_FAILED(self)
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_set_state(
                self,
                RTI_MQTT_ClientStateKind_CONNECTED,
                NULL)) {
        /* TODO Log error */
        goto done;
    }
    RTI_MQTT_LOG_1("client CONNECTED", "client=%p", self)

    retval = DDS_RETCODE_OK;
done:
    RTI_MQTT_Mutex_release_from_state(&self->cfg_lock, &locked);

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Client_disconnect(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean unsubscribe = DDS_BOOLEAN_FALSE, locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_disconnect)

    RTI_MQTT_Mutex_assert_w_state(&self->cfg_lock, &locked);

    if (RTI_MQTT_Client_get_state(self)
        == RTI_MQTT_ClientStateKind_DISCONNECTED) {
        RTI_MQTT_TRACE_1(
                "client ALREADY disconnected:",
                "client=%s",
                self->data->config->id)
        retval = DDS_RETCODE_OK;
        goto done;
    }
    if (RTI_MQTT_Client_get_state(self) == RTI_MQTT_ClientStateKind_CONNECTING
        || RTI_MQTT_Client_get_state(self)
                == RTI_MQTT_ClientStateKind_DISCONNECTING) {
        RTI_MQTT_ERROR_2(
                "invalid client state for connection:",
                "client=%p, state=%s",
                self,
                RTI_MQTT_ClientStateKind_as_string(
                        RTI_MQTT_Client_get_state(self)))
        goto done;
    }

    unsubscribe = self->data->config->unsubscribe_on_disconnect;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_set_state(
                self,
                RTI_MQTT_ClientStateKind_DISCONNECTING,
                NULL)) {
        /* TODO Log error */
        goto done;
    }

    RTI_MQTT_Mutex_release_w_state(&self->cfg_lock, &locked);

    if (unsubscribe) {
        if (DDS_RETCODE_OK != RTI_MQTT_Client_cancel_all_subscriptions(self)) {
            RTI_MQTT_LOG_CLIENT_CANCEL_SUBSCRIPTIONS_FAILED(self)
            goto done;
        }
    }

    if (DDS_RETCODE_OK != RTI_MQTT_ClientMqttApi_disconnect(self)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_wait_for_request(self, self->req_disconnect)) {
        RTI_MQTT_LOG_CLIENT_WAIT_FAILED(self, "disconnection")
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_set_state(
                self,
                RTI_MQTT_ClientStateKind_DISCONNECTED,
                NULL)) {
        /* TODO Log error */
        goto done;
    }

    RTI_MQTT_LOG_1("client DISCONNECTED", "client=%s", self->data->config->id)

    retval = DDS_RETCODE_OK;
done:
    RTI_MQTT_Mutex_release_from_state(&self->cfg_lock, &locked);

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Client_subscribe(
        struct RTI_MQTT_Client *self,
        RTI_MQTT_SubscriptionConfig *config,
        struct RTI_MQTT_Subscription **sub_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Subscription *sub = NULL;
    DDS_Boolean sub_added = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_subscribe)

    *sub_out = NULL;

    if (DDS_RETCODE_OK != RTI_MQTT_Subscription_new(self, config, &sub)) {
        RTI_MQTT_LOG_SUBSCRIPTION_CREATE_FAILED(self, config)
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Client_add_subscription(self, sub)) {
        /* TODO Log error */
        goto done;
    }
    sub_added = DDS_BOOLEAN_TRUE;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_create_subscription_requests(self, sub)) {
        /* TODO Log error */
        goto done;
    }

    /* Subscribe on Broker */
    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_submit_subscriptions(self, sub->req_sub)) {
        RTI_MQTT_LOG_CLIENT_SUBMIT_SUBSCRIPTION_FAILED(self, sub)
        goto done;
    }

#if RTI_MQTT_USE_LOG
    RTI_MQTT_Mutex_assert(&self->sub_lock);
    RTI_MQTT_LOG_2("subscription CREATED", "client=%p, sub=%p", self, sub)
    {
        DDS_UnsignedLong i = 0, topics_len = 0;
        topics_len =
                DDS_StringSeq_get_length(&sub->data->config->topic_filters);
        for (i = 0; i < topics_len; i++) {
            const char *topic = *DDS_StringSeq_get_reference(
                    &sub->data->config->topic_filters,
                    i);

            RTI_MQTT_LOG_2("  - topic", "[%d]: %s", i, topic)
        }
    }
    RTI_MQTT_LOG_1("  - qos:", "%d", sub->data->config->max_qos)
    RTI_MQTT_LOG_1(
            "  - queue_size:",
            "%d",
            sub->data->config->message_queue_size)
    RTI_MQTT_Mutex_release(&self->sub_lock);
#endif /* RTI_MQTT_USE_LOG */

    *sub_out = sub;
    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_delete_subscription_requests(self, sub)) {
            /* TODO Log error */
        }
        if (sub_added) {
            if (DDS_RETCODE_OK
                != RTI_MQTT_Client_remove_subscription(self, sub)) {
                /* TODO Log error */
            }
        }
        if (sub != NULL) {
            RTI_MQTT_Subscription_delete(sub);
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Client_unsubscribe(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_unsubscribe)

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_cancel_subscriptions(self, sub->req_unsub)) {
        RTI_MQTT_LOG_CLIENT_CANCEL_SUBSCRIPTION_FAILED(self, sub)
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_delete_subscription_requests(self, sub)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:

    if (sub != NULL) {
        if (DDS_RETCODE_OK != RTI_MQTT_Client_remove_subscription(self, sub)) {
            /* TODO Log error */
        }
        RTI_MQTT_Subscription_delete(sub);
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Client_publish(
        struct RTI_MQTT_Client *self,
        RTI_MQTT_PublicationConfig *config,
        struct RTI_MQTT_Publication **pub_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Publication *pub = NULL;
    DDS_Boolean pub_added = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_publish)

    *pub_out = NULL;

    if (DDS_RETCODE_OK != RTI_MQTT_Publication_new(self, config, &pub)) {
        RTI_MQTT_LOG_PUBLICATION_CREATE_FAILED(self, config)
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Client_add_publication(self, pub)) {
        /* TODO Log error */
        goto done;
    }
    pub_added = DDS_BOOLEAN_TRUE;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_create_publication_requests(self, pub)) {
        /* TODO Log error */
        goto done;
    }

#if RTI_MQTT_USE_LOG
    RTI_MQTT_Mutex_assert(&self->pub_lock);
    RTI_MQTT_LOG_2("publication CREATED", "client=%p, pub=%p", self, pub)
    if (pub->data->config->use_message_info) {
        RTI_MQTT_LOG_1(
                "  - use_message_info:",
                "%d",
                pub->data->config->use_message_info)
    } else {
        RTI_MQTT_LOG_1("  - topic:", "%s", pub->data->config->topic)
        RTI_MQTT_LOG_1(
                "  - qos:",
                "%s",
                RTI_MQTT_QosLevel_as_string(pub->data->config->qos))
        RTI_MQTT_LOG_1("  - retained:", "%d", pub->data->config->retained)
    }
    RTI_MQTT_LOG_2(
            "  - max wait time:",
            "%ds %uns",
            pub->data->config->max_wait_time.seconds,
            pub->data->config->max_wait_time.nanoseconds)
    RTI_MQTT_Mutex_release(&self->pub_lock);
#endif /* RTI_MQTT_USE_LOG */

    *pub_out = pub;
    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_delete_publication_requests(self, pub)) {
            /* TODO Log error */
        }
        if (pub_added) {
            if (DDS_RETCODE_OK
                != RTI_MQTT_Client_remove_publication(self, pub)) {
                /* TODO Log error */
            }
        }

        if (pub != NULL) {
            RTI_MQTT_Publication_delete(pub);
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Client_unpublish(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;


    RTI_MQTT_LOG_FN(RTI_MQTT_Client_unpublish)

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_delete_publication_requests(self, pub)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:

    if (pub != NULL) {
        if (DDS_RETCODE_OK != RTI_MQTT_Client_remove_publication(self, pub)) {
            /* TODO Log error */
        }
        RTI_MQTT_Publication_delete(pub);
    }

    return retval;
}


/*****************************************************************************
 *                     Private Functions Implementation
 *****************************************************************************/

static DDS_ReturnCode_t RTI_MQTT_Client_initialize(
        struct RTI_MQTT_Client *self,
        RTI_MQTT_ClientConfig *config)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Client def_self = RTI_MQTT_Client_INITIALIZER;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_initialize)

    *self = def_self;

    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_initialize(&self->cfg_lock)) {
        /* TODO Log error */
        return DDS_RETCODE_ERROR;
    }
    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_initialize(&self->mqtt_lock)) {
        /* TODO Log error */
        (void) RTI_MQTT_Mutex_finalize(&self->cfg_lock);
        return DDS_RETCODE_ERROR;
    }
    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_initialize(&self->sub_lock)) {
        /* TODO Log error */
        (void) RTI_MQTT_Mutex_finalize(&self->mqtt_lock);
        (void) RTI_MQTT_Mutex_finalize(&self->cfg_lock);
        return DDS_RETCODE_ERROR;
    }
    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_initialize(&self->pub_lock)) {
        /* TODO Log error */
        (void) RTI_MQTT_Mutex_finalize(&self->sub_lock);
        (void) RTI_MQTT_Mutex_finalize(&self->mqtt_lock);
        (void) RTI_MQTT_Mutex_finalize(&self->cfg_lock);
        return DDS_RETCODE_ERROR;
    }

    /* Create a RTI_MQTT_ClientStatus object to store the client's data */
    if (DDS_RETCODE_OK
        != RTI_MQTT_ClientStatus_new(DDS_BOOLEAN_TRUE, &self->data)) {
        RTI_MQTT_LOG_CREATE_DATA_FAILED("RTI_MQTT_ClientStatus")
        goto done;
    }
    self->data->state = RTI_MQTT_ClientStateKind_DISCONNECTED;

    /* Copy configuration from user to Client's state */
    if (!RTI_MQTT_ClientConfig_copy(self->data->config, config)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_MQTT_ClientConfig",
                config,
                self->data->config)
        goto done;
    }

    /* -------- Validate user-supplied configuration -------- */
    if (DDS_StringSeq_get_length(&self->data->config->server_uris) == 0) {
        RTI_MQTT_LOG_CLIENT_INVALID_CONFIG_DETECTED(
                self,
                "no server URIs specified")
        goto done;
    }

    if (!RTI_MQTT_SubscriptionPtrSeq_initialize(&self->subscriptions)) {
        RTI_MQTT_LOG_INITIALIZE_SEQUENCE_FAILED(&self->subscriptions)
        goto done;
    }

    if (!RTI_MQTT_PublicationPtrSeq_initialize(&self->publications)) {
        RTI_MQTT_LOG_INITIALIZE_SEQUENCE_FAILED(&self->publications)
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_new_request(
                self,
                RTI_MQTT_Client_on_connection_result,
                NULL,
                &self->data->config->connect_timeout,
                &self->req_connect)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_new_request(
                self,
                RTI_MQTT_Client_on_disconnection_result,
                NULL,
                &self->data->config->max_reply_timeout,
                &self->req_disconnect)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_new_request(
                self,
                RTI_MQTT_Client_on_subscription_result,
                &self->req_ctx_sub,
                &self->data->config->max_reply_timeout,
                &self->req_sub)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_new_request(
                self,
                RTI_MQTT_Client_on_unsubscription_result,
                &self->req_ctx_sub,
                &self->data->config->max_reply_timeout,
                &self->req_unsub)) {
        /* TODO Log error */
        goto done;
    }

    /* Initialize MQTT Client state */
    if (DDS_RETCODE_OK != RTI_MQTT_ClientMqttApi_create_client(self)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DDS_RETCODE_OK != retcode) {
        if (DDS_RETCODE_OK != RTI_MQTT_Client_finalize(self)) {
            /* TODO Log error */
        }
    }

    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_finalize(struct RTI_MQTT_Client *self)
{
    DDS_UnsignedLong seq_len = 0, i = 0;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_finalize)

    seq_len = RTI_MQTT_SubscriptionPtrSeq_get_length(&self->subscriptions);
    while (seq_len > 0) {
        struct RTI_MQTT_Subscription *sub =
                *RTI_MQTT_SubscriptionPtrSeq_get_reference(
                        &self->subscriptions,
                        0);
        if (DDS_RETCODE_OK != RTI_MQTT_Client_unsubscribe(self, sub)) {
            /* TODO Log error */
            goto done;
        }
        seq_len = RTI_MQTT_SubscriptionPtrSeq_get_length(&self->subscriptions);
    }

    seq_len = RTI_MQTT_PublicationPtrSeq_get_length(&self->publications);
    while (seq_len > 0) {
        struct RTI_MQTT_Publication *pub =
                *RTI_MQTT_PublicationPtrSeq_get_reference(
                        &self->publications,
                        0);
        if (DDS_RETCODE_OK != RTI_MQTT_Client_unpublish(self, pub)) {
            /* TODO Log error */
            goto done;
        }
        seq_len = RTI_MQTT_PublicationPtrSeq_get_length(&self->publications);
    }

    if (!RTI_MQTT_SubscriptionPtrSeq_finalize(&self->subscriptions)) {
        RTI_MQTT_LOG_FINALIZE_SEQUENCE_FAILED(&self->subscriptions)
        goto done;
    }

    if (!RTI_MQTT_PublicationPtrSeq_finalize(&self->publications)) {
        RTI_MQTT_LOG_FINALIZE_SEQUENCE_FAILED(&self->publications)
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_ClientMqttApi_Paho_delete_client(self)) {
        /* TODO Log error */
        goto done;
    }

    if (self->req_connect != NULL) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_delete_request(self, &self->req_connect)) {
            /* TODO Log error */
            goto done;
        }
    }

    if (self->req_disconnect != NULL) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_delete_request(self, &self->req_disconnect)) {
            /* TODO Log error */
            goto done;
        }
    }

    if (self->req_sub != NULL) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_delete_request(self, &self->req_sub)) {
            /* TODO Log error */
            goto done;
        }
    }

    if (self->req_unsub != NULL) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_delete_request(self, &self->req_unsub)) {
            /* TODO Log error */
            goto done;
        }
    }

    if (!RTI_MQTT_SubscriptionParamsSeq_finalize(&self->req_ctx_sub.params)) {
        /* TODO Log error */
        goto done;
    }

    if (self->data != NULL) {
        RTI_MQTT_ClientStatusTypeSupport_delete_data(self->data);
        self->data = NULL;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_finalize(&self->pub_lock)) {
        /* TODO Log error */
        goto done;
    }
    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_finalize(&self->sub_lock)) {
        /* TODO Log error */
        goto done;
    }
    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_finalize(&self->mqtt_lock)) {
        /* TODO Log error */
        goto done;
    }
    if (DDS_RETCODE_OK != RTI_MQTT_Mutex_finalize(&self->cfg_lock)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_set_state(
        struct RTI_MQTT_Client *self,
        RTI_MQTT_ClientStateKind next_state,
        RTI_MQTT_ClientStateKind *prev_state_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_ClientStateKind prev = RTI_MQTT_ClientStateKind_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_set_state)

    RTI_MQTT_Mutex_assert(&self->cfg_lock);

    prev = self->data->state;

    self->data->state = next_state;

    if (prev_state_out != NULL) {
        *prev_state_out = prev;
    }

    RTI_MQTT_LOG_3(
            "client STATE CHANGED",
            "client=%s, state=%s, prev=%s",
            self->data->config->id,
            RTI_MQTT_ClientStateKind_as_string(self->data->state),
            RTI_MQTT_ClientStateKind_as_string(prev))

    retval = DDS_RETCODE_OK;

    RTI_MQTT_Mutex_release(&self->cfg_lock);

    return retval;
}

static DDS_ReturnCode_t RTI_MQTT_Client_get_subscription_params(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub,
        struct RTI_MQTT_SubscriptionParamsSeq *params)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong t_filters_len = 0, params_len = 0, params_len_start = 0,
                     params_i = 0, i = 0;

    params_len_start = RTI_MQTT_SubscriptionParamsSeq_get_length(params);

    t_filters_len = DDS_StringSeq_get_length(&sub->data->config->topic_filters);

    params_len = params_len_start + t_filters_len;

    if (!RTI_MQTT_SubscriptionParamsSeq_ensure_length(
                params,
                params_len,
                params_len)) {
        /* TODO Log error */
        goto done;
    }

    for (params_i = params_len_start, i = 0;
         i < t_filters_len && params_i < params_len;
         i++, params_i++) {
        char *t_filter = *DDS_StringSeq_get_reference(
                &sub->data->config->topic_filters,
                i);
        RTI_MQTT_SubscriptionParams *sub_p =
                RTI_MQTT_SubscriptionParamsSeq_get_reference(params, params_i);

        sub_p->max_qos = sub->data->config->max_qos;
        DDS_String_replace(&sub_p->topic, t_filter);
        if (sub_p->topic == NULL) {
            /* TODO Log error */
            goto done;
        }
    }

#if RTI_MQTT_USE_TRACE
    RTI_MQTT_TRACE_2(
            "SUBSCRIPTION parameters:",
            "sub=%p len=%d",
            sub,
            params_len)
    for (i = params_len_start; i < params_len; i++) {
        RTI_MQTT_SubscriptionParams *sub_p =
                RTI_MQTT_SubscriptionParamsSeq_get_reference(params, i);
        RTI_MQTT_TRACE_2(
                "  - ",
                "%s [%s]",
                sub_p->topic,
                RTI_MQTT_QosLevel_as_string(sub_p->max_qos))
    }
#endif /* RTI_MQTT_USE_TRACE */

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        if (!RTI_MQTT_SubscriptionParamsSeq_set_length(
                    params,
                    params_len_start)) {
            /* TODO Log error */
        }
    }
    return retcode;
}


static DDS_ReturnCode_t RTI_MQTT_Client_get_all_subscription_params(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_SubscriptionParamsSeq *params)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong subs_len = 0, i = 0, j = 0, params_len = 0,
                     params_all_len = 0;

    subs_len = RTI_MQTT_SubscriptionPtrSeq_get_length(&self->subscriptions);

    if (!RTI_MQTT_SubscriptionParamsSeq_set_length(params, 0)) {
        /* TODO Log error */
        goto done;
    }

    for (i = 0; i < subs_len; i++, params_all_len += params_len) {
        struct RTI_MQTT_Subscription *sub =
                *RTI_MQTT_SubscriptionPtrSeq_get_reference(
                        &self->subscriptions,
                        i);

        params_len =
                RTI_MQTT_SubscriptionParamsSeq_get_length(&sub->req_ctx.params);

        if (!RTI_MQTT_SubscriptionParamsSeq_ensure_length(
                    params,
                    params_all_len + params_len,
                    params_all_len + params_len)) {
            /* TODO Log error */
            goto done;
        }

        for (j = 0; j < params_len; j++) {
            if (!RTI_MQTT_SubscriptionParams_copy(
                        RTI_MQTT_SubscriptionParamsSeq_get_reference(
                                params,
                                params_all_len + j),
                        RTI_MQTT_SubscriptionParamsSeq_get_reference(
                                &sub->req_ctx.params,
                                j))) {
                /* TODO Log error */
                goto done;
            }
        }
    }

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        RTI_MQTT_SubscriptionParamsSeq_set_length(params, 0);
    }
    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_reconnect(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_ClientStateKind client_state = RTI_MQTT_Client_get_state(self);
    DDS_Boolean locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_reconnect)

#if 0
    /* This function can only be called while the client is connected
       to the Broker (or just thinks it still is) */
    if (client_state != RTI_MQTT_ClientStateKind_CONNECTED)
    {
        RTI_MQTT_LOG_UNEXPECTED_CLIENT_STATE_DETECTED(
                    self, RTI_MQTT_ClientStateKind_CONNECTED, client_state)
        goto done;
    }
#endif

    RTI_MQTT_Mutex_assert_w_state(&self->cfg_lock, &locked);

    /* If we're configured to not reconnect, we do nothing and exit */
    if (!self->data->config->reconnect) {
        RTI_MQTT_LOG_1("SKIP reconnect:", "client=%s", self->data->config->id)
        retval = DDS_RETCODE_OK;
        goto done;
    }

    RTI_MQTT_Mutex_release_w_state(&self->cfg_lock, &locked);

    if (DDS_RETCODE_OK != RTI_MQTT_Client_connect(self)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    RTI_MQTT_Mutex_release_from_state(&self->cfg_lock, &locked);
    return retval;
}

static DDS_ReturnCode_t
        RTI_MQTT_Client_submit_all_subscriptions(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_PendingRequest req_copy =
            RTI_MQTT_PendingRequest_INITIALIZER;
    struct RTI_MQTT_SubscriptionRequestContext req_ctx_copy =
            RTI_MQTT_SubscriptionRequestContext_INITIALIZER;
    DDS_Boolean locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_Mutex_assert_w_state(&self->sub_lock, &locked);
    req_copy = *self->req_sub;
    req_copy.context = &req_ctx_copy;
    if (!RTI_MQTT_SubscriptionParamsSeq_copy(
                &req_ctx_copy.params,
                &self->req_ctx_sub.params)) {
        /* TODO Log error */
        goto done;
    }
    RTI_MQTT_Mutex_release_w_state(&self->sub_lock, &locked);

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_submit_subscriptions(self, &req_copy)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release_from_state(&self->sub_lock, &locked);

    if (!RTI_MQTT_SubscriptionParamsSeq_finalize(&req_ctx_copy.params)) {
        /* TODO Log error */
    }

    return retcode;
}

static DDS_ReturnCode_t
        RTI_MQTT_Client_cancel_all_subscriptions(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_PendingRequest req_copy =
            RTI_MQTT_PendingRequest_INITIALIZER;
    struct RTI_MQTT_SubscriptionRequestContext req_ctx_copy =
            RTI_MQTT_SubscriptionRequestContext_INITIALIZER;
    DDS_Boolean locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_Mutex_assert_w_state(&self->sub_lock, &locked);
    req_copy = *self->req_unsub;
    req_copy.context = &req_ctx_copy;
    if (!RTI_MQTT_SubscriptionParamsSeq_copy(
                &req_ctx_copy.params,
                &self->req_ctx_sub.params)) {
        /* TODO Log error */
        goto done;
    }
    RTI_MQTT_Mutex_release_w_state(&self->sub_lock, &locked);

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_cancel_subscriptions(self, &req_copy)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release_from_state(&self->sub_lock, &locked);

    if (!RTI_MQTT_SubscriptionParamsSeq_finalize(&req_ctx_copy.params)) {
        /* TODO Log error */
    }

    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_submit_subscriptions(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0;
    struct RTI_MQTT_SubscriptionRequestContext *req_ctx =
            (struct RTI_MQTT_SubscriptionRequestContext *) req->context;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_submit_subscriptions)

    seq_len = RTI_MQTT_SubscriptionParamsSeq_get_length(&req_ctx->params);

#if RTI_MQTT_USE_LOG
    {
        DDS_UnsignedLong i = 0;

        RTI_MQTT_LOG_2(
                "SUBMIT subscriptions:",
                "client=%p, subs=%d",
                self,
                seq_len)
        for (i = 0; i < seq_len; i++) {
            RTI_MQTT_SubscriptionParams *sub_p =
                    RTI_MQTT_SubscriptionParamsSeq_get_reference(
                            &req_ctx->params,
                            i);
            RTI_MQTT_LOG_2(
                    "  - ",
                    "%s [%s]",
                    sub_p->topic,
                    RTI_MQTT_QosLevel_as_string(sub_p->max_qos))
        }
    }
#endif /* RTI_MQTT_USE_LOG */

    if (seq_len == 0) {
        return DDS_RETCODE_OK;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_ClientMqttApi_submit_subscriptions(self, req)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Client_wait_for_request(self, req)) {
        RTI_MQTT_LOG_CLIENT_WAIT_FAILED(self, "subscription")
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:

    return retval;
}


static DDS_ReturnCode_t RTI_MQTT_Client_cancel_subscriptions(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0;
    struct RTI_MQTT_SubscriptionRequestContext *req_ctx =
            (struct RTI_MQTT_SubscriptionRequestContext *) req->context;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_cancel_subscriptions)

    seq_len = RTI_MQTT_SubscriptionParamsSeq_get_length(&req_ctx->params);

#if RTI_MQTT_USE_LOG
    {
        DDS_UnsignedLong i = 0;
        RTI_MQTT_LOG_2(
                "CANCEL subscriptions:",
                "client=%p, subs=%d",
                self,
                seq_len)
        for (i = 0; i < seq_len; i++) {
            RTI_MQTT_SubscriptionParams *sub_p =
                    RTI_MQTT_SubscriptionParamsSeq_get_reference(
                            &req_ctx->params,
                            i);
            RTI_MQTT_LOG_2(
                    "  - ",
                    "%s [%s]",
                    sub_p->topic,
                    RTI_MQTT_QosLevel_as_string(sub_p->max_qos))
        }
    }
#endif /* RTI_MQTT_USE_LOG */

    if (seq_len == 0) {
        return DDS_RETCODE_OK;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_ClientMqttApi_cancel_subscriptions(self, req)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Client_wait_for_request(self, req)) {
        RTI_MQTT_LOG_CLIENT_WAIT_FAILED(self, "unsubscription")
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}


static DDS_ReturnCode_t RTI_MQTT_Client_add_subscription(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0;
    struct RTI_MQTT_Subscription **sub_ref = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_add_subscription)

    RTI_MQTT_Mutex_assert(&self->sub_lock);
    seq_len = RTI_MQTT_SubscriptionPtrSeq_get_length(&self->subscriptions);

    if (!RTI_MQTT_SubscriptionPtrSeq_ensure_length(
                &self->subscriptions,
                seq_len + 1,
                seq_len + 1)) {
        RTI_MQTT_LOG_SET_SEQUENCE_ENSURE_LENGTH_FAILED(
                &self->subscriptions,
                seq_len + 1,
                seq_len + 1)
        goto done;
    }

    sub_ref = RTI_MQTT_SubscriptionPtrSeq_get_reference(
            &self->subscriptions,
            seq_len);
    *sub_ref = sub;

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->sub_lock);
    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_remove_subscription(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Subscription *sub)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i = 0, seq_len = 0, rm_i = 0;
    DDS_Boolean rm_sub = DDS_BOOLEAN_FALSE;
    struct RTI_MQTT_Subscription **sub_ref = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_remove_subscription)

    RTI_MQTT_Mutex_assert(&self->sub_lock);

    seq_len = RTI_MQTT_SubscriptionPtrSeq_get_length(&self->subscriptions);

    for (i = 0; i < seq_len; i++) {
        sub_ref = RTI_MQTT_SubscriptionPtrSeq_get_reference(
                &self->subscriptions,
                i);
        if (sub == *sub_ref) {
            rm_sub = DDS_BOOLEAN_TRUE;
            rm_i = i;
            break;
        }
    }
    if (!rm_sub) {
        /* The subscription doesn't belong to this Client */
        RTI_MQTT_LOG_CLIENT_UNKNOWN_SUBSCRIPTION_DETECTED(self, sub)
        goto done;
    }

    for (i = rm_i; i + 1 < seq_len; i++) {
        *RTI_MQTT_SubscriptionPtrSeq_get_reference(&self->subscriptions, i) =
                *RTI_MQTT_SubscriptionPtrSeq_get_reference(
                        &self->subscriptions,
                        i + 1);
    }
    if (!RTI_MQTT_SubscriptionPtrSeq_set_length(
                &self->subscriptions,
                seq_len - 1)) {
        RTI_MQTT_LOG_SET_SEQUENCE_LENGTH_FAILED(
                &self->subscriptions,
                seq_len - 1)
        goto done;
    }
    if (!RTI_MQTT_SubscriptionPtrSeq_set_maximum(
                &self->subscriptions,
                seq_len - 1)) {
        RTI_MQTT_LOG_SET_SEQUENCE_MAX_FAILED(&self->subscriptions, seq_len - 1)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->sub_lock) return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_add_publication(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0;
    struct RTI_MQTT_Publication **pub_ref = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_add_publication)

    RTI_MQTT_Mutex_assert(&self->pub_lock);
    seq_len = RTI_MQTT_PublicationPtrSeq_get_length(&self->publications);

    if (!RTI_MQTT_PublicationPtrSeq_ensure_length(
                &self->publications,
                seq_len + 1,
                seq_len + 1)) {
        RTI_MQTT_LOG_SET_SEQUENCE_ENSURE_LENGTH_FAILED(
                &self->publications,
                seq_len + 1,
                seq_len + 1)
        goto done;
    }

    pub_ref = RTI_MQTT_PublicationPtrSeq_get_reference(
            &self->publications,
            seq_len);

    *pub_ref = pub;

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->pub_lock);

    return retcode;
}

static DDS_ReturnCode_t RTI_MQTT_Client_remove_publication(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_Publication *pub)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i = 0, seq_len = 0, rm_i = 0;
    DDS_Boolean rm_pub = DDS_BOOLEAN_FALSE;
    struct RTI_MQTT_Publication **pub_ref = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_Client_remove_publication)

    RTI_MQTT_Mutex_assert(&self->pub_lock);

    seq_len = RTI_MQTT_PublicationPtrSeq_get_length(&self->publications);

    for (i = 0; i < seq_len; i++) {
        pub_ref = RTI_MQTT_PublicationPtrSeq_get_reference(
                &self->publications,
                i);
        if (pub == *pub_ref) {
            rm_pub = DDS_BOOLEAN_TRUE;
            rm_i = i;
            break;
        }
    }
    if (!rm_pub) {
        /* The publication doesn't belong to this Client */
        RTI_MQTT_LOG_CLIENT_UNKNOWN_PUBLICATION_DETECTED(self, pub)
        goto done;
    }
    for (i = rm_i; i + 1 < seq_len; i++) {
        *RTI_MQTT_PublicationPtrSeq_get_reference(&self->publications, i) =
                *RTI_MQTT_PublicationPtrSeq_get_reference(
                        &self->publications,
                        i + 1);
    }
    if (!RTI_MQTT_PublicationPtrSeq_set_length(
                &self->publications,
                seq_len - 1)) {
        RTI_MQTT_LOG_SET_SEQUENCE_LENGTH_FAILED(
                &self->publications,
                seq_len - 1)
        goto done;
    }
    if (!RTI_MQTT_PublicationPtrSeq_set_maximum(
                &self->publications,
                seq_len - 1)) {
        RTI_MQTT_LOG_SET_SEQUENCE_MAX_FAILED(&self->publications, seq_len - 1)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->pub_lock);

    return retcode;
}
