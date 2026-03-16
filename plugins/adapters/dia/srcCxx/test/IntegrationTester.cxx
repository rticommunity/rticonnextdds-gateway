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
#include "PostgreSqlOdbcTestHandler.hpp"
#include "IntegrationTester.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "OdbcStatementManager.hpp"
#include "SqlTable.hpp"
#include "DiaTestTypes.hpp"
#include <iostream>
#include <rti/core/xtypes/DynamicDataImpl.hpp>
#include <rti/routing/PropertySet.hpp>
#include <rti/routing/StreamInfo.hpp>
#include <rtitest/TestAssertion.hpp>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <dds/dds.hpp>
#include <cstring>
#include <algorithm>

#include <rti/routing/RoutingService.hpp>
#include <rti/util/util.hpp>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace rti::routing;
using namespace rti::adapter::dis;
using namespace rti::adapter::dis::odbc;
using namespace dds::core::xtypes;
using namespace shapes;
using namespace rti::routing;

IntegrationTester::IntegrationTester() : rti::test::Tester("IntegrationTester")
{
    RTI_TEST_FUNCTION_ADD(IntegrationTester, test_sample_insertion);
	RTI_TEST_FUNCTION_ADD(
			IntegrationTester,
			test_sample_insertion_multiple_domains);
    RTI_TEST_FUNCTION_ADD(IntegrationTester, test_dispose_instance);
    RTI_TEST_FUNCTION_ADD(IntegrationTester, test_no_dispose_instance);
    RTI_TEST_FUNCTION_ADD(IntegrationTester, test_instance_history);
	RTI_TEST_FUNCTION_ADD(IntegrationTester, test_instance_history_batch);
	RTI_TEST_FUNCTION_ADD(IntegrationTester, test_instance_history_two_topics);
	RTI_TEST_FUNCTION_ADD(IntegrationTester, test_instance_history_unlimited);
}

void IntegrationTester::test_sample_insertion()
{
	// Create Database
	MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
	MariaDBOdbcTestHandler mariadb_handler(mariadb_config);
	
	PostgreSqlOdbcTestHandler::ConnectionConfig postgres_config;
	PostgreSqlOdbcTestHandler postgres_handler(postgres_config);

	const std::string cfg_file =
           "plugins/adapters/dia/resources/xml/test_dia_configuration.xml";
	const std::string service_name = "test_mariadb_postgre_dispose";

	Service service(
			ServiceProperty()
			.cfg_file(cfg_file)
			.service_name(service_name));
	service.start();

	dds::domain::DomainParticipant participant(0);
	dds::topic::Topic<ShapeExtended> topic(participant, "Shapes");
	dds::pub::Publisher publisher(participant);
	dds::pub::DataWriter<ShapeExtended> writer(publisher, topic);
	
	// Wait for discovery
	DDSTestContext test_context;
	RTI_TEST_WAIT_FOR_GREATER_OR_EQUALS(
            DDSCTestContext_getMatchingSubscriptionsLength(
                    test_context, 
                    writer->native_writer()),
            1,
            10);

	// Create data sample for writing
	auto sample = ShapeExtended(
            Type::SQUARE,
            Color::RED, 10,
			Coordinates(0, 0),
            ShapeFillKind::SOLID_FILL,
            "little_square");

	for (int i=0; i<5; i++) {
		writer.write(sample);
		rti::util::sleep(dds::core::Duration(0, 500000000));
	}

    // Validate row counts across both databases
    const std::string table_name = "Shapes@0";
    std::vector<AbstractTestDbHandler*> handlers = {&mariadb_handler, &postgres_handler};
    validate_row_counts(handlers, table_name, 1);
	
	service.stop();
}

