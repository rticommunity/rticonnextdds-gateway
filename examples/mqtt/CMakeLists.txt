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

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mqtt-agent")

# Shapes example uses other plugins different to the MQTT adapter and all of
# them should be enabled to add this example
if (RTIGATEWAY_ENABLE_MQTT
        AND RTIGATEWAY_ENABLE_TSFM_FIELD
        AND RTIGATEWAY_ENABLE_TSFM_JSON)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mqtt-shapes")
endif()