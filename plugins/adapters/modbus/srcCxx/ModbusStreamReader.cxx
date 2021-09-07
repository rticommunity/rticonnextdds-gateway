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
#include <fstream>
#include <iostream>
#include <chrono>
#include <string>

#include <rti/routing/TypeInfo.hpp>

#include <dds/dds.hpp>

#include "DynamicDataHelpers.hpp"
#include "ModbusAdapterConfiguration.hpp"
#include "ModbusStreamReader.hpp"


using namespace dds::core::xtypes;
using namespace dds::domain;

using namespace rti::common;
using namespace rti::adapter::modbus;

void ModbusStreamReader::on_data_available_thread()
{
    while (!stop_thread_) {
        read_data_from_modbus();
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_msecs_));
    }
}

void ModbusStreamReader::modbus_reading_thread()
{
    while (!stop_thread_) {
        reader_listener_->on_data_available(this);
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_msecs_));
    }
}

ModbusStreamReader::ModbusStreamReader(
        const PropertySet& properties,
        const rti::routing::StreamInfo& stream_info,
        rti::routing::adapter::StreamReaderListener* listener,
        LibModbusClient& connection)
        : info_(stream_info),
          connection_(connection),
          config_(RoutingServiceEntityType::stream_reader)
{
    reader_listener_ = listener;
    adapter_type_ = static_cast<DynamicType *>(
            stream_info.type_info().type_representation());
    cached_data_ = new DynamicData(*adapter_type_);

    if (adapter_type_->kind() != TypeKind::STRUCTURE_TYPE) {
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
    if (properties.find("polling_period_msec") != properties.end()) {
        timeout_msecs_ = std::stoi(properties.at("polling_period_msec"));
    }

    const StructType &dynamic_struct =
            static_cast<const StructType &>(*adapter_type_);

    config_.check_configuration_consistency(dynamic_struct);

    if (timeout_msecs_ >= 0) {
        modbus_thread_ =
                std::thread(&ModbusStreamReader::modbus_reading_thread, this);
        on_data_available_thread_ = std::thread(
                &ModbusStreamReader::on_data_available_thread,
                this);
    }
}

ModbusStreamReader::~ModbusStreamReader()
{
    std::lock_guard<std::mutex> guard(cached_data_mutex_);
    delete cached_data_;

    // To delete the threads we enable stop_thread and then join to them
    stop_thread_ = true;
    if (modbus_thread_.joinable()) {
        modbus_thread_.join();
    }
    if (on_data_available_thread_.joinable()) {
        on_data_available_thread_.join();
    }
}

void ModbusStreamReader::read_data_from_modbus()
{
    // This protection is required since take() executes on a different
    // Routing Service thread.

    std::lock_guard<std::mutex> guard(cached_data_mutex_);

    const StructType &struct_type =
            static_cast<const StructType &>(*adapter_type_);

    for (auto mace : config_.config()) {
        auto member_kind =
                dynamic_data::get_member_type(struct_type, mace.field())
                        .kind();
        TypeKind element_kind = member_kind;
        std::vector<long double> float_vector;

        // Sets the slave ID before reading, only when the datatype is not
        // constant and the slave ID is different from the previous one.
        try {
            bool is_constant = mace.modbus_datatype()
                    == ModbusDataType::constant_value;
            bool is_same_slave_id = connection_.get_slave_id()
                    == mace.modbus_slave_device_id();
            if (!is_constant && !is_same_slave_id) {
                connection_.set_slave_id(mace.modbus_slave_device_id());
            }
        } catch (const std::exception &ex) {
            std::cerr << ex.what() << std::endl;
            continue;
        }

        // if this is an array or a sequence we should extract the kind of the
        // internal elements
        if (member_kind == TypeKind::ARRAY_TYPE) {
            DynamicType dynamic_type =
                    dynamic_data::get_member_type(struct_type, mace.field());
            const ArrayType &array_type =
                    static_cast<const ArrayType &>(dynamic_type);
            element_kind = array_type.content_type().kind();
        }
        if (member_kind == TypeKind::SEQUENCE_TYPE) {
            DynamicType dynamic_type =
                    dynamic_data::get_member_type(struct_type, mace.field());
            const SequenceType &sequence_type =
                    static_cast<const SequenceType &>(dynamic_type);
            element_kind = sequence_type.content_type().kind();
        }

        if (mace.modbus_datatype() == ModbusDataType::constant_value) {
            if(element_kind == TypeKind::STRING_TYPE) {
                if (mace.constant_kind() != ConstantValueKind::string_kind) {
                    std::string error(
                            "Error: the field <" + mace.field() + "> is a "
                            "string but the constant value is not a string.");
                    throw std::runtime_error(error);
                }
                const StringType &string_type = static_cast<const StringType &>(
                    struct_type.member(mace.field()).type());
                // If the type is a string, check that the content fits into
                // the DDS String
                if (string_type.bounds() >= mace.value_string().length()) {
                    cached_data_->value<std::string>(
                            mace.field(),
                            mace.value_string());
                } else {
                    std::string error(
                            "Error: the string constant value <"
                            + mace.value_string()
                            + "> doesn't fit in the bounds of the field <"
                            + mace.field() + ">.");
                    throw std::runtime_error(error);
                }
            } else {
                if (mace.constant_kind() != ConstantValueKind::double_kind
                        && mace.constant_kind() != ConstantValueKind::boolean_kind
                        && mace.constant_kind() != ConstantValueKind::integer_kind) {
                dynamic_data::set_dds_primitive_or_enum_type_value(
                        *cached_data_,
                        element_kind,
                        mace.field(),
                        mace.value_numeric());
            }
            continue;
        } else if (mace.modbus_datatype() == ModbusDataType::coil_boolean
                || mace.modbus_datatype()
                        == ModbusDataType::discrete_input_boolean) {
            // read coils and store them in a uint8_t array
            std::vector<uint8_t> values(mace.modbus_register_count());
            bool read_discrete_input =
                    mace.modbus_datatype()
                            == ModbusDataType::discrete_input_boolean;

            auto size = -1;
            try {
                size = connection_.read_coils(
                    values,
                    mace.modbus_register_address(),
                    mace.modbus_register_count(),
                    read_discrete_input);
            } catch (const std::exception &ex) {
                std::cerr << ex.what() << std::endl;
                // if the value is not correct, we don't store it in
                // the cached_data_
                continue;
            }
            if (size == 0 && struct_type.member(mace.field()).is_optional()) {
                // unset the field as it is optional and couldn't be read
                cached_data_->clear_optional_member(mace.field());
            }
            if (size > 0) {
                // set the read value into float_vector
                for (int i = 0; i < mace.modbus_register_count(); ++i) {
                    float_vector.push_back(values[i]);
                }
            }

            // if no value has been read and the field is not optional, do
            // nothing and keep the previous value in the dynamic data
        } else {
            // read registers of any type
            std::vector<uint16_t> registers(mace.modbus_register_count());
            bool read_input_registers =
                    mace.modbus_datatype()
                            == ModbusDataType::input_register_int8
                    || mace.modbus_datatype()
                            == ModbusDataType::input_register_int16
                    || mace.modbus_datatype()
                            == ModbusDataType::input_register_int32
                    || mace.modbus_datatype()
                            == ModbusDataType::input_register_int64
                    || mace.modbus_datatype()
                            == ModbusDataType::input_register_float_abcd
                    || mace.modbus_datatype()
                            == ModbusDataType::input_register_float_badc
                    || mace.modbus_datatype()
                            == ModbusDataType::input_register_float_cdab
                    || mace.modbus_datatype()
                            == ModbusDataType::input_register_float_dcba;

            int size = -1;
            try {
                size = connection_.read_registers(
                        registers,
                        mace.modbus_register_address(),
                        mace.modbus_register_count(),
                        read_input_registers);
                if (size < 1
                        && struct_type.member(mace.field()).is_optional()) {
                    // unset the field as it is optional and couldn't be read
                    cached_data_->clear_optional_member(mace.field());
                }
                if (size > 0) {
                    // set the read value into float_vector
                    float_vector =
                            mace.get_float_value(registers, element_kind);
                }
            } catch (const std::exception &ex) {
                std::cerr << ex.what() << std::endl;
                // if the value is not correct, we don't store it in
                // the cached_data_
                continue;
            }
        }

        if (float_vector.size() == 1) {
            // an array with one element means that it's a simple value
            dynamic_data::set_dds_primitive_or_enum_type_value(
                    *cached_data_,
                    element_kind,
                    mace.field(),
                    float_vector[0]);
        } else if (float_vector.size() > 1) {
            dynamic_data::set_vector_values(
                    *cached_data_,
                    element_kind,
                    mace.field(),
                    float_vector);
            // when checking type_consistency() we ensure that the number of
            // elements won't be higher than mace.array_elements(). Therefore
            // float_vector.size() can be used safely.
        }
        // in case of the size == 0, do nothing because nothing has been read
    }
}

void ModbusStreamReader::read(
        std::vector<dds::core::xtypes::DynamicData *>& samples,
        std::vector<dds::sub::SampleInfo *>& infos)
{
    // If no reading thread has been created, the sample is filled out
    // asynchronously.
    if (timeout_msecs_ < 0) {
        read_data_from_modbus();
    }

    /**
     * This protection is required since take() executes on a different
     * Routing Service thread.
     */
    std::lock_guard<std::mutex> guard(cached_data_mutex_);

    /**
     * Note that we read one sample at a time from modbus in the
     * function modbus_reading_thread()
     */
    samples.resize(1);
    infos.resize(1);

    // copy cached data into the samples
    std::unique_ptr<DynamicData> sample(new DynamicData(*cached_data_));

    samples[0] = sample.release();

    return;
}

void ModbusStreamReader::take(
        std::vector<dds::core::xtypes::DynamicData *>& samples,
        std::vector<dds::sub::SampleInfo *>& infos)
{
    // there is no difference between take or read, however the take() runs
    // the return_loan to delete the samples.
    read(samples, infos);
}

void ModbusStreamReader::return_loan(
        std::vector<dds::core::xtypes::DynamicData *>& samples,
        std::vector<dds::sub::SampleInfo *>& infos)
{
    for (int i = 0; i < samples.size(); ++i) {
        delete samples[i];
        delete infos[i];
    }
    samples.clear();
    infos.clear();
}
