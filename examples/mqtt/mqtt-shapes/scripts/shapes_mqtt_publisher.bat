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

set SHAPE_COLOR="CYAN"
if not "%1" == "" (set SHAPE_COLOR=%1)
set SHAPE_TOPIC=%2
set SHAPE_X_MIN=0
set SHAPE_X_MAX=240
set SHAPE_Y_MIN=0
set SHAPE_Y_MAX=240
set SHAPE_SIZE_MIN=30
set SHAPE_SIZE_MAX=60
set SHAPE_PERIOD=3
if defined RTI_MQTTSHAPES_SHAPES_RATE (
  set SHAPE_PERIOD=%RTI_MQTTSHAPES_SHAPES_RATE%
)
set SHAPE_QOS=0

if no %SHAPE_TOPIC% == "" (
  call :publish_topic %SHAPE_TOPIC%
) else (
  call :publish_topic mqtt/circles
)


@REM random_no <min> <max> <ret_value>
:random_no
  set /A %~3=( %RANDOM% * %~2 / 32768 + 1 ) + %~1
  exit /B 0

@REM random_topic <ret_value>
:random_topic
  call :random_no 0,2,number
  if "%number%" == "0" (%~1=mqtt/circles) else ^
  if "%number%" == "1" (%~1=mqtt/squares) else ^
  if "%number%" == "2" (%~1=mqtt/triangles)
  exit /B 0

@REM random_shape <json_output>
:random_shape
  call :random_no %SHAPE_SIZE_MIN%,%SHAPE_SIZE_MAX%,shape_size
  set /A shape_half=%shape_size%/2
  set /A x_min=%SHAPE_X_MIN%+%shape_half%
  set /A x_max=%SHAPE_X_MAX%-%shape_half%
  set /A y_min=%SHAPE_Y_MIN%+%shape_half%
  set /A y_max=%SHAPE_Y_MAX%-%shape_half%
  call :random_no %x_min%,%x_max%,shape_x
  call :random_no %y_min%,%y_max%,shape_y
  set %~1="{ \"color\": \"%SHAPE_COLOR%\" , \"x\": %shape_x%, \"y\": %shape_y%, \"shapesize\": %shape_size% }"
  exit /B 0

@REM random_period <ret_value>
:random_period
  call :random_no 1,5,%~1
  exit /B 0

@REM publish_topic <topic>
:publish_topic
  setlocal
  set topic=%1
  @echo STARTED publishing topic=%topic%, time=%DATE% %TIME%

  :loop
      call :random_shape shape
      @REM mosquitto_pub -q 2 -r -t %topic% -m %shape%
      @echo PUBLISHING[%topic%][q=%SHAPE_QOS%]: %shape%
      call mosquitto_pub -r -q %SHAPE_QOS% -t %topic% -m %shape%
      @REM :random_period sleep_period
      @REM timeout /nobreak /t %sleep_period%
      timeout /nobreak /t %SHAPE_PERIOD% > NUL
  if errorlevel == 0 goto loop
  endlocal
  exit /B 0
