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

#if MQTT_CLIENT_API == MQTT_CLIENT_API_PAHO_C

    #define RTI_MQTT_LOG_ARGS "RTI::MQTT::Client::Paho"

void RTI_MQTT_ClientMqttApi_Paho_on_success(
        void *ctx,
        MQTTAsync_successData *response)
{
    struct RTI_MQTT_PendingRequest *req =
            (struct RTI_MQTT_PendingRequest *) ctx;
    RTI_MQTT_PendingRequest_handle_result(req, DDS_RETCODE_OK);
}

void RTI_MQTT_ClientMqttApi_Paho_on_failure(
        void *ctx,
        MQTTAsync_failureData *response)
{
    struct RTI_MQTT_PendingRequest *req =
            (struct RTI_MQTT_PendingRequest *) ctx;
    RTI_MQTT_PendingRequest_handle_result(req, DDS_RETCODE_ERROR);
}

DDS_ReturnCode_t
        RTI_MQTT_ClientMqttApi_Paho_create_client(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    char *client_addr = NULL;
    char *client_id = NULL;
    int client_persistence = MQTTCLIENT_PERSISTENCE_NONE;
    void *client_persistence_storage = NULL;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_create_client)

    RTI_MQTT_Mutex_assert(&self->mqtt_lock);

    client_addr =
            *DDS_StringSeq_get_reference(&self->data->config->server_uris, 0);
    if (client_addr == NULL || RTI_MQTT_String_length(client_addr) == 0) {
        RTI_MQTT_LOG_CLIENT_INVALID_CONFIG_DETECTED(
                self,
                "invalid server address")
        goto done;
    }

    client_id = self->data->config->id;
    if (client_id == NULL || RTI_MQTT_String_length(client_id) == 0) {
        RTI_MQTT_LOG_CLIENT_INVALID_CONFIG_DETECTED(
                self,
                "invalid client id specified")
        goto done;
    }

    switch (self->data->config->persistence_level) {
    case RTI_MQTT_PersistenceLevel_DURABLE:
        client_persistence = MQTTCLIENT_PERSISTENCE_DEFAULT;
        client_persistence_storage = self->data->config->persistence_storage;
        break;

    default:
        client_persistence = MQTTCLIENT_PERSISTENCE_NONE;
        break;
    }

    if (MQTTASYNC_SUCCESS
        != MQTTAsync_create(
                &self->client,
                client_addr,
                client_id,
                client_persistence,
                client_persistence_storage)) {
        RTI_MQTT_LOG_CLIENT_PAHO_C_CREATE_CLIENT_FAILED(self)
        goto done;
    }

    RTI_MQTT_LOG_4(
            "created MQTT client:",
            "addr=%s, id=%s, persistence=%d, storage=%s",
            client_addr,
            client_id,
            client_persistence,
            (const char *) client_persistence_storage)

    if (MQTTASYNC_SUCCESS
        != MQTTAsync_setCallbacks(
                self->client,
                self,
                RTI_MQTT_ClientMqttApi_Paho_on_connection_lost,
                RTI_MQTT_ClientMqttApi_Paho_on_message_arrived,
                NULL)) {
        RTI_MQTT_LOG_CLIENT_PAHO_C_SET_CALLBACKS_FAILED(self)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        if (self->client != NULL) {
            MQTTAsync_destroy(self->client);
            self->client = NULL;
        }
    }
    RTI_MQTT_Mutex_release(&self->mqtt_lock);
    return retcode;
}

DDS_ReturnCode_t
        RTI_MQTT_ClientMqttApi_Paho_delete_client(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_delete_client)

    RTI_MQTT_Mutex_assert(&self->mqtt_lock);

    if (self->client == NULL) {
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    RTI_MQTT_LOG_1("deleting MQTT client:", "client=%p", self->client)

    MQTTAsync_destroy(&self->client);
    self->client = NULL;

    retcode = DDS_RETCODE_OK;
done:
    RTI_MQTT_Mutex_release(&self->mqtt_lock);
    return retcode;
}

