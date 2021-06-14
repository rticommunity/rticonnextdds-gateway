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

#[[
rtigw_list_prefix
-----------------

 * Brief: prepend a prefix to all the elements of a list
 * Params:
 ** LIST: list to add the prefix to all its elements
 ** PREFIX: prefix to add
 ** _OUT_VARIABLE: output list that will contain the list with the prefix

 * How to use it:

    rtigw_list_prefix(MY_OUT_VAR
        INPUT_LIST my_list
        PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/"
    )
]]

function(rtigw_list_prefix _OUT_VARIABLE)
    set(_BOOLEANS)
    set(_SINGLE_VALUE_ARGS INPUT_LIST PREFIX)
    set(_MULTI_VALUE_ARGS)

    cmake_parse_arguments(_args
        "${_BOOLEANS}"
        "${_SINGLE_VALUE_ARGS}"
        "${_MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    set(_result)
    foreach(_val ${_args_INPUT_LIST})
        list(APPEND _result "${_args_PREFIX}${_val}")
    endforeach()
    set(${_OUT_VARIABLE} ${_result} PARENT_SCOPE)
endfunction()

function(rtigw_copy_files)
    set(_BOOLEANS)
    set(_SINGLE_VALUE_ARGS CUSTOM_TARGET_NAME OUTPUT_DIR)
    set(_MULTI_VALUE_ARGS LIST_OF_FILES_RELATIVE_PATH)

    cmake_parse_arguments(_args
        "${_BOOLEANS}"
        "${_SINGLE_VALUE_ARGS}"
        "${_MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    # Create the list of the input and output files to copy
    rtigw_list_prefix(_files_input
        INPUT_LIST ${_args_LIST_OF_FILES_RELATIVE_PATH}
        PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/"
    )

    rtigw_list_prefix(_files_output
        INPUT_LIST ${_args_LIST_OF_FILES_RELATIVE_PATH}
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/"
    )

    # Create the custom command
    add_custom_command(
        # Indicate what files are generated if this command is executed
        OUTPUT
            ${_files_output}
        # Comment to show while building
        COMMENT
            "Copying ${_args_CUSTOM_TARGET_NAME} files to binary dir"
        # Create the config directory if it doesn't exist
        COMMAND
            ${CMAKE_COMMAND} -E make_directory "${_args_OUTPUT_DIR}"
        # Copy the files (only if they are different)
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${_files_input} "${_args_OUTPUT_DIR}"
    )
    # Create the custom target
    add_custom_target(${_args_CUSTOM_TARGET_NAME}
        # To execute this custom target (that will do nothing), we need first to
        # generate these files. This triggers the custom command from the previous
        # statement
        DEPENDS
            ${_files_output}
    )
endfunction()


#[[
rtigw_generate_doc
------------------

 * Brief: generate Sphinx doc form source
 * Params:
 ** PLUGIN: name of the plugin that is generating documentation.
 ** TARGET: custom target name that will be created to generate doc.
 ** INPUT: input directory where the Sphinx source files are located.
 ** OUTPUT: output directory to the generated doc.
 ** DEPENDS: dependencies that will be added. These will be the Sphinx files.
 ** DOXYGEN: boolean to indicate that it requires to generate doxygen doc.

 * How to use it:

    rtigw_generate_doc(
        PLUGIN "GENERAL"
        TARGET "general-doc"
        INPUT "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html"
        DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/conf.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/building.rst"
    )
]]
function(rtigw_generate_doc)
    set(_BOOLEANS DOXYGEN)
    set(_SINGLE_VALUE_ARGS PLUGIN TARGET INPUT OUTPUT)
    set(_MULTI_VALUE_ARGS DEPENDS)

    cmake_parse_arguments(_args
        "${_BOOLEANS}"
        "${_SINGLE_VALUE_ARGS}"
        "${_MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    if (NOT SPHINX_BIN)
        set(SPHINX_BIN "sphinx-build")
    endif()

    add_custom_command(
        OUTPUT
            ${_args_OUTPUT}
        COMMENT
            "Generating ${_args_PLUGIN_NAME} Documentation"
        COMMAND
            ${SPHINX_BIN} ${_args_INPUT} ${_args_OUTPUT} ${SPHINX_OPTS}
        DEPENDS
            ${_args_DEPENDS}
        VERBATIM
    )

    add_custom_target(${_args_TARGET} ALL
        DEPENDS
            "${_args_OUTPUT}"
    )

    if (${_args_DOXYGEN})
        if (NOT doxygen_bin)
            set(doxygen_bin "doxygen")
        endif()

        add_custom_command(
            OUTPUT
                "${_args_INPUT_DIR}/doxyoutput"
            COMMAND
                ${doxygen_bin} "${_args_INPUT_DIR}/Doxyfile"
            WORKING_DIRECTORY
                "${_args_INPUT}"
            DEPENDS
                "${_args_INPUT}/Doxyfile"
            VERBATIM
        )
        add_custom_target("${_args_TARGET}-doxygen"
            DEPENDS
                "${_args_INPUT}/doxyoutput"
        )

        add_dependencies(${_args_TARGET} ${_args_TARGET}-doxygen)
    endif()

endfunction()

function(rtigw_configure_plugin_defines)
    if (RTIGATEWAY_ENABLE_LOG
            OR CMAKE_BUILD_TYPE STREQUAL "Debug"
            OR NOT CMAKE_BUILD_TYPE)
        list(APPEND ${RSPLUGIN_PREFIX}_DEFINES ${RSPLUGIN_PREFIX}_ENABLE_LOG)
    else()
        list(APPEND ${RSPLUGIN_PREFIX}_DEFINES ${RSPLUGIN_PREFIX}_DISABLE_LOG)
    endif()

    if (RTIGATEWAY_ENABLE_TRACE)
        list(APPEND ${RSPLUGIN_PREFIX}_DEFINES ${RSPLUGIN_PREFIX}_ENABLE_TRACE)
    endif()

    if (RTIGATEWAY_ENABLE_SSL)
        list(APPEND ${RSPLUGIN_PREFIX}_DEFINES ${RSPLUGIN_PREFIX}_ENABLE_SSL)
    endif()

    set(${RSPLUGIN_PREFIX}_DEFINES     "${${RSPLUGIN_PREFIX}_DEFINES}"
            CACHE INTERNAL "List compiler defines")
endfunction()

function(rtigw_set_required_variable _OUT_VARIABLE)
    set(candidate_vars ${ARGN})
    foreach(v ${candidate_vars})
        # Give preference to variables passed directly to cmake
        if (DEFINED ${v})
            message(STATUS "${_OUT_VARIABLE} = ${${v}}")
            set(${_OUT_VARIABLE} ${${v}} PARENT_SCOPE)
            return()
        elseif (DEFINED ENV{${v}})
            message(STATUS "${_OUT_VARIABLE} = $ENV{${v}}")
            set(${_OUT_VARIABLE} $ENV{${v}} PARENT_SCOPE)
            return()
        endif()
    endforeach()
    message(FATAL_ERROR
        "Failed to determine value for ${_OUT_VARIABLE}. "
        "Please specify one of '${candidate_vars}' either in your "
        "environment, or as an argument to cmake.")
endfunction()

function(rtigw_init_install_path)
    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX        "${CMAKE_CURRENT_BINARY_DIR}/install")
        message(STATUS "Using default CMAKE_INSTALL_PREFIX = '${CMAKE_INSTALL_PREFIX}'")
    endif()
