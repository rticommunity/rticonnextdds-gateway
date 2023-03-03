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

macro (rtigw_configure_paho_build)
    set(PAHO_BUILD_DOCUMENTATION        FALSE
        CACHE INTERNAL "Enable building Paho (C Posix)'s documentation"
        FORCE)
    set(PAHO_BUILD_SAMPLES              FALSE
        CACHE INTERNAL "Enable building Paho (C Posix)'s sample programs"
        FORCE)
    set(PAHO_BUILD_DEB_PACKAGE          FALSE
        CACHE INTERNAL "Enable building Paho (C Posix)'s Debian package"
        FORCE)
    set(PAHO_ENABLE_TESTING             FALSE
        CACHE INTERNAL "Enable building and running Paho (C Posix)'s tests"
        FORCE)
    set(PAHO_ENABLE_CPACK               FALSE
        CACHE INTERNAL "Enable use of CPack to generate Paho (C Posix) bundles"
        FORCE)
    set(PAHO_WITH_SSL ${RTIGATEWAY_ENABLE_SSL}
        CACHE INTERNAL "Enable building Paho (C Posix) with OpenSSL support"
        FORCE)
endmacro()

macro (rtigw_copy_dll_from_third_party_bin_to_lib _LIBNAME)
    install(
        CODE "file(COPY \"${ABSOLUTE_INSTALL_PREFIX}/third-party/bin/${_LIBNAME}.dll\"
                DESTINATION \"${ABSOLUTE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}\")")
endmacro()

macro (rtigw_install_dependencies_dll)
    if(WIN32)
        get_filename_component(ABSOLUTE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} ABSOLUTE)
        install(CODE "set(ABSOLUTE_INSTALL_PREFIX \"${ABSOLUTE_INSTALL_PREFIX}\")")

        if(RTIGATEWAY_ENABLE_MQTT)
            if (${RTIGATEWAY_ENABLE_SSL})
                rtigw_copy_dll_from_third_party_bin_to_lib("paho-mqtt3as")
                rtigw_copy_dll_from_third_party_bin_to_lib("paho-mqtt3cs")
            else()
                rtigw_copy_dll_from_third_party_bin_to_lib("paho-mqtt3a")
                rtigw_copy_dll_from_third_party_bin_to_lib("paho-mqtt3c")
            endif()
        endif()

        if(RTIGATEWAY_ENABLE_KAFKA)
            rtigw_copy_dll_from_third_party_bin_to_lib("rdkafka")
            rtigw_copy_dll_from_third_party_bin_to_lib("rdkafka++")
        endif()
    endif()
endmacro()

macro (rtigw_configure_rd_kafka_build)
    set(RDKAFKA_BUILD_EXAMPLES        FALSE
        CACHE INTERNAL "Enable building RD Kafka's documentation"
        FORCE)
    set(RDKAFKA_BUILD_TESTS              FALSE
        CACHE INTERNAL "Enable building RD Kafka's sample programs"
        FORCE)
    set(WITH_BUNDLED_SSL FALSE
        CACHE INTERNAL "Enable building RD Kafka with OpenSSL support"
        FORCE)
    set(WITH_SSL ${RTIGATEWAY_ENABLE_SSL}
        CACHE INTERNAL "Enable building RD Kafka with OpenSSL support"
        FORCE)
endmacro()
