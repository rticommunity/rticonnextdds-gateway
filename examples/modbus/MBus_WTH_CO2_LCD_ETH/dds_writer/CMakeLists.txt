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

include("${RESOURCE_DIR}/cmake/DdsFunctions.cmake")
include("${RESOURCE_DIR}/cmake/CommonFunctions.cmake")

set(IDL_NAME "MBus_WTH_CO2_LCD_ETH_WRITE")

# Generate codegen files
rtigw_connextdds_run_codegen(
    IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../idl/${IDL_NAME}.idl"
    LANGUAGE "C"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(MBus_WTH_CO2_LCD_ETH_WRITE
    "${CMAKE_CURRENT_SOURCE_DIR}/${IDL_NAME}_publisher.c"
    ${GENERATED_SRC_FILES}
)

target_include_directories(MBus_WTH_CO2_LCD_ETH_WRITE
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(MBus_WTH_CO2_LCD_ETH_WRITE
    PRIVATE
        RTIConnextDDS::c_api
)

# staging files
install(
    TARGETS MBus_WTH_CO2_LCD_ETH_WRITE
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    PERMISSIONS OWNER_EXECUTE
)
