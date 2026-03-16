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

#include "SqlTableTester.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "OdbcStatementManager.hpp"
#include "TestUtils.hpp"
#include "AbstractTestDbHandler.hpp"
#include "MariaDBOdbcTestHandler.hpp"
#include "SqlTable.hpp"
#include "DiaConnection.hpp"
#include "DiaTestTypes.hpp"
#include "SampledataFacade.hpp"
#include "MetadataFacade.hpp"
#include <iostream>
#include <rtitest/Tester.hpp>
#include <rti/core/xtypes/DynamicDataImpl.hpp>
#include <rti/routing/PropertySet.hpp>
#include <rti/routing/StreamInfo.hpp>
#include <rti/routing/TypeInfo.hpp>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <dds/dds.hpp>
#include <cstring>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace rti::routing;
using namespace rti::adapter::dis;
using namespace rti::adapter::dis::odbc;
using namespace dds::core::xtypes;
using namespace shapes;

SQLTableTester::SQLTableTester() : rti::test::Tester("SQLTableTester")
{
    RTI_TEST_FUNCTION_ADD(SQLTableTester, test_table_creation_deletion);
    RTI_TEST_FUNCTION_ADD(SQLTableTester, test_sample_insertion);
    RTI_TEST_FUNCTION_ADD(SQLTableTester, test_dispose_instance);
    RTI_TEST_FUNCTION_ADD(SQLTableTester, test_type_representation_xcdr);
    RTI_TEST_FUNCTION_ADD(SQLTableTester, test_type_representation_xml);
}

void SQLTableTester::test_table_creation_deletion()
{
	// Initialize properties
	PropertySet properties = PropertySet{
		{"DSN", "mariadb"},
	};

    // Create Database
    MariaDBOdbcTestHandler::ConnectionConfig sql_config;
	MariaDBOdbcTestHandler sql_handler(sql_config);

	// Create TopicProperties
	auto topic_props = std::make_shared<TopicProperties>();
	topic_props->delete_type_on_dispose = true;
	topic_props->drop_table_on_dispose = true;
	topic_props->format = TopicStorageFormat::json;

	// Create StreamInfo with valid TypeInfo
	const dds::core::xtypes::StructType& type = 
            rti::topic::dynamic_type<ShapeExtended>::get();
    rti::routing::TypeInfo type_info(type.name());
    type_info.dynamic_type(&type);
    rti::routing::StreamInfo stream_info("StreamName", type_info.type_name());
    stream_info.type_info() = type_info;
	auto table_name = stream_info.stream_name() + "@" + "0";
    TypeRepresentationFormat type_format = TypeRepresentationFormat::XCDR;
	{
		// Connect to database
		std::shared_ptr<OdbcConnectionManager> conn(
				new OdbcConnectionManager(OdbcEnvironmentManager()));
		conn->connect_to_database(properties);
		auto sql_table = SqlTable(
                topic_props,
                stream_info,
                conn,
                QueryConverterFormat::mariadb,
                type_format,
                0);
		auto& statement_manager = sql_table.get_statement_manager();

		// check square@0 table has been created
        RTI_TEST_ASSERT_EQUALS_INT(
                1,
                count_tables(statement_manager, table_name));

		// check types table has been created
        RTI_TEST_ASSERT_EQUALS_INT(
                1,
                count_tables(statement_manager, "types"));

		// check types contains the square entry
        RTI_TEST_ASSERT_EQUALS_INT(
                1,
                count_types_table_rows(statement_manager, stream_info));
	}

	// Connect to database
    std::shared_ptr<OdbcConnectionManager> conn(
            new OdbcConnectionManager(OdbcEnvironmentManager()));
	conn->connect_to_database(properties);
	auto statement_manager = OdbcStatementManager(conn);
	statement_manager.allocate_statement(OdbcStatementKind::custom);

	// check square@0 table has been dropped by the destructor
    RTI_TEST_ASSERT_EQUALS_INT(0, count_tables(statement_manager, table_name));

	// check types table has not been dropped by the destructor
    RTI_TEST_ASSERT_EQUALS_INT(
            1,
            count_tables(statement_manager, "types"));

	// check types contains the square entry
    RTI_TEST_ASSERT_EQUALS_INT(
            0,
            count_types_table_rows(statement_manager, stream_info));
}

