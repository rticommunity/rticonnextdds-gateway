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

#include <algorithm>

#include <rti/routing/TypeInfo.hpp>
#include <rti/topic/PrintFormat.hpp>
#include <rti/topic/to_string.hpp>

#include <dds/dds.hpp>

#include "DynamicDataHelpers.hpp"
#include "ModbusStreamWriter.hpp"

using namespace dds::core::xtypes;
using namespace dds::domain;

using namespace rti::common;
using namespace rti::adapter::modbus;

ModbusStreamWriter::ModbusStreamWriter(
        const PropertySet& properties,
        const StreamInfo& stream_info,
        LibModbusClient& connection)
        : info_(stream_info),
          connection_(connection),
          config_(RoutingServiceEntityType::stream_writer)
{
    DynamicType *type =
            static_cast<DynamicType *>(info_.type_info().type_representation());

    if (type->kind() != TypeKind::STRUCTURE_TYPE) {
        std::string error("Error: the main element is not a struct.");
        throw std::runtime_error(error);
    }

    // if both configuration_***_json properties are set, both configurations
    // are merged
    if (properties.find("configuration_file_json") != properties.end()) {
        auto property_value = properties.at("configuration_file_json");
        config_.parse_json_config_file(property_value);
    }
    if (properties.find("configuration_string_json") != properties.end()) {
        auto property_value = properties.at("configuration_string_json");
        config_.parse_json_config_string(property_value);
    }

    const StructType &dynamic_struct = static_cast<const StructType &>(*type);

    config_.check_configuration_consistency(dynamic_struct);
}

int ModbusStreamWriter::write(
        const std::vector<dds::core::xtypes::DynamicData *>& samples,
        const std::vector<dds::sub::SampleInfo *>& infos)
{
    for (int i = 0; i < samples.size(); i++) {
        auto sample = samples[i];
        auto info = infos[i];

        if (info->valid()) {
            // mace -> ModbusAdapterConfigurationElement
            for (auto mace : config_.config()) {
                const StructType &dynamic_struct =
                        static_cast<const StructType &>(sample->type());
                auto member_kind = dynamic_data::get_member_type(
                                           dynamic_struct,
                                           mace.field()).kind();
                TypeKind element_kind = member_kind;
                std::vector<long double> float_vector;

                // If the type is optional and it is not set, do nothing
                if (!sample->member_exists(mace.field())) {
                    continue;
                }

                // get the values that will be written for this specific field
                if (is_primitive_type(
                            dynamic_struct.member(mace.field()).type())
                        || member_kind == TypeKind::ENUMERATION_TYPE) {
                    // an array with one element means that it's a simple value
                    float_vector.push_back(
                            dynamic_data::
                                    get_dds_primitive_or_enum_type_value(
                                            *sample,
                                            mace.field()));
                } else if (member_kind == TypeKind::ARRAY_TYPE
                        || member_kind == TypeKind::SEQUENCE_TYPE) {
                    element_kind =
                            sample->member_info(mace.field()).element_kind();
                    float_vector = dynamic_data::get_vector_values(
                            *sample,
                            mace.field());
                    // when checking type_consistency() we ensure that the
                    // number of elements won't be higher than
                    // mace.array_elements(). Therefore float_vector.size() can
                    // be used safely.
                }

                if (mace.modbus_datatype() == ModbusDataType::coil_boolean) {
                    std::vector<uint8_t> values(mace.modbus_register_count());

                    for (int i = 0; i < float_vector.size(); ++i) {
                        values[i] = static_cast<uint8_t>(float_vector[i]);
                    }
                    // write coils to a modbus server
                    try {
                        connection_.write_coils(
                                mace.modbus_register_address(),
                                mace.modbus_register_count(),
                                values);
                        // no need of checking write_coils error, because it
                        // is logged inside
                    } catch (const std::exception &ex) {
                        std::cerr << ex.what() << std::endl;
                        continue;
                    }
                } else {
                    std::vector<uint16_t>
                            registers(mace.modbus_register_count());
                    try {
                        mace.get_registers_value(
                                registers,
                                float_vector,
                                element_kind);
                        // write register/s to the modbus device
                        connection_.write_registers(
                                mace.modbus_register_address(),
                                mace.modbus_register_count(),
                                registers);
                    } catch (const std::exception &ex) {
                        std::cerr << ex.what() << std::endl;
                        continue;
                    }
                }
            }
        }
    }
    return 0;
}
