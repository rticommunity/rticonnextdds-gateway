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
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/proto
    COMMAND 
        ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/proto
    COMMENT "Creating proto output directory"
)

###############################################################################
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/proto/message.pb.h
           ${CMAKE_CURRENT_BINARY_DIR}/proto/message.pb.cc
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/proto
         -I ${PROTOBUF2DDS_COMMON_DIR}/test/proto
         ${PROTOBUF2DDS_COMMON_DIR}/test/proto/message.proto
    DEPENDS
        ${PROTOBUF2DDS_COMMON_DIR}/test/proto/message.proto
        ${CMAKE_CURRENT_BINARY_DIR}/proto
    COMMENT "Generating Protobuf Descriptor Set from message.proto"
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/proto/another_message.pb.h
           ${CMAKE_CURRENT_BINARY_DIR}/proto/another_message.pb.cc
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/proto
         -I ${PROTOBUF2DDS_COMMON_DIR}/test/proto
         ${PROTOBUF2DDS_COMMON_DIR}/test/proto/another_message.proto
    DEPENDS
        ${PROTOBUF2DDS_COMMON_DIR}/test/proto/another_message.proto
        ${CMAKE_CURRENT_BINARY_DIR}/proto
    COMMENT "Generating Protobuf Descriptor Set from another_message.proto"
)

###############################################################################
include(ConnextDdsCodegen)

connextdds_rtiddsgen_run(
    IDL_FILE ${PROTOBUF2DDS_COMMON_DIR}/test/idl/serialized_message.idl
    LANG "C++11"
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/idl
    EXTRA_ARGS
        "-unboundedSupport"
        ${RTIDDSGEN_CXX11_LEGACY_API_ARGS}
)

connextdds_rtiddsgen_run(
    IDL_FILE ${PROTOBUF2DDS_COMMON_DIR}/test/idl/message.idl
    LANG "C++11"
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/idl
    EXTRA_ARGS
        "-unboundedSupport"
        ${RTIDDSGEN_CXX11_LEGACY_API_ARGS}
)

connextdds_rtiddsgen_run(
    IDL_FILE ${PROTOBUF2DDS_COMMON_DIR}/test/idl/another_message.idl
    LANG "C++11"
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/idl
    EXTRA_ARGS
        "-unboundedSupport"
        ${RTIDDSGEN_CXX11_LEGACY_API_ARGS}
)

###############################################################################
add_library(tsfm_protobuf_test_message STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/idl/message.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/message.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/idl/messagePlugin.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/messagePlugin.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/idl/another_message.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/another_message.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/idl/another_messagePlugin.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/another_messagePlugin.cxx
)
target_include_directories(tsfm_protobuf_test_message PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/idl
)
target_link_libraries(tsfm_protobuf_test_message
    PUBLIC
        RTIConnextDDS::cpp2_api
)

###############################################################################
add_library(tsfm_protobuf_test_serialized_message STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/idl/serialized_message.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/serialized_message.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/idl/serialized_messagePlugin.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/idl/serialized_messagePlugin.cxx
)
target_include_directories(tsfm_protobuf_test_serialized_message PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/idl
)
target_link_libraries(tsfm_protobuf_test_serialized_message
    PUBLIC
        RTIConnextDDS::cpp2_api
)

###############################################################################
add_library(tsfm_protobuf_test_message_pb STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/proto/message.pb.h
    ${CMAKE_CURRENT_BINARY_DIR}/proto/message.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/proto/another_message.pb.h
    ${CMAKE_CURRENT_BINARY_DIR}/proto/another_message.pb.cc
)
target_include_directories(tsfm_protobuf_test_message_pb PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/proto
)
target_link_libraries(tsfm_protobuf_test_message_pb
    PUBLIC
        ${Protobuf_LIBRARIES}
)

###############################################################################

add_executable(tsfm_protobuf_tester
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.cxx
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.hpp
    srcCxx/ProtobufMessageTester.cxx
    srcCxx/ProtobufMessageTester.hpp
    ${PROTOBUF2DDS_COMMON_DIR}/test/srcCxx/ProtobufMessageUtils.cxx
    ${PROTOBUF2DDS_COMMON_DIR}/test/srcCxx/ProtobufMessageUtils.hpp
)
target_include_directories(tsfm_protobuf_tester PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
    ${PROTOBUF2DDS_COMMON_DIR}/test/srcCxx
    ${TEST_COMMON_DIR}/srcCxx
)
target_link_libraries(tsfm_protobuf_tester
    PRIVATE
        tsfm_protobuf_test_message_pb
        tsfm_protobuf_test_serialized_message
)

###############################################################################

add_executable(tsfm_protobuf_repeater
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.cxx
    ${TEST_COMMON_DIR}/srcCxx/BaseTester.hpp
    ${TEST_COMMON_DIR}/srcCxx/TestMessageRepeaterMain.cxx
    ${TEST_COMMON_DIR}/srcCxx/TestMessageRepeater.cxx
    ${TEST_COMMON_DIR}/srcCxx/TestMessageRepeater.hpp
)
target_include_directories(tsfm_protobuf_repeater PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
    ${TEST_COMMON_DIR}/srcCxx
)
target_link_libraries(tsfm_protobuf_repeater
    PRIVATE
        tsfm_protobuf_test_message
)

###############################################################################
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/message.pbdesc
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --descriptor_set_out=${CMAKE_CURRENT_BINARY_DIR}/message.pbdesc
         --include_imports
         -I ${PROTOBUF2DDS_COMMON_DIR}/test/proto
         ${PROTOBUF2DDS_COMMON_DIR}/test/proto/message.proto
    DEPENDS
        ${PROTOBUF2DDS_COMMON_DIR}/test/proto/message.proto
        ${PROTOBUF2DDS_COMMON_DIR}/test/proto/another_message.proto
    COMMENT "Generating Protobuf C++ source files from message.proto"
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_protobuf_test_qos.xml
        ${CMAKE_CURRENT_BINARY_DIR}/message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/another_message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/serialized_message.xml
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_protobuf_test_qos.xml
        ${PROTOBUF2DDS_COMMON_DIR}/test/xml/message.xml
        ${PROTOBUF2DDS_COMMON_DIR}/test/xml/another_message.xml
        ${PROTOBUF2DDS_COMMON_DIR}/test/xml/serialized_message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_protobuf_test_qos.xml
        ${PROTOBUF2DDS_COMMON_DIR}/test/xml/message.xml
        ${PROTOBUF2DDS_COMMON_DIR}/test/xml/another_message.xml
        ${PROTOBUF2DDS_COMMON_DIR}/test/xml/serialized_message.xml
)

rtigw_generate_test_routing_service_config(
    ${CMAKE_CURRENT_SOURCE_DIR}/xml/tsfm_protobuf_test.xml
    ${CMAKE_CURRENT_BINARY_DIR}/tsfm_protobuf_test.xml
)

add_custom_target(
    tsfm_protobuf_test_files ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_protobuf_test.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tsfm_protobuf_test_qos.xml
        ${CMAKE_CURRENT_BINARY_DIR}/message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/another_message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/serialized_message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/message.pbdesc
)

###############################################################################
rtigw_register_test(
    tsfm_protobuf
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_test.py
)
