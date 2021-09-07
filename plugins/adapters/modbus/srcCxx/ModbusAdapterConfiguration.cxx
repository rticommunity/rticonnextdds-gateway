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

#include <fstream>
#include <iostream>
#include <vector>

#include <dds/dds.hpp>

#include "DynamicDataHelpers.hpp"
#include "LibModbusClient.hpp"
#include "ModbusAdapterConfiguration.hpp"
#include "json.hpp"

using namespace rti::adapter::modbus;
using namespace rti::common;
using namespace rti::json;
using namespace dds::core::xtypes;

///////////////////////////////////////////////////////////////////////////////
////////////////// ModbusAdapterConfigurationElement class ////////////////////
///////////////////////////////////////////////////////////////////////////////

ModbusAdapterConfigurationElement::ModbusAdapterConfigurationElement()
        : field_(""),
          modbus_register_address_(-1),
          modbus_register_count_(0),
          modbus_datatype_(ModbusDataType::no_modbus_datatype),
          modbus_slave_device_id_(LibModbusClient::MODBUS_DEFAULT_DEVICE_ID),
          modbus_min_value_(RTI_FLOAT_MIN),
          modbus_max_value_(RTI_FLOAT_MAX),
          modbus_valid_values_(),
          data_factor_(1),
          data_offset_(0),
          constant_kind_(ConstantValueKind::undefined_kind),
          array_elements_(0)
{
}

bool ModbusAdapterConfigurationElement::is_compatible_dds_datatype(
        dds::core::xtypes::TypeKind dds_datatype)
{
    bool is_compatible = false;

    // Check that if the datatype is unsigned, its minimun is >= 0
    // int8 and uint8 are mapped as octets --> unsigned, therefore
    // the int8 won't allow negative values.
    // int8 and uint8 doesn't require to have a min_value > 0, both
    // will act as octets and may store "negative numbers".
    if (modbus_datatype() != ModbusDataType::constant_value
            && modbus_datatype() != ModbusDataType::coil_boolean
            && modbus_datatype() != ModbusDataType::discrete_input_boolean) {
        if (!dynamic_data::is_signed_kind(dds_datatype)
                && modbus_datatype() != ModbusDataType::holding_register_int8
                && modbus_datatype() != ModbusDataType::input_register_int8) {
            if (data_factor() * modbus_min_value() + data_offset() < 0) {
                std::string error("Error: the minimum value produced by "
                        "(data_factor() * modbus_min_value() + data_offset()) "
                        "of the field by <" + field() + "> should greater or "
                        "equal to 0.");
                throw std::runtime_error(error);
            }
        }
    }

    if (dds_datatype == TypeKind::ENUMERATION_TYPE) {
        switch (modbus_datatype()) {
        case ModbusDataType::holding_register_int8:
        case ModbusDataType::input_register_int8:
        case ModbusDataType::holding_register_int16:
        case ModbusDataType::input_register_int16:
        case ModbusDataType::holding_register_int32:
        case ModbusDataType::input_register_int32:
        case ModbusDataType::holding_register_int64:
        case ModbusDataType::input_register_int64:
            if (!modbus_valid_values_.empty()) {
                is_compatible = true;
            }
            break;
        default:
            is_compatible = false;
        }
    } else {
        switch (modbus_datatype()) {
        case ModbusDataType::holding_register_int8:
        case ModbusDataType::input_register_int8:
            is_compatible = dds_datatype == TypeKind::CHAR_8_TYPE
                    || dds_datatype == TypeKind::UINT_8_TYPE;
        case ModbusDataType::holding_register_int16:
        case ModbusDataType::input_register_int16:
            is_compatible = is_compatible
                    || dds_datatype == TypeKind::INT_16_TYPE
                    || dds_datatype == TypeKind::UINT_16_TYPE;
        case ModbusDataType::holding_register_int32:
        case ModbusDataType::input_register_int32:
            is_compatible = is_compatible
                    || dds_datatype == TypeKind::INT_32_TYPE
                    || dds_datatype == TypeKind::UINT_32_TYPE;
        case ModbusDataType::holding_register_int64:
        case ModbusDataType::input_register_int64:
            is_compatible = is_compatible
                    || dds_datatype == TypeKind::INT_64_TYPE
                    || dds_datatype == TypeKind::UINT_64_TYPE;
        case ModbusDataType::holding_register_float_abcd:
        case ModbusDataType::holding_register_float_badc:
        case ModbusDataType::holding_register_float_cdab:
        case ModbusDataType::holding_register_float_dcba:
        case ModbusDataType::input_register_float_abcd:
        case ModbusDataType::input_register_float_badc:
        case ModbusDataType::input_register_float_cdab:
        case ModbusDataType::input_register_float_dcba:
            is_compatible = is_compatible
                    || dds_datatype == TypeKind::FLOAT_32_TYPE
                    || dds_datatype == TypeKind::FLOAT_64_TYPE;
            break;
        case ModbusDataType::coil_boolean:
        case ModbusDataType::discrete_input_boolean:
            // A COIL may fit into a boolean or any integer value
            is_compatible = dds_datatype == TypeKind::BOOLEAN_TYPE
                    || dds_datatype == TypeKind::CHAR_8_TYPE
                    || dds_datatype == TypeKind::UINT_8_TYPE
                    || dds_datatype == TypeKind::INT_16_TYPE
                    || dds_datatype == TypeKind::UINT_16_TYPE
                    || dds_datatype == TypeKind::INT_32_TYPE
                    || dds_datatype == TypeKind::UINT_32_TYPE
                    || dds_datatype == TypeKind::INT_64_TYPE
                    || dds_datatype == TypeKind::UINT_64_TYPE;
            break;
        case ModbusDataType::constant_value:
            is_compatible = dds_datatype == TypeKind::STRING_TYPE;
            break;
        default:
            is_compatible = false;
            break;
        }
    }
    return is_compatible;
}

