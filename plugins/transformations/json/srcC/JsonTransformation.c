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

#include "ndds/ndds_c.h"

#include "JsonTransformationInfrastructure.h"
#include "TransformationPlatform.h"
#include "TransformationSimple.h"
#include "DynamicDataHelpers.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::json"

#define RTI_TSFM_JsonTransformation_validate_container_tckind(tck_) \
    ((tck_) == DDS_TK_STRUCT || (tck_) == DDS_TK_VALUE)

static DDS_ReturnCode_t
        RTI_TSFM_JsonTransformation_validate_buffer_member(
                RTI_TSFM_JsonTransformation *self,
                struct DDS_TypeCode *base_type,
                const char *member_name)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    struct DDS_TypeCode *member_type = NULL, *member_content_type = NULL;
    DDS_TCKind tckind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformation_validate_buffer_member)

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


    switch (tckind) {
    case DDS_TK_ARRAY: {
        DDS_UnsignedLong array_dimensions = 0;
        /* If the TC is array, check it has only one dimension */
        array_dimensions = DDS_TypeCode_array_dimension_count(member_type, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }
        if (array_dimensions != 1) {
            /* TODO Log error */
            goto done;
        }
    }

    /* Do not break because checking the inner datatype is the same for seqs */
    case DDS_TK_SEQUENCE:
        /*
         * In case of a Sequence/Array, check that the inner type is DDS_Octet
         * or DDS_Char
         */
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

    case DDS_TK_STRING:
        /* Do nothing */
        break;

    default:
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}


