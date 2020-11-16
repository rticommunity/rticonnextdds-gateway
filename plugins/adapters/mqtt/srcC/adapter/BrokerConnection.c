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

#include "BrokerConnection.h"
#include "Infrastructure.h"
#include "Properties.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::RS::Connection"

DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnection_new(
        const struct RTI_RoutingServiceStreamReaderListener
                *input_stream_discovery_listener,
        const struct RTI_RoutingServiceStreamReaderListener
                *output_stream_discovery_listener,
        const struct RTI_RoutingServiceTypeInfo **ed_types,
        int ed_types_count,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_BrokerConnection **connection_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_RS_MQTT_BrokerConnection *conn = NULL;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_new)

    conn = (struct RTI_RS_MQTT_BrokerConnection *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_RS_MQTT_BrokerConnection));
    if (conn == NULL) {
        /* TODO Log error */
        goto done;
    }

    conn->client = NULL;
    conn->config = NULL;

    conn->tc_message = NULL;

    conn->disc_reader_in = NULL;
    conn->disc_reader_out = NULL;

    conn->listener_discovery_in = input_stream_discovery_listener;
    conn->listener_discovery_out = output_stream_discovery_listener;

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_BrokerConnectionConfig_parse_from_properties(
                properties,
                &conn->config)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_MessageReader_new_discovery(
                conn,
                conn->listener_discovery_in,
                properties,
                env,
                &conn->disc_reader_in)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_MessageReader_new_discovery(
                conn,
                conn->listener_discovery_out,
                properties,
                env,
                &conn->disc_reader_out)) {
        /* TODO Log error */
        goto done;
    }

    conn->tc_message = RTI_MQTT_Message_get_typecode();
    if (conn->tc_message == NULL) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_new(&conn->config->client, &conn->client)) {
        /* TODO Log error */
        goto done;
    }

    if (!RTI_RS_MQTT_MessageWriterPtrSeq_initialize(&conn->writers)) {
        /* TODO Log error */
        goto done;
    }

    if (!RTI_RS_MQTT_MessageReaderPtrSeq_initialize(&conn->readers)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK != RTI_MQTT_Client_connect(conn->client)) {
        /* TODO Log error */
        goto done;
    }

    *connection_out = conn;

    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (conn != NULL) {
            RTI_RS_MQTT_BrokerConnection_delete(conn);
        }
    }

    return retval;
}

void RTI_RS_MQTT_BrokerConnection_delete(
        struct RTI_RS_MQTT_BrokerConnection *self)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_delete)

    if (self->client != NULL
        && DDS_RETCODE_OK != RTI_MQTT_Client_disconnect(self->client)) {
        /* TODO Log error */
    }

    if (self->disc_reader_in != NULL) {
        RTI_RS_MQTT_MessageReader_delete(self->disc_reader_in);
    }
    if (self->disc_reader_out != NULL) {
        RTI_RS_MQTT_MessageReader_delete(self->disc_reader_out);
    }
    if (self->config != NULL) {
        RTI_RS_MQTT_BrokerConnectionConfig_delete(self->config);
    }
    if (self->client != NULL) {
        RTI_MQTT_Client_delete(self->client);
    }
    if (self->tc_message != NULL) {
        /* Nothing to finalize */
    }

    if (!RTI_RS_MQTT_MessageReaderPtrSeq_finalize(&self->readers)) {
        /* TODO Log error */
    }

    if (!RTI_RS_MQTT_MessageWriterPtrSeq_finalize(&self->writers)) {
        /* TODO Log error */
    }

    RTI_MQTT_Heap_free(self);
}

