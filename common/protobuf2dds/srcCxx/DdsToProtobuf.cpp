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
#include <google/protobuf/reflection.h>

#include "DynamicDataUtils.hpp"
#include "ProtobufConverterUtils.hpp"

#include "ScopeExitAction.hpp"
#include "TransformationLog.h"

#define RTI_TSFM_LOG_ARGS "dds2proto"

namespace rti {
namespace dyndata {
namespace protobuf {

static
bool
dds_singular_member_to_pb(
    ProtobufConverterState * const convState,
    DDS_DynamicData & ddsData,
    const DDS_TypeCode * const ddsTc,
    const char * const ddsTcName,
    const DDS_DynamicDataMemberInfo & ddsMemberInfo,
    const DDS_TypeCode * const ddsMemberTc,
    const DDS_TCKind ddsMemberTcKind,
    google::protobuf::Message & pbMessage,
    const google::protobuf::Reflection * const pbRefl,
    const google::protobuf::FieldDescriptor * const pbFieldDesc)
{
    switch (ddsMemberTcKind) {
        case DDS_TK_STRUCT:
        case DDS_TK_VALUE:
        {
            DDS_DynamicData *ddsMemberData = convState->get_cached_dds_data(ddsMemberTc);
            if (nullptr == ddsMemberData)
            {
                RTI_TSFM_ERROR_2(
                    "failed to initialize DynamicData for singular complex member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }

            if (DDS_RETCODE_OK !=
                DDS_DynamicData_bind_complex_member(
                    &ddsData,
                    ddsMemberData,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to bind DynamicData complex member for singular complex member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            auto ddsMemberDataUnbindAction = make_scope_exit(
                [&ddsData, ddsMemberData]() {
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_unbind_complex_member(&ddsData, ddsMemberData))
                    {
                        RTI_TSFM_ERROR("failed to unbind DynamicData complex member")
                    }
                });

            google::protobuf::Message * const pbSubMessage =
                pbRefl->MutableMessage(&pbMessage, pbFieldDesc);
            if (pbSubMessage == nullptr) {
                RTI_TSFM_ERROR_2(
                    "failed to get mutable protobuf sub-message for field",
                    "%s, field=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }

            if (!dds_to_protobuf(*pbSubMessage, *ddsMemberData, convState))
            {
                RTI_TSFM_ERROR_2(
                    "failed to convert DynamicData sub-message to protobuf message",
                    "%s, field=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }

            break;
        }
        case DDS_TK_ENUM:
        {
            DDS_Long ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_long(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member enum value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetEnumValue(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_SHORT:
        {
            DDS_Short ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_short(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member short value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetInt32(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_LONG:
        {
            DDS_Long ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_long(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member long value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetInt32(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_LONGLONG:
        {
            DDS_LongLong ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_longlong(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member long long value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetInt64(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_USHORT:
        {
            DDS_UnsignedShort ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_ushort(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member unsigned short value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetUInt32(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_ULONG:
        {
            DDS_UnsignedLong ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_ulong(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member unsigned long value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetUInt32(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_ULONGLONG:
        {
            DDS_UnsignedLongLong ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_ulonglong(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member unsigned long long value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetUInt64(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_FLOAT:
        {
            DDS_Float ddsValue = .0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_float(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member float value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetFloat(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_DOUBLE:
        {
            DDS_Double ddsValue = .0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_double(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member double value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetDouble(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_BOOLEAN:
        {
            DDS_Boolean ddsValue = DDS_BOOLEAN_FALSE;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_boolean(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member boolean value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetBool(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_CHAR:
        {
            DDS_Char ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_char(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member char value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetInt32(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_OCTET:
        {
            DDS_Octet ddsValue = 0;
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_get_octet(
                    &ddsData,
                    &ddsValue,
                    nullptr,
                    ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member octet value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetUInt32(&pbMessage, pbFieldDesc, ddsValue);
            break;
        }
        case DDS_TK_STRING:
        {
            DDS_UnsignedLong ddsValueLen = convState->str_buffer.size();
            if (convState->str_buffer.size() == 0) {
                convState->str_buffer.resize(1);
                ddsValueLen = 1;
            }
            char * ddsValuePtr = &convState->str_buffer[0];

            DDS_ReturnCode_t getStrRc =
                DDS_DynamicData_get_string(
                    &ddsData,
                    &ddsValuePtr,
                    &ddsValueLen,
                    nullptr,
                    ddsMemberInfo.member_id);
            if (DDS_RETCODE_PRECONDITION_NOT_MET == getStrRc) {
                convState->str_buffer.resize(ddsValueLen);
                ddsValuePtr = &convState->str_buffer[0];
                getStrRc =
                    DDS_DynamicData_get_string(
                        &ddsData,
                        &ddsValuePtr,
                        &ddsValueLen,
                        nullptr,
                        ddsMemberInfo.member_id);
            }
            if (DDS_RETCODE_OK != getStrRc) {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member string value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            convState->str_buffer.resize(ddsValueLen - 1);
            pbRefl->SetString(&pbMessage, pbFieldDesc, convState->str_buffer);
            break;
        }
        case DDS_TK_SEQUENCE:
        {
            convState->str_buffer.resize(ddsMemberInfo.element_count);
            DDS_OctetSeq bytesSeq = DDS_SEQUENCE_INITIALIZER;
            if (!DDS_OctetSeq_loan_contiguous(
                    &bytesSeq,
                    reinterpret_cast<DDS_Octet*>(&convState->str_buffer[0]),
                    0,
                    ddsMemberInfo.element_count))
            {
                RTI_TSFM_ERROR_2(
                    "failed to loan DynamicData member OctetSeq buffer for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            if (DDS_RETCODE_OK
                != DDS_DynamicData_get_octet_seq(
                    &ddsData, &bytesSeq, nullptr, ddsMemberInfo.member_id))
            {
                RTI_TSFM_ERROR_2(
                    "failed to get DynamicData member OctetSeq value for member",
                    "%s, member=%s",
                    ddsTcName,
                    ddsMemberInfo.member_name)
                return false;
            }
            pbRefl->SetString(&pbMessage, pbFieldDesc, convState->str_buffer);
            break;
        }
        default:
        {
            RTI_TSFM_ERROR_3(
                "unsupported DynamicData scalar member typecode kind",
                "%s, member=%s, kind=%d",
                ddsTcName,
                ddsMemberInfo.member_name,
                ddsMemberTcKind)
            return false;
        }
    }

    return true;
}

static
bool
dds_collection_member_to_pb(
    ProtobufConverterState * const convState,
    DDS_DynamicData & ddsData,
    const DDS_TypeCode * const ddsTc,
    const char * const ddsTcName,
    const DDS_DynamicDataMemberInfo & ddsMemberInfo,
    const DDS_TypeCode * const ddsMemberTc,
    google::protobuf::Message & pbMessage,
    const google::protobuf::Reflection * const pbRefl,
    const google::protobuf::FieldDescriptor * const pbFieldDesc)
{
    if (ddsMemberInfo.element_count == 0) {
        // Empty sequence, nothing to do
        return true;
    }

    DDS_DynamicData *ddsMemberData = convState->get_cached_dds_data(ddsMemberTc);
    if (nullptr == ddsMemberData)
    {
        RTI_TSFM_ERROR_2(
            "failed to initialize DynamicData for collection member",
            "%s, member=%s",
            ddsTcName,
            ddsMemberInfo.member_name)
        return false;
    }

    if (DDS_RETCODE_OK !=
        DDS_DynamicData_bind_complex_member(
            &ddsData,
            ddsMemberData,
            nullptr,
            ddsMemberInfo.member_id))
    {
        RTI_TSFM_ERROR_2(
            "failed to bind DynamicData complex member for repeated field",
            "%s, field=%s",
            ddsTcName,
            ddsMemberInfo.member_name)
        return false;
    }
    auto ddsMemberDataUnbindAction = make_scope_exit(
        [&ddsData, ddsMemberData]() {
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_unbind_complex_member(&ddsData, ddsMemberData))
            {
                RTI_TSFM_ERROR("failed to unbind DynamicData complex member")
            }
        });

    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    const DDS_TypeCode * elTc = DDS_TypeCode_content_type(ddsMemberTc, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE || elTc == nullptr) {
        RTI_TSFM_ERROR_2(
            "failed to get DynamicData member type for collection member's elements",
            "%s, member=%s",
            ddsTcName,
            ddsMemberInfo.member_name)
        return false;
    }

    DDS_TCKind elTcKind = DDS_TK_NULL;
    if (!rti::dyndata::resolve_type_code(elTc, elTcKind)) {
        RTI_TSFM_ERROR_2(
            "failed to resolve DynamicData element typecode kind for collection member",
            "%s, member=%s",
            ddsTcName,
            ddsMemberInfo.member_name)
        return false;
    }

    switch (elTcKind) {
        case DDS_TK_STRUCT:
        case DDS_TK_VALUE:
        {
            DDS_DynamicData *elData = convState->get_cached_dds_data(elTc);
            if (nullptr == elData)
            {
                RTI_TSFM_ERROR_1(
                    "failed to initialize DynamicData for collection element",
                    "member=%s",
                    ddsMemberInfo.member_name)
                return false;
            }

            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_bind_complex_member(
                        ddsMemberData,
                        elData,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to bind DynamicData complex element for member",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                auto elDataUnbindAction = make_scope_exit(
                    [ddsMemberData, elData]() {
                        if (DDS_RETCODE_OK !=
                            DDS_DynamicData_unbind_complex_member(ddsMemberData, elData))
                        {
                            RTI_TSFM_ERROR(
                                "failed to unbind DynamicData complex element from member")
                        }
                    });

                google::protobuf::Message * const pbSubMessage =
                    pbRefl->AddMessage(&pbMessage, pbFieldDesc);
                if (pbSubMessage == nullptr) {
                    RTI_TSFM_ERROR_3(
                        "failed to get mutable protobuf sub-message for field",
                        "%s, field=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }

                if (!dds_to_protobuf(*pbSubMessage, *elData, convState)) {
                    RTI_TSFM_ERROR_3(
                        "failed to convert DynamicData sub-message to protobuf message",
                        "%s, field=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
            }

            break;
        }
        case DDS_TK_ENUM:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_Long ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_long(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member enum value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddEnumValue(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_SHORT:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_Short ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_short(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member short value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddInt32(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_LONG:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_Long ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_long(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member long value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddInt32(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_LONGLONG:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_LongLong ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_longlong(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member long long value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddInt64(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_USHORT:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_UnsignedShort ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_ushort(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member unsigned short value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddUInt32(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_ULONG:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_UnsignedLong ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_ulong(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member unsigned long value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddUInt32(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_ULONGLONG:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_UnsignedLongLong ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_ulonglong(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member unsigned long long value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddUInt64(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_FLOAT:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_Float ddsValue = .0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_float(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member float value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddFloat(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_DOUBLE:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_Double ddsValue = .0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_double(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member double value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddDouble(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_BOOLEAN:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_Boolean ddsValue = DDS_BOOLEAN_FALSE;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_boolean(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member boolean value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddBool(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_CHAR:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_Char ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_char(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member char value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddInt32(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_OCTET:
        {
            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_Octet ddsValue = 0;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_octet(
                        ddsMemberData,
                        &ddsValue,
                        nullptr,
                        i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member octet value at index",
                        "%s, member=%s, index=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }
                pbRefl->AddUInt32(&pbMessage, pbFieldDesc, ddsValue);
            }
            break;
        }
        case DDS_TK_STRING:
        {
            DDS_UnsignedLong ddsValueLen = convState->str_buffer.size();
            if (convState->str_buffer.size() == 0) {
                convState->str_buffer.resize(1);
                ddsValueLen = 1;
            }

            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                char * ddsValuePtr = &convState->str_buffer[0];

                DDS_ReturnCode_t getStrRc =
                    DDS_DynamicData_get_string(
                        ddsMemberData,
                        &ddsValuePtr,
                        &ddsValueLen,
                        nullptr,
                        i + 1);
                if (DDS_RETCODE_PRECONDITION_NOT_MET == getStrRc) {
                    convState->str_buffer.resize(ddsValueLen);
                    ddsValuePtr = &convState->str_buffer[0];
                    getStrRc =
                        DDS_DynamicData_get_string(
                            ddsMemberData,
                            &ddsValuePtr,
                            &ddsValueLen,
                            nullptr,
                            i + 1);
                }
                if (DDS_RETCODE_OK != getStrRc) {
                    RTI_TSFM_ERROR_2(
                        "failed to get DynamicData member string value for member",
                        "%s, member=%s",
                        ddsTcName,
                        ddsMemberInfo.member_name)
                    return false;
                }
                convState->str_buffer.resize(ddsValueLen - 1);
                pbRefl->AddString(&pbMessage, pbFieldDesc, convState->str_buffer);
            }
            break;
        }
        case DDS_TK_SEQUENCE:
        {
            DDS_OctetSeq bytesSeq = DDS_SEQUENCE_INITIALIZER;

            for (DDS_UnsignedLong i = 0; i < ddsMemberInfo.element_count; i++) {
                DDS_DynamicDataMemberInfo elMemberInfo = DDS_DynamicDataMemberInfo_INITIALIZER;
                if (DDS_RETCODE_OK !=
                    DDS_DynamicData_get_member_info_by_index(
                        ddsMemberData, &elMemberInfo, i))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member info for sequence element",
                        "%s, member=%s, index=%d",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }

                convState->str_buffer.resize(elMemberInfo.element_count);

                if (!DDS_OctetSeq_loan_contiguous(
                        &bytesSeq,
                        reinterpret_cast<DDS_Octet*>(&convState->str_buffer[0]),
                        0,
                        elMemberInfo.element_count))
                {
                    RTI_TSFM_ERROR_4(
                        "failed to loan DynamicData member OctetSeq buffer for member",
                        "%s, member=%s, index=%d, count=%u",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i,
                        elMemberInfo.element_count)
                    return false;
                }
                auto unloanSeqAction = make_scope_exit(
                    [&bytesSeq]() {
                        if (!DDS_OctetSeq_unloan(&bytesSeq)) {
                            RTI_TSFM_ERROR("failed to unloan DynamicData OctetSeq buffer")
                        }
                    });

                if (DDS_RETCODE_OK
                    != DDS_DynamicData_get_octet_seq(
                        ddsMemberData, &bytesSeq, nullptr, i + 1))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to get DynamicData member OctetSeq value for member",
                        "%s, member=%s, index=%d",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        i)
                    return false;
                }

                pbRefl->AddString(&pbMessage, pbFieldDesc, convState->str_buffer);
            }
            break;

        }
        default:
        {
            RTI_TSFM_ERROR_3(
                "unsupported DynamicData collection member typecode kind",
                "%s, member=%s, kind=%d",
                ddsTcName,
                ddsMemberInfo.member_name,
                elTcKind)
            return false;
        }
    }

    return true;
}

bool
dds_to_protobuf(
    google::protobuf::Message & pbMessage,
    DDS_DynamicData & ddsData,
    ProtobufConverterState * const state)
{
    ProtobufConverterState localState;
    ProtobufConverterState * const convState = (state != nullptr) ? state : &localState;

    const google::protobuf::Descriptor * const pbDesc = pbMessage.GetDescriptor();
    if (nullptr == pbDesc) {
        RTI_TSFM_ERROR("failed to get protobuf message descriptor")
        return false;
    }

    const google::protobuf::Reflection * const pbRefl = pbMessage.GetReflection();
    if (nullptr == pbRefl) {
        RTI_TSFM_ERROR_1(
            "failed to get protobuf message reflection",
            "%s",
            pbDesc->full_name().c_str())
        return false;
    }

    /* Iterate over members in the DDS message, and match them to fields in the PB message */
    DDS_ExceptionCode_t ddsEx = DDS_NO_EXCEPTION_CODE;

    const DDS_TypeCode * ddsTc = DDS_DynamicData_get_type(&ddsData);
    if (ddsTc == nullptr) {
        RTI_TSFM_ERROR("failed to get DynamicData typecode")
        return false;
    }

    const char * const ddsTcName = DDS_TypeCode_name(ddsTc, &ddsEx);
    if (ddsTcName == nullptr || ddsEx != DDS_NO_EXCEPTION_CODE) {
        RTI_TSFM_ERROR_1(
            "failed to get DynamicData typecode name",
            "exception=%d",
            ddsEx)
        return false;
    }

    DDS_TCKind ddsTcKind = DDS_DynamicData_get_type_kind(&ddsData);
    if (ddsTcKind != DDS_TK_STRUCT && ddsTcKind != DDS_TK_VALUE && ddsTcKind != DDS_TK_ALIAS) {
        RTI_TSFM_ERROR_2(
            "unsupported DynamicData typecode kind",
            "%s, kind=%d, supported kinds are TK_STRUCT, TK_VALUE, TK_ALIAS",
            ddsTcName,
            ddsTcKind)
        return false;
    }
    if (!rti::dyndata::resolve_type_code(ddsTc, ddsTcKind)) {
        RTI_TSFM_ERROR_1(
            "failed to resolve DynamicData typecode kind",
            "%s",
            ddsTcName)
        return false;
    }

    const DDS_UnsignedLong ddsMemberCount = DDS_DynamicData_get_member_count(&ddsData);
    if (ddsMemberCount == 0) {
        // No members in DDS DynamicData, nothing to do
        return true;
    }

    DDS_DynamicDataMemberInfo ddsMemberInfo = DDS_DynamicDataMemberInfo_INITIALIZER;
    const DDS_TypeCode * ddsMemberTc = nullptr;
    DDS_TCKind ddsMemberTcKind = DDS_TK_NULL;

    for (DDS_UnsignedLong i = 0; i < ddsMemberCount; ++i) {
        if (DDS_RETCODE_OK !=
            DDS_DynamicData_get_member_info_by_index(&ddsData, &ddsMemberInfo, i))
        {
            RTI_TSFM_ERROR_2(
                "failed to get DynamicData member info for index",
                "%s, index=%d",
                ddsTcName,
                i)
            return false;
        }

        if (!DDS_DynamicData_member_exists(&ddsData, nullptr, ddsMemberInfo.member_id)) {
            // Member does not exist in the data, skip it
            continue;
        }

        if (DDS_RETCODE_OK !=
            DDS_DynamicData_get_member_type(&ddsData, &ddsMemberTc, nullptr, ddsMemberInfo.member_id))
        {
            RTI_TSFM_ERROR_3(
                "failed to get DynamicData member type for member",
                "%s, member=%s, index=%d",
                ddsTcName,
                ddsMemberInfo.member_name,
                i)
            return false;
        }

        if (!rti::dyndata::resolve_type_code(ddsMemberTc, ddsMemberTcKind)) {
            RTI_TSFM_ERROR_3(
                "failed to resolve DynamicData member typecode kind for member",
                "%s, member=%s, index=%d",
                ddsTcName,
                ddsMemberInfo.member_name,
                i)
            return false;
        }

        const google::protobuf::FieldDescriptor * const pbFieldDesc =
            pbDesc->FindFieldByName(ddsMemberInfo.member_name);
        if (pbFieldDesc == nullptr) {
            RTI_TSFM_ERROR_2(
                "failed to find protobuf field descriptor for DynamicData member",
                "%s, member=%s",
                ddsTcName,
                ddsMemberInfo.member_name)
            return false;
        }

        switch (ddsMemberTcKind) {
            case DDS_TK_ARRAY:
            case DDS_TK_SEQUENCE:
            {
                if (pbFieldDesc->type() == google::protobuf::FieldDescriptor::TYPE_BYTES
                    && ddsMemberInfo.element_kind == DDS_TK_OCTET)
                {
                    if (!dds_singular_member_to_pb(
                        convState,
                        ddsData,
                        ddsTc,
                        ddsTcName,
                        ddsMemberInfo,
                        ddsMemberTc,
                        ddsMemberTcKind,
                        pbMessage,
                        pbRefl,
                        pbFieldDesc))
                    {
                        RTI_TSFM_ERROR_3(
                            "failed to convert DynamicData bytes member to protobuf field",
                            "%s, member=%s, kind=%d",
                            ddsTcName,
                            ddsMemberInfo.member_name,
                            ddsMemberTcKind)
                        return false;
                    }
                } else {
                    if (!dds_collection_member_to_pb(
                            convState,
                            ddsData,
                            ddsTc,
                            ddsTcName,
                            ddsMemberInfo,
                            ddsMemberTc,
                            pbMessage,
                            pbRefl,
                            pbFieldDesc))
                    {
                        RTI_TSFM_ERROR_3(
                            "failed to convert DynamicData repeated member to protobuf field",
                            "%s, member=%s, kind=%d",
                            ddsTcName,
                            ddsMemberInfo.member_name,
                            ddsMemberTcKind)
                        return false;
                    }
                }

                break;
            }
            case DDS_TK_STRUCT:
            case DDS_TK_VALUE:
            case DDS_TK_ENUM:
            case DDS_TK_SHORT:
            case DDS_TK_LONG:
            case DDS_TK_LONGLONG:
            case DDS_TK_USHORT:
            case DDS_TK_ULONG:
            case DDS_TK_ULONGLONG:
            case DDS_TK_FLOAT:
            case DDS_TK_DOUBLE:
            case DDS_TK_BOOLEAN:
            case DDS_TK_CHAR:
            case DDS_TK_OCTET:
            case DDS_TK_STRING:
            {
                if (!dds_singular_member_to_pb(
                        convState,
                        ddsData,
                        ddsTc,
                        ddsTcName,
                        ddsMemberInfo,
                        ddsMemberTc,
                        ddsMemberTcKind,
                        pbMessage,
                        pbRefl,
                        pbFieldDesc))
                {
                    RTI_TSFM_ERROR_3(
                        "failed to convert DynamicData scalar member to protobuf field",
                        "%s, member=%s, kind=%d",
                        ddsTcName,
                        ddsMemberInfo.member_name,
                        ddsMemberTcKind)
                    return false;
                }
                break;
            }
            default:
            {
                RTI_TSFM_ERROR_3(
                    "unsupported DynamicData member typecode kind",
                    "%s, member=%s, kind=%d",
                    ddsTcName,
                    ddsMemberInfo.member_name,
                    ddsMemberTcKind)
                return false;
            }
        }
    }

    return true;
}


} // namespace protobuf
} // namespace dyndata
} // namespace rti

