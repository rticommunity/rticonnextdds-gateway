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

cmake_minimum_required (VERSION 3.5)
SET(BUILD_SHARED_LIBS ON)

# This project assumes that C code is C99 and C++ code is C++11. If that
# shouldn't be the behavior for any target, it might be changed per target.
#   set_property(TARGET target_name PROPERTY CXX_STANDARD 11)
#   set_property(TARGET target_name PROPERTY CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED ON)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project (ConnextGateways VERSION 1.0.0)

# Set common variables
set (GATEWAY_DOC "${CMAKE_CURRENT_SOURCE_DIR}/doc")
set (COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common")
set (JSON_PARSER_WRAPPER_DIR "${COMMON_DIR}/json_parser")
set (DDS_COMMON_DIR "${COMMON_DIR}/dds_specific")
set (TRANSFORMATION_COMMON_DIR "${COMMON_DIR}/transformation")
set (THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
set (LIBMODBUS_DIR "${THIRD_PARTY_DIR}/libmodbus")
set (PAHO_MQTT_C_DIR "${THIRD_PARTY_DIR}/paho.mqtt.c")
set (LIBRD_KAFKA_C_DIR "${THIRD_PARTY_DIR}/librdkafka")
set (RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")
set (UTILS_COMMON_DIR "${COMMON_DIR}/utils")
set (JSON_PARSER_DIR "${THIRD_PARTY_DIR}/json_parser")
set (ADAPTERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/adapters")
set (PROCESSORS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/processors")
set (TRANSFORMATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/transformations")
set (EXAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

set (STAGING_LIB_DIR "lib")
set (STAGING_BIN_DIR "bin")
set (STAGING_DOC_DIR "doc")
set (STAGING_TEST_DIR "test")
set (STAGING_EXAMPLES_DIR "examples")
set (STAGING_RESOURCE_DIR "resource")
set (STAGING_INCLUDE_DIR "include")

# If the RTIGATEWAY_ENABLE_${AdapterName} is not specified, it will take the
# value of the RTIGATEWAY_ENABLE_ALL that is ON by default
option(RTIGATEWAY_ENABLE_ALL "Build everything" ON)
option(RTIGATEWAY_ENABLE_MODBUS "Build Modbus Adapter" ${RTIGATEWAY_ENABLE_ALL})
option(RTIGATEWAY_ENABLE_MQTT "Build MQTT Adapter" ${RTIGATEWAY_ENABLE_ALL})
option(RTIGATEWAY_ENABLE_KAFKA "Build KAFKA Adapter" ${RTIGATEWAY_ENABLE_ALL})
option(RTIGATEWAY_ENABLE_FWD "Build Forwarder Processor" ${RTIGATEWAY_ENABLE_ALL})
option(RTIGATEWAY_ENABLE_TSFM_FIELD "Build Field Transformation" ${RTIGATEWAY_ENABLE_ALL})
option(RTIGATEWAY_ENABLE_TSFM_JSON "Build JSON Transformation" ${RTIGATEWAY_ENABLE_ALL})
option(RTIGATEWAY_ENABLE_TESTS "Build tester applications for enabled plugins" ${RTIGATEWAY_ENABLE_ALL})
option(RTIGATEWAY_ENABLE_EXAMPLES "Build examples applications for enabled plugins" ${RTIGATEWAY_ENABLE_ALL})
option(RTIGATEWAY_ENABLE_DOCS "Build documentation for enabled plugins" OFF)
option(RTIGATEWAY_ENABLE_SSL "Enable support for SSL/TLS" OFF)
option(RTIGATEWAY_ENABLE_LOG "Enable logging to stdout" OFF)
option(RTIGATEWAY_ENABLE_TRACE "Enable support for trace-level logging" OFF)

include("${RESOURCE_DIR}/cmake/common_functions.cmake")

init_install_path()

# Only check RTI Connext DDS dependencies if at least one plugin
# will be build.
if (RTIGATEWAY_ENABLE_MODBUS
        OR RTIGATEWAY_ENABLE_MQTT
        OR RTIGATEWAY_ENABLE_KAFKA
        OR RTIGATEWAY_ENABLE_FWD
        OR RTIGATEWAY_ENABLE_TSFM_FIELD
        OR RTIGATEWAY_ENABLE_TSFM_JSON)
    # Configure Connext DDS dependencies
    set_required_variable(CONNEXTDDS_DIR NDDSHOME CONNEXTDDS_DIR)

    # CONNEXTDDS_ARCH is not required, but recommended
    # eg -DCONNEXTDDS_ARCH=x64Linux4gcc7.3.0
    if (DEFINED CONNEXTDDS_ARCH)
        message(STATUS "CONNEXTDDS_ARCH set to <${CONNEXTDDS_ARCH}>")
    endif()

    list(APPEND CMAKE_MODULE_PATH "${CONNEXTDDS_DIR}/resource/cmake")
    find_package(RTIConnextDDS "6.0.1" COMPONENTS routing_service core REQUIRED)
    message(STATUS "definition added ${CONNEXTDDS_DEFINITIONS}")

    # Configure Connext DDS libraries variables
    set(ROUTING_SERVICE_CXX_LIBRARIES
        RTIConnextDDS::routing_service_cpp2
    )
    set(ROUTING_SERVICE_C_LIBRARIES
        RTIConnextDDS::routing_service_c
    )
endif()

# Configure third-party
add_subdirectory("${THIRD_PARTY_DIR}")

if (RTIGATEWAY_ENABLE_TESTS)
    enable_testing()
endif()

# Modbus Adapter
if (RTIGATEWAY_ENABLE_MODBUS)
    set (RSPLUGIN_PREFIX "RTI_MODBUS")
    set (STAGING_PLUGIN_DIR "modbus")

    add_subdirectory("${ADAPTERS_DIR}/modbus")
    add_subdirectory("${RESOURCE_DIR}/modbus")

    if (RTIGATEWAY_ENABLE_EXAMPLES)
        add_subdirectory("${EXAMPLES_DIR}/modbus")
    endif()
endif()

if (RTIGATEWAY_ENABLE_MQTT)
    set (RSPLUGIN_PREFIX "RTI_MQTT")
    set (STAGING_PLUGIN_DIR "mqtt")

    add_subdirectory("${ADAPTERS_DIR}/mqtt")

    if (RTIGATEWAY_ENABLE_EXAMPLES)
        add_subdirectory("${EXAMPLES_DIR}/mqtt/mqtt-agent")
    endif()
endif()

if (RTIGATEWAY_ENABLE_KAFKA)
    set (RSPLUGIN_PREFIX "RTI_KAFKA")
    set (STAGING_PLUGIN_DIR "kafka")

    add_subdirectory("${ADAPTERS_DIR}/kafka")

    #if (RTIGATEWAY_ENABLE_EXAMPLES)
    #    add_subdirectory("${EXAMPLES_DIR}/kafka")
    #endif()
endif()

if (RTIGATEWAY_ENABLE_FWD)
    set (RSPLUGIN_PREFIX "RTI_PRCS_FWD")
    set (STAGING_PLUGIN_DIR "fwd")
    add_subdirectory("${PROCESSORS_DIR}/fwd")
endif()

if (RTIGATEWAY_ENABLE_TSFM_FIELD)
    set (RSPLUGIN_PREFIX "RTI_TSFM_FIELD")
    set (STAGING_PLUGIN_DIR "tsfm_field")
    add_subdirectory("${TRANSFORMATIONS_DIR}/field")
endif()

if (RTIGATEWAY_ENABLE_TSFM_JSON)
    set (RSPLUGIN_PREFIX "RTI_TSFM_JSON")
    set (STAGING_PLUGIN_DIR "tsfm_json")
    add_subdirectory("${TRANSFORMATIONS_DIR}/json")
endif()

# Shapes example
if (RTIGATEWAY_ENABLE_MQTT
        AND RTIGATEWAY_ENABLE_TSFM_FIELD
        AND RTIGATEWAY_ENABLE_TSFM_JSON
        AND RTIGATEWAY_ENABLE_EXAMPLES)
    # Although this example uses several plugins, it belongs to mqtt
    set (STAGING_PLUGIN_DIR "mqtt")
    add_subdirectory("${EXAMPLES_DIR}/mqtt/mqtt-shapes")
endif()

if (RTIGATEWAY_ENABLE_DOCS)
    add_subdirectory("${GATEWAY_DOC}")
endif()
