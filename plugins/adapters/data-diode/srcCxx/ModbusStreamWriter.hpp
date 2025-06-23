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

#include <rti/routing/adapter/AdapterPlugin.hpp>
#include <rti/routing/adapter/StreamWriter.hpp>

#include "ModbusAdapterConfiguration.hpp"
#include "LibModbusClient.hpp"

using namespace dds::core;
using namespace dds::domain;

namespace rti { namespace adapter { namespace modbus {

using namespace rti::routing;
using namespace rti::routing::adapter;

/**
 * @class ModbusStreamWriter
 *
 * @brief implementation of the DynamicDataStreamWriter.
 *
 * This class implements the functions in DynamicDataStreamWriter
 */

class ModbusStreamWriter : public DynamicDataStreamWriter {
public:
    /**
     * @brief Parametrized constructor
     * @details constructor that creates a StreamWriter with the specified
     * modbus connection. It reads the properties to load the JSON configuration
     */
    ModbusStreamWriter(
            const PropertySet& properties,
            const StreamInfo& stream_info,
            LibModbusClient& connection);

    /**
     * @brief Calls LibModbusClient functions to write data to the modbus
     * device specified in the connection. This function will write all the
     * samples to modbus registers depending on the ModbusAdapterConfiguration.
     */
    int write(
            const std::vector<dds::core::xtypes::DynamicData *>& samples,
            const std::vector<dds::sub::SampleInfo *>& infos);

private:
    ModbusAdapterConfiguration config_;
    const StreamInfo& info_;
    LibModbusClient& connection_;
};

}}}  // namespace rti::adapter::modbus
