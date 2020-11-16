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

#ifndef ClientPaho_h
#define ClientPaho_h

#if MQTT_CLIENT_API == MQTT_CLIENT_API_PAHO_C

/* Paho C Asynchronous Client Public Header */
#include "Infrastructure.h"
#include "MQTTAsync.h"

/*****************************************************************************
 *                               MQTT Client API Methods
 *****************************************************************************/

DDS_ReturnCode_t
        RTI_MQTT_ClientMqttApi_Paho_create_client(struct RTI_MQTT_Client *self);

DDS_ReturnCode_t
        RTI_MQTT_ClientMqttApi_Paho_delete_client(struct RTI_MQTT_Client *self);

DDS_ReturnCode_t
        RTI_MQTT_ClientMqttApi_Paho_connect(struct RTI_MQTT_Client *self);

DDS_ReturnCode_t
        RTI_MQTT_ClientMqttApi_Paho_disconnect(struct RTI_MQTT_Client *self);

DDS_ReturnCode_t RTI_MQTT_ClientMqttApi_Paho_submit_subscriptions(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req);

DDS_ReturnCode_t RTI_MQTT_ClientMqttApi_Paho_cancel_subscriptions(
        struct RTI_MQTT_Client *self,
        struct RTI_MQTT_PendingRequest *req);

DDS_ReturnCode_t RTI_MQTT_ClientMqttApi_Paho_write_message(
        struct RTI_MQTT_Client *self,
        const char *buffer,
        DDS_UnsignedLong buffer_len,
        const char *topic,
        RTI_MQTT_WriteParams *params,
        struct RTI_MQTT_PendingRequest *req);

#define RTI_MQTT_ClientMqttApi_Client MQTTAsync
#define RTI_MQTT_ClientMqttApi_Client_INITIALIZER NULL

#define RTI_MQTT_ClientMqttApi_create_client \
        RTI_MQTT_ClientMqttApi_Paho_create_client

#define RTI_MQTT_ClientMqttApi_delete_client \
        RTI_MQTT_ClientMqttApi_Paho_delete_client

#define RTI_MQTT_ClientMqttApi_connect RTI_MQTT_ClientMqttApi_Paho_connect

#define RTI_MQTT_ClientMqttApi_disconnect \
        RTI_MQTT_ClientMqttApi_Paho_disconnect

#define RTI_MQTT_ClientMqttApi_submit_subscriptions \
        RTI_MQTT_ClientMqttApi_Paho_submit_subscriptions

#define RTI_MQTT_ClientMqttApi_cancel_subscriptions \
        RTI_MQTT_ClientMqttApi_Paho_cancel_subscriptions

#define RTI_MQTT_ClientMqttApi_write_message \
        RTI_MQTT_ClientMqttApi_Paho_write_message

/*****************************************************************************
 *                             Paho-specific Methods
 *****************************************************************************/

void RTI_MQTT_ClientMqttApi_Paho_on_success(
        void *ctx,
        MQTTAsync_successData *response);

void RTI_MQTT_ClientMqttApi_Paho_on_failure(
        void *ctx,
        MQTTAsync_failureData *response);

void RTI_MQTT_ClientMqttApi_Paho_on_connection_lost(void *ctx, char *cause);

int RTI_MQTT_ClientMqttApi_Paho_on_message_arrived(
        void *ctx,
        char *topic_name,
        int topic_len,
        MQTTAsync_message *message);

#endif /* MQTT_CLIENT_API */


#endif /* ClientPaho_h */