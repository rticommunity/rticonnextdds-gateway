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

function(copy_files)
    set(BOOLEANS)
    set(SINGLE_VALUE_ARGS CUSTOM_TARGET_NAME OUTPUT_DIR)
    set(MULTI_VALUE_ARGS LIST_OF_FILES_RELATIVE_PATH)

    cmake_parse_arguments(_COMMON_FUNCTIONS
        "${BOOLEANS}"
        "${SINGLE_VALUE_ARGS}"
        "${MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    # Create the list of files to copy in the origin
    foreach(FILE ${_COMMON_FUNCTIONS_LIST_OF_FILES_RELATIVE_PATH})
        list(APPEND FILES_TO_COPY_ORIGIN "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
    endforeach(FILE ${_COMMON_FUNCTIONS_LIST_OF_FILES_RELATIVE_PATH})

    # Create the list of files to copy in the output
    foreach(FILE ${_COMMON_FUNCTIONS_LIST_OF_FILES_RELATIVE_PATH})
        list(APPEND FILES_TO_COPY_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FILE}")
    endforeach(FILE ${_COMMON_FUNCTIONS_LIST_OF_FILES_RELATIVE_PATH})


    # Create the custom command
    add_custom_command(
        # Indicate what files are generated if this command is executed
        OUTPUT
            ${FILES_TO_COPY_OUTPUT}
        # Comment to show while building
        COMMENT
            "Copying ${_COMMON_FUNCTIONS_CUSTOM_TARGET_NAME} files to binary dir"
        # Create the config directory if it doesn't exist
        COMMAND
            ${CMAKE_COMMAND} -E make_directory
                "${_COMMON_FUNCTIONS_OUTPUT_DIR}"
        # Copy the files (only if they are different)
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
                ${FILES_TO_COPY_ORIGIN}
                "${_COMMON_FUNCTIONS_OUTPUT_DIR}"
    )
    # Create the custom target
    add_custom_target(${_COMMON_FUNCTIONS_CUSTOM_TARGET_NAME}
        # To execute this custom target (that will do nothing), we need first to
        # generate these files. This triggers the custom command from the previous
        # statement
        DEPENDS
            ${FILES_TO_COPY_OUTPUT}
    )
endfunction()


###############################################################################

function(generate_doc)
    set(BOOLEANS DOXYGEN_ENABLED)
    set(SINGLE_VALUE_ARGS PLUGIN_NAME CUSTOM_TARGET_NAME INPUT_DIR OUTPUT_DIR)
    set(MULTI_VALUE_ARGS DEPENDENCIES)

    cmake_parse_arguments(_COMMON_FUNCTIONS
        "${BOOLEANS}"
        "${SINGLE_VALUE_ARGS}"
        "${MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    if (NOT SPHINX_BIN)
        set(SPHINX_BIN "sphinx-build")
    endif()

    add_custom_command(
        OUTPUT
            ${_COMMON_FUNCTIONS_OUTPUT_DIR}
        COMMENT
            "Generating ${_COMMON_FUNCTIONS_PLUGIN_NAME} Documentation"
        COMMAND
            ${SPHINX_BIN}
                ${_COMMON_FUNCTIONS_INPUT_DIR}
                ${_COMMON_FUNCTIONS_OUTPUT_DIR}
                ${SPHINX_OPTS}
        DEPENDS
            ${_COMMON_FUNCTIONS_DEPENDENCIES}
        VERBATIM
    )

    add_custom_target(${_COMMON_FUNCTIONS_CUSTOM_TARGET_NAME} ALL
        DEPENDS
            "${_COMMON_FUNCTIONS_OUTPUT_DIR}"
    )

    if (${_COMMON_FUNCTIONS_DOXYGEN_ENABLED})
        if (NOT DOXYGEN_BIN)
            set(DOXYGEN_BIN "doxygen")
        endif()

        add_custom_command(
            OUTPUT
                "${_COMMON_FUNCTIONS_INPUT_DIR}/doxyoutput"
            COMMAND
                ${DOXYGEN_BIN} "${_COMMON_FUNCTIONS_INPUT_DIR}/Doxyfile"
            WORKING_DIRECTORY
                "${_COMMON_FUNCTIONS_INPUT_DIR}"
            DEPENDS
                "${_COMMON_FUNCTIONS_INPUT_DIR}/Doxyfile"
            VERBATIM
        )
        add_custom_target("${_COMMON_FUNCTIONS_CUSTOM_TARGET_NAME}_DOXYGEN"
            DEPENDS
                "${_COMMON_FUNCTIONS_INPUT_DIR}/doxyoutput"
        )

        add_dependencies(${_COMMON_FUNCTIONS_CUSTOM_TARGET_NAME}
            ${_COMMON_FUNCTIONS_CUSTOM_TARGET_NAME}_DOXYGEN)
    endif()

endfunction()

macro(configure_plugin_defines)
    if(RTIGATEWAY_ENABLE_LOG
            OR CMAKE_BUILD_TYPE STREQUAL "Debug"
            OR NOT CMAKE_BUILD_TYPE)
        list(APPEND ${RSPLUGIN_PREFIX}_DEFINES ${RSPLUGIN_PREFIX}_ENABLE_LOG)
    else()
        list(APPEND ${RSPLUGIN_PREFIX}_DEFINES ${RSPLUGIN_PREFIX}_DISABLE_LOG)
    endif()

    if(RTIGATEWAY_ENABLE_TRACE)
        list(APPEND ${RSPLUGIN_PREFIX}_DEFINES ${RSPLUGIN_PREFIX}_ENABLE_TRACE)
    endif()

    if(RTIGATEWAY_ENABLE_SSL)
        list(APPEND ${RSPLUGIN_PREFIX}_DEFINES ${RSPLUGIN_PREFIX}_ENABLE_SSL)
    endif()

    set(${RSPLUGIN_PREFIX}_DEFINES     "${${RSPLUGIN_PREFIX}_DEFINES}"
            CACHE INTERNAL "List compiler defines")
endmacro()

function(set_required_variable out_variable)
    set(candidate_vars ${ARGN})
    foreach(v ${candidate_vars})
        # Give preference to variables passed directly to cmake
        if(DEFINED ${v})
            message(STATUS "${out_variable} = ${${v}}")
            set(${out_variable} ${${v}} PARENT_SCOPE)
            return()
        elseif(DEFINED ENV{${v}})
            message(STATUS "${out_variable} = $ENV{${v}}")
            set(${out_variable} $ENV{${v}} PARENT_SCOPE)
            return()
        endif()
    endforeach()
    message(FATAL_ERROR
        "Failed to determine value for ${out_variable}. "
        "Please specify one of '${candidate_vars}' either in your "
        "environment, or as an argument to cmake.")
endfunction()

macro(init_install_path)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX        "${CMAKE_CURRENT_BINARY_DIR}/install")
        message(STATUS "Using default CMAKE_INSTALL_PREFIX = '${CMAKE_INSTALL_PREFIX}'")
    endif()
endmacro()

#[[

init_project_configuration
--------------------------

 * Type: macro
 * Brief: configures all the global parameters used by the RTI Gateway
 * Params: no parameters
 * Restrictions: this should only be used in the main CMakeLists.txt
 * How to use it:

    init_project_configuration()
]]

