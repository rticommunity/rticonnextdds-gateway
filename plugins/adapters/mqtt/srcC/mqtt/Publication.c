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

#include "Publication.h"
#include "Client.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::Publication"


static DDS_ReturnCode_t RTI_MQTT_Publication_initialize(
        struct RTI_MQTT_Publication *self,
        struct RTI_MQTT_Client *client,
        RTI_MQTT_PublicationConfig *config);

static DDS_ReturnCode_t
        RTI_MQTT_Publication_finalize(struct RTI_MQTT_Publication *self);

static DDS_Boolean RTI_MQTT_Publication_is_configuration_valid(
        struct RTI_MQTT_Publication *pub,
        RTI_MQTT_QosLevel qos,
        const char *topic);

static DDS_ReturnCode_t RTI_MQTT_Publication_store_topic(
        struct RTI_MQTT_Publication *self,
        const char *topic);

DDS_ReturnCode_t RTI_MQTT_Publication_new(
        struct RTI_MQTT_Client *client,
        RTI_MQTT_PublicationConfig *config,
        struct RTI_MQTT_Publication **pub_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Publication *pub = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_new)

    *pub_out = NULL;

    pub = (struct RTI_MQTT_Publication *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_MQTT_Publication));
    if (pub == NULL) {
        RTI_MQTT_HEAP_ALLOCATE_FAILED(sizeof(struct RTI_MQTT_Publication))
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Publication_initialize(pub, client, config)) {
        RTI_MQTT_LOG_PUBLICATION_INIT_FAILED(pub, client, config)
        goto done;
    }

    *pub_out = pub;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (pub != NULL) {
            RTI_MQTT_Heap_free(pub);
        }
    }

    return retval;
}

void RTI_MQTT_Publication_delete(struct RTI_MQTT_Publication *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_delete)

    if (self == NULL) {
        return;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Publication_finalize(self)) {
        RTI_MQTT_LOG_PUBLICATION_FINALIZE_FAILED(self)
    }

    RTI_MQTT_Heap_free(self);
}

