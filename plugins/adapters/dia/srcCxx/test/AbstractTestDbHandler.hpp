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

#ifndef ABSTRACT_TEST_DB_HANDLER_HPP
#define ABSTRACT_TEST_DB_HANDLER_HPP

#include <string>
#include <vector>

namespace rti { namespace adapter { namespace dis { namespace test {

/**
 * @brief Abstract base class for database test handlers
 * 
 * This class provides a common interface for different database test handlers
 * (MariaDB, PostgreSQL, etc.) and implements shared functionality for:
 * - Connecting to existing databases
 * - Executing queries
 * - Counting rows in tables
 * 
 * Derived classes must implement database-specific operations.
 * Note: Databases are expected to be pre-created (e.g., by Docker Compose).
 */
class AbstractTestDbHandler {
public:
    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~AbstractTestDbHandler() = default;

    /**
     * @brief Get the row count of a table using COUNT(*)
     * @param tableName The name of the table to count rows from
     * @param whereClause Optional WHERE clause (without the WHERE keyword)
     * @return Number of rows in the table
     */
    virtual unsigned long long getRowCount(
            const std::string& tableName,
            const std::string& whereClause = "") = 0;

    /**
     * @brief Execute a raw SQL query (for DDL statements like CREATE, DROP, etc.)
     * @param query The SQL query to execute
     */
    virtual void executeQuery(const std::string& query) = 0;

    /**
     * @brief Get all x coordinates from a table ordered by RTI_HISTORY_ORDER
     * @param tableName The name of the table to query
     * @return Vector of x coordinate values ordered by history order (oldest to newest)
     */
    virtual std::vector<int> coordinatesX(const std::string& tableName) = 0;

    /**
     * @brief List all table names currently present in the database
     * @return Vector with table names
     */
    virtual std::vector<std::string> listTables() = 0;

    /**
     * @brief Drop all tables in the database
     * Used to clean up before tests to ensure a fresh state
     */
    virtual void drop_all_tables() = 0;

protected:
    /**
     * @brief Protected constructor to prevent direct instantiation
     * @param database_name Name of the test database
     */
    explicit AbstractTestDbHandler(const std::string& database_name)
        : database_name_(database_name) {}

    /**
     * @brief Connect to the database
     * Must be implemented by derived classes
     */
    virtual void connect() = 0;

    /**
     * @brief Disconnect from the database
     * Must be implemented by derived classes
     */
    virtual void disconnect() = 0;

    /**
     * @brief Get the database name
     */
    const std::string& get_database_name() const {
        return database_name_;
    }

    /**
     * @brief Database name for testing
     */
    std::string database_name_;

    // Prevent copying
    AbstractTestDbHandler(const AbstractTestDbHandler&) = delete;
    AbstractTestDbHandler& operator=(const AbstractTestDbHandler&) = delete;
};

}}}}  // namespace rti::adapter::dis::test

#endif // ABSTRACT_TEST_DB_HANDLER_HPP
