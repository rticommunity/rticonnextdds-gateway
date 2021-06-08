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

#pragma once

#include <dds/dds.hpp>

namespace rti { namespace common { namespace dynamic_data {
long double get_dds_primitive_or_enum_type_value(
        const dds::core::xtypes::DynamicData& data,
        const std::string& field);

std::vector<long double> get_vector_values(
        const dds::core::xtypes::DynamicData& data,
        const std::string& field);

void set_dds_primitive_or_enum_type_value(
        dds::core::xtypes::DynamicData& data,
        const dds::core::xtypes::TypeKind type,
        const std::string& field,
        const long double& float_value);

void set_vector_values(
        dds::core::xtypes::DynamicData& data,
        const dds::core::xtypes::TypeKind type,
        const std::string& field,
        const std::vector<long double>& float_vector);

dds::core::xtypes::DynamicType get_member_type(
        const dds::core::xtypes::StructType& struct_type,
        const std::string& field_name);

bool is_signed_kind(const dds::core::xtypes::TypeKind kind);
}}}  // namespace rti::common::dynamic_data