static DDS_ReturnCode_t
        RTI_TSFM_JsonTransformation_validate_input_type(
                RTI_TSFM_JsonTransformation *self,
                struct DDS_TypeCode *input_type)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformation_validate_input_type)

    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /* Nothing to check for serializer mode, since we can serialize any
         * DDS type using DDS_DynamicData_print_json */
        goto done; /* DDS_RETCODE_OK */
    }

    retcode = RTI_TSFM_JsonTransformation_validate_buffer_member(
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
        RTI_TSFM_JsonTransformation_validate_output_type(
                RTI_TSFM_JsonTransformation *self,
                struct DDS_TypeCode *output_type)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_UnsignedLong members_count = 0, i = 0, buffer_member_id = 0;
    struct DDS_TypeCode *member_type = NULL, *member_content_type = NULL;
    DDS_TCKind tckind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformation_validate_output_type)

    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /* Check that output type has the specified buffer member */
        if (DDS_RETCODE_OK
            != RTI_TSFM_JsonTransformation_validate_buffer_member(
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
    if (!RTI_TSFM_JsonTransformation_validate_container_tckind(tckind)) {
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

DDS_Boolean RTI_TSFM_JsonTransformation_preallocate_buffers(
        RTI_TSFM_JsonTransformation *self,
        struct DDS_TypeCode *tc)
{
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_TCKind member_kind = DDS_TK_NULL;
    DDS_TypeCode *content_tc = NULL;
    DDS_TCKind content_kind = DDS_TK_NULL;
    DDS_UnsignedLong length = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformation_preallocate_buffers)

     /* Initialize buffers. The transformation will only use one of them */
    self->state->json_buffer = NULL;
    self->state->json_buffer_size = 0;

    if (!DDS_OctetSeq_initialize(&self->state->octet_seq)) {
        /* TODO log error */
        goto done;
    }

    if (!DDS_CharSeq_initialize(&self->state->char_seq)) {
        /* TODO log error */
        goto done;
    }

    /*
     * Preallocate initial serealized size depending on the type that
     * will be used.
     * If the member is unbounded, it will use initial_serialized_size_for_unbounded
     * If the member is bounded, the maximum size will be allocated.
     */
    tc = RTI_COMMON_TypeCode_get_member_type(tc, self->config->buffer_member);
    if (tc == NULL) {
        /* TODO Log error */
        goto done;
    }

    /* Get the length of arrays, or maximum length for strings/sequences */
    self->state->json_buffer_bound = DDS_TypeCode_length(tc, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        /* TODO Log error */
        goto done;
    }

    /* Get the the maximum (bounded) or the initial unbounded size. */
    if (self->state->json_buffer_bound == RTI_INT32_MAX) {
        length = self->config->initial_serialized_size_for_unbounded;
    } else {
        /* Bounded string/sequences and arrays */
        length = self->state->json_buffer_bound;
        self->config->initial_serialized_size_for_unbounded = 0;
    }

    member_kind = DDS_TypeCode_kind(tc, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        /* TODO Log error */
        goto done;
    }

    /* Preallocate the json_buffer if it is not a sequence in deserialize mode */
    if (member_kind != DDS_TK_SEQUENCE
            || self->config->parent.type != RTI_TSFM_TransformationKind_DESERIALIZER) {
        if (self->state->json_buffer == NULL) {
            self->state->json_buffer = DDS_String_alloc(length);
            if (self->state->json_buffer == NULL) {
                /* TODO Log error */
                goto done;
            }
            self->state->json_buffer_size = length;
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

DDS_ReturnCode_t RTI_TSFM_JsonTransformation_initialize(
        RTI_TSFM_JsonTransformation *self,
        RTI_TSFM_JsonTransformationPlugin *plugin,
        const struct RTI_RoutingServiceTypeInfo *input_type_info,
        const struct RTI_RoutingServiceTypeInfo *output_type_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_TypeCode *tc = NULL;
    DDS_TCKind member_kind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformation_initialize)

    retcode = RTI_TSFM_Transformation_initialize(
            &self->parent,
            &plugin->parent,
            input_type_info,
            output_type_info,
            properties,
            env);
    if(retcode != DDS_RETCODE_OK) {
        /* TODO log error */
        goto done;
    }

    if (input_type_info->type_representation_kind
            != RTI_ROUTING_SERVICE_TYPE_REPRESENTATION_DYNAMIC_TYPE) {
        /* TODO Log error */
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    tc = (struct DDS_TypeCode *) input_type_info->type_representation;

    retcode = RTI_TSFM_JsonTransformation_validate_input_type(self, tc);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO Log error */
        goto done;
    }

    tc = (struct DDS_TypeCode *) output_type_info->type_representation;

    retcode = RTI_TSFM_JsonTransformation_validate_output_type(self, tc);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO Log error */
        goto done;
    }

    /* Preallocate json_buffer depending on the type that will be used. */
    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /*
         * If we are serializing from DynamicData to JSON, the TypeCode is
         * the ouput
         */
        if (!RTI_TSFM_JsonTransformation_preallocate_buffers(
                self,
                (struct DDS_TypeCode *) output_type_info->type_representation)) {
            /* TODO Log error */
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    } else {
        /*
         * If we are serializing from DynamicData to JSON, the TypeCode is
         * the ouput
         */
        if (!RTI_TSFM_JsonTransformation_preallocate_buffers(
                self,
                (struct DDS_TypeCode *) input_type_info->type_representation)) {
            /* TODO Log error */
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }

done:
    return retcode;
}


DDS_ReturnCode_t
        RTI_TSFM_JsonTransformationConfig_parse_from_properties(
                RTI_TSFM_JsonTransformationConfig *self,
                const struct RTI_RoutingServiceProperties *properties)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_TSFM_LOG_FN(
            RTI_TSFM_JsonTransformationConfig_parse_from_properties)

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationConfig_parse_from_properties(
                &self->parent,
                properties)) {
        /* TODO Log error */
        goto done;
    }

    self->initial_serialized_size_for_unbounded =
            RTI_TSFM_JSON_INITIAL_SERIALIZED_SIZE_FOR_UNBOUNDED_DEFAULT;
    self->indent = RTI_TSFM_JSON_INDENT_DEFAULT;

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_PROPERTY_BUFFER_MEMBER,
            DDS_String_replace(&self->buffer_member, pval);
            if (self->buffer_member == NULL) {
                /* TODO Log error */
                goto done;
            })

    /* For backward compatibility */
    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_PROPERTY_SERIALIZED_SIZE_MIN,
            RTI_TSFM_WARNING_2(
                    "Property deprecated",
                    "<%s>, use <%s> instead.",
                    RTI_TSFM_JSON_PROPERTY_SERIALIZED_SIZE_MIN,
                    RTI_TSFM_JSON_PROPERTY_INITIAL_SERIALIZED_SIZE_FOR_UNBOUNDED)
            self->initial_serialized_size_for_unbounded =
                    RTI_TSFM_String_to_long(pval, NULL, 0);)

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_PROPERTY_INITIAL_SERIALIZED_SIZE_FOR_UNBOUNDED,
            self->initial_serialized_size_for_unbounded =
                    RTI_TSFM_String_to_long(pval, NULL, 0);)

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_PROPERTY_INDENT,
            self->indent = RTI_TSFM_String_to_long(pval, NULL, 0);)

    retcode = DDS_RETCODE_OK;
