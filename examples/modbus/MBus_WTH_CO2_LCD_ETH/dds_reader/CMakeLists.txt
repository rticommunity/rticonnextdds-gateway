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

include(${RESOURCE_DIR}/cmake/dds_functions.cmake)
include(${RESOURCE_DIR}/cmake/common_functions.cmake)

# Copy the USER_QOS_PROFILES before generating the code
# List of the files to copy
set(CO2_LCD_ETH_EXAMPLE_READER_QOS_FILE
    "USER_QOS_PROFILES.xml"
)

copy_files(
    CUSTOM_TARGET_NAME CO2_LCD_ETH_EXAMPLE_READER
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    LIST_OF_FILES_RELATIVE_PATH ${CO2_LCD_ETH_EXAMPLE_READER_QOS_FILE})

set(IDL_NAME "MBus_WTH_CO2_LCD_ETH_INPUT")

# Generate codegen files
connextdds_run_codegen(
    IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../idl/${IDL_NAME}.idl"
    LANGUAGE "C"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(MBus_WTH_CO2_LCD_ETH_INPUT
    "${CMAKE_CURRENT_SOURCE_DIR}/${IDL_NAME}_subscriber.c"
    ${GENERATED_SRC_FILES}
)

target_include_directories(MBus_WTH_CO2_LCD_ETH_INPUT
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(MBus_WTH_CO2_LCD_ETH_INPUT
    PRIVATE
        RTIConnextDDS::c_api
)

# add the dependency of the copying step to the current target
add_dependencies(MBus_WTH_CO2_LCD_ETH_INPUT
    CO2_LCD_ETH_EXAMPLE_READER)

# staging files
install(
    TARGETS MBus_WTH_CO2_LCD_ETH_INPUT
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    PERMISSIONS OWNER_EXECUTE
)
