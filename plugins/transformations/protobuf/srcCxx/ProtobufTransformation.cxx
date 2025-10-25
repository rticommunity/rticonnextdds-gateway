/******************************************************************************/
/* (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#include "ProtobufTransformation.hpp"
#include "ProtobufDescriptorSet.hpp"
#include "TransformationPlatform.h"
#include "TransformationSimple.h"
#include "DynamicDataHelpers.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::protobuf"


DDS_ReturnCode_t
RTI_TSFM_ProtobufTransformationConfig_parse_from_properties(
    RTI_TSFM_ProtobufTransformationConfig *self,
    const struct RTI_RoutingServiceProperties *properties)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_TSFM_LOG_FN(
            RTI_TSFM_ProtobufTransformationConfig_parse_from_properties)

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationConfig_parse_from_properties(
                &self->parent,
                properties)) {
        /* TODO Log error */
        goto done;
    }

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_PROTO_PROPERTY_BUFFER_MEMBER,
            DDS_String_replace(&self->buffer_member, pval);
            if (self->buffer_member == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_PROTO_PROPERTY_DESCRIPTOR_FILE,
            DDS_String_replace(&self->descriptor_file, pval);
            if (self->descriptor_file == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_PROTO_PROPERTY_MESSAGE_TYPE,
            DDS_String_replace(&self->message_type, pval);
            if (self->message_type == NULL) {
                /* TODO Log error */
                goto done;
            })

    retcode = DDS_RETCODE_OK;
done:
    return retcode;
}

static
RTI_TSFM_ProtobufTransformationState *
RTI_TSFM_ProtobufTransformationState_create_data()
{
    RTI_TSFM_ProtobufTransformationState *retval = NULL, *state = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformationState_create_data)

    state = (RTI_TSFM_ProtobufTransformationState *)
            RTI_TSFM_Heap_allocate(
                    sizeof(RTI_TSFM_ProtobufTransformationState));
    if (state == NULL) {
        /* TODO Log error */
        goto done;
    }

    state->msg_converter = nullptr;
    state->message_type = nullptr;
    state->pb_buffer = nullptr;

    if (!DDS_OctetSeq_initialize(&state->octet_seq)) {
        /* TODO log error */
        goto done;
    }

    if (!DDS_CharSeq_initialize(&state->char_seq)) {
        /* TODO log error */
        goto done;
    }

    retval = state;

done:
    if (retval == NULL) {
        if (state != NULL) {
            RTI_TSFM_Heap_free(state);
        }
    }
    return retval;
}

static
void
RTI_TSFM_ProtobufTransformationState_delete_data(
    RTI_TSFM_ProtobufTransformationState *data)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformationState_delete_data)

    if (nullptr != data->msg_converter) {
        delete data->msg_converter;
        data->msg_converter = NULL;
    }
    if (nullptr != data->message_type) {
        delete data->message_type;
        data->message_type = NULL;
    }
    if (data->pb_buffer != NULL) {
        delete data->pb_buffer;
        data->pb_buffer = NULL;
    }
    DDS_OctetSeq_finalize(&data->octet_seq);
    DDS_CharSeq_finalize(&data->char_seq);
    RTI_TSFM_Heap_free(data);
}


void
RTI_TSFM_ProtobufTransformationConfigTypeSupport_delete_data(
    RTI_TSFM_ProtobufTransformationConfig *sample)
{
    /* delete parent */
    if (sample->parent.input_type != NULL) {
        DDS_String_free(sample->parent.input_type);
        sample->parent.input_type = NULL;
    }
    if (sample->parent.output_type != NULL) {
        DDS_String_free(sample->parent.output_type);
        sample->parent.output_type = NULL;
    }

    /* Delete the rest of fields*/
    if (sample->buffer_member != NULL) {
        DDS_String_free(sample->buffer_member);
        sample->buffer_member = NULL;
    }

    if (sample->descriptor_file != NULL) {
        DDS_String_free(sample->descriptor_file);
        sample->descriptor_file = NULL;
    }

    if (sample->message_type != NULL) {
        DDS_String_free(sample->message_type);
        sample->message_type = NULL;
    }

    RTIOsapiHeap_freeStructure(sample);
    sample = NULL;
}

