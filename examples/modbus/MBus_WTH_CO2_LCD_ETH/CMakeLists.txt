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

set(EXAMPLE_NAME MBus_WTH_CO2_LCD_ETH)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dds_writer")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dds_reader")

# Copy the USER_QOS_PROFILES before generating the code
# List of the files to copy
# This is not needed since we could install that file directly. However, for
# consistency between the 'build' and 'install' directories, this is copied
# to the binary directory so codegen doesn't generate 'USER_QOS_PROFILES.xml' in
# the binary directory.

rtigw_copy_files(
    CUSTOM_TARGET_NAME co2-lcd-eth-example-config
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    LIST_OF_FILES_RELATIVE_PATH "USER_QOS_PROFILES.xml")

# add the dependency of the copying step to the current target
add_dependencies(MBus_WTH_CO2_LCD_ETH_INPUT co2-lcd-eth-example-config)

# staging files
# Config files

set(CO2_LCD_ETH_EXAMPLE_INSTALL_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/USER_QOS_PROFILES.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/RSConfigAdapter.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/MBus_WTH_CO2_LCD_ETH_input_config.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/MBus_WTH_CO2_LCD_ETH_output_config.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/MBus_xTH_CO2_LCD_ETH_datasheet.pdf"
)

install(
    FILES ${CO2_LCD_ETH_EXAMPLE_INSTALL_FILES}
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/idl"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)
