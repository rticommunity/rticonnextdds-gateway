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
 * @file rtiadapt_mqtt_log_messages.h
 * @brief Header file for the MQTT Adapter's log messages.
 * 
 * This header file defines several macros which are used by the MQTT Adapter
 * code to generate log messages.
 */

#ifndef rtiadapt_mqtt_log_messages_h
#define rtiadapt_mqtt_log_messages_h

/** @internal */

/*****************************************************************************/
/************* ------------- ERROR MESSAGES ------------- ********************/
/*****************************************************************************/

#define RTI_MQTT_INTERNAL_ERROR(msg_) \
    RTI_MQTT_ERROR_1("internal error:","%s",(msg_))

#define RTI_MQTT_HEAP_ALLOCATE_FAILED(s_) \
    RTI_MQTT_ERROR_1("failed to allocate memory:","size=%s",#s_)

#define RTI_MQTT_OBJECT_DELETE_FAILED(t_,o_) \
    RTI_MQTT_ERROR_2("failed to delete object:","type=%s, obj=%p", (t_), (o_))

#define RTI_MQTT_THREAD_SPAWN_FAILED(msg_) \
    RTI_MQTT_ERROR_1("failed to spawn thread:","msg=%s",msg_)

#define RTI_MQTT_TIME_OVERFLOW_DETECTED(t_) \
    RTI_MQTT_ERROR_2("detected time overflow:",\
        "seconds=%d, nanoseconds=%u",(t_)->seconds,(t_)->nanoseconds)

#define RTI_MQTT_TIME_NORMALIZE_FAILED(t_) \
    RTI_MQTT_ERROR_2("failed to normalize time:",\
        "seconds=%d, nanoseconds=%u",(t_)->seconds,(t_)->nanoseconds)

#define RTI_MQTT_TIME_TO_SECONDS_FAILED(t_) \
    RTI_MQTT_ERROR_2("failed to convert to seconds:",\
        "seconds=%d, nanoseconds=%u",(t_)->seconds,(t_)->nanoseconds)

#define RTI_MQTT_TIME_TO_DURATION_FAILED(t_) \
    RTI_MQTT_ERROR_2("failed to convert to DDS_Duration:",\
        "seconds=%d, nanoseconds=%u",(t_)->seconds,(t_)->nanoseconds)

#define RTI_MQTT_WAITSET_WAIT_FAILED(w_) \
    RTI_MQTT_ERROR_1("failed to wait on waitset:","waitset=%p",(w_))

#define RTI_MQTT_OCTET_SEQ_TO_STRING_FAILED(s_) \
    RTI_MQTT_ERROR_1("failed to convert octet sequence to string:",\
        "seq=%p",(s_))

#define RTI_MQTT_PTHREAD_ATTR_INIT_FAILED(rc_) \
    RTI_MQTT_ERROR_1("failed to initialize pthread attributes:","rc=%d",(rc_))

#define RTI_MQTT_PTHREAD_ATTR_DESTROY_FAILED(rc_) \
    RTI_MQTT_ERROR_1("failed to destroy pthread attributes:","rc=%d",(rc_))

#define RTI_MQTT_PTHREAD_SET_DETACHED_FAILED(rc_) \
    RTI_MQTT_ERROR_1("failed to set pthread detached:","rc=%d",(rc_))

#define RTI_MQTT_PTHREAD_CREATE_FAILED(rc_) \
    RTI_MQTT_ERROR_1("failed to create pthread:","rc=%d",(rc_))

#define RTI_MQTT_PTHREAD_JOIN_FAILED(t_,rc_) \
    RTI_MQTT_ERROR_2("failed to join pthread:","thread=%p, rc=%d",\
        (t_),(rc_))

#define RTI_MQTT_WIN_BEGIN_THREAD_FAILED() \
    RTI_MQTT_ERROR("failed to create windows thread")

#define RTI_MQTT_INVALID_SIZE_OF_CHAR_DETECTED(e_) \
    RTI_MQTT_ERROR_2("invalid sizeof(char) detected:",\
        "size=%u, expected=%u",\
        (unsigned int)sizeof(char),\
        (unsigned int)(e_))

#define RTI_MQTT_LOG_CLIENT_INVALID_CONFIG_DETECTED(c_,msg_) \
    RTI_MQTT_ERROR_2("invalid client configuration:",\
        "client=%p, msg='%s'", (c_), (msg_))


#define RTI_MQTT_LOG_CLIENT_INIT_FAILED(c_,cfg_) \
    RTI_MQTT_ERROR_2("failed to initialize RTI_MQTT_Client:",\
        "client=%p, config=%p", (c_), (cfg_))


#define RTI_MQTT_LOG_CLIENT_FINALIZE_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to finalize RTI_MQTT_Client:",\
        "client=%p", (c_))

