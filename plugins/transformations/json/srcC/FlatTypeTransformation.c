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

#include "FlatTypeInfrastructure.h"
#include "TransformationPlatform.h"
#include "TransformationSimple.h"
#include "DynamicDataHelpers.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::json::flat"

static RTIBool
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping_initialize_w_params(
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *self,
                const struct DDS_TypeAllocationParams_t *allocParams)
{
    self->kind = DDS_TK_NULL;
    self->id = 0;
    self->name = NULL;
    return RTI_TRUE;
}

static RTIBool
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping_finalize_w_params(
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *self,
                const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    /* WARNING self->name is not deallocated because it's "borrowed" from
       the DDS_TypeCode object from which the mapping was generated */
    return RTI_TRUE;
}

#define RTI_TSFM_Json_FlatTypeTransformation_MemberMapping_copy_string( \
        dst_,                                                           \
        src_)                                                           \
    {                                                                   \
        if ((src_) != NULL) {                                           \
            DDS_String_replace(&(dst_), (src_));                        \
            if ((dst_) == NULL) {                                       \
                /* TODO Log error */                                    \
                goto done;                                              \
            }                                                           \
        } else if ((dst_) != NULL) {                                    \
            DDS_String_free((dst_));                                    \
            (dst_) = NULL;                                              \
        }                                                               \
    }

static RTIBool RTI_TSFM_Json_FlatTypeTransformation_MemberMapping_copy(
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *dst,
        const RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *src)
{
    dst->name = src->name;
    dst->kind = src->kind;
    dst->id = src->id;

    return RTI_TRUE;
}

#define T RTI_TSFM_Json_FlatTypeTransformation_MemberMapping
#define TSeq RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq
#define T_initialize_w_params \
    RTI_TSFM_Json_FlatTypeTransformation_MemberMapping_initialize_w_params
#define T_finalize_w_params \
    RTI_TSFM_Json_FlatTypeTransformation_MemberMapping_finalize_w_params
#define T_copy RTI_TSFM_Json_FlatTypeTransformation_MemberMapping_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T

/**
 * @brief Check that a DDS_TCKind value is of one of the supported types.
 *
 * Currently only primitive type kinds are supported (except for LongDouble,
 * Wstring, and Wchar).
 */
#define RTI_TSFM_Json_FlatTypeTransformation_validate_member_tckind(tck_) \
    ((tck_) == DDS_TK_SHORT || (tck_) == DDS_TK_LONG                      \
     || (tck_) == DDS_TK_USHORT || (tck_) == DDS_TK_ULONG                 \
     || (tck_) == DDS_TK_FLOAT || (tck_) == DDS_TK_DOUBLE                 \
     || (tck_) == DDS_TK_BOOLEAN || (tck_) == DDS_TK_CHAR                 \
     || (tck_) == DDS_TK_OCTET || (tck_) == DDS_TK_ENUM                   \
     || (tck_) == DDS_TK_STRING || (tck_) == DDS_TK_LONGLONG              \
     || (tck_) == DDS_TK_ULONGLONG)

#define RTI_TSFM_Json_FlatTypeTransformation_validate_container_tckind(tck_) \
    ((tck_) == DDS_TK_STRUCT || (tck_) == DDS_TK_VALUE)

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_validate_buffer_member(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *mapping,
                struct DDS_TypeCode *base_type,
                const char *member_name)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    struct DDS_TypeCode *member_type = NULL, *member_content_type = NULL;
    DDS_TCKind tckind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_validate_buffer_member)

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
     * In case of a Sequence/Array, check that the inner type is DDS_Octet or
     * DDS_Char
     */
    if (tckind == DDS_TK_SEQUENCE || tckind == DDS_TK_ARRAY) {
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
    } else if (tckind != DDS_TK_STRING) {
        /* TODO Log error */
        goto done;
    }

    /* As this might be a nested type, we cannot fill out the mapping*/

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}