macro(init_project_configuration)

    SET(BUILD_SHARED_LIBS ON)

    # This project assumes that C code is C99 and C++ code is C++11. If that
    # shouldn't be the behavior for any target, it might be changed per target.
    #   set_property(TARGET target_name PROPERTY CXX_STANDARD 11)
    #   set_property(TARGET target_name PROPERTY CXX_STANDARD_REQUIRED ON)
    set (CMAKE_C_STANDARD 99)
    set (CMAKE_C_STANDARD_REQUIRED ON)
    set (CMAKE_CXX_STANDARD 11)
    set (CMAKE_CXX_STANDARD_REQUIRED ON)

    # Set common variables
    set (GATEWAY_DOC "${CMAKE_CURRENT_SOURCE_DIR}/doc")
    set (COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common")
    set (JSON_PARSER_WRAPPER_DIR "${COMMON_DIR}/json_parser")
    set (DDS_COMMON_DIR "${COMMON_DIR}/dds_specific")
    set (TRANSFORMATION_COMMON_DIR "${COMMON_DIR}/transformation")
    set (THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
    set (LIBMODBUS_DIR "${THIRD_PARTY_DIR}/libmodbus")
    set (PAHO_MQTT_C_DIR "${THIRD_PARTY_DIR}/paho.mqtt.c")
    set (RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")
    set (UTILS_COMMON_DIR "${COMMON_DIR}/utils")
    set (JSON_PARSER_DIR "${THIRD_PARTY_DIR}/json_parser")
    set (ADAPTERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/adapters")
    set (PROCESSORS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/processors")
    set (TRANSFORMATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/transformations")
    set (EXAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

    set (STAGING_LIB_DIR "lib")
    set (STAGING_BIN_DIR "bin")
    set (STAGING_DOC_DIR "doc")
    set (STAGING_TEST_DIR "test")
    set (STAGING_EXAMPLES_DIR "examples")
    set (STAGING_RESOURCE_DIR "resource")
    set (STAGING_INCLUDE_DIR "include")

    # If the RTIGATEWAY_ENABLE_${PluginName} is not specified, it will take the
    # value of the RTIGATEWAY_ENABLE_ALL that is ON by default
    option(RTIGATEWAY_ENABLE_ALL "Build everything" ON)
    option(RTIGATEWAY_ENABLE_MODBUS "Build Modbus Adapter" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_MQTT "Build MQTT Adapter" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_FWD "Build Forwarder Processor" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_TSFM_FIELD "Build Field Transformation" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_TSFM_JSON "Build JSON Transformation" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_TESTS "Build tester applications for enabled plugins" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_EXAMPLES "Build examples applications for enabled plugins" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_DOCS "Build documentation for enabled plugins" OFF)
    option(RTIGATEWAY_ENABLE_SSL "Enable support for SSL/TLS" OFF)
    option(RTIGATEWAY_ENABLE_LOG "Enable logging to stdout" OFF)
    option(RTIGATEWAY_ENABLE_TRACE "Enable support for trace-level logging" OFF)

    init_install_path()

    # Set RTIGATEWAY_CONFIGURE_CONNEXT to check whether we need to configure
    # RTI Connext DDS.
    set(RTIGATEWAY_CONFIGURE_CONNEXT OFF)
    if(RTIGATEWAY_ENABLE_MODBUS
            OR RTIGATEWAY_ENABLE_MQTT
            OR RTIGATEWAY_ENABLE_FWD
            OR RTIGATEWAY_ENABLE_TSFM_FIELD
            OR RTIGATEWAY_ENABLE_TSFM_JSON)
        set(RTIGATEWAY_CONFIGURE_CONNEXT ON)
    endif()

    if (RTIGATEWAY_ENABLE_TESTS)
        enable_testing()
    endif()

endmacro()

#[[
configure_connext_dds
---------------------

 * Type: macro
 * Brief: configures RTI Connext DDS by using its FindPackage
 * Params:
 ** _CONNEXTDDS_VERSION: version of RTI Connext DDS to configure
 * Restrictions: CONNEXTDDS_DIR should be set
 * How to use it:

    configure_connext_dds(6.1.0)
]]

macro(configure_connext_dds _CONNEXTDDS_VERSION)
    # Configure Connext DDS dependencies
    set_required_variable(CONNEXTDDS_DIR NDDSHOME CONNEXTDDS_DIR)

    # CONNEXTDDS_ARCH is not required, but recommended
    # eg -DCONNEXTDDS_ARCH=x64Linux4gcc7.3.0
    if (DEFINED CONNEXTDDS_ARCH)
        message(STATUS "CONNEXTDDS_ARCH set to <${CONNEXTDDS_ARCH}>")
    endif()

    list(APPEND CMAKE_MODULE_PATH "${CONNEXTDDS_DIR}/resource/cmake")

    find_package(
        RTIConnextDDS
            ${_CONNEXTDDS_VERSION}
        COMPONENTS
            routing_service core
        REQUIRED)

    message(STATUS "definition added ${CONNEXTDDS_DEFINITIONS}")

    # Configure Connext DDS libraries variables
    set(ROUTING_SERVICE_CXX_LIBRARIES
        RTIConnextDDS::routing_service_cpp2
    )
    set(ROUTING_SERVICE_C_LIBRARIES
        RTIConnextDDS::routing_service_c
    )
endmacro()

#[[

add_subdirectory_if_exists
--------------------------

 * Type: macro
 * Brief: adds a subdirectory if it exists
 * Params:
 ** _SUBDIRECTORY_DIR: path to the subdirectory to add
 * How to use it:

    add_subdirectory_if_exists("${CMAKE_CURRENT_SOURCE_DIR}/mySubdirectory")
]]
macro(add_subdirectory_if_exists _SUBDIRECTORY_DIR)
    if(IS_DIRECTORY "${_SUBDIRECTORY_DIR}")
        add_subdirectory("${_SUBDIRECTORY_DIR}")
    endif()
endmacro()

#[[

add_doc_if_exist
----------------

 * Type: macro
 * Brief: adds the "doc" directory if it exists in the current source
          directory. This folder is only added if "building doc" is enabled.
 * Params: no parameters
 * How to use it:

    add_doc_if_exist()
]]
macro(add_doc_if_exist)
    if (RTIGATEWAY_ENABLE_DOCS)
        add_subdirectory_if_exists("${CMAKE_CURRENT_SOURCE_DIR}/doc")
    endif()
endmacro()

#[[

add_test_if_exist
-----------------

 * Type: macro
 * Brief: adds the "test" directory if it exists in the current source
          directory. This folder is only added if testing is enabled.
 * Params: no parameters
 * How to use it:

    add_test_if_exist()
]]
macro(add_test_if_exist)
    if (RTIGATEWAY_ENABLE_TESTS)
        add_subdirectory_if_exists("${CMAKE_CURRENT_SOURCE_DIR}/test")
    endif()