#define RTI_MQTT_LOG_CLIENT_SET_ERROR_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to set client error:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_SET_STATE_FAILED(c_, s_) \
    RTI_MQTT_ERROR_2("failed to set client state:","client=%p, state=%d",\
            (c_), (s_))

#define RTI_MQTT_LOG_CLIENT_SET_GUARD_CONDITION_TRIGGER_FAILED(c_, g_, t_) \
    RTI_MQTT_ERROR_3("failed to set client's guard condition trigger:",\
            "client=%p, condition=%p, trigger=%d",(c_),(g_),(t_))

#define RTI_MQTT_LOG_CLIENT_UNSUPPORTED_PROTOCOL_VERSION_DETECTED(c_,v_) \
    RTI_MQTT_ERROR_2("unsupported protocol version detected:",\
        "client=%p, version=%d",(c_), (v_))

#define RTI_MQTT_LOG_CLIENT_CONNECT_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to connect client:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_RECONNECT_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to reconnect client:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_DISCONNECT_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to disconnect client:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_WAIT_FAILED(c_,e_) \
    RTI_MQTT_ERROR_2("failed while waiting for event:",\
        "client=%p, event='%s'",(c_),(e_))

#define RTI_MQTT_LOG_UNEXPECTED_CLIENT_STATE_DETECTED(c_,e_,f_) \
    RTI_MQTT_ERROR_3("unexpected client state:",\
        "client=%p, expected=%d, found=%d",(c_),(e_),(f_))

#define RTI_MQTT_LOG_CLIENT_SUBMIT_SUBSCRIPTIONS_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to submit subscriptions:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_SUBMIT_SUBSCRIPTION_FAILED(c_,s_) \
    RTI_MQTT_ERROR_2("failed to submit subscription:",\
        "client=%p, sub=%p",(c_),(s_))

#define RTI_MQTT_LOG_CLIENT_CANCEL_SUBSCRIPTIONS_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to cancel subscriptions:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_CANCEL_SUBSCRIPTION_FAILED(c_,s_) \
    RTI_MQTT_ERROR_2("failed to cancel subscription:",\
        "client=%p, sub=%p",(c_),(s_))

#define RTI_MQTT_LOG_CLIENT_UNKNOWN_SUBSCRIPTION_DETECTED(c_,s_) \
    RTI_MQTT_ERROR_2("subscription does not belong to client:",\
        "client=%p, sub=%p",(c_),(s_))

#define RTI_MQTT_LOG_CLIENT_UNKNOWN_PUBLICATION_DETECTED(c_,p_) \
    RTI_MQTT_ERROR_2("publication does not belong to client:",\
        "client=%p, pub=%p",(c_),(p_))

#define RTI_MQTT_LOG_CLIENT_NOTIFY_WRITE_RESULTS_FAILED(c_,p_) \
    RTI_MQTT_ERROR_2("failed to notify write results:",\
        "client=%p, pub=%p",(c_),(p_))

#define RTI_MQTT_LOG_CLIENT_NOTIFY_DELIVERY_RESULTS_FAILED(c_,p_) \
    RTI_MQTT_ERROR_2("failed to notify delivery results:",\
        "client=%p, pub=%p",(c_),(p_))

#define RTI_MQTT_LOG_CLIENT_LAST_WRITE_FAILED(c_,p_) \
    RTI_MQTT_ERROR_2("failed to write message:","client=%p, pub=%p",(c_),(p_))

#define RTI_MQTT_LOG_CLIENT_MATCH_SUBSCRIPTION_FAILED(c_,s_) \
    RTI_MQTT_ERROR_2("failed to match subscription:",\
        "client=%p, sub=%p",(c_),(s_))

#define RTI_MQTT_LOG_CLIENT_SUBSCRIPTION_RECEIVE_FAILED(c_,s_) \
    RTI_MQTT_ERROR_2("failed to deliver data to subscription:",\
        "client=%p, sub=%p",(c_),(s_))