static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_validate_input_type(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                struct DDS_TypeCode *input_type)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *mapping = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_validate_input_type)

    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /* Nothing to check for serializer mode, since we can serialize any
         * DDS type using DDS_DynamicData_print_json */
        goto done; /* DDS_RETCODE_OK */
    }

    /* Check that output type has the specified buffer member */
    if (!RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_ensure_length(
                &self->state->input_mappings,
                1,
                1)) {
        /* TODO Log error */
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    mapping =
            RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_get_reference(
                    &self->state->input_mappings,
                    0);

    retcode = RTI_TSFM_Json_FlatTypeTransformation_validate_buffer_member(
                self,
                mapping,
                input_type,
                self->config->buffer_member);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO Log error */
        goto done;
    }

done:
    if (retcode != DDS_RETCODE_OK) {
        if (!RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_set_length(
                    &self->state->input_mappings,
                    0)) {
            /* TODO Log error */
        }
    }
    return retcode;
}

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_validate_output_type(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                struct DDS_TypeCode *output_type)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_UnsignedLong members_count = 0, i = 0, buffer_member_id = 0;
    struct DDS_TypeCode *member_type = NULL, *member_content_type = NULL;
    DDS_TCKind tckind = DDS_TK_NULL;
    RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *mapping = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_validate_output_type)

    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /* Check that output type has the specified buffer member */
        if (!RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_ensure_length(
                &self->state->output_mappings,
                1,
                1)) {
            /* TODO Log error */
            goto done;
        }

        mapping =
                RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_get_reference(
                        &self->state->output_mappings,
                        0);

        if (DDS_RETCODE_OK
            != RTI_TSFM_Json_FlatTypeTransformation_validate_buffer_member(
                    self,
                    mapping,
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
    if (!RTI_TSFM_Json_FlatTypeTransformation_validate_container_tckind(tckind)) {
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

    /* Allocate entries for output_mappings in transformation */
    if (!RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_ensure_length(
                &self->state->output_mappings,
                members_count,
                members_count)) {
        /* TODO Log error */
        goto done;
    }

    for (i = 0; i < members_count; i++) {
        mapping =
                RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_get_reference(
                        &self->state->output_mappings,
                        i);
        member_type = DDS_TypeCode_member_type(output_type, i, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }
        tckind = DDS_TypeCode_kind(member_type, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }

        if (!RTI_TSFM_Json_FlatTypeTransformation_validate_member_tckind(
                    tckind)) {
            /* TODO Log error */
            goto done;
        }

        mapping->kind = tckind;
        mapping->id = DDS_TypeCode_member_id(output_type, i, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }

        mapping->name = DDS_TypeCode_member_name(output_type, i, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }

        mapping->optional =
                !DDS_TypeCode_is_member_required(output_type, i, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }

        if (mapping->kind == DDS_TK_STRING) {
            /* Store max_len flag for string values */
            mapping->max_len = DDS_TypeCode_length(member_type, &ex);
            if (ex != DDS_NO_EXCEPTION_CODE) {
                /* TODO Log error */
                goto done;
            }
        }
        RTI_TSFM_LOG_4(
                "created MAPPING:",
                "name=%s, id=%d, kind=%d, max_len=%d",
                mapping->name,
                mapping->id,
                mapping->kind,
                mapping->max_len)
    }

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        if (!RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_set_length(
                    &self->state->output_mappings,
                    0)) {
            /* TODO Log error */
        }
    }
    return retcode;
}

DDS_Boolean RTI_TSFM_Json_FlatTypeTransformation_preallocate_buffers(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        struct DDS_TypeCode *tc)
{
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_TCKind member_kind = DDS_TK_NULL;
    DDS_TypeCode *content_tc = NULL;
    DDS_TCKind content_kind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_preallocate_buffers)

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

    /* Preallocate serialized_size_min depending on the type that will be used */
    tc = RTI_COMMON_TypeCode_get_member_type(tc, self->config->buffer_member);
    if (tc == NULL) {
        /* TODO Log error */
        goto done;
    }

    member_kind = DDS_TypeCode_kind(tc, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        goto done;
    }

    switch(member_kind) {
    /*
     * Currently, this function only preallocates strings.
     */
    case DDS_TK_ARRAY:
        content_tc = DDS_TypeCode_content_type(tc, &ex);
        if (content_tc = NULL) {
            /* TODO Log error */
            goto done;
        }

        /* Only arrays of DDS_Octet and DDS_Char are supported*/
        content_kind = DDS_TypeCode_kind(content_tc, &ex);
        if ((content_kind != DDS_TK_OCTET && content_kind != DDS_TK_CHAR)
                || ex != DDS_NO_EXCEPTION_CODE) {
            /* TODO Log error */
            goto done;
        }
        /* Do not break because arrays will use json_buffer as well as strings */
    case DDS_TK_STRING:
        if (self->state->json_buffer == NULL) {
            self->state->json_buffer = DDS_String_alloc(
                self->config->serialized_size_min);
            if (self->state->json_buffer == NULL) {
                /* TODO Log error */
                goto done;
            }
            self->state->json_buffer_size = self->config->serialized_size_min;
        }
        break;
    case DDS_TK_SEQUENCE:
        /*
         * We cannot preallocate sequences because we do a loan_contiguous in
         * the serialization method and it requires that the sequence has 0 size
         */
        break;

    default:
        /* TODO Log error */
        goto done;
        break;
    }

    ok = DDS_BOOLEAN_TRUE;

done:
    return ok;
}


DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_initialize(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        RTI_TSFM_Json_FlatTypeTransformationPlugin *plugin,
        const struct RTI_RoutingServiceTypeInfo *input_type_info,
        const struct RTI_RoutingServiceTypeInfo *output_type_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_TypeCode *tc = NULL;
    DDS_TCKind member_kind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_initialize)

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

    retcode = RTI_TSFM_Json_FlatTypeTransformation_validate_input_type(self, tc);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO Log error */
        goto done;
    }

    tc = (struct DDS_TypeCode *) output_type_info->type_representation;

    retcode = RTI_TSFM_Json_FlatTypeTransformation_validate_output_type(self, tc);
    if (retcode != DDS_RETCODE_OK) {
        /* TODO Log error */
        goto done;
    }

    /* Preallocate serialized_size_min depending on the type that will be used */
    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /*
         * If we are serializing from DynamicData to JSON, the TypeCode is
         * the ouput
         */
        if (!RTI_TSFM_Json_FlatTypeTransformation_preallocate_buffers(
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
        if (!RTI_TSFM_Json_FlatTypeTransformation_preallocate_buffers(
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
        RTI_TSFM_Json_FlatTypeTransformationConfig_parse_from_properties(
                RTI_TSFM_Json_FlatTypeTransformationConfig *self,
                const struct RTI_RoutingServiceProperties *properties)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_TSFM_LOG_FN(
            RTI_TSFM_Json_FlatTypeTransformationConfig_parse_from_properties)

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationConfig_parse_from_properties(
                &self->parent,
                properties)) {
        /* TODO Log error */
        goto done;
    }

    self->serialized_size_min = 255;
    self->serialized_size_incr = self->serialized_size_min;
    self->serialized_size_max = -1;
    self->indent = 0;

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_BUFFER_MEMBER,
            DDS_String_replace(&self->buffer_member, pval);
            if (self->buffer_member == NULL) {
                /* TODO Log error */
                goto done;
            })

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_SERIALIZED_SIZE_MIN,
            self->serialized_size_min =
                    RTI_TSFM_String_to_long(pval, NULL, 0);
            self->serialized_size_incr = self->serialized_size_min;)

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_SERIALIZED_SIZE_MAX,
            self->serialized_size_max =
                    RTI_TSFM_String_to_long(pval, NULL, 0);)

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_SERIALIZED_SIZE_INCR,
            self->serialized_size_incr =
                    RTI_TSFM_String_to_long(
                            pval,
                            NULL,
                            0);)

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_INDENT,
            self->indent =
                    RTI_TSFM_String_to_long(
                            pval,
                            NULL,
                            0);)

    retcode = DDS_RETCODE_OK;
