###############################################################################
#  (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. #
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

###############################################################################
include(ConnextDdsCodegen)

connextdds_rtiddsgen_run(
    IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/idl/tsfm_json_test_message.idl
    LANG "C++11"
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/idl
    EXTRA_ARGS
        -unboundedSupport
        ${RTIDDSGEN_CXX11_LEGACY_API_ARGS}
)

###############################################################################
add_library(tsfm_json_test_message STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/idl/tsfm_json_test_message.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/tsfm_json_test_message.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/idl/tsfm_json_test_messagePlugin.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/tsfm_json_test_messagePlugin.cxx
)
target_include_directories(tsfm_json_test_message PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/idl
)
target_link_libraries(tsfm_json_test_message
    PUBLIC
        RTIConnextDDS::cpp2_api
)

###############################################################################

add_executable(tsfm_json_tester
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.cxx
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.hpp
    srcCxx/JsonMessageTester.cxx
    srcCxx/JsonMessageTester.hpp
)
target_include_directories(tsfm_json_tester PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
    ${TEST_COMMON_DIR}/srcCxx
)
target_link_libraries(tsfm_json_tester
    PRIVATE
        tsfm_json_test_message
)

###############################################################################

add_executable(tsfm_json_repeater
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.cxx
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.hpp
    ${TEST_COMMON_DIR}/srcCxx/TestMessageRepeaterMain.cxx
    ${TEST_COMMON_DIR}/srcCxx/TestMessageRepeater.cxx
    ${TEST_COMMON_DIR}/srcCxx/TestMessageRepeater.hpp
)
target_include_directories(tsfm_json_repeater PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
    ${TEST_COMMON_DIR}/srcCxx
)
target_link_libraries(tsfm_json_repeater
    PRIVATE
        tsfm_json_test_message
)

###############################################################################
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_json_test_message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_json_test_qos.xml
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_json_test_message.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_json_test_qos.xml
        ${CMAKE_CURRENT_BINARY_DIR}/
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_json_test_message.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_json_test_qos.xml
)

rtigw_generate_test_routing_service_config(
    ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_json_test.xml
    ${CMAKE_CURRENT_BINARY_DIR}/tsfm_json_test.xml
)

add_custom_target(
    tsfm_json_test_files ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_json_test.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_json_test_message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_json_test_qos.xml
)

###############################################################################
rtigw_register_test(
    tsfm_json
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_test.py
)