#define RTI_MQTT_LOG_CLIENT_SET_SUBSCRIPTION_STATE_FAILED(c_,s_,st_) \
    RTI_MQTT_ERROR_3("failed to set subscription state:",\
        "client=%p, sub=%p, state=%d",(c_),(s_), (st_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_CREATE_CLIENT_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to create Paho C client:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_SET_CALLBACKS_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to set Paho C callbacks:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_CONNECT_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to connect using Paho C:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_DISCONNECT_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to disconnect using Paho C:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_SUBSCRIBE_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to subscribe using Paho C:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_UNSUBSCRIBE_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to unsubscribe using Paho C:","client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_MSG_FROM_DDS_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to create message from DDS using Paho C:",\
        "client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_MSG_TO_DDS_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to convert message to DDS using Paho C:",\
        "client=%p",(c_))

#define RTI_MQTT_LOG_CLIENT_PAHO_C_SEND_FAILED(c_) \
    RTI_MQTT_ERROR_1("failed to send message using Paho C:","client=%p",(c_))

#define RTI_MQTT_LOG_CREATE_DATA_FAILED(t_) \
    RTI_MQTT_ERROR_1("failed to create data:","type=%s",(t_))

#define RTI_MQTT_LOG_COPY_DATA_FAILED(t_,src_,dst_) \
    RTI_MQTT_ERROR_3("failed to copy data:","type=%s, src=%p, dst=%p",\
        (t_), (src_), (dst_))

#define RTI_MQTT_LOG_CREATE_WAITSET_FAILED(t_) \
    RTI_MQTT_ERROR_1("failed to create WaitSet:","type=%s",(t_))

#define RTI_MQTT_LOG_CREATE_GUARD_CONDITION_FAILED(t_) \
    RTI_MQTT_ERROR_1("failed to create GuardCondition:","type=%s",(t_))

#define RTI_MQTT_LOG_ATTACH_CONDITION_FAILED(w_,c_) \
    RTI_MQTT_ERROR_2("failed to attach condition:",\
        "waitset=%p, condition=%p",(w_),(c_))

#define RTI_MQTT_LOG_DETACH_CONDITION_FAILED(w_,c_) \
    RTI_MQTT_ERROR_2("failed to detach condition:",\
        "waitset=%p, condition=%p",(w_),(c_))

#define RTI_MQTT_LOG_INITIALIZE_SEQUENCE_FAILED(s_) \
    RTI_MQTT_ERROR_1("failed to initialize sequence:","seq=%p",(s_))

#define RTI_MQTT_LOG_FINALIZE_SEQUENCE_FAILED(s_) \
    RTI_MQTT_ERROR_1("failed to finalize sequence:","seq=%p",(s_))

#define RTI_MQTT_LOG_SET_SEQUENCE_MAX_FAILED(s_,m_) \
    RTI_MQTT_ERROR_2("failed to set sequence max:","seq=%p, max=%d",(s_),(m_))

#define RTI_MQTT_LOG_SET_SEQUENCE_LENGTH_FAILED(s_,l_) \
    RTI_MQTT_ERROR_2("failed to set sequence length:", \
        "seq=%p, length=%d",(s_),(l_))

#define RTI_MQTT_LOG_SET_SEQUENCE_ENSURE_LENGTH_FAILED(s_,l_,m_) \
    RTI_MQTT_ERROR_3("failed to ensure sequence length:", \
        "seq=%p, length=%d, max=%d",(s_),(l_),(m_))

#define RTI_MQTT_LOG_RECEIVED_MESSAGE_CREATE_FAILED() \
    RTI_MQTT_ERROR("failed to create RTI_MQTT_ReceivedMessage")

#define RTI_MQTT_LOG_RECEIVED_MESSAGE_INIT_FAILED(m_) \
    RTI_MQTT_ERROR_1("failed to initialize RTI_MQTT_ReceivedMessage:",\
        "%p",(m_))

#define RTI_MQTT_LOG_RECEIVED_MESSAGE_FINALIZE_FAILED(m_) \
    RTI_MQTT_ERROR_1("failed to finalize RTI_MQTT_ReceivedMessage:",\
        "%p",(m_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_INIT_FAILED(q_,s_,st_) \
    RTI_MQTT_ERROR_3("failed to initialize RTI_MQTT_MessageReceiveQueue:",\
        "queue=%p, size=%u, state=%p",(q_), (s_), (st_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_INIT_CIRCULAR_FAILED(q_) \
    RTI_MQTT_ERROR("failed to initialize RTI_MQTT_MessageReceiveQueue " \
                   "as a circular queue")

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_INIT_UNBOUNDED_FAILED(q_) \
    RTI_MQTT_ERROR("failed to initialize RTI_MQTT_MessageReceiveQueue " \
                   "as an unbounded buffer")

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_FINALIZE_FAILED(q_) \
    RTI_MQTT_ERROR_1("failed to finalize RTI_MQTT_MessageReceiveQueue:",\
        "%p",(q_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_RECEIVE_UNBOUNDED_FAILED(q_) \
    RTI_MQTT_ERROR_1("failed to receive in unbounded " \
                     "RTI_MQTT_MessageReceiveQueue:","%p",(q_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_UNEXPECTED_MESSAGE_BUFFER(q_) \
    RTI_MQTT_ERROR_1("unexpected message buffer passed to " \
                     "RTI_MQTT_MessageReceiveQueue_read:",\
                     "queue=%p",(q_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_ALREADY_IN_PROGRESS(q_) \
    RTI_MQTT_ERROR_1("read already in progress on" \
                     "RTI_MQTT_MessageReceiveQueue:",\
                     "queue=%p",(q_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_NOT_IN_PROGRESS(q_) \
    RTI_MQTT_ERROR_1("no read in progress on" \
                     "RTI_MQTT_MessageReceiveQueue:",\
                     "queue=%p",(q_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_CIRCULAR_FAILED(q_) \
    RTI_MQTT_ERROR_1("failed to read messages from circular " \
                     "RTI_MQTT_MessageReceiveQueue:","%p",(q_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_READ_UNBOUNDED_FAILED(q_) \
    RTI_MQTT_ERROR_1("failed to read messages from unbounded " \
                     "RTI_MQTT_MessageReceiveQueue:","%p",(q_))

#define RTI_MQTT_LOG_MSG_RECV_QUEUE_MESSAGE_LOST(q_,m_) \
    RTI_MQTT_ERROR_2("message lost by RTI_MQTT_MessageReceiveQueue:",\
                     "queue=%p, topic=%s",\
                     (q_),\
                     ((m_) != NULL && (m_)->info != NULL && \
                        (m_)->info->topic != NULL)? \
                                (m_)->info->topic : "unknown")

#define RTI_MQTT_LOG_PUBLICATION_CREATE_FAILED(c_,cfg_) \
    RTI_MQTT_ERROR_2("failed to create RTI_MQTT_Publication:",\
        "client=%p, config=%p",(c_), (cfg_))

#define RTI_MQTT_LOG_PUBLICATION_INIT_FAILED(p_,c_,cfg_) \
    RTI_MQTT_ERROR_3("failed to initialize RTI_MQTT_Publication:",\
        "pub=%p, client=%p, config=%p",(p_), (c_), (cfg_))

#define RTI_MQTT_LOG_PUBLICATION_FINALIZE_FAILED(p_) \
    RTI_MQTT_ERROR_1("failed to finalize RTI_MQTT_Publication:",\
        "pub=%p",(p_))

#define RTI_MQTT_LOG_PUBLICATION_INVALID_INIT_CONFIG_DETECTED(p_) \
    RTI_MQTT_ERROR_1("invalid initial configuration detected for "\
        "publication:","pub=%p", (p_))

#define RTI_MQTT_LOG_PUBLICATION_INVALID_WRITE_CONFIG_DETECTED(p_) \
    RTI_MQTT_ERROR_1("invalid write configuration detected for "\
        "publication:","pub=%p", (p_))

#define RTI_MQTT_LOG_PUBLICATION_INVALID_QOS_DETECTED(p_,q_) \
    RTI_MQTT_ERROR_2("invalid QoS detected for publication:",\
        "pub=%p, qos=%d", (p_), (q_))

#define RTI_MQTT_LOG_PUBLICATION_INVALID_TOPIC_DETECTED(p_,t_) \
    RTI_MQTT_ERROR_2("invalid Topic detected for publication:",\
        "pub=%p, topic='%s'", (p_), ((t_) == NULL)?"<null>":(t_))

#define RTI_MQTT_LOG_CLIENT_WAIT_FOR_WRITE_RESULTS_FAILED(c_,p_) \
    RTI_MQTT_ERROR_2("failed to wait for write results:", \
        "client=%p, pub=%p", (c_), (p_))

#define RTI_MQTT_LOG_PUBLICATION_WRITE_ALREADY_IN_PROGRESS(p_) \
    RTI_MQTT_ERROR_1("write already in progress on publication:", \
        "pub=%p", (p_))

#define RTI_MQTT_LOG_PUBLICATION_WRITE_MESSAGE_INFO_NOT_FOUND(p_,msg_) \
    RTI_MQTT_ERROR_2("message info required to write message not found:", \
        "pub=%p, msg=%p", (p_), (msg_))

#define RTI_MQTT_LOG_PUBLICATION_WRITE_MESSAGE_FAILED(p_,t_,q_,r_,msg_) \
    RTI_MQTT_ERROR_5("failed to write message to broker:", \
        "pub=%p, topic=%s, qos=%d, ret=%d, msg=%p", \
        (p_), (t_), (q_), (r_), (msg_))

#define RTI_MQTT_LOG_PUBLICATION_WRITE_NOT_IN_PROGRESS(p_) \
    RTI_MQTT_ERROR_1("no write currently in progress for publication:", \
        "pub=%p", (p_))


#define RTI_MQTT_LOG_PUBLICATION_NO_PENDING_MESSAGES_FOUND(p_) \
    RTI_MQTT_ERROR_1("no pending messages found for publication:", \
        "pub=%p", (p_))

#define RTI_MQTT_LOG_PUBLICATION_UNEXPECTED_DELIVERY_RESULT(p_, q_) \
    RTI_MQTT_ERROR_2("unexpected delivery result:", \
        "pub=%p, last_qos=%d", (p_), (q_))

#define RTI_MQTT_LOG_SUBSCRIPTION_CREATE_FAILED(c_,cfg_) \
    RTI_MQTT_ERROR_2("failed to create RTI_MQTT_Subscription:",\
        "client=%p, config=%p",(c_), (cfg_))

#define RTI_MQTT_LOG_SUBSCRIPTION_INIT_FAILED(s_,c_,cfg_) \
    RTI_MQTT_ERROR_3("failed to initialize RTI_MQTT_Subscription:",\
        "sub=%p, client=%p, config=%p",(s_), (c_), (cfg_))

#define RTI_MQTT_LOG_SUBSCRIPTION_FINALIZE_FAILED(s_) \
    RTI_MQTT_ERROR_1("failed to finalize RTI_MQTT_Subscription:",\
        "sub=%p",(s_))

#define RTI_MQTT_LOG_SUBSCRIPTION_CREATE_MSG_QUEUE_FAILED(s_,sz_,st_) \
    RTI_MQTT_ERROR_3("failed to create message queue for subscription:",\
        "sub=%p, size=%u, state=%p",(s_), (sz_), (st_))

#define RTI_MQTT_LOG_SUBSCRIPTION_TOPIC_MATCH_FAILED(s_,f_,t_) \
    RTI_MQTT_ERROR_3("failed to match topic:",\
        "sub=%p, filter=%s, topic=%s",(s_), \
        ((f_) == NULL)?"<null>":(f_),\
        ((t_) == NULL)?"<null>":(t_))

#define RTI_MQTT_LOG_SUBSCRIPTION_ADD_TO_QUEUE_FAILED(s_,msg_) \
    RTI_MQTT_ERROR_2("failed to add message to subscription queue:",\
        "sub=%p, msg=%p",(s_), (msg_))

#define RTI_MQTT_LOG_SUBSCRIPTION_READ_FROM_QUEUE_FAILED(s_,max_) \
    RTI_MQTT_ERROR_2("failed to read messages from subscription queue:",\
        "sub=%p, max=%d",(s_), (max_))

#define RTI_MQTT_LOG_SUBSCRIPTION_RETURN_QUEUE_LOAN_FAILED(s_) \
    RTI_MQTT_ERROR_1("failed to return messages to subscription queue:",\
        "sub=%p",(s_))


#define RTI_MQTT_QOS_LEVEL_TO_MQTT_FAILED(l_) \
    RTI_MQTT_ERROR_1("failed to convert qos level:","qos=%d",(l_))

/** @endinternal */

#endif  /* rtiadapt_mqtt_log_messages_h */
