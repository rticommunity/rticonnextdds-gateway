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

#include "json.h"
#include "ndds/ndds_c.h"

#include "FlatTypeInfrastructure.h"
#include "TransformationPlatform.h"
#include "TransformationSimple.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::json::flat"

static RTIBool
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping_initialize_w_params(
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *self,
                const struct DDS_TypeAllocationParams_t *allocParams)
{
    self->kind = DDS_TK_NULL;
    self->id = 0;
    self->name = NULL;
    self->parse_fn = NULL;
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
    dst->parse_fn = src->parse_fn;

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

#define RTI_TSFM_Json_MemberValue_validate_string(name_, val_, max_len_) \
    {                                                                    \
        switch ((val_)->type) {                                          \
        case json_string:                                                \
            if ((val_)->u.string.length > (max_len_)) {                  \
                RTI_TSFM_ERROR_3(                                        \
                        "INVALID JSON: string value too long:",          \
                        "member=%s, max_len=%d, found_len=%d",           \
                        (name_),                                         \
                        (max_len_),                                      \
                        (val_)->u.string.length)                         \
                goto done;                                               \
            }                                                            \
            break;                                                       \
        default:                                                         \
            RTI_TSFM_ERROR_2(                                            \
                    "INVALID JSON: member expected to be a string:",     \
                    "member=%s, found_type=%d",                          \
                    (name_),                                             \
                    (val_)->type)                                        \
            goto done;                                                   \
        }                                                                \
    }

#define RTI_TSFM_Json_MemberValue_validate_integer(name_, val_)        \
    {                                                                  \
        switch ((val_)->type) {                                        \
        case json_integer:                                             \
            break;                                                     \
        default:                                                       \
            RTI_TSFM_ERROR_2(                                          \
                    "INVALID JSON: member expected to be an integer:", \
                    "member=%s, found_type=%d",                        \
                    (name_),                                           \
                    (val_)->type)                                      \
            goto done;                                                 \
        }                                                              \
    }

#define RTI_TSFM_Json_MemberValue_validate_double(name_, val_)       \
    {                                                                \
        switch ((val_)->type) {                                      \
        case json_double:                                            \
            break;                                                   \
        default:                                                     \
            RTI_TSFM_ERROR_2(                                        \
                    "INVALID JSON: member expected to be a double:", \
                    "member=%s, found_type=%d",                      \
                    (name_),                                         \
                    (val_)->type)                                    \
            goto done;                                               \
        }                                                            \
    }

#define RTI_TSFM_Json_MemberValue_validate_boolean(name_, val_)       \
    {                                                                 \
        switch ((val_)->type) {                                       \
        case json_boolean:                                            \
            break;                                                    \
        default:                                                      \
            RTI_TSFM_ERROR_2(                                         \
                    "INVALID JSON: member expected to be a boolean:", \
                    "member=%s, found_type=%d",                       \
                    (name_),                                          \
                    (val_)->type)                                     \
            goto done;                                                \
        }                                                             \
    }

static DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_parse_member_short(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
        json_value *json_member_val,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Short member_val = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_short)

    RTI_TSFM_Json_MemberValue_validate_integer(member->name, json_member_val);

    member_val = (DDS_Short) json_member_val->u.integer;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_short(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_parse_member_long(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
        json_value *json_member_val,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Long member_val = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_long)

    RTI_TSFM_Json_MemberValue_validate_integer(member->name, json_member_val);

    member_val = (DDS_Long) json_member_val->u.integer;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_long(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_parse_member_ushort(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
                json_value *json_member_val,
                DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedShort member_val = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_ushort)

    RTI_TSFM_Json_MemberValue_validate_integer(member->name, json_member_val);

    member_val = (DDS_UnsignedShort) json_member_val->u.integer;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_ushort(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_parse_member_ulong(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
        json_value *json_member_val,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong member_val = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_ulong)

    RTI_TSFM_Json_MemberValue_validate_integer(member->name, json_member_val);

    member_val = (DDS_UnsignedLong) json_member_val->u.integer;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_ulong(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_parse_member_ulonglong(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
                json_value *json_member_val,
                DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLongLong member_val = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_ulonglong)

    RTI_TSFM_Json_MemberValue_validate_integer(member->name, json_member_val);

    member_val = (DDS_UnsignedLongLong) json_member_val->u.integer;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_ulonglong(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_parse_member_longlong(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
                json_value *json_member_val,
                DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_LongLong member_val = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_longlong)

    RTI_TSFM_Json_MemberValue_validate_integer(member->name, json_member_val);

    member_val = (DDS_LongLong) json_member_val->u.integer;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_longlong(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_parse_member_float(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
        json_value *json_member_val,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Float member_val = .0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_float)

    RTI_TSFM_Json_MemberValue_validate_double(member->name, json_member_val);

    member_val = (DDS_Float) json_member_val->u.dbl;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_float(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_parse_member_double(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
                json_value *json_member_val,
                DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Double member_val = .0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_double)

    RTI_TSFM_Json_MemberValue_validate_double(member->name, json_member_val);

    member_val = (DDS_Double) json_member_val->u.dbl;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_double(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_parse_member_octet(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
        json_value *json_member_val,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Octet member_val = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_octet)

    RTI_TSFM_Json_MemberValue_validate_integer(member->name, json_member_val);

    member_val = (DDS_Octet) json_member_val->u.integer;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_octet(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_parse_member_boolean(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
                json_value *json_member_val,
                DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Boolean member_val = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_boolean)

    RTI_TSFM_Json_MemberValue_validate_boolean(member->name, json_member_val);

    member_val = (DDS_Boolean) json_member_val->u.boolean;

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_boolean(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_parse_member_char(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
        json_value *json_member_val,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Char member_val = '\0';

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_char)

    RTI_TSFM_Json_MemberValue_validate_string(member->name, json_member_val, 1);

    if (json_member_val->u.string.length == 0) {
        /* TODO Log error */
        goto done;
    }

    member_val = (DDS_Char) json_member_val->u.string.ptr[0];

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_char(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                member_val)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_parse_member_string(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *member,
                json_value *json_member_val,
                DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_parse_member_string)

    RTI_TSFM_Json_MemberValue_validate_string(
            member->name,
            json_member_val,
            member->max_len);

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_string(
                sample,
                member->name,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                json_member_val->u.string.ptr)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

#define RTI_TSFM_Json_FlatTypeTransformation_parse_member_enum \
    RTI_TSFM_Json_FlatTypeTransformation_parse_member_long

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_get_member_parse_function(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                DDS_TCKind member_kind,
                RTI_TSFM_Json_FlatTypeTransformation_ParseMemberFn
                        *parse_fn_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_Json_FlatTypeTransformation_ParseMemberFn parse_fn = NULL;


    RTI_TSFM_LOG_FN(
            RTI_TSFM_Json_FlatTypeTransformation_get_member_parse_function)

    switch (member_kind) {
    case DDS_TK_SHORT:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_short;
        break;
    case DDS_TK_LONG:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_long;
        break;
    case DDS_TK_USHORT:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_ushort;
        break;
    case DDS_TK_ULONG:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_ulong;
        break;
    case DDS_TK_LONGLONG:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_longlong;
        break;
    case DDS_TK_ULONGLONG:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_ulonglong;
        break;
    case DDS_TK_FLOAT:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_float;
        break;
    case DDS_TK_DOUBLE:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_double;
        break;
    case DDS_TK_OCTET:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_octet;
        break;
    case DDS_TK_CHAR:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_char;
        break;
    case DDS_TK_BOOLEAN:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_boolean;
        break;
    case DDS_TK_STRING:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_string;
        break;
    case DDS_TK_ENUM:
        parse_fn = RTI_TSFM_Json_FlatTypeTransformation_parse_member_enum;
        break;
    default:
        /* TODO Log error */
        goto done;
    }

    *parse_fn_out = parse_fn;

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_validate_buffer_member(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *mapping,
                struct DDS_TypeCode *base_type,
                const char *member_name)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_UnsignedLong buffer_member_id = 0;
    struct DDS_TypeCode *member_type = NULL, *member_content_type = NULL;
    DDS_TCKind tckind = DDS_TK_NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_validate_buffer_member)

    /* Currently we don't validate nor create a mapping for the "buffer member"
       because users specify it as a possibly nested selector but
       DDS_TypeCode_find_member_by_name doesn't support nested selectors. */

#if 0

    /* Check that output type has the specified buffer member */
    buffer_member_id = 
        DDS_TypeCode_find_member_by_name(base_type, member_name, &ex);
    if (DDS_NO_EXCEPTION_CODE != ex)
    {
        /* TODO Log error */
        goto done;
    }
    member_type = DDS_TypeCode_member_type(base_type, buffer_member_id, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE)
    {
        /* TODO Log error */
        goto done;
    }
    tckind = DDS_TypeCode_kind(member_type, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE)
    {
        /* TODO Log error */
        goto done;
    }

    if (tckind != DDS_TK_SEQUENCE)
    {
        /* TODO Log error */
        goto done;
    }

    member_content_type = DDS_TypeCode_content_type(member_type, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE)
    {
        /* TODO Log error */
        goto done;
    }

    tckind = DDS_TypeCode_kind(member_content_type, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE)
    {
        /* TODO Log error */
        goto done;
    }

    if (tckind != DDS_TK_OCTET)
    {
        /* TODO Log error */
        goto done;
    }

    mapping->name = self->config->buffer_member;
    mapping->id = buffer_member_id;
    mapping->max_len = DDS_TypeCode_length(member_type,&ex);
    if (ex != DDS_NO_EXCEPTION_CODE)
    {
        /* TODO Log error */
        goto done;
    }
    mapping->kind = DDS_TK_SEQUENCE;
    mapping->optional =
        !DDS_TypeCode_is_member_required(base_type, mapping->id, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE)
    {
        /* TODO Log error */
        goto done;
    }
#endif

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}


static DDS_ReturnCode_t
        RTI_TSFM_Json_FlatTypeTransformation_validate_input_type(
                RTI_TSFM_Json_FlatTypeTransformation *self,
                struct DDS_TypeCode *input_type)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *mapping = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_validate_input_type)

    if (self->config->parent.type == RTI_TSFM_TransformationKind_SERIALIZER) {
        /* Nothing to check for serializer mode, since we can serialize any
         * DDS type using DDS_DynamicData_print_json */
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    /* Check that output type has the specified buffer member */
    if (!RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_ensure_length(
                &self->state->input_mappings,
                1,
                1)) {
        /* TODO Log error */
        goto done;
    }

    mapping =
            RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_get_reference(
                    &self->state->input_mappings,
                    0);

    if (DDS_RETCODE_OK
        != RTI_TSFM_Json_FlatTypeTransformation_validate_buffer_member(
                self,
                mapping,
                input_type,
                self->config->buffer_member)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

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

    /* Check that the base type is a struct (only type supported for now) */
    tckind = DDS_TypeCode_kind(output_type, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        /* TODO Log error */
        goto done;
    }
    if (tckind != DDS_TK_STRUCT) {
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

        if (RTI_TSFM_Json_FlatTypeTransformation_get_member_parse_function(
                    self,
                    mapping->kind,
                    &mapping->parse_fn)
            != DDS_RETCODE_OK) {
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

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_initialize)

    if (DDS_RETCODE_OK
        != RTI_TSFM_Transformation_initialize(
                &self->parent,
                &plugin->parent,
                input_type_info,
                output_type_info,
                properties,
                env)) {
        goto done;
    }

    if (input_type_info->type_representation_kind
        != RTI_ROUTING_SERVICE_TYPE_REPRESENTATION_DYNAMIC_TYPE) {
        /* TODO Log error */
        goto done;
    }

    tc = (struct DDS_TypeCode *) input_type_info->type_representation;

    if (DDS_RETCODE_OK
        != RTI_TSFM_Json_FlatTypeTransformation_validate_input_type(self, tc)) {
        /* TODO Log error */
        goto done;
    }

    tc = (struct DDS_TypeCode *) output_type_info->type_representation;

    if (DDS_RETCODE_OK
        != RTI_TSFM_Json_FlatTypeTransformation_validate_output_type(
                self,
                tc)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

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
    struct DDS_OctetSeq buffer_seq = DDS_SEQUENCE_INITIALIZER;
    DDS_Boolean serialized = DDS_BOOLEAN_FALSE,
                buffer_seq_initd = DDS_BOOLEAN_FALSE,
                failed_serialization = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong serialized_size = 0;
    char *p = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_serialize)

    while (!serialized) {
        if (failed_serialization || self->state->json_buffer == NULL
            || self->state->json_buffer_size == 0) {
            failed_serialization = DDS_BOOLEAN_FALSE;
            if (DDS_RETCODE_OK
                != RTI_TSFM_realloc_buffer(
                        self->config->serialized_size_min,
                        self->config->serialized_size_incr,
                        self->config->serialized_size_max,
                        &self->state->json_buffer,
                        &self->state->json_buffer_size)) {
                /* TODO Log error */
                goto done;
            }
        }

        serialized_size = self->state->json_buffer_size;
        if (DDS_RETCODE_OK
            != DDS_DynamicDataFormatter_to_json(
                    sample_in,
                    self->state->json_buffer,
                    &serialized_size,
                    self->config->indent)) {
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

    if (!DDS_OctetSeq_loan_contiguous(
                &buffer_seq,
                (DDS_Octet *) self->state->json_buffer,
                serialized_size,
                self->state->json_buffer_size)) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != DDS_DynamicData_set_octet_seq(
                sample_out,
                self->config->buffer_member,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                &buffer_seq)) {
        /* TODO Log error */
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

DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_find_json_member(
        const char *member_name,
        json_value *value,
        json_object_entry **member_out)
{
    DDS_UnsignedLong i = 0;
    json_object_entry *member = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_find_json_member)

    *member_out = NULL;

    for (i = 0; i < value->u.object.length && *member_out == NULL; i++) {
        member = &value->u.object.values[i];

        if (RTI_TSFM_String_compare(member->name, member_name) == 0) {
            *member_out = member;
        }
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t RTI_TSFM_Json_FlatTypeTransformation_deserialize_sample(
        RTI_TSFM_Json_FlatTypeTransformation *self,
        const char *json_buffer,
        DDS_UnsignedLong json_buffer_size,
        DDS_DynamicData *sample)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    json_value *value = NULL, *member_value = NULL;
    json_object_entry *member = NULL;
    DDS_UnsignedLong i = 0, mappings_len = 0;
    DDS_Long member_val_l = 0;
    RTI_TSFM_Json_FlatTypeTransformation_MemberMapping *mapping = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_deserialize_sample)

    /* json_buffer should not be printed with printf() because there is
     * no guarantee that the string is well terminated (and most likely
     * it won't be, i.e. no 'nul' terminator) */

    value = json_parse(json_buffer, json_buffer_size);
    if (value == NULL) {
        /* TODO Log error */
        RTI_TSFM_ERROR("failed: json_parse")
        goto done;
    }
    if (value->type != json_object) {
        /* TODO Log error */
        RTI_TSFM_ERROR("failed: invalid parsed value")
        goto done;
    }

    mappings_len =
            RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_get_length(
                    &self->state->output_mappings);

    for (i = 0; i < mappings_len; i++) {
        mapping =
                RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq_get_reference(
                        &self->state->output_mappings,
                        i);
        if (DDS_RETCODE_OK
            != RTI_TSFM_Json_FlatTypeTransformation_find_json_member(
                    mapping->name,
                    value,
                    &member)) {
            /* TODO Log error */
            goto done;
        }
        if (member == NULL) {
            if (!mapping->optional) {
                RTI_TSFM_ERROR_1(
                        "required member not found in JSON sample:",
                        "member=%s",
                        mapping->name)
                goto done;
            }
            continue;
        }

        if (DDS_RETCODE_OK
            != RTI_TSFM_Json_FlatTypeTransformation_parse_member(
                    self,
                    mapping,
                    member->value,
                    sample)) {
            /* TODO Log error */
            goto done;
        }
    }

#if 0
    DDS_DynamicData_print(sample, stdout, 0);
#endif

    retcode = DDS_RETCODE_OK;

done:
    if (value != NULL) {
        json_value_free(value);
    }

    return retcode;
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
    struct DDS_OctetSeq buffer_seq = DDS_SEQUENCE_INITIALIZER;
    char *buffer_seq_buff = NULL;
    DDS_UnsignedLong buffer_seq_max = 0, buffer_seq_len = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Json_FlatTypeTransformation_deserialize)

    if (DDS_RETCODE_OK
        != DDS_DynamicData_get_octet_seq(
                sample_in,
                &buffer_seq,
                self->config->buffer_member,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
        /* TODO Log error */
        goto done;
    }

    buffer_seq_buff = (char *) DDS_OctetSeq_get_contiguous_buffer(&buffer_seq);
    if (buffer_seq_buff == NULL) {
        /* TODO Log error */
        goto done;
    }
    /* The buffer should not be printed with printf() because there is
     * no guarantee that the string is well terminated (and most likely
     * it won't be, i.e. no 'nul' terminator) */
    buffer_seq_max = DDS_OctetSeq_get_maximum(&buffer_seq);
    buffer_seq_len = DDS_OctetSeq_get_length(&buffer_seq);

    if (DDS_RETCODE_OK
        != RTI_TSFM_Json_FlatTypeTransformation_deserialize_sample(
                self,
                buffer_seq_buff,
                buffer_seq_len,
                sample_out)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;
done:

    if (!DDS_OctetSeq_finalize(&buffer_seq)) {
        /* TODO Log error */
    }

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

    sample->buffer_member = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&sample->buffer_member, "", (0), RTI_FALSE);
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
