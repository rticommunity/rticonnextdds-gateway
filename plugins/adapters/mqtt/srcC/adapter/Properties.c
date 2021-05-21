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

#include "Properties.h"
#include "Infrastructure.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::RS::Infrastructure"

const RTI_RS_MQTT_BrokerConnectionConfig
        RTI_RS_MQTT_BrokerConnectionConfig_DEFAULT =
                RTI_RS_MQTT_BrokerConnectionConfig_INITIALIZER;
const RTI_RS_MQTT_MessageReaderConfig RTI_RS_MQTT_MessageReaderConfig_DEFAULT =
        RTI_RS_MQTT_MessageReaderConfig_INITIALIZER;
const RTI_RS_MQTT_MessageWriterConfig RTI_RS_MQTT_MessageWriterConfig_DEFAULT =
        RTI_RS_MQTT_MessageWriterConfig_INITIALIZER;

#define RTI_MQTT_URI_PREFIX_SSL "ssl"

static DDS_ReturnCode_t RTI_MQTT_MqttProtocolVersion_from_string(
        const char *str,
        RTI_MQTT_MqttProtocolVersion *proto_out)
{
    if (RTI_MQTT_String_compare(str, "default") == 0
        || RTI_MQTT_String_compare(str, "MQTT_DEFAULT") == 0) {
        *proto_out = RTI_MQTT_MqttProtocolVersion_MQTT_DEFAULT;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "3.1") == 0
            || RTI_MQTT_String_compare(str, "MQTT_3_1") == 0) {
        *proto_out = RTI_MQTT_MqttProtocolVersion_MQTT_3_1;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "3.1.1") == 0
            || RTI_MQTT_String_compare(str, "MQTT_3_1_1") == 0) {
        *proto_out = RTI_MQTT_MqttProtocolVersion_MQTT_3_1_1;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "5") == 0
            || RTI_MQTT_String_compare(str, "MQTT_5") == 0) {
        *proto_out = RTI_MQTT_MqttProtocolVersion_MQTT_5;
        return DDS_RETCODE_OK;
    }

    return DDS_RETCODE_ERROR;
}


static DDS_ReturnCode_t RTI_MQTT_SslTlsProtocolVersion_from_string(
        const char *str,
        RTI_MQTT_SslTlsProtocolVersion *proto_out)
{
    if (RTI_MQTT_String_compare(str, "default") == 0
        || RTI_MQTT_String_compare(str, "TLS_DEFAULT") == 0) {
        *proto_out = RTI_MQTT_SslTlsProtocolVersion_TLS_DEFAULT;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "1.0") == 0
            || RTI_MQTT_String_compare(str, "TLS_1_0") == 0) {
        *proto_out = RTI_MQTT_SslTlsProtocolVersion_TLS_1_0;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "1.1") == 0
            || RTI_MQTT_String_compare(str, "TLS_1_1") == 0) {
        *proto_out = RTI_MQTT_SslTlsProtocolVersion_TLS_1_1;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "1.2") == 0
            || RTI_MQTT_String_compare(str, "TLS_1_2") == 0) {
        *proto_out = RTI_MQTT_SslTlsProtocolVersion_TLS_1_2;
        return DDS_RETCODE_OK;
    }

    return DDS_RETCODE_ERROR;
}


static DDS_ReturnCode_t RTI_MQTT_QosLevel_from_string(
        const char *str,
        RTI_MQTT_QosLevel *qos_out)
{
    if (RTI_MQTT_String_compare(str, "zero") == 0
        || RTI_MQTT_String_compare(str, "ZERO") == 0
        || RTI_MQTT_String_compare(str, "Zero") == 0
        || RTI_MQTT_String_compare(str, "0") == 0) {
        *qos_out = RTI_MQTT_QosLevel_ZERO;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "one") == 0
            || RTI_MQTT_String_compare(str, "ONE") == 0
            || RTI_MQTT_String_compare(str, "One") == 0
            || RTI_MQTT_String_compare(str, "1") == 0) {
        *qos_out = RTI_MQTT_QosLevel_ONE;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "two") == 0
            || RTI_MQTT_String_compare(str, "TWO") == 0
            || RTI_MQTT_String_compare(str, "Two") == 0
            || RTI_MQTT_String_compare(str, "2") == 0) {
        *qos_out = RTI_MQTT_QosLevel_TWO;
        return DDS_RETCODE_OK;
    }

    return DDS_RETCODE_ERROR;
}

