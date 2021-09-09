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

#include <cfloat>
#include <iostream>
#include <vector>

#include <dds/dds.hpp>

namespace rti { namespace adapter { namespace modbus {

// enum that identifies the kind of data read/write from/to a modbus device
enum class ModbusDataType {
    coil_boolean,
    holding_register_int8,
    holding_register_int16,
    holding_register_int32,
    holding_register_int64,
    holding_register_float_abcd,
    holding_register_float_badc,
    holding_register_float_cdab,
    holding_register_float_dcba,
    constant_value,
    discrete_input_boolean,
    input_register_int8,
    input_register_int16,
    input_register_int32,
    input_register_int64,
    input_register_float_abcd,
    input_register_float_badc,
    input_register_float_cdab,
    input_register_float_dcba,
    no_modbus_datatype
};

static std::string modbus_datatype_to_string(ModbusDataType type) {
    if (type == ModbusDataType::coil_boolean) {
        return "coil_boolean";
    } else if (type == ModbusDataType::holding_register_int8) {
        return "holding_register_int8";
    } else if (type == ModbusDataType::holding_register_int16) {
        return "holding_register_int16";
    } else if (type == ModbusDataType::holding_register_int32) {
        return "holding_register_int32";
    } else if (type == ModbusDataType::holding_register_int64) {
        return "holding_register_int64";
    } else if (type == ModbusDataType::holding_register_float_abcd) {
        return "holding_register_float_abcd";
    } else if (type == ModbusDataType::holding_register_float_badc) {
        return "holding_register_float_badc";
    } else if (type == ModbusDataType::holding_register_float_cdab) {
        return "holding_register_float_cdab";
    } else if (type == ModbusDataType::holding_register_float_dcba) {
        return "holding_register_float_dcba";
    } else if (type == ModbusDataType::constant_value) {
        return "constant_value";
    } else if (type == ModbusDataType::discrete_input_boolean) {
        return "discrete_input_boolean";
    } else if (type == ModbusDataType::input_register_int8) {
        return "input_register_int8";
    } else if (type == ModbusDataType::input_register_int16) {
        return "input_register_int16";
    } else if (type == ModbusDataType::input_register_int32) {
        return "input_register_int32";
    } else if (type == ModbusDataType::input_register_int64) {
        return "input_register_int64";
    } else if (type == ModbusDataType::input_register_float_abcd) {
        return "input_register_float_abcd";
    } else if (type == ModbusDataType::input_register_float_badc) {
        return "input_register_float_badc";
    } else if (type == ModbusDataType::input_register_float_cdab) {
        return "input_register_float_cdab";
    } else if (type == ModbusDataType::input_register_float_dcba) {
        return "input_register_float_dcba";
    } else if (type == ModbusDataType::no_modbus_datatype) {
        return "no_modbus_datatype";
    }
    // If the function reaches this point, it is considered as
    // no_modbus_datatype
    return "no_modbus_datatype";
}

// identifies the type of the entity of RS
enum class RoutingServiceEntityType { stream_writer, stream_reader };

// forward-declaration of the ModbusAdapterConfiguration class
class ModbusAdapterConfiguration;

/**
 * @class ModbusAdapterConfigurationElement
 *
 * @brief Stores and handle all the information related to a configuration
 * element (one element of the array in the JSON configuration).
 */
class ModbusAdapterConfigurationElement {
    // This class is fried of ModbusAdapterConfiguration, so
    // ModbusAdapterConfiguration will have access to
    // ModbusAdapterConfigurationElement private and protected elements
public:
    friend class ModbusAdapterConfiguration;

public:
    /**
     * @brief Default constructor
     */
    ModbusAdapterConfigurationElement();
    ~ModbusAdapterConfigurationElement()
    {
    }

    /**
     * @brief Check that the dds_datatype is compatible with the corresponding
     * element configuration
     * @param dds_datatype the output/input DDS TypeKind. Depending on this
     * datatype, different checkings should be performed to be compatible.
     * @return A boolean that says if the current configuration is compatible
     * with the DDS Datatype.
     */
    bool is_compatible_dds_datatype(dds::core::xtypes::TypeKind dds_datatype);

    /**
     * @brief Translates the value to a string
     * @param value the value to translate to string
     * @param element_kind the kind of the DDS datatype
     * @return A string that contains the numeric value of 'value'
     */
    std::string get_value_string(
            long double value,
            dds::core::xtypes::TypeKind element_kind);
    /**
     * @brief Check that the value read or that will be written to a modbus
     * device is correct and consistent with the configuration. Throws
     * an exception in case the value is not correct.
     * @param float_value the value to be read/written from/to a modbus device
     * @param index the index of the float_value in the array it belongs to.
     * This parameter is only used for debuggability.
     * @param element_kind the kind of the DDS datatype
     * @throw exception in case the value is not correct/consistent
     */
    void check_correct_value(
            long double float_value,
            size_t index,
            dds::core::xtypes::TypeKind element_kind);

    /**
     * @brief Translates the float_vector into an array of uint16_t (registers)
     * that is prepared to be written into a modbus device and applies the
     * linear transformation defined by data_factor and data_offset.
     * @param [out] output the uint16 array that may be written to a modbus
     * device
     * @param float_vector the array which contains the numbers to translate
     * @param element_kind the kind of the DDS datatype
     */
    void get_registers_value(
            std::vector<uint16_t>& output,
            const std::vector<long double>& float_vector,
            dds::core::xtypes::TypeKind element_kind);

    /**
     * @brief Translates an array of uint16_t (registers) into an array of
     * long_double and applies the linear transformation defined by data_factor
     * and data_offset.
     * @param input the uint16 array that has been read from a modbus device
     * @param element_kind the kind of the DDS datatype
     * @return The long double vector which contains the corresponding values
     * translated from the modbus registers (uint16_t)
     */
    std::vector<long double> get_float_value(
            const std::vector<uint16_t>& input,
            dds::core::xtypes::TypeKind element_kind);

    /**
     * @brief Check that the configuration provided is consistent or has any
     * kind of errors on it.
     * @throw exception in case the configuration is not correct/consistent.
     */
    void check_for_errors(void);

public:
    // public getters
    inline std::string const& field() const
    {
        return field_;
    }
    inline int const modbus_register_address() const
    {
        return modbus_register_address_;
    }
    inline int const modbus_register_count() const
    {
        return modbus_register_count_;
    }
    inline ModbusDataType const modbus_datatype() const
    {
        return modbus_datatype_;
    }
    inline uint8_t const modbus_slave_device_id() const
    {
        return modbus_slave_device_id_;
    }
    inline long double const modbus_min_value() const
    {
        return modbus_min_value_;
    }
    inline long double const modbus_max_value()
    {
        return modbus_max_value_;
    }
    inline std::vector<long double> const &modbus_valid_values() const
    {
        return modbus_valid_values_;
    }
    inline float const data_factor() const
    {
        return data_factor_;
    }
    inline float const data_offset() const
    {
        return data_offset_;
    }
    inline std::string const value() const
    {
        return value_;
    }
    inline size_t const array_elements() const
    {
        return array_elements_;
    }

private:
    // private functions
    /**
     * @brief Calculates the number of registers that a primitive type will use
     * @return The number of modbus registers that a specific datatype uses,
     * depeneding on the configuration provided.
     */

    size_t number_of_registers_primitive_type();
    /**
     * @brief Calculates the number of elements that an array will contain. This
     * is different of the register_count when the type cannot be stored in a
     * modbus register (uint16). For example an array of 2 int32 will have:
     * array_elements_ = 2;
     * modbus_register_Count = 4;
     * @return The number of array elements for that specific configuration.
     */
    size_t calculate_array_elements();

    /**
     * @brief Sets the array_elements, if we have an array of 1 element, that
     * means that we have a simple element. Arrays of 1 elements are not
     * supported
     * @return The number of array elements for that specific configuration.
     */
    void set_array_elements_from_config();

private:
    // fields
    std::string field_;
    int modbus_register_address_;
    int modbus_register_count_;
    ModbusDataType modbus_datatype_;
    uint8_t modbus_slave_device_id_;
    long double modbus_min_value_;
    long double modbus_max_value_;
    std::vector<long double> modbus_valid_values_;
    float data_factor_;
    float data_offset_;
    std::string value_;

    // Number of array elements, 0 means that this is not an array.
    // For example an array of 2 int32 will have:
    // array_elements_ = 2;
    // modbus_register_Count = 4;
    size_t array_elements_;
};

class ModbusAdapterConfiguration {
public:
    ModbusAdapterConfiguration(RoutingServiceEntityType kind)
    {
        kind_ = kind;
    }
    ~ModbusAdapterConfiguration()
    {
    }

    /**
     * @brief Parse a JSON configuration and store everything into a
     * ModbusAdapterConfigurationElement.
     * @param json_file_name the path to the *.json file where the configuration
     * is stored.
     */
    void parse_json_config_file(std::string& json_file_name);

    /**
     * @brief Parse a JSON configuration and store everything into a
     * ModbusAdapterConfigurationElement.
     * @param json_content the JSON string to be parsed
     * is stored.
     */
    void parse_json_config_string(const std::string& json_content);

    /**
     * @brief Check that the configuration created (for every element) is
     * consistent with the dds_type provided from RS.
     * @param dds_type the type that will be writting or reading data
     * to/from DDS.
     */
    void check_configuration_consistency(
            const dds::core::xtypes::StructType& dds_type);

    // public getters
    inline std::vector<ModbusAdapterConfigurationElement> const &config() const
    {
        return config_;
    }

private:
    // identifies a StreamWriter or StreamReader
    inline RoutingServiceEntityType const kind() const
    {
        return kind_;
    }
    std::vector<ModbusAdapterConfigurationElement> config_;
    RoutingServiceEntityType kind_;
};
}}}  // namespace rti::adapter::modbus
