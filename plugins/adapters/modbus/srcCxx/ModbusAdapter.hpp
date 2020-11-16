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

#include <rti/routing/RoutingService.hpp>
#include <rti/routing/adapter/AdapterPlugin.hpp>

#include <dds/core/macros.hpp>

using namespace rti::routing;
using namespace rti::routing::adapter;

namespace rti { namespace adapter { namespace modbus {
class ModbusAdapter : public rti::routing::adapter::AdapterPlugin {

public:
    explicit ModbusAdapter(rti::routing::PropertySet& property);

    Connection* create_connection(
            rti::routing::adapter::detail::StreamReaderListener *,
            rti::routing::adapter::detail::StreamReaderListener *,
            const PropertySet& properties);

    void delete_connection(Connection* connection);

    rti::config::LibraryVersion get_version() const;
};

/**
 * This macro defines a C-linkage symbol that can be used as create function
 * for plug-in registration through XML.
 *
 * The generated symbol has the name:
 *
 * \code
 * ModbusAdapter_create_adapter_plugin
 * \endcode
 */
RTI_ADAPTER_PLUGIN_CREATE_FUNCTION_DECL(ModbusAdapter)


}}}  // namespace rti::adapter::modbus
