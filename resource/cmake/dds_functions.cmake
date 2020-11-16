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

function(connextdds_run_codegen)
    set(BOOLEANS)
    set(SINGLE_VALUE_ARGS IDL_FILE LANGUAGE OUTPUT_DIR)
    set(MULTI_VALUE_ARGS INPUT_PARAMS)

    cmake_parse_arguments(_CONNEXTDDS
        "${BOOLEANS}"
        "${SINGLE_VALUE_ARGS}"
        "${MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    if(NOT _CONNEXTDDS_IDL_FILE)
        message(FATAL_ERROR "No IDL_FILE variable when calling codegen.")
    endif()
    if(NOT _CONNEXTDDS_LANGUAGE)
        message(FATAL_ERROR "No LANGUAGE variable when calling codegen.")
    endif()
    if(NOT _CONNEXTDDS_OUTPUT_DIR)
        message(FATAL_ERROR "No OUTPUT_DIR variable when calling codegen.")
    endif()

    if(NOT ${_CONNEXTDDS_LANGUAGE} MATCHES "^C(\\+\\+|$)(11|$)")
        message(FATAL_ERROR "Calling rtiddsgen with language <${_CONNEXTDDS_LANGUAGE}>."
            "This language is unsupported, only C, C++ and C++11 are available")
    endif()

    # Source files to be generated C
    set(GENERATED_C_FILES
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}.c"
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}Plugin.c"
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}Support.c"
    )

    # Source files to be generated C++11
    set(GENERATED_CXX_FILES
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}.cxx"
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}Plugin.cxx"
    )

    # Header files to be generated C and C++
    set(GENERATED_HEADER_C_FILES
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}.h"
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}Plugin.h"
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}Support.h"
    )

    # Header files to be generated C++11
    set(GENERATED_HEADER_CXX_FILES
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}.hpp"
        "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}Plugin.hpp"
    )

    if(${_CONNEXTDDS_LANGUAGE} MATCHES "^C$")
        set(GENERATED_SRC_FILES ${GENERATED_C_FILES})
        set(GENERATED_FILES
            ${GENERATED_C_FILES}
            ${GENERATED_HEADER_C_FILES}
        )
    elseif(${_CONNEXTDDS_LANGUAGE} MATCHES "C\\+\\+$")
        # If we are using classic C++, codegen also generates the TypeSupport
        list(APPEND GENERATED_CXX_FILES
            "${_CONNEXTDDS_OUTPUT_DIR}/${IDL_NAME}Support.cxx"
        )
        set(GENERATED_SRC_FILES ${GENERATED_CXX_FILES})
        set(GENERATED_FILES
            ${GENERATED_CXX_FILES}
            ${GENERATED_HEADER_C_FILES}
        )
    elseif(${_CONNEXTDDS_LANGUAGE} MATCHES "C\\+\\+11$")
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
    add_custom_target(IDL_${IDL_NAME}_CONNEXTDDS_SOURCES
        MAIN_DEPENDENCY
            ${GENERATED_FILES}
    )

    # Command to create the source files
    add_custom_command(
        OUTPUT
            ${GENERATED_FILES}
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${_CONNEXTDDS_OUTPUT_DIR}
        COMMAND
            ${RTICODEGEN}
                -language ${_CONNEXTDDS_LANGUAGE}
                -replace
                -d ${_CONNEXTDDS_OUTPUT_DIR}
                ${_CONNEXTDDS_INPUT_PARAMS}
                ${_CONNEXTDDS_IDL_FILE}
        COMMENT "Generating the type for ${IDL_NAME}"
        MAIN_DEPENDENCY
            ${_CONNEXTDDS_IDL_FILE}
        VERBATIM
    )

    set(GENERATED_SRC_FILES ${GENERATED_SRC_FILES} PARENT_SCOPE)
endfunction()
