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

#include "MessageReader.h"
#include "BrokerConnection.h"
#include "Infrastructure.h"
#include "Properties.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::RS::Reader"

static void RTI_RS_MQTT_MessageReader_on_mqtt_data_available(
        void *listener_data,
        struct RTI_MQTT_Subscription *sub)
{
    struct RTI_RS_MQTT_MessageReader *self =
            (struct RTI_RS_MQTT_MessageReader *) listener_data;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_on_mqtt_data_available)

    if (self->listener != NULL) {
        RTI_MQTT_TRACE_1("notifying DATA AVAILABLE:", "sub=%p", sub)
        self->listener->on_data_available(self, self->listener->listener_data);
    }
}

static DDS_ReturnCode_t RTI_RS_MQTT_MessageReader_new_internal(
        struct RTI_RS_MQTT_BrokerConnection *connection,
        const struct RTI_RoutingServiceStreamReaderListener *listener,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        DDS_Boolean discovery,
        struct RTI_RS_MQTT_MessageReader **reader_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_RS_MQTT_MessageReader *reader = NULL;
    struct DDS_SampleInfoSeq def_info_seq = DDS_SEQUENCE_INITIALIZER;
    struct RTI_MQTT_DDS_SampleInfoPtrSeq def_info_ptr_seq =
            DDS_SEQUENCE_INITIALIZER;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_new_internal)

    reader = (struct RTI_RS_MQTT_MessageReader *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_RS_MQTT_MessageReader));
    if (reader == NULL) {
        /* TODO Log error */
        goto done;
    }

    reader->sub = NULL;
    reader->config = NULL;
    reader->info_seq = def_info_seq;
    reader->info_ptr_seq = def_info_ptr_seq;
    reader->listener = listener;
    reader->connection = connection;

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_MessageReaderConfig_parse_from_properties(
                properties,
                &reader->config)) {
        /* TODO Log error */
        goto done;
    }

    if (!discovery) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_Client_subscribe(
                    reader->connection->client,
                    &reader->config->sub,
                    &reader->sub)) {
            /* TODO Log error */
            goto done;
        }

        if (DDS_RETCODE_OK
            != RTI_MQTT_Subscription_set_data_available_listener(
                    reader->sub,
                    RTI_RS_MQTT_MessageReader_on_mqtt_data_available,
                    reader)) {
            /* TODO Log error */
            goto done;
        }
    }

    *reader_out = reader;

    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (reader->config != NULL) {
            RTI_RS_MQTT_MessageReaderConfig_delete(reader->config);
        }
        if (reader->sub != NULL) {
            RTI_MQTT_Client_unsubscribe(
                    reader->connection->client,
                    reader->sub);
        }

        RTI_MQTT_Heap_free(reader);
    }

    return retval;
}

DDS_ReturnCode_t RTI_RS_MQTT_MessageReader_new_discovery(
        struct RTI_RS_MQTT_BrokerConnection *connection,
        const struct RTI_RoutingServiceStreamReaderListener *listener,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_MessageReader **reader_out)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_new_discovery)

    return RTI_RS_MQTT_MessageReader_new_internal(
            connection,
            listener,
            properties,
            env,
            DDS_BOOLEAN_TRUE,
            reader_out);
}

DDS_ReturnCode_t RTI_RS_MQTT_MessageReader_new(
        struct RTI_RS_MQTT_BrokerConnection *connection,
        const struct RTI_RoutingServiceStreamReaderListener *listener,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_MessageReader **reader_out)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_new)

    return RTI_RS_MQTT_MessageReader_new_internal(
            connection,
            listener,
            properties,
            env,
            DDS_BOOLEAN_FALSE,
            reader_out);
}

void RTI_RS_MQTT_MessageReader_delete(struct RTI_RS_MQTT_MessageReader *reader)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_delete)

    if (reader->config != NULL) {
        RTI_RS_MQTT_MessageReaderConfig_delete(reader->config);
    }
    if (reader->sub != NULL) {
        RTI_MQTT_Client_unsubscribe(reader->connection->client, reader->sub);
    }

    if (!DDS_SampleInfoSeq_finalize(&reader->info_seq)) {
        /* TODO Log error */
    }

    if (!RTI_MQTT_DDS_SampleInfoPtrSeq_finalize(&reader->info_ptr_seq)) {
        /* TODO Log error */
    }

    RTI_MQTT_Heap_free(reader);
}