endmacro()

#[[

add_plugin
----------

 * Type: function
 * Brief: adds the corresponding plugin directories if they are added. These
          directories are: the main plugin directory, related resources and
          related examples for that plugin.
 * Params:
 ** ENABLE_PLUGIN: boolean that indicates that this plugin is enabled
 ** PREFIX: plugin prefix
 ** STAGING_DIR: staging directory name for the plugin
 ** PLUGIN_DIR_NAME: name that identifies the folder of that plugin
 ** PLUGIN_TYPE: indicates whether a plugin is an "ADAPTER", a "PROCESSOR" or a
                 "TRANSFORMATION"
 * Restrictions: CONNEXTDDS_DIR should be set
 * How to use it:

    add_plugin(
        ENABLE_PLUGIN RTIGATEWAY_ENABLE_MODBUS
        PREFIX "RTI_MODBUS"
        STAGING_DIR "modbus"
        PLUGIN_DIR_NAME "modbus"
        PLUGIN_TYPE "ADAPTER"
    )
]]
function(add_plugin)
    set(BOOLEANS ENABLE_PLUGIN)
    set(SINGLE_VALUE_ARGS PREFIX STAGING_DIR PLUGIN_DIR_NAME PLUGIN_TYPE)
    set(MULTI_VALUE_ARGS)

    cmake_parse_arguments(_COMMON_FUNCTIONS
        "${BOOLEANS}"
        "${SINGLE_VALUE_ARGS}"
        "${MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    set(PLUGIN_TYPE_DIR "")
    if(${_COMMON_FUNCTIONS_PLUGIN_TYPE} STREQUAL "ADAPTER")
        set(PLUGIN_TYPE_DIR ${ADAPTERS_DIR})
    elseif(${_COMMON_FUNCTIONS_PLUGIN_TYPE} STREQUAL "PROCESSOR")
        set(PLUGIN_TYPE_DIR ${PROCESSORS_DIR})
    elseif(${_COMMON_FUNCTIONS_PLUGIN_TYPE} STREQUAL "TRANSFORMATION")
        set(PLUGIN_TYPE_DIR ${TRANSFORMATIONS_DIR})
    else()
        mesasge(FATAL_ERROR
            "The plugin ${_COMMON_FUNCTIONS_PLUGIN_DIR_NAME} is not identified
            as ADAPTER, PROCESSOR nor TRANSFORMATION"
        )
    endif()

    if (${_COMMON_FUNCTIONS_ENABLE_PLUGIN})
        set (RSPLUGIN_PREFIX ${_COMMON_FUNCTIONS_PREFIX})
        set (STAGING_PLUGIN_DIR ${_COMMON_FUNCTIONS_STAGING_DIR})

        # Add plugin
        add_subdirectory_if_exists(
            "${PLUGIN_TYPE_DIR}/${_COMMON_FUNCTIONS_PLUGIN_DIR_NAME}"
        )
        # Add resources
        add_subdirectory_if_exists(
            "${RESOURCE_DIR}/${_COMMON_FUNCTIONS_PLUGIN_DIR_NAME}"
        )

        # Add examples if they are enabled
        if (RTIGATEWAY_ENABLE_EXAMPLES)
            add_subdirectory_if_exists(
                "${EXAMPLES_DIR}/${_COMMON_FUNCTIONS_PLUGIN_DIR_NAME}"
            )
        endif()

    endif()

endfunction()
