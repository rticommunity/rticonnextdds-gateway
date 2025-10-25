#  (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
#
#  RTI grants Licensee a license to use, modify, compile, and create
#  derivative works of the software solely for use with RTI Connext DDS.
#  Licensee may redistribute copies of the software provided that all such
#  copies are subject to this license.
#  The software is provided "as is", with no warranty of any type, including
#  any warranty for fitness for any purpose. RTI is under no obligation to
#  maintain or support the software.  RTI shall not be liable for any
#  incidental or consequential damages arising out of the use or inability to
#  use the software.
import sys

class TestLog:
    PREFIX = ""

    @classmethod
    def set_prefix(cls, prefix: str) -> None:
        cls.PREFIX = prefix
    
    @classmethod
    def log(cls, level: str, message: str, *args) -> None:
        print(f"[{level}] {cls.PREFIX}{message.format(*args)}", file=sys.stderr)

    @classmethod
    def info(cls, message: str, *args) -> None:
        cls.log("INFO", message, *args)
    
    @classmethod
    def warning(cls, message: str, *args) -> None:
        cls.log("WARNING", message, *args)
    
    @classmethod
    def error(cls, message: str, *args) -> None:
        cls.log("ERROR", message, *args)
    
    @classmethod
    def debug(cls, message: str, *args) -> None:
        cls.log("DEBUG", message, *args)

