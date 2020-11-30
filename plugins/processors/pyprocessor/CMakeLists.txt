#
# (c) 2018 Copyright, Real-Time Innovations, Inc.  All rights reserved.
#
#  RTI grants Licensee a license to use, modify, compile, and create derivative
#  works of the Software.  Licensee has the right to distribute object form
#  only for use with RTI products.  The Software is provided "as is", with no
#  warranty of any type, including any warranty for fitness for any purpose.
#  RTI is under no obligation to maintain or support the Software.  RTI shall
#  not be liable for any incidental or consequential damages arising out of the
#  use or inability to use the software.


cmake_minimum_required(VERSION 3.10)

project (PyProcessor)
# Set variables
set(RSPLUGIN_LIB_NAME rtirspyprocessor)

find_package(PythonLibs 3.9 REQUIRED)


# Define the library that will provide the  Processor plugin
add_library(${RSPLUGIN_LIB_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NativeUtils.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyServiceGlobals.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyDynamicData.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PySamples.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyInput.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyOutput.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyRoute.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyProcessor.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PyService.cxx")

# Set target properties for lang requirement output library name
set_target_properties(${RSPLUGIN_LIB_NAME}
    PROPERTIES
        CXX_STANDARD 11
        OUTPUT_NAME_DEBUG ${RSPLUGIN_LIB_NAME}d
        LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${output_dir}"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${output_dir}"
        ARCHIVE_OUTPUT_DIRECTORY "${output_dir}"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${output_dir}"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${output_dir}"
        RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${output_dir}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${output_dir}"
)



target_link_libraries(${RSPLUGIN_LIB_NAME}
    ${ROUTING_SERVICE_CXX_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

target_include_directories(${RSPLUGIN_LIB_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${PYTHON_INCLUDE_DIRS}"
)

configure_plugin_defines()

# Stagging targets
install(
    TARGETS ${RSPLUGIN_LIB_NAME}
    DESTINATION "${STAGING_LIB_DIR}"
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/modules"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/"
)

###############################################################################
# Build documentation if they are enabled

if (RTIGATEWAY_ENABLE_DOCS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/doc")
endif()
