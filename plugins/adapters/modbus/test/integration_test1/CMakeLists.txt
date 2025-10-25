###############################################################################
#  (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. #
#                                                                             #
#  RTI grants Licensee a license to use, modify, compile, and create          #
#  derivative works of the software solely for use with RTI Connext DDS.      #
#  Licensee may redistribute copies of the software provided that all such    #
#  copies are subject to this license.                                        #
#  The software is provided "as is", with no warranty of any type, including  #
#  any warranty for fitness for any purpose. RTI is under no obligation to    #
#  maintain or support the software.  RTI shall not be liable for any         #
#  incidental or consequential damages arising out of the use or inability to #
#  use the software.                                                          #
#                                                                             #
###############################################################################

include(ConnextDdsGenerateFiles)
include(ConnextDdsCodegen)

set(TEST_NAME modbus_integration_test1)

set(IDL_NAME "TestType")
set(IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dds_type")

# Generate codegen files
connextdds_rtiddsgen_run(
    IDL_FILE "${IDL_DIR}/${IDL_NAME}.idl"
    LANG "C"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dds_type"
)

add_executable(${TEST_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/dds_type/TestType_main.c"
    ${${IDL_NAME}_C_SOURCES}
)

target_include_directories(${TEST_NAME}
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/dds_type"
)

target_link_libraries(${TEST_NAME}
    RTIConnextDDS::c_api
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/test_pub_config.json
        ${CMAKE_CURRENT_BINARY_DIR}/test_sub_config.json
        ${CMAKE_CURRENT_BINARY_DIR}/USER_QOS_PROFILES.xml
    COMMAND
        ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/config/test_pub_config.json
            ${CMAKE_CURRENT_SOURCE_DIR}/config/test_sub_config.json
            ${CMAKE_CURRENT_SOURCE_DIR}/dds_type/USER_QOS_PROFILES.xml
            ${CMAKE_CURRENT_BINARY_DIR}/
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/config/test_pub_config.json
        ${CMAKE_CURRENT_SOURCE_DIR}/config/test_sub_config.json
        ${CMAKE_CURRENT_SOURCE_DIR}/dds_type/USER_QOS_PROFILES.xml
)

rtigw_generate_test_routing_service_config(
    ${CMAKE_CURRENT_SOURCE_DIR}/RSConfigTest.xml
    ${CMAKE_CURRENT_BINARY_DIR}/RSConfigTest.xml
)

add_custom_target(
    modbus_adapter_test1_files ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/test_pub_config.json
        ${CMAKE_CURRENT_BINARY_DIR}/test_sub_config.json
        ${CMAKE_CURRENT_BINARY_DIR}/RSConfigTest.xml
        ${CMAKE_CURRENT_BINARY_DIR}/USER_QOS_PROFILES.xml
)

###############################################################################
rtigw_register_test(
    adapt_modbus_inttest1
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_test.py
)