/******************************************************************************/
/* (c) 2021 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
/*                                                                            */
/* RTI grants Licensee a license to use, modify, compile, and create          */
/* derivative works of the software solely for use with RTI Connext DDS.      */
/* Licensee may redistribute copies of the software provided that all such    */
/* copies are subject to this license.                                        */
/* The software is provided "as is", with no warranty of any type, including  */
/* any warranty for fitness for any purpose. RTI is under no obligation to    */
/* maintain or support the software.  RTI shall not be liable for any         */
/* incidental or consequential damages arising out of the use or inability to */
/* use the software.                                                          */
/*                                                                            */
/******************************************************************************/

/* =========================================================================*/
/* RTI Routing Service Kafka Adapter                                        */
/* =========================================================================*/
/* Kafka adapter bridges data between DDS and Kafka */

#include "KafkaAdapter.h"

/*
 * Entry point to the Kafka adapter plugin
 */
RTI_USER_DLL_EXPORT
struct RTI_RoutingServiceAdapterPlugin *RTI_RS_Kafka_AdapterPlugin_create(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_KafkaAdapterPlugin *adapter = NULL;
    struct RTI_RoutingServiceVersion version = KAFKA_ADAPTER_VERSION;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    adapter = calloc(1, sizeof(struct RTI_RS_KafkaAdapterPlugin));

    if (adapter == NULL) {
        RTI_RoutingServiceEnvironment_set_error(
                env,
                "RTI_RS_KafkaAdpterPlugin memory allocation error");
        return NULL;
    }

    RTI_RoutingServiceAdapterPlugin_initialize(&adapter->base);

    adapter->base.plugin_version = version;

    /* Assign the function pointers */

    adapter->base.adapter_plugin_delete = RTI_RS_KafkaAdapterPlugin_delete;

    adapter->base.adapter_plugin_create_connection =
            RTI_RS_KafkaAdapterPlugin_create_connection;
    adapter->base.adapter_plugin_delete_connection =
            RTI_RS_KafkaAdapterPlugin_delete_connection;

    adapter->base.connection_create_session = NULL;
    adapter->base.connection_delete_session = NULL;

    adapter->base.connection_create_stream_writer =
            RTI_RS_KafkaConnection_create_stream_writer;
    adapter->base.connection_delete_stream_writer =
            RTI_RS_KafkaConnection_delete_stream_writer;

    adapter->base.connection_create_stream_reader =
            RTI_RS_KafkaConnection_create_stream_reader;
    adapter->base.connection_delete_stream_reader =
            RTI_RS_KafkaConnection_delete_stream_reader;

    adapter->base.stream_writer_write = RTI_RS_KafkaStreamWriter_write;

    adapter->base.stream_reader_read = RTI_RS_KafkaStreamReader_read;
    adapter->base.stream_reader_return_loan =
            RTI_RS_KafkaStreamReader_return_loan;
    adapter->base.stream_reader_update = NULL;

    return (struct RTI_RoutingServiceAdapterPlugin *) adapter;
}

void RTI_RS_KafkaAdapterPlugin_delete(
        struct RTI_RoutingServiceAdapterPlugin *adapter,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    free(adapter);
}

/* Create a Kafka adapter connection with configurations */
RTI_RoutingServiceConnection RTI_RS_KafkaAdapterPlugin_create_connection(
        struct RTI_RoutingServiceAdapterPlugin *adapter,
        const char *routing_service_name,
        const char *routing_service_group_name,
        const struct RTI_RoutingServiceStreamReaderListener
                *input_disc_listener,
        const struct RTI_RoutingServiceStreamReaderListener
                *output_disc_listener,
        const struct RTI_RoutingServiceTypeInfo **registeredTypes,
        int registeredTypeCount,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    struct RTI_RS_KafkaConnection *connection;

    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    connection = calloc(1, sizeof(struct RTI_RS_KafkaConnection));

    if (connection == NULL) {
        RTI_RoutingServiceEnvironment_set_error(
                env,
                "RTI_RS_KafkaConnection memory allocation error");
        return NULL;
    }

    connection->adapter = (struct RTI_RS_KafkaAdapterPlugin *) adapter;

    connection->bootstrap_servers =
            RTI_RoutingServiceProperties_lookup_property(
                    properties,
                    "bootstrap.servers");
    if (connection->bootstrap_servers == NULL) {
        RTI_RoutingServiceEnvironment_set_error(
                env,
                "bootstrap.servers missing property brokers");
        return NULL;
    }

    return connection;
}

void RTI_RS_KafkaAdapterPlugin_delete_connection(
        struct RTI_RoutingServiceAdapterPlugin *adapter,
        RTI_RoutingServiceConnection connection,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_RoutingServiceLogger_log(
            RTI_ROUTING_SERVICE_VERBOSITY_DEBUG,
            "%s",
            __func__);

    free(connection);
}

#undef ROUTER_CURRENT_SUBMODULE