void SQLTableTester::test_sample_insertion()
{
	PropertySet properties = PropertySet{
		{"DSN", "mariadb"},
	};

	// Create Database
	MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
	MariaDBOdbcTestHandler mariadb_handler(mariadb_config);

	// Create TopicProperties
	auto topic_props = std::make_shared<TopicProperties>();
	topic_props->delete_type_on_dispose = false;
	topic_props->drop_table_on_dispose = false;
	topic_props->format = TopicStorageFormat::json;

	// Insert sample in database
	{
		// Connect to database
        const dds::core::xtypes::StructType& type = rti::topic::dynamic_type<ShapeExtended>::get();
        rti::routing::TypeInfo type_info(type.name());
        type_info.dynamic_type(&type);
        rti::routing::StreamInfo stream_info("StreamName", type_info.type_name());
        stream_info.type_info() = type_info;
        TypeRepresentationFormat type_format = TypeRepresentationFormat::XCDR;
		std::shared_ptr<OdbcConnectionManager> conn(
				new OdbcConnectionManager(OdbcEnvironmentManager()));
		conn->connect_to_database(properties);
		auto sql_table = SqlTable(
                topic_props, 
                stream_info, 
                conn, 
                QueryConverterFormat::mariadb,
                type_format,
                0);
		auto& statement_manager = sql_table.get_statement_manager();

		// Mock sample data
		const auto sample = mock_sample();

		// Mock sample metadata
		auto sample_info = mock_sample_info();

		// Perform sample insertion
		sql_table.insert_sample(&sample, &sample_info);

		// Check JSON value coincides with inserted value
		auto table_name = stream_info.stream_name().append("@").append("0");
		auto original_json_value = 
                SampledataFacade().transform_sample_to_json(&sample);
		check_json_value(statement_manager, table_name, original_json_value);
	}
}

void SQLTableTester::test_dispose_instance()
{
    PropertySet properties = PropertySet{
        {"DSN", "mariadb"},
    };

    // Create Database
    MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
    MariaDBOdbcTestHandler mariadb_handler(mariadb_config);

    // Create TopicProperties
    auto topic_props = std::make_shared<TopicProperties>();
    topic_props->delete_on_dispose = true;
    topic_props->delete_type_on_dispose = false;
    topic_props->drop_table_on_dispose = false;
    topic_props->format = TopicStorageFormat::json;

    // Insert sample in database
    {
        // Connect to database
        const dds::core::xtypes::StructType& type = rti::topic::dynamic_type<ShapeExtended>::get();
        rti::routing::TypeInfo type_info(type.name());
        type_info.dynamic_type(&type);
        rti::routing::StreamInfo stream_info("StreamName", type_info.type_name());
        stream_info.type_info() = type_info;
        TypeRepresentationFormat type_format = TypeRepresentationFormat::XCDR;
        std::shared_ptr<OdbcConnectionManager> conn(
                new OdbcConnectionManager(OdbcEnvironmentManager()));
        conn->connect_to_database(properties);
        auto sql_table = SqlTable(
                topic_props, 
                stream_info, 
                conn, 
                QueryConverterFormat::mariadb,
                type_format,
                0);
        auto& statement_manager = sql_table.get_statement_manager();

        // Mock sample data
        const auto sample = mock_sample();

        // Mock sample metadata
        auto sample_info = mock_sample_info();

        // Insert sample of first instance_handle
        sql_table.insert_sample(&sample, &sample_info);
        sql_table.insert_sample(&sample, &sample_info);
        sql_table.insert_sample(&sample, &sample_info);

        // Insert sample of second instance_handle
        sample_info.extensions().native().instance_handle.keyHash.value[3] = 27;
        sql_table.insert_sample(&sample, &sample_info);
        sql_table.insert_sample(&sample, &sample_info);

        auto table_name = stream_info.stream_name().append("@").append("0");
        RTI_TEST_ASSERT_EQUALS_INT(
                2,
                mariadb_handler.getRowCount(table_name));

        // Remove second instance handle
        sql_table.remove_instance(&sample_info);

        RTI_TEST_ASSERT_EQUALS_INT(
                1,
                mariadb_handler.getRowCount(table_name));

        // Remove first instance handle
        sample_info.extensions().native().instance_handle.keyHash.value[3] = 0;
        sql_table.remove_instance(&sample_info);
        RTI_TEST_ASSERT_EQUALS_INT(
                0,
                mariadb_handler.getRowCount(table_name));
	}
}

