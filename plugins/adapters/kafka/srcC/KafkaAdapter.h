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

#ifndef KafkaAdapter_h
#define KafkaAdapter_h

#include "rtiadapt_kafka.h"

#include "KafkaConnection.h"
#include "KafkaStreamWriter.h"
#include "KafkaStreamReader.h"


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

struct RTI_RS_KafkaAdapterPlugin {
    struct RTI_RoutingServiceAdapterPlugin base;
};

DllExport struct RTI_RoutingServiceAdapterPlugin *
RTI_RS_Kafka_AdapterPlugin_create(
    const struct RTI_RoutingServiceProperties *properties,
    RTI_RoutingServiceEnvironment *env);

void RTI_RS_KafkaAdapterPlugin_delete(
    struct RTI_RoutingServiceAdapterPlugin *adapter,
    RTI_RoutingServiceEnvironment *env);

RTI_RoutingServiceConnection
RTI_RS_KafkaAdapterPlugin_create_connection(
    struct RTI_RoutingServiceAdapterPlugin *adapter,
    const char *routing_service_name,
    const char *routing_service_group_name,
    const struct RTI_RoutingServiceStreamReaderListener *input_disc_listener,
    const struct RTI_RoutingServiceStreamReaderListener *output_disc_listener,
    const struct RTI_RoutingServiceTypeInfo **registeredTypes,
    int registeredTypeCount,
    const struct RTI_RoutingServiceProperties *properties,
    RTI_RoutingServiceEnvironment *env);

void RTI_RS_KafkaAdapterPlugin_delete_connection(
    struct RTI_RoutingServiceAdapterPlugin *adapter,
    RTI_RoutingServiceConnection connection,
    RTI_RoutingServiceEnvironment *env);

#endif /* KafkaAdapter_h */