done:
    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_JsonTransformation_serialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_JsonTransformation *self =
            (RTI_TSFM_JsonTransformation *) transform;
    DDS_Boolean serialized = DDS_BOOLEAN_FALSE,
                buffer_seq_initd = DDS_BOOLEAN_FALSE,
                failed_serialization = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong serialized_size = 0;
    struct DDS_DynamicDataMemberInfo member_info =
            DDS_DynamicDataMemberInfo_INITIALIZER;
    char *p = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformation_serialize)

    do {
        serialized_size = self->state->json_buffer_size;
        /*
         * Serializing from DynamicData to JSON will always use
         * self->state->json_buffer because to_json function returns a string.
         * serialized_size will contain the actual size of the json_buffer
         * string
         */
        retcode = DDS_DynamicDataFormatter_to_json(
                sample_in,
                self->state->json_buffer,
                &serialized_size,
                self->config->indent);
        if (retcode == DDS_RETCODE_OUT_OF_RESOURCES) {
            /* Error if it is bounded because the buffer is not big enough */
            if (self->state->json_buffer_bound != RTI_INT32_MAX) {
                RTI_TSFM_ERROR("not enough space in the json_buffer")
                goto done;
            }
            /*
             * If the datatype is unbounded, realloc the serialized_size
             * with the serialized_size which already contains the '\0'
             */
            DDS_String_free(self->state->json_buffer);
            self->state->json_buffer = DDS_String_alloc(
                    RTI_TSFM_JSON_BUFFER_SIZE_INCREMENT(serialized_size));
            if (self->state->json_buffer == NULL) {
                RTI_TSFM_ERROR("error allocating json_buffer")
                goto done;
            }
            self->state->json_buffer_size =
                    RTI_TSFM_JSON_BUFFER_SIZE_INCREMENT(serialized_size);
        } else if (retcode != DDS_RETCODE_OK) {
            RTI_TSFM_ERROR("error transforming to JSON")
            goto done;
        }
    } while (retcode != DDS_RETCODE_OK);

    if (self->config->indent == 0) {
        /* Replace all '\n' with a space */
        for (p = self->state->json_buffer; (p = strchr(p, '\n')) != NULL;
                p++) {
            *p = ' ';
        }
    }

    /*
     * Select the method to set the output depending on the type:
     * DDS_OctetSeq or string.
     */
    retcode = DDS_DynamicData_get_member_info(
            sample_out,
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
    case DDS_TK_STRING:
        retcode = DDS_DynamicData_set_string(
                    sample_out,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    self->state->json_buffer);
        if (retcode != DDS_RETCODE_OK) {
            RTI_TSFM_ERROR_1(
                    "unable to set_string for member",
                    "%s",
                    self->config->buffer_member)
            goto done;
        }
        break;

    case DDS_TK_SEQUENCE: {
        switch (member_info.element_kind) {

        case DDS_TK_CHAR:
            retcode = RTI_COMMON_DynamicData_set_char_seq_from_string(
                    sample_out,
                    &self->state->char_seq,
                    self->config->buffer_member,
                    self->state->json_buffer,
                    serialized_size);
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
                    self->state->json_buffer,
                    serialized_size);
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
                    self->state->json_buffer_size,
                    (const DDS_Char *) self->state->json_buffer);
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
                    self->state->json_buffer_size,
                    (const DDS_Octet *) self->state->json_buffer);
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
                "incompatible kind (only Strings and DDS_OctetSeq are "
                        "supported) of member : ",
                "%s",
                self->config->buffer_member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    retcode = DDS_RETCODE_OK;
