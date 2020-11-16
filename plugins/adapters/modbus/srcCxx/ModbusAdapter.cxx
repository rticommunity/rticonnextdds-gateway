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

#include "ModbusAdapter.hpp"
#include "ModbusConnection.hpp"

using namespace rti::adapter::modbus;

ModbusAdapter::ModbusAdapter(rti::routing::PropertySet &properties)
{
}

Connection *ModbusAdapter::create_connection(
        rti::routing::adapter::detail::StreamReaderListener *,
        rti::routing::adapter::detail::StreamReaderListener *,
        const PropertySet& properties)
{
    return new ModbusConnection(properties);
};


void ModbusAdapter::delete_connection(Connection *connection)
{
    delete connection;
}

rti::config::LibraryVersion ModbusAdapter::get_version() const
{
    return rti::config::LibraryVersion(1, 0, 0, 'r');
};

extern "C" RTI_ADAPTER_PLUGIN_CREATE_FUNCTION_DEF(ModbusAdapter);