std::string ModbusAdapterConfigurationElement::get_value_string(
        long double value,
        TypeKind element_kind)
{
    switch (element_kind.underlying()) {
    case TypeKind::CHAR_8_TYPE:
        return std::to_string((int8_t) value);
    case TypeKind::UINT_8_TYPE:
        return std::to_string((uint8_t) value);
    case TypeKind::INT_16_TYPE:
        return std::to_string((int16_t) value);
    case TypeKind::UINT_16_TYPE:
        return std::to_string((uint16_t) value);
    case TypeKind::ENUMERATION_TYPE:
    case TypeKind::INT_32_TYPE:
        return std::to_string((int32_t) value);
    case TypeKind::UINT_32_TYPE:
        return std::to_string((uint32_t) value);
    case TypeKind::INT_64_TYPE:
        return std::to_string((int64_t) value);
    case TypeKind::UINT_64_TYPE:
        return std::to_string((uint64_t) value);
    case TypeKind::FLOAT_32_TYPE:
        return std::to_string((float) value);
    case TypeKind::FLOAT_64_TYPE:
        return std::to_string((double) value);
    default:
        // Unsupported datatype, return empty string
        return std::string("");
    }
}

void ModbusAdapterConfigurationElement::check_for_errors(void)
{
    // Check for manatory parameters are correctly set:
    //  - field
    //  - modbus_datatype
    //  - modbus_register_address
    if (field() == "") {
        std::string error(
                "Error: the field parameter is not set in at least "
                "one parameter.");
        throw std::runtime_error(error);
    }

    if (modbus_datatype() == ModbusDataType::no_modbus_datatype) {
        std::string error(
                "Error: the modbus_datatype parameter of <" + field()
                + "> is not set.");
        throw std::runtime_error(error);
    }

    if (modbus_register_address() == -1
        && modbus_datatype() != ModbusDataType::constant_value) {
        std::string error(
                "Error: the modbus_register_address parameter of <" + field()
                + "> is not set.");
        throw std::runtime_error(error);
    }

    // if value is set and CONSTANT_VALUE is not it's datatype
    if (modbus_datatype() != ModbusDataType::constant_value
            && constant_kind() != ConstantValueKind::undefined_kind) {
        std::string error(
                "Error: value parameter of <" + field()
                + "> is set and its datatype is <"
                + modbus_datatype_to_string(modbus_datatype()) + ">");
        throw std::runtime_error(error);
    }

    // if the datatype is COIL_BOOLEAN or CONSTANT_VALUE and any of the
    // following parameters are set, print a warning. These parameters will
    // be ignored:
    //  - modbus_min_value
    //  - modbus_max_value
    //  - modbus_valid_values
    //  - output_data_factor
    //  - output_data_offset
    //  - input_data_factor
    //  - input_data_offset
    if (modbus_datatype() == ModbusDataType::coil_boolean
        || modbus_datatype() == ModbusDataType::constant_value) {
        if (modbus_min_value() != RTI_FLOAT_MIN) {
            std::cerr
                    << "Warning: The parameter modbus_min_value of the field <"
                    << field() << "> will be ignored as it is a COIL_BOOLEAN "
                    " or has a constant value set.";
        }
        if (modbus_max_value() != RTI_FLOAT_MAX) {
            std::cerr
                    << "Warning: The parameter modbus_max_value of the field <"
                    << field() << "> will be ignored as it is a COIL_BOOLEAN "
                    " or has a constant value set.";
        }
        if (!modbus_valid_values().empty()) {
            std::cerr << "Warning: The parameter modbus_valid_values of the "
                    "field <" << field() << "> will be ignored as it is a "
                    "COIL_BOOLEAN or has a constant value set.";
        }
        if (data_factor() != 1) {
            std::cerr << "Warning: The parameter data_factor of the field <"
                    << field() << "> will be ignored as it is a COIL_BOOLEAN "
                    " or has a constant value set.";
        }
        if (data_offset() != 0) {
            std::cerr << "Warning: The parameter data_offset of the field <"
                    << field() << "> will be ignored as it is a COIL_BOOLEAN "
                    " or has a constant value set.";
        }
    }

    // if this is a CONSTANT_VALUE the following parameters will be ignored:
    //  - modbus_register_address
    //  - modbus_register_count
    if (modbus_datatype() == ModbusDataType::constant_value) {
        if (modbus_register_address() != -1) {
            std::cerr << "Warning: The parameter modbus_register_address "
                    "of the field <" << field() << "> will be ignored "
                    "as it has a constant value set.";
        }
        if (modbus_register_count() != 0) {
            std::cerr << "Warning: The parameter modbus_register_count "
                    "of the field <" << field() << "> will be ignored "
                    "as it has a constant value set.";
        }
    }
}