DDS_ReturnCode_t
        RTI_MQTT_ClientMqttApi_Paho_connect(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    #if RTI_MQTT_USE_SSL
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    #endif /* RTI_MQTT_USE_SSL */

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_connect)

    RTI_MQTT_Mutex_assert(&self->mqtt_lock);

    if (DDS_RETCODE_OK
        != RTI_MQTT_Time_to_seconds(
                &self->data->config->keep_alive_period,
                &conn_opts.keepAliveInterval)) {
        RTI_MQTT_TIME_TO_SECONDS_FAILED(&self->data->config->keep_alive_period)
        goto done;
    }

    conn_opts.cleansession = self->data->config->clean_session;
    conn_opts.maxInflight = self->data->config->max_unack_messages;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Time_to_seconds(
                &self->data->config->connect_timeout,
                &conn_opts.connectTimeout)) {
        RTI_MQTT_TIME_TO_SECONDS_FAILED(&self->data->config->connect_timeout)
        goto done;
    }

    conn_opts.serverURIs = DDS_StringSeq_get_contiguous_buffer(
            &self->data->config->server_uris);
    conn_opts.serverURIcount =
            DDS_StringSeq_get_length(&self->data->config->server_uris);

    switch (self->data->config->protocol_version) {
    case RTI_MQTT_MqttProtocolVersion_MQTT_DEFAULT:
        conn_opts.MQTTVersion = MQTTVERSION_DEFAULT;
        break;
    case RTI_MQTT_MqttProtocolVersion_MQTT_3_1_1:
        conn_opts.MQTTVersion = MQTTVERSION_3_1_1;
        break;
    case RTI_MQTT_MqttProtocolVersion_MQTT_3_1:
        conn_opts.MQTTVersion = MQTTVERSION_3_1;
        break;
    default:
        RTI_MQTT_LOG_CLIENT_UNSUPPORTED_PROTOCOL_VERSION_DETECTED(
                self,
                self->data->config->protocol_version)
        goto done;
        break;
    }

    conn_opts.automaticReconnect = DDS_BOOLEAN_FALSE;

    conn_opts.username = self->data->config->username;

    if (self->data->config->password != NULL) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_DDS_OctetSeq_to_string(
                    self->data->config->password,
                    (char **) &conn_opts.password)) {
            RTI_MQTT_OCTET_SEQ_TO_STRING_FAILED(self->data->config->password)
            goto done;
        }
    }

    #if RTI_MQTT_USE_SSL
    if (self->data->config->ssl_tls_config != NULL) {
        if (self->data->config->ssl_tls_config->ca != NULL
            && RTI_MQTT_String_length(self->data->config->ssl_tls_config->ca)
                    > 0) {
            ssl_opts.trustStore = self->data->config->ssl_tls_config->ca;
        }
        if (self->data->config->ssl_tls_config->private_key != NULL
            && RTI_MQTT_String_length(
                       self->data->config->ssl_tls_config->private_key)
                    > 0) {
            ssl_opts.privateKey =
                    self->data->config->ssl_tls_config->private_key;
        }
        if (self->data->config->ssl_tls_config->private_key_password != NULL
            && RTI_MQTT_String_length(
                       self->data->config->ssl_tls_config->private_key_password)
                    > 0) {
            ssl_opts.privateKeyPassword =
                    self->data->config->ssl_tls_config->private_key_password;
        }
        if (self->data->config->ssl_tls_config->identity != NULL
            && RTI_MQTT_String_length(
                       self->data->config->ssl_tls_config->identity)
                    > 0) {
            ssl_opts.keyStore = self->data->config->ssl_tls_config->identity;
        }
        if (self->data->config->ssl_tls_config->cypher_suites != NULL
            && RTI_MQTT_String_length(
                       self->data->config->ssl_tls_config->cypher_suites)
                    > 0) {
            ssl_opts.enabledCipherSuites =
                    self->data->config->ssl_tls_config->cypher_suites;
        }

        switch (self->data->config->ssl_tls_config->protocol_version) {
        case RTI_MQTT_SslTlsProtocolVersion_TLS_DEFAULT:
            ssl_opts.sslVersion = MQTT_SSL_VERSION_DEFAULT;
            break;
        case RTI_MQTT_SslTlsProtocolVersion_TLS_1_0:
            ssl_opts.sslVersion = MQTT_SSL_VERSION_TLS_1_0;
            break;
        case RTI_MQTT_SslTlsProtocolVersion_TLS_1_1:
            ssl_opts.sslVersion = MQTT_SSL_VERSION_TLS_1_1;
            break;
        case RTI_MQTT_SslTlsProtocolVersion_TLS_1_2:
            ssl_opts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;
            break;
        default:
            /* TODO Log error */
            goto done;
        }

        ssl_opts.enableServerCertAuth =
                self->data->config->ssl_tls_config->verify_server_certificate;

        conn_opts.ssl = &ssl_opts;
    }
    #endif

    conn_opts.onSuccess = RTI_MQTT_ClientMqttApi_Paho_on_success;
    conn_opts.onFailure = RTI_MQTT_ClientMqttApi_Paho_on_failure;
    conn_opts.context = self->req_connect;

    /* Print out MQTTAsync configuration */
    RTI_MQTT_LOG("MQTTAsync configuration:")
    {
        int i = 0;
        RTI_MQTT_LOG_1("  - server URIs:", "%d", conn_opts.cleansession)
        for (i = 0; i < conn_opts.serverURIcount; i++) {
            const char *server_uri = conn_opts.serverURIs[i];
            RTI_MQTT_LOG_2("    - ", "%u: %s", i, server_uri)
        }
    }
    RTI_MQTT_LOG_1("  - clean session:", "%d", conn_opts.cleansession)
    RTI_MQTT_LOG_1("  - protocol version:", "%d", conn_opts.MQTTVersion)
    RTI_MQTT_LOG_1("  - keep alive period:", "%d", conn_opts.keepAliveInterval)
    RTI_MQTT_LOG_1("  - connection timeout:", "%d", conn_opts.connectTimeout)
    RTI_MQTT_LOG_1("  - username:", "%s", conn_opts.username)
    RTI_MQTT_LOG_1("  - password:", "%s", conn_opts.password)
    if (conn_opts.ssl != NULL) {
        RTI_MQTT_LOG_1("  - CA:", "%s", conn_opts.ssl->trustStore)
        RTI_MQTT_LOG_1("  - ID:", "%s", conn_opts.ssl->keyStore)
        RTI_MQTT_LOG_1("  - Key:", "%s", conn_opts.ssl->privateKey)
    }
    if (MQTTASYNC_SUCCESS != MQTTAsync_connect(self->client, &conn_opts)) {
        RTI_MQTT_LOG_CLIENT_PAHO_C_CONNECT_FAILED(self)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->mqtt_lock);

    if (conn_opts.password != NULL) {
        RTI_MQTT_Heap_free((char *) conn_opts.password);
        conn_opts.password = NULL;
    }
    return retcode;
}

