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

#include "MqttAgent.h"

#define RTI_MQTT_LOG_ARGS "MqttAgent::main"

int main(int argc, const char **argv)
{
    int rc = 1;
    struct MqttAgent agent;
#if USE_APP_GEN
    const char *arg_participant_name = MQTT_AGENT_PARTICIPANT,
               *arg_writer_name = MQTT_AGENT_WRITER,
               *arg_reader_name = MQTT_AGENT_READER;
#endif /* USE_APP_GEN */

    if (0
        != MqttAgent_initialize(
                &agent
#if USE_APP_GEN
                ,
                arg_participant_name,
                arg_reader_name,
                arg_writer_name
#endif /* USE_APP_GEN */
                )) {
        RTI_MQTT_ERROR("failed to initialize agent")
        goto done;
    }

    if (0 != MqttAgent_main(&agent)) {
        RTI_MQTT_ERROR("failed to run agent")
        goto done;
    }

    MqttAgent_finalize(&agent);

    rc = 0;
done:

    DDS_DomainParticipantFactory_finalize_instance();

    return rc;
}