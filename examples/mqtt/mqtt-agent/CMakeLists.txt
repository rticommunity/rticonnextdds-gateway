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

project(rtiroutingservice-adapter-mqtt-examples-agent C)
cmake_minimum_required(VERSION 3.7.0)

###############################################################################

set(MQTTAGENT_SOURCE_C
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/MqttAgent.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/MqttAgentSubscriber.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/srcC/MqttAgentPublisher.c")

set(EXAMPLE_NAME mqtt-agent)

add_executable(${EXAMPLE_NAME}
    ${MQTTAGENT_SOURCE_C})


target_link_libraries(${EXAMPLE_NAME}
    PRIVATE
        rtimqttadapter
)

target_include_directories(${EXAMPLE_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/srcC"
)

target_compile_definitions(${EXAMPLE_NAME}
    PUBLIC
        ${MQTT_DEFINES_OUT}
)

# staging files
install(
    TARGETS ${EXAMPLE_NAME}
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE
)

install(
    DIRECTORY "etc"
    DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}/${EXAMPLE_NAME}"
)

###############################################################################
