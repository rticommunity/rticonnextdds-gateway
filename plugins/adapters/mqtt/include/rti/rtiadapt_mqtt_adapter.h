/******************************************************************************/
/* (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

/**
 * @file rtiadapt_mqtt_adapter.h
 * @brief Header file for the implementation of the RTI Routing Service
 * Adapter API provided by the MQTT Adapter.
 *
 * This header file defines the create function that must be registered
 * on RTI Routing Service to create an instance of the MQTT Adapter.
 *
 */

#ifndef rtiadapt_mqtt_adapter_h
#define rtiadapt_mqtt_adapter_h

/*****************************************************************************
 *                       RTI Routing Service SDK
 *****************************************************************************/
#include "routingservice/routingservice_adapter.h"

/*****************************************************************************
 *                       RTI MQTT Adapter Plugin
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup RtiMqtt_Adapter
 * @{
 */

/**
 * @brief A constant containing the version identifier of the MQTT Adapter
 * library.
 *
 */
extern const struct RTI_RoutingServiceVersion RTI_RS_MQTT_AdapterPlugin_VERSION;

/**
 * @brief Major part of the MQTT Adapter version identifier.
 */
#define RTI_RS_MQTT_ADAPTER_VERSION_MAJOR 1
/**
 * @brief Minor part of the MQTT Adapter version identifier.
 */
#define RTI_RS_MQTT_ADAPTER_VERSION_MINOR 0
/**
 * @brief Release part of the MQTT Adapter version identifier.
 */
#define RTI_RS_MQTT_ADAPTER_VERSION_RELEASE 0
/**
 * @brief Revision part of the MQTT Adapter version identifier.
 */
#define RTI_RS_MQTT_ADAPTER_VERSION_REVISION 0

/**
 * @brief Constructor function used to load the MQTT Adapter on RTI Routing
 * Service.
 */
struct RTI_RoutingServiceAdapterPlugin *RTI_RS_MQTT_AdapterPlugin_create(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* rtiadapt_mqtt_adapter_h */
