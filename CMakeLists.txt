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
project (RTIConnextGateway VERSION 6.1.0)

include(resource/cmake/CommonFunctions.cmake)

rtigw_init_globals()

# Configure third-party
add_subdirectory("${THIRD_PARTY_DIR}")

# Adding adapters
rtigw_add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_MODBUS
    PREFIX "RTI_MODBUS"
    STAGING_DIR "modbus"
    PLUGIN_DIR_NAME "modbus"
    PLUGIN_TYPE "ADAPTER"
)

rtigw_add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_MQTT
    PREFIX "RTI_MQTT"
    STAGING_DIR "mqtt"
    PLUGIN_DIR_NAME "mqtt"
    PLUGIN_TYPE "ADAPTER"
)

rtigw_add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_KAFKA
    PREFIX "RTI_KAFKA"
    STAGING_DIR "kafka"
    PLUGIN_DIR_NAME "kafka"
    PLUGIN_TYPE "ADAPTER"
)

# Adding processors
rtigw_add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_FWD
    PREFIX "RTI_PRCS_FWD"
    STAGING_DIR "fwd"
    PLUGIN_DIR_NAME "fwd"
    PLUGIN_TYPE "PROCESSOR"
)

# Adding transformations
rtigw_add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_TSFM_FIELD
    PREFIX "RTI_TSFM_FIELD"
    STAGING_DIR "tsfm_field"
    PLUGIN_DIR_NAME "field"
    PLUGIN_TYPE "TRANSFORMATION"
)

rtigw_add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_TSFM_JSON
    PREFIX "RTI_TSFM_JSON"
    STAGING_DIR "tsfm_json"
    PLUGIN_DIR_NAME "json"
    PLUGIN_TYPE "TRANSFORMATION"
)

rtigw_add_plugin(
    ENABLE_PLUGIN RTIGATEWAY_ENABLE_TSFM_SEQ2ARRAY
    PREFIX "RTI_TSFM_SEQ2ARRAY"
    STAGING_DIR "tsfm_seq2array"
    PLUGIN_DIR_NAME "seq2array"
    PLUGIN_TYPE "TRANSFORMATION"
)

# Add doc if exists
rtigw_add_doc()