endfunction()

#[[
rtigw_configure_connextdds
--------------------------

 * Brief: configures RTI Connext DDS by using its FindPackage
 * Params:
 ** _CONNEXTDDS_VERSION: version of RTI Connext DDS to configure
 * Restrictions: CONNEXTDDS_DIR should be set
 * How to use it:

    rtigw_configure_connextdds(6.1.0)
]]

function(rtigw_configure_connextdds _CONNEXTDDS_VERSION)
    # Configure Connext DDS dependencies
    rtigw_set_required_variable(CONNEXTDDS_DIR NDDSHOME CONNEXTDDS_DIR)

    # CONNEXTDDS_ARCH is not required, but recommended
    # eg -DCONNEXTDDS_ARCH=x64Linux4gcc7.3.0
    if (NOT DEFINED CONNEXTDDS_ARCH AND DEFINED ENV{CONNEXTDDS_ARCH})
        set(CONNEXTDDS_ARCH "$ENV{CONNEXTDDS_ARCH}")
    endif()

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

endfunction()

#[[

rtigw_init_globals
------------------

 * Brief: configures all the global parameters used by the RTI Gateway
 * Params: no parameters
 * Restrictions: this should only be used in the main CMakeLists.txt
 * How to use it:

    rtigw_init_globals()
]]

