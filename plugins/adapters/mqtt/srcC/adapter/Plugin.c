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

#include "Plugin.h"

#include "Infrastructure.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::RS::Plugin"

const struct RTI_RoutingServiceVersion RTI_RS_MQTT_AdapterPlugin_VERSION = {
    RTI_RS_MQTT_ADAPTER_VERSION_MAJOR,
    RTI_RS_MQTT_ADAPTER_VERSION_MINOR,
    RTI_RS_MQTT_ADAPTER_VERSION_RELEASE,
    RTI_RS_MQTT_ADAPTER_VERSION_REVISION
};

DDS_ReturnCode_t RTI_RS_MQTT_AdapterPlugin_new(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_AdapterPlugin **plugin_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_RS_MQTT_AdapterPlugin *plugin = NULL;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_AdapterPlugin_new)

    plugin = (struct RTI_RS_MQTT_AdapterPlugin *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_RS_MQTT_AdapterPlugin));
    if (plugin == NULL) {
        /* TODO Log error */
        goto done;
    }

    RTI_RoutingServiceAdapterPlugin_initialize(&plugin->parent);

    if (!RTI_RS_MQTT_BrokerConnectionPtrSeq_initialize(&plugin->connections)) {
        /* TODO Log error */
        goto done;
    }

    plugin->parent.plugin_version = RTI_RS_MQTT_AdapterPlugin_VERSION;

    plugin->parent.adapter_plugin_delete = RTI_RS_MQTT_AdapterPlugin_delete;
    plugin->parent.adapter_plugin_create_connection =
            RTI_RS_MQTT_AdapterPlugin_create_connection;
    plugin->parent.adapter_plugin_delete_connection =
            RTI_RS_MQTT_AdapterPlugin_delete_connection;
#if 0
    plugin->parent.connection_create_session = 
                    RTI_RS_MQTT_BrokerConnection_create_session;
    plugin->parent.connection_delete_session =
                    RTI_RS_MQTT_BrokerConnection_delete_session;
#else
    plugin->parent.connection_create_session = NULL;
    plugin->parent.connection_delete_session = NULL;
#endif

    plugin->parent.connection_create_stream_reader =
            RTI_RS_MQTT_BrokerConnection_create_stream_reader;
    plugin->parent.connection_delete_stream_reader =
            RTI_RS_MQTT_BrokerConnection_delete_stream_reader;
    plugin->parent.connection_create_stream_writer =
            RTI_RS_MQTT_BrokerConnection_create_stream_writer;
    plugin->parent.connection_delete_stream_writer =
            RTI_RS_MQTT_BrokerConnection_delete_stream_writer;
#if 0
    plugin->parent.connection_get_input_stream_discovery_reader = 
                    RTI_RS_MQTT_BrokerConnection_get_input_discovery_reader;
    plugin->parent.connection_get_output_stream_discovery_reader =
                    RTI_RS_MQTT_BrokerConnection_get_output_discovery_reader;
#else
    plugin->parent.connection_get_input_stream_discovery_reader = NULL;
    plugin->parent.connection_get_output_stream_discovery_reader = NULL;
#endif
    plugin->parent.connection_copy_type_representation = NULL;
    plugin->parent.connection_delete_type_representation = NULL;
    plugin->parent.connection_to_string =
            RTI_RS_MQTT_BrokerConnection_to_string;
    plugin->parent.connection_update = RTI_RS_MQTT_BrokerConnection_update;
#if 0
    plugin->parent.session_update =
                    RTI_RS_MQTT_BrokerConnection_update_session;
#else
    plugin->parent.session_update = NULL;
#endif
    plugin->parent.stream_reader_read = RTI_RS_MQTT_MessageReader_read;
    plugin->parent.stream_reader_return_loan =
            RTI_RS_MQTT_MessageReader_return_loan;
    plugin->parent.stream_reader_update = RTI_RS_MQTT_MessageReader_update;
    plugin->parent.stream_writer_write = RTI_RS_MQTT_MessageWriter_write;
    plugin->parent.stream_writer_update = RTI_RS_MQTT_MessageWriter_update;

    plugin->parent.user_object = plugin;

    *plugin_out = plugin;

    retval = DDS_RETCODE_OK;
done:
    if (retval != DDS_RETCODE_OK) {
        if (plugin != NULL) {
            RTI_MQTT_Heap_free(plugin);
        }
    }

    return retval;
}

void RTI_RS_MQTT_AdapterPlugin_delete(
        struct RTI_RoutingServiceAdapterPlugin *plugin,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTI_RS_MQTT_AdapterPlugin *self =
            (struct RTI_RS_MQTT_AdapterPlugin *) plugin;
    DDS_UnsignedLong seq_len = 0;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_AdapterPlugin_delete)

    seq_len = RTI_RS_MQTT_BrokerConnectionPtrSeq_get_length(&self->connections);

    while (seq_len > 0) {
        struct RTI_RS_MQTT_BrokerConnection *conn =
                *RTI_RS_MQTT_BrokerConnectionPtrSeq_get_reference(
                        &self->connections,
                        0);

        RTI_RS_MQTT_AdapterPlugin_delete_connection(plugin, conn, env);

        seq_len = RTI_RS_MQTT_BrokerConnectionPtrSeq_get_length(
                &self->connections);
    }

    if (!RTI_RS_MQTT_BrokerConnectionPtrSeq_finalize(&self->connections)) {
        /* TODO Log error */
        goto done;
    }

    RTI_MQTT_Heap_free(self);

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval) {
        /* TODO Log error */
    }
}