void RTI_RS_MQTT_MessageReader_update(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_update)
    /* TODO Implement me */
}

static void RTI_RS_MQTT_MessageReader_read_discovery(
        struct RTI_RS_MQTT_MessageReader *self,
        RTI_RoutingServiceSample **samples_list_out,
        RTI_RoutingServiceSampleInfo **info_list_out,
        int *count,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean messages_read = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong tot_streams = 0, i = 0, allocd_messages = 0;
    struct RTI_RoutingServiceStreamInfo **samples_list = NULL;
    DDS_Boolean input = (self->connection->disc_reader_in == self);

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_read_discovery)

    *samples_list_out = NULL;
    *info_list_out = NULL;
    *count = 0;

    samples_list =
            (struct RTI_RoutingServiceStreamInfo **) RTI_MQTT_Heap_allocate(
                    sizeof(struct RTI_RoutingServiceStreamInfo *)
                    * tot_streams);
    if (samples_list == NULL) {
        /* TODO Log error */
        goto done;
    }
    RTI_MQTT_Memory_zero(
            samples_list,
            sizeof(struct RTI_RoutingServiceStreamInfo *) * tot_streams);

    for (i = 0; i < tot_streams; i++) {
        struct RTI_RoutingServiceStreamInfo *sample = NULL;

        samples_list[i] = sample;
        allocd_messages += 1;
    }

    *samples_list_out = (RTI_RoutingServiceSample *) samples_list;
    *count = allocd_messages;

    retval = DDS_RETCODE_OK;

done:

    if (retval != DDS_RETCODE_OK) {
        if (samples_list != NULL) {
            for (i = 0; i < allocd_messages; i++) {
            }
            RTI_MQTT_Heap_free(samples_list);
        }
    }
}

void RTI_RS_MQTT_MessageReader_read(
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceSample **samples_list_out,
        RTI_RoutingServiceSampleInfo **info_list_out,
        int *count,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_RS_MQTT_MessageReader *self =
            (struct RTI_RS_MQTT_MessageReader *) stream_reader;
    struct DDS_DynamicDataSeq messages = DDS_SEQUENCE_INITIALIZER;
    DDS_UnsignedLong i = 0;
    DDS_Boolean messages_read = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong allocd_messages = 0, messages_in_len = 0;
    DDS_DynamicData **samples_list = NULL;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_read)

    /* Check if stream_reader is a discovery reader owned by the
       connection, in which case, call the appropriate read method */
    if (RTI_RS_MQTT_MessageReader_is_discovery(self)) {
        RTI_RS_MQTT_MessageReader_read_discovery(
                self,
                samples_list_out,
                info_list_out,
                count,
                env);
        return;
    }

    *samples_list_out = NULL;
    *info_list_out = NULL;
    *count = 0;

    if (DDS_RETCODE_OK
        != RTI_MQTT_Subscription_read(
                self->sub,
                RTI_MQTT_SUBSCRIPTION_READ_LENGTH_UNLIMITED,
                &messages)) {
        /* TODO Log error */
        goto done;
    }
    messages_in_len = DDS_DynamicDataSeq_get_length(&messages);

    if (messages_in_len == 0) {
        /* Nothing to do */
        retval = DDS_RETCODE_OK;
        goto done;
    }
    messages_read = DDS_BOOLEAN_TRUE;

    samples_list = DDS_DynamicDataSeq_get_discontiguous_buffer(&messages);
    if (samples_list == NULL) {
        /* TODO Log error */
        goto done;
    }
    allocd_messages = DDS_DynamicDataSeq_get_length(&messages);

    if (!DDS_SampleInfoSeq_ensure_length(
                &self->info_seq,
                allocd_messages,
                allocd_messages)) {
        /* TODO Log error */
        goto done;
    }

    if (!RTI_MQTT_DDS_SampleInfoPtrSeq_ensure_length(
                &self->info_ptr_seq,
                allocd_messages,
                allocd_messages)) {
        /* TODO Log error */
        goto done;
    }

    for (i = 0; i < allocd_messages; i++) {
        // struct DDS_SampleInfo *info = &(infos[i]);
        struct DDS_SampleInfo *info =
                DDS_SampleInfoSeq_get_reference(&self->info_seq, i);
        struct DDS_SampleInfo **info_ref =
                RTI_MQTT_DDS_SampleInfoPtrSeq_get_reference(
                        &self->info_ptr_seq,
                        i);

        info->valid_data = DDS_BOOLEAN_TRUE;
        *info_ref = info;
    }

    *samples_list_out = (RTI_RoutingServiceSample *) samples_list;
    *info_list_out = (RTI_RoutingServiceSampleInfo *)
            RTI_MQTT_DDS_SampleInfoPtrSeq_get_contiguous_buffer(
                    &self->info_ptr_seq);
    *count = allocd_messages;

    retval = DDS_RETCODE_OK;

