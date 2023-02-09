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
 ** INPUT_LIST: list to add the prefix to all its elements
 ** PREFIX: prefix to add
 ** _OUT_VARIABLE: output list that will contain the list with the prefix

 * How to use it:

    rtigw_list_prefix(
        ${INPUT_LIST}
        "${CMAKE_CURRENT_SOURCE_DIR}/"
        _files_input
    )
]]

function(rtigw_list_prefix INPUT_LIST PREFIX _OUT_VARIABLE)
    set(_result)
    foreach(_val ${INPUT_LIST})
        list(APPEND _result "${PREFIX}${_val}")
    endforeach()
    set(${_OUT_VARIABLE} ${_result} PARENT_SCOPE)
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
            "Generating ${_args_PLUGIN} Documentation"
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
                "${_args_INPUT}/doxyoutput"
            COMMAND
                ${doxygen_bin} "${_args_INPUT}/Doxyfile"
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

macro(rtigw_init_install_path)
    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX        "${CMAKE_CURRENT_BINARY_DIR}/install")
        message(STATUS "Using default CMAKE_INSTALL_PREFIX = '${CMAKE_INSTALL_PREFIX}'")
    endif()
endmacro()

#[[
rtigw_configure_connextdds
--------------------------

 * Brief: configures RTI Connext DDS by using its FindPackage
 * Params:
 ** _CONNEXTDDS_VERSION: version of RTI Connext DDS to configure
 * Preconditions: FindRTIConnextDDS.cmake should be added to CMAKE_MODULE_PATH
 * How to use it:

    rtigw_configure_connextdds(6.1.0)
]]

function(rtigw_configure_connextdds _CONNEXTDDS_VERSION)
    # CONNEXTDDS_ARCH is not required, but recommended
    # eg -DCONNEXTDDS_ARCH=x64Linux4gcc7.3.0
    if (NOT DEFINED CONNEXTDDS_ARCH AND DEFINED ENV{CONNEXTDDS_ARCH})
        set(CONNEXTDDS_ARCH "$ENV{CONNEXTDDS_ARCH}")
    endif()

    if (DEFINED CONNEXTDDS_ARCH)
        message(STATUS "CONNEXTDDS_ARCH set to <${CONNEXTDDS_ARCH}>")
    endif()

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

 * Brief: configures all the global parameters used by the RTI Connext Gateway
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
    set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")
    set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
    set(ADAPTERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/adapters")
    set(PROCESSORS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/processors")
    set(TRANSFORMATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/transformations")
    set(EXAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

    set(LIBMODBUS_DIR "${THIRD_PARTY_DIR}/libmodbus")
    set(PAHO_MQTT_C_DIR "${THIRD_PARTY_DIR}/paho.mqtt.c")
    set(LIBRD_KAFKA_C_DIR "${THIRD_PARTY_DIR}/librdkafka")
    set(JSON_PARSER_DIR "${THIRD_PARTY_DIR}/json_parser")
    set(UTILS_COMMON_DIR "${COMMON_DIR}/utils")
    set(JSON_PARSER_WRAPPER_DIR "${COMMON_DIR}/json_parser")
    set(TRANSFORMATION_COMMON_DIR "${COMMON_DIR}/transformation")
    set(DDS_COMMON_DIR "${COMMON_DIR}/dds_specific")
    set(RTICMAKE_COMMON_DIR "${RESOURCE_DIR}/cmake")
    set(RTICMAKE_UTILS_MODULES_DIR "${THIRD_PARTY_DIR}/rticonnextdds-cmake-utils/cmake/Modules")

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
    option(RTIGATEWAY_ENABLE_KAFKA "Build KAFKA Adapter" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_FWD "Build Forwarder Processor" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_TSFM_FIELD "Build Field Transformation" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_TSFM_JSON "Build JSON Transformation" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_TSFM_SEQUENCE2ARRAY "Build Sequence2Array Transformation" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_TESTS "Build tester applications for enabled plugins" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_EXAMPLES "Build examples applications for enabled plugins" ${RTIGATEWAY_ENABLE_ALL})
    option(RTIGATEWAY_ENABLE_DOCS "Build documentation for enabled plugins" OFF)
    option(RTIGATEWAY_ENABLE_SSL "Enable support for SSL/TLS" OFF)
    option(RTIGATEWAY_ENABLE_LOG "Enable logging to stdout" OFF)
    option(RTIGATEWAY_ENABLE_TRACE "Enable support for trace-level logging" OFF)

    rtigw_init_install_path()

    list(APPEND CMAKE_MODULE_PATH "${RTICMAKE_UTILS_MODULES_DIR}")

    rtigw_configure_connextdds(6.1.2)

    if (RTIGATEWAY_ENABLE_TESTS)
        enable_testing()
    endif()

endmacro()

#[[

rtigw_add_subdirectory_if
-------------------------

 * Brief: adds a subdirectory if it exists depending of the variable value of
          `IF`. If IF is OFF, it won't be added. If IF is not defined or ON,
          the directory will be added.
 * Params:
 ** _dir: path to the subdirectory to add
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
        set(_args_IF ON)
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
 * Preconditions: CONNEXTDDS_DIR or NDDSHOME environment variable should be set
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
            "${EXAMPLES_DIR}/${_args_PLUGIN_DIR_NAME}" IF RTIGATEWAY_ENABLE_EXAMPLES
        )

    endif()

endfunction()
