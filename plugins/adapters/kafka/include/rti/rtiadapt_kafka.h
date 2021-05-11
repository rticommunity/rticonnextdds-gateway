/*
 * (c) 2020 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */
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
