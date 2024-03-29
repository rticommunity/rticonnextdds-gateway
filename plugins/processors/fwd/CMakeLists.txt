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

cmake_minimum_required(VERSION 3.7.0)

include(ConnextDdsCodegen)

# Set variables
set(RSPLUGIN_LIB_NAME rtifwdprocessor)

# Call codegen
# Generate codegen files
set(IDL_NAME "rtiprocess_fwd_types")
set(IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/idl")

connextdds_rtiddsgen_run(
    IDL_FILE "${IDL_DIR}/${IDL_NAME}.idl"
    LANG "C++11"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/idl"
    EXTRA_ARGS
        "-unboundedSupport"
        "-qualifiedEnumerator"
)

# Create library
add_library(
    ${RSPLUGIN_LIB_NAME}
    SHARED
        "${JSON_PARSER_DIR}/json.c"
        "srcCxx/ByInputNameForwardingEngine.cxx"
        "srcCxx/ByInputValueForwardingEngine.cxx"
        "srcCxx/ForwardingEngine.cxx"
        "srcCxx/Properties.cxx"
        "${${IDL_NAME}_CXX11_SOURCES}"
)

set_target_properties(${RSPLUGIN_LIB_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(
    ${RSPLUGIN_LIB_NAME}
    PRIVATE
        ${CONNEXTDDS_INCLUDE_DIRS}
        "${CMAKE_CURRENT_SOURCE_DIR}/include/rti"
        "${JSON_PARSER_DIR}/"
        "${CMAKE_CURRENT_BINARY_DIR}/idl"
)

target_link_libraries(
    ${RSPLUGIN_LIB_NAME}
    RTIConnextDDS::routing_service_cpp2
)

rtigw_configure_plugin_defines()

target_compile_definitions(
    ${RSPLUGIN_LIB_NAME}
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
# Build documentation and tests if they are enabled
###############################################################################
rtigw_add_doc()
rtigw_add_tests()