RTI_TSFM_ProtobufTransformationConfig *
RTI_TSFM_ProtobufTransformationConfigTypeSupport_create_data(void)
{
    RTI_TSFM_ProtobufTransformationConfig *sample = NULL;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTIOsapiHeap_allocateStructure(
            &(sample),
            RTI_TSFM_ProtobufTransformationConfig);
    if (sample == NULL) {
        return NULL;
    }

    sample->buffer_member = DDS_String_alloc(0);
    RTICdrType_copyStringEx(&sample->buffer_member, "", 0, RTI_FALSE);
    if (sample->buffer_member == NULL) {
        goto done;
    }

    sample->descriptor_file = DDS_String_alloc(0);
    RTICdrType_copyStringEx(&sample->descriptor_file, "", 0, RTI_FALSE);
    if (sample->descriptor_file == NULL) {
        goto done;
    }

    sample->message_type = DDS_String_alloc(0);
    RTICdrType_copyStringEx(&sample->message_type, "", 0, RTI_FALSE);
    if (sample->message_type == NULL) {
        goto done;
    }

    ok = DDS_BOOLEAN_TRUE;
done:
    if (!ok) {
        RTI_TSFM_ProtobufTransformationConfigTypeSupport_delete_data(
                sample);
        sample = NULL;
    }
    return sample;
}

#define RTI_TSFM_ProtobufTransformation_validate_container_tckind(tck_) \
    ((tck_) == DDS_TK_STRUCT || (tck_) == DDS_TK_VALUE)

static DDS_ReturnCode_t
RTI_TSFM_ProtobufTransformation_validate_buffer_member(
    RTI_TSFM_ProtobufTransformation *self,
    const struct DDS_TypeCode *base_type,
    const char *member_name)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    const struct DDS_TypeCode *member_type = NULL, *member_content_type = NULL;
    DDS_TCKind tckind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformation_validate_buffer_member)

    /* Check that output type has the specified buffer member */
    member_type = RTI_COMMON_TypeCode_get_member_type(base_type, member_name);
    if (member_type == NULL) {
        goto done;
    }

    tckind = DDS_TypeCode_kind(member_type, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        /* TODO Log error */
        goto done;
    }

    /*
     * Validate the member type which must be an array, a sequence or a string.
     * Further validation of the elements of "container" types will be performed
     * later if needed.
     */
    switch (tckind) {
    case DDS_TK_ARRAY: {
        DDS_UnsignedLong array_dimensions = 0;
        /* If the TC is array, check it has only one dimension */
        array_dimensions = DDS_TypeCode_array_dimension_count(member_type, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            RTI_TSFM_ERROR_1("Getting dimensions for array ", "<%s>", member_name)
            goto done;
        }
        if (array_dimensions != 1) {
            RTI_TSFM_ERROR_1(
                "Container arrays should have one dimension, member ",
                "<%s>",
                member_name)
            goto done;
        }
        break;
    }
    case DDS_TK_SEQUENCE:
        /* No further validation for type */
        break;

    default:
        RTI_TSFM_ERROR_1("Incompatible datatype for member ", "<%s>", member_name)
        goto done;
    }

    /*
     * In case of a Sequence/Array, check that the inner type is DDS_Octet
     * or DDS_Char
     */
    switch (tckind) {
    case DDS_TK_ARRAY:
    case DDS_TK_SEQUENCE:
        member_content_type = DDS_TypeCode_content_type(member_type, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }

        tckind = DDS_TypeCode_kind(member_content_type, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }

        if (tckind != DDS_TK_OCTET && tckind != DDS_TK_CHAR) {
            /* TODO Log error */
            goto done;
        }
        break;

    default:
        /* nothing to do */
        break;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}


static DDS_ReturnCode_t
RTI_TSFM_ProtobufTransformation_validate_input_type(
    RTI_TSFM_ProtobufTransformation *self,
    const struct DDS_TypeCode *input_type)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformation_validate_input_type)

    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /* Nothing to check for serializer mode */
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    retcode = RTI_TSFM_ProtobufTransformation_validate_buffer_member(
                self,
                input_type,
                self->config->buffer_member);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO Log error */
        goto done;
    }

done:
    return retcode;
}

