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

#ifndef MARIADB_ODBC_TEST_HANDLER_HPP
#define MARIADB_ODBC_TEST_HANDLER_HPP

#include "AbstractTestDbHandler.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "OdbcConnectionManager.hpp"
#include "QueryConverter.hpp"
#include <string>
#include <vector>
#include <memory>
#include <sqlext.h>
#include <sqltypes.h>

namespace rti { namespace adapter { namespace dis { namespace test {

/**
 * @brief MariaDB database handler using ODBC
 */
class MariaDBOdbcTestHandler : public AbstractTestDbHandler {
public:
    struct ConnectionConfig {
        std::string dsn = "mariadb";
        std::string user = "root";
        std::string password = "mypassword";
    };

    explicit MariaDBOdbcTestHandler(
            const ConnectionConfig& config,
            const std::string& database_name = "mariadb_test_database");

    ~MariaDBOdbcTestHandler();

    unsigned long long getRowCount(
            const std::string& tableName,
            const std::string& whereClause = "") override;

    void executeQuery(const std::string& query) override;

    std::vector<int> coordinatesX(const std::string& tableName) override;

    std::vector<std::string> listTables() override;

    void drop_all_tables() override;

private:
    void connect() override;
    void disconnect() override;

    ConnectionConfig config_;
    std::unique_ptr<rti::adapter::dis::odbc::OdbcEnvironmentManager> odbc_env_;
    std::unique_ptr<rti::adapter::dis::odbc::OdbcConnectionManager> odbc_conn_;
    std::unique_ptr<QueryConverter> query_converter_;

    MariaDBOdbcTestHandler(const MariaDBOdbcTestHandler&) = delete;
    MariaDBOdbcTestHandler& operator=(const MariaDBOdbcTestHandler&) = delete;
};

}}}}  // namespace rti::adapter::dis::test

#endif // MARIADB_ODBC_TEST_HANDLER_HPP
