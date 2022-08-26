###############################################################################
#  (c) 2022 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. #
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

cmake_minimum_required(VERSION 3.12.0)

# Set variables
set(RSPLUGIN_LIB_NAME rtisequence2arraytransf)

# Create library
add_library(
    ${RSPLUGIN_LIB_NAME}
    SHARED
        "srcCxx/Sequence2ArrayTransformation.cxx"
        "${DDS_COMMON_DIR}/srcCxx/DynamicDataHelpers.cxx"
)

set_target_properties(${RSPLUGIN_LIB_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(
    ${RSPLUGIN_LIB_NAME}
    PRIVATE
        ${CONNEXTDDS_INCLUDE_DIRS}
        "srcCxx/"
        "${DDS_COMMON_DIR}/srcCxx"
        "${UTILS_COMMON_DIR}/srcCxx"
)

target_link_libraries(
    ${RSPLUGIN_LIB_NAME}
    RTIConnextDDS::routing_service_cpp2
)

# Stagging targets
install(
    TARGETS ${RSPLUGIN_LIB_NAME}
    DESTINATION "${STAGING_LIB_DIR}"
)

###############################################################################
# Build documentation and tests if they are enabled
###############################################################################
rtigw_add_doc()
rtigw_add_tests()
