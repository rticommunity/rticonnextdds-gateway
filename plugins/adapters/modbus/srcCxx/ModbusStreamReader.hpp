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

#include <mutex>
#include <thread>

#include <rti/routing/adapter/AdapterPlugin.hpp>
#include <rti/routing/adapter/StreamReader.hpp>

#include "ModbusAdapterConfiguration.hpp"
#include "LibModbusClient.hpp"

using namespace dds::core;
using namespace dds::domain;

namespace rti { namespace adapter { namespace modbus {

using namespace rti::routing;
using namespace rti::routing::adapter;

/**
 * @class ModbusStreamReader
 *
 * @brief implementation of the DynamicDataStreamReader.
 *
 * This class implements the functions in DynamicDataStreamReader
 */

class ModbusStreamReader : public DynamicDataStreamReader {
public:
    /**
     * @brief Parametrized constructor
     * @details constructor that creates a StreamReader with the specified
     * modbus connection. It reads the properties to load the JSON configuration
     * as well as the polling period.
     */
    ModbusStreamReader(
            const PropertySet& properties,
            const rti::routing::StreamInfo& info,
            rti::routing::adapter::StreamReaderListener* listener,
            LibModbusClient& connection);

    ~ModbusStreamReader();

    /**
     * @brief Copy the content of the cached_data_ into the DynamicData
     * samples[1]
     */
    void read(
            std::vector<dds::core::xtypes::DynamicData *>& samples,
            std::vector<dds::sub::SampleInfo *>& infos) final;

    /**
     * @brief Copy the content of the cached_data_ into the DynamicData
     * samples[1]
     *
     * @see read
     */
    void take(
            std::vector<dds::core::xtypes::DynamicData *>& samples,
            std::vector<dds::sub::SampleInfo *>& infos) final;

    /**
     * @brief Delete the DynamicData and SampleInfo passed as parameters
     */
    void return_loan(
            std::vector<dds::core::xtypes::DynamicData *>& samples,
            std::vector<dds::sub::SampleInfo *>& infos) final;

private:
    /**
     * @brief Creates a new thread that puts the data from a modbus device
     * into the StreamReaderListener.
     */
    void modbus_reading_thread();

    /**
     * @brief Creates a new thread that reads data from a modbus device
     */
    void on_data_available_thread();

    /**
     * @brief Calls LibModbusClient functions to read data from the modbus
     * device specified in the connection. This function will read modbus
     * registers depending on the ModbusAdapterConfiguration and store these
     * values in cached_data_
     */
    void read_data_from_modbus();

private:
    ModbusAdapterConfiguration config_;
    const StreamInfo& info_;
    LibModbusClient& connection_;
    StreamReaderListener *reader_listener_;
    dds::core::xtypes::DynamicType *adapter_type_;
    dds::core::xtypes::DynamicData *cached_data_;
    std::thread modbus_thread_;
    std::thread on_data_available_thread_;
    std::mutex cached_data_mutex_;

    int timeout_msecs_ = -1;
    bool stop_thread_ = false;
};

}}}  // namespace rti::adapter::modbus
