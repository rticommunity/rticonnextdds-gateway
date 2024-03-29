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

include("${RTICMAKE_COMMON_DIR}/ConfigureThirdPartyBuilds.cmake")

# Change the CMAKE_INSTALL variables to install those files under third-party
# directory. In this way, the third-party object are differentiated from the
# RTI Connext Gateway objects.
# The third-party libraries are 'installed' in the lib directory, outside
# the third-party/ folder because the plugins do have a direct dependency
# with them.

if (CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_BINDIR "third-party/bin")
    set(CMAKE_INSTALL_INCLUDEDIR "third-party/include")
    set(CMAKE_INSTALL_DOCDIR "third-party/share/doc")
endif()

if (RTIGATEWAY_ENABLE_MODBUS)
    add_subdirectory("${LIBMODBUS_DIR}")
endif()


if (RTIGATEWAY_ENABLE_MQTT)
    rtigw_configure_paho_build()

    add_subdirectory("${PAHO_MQTT_C_DIR}")
endif()


if (RTIGATEWAY_ENABLE_KAFKA)
    rtigw_configure_rd_kafka_build()

    add_subdirectory("${LIBRD_KAFKA_C_DIR}")
endif()

# The following subdirectory should copy dlls that are in the bin directory to
# lib folder, that is where the dlls should be. This has to be the last thing
# we do in this CMakeLists.txt file, because it needs the dlls to be created
# and installed before copying them.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/copy_dependencies")