struct RTI_RoutingServiceAdapterPlugin *RTI_RS_MQTT_AdapterPlugin_create(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTI_RS_MQTT_AdapterPlugin *plugin = NULL;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_AdapterPlugin_create)

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_AdapterPlugin_new(properties, env, &plugin)) {
        /* TODO Log error */
        goto done;
    }

    RTI_MQTT_LOG_1("new ADAPTER created:", "adapter=%p", plugin)

    retval = DDS_BOOLEAN_TRUE;
done:
    if (retval) {
        return &plugin->parent;
    } else {
        return NULL;
    }
}


RTI_RoutingServiceConnection RTI_RS_MQTT_AdapterPlugin_create_connection(
        struct RTI_RoutingServiceAdapterPlugin *plugin,
        const char *routing_service_name,
        const char *routing_service_group_name,
        const struct RTI_RoutingServiceStreamReaderListener
                *input_stream_discovery_listener,
        const struct RTI_RoutingServiceStreamReaderListener
                *output_stream_discovery_listener,
        const struct RTI_RoutingServiceTypeInfo **ed_types,
        int ed_types_count,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTI_RS_MQTT_AdapterPlugin *self =
            (struct RTI_RS_MQTT_AdapterPlugin *) plugin;
    struct RTI_RS_MQTT_BrokerConnection *conn = NULL, **conn_ref = NULL,
                                        *conn_out = NULL;
    DDS_UnsignedLong cur_conn_len = 0;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_AdapterPlugin_create_connection)
    RTI_MQTT_LOG_2(
            "create CONNECTION",
            "rs=%s, rs_group=%s",
            routing_service_name,
            routing_service_group_name)

    cur_conn_len =
            RTI_RS_MQTT_BrokerConnectionPtrSeq_get_length(&self->connections);

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_BrokerConnection_new(
                input_stream_discovery_listener,
                output_stream_discovery_listener,
                ed_types,
                ed_types_count,
                properties,
                env,
                &conn)) {
        /* TODO Log error */
        goto done;
    }

    if (!RTI_RS_MQTT_BrokerConnectionPtrSeq_ensure_length(
                &self->connections,
                cur_conn_len + 1,
                cur_conn_len + 1)) {
        /* TODO Log error */
        goto done;
    }

    conn_ref = RTI_RS_MQTT_BrokerConnectionPtrSeq_get_reference(
            &self->connections,
            cur_conn_len);

    *conn_ref = conn;

    RTI_MQTT_LOG_2(
            "new CONNECTION created:",
            "adapter=%p, connection=%p",
            self,
            conn)

    conn_out = conn;

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval) {
        if (conn_ref != NULL && *conn_ref != NULL) {
            *conn_ref = NULL;
            if (!RTI_RS_MQTT_BrokerConnectionPtrSeq_set_length(
                        &self->connections,
                        cur_conn_len)) {
                /* TODO Log error */
                goto done;
            }
        }
        if (conn != NULL) {
            RTI_RS_MQTT_BrokerConnection_delete(conn);
        }
    }

    return conn_out;
}

void RTI_RS_MQTT_AdapterPlugin_delete_connection(
        struct RTI_RoutingServiceAdapterPlugin *plugin,
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_MQTT_AdapterPlugin *self =
            (struct RTI_RS_MQTT_AdapterPlugin *) plugin;
    struct RTI_RS_MQTT_BrokerConnection *conn =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;
    DDS_UnsignedLong seq_len = 0, i = 0, rm_i = 0;
    DDS_Boolean do_rm = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_AdapterPlugin_delete_connection)

    seq_len = RTI_RS_MQTT_BrokerConnectionPtrSeq_get_length(&self->connections);

    for (i = 0; i < seq_len && !do_rm; i++) {
        if (*RTI_RS_MQTT_BrokerConnectionPtrSeq_get_reference(
                    &self->connections,
                    i)
            == conn) {
            rm_i = i;
            do_rm = DDS_BOOLEAN_TRUE;
        }
    }

    if (!do_rm) {
        /* TODO Log error */
        return;
    }

    RTI_RS_MQTT_BrokerConnection_delete(conn);

    for (i = rm_i; i < seq_len - 1; i++) {
        *RTI_RS_MQTT_BrokerConnectionPtrSeq_get_reference(
                &self->connections,
                i) =
                *RTI_RS_MQTT_BrokerConnectionPtrSeq_get_reference(
                        &self->connections,
                        i + 1);
    }

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_BrokerConnectionPtrSeq_set_length(
                &self->connections,
                seq_len - 1)) {
        /* TODO Log error */
    }
}

void RTI_RS_MQTT_AdapterPlugin_update(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_AdapterPlugin_update)
    /* TODO Implement me */
}