void ModbusAdapterConfigurationElement::check_correct_value(
        long double float_value,
        size_t index,
        TypeKind element_kind)
{
    // The value is not out of range
    if (float_value < modbus_min_value() || float_value > modbus_max_value()) {
        std::string element_number =
                array_elements() > 0 ? "[" + std::to_string(index) + "]" : "";
        std::string error(
                "Error: value <" + get_value_string(float_value, element_kind)
                + "> of element <" + field() + element_number
                + "> out of range ["
                + get_value_string(modbus_min_value(), element_kind) + ","
                + get_value_string(modbus_max_value(), element_kind) + "].");
        throw std::runtime_error(error);
    }

    if (!modbus_valid_values().empty()) {
        // if there are valid values, check the value is in the list
        if (!std::any_of(
                    modbus_valid_values().begin(),
                    modbus_valid_values().end(),
                    [&](long double elem) { return elem == float_value; })) {
            // element not found in the modbus_valid_values list
            std::string element_number = array_elements() > 0
                    ? "[" + std::to_string(index) + "]"
                    : "";
            std::string error(
                    "Error: value <"
                    + get_value_string(float_value, element_kind)
                    + "> of element <" + field() + element_number
                    + "> not in the modbus_valid_values list.");
            throw std::runtime_error(error);
        }
    }
}

void ModbusAdapterConfigurationElement::get_registers_value(
        std::vector<uint16_t>& output,
        const std::vector<long double>& float_vector,
        TypeKind element_kind)
{
    auto array_data = reinterpret_cast<uint16_t *>(output.data());
    for (int i = 0; i < float_vector.size(); ++i) {
        // check that the data_offset + value * data_factor is a correct
        // value. This is done, because the linear transformation es what
        // will be written in modbus
        check_correct_value(
                data_offset() + float_vector[i] * data_factor(),
                i,
                element_kind);

        switch (modbus_datatype()) {
        case ModbusDataType::holding_register_int8:
            // cast the element to it corresponding type after doing all the
            // maths with the data_factor and data_offset
            array_data[0] = static_cast<uint8_t>(
                    data_offset() + float_vector[i] * data_factor());
            break;
        case ModbusDataType::holding_register_int16:
            array_data[0] = static_cast<uint16_t>(
                    data_offset() + float_vector[i] * data_factor());
            break;
        case ModbusDataType::holding_register_int32:
            LibModbusClient::int32_to_int16(
                    array_data,
                    static_cast<uint32_t>(
                            data_offset() + float_vector[i] * data_factor()));
            break;
        case ModbusDataType::holding_register_int64:
            LibModbusClient::int64_to_int16(
                    array_data,
                    static_cast<uint64_t>(
                            data_offset() + float_vector[i] * data_factor()));
            break;
        case ModbusDataType::holding_register_float_abcd:
            LibModbusClient::float_to_int16_abcd(
                    array_data,
                    static_cast<float>(
                            data_offset() + float_vector[i] * data_factor()));
            break;
        case ModbusDataType::holding_register_float_badc:
            LibModbusClient::float_to_int16_badc(
                    array_data,
                    static_cast<float>(
                            data_offset() + float_vector[i] * data_factor()));
            break;
        case ModbusDataType::holding_register_float_cdab:
            LibModbusClient::float_to_int16_cdab(
                    array_data,
                    static_cast<float>(
                            data_offset() + float_vector[i] * data_factor()));
            break;
        case ModbusDataType::holding_register_float_dcba:
            LibModbusClient::float_to_int16_dcba(
                    array_data,
                    static_cast<float>(
                            data_offset() + float_vector[i] * data_factor()));
            break;
        default:
            // INPUT registers cannot be here because this function translates
            // from float_vector to uint16_t* to write into a modbus device,
            // and INPUT registers are read-only
            std::string error(
                    "Error: invalid datatype <"
                    + modbus_datatype_to_string(modbus_datatype())
                    + "> in the field <" + field() + ">.");
            throw std::runtime_error(error);
        }
        // update the pointer to point to the next 'empty' element. The step
        // of this array (uint16_t) depends on the type which has been written
        // so the step will be the primitive_type()
        array_data += number_of_registers_primitive_type();
    }
}

std::vector<long double> ModbusAdapterConfigurationElement::get_float_value(
        const std::vector<uint16_t>& input,
        TypeKind element_kind)
{
    std::vector<long double> float_vector;

    for (size_t i = 0;
            i < modbus_register_count();
            i += number_of_registers_primitive_type()) {
        size_t index = i / number_of_registers_primitive_type();

        switch (modbus_datatype()) {
        // in this case, check_corrent_value() checks directly the value
        // that has been read from modbus.
        case ModbusDataType::holding_register_int8:
        case ModbusDataType::input_register_int8: {
            // cast the value into signed/unsigned depending on
            // the datatype used
            if (dynamic_data::is_signed_kind(element_kind)) {
                int8_t value = static_cast<int8_t>(input[i]);
                check_correct_value(
                        static_cast<long double>(static_cast<int8_t>(value)),
                        index,
                        element_kind);

                float_vector.push_back(
                        (int8_t) data_offset() + value * data_factor());
            } else {
                uint8_t value = static_cast<uint8_t>(input[i]);
                check_correct_value(
                        static_cast<long double>(static_cast<uint8_t>(value)),
                        index,
                        element_kind);

                float_vector.push_back(
                        (unsigned) (int8_t) data_offset()
                        + value * data_factor());
            }
            break;
        }
        case ModbusDataType::holding_register_int16:
        case ModbusDataType::input_register_int16: {
            if (dynamic_data::is_signed_kind(element_kind)) {
                int16_t value = input[i];
                check_correct_value(value, index, element_kind);

                float_vector.push_back(
                        (int16_t) data_offset() + value * data_factor());
            } else {
                uint16_t value = input[i];
                check_correct_value(
                        static_cast<long double>(static_cast<uint16_t>(value)),
                        index,
                        element_kind);

                float_vector.push_back(
                        (unsigned) (int16_t) data_offset()
                        + value * data_factor());
            }
            break;
        }
        case ModbusDataType::holding_register_int32:
        case ModbusDataType::input_register_int32: {
            if (dynamic_data::is_signed_kind(element_kind)) {
                int32_t value = 0;
                LibModbusClient::int16_to_int32(
                        (uint32_t &) value,
                        const_cast<uint16_t *>(input.data()) + i);

                check_correct_value(
                        static_cast<long double>(static_cast<int32_t>(value)),
                        index,
                        element_kind);

                float_vector.push_back(
                        (int32_t) data_offset() + value * data_factor());
            } else {
                uint32_t value = 0;
                LibModbusClient::int16_to_int32(
                        value,
                        const_cast<uint16_t *>(input.data()) + i);

                check_correct_value(
                        static_cast<long double>(static_cast<uint32_t>(value)),
                        index,
                        element_kind);

                float_vector.push_back(
                        (unsigned) (int32_t) data_offset()
                        + value * data_factor());
            }
            break;
        }
        case ModbusDataType::holding_register_int64:
        case ModbusDataType::input_register_int64: {
            if (dynamic_data::is_signed_kind(element_kind)) {
                int64_t value = 0;
                LibModbusClient::int16_to_int64(
                        (uint64_t &) value,
                        const_cast<uint16_t *>(input.data()) + i);

                // The int16_to_int64() function uses unsigned types, but this
                // case is for signed numbers, therefore we need to cast it
                // to a signed type before checking it is a correct value.
                check_correct_value(
                    static_cast<long double>(static_cast<int64_t>(value)),
                    index,
                    element_kind);

                float_vector.push_back(
                        (int64_t) data_offset() + value * data_factor());
            } else {
                uint64_t value = 0;
                LibModbusClient::int16_to_int64(
                        value,
                        const_cast<uint16_t *>(input.data()) + i);

                check_correct_value(
                    static_cast<long double>(static_cast<uint64_t>(value)),
                    index,
                    element_kind);

                float_vector.push_back(
                        (unsigned) (int64_t) data_offset()
                        + value * data_factor());
            }
            break;
        }
        case ModbusDataType::holding_register_float_abcd:
        case ModbusDataType::input_register_float_abcd: {
            float value = 0;
            LibModbusClient::int16_to_float_abcd(
                    value,
                    const_cast<uint16_t *>(input.data()) + i);

            check_correct_value(
                    static_cast<long double>(value),
                    index,
                    element_kind);

            float_vector.push_back(data_offset() + value * data_factor());

            break;
        }
        case ModbusDataType::holding_register_float_badc:
        case ModbusDataType::input_register_float_badc: {
            float value = 0;
            LibModbusClient::int16_to_float_badc(
                    value,
                    const_cast<uint16_t *>(input.data()) + i);

            check_correct_value(
                    static_cast<long double>(value),
                    index,
                    element_kind);

            float_vector.push_back(data_offset() + value * data_factor());

            break;
        }
        case ModbusDataType::holding_register_float_cdab:
        case ModbusDataType::input_register_float_cdab: {
            float value = 0;
            LibModbusClient::int16_to_float_cdab(
                    value,
                    const_cast<uint16_t *>(input.data()) + i);

            check_correct_value(
                    static_cast<long double>(value),
                    index,
                    element_kind);

            float_vector.push_back(data_offset() + value * data_factor());

            break;
        }
        case ModbusDataType::holding_register_float_dcba:
        case ModbusDataType::input_register_float_dcba: {
            float value = 0;
            LibModbusClient::int16_to_float_dcba(
                    value,
                    const_cast<uint16_t *>(input.data()) + i);

            check_correct_value(
                    static_cast<long double>(value),
                    index,
                    element_kind);

            float_vector.push_back(data_offset() + value * data_factor());

            break;
        }
        default:
            std::string error(
                    "Error: invalid datatype <"
                    + modbus_datatype_to_string(modbus_datatype())
                    + "> in the field <" + field() + ">.");
            throw std::runtime_error(error);
        }
    }
    return float_vector;
}

size_t ModbusAdapterConfigurationElement::number_of_registers_primitive_type()
{
    size_t number_of_registers_type = 0;

    switch (modbus_datatype_) {
    // The number_of_registers_type is divided by 2 because modbus uses
    // 16bits registers
    case ModbusDataType::coil_boolean:
    case ModbusDataType::discrete_input_boolean:
    case ModbusDataType::holding_register_int8:
    case ModbusDataType::input_register_int8:
    case ModbusDataType::holding_register_int16:
    case ModbusDataType::input_register_int16:
        number_of_registers_type = sizeof(uint16_t) / 2;
        break;
    case ModbusDataType::holding_register_int32:
    case ModbusDataType::input_register_int32:
        number_of_registers_type = sizeof(uint32_t) / 2;
        break;
    case ModbusDataType::holding_register_int64:
    case ModbusDataType::input_register_int64:
        number_of_registers_type = sizeof(uint64_t) / 2;
        break;
    case ModbusDataType::holding_register_float_abcd:
    case ModbusDataType::input_register_float_abcd:
    case ModbusDataType::holding_register_float_badc:
    case ModbusDataType::input_register_float_badc:
    case ModbusDataType::holding_register_float_cdab:
    case ModbusDataType::input_register_float_cdab:
    case ModbusDataType::holding_register_float_dcba:
    case ModbusDataType::input_register_float_dcba:
        number_of_registers_type = sizeof(float) / 2;
        break;
    default:
        std::string error = "Error: Unsupported datatype "
                + modbus_datatype_to_string(modbus_datatype());
        throw std::runtime_error(error);
    }
    return number_of_registers_type;
}

