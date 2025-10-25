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

#include <chrono>
#include <string>
#include <iostream>
#include <thread>

#include "rdkafka.h"
#include "shape_type.pb.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: shapes_kafka_subscriber <server_url> <topic>" << std::endl;
        return 1;
    }

    const std::string serverUrl = argv[1];
    const std::string topicName = argv[2];

    int rc = 0;
    rd_kafka_t *consumer = nullptr;

    try {
        rd_kafka_conf_t *kafkaConf = rd_kafka_conf_new();
        if (kafkaConf == nullptr) {
            throw std::runtime_error("Failed to create Kafka configuration");
        }

        char errstr[512];
        rd_kafka_conf_res_t res = rd_kafka_conf_set(
            kafkaConf, "bootstrap.servers", serverUrl.c_str(), errstr, sizeof(errstr));
        if (res != RD_KAFKA_CONF_OK) {
            throw std::runtime_error(
                std::string("Failed to set bootstrap.servers: ") + errstr);
        }

        res = rd_kafka_conf_set(
            kafkaConf, "group.id", "shapesdemo", errstr, sizeof(errstr));
        if (res != RD_KAFKA_CONF_OK) {
            throw std::runtime_error(
                std::string("Failed to set bootstrap.servers: ") + errstr);
        }

        consumer = rd_kafka_new(
            RD_KAFKA_CONSUMER, kafkaConf, errstr, sizeof(errstr));
        if (consumer == nullptr) {
            throw std::runtime_error(
                std::string("Failed to create Kafka producer: ") + errstr);
        }
        rd_kafka_poll_set_consumer(consumer);

        rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
        if (subscription == nullptr) {
            throw std::runtime_error("Failed to create Kafka topic partition list");
        }
        rd_kafka_topic_partition_list_add(subscription, topicName.c_str(), RD_KAFKA_PARTITION_UA);

        rd_kafka_resp_err_t err = rd_kafka_subscribe(consumer, subscription);
        rd_kafka_topic_partition_list_destroy(subscription);
        if (err) {
            throw std::runtime_error(
                std::string("Failed to subscribe to topic: ") +
                rd_kafka_err2str(err));
        }

        // run forever
        for (;;) {
            rd_kafka_message_t *message = rd_kafka_consumer_poll(consumer, 500);
            if (!message) {
                continue;
            }

            if (message->err) {
                if (message->err != RD_KAFKA_RESP_ERR__PARTITION_EOF) {
                    std::cout << "ERROR: Consumer error: "
                              << rd_kafka_message_errstr(message)
                              << std::endl;
                    throw std::runtime_error("consumer error");
                }
            } else {
                ShapeType shape;
                if (!shape.ParseFromArray(message->payload, message->len)) {
                    std::cout << "ERROR failed to parse shape from message on topic " << topicName << std::endl;
                } else {
                    std::cout << "received shape on '" << topicName << "': "
                            << "color='" << shape.color() << "', "
                            << "x=" << shape.x() << ", "
                            << "y=" << shape.y() << ", "
                            << "size=" << shape.shapesize()
                            << std::endl;
                }
            }

            rd_kafka_message_destroy(message);
        }
    } catch (const std::exception &ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        rc = 1;
    }

    if (consumer != nullptr) {
        rd_kafka_consumer_close(consumer);
        rd_kafka_destroy(consumer);
        consumer = nullptr;
    }

    return rc;
}
