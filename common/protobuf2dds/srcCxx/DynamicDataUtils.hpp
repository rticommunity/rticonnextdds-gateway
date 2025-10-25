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
#ifndef DynamicDataUtils_hpp
#define DynamicDataUtils_hpp

#include "ndds/ndds_c.h"

namespace rti {
namespace dyndata {

bool
resolve_type_code(
    const DDS_TypeCode * & type_code,
    DDS_TCKind & type_code_kind);

bool
get_member_info(
    DDS_DynamicData & data,
    const char * const member_name,
    DDS_DynamicDataMemberInfo & member_info,
    const DDS_TypeCode * & member_tc,
    DDS_TCKind & member_tc_kind);

bool
set_fuzzy_long(
    DDS_DynamicData & data,
    const DDS_DynamicDataMemberId member_id,
    const DDS_TCKind member_kind,
    const int32_t value);

bool
set_fuzzy_ulong(
    DDS_DynamicData & data,
    const DDS_DynamicDataMemberId member_id,
    const DDS_TCKind member_kind,
    const uint32_t value);

bool
set_fuzzy_bytes(
    DDS_DynamicData & data,
    const DDS_DynamicDataMemberId member_id,
    const DDS_TypeCode * const member_tc,
    const void * const value,
    const DDS_UnsignedLong value_len);

} // namespace dyndata
} // namespace rti

#endif // DynamicDataUtils_hpp
