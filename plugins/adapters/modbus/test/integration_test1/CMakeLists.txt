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

include("${RESOURCE_DIR}/cmake/CommonFunctions.cmake")
include("${RESOURCE_DIR}/cmake/DdsFunctions.cmake")

set(TEST_NAME integration_test1)

# Copy the USER_QOS_PROFILES before generating the code
# This is not neede since we could install that file directly. However, for
# consistency between the 'build' and 'install' directories, this is copied
# to the binary directory so codegen doesn't generate this file in the binary
# directory.
copy_files(
    CUSTOM_TARGET_NAME integration-test1-copy-qos
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/dds_type"
    LIST_OF_FILES_RELATIVE_PATH "dds_type/USER_QOS_PROFILES.xml")


set(IDL_NAME "TestType")

# Generate codegen files
connextdds_run_codegen(
    IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dds_type/${IDL_NAME}.idl"
    LANGUAGE "C"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/dds_type"
)

add_executable(${TEST_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/dds_type/${IDL_NAME}_main.c"
    ${GENERATED_SRC_FILES}
)

target_include_directories(${TEST_NAME}
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/dds_type"
)

target_link_libraries(${TEST_NAME}
    RTIConnextDDS::c_api
)

# add the dependency of the copying step to the current target
add_dependencies(${TEST_NAME} integration-test1-copy-qos)

# staging rules
# Config files

set(CO2_LCD_ETH_EXAMPLE_INSTALL_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/config/test_pub_config.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/config/test_sub_config.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/RSConfigTest.xml"
        "${CMAKE_CURRENT_BINARY_DIR}/dds_type/USER_QOS_PROFILES.xml"
)

install(
    TARGETS ${TEST_NAME}
    DESTINATION "${STAGING_TEST_DIR}/${STAGING_PLUGIN_DIR}/${TEST_NAME}"
    PERMISSIONS OWNER_EXECUTE
)

install(
    FILES ${CO2_LCD_ETH_EXAMPLE_INSTALL_FILES}
    DESTINATION "${STAGING_TEST_DIR}/${STAGING_PLUGIN_DIR}/${TEST_NAME}"
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/dds_type/TestType.idl"
    DESTINATION "${STAGING_TEST_DIR}/${STAGING_PLUGIN_DIR}/${TEST_NAME}/idl"
)