void IntegrationTester::test_sample_insertion_multiple_domains()
{
    // Create Database
    MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
    MariaDBOdbcTestHandler mariadb_handler(mariadb_config);

    const std::string cfg_file =
       "plugins/adapters/dia/resources/xml/test_dia_configuration.xml";
    const std::string service_name =
        "test_sample_insertion_two_topics_two_domains";

    Service service(
            ServiceProperty()
            .cfg_file(cfg_file)
            .service_name(service_name));
    service.start();

    //  Domain 0
    dds::domain::DomainParticipant participant_domain_0(0);
    dds::topic::Topic<ShapeExtended> topic_domain_0(
            participant_domain_0,
            "ShapesDomain0");
    dds::pub::Publisher publisher_domain_0(participant_domain_0);
    dds::pub::DataWriter<ShapeExtended> writer_domain_0(
	        publisher_domain_0,
	        topic_domain_0);

    // Domain 1
    dds::domain::DomainParticipant participant_domain_1(1);
    dds::topic::Topic<ShapeExtended> topic_domain_1(
            participant_domain_1,
            "ShapesDomain1");
	dds::pub::Publisher publisher_domain_1(participant_domain_1);
	dds::pub::DataWriter<ShapeExtended> writer_domain_1(
	        publisher_domain_1,
	        topic_domain_1);

	// Wait for discovery
    DDSTestContext test_context;
    RTI_TEST_WAIT_FOR_GREATER_OR_EQUALS(
        DDSCTestContext_getMatchingSubscriptionsLength(
            test_context,
            writer_domain_0->native_writer()),
        1,
        10);
    RTI_TEST_WAIT_FOR_GREATER_OR_EQUALS(
        DDSCTestContext_getMatchingSubscriptionsLength(
            test_context,
            writer_domain_1->native_writer()),
        1,
        10);

    auto sample_domain_0 = ShapeExtended(
        Type::SQUARE,
        Color::RED,
        10,
            Coordinates(0, 0),
        ShapeFillKind::SOLID_FILL,
        "square_domain_0");
    auto sample_domain_1 = ShapeExtended(
        Type::CIRCLE,
        Color::BLUE,
        10,
            Coordinates(0, 0),
        ShapeFillKind::SOLID_FILL,
        "circle_domain_1");

    writer_domain_0.write(sample_domain_0);
    writer_domain_1.write(sample_domain_1);

    rti::util::sleep(dds::core::Duration(1));

    std::vector<AbstractTestDbHandler*> handlers = {
        &mariadb_handler
    };

    const std::string table_domain_0 = "ShapesDomain0@0";
    const std::string table_domain_1 = "ShapesDomain1@1";

    validate_row_counts(handlers, table_domain_0, 1);
    validate_row_counts(handlers, table_domain_1, 1);

    const std::string wrong_table_domain_0 = "ShapesDomain0@1";
    const std::string wrong_table_domain_1 = "ShapesDomain1@0";

    std::vector<std::string> mariadb_tables = mariadb_handler.listTables();
    RTI_TEST_ASSERT(
	    std::find(mariadb_tables.begin(), mariadb_tables.end(), table_domain_0)
	            != mariadb_tables.end());
    RTI_TEST_ASSERT(
        std::find(mariadb_tables.begin(), mariadb_tables.end(), table_domain_1)
	            != mariadb_tables.end());
    RTI_TEST_ASSERT(
        std::find(
                mariadb_tables.begin(),
                mariadb_tables.end(),
                wrong_table_domain_0)
                == mariadb_tables.end());
    RTI_TEST_ASSERT(
        std::find(
                mariadb_tables.begin(),
                mariadb_tables.end(),
                wrong_table_domain_1)
                == mariadb_tables.end());

    service.stop();
}

