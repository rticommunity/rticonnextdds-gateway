###############################################################################
#  (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. #
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

set(RSPLUGIN_LIB_NAME "rtidatadiodeadapter")

add_library(${RSPLUGIN_LIB_NAME}
    SHARED
        "${DDS_COMMON_DIR}/srcCxx/DynamicDataHelpers.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/SocketAdapter.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/SocketConnection.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/SocketInputDiscoveryStreamReader.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/SocketStreamWriter.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/SocketStreamReader.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx/UdpSocketUtils.cxx"
)

set_target_properties(${RSPLUGIN_LIB_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(${RSPLUGIN_LIB_NAME}
    PRIVATE
        ${CONNEXTDDS_INCLUDE_DIRS}
        "${CMAKE_CURRENT_SOURCE_DIR}/srcCxx"
        "${UTILS_COMMON_DIR}/srcCxx"
        "${DDS_COMMON_DIR}/srcCxx"
        "${JSON_PARSER_WRAPPER_DIR}/srcCxx"
        "${JSON_PARSER_DIR}/"
)

target_link_libraries(${RSPLUGIN_LIB_NAME}
    PRIVATE
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