static DDS_ReturnCode_t RTI_MQTT_PersistenceLevel_from_string(
        const char *str,
        RTI_MQTT_PersistenceLevel *level_out)
{
    if (RTI_MQTT_String_compare(str, "none") == 0
        || RTI_MQTT_String_compare(str, "NONE") == 0
        || RTI_MQTT_String_compare(str, "n") == 0
        || RTI_MQTT_String_compare(str, "N") == 0
        || RTI_MQTT_String_compare(str, "no") == 0
        || RTI_MQTT_String_compare(str, "NO") == 0
        || RTI_MQTT_String_compare(str, "No") == 0
        || RTI_MQTT_String_compare(str, "0") == 0) {
        *level_out = RTI_MQTT_PersistenceLevel_NONE;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "durable") == 0
            || RTI_MQTT_String_compare(str, "DURABLE") == 0
            || RTI_MQTT_String_compare(str, "Durable") == 0
            || RTI_MQTT_String_compare(str, "y") == 0
            || RTI_MQTT_String_compare(str, "Y") == 0
            || RTI_MQTT_String_compare(str, "yes") == 0
            || RTI_MQTT_String_compare(str, "YES") == 0
            || RTI_MQTT_String_compare(str, "Yes") == 0
            || RTI_MQTT_String_compare(str, "1") == 0) {
        *level_out = RTI_MQTT_PersistenceLevel_DURABLE;
        return DDS_RETCODE_OK;
    }

    return DDS_RETCODE_ERROR;
}

static DDS_ReturnCode_t
        DDS_StringSeq_from_string(const char *str, struct DDS_StringSeq *seq)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    size_t str_len = 0, cur_str_len = 0, consumed_len = 0,
                     seq_len = 0, orig_seq_len = 0;
    const char *str_ptr = str, *cur_str = NULL;
    char cur_ch = '\0', *seq_val = NULL;

    seq_len = DDS_StringSeq_get_length(seq);
    orig_seq_len = seq_len;

    str_len = RTI_MQTT_String_length(str);
    if (str_len == 0) {
        if (DDS_RETCODE_OK != DDS_StringSeq_set_length(seq, 0)) {
            /* TODO Log error */
            goto done;
        }
        retval = DDS_RETCODE_OK;
        goto done;
    }

    consumed_len = 0;
    while (consumed_len < str_len) {
        cur_str = str + consumed_len;
        cur_str_len = 0;

        cur_ch = cur_str[cur_str_len];
        while (cur_ch != ';' && cur_ch != '\0') {
            cur_str_len += 1;
            cur_ch = cur_str[cur_str_len];
        }

        if (!DDS_StringSeq_ensure_length(
                seq,
                (DDS_Long) seq_len + 1,
                (DDS_Long) seq_len + 1)) {
            /* TODO Log error */
            goto done;
        }

        seq_val = DDS_String_alloc(sizeof(char) * (cur_str_len));
        if (seq_val == NULL) {
            /* TODO Log error */
            goto done;
        }

        RTIOsapiMemory_copy(seq_val, cur_str, sizeof(char) * cur_str_len);
        seq_val[cur_str_len] = '\0';

        *DDS_StringSeq_get_reference(seq, (DDS_Long) seq_len) = seq_val;

        consumed_len += cur_str_len + 1;
        seq_len += 1;
    }

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (DDS_RETCODE_OK !=
                DDS_StringSeq_set_length(seq, (DDS_Long) orig_seq_len)) {
            /* TODO Log error */
        }
    }

    return retval;
}


