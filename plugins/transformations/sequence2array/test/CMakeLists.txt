###############################################################################
#  (c) 2022 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. #
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
    IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/idl/tsfm_seq2array_test_message.idl
    LANG "C++11"
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/idl
    EXTRA_ARGS
        -unboundedSupport
        ${RTIDDSGEN_CXX11_LEGACY_API_ARGS}
)

set(COMMON_DEFS)

if (RTICONNEXTDDS_VERSION VERSION_LESS "7.6.0")
    list(APPEND COMMON_DEFS -DRTICONNEXTDDS_HAS_LEGACY_UNIONS=1)
endif()
if (RTICONNEXTDDS_HAS_OSTREAM_OPERATORS)
    list(APPEND COMMON_DEFS -DRTICONNEXTDDS_HAS_OSTREAM_OPERATORS=1)
endif()

###############################################################################
add_library(tsfm_seq2array_test_message STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/idl/tsfm_seq2array_test_message.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/tsfm_seq2array_test_message.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/idl/tsfm_seq2array_test_messagePlugin.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/tsfm_seq2array_test_messagePlugin.cxx
)

target_include_directories(tsfm_seq2array_test_message PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/idl
)
target_link_libraries(tsfm_seq2array_test_message
    PUBLIC
        RTIConnextDDS::cpp2_api
)
target_compile_definitions(tsfm_seq2array_test_message
    PUBLIC
        ${COMMON_DEFS}
)
###############################################################################
add_executable(tsfm_seq2array_inttest1
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.cxx
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.hpp
    srcCxx/Seq2ArrayTester.hpp
    srcCxx/Seq2ArrayIntegrationTest1.cxx
)
target_include_directories(tsfm_seq2array_inttest1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
    ${TEST_COMMON_DIR}/srcCxx
)
target_link_libraries(tsfm_seq2array_inttest1
    PRIVATE
        tsfm_seq2array_test_message
)

###############################################################################
add_executable(tsfm_seq2array_inttest2
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.cxx
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.hpp
    srcCxx/Seq2ArrayTester.hpp
    srcCxx/Seq2ArrayIntegrationTest2.cxx
)
target_include_directories(tsfm_seq2array_inttest2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
    ${TEST_COMMON_DIR}/srcCxx
)
target_link_libraries(tsfm_seq2array_inttest2
    PRIVATE
        tsfm_seq2array_test_message
)

###############################################################################
add_executable(tsfm_seq2array_inttest3
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.cxx
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.hpp
    srcCxx/Seq2ArrayTester.hpp
    srcCxx/Seq2ArrayIntegrationTest3.cxx
)
target_include_directories(tsfm_seq2array_inttest3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
    ${TEST_COMMON_DIR}/srcCxx
)
target_link_libraries(tsfm_seq2array_inttest3
    PRIVATE
        tsfm_seq2array_test_message
)

###############################################################################
add_executable(tsfm_seq2array_inttest4
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.cxx
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.hpp
    srcCxx/Seq2ArrayTester.hpp
    srcCxx/Seq2ArrayIntegrationTest4.cxx
)
target_include_directories(tsfm_seq2array_inttest4 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
    ${TEST_COMMON_DIR}/srcCxx
)
target_link_libraries(tsfm_seq2array_inttest4
    PRIVATE
        tsfm_seq2array_test_message
)

###############################################################################
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_test_qos.xml
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_seq2array_test_qos.xml
        ${CMAKE_CURRENT_BINARY_DIR}/
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_seq2array_test_qos.xml
)

rtigw_generate_test_routing_service_config(
    ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_seq2array_inttest1.xml
    ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_inttest1.xml
)

rtigw_generate_test_routing_service_config(
    ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_seq2array_inttest2.xml
    ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_inttest2.xml
)

rtigw_generate_test_routing_service_config(
    ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_seq2array_inttest3.xml
    ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_inttest3.xml
)

rtigw_generate_test_routing_service_config(
    ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_seq2array_inttest4.xml
    ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_inttest4.xml
)

add_custom_target(
    tsfm_seq2array_test_files ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_test_qos.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_inttest1.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_inttest2.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_inttest3.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_seq2array_inttest4.xml
)

###############################################################################
rtigw_register_test(
    tsfm_seq2array_inttest1
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_test.py -n 1
)
rtigw_register_test(
    tsfm_seq2array_inttest2
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_test.py -n 2
)
rtigw_register_test(
    tsfm_seq2array_inttest3
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_test.py -n 3
)
rtigw_register_test(
    tsfm_seq2array_inttest4
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_test.py -n 4
)
