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

#include "FieldInfrastructure.h"
#include "Transformation.h"
#include "TransformationPlatform.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::field::primitive"

#define RTI_TSFM_FIELD_PRIMITIVE_MAX_SERIALIZED_SIZE_DEFAULT 255

#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_SHORT "%hd"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_LONG "%d"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_USHORT "%hu"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_ULONG "%u"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_FLOAT "%f"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_DOUBLE "%f"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_BOOLEAN "%d"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_CHAR "%c"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_OCTET "%x"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_STRING "%s"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_LONGLONG "%lld"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_ULONGLONG "%llu"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_LONGDOUBLE "%Lf"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_WCHAR "%c"
#define RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_WSTRING "%c"

#define T RTI_TSFM_Field_PrimitiveTransformation
#define TConfig RTI_TSFM_Field_PrimitiveTransformationConfig
#define TState RTI_TSFM_Field_PrimitiveTransformationState
#define T_static
#include "TransformationTemplateDeclare.h"

DDS_ReturnCode_t
        RTI_TSFM_Field_PrimitiveTransformationConfig_parse_from_properties(
                RTI_TSFM_Field_PrimitiveTransformationConfig *self,
                const struct RTI_RoutingServiceProperties *properties)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    const char *default_fmt = NULL;

    RTI_TSFM_LOG_FN(
            RTI_TSFM_Field_PrimitiveTransformationConfig_parse_from_properties)

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationConfig_parse_from_properties(
                &self->parent,
                properties)) {
        RTI_TSFM_ERROR("failed to parse TransformationConfig")
        goto done;
    }

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_BUFFER_MEMBER,
            DDS_String_replace(&self->buffer_member, pval);
            if (self->buffer_member == NULL) {
                RTI_TSFM_ERROR_1(
                        "failed to parse property:",
                        "%s",
                        RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_BUFFER_MEMBER)
                goto done;
            })

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_FIELD,
            DDS_String_replace(&self->field, pval);
            if (self->field == NULL) {
                RTI_TSFM_ERROR_1(
                        "failed to parse property:",
                        "%s",
                        RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_FIELD)
                goto done;
            })

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_SERIALIZATION_FORMAT,
            DDS_String_replace(
                    &self->serialization_format,
                    pval);
            if (self->serialization_format == NULL) {
                RTI_TSFM_ERROR_1(
                        "failed to parse property:",
                        "%s",
                        RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_SERIALIZATION_FORMAT)
                goto done;
            })

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_MAX_SERIALIZED_SIZE,
            self->max_serialized_size = RTI_TSFM_String_to_long(pval, NULL, 0);
            if (self->max_serialized_size == 0) {
                RTI_TSFM_ERROR(
                        "max_serialized_size must be "
                        "greater than 0")
                goto done;
            })

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_FIELD_TYPE,
            RTI_TSFM_Field_FieldType tck_v =
                    RTI_TSFM_Field_FieldType_UNKNOWN;
            if (DDS_RETCODE_OK !=
                    RTI_TSFM_Field_FieldType_from_string(pval, &tck_v)) {
                RTI_TSFM_ERROR_1(
                        "failed to parse "
                        "property:",
                        "%s",
                        RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_FIELD_TYPE)
                goto done;
            }
            self->field_type = tck_v;)

    switch (self->field_type) {
    case RTI_TSFM_Field_FieldType_SHORT:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_SHORT;
        break;
    case RTI_TSFM_Field_FieldType_LONG:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_LONG;
        break;
    case RTI_TSFM_Field_FieldType_USHORT:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_USHORT;
        break;
    case RTI_TSFM_Field_FieldType_ULONG:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_ULONG;
        break;
    case RTI_TSFM_Field_FieldType_FLOAT:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_FLOAT;
        break;
    case RTI_TSFM_Field_FieldType_DOUBLE:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_DOUBLE;
        break;
    case RTI_TSFM_Field_FieldType_BOOLEAN:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_BOOLEAN;
        break;
    case RTI_TSFM_Field_FieldType_CHAR:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_CHAR;
        break;
    case RTI_TSFM_Field_FieldType_OCTET:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_OCTET;
        break;
    case RTI_TSFM_Field_FieldType_STRING:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_STRING;
        break;
    case RTI_TSFM_Field_FieldType_LONGLONG:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_LONGLONG;
        break;
    case RTI_TSFM_Field_FieldType_ULONGLONG:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_ULONGLONG;
        break;
    case RTI_TSFM_Field_FieldType_LONGDOUBLE:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_LONGDOUBLE;
        break;
    case RTI_TSFM_Field_FieldType_WCHAR:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_WCHAR;
        break;
    case RTI_TSFM_Field_FieldType_WSTRING:
        default_fmt = RTI_TSFM_FIELD_PRIMITIVE_FORMAT_DEFAULT_WSTRING;
        break;
    default:
        RTI_TSFM_ERROR_1("unsupported field type:", "%d", self->field_type)
        goto done;
    }

    if (self->max_serialized_size == 0) {
        self->max_serialized_size =
                RTI_TSFM_FIELD_PRIMITIVE_MAX_SERIALIZED_SIZE_DEFAULT;
    }

    if (RTI_TSFM_String_length(self->serialization_format) == 0) {
        DDS_String_replace(&self->serialization_format, default_fmt);
        if (self->serialization_format == NULL) {
            RTI_TSFM_ERROR("failed to determine serialization format")
            goto done;
        }
    }

    retcode = DDS_RETCODE_OK;
