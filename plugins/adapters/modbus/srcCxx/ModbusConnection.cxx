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

#include <iostream>

#include "LibModbusClient.hpp"
#include "ModbusConnection.hpp"
#include "ModbusStreamReader.hpp"
#include "ModbusStreamWriter.hpp"

using namespace rti::adapter::modbus;

ModbusConnection::ModbusConnection(const PropertySet& properties)
{
    // Get the properties values that identifies the IP and Port
    if (properties.find("modbus_server_ip") != properties.end()) {
        ip_address_ = properties.at("modbus_server_ip");
    }

    if (properties.find("modbus_server_port") != properties.end()) {
        port_number_ = std::stoi(properties.at("modbus_server_port"));
    }

    mbw_ = new LibModbusClient(ip_address_, port_number_);

    if (properties.find("modbus_response_timeout_msec") != properties.end()) {
        uint32_t msec = std::stoi(properties.at("modbus_response_timeout_msec"));
        uint32_t sec = static_cast<uint32_t>(msec / 1000);
        uint32_t usec = (msec - sec * 1000) * 1000;

        mbw_->set_response_timeout(sec, usec);
    }
}

ModbusConnection::~ModbusConnection()
{
    delete mbw_;
}

StreamReader* ModbusConnection::create_stream_reader(
        Session* session,
        const StreamInfo& info,
        const PropertySet& properties,
        StreamReaderListener* listener)
{
    return new ModbusStreamReader(properties, info, listener, *mbw_);
};

void ModbusConnection::delete_stream_reader(StreamReader* reader)
{
    delete reader;
}

StreamWriter* ModbusConnection::create_stream_writer(
        Session* session,
        const StreamInfo& info,
        const PropertySet& properties)
{
    return new ModbusStreamWriter(properties, info, *mbw_);
};

void ModbusConnection::delete_stream_writer(StreamWriter* writer)
{
    delete writer;
}