DDS_ReturnCode_t
        RTI_MQTT_ClientMqttApi_Paho_disconnect(struct RTI_MQTT_Client *self)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_disconnect)

    RTI_MQTT_Mutex_assert(&self->mqtt_lock);

    opts.onSuccess = RTI_MQTT_ClientMqttApi_Paho_on_success;
    opts.onFailure = RTI_MQTT_ClientMqttApi_Paho_on_failure;
    opts.context = self->req_disconnect;

    if (MQTTASYNC_SUCCESS != MQTTAsync_disconnect(self->client, &opts)) {
        RTI_MQTT_LOG_CLIENT_PAHO_C_DISCONNECT_FAILED(self)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release(&self->mqtt_lock);

    return retcode;
}

DDS_ReturnCode_t RTI_MQTT_ClientMqttApi_Paho_submit_subscriptions(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_SubscriptionRequestContext *req_ctx =
            (struct RTI_MQTT_SubscriptionRequestContext *) req->context;
    int *sub_qoss = NULL;
    char **sub_topics = NULL;
    DDS_UnsignedLong seq_len = 0, i = 0;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    DDS_Boolean locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_submit_subscriptions)

    seq_len = RTI_MQTT_SubscriptionParamsSeq_get_length(&req_ctx->params);

    sub_qoss = (int *) RTI_MQTT_Heap_allocate(sizeof(int) * seq_len);
    if (sub_qoss == NULL) {
        /* TODO Log error */
        goto done;
    }

    sub_topics = (char **) RTI_MQTT_Heap_allocate(sizeof(char *) * seq_len);
    if (sub_topics == NULL) {
        /* TODO Log error */
        goto done;
    }

    for (i = 0; i < seq_len; i++) {
        RTI_MQTT_SubscriptionParams *p =
                RTI_MQTT_SubscriptionParamsSeq_get_reference(
                        &req_ctx->params,
                        i);
        int *qos_ref = &(sub_qoss[i]);
        char **topic_ref = &(sub_topics[i]);

        if (DDS_RETCODE_OK
            != RTI_MQTT_QosLevel_to_mqtt_qos(p->max_qos, qos_ref)) {
            /* TODO Log error */
            goto done;
        }
        *topic_ref = p->topic;
    }
    opts.onSuccess = RTI_MQTT_ClientMqttApi_Paho_on_success;
    opts.onFailure = RTI_MQTT_ClientMqttApi_Paho_on_failure;
    opts.context = req;

    #if RTI_MQTT_USE_TRACE
    RTI_MQTT_TRACE_2(
            "submit SUBSCRIPTIONS with Paho:",
            "client=%p, subs=%d",
            self->client,
            seq_len)
    for (i = 0; i < seq_len; i++) {
        RTI_MQTT_TRACE_2("  -", "%s [%d]", sub_topics[i], sub_qoss[i])
    }
    #endif /* RTI_MQTT_USE_TRACE */

    RTI_MQTT_Mutex_assert_w_state(&self->mqtt_lock, &locked);
    if (MQTTASYNC_SUCCESS
        != MQTTAsync_subscribeMany(
                self->client,
                seq_len,
                sub_topics,
                sub_qoss,
                &opts)) {
        RTI_MQTT_LOG_CLIENT_PAHO_C_SUBSCRIBE_FAILED(self)
        goto done;
    }
    RTI_MQTT_Mutex_release_w_state(&self->mqtt_lock, &locked);

    retcode = DDS_RETCODE_OK;