done:
    RTI_TSFM_TRACE_1(
            "RTI_TSFM_Field_PrimitiveTransformationConfig_parse_from_"
            "properties:",
            "retcode=%d",
            retcode)

    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_Field_PrimitiveTransformation_serialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_Field_PrimitiveTransformation *self =
            (RTI_TSFM_Field_PrimitiveTransformation *) transform;
    struct DDS_OctetSeq buffer_seq = DDS_SEQUENCE_INITIALIZER;
    DDS_UnsignedLong serialized_size = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Field_PrimitiveTransformation_serialize)

    if (self->state->msg_payload == NULL) {
        self->state->msg_payload_size = self->config->max_serialized_size;
        self->state->msg_payload =
                DDS_String_alloc(self->state->msg_payload_size);
        if (self->state->msg_payload == NULL) {
            RTI_TSFM_ERROR("failed to allocate message payload buffer")
            goto done;
        }
    }

    switch (self->config->field_type) {
    case RTI_TSFM_Field_FieldType_SHORT: {
        DDS_Short v_short = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_short(
                    sample_in,
                    &v_short,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_short);

        break;
    }
    case RTI_TSFM_Field_FieldType_LONG: {
        DDS_Long v_long = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_long(
                    sample_in,
                    &v_long,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_long);

        break;
    }
    case RTI_TSFM_Field_FieldType_USHORT: {
        DDS_UnsignedShort v_ushort = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_ushort(
                    sample_in,
                    &v_ushort,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_ushort);

        break;
    }
    case RTI_TSFM_Field_FieldType_ULONG: {
        DDS_UnsignedLong v_ulong = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_ulong(
                    sample_in,
                    &v_ulong,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_ulong);

        break;
    }
    case RTI_TSFM_Field_FieldType_FLOAT: {
        DDS_Float v_float = .0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_float(
                    sample_in,
                    &v_float,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_float);

        break;
    }
    case RTI_TSFM_Field_FieldType_DOUBLE: {
        DDS_Double v_double = .0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_double(
                    sample_in,
                    &v_double,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_double);

        break;
    }
    case RTI_TSFM_Field_FieldType_BOOLEAN: {
        DDS_Boolean v_bool = DDS_BOOLEAN_FALSE;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_boolean(
                    sample_in,
                    &v_bool,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_bool);

        break;
    }
    case RTI_TSFM_Field_FieldType_CHAR: {
        DDS_Char v_char = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_char(
                    sample_in,
                    &v_char,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_char);

        break;
    }
    case RTI_TSFM_Field_FieldType_OCTET: {
        DDS_Octet v_octet = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_octet(
                    sample_in,
                    &v_octet,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_octet);

        break;
    }
    case RTI_TSFM_Field_FieldType_STRING: {
        char *v_string = NULL;
        DDS_UnsignedLong val_len = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_string(
                    sample_in,
                    &v_string,
                    &val_len,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_string);

        DDS_String_free(v_string);

        v_string = NULL;

        break;
    }
    case RTI_TSFM_Field_FieldType_LONGLONG: {
        DDS_LongLong v_llong = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_longlong(
                    sample_in,
                    &v_llong,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_llong);

        break;
    }
    case RTI_TSFM_Field_FieldType_ULONGLONG: {
        DDS_UnsignedLongLong v_ullong = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_ulonglong(
                    sample_in,
                    &v_ullong,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_ullong);

        break;
    }
    case RTI_TSFM_Field_FieldType_LONGDOUBLE: {
        DDS_LongDouble v_ldouble;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_longdouble(
                    sample_in,
                    &v_ldouble,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_ldouble);

        break;
    }
    case RTI_TSFM_Field_FieldType_WCHAR: {
        DDS_Wchar v_wchar = 0;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_wchar(
                    sample_in,
                    &v_wchar,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        sprintf(self->state->msg_payload,
                self->config->serialization_format,
                v_wchar);

        break;
    }
    case RTI_TSFM_Field_FieldType_WSTRING: {
        DDS_Wchar *v_wstring = NULL;
        DDS_UnsignedLong val_len = 0, i = 0, str_len = 0;
        char *cur_buf = NULL;
        DDS_Wchar *cur_str = NULL;

        if (DDS_RETCODE_OK
            != DDS_DynamicData_get_wstring(
                    sample_in,
                    &v_wstring,
                    &val_len,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED)) {
            RTI_TSFM_ERROR_1("failed to get field:", "%s", self->config->field)
            goto done;
        }

        for (i = 0; i < val_len; i++) {
            cur_str = v_wstring + i;
            cur_buf = self->state->msg_payload + str_len;

            sprintf(cur_buf, self->config->serialization_format, cur_str);

            str_len += RTI_TSFM_String_length(cur_buf);
        }

        DDS_Wstring_free(v_wstring);

        break;
    }
    default:
        /* should never get here */
        RTI_TSFM_LOG_1("unsupported type kind:", "%d", self->config->field_type)
        goto done;
    }

    serialized_size = RTI_TSFM_String_length(self->state->msg_payload);

    if (!DDS_OctetSeq_loan_contiguous(
                &buffer_seq,
                (DDS_Octet *)self->state->msg_payload,
                serialized_size,
                self->state->msg_payload_size)) {
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
            "RTI_TSFM_Field_PrimitiveTransformation_serialize:",
            "retcode=%d",
            retcode)

    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_Field_PrimitiveTransformation_deserialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_Field_PrimitiveTransformation *self =
            (RTI_TSFM_Field_PrimitiveTransformation *) transform;
    struct DDS_OctetSeq buffer_seq = DDS_SEQUENCE_INITIALIZER;
    char *buffer_seq_buff = NULL;
    DDS_UnsignedLong buffer_seq_max = 0, buffer_seq_len = 0;
    DDS_Long field_val = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Field_PrimitiveTransformation_deserialize)

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

    /* The payload might not be 'nul' terminated, so we copy it into
       a buffer and make sure there is one, so we can interpret safely as
       a string */
    if (buffer_seq_len > self->state->msg_payload_size) {
        self->state->msg_payload_size = buffer_seq_len;
        if (self->state->msg_payload != NULL) {
            DDS_String_free(self->state->msg_payload);
        }
        self->state->msg_payload =
                DDS_String_alloc(self->state->msg_payload_size);
        if (self->state->msg_payload != NULL) {
            /* TODO Log error */
            goto done;
        }
    }
    RTI_TSFM_Memory_copy(
            self->state->msg_payload,
            buffer_seq_buff,
            sizeof(char) * self->state->msg_payload_size);

    self->state->msg_payload[self->state->msg_payload_size] = '\0';


    switch (self->config->field_type) {
    case RTI_TSFM_Field_FieldType_SHORT: {
        /* TODO Check overflow */
        DDS_Short v_short =
                RTI_TSFM_String_to_long(self->state->msg_payload, NULL, 0);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_short(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_short)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_LONG: {
        DDS_Long v_long =
                RTI_TSFM_String_to_long(self->state->msg_payload, NULL, 0);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_long(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_long)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_USHORT: {
        /* TODO Check for overlow */
        DDS_UnsignedShort v_ushort =
                RTI_TSFM_String_to_ulong(self->state->msg_payload, NULL, 0);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_ushort(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_ushort)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_ULONG: {
        DDS_UnsignedLong v_ulong =
                RTI_TSFM_String_to_ulong(self->state->msg_payload, NULL, 0);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_ulong(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_ulong)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_FLOAT: {
        /* TODO Check for overflow */
        DDS_Float v_float =
                RTI_TSFM_String_to_double(self->state->msg_payload, NULL);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_float(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_float)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_DOUBLE: {
        DDS_Double v_double =
                RTI_TSFM_String_to_double(self->state->msg_payload, NULL);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_double(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_double)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_BOOLEAN: {
        DDS_Boolean v_bool =
                RTI_TSFM_String_to_long(self->state->msg_payload, NULL, 0);

        switch (v_bool) {
        case DDS_BOOLEAN_TRUE:
        case DDS_BOOLEAN_FALSE:
            break;

        default:
            /* TODO Log error */
            goto done;
        }

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_boolean(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_bool)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_CHAR: {
        DDS_Char v_char = self->state->msg_payload[0];

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_char(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_char)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_OCTET: {
        /* TODO Check for overflow */
        DDS_Octet v_octet =
                RTI_TSFM_String_to_long(self->state->msg_payload, NULL, 0);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_octet(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_octet)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_STRING: {
        char *v_string = NULL;
        DDS_UnsignedLong val_len =
                RTI_TSFM_String_to_double(self->state->msg_payload, NULL);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_string(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_string)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_LONGLONG: {
        DDS_LongLong v_llong =
                RTI_TSFM_String_to_long(self->state->msg_payload, NULL, 0);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_longlong(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_llong)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_ULONGLONG: {
        DDS_UnsignedLongLong v_ullong =
                RTI_TSFM_String_to_long(self->state->msg_payload, NULL, 0);

        if (DDS_RETCODE_OK
            != DDS_DynamicData_set_ulonglong(
                    sample_in,
                    self->config->field,
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    v_ullong)) {
            RTI_TSFM_ERROR_1("failed to set field:", "%s", self->config->field)
            goto done;
        }

        break;
    }
    case RTI_TSFM_Field_FieldType_LONGDOUBLE:
    case RTI_TSFM_Field_FieldType_WSTRING:
    case RTI_TSFM_Field_FieldType_WCHAR:
    default:
        /* should never get here */
        RTI_TSFM_LOG_1("unsupported type kind:", "%d", self->config->field_type)
        goto done;
    }

    retcode = DDS_RETCODE_OK;
done:

    if (!DDS_OctetSeq_finalize(&buffer_seq)) {
        /* TODO Log error */
    }

    RTI_TSFM_TRACE_1(
            "RTI_TSFM_Field_PrimitiveTransformation_deserialize:",
            "retcode=%d",
            retcode)

    return retcode;
}

void RTI_TSFM_Field_PrimitiveTransformationStateTypeSupport_delete_data(
        RTI_TSFM_Field_PrimitiveTransformationState *sample)
{
    if (sample == NULL) {
        return;
    }

    if (sample->msg_payload != NULL) {
        DDS_String_free(sample->msg_payload);
        sample->msg_payload = NULL;
    }

    RTIOsapiHeap_freeStructure(sample);
}

RTI_TSFM_Field_PrimitiveTransformationState *
        RTI_TSFM_Field_PrimitiveTransformationStateTypeSupport_create_data(void)
{
    RTI_TSFM_Field_PrimitiveTransformationState *sample = NULL;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTIOsapiHeap_allocateStructure(
            &(sample),
            RTI_TSFM_Field_PrimitiveTransformationState);

    if (sample == NULL) {
        goto done;
    }

    sample->msg_payload = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&sample->msg_payload, "", (0), RTI_FALSE);
    if (sample->msg_payload == NULL) {
        goto done;
    }

    sample->msg_payload_size = 0u;
    ok = DDS_BOOLEAN_TRUE;
done:
    if (!ok) {
        RTI_TSFM_Field_PrimitiveTransformationStateTypeSupport_delete_data(
                sample);
        sample = NULL;
    }
    return sample;
}

void RTI_TSFM_Field_PrimitiveTransformationConfigTypeSupport_delete_data(
        RTI_TSFM_Field_PrimitiveTransformationConfig *sample)
{
    if (sample == NULL) {
        return;
    }

    /* Free parent*/
    if (sample->parent.input_type != NULL) {
        DDS_String_free(sample->parent.input_type);
        sample->parent.input_type = NULL;
    }
    if (sample->parent.output_type != NULL) {
        DDS_String_free(sample->parent.output_type);
        sample->parent.output_type = NULL;
    }

    /* Free rest of the fields */
    if (sample->buffer_member != NULL) {
        DDS_String_free(sample->buffer_member);
        sample->buffer_member = NULL;
    }
    if (sample->field != NULL) {
        DDS_String_free(sample->field);
        sample->field = NULL;
    }

    if (sample->serialization_format != NULL) {
        DDS_String_free(sample->serialization_format);
        sample->serialization_format = NULL;
    }

    RTIOsapiHeap_freeStructure(sample);
}

RTI_TSFM_Field_PrimitiveTransformationConfig *
        RTI_TSFM_Field_PrimitiveTransformationConfigTypeSupport_create_data(
                void)
{
    RTI_TSFM_Field_PrimitiveTransformationConfig *sample = NULL;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTIOsapiHeap_allocateStructure(
            &(sample),
            RTI_TSFM_Field_PrimitiveTransformationConfig);

    if (sample == NULL) {
        goto done;
    }
    /* parent type */
    sample->parent.type = RTI_TSFM_TransformationKind_SERIALIZER;
    sample->parent.input_type = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&sample->parent.input_type, "", (0), RTI_FALSE);
    if (sample->parent.input_type == NULL) {
        goto done;
    }

    sample->parent.output_type = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&sample->parent.output_type, "", (0), RTI_FALSE);
    if (sample->parent.output_type == NULL) {
        goto done;
    }

    /* It's own type */
    sample->buffer_member = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&sample->buffer_member, "", (0), RTI_FALSE);
    if (sample->buffer_member == NULL) {
        goto done;
    }
    sample->field = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&sample->field, "", (0), RTI_FALSE);
    if (sample->field == NULL) {
        goto done;
    }

    sample->field_type = RTI_TSFM_Field_FieldType_UNKNOWN;

    sample->max_serialized_size = 0u;

    sample->serialization_format = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&sample->serialization_format, "", (0), RTI_FALSE);
    if (sample->serialization_format == NULL) {
        goto done;
    }

    ok = DDS_BOOLEAN_TRUE;

done:
    if (!ok) {
        RTI_TSFM_Field_PrimitiveTransformationConfigTypeSupport_delete_data(
                sample);
        sample = NULL;
    }
    return sample;
}

#define T RTI_TSFM_Field_PrimitiveTransformation
#define TConfig RTI_TSFM_Field_PrimitiveTransformationConfig
#define TState RTI_TSFM_Field_PrimitiveTransformationState
#define T_static
#include "TransformationTemplateDefine.h"
