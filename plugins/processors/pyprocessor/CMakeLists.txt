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

cmake_minimum_required(VERSION 3.10)

# Set variables
set(RSPLUGIN_LIB_NAME rtipyprocessor)

find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)

# Define the library that will provide the  Processor plugin
add_library(${RSPLUGIN_LIB_NAME}
    SHARED
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/NativeUtils.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/PyServiceGlobals.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/PyDynamicData.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/PySamples.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/PyInput.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/PyOutput.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/PyRoute.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/PyProcessor.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/PyService.cxx"
)

# Set target properties for lang requirement output library name
set_target_properties(${RSPLUGIN_LIB_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_link_libraries(${RSPLUGIN_LIB_NAME}
    PUBLIC
        ${ROUTING_SERVICE_C_LIBRARIES}
        ${ROUTING_SERVICE_CXX_LIBRARIES}
        ${Python_LIBRARIES}
)

target_include_directories(${RSPLUGIN_LIB_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx"
        ${Python_INCLUDE_DIRS}
)

configure_plugin_defines()

# Stagging targets
install(
    TARGETS ${RSPLUGIN_LIB_NAME}
    DESTINATION "${STAGING_LIB_DIR}"
)

###############################################################################
# Build documentation if it is enabled

if (RTIGATEWAY_ENABLE_DOCS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/doc")
endif()