static DDS_ReturnCode_t RTI_RS_MQTT_BrokerConnection_validate_stream(
        struct RTI_RS_MQTT_BrokerConnection *self,
        const struct RTI_RoutingServiceStreamInfo *stream_info)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_validate_stream)

    if (stream_info->type_info.type_representation_kind
                != RTI_ROUTING_SERVICE_TYPE_REPRESENTATION_DYNAMIC_TYPE
        || (RTI_MQTT_String_compare(
                    stream_info->type_info.type_name,
                    RTI_MQTT_MessageTypeSupport_get_type_name())
                    != 0
            && RTI_MQTT_String_compare(
                       stream_info->type_info.type_name,
                       RTI_MQTT_KeyedMessageTypeSupport_get_type_name())
                    != 0)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

RTI_RoutingServiceStreamReader
        RTI_RS_MQTT_BrokerConnection_create_stream_reader(
                RTI_RoutingServiceConnection connection,
                RTI_RoutingServiceSession session,
                const struct RTI_RoutingServiceStreamInfo *stream_info,
                const struct RTI_RoutingServiceProperties *properties,
                const struct RTI_RoutingServiceStreamReaderListener *listener,
                RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTI_RS_MQTT_BrokerConnection *self =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;
    struct RTI_RS_MQTT_MessageReader *reader = NULL, **reader_ref = NULL,
                                     *reader_out = NULL;
    DDS_UnsignedLong cur_reader_len = 0;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_create_stream_reader)

    RTI_MQTT_LOG_1("create READER for", "%s", stream_info->stream_name)

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_BrokerConnection_validate_stream(self, stream_info)) {
        /* TODO Log error */
        goto done;
    }

    cur_reader_len = RTI_RS_MQTT_MessageReaderPtrSeq_get_length(&self->readers);

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_MessageReader_new(
                self,
                listener,
                properties,
                env,
                &reader)) {
        /* TODO Log error */
        goto done;
    }

    if (!RTI_RS_MQTT_MessageReaderPtrSeq_ensure_length(
                &self->readers,
                cur_reader_len + 1,
                cur_reader_len + 1)) {
        /* TODO Log error */
        goto done;
    }

    reader_ref = RTI_RS_MQTT_MessageReaderPtrSeq_get_reference(
            &self->readers,
            cur_reader_len);

    *reader_ref = reader;

    reader_out = reader;

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval) {
        if (reader_ref != NULL && *reader_ref != NULL) {
            *reader_ref = NULL;
            if (!RTI_RS_MQTT_MessageReaderPtrSeq_set_length(
                        &self->readers,
                        cur_reader_len)) {
                /* TODO Log error */
                goto done;
            }
        }
        if (reader != NULL) {
            RTI_RS_MQTT_MessageReader_delete(reader);
        }
    }

    return (RTI_RoutingServiceStreamReader) reader_out;
}

void RTI_RS_MQTT_BrokerConnection_delete_stream_reader(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_MQTT_BrokerConnection *self =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;
    struct RTI_RS_MQTT_MessageReader *reader =
            (struct RTI_RS_MQTT_MessageReader *) stream_reader;
    DDS_UnsignedLong seq_len = 0, i = 0, rm_i = 0;
    DDS_Boolean do_rm = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_delete_stream_reader)

    seq_len = RTI_RS_MQTT_MessageReaderPtrSeq_get_length(&self->readers);

    for (i = 0; i < seq_len && !do_rm; i++) {
        if (*RTI_RS_MQTT_MessageReaderPtrSeq_get_reference(&self->readers, i)
            == reader) {
            rm_i = i;
            do_rm = DDS_BOOLEAN_TRUE;
        }
    }

    if (!do_rm) {
        /* TODO Log error */
        return;
    }

    RTI_RS_MQTT_MessageReader_delete(reader);

    for (i = rm_i; i < seq_len - 1; i++) {
        *RTI_RS_MQTT_MessageReaderPtrSeq_get_reference(&self->readers, i) =
                *RTI_RS_MQTT_MessageReaderPtrSeq_get_reference(
                        &self->readers,
                        i + 1);
    }

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_MessageReaderPtrSeq_set_length(
                &self->readers,
                seq_len - 1)) {
        /* TODO Log error */
    }
}

RTI_RoutingServiceStreamWriter
        RTI_RS_MQTT_BrokerConnection_create_stream_writer(
                RTI_RoutingServiceConnection connection,
                RTI_RoutingServiceSession session,
                const struct RTI_RoutingServiceStreamInfo *stream_info,
                const struct RTI_RoutingServiceProperties *properties,
                RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTI_RS_MQTT_BrokerConnection *self =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;
    struct RTI_RS_MQTT_MessageWriter *writer = NULL, **writer_ref = NULL,
                                     *writer_out = NULL;
    DDS_UnsignedLong cur_writer_len = 0;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_create_stream_writer)

    RTI_MQTT_LOG_1("create WRITER for", "%s", stream_info->stream_name)

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_BrokerConnection_validate_stream(self, stream_info)) {
        /* TODO Log error */
        goto done;
    }

    cur_writer_len = RTI_RS_MQTT_MessageWriterPtrSeq_get_length(&self->writers);

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_MessageWriter_new(self, properties, env, &writer)) {
        /* TODO Log error */
        goto done;
    }

    if (!RTI_RS_MQTT_MessageWriterPtrSeq_ensure_length(
                &self->writers,
                cur_writer_len + 1,
                cur_writer_len + 1)) {
        /* TODO Log error */
        goto done;
    }

    writer_ref = RTI_RS_MQTT_MessageWriterPtrSeq_get_reference(
            &self->writers,
            cur_writer_len);

    *writer_ref = writer;

    writer_out = writer;

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval) {
        if (writer_ref != NULL && *writer_ref != NULL) {
            *writer_ref = NULL;
            if (!RTI_RS_MQTT_MessageWriterPtrSeq_set_length(
                        &self->writers,
                        cur_writer_len)) {
                /* TODO Log error */
                goto done;
            }
        }
        if (writer != NULL) {
            RTI_RS_MQTT_MessageWriter_delete(writer);
        }
    }

    return (RTI_RoutingServiceStreamReader) writer_out;
}

