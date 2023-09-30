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
/**
 * @brief Gets a DDS primitive or enum value for a specific field.
 * @param data Dynamic Data which contains the value.
 * @param field Field of the DynamicData to retrieve the value from.
 * @return The value from 'data' specified by 'field' as a long double.
 */
long double get_dds_primitive_or_enum_type_value(
        const dds::core::xtypes::DynamicData& data,
        const std::string& field);

/**
 * @brief Gets a DDS vector of values for a specific field.
 * @param data Dynamic Data which contains the value.
 * @param field Field of the DynamicData to retrieve the value from.
 * @return The value from 'data' specified by 'field' as a long double vector.
 */
std::vector<long double> get_vector_values(
        const dds::core::xtypes::DynamicData& data,
        const std::string& field);

/**
 * @brief Set a Dynamic Data primitive or enum value from a long double.
 * @param data Dynamic Data which contains the value.
 * @param type the typekind of the element to set.
 * @param field Field of the DynamicData to retrieve the value from.
 * @param float_value the value to set.
 */
void set_dds_primitive_or_enum_type_value(
        dds::core::xtypes::DynamicData& data,
        const dds::core::xtypes::TypeKind type,
        const std::string& field,
        const long double& float_value);

/**
 * @brief Set a Dynamic Data array from a long double vector.
 * @param data Dynamic Data which contains the value.
 * @param type the typekind of the element inside the vector
 * @param field Field of the DynamicData to retrieve the value from.
 * @param float_vector the array to set.
 */
void set_vector_values(
        dds::core::xtypes::DynamicData& data,
        const dds::core::xtypes::TypeKind type,
        const std::string& field,
        const std::vector<long double>& float_vector);

/**
 * @brief Check whether a TypeKind is signed or unsigned.
 * @param kind TypeKind for checking.
 * @return True if 'kind' is signed, false otherwise.
 */
bool is_signed_kind(const dds::core::xtypes::TypeKind kind);

/**
 * @brief Get the type of a nested member identified by field_name inside a
 * StructType.
 * @param struct_type StructType that contains the information about the struct.
 * @param field_name Field of the DynamicData to retrieve the type from. The
 * field name may identify nested elements. For example "shape.x" will look at
 * the struct 'shape', then at the element 'x'.
 * @return The DynamicType of the element 'field_name' inside the struct.
 */
dds::core::xtypes::DynamicType get_member_type(
        const dds::core::xtypes::StructType& struct_type,
        const std::string& field_name);

/**
 * @brief Copy an array or a sequence of primitive elements from a Dynamic Data
 * into an array with the same type of elements in another DynamicData sample.
 * The index specifies the sequence or array that is being copied.
 * @param input the input DynamicData.
 * @param output the output DynamicData.
 * @param index that identifies the field to copy (in both, input and output).
 * @param max_elements maximum number of element to copy. This is usually the
 * array max elements.
 */
void copy_primitive_array_elements(
        dds::core::xtypes::DynamicData& input,
        dds::core::xtypes::DynamicData& output,
        uint32_t index,
        uint32_t max_elements);

/**
 * @brief Copy an array or a sequence of primitive elements from a Dynamic Data
 * into a sequence with the same type of elements in another DynamicData sample.
 * The index specifies the sequence or array that is being copied.
 * @param input the input DynamicData.
 * @param output the output DynamicData.
 * @param index that identifies the field to copy (in both, input and output).
 * @param max_elements maximum number of element to copy. This is usually the
 * array max elements.
 */
void copy_primitive_sequence_elements(
        dds::core::xtypes::DynamicData& input,
        dds::core::xtypes::DynamicData& output,
        uint32_t index,
        uint32_t max_elements);

/**
 * @brief Copy a primitive element from a Dynamic Data to another. The element
 * in both Dynamic Data should be located at 'index'
 * @param input the input DynamicData.
 * @param output the output DynamicData.
 * @param index that identifies the field to copy (in both, input and output).
 * @param max_elements maximum number of element to copy. This is usually the
 * array max elements.
 */
void copy_primitive_member(
        dds::core::xtypes::DynamicData& input,
        dds::core::xtypes::DynamicData& output,
        uint32_t index);

}}}  // namespace rti::common::dynamic_data
