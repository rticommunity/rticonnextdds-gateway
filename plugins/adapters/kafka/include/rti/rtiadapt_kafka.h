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

#ifndef rtiadapt_kafka_h
#define rtiadapt_kafka_h


/*****************************************************************************
 *                          DDS C API Selection
 *****************************************************************************/
#include "ndds/ndds_c.h"


/*****************************************************************************
 *                       RTI Routing Service SDK
 *****************************************************************************/
#include "routingservice/routingservice_adapter.h"
#include "routingservice/routingservice_service.h"


/*****************************************************************************
 *                       RTI File Adapter Plugin
 *****************************************************************************/
#ifdef __cplusplus
    extern "C" {
#endif

/**
 * @addtogroup RtiFile_Adapter
 * @{
 */

/**
 * @brief Constructor function used to load the File Adapter on RTI Routing
 * Service.
 */
extern struct RTI_RoutingServiceAdapterPlugin * 
RTI_RS_Kafka_AdapterPlugin_create(
    const struct RTI_RoutingServiceProperties * properties,
    RTI_RoutingServiceEnvironment * env);

/** @} */

#ifdef __cplusplus
    } /* extern "C" */
#endif



#endif /* rtiadapt_kafka_h */