void IntegrationTester::test_dispose_instance()
{
	// Create Database
	MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
	MariaDBOdbcTestHandler mariadb_handler(mariadb_config);
	
	PostgreSqlOdbcTestHandler::ConnectionConfig postgres_config;
	PostgreSqlOdbcTestHandler postgres_handler(postgres_config);

	const std::string cfg_file =
           "plugins/adapters/dia/resources/xml/test_dia_configuration.xml";
	const std::string service_name = "test_mariadb_postgre_dispose";

	Service service(
			ServiceProperty()
			.cfg_file(cfg_file)
			.service_name(service_name));
	service.start();

	dds::domain::DomainParticipant participant(0);
	dds::topic::Topic<ShapeExtended> topic(participant, "Shapes");
	dds::pub::Publisher publisher(participant);
	dds::pub::DataWriter<ShapeExtended> writer(publisher, topic);

	// Wait for discovery
	DDSTestContext test_context;
	RTI_TEST_WAIT_FOR_GREATER_OR_EQUALS(
            DDSCTestContext_getMatchingSubscriptionsLength(
                    test_context, 
                    writer->native_writer()),
            1,
            10);

	// Create data sample for writing
	auto sample_square = ShapeExtended(
            Type::SQUARE,
            Color::RED, 
            10,
            Coordinates(0, 0),
            ShapeFillKind::SOLID_FILL,
            "little_square");
    auto sample_circle = ShapeExtended(
            Type::CIRCLE,
            Color::RED,
            10,
            Coordinates(0, 0),
            ShapeFillKind::SOLID_FILL,
            "little_circle");

	for (int i=0; i<5; i++) {
		writer.write(sample_square);
		writer.write(sample_circle);
		rti::util::sleep(dds::core::Duration(1));
		writer.write(sample_circle);
		rti::util::sleep(dds::core::Duration(1));
	}

    // Validate row counts across both databases
    const std::string table_name = "Shapes@0";
    std::vector<AbstractTestDbHandler*> handlers = {&mariadb_handler, &postgres_handler};
    validate_row_counts(handlers, table_name, 2); 

    // Dispose first instance
    dds::core::InstanceHandle handle = writer.lookup_instance(sample_square);
    writer.dispose_instance(handle);

    // After disposing square instance, should have 1 row left (only circle)
    validate_row_counts(handlers, table_name, 1);

    // Dispose second instance
    handle = writer.lookup_instance(sample_circle);
    writer.dispose_instance(handle);

    // After disposing circle instance, should have 0 rows left
    validate_row_counts(handlers, table_name, 0);

    service.stop();
}

void IntegrationTester::test_no_dispose_instance()
{
	// Create Database
	MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
	MariaDBOdbcTestHandler mariadb_handler(mariadb_config);
	
	PostgreSqlOdbcTestHandler::ConnectionConfig postgres_config;
	PostgreSqlOdbcTestHandler postgres_handler(postgres_config);

	const std::string cfg_file =
           "plugins/adapters/dia/resources/xml/test_dia_configuration.xml";
	const std::string service_name = "test_mariadb_postgre_no_dispose";

	Service service(
			ServiceProperty()
			.cfg_file(cfg_file)
			.service_name(service_name));
	service.start();

	// Allow routing service to initialize
	rti::util::sleep(dds::core::Duration(1));

	dds::domain::DomainParticipant participant(0);
	dds::topic::Topic<ShapeExtended> topic(participant, "Shapes");
	dds::pub::Publisher publisher(participant);
	dds::pub::DataWriter<ShapeExtended> writer(publisher, topic);

	// Wait for discovery
	DDSTestContext test_context;
	RTI_TEST_WAIT_FOR_GREATER_OR_EQUALS(
            DDSCTestContext_getMatchingSubscriptionsLength(
                    test_context, 
                    writer->native_writer()),
            1,
            10);

	// Create data sample for writing
	auto sample_square = ShapeExtended(Type::SQUARE, Color::RED, 10,
			Coordinates(0, 0), ShapeFillKind::SOLID_FILL, "little_square");
	auto sample_circle = ShapeExtended(Type::CIRCLE, Color::RED, 10,
			Coordinates(0, 0), ShapeFillKind::SOLID_FILL, "little_square");

	for (int i=0; i<5; i++) {
		writer.write(sample_square);
		writer.write(sample_circle);
		rti::util::sleep(dds::core::Duration(1));
		writer.write(sample_circle);
		rti::util::sleep(dds::core::Duration(1));
	}

    // Validate row counts across both databases
    const std::string table_name = "Shapes@0";
    std::vector<AbstractTestDbHandler*> handlers = {&mariadb_handler, &postgres_handler};
    validate_row_counts(handlers, table_name, 2);

    // Dispose first instance 
    dds::core::InstanceHandle handle = writer.lookup_instance(sample_square);
    writer.dispose_instance(handle);

    // After disposing square instance, should have 2 rows as dispose is disabled
    validate_row_counts(handlers, table_name, 2);

    service.stop();
}

