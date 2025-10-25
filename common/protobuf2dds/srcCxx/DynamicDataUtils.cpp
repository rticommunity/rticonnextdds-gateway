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
// Avoid definition of min/max macros on Windows
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <limits>

#include "DynamicDataConverter.hpp"

namespace rti {
namespace dyndata {

bool
resolve_type_code(
    const DDS_TypeCode * & type_code,
    DDS_TCKind & type_code_kind)
{
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    DDS_TCKind tc_kind = DDS_TypeCode_kind(type_code, &ex);
    if (ex != DDS_NO_EXCEPTION_CODE) {
        return false;
    }

    if (tc_kind == DDS_TK_ALIAS) {
        const DDS_TypeCode * const content_tc = DDS_TypeCode_content_type(type_code, &ex);
        if (content_tc == nullptr || ex != DDS_NO_EXCEPTION_CODE) {
            return false;
        }
        type_code = content_tc;
        if (!resolve_type_code(type_code, tc_kind)) {
            return false;
        }
    }
    type_code_kind = tc_kind;
    return true;
}


bool
get_member_info(
    DDS_DynamicData & data,
    const char * const member_name,
    DDS_DynamicDataMemberInfo & member_info,
    const DDS_TypeCode * & member_tc,
    DDS_TCKind & member_tc_kind)
{
    if (DDS_RETCODE_OK !=
        DDS_DynamicData_get_member_info(
            &data,
            &member_info,
            member_name,
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED))
    {
        return false;
    }

    if (DDS_RETCODE_OK !=
        DDS_DynamicData_get_member_type(
            &data, &member_tc, nullptr, member_info.member_id))
    {
        return false;
    }

    if (!resolve_type_code(member_tc, member_tc_kind)) {
        return false;
    }

    return true;
}

bool
set_fuzzy_long(
    DDS_DynamicData & data,
    const DDS_DynamicDataMemberId member_id,
    const DDS_TCKind member_kind,
    const int32_t value)
{
    switch (member_kind) {
        case DDS_TK_SHORT:
        {
            if (value < std::numeric_limits<DDS_Short>::min() ||
                value > std::numeric_limits<DDS_Short>::max())
            {
                return false;
            }
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_short(
                    &data,
                    nullptr,
                    member_id,
                    static_cast<DDS_Short>(value)))
            {
                return false;
            }
            break;
        }
        case DDS_TK_CHAR:
        {
            if (value < std::numeric_limits<DDS_Char>::min() ||
                value > std::numeric_limits<DDS_Char>::max())
            {
                return false;
            }
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_char(
                    &data,
                    nullptr,
                    member_id,
                    static_cast<DDS_Char>(value)))
            {
                return false;
            }
            break;
        }
        default:
        {
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_long(
                    &data,
                    nullptr,
                    member_id,
                    value))
            {
                return false;
            }
            break;
        }
    }

    return true;
}


bool
set_fuzzy_ulong(
    DDS_DynamicData & data,
    const DDS_DynamicDataMemberId member_id,
    const DDS_TCKind member_kind,
    const uint32_t value)
{
    switch (member_kind) {
        case DDS_TK_USHORT:
        {
            if (value > std::numeric_limits<DDS_UnsignedShort>::max()) {
                return false;
            }
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_ushort(
                    &data,
                    nullptr,
                    member_id,
                    static_cast<DDS_UnsignedShort>(value)))
            {
                return false;
            }
            break;
        }
        case DDS_TK_OCTET:
        {
            if (value > std::numeric_limits<DDS_Octet>::max()) {
                return false;
            }
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_octet(
                    &data,
                    nullptr,
                    member_id,
                    static_cast<DDS_Octet>(value)))
            {
                return false;
            }
            break;
        }
        default:
        {
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_ulong(
                    &data,
                    nullptr,
                    member_id,
                    value))
            {
                return false;
            }
            break;
        }
    }

    return true;
}

bool
set_fuzzy_bytes(
    DDS_DynamicData & data,
    const DDS_DynamicDataMemberId member_id,
    const DDS_TypeCode * const member_tc,
    const void * const value,
    const DDS_UnsignedLong value_len)
{
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
    const DDS_TypeCode * el_tc = DDS_TypeCode_content_type(member_tc, &ex);
    if (el_tc == nullptr || ex != DDS_NO_EXCEPTION_CODE) {
        return false;
    }
    DDS_TCKind el_tc_kind = DDS_TK_NULL;
    if (!resolve_type_code(el_tc, el_tc_kind)) {
        return false;
    }
    switch (el_tc_kind) {
        case DDS_TK_CHAR:
        {
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_char_array(
                    &data,
                    nullptr,
                    member_id,
                    value_len,
                    static_cast<const DDS_Char *>(value)))
            {
                return false;
            }
            break;
        }
        default:
        {
            if (DDS_RETCODE_OK !=
                DDS_DynamicData_set_octet_array(
                    &data,
                    nullptr,
                    member_id,
                    value_len,
                    static_cast<const DDS_Octet *>(value)))
            {
                return false;
            }
            break;
        }
    }
    return true;
}

} // namespace dyndata
} // namespace rti