static DDS_ReturnCode_t
RTI_TSFM_ProtobufTransformation_validate_output_type(
    RTI_TSFM_ProtobufTransformation *self,
    const struct DDS_TypeCode *output_type)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_UnsignedLong members_count = 0, i = 0, buffer_member_id = 0;
    const struct DDS_TypeCode *member_type = NULL, *member_content_type = NULL;
    DDS_TCKind tckind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformation_validate_output_type)

    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /* Check that output type has the specified buffer member */
        if (DDS_RETCODE_OK
            != RTI_TSFM_ProtobufTransformation_validate_buffer_member(
                    self,
                    output_type,
                    self->config->buffer_member)) {
            /* TODO Log error */
            goto done;
        }

        retcode = DDS_RETCODE_OK;
        goto done;
    }

    /* Configure output for deserializer mode */

    /* Check that the base type is supported (struct or value) */
    tckind = DDS_TypeCode_kind(output_type, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        /* TODO Log error */
        goto done;
    }
    if (!RTI_TSFM_ProtobufTransformation_validate_container_tckind(tckind)) {
        /* TODO Log error */
        goto done;
    }

    members_count = DDS_TypeCode_member_count(output_type, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        /* TODO Log error */
        goto done;
    }

    if (members_count == 0) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}


DDS_Boolean RTI_TSFM_ProtobufTransformation_preallocate_buffers(
        RTI_TSFM_ProtobufTransformation *self,
        const struct DDS_TypeCode *tc)
{
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_TCKind member_kind = DDS_TK_NULL;
    DDS_UnsignedLong buffer_size = 0;
    const DDS_TypeCode *member_tc = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformation_preallocate_buffers)

    member_tc = RTI_COMMON_TypeCode_get_member_type(tc, self->config->buffer_member);
    if (member_tc == NULL) {
        /* TODO Log error */
        goto done;
    }

    buffer_size = DDS_TypeCode_length(member_tc, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        /* TODO Log error */
        goto done;
    }

    member_kind = DDS_TypeCode_kind(member_tc, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        /* TODO Log error */
        goto done;
    }

    /* Preallocate the pb_buffer if it is not a sequence in deserialize mode */
    self->state->pb_buffer = new(std::nothrow) std::string();
    if (self->state->pb_buffer == NULL) {
        /* TODO Log error */
        goto done;
    }

    if (buffer_size < RTI_INT32_MAX
        && self->state->pb_buffer == NULL
        && (member_kind != DDS_TK_SEQUENCE
            || self->config->parent.type != RTI_TSFM_TransformationKind_DESERIALIZER)) {
        try {
            self->state->pb_buffer->resize(buffer_size);
        } catch (... ) {
            /* TODO Log error */
            goto done;
        }
    }

    /*
     * We cannot preallocate sequences because we do a loan_contiguous in
     * the serialization method and it requires that the sequence has 0 size.
     * Also, the deserialize mode will get the sequences from the DynamicData
     */

    ok = DDS_BOOLEAN_TRUE;

done:
    return ok;
}

