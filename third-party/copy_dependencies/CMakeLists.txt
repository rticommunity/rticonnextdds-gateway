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

# This is done in oder to copy dlls files to the correct location. In Windows,
# these libraries may be installed in the 'bin' directory, with the executables,
# however, they should be in the lib directory.
# Adding a subdirectory is the only way of running a post install script.
# This happens because the cmake_install.cmake always puts the add_subdirectory
# calls at the end, even if the 'install(CODE ...' is defined after the
# add_subdirectory calls.
# We need to create and install the dlls before copying them.

# This is required because in Windows, libraries (.lib) and runtimes (.dll) may
# have different installation folders.

include("${RESOURCE_DIR}/cmake/ConfigureThirdPartyBuilds.cmake")

rtigw_install_dependencies_dll()
