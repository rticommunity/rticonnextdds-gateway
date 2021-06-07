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

include("${RESOURCE_DIR}/cmake/common_functions.cmake")

set(EXAMPLE_NAME json_shapes)

# Copy the configuration files to the binary diractory
# List of the files to copy
set(JSON_SHAPES_EXAMPLE_CONFIG_FILES
    "json_to_shapes_transf.xml"
    "shapes_to_json_transf.xml"
)

copy_files(
    CUSTOM_TARGET_NAME JSON_SHAPES_EXAMPLE_CONFIG
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    LIST_OF_FILES_RELATIVE_PATH ${JSON_SHAPES_EXAMPLE_CONFIG_FILES}
)

add_dependencies(${RSPLUGIN_LIB_NAME} JSON_SHAPES_EXAMPLE_CONFIG)

# staging files
# Config files
foreach(FILE ${JSON_SHAPES_EXAMPLE_CONFIG_FILES})
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/${FILE}"
        DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    )
endforeach()

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)
