#!/bin/sh
#
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


SHAPE_COLOR="${1:-CYAN}"
SHAPE_TOPIC="${2}"
SHAPE_X_MIN=0
SHAPE_X_MAX=240
SHAPE_Y_MIN=0
SHAPE_Y_MAX=240
SHAPE_SIZE_MIN=30
SHAPE_SIZE_MAX=60
SHAPE_PERIOD="${RTI_MQTTSHAPES_SHAPES_RATE:-3}"
SHAPE_QOS=0
SHAPE_RETAINED=

random_no()
{
    shuf -i ${1}-${2} -n 1
}

random_topic()
{
  case $(random_no 0 2) in
  0) echo "mqtt/circles"
     ;;
  1) echo "mqtt/squares"
     ;;
  2) echo "mqtt/triangles"
     ;;
  esac
}

random_shape()
{
  shape_size=$(random_no "${SHAPE_SIZE_MIN}" "${SHAPE_SIZE_MAX}")
  shape_half=$(expr ${shape_size} / 2)
  x_min=$(expr ${SHAPE_X_MIN} + ${shape_half})
  x_max=$(expr ${SHAPE_X_MAX} - ${shape_half})
  y_min=$(expr ${SHAPE_Y_MIN} + ${shape_half})
  y_max=$(expr ${SHAPE_Y_MAX} - ${shape_half})
  shape_x=$(random_no ${x_min} ${x_max})
  shape_y=$(random_no ${y_min} ${y_max})
  shape_json="$(printf '{ "color": "%s", "x": %d, "y": %d, "shapesize": %d }' \
                       ${SHAPE_COLOR} ${shape_x} ${shape_y} ${shape_size})"
  printf '%s' "${shape_json}"
}

publish_topic()
{
  topic="${1}"
  printf "STARTED publishing topic=%s, time=%d\n" \
        "${topic}" "$(date +%s)"
  while :; do
      shape="$(random_shape)"
      printf 'PUBLISHING[%s][q=%d]: %s\n' \
              "${topic}" "${SHAPE_QOS}" "${shape}" >&2
      printf "%s\n" "${shape}"
      sleep ${SHAPE_PERIOD}
  done | mosquitto_pub -l \
                       $([ -z "${SHAPE_RETAINED}" ] || printf "%s" "-r") \
                       -q ${SHAPE_QOS} \
                       -t "${topic}"
}

publish_topic "${SHAPE_TOPIC}"
