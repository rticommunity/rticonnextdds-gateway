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

#include "DynamicDataHelpers.hpp"
#include "UtilsStrings.hpp"

using namespace dds::core::xtypes;

long double
    rti::common::dynamic_data::get_dds_primitive_or_enum_type_value(
        const DynamicData& data,
        const std::string &field)
{
    long double float_value = 0;
    switch (data.member_info(field).member_kind().underlying()) {
    case TypeKind::BOOLEAN_TYPE:
        float_value = data.value<bool>(field) ? 1 : 0;
        break;
    case TypeKind::CHAR_8_TYPE:
        float_value = data.value<char>(field);
        break;
    case TypeKind::UINT_8_TYPE:
        float_value = data.value<uint8_t>(field);
        break;
    case TypeKind::INT_16_TYPE:
        float_value = data.value<int16_t>(field);
        break;
    case TypeKind::UINT_16_TYPE:
        float_value = data.value<uint16_t>(field);
        break;
    case TypeKind::INT_32_TYPE:
    case TypeKind::ENUMERATION_TYPE:
        float_value = data.value<int32_t>(field);
        break;
    case TypeKind::UINT_32_TYPE:
        float_value = data.value<uint32_t>(field);
        break;
    case TypeKind::INT_64_TYPE:
        float_value = static_cast<long double>(data.value<int64_t>(field));
        break;
    case TypeKind::UINT_64_TYPE:
        float_value = static_cast<long double>(data.value<uint64_t>(field));
        break;
    case TypeKind::FLOAT_32_TYPE:
        float_value = data.value<float>(field);
        break;
    case TypeKind::FLOAT_64_TYPE:
        float_value = data.value<double>(field);
        break;
    default:
        std::string error("Error: unsupported type of <" + field + ">");
        throw std::runtime_error(error);
    }
    return float_value;
}

std::vector<long double>
    rti::common::dynamic_data::get_vector_values(
        const DynamicData& data,
        const std::string& field)
{
    std::vector<long double> float_vector;
    size_t size = data.member_info(field).element_count();

    switch (data.member_info(field).element_kind().underlying()) {
    case TypeKind::CHAR_8_TYPE: {
        std::vector<char> members(size);
        data.get_values<char>(field, members);
        for (auto member : members) {
            float_vector.push_back(member);
        }
        break;
    }
    case TypeKind::BOOLEAN_TYPE:
    // booleans are stored in memory as uint8_t for arrays/seqs
    case TypeKind::UINT_8_TYPE: {
        std::vector<uint8_t> members(size);
        data.get_values<uint8_t>(field, members);
        for (auto member : members) {
            float_vector.push_back(member);
        }
        break;
    }
    case TypeKind::INT_16_TYPE: {
        std::vector<int16_t> members(size);
        data.get_values<int16_t>(field, members);
        for (auto member : members) {
            float_vector.push_back(member);
        }
        break;
    }
    case TypeKind::UINT_16_TYPE: {
        std::vector<uint16_t> members(size);
        data.get_values<uint16_t>(field, members);
        for (auto member : members) {
            float_vector.push_back(member);
        }
        break;
    }
    case TypeKind::ENUMERATION_TYPE:
    case TypeKind::INT_32_TYPE: {
        std::vector<DDS_Long> members(size);
        data.get_values<DDS_Long>(field, members);
        for (auto member : members) {
            float_vector.push_back(member);
        }
        break;
    }
    case TypeKind::UINT_32_TYPE: {
        std::vector<DDS_UnsignedLong> members(size);
        data.get_values<DDS_UnsignedLong>(field, members);
        for (auto member : members) {
            float_vector.push_back(member);
        }
        break;
    }
    case TypeKind::INT_64_TYPE: {
        std::vector<DDS_LongLong> members(size);
        data.get_values<DDS_LongLong>(field, members);
        for (auto member : members) {
            float_vector.push_back(static_cast<long double>(member));
        }
        break;
    }
    case TypeKind::UINT_64_TYPE: {
        std::vector<DDS_UnsignedLongLong> members(size);
        data.get_values<DDS_UnsignedLongLong>(field, members);
        for (auto member : members) {
            float_vector.push_back(static_cast<long double>(member));
        }
        break;
    }
    case TypeKind::FLOAT_32_TYPE: {
        std::vector<float> members(size);
        data.get_values<float>(field, members);
        for (auto member : members) {
            float_vector.push_back(member);
        }
        break;
    }
    case TypeKind::FLOAT_64_TYPE: {
        std::vector<double> members(size);
        data.get_values<double>(field, members);
        for (auto member : members) {
            float_vector.push_back(member);
        }
        break;
    }
    default:
        std::string error("Error: unsupported member type of <" + field + ">");
        throw std::runtime_error(error);
    }
    return float_vector;
}


void rti::common::dynamic_data::set_dds_primitive_or_enum_type_value(
        DynamicData& data,
        const TypeKind type,
        const std::string& field,
        const long double& float_value)
{
    switch (type.underlying()) {
    case TypeKind::BOOLEAN_TYPE:
        data.value<bool>(field, static_cast<bool>(float_value ? 1 : 0));
        break;
    case TypeKind::CHAR_8_TYPE:
        data.value<char>(field, static_cast<char>(float_value));
        break;
    case TypeKind::UINT_8_TYPE:
        data.value<uint8_t>(field, static_cast<uint8_t>(float_value));
        break;
    case TypeKind::INT_16_TYPE:
        data.value<int16_t>(field, static_cast<int16_t>(float_value));
        break;
    case TypeKind::UINT_16_TYPE: {
        // data.value<uint16_t>(field, static_cast<uint16_t>(float_value));
        // affected by CORE-10286, use C API as a workaround
        auto retcode = DDS_DynamicData_set_ushort(
                &data.native(),
                field.c_str(),
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                static_cast<uint16_t>(float_value));
        if (retcode != DDS_RETCODE_OK) {
            std::string error("Error: setting ushort value of <" + field + ">");
            throw std::runtime_error(error);
        }
        break;
    }
    case TypeKind::INT_32_TYPE:
    case TypeKind::ENUMERATION_TYPE:
        data.value<int32_t>(field, static_cast<int32_t>(float_value));
        break;
    case TypeKind::UINT_32_TYPE:
        data.value<uint32_t>(field, static_cast<uint32_t>(float_value));
        break;
    case TypeKind::INT_64_TYPE:
        data.value<int64_t>(field, static_cast<int64_t>(float_value));
        break;
    case TypeKind::UINT_64_TYPE:
        data.value<uint64_t>(field, static_cast<uint64_t>(float_value));
        break;
    case TypeKind::FLOAT_32_TYPE:
        data.value<float>(field, static_cast<float>(float_value));
        break;
    case TypeKind::FLOAT_64_TYPE:
        data.value<double>(field, static_cast<double>(float_value));
        break;
    default:
        std::string error("Error: unsupported type of <" + field + ">");
        throw std::runtime_error(error);
    }
}