done:

    if (retval != DDS_RETCODE_OK) {
        if (messages_read) {
            if (DDS_RETCODE_OK
                != RTI_MQTT_Subscription_return_loan(self->sub, &messages)) {
                /* TODO Log error */
                goto done;
            }
        }
    }
}

void RTI_RS_MQTT_MessageReader_return_loan(
        RTI_RoutingServiceStreamReader stream_reader,
        RTI_RoutingServiceSample *sample_list_in,
        RTI_RoutingServiceSampleInfo *info_list,
        int count,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RTI_RS_MQTT_MessageReader *self =
            (struct RTI_RS_MQTT_MessageReader *) stream_reader;
    DDS_DynamicData **samples_list = (DDS_DynamicData **) sample_list_in;
    DDS_UnsignedLong i = 0;
    struct DDS_DynamicDataSeq messages = DDS_SEQUENCE_INITIALIZER;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageReader_return_loan)

    if (!DDS_DynamicDataSeq_loan_discontiguous(
                &messages,
                samples_list,
                count,
                count)) {
        /* TODO Log error */
        goto done;
    }
    if (DDS_RETCODE_OK
        != RTI_MQTT_Subscription_return_loan(self->sub, &messages)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:

    if (!retval) {
        /* TODO Log error */
    }
}

RTIBool RTI_RS_MQTT_MessageReaderPtr_initialize_w_params(
        struct RTI_RS_MQTT_MessageReader **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}


RTIBool RTI_RS_MQTT_MessageReaderPtr_finalize_w_params(
        struct RTI_RS_MQTT_MessageReader **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_RS_MQTT_MessageReaderPtr_copy(
        struct RTI_RS_MQTT_MessageReader **dst,
        const struct RTI_RS_MQTT_MessageReader **src)
{
    *dst = (struct RTI_RS_MQTT_MessageReader *) *src;
    return RTI_TRUE;
}

#define T struct RTI_RS_MQTT_MessageReader *
#define TSeq RTI_RS_MQTT_MessageReaderPtrSeq
#define T_initialize_w_params RTI_RS_MQTT_MessageReaderPtr_initialize_w_params
#define T_finalize_w_params RTI_RS_MQTT_MessageReaderPtr_finalize_w_params
#define T_copy RTI_RS_MQTT_MessageReaderPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T

RTIBool RTI_MQTT_DDS_SampleInfoPtr_initialize_w_params(
        struct DDS_SampleInfo **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_MQTT_DDS_SampleInfoPtr_finalize_w_params(
        struct DDS_SampleInfo **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_MQTT_DDS_SampleInfoPtr_copy(
        struct DDS_SampleInfo **dst,
        const struct DDS_SampleInfo **src)
{
    *dst = (struct DDS_SampleInfo *) *src;
    return RTI_TRUE;
}

#define T struct DDS_SampleInfo *
#define TSeq RTI_MQTT_DDS_SampleInfoPtrSeq
#define T_initialize_w_params RTI_MQTT_DDS_SampleInfoPtr_initialize_w_params
#define T_finalize_w_params RTI_MQTT_DDS_SampleInfoPtr_finalize_w_params
#define T_copy RTI_MQTT_DDS_SampleInfoPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T