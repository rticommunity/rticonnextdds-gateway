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

set(TEST_NAME seq2array_integration_test1)

# Publisher
set(IDL_NAME "testPublisher")
set(IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/publisher")

# Generate codegen files
connextdds_rtiddsgen_run(
    IDL_FILE "${IDL_DIR}/${IDL_NAME}.idl"
    LANG "C++11"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/publisher"
)

add_executable(${TEST_NAME}_publisher
    "${CMAKE_CURRENT_SOURCE_DIR}/publisher/test_publisher.cxx"
    "${${IDL_NAME}_CXX11_SOURCES}"
)

target_include_directories(${TEST_NAME}_publisher
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/publisher"
)

target_link_libraries(${TEST_NAME}_publisher
    RTIConnextDDS::cpp2_api
)

# Subscriber
set(IDL_NAME "testSubscriber")
set(IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/subscriber")

# Generate codegen files
connextdds_rtiddsgen_run(
    IDL_FILE "${IDL_DIR}/${IDL_NAME}.idl"
    LANG "C++11"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/subscriber"
)

add_executable(${TEST_NAME}_subscriber
    "${CMAKE_CURRENT_SOURCE_DIR}/subscriber/test_subscriber.cxx"
    "${${IDL_NAME}_CXX11_SOURCES}"
)

target_include_directories(${TEST_NAME}_subscriber
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/subscriber"
)

target_link_libraries(${TEST_NAME}_subscriber
    RTIConnextDDS::cpp2_api
)


# staging rules
# Config files

set(SEQ2ARRAY_TRANSFORMATION_TEST_INSTALL_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/RsSeq2ArrayTransfTest.xml"
)

install(
    TARGETS ${TEST_NAME}_publisher
    DESTINATION "${STAGING_TEST_DIR}/${STAGING_PLUGIN_DIR}/${TEST_NAME}/publisher"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE
)

install(
    TARGETS ${TEST_NAME}_subscriber
    DESTINATION "${STAGING_TEST_DIR}/${STAGING_PLUGIN_DIR}/${TEST_NAME}/subscriber"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE
)

install(
    FILES ${SEQ2ARRAY_TRANSFORMATION_TEST_INSTALL_FILES}
    DESTINATION "${STAGING_TEST_DIR}/${STAGING_PLUGIN_DIR}/${TEST_NAME}"
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/publisher/testPublisher.idl"
    DESTINATION "${STAGING_TEST_DIR}/${STAGING_PLUGIN_DIR}/${TEST_NAME}/publisher"
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/subscriber/testSubscriber.idl"
    DESTINATION "${STAGING_TEST_DIR}/${STAGING_PLUGIN_DIR}/${TEST_NAME}/subscriber"
)