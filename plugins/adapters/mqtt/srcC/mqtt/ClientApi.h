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

#ifndef ClientApi_h
#define ClientApi_h

#if MQTT_CLIENT_API == MQTT_CLIENT_API_PAHO_C
    #include "ClientApiPaho.h"
#elif MQTT_CLIENT_API == MQTT_CLIENT_API_MOSQUITTO
    #error "Mosquitto not supported yet as Client API"
#else
    #error "Invalid MQTT Client API implementation selected"
#endif


#if !defined(RTI_MQTT_ClientMqttApi_Client)                      \
        || !defined(RTI_MQTT_ClientMqttApi_create_client)        \
        || !defined(RTI_MQTT_ClientMqttApi_delete_client)        \
        || !defined(RTI_MQTT_ClientMqttApi_connect)              \
        || !defined(RTI_MQTT_ClientMqttApi_disconnect)           \
        || !defined(RTI_MQTT_ClientMqttApi_submit_subscriptions) \
        || !defined(RTI_MQTT_ClientMqttApi_cancel_subscriptions) \
        || !defined(RTI_MQTT_ClientMqttApi_write_message)
    #error "Invalid MQTT Client API implementation loaded"
#endif


#endif /* ClientApi_h */