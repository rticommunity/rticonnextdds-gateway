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
rtigw_find_protobuf()
if(NOT Protobuf_FOUND)
    message(WARNING "Protobuf library not found. Example mqtt-shapes-advanced will not be available.")
    return()
endif()
###############################################################################

###############################################################################
# Protobuf helper library
###############################################################################
set(BUILD_SHARED_LIBS ON)

add_library(protobufshapes
    SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/ProtobufShapes.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/ProtobufShapes.h"
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.cc"
)

target_compile_definitions(protobufshapes
    PRIVATE
        PROTOBUFSHAPES_EXPORT_DLL
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.h"
           "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.cc"
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
         -I "${CMAKE_CURRENT_SOURCE_DIR}/proto"
         "${CMAKE_CURRENT_SOURCE_DIR}/proto/shape_type.proto"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/proto/shape_type.proto"
    COMMENT "Generating Protobuf Descriptor Set from shape_type.proto"
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pbdesc"
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --descriptor_set_out="${CMAKE_CURRENT_BINARY_DIR}/shape_type.pbdesc"
         --include_imports
         -I "${CMAKE_CURRENT_SOURCE_DIR}/proto"
         "${CMAKE_CURRENT_SOURCE_DIR}/proto/shape_type.proto"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/proto/shape_type.proto"
    COMMENT "Generating Protobuf C++ source files from shape_type.proto"
)

add_custom_target(
    generate_mqtt_protobuf_shapes_adv ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pbdesc"
)

target_link_libraries(protobufshapes
    PUBLIC
        RTIConnextDDS::c_api
)

target_link_libraries(protobufshapes
    PRIVATE
        ${Protobuf_LIBRARIES}
)

target_include_directories(protobufshapes
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
)

target_include_directories(protobufshapes
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

###############################################################################
###############################################################################
include(ConnextDdsCodegen)

set(EXAMPLE_NAME mqtt-shapes-advanced)

set(IDL_NAME "ShapeType")
set(IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/idl")

# Generate codegen files
connextdds_rtiddsgen_run(
    IDL_FILE "${IDL_DIR}/${IDL_NAME}.idl"
    LANG "C"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/idl"
)

add_executable(${EXAMPLE_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/ShapesAgentDds.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/ShapesAgentDdsSubscriber.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/ShapesAgentDdsPublisher.c"
    ${${IDL_NAME}_C_SOURCES}
)

target_link_libraries(${EXAMPLE_NAME}
    PRIVATE
        rtimqttadapter
        RTIConnextDDS::c_api
        protobufshapes
)

target_include_directories(${EXAMPLE_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/srcC"
        "${CMAKE_CURRENT_BINARY_DIR}/idl"
        "${TRANSFORMATION_COMMON_DIR}/srcC"
)

if(DEFINED ENV{RTI_MQTTSHAPES_SHAPES_RATE} OR DEFINED RTI_MQTTSHAPES_SHAPES_RATE)
    rtigw_set_required(RTI_MQTTSHAPES_SHAPES_RATE RTI_MQTTSHAPES_SHAPES_RATE)
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
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE
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

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

###############################################################################
###############################################################################

install(
    TARGETS protobufshapes
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pbdesc"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)
