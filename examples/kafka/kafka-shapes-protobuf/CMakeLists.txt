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

set(EXAMPLE_NAME kafka-shapes-protobuf)

###############################################################################
rtigw_find_protobuf()
if(NOT Protobuf_FOUND)
    message(WARNING "Protobuf library not found. Example ${EXAMPLE_NAME} will not be available.")
    return()
endif()
###############################################################################

install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/shapesdemo_kafka_protobuf.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/demo_scenario.png"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

install(
    DIRECTORY scripts
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
    generate_kafka_protobuf_shapes ALL
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
    generate_kafka_protobuf_shapes_interfaces ALL
    DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.h"
        "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.cc"
)

###############################################################################
# Build Kafka Publisher/Subscriber applications
###############################################################################
add_executable(shapes_kafka_publisher
    srcCxx/shapes_kafka_publisher.cxx
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.h"
)
target_include_directories(shapes_kafka_publisher PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx"
)
target_link_libraries(shapes_kafka_publisher
    ${Protobuf_LIBRARIES}
    rdkafka
)
add_dependencies(shapes_kafka_publisher
    generate_kafka_protobuf_shapes_interfaces
)

add_executable(shapes_kafka_subscriber
    srcCxx/shapes_kafka_subscriber.cxx
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/shape_type.pb.h"
)
target_include_directories(shapes_kafka_subscriber PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx"
)
target_link_libraries(shapes_kafka_subscriber
    ${Protobuf_LIBRARIES}
    rdkafka
)
add_dependencies(shapes_kafka_subscriber
    generate_kafka_protobuf_shapes_interfaces
)

install(
    TARGETS
        shapes_kafka_publisher
        shapes_kafka_subscriber
    RUNTIME DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}/bin"
)
