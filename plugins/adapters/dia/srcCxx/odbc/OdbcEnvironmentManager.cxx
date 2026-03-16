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

#include "OdbcEnvironmentManager.hpp"
#include <rti/routing/Logger.hpp>
#include <string>

using namespace rti::adapter::dis::odbc;

OdbcEnvironmentManager::OdbcEnvironmentManager()
{
	SQLRETURN retcode = SQLAllocHandle(
			SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment_);
	auto diagnostics = get_handle_diagnostics(
			environment_, SQL_HANDLE_ENV);
	if(!SQL_SUCCEEDED(retcode)) {
		std::string error_text = "Unable to allocate environment handle\n";
		auto error_message = build_diagnostics(
				error_text,
				std::to_string(retcode),
				diagnostics);
		throw std::runtime_error(error_message);
	}

    if (rti::routing::Logger::instance().service_verbosity() ==
            rti::config::Verbosity::STATUS_LOCAL) {
        std::string success_text = "Environment handle succesfully allocated";
        auto success_message = build_diagnostics(
                success_text,
                std::to_string(retcode),
                diagnostics);
        rti::routing::Logger::instance().local(success_message);
    }

	/* Configure environment for ODBC 3 support */
	SQLSetEnvAttr(
			environment_, SQL_ATTR_ODBC_VERSION,
			(void *) SQL_OV_ODBC3, 0);
}

SQLHDBC OdbcEnvironmentManager::create_connection() const
{
	SQLHDBC connection;
	SQLRETURN retcode = SQLAllocHandle(
			SQL_HANDLE_DBC, environment_, &connection);
	auto diagnostics = get_handle_diagnostics(environment_, SQL_HANDLE_ENV);
	if(!SQL_SUCCEEDED(retcode)) {
		std::string error_text =  "Unable to allocate connection handle";
		auto error_message = build_diagnostics(
				error_text,
				std::to_string(retcode),
				diagnostics);
		throw std::runtime_error(error_message);
	}

    if (rti::routing::Logger::instance().service_verbosity() ==
            rti::config::Verbosity::STATUS_LOCAL) {
        std::string success_text = "Connection handle succesfully allocated";
        auto success_message = build_diagnostics(
                success_text,
                std::to_string(retcode),
                diagnostics);
        rti::routing::Logger::instance().local(success_message);
    }

	/* Configure connection timeout to 5 seconds*/
	SQLSetConnectAttr(connection, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	return connection;
}

OdbcEnvironmentManager::~OdbcEnvironmentManager()
{
	SQLFreeHandle(SQL_HANDLE_ENV, environment_);
}