done:

    RTI_MQTT_Mutex_release_from_state(&self->mqtt_lock, &locked);

    if (sub_qoss != NULL) {
        RTI_MQTT_Heap_free(sub_qoss);
    }
    if (sub_topics != NULL) {
        RTI_MQTT_Heap_free(sub_topics);
    }

    return retcode;
}

DDS_ReturnCode_t RTI_MQTT_ClientMqttApi_Paho_cancel_subscriptions(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_MQTT_SubscriptionRequestContext *req_ctx =
            (struct RTI_MQTT_SubscriptionRequestContext *) req->context;
    char **sub_topics = NULL;
    DDS_UnsignedLong seq_len = 0, i = 0;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    DDS_Boolean locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_cancel_subscriptions)

    seq_len = RTI_MQTT_SubscriptionParamsSeq_get_length(&req_ctx->params);

    sub_topics = (char **) RTI_MQTT_Heap_allocate(sizeof(char *) * seq_len);
    if (sub_topics == NULL) {
        /* TODO Log error */
        goto done;
    }

    for (i = 0; i < seq_len; i++) {
        RTI_MQTT_SubscriptionParams *p =
                RTI_MQTT_SubscriptionParamsSeq_get_reference(
                        &req_ctx->params,
                        i);
        char **topic_ref = &(sub_topics[i]);
        *topic_ref = p->topic;
    }
    opts.onSuccess = RTI_MQTT_ClientMqttApi_Paho_on_success;
    opts.onFailure = RTI_MQTT_ClientMqttApi_Paho_on_failure;
    opts.context = req;

    #if RTI_MQTT_USE_TRACE
    RTI_MQTT_TRACE_2(
            "cancel SUBSCRIPTIONS with Paho:",
            "client=%p, subs=%d",
            self->client,
            seq_len)
    for (i = 0; i < seq_len; i++) {
        RTI_MQTT_TRACE_1("  -", "%s", sub_topics[i])
    }
    #endif /* RTI_MQTT_USE_TRACE */

    RTI_MQTT_Mutex_assert_w_state(&self->mqtt_lock, &locked);

    if (MQTTASYNC_SUCCESS
        != MQTTAsync_unsubscribeMany(
                self->client,
                seq_len,
                sub_topics,
                &opts)) {
        RTI_MQTT_LOG_CLIENT_PAHO_C_SUBSCRIBE_FAILED(self)
        goto done;
    }
    RTI_MQTT_Mutex_release_w_state(&self->mqtt_lock, &locked);

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release_from_state(&self->mqtt_lock, &locked);
    if (sub_topics != NULL) {
        RTI_MQTT_Heap_free(sub_topics);
    }
    return retcode;
}