void IntegrationTester::test_instance_history()
{
	// Create Database
	MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
	MariaDBOdbcTestHandler mariadb_handler(mariadb_config);
	
	PostgreSqlOdbcTestHandler::ConnectionConfig postgres_config;
	PostgreSqlOdbcTestHandler postgres_handler(postgres_config);

	const std::string cfg_file =
           "plugins/adapters/dia/resources/xml/test_dia_configuration.xml";
	const std::string service_name = "test_instance_history";

	Service service(
			ServiceProperty()
			.cfg_file(cfg_file)
			.service_name(service_name));
	service.start();

	dds::domain::DomainParticipant participant(0);
	dds::topic::Topic<ShapeExtended> topic(participant, "Shapes");
	dds::pub::Publisher publisher(participant);
	dds::pub::DataWriter<ShapeExtended> writer(publisher, topic);

	// Wait for discovery - allow routing service to discover the DataWriter
	rti::util::sleep(dds::core::Duration(2));

	// Create data sample for writing (single instance)
	auto sample = ShapeExtended(
            Type::SQUARE,
            Color::RED, 10,
			Coordinates(0, 0),
            ShapeFillKind::SOLID_FILL,
            "little_square");

	// Write 5 samples for the same instance
	for (int i=0; i<5; i++) {
		sample.coordinates().x(i * 10);
		sample.coordinates().y(i * 10);
		writer.write(sample);
		rti::util::sleep(dds::core::Duration(0, 500000000));
	}

	// Give the routing service time to process and write samples to the database
	rti::util::sleep(dds::core::Duration(2));

    // Validate mariadb row count - should be 3 (instance_history limit)
    const std::string table_name = "Shapes@0";
    unsigned long long mariadb_row_count = mariadb_handler.getRowCount(table_name);

    RTI_TEST_ASSERT_EQUALS_INT(3, mariadb_row_count);

    // Validate PostgreSQL row count - should be 3 (instance_history limit)
    unsigned long long postgres_row_count = 
			postgres_handler.getRowCount(table_name);

    RTI_TEST_ASSERT_EQUALS_INT(3, postgres_row_count);

    // Verify that the oldest samples (0 and 10) were removed
    // and only the newest 3 samples remain (20, 30, 40)
    std::vector<int> mariadb_x_coords = mariadb_handler.coordinatesX(table_name);
    RTI_TEST_ASSERT_EQUALS_INT(3, mariadb_x_coords.size());
    RTI_TEST_ASSERT_EQUALS_INT(20, mariadb_x_coords[0]);
    RTI_TEST_ASSERT_EQUALS_INT(30, mariadb_x_coords[1]);
    RTI_TEST_ASSERT_EQUALS_INT(40, mariadb_x_coords[2]);

    std::vector<int> postgres_x_coords = postgres_handler.coordinatesX(table_name);
    RTI_TEST_ASSERT_EQUALS_INT(3, postgres_x_coords.size());
    RTI_TEST_ASSERT_EQUALS_INT(20, postgres_x_coords[0]);
    RTI_TEST_ASSERT_EQUALS_INT(30, postgres_x_coords[1]);
    RTI_TEST_ASSERT_EQUALS_INT(40, postgres_x_coords[2]);
	
	service.stop();
}

