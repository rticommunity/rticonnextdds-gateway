###############################################################################
#  (c) 2021 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. #
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

set(EXAMPLE_NAME modbus_serial_device)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dds_reader")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dds_writer")

# Copy the USER_QOS_PROFILES before generating the code
# List of the files to copy
# This is not needed since we could install that file directly. However, for
# consistency between the 'build' and 'install' directories, this is copied
# to the binary directory so codegen doesn't generate 'USER_QOS_PROFILES.xml' in
# the binary directory.

connextdds_copy_files(
    INPUT_FILES
        "USER_QOS_PROFILES.xml"
    OUTPUT_DIR
        "${CMAKE_CURRENT_BINARY_DIR}"
)

# staging files
# Config files

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/USER_QOS_PROFILES.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/RSConfigAdapter.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/modbus_serial_device_config.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/temperature_correction_config.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION
        "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/idl"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)