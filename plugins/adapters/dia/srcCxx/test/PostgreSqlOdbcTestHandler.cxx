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

#include "PostgreSqlOdbcTestHandler.hpp"
#include <sstream>
#include <iostream>
#include <rtitest/Tester.hpp>

namespace rti { namespace adapter { namespace dis { namespace test {

PostgreSqlOdbcTestHandler::PostgreSqlOdbcTestHandler(
        const ConnectionConfig& config,
        const std::string& database_name)
    : AbstractTestDbHandler(database_name),
      config_(config),
      odbc_env_(new rti::adapter::dis::odbc::OdbcEnvironmentManager()),
      odbc_conn_(new rti::adapter::dis::odbc::OdbcConnectionManager(*odbc_env_)),
      query_converter_(nullptr)
{
    query_converter_ = QueryConverterFactory().create_query_converter(
            QueryConverterFormat::postgresql);
    
    connect();
    drop_all_tables();
}

PostgreSqlOdbcTestHandler::~PostgreSqlOdbcTestHandler()
{
    if (odbc_conn_) {
        disconnect();
    }
}

void PostgreSqlOdbcTestHandler::connect()
{
    rti::routing::PropertySet properties;
    properties["DSN"] = config_.dsn;
    
    if (!config_.user.empty()) {
        properties["UID"] = config_.user;
    }
    if (!config_.password.empty()) {
        properties["PWD"] = config_.password;
    }
    
    odbc_conn_->connect_to_database(properties);
}

void PostgreSqlOdbcTestHandler::disconnect()
{
    odbc_conn_->disconnect_from_database();
}

unsigned long long PostgreSqlOdbcTestHandler::getRowCount(
        const std::string& tableName,
        const std::string& whereClause)
{
    std::ostringstream query;
    query << "SELECT COUNT(*) as row_count FROM " 
          << query_converter_->quotes() << tableName << query_converter_->quotes();

    if (!whereClause.empty()) {
        query << " WHERE " << whereClause;
    }

    SQLHSTMT stmt = odbc_conn_->create_statement();
    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)query.str().c_str(), SQL_NTS);
    RTI_TEST_ASSERT(SQL_SUCCEEDED(ret));

    ret = SQLFetch(stmt);
    RTI_TEST_ASSERT(SQL_SUCCEEDED(ret));

    SQLBIGINT count = 0;
    SQLLEN indicator;
    ret = SQLGetData(stmt, 1, SQL_C_SBIGINT, &count, sizeof(count), &indicator);
    RTI_TEST_ASSERT(SQL_SUCCEEDED(ret));

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    
    // Commit the transaction (auto-commit is disabled by default)
    odbc_conn_->commit_transaction();

    return static_cast<unsigned long long>(count);
}

void PostgreSqlOdbcTestHandler::executeQuery(const std::string& query)
{
    SQLHSTMT stmt = odbc_conn_->create_statement();
    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    RTI_TEST_ASSERT(SQL_SUCCEEDED(ret));
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    
    // Commit the transaction (auto-commit is disabled by default)
    odbc_conn_->commit_transaction();
}

std::vector<int> PostgreSqlOdbcTestHandler::coordinatesX(const std::string& tableName)
{
    std::vector<int> coordinates;
    
    // Query to get x coordinates from JSON column, ordered by RTI_HISTORY_ORDER
    // json_sample is VARCHAR, so cast to JSONB first, then extract
    std::string query = "SELECT (json_sample::jsonb->'coordinates'->>'x')::int FROM " + 
                        query_converter_->quotes() + tableName + query_converter_->quotes() + 
                        " ORDER BY RTI_HISTORY_ORDER";
    
    SQLHSTMT stmt = odbc_conn_->create_statement();
    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    
    if (!SQL_SUCCEEDED(ret)) {
        std::cerr << "Query failed: " << query << std::endl;
        std::cerr << "Return code: " << ret << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return coordinates;
    }
    
    while (SQL_SUCCEEDED(SQLFetch(stmt))) {
        SQLINTEGER x_value = 0;
        SQLLEN indicator;
        ret = SQLGetData(stmt, 1, SQL_C_SLONG, &x_value, sizeof(x_value), &indicator);
        RTI_TEST_ASSERT(SQL_SUCCEEDED(ret));
        coordinates.push_back(static_cast<int>(x_value));
    }
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    
    // Commit the transaction (auto-commit is disabled by default)
    odbc_conn_->commit_transaction();
    
    return coordinates;
}

std::vector<std::string> PostgreSqlOdbcTestHandler::listTables()
{
    // Query all user tables from the public schema.
    std::string query = "SELECT tablename FROM pg_tables WHERE schemaname = 'public'";

    SQLHSTMT stmt = odbc_conn_->create_statement();
    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    std::vector<std::string> tables;
    if (!SQL_SUCCEEDED(ret)) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return tables;
    }

    while (SQL_SUCCEEDED(SQLFetch(stmt))) {
        char table_name[256];
        SQLLEN indicator;
        ret = SQLGetData(stmt, 1, SQL_C_CHAR, table_name, sizeof(table_name), &indicator);
        if (SQL_SUCCEEDED(ret)) {
            tables.push_back(std::string(table_name));
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    // Commit the transaction (auto-commit is disabled by default)
    odbc_conn_->commit_transaction();

    return tables;
}

void PostgreSqlOdbcTestHandler::drop_all_tables()
{
    std::vector<std::string> tables = listTables();

    // Drop each table
    for (const auto& table : tables) {
        std::string drop_query = "DROP TABLE IF EXISTS " + 
                                 query_converter_->quotes() + table + query_converter_->quotes() +
                                 " CASCADE";
        try {
            executeQuery(drop_query);
        } catch (...) {
            // Ignore errors during cleanup
        }
    }
}

}}}}  // namespace rti::adapter::dis::test
