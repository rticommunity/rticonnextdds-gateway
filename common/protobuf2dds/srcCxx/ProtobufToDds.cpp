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
#include <vector>

#include <google/protobuf/reflection.h>

#include "DynamicDataUtils.hpp"
#include "ProtobufConverterUtils.hpp"

#include "ScopeExitAction.hpp"
#include "TransformationLog.h"

#ifdef _WIN32
// windows.h #defines GetMessage() as a macro).
#undef GetMessage
#endif

#define RTI_TSFM_LOG_ARGS "proto2dds"

namespace rti {
namespace dyndata {
namespace protobuf {

static
bool
pb_singular_field_to_dds(
    ProtobufConverterState * const convState,
    DDS_DynamicData & ddsData,
    const DDS_DynamicDataMemberInfo & ddsMemberInfo,
    const DDS_TypeCode * const ddsMemberTc,
    const DDS_TCKind ddsMemberTcKind,
    const google::protobuf::Message & pbMessage,
    const google::protobuf::Descriptor * const pbDesc,
    const google::protobuf::Reflection * const pbRefl,
    const google::protobuf::FieldDescriptor * const pbFieldDesc)
{
    if (!pbRefl->HasField(pbMessage, pbFieldDesc)) {
        // Field not set, nothing to do
        return true;
    }

    switch (pbFieldDesc->type()) {
        case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
        {
            const double pbValue = pbRefl->GetDouble(pbMessage, pbFieldDesc);
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_double(
                    &ddsData,
                    nullptr,
                    ddsMemberInfo.member_id,
                    pbValue))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member double value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_FLOAT:
        {
            const float pbValue = pbRefl->GetFloat(pbMessage, pbFieldDesc);
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_float(
                    &ddsData,
                    nullptr,
                    ddsMemberInfo.member_id,
                    pbValue))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member float value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_INT64:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
        case google::protobuf::FieldDescriptor::TYPE_SINT64:
        {
            const int64_t pbValue = pbRefl->GetInt64(pbMessage, pbFieldDesc);
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_longlong(
                    &ddsData,
                    nullptr,
                    ddsMemberInfo.member_id,
                    pbValue))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member long long value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_UINT64:
        case google::protobuf::FieldDescriptor::TYPE_FIXED64:
        {
            const uint64_t pbValue = pbRefl->GetUInt64(pbMessage, pbFieldDesc);
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_ulonglong(
                    &ddsData,
                    nullptr,
                    ddsMemberInfo.member_id,
                    pbValue))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member unsigned long long value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_INT32:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
        case google::protobuf::FieldDescriptor::TYPE_SINT32:
        {
            const int32_t pbValue = pbRefl->GetInt32(pbMessage, pbFieldDesc);
            if (!rti::dyndata::set_fuzzy_long(
                    ddsData,
                    ddsMemberInfo.member_id,
                    ddsMemberTcKind,
                    pbValue))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member long value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_UINT32:
        case google::protobuf::FieldDescriptor::TYPE_FIXED32:
        {
            const uint32_t pbValue = pbRefl->GetUInt32(pbMessage, pbFieldDesc);
            if (!rti::dyndata::set_fuzzy_ulong(
                    ddsData,
                    ddsMemberInfo.member_id,
                    ddsMemberTcKind,
                    pbValue))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member unsigned long value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_BOOL:
        {
            const bool pbValue = pbRefl->GetBool(pbMessage, pbFieldDesc);
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_boolean(
                    &ddsData,
                    nullptr,
                    ddsMemberInfo.member_id,
                    pbValue))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member boolean value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_STRING:
        {
            const std::string pbValue = pbRefl->GetString(pbMessage, pbFieldDesc);
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_string(
                    &ddsData,
                    nullptr,
                    ddsMemberInfo.member_id,
                    pbValue.c_str()))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member string value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_GROUP:
        case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
        {
            const google::protobuf::Message & pbSubMessage =
                pbRefl->GetMessage(pbMessage, pbFieldDesc);
            DDS_DynamicData *ddsMemberData =
                convState->get_cached_dds_data(ddsMemberTc);
            if (nullptr == ddsMemberData)
            {
                RTI_TSFM_ERROR_2(
                    "failed to initialize DynamicData for singular complex member",
                    "%s, member=%s",
                    pbDesc->full_name().c_str(),
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
                    "failed to bind DynamicData singular complex member for field",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
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
            if (!protobuf_to_dds(*ddsMemberData, pbSubMessage, convState)) {
                RTI_TSFM_ERROR_2(
                    "failed to convert protobuf sub-message to DynamicData",
                    "%s, type=%s",
                    pbFieldDesc->name().c_str(),
                    pbFieldDesc->type_name())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_BYTES:
        {
            const std::string pbValue = pbRefl->GetString(pbMessage, pbFieldDesc);
            if (!rti::dyndata::set_fuzzy_bytes(
                    ddsData,
                    ddsMemberInfo.member_id,
                    ddsMemberTc,
                    pbValue.data(),
                    static_cast<DDS_UnsignedLong>(pbValue.size())))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member bytes value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        case google::protobuf::FieldDescriptor::TYPE_ENUM:
        {
            const google::protobuf::EnumValueDescriptor * pbEnumValueDesc =
                pbRefl->GetEnum(pbMessage, pbFieldDesc);
            if (pbEnumValueDesc == nullptr) {
                RTI_TSFM_ERROR_2(
                    "failed to get enum value descriptor for field",
                    "%s, type=%s",
                    pbFieldDesc->name().c_str(),
                    pbFieldDesc->type_name())
                return false;
            }
            const int pbEnumValue = pbEnumValueDesc->number();
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_long(
                    &ddsData,
                    nullptr,
                    ddsMemberInfo.member_id,
                    pbEnumValue))
            {
                RTI_TSFM_ERROR_2(
                    "failed to set DynamicData member enum value",
                    "%s, field=%s",
                    pbDesc->full_name().c_str(),
                    pbFieldDesc->name().c_str())
                return false;
            }
            break;
        }
        default:
        {
            RTI_TSFM_ERROR_2(
                "unsupported protobuf field type",
                "%s, type=%d",
                pbFieldDesc->name().c_str(),
                pbFieldDesc->type())
            return false;
        }
    }

    return true;
}


static
bool
pb_repeated_field_to_dds(
    ProtobufConverterState * const convState,
    DDS_DynamicData & ddsData,
    const DDS_DynamicDataMemberInfo & ddsMemberInfo,
    const DDS_TypeCode * const ddsMemberTc,
    const DDS_TCKind ddsMemberTcKind,
    const google::protobuf::Message & pbMessage,
    const google::protobuf::Descriptor * const pbDesc,
    const google::protobuf::Reflection * const pbRefl,
    const google::protobuf::FieldDescriptor * const pbFieldDesc)
{
    const int pbFieldSize = pbRefl->FieldSize(pbMessage, pbFieldDesc);
    if (pbFieldSize == 0) {
        return true;
    }

    DDS_DynamicData *ddsMemberData = convState->get_cached_dds_data(ddsMemberTc);
    if (nullptr == ddsMemberData)
    {
        RTI_TSFM_ERROR_2(
            "failed to initialize DynamicData for repeated member",
            "%s, member=%s",
            pbDesc->full_name().c_str(),
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
            pbDesc->full_name().c_str(),
            pbFieldDesc->name().c_str())
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
            pbDesc->full_name().c_str(),
            ddsMemberInfo.member_name)
        return false;
    }

    DDS_TCKind elTcKind = DDS_TK_NULL;
    if (!rti::dyndata::resolve_type_code(elTc, elTcKind)) {
        RTI_TSFM_ERROR_2(
            "failed to get DynamicData element type for collection member's elements",
            "%s, member=%s",
            pbDesc->full_name().c_str(),
            ddsMemberInfo.member_name)
        return false;
    }

    if (pbFieldDesc->is_map()
        || pbFieldDesc->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE
        || pbFieldDesc->type() == google::protobuf::FieldDescriptor::TYPE_GROUP) {
        // Map fields are treated as repeated message fields
        DDS_DynamicData *elData = convState->get_cached_dds_data(elTc);
        if (nullptr == elData)
        {
            RTI_TSFM_ERROR_1(
                "failed to initialize DynamicData for collection element",
                "member=%s",
                ddsMemberInfo.member_name)
            return false;
        }

        for (int j = 0; j < pbFieldSize; ++j) {
            const google::protobuf::Message & pbSubMessage =
                pbRefl->GetRepeatedMessage(pbMessage, pbFieldDesc, j);

            if (DDS_RETCODE_OK !=
                DDS_DynamicData_bind_complex_member(
                    ddsMemberData,
                    elData,
                    nullptr,
                    j + 1))
            {
                RTI_TSFM_ERROR_3(
                    "failed to bind DynamicData aggregate for repeated field",
                    "%s, type=%s, index=%d",
                    pbFieldDesc->name().c_str(),
                    pbFieldDesc->type_name(),
                    j)
                return false;
            }
            auto elDataUnbindAction = make_scope_exit(
                [ddsMemberData, elData]() {
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_unbind_complex_member(ddsMemberData, elData))
                    {
                        RTI_TSFM_ERROR("failed to unbind DynamicData aggregate")
                    }
                });
            if (!protobuf_to_dds(*elData, pbSubMessage, convState)) {
                RTI_TSFM_ERROR_3(
                    "failed to convert protobuf sub-message to DynamicData",
                    "%s, type=%s, index=%d",
                    pbFieldDesc->name().c_str(),
                    pbFieldDesc->type_name(),
                    j)
                return false;
            }
        }
    } else {
        // Scalar repeated field
        for (int j = 0; j < pbFieldSize; ++j) {
            switch (pbFieldDesc->type()) {
                case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                {
                    const double pbValue = pbRefl->GetRepeatedDouble(pbMessage, pbFieldDesc, j);
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_set_double(
                            ddsMemberData,
                            nullptr,
                            j + 1,
                            pbValue))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member double value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                {
                    const float pbValue = pbRefl->GetRepeatedFloat(pbMessage, pbFieldDesc, j);
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_set_float(
                            ddsMemberData,
                            nullptr,
                            j + 1,
                            pbValue))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member float value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_INT64:
                case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
                case google::protobuf::FieldDescriptor::TYPE_SINT64:
                {
                    const int64_t pbValue = pbRefl->GetRepeatedInt64(pbMessage, pbFieldDesc, j);
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_set_longlong(
                            ddsMemberData,
                            nullptr,
                            j + 1,
                            pbValue))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member long long value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_UINT64:
                case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                {
                    const uint64_t pbValue = pbRefl->GetRepeatedUInt64(pbMessage, pbFieldDesc, j);
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_set_ulonglong(
                            ddsMemberData,
                            nullptr,
                            j + 1,
                            pbValue))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member unsigned long long value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_INT32:
                case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
                case google::protobuf::FieldDescriptor::TYPE_SINT32:
                {
                    const int32_t pbValue = pbRefl->GetRepeatedInt32(pbMessage, pbFieldDesc, j);
                    if (!rti::dyndata::set_fuzzy_long(
                            *ddsMemberData,
                            j + 1,
                            elTcKind,
                            pbValue))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member long value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_UINT32:
                case google::protobuf::FieldDescriptor::TYPE_FIXED32:
                {
                    const uint32_t pbValue = pbRefl->GetRepeatedUInt32(pbMessage, pbFieldDesc, j);
                    if (!rti::dyndata::set_fuzzy_ulong(
                            *ddsMemberData,
                            j + 1,
                            elTcKind,
                            pbValue))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member unsigned long value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_BOOL:
                {
                    const bool pbValue = pbRefl->GetRepeatedBool(pbMessage, pbFieldDesc, j);
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_set_boolean(
                            ddsMemberData,
                            nullptr,
                            j + 1,
                            pbValue))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member boolean value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_STRING:
                {
                    const std::string pbValue = pbRefl->GetRepeatedString(pbMessage, pbFieldDesc, j);
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_set_string(
                            ddsMemberData,
                            nullptr,
                            j + 1,
                            pbValue.c_str()))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member string value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_BYTES:
                {
                    const std::string pbValue = pbRefl->GetRepeatedString(pbMessage, pbFieldDesc, j);
                    if (!rti::dyndata::set_fuzzy_bytes(
                            *ddsMemberData,
                            j + 1,
                            elTc,
                            pbValue.data(),
                            static_cast<DDS_UnsignedLong>(pbValue.size())))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member bytes sequence value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                case google::protobuf::FieldDescriptor::TYPE_ENUM:
                {
                    const google::protobuf::EnumValueDescriptor * pbEnumValueDesc =
                        pbRefl->GetRepeatedEnum(pbMessage, pbFieldDesc, j);
                    if (pbEnumValueDesc == nullptr) {
                        RTI_TSFM_ERROR_3(
                            "failed to get enum value descriptor for repeated field",
                            "%s, type=%s, index=%d",
                            pbFieldDesc->name().c_str(),
                            pbFieldDesc->type_name(),
                            j)
                        return false;
                    }
                    const int pbEnumValue = pbEnumValueDesc->number();
                    if (DDS_RETCODE_OK !=
                        DDS_DynamicData_set_long(
                            ddsMemberData,
                            nullptr,
                            j + 1,
                            pbEnumValue))
                    {
                        RTI_TSFM_ERROR_2(
                            "failed to set DynamicData member enum value at index",
                            "%s[%d]",
                            ddsMemberInfo.member_name,
                            j)
                        return false;
                    }
                    break;
                }
                default:
                {
                    RTI_TSFM_ERROR_3(
                        "unsupported protobuf repeated scalar field type",
                        "%s, type=%d, index=%d",
                        pbFieldDesc->name().c_str(),
                        pbFieldDesc->type(),
                        j)
                    return false;
                }
            }
        }
    }

    return true;
}


bool
protobuf_to_dds(
    DDS_DynamicData & ddsData,
    const google::protobuf::Message & pbMessage,
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
    
    std::vector<const google::protobuf::FieldDescriptor *> pbFields;
    pbRefl->ListFields(pbMessage, &pbFields);
    int pbFieldsLen = static_cast<int>(pbFields.size());
    
    if (pbFieldsLen == 0) {
        // No fields set in PB message, nothing to do
        return true;
    }

    DDS_DynamicDataMemberInfo ddsMemberInfo = DDS_DynamicDataMemberInfo_INITIALIZER;

    const DDS_TypeCode * ddsMemberTc;
    DDS_TCKind ddsMemberTcKind = DDS_TK_NULL;

    for (int i = 0; i < pbFieldsLen; ++i) {
        const google::protobuf::FieldDescriptor * const pbFieldDesc = pbFields[i];

        if (!rti::dyndata::get_member_info(
                ddsData,
                pbFieldDesc->name().c_str(),
                ddsMemberInfo,
                ddsMemberTc,
                ddsMemberTcKind))
        {
            RTI_TSFM_ERROR_2(
                "failed to get DynamicData member info for protobuf field",
                "%s, field=%s",
                pbDesc->full_name().c_str(),
                pbFieldDesc->name().c_str())
            return false;
        }

        if (pbFieldDesc->is_repeated() || pbFieldDesc->is_map()) {
            if (!pb_repeated_field_to_dds(
                    convState,
                    ddsData,
                    ddsMemberInfo,
                    ddsMemberTc,
                    ddsMemberTcKind,
                    pbMessage,
                    pbDesc,
                    pbRefl,
                    pbFieldDesc))
            {
                RTI_TSFM_ERROR_2(
                    "failed to convert repeated protobuf field to DynamicData",
                    "%s, type=%s",
                    pbFieldDesc->name().c_str(),
                    pbFieldDesc->type_name())
                return false;
            }
        } else {
            if (!pb_singular_field_to_dds(
                    convState,
                    ddsData,
                    ddsMemberInfo,
                    ddsMemberTc,
                    ddsMemberTcKind,
                    pbMessage,
                    pbDesc,
                    pbRefl,
                    pbFieldDesc))
            {
                RTI_TSFM_ERROR_2(
                    "failed to convert singular protobuf field to DynamicData",
                    "%s, type=%s",
                    pbFieldDesc->name().c_str(),
                    pbFieldDesc->type_name())
                return false;
            }
        }
    }

    return true;
}

} // namespace protobuf
} // namespace dyndata
} // namespace rti

