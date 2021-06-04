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

cmake_minimum_required (VERSION 3.5)

include("${RESOURCE_DIR}/cmake/CommonFunctions.cmake")

generate_doc(
    PLUGIN_NAME "Field Transformation"
    CUSTOM_TARGET_NAME "FIELD_TRANSFORMATION_DOC"
    INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/html"
    DEPENDENCIES
        "${CMAKE_CURRENT_SOURCE_DIR}/conf.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/additional_resources.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/configuration.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/copyright.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/index.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/introduction.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/release_notes.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/static/"
)

# staging documentation
install(
    DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/"
    DESTINATION "${STAGING_DOC_DIR}/transformations/${STAGING_PLUGIN_DIR}"
)
