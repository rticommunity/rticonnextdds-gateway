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

set(EXAMPLE_NAME mqtt-shapes-protobuf)

###############################################################################
rtigw_find_protobuf()
if(NOT Protobuf_FOUND)
    message(WARNING "Protobuf library not found. Example ${EXAMPLE_NAME} will not be available.")
    return()
endif()
###############################################################################

install(
    FILES
        README.md
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

install(
    DIRECTORY scripts etc
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    USE_SOURCE_PERMISSIONS
)

###############################################################################
# Generate Protobuf Descriptor Set used by Protobuf transformation plugin
###############################################################################
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
    generate_mqtt_protobuf_shapes ALL
    DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pbdesc"
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pbdesc"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

###############################################################################
# Generate C++ Protobuf interfaces
###############################################################################
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

add_custom_target(
    generate_mqtt_protobuf_shapes_interfaces ALL
    DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.h"
        "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.cc"
)

###############################################################################
# Build MQTT Publisher/Subscriber applications
###############################################################################
if(RTIGATEWAY_ENABLE_SSL)
    # Paho with SSL/TLS support
    set(PAHO_LIBRARY "paho-mqtt3as")
else()
    set(PAHO_LIBRARY "paho-mqtt3a")
endif()

add_executable(shapes_mqtt_publisher
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/shapes_mqtt_publisher.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/shapes_mqtt_node.hxx"
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.cc"
)
target_link_libraries(shapes_mqtt_publisher
    PRIVATE
        ${Protobuf_LIBRARIES}
        ${PAHO_LIBRARY}
)
target_include_directories(shapes_mqtt_publisher
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
)
add_dependencies(shapes_mqtt_publisher
    generate_mqtt_protobuf_shapes_interfaces
)

add_executable(shapes_mqtt_subscriber
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/shapes_mqtt_subscriber.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/shapes_mqtt_node.hxx"
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.cc"
)
target_link_libraries(shapes_mqtt_subscriber
    PRIVATE
        ${Protobuf_LIBRARIES}
        ${PAHO_LIBRARY}
)
target_include_directories(shapes_mqtt_subscriber
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCxx
)
add_dependencies(shapes_mqtt_subscriber
    generate_mqtt_protobuf_shapes_interfaces
)

install(
    TARGETS
        shapes_mqtt_publisher
        shapes_mqtt_subscriber
    RUNTIME DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}/bin"
)
