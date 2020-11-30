import sys
from ctypes import util, PyDLL

class RoutingService:
    def __init__(self, property={}, **kwargs):
        NotImplemented

    @staticmethod
    def init():
        rs_lib_path = util.find_library('rtipyroutingserviced')
        if rs_lib_path is None:
            raise ImportError('RoutingService native library not found.')
        rs_lib = PyDLL(rs_lib_path)
        if  rs_lib.PyService_load() != 0:
            print(sys.last_value)