DDS_ReturnCode_t RTI_MQTT_Publication_write(
        struct RTI_MQTT_Publication *self,
        DDS_DynamicData *message)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_WriteParams params = RTI_MQTT_WriteParams_INITIALIZER;
    char *payload_buffer = NULL, *topic = NULL;
    DDS_UnsignedLong topic_len = 0, payload_len = 0;
    struct DDS_OctetSeq payload = DDS_SEQUENCE_INITIALIZER;
    DDS_Boolean use_message_info = DDS_BOOLEAN_FALSE,
                locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_write)

    /* TODO RM Mutex: only used to protect self->data->config */
    RTI_MQTT_Mutex_assert_w_state(&self->client->pub_lock, &locked);

    use_message_info = self->data->config->use_message_info;

    if (!use_message_info) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Publication_store_topic(
                    self,
                    self->data->config->topic)) {
            /* TODO Log error */
            goto done;
        }
        params.retained = self->data->config->retained;
        params.qos_level = self->data->config->qos;
    }

    /* TODO RM Mutex: only used to protect self->data->config */
    RTI_MQTT_Mutex_release_w_state(&self->client->pub_lock, &locked);

    if (use_message_info) {
        if (!DDS_DynamicData_member_exists(
                    message,
                    "info",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_MQTT_LOG_PUBLICATION_WRITE_MESSAGE_INFO_NOT_FOUND(self, message)
            goto done;
        }
        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_long(
                    message,
                    (DDS_Long *) &params.qos_level,
                    "info.qos_level",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            /* TODO Log error */
            goto done;
        }
        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_boolean(
                    message,
                    &params.retained,
                    "info.retained",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            /* TODO Log error */
            goto done;
        }
        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_string(
                    message,
                    &topic,
                    &topic_len,
                    "topic",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            /* TODO Log error */
            goto done;
        }
        DDS_String_replace(&self->req_ctx.topic, topic);
        if (self->req_ctx.topic == NULL) {
            /* TODO Log error */
            goto done;
        }
        self->req_ctx.topic_len = topic_len;
    }

    if (DDS_RETCODE_OK
        != DDS_DynamicData_get_octet_seq(
                message,
                &payload,
                "payload.data",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        /* TODO Log error */
        goto done;
    }

    if (!DDS_OctetSeq_copy(&self->req_ctx.payload, &payload)) {
        /* TODO Log error */
        goto done;
    }

    payload_buffer = DDS_OctetSeq_get_contiguous_buffer(&self->req_ctx.payload);
    payload_len = DDS_OctetSeq_get_length(&self->req_ctx.payload);

    if (DDS_RETCODE_OK
        != RTI_MQTT_Publication_write_w_params(
                self,
                payload_buffer,
                payload_len,
                self->req_ctx.topic,
                &params)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    /* TODO RM Mutex: only used to protect self->data->config */
    RTI_MQTT_Mutex_release_from_state(&self->client->pub_lock, &locked);

    if (topic != NULL) {
        DDS_String_free(topic);
    }
    if (!DDS_OctetSeq_finalize(&payload)) {
        /* TODO Log error */
    }

    return retval;
}


DDS_ReturnCode_t RTI_MQTT_Publication_on_write_result(
        struct RTI_MQTT_Publication *self,
        DDS_ReturnCode_t result)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_on_write_result)

    switch (self->req_ctx.last_write_qos) {
    case RTI_MQTT_QosLevel_ZERO:
    case RTI_MQTT_QosLevel_ONE:
        self->data->message_status->pending_count -= 1;

        if (DDS_RETCODE_OK == result) {
            self->data->message_status->ok_count += 1;
        } else {
            self->data->message_status->error_count += 1;
        }
        break;

    case RTI_MQTT_QosLevel_TWO:
        if (DDS_RETCODE_OK != result) {
            self->data->message_status->pending_count -= 1;
            self->data->message_status->error_count += 1;
        }

        break;

    default:
        RTI_MQTT_INTERNAL_ERROR(
                "unexpected QoS in RTI_MQTT_Publication_on_write_result")
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Publication_on_write_delivery_result(
        struct RTI_MQTT_Publication *self,
        DDS_ReturnCode_t result)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_on_write_delivery_result)

    /* This callback should be called only for messages with Qos 2*/
    if (self->req_ctx.last_write_qos != RTI_MQTT_QosLevel_TWO) {
        RTI_MQTT_LOG_PUBLICATION_UNEXPECTED_DELIVERY_RESULT(
                self,
                self->req_ctx.last_write_qos)
        goto done;
    }

    /* The on_write_result callback should have been called before this one */
    if (self->data->message_status->pending_count == 0) {
        RTI_MQTT_LOG_PUBLICATION_NO_PENDING_MESSAGES_FOUND(self)
        goto done;
    }

    self->data->message_status->pending_count -= 1;


    if (result == DDS_RETCODE_OK) {
        self->data->message_status->ok_count += 1;
    } else {
        self->data->message_status->error_count += 1;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

static DDS_Boolean RTI_MQTT_Publication_is_configuration_valid(
        struct RTI_MQTT_Publication *pub,
        RTI_MQTT_QosLevel qos,
        const char *topic)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong topic_len = 0;

    if (!RTI_MQTT_QosLevel_is_valid(qos)) {
        RTI_MQTT_LOG_PUBLICATION_INVALID_QOS_DETECTED(pub, qos)
        goto done;
    }
    if (topic == NULL) {
        RTI_MQTT_LOG_PUBLICATION_INVALID_TOPIC_DETECTED(pub, topic)
        goto done;
    }
    topic_len = RTI_MQTT_String_length(topic);
    if (topic_len == 0 || topic_len > MQTT_TOPIC_NAME_MAX_LEN) {
        RTI_MQTT_LOG_PUBLICATION_INVALID_TOPIC_DETECTED(pub, topic)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:
    return retval;
}

static DDS_ReturnCode_t RTI_MQTT_Publication_initialize(
        struct RTI_MQTT_Publication *self,
        struct RTI_MQTT_Client *client,
        RTI_MQTT_PublicationConfig *config)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Publication def_self = RTI_MQTT_Publication_INITIALIZER;

    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_initialize)

    *self = def_self;

    self->client = client;

    if (!config->use_message_info
        && !RTI_MQTT_Publication_is_configuration_valid(
                self,
                config->qos,
                config->topic)) {
        RTI_MQTT_LOG_PUBLICATION_INVALID_INIT_CONFIG_DETECTED(self)
        goto done;
    }

    if (!DDS_OctetSeq_initialize(&self->req_ctx.payload)) {
        RTI_MQTT_LOG_INITIALIZE_SEQUENCE_FAILED(&self->req_ctx.payload)
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_PublicationStatus_new(DDS_BOOLEAN_TRUE, &self->data)) {
        RTI_MQTT_LOG_CREATE_DATA_FAILED("RTI_MQTT_PublicationStatus")
        goto done;
    }

    if (!RTI_MQTT_PublicationConfig_copy(self->data->config, config)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_MQTT_PublicationConfig",
                config,
                self->data->config)
        goto done;
    }

    if (self->data->config->use_message_info
        && DDS_RETCODE_OK
                != RTI_MQTT_Publication_store_topic(
                        self,
                        self->data->config->topic)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        if (DDS_RETCODE_OK != RTI_MQTT_Publication_finalize(self)) {
            /* TODO Log error */
        }
    }
    return retval;
}

