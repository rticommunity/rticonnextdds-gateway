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

###############################################################################
###############################################################################

set(EXAMPLE_NAME mqtt-shapes)

set(IDL_NAME "ShapeType")

# Generate codegen files
rtigw_connextdds_run_codegen(
    IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/idl/${IDL_NAME}.idl"
    LANGUAGE "C"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/idl"
)

add_executable(${EXAMPLE_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/ShapesAgentDds.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/ShapesAgentDdsSubscriber.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/ShapesAgentDdsPublisher.c"
    ${GENERATED_SRC_FILES}
)

target_link_libraries(${EXAMPLE_NAME}
    PRIVATE
        rtimqttadapter
        RTIConnextDDS::c_api
)

target_include_directories(${EXAMPLE_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/srcC"
        "${CMAKE_CURRENT_BINARY_DIR}/idl"
        "${TRANSFORMATION_COMMON_DIR}/srcC"
)

if(DEFINED ENV{RTI_MQTTSHAPES_SHAPES_RATE} OR DEFINED RTI_MQTTSHAPES_SHAPES_RATE)
    rtigw_set_required_variable(RTI_MQTTSHAPES_SHAPES_RATE RTI_MQTTSHAPES_SHAPES_RATE)
    append_to_list(SHAPESAGENT_DEFINES "-DSHAPES_AGENT_WRITER_LOOP_SEC=${RTI_MQTTSHAPES_SHAPES_RATE}")
endif()

target_compile_definitions(${EXAMPLE_NAME}
    PUBLIC
        ${SHAPESAGENT_DEFINES}
)

# staging files
install(
    TARGETS ${EXAMPLE_NAME}
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    PERMISSIONS OWNER_EXECUTE
)

install(
    FILES "README.md"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

###############################################################################
###############################################################################

install(
    DIRECTORY etc
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}")

install(
    DIRECTORY scripts
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    USE_SOURCE_PERMISSIONS)

###############################################################################
###############################################################################