void IntegrationTester::test_instance_history_batch()
{
	// Create Database
	MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
	MariaDBOdbcTestHandler mariadb_handler(mariadb_config);
	
	PostgreSqlOdbcTestHandler::ConnectionConfig postgres_config;
	PostgreSqlOdbcTestHandler postgres_handler(postgres_config);

	const std::string cfg_file =
           "plugins/adapters/dia/resources/xml/test_dia_configuration.xml";
	const std::string service_name = "test_instance_history_batch";

	Service service(
			ServiceProperty()
			.cfg_file(cfg_file)
			.service_name(service_name));
	service.start();

	dds::domain::DomainParticipant participant(0);
	dds::topic::Topic<ShapeExtended> topic(participant, "Shapes");
	dds::pub::Publisher publisher(participant);
	dds::pub::DataWriter<ShapeExtended> writer(publisher, topic);

	// Wait for discovery - allow routing service to discover the DataWriter
	rti::util::sleep(dds::core::Duration(2));

	// Create data sample for writing (single instance)
	auto sample_square = ShapeExtended(Type::SQUARE, Color::RED, 10,
			Coordinates(0, 0), ShapeFillKind::SOLID_FILL, "little_square");
	auto sample_circle = ShapeExtended(Type::CIRCLE, Color::RED, 10,
			Coordinates(0, 0), ShapeFillKind::SOLID_FILL, "little_square");

	// Write 5 samples rapidly to ensure they're processed as a batch
	// With instance_history=2, only the newest 2 should be kept
	for (int i=0; i<5; i++) {
		sample_circle.coordinates().x(i * 10);
		sample_circle.coordinates().y(i * 10);
		writer.write(sample_circle);
		sample_square.coordinates().x(i * 20);
		sample_square.coordinates().y(i * 20);
		writer.write(sample_square);

		// Very short sleep to keep samples in same batch
		rti::util::sleep(dds::core::Duration(0, 1000000));
	}

	// Give the routing service time to process the batch
	rti::util::sleep(dds::core::Duration(2));

    // Validate row count - should be 4 total (2 instances x 2 samples each)
    const std::string table_name = "Shapes@0";
    unsigned long long mariadb_row_count = mariadb_handler.getRowCount(table_name);
    RTI_TEST_ASSERT_EQUALS_INT(4, mariadb_row_count);

    unsigned long long postgres_row_count = 
			postgres_handler.getRowCount(table_name);
    RTI_TEST_ASSERT_EQUALS_INT(4, postgres_row_count);

    // Verify that only the newest 2 samples per instance remain
    // For each instance (square and circle), the first 3 samples (0, 10, 20 on circle)
    // should have been filtered out, keeping only (30 and 40 on circle)
    std::vector<int> mariadb_x_coords = mariadb_handler.coordinatesX(table_name);
    RTI_TEST_ASSERT_EQUALS_INT(4, mariadb_x_coords.size());
    RTI_TEST_ASSERT_EQUALS_INT(30, mariadb_x_coords[0]);
    RTI_TEST_ASSERT_EQUALS_INT(40, mariadb_x_coords[1]);
    RTI_TEST_ASSERT_EQUALS_INT(60, mariadb_x_coords[2]);
    RTI_TEST_ASSERT_EQUALS_INT(80, mariadb_x_coords[3]);

    std::vector<int> postgres_x_coords = postgres_handler.coordinatesX(table_name);
    RTI_TEST_ASSERT_EQUALS_INT(4, postgres_x_coords.size());
    RTI_TEST_ASSERT_EQUALS_INT(30, postgres_x_coords[0]);
    RTI_TEST_ASSERT_EQUALS_INT(40, postgres_x_coords[1]);
    RTI_TEST_ASSERT_EQUALS_INT(60, postgres_x_coords[2]);
    RTI_TEST_ASSERT_EQUALS_INT(80, postgres_x_coords[3]);
	
	service.stop();
}