DDS_ReturnCode_t RTI_MQTT_ClientMqttApi_Paho_write_message(
        struct RTI_MQTT_Client *self,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        const char *topic,
        RTI_MQTT_WriteParams *params,
        struct RTI_MQTT_PendingRequest *req)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    MQTTAsync_message async_msg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    DDS_Boolean locked = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_write_message)

    async_msg.payload = (char *) buffer;
    async_msg.payloadlen = buffer_len;

    if (DDS_RETCODE_OK
        != RTI_MQTT_QosLevel_to_mqtt_qos(params->qos_level, &async_msg.qos)) {
        RTI_MQTT_QOS_LEVEL_TO_MQTT_FAILED(params->qos_level)
        goto done;
    }

    async_msg.retained = params->retained;

    opts.onSuccess = RTI_MQTT_ClientMqttApi_Paho_on_success;
    opts.onFailure = RTI_MQTT_ClientMqttApi_Paho_on_failure;
    opts.context = req;

    RTI_MQTT_Mutex_assert_w_state(&self->mqtt_lock, &locked);

    if (MQTTASYNC_SUCCESS
        != MQTTAsync_sendMessage(self->client, topic, &async_msg, &opts)) {
        RTI_MQTT_LOG_CLIENT_PAHO_C_SEND_FAILED(self)
        goto done;
    }

    RTI_MQTT_Mutex_release_w_state(&self->mqtt_lock, &locked);

    retcode = DDS_RETCODE_OK;

done:
    RTI_MQTT_Mutex_release_from_state(&self->mqtt_lock, &locked);

    if (retcode != DDS_RETCODE_OK) {
        /* handle failure */
    }
    return retcode;
}

void *RTI_MQTT_ClientMqttApi_Paho_connection_lost_thread(void *arg)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE, locked = DDS_BOOLEAN_FALSE;
    struct RTI_MQTT_Client *self = (struct RTI_MQTT_Client *) arg;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_connection_lost_thread)

    RTI_MQTT_LOG_1("restoring state from connection LOST", "client=%p", self)

    if (DDS_RETCODE_OK != RTI_MQTT_Client_on_connection_lost(self)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:
    RTI_MQTT_Mutex_release_from_state(&self->mqtt_lock, &locked);

    if (!retval) {
        /* TODO Log error */
    }

    return NULL;
}

void RTI_MQTT_ClientMqttApi_Paho_on_connection_lost(void *ctx, char *cause)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE, locked = DDS_BOOLEAN_FALSE;
    struct RTI_MQTT_Client *self = (struct RTI_MQTT_Client *) ctx;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_on_connection_lost)

    RTI_MQTT_ERROR_1("connection LOST", "cause=%s", cause)

    if (DDS_RETCODE_OK
        != RTI_MQTT_Thread_spawn(
                RTI_MQTT_ClientMqttApi_Paho_connection_lost_thread,
                self,
                NULL)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval) {
        /* TODO Log error */
    }
}

int RTI_MQTT_ClientMqttApi_Paho_on_message_arrived(
        void *ctx,
        char *topic_name,
        int topic_len,
        MQTTAsync_message *message)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    RTI_MQTT_MessageInfo msg_info;
    struct RTI_MQTT_Client *self = (struct RTI_MQTT_Client *) ctx;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientMqttApi_Paho_on_message_arrived)

    RTI_MQTT_TRACE_1("message RECEIVED:", "topic=%s", topic_name)

    if (DDS_RETCODE_OK
        != RTI_MQTT_QosLevel_from_mqtt_qos(message->qos, &msg_info.qos_level)) {
        /* TODO Log error */
        goto done;
    }
    msg_info.retained =
            (message->retained) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
    msg_info.duplicate = (message->dup) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_on_message_arrived(
                self,
                topic_name,
                message->payload,
                message->payloadlen,
                &msg_info)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topic_name);

    return retval;
}


#endif /* MQTT_CLIENT_API */