macro(rtigw_init_globals)

    SET(BUILD_SHARED_LIBS ON)

    # This project assumes that C code is C99 and C++ code is C++11. If that
    # shouldn't be the behavior for any target, it might be changed per target.
    #   set_property(TARGET target_name PROPERTY CXX_STANDARD 11)
    #   set_property(TARGET target_name PROPERTY CXX_STANDARD_REQUIRED ON)
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Set common variables
    set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common")
    set(JSON_PARSER_WRAPPER_DIR "${COMMON_DIR}/json_parser")
    set(DDS_COMMON_DIR "${COMMON_DIR}/dds_specific")
    set(TRANSFORMATION_COMMON_DIR "${COMMON_DIR}/transformation")
    set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
    set(LIBMODBUS_DIR "${THIRD_PARTY_DIR}/libmodbus")
    set(PAHO_MQTT_C_DIR "${THIRD_PARTY_DIR}/paho.mqtt.c")
    set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")
    set(UTILS_COMMON_DIR "${COMMON_DIR}/utils")
    set(JSON_PARSER_DIR "${THIRD_PARTY_DIR}/json_parser")
    set(ADAPTERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/adapters")
    set(PROCESSORS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/processors")
    set(TRANSFORMATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/transformations")
    set(EXAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

    set(STAGING_LIB_DIR "lib")
    set(STAGING_BIN_DIR "bin")
    set(STAGING_DOC_DIR "doc")
    set(STAGING_TEST_DIR "test")
    set(STAGING_EXAMPLES_DIR "examples")
    set(STAGING_RESOURCE_DIR "resource")
    set(STAGING_INCLUDE_DIR "include")

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

    rtigw_init_install_path()

    # Only check RTI Connext DDS dependencies if at least one plugin will be built.
    rtigw_configure_connextdds(6.0.1)

    if (RTIGATEWAY_ENABLE_TESTS)
        enable_testing()
    endif()

endmacro()

#[[

rtigw_add_subdirectory_if
-------------------------

 * Brief: adds a subdirectory if it exists
 * Params:
 ** _SUBDIRECTORY_DIR: path to the subdirectory to add
 * How to use it:

    rtigw_add_subdirectory_if("${CMAKE_CURRENT_SOURCE_DIR}/mySubdirectory"
        IF RTIGATEWAY_ENABLE_DOCS)
]]
function(rtigw_add_subdirectory_if _dir)
    set(_BOOLEANS)
    set(_SINGLE_VALUE_ARGS IF)
    set(_MULTI_VALUE_ARGS)

    cmake_parse_arguments(_args
        "${_BOOLEANS}"
        "${_SINGLE_VALUE_ARGS}"
        "${_MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    if (NOT DEFINED _args_IF)
        set(_args_IF OFF)
    endif()

    if (IS_DIRECTORY "${_dir}" AND ${_args_IF})
        add_subdirectory("${_dir}")
    endif()

endfunction()

#[[

rtigw_add_doc
-------------

 * Brief: adds the "doc" directory if it exists in the current source
          directory. This folder is only added if "building doc" is enabled.
 * Params: no parameters
 * How to use it:

    rtigw_add_doc()
]]
function(rtigw_add_doc)
    rtigw_add_subdirectory_if("${CMAKE_CURRENT_SOURCE_DIR}/doc" IF RTIGATEWAY_ENABLE_DOCS)
endfunction()

#[[

rtigw_add_tests
---------------

 * Brief: adds the "test" directory if it exists in the current source
          directory. This folder is only added if testing is enabled.
 * Params: no parameters
 * How to use it:

    rtigw_add_tests()
]]
function(rtigw_add_tests)
    rtigw_add_subdirectory_if("${CMAKE_CURRENT_SOURCE_DIR}/test" IF RTIGATEWAY_ENABLE_TESTS)
endfunction()

#[[

rtigw_add_plugin
----------------

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

    rtigw_add_plugin(
        ENABLE_PLUGIN RTIGATEWAY_ENABLE_MODBUS
        PREFIX "RTI_MODBUS"
        STAGING_DIR "modbus"
        PLUGIN_DIR_NAME "modbus"
        PLUGIN_TYPE "ADAPTER"
    )
]]
function(rtigw_add_plugin)
    set(_BOOLEANS)
    set(_SINGLE_VALUE_ARGS PREFIX STAGING_DIR PLUGIN_DIR_NAME PLUGIN_TYPE ENABLE_PLUGIN)
    set(_MULTI_VALUE_ARGS)

    cmake_parse_arguments(_args
        "${_BOOLEANS}"
        "${_SINGLE_VALUE_ARGS}"
        "${_MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    set(PLUGIN_TYPE_DIR "")
    if (${_args_PLUGIN_TYPE} STREQUAL "ADAPTER")
        set(PLUGIN_TYPE_DIR ${ADAPTERS_DIR})
    elseif(${_args_PLUGIN_TYPE} STREQUAL "PROCESSOR")
        set(PLUGIN_TYPE_DIR ${PROCESSORS_DIR})
    elseif(${_args_PLUGIN_TYPE} STREQUAL "TRANSFORMATION")
        set(PLUGIN_TYPE_DIR ${TRANSFORMATIONS_DIR})
    else()
        mesasge(FATAL_ERROR
            "The plugin ${_args_PLUGIN_DIR_NAME} is not identified
            as ADAPTER, PROCESSOR nor TRANSFORMATION"
        )
    endif()

    if (${_args_ENABLE_PLUGIN})
        set(RSPLUGIN_PREFIX ${_args_PREFIX})
        set(STAGING_PLUGIN_DIR ${_args_STAGING_DIR})
        # Add plugin
        add_subdirectory("${PLUGIN_TYPE_DIR}/${_args_PLUGIN_DIR_NAME}")

        # Add resources
        rtigw_add_subdirectory_if(
            "${RESOURCE_DIR}/${_args_PLUGIN_DIR_NAME}"
        )

        # Add examples if they are enabled
        rtigw_add_subdirectory_if(
            "${EXAMPLES_DIR}/${_args_PLUGIN_DIR_NAME}" IF RTIGATEWAY_ENABLE_EXAMPLES)

    endif()

endfunction()
