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

include("${RTICMAKE_COMMON_DIR}/CommonFunctions.cmake")

rtigw_generate_doc(
    PLUGIN "Modbus Adapter"
    TARGET modbus-adapter-doc
    INPUT "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/conf.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/additional_resources.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/configuration.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/copyright.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/index.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/introduction.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/release_notes.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/vars.rst"
        "${CMAKE_CURRENT_SOURCE_DIR}/_static/"
)

# staging documentation
install(
    DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/"
    DESTINATION "${STAGING_DOC_HTML_DIR}/adapters/${STAGING_PLUGIN_DIR}"
)

install(
    DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pdf/latex/"
    DESTINATION "${STAGING_DOC_PDF_DIR}/adapters/${STAGING_PLUGIN_DIR}"
    FILES_MATCHING PATTERN "*.pdf"
)
