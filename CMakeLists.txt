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

cmake_minimum_required (VERSION 3.10)
project (RTIGateway VERSION 6.1.0)

include("${CMAKE_CURRENT_SOURCE_DIR}/resource/cmake/common_functions.cmake")

init_project_configuration()

# Only check RTI Connext DDS dependencies if at least one plugin will be built.
if (RTIGATEWAY_CONFIGURE_CONNEXT)
    configure_connext_dds(6.0.1)
endif()

# Configure third-party
add_subdirectory("${THIRD_PARTY_DIR}")

# Adding adapters
add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_MODBUS
    PREFIX "RTI_MODBUS"
    STAGING_DIR "modbus"
    PLUGIN_DIR_NAME "modbus"
    PLUGIN_TYPE "ADAPTER"
)

add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_MQTT
    PREFIX "RTI_MQTT"
    STAGING_DIR "mqtt"
    PLUGIN_DIR_NAME "mqtt"
    PLUGIN_TYPE "ADAPTER"
)

# Adding processors
add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_FWD
    PREFIX "RTI_PRCS_FWD"
    STAGING_DIR "fwd"
    PLUGIN_DIR_NAME "fwd"
    PLUGIN_TYPE "PROCESSOR"
)

# Adding transformations
add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_FWD
    PREFIX "RTI_TSFM_FIELD"
    STAGING_DIR "tsfm_field"
    PLUGIN_DIR_NAME "field"
    PLUGIN_TYPE "TRANSFORMATION"
)

add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_FWD
    PREFIX "RTI_TSFM_JSON"
    STAGING_DIR "tsfm_json"
    PLUGIN_DIR_NAME "json"
    PLUGIN_TYPE "TRANSFORMATION"
)

# Add doc if exists
add_doc_if_exist()
