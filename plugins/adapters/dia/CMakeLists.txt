#
# (c) 2023 Copyright, Real-Time Innovations, Inc.  All rights reserved.
#
#  RTI grants Licensee a license to use, modify, compile, and create derivative
#  works of the Software.  Licensee has the right to distribute object form
#  only for use with RTI products.  The Software is provided "as is", with no
#  warranty of any type, including any warranty for fitness for any purpose.
#  RTI is under no obligation to maintain or support the Software.  RTI shall
#  not be liable for any incidental or consequential damages arising out of the
#  use or inability to use the software.
#
cmake_minimum_required(VERSION 3.11)
project(DIA)

###############################################################################
# Binary
###############################################################################
# Target name
set(RSPLUGIN_LIB_NAME "rtiDatabaseIntegrationAdapter")

add_library(${RSPLUGIN_LIB_NAME}
    SHARED
	"${JSON_PARSER_DIR}/json.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/DiaAdapter.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/DiaConnection.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/DiaStreamWriter.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/SqlTable.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/TableFields.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/Utils.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/Queries.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/QueryConverter.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/odbc/OdbcEnvironmentManager.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/odbc/OdbcConnectionManager.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/odbc/OdbcStatementManager.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/odbc/OdbcStruct.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/odbc/MetadataFacade.cxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/odbc/SampledataFacade.cxx"
)

target_include_directories(${RSPLUGIN_LIB_NAME}
    PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx"
	"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/odbc"
	"${JSON_PARSER_WRAPPER_DIR}/srcCxx"
	"${JSON_PARSER_DIR}/"
)

target_link_libraries(${RSPLUGIN_LIB_NAME}
    PUBLIC
	RTIConnextDDS::routing_service_infrastructure
	RTIConnextDDS::cpp2_api
	odbc
)

install(
    TARGETS ${RSPLUGIN_LIB_NAME}
    DESTINATION "${STAGING_LIB_DIR}"
)


# Set properties
set_property(TARGET ${RSPLUGIN_LIB_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${RSPLUGIN_LIB_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_target_properties(${RSPLUGIN_LIB_NAME}
    PROPERTIES
        DEBUG_POSTFIX "d"
)

###############################################################################
# Tester
###############################################################################
# Test target name
set(RSPLUGIN_TESTER_NAME "rtiDatabaseIntegrationTester")

# IDL files
include(ConnextDdsGenerateFiles)
include(ConnextDdsCodegen)

set(ADAPTER_DIA_TEST_IDL DiaTestTypes)
set(IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/idl")
connextdds_rtiddsgen_run(
	IDL_FILE "${IDL_DIR}/${ADAPTER_DIA_TEST_IDL}.idl"
	LANG C++11
	OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/idl"
)

add_custom_target(${ADAPTER_DIA_TEST_IDL}
    DEPENDS
	${${ADAPTER_DIA_TEST_IDL}_CXX11_SOURCES}
)

# Build tests for the adapter
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

add_executable(${RSPLUGIN_TESTER_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test/Tester.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test/SqlTableTester.cxx"
		"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test/AdapterTester.cxx"
		"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test/OdbcConnectionManagerTester.cxx"
		"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test/IntegrationTester.cxx"
		"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test/MariaDBOdbcTestHandler.cxx"
		"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test/PostgreSqlOdbcTestHandler.cxx"
		"${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test/TestUtils.cxx"
		"${${ADAPTER_DIA_TEST_IDL}_CXX11_SOURCES}"
)

target_include_directories(${RSPLUGIN_TESTER_NAME}
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/odbc"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/test"
    "${CMAKE_CURRENT_BINARY_DIR}/idl"
)

add_dependencies(${RSPLUGIN_TESTER_NAME}
	${ADAPTER_DIA_TEST_IDL}
)

target_link_libraries(${RSPLUGIN_TESTER_NAME}
    PUBLIC
    ${RSPLUGIN_LIB_NAME}
    RTIConnextDDS::routing_service_infrastructure
    RTIConnextDDS::cpp2_api
    RTIConnextDDS::routing_service
    RTIConnextDDS::test
    nddsctesthelpers
    odbc
)

add_custom_target(${PROJECT_NAME}_codegen_sources
    DEPENDS
    ${DiaTestTypes_CXX11_GENERATED_SOURCES}
)

###############################################################################
# Build tests and documentation if they are enabled
###############################################################################
rtigw_add_doc()
rtigw_add_tests()
