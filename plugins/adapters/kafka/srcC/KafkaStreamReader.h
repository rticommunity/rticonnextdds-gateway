/*
 * (c) 2021 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */

#ifndef KafkaStreamReader_h
#define KafkaStreamReader_h

#include "rtiadapt_kafka.h"

/* Kafka C library */
#include "rdkafka.h"

#ifdef RTI_WIN32
  #define DllExport __declspec( dllexport )
  #include <Winsock2.h>
  #include <process.h>
#else
  #define DllExport
  #include <sys/select.h>
  #include <semaphore.h>
  #include <pthread.h> 
#endif

struct RTI_RS_KafkaStreamReader
{
    rd_kafka_t *rk;  /* rdkafka consumer instance handle */
    rd_kafka_conf_t *conf;  /* rdkafka ponfiguration object */
    const char *topic;  /* Topic to consume */
    pthread_t thread;
    int run;
    struct RTI_RoutingServiceStreamReaderListener listener;
    struct DDS_DynamicData     **sample_list;
    struct DDS_SampleInfo      **info_list;
    struct DDS_TypeCode *type_code;
    //struct DDS_Duration_t polling_period;
};

void RTI_RS_KafkaStreamReader_read(
    RTI_RoutingServiceStreamReader stream_reader,
    RTI_RoutingServiceSample **sample_list,
    RTI_RoutingServiceSampleInfo **info_list,
    int *count,
    RTI_RoutingServiceEnvironment *env);

void RTI_RS_KafkaStreamReader_return_loan(
    RTI_RoutingServiceStreamReader stream_reader,
    RTI_RoutingServiceSample *sample_list,
    RTI_RoutingServiceSampleInfo *info_list,
    int count,
    RTI_RoutingServiceEnvironment *env);

void *RTI_RS_KafkaStreamReader_run(void *threadParam);

#endif /* KafkaStreamReader_h */
