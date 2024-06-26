###############################################################################
#  (c) 2024 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. #
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

include("${RESOURCE_DIR}/cmake/CommonFunctions.cmake")

set(EXAMPLE_NAME polygon2)
set(PUBLISHER_NAME PolygonArray)
set(SUBSCRIBER_NAME PolygonSeq)

# Publisher
# Generate codegen files
connextdds_rtiddsgen_run(
    IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/publisher/${PUBLISHER_NAME}.idl"
    LANG "C++11"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/publisher"
)

add_executable(${PUBLISHER_NAME}_publisher
    "${CMAKE_CURRENT_SOURCE_DIR}/publisher/${PUBLISHER_NAME}_publisher.cxx"
    "${${PUBLISHER_NAME}_CXX11_SOURCES}"
)

target_include_directories(${PUBLISHER_NAME}_publisher
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/publisher"
)

target_link_libraries(${PUBLISHER_NAME}_publisher
    RTIConnextDDS::cpp2_api
)

# Subscriber
# Generate codegen files
connextdds_rtiddsgen_run(
    IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/subscriber/${SUBSCRIBER_NAME}.idl"
    LANG "C++11"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/subscriber"
)

add_executable(${SUBSCRIBER_NAME}_subscriber
    "${CMAKE_CURRENT_SOURCE_DIR}/subscriber/${SUBSCRIBER_NAME}_subscriber.cxx"
    "${${SUBSCRIBER_NAME}_CXX11_SOURCES}"
)

target_include_directories(${SUBSCRIBER_NAME}_subscriber
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/subscriber"
)

target_link_libraries(${SUBSCRIBER_NAME}_subscriber
    RTIConnextDDS::cpp2_api
)

# staging files
# Config files

set(SEQUENCE2ARRAY_TRANSFORMATION_INSTALL_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/RsArray2SequenceTransf.xml"
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
)

install(
    FILES ${SEQUENCE2ARRAY_TRANSFORMATION_INSTALL_FILES}
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

install(
    TARGETS ${PUBLISHER_NAME}_publisher
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}/publisher"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE
)

install(
    TARGETS ${SUBSCRIBER_NAME}_subscriber
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}/subscriber"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/publisher/${PUBLISHER_NAME}.idl"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}/publisher"
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/subscriber/${SUBSCRIBER_NAME}.idl"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}/subscriber"
)
