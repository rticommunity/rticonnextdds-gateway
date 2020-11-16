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

macro (configure_paho_build)
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