void rti::common::dynamic_data::set_vector_values(
        DynamicData& data,
        const TypeKind type,
        const std::string& field,
        const std::vector<long double>& float_vector)
{
    switch (type.underlying()) {
    case TypeKind::CHAR_8_TYPE: {
        std::vector<char> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<char>(element));
        }
        data.set_values<char>(field, values);
        break;
    }
    case TypeKind::BOOLEAN_TYPE:
    // booleans are stored in memory as an uint8 for arrays/seqs
    case TypeKind::UINT_8_TYPE: {
        std::vector<uint8_t> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<uint8_t>(element));
        }
        data.set_values<uint8_t>(field, values);
        break;
    }
    case TypeKind::INT_16_TYPE: {
        std::vector<int16_t> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<int16_t>(element));
        }
        data.set_values<int16_t>(field, values);
        break;
    }
    case TypeKind::UINT_16_TYPE: {
        std::vector<uint16_t> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<uint16_t>(element));
        }
        // data.set_values<uint16_t>(field, values);
        // affected by CORE-10286, use C API as a workaround
        auto retcode = DDS_DynamicData_set_ushort_array(
                &data.native(),
                field.c_str(),
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                static_cast<DDS_UnsignedLong>(values.size()),
                &values[0]);
        if (retcode != DDS_RETCODE_OK) {
            std::string error(
                    "Error: setting ushort array value of <" + field + ">");
            throw std::runtime_error(error);
        }
        break;
    }
    case TypeKind::ENUMERATION_TYPE:
    case TypeKind::INT_32_TYPE: {
        std::vector<DDS_Long> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<DDS_Long>(element));
        }
        data.set_values<DDS_Long>(field, values);
        break;
    }
    case TypeKind::UINT_32_TYPE: {
        std::vector<DDS_UnsignedLong> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<DDS_UnsignedLong>(element));
        }
        data.set_values<DDS_UnsignedLong>(field, values);
        break;
    }
    case TypeKind::INT_64_TYPE: {
        std::vector<DDS_LongLong> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<DDS_LongLong>(element));
        }
        data.set_values<DDS_LongLong>(field, values);
        break;
    }
    case TypeKind::UINT_64_TYPE: {
        std::vector<DDS_UnsignedLongLong> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<DDS_UnsignedLongLong>(element));
        }
        data.set_values<DDS_UnsignedLongLong>(field, values);
        break;
    }
    case TypeKind::FLOAT_32_TYPE: {
        std::vector<float> values;
        for (auto element : float_vector) {
            values.push_back(static_cast<float>(element));
        }
        data.set_values<float>(field, values);
        break;
    }
    case TypeKind::FLOAT_64_TYPE: {
        std::vector<double> values;
        for (auto element : float_vector) {
            values.push_back(element);
        }
        data.set_values<double>(field, values);
        break;
    }
    default:
        std::string error("Error: unsupported member type of <" + field + ">");
        throw std::runtime_error(error);
    }
}

bool rti::common::dynamic_data::is_signed_kind(TypeKind kind)
{
    switch (kind.underlying()) {
    case TypeKind::CHAR_8_TYPE:
    case TypeKind::INT_16_TYPE:
    case TypeKind::ENUMERATION_TYPE:
    case TypeKind::INT_32_TYPE:
    case TypeKind::INT_64_TYPE:
    case TypeKind::FLOAT_32_TYPE:
    case TypeKind::FLOAT_64_TYPE:
    case TypeKind::FLOAT_128_TYPE:
        return true;
    case TypeKind::UINT_8_TYPE:
    case TypeKind::UINT_16_TYPE:
    case TypeKind::UINT_32_TYPE:
    case TypeKind::UINT_64_TYPE:
        return false;
    default:
        std::string error("Error: unsupported numeric member type.");
        throw std::runtime_error(error);
    }
}

DynamicType rti::common::dynamic_data::get_member_type(
        const StructType& struct_type,
        const std::string& field_name)
{
    DynamicType *type = nullptr;
    // if nested type
    if (field_name.find('.') != std::string::npos) {
        std::vector<std::string> nested_elements;
        nested_elements = rti::utils::strings::split(field_name, '.');
        type = const_cast<DynamicType *>(
                &struct_type.member(nested_elements[0]).type());
        // the loop starts in 1 because we already has taken the first element
        for (int i = 1; i < nested_elements.size(); ++i) {
            if (type->kind() == TypeKind::STRUCTURE_TYPE) {
                const StructType &dynamic_struct =
                        static_cast<const StructType &>(*type);
                type = const_cast<DynamicType *>(
                        &dynamic_struct.member(nested_elements[i]).type());
            } else {
                std::string error(
                        "Error: nested member only can belong to structs.");
                throw std::runtime_error(error);
            }
        }
    } else {
        type = const_cast<DynamicType *>(
                &struct_type.member(field_name).type());
    }
    return *type;
}
