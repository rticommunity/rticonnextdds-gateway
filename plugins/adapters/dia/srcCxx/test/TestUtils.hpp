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

#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include <sqlext.h>
#include <string>
#include <vector>
#include "OdbcStatementManager.hpp"
#include "AbstractTestDbHandler.hpp"
#include "ddsctesthelpers/test_context.h"
#include <rti/routing/StreamInfo.hpp>
#include <rti/core/xtypes/DynamicDataImpl.hpp>
#include <rti/routing/PropertySet.hpp>
#include <dds/dds.hpp>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace dds::core::xtypes;

/**
 * @brief RAII wrapper for DDSCTestContext
 * 
 * Automatically initializes a DDSCTestContext on construction and
 * properly cleans it up on destruction, ensuring no resource leaks.
 */
class DDSTestContext {
public:
    /**
     * @brief Creates and initializes DDSCTestContext
     * @throws Assertion failure if creation or initialization fails
     */
    DDSTestContext();

    /**
     * @brief Finalizes and deletes DDSCTestContext
     */
    ~DDSTestContext();

    /**
     * @brief Get the underlying DDSCTestContext pointer
     * @return Pointer to the managed DDSCTestContext
     */
    DDSCTestContext* get() const { return test_context_; }

    /**
     * @brief Implicit conversion to DDSCTestContext* for convenience
     */
    operator DDSCTestContext*() const { return test_context_; }

    // Prevent copying
    DDSTestContext(const DDSTestContext&) = delete;
    DDSTestContext& operator=(const DDSTestContext&) = delete;

private:
    DDSCTestContext* test_context_;
};

/**
 * @brief Validate row counts across multiple database handlers
 * @param handlers Vector of database handlers to validate
 * @param table_name Name of the table to check
 * @param expected_count Expected number of rows
 */
void validate_row_counts(
        const std::vector<AbstractTestDbHandler*>& handlers,
        const std::string& table_name,
        unsigned long long expected_count);

/**
 * @brief Counts the number of results from a given statement
 */
int count_query_results(const SQLHSTMT & statement);

/**
 * @brief Counts the number of tables with a given name in the current
 *		database
 */
int count_tables(
		const rti::adapter::dis::odbc::OdbcStatementManager& statement_manager,
		const std::string& table_name);

int count_table_rows(
		const rti::adapter::dis::odbc::OdbcStatementManager& statement_manager,
		const std::string& table_name);
/**
 * @brief Counts the number of rows in types for a given stream
 */
int count_types_table_rows(
		const rti::adapter::dis::odbc::OdbcStatementManager& statement_manager,
		const rti::routing::StreamInfo& info);

/**
 * @brief Checks that a given json value stored in a given table is equal to
 *		the original json value
 */
void check_json_value(
		const rti::adapter::dis::odbc::OdbcStatementManager& statement_manager,
		const std::string & table_name,
		std::string original_json_value);

/**
 * @brief Mocks a DynamicData sample of type ShapeExtended
 */
DynamicData mock_sample();

/**
 * @brief Mocks a SampleInfo object
 */
dds::sub::SampleInfo mock_sample_info();

}}}}  // namespace rti::adapter::dis::test

#endif
