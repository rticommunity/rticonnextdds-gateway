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

#ifndef ODBCCONNECTIONMANAGER_HPP
#define ODBCCONNECTIONMANAGER_HPP

#include <rti/routing/PropertySet.hpp>
#include "OdbcEnvironmentManager.hpp"
#include <sqlext.h>
#include <sqltypes.h>
#include "Utils.hpp"

namespace rti { namespace adapter { namespace dis { namespace odbc {

/**
 * @brief OdbcConnectionManager class. This class acts as a factory for
 * OdbcStatementManager objects.
 */
class OdbcConnectionManager {

public:
	OdbcConnectionManager(const OdbcEnvironmentManager& odbc_env);

	~OdbcConnectionManager();

	/**
	 * @brief Attempts to connect to the database given in the
	 *		PropertySet
	 *
	 * @param property_set PropertySet object from which a DSN will be read
	 */
	void connect_to_database(const rti::routing::PropertySet& property_set);

	/**
	 * @brief Attempts to disconnect from the currently connected
	 *		database
	 */
	void disconnect_from_database();

	SQLHSTMT create_statement() const;

	/**
	 * @brief Commits the current transaction
	 */
	void commit_transaction();

	/**
	 * @brief Rolls back the current transaction
	 */
	void rollback_transaction();

private:
	SQLHDBC connection_;
};

} } } } // namespace rti::adapter::dis::odbc

#endif