static DDS_ReturnCode_t
        RTI_MQTT_Publication_finalize(struct RTI_MQTT_Publication *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_MQTT_Publication def_self = RTI_MQTT_Publication_INITIALIZER;

    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_finalize)

    if (self->data != NULL) {
        RTI_MQTT_PublicationStatusTypeSupport_delete_data(self->data);
        self->data = NULL;
    }

    if (!DDS_OctetSeq_finalize(&self->req_ctx.payload)) {
        RTI_MQTT_LOG_FINALIZE_SEQUENCE_FAILED(&self->req_ctx.payload)
    }

    if (self->req_ctx.topic != NULL) {
        DDS_String_free(self->req_ctx.topic);
        self->req_ctx.topic = NULL;
        self->req_ctx.topic_len = 0;
    }

    *self = def_self;

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Publication_write_w_params(
        struct RTI_MQTT_Publication *self,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        const char *topic,
        RTI_MQTT_WriteParams *params)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_write_w_params)

    if (!RTI_MQTT_Publication_is_configuration_valid(
                self,
                params->qos_level,
                topic)) {
        RTI_MQTT_LOG_PUBLICATION_INVALID_WRITE_CONFIG_DETECTED(self)
        goto done;
    }

    self->req_ctx.last_write_qos = params->qos_level;
    self->data->message_status->pending_count += 1;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_write_message(
                self->client,
                self,
                buffer,
                buffer_len,
                topic,
                params)) {
        RTI_MQTT_LOG_PUBLICATION_WRITE_MESSAGE_FAILED(
                self,
                topic,
                params->qos_level,
                params->retained,
                buffer)
        self->data->message_status->pending_count -= 1;
        goto done;
    }

    self->data->message_status->sent_count += 1;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_wait_for_write_result(self->client, self)) {
        RTI_MQTT_LOG_CLIENT_WAIT_FOR_WRITE_RESULTS_FAILED(self->client, self)
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:

    return retval;
}

static DDS_ReturnCode_t RTI_MQTT_Publication_store_topic(
        struct RTI_MQTT_Publication *self,
        const char *topic)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong topic_len = 0;

    RTI_MQTT_LOG_FN(RTI_MQTT_Publication_store_topic)

    if (self->req_ctx.topic == NULL) {
        self->req_ctx.topic = DDS_String_dup(topic);
        if (self->req_ctx.topic == NULL) {
            /* TODO Log error */
            goto done;
        }
        self->req_ctx.topic_len = RTI_MQTT_String_length(self->req_ctx.topic);
    } else {
        topic_len = RTI_MQTT_String_length(topic);

        if (topic_len <= self->req_ctx.topic_len) {
            RTI_MQTT_Memory_copy(
                    self->req_ctx.topic,
                    topic,
                    sizeof(char) * topic_len);
            self->req_ctx.topic[topic_len] = '\0';
            if (self->req_ctx.topic == NULL) {
                /* TODO Log error */
                goto done;
            }
        } else {
            DDS_String_replace(&self->req_ctx.topic, topic);
            if (self->req_ctx.topic == NULL) {
                /* TODO Log error */
                goto done;
            }
            self->req_ctx.topic_len =
                    RTI_MQTT_String_length(self->req_ctx.topic);
        }
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

RTIBool RTI_MQTT_PublicationPtr_initialize_w_params(
        struct RTI_MQTT_Publication **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_MQTT_PublicationPtr_finalize_w_params(
        struct RTI_MQTT_Publication **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_MQTT_PublicationPtr_copy(
        struct RTI_MQTT_Publication **dst,
        const struct RTI_MQTT_Publication **src)
{
    *dst = (struct RTI_MQTT_Publication *) *src;
    return RTI_TRUE;
}

#define T struct RTI_MQTT_Publication *
#define TSeq RTI_MQTT_PublicationPtrSeq
#define T_initialize_w_params RTI_MQTT_PublicationPtr_initialize_w_params
#define T_finalize_w_params RTI_MQTT_PublicationPtr_finalize_w_params
#define T_copy RTI_MQTT_PublicationPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T
