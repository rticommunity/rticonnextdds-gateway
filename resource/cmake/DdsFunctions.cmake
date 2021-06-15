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

#[[
connextdds_rtiddsgen_run
----------------

 * Brief: generate code using rtiddsgen from an input IDL file
 * Params:
 ** IDL_FILE: the IDL filename that will be used to generate code.
 ** LANG: the language to generate source files for. Only C, C++ and C++11 are
          currently supported.
 ** OUTPUT_DIR: the directory where to put generated files.
 ** FILES: list of files to be copied with relative path form
           ${CMAKE_CURRENT_SOURCE_DIR}.

 * The variable IDL_NAME shall be defined with the name of the IDL (wihtout
   extension)

 * How to use it:

    connextdds_rtiddsgen_run(
        IDL_FILE "${IDL_DIR}/${IDL_NAME}.idl"
        LANG "C++11"
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/idl"
        EXTRA_ARGS
            "-unboundedSupport"
            "-qualifiedEnumerator"
    )
]]

function(connextdds_rtiddsgen_run)
    set(_BOOLEANS)
    set(_SINGLE_VALUE_ARGS IDL_FILE LANG OUTPUT_DIR)
    set(_MULTI_VALUE_ARGS EXTRA_ARGS)

    cmake_parse_arguments(_args
        "${_BOOLEANS}"
        "${_SINGLE_VALUE_ARGS}"
        "${_MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    if (NOT _args_IDL_FILE)
        message(FATAL_ERROR "No IDL_FILE variable when calling codegen.")
    endif()
    if (NOT _args_LANG)
        message(FATAL_ERROR "No LANG variable when calling codegen.")
    endif()
    if (NOT _args_OUTPUT_DIR)
        message(FATAL_ERROR "No OUTPUT_DIR variable when calling codegen.")
    endif()

    if (NOT ${_args_LANG} MATCHES "^C(\\+\\+|$)(11|$)")
        message(FATAL_ERROR "Calling rtiddsgen with language <${_args_LANG}>."
            "This language is unsupported, only C, C++ and C++11 are available")
    endif()

    # Source files to be generated C
    set(GENERATED_C_FILES
        "${_args_OUTPUT_DIR}/${IDL_NAME}.c"
        "${_args_OUTPUT_DIR}/${IDL_NAME}Plugin.c"
        "${_args_OUTPUT_DIR}/${IDL_NAME}Support.c"
    )

    # Source files to be generated C++11
    set(GENERATED_CXX_FILES
        "${_args_OUTPUT_DIR}/${IDL_NAME}.cxx"
        "${_args_OUTPUT_DIR}/${IDL_NAME}Plugin.cxx"
    )

    # Header files to be generated C and C++
    set(GENERATED_HEADER_C_FILES
        "${_args_OUTPUT_DIR}/${IDL_NAME}.h"
        "${_args_OUTPUT_DIR}/${IDL_NAME}Plugin.h"
        "${_args_OUTPUT_DIR}/${IDL_NAME}Support.h"
    )

    # Header files to be generated C++11
    set(GENERATED_HEADER_CXX_FILES
        "${_args_OUTPUT_DIR}/${IDL_NAME}.hpp"
        "${_args_OUTPUT_DIR}/${IDL_NAME}Plugin.hpp"
    )

    if (${_args_LANG} MATCHES "^C$")
        set(GENERATED_SRC_FILES ${GENERATED_C_FILES})
        set(GENERATED_FILES
            ${GENERATED_C_FILES}
            ${GENERATED_HEADER_C_FILES}
        )
    elseif(${_args_LANG} MATCHES "C\\+\\+$")
        # If we are using classic C++, codegen also generates the TypeSupport
        list(APPEND GENERATED_CXX_FILES
            "${_args_OUTPUT_DIR}/${IDL_NAME}Support.cxx"
        )
        set(GENERATED_SRC_FILES ${GENERATED_CXX_FILES})
        set(GENERATED_FILES
            ${GENERATED_CXX_FILES}
            ${GENERATED_HEADER_C_FILES}
        )
    elseif(${_args_LANG} MATCHES "C\\+\\+11$")
        set(GENERATED_SRC_FILES ${GENERATED_CXX_FILES})
        set(GENERATED_FILES
            ${GENERATED_CXX_FILES}
            ${GENERATED_HEADER_CXX_FILES}
        )
    endif()

    # Create a target to generate the source files
    # Instead of creating here the source files using the COMMAND parameter
    # from add_custom_target, we create a CMake custom command. The reason is
    # the COMMAND from add_custom_target will be executed always and the
    # custom command will be executed only if the IDL file was modified
    add_custom_target(idl-${IDL_NAME}-connextdds-sources
        MAIN_DEPENDENCY
            ${GENERATED_FILES}
    )

    # Command to create the source files
    add_custom_command(
        OUTPUT
            ${GENERATED_FILES}
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${_args_OUTPUT_DIR}
        COMMAND
            ${RTICODEGEN}
                -language ${_args_LANG}
                -replace
                -d ${_args_OUTPUT_DIR}
                ${_args_EXTRA_ARGS}
                ${_args_IDL_FILE}
        COMMENT "Generating the type for ${IDL_NAME}"
        MAIN_DEPENDENCY
            ${_args_IDL_FILE}
        VERBATIM
    )

    set(GENERATED_SRC_FILES ${GENERATED_SRC_FILES} PARENT_SCOPE)
endfunction()
