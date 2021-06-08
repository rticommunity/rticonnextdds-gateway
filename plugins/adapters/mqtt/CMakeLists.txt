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

###############################################################################
# CMake project declarations
###############################################################################
cmake_minimum_required(VERSION 3.7.0)

include(${RESOURCE_DIR}/cmake/dds_functions.cmake)

###############################################################################
# Set variables
set(RSPLUGIN_LIB_NAME rtimqttadapter)

option(RTI_MQTT_ENABLE_STATIC_TYPES "Enable support for statically typed messages" OFF)

if(RTIGATEWAY_ENABLE_SSL)
    # Paho with SSL/TLS support
    set(PAHO_LIBRARY "paho-mqtt3as")
else()
    set(PAHO_LIBRARY "paho-mqtt3a")
endif()

###############################################################################
# Run codegen
###############################################################################
set(ADAPTER_MQTT_IDL
    rtiadapt_mqtt_types
    rtiadapt_mqtt_types_message
    rtiadapt_mqtt_types_client
    rtiadapt_mqtt_types_adapter
)

set(GENERATED_SRC_IDL_FILES)
foreach(IDL_NAME ${ADAPTER_MQTT_IDL})
    # Generate codegen files
    set(CODEGEN_OPTIONS "-unboundedSupport" "-qualifiedEnumerator")
    connextdds_run_codegen(
        IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/idl/${IDL_NAME}.idl"
        LANGUAGE "C"
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/idl"
        INPUT_PARAMS ${CODEGEN_OPTIONS}
    )
    list(APPEND GENERATED_SRC_IDL_FILES ${GENERATED_SRC_FILES})
endforeach()


###############################################################################
# Create library
###############################################################################

# This variable is used in the tests folder, that's why the source files are not
# directly set in the 'add_library'
set(RTI_MQTT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/mqtt/Client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/mqtt/ClientApiPaho.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/mqtt/Subscription.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/mqtt/Publication.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/mqtt/Message.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/mqtt/Infrastructure.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/adapter/Plugin.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/adapter/BrokerConnection.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/adapter/MessageReader.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/adapter/MessageWriter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/adapter/Properties.c"
)

add_library(${RSPLUGIN_LIB_NAME}
    SHARED
        ${GENERATED_SRC_IDL_FILES}
        ${RTI_MQTT_SOURCES}
)

set_target_properties(${RSPLUGIN_LIB_NAME} PROPERTIES DEBUG_POSTFIX "d")

# As this target can be used as a library as well (including functions from the
# generated code), we need to export those functions. In order to do that, we
# explort all symbols on Windows.
if(WIN32)
    set_target_properties(${RSPLUGIN_LIB_NAME}
        PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

# This variable is used in the tests folder, that's why the include dirs are not
# directly set in the 'target_include_directories'
set(RTI_MQTT_INCLUDES
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/srcC/adapter>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/srcC/mqtt>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/rti>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/idl>"
    "$<BUILD_INTERFACE:${PAHO_MQTT_C_DIR}/src>"
    "$<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/rti>"
)

target_include_directories(${RSPLUGIN_LIB_NAME}
    PUBLIC
        ${CONNEXTDDS_INCLUDE_DIRS}
        ${RTI_MQTT_INCLUDES}
)

target_link_libraries(${RSPLUGIN_LIB_NAME}
    PUBLIC
        ${ROUTING_SERVICE_C_LIBRARIES}
        ${PAHO_LIBRARY}
)

configure_plugin_defines()

if(RTI_MQTT_ENABLE_STATIC_TYPES)
    list(APPEND ${RSPLUGIN_PREFIX}_DEFINES RTI_MQTT_ENABLE_STATIC_TYPES)
endif()

target_compile_definitions(${RSPLUGIN_LIB_NAME}
    PUBLIC
        ${${RSPLUGIN_PREFIX}_DEFINES}
)

# Stagging targets
install(
    TARGETS ${RSPLUGIN_LIB_NAME}
    DESTINATION "${STAGING_LIB_DIR}"
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/rti/"
    DESTINATION "${STAGING_INCLUDE_DIR}/rti/${STAGING_PLUGIN_DIR}"
)

###############################################################################
# Build tests and documentation if they are enabled
###############################################################################
if(RTIGATEWAY_ENABLE_DOCS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/doc")
endif()