done:
    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_serialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_Json_FlatTypeTransformation *self =
            (RTI_TSFM_Json_FlatTypeTransformation *) transform;
    DDS_Boolean serialized = DDS_BOOLEAN_FALSE,
                buffer_seq_initd = DDS_BOOLEAN_FALSE,
                failed_serialization = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong serialized_size = 0;
    struct DDS_DynamicDataMemberInfo member_info =
            DDS_DynamicDataMemberInfo_INITIALIZER;
    char *p = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_serialize)

    while (!serialized) {
        if (failed_serialization || self->state->json_buffer == NULL
            || self->state->json_buffer_size == 0) {
            failed_serialization = DDS_BOOLEAN_FALSE;
            retcode = RTI_TSFM_realloc_buffer(
                    self->config->serialized_size_min,
                    self->config->serialized_size_incr,
                    self->config->serialized_size_max,
                    &self->state->json_buffer,
                    &self->state->json_buffer_size);
            if (retcode != DDS_RETCODE_OK) {
                RTI_TSFM_ERROR("unable to realloc_buffer")
                goto done;
            }
        }

        /*
         * Serializing from DynamicData to JSON will always use
         * self->state->json_buffer because to_json function returns a string
         */
        serialized_size = self->state->json_buffer_size;
        retcode = DDS_DynamicDataFormatter_to_json(
                sample_in,
                self->state->json_buffer,
                &serialized_size,
                self->config->indent);
        if (retcode != DDS_RETCODE_OK) {
            failed_serialization = DDS_BOOLEAN_TRUE;
            continue;
        }

        /* update serialized_size to actual length of string */
        p = strchr(self->state->json_buffer, '\0');
        if (p == NULL) {
            /* TODO log */
            goto done;
        }
        serialized_size = (DDS_UnsignedLong) (p - self->state->json_buffer);
        if (serialized_size == 0) {
            /* empty message */
            /* TODO log */
            goto done;
        }

        if (self->config->indent == 0) {
            /* Replace all '\n' with a space */
            for (p = self->state->json_buffer; (p = strchr(p, '\n')) != NULL;
                 p++) {
                *p = ' ';
            }
        }

        serialized = DDS_BOOLEAN_TRUE;
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
        case DDS_TK_OCTET:
            if (!DDS_OctetSeq_loan_contiguous(
                    &self->state->octet_seq,
                    (DDS_Octet *) self->state->json_buffer,
                    serialized_size,
                    self->state->json_buffer_size)) {
                RTI_TSFM_ERROR_1(
                        "unable to loan_contiguous in DDS_OctetSeq for member",
                        "%s",
                        self->config->buffer_member)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }

            retcode = DDS_DynamicData_set_octet_seq(
                    sample_out,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    &self->state->octet_seq);
            if (retcode != DDS_RETCODE_OK) {
                RTI_TSFM_ERROR_1(
                    "unable to set_octet_seq for member",
                    "%s",
                    self->config->buffer_member)
                goto done;
            }

            if (!DDS_OctetSeq_unloan(&self->state->octet_seq)) {
                RTI_TSFM_ERROR("unable to unloan DDS_Octet sequence");
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }

            break;
        case DDS_TK_CHAR:
            if (!DDS_CharSeq_loan_contiguous(
                    &self->state->char_seq,
                    (DDS_Octet *) self->state->json_buffer,
                    serialized_size,
                    self->state->json_buffer_size)) {
                RTI_TSFM_ERROR_1(
                        "unable to loan_contiguous in DDS_ChartSeq for member",
                        "%s",
                        self->config->buffer_member)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }

            retcode = DDS_DynamicData_set_char_seq(
                    sample_out,
                    self->config->buffer_member,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    &self->state->char_seq);
            if (retcode != DDS_RETCODE_OK) {
                RTI_TSFM_ERROR_1(
                    "unable to set_char_seq for member",
                    "%s",
                    self->config->buffer_member)
                goto done;
            }

            if (!DDS_CharSeq_unloan(&self->state->char_seq)) {
                RTI_TSFM_ERROR("unable to unloan DDS_Char sequence");
                retcode = DDS_RETCODE_ERROR;
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
            "RTI_TSFM_Json_FlatTypeTransformation_serialize:",
            "retcode=%d",
            retcode)

    return retcode;
}

