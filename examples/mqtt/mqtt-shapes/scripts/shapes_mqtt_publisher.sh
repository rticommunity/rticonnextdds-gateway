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

# random_no <min> <max>
random_no()
{
    # awk -v min=${1} -v max=${2} "BEGIN{srand($(date +%s)); print int(min+rand()*(max-min+1))}"
    # sleep 0.5
    shuf -i ${1}-${2} -n 1
}

# random_topic
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

# random_shape
random_shape()
{
  shape_size=$(random_no "${SHAPE_SIZE_MIN}" "${SHAPE_SIZE_MAX}")
  shape_half=$(expr ${shape_size} / 2)
  x_min=$(expr ${SHAPE_X_MIN} + ${shape_half})
  x_max=$(expr ${SHAPE_X_MAX} - ${shape_half})
  y_min=$(expr ${SHAPE_Y_MIN} + ${shape_half})
  y_max=$(expr ${SHAPE_Y_MAX} - ${shape_half})
  shape_x=$(random_no ${x_min} ${x_max})
  # sleep 0.5
  shape_y=$(random_no ${y_min} ${y_max})
  shape_json="$(printf '{ "color": "%s", "x": %d, "y": %d, "shapesize": %d }' \
                       ${SHAPE_COLOR} ${shape_x} ${shape_y} ${shape_size})"
  printf '%s' "${shape_json}"
}

random_period()
{
  printf "%d.%d" \
    0 \
    $(random_no 1 1000)
    # $(random_no 0 ${SHAPE_PERIOD}) 
}

publish_topic()
{
  topic="${1}"
  printf "STARTED publishing topic=%s, time=%d\n" \
        "${topic}" "$(date +%s)"
  while :; do
      shape="$(random_shape)"
      # mosquitto_pub -q 2 -r -t "${topic}" -m "${shape}"
      printf 'PUBLISHING[%s][q=%d]: %s\n' \
              "${topic}" "${SHAPE_QOS}" "${shape}" >&2
      printf "%s\n" "${shape}"
      # sleep_period="$(random_period)"
      # sleep ${sleep_period}
      sleep ${SHAPE_PERIOD}
  # done
  done | mosquitto_pub -l \
                       $([ -z "${SHAPE_RETAINED}" ] || printf "%s" "-r") \
                       -q ${SHAPE_QOS} \
                       -t "${topic}" 
}

kill_publishers()
{
  printf "killing child processes: ${PUB_PIDS}"
  kill ${PUB_PIDS}
  ps aux  | grep "/bin/sh ./bin/shapes_mqtt_publisher.sh ${SHAPE_COLOR}" | 
    grep -v grep | awk '{print $2;}' | xargs kill
}

trap kill_publishers INT

PUB_PIDS=""

if [ -n "${SHAPE_TOPIC}" ]; then
  publish_topic "${SHAPE_TOPIC}" &
  PUB_PIDS="$! ${PUB_PIDS}"
else
  publish_topic "mqtt/circles" &
  PUB_PIDS="$! ${PUB_PIDS}"
  publish_topic "mqtt/squares" &
  PUB_PIDS="$! ${PUB_PIDS}"
  publish_topic "mqtt/triangles" &
  PUB_PIDS="$! ${PUB_PIDS}"
fi


wait