size_t ModbusAdapterConfigurationElement::calculate_array_elements()
{
    size_t number_of_registers_type = number_of_registers_primitive_type();

    // if the modbus_register_count_ divided by the number of registers that
    // each type occupated is not exaclty 1, this is an array or an invalid
    // input
    if (modbus_register_count() % number_of_registers_type != 0
        || modbus_register_count() / number_of_registers_type < 1) {
        std::string error =
                "Error: invalid modbus_register_count with the "
                "field <" + field() + "> of the datatype <"
                + modbus_datatype_to_string(modbus_datatype()) + ">.";
        throw std::runtime_error(error);
    }

    return modbus_register_count() / number_of_registers_type;
}

void ModbusAdapterConfigurationElement::set_array_elements_from_config()
{
    array_elements_ = calculate_array_elements();

    if (array_elements() == 1) {
        // arrays of 1 elements are not supported, this defines a primitive
        // datatype
        array_elements_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
////////////////////// ModbusAdapterConfiguration class ///////////////////////
///////////////////////////////////////////////////////////////////////////////
void ModbusAdapterConfiguration::parse_json_config_file(
        std::string& json_file_name)
{
    std::ifstream json_file(json_file_name);

    if (json_file.good() == false) {
        std::string error("JSON file <" + json_file_name + "> doesn't exist.");
        throw std::runtime_error(error);
    }

    std::string json_content(
            (std::istreambuf_iterator<char>(json_file)),
            std::istreambuf_iterator<char>());

    parse_json_config_string(json_content);
}

void ModbusAdapterConfiguration::parse_json_config_string(
        const std::string& json_content)
{
    json_document json_doc;
    json_doc.parse(json_content.c_str(), json_content.length());
    json_value *node = json_doc.first_node();

    if (node->type != json_array) {
        throw std::runtime_error(
                "Error in the JSON configuration, it should contain an array.");
    }

    // loop through all the elements in the top-level JSON array
    size_t length = node->u.array.length;
    for (size_t i = 0; i < length; ++i) {
        json_value *node_object = node->u.array.values[i];
        if (node_object->type != json_object) {
            throw std::runtime_error(
                    "Error in the JSON configuration, there "
                    "should be objects inside the array.");
        }

        // create a ModbusAdapterConfigurationElement and fill it out with
        // the information in the JSON
        ModbusAdapterConfigurationElement mace;
        size_t object_length = node_object->u.object.length;
        for (size_t j = 0; j < object_length; j++) {
            std::string element_name(node_object->u.object.values[j].name);

            // all the elements that may appear
            if (element_name == "field") {
                json_value *string_node = node_object->u.object.values[j].value;
                if (string_node->type != json_string) {
                    throw std::runtime_error(
                            "Error in the JSON configuration <field>.");
                }

                mace.field_ = string_node->u.string.ptr;
            } else if (element_name == "modbus_register_address") {
                json_value *number_node = node_object->u.object.values[j].value;
                if (number_node->type != json_integer) {
                    throw std::runtime_error(
                            "Error in the JSON configuration "
                            "<modbus_register_address>.");
                }

                mace.modbus_register_address_ =
                        static_cast<int>(number_node->u.integer);
            } else if (element_name == "modbus_register_count") {
                json_value *number_node = node_object->u.object.values[j].value;
                if (number_node->type != json_integer) {
                    throw std::runtime_error(
                            "Error in the JSON configuration "
                            "<modbus_register_count>.");
                }

                mace.modbus_register_count_ = static_cast<int>(number_node->u.integer);
            } else if (element_name == "modbus_slave_device_id") {
                json_value *number_node = node_object->u.object.values[j].value;
                if (number_node->type != json_integer) {
                    throw std::runtime_error(
                            "Error in the JSON configuration "
                            "<modbus_slave_device_id>.");
                }

                mace.modbus_slave_device_id_ = static_cast<uint8_t>(number_node->u.integer);
            } else if (element_name == "modbus_datatype") {
                json_value *string_node = node_object->u.object.values[j].value;
                if (string_node->type != json_string) {
                    throw std::runtime_error(
                            "Error in the JSON configuration "
                            "<modbus_datatype>.");
                }

                // translates the string of the type to an enum element
                std::string type_string = string_node->u.string.ptr;
                if (type_string == "HOLDING_REGISTER_INT16") {
                    mace.modbus_datatype_
                            = ModbusDataType::holding_register_int16;
                } else if (type_string == "HOLDING_REGISTER_INT8") {
                    mace.modbus_datatype_ =
                            ModbusDataType::holding_register_int8;
                } else if (type_string == "COIL_BOOLEAN") {
                    mace.modbus_datatype_ =
                            ModbusDataType::coil_boolean;
                } else if (type_string == "HOLDING_REGISTER_INT32") {
                    mace.modbus_datatype_ =
                            ModbusDataType::holding_register_int32;
                } else if (type_string == "HOLDING_REGISTER_INT64") {
                    mace.modbus_datatype_ =
                            ModbusDataType::holding_register_int64;
                } else if (type_string == "HOLDING_REGISTER_FLOAT_ABCD") {
                    mace.modbus_datatype_ =
                            ModbusDataType::holding_register_float_abcd;
                } else if (type_string == "HOLDING_REGISTER_FLOAT_BADC") {
                    mace.modbus_datatype_ =
                            ModbusDataType::holding_register_float_badc;
                } else if (type_string == "HOLDING_REGISTER_FLOAT_CDAB") {
                    mace.modbus_datatype_ =
                            ModbusDataType::holding_register_float_cdab;
                } else if (type_string == "HOLDING_REGISTER_FLOAT_DCBA") {
                    mace.modbus_datatype_ =
                            ModbusDataType::holding_register_float_dcba;
                } else if (type_string == "INPUT_REGISTER_INT16") {
                    mace.modbus_datatype_ =
                            ModbusDataType::input_register_int16;
                } else if (type_string == "INPUT_REGISTER_INT8") {
                    mace.modbus_datatype_ =
                            ModbusDataType::input_register_int8;
                } else if (type_string == "DISCRETE_INPUT_BOOLEAN") {
                    mace.modbus_datatype_ =
                            ModbusDataType::discrete_input_boolean;
                } else if (type_string == "INPUT_REGISTER_INT32") {
                    mace.modbus_datatype_ =
                            ModbusDataType::input_register_int32;
                } else if (type_string == "INPUT_REGISTER_INT64") {
                    mace.modbus_datatype_ =
                            ModbusDataType::input_register_int64;
                } else if (type_string == "INPUT_REGISTER_FLOAT_ABCD") {
                    mace.modbus_datatype_ =
                            ModbusDataType::input_register_float_abcd;
                } else if (type_string == "INPUT_REGISTER_FLOAT_BADC") {
                    mace.modbus_datatype_ =
                            ModbusDataType::input_register_float_badc;
                } else if (type_string == "INPUT_REGISTER_FLOAT_CDAB") {
                    mace.modbus_datatype_ =
                            ModbusDataType::input_register_float_cdab;
                } else if (type_string == "INPUT_REGISTER_FLOAT_DCBA") {
                    mace.modbus_datatype_ =
                            ModbusDataType::input_register_float_dcba;
                } else {
                    std::string error =
                            "Error: Unknown value of modbus_datatype <"
                            + type_string + ">";
                    throw std::runtime_error(error);
                }
            } else if (element_name == "modbus_min_value") {
                // there are several values in this configuration that might
                // contain an integer or a double, get the correct value
                // independently what it stores:
                // - modbus_min_value
                // - modbus_max_value
                // - modbus_valid_values
                // - output_data_factor
                // - output_data_offset
                // - input_data_factor
                // - input_data_offset

                json_value *number_node = node_object->u.object.values[j].value;

                if (number_node->type == json_integer) {
                    mace.modbus_min_value_ =
                            (long double) number_node->u.integer;
                } else if (number_node->type == json_double) {
                    mace.modbus_min_value_ = (long double) number_node->u.dbl;
                } else {
                    throw std::runtime_error(
                            "Error in the JSON configuration "
                            "<modbus_min_value>.");
                }
            } else if (element_name == "modbus_max_value") {
                json_value *number_node = node_object->u.object.values[j].value;

                if (number_node->type == json_integer) {
                    mace.modbus_max_value_ =
                            (long double) number_node->u.integer;
                } else if (number_node->type == json_double) {
                    mace.modbus_max_value_ = (long double) number_node->u.dbl;
                } else {
                    throw std::runtime_error(
                            "Error in the JSON configuration "
                            "<modbus_max_value>.");
                }
            } else if (element_name == "modbus_valid_values") {
                json_value *array_node = node_object->u.object.values[j].value;

                if (array_node->type != json_array) {
                    throw std::runtime_error(
                            "Error in the JSON configuration "
                            "<modbus_valid_values>.");
                }
                size_t array_length = array_node->u.array.length;
                for (unsigned int k = 0; k < array_length; ++k) {
                    json_value *node_number = array_node->u.array.values[k];

                    if (node_number->type == json_double) {
                        mace.modbus_valid_values_.push_back(
                                (long double) node_number->u.dbl);
                    } else if (node_number->type == json_integer) {
                        mace.modbus_valid_values_.push_back(
                                (long double) node_number->u.integer);
                    } else {
                        throw std::runtime_error(
                                "Error in the JSON configuration "
                                "<modbus_valid_values>");
                    }
                }
            } else if (element_name == "output_data_factor") {
                if (kind() == RoutingServiceEntityType::stream_reader) {
                    std::string error(
                            "Error in the JSON configuration. Unsupported "
                            "tag <output_data_factor> of the element <"
                            + element_name + "> in a StreamReader.");
                    throw std::runtime_error(error);
                } else if (kind() == RoutingServiceEntityType::stream_writer) {
                    json_value *number_node =
                            node_object->u.object.values[j].value;

                    if (number_node->type == json_integer) {
                        mace.data_factor_ = (float) number_node->u.integer;
                    } else if (number_node->type == json_double) {
                        mace.data_factor_ = (float) number_node->u.dbl;
                    } else {
                        throw std::runtime_error(
                                "Error in the JSON configuration "
                                "value of <output_data_factor>.");
                    }
                }
            } else if (element_name == "output_data_offset") {
                if (kind() == RoutingServiceEntityType::stream_reader) {
                    std::string error(
                            "Error in the JSON configuration. Unsupported "
                            "tag <output_data_offset> of the element <"
                            + element_name + "> in a StreamReader.");
                    throw std::runtime_error(error);
                } else if (kind() == RoutingServiceEntityType::stream_writer) {
                    json_value *number_node =
                            node_object->u.object.values[j].value;

                    if (number_node->type == json_integer) {
                        mace.data_offset_ = (float) number_node->u.integer;
                    } else if (number_node->type == json_double) {
                        mace.data_offset_ = (float) number_node->u.dbl;
                    } else {
                        throw std::runtime_error(
                                "Error in the JSON configuration "
                                "value of <output_data_offset>.");
                    }
                }
            } else if (element_name == "input_data_factor") {
                if (kind() == RoutingServiceEntityType::stream_writer) {
                    std::string error(
                            "Error in the JSON configuration. Unsupported "
                            "tag <input_data_factor> of the element <"
                            + element_name + "> in a StreamWriter.");
                    throw std::runtime_error(error);
                } else if (kind() == RoutingServiceEntityType::stream_reader) {
                    json_value *number_node =
                            node_object->u.object.values[j].value;

                    if (number_node->type == json_integer) {
                        mace.data_factor_ = (float) number_node->u.integer;
                    } else if (number_node->type == json_double) {
                        mace.data_factor_ = (float) number_node->u.dbl;
                    } else {
                        throw std::runtime_error(
                                "Error in the JSON configuration "
                                "value of <input_data_factor>.");
                    }
                }
            } else if (element_name == "input_data_offset") {
                if (kind() == RoutingServiceEntityType::stream_writer) {
                    std::string error(
                            "Error in the JSON configuration. Unsupported "
                            "tag <input_data_offset> of the element <"
                            + element_name + "> in a StreamWriter.");
                    throw std::runtime_error(error);
                } else if (kind() == RoutingServiceEntityType::stream_reader) {
                    json_value *number_node =
                            node_object->u.object.values[j].value;

                    if (number_node->type == json_integer) {
                        mace.data_offset_ = (float) number_node->u.integer;
                    } else if (number_node->type == json_double) {
                        mace.data_offset_ = (float) number_node->u.dbl;
                    } else {
                        throw std::runtime_error(
                                "Error in the JSON configuration "
                                "value of <input_data_offset>.");
                    }
                }
            } else if (element_name == "value") {
                json_value *value_node = node_object->u.object.values[j].value;
                if (value_node->type == json_integer) {
                    mace.constant_kind_ = ConstantValueKind::integer_kind;
                    mace.value_numeric_ = static_cast<long double>(
                            value_node->u.integer);
                } else if (value_node->type == json_double) {
                    mace.constant_kind_ = ConstantValueKind::double_kind;
                    mace.value_numeric_ = static_cast<long double>(
                            value_node->u.dbl);
                } else if (value_node->type == json_boolean) {
                    mace.constant_kind_ = ConstantValueKind::boolean_kind;
                    mace.value_numeric_ = static_cast<long double>(
                            value_node->u.boolean);
                } else if (value_node->type == json_string) {
                    mace.constant_kind_ = ConstantValueKind::string_kind;
                    mace.value_string_ = value_node->u.string.ptr;
                } else {
                    throw std::runtime_error(
                            "Error in the JSON configuration "
                            "value of <value>. Unsupported value.");
                }
            } else {
                std::string error(
                        "Error in the JSON configuration. Unsupported "
                        "element <"
                        + element_name + ">.");
                throw std::runtime_error(error);
            }
        }

        // Set CONSTANT_VALUE datatype if there is a value
        if (mace.modbus_datatype() == ModbusDataType::no_modbus_datatype
            && mace.constant_kind() != ConstantValueKind::undefined_kind) {
            mace.modbus_datatype_ = ModbusDataType::constant_value;
        }

        // check for errors/consistency in the elements read
        mace.check_for_errors();

        // if register_count is set, this might be an array, if so the
        // array_elements will be set
        if (mace.modbus_register_count() != 0
            && mace.modbus_datatype() != ModbusDataType::constant_value) {
            mace.set_array_elements_from_config();
        }
        if (mace.array_elements() == 0
            && mace.modbus_datatype() != ModbusDataType::constant_value) {
            // primitive type, update the register count
            mace.modbus_register_count_ =
                    static_cast<int>(mace.number_of_registers_primitive_type());
        }

        config_.push_back(mace);
    }
}

void ModbusAdapterConfiguration::check_configuration_consistency(
        const StructType& dds_type)
{
    for (auto element : config()) {
        // a StreamWriter cannot hancdle 'value' tag in the configuration,
        // this means that it cannot handle constant strings values
        if (kind() == RoutingServiceEntityType::stream_writer) {
            if (element.modbus_datatype() == ModbusDataType::constant_value) {
                std::string error(
                        "Error: incompatible field <value> of element <"
                        + element.field()
                        + "> in a <output> tag/Stream Writer.");
                throw std::runtime_error(error);
            }
            // a StreamWriter cannot be configured with read-only elements
            if (element.modbus_datatype()
                        == ModbusDataType::discrete_input_boolean
                || element.modbus_datatype()
                        == ModbusDataType::input_register_int8
                || element.modbus_datatype()
                        == ModbusDataType::input_register_int16
                || element.modbus_datatype()
                        == ModbusDataType::input_register_int32
                || element.modbus_datatype()
                        == ModbusDataType::input_register_int64
                || element.modbus_datatype()
                        == ModbusDataType::input_register_float_abcd
                || element.modbus_datatype()
                        == ModbusDataType::input_register_float_badc
                || element.modbus_datatype()
                        == ModbusDataType::input_register_float_cdab
                || element.modbus_datatype()
                        == ModbusDataType::input_register_float_dcba) {
                std::string error(
                        "Error: incompatible modbus_datatype of element <"
                        + element.field()
                        + "> in a <output> tag/Stream Writer.");
                throw std::runtime_error(error);
            }
        }

        if (element.array_elements() != 0) {
            // array or sequence
            auto type_by_name =
                    dynamic_data::get_member_type(dds_type, element.field());

            bool is_array = type_by_name.kind() == TypeKind::ARRAY_TYPE;
            bool is_sequence = type_by_name.kind() == TypeKind::SEQUENCE_TYPE;
            if (!is_array && !is_sequence) {
                std::string error(
                        "Error: the element " + element.field()
                        + " is configured as an array/sequence but it is not.");
                throw std::runtime_error(error);
            }
            // if it is an array, the number of elements that will be
            // read/written from/to a modbus device have to match with the DDS
            // array size
            if (is_array) {
                const ArrayType &dynamic_array =
                        static_cast<const ArrayType &>(type_by_name);

                // set the minimum to 0 if the datatype is an unsigned number.
                // int8_t and uint8_t are mapped to octets. They both can store
                // "negative numbers".
                if (element.modbus_datatype() != ModbusDataType::coil_boolean
                    && element.modbus_datatype()
                            != ModbusDataType::constant_value
                    && element.modbus_datatype()
                            != ModbusDataType::discrete_input_boolean
                    && element.modbus_datatype()
                            != ModbusDataType::holding_register_int8
                    && element.modbus_datatype()
                            != ModbusDataType::input_register_int8
                    && !dynamic_data::is_signed_kind(
                            dynamic_array.content_type().kind())) {
                    element.modbus_min_value_ = 0;
                }

                if (dynamic_array.dimension_count() != 1) {
                    std::string error(
                            "Error: multidimensional arrays not supported: "
                            + element.field_);
                    throw std::runtime_error(error);
                }
                if (dynamic_array.total_element_count()
                    != element.array_elements()) {
                    std::string error(
                            "Error: array size of <" + element.field()
                            + "> is incompatible with modbus_registers_count");
                    throw std::runtime_error(error);
                }
                if (!element.is_compatible_dds_datatype(
                            dynamic_array.content_type().kind())) {
                    std::string error(
                            "Error: array element type of <" + element.field()
                            + "> is incompatible with modbus_datatype");
                    throw std::runtime_error(error);
                }
            } else {
                // sequence
                // the number of element read/written from/to a modbus device
                // have to be lower or equal to the sequence bounds
                const SequenceType &dynamic_seq =
                        static_cast<const SequenceType &>(type_by_name);
                if (dynamic_seq.bounds() < element.array_elements()) {
                    std::string error(
                            "Error: sequence bounds of <" + element.field()
                            + "> is incompatible with modbus_registers_count");
                    throw std::runtime_error(error);
                }

                // set the minimum to 0 if the datatype is an unsigned number.
                // int8_t and uint8_t are mapped to octets. They both can store
                // "negative numbers".
                if (element.modbus_datatype() != ModbusDataType::coil_boolean
                        && element.modbus_datatype()
                                != ModbusDataType::constant_value
                        && element.modbus_datatype()
                                != ModbusDataType::discrete_input_boolean
                        && element.modbus_datatype()
                                != ModbusDataType::holding_register_int8
                        && element.modbus_datatype()
                                != ModbusDataType::input_register_int8
                        && !dynamic_data::is_signed_kind(
                                dynamic_seq.content_type().kind())) {
                    element.modbus_min_value_ = 0;
                }

                if (!element.is_compatible_dds_datatype(
                            dynamic_seq.content_type().kind())) {
                    std::string error(
                            "Error: sequence element type of <"
                            + element.field()
                            + "> is incompatible with modbus_datatype");
                    throw std::runtime_error(error);
                }
            }
        } else {
            // primitive type or enum
            auto field_type =
                    dynamic_data::get_member_type(dds_type, element.field());
            auto field_kind = field_type.kind();
            bool is_compatible = false;

            // set the minimum to 0 if the datatype is an unsigned number.
            // int8_t and uint8_t are mapped to octets. They both can store
            // "negative numbers".
            if (element.modbus_datatype() != ModbusDataType::coil_boolean
                    && element.modbus_datatype()
                            != ModbusDataType::constant_value
                    && element.modbus_datatype()
                            != ModbusDataType::discrete_input_boolean
                    && element.modbus_datatype()
                            != ModbusDataType::holding_register_int8
                    && element.modbus_datatype()
                            != ModbusDataType::input_register_int8
                    && !dynamic_data::is_signed_kind(field_type.kind())) {
                element.modbus_min_value_ = 0;
            }

            // primitive type or enum
            // This checks that:
            // - If it is a STREAM_WRITER the datatype of the
            // DynamicData is the same that the one that Modbus receives
            // - If this is a STREAM_READER the datatype that Modbus reads goes
            // to a compatible DynamicData field. This means that the size of
            // the DynamicData is higher or a float. Eg, a
            // HOLDING_REGISTER_INT16 can be stored in a float, a double, a
            // int16_t, a int32_t, a int64_t, a uint16_t, a uint32_t, a
            // uint64_t, or an enum.
            // - All enums should contain a valid_values list.
            is_compatible = element.is_compatible_dds_datatype(field_kind);

            // At this point, enum should contain a valid_values list
            if (field_kind == TypeKind::ENUMERATION_TYPE) {
                const EnumType &dynamic_enum =
                        static_cast<const EnumType &>(field_type);
                for (auto member : element.modbus_valid_values()) {
                    if (dynamic_enum.find_member_by_ordinal((int32_t) member)
                            == EnumType::INVALID_INDEX) {
                        std::string error = "Error: the value of the "
                            "modbus_valid_values <"
                            + std::to_string(static_cast<int32_t>(member))
                            + "> is not a value of the DDS enum <"
                            + element.field() + ">.";
                        throw std::runtime_error(error);
                    }
                }
            }

            // Throw an error if the datatype is not compatible
            if (!is_compatible) {
                std::string error = "Error: Incompatible datatype <"
                        + modbus_datatype_to_string(element.modbus_datatype())
                        + "> with field <" + element.field() + ">";
                throw std::runtime_error(error);
            }
        }
    }
}