static DDS_ReturnCode_t
        DDS_OctetSeq_from_string(const char *str, struct DDS_OctetSeq *seq)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    char *seq_val = NULL;
    size_t str_len = 0, orig_seq_len = 0;


    str_len = RTI_MQTT_String_length(str);
    if (str_len == 0) {
        if (!DDS_OctetSeq_set_length(seq, 0)) {
            RTI_MQTT_ERROR("failed to set octet sequence length")
            goto done;
        }
        retval = DDS_RETCODE_OK;
        goto done;
    }
    orig_seq_len = DDS_OctetSeq_get_length(seq);

    if (!DDS_OctetSeq_ensure_length(
            seq,
            (DDS_Long) str_len,
            (DDS_Long) str_len)) {
        RTI_MQTT_ERROR_1(
                "failed to ensure octet sequence length:",
                "%u",
                (DDS_UnsignedLong) str_len + 1)
        goto done;
    }

    seq_val = (char *) DDS_OctetSeq_get_contiguous_buffer(seq);

    RTI_MQTT_Memory_copy(seq_val, str, sizeof(char) * str_len);

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (!DDS_OctetSeq_set_length(seq, (DDS_Long) orig_seq_len)) {
            RTI_MQTT_ERROR_1(
                    "failed to reset octet sequence length:",
                    "%u",
                    (DDS_UnsignedLong) orig_seq_len)
        }
    }

    return retval;
}

DDS_ReturnCode_t DDS_Boolean_from_string(const char *str, DDS_Boolean *bool_out)
{
    if (RTI_MQTT_String_compare(str, "t") == 0
        || RTI_MQTT_String_compare(str, "T") == 0
        || RTI_MQTT_String_compare(str, "true") == 0
        || RTI_MQTT_String_compare(str, "TRUE") == 0
        || RTI_MQTT_String_compare(str, "True") == 0
        || RTI_MQTT_String_compare(str, "y") == 0
        || RTI_MQTT_String_compare(str, "Y") == 0
        || RTI_MQTT_String_compare(str, "yes") == 0
        || RTI_MQTT_String_compare(str, "YES") == 0
        || RTI_MQTT_String_compare(str, "Yes") == 0
        || RTI_MQTT_String_compare(str, "1") == 0) {
        *bool_out = DDS_BOOLEAN_TRUE;
        return DDS_RETCODE_OK;
    } else if (
            RTI_MQTT_String_compare(str, "f") == 0
            || RTI_MQTT_String_compare(str, "F") == 0
            || RTI_MQTT_String_compare(str, "false") == 0
            || RTI_MQTT_String_compare(str, "FALSE") == 0
            || RTI_MQTT_String_compare(str, "False") == 0
            || RTI_MQTT_String_compare(str, "n") == 0
            || RTI_MQTT_String_compare(str, "N") == 0
            || RTI_MQTT_String_compare(str, "no") == 0
            || RTI_MQTT_String_compare(str, "NO") == 0
            || RTI_MQTT_String_compare(str, "No") == 0
            || RTI_MQTT_String_compare(str, "0") == 0) {
        *bool_out = DDS_BOOLEAN_FALSE;
        return DDS_RETCODE_OK;
    }
    return DDS_RETCODE_ERROR;
}

#define RTI_RS_MQTT_lookup_property(pp_, p_, set_)                         \
    {                                                                      \
        const char *pval =                                                 \
                RTI_RoutingServiceProperties_lookup_property((pp_), (p_)); \
        if (pval != NULL) {                                                \
            RTI_MQTT_LOG_1("property FOUND:", "%s", (p_))                  \
            set_                                                           \
        } else {                                                           \
            RTI_MQTT_TRACE_1("property NOT FOUND:", "%s", (p_))            \
        }                                                                  \
    }


DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnectionConfig_default(
        RTI_RS_MQTT_BrokerConnectionConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean config_allocd = DDS_BOOLEAN_FALSE;
    RTI_RS_MQTT_BrokerConnectionConfig *config = *config_out;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnectionConfig_default)

    if (config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_RS_MQTT_BrokerConnectionConfig_new(
                    DDS_BOOLEAN_FALSE,
                    &config)) {
            /* TODO Log error */
            goto done;
        }
        config_allocd = DDS_BOOLEAN_TRUE;
    }

    if (!RTI_RS_MQTT_BrokerConnectionConfig_copy(
                config,
                &RTI_RS_MQTT_BrokerConnectionConfig_DEFAULT)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_RS_MQTT_BrokerConnectionConfig",
                config,
                &RTI_RS_MQTT_BrokerConnectionConfig_DEFAULT)
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        if (config != NULL && config_allocd) {
            RTI_RS_MQTT_BrokerConnectionConfig_delete(config);
        }
    }
    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnectionConfig_new(
        DDS_Boolean allocate_optional,
        RTI_RS_MQTT_BrokerConnectionConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_RS_MQTT_BrokerConnectionConfig *config = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnectionConfig_new)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = allocate_optional;

    config = RTI_RS_MQTT_BrokerConnectionConfigTypeSupport_create_data_w_params(
            &alloc_params);
    if (config == NULL) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