done:

    RTI_TSFM_TRACE_1(
            "RTI_TSFM_JsonTransformation_serialize:",
            "retcode=%d",
            retcode)

    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_JsonTransformation_deserialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_JsonTransformation *self =
            (RTI_TSFM_JsonTransformation *) transform;
    char *buffer = NULL;
    DDS_UnsignedLong buffer_seq_max = 0, buffer_seq_len = 0;
    struct DDS_DynamicDataMemberInfo member_info =
            DDS_DynamicDataMemberInfo_INITIALIZER;

    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformation_deserialize)

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
        /*
         * current_len is needed because the get_<type>_array will modify it and
         * we don't want to modify state->json_buffer_size
         */
        DDS_UnsignedLong current_len = self->state->json_buffer_size;
        switch (member_info.element_kind) {
        case DDS_TK_CHAR:
            retcode = DDS_DynamicData_get_char_array(
                    sample_in,
                    (DDS_Char *) self->state->json_buffer,
                    &current_len,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            break;

        case DDS_TK_OCTET:
            retcode = DDS_DynamicData_get_octet_array(
                    sample_in,
                    (DDS_Octet *) self->state->json_buffer,
                    &current_len,
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

        buffer = self->state->json_buffer;

        break;
    }
    case DDS_TK_STRING:
        do {
            DDS_ReturnCode_t aux_retcode = DDS_RETCODE_OK;
            /*
             * current_len is needed because the get_string will modify it and
             * we don't want to modify state->json_buffer_size
             */
            DDS_UnsignedLong current_len = self->state->json_buffer_size;
            /*
             * Get the string or the array buffer (DDS_Char/DDS_Octet), if
             * json_buffer doesn't have enough space, realloc it
             */
            retcode = DDS_DynamicData_get_string(
                    sample_in,
                    &self->state->json_buffer,
                    &current_len,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);

            /*
             * the DDS_RETCODE_PRECONDITION_NOT_MET means that there is no
             * enough space to save the string and the current_len contains
             * the size that we need (including '\0')
             */
            if (retcode == DDS_RETCODE_PRECONDITION_NOT_MET) {
                /* Error if it is bounded because the buffer is not big enough */
                if (self->state->json_buffer_bound != RTI_INT32_MAX) {
                    RTI_TSFM_ERROR("not enough space in the json_buffer")
                    goto done;
                }
                DDS_String_free(self->state->json_buffer);
                self->state->json_buffer = DDS_String_alloc(
                        RTI_TSFM_JSON_BUFFER_SIZE_INCREMENT(current_len));
                if (aux_retcode != DDS_RETCODE_OK) {
                    RTI_TSFM_ERROR("unable to realloc_buffer")
                    goto done;
                }
                self->state->json_buffer_size =
                        RTI_TSFM_JSON_BUFFER_SIZE_INCREMENT(current_len);
            } else if (retcode != DDS_RETCODE_OK) {
                RTI_TSFM_ERROR_1(
                    "unable to get_string from DynamicData member",
                    "%s",
                    self->config->buffer_member)
                goto done;
            }
        } while (retcode != DDS_RETCODE_OK);

        buffer = self->state->json_buffer;

        break;

    case DDS_TK_SEQUENCE:
        switch (member_info.element_kind) {
        case DDS_TK_CHAR:
            retcode = RTI_COMMON_DynamicData_get_char_seq_contiguous_buffer(
                    sample_in,
                    &buffer,
                    &self->state->char_seq,
                    self->config->buffer_member);
            break;

        case DDS_TK_OCTET:
            retcode = RTI_COMMON_DynamicData_get_octet_seq_contiguous_buffer(
                    sample_in,
                    &buffer,
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

    retcode = DDS_DynamicDataFormatter_from_json(sample_out, buffer);
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_ERROR("unable to format from json")
        goto done;
    }

done:

    RTI_TSFM_TRACE_1(
            "RTI_TSFM_JsonTransformation_deserialize:",
            "retcode=%d",
            retcode)

    return retcode;
}


static RTI_TSFM_JsonTransformationState *
        RTI_TSFM_JsonTransformationState_create_data()
{
    RTI_TSFM_JsonTransformationState *retval = NULL, *state = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformationState_create_data)

    state = (RTI_TSFM_JsonTransformationState *)
            RTI_TSFM_Heap_allocate(
                    sizeof(RTI_TSFM_JsonTransformationState));
    if (state == NULL) {
        /* TODO Log error */
        goto done;
    }

    state->json_buffer = NULL;
    state->json_buffer_size = 0;
    state->json_buffer_bound = 0;

    retval = state;

done:
    if (retval == NULL) {
        if (state != NULL) {
            RTI_TSFM_Heap_free(state);
        }
    }
    return retval;
}

static void RTI_TSFM_JsonTransformationState_delete_data(
        RTI_TSFM_JsonTransformationState *data)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_JsonTransformationState_delete_data)

    if (data->json_buffer != NULL) {
        DDS_String_free(data->json_buffer);
    }
    DDS_OctetSeq_finalize(&data->octet_seq);
    DDS_CharSeq_finalize(&data->char_seq);
    RTI_TSFM_Heap_free(data);
}

