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
