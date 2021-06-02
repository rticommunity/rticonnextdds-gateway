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

# Set variables
set(RSPLUGIN_LIB_NAME rtifwdprocessor)

# Call codegen
# Generate codegen files
set(IDL_NAME "rtiprocess_fwd_types")
set(CODEGEN_OPTIONS "-unboundedSupport" "-qualifiedEnumerator")
connextdds_run_codegen(
    IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/idl/${IDL_NAME}.idl"
    LANGUAGE "C++11"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/idl"
    INPUT_PARAMS ${CODEGEN_OPTIONS}
)
list(APPEND GENERATED_SRC_IDL_FILES ${GENERATED_SRC_FILES})

# Create library
add_library(
    ${RSPLUGIN_LIB_NAME}
    SHARED
        "${JSON_PARSER_DIR}/json.c"
        "srcCxx/ByInputNameForwardingEngine.cxx"
        "srcCxx/ByInputValueForwardingEngine.cxx"
        "srcCxx/ForwardingEngine.cxx"
        "srcCxx/Properties.cxx"
        "${GENERATED_SRC_IDL_FILES}"
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
    ${ROUTING_SERVICE_CXX_LIBRARIES}
)

configure_plugin_defines()

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
# Build documentation if they are enabled
add_doc_if_exist()
add_test_if_exist()
