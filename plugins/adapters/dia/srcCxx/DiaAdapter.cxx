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

#include "DiaAdapter.hpp"
#include "DiaConnection.hpp"

using namespace rti::adapter::dis;
using namespace rti::adapter::dis::odbc;
using namespace rti::routing;
using namespace rti::routing::adapter;

DiaAdapter::DiaAdapter(PropertySet& properties) :
	odbc_environment_manager_(OdbcEnvironmentManager())
{}

Connection * DiaAdapter::create_connection(
		rti::routing::adapter::detail::StreamReaderListener *,
		rti::routing::adapter::detail::StreamReaderListener *,
		const PropertySet& properties)
{
	DiaConnection *diaConnection =
            new DiaConnection(properties, odbc_environment_manager_);

	return diaConnection;
}

void DiaAdapter::delete_connection(Connection *connection)
{
	/**
	 * Perform cleanup pertaining to the connection object here.
	 */
	delete connection;
}

rti::config::LibraryVersion DiaAdapter::get_version() const
{
    return { 1, 0, 0, 'r' };
}

extern "C" RTI_USER_DLL_EXPORT
RTI_ADAPTER_PLUGIN_CREATE_FUNCTION_DEF(DiaAdapter);
