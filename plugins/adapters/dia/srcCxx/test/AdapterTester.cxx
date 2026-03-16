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

#include "AdapterTester.hpp"
#include "DiaConnection.hpp"
#include "DiaStreamWriter.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "MariaDBOdbcTestHandler.hpp"
#include "DiaTestTypes.hpp"
#include <rtitest/Tester.hpp>
#include <rti/routing/PropertySet.hpp>
#include <rti/routing/StreamInfo.hpp>
#include <rti/routing/TypeInfo.hpp>
#include <rti/routing/adapter/StreamWriter.hpp>
#include <rti/core/xtypes/DynamicDataImpl.hpp>
#include <iostream>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace rti::routing;
using namespace rti::adapter::dis;
using namespace rti::adapter::dis::odbc;

AdapterTester::AdapterTester() : rti::test::Tester("AdapterTester")
{
    RTI_TEST_FUNCTION_ADD(AdapterTester, test_json_topic_properties_parsing);
}

void AdapterTester::test_json_topic_properties_parsing()
{
    // Create Database
    MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
    MariaDBOdbcTestHandler mariadb_handler(mariadb_config);
    
    // Create ODBC environment
    OdbcEnvironmentManager odbc_env;
    
    // Initialize properties with JSON file path
    PropertySet connection_properties = PropertySet{
        {"DSN", "mariadb"},
        {"topic_properties", "plugins/adapters/dia/resources/json/test_topic_properties_parsing.json"},
        {"query_converter", "mariadb"}
    };

    // Create DiaConnection
    DiaConnection connection(connection_properties, odbc_env);

    // Create StreamWriter properties with domain_id
    PropertySet writer_properties = PropertySet{
        {"dds.domain_id", "0"}
    };

    // Create TypeInfo with valid DynamicType for ShapeExtended
    const dds::core::xtypes::StructType& shape_type = 
            rti::topic::dynamic_type<shapes::ShapeExtended>::get();
    rti::routing::TypeInfo type_info(shape_type.name());
    type_info.dynamic_type(&shape_type);

    // Test Topic1 with instance_history = 3
    {
        StreamInfo stream_info_topic1 = StreamInfo("Topic1", type_info.type_name());
        stream_info_topic1.type_info() = type_info;

        // Create stream writer for Topic1
        auto* writer = connection.create_stream_writer(
            nullptr,
            stream_info_topic1,
            writer_properties);

        // Cast to DiaStreamWriter to access internal properties
        DiaStreamWriter* dis_writer = dynamic_cast<DiaStreamWriter*>(writer);
        RTI_TEST_ASSERT(dis_writer != nullptr);

        // Verify instance_history is 3 for Topic1
        RTI_TEST_ASSERT_EQUALS_INT(3, dis_writer->instance_history());

        // Verify delete_on_dispose is false
        RTI_TEST_ASSERT(!dis_writer->delete_on_dispose());

        // Clean up
        delete writer;
    }

    // Test Topic2 with instance_history = 2
    {
        StreamInfo stream_info_topic2 = StreamInfo("Topic2", type_info.type_name());
        stream_info_topic2.type_info() = type_info;

        // Create stream writer for Topic2
        auto* writer = connection.create_stream_writer(
            nullptr,
            stream_info_topic2,
            writer_properties);

        // Cast to DiaStreamWriter to access internal properties
        DiaStreamWriter* dis_writer = dynamic_cast<DiaStreamWriter*>(writer);
        RTI_TEST_ASSERT(dis_writer != nullptr);

        // Verify instance_history is 2 for Topic2
        RTI_TEST_ASSERT_EQUALS_INT(2, dis_writer->instance_history());

        // Verify delete_on_dispose is false
        RTI_TEST_ASSERT(!dis_writer->delete_on_dispose());

        // Clean up
        delete writer;
    }

    // Test default properties for unmatched topic
    {
        StreamInfo stream_info_unknown = StreamInfo("UnknownTopic", type_info.type_name());
        stream_info_unknown.type_info() = type_info;

        // Create stream writer for unknown topic (should use defaults)
        auto* writer = connection.create_stream_writer(
            nullptr,
            stream_info_unknown,
            writer_properties);

        // Cast to DiaStreamWriter to access internal properties
        DiaStreamWriter* dis_writer = dynamic_cast<DiaStreamWriter*>(writer);
        RTI_TEST_ASSERT(dis_writer != nullptr);

        // Verify default instance_history is 0
        RTI_TEST_ASSERT_EQUALS_INT(0, dis_writer->instance_history());

        // Clean up
        delete writer;
    }
}

} } } } // namespace rti::adapter::dis::test
