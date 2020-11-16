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
set(RSPLUGIN_LIB_NAME rtijsontransf)

# Create library
add_library(
    ${RSPLUGIN_LIB_NAME}
    SHARED
        "srcC/FlatTypeTransformation.c"
        "${JSON_PARSER_DIR}/json.c"
        "${TRANSFORMATION_COMMON_DIR}/srcC/TransformationInfrastructure.c"
        "${TRANSFORMATION_COMMON_DIR}/srcC/Transformation.c"
        "${TRANSFORMATION_COMMON_DIR}/srcC/TransformationPlugin.c"
        "${TRANSFORMATION_COMMON_DIR}/srcC/TransformationUserPlugin.c"
)

target_include_directories(
    ${RSPLUGIN_LIB_NAME}
    PRIVATE
        ${CONNEXTDDS_INCLUDE_DIRS}
        "srcC/"
        "${TRANSFORMATION_COMMON_DIR}/srcC/"
        "${JSON_PARSER_DIR}/"
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

###############################################################################
# Build documentation and testes if they are enabled
if (RTIGATEWAY_ENABLE_DOCS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/doc")
endif()