void RTI_TSFM_JsonTransformationConfigTypeSupport_delete_data(
        RTI_TSFM_JsonTransformationConfig *sample)
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

    RTIOsapiHeap_freeStructure(sample);
    sample = NULL;
}

RTI_TSFM_JsonTransformationConfig *
        RTI_TSFM_JsonTransformationConfigTypeSupport_create_data(void)
{
    RTI_TSFM_JsonTransformationConfig *sample = NULL;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTIOsapiHeap_allocateStructure(
            &(sample),
            RTI_TSFM_JsonTransformationConfig);
    if (sample == NULL) {
        return NULL;
    }

    sample->buffer_member = DDS_String_alloc(0);
    RTICdrType_copyStringEx(&sample->buffer_member, "", 0, RTI_FALSE);
    if (sample->buffer_member == NULL) {
        goto done;
    }

    sample->initial_serialized_size_for_unbounded = 0u;

    sample->indent = 0u;

    ok = DDS_BOOLEAN_TRUE;
done:
    if (!ok) {
        RTI_TSFM_JsonTransformationConfigTypeSupport_delete_data(
                sample);
        sample = NULL;
    }
    return sample;
}

#define T RTI_TSFM_JsonTransformation
#define T_initialize RTI_TSFM_JsonTransformation_initialize
#define TConfig RTI_TSFM_JsonTransformationConfig
#define TState RTI_TSFM_JsonTransformationState
#define TState_new RTI_TSFM_JsonTransformationState_create_data
#define TState_delete RTI_TSFM_JsonTransformationState_delete_data
#define T_static
#include "TransformationTemplateDefine.h"