void RTI_RS_MQTT_BrokerConnectionConfig_delete(
        RTI_RS_MQTT_BrokerConnectionConfig *self)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnectionConfig_delete)
    RTI_RS_MQTT_BrokerConnectionConfigTypeSupport_delete_data(self);
}

static DDS_ReturnCode_t RTI_MQTT_ClientConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_MQTT_ClientConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_ClientConfig *config = *config_out;
    DDS_Boolean loaned_config = DDS_BOOLEAN_FALSE,
                allocd_config = DDS_BOOLEAN_FALSE;

    loaned_config = (config != NULL);

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientConfig_parse_from_properties)

    if (DDS_RETCODE_OK != RTI_MQTT_ClientConfig_default(&config)) {
        /* TODO Log error */
        goto done;
    }
    if (!loaned_config) {
        allocd_config = DDS_BOOLEAN_TRUE;
    }

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_ID,
            DDS_String_free(config->id);
            config->id = DDS_String_dup(pval);
            if (config->id == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_PROTOCOL_VERSION,
            RTI_MQTT_MqttProtocolVersion proto_v =
                    RTI_MQTT_MqttProtocolVersion_MQTT_DEFAULT;
            if (DDS_RETCODE_OK
                != RTI_MQTT_MqttProtocolVersion_from_string(
                        pval,
                        &proto_v)) {
                /* TODO Log error */
                goto done;
            } config->protocol_version = proto_v;)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_SERVERS,
            struct DDS_StringSeq servers =
                    DDS_SEQUENCE_INITIALIZER;
            DDS_Boolean failed = DDS_BOOLEAN_FALSE;
            if (DDS_RETCODE_OK
                != DDS_StringSeq_from_string(pval, &servers)) {
                /* TODO Log error */
                goto done;
            } if (!DDS_StringSeq_copy(&config->server_uris, &servers)) {
                /* TODO Log error */
                failed = DDS_BOOLEAN_TRUE;
            } if (!DDS_StringSeq_finalize(&servers)) {
                /* TODO Log error */
                failed = DDS_BOOLEAN_TRUE;
            } if (failed) { goto done; })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_CONNECTION_TIMEOUT_SECONDS,
            config->connect_timeout.seconds =
                    RTI_MQTT_String_to_long(
                            pval,
                            NULL,
                            0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_CONNECTION_TIMEOUT_NANOSECONDS,
            config->connect_timeout
                    .nanoseconds =
                    RTI_MQTT_String_to_long(
                            pval,
                            NULL,
                            0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_MAX_CONNECTION_RETRIES,
            config->max_connection_retries =
                    RTI_MQTT_String_to_long(
                            pval,
                            NULL,
                            0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_KEEP_ALIVE_PERIOD_SECONDS,
            config->connect_timeout
                    .seconds =
                    RTI_MQTT_String_to_long(
                            pval,
                            NULL,
                            0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_KEEP_ALIVE_PERIOD_NANOSECONDS,
            config->connect_timeout
                    .nanoseconds = RTI_MQTT_String_to_long(
                    pval,
                    NULL,
                    0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_CLEAN_SESSION,
            if (DDS_RETCODE_OK
                != DDS_Boolean_from_string(
                        pval,
                        &config->clean_session)) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_UNSUBSCRIBE_ON_DISCONNECT,
            if (DDS_RETCODE_OK
                != DDS_Boolean_from_string(
                        pval,
                        &config->unsubscribe_on_disconnect)) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_MAX_REPLY_TIMEOUT_SECONDS,
            config->max_reply_timeout
                    .seconds = RTI_MQTT_String_to_long(
                    pval,
                    NULL,
                    0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_MAX_REPLY_TIMEOUT_NANOSECONDS,
            config->max_reply_timeout
                    .nanoseconds = RTI_MQTT_String_to_long(
                    pval,
                    NULL,
                    0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_RECONNECT,
            if (DDS_RETCODE_OK!= DDS_Boolean_from_string(
                        pval,
                        &config->reconnect)) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_MAX_UNACK_MESSAGES,
            config->max_unack_messages = RTI_MQTT_String_to_long(
                    pval,
                    NULL,
                    0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_PERSISTENCE,
            if (DDS_RETCODE_OK
                != RTI_MQTT_PersistenceLevel_from_string(
                        pval,
                        &config->persistence_level)) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_PERSISTENCE_STORAGE,
            config->persistence_storage = DDS_String_dup(pval);
            if (config->persistence_storage == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_USERNAME,
            config->username = DDS_String_dup(pval);
            if (config->username == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_PASSWORD,
                struct DDS_OctetSeq password = DDS_SEQUENCE_INITIALIZER;
                DDS_Boolean failed = DDS_BOOLEAN_FALSE;
                RTI_MQTT_LOG_1("password:", "%s", pval);
                if (DDS_RETCODE_OK != DDS_OctetSeq_from_string(
                            pval,
                            &password)) {
                    RTI_MQTT_ERROR_1(
                            "failed to convert string to OctetSeq:",
                            "%s",
                            pval)
                    goto done;
                } if (config->password != NULL) {
                    if (!DDS_OctetSeq_copy(
                                config->password,
                                &password)) {
                        RTI_MQTT_ERROR(
                                "failed to copy password sequence")
                        failed =
                                DDS_BOOLEAN_TRUE;
                    }
                    if (!DDS_OctetSeq_finalize(&password)) {
                        RTI_MQTT_ERROR(
                                "failed to finalize password sequence")
                        failed = DDS_BOOLEAN_TRUE;
                    }
                } else {
                    config->password = (struct DDS_OctetSeq *)
                            RTI_MQTT_Heap_allocate(sizeof(
                                    struct
                                    DDS_OctetSeq));
                    if (config->password == NULL) {
                        RTI_MQTT_ERROR(
                                "failed to allocate password sequence")
                        if (!DDS_OctetSeq_finalize(&password)) {
                            RTI_MQTT_ERROR(
                                    "failed to finalize password sequence")
                        }
                        failed = DDS_BOOLEAN_TRUE;
                    } else {
                        *config->password = password;
                    }
                }

                if (failed) {
                    RTI_MQTT_ERROR("password parsing FAILED")
                    goto done;
                })

#if RTI_MQTT_USE_SSL

    #define allocate_ssl_tls_config(c_)                                        \
        {                                                                      \
            if ((c_)->ssl_tls_config == NULL) {                                \
                (c_)->ssl_tls_config =                                         \
                        (RTI_MQTT_SslTlsConfig *) RTI_MQTT_Heap_allocate(      \
                                sizeof(RTI_MQTT_SslTlsConfig));                \
                if ((c_)->ssl_tls_config == NULL) {                            \
                    /* TODO Log error */                                       \
                    goto done;                                                 \
                }                                                              \
                if (!RTI_MQTT_SslTlsConfig_initialize((c_)->ssl_tls_config)) { \
                    /* TODO Log error */                                       \
                    goto done;                                                 \
                }                                                              \
                /* Set default value here. We might want to refactor this      \
                 * macro to                                                    \
                 * it's own function (e.g. RTI_MQTT_SslTlsConfig_new) */       \
                (c_)->ssl_tls_config->verify_server_certificate =              \
                        DDS_BOOLEAN_TRUE;                                      \
            }                                                                  \
        }
    /* Scan server URIs for SSL URIS, and allocate a configuration if
       at least one is found */
    {
        DDS_UnsignedLong uris_len = 0, i = 0;

        uris_len = DDS_StringSeq_get_length(&config->server_uris);

        RTI_MQTT_LOG("check URIs for SSL")

        for (i = 0; i < uris_len && config->ssl_tls_config == NULL; i++) {
            char *uri = *DDS_StringSeq_get_reference(&config->server_uris, i);

            RTI_MQTT_LOG_1("checking URI:", "%s", uri)

            /* Check if uri starts with SSL prefix */
            if (uri == RTI_MQTT_String_find_substring(
                        uri,
                        RTI_MQTT_URI_PREFIX_SSL)) {
                allocate_ssl_tls_config(config);
                RTI_MQTT_LOG("allocated SSL/TLS config")
            }
        }
    }

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_SSL_CA,
            allocate_ssl_tls_config(config);
            DDS_String_replace(&config->ssl_tls_config->ca, pval);
            if (config->ssl_tls_config->ca == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_SSL_IDENTITY,
            allocate_ssl_tls_config(config);
            DDS_String_replace(&config->ssl_tls_config->identity, pval);
            if (config->ssl_tls_config->identity == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_SSL_PRIVATE_KEY,
            allocate_ssl_tls_config(config);
            DDS_String_replace(
                    &config->ssl_tls_config->private_key,
                    pval);
            if (config->ssl_tls_config->private_key == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_SSL_PRIVATE_KEY_PASSWORD,
            allocate_ssl_tls_config(config);
            DDS_String_replace(
                    &config->ssl_tls_config
                                ->private_key_password,
                    pval);
            if (config->ssl_tls_config->private_key_password == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_SSL_CYPHER_SUITES,
            allocate_ssl_tls_config(config);
            DDS_String_replace( &config->ssl_tls_config->cypher_suites, pval);
            if (config->ssl_tls_config
                        ->cypher_suites
                == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_SSL_PROTOCOL_VERSION,
            allocate_ssl_tls_config(
                    config);
            if (DDS_RETCODE_OK
                != RTI_MQTT_SslTlsProtocolVersion_from_string(
                        pval,
                        &config->ssl_tls_config->protocol_version)) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_CLIENT_SSL_VERIFY_SERVER_CERTIFICATE,
            allocate_ssl_tls_config(
                    config);
            if (DDS_RETCODE_OK
                != DDS_Boolean_from_string(
                        pval,
                        &config->ssl_tls_config->verify_server_certificate)) {
                /* TODO Log
                    * error */
                goto done;
            })

#endif /* RTI_MQTT_USE_SSL */

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (config != NULL && allocd_config) {
            RTI_MQTT_ClientConfig_delete(config);
        }
    }
    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnectionConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RS_MQTT_BrokerConnectionConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_RS_MQTT_BrokerConnectionConfig *config = *config_out;
    DDS_Boolean loaned_config = DDS_BOOLEAN_FALSE,
                allocd_config = DDS_BOOLEAN_FALSE;
    RTI_MQTT_ClientConfig *client_config = NULL;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnectionConfig_parse_from_properties)

    loaned_config = (config != NULL);

    if (DDS_RETCODE_OK != RTI_RS_MQTT_BrokerConnectionConfig_default(&config)) {
        /* TODO Log error */
        goto done;
    }

    if (!loaned_config) {
        allocd_config = DDS_BOOLEAN_TRUE;
    }

    client_config = &config->client;

    if (DDS_RETCODE_OK
        != RTI_MQTT_ClientConfig_parse_from_properties(
                properties,
                &client_config)) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (config != NULL && allocd_config) {
            RTI_RS_MQTT_BrokerConnectionConfig_delete(config);
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_MessageReaderConfig_default(
        RTI_RS_MQTT_MessageReaderConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean config_allocd = DDS_BOOLEAN_FALSE;
    RTI_RS_MQTT_MessageReaderConfig *config = *config_out;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReaderConfig_default)

    if (config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_RS_MQTT_MessageReaderConfig_new(
                    DDS_BOOLEAN_FALSE,
                    &config)) {
            /* TODO Log error */
            goto done;
        }
        config_allocd = DDS_BOOLEAN_TRUE;
    }

    if (!RTI_RS_MQTT_MessageReaderConfig_copy(
                config,
                &RTI_RS_MQTT_MessageReaderConfig_DEFAULT)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_RS_MQTT_MessageReaderConfig",
                config,
                &RTI_RS_MQTT_MessageReaderConfig_DEFAULT)
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        if (config != NULL && config_allocd) {
            RTI_RS_MQTT_MessageReaderConfig_delete(config);
        }
    }
    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_MessageReaderConfig_new(
        DDS_Boolean allocate_optional,
        RTI_RS_MQTT_MessageReaderConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_RS_MQTT_MessageReaderConfig *config = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReaderConfig_new)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = allocate_optional;

    config = RTI_RS_MQTT_MessageReaderConfigTypeSupport_create_data_w_params(
            &alloc_params);
    if (config == NULL) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

void RTI_RS_MQTT_MessageReaderConfig_delete(
        RTI_RS_MQTT_MessageReaderConfig *self)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReaderConfig_delete)

    RTI_RS_MQTT_MessageReaderConfigTypeSupport_delete_data(self);
}

static DDS_ReturnCode_t RTI_MQTT_SubscriptionConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_MQTT_SubscriptionConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_SubscriptionConfig *config = *config_out;
    DDS_Boolean loaned_config = DDS_BOOLEAN_FALSE,
                allocd_config = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_SubscriptionConfig_parse_from_properties)

    loaned_config = (config != NULL);

    if (DDS_RETCODE_OK != RTI_MQTT_SubscriptionConfig_default(&config)) {
        /* TODO Log error */
        goto done;
    }
    if (!loaned_config) {
        allocd_config = DDS_BOOLEAN_TRUE;
    }

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_SUBSCRIPTION_TOPICS,
            struct DDS_StringSeq topics = DDS_SEQUENCE_INITIALIZER;
            DDS_Boolean failed = DDS_BOOLEAN_FALSE;
            if (DDS_RETCODE_OK != DDS_StringSeq_from_string(pval, &topics)) {
                /* TODO Log error */
                goto done;
            } if (!DDS_StringSeq_copy(&config->topic_filters, &topics)) {
                /* TODO Log error */
                failed = DDS_BOOLEAN_TRUE;
            } if (!DDS_StringSeq_finalize(&topics)) {
                /* TODO Log error */
                failed = DDS_BOOLEAN_TRUE;
            } if (failed) { goto done; })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_SUBSCRIPTION_MAX_QOS,
            RTI_MQTT_QosLevel qos = RTI_MQTT_QosLevel_UNKNOWN;
            if (DDS_RETCODE_OK != RTI_MQTT_QosLevel_from_string(pval, &qos)) {
                /* TODO Log error */
                goto done;
            } config->max_qos = qos;)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_SUBSCRIPTION_QUEUE_SIZE,
            config->message_queue_size =
                    RTI_MQTT_String_to_long(pval, NULL, 0);)

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (config != NULL && allocd_config) {
            RTI_MQTT_SubscriptionConfig_delete(config);
        }
    }
    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_MessageReaderConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RS_MQTT_MessageReaderConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_RS_MQTT_MessageReaderConfig *config = *config_out;
    DDS_Boolean loaned_config = DDS_BOOLEAN_FALSE,
                allocd_config = DDS_BOOLEAN_FALSE;
    RTI_MQTT_SubscriptionConfig *sub_config = NULL;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReaderConfig_parse_from_properties)

    loaned_config = (config != NULL);

    if (DDS_RETCODE_OK != RTI_RS_MQTT_MessageReaderConfig_default(&config)) {
        /* TODO Log error */
        goto done;
    }

    if (!loaned_config) {
        allocd_config = DDS_BOOLEAN_TRUE;
    }

    sub_config = &config->sub;

    if (DDS_RETCODE_OK
        != RTI_MQTT_SubscriptionConfig_parse_from_properties(
                properties,
                &sub_config)) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (config != NULL && allocd_config) {
            RTI_RS_MQTT_MessageReaderConfig_delete(config);
        }
    }
    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_MessageWriterConfig_default(
        RTI_RS_MQTT_MessageWriterConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean config_allocd = DDS_BOOLEAN_FALSE;
    RTI_RS_MQTT_MessageWriterConfig *config = *config_out;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageWriterConfig_default)

    if (config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_RS_MQTT_MessageWriterConfig_new(
                    DDS_BOOLEAN_FALSE,
                    &config)) {
            /* TODO Log error */
            goto done;
        }
        config_allocd = DDS_BOOLEAN_TRUE;
    }

    if (!RTI_RS_MQTT_MessageWriterConfig_copy(
                config,
                &RTI_RS_MQTT_MessageWriterConfig_DEFAULT)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_RS_MQTT_MessageWriterConfig",
                config,
                &RTI_RS_MQTT_MessageWriterConfig_DEFAULT)
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        if (config != NULL && config_allocd) {
            RTI_RS_MQTT_MessageWriterConfig_delete(config);
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_MessageWriterConfig_new(
        DDS_Boolean allocate_optional,
        RTI_RS_MQTT_MessageWriterConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_RS_MQTT_MessageWriterConfig *config = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageWriterConfig_new)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = allocate_optional;

    config = RTI_RS_MQTT_MessageWriterConfigTypeSupport_create_data_w_params(
            &alloc_params);
    if (config == NULL) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

void RTI_RS_MQTT_MessageWriterConfig_delete(
        RTI_RS_MQTT_MessageWriterConfig *self)
{
    RTI_RS_MQTT_MessageWriterConfigTypeSupport_delete_data(self);
}


static DDS_ReturnCode_t RTI_MQTT_PublicationConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_MQTT_PublicationConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_PublicationConfig *config = *config_out;
    DDS_Boolean loaned_config = DDS_BOOLEAN_FALSE,
                allocd_config = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_PublicationConfig_parse_from_properties)

    loaned_config = (config != NULL);

    if (DDS_RETCODE_OK != RTI_MQTT_PublicationConfig_default(&config)) {
        /* TODO Log error */
        goto done;
    }
    if (!loaned_config) {
        allocd_config = DDS_BOOLEAN_TRUE;
    }

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_PUBLICATION_TOPIC,
            DDS_String_replace(&config->topic, pval);
            if (config->topic == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_PUBLICATION_QOS,
            RTI_MQTT_QosLevel qos = RTI_MQTT_QosLevel_UNKNOWN;
            if (DDS_RETCODE_OK
                != RTI_MQTT_QosLevel_from_string(pval, &qos)) {
                /* TODO Log error */
                goto done;
            } config->qos = qos;)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_PUBLICATION_RETAINED,
            if (DDS_RETCODE_OK != DDS_Boolean_from_string(
                        pval,
                        &config->retained)) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_PUBLICATION_USE_MESSAGE_INFO,
            if (DDS_RETCODE_OK
                != DDS_Boolean_from_string(
                        pval,
                        &config->use_message_info)) {
                /* TODO Log error */
                goto done;
            })

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_PUBLICATION_MAX_WAIT_TIME_SECONDS,
            config->max_wait_time.seconds =
                    RTI_MQTT_String_to_long(pval, NULL, 0);)

    RTI_RS_MQTT_lookup_property(
            properties,
            RTI_MQTT_PROPERTY_PUBLICATION_MAX_WAIT_TIME_NANOSECONDS,
            config->max_wait_time.nanoseconds =
                    RTI_MQTT_String_to_long(pval, NULL, 0);)

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (config != NULL && allocd_config) {
            RTI_MQTT_PublicationConfig_delete(config);
        }
    }
    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_MessageWriterConfig_parse_from_properties(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RS_MQTT_MessageWriterConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_RS_MQTT_MessageWriterConfig *config = *config_out;
    DDS_Boolean loaned_config = DDS_BOOLEAN_FALSE,
                allocd_config = DDS_BOOLEAN_FALSE;
    RTI_MQTT_PublicationConfig *pub_config = NULL;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageWriterConfig_parse_from_properties)

    loaned_config = (config != NULL);

    if (DDS_RETCODE_OK != RTI_RS_MQTT_MessageWriterConfig_default(&config)) {
        /* TODO Log error */
        goto done;
    }

    if (!loaned_config) {
        allocd_config = DDS_BOOLEAN_TRUE;
    }

    pub_config = &config->pub;

    if (DDS_RETCODE_OK != RTI_MQTT_PublicationConfig_parse_from_properties(
                properties,
                &pub_config)) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (config != NULL && allocd_config) {
            RTI_RS_MQTT_MessageWriterConfig_delete(config);
        }
    }
    return retval;
}
