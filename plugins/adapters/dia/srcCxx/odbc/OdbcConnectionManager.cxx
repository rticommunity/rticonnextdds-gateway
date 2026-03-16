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

#include "OdbcConnectionManager.hpp"
#include <rti/routing/Logger.hpp>
#include <string>

using namespace rti::adapter::dis::odbc;

OdbcConnectionManager::OdbcConnectionManager(
        const OdbcEnvironmentManager& odbc_env)
    : connection_(odbc_env.create_connection())
{}

OdbcConnectionManager::~OdbcConnectionManager()
{
	SQLFreeHandle(SQL_HANDLE_DBC, connection_);
}

void OdbcConnectionManager::connect_to_database(
		const rti::routing::PropertySet& properties)
{
	// Build connection string from DSN and optional parameters
	auto dsn_it = properties.find("DSN");
	if (dsn_it == properties.end()) {
		throw std::runtime_error("DSN property is required");
	}
	
	std::string connection_string = "DSN=" + dsn_it->second + ";";
	
	// Add optional DATABASE if provided and not empty
	auto db_it = properties.find("DATABASE");
	if (db_it != properties.end() && !db_it->second.empty()) {
		connection_string += "DATABASE=" + db_it->second + ";";
	}
	
	// Add optional UID if provided
	auto uid_it = properties.find("UID");
	if (uid_it != properties.end() && !uid_it->second.empty()) {
		connection_string += "UID=" + uid_it->second + ";";
	}
	
	// Add optional PWD if provided
	auto pwd_it = properties.find("PWD");
	if (pwd_it != properties.end() && !pwd_it->second.empty()) {
		connection_string += "PWD=" + pwd_it->second + ";";
	}

	SQLCHAR outstr[1024];
	SQLSMALLINT outstrlen;
	SQLRETURN retcode = SQLDriverConnect(
			connection_, NULL, 
			(SQLCHAR*)connection_string.c_str(), 
			SQL_NTS, 
			outstr, 
			sizeof(outstr),
			&outstrlen, 
			SQL_DRIVER_NOPROMPT);

	auto diagnostics = get_handle_diagnostics(connection_, SQL_HANDLE_DBC);
	if(!SQL_SUCCEEDED(retcode)) {
		std::string error_text;
		error_text.append("Unable to connect to provided DSN: ")
                .append(dsn_it->second);
		auto error_message = build_diagnostics(
				error_text,
				std::to_string(retcode),
				diagnostics);
		throw std::runtime_error(error_message);
	}

    if (rti::routing::Logger::instance().service_verbosity() ==
            rti::config::Verbosity::STATUS_LOCAL) {
        std::string success_text;
        success_text.append("Succesfully connected to provided DSN: ")
                .append(dsn_it->second);
        auto success_message = build_diagnostics(
                success_text,
                std::to_string(retcode),
                diagnostics);
        rti::routing::Logger::instance().local(success_message);
    }

	// Disable auto-commit mode by default to enable transaction control
	retcode = SQLSetConnectAttr(
			connection_,
			SQL_ATTR_AUTOCOMMIT,
			(SQLPOINTER)SQL_AUTOCOMMIT_OFF,
			0);

	diagnostics = get_handle_diagnostics(connection_, SQL_HANDLE_DBC);
	if (!SQL_SUCCEEDED(retcode)) {
		std::string error_text = "Unable to disable auto-commit mode";
		auto error_message = build_diagnostics(
				error_text,
				std::to_string(retcode),
				diagnostics);
		throw std::runtime_error(error_message);
	}
}

void OdbcConnectionManager::disconnect_from_database()
{
	SQLDisconnect(connection_);
}

SQLHSTMT OdbcConnectionManager::create_statement() const
{
    SQLHSTMT statement = SQL_NULL_HSTMT;
    SQLRETURN retcode = SQLAllocHandle(
            SQL_HANDLE_STMT, connection_, &statement);

    auto diagnostics = get_handle_diagnostics(connection_, SQL_HANDLE_STMT);
    if(!SQL_SUCCEEDED(retcode)) {
        std::string error_text = "Unable to allocate statement handle";
        auto error_message = build_diagnostics(
                error_text,
                std::to_string(retcode),
                diagnostics);
        throw std::runtime_error(error_message);
    }

    return statement;
}

void OdbcConnectionManager::commit_transaction()
{
	// Commit the transaction
	SQLRETURN retcode = SQLEndTran(SQL_HANDLE_DBC, connection_, SQL_COMMIT);

	auto diagnostics = get_handle_diagnostics(connection_, SQL_HANDLE_DBC);
	if (!SQL_SUCCEEDED(retcode)) {
		std::string error_text = "Unable to commit transaction";
		auto error_message = build_diagnostics(
				error_text,
				std::to_string(retcode),
				diagnostics);
		throw std::runtime_error(error_message);
	}
}

void OdbcConnectionManager::rollback_transaction()
{
	// Rollback the transaction
	SQLRETURN retcode = SQLEndTran(SQL_HANDLE_DBC, connection_, SQL_ROLLBACK);

	auto diagnostics = get_handle_diagnostics(connection_, SQL_HANDLE_DBC);
	if (!SQL_SUCCEEDED(retcode)) {
		std::string error_text = "Unable to rollback transaction";
		auto error_message = build_diagnostics(
				error_text,
				std::to_string(retcode),
				diagnostics);
		// Log the error but don't throw during rollback
		rti::routing::Logger::instance().error(error_message);
	}
}
