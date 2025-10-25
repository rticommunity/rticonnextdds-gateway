/******************************************************************************/
/* (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#ifndef ShapesMqttNode_hxx_
#define ShapesMqttNode_hxx_

#include <chrono>
#include <string>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <thread>

#include "MQTTAsync.h"

class ShapesMqttNode {
public:
    ShapesMqttNode(
        const std::string &clientId,
        const std::string &serverUrl)
    {
        if (MQTTASYNC_SUCCESS
            != MQTTAsync_create(
                    &client_,
                    serverUrl.c_str(),
                    clientId.c_str(),
                    MQTTCLIENT_PERSISTENCE_NONE,
                    nullptr)) {
            throw std::runtime_error("failed to create MQTT client");
        }

        if (MQTTASYNC_SUCCESS
            != MQTTAsync_setCallbacks(
                    client_,
                    this,
                    nullptr,
                    [] (
                        void *ctx,
                        char *topicName,
                        int topicLen,
                        MQTTAsync_message *message) -> int {
                    auto self = static_cast<ShapesMqttNode*>(ctx);
                    // ignore duplicate messages
                    if (!message->dup) {
                        self->onMessageArrived(topicName, message);
                    }
                    MQTTAsync_freeMessage(&message);
                    MQTTAsync_free(topicName);
                    return 1;
                    },
                    nullptr)) {
            throw std::runtime_error("failed to set MQTT callbacks");
        }

        MQTTAsync_connectOptions opts = MQTTAsync_connectOptions_initializer;
        opts.keepAliveInterval = 30;
        opts.cleansession = true;

        makeMqttOp(
            opts,
            [this, &opts]() {
                if (MQTTASYNC_SUCCESS != MQTTAsync_connect(client_, &opts)) {
                    throw std::runtime_error("failed to connect to MQTT broker");
                }
            },
            [this, &serverUrl]() {
                std::cout << "connected to broker '" << serverUrl << "'" << std::endl;
            },
            [this, &serverUrl]() {
                std::cout << "ERROR failed to connect to broker: " << serverUrl << std::endl;
                throw std::runtime_error("failed to connect to broker");
            }
        );
    }
    
    virtual ~ShapesMqttNode() {
        MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
        opts.onSuccess = ShapesMqttNode::onMqttSuccess;
        opts.onFailure = ShapesMqttNode::onMqttFailure;
        opts.context = this;

        if (MQTTASYNC_SUCCESS != MQTTAsync_disconnect(client_, &opts)) {
            std::cerr << "ERROR failed to disconnect from broker" << std::endl;
        }

        MQTTAsync_destroy(&client_);
    }

protected:
    MQTTAsync client_;
    bool active_{true};
    bool opResult_{false};
    bool opResultAvail_{false};
    std::mutex opResultMutex_;
    std::condition_variable opResultVar_;

    void onOpResult(const bool result)
    {
        std::unique_lock<std::mutex> lk(opResultMutex_);
        opResultAvail_ = true;
        opResult_ = result;
        lk.unlock();
        opResultVar_.notify_all();
    }

    static void onMqttSuccess(void *ctx, MQTTAsync_successData *response) {
        auto self = static_cast<ShapesMqttNode *>(ctx);
        self->onOpResult(true);
    }

    static void onMqttFailure(void *ctx, MQTTAsync_failureData *response) {
        auto self = static_cast<ShapesMqttNode *>(ctx);
        self->onOpResult(false);
    }

    template<
        typename Options,
        typename Operation,
        typename OnSuccess,
        typename OnFailure>
    bool makeMqttOp(
        Options & opts,
        Operation && op,
        OnSuccess && onSuccess,
        OnFailure && onFailure)
    {
        opts.onSuccess = ShapesMqttNode::onMqttSuccess;
        opts.onFailure = ShapesMqttNode::onMqttFailure;
        opts.context = this;

        std::unique_lock<std::mutex> lk(opResultMutex_);
        opResultAvail_ = false;

        op();

        std::chrono::seconds opTimeout{5};
        auto waitOk = opResultVar_.wait_until(lk,
            std::chrono::steady_clock::now() + opTimeout,
            [this]() {
                return opResultAvail_;
            });
        
        bool result = waitOk && opResult_;

        if (result) {
            onSuccess();
        } else {
            onFailure();
        }

        return result;
    }

    virtual void onMessageArrived(const std::string & topicName, MQTTAsync_message * const message)
    {
        std::cout << "received message on '" << topicName << "'" << std::endl;
    }

    void subscribeData(const std::string &topicIn, const int qos)
    {
        MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
        makeMqttOp(
            opts,
            [this, &topicIn, &qos, &opts]() {
                if (MQTTASYNC_SUCCESS != MQTTAsync_subscribe(client_, topicIn.c_str(), qos, &opts)) {
                    throw std::runtime_error("failed to subscribe through MQTT broker");
                }
            },
            [this, &topicIn]() {
                std::cout << "subscribed to topic '" << topicIn << "'" << std::endl;
            },
            [this, &topicIn]() {
                std::cout << "ERROR failed to subscribe to topic '" << &topicIn << "'" << std::endl;
                throw std::runtime_error("failed to subscriber to topic");
            }
        );
    }

public:
    void publishData(
        const std::string &topicOut,
        const int qos,
        void * const payload,
        const size_t payloadLen)
    { 
        MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
        MQTTAsync_message msg = MQTTAsync_message_initializer;
        msg.payload = payload;
        msg.payloadlen = payloadLen;
        msg.qos = qos;

        bool published = false;
        int attempt = 1;
        for (; !published && active_; attempt++) {
            published = makeMqttOp(
                opts,
                [this, &topicOut, &opts, &msg]() {
                    if (MQTTASYNC_SUCCESS != MQTTAsync_sendMessage(client_, topicOut.c_str(), &msg, &opts)) {
                        throw std::runtime_error("failed to publish through MQTT broker");
                    }
                },
                [this, &topicOut]() {
                    std::cout << "published data to topic '" << topicOut << "'" << std::endl;
                },
                [this, &topicOut, &attempt]() {
                    std::cout
                        << "ERROR failed to publish data to topic '"<< topicOut << "'"
                        << " [attempt: " << attempt << "]"
                        << std::endl;
                }
            );
            if (!published) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
};

#endif // ShapesMqttNode_hxx_
