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

#include <memory>

#include <rti/routing/adapter/AdapterPlugin.hpp>
#include <rti/routing/adapter/Connection.hpp>

#include "ModbusClient.hpp"
#include "LibModbusClient.hpp"
#include "ModbusStreamWriter.hpp"

namespace rti { namespace adapter { namespace modbus {
using namespace rti::routing;
using namespace rti::routing::adapter;

using namespace dds::core::xtypes;
using namespace dds::domain;

class ModbusConnection : public Connection {
public:
    ModbusConnection(const PropertySet& properties);

    ~ModbusConnection();

    StreamReader* create_stream_reader(
            Session* session,
            const StreamInfo& info,
            const PropertySet& properties,
            StreamReaderListener* listener);

    void delete_stream_reader(StreamReader *reader);

    StreamWriter* create_stream_writer(
            Session* session,
            const StreamInfo& info,
            const PropertySet& properties);

    void delete_stream_writer(StreamWriter* writer);

private:
    std::string ip_address_ = "";
    uint16_t port_number_ = 0;
    LibModbusClient *mbw_;
};

}}}  // namespace rti::adapter::modbus
