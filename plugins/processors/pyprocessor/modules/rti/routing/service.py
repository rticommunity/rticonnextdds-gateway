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

import sys
from ctypes import util, PyDLL

class RoutingService:
    def __init__(self, property={}, **kwargs):
        NotImplemented

    @staticmethod
    def init():
        rs_lib_path = util.find_library('rtirspyprocessor')
        if rs_lib_path is None:
            raise ImportError('RoutingService native library not found.')
        rs_lib = PyDLL(rs_lib_path)
        if  rs_lib.PyService_load() != 0:
            print(sys.last_value)