/**
 * @brief Add a nul terminator ('\0') at the last element of an octet sequence
 * if it doesn't already end with it. This function increases the length of the
 * sequence by one if there is no room for the nul terminator character.
 * @param self the sequence to add the nul character.
 * @return A boolean that says whether the terminator was has been added
 * successfully.
 */
DDS_Boolean RTI_TSFM_Json_FlatTypeTransformation_octet_seq_assert_terminator(
        struct DDS_OctetSeq *self)
{
    DDS_Long length = 0;

    length = DDS_OctetSeq_get_length(self);

    /* The sequence is well terminated, no need to add a nul terminator */
    if (*DDS_OctetSeq_get_reference(self, length - 1) == '\0') {
        return DDS_BOOLEAN_TRUE;
    }

    /* The nul terminator cannot be added without modifying the length */
    if (!DDS_OctetSeq_ensure_length(self, length + 1, length + 1)) {
        RTI_TSFM_ERROR("failed to ensure_length of an octet sequence")
        return DDS_BOOLEAN_FALSE;
    }

    /*
     * length contains the length before increasing it,
     * hence it is pointing ot the new element.
     */
    *DDS_OctetSeq_get_reference(self, length) = '\0';

    return DDS_BOOLEAN_TRUE;

}

DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_deserialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_Json_FlatTypeTransformation *self =
            (RTI_TSFM_Json_FlatTypeTransformation *) transform;
    char *buffer = NULL;
    DDS_UnsignedLong buffer_seq_max = 0, buffer_seq_len = 0;
    struct DDS_DynamicDataMemberInfo member_info =
            DDS_DynamicDataMemberInfo_INITIALIZER;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_deserialize)

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
    case DDS_TK_ARRAY:
    case DDS_TK_STRING:
        do {
            DDS_ReturnCode_t aux_retcode = DDS_RETCODE_OK;
            /* current_len is needed because the get_string will modify and
             * we don't want to modify state->json_buffer
             */
            DDS_UnsignedLong current_len = self->state->json_buffer_size;
            /*
             * Get the string or the array buffer (DDS_Char/DDS_Octet), if
             * json_buffer doesn't have enough space, realloc it
             */
            if (member_info.element_kind == DDS_TK_CHAR) {
                retcode = DDS_DynamicData_get_char_array(
                        sample_in,
                        (DDS_Char *) &self->state->json_buffer,
                        &current_len,
                        self->config->buffer_member,
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            } else if (member_info.element_kind == DDS_TK_CHAR) {
                retcode = DDS_DynamicData_get_octet_array(
                        sample_in,
                        (DDS_Octet *) &self->state->json_buffer,
                        &current_len,
                        self->config->buffer_member,
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            } else {
                retcode = DDS_DynamicData_get_string(
                        sample_in,
                        &self->state->json_buffer,
                        &current_len,
                        self->config->buffer_member,
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            }
            /*
             * the DDS_RETCODE_PRECONDITION_NOT_MET means that there is no
             * enough space to save the string and the json_buffer_size contains
             * the size that we need
             */
            if (retcode == DDS_RETCODE_PRECONDITION_NOT_MET) {
                aux_retcode = RTI_TSFM_realloc_buffer(
                        self->config->serialized_size_min,
                        self->config->serialized_size_incr,
                        self->config->serialized_size_max,
                        &self->state->json_buffer,
                        &self->state->json_buffer_size);
                if (aux_retcode != DDS_RETCODE_OK) {
                    RTI_TSFM_ERROR("unable to realloc_buffer")
                    goto done;
                }
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
        retcode = DDS_DynamicData_get_octet_seq(
                sample_in,
                &self->state->octet_seq,
                self->config->buffer_member,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
        if (retcode != DDS_RETCODE_OK) {
            RTI_TSFM_ERROR_1(
                    "unable to retreive DDS_OctetSeq: ",
                    "%s",
                    self->config->buffer_member)
            goto done;
        }

        /*
         * The buffer might or might not be well terminated with '\0'. We
         * need to ensure that it is.
         */
        if (!RTI_TSFM_Json_FlatTypeTransformation_octet_seq_assert_terminator(
                &self->state->octet_seq)) {
            RTI_TSFM_ERROR_1("failed to assert nul terminator: ",
                    "%s", self->config->buffer_member)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }

        /* We have successfully added a nul terminator to the sequence */
        buffer = (char *) DDS_OctetSeq_get_contiguous_buffer(
                &self->state->octet_seq);
        if (buffer == NULL) {
            RTI_TSFM_ERROR_1(
                    "unable to get_contiguous_buffer of element: ",
                    "%s",
                    self->config->buffer_member)
            retcode = DDS_RETCODE_ERROR;
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
            "RTI_TSFM_Json_FlatTypeTransformation_deserialize:",
            "retcode=%d",
            retcode)

    return retcode;
}


static RTI_TSFM_Json_FlatTypeTransformationState *
        RTI_TSFM_Json_FlatTypeTransformationState_create_data()
{
    RTI_TSFM_Json_FlatTypeTransformationState *retval = NULL, *state = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformationState_create_data)

    state = (RTI_TSFM_Json_FlatTypeTransformationState *)
            RTI_TSFM_Heap_allocate(
                    sizeof(RTI_TSFM_Json_FlatTypeTransformationState));
    if (state == NULL) {
        /* TODO Log error */
        goto done;
    }

    if (!RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_initialize(
                &state->output_mappings)) {
        /* TODO Log error */
        goto done;
    }
    if (!RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_initialize(
                &state->input_mappings)) {
        /* TODO Log error */
        goto done;
    }
    state->json_buffer = NULL;
    state->json_buffer_size = 0;

    retval = state;

done:
    if (retval == NULL) {
        if (state != NULL) {
            RTI_TSFM_Heap_free(state);
        }
    }
    return retval;
}

static void RTI_TSFM_Json_FlatTypeTransformationState_delete_data(
        RTI_TSFM_Json_FlatTypeTransformationState *data)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformationState_delete_data)

    RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_finalize(
            &data->output_mappings);
    RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_finalize(
            &data->input_mappings);
    if (data->json_buffer != NULL) {
        DDS_String_free(data->json_buffer);
    }
    DDS_OctetSeq_finalize(&data->octet_seq);
    RTI_TSFM_Heap_free(data);
}

void RTI_TSFM_Json_FlatTypeTransformationConfigTypeSupport_delete_data(
        RTI_TSFM_Json_FlatTypeTransformationConfig *sample)
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

RTI_TSFM_Json_FlatTypeTransformationConfig *
        RTI_TSFM_Json_FlatTypeTransformationConfigTypeSupport_create_data(void)
{
    RTI_TSFM_Json_FlatTypeTransformationConfig *sample = NULL;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTIOsapiHeap_allocateStructure(
            &(sample),
            RTI_TSFM_Json_FlatTypeTransformationConfig);
    if (sample == NULL) {
        return NULL;
    }

    sample->buffer_member = DDS_String_alloc(0);
    RTICdrType_copyStringEx(&sample->buffer_member, "", 0, RTI_FALSE);
    if (sample->buffer_member == NULL) {
        goto done;
    }

    sample->serialized_size_max = 0;

    sample->serialized_size_min = 0u;

    sample->serialized_size_incr = 0u;

    sample->indent = 0u;

    ok = DDS_BOOLEAN_TRUE;
done:
    if (!ok) {
        RTI_TSFM_Json_FlatTypeTransformationConfigTypeSupport_delete_data(
                sample);
        sample = NULL;
    }
    return sample;
}

#define T RTI_TSFM_Json_FlatTypeTransformation
#define T_initialize RTI_TSFM_Json_FlatTypeTransformation_initialize
#define TConfig RTI_TSFM_Json_FlatTypeTransformationConfig
#define TState RTI_TSFM_Json_FlatTypeTransformationState
#define TState_new RTI_TSFM_Json_FlatTypeTransformationState_create_data
#define TState_delete RTI_TSFM_Json_FlatTypeTransformationState_delete_data
#define T_static
#include "TransformationTemplateDefine.h"
