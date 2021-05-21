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

#include "MessageWriter.h"
#include "BrokerConnection.h"
#include "Infrastructure.h"
#include "Properties.h"

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::RS::Writer"

DDS_ReturnCode_t RTI_RS_MQTT_MessageWriter_new(
        struct RTI_RS_MQTT_BrokerConnection *connection,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        struct RTI_RS_MQTT_MessageWriter **writer_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    struct RTI_RS_MQTT_MessageWriter *writer = NULL;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageWriter_new)

    *writer_out = NULL;

    writer = (struct RTI_RS_MQTT_MessageWriter *) RTI_MQTT_Heap_allocate(
            sizeof(struct RTI_RS_MQTT_MessageWriter));
    if (writer == NULL) {
        /* TODO Log error */
        goto done;
    }

    writer->pub = NULL;
    writer->config = NULL;

    writer->connection = connection;

    if (DDS_RETCODE_OK
        != RTI_RS_MQTT_MessageWriterConfig_parse_from_properties(
                properties,
                &writer->config)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_MQTT_Client_publish(
                writer->connection->client,
                &writer->config->pub,
                &writer->pub)) {
        /* TODO Log error */
        goto done;
    }

    *writer_out = writer;

    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (writer != NULL) {
            RTI_RS_MQTT_MessageWriter_delete(writer);
        }
    }

    return retval;
}

void RTI_RS_MQTT_MessageWriter_delete(struct RTI_RS_MQTT_MessageWriter *writer)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageWriter_delete)

    if (writer->config != NULL) {
        RTI_RS_MQTT_MessageWriterConfig_delete(writer->config);
    }
    if (writer->pub != NULL) {
        RTI_MQTT_Client_unpublish(writer->connection->client, writer->pub);
    }

    RTI_MQTT_Heap_free(writer);
}


void RTI_RS_MQTT_MessageWriter_update(
        RTI_RoutingServiceAdapterEntity entity,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageWriter_update)
    /* TODO Implement me */
}

int RTI_RS_MQTT_MessageWriter_write(
        RTI_RoutingServiceStreamWriter stream_writer,
        const RTI_RoutingServiceSample *samples_list_in,
        const RTI_RoutingServiceSampleInfo *info_list_in,
        int count,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_MQTT_MessageWriter *writer =
            (struct RTI_RS_MQTT_MessageWriter *) stream_writer;
    DDS_DynamicData **samples_list = (DDS_DynamicData **) samples_list_in;
    int written_messages = 0;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;
    DDS_Boolean failed = DDS_BOOLEAN_FALSE;
    int i = 0;

    RTI_MQTT_LOG_FN(RTI_RS_MQTT_MessageWriter_write)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = RTI_TRUE;

    for (i = 0; i < count; i++) {
        DDS_DynamicData *sample = samples_list[i];
        if (DDS_RETCODE_OK != RTI_MQTT_Publication_write(writer->pub, sample)) {
            /* TODO Log error */
            goto done;
        }
        written_messages += 1;
    }

done:

    return written_messages;
}


RTIBool RTI_RS_MQTT_MessageWriterPtr_initialize_w_params(
        struct RTI_RS_MQTT_MessageWriter **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}


RTIBool RTI_RS_MQTT_MessageWriterPtr_finalize_w_params(
        struct RTI_RS_MQTT_MessageWriter **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_RS_MQTT_MessageWriterPtr_copy(
        struct RTI_RS_MQTT_MessageWriter **dst,
        const struct RTI_RS_MQTT_MessageWriter **src)
{
    *dst = (struct RTI_RS_MQTT_MessageWriter *) *src;
    return RTI_TRUE;
}


#define T struct RTI_RS_MQTT_MessageWriter *
#define TSeq RTI_RS_MQTT_MessageWriterPtrSeq
#define T_initialize_w_params RTI_RS_MQTT_MessageWriterPtr_initialize_w_params
#define T_finalize_w_params RTI_RS_MQTT_MessageWriterPtr_finalize_w_params
#define T_copy RTI_RS_MQTT_MessageWriterPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T