void SQLTableTester::test_type_representation_xcdr()
{
    PropertySet properties = PropertySet{
        {"DSN", "mariadb"},
    };

    // Create Database
    MariaDBOdbcTestHandler::ConnectionConfig sql_config;
    MariaDBOdbcTestHandler sql_handler(sql_config);

	// Create TopicProperties
	auto topic_props = std::make_shared<TopicProperties>();
	topic_props->delete_type_on_dispose = true;
	topic_props->drop_table_on_dispose = true;
	topic_props->format = TopicStorageFormat::json;

    // Create StreamInfo with valid TypeInfo
    const dds::core::xtypes::StructType& type = 
            rti::topic::dynamic_type<ShapeExtended>::get();
    rti::routing::TypeInfo type_info(type.name());
    type_info.dynamic_type(&type);
    rti::routing::StreamInfo stream_info("StreamName", type_info.type_name());
    stream_info.type_info() = type_info;

    // Get original XCDR representation using MetadataFacade
    auto original_xcdr = MetadataFacade::extract_type_xcdr(&stream_info);

    {
        // Insert type with XCDR format
        TypeRepresentationFormat type_format = TypeRepresentationFormat::XCDR;
		std::shared_ptr<OdbcConnectionManager> conn(
				new OdbcConnectionManager(OdbcEnvironmentManager()));
		conn->connect_to_database(properties);
        auto sql_table = SqlTable(
                topic_props,
                stream_info,
                conn, 
                QueryConverterFormat::mariadb,
                type_format,
                0);
        auto& statement_manager = sql_table.get_statement_manager();

        // Prepare query to retrieve type_representation from types
        std::string query = "SELECT type_representation FROM types WHERE topic_name = 'StreamName'";
        statement_manager.allocate_statement(OdbcStatementKind::custom);
        statement_manager.prepare_query(OdbcStatementKind::custom, query);
        statement_manager.execute(OdbcStatementKind::custom);

        // Fetch the result
        auto statement = statement_manager.get_statement(OdbcStatementKind::custom);
        SQLRETURN retcode = SQLFetch(statement);
        RTI_TEST_ASSERT(SQL_SUCCEEDED(retcode));

        // Retrieve the binary data
        SQLLEN indicator = 0;
        std::vector<uint8_t> retrieved_xcdr(original_xcdr.size());
        retcode = SQLGetData(
                statement,
                1,
                SQL_C_BINARY,
                retrieved_xcdr.data(),
                retrieved_xcdr.size(),
                &indicator);
        RTI_TEST_ASSERT(SQL_SUCCEEDED(retcode));

        // Compare original and retrieved XCDR data
        RTI_TEST_ASSERT_EQUALS_INT(
                static_cast<int>(original_xcdr.size()),
                static_cast<int>(indicator));
        RTI_TEST_ASSERT(
                std::equal(
                        original_xcdr.begin(),
                        original_xcdr.end(),
                        retrieved_xcdr.begin()));
    }
}

void SQLTableTester::test_type_representation_xml()
{
    PropertySet properties = PropertySet{
        {"DSN", "mariadb"},
    };

    // Create Database
    MariaDBOdbcTestHandler::ConnectionConfig sql_config;
    MariaDBOdbcTestHandler sql_handler(sql_config);

	// Create TopicProperties
	auto topic_props = std::make_shared<TopicProperties>();
	topic_props->delete_type_on_dispose = true;
	topic_props->drop_table_on_dispose = true;
	topic_props->format = TopicStorageFormat::json;

    // Create StreamInfo with valid TypeInfo
    const dds::core::xtypes::StructType& type = 
            rti::topic::dynamic_type<ShapeExtended>::get();
    rti::routing::TypeInfo type_info(type.name());
    type_info.dynamic_type(&type);
    rti::routing::StreamInfo stream_info("StreamName", type_info.type_name());
    stream_info.type_info() = type_info;
	auto table_name = stream_info.stream_name() + "@" + "0";

    // Get original XML representation using MetadataFacade
    auto original_xml = MetadataFacade::extract_type_xml(&stream_info);

    {
        // Insert type with XML format
        TypeRepresentationFormat type_format = TypeRepresentationFormat::XML;
		std::shared_ptr<OdbcConnectionManager> conn(
				new OdbcConnectionManager(OdbcEnvironmentManager()));
		conn->connect_to_database(properties);
        auto sql_table = SqlTable(
                topic_props,
                stream_info,
                conn,
                QueryConverterFormat::mariadb,
                type_format,
                0);
        auto& statement_manager = sql_table.get_statement_manager();

        // Prepare query to retrieve type_representation from types
        std::string query = "SELECT type_representation FROM types WHERE topic_name = 'StreamName'";
        statement_manager.allocate_statement(OdbcStatementKind::custom);
        statement_manager.prepare_query(OdbcStatementKind::custom, query);
        statement_manager.execute(OdbcStatementKind::custom);

        // Fetch the result
        auto statement = statement_manager.get_statement(OdbcStatementKind::custom);
        SQLRETURN retcode = SQLFetch(statement);
        RTI_TEST_ASSERT(SQL_SUCCEEDED(retcode));

        // Retrieve the text data
        SQLLEN indicator = 0;
        std::vector<char> retrieved_xml(original_xml.length() + 1);
        retcode = SQLGetData(
                statement,
                1,
                SQL_C_CHAR,
                retrieved_xml.data(),
                retrieved_xml.size(),
                &indicator);
        RTI_TEST_ASSERT(SQL_SUCCEEDED(retcode));

        // Compare original and retrieved XML data
        std::string retrieved_xml_str(retrieved_xml.data());
        RTI_TEST_ASSERT_EQUALS_INT(
                static_cast<int>(original_xml.length()),
                static_cast<int>(retrieved_xml_str.length()));
        RTI_TEST_ASSERT(original_xml == retrieved_xml_str);
    }
}

}}}} // rti::adapter::dis::test