void IntegrationTester::test_instance_history_two_topics()
{
	// Create Database
	MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
	MariaDBOdbcTestHandler mariadb_handler(mariadb_config);
	
	PostgreSqlOdbcTestHandler::ConnectionConfig postgres_config;
	PostgreSqlOdbcTestHandler postgres_handler(postgres_config);

	const std::string cfg_file =
           "plugins/adapters/dia/resources/xml/test_dia_configuration.xml";
	const std::string service_name = "test_instance_history_two_topics";

	Service service(
			ServiceProperty()
			.cfg_file(cfg_file)
			.service_name(service_name));
	service.start();

	dds::domain::DomainParticipant participant(0);
	dds::topic::Topic<ShapeExtended> topic1(participant, "Topic1");
	dds::topic::Topic<ShapeExtended> topic2(participant, "Topic2");
	dds::pub::Publisher publisher(participant);
	dds::pub::DataWriter<ShapeExtended> writer1(publisher, topic1);
	dds::pub::DataWriter<ShapeExtended> writer2(publisher, topic2);

	// Wait for discovery
	rti::util::sleep(dds::core::Duration(2));

	// Create data samples for writing
	auto sample1 = ShapeExtended(
            Type::SQUARE,
            Color::RED, 10,
			Coordinates(0, 0),
            ShapeFillKind::SOLID_FILL,
            "topic1_square");

	auto sample2 = ShapeExtended(
            Type::CIRCLE,
            Color::BLUE, 10,
			Coordinates(0, 0),
            ShapeFillKind::SOLID_FILL,
            "topic2_circle");

	// Write 5 samples to Topic1 (instance_history=3)
	for (int i = 0; i < 5; i++) {
		sample1.coordinates().x(i * 10);
		sample1.coordinates().y(i * 10);
		writer1.write(sample1);
		rti::util::sleep(dds::core::Duration(0, 500000000));
	}

	// Write 5 samples to Topic2 (instance_history=2)
	for (int i = 0; i < 5; i++) {
		sample2.coordinates().x((i + 1) * 100);
		sample2.coordinates().y((i + 1) * 100);
		writer2.write(sample2);
		rti::util::sleep(dds::core::Duration(0, 500000000));
	}

	// Give the routing service time to process
	rti::util::sleep(dds::core::Duration(2));

	// Verify Topic1 has 3 samples (instance_history=3)
	const std::string table1_name = "Topic1@0";
	unsigned long long mariadb_topic1_count = mariadb_handler.getRowCount(table1_name);
	RTI_TEST_ASSERT_EQUALS_INT(3, mariadb_topic1_count);

	unsigned long long postgres_topic1_count = 
			postgres_handler.getRowCount(table1_name);
	RTI_TEST_ASSERT_EQUALS_INT(3, postgres_topic1_count);

	// Verify Topic1 contains the newest 3 samples (20, 30, 40)
	std::vector<int> mariadb_topic1_x = mariadb_handler.coordinatesX(table1_name);
	RTI_TEST_ASSERT_EQUALS_INT(3, mariadb_topic1_x.size());
	RTI_TEST_ASSERT_EQUALS_INT(20, mariadb_topic1_x[0]);
	RTI_TEST_ASSERT_EQUALS_INT(30, mariadb_topic1_x[1]);
	RTI_TEST_ASSERT_EQUALS_INT(40, mariadb_topic1_x[2]);

	std::vector<int> postgres_topic1_x = postgres_handler.coordinatesX(table1_name);
	RTI_TEST_ASSERT_EQUALS_INT(3, postgres_topic1_x.size());
	RTI_TEST_ASSERT_EQUALS_INT(20, postgres_topic1_x[0]);
	RTI_TEST_ASSERT_EQUALS_INT(30, postgres_topic1_x[1]);
	RTI_TEST_ASSERT_EQUALS_INT(40, postgres_topic1_x[2]);

	// Verify Topic2 has 2 samples (instance_history=2)
	const std::string table2_name = "Topic2@0";
	unsigned long long mariadb_topic2_count = mariadb_handler.getRowCount(table2_name);
	RTI_TEST_ASSERT_EQUALS_INT(2, mariadb_topic2_count);

	unsigned long long postgres_topic2_count = 
			postgres_handler.getRowCount(table2_name);
	RTI_TEST_ASSERT_EQUALS_INT(2, postgres_topic2_count);

	// Verify Topic2 contains the newest 2 samples (400, 500)
	std::vector<int> mariadb_topic2_x = mariadb_handler.coordinatesX(table2_name);
	RTI_TEST_ASSERT_EQUALS_INT(2, mariadb_topic2_x.size());
	RTI_TEST_ASSERT_EQUALS_INT(400, mariadb_topic2_x[0]);
	RTI_TEST_ASSERT_EQUALS_INT(500, mariadb_topic2_x[1]);

	std::vector<int> postgres_topic2_x = postgres_handler.coordinatesX(table2_name);
	RTI_TEST_ASSERT_EQUALS_INT(2, postgres_topic2_x.size());
	RTI_TEST_ASSERT_EQUALS_INT(400, postgres_topic2_x[0]);
	RTI_TEST_ASSERT_EQUALS_INT(500, postgres_topic2_x[1]);
	
	service.stop();
}