void RTI_RS_MQTT_BrokerConnection_delete_stream_writer(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceStreamWriter stream_writer,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_MQTT_BrokerConnection *self =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;
    struct RTI_RS_MQTT_MessageWriter *writer =
            (struct RTI_RS_MQTT_MessageWriter *) stream_writer;
    DDS_UnsignedLong seq_len = 0, i = 0, rm_i = 0;
    DDS_Boolean do_rm = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_delete_stream_writer)

    seq_len = RTI_RS_MQTT_MessageWriterPtrSeq_get_length(&self->writers);

    for (i = 0; i < seq_len && !do_rm; i++) {
        if (*RTI_RS_MQTT_MessageWriterPtrSeq_get_reference(&self->writers, i)
            == writer) {
            rm_i = i;
            do_rm = DDS_BOOLEAN_TRUE;
        }
    }

    if (!do_rm) {
        /* TODO Log error */
        return;
    }

    RTI_RS_MQTT_MessageWriter_delete(writer);

    for (i = rm_i; i < seq_len - 1; i++) {
        *RTI_RS_MQTT_MessageWriterPtrSeq_get_reference(&self->writers, i) =
                *RTI_RS_MQTT_MessageWriterPtrSeq_get_reference(
                        &self->writers,
                        i + 1);
    }

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_MessageWriterPtrSeq_set_length(
                &self->writers,
                seq_len - 1)) {
        /* TODO Log error */
    }
}

RTI_RoutingServiceStreamReader
        RTI_RS_MQTT_BrokerConnection_get_input_discovery_reader(
                RTI_RoutingServiceConnection connection,
                RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_MQTT_BrokerConnection *self =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_get_input_discovery_reader)

    return (RTI_RoutingServiceStreamReader) self->disc_reader_in;
}

RTI_RoutingServiceStreamReader
        RTI_RS_MQTT_BrokerConnection_get_output_discovery_reader(
                RTI_RoutingServiceConnection connection,
                RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_MQTT_BrokerConnection *self =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_get_output_discovery_reader)

    return (RTI_RoutingServiceStreamReader) self->disc_reader_out;
}

void RTI_RS_MQTT_BrokerConnection_update(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_update)
    /* TODO Implement me */
}

void RTI_RS_MQTT_BrokerConnection_update_session(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_update_session)
    /* TODO Implement me */
}

const char *RTI_RS_MQTT_BrokerConnection_to_string(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_MQTT_BrokerConnection *self =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_to_string)

    return RTI_MQTT_Client_get_id(self->client);
}

RTI_RoutingServiceSession RTI_RS_MQTT_BrokerConnection_create_session(
        RTI_RoutingServiceConnection connection,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_RoutingServiceSession retval = NULL;
    struct RTI_RS_MQTT_BrokerConnection *self =
            (struct RTI_RS_MQTT_BrokerConnection *) connection;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_create_session)

    if (DDS_RETCODE_OK != RTI_MQTT_Client_connect(self->client)) {
        /* TODO Log error */
        goto done;
    }

    RTI_MQTT_LOG_1("SESSION connected:", "connection=%p", self)

    retval = (RTI_RoutingServiceSession) self;

done:

    return retval;
}

void RTI_RS_MQTT_BrokerConnection_delete_session(
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceSession session,
        RTI_RoutingServiceEnvironment *env) {
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_BrokerConnection_delete_session)
    /* Nothing to do */
}


RTIBool RTI_RS_MQTT_BrokerConnectionPtr_initialize_w_params(
        struct RTI_RS_MQTT_BrokerConnection **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}


RTIBool RTI_RS_MQTT_BrokerConnectionPtr_finalize_w_params(
        struct RTI_RS_MQTT_BrokerConnection **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_RS_MQTT_BrokerConnectionPtr_copy(
        struct RTI_RS_MQTT_BrokerConnection **dst,
        const struct RTI_RS_MQTT_BrokerConnection **src)
{
    *dst = (struct RTI_RS_MQTT_BrokerConnection *) *src;
    return RTI_TRUE;
}


#define T struct RTI_RS_MQTT_BrokerConnection *
#define TSeq RTI_RS_MQTT_BrokerConnectionPtrSeq
#define T_initialize_w_params \
    RTI_RS_MQTT_BrokerConnectionPtr_initialize_w_params
#define T_finalize_w_params RTI_RS_MQTT_BrokerConnectionPtr_finalize_w_params
#define T_copy RTI_RS_MQTT_BrokerConnectionPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T
