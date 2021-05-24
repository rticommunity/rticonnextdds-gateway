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

set(RSPLUGIN_LIB_NAME "rtimodbusadapter")

set(CMAKE_DEBUG_POSTFIX "d")

add_library(${RSPLUGIN_LIB_NAME}
    SHARED
        "${JSON_PARSER_DIR}/json.c"
        "${DDS_COMMON_DIR}/srcCxx/DynamicDataFunc.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/ModbusAdapter.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/ModbusAdapterConfiguration.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/ModbusConnection.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/ModbusStreamWriter.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/ModbusStreamReader.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/LibModbusClient.cxx"
)

target_include_directories(${RSPLUGIN_LIB_NAME}
    PRIVATE
        ${CONNEXTDDS_INCLUDE_DIRS}
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx"
        "${LIBMODBUS_DIR}/src"
        "${UTILS_COMMON_DIR}/srcCxx"
        "${DDS_COMMON_DIR}/srcCxx"
        "${JSON_PARSER_WRAPPER_DIR}/srcCxx"
        "${JSON_PARSER_DIR}/"
)

target_link_libraries(${RSPLUGIN_LIB_NAME}
    PRIVATE
        ${ROUTING_SERVICE_CXX_LIBRARIES}
        modbus
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/utilities")

# Stagging targets
install(
    TARGETS ${RSPLUGIN_LIB_NAME}
    DESTINATION "${STAGING_LIB_DIR}"
)

###############################################################################
# Build tests and documentation if they are enabled
###############################################################################
if (RTIGATEWAY_ENABLE_DOCS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/doc")
endif()

if (RTIGATEWAY_ENABLE_TESTS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")
endif()
