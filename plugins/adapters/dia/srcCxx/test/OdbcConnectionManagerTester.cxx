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

#include "TestUtils.hpp"
#include "MariaDBOdbcTestHandler.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "OdbcConnectionManagerTester.hpp"
#include "SqlTable.hpp"
#include "odbc/OdbcConnectionManager.hpp"
#include <rti/routing/PropertySet.hpp>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace rti::routing;
using namespace rti::adapter::dis;
using namespace rti::adapter::dis::odbc;

OdbcConnectionManagerTester::OdbcConnectionManagerTester() 
    : rti::test::Tester("OdbcConnectionManagerTester")
{
    RTI_TEST_FUNCTION_ADD(OdbcConnectionManagerTester, test_database_connection);
}

/**
 * @brief Tests correct connection to mariadb related DSN can be performed as well
 *		as an error is thrown when not
 */
void OdbcConnectionManagerTester::test_database_connection()
{
	// Set up database using ODBC
	MariaDBOdbcTestHandler::ConnectionConfig sql_config;
	MariaDBOdbcTestHandler sql_handler(sql_config);

	// Create Connection Manager
	OdbcConnectionManager odbc_connection_manager{OdbcEnvironmentManager()};

	// Test correct domain source name
	PropertySet correctProperties;
	correctProperties["DSN"] = "mariadb";
    odbc_connection_manager.connect_to_database(correctProperties);

	// Test incorrect domain source name
	PropertySet incorrectProperties;
	incorrectProperties["DSN"] = "WrongDSN";
    RTI_TEST_ASSERT_EXCEPTION(
            odbc_connection_manager.connect_to_database(correctProperties), 
            std::runtime_error);
}

}}}} // rti::adapter::dis::test