void IntegrationTester::test_instance_history_unlimited()
{
	// Create Database
	MariaDBOdbcTestHandler::ConnectionConfig mariadb_config;
	MariaDBOdbcTestHandler mariadb_handler(mariadb_config);

	PostgreSqlOdbcTestHandler::ConnectionConfig postgres_config;
	PostgreSqlOdbcTestHandler postgres_handler(postgres_config);

	const std::string cfg_file =
           "plugins/adapters/dia/resources/xml/test_dia_configuration.xml";
	const std::string service_name = "test_instance_history_unlimited";

	Service service(
			ServiceProperty()
			.cfg_file(cfg_file)
			.service_name(service_name));
	service.start();

	dds::domain::DomainParticipant participant(0);
	dds::topic::Topic<ShapeExtended> topic(participant, "Shapes");
	dds::pub::Publisher publisher(participant);
	dds::pub::DataWriter<ShapeExtended> writer(publisher, topic);

	// Wait for discovery
	rti::util::sleep(dds::core::Duration(2));

	auto sample = ShapeExtended(
            Type::SQUARE,
            Color::RED,
            10,
			Coordinates(0, 0),
            ShapeFillKind::SOLID_FILL,
            "history_unlimited_square");

	for (int i = 0; i < 10; i++) {
		sample.coordinates().x(i * 10);
		sample.coordinates().y(i * 10);
		writer.write(sample);
		rti::util::sleep(dds::core::Duration(0, 500000000));
	}

	// Give routing service time to process
	rti::util::sleep(dds::core::Duration(2));

	const std::string table_name = "Shapes@0";

	unsigned long long mariadb_row_count = mariadb_handler.getRowCount(table_name);
	RTI_TEST_ASSERT_EQUALS_INT(10, mariadb_row_count);

	std::vector<int> mariadb_x_coords = mariadb_handler.coordinatesX(table_name);
	RTI_TEST_ASSERT_EQUALS_INT(10, mariadb_x_coords.size());
	for (int i = 0; i < 10; i++) {
		RTI_TEST_ASSERT_EQUALS_INT(i * 10, mariadb_x_coords[i]);
	}

	unsigned long long postgres_row_count = postgres_handler.getRowCount(table_name);
	RTI_TEST_ASSERT_EQUALS_INT(10, postgres_row_count);

	std::vector<int> postgres_x_coords = postgres_handler.coordinatesX(table_name);
	RTI_TEST_ASSERT_EQUALS_INT(10, postgres_x_coords.size());
	for (int i = 0; i < 10; i++) {
		RTI_TEST_ASSERT_EQUALS_INT(i * 10, postgres_x_coords[i]);
	}

	service.stop();
}

}}}} // rti::adapter::dis::test


