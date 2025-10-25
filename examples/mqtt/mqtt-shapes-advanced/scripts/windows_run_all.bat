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

(
    START mosquitto -c etc/mosquitto/mosquitto.conf -p 1883
    START scripts\shapes_mqtt_publisher.bat GREEN
    START scripts\shapes_mqtt_subscriber.bat "#"
    START mqtt-shapes.exe
    START rtishapesdemo -compact -workspaceFile etc/shapes_demo_workspace.xml -dataType Shape -pubInterval 1000
    START rtiroutingservice -cfgFile etc/shapes_bridge.xml -cfgName shapes_bridge
) | pause
