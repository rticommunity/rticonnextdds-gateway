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

rtigw_find_protobuf()
if(NOT Protobuf_FOUND)
    message(WARNING "Protobuf library not found. Protobuf Transformation plugin will not be built.")
    return()
endif()

# Set variables
set(RSPLUGIN_LIB_NAME rtiprotobuftransf)

# Create library
add_library(
    ${RSPLUGIN_LIB_NAME}
    SHARED
        "srcCxx/ProtobufTransformation.cxx"
        "srcCxx/ProtobufTransformation.hpp"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/ProtobufDynamicDataConverter.cxx"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/ProtobufDynamicDataConverter.hpp"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/ProtobufDescriptorSet.cxx"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/ProtobufDescriptorSet.hpp"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/DynamicDataConverter.hpp"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/DynamicDataUtils.hpp"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/DynamicDataUtils.cpp"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/ProtobufConverterUtils.hpp"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/ProtobufToDds.cpp"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/DdsToProtobuf.cpp"
        "${TRANSFORMATION_COMMON_DIR}/srcC/TransformationInfrastructure.c"
        "${TRANSFORMATION_COMMON_DIR}/srcC/Transformation.c"
        "${TRANSFORMATION_COMMON_DIR}/srcC/TransformationPlugin.c"
        "${TRANSFORMATION_COMMON_DIR}/srcC/TransformationUserPlugin.c"
        "${DDS_COMMON_DIR}/srcC/SequenceHelpers.c"
        "${DDS_COMMON_DIR}/srcC/DynamicDataHelpers.c"
        "${UTILS_COMMON_DIR}/srcCxx/ScopeExitAction.hpp"
)

set_target_properties(${RSPLUGIN_LIB_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(
    ${RSPLUGIN_LIB_NAME}
    PRIVATE
        ${CONNEXTDDS_INCLUDE_DIRS}
        "srcCxx/"
        "${TRANSFORMATION_COMMON_DIR}/srcC/"
        "${PROTOBUF2DDS_COMMON_DIR}/srcCxx/"
        "${DDS_COMMON_DIR}/srcC/"
        "${UTILS_COMMON_DIR}/srcCxx/"
)

target_link_libraries(
    ${RSPLUGIN_LIB_NAME}
    RTIConnextDDS::routing_service_cpp2
    ${Protobuf_LIBRARIES}
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

###############################################################################
# Build documentation and tests if they are enabled
###############################################################################
rtigw_add_doc()
rtigw_add_tests()
