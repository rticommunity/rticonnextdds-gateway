/******************************************************************************/
/* (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#ifndef DIAADAPTER_HPP
#define DIAADAPTER_HPP

#include <dds/core/corefwd.hpp>
#include <dds/core/macros.hpp>
#include <rti/routing/PropertySet.hpp>
#include <rti/routing/RoutingService.hpp>
#include <rti/routing/adapter/AdapterPlugin.hpp>
#include "OdbcEnvironmentManager.hpp"

namespace rti { namespace adapter { namespace dis {

/**
 * @brief Adapter class and entry point into the Database Integration Service.
 * It acts as a factory for DiaConnection objects which are passed an
 * OdbcConnectionManager object upon creation by means of the
 * odbc_environment_manager_.
 */
class DiaAdapter : public rti::routing::adapter::AdapterPlugin {

public:
	explicit DiaAdapter(rti::routing::PropertySet &);

	rti::routing::adapter::Connection * create_connection(
			rti::routing::adapter::detail::StreamReaderListener *,
			rti::routing::adapter::detail::StreamReaderListener *,
			const rti::routing::PropertySet &) final;

	void delete_connection(rti::routing::adapter::Connection *connection) final;

	rti::config::LibraryVersion get_version() const;

private:
	odbc::OdbcEnvironmentManager odbc_environment_manager_;
};

// XML configuration hook
RTI_ADAPTER_PLUGIN_CREATE_FUNCTION_DECL(DiaAdapter)

} } } // namespace rti::adapter::dis

#endif