DDS_ReturnCode_t
RTI_TSFM_ProtobufTransformation_initialize(
    RTI_TSFM_ProtobufTransformation *self,
    RTI_TSFM_ProtobufTransformationPlugin *plugin,
    const struct RTI_RoutingServiceTypeInfo *input_type_info,
    const struct RTI_RoutingServiceTypeInfo *output_type_info,
    const struct RTI_RoutingServiceProperties *properties,
    RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    const struct DDS_TypeCode *tc = NULL;
    const struct RTI_RoutingServiceTypeInfo *serialized_type_info = NULL;
    const struct RTI_RoutingServiceTypeInfo *dds_type_info = NULL;
    DDS_TCKind member_kind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformation_initialize)

    retcode = RTI_TSFM_Transformation_initialize(
            &self->parent,
            &plugin->parent,
            input_type_info,
            output_type_info,
            properties,
            env);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO log error */
        goto done;
    }

    if (input_type_info->type_representation_kind
            != RTI_ROUTING_SERVICE_TYPE_REPRESENTATION_DYNAMIC_TYPE) {
        /* TODO Log error */
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    tc = (const struct DDS_TypeCode *) input_type_info->type_representation;

    retcode = RTI_TSFM_ProtobufTransformation_validate_input_type(self, tc);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO Log error */
        goto done;
    }

    tc = (const struct DDS_TypeCode *) output_type_info->type_representation;

    retcode = RTI_TSFM_ProtobufTransformation_validate_output_type(self, tc);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO Log error */
        goto done;
    }

    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        serialized_type_info = output_type_info;
        dds_type_info = input_type_info;    
    } else {
        /*
         * If we are serializing from DynamicData to Protobuf, the TypeCode is
         * the ouput
         */
        serialized_type_info = input_type_info;
        dds_type_info = output_type_info;
    }

    tc = (const struct DDS_TypeCode *) serialized_type_info->type_representation;
    if (!RTI_TSFM_ProtobufTransformation_preallocate_buffers(self, tc)) {
        /* TODO Log error */
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    if (self->config->message_type != nullptr && self->config->message_type[0] != '\0') {
        self->state->message_type = new(std::nothrow) std::string(self->config->message_type);
    } else {
        self->state->message_type = new(std::nothrow) std::string(dds_type_info->type_name);
        // generate PB type name from DDS type name by replacing "::" with "."
        size_t pos = 0;
        while ((pos = self->state->message_type->find("::", pos)) != std::string::npos) {
            self->state->message_type->replace(pos, 2, ".");
            pos += 1;
        }
    }
    if (self->state->message_type == NULL) {
        /* TODO Log error */
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    /* Create the MessageConverter */
    try {
        using namespace rti::dyndata::protobuf;
        google::protobuf::FileDescriptorSet pbDescriptorSet =
            parse_descriptor_set(self->config->descriptor_file);
        self->state->msg_converter = new ProtobufDynamicDataConverter(
                pbDescriptorSet,
                *self->state->message_type);
    } catch (... ) {
        RTI_TSFM_ERROR("error creating Protobuf MessageConverter")
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    retcode = DDS_RETCODE_OK;
done:
    return retcode;
}


DDS_ReturnCode_t RTI_TSFM_ProtobufTransformation_serialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_ProtobufTransformation *self =
            (RTI_TSFM_ProtobufTransformation *) transform;
    DDS_Boolean serialized = DDS_BOOLEAN_FALSE,
                buffer_seq_initd = DDS_BOOLEAN_FALSE,
                failed_serialization = DDS_BOOLEAN_FALSE;
    size_t serialized_size = 0,
            max_serialized_size = 0;
    struct DDS_DynamicDataMemberInfo member_info =
            DDS_DynamicDataMemberInfo_INITIALIZER;

    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformation_serialize)

    if (!self->state->msg_converter->serialize(
            *sample_in,
            *self->state->pb_buffer)) {
        RTI_TSFM_ERROR("error transforming to DDS sample to Protobuf")
        goto done;
    }

    /*
     * Select the method to set the output depending on the type:
     * DDS_OctetSeq or string.
     */
    if (DDS_RETCODE_OK !=
        DDS_DynamicData_get_member_info(
            sample_out,
            &member_info,
            self->config->buffer_member,
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        RTI_TSFM_ERROR_1(
                "unable to get_member_info for member",
                "%s",
                self->config->buffer_member)
        goto done;
    }

    switch (member_info.member_kind) {
    case DDS_TK_SEQUENCE: {
        switch (member_info.element_kind) {

        case DDS_TK_CHAR:
            retcode = RTI_COMMON_DynamicData_set_char_seq_from_string(
                    sample_out,
                    &self->state->char_seq,
                    self->config->buffer_member,
                    self->state->pb_buffer->data(),
                    self->state->pb_buffer->size(),
                    self->state->pb_buffer->length());
            if (retcode != DDS_RETCODE_OK) {
                RTI_TSFM_ERROR_1(
                        "unable to set_char_seq_from_string for member",
                        "%s",
                        self->config->buffer_member)
                goto done;
            }
            break;

        case DDS_TK_OCTET:
            retcode = RTI_COMMON_DynamicData_set_octet_seq_from_string(
                    sample_out,
                    &self->state->octet_seq,
                    self->config->buffer_member,
                    self->state->pb_buffer->data(),
                    self->state->pb_buffer->size(),
                    self->state->pb_buffer->length());
            if (retcode != DDS_RETCODE_OK) {
                RTI_TSFM_ERROR_1(
                        "unable to set_octet_seq_from_string for member",
                        "%s",
                        self->config->buffer_member)
                goto done;
            }
            break;

        default:
            RTI_TSFM_ERROR_1(
                "sequence member_kind not supported",
                "%d",
                member_info.element_kind);
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }
    break;

    case DDS_TK_ARRAY:
        switch (member_info.element_kind) {
        case DDS_TK_CHAR:
            retcode = DDS_DynamicData_set_char_array(
                    sample_out,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    self->state->pb_buffer->length(),
                    self->state->pb_buffer->data());
            if (retcode != DDS_RETCODE_OK) {
                RTI_TSFM_ERROR_1(
                    "unable to set_char_array for member",
                    "%s",
                    self->config->buffer_member)
                goto done;
            }
            break;

        case DDS_TK_OCTET:
            retcode = DDS_DynamicData_set_octet_array(
                    sample_out,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    self->state->pb_buffer->length(),
                    reinterpret_cast<const DDS_Octet*>(self->state->pb_buffer->data()));
            if (retcode != DDS_RETCODE_OK) {
                RTI_TSFM_ERROR_1(
                    "unable to set_octet_array for member",
                    "%s",
                    self->config->buffer_member)
                goto done;
            }
            break;

        default:
            RTI_TSFM_ERROR_1(
                "array member_kind not supported",
                "%d",
                member_info.element_kind);
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
        break;

    default:
        RTI_TSFM_ERROR_1(
                "incompatible kind of member (only DDS_CharSeq and DDS_OctetSeq are supported):",
                "%s",
                self->config->buffer_member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    retcode = DDS_RETCODE_OK;
done:

    RTI_TSFM_TRACE_1(
            "RTI_TSFM_ProtobufTransformation_serialize:",
            "retcode=%d",
            retcode)

    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_ProtobufTransformation_deserialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_ProtobufTransformation *self =
            (RTI_TSFM_ProtobufTransformation *) transform;
    char *buffer = NULL;
    DDS_UnsignedLong buffer_len = 0;
    struct DDS_DynamicDataMemberInfo member_info =
            DDS_DynamicDataMemberInfo_INITIALIZER;

    RTI_TSFM_LOG_FN(RTI_TSFM_ProtobufTransformation_deserialize)

    retcode = DDS_DynamicData_get_member_info(
            sample_in,
            &member_info,
            self->config->buffer_member,
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_ERROR_1(
                "unable to get_member_info for member",
                "%s",
                self->config->buffer_member)
        goto done;
    }

    switch (member_info.member_kind) {
    case DDS_TK_ARRAY: {
        buffer = &(*self->state->pb_buffer)[0];

        switch (member_info.element_kind) {
        case DDS_TK_CHAR:
            retcode = DDS_DynamicData_get_char_array(
                    sample_in,
                    buffer,
                    &buffer_len,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            break;

        case DDS_TK_OCTET:
            retcode = DDS_DynamicData_get_octet_array(
                    sample_in,
                    reinterpret_cast<DDS_Octet*>(buffer),
                    &buffer_len,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            break;

        default:
             RTI_TSFM_ERROR_1(
                "unsupported datatype for array, member",
                "%s",
                self->config->buffer_member)
            goto done;
        }

        if (retcode != DDS_RETCODE_OK) {
            RTI_TSFM_ERROR_1(
                    "unable to get_array from DynamicData. Check that it has "
                            "enough space",
                    "member <%s>",
                    self->config->buffer_member)
            goto done;
        }

        break;
    }
    case DDS_TK_SEQUENCE:
        switch (member_info.element_kind) {
        case DDS_TK_CHAR:
            retcode = RTI_COMMON_DynamicData_get_char_seq_contiguous_buffer(
                    sample_in,
                    &buffer,
                    &buffer_len,
                    &self->state->char_seq,
                    self->config->buffer_member);
            break;

        case DDS_TK_OCTET:
            retcode = RTI_COMMON_DynamicData_get_octet_seq_contiguous_buffer(
                    sample_in,
                    &buffer,
                    &buffer_len,
                    &self->state->octet_seq,
                    self->config->buffer_member);
            break;

        default:
            RTI_TSFM_ERROR_1(
                "unsupported datatype for array, member",
                "%s",
                self->config->buffer_member)
            goto done;
        }

        if (retcode != DDS_RETCODE_OK) {
            RTI_TSFM_ERROR_1(
                    "unable to get_contiguous_buffer from DynamicData member",
                    "%s",
                    self->config->buffer_member)
            goto done;
        }
        break;

    default:
        RTI_TSFM_ERROR_1(
                "incompatible kind (only Strings and DDS_OctetSeq supported) "
                        "of member : ",
                "%s",
                self->config->buffer_member)
        goto done;
    }

    if (!self->state->msg_converter->deserialize(
            *sample_out,
            buffer,
            buffer_len)) {
        RTI_TSFM_ERROR("unable to transform Protobuf message to DDS sample")
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    RTI_TSFM_TRACE_1(
            "RTI_TSFM_ProtobufTransformation_deserialize:",
            "retcode=%d",
            retcode)

    return retcode;
}


#define T RTI_TSFM_ProtobufTransformation
#define T_initialize RTI_TSFM_ProtobufTransformation_initialize
#define TConfig RTI_TSFM_ProtobufTransformationConfig
#define TState RTI_TSFM_ProtobufTransformationState
#define TState_new RTI_TSFM_ProtobufTransformationState_create_data
#define TState_delete RTI_TSFM_ProtobufTransformationState_delete_data
#define T_static
#include "TransformationTemplateDefine.h"
