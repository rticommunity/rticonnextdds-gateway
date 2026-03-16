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
#include "DiaTestTypes.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "MetadataFacade.hpp"
#include "SampledataFacade.hpp"
#include "AbstractTestDbHandler.hpp"
#include "rtitest/TestAssertion.hpp"
#include <rti/core/xtypes/DynamicDataImpl.hpp>
#include <stdexcept>
#include <thread>
#include <chrono>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace dds::core::xtypes;
using namespace rti::adapter::dis::odbc;
using namespace shapes;

/**
 * @brief DDSTestContext implementation
 */
DDSTestContext::DDSTestContext()
    : test_context_(nullptr)
{
    test_context_ = DDSCTestContext_new();
    RTI_TEST_ASSERT(test_context_ != nullptr);
}

DDSTestContext::~DDSTestContext()
{
    if (test_context_ != nullptr) {
        DDSCTestContext_delete(test_context_);
        test_context_ = nullptr;
    }
}

/**
 * @brief Validate row counts across multiple database handlers with retry logic
 * 
 * This function polls the database row counts with a delay between attempts,
 * allowing time for asynchronous operations (like DDS dispose events) to
 * propagate and be processed before validation.
 * 
 * @param handlers Vector of database handlers to check
 * @param table_name Name of the table to check
 * @param expected_count Expected number of rows
 * @param timeout_seconds Maximum time to wait (default 10 seconds)
 * @param poll_interval_ms Delay between polling attempts (default 100ms)
 */
void validate_row_counts(
        const std::vector<AbstractTestDbHandler*>& handlers,
        const std::string& table_name,
        unsigned long long expected_count)
{
    const int timeout_seconds = 10;
    const int poll_interval_ms = 100;
    const int max_attempts = (timeout_seconds * 1000) / poll_interval_ms;
    
    for (size_t i = 0; i < handlers.size(); i++) {
        auto* handler = handlers[i];
        bool success = false;
        unsigned long long row_count = 0;
        
        // Poll with delay to allow async operations to complete
        for (int attempt = 0; attempt < max_attempts; attempt++) {
            row_count = handler->getRowCount(table_name);
            
            if (row_count == expected_count) {
                success = true;
                break;
            }
            
            // Sleep before next attempt
            std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
        }
        
        // Final check with assertion
        if (!success) {
            printf("  Database[%zu] has %llu rows (expected %llu) - FAILED after %d seconds\n", 
                   i, row_count, expected_count, timeout_seconds);
        }
        RTI_TEST_ASSERT(success);
    }
}

int count_query_results(const SQLHSTMT & statement)
{
	int count = 0;
	RETCODE retcode;
	std::string error_text;

	retcode = SQLBindCol(statement, 1, SQL_C_LONG, &count, 0, NULL);
	RTI_TEST_ASSERT(SQL_SUCCEEDED(retcode));

	retcode = SQLFetch(statement);
	RTI_TEST_ASSERT(SQL_SUCCEEDED(retcode));

	return count;
}

int count_tables(
		const OdbcStatementManager& statement_manager,
		const std::string& table_name)
{
	std::string sql_query = ((std::string)"SELECT COUNT(*) "
			"FROM INFORMATION_SCHEMA.TABLES "
			"WHERE TABLE_NAME = '")
				.append(table_name)
				.append("'");
	statement_manager.execute_query(sql_query);
	return count_query_results(
            statement_manager.get_statement(OdbcStatementKind::custom));
}

int count_table_rows(
		const OdbcStatementManager& statement_manager,
		const std::string& table_name)
{
	std::string sql_query = ("SELECT COUNT(*) FROM `");
	sql_query.append(table_name).append("`");
	statement_manager.execute_query(sql_query);

	return count_query_results(
            statement_manager.get_statement(OdbcStatementKind::custom));
}

int count_types_table_rows(
		const OdbcStatementManager& statement_manager,
		const rti::routing::StreamInfo& info)
{
	std::string sql_query = ((std::string)"SELECT COUNT(*) FROM types WHERE "
			"topic_name = '")
				.append(info.stream_name())
				.append("'");
	statement_manager.execute_query(sql_query);

	return count_query_results(
            statement_manager.get_statement(OdbcStatementKind::custom));
}

void check_json_value(
		const OdbcStatementManager& statement_manager,
		const std::string & table_name,
		std::string original_json_value)
{
	std::string statement = "SELECT json_sample FROM `" + table_name + "`";
	bool retcode = statement_manager.execute_query(statement);
	RTI_TEST_ASSERT(SQL_SUCCEEDED(retcode));

	SQLCHAR json_value[255];
	SQLLEN indicator;
	auto statement_handler = 
            statement_manager.get_statement(OdbcStatementKind::custom);

	RTI_TEST_ASSERT(SQLFetch(statement_handler) == SQL_SUCCESS);

	retcode = SQLGetData(statement_handler, 1, SQL_C_CHAR, json_value,
			sizeof(json_value), &indicator);
	RTI_TEST_ASSERT(SQL_SUCCEEDED(retcode));
	RTI_TEST_ASSERT(
			!original_json_value.compare(
                    reinterpret_cast<const char *>(json_value)));
}

DynamicData mock_sample()
{
	auto idl_sample = ShapeExtended(Type::SQUARE, Color::RED, 10,
			Coordinates(0, 0), ShapeFillKind::SOLID_FILL, "little_square");
	DynamicData sample = rti::core::xtypes::convert<ShapeExtended>(idl_sample);

	return sample;
}

dds::sub::SampleInfo mock_sample_info()
{
	// Create sample_info
	dds::sub::SampleInfo sample_info;
	DDS_SampleInfo &c_info = sample_info.extensions().native();

	// Initialize numeric metadata
	c_info.reception_timestamp.sec = 1736611200;
	c_info.reception_timestamp.nanosec = 50000;

	// Initialize binary metadata
	c_info.instance_handle.keyHash.value[2] = 54;
	c_info.instance_handle.isValid = 1;
	auto binary = MetadataFacade().return_instance_handle_as_vector(
			sample_info.instance_handle());
	DDS_InstanceHandle_t c_handle = DDS_HANDLE_NIL;
	c_handle.isValid = 1;
	dds::core::InstanceHandle value_back(c_handle);
	MetadataFacade().vector_to_instance_handle(value_back, binary);
	// Compare instance_handle's
	RTI_TEST_ASSERT(sample_info.instance_handle() == value_back);

	return sample_info;
}

}}}}  // namespace rti::adapter::dis::test
