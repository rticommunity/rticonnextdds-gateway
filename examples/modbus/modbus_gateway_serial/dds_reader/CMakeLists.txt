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

include(ConnextDdsCodegen)

set(IDL_NAME "modbus_serial_device")
set(IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../idl")

# Generate codegen files
connextdds_rtiddsgen_run(
    IDL_FILE "${IDL_DIR}/${IDL_NAME}.idl"
    LANG "C"
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(modbus_serial_device
    "${CMAKE_CURRENT_SOURCE_DIR}/${IDL_NAME}_subscriber.c"
    ${${IDL_NAME}_C_SOURCES}
)

target_include_directories(modbus_serial_device
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(modbus_serial_device
    PRIVATE
        RTIConnextDDS::c_api
)

# staging files
install(
    TARGETS modbus_serial_device
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    PERMISSIONS OWNER_EXECUTE
)
