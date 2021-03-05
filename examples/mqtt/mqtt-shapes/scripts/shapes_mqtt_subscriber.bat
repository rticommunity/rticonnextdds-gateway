@REM ###########################################################################
@REM (c) 2021 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
@REM
@REM RTI grants Licensee a license to use, modify, compile, and create
@REM derivative works of the software solely for use with RTI Connext DDS.
@REM Licensee may redistribute copies of the software provided that all such
@REM copies are subject to this license.
@REM The software is provided "as is", with no warranty of any type, including
@REM any warranty for fitness for any purpose. RTI is under no obligation to
@REM maintain or support the software.  RTI shall not be liable for any
@REM incidental or consequential damages arising out of the use or inability to
@REM use the software.
@REM ###########################################################################

@echo off

set SHAPE_TOPIC="#"
if not "%1" == "" (set SHAPE_TOPIC=%1)

call mosquitto_sub -t %SHAPE_TOPIC% -v
