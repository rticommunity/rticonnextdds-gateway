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

void fill_random_shape(
    ShapeType &shape,
    const std::string & color,
    const uint32_t minSize = 10,
    const uint32_t maxSize = 50,
    const uint32_t minX = 0,
    const uint32_t maxX = 255,
    const uint32_t minY = 0,
    const uint32_t maxY = 255)
{
    shape.set_color(color);
    shape.set_x(minX + (std::rand() % (maxX - minX + 1)));
    shape.set_y(minY + (std::rand() % (maxY - minY + 1)));
    shape.set_shapesize(minSize + (std::rand() % (maxSize - minSize + 1)));
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "usage: shapes_kafka_publisher <server_url> <color> <topic>" << std::endl;
        return 1;
    }

    const std::string serverUrl = argv[1];
    const std::string color = argv[2];
    const std::string topicName = argv[3];

    int rc = 0;
    rd_kafka_t *producer = nullptr;
    rd_kafka_topic_t *topic = nullptr;

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

        res = rd_kafka_conf_set(kafkaConf, "acks", "all", errstr, sizeof(errstr));
        if (res != RD_KAFKA_CONF_OK) {
            throw std::runtime_error(
                std::string("Failed to set acks: ") + errstr);
        }
        
        producer = rd_kafka_new(
            RD_KAFKA_PRODUCER, kafkaConf, errstr, sizeof(errstr));
        if (producer == nullptr) {
            throw std::runtime_error(
                std::string("Failed to create Kafka producer: ") + errstr);
        }

        topic = rd_kafka_topic_new(producer, topicName.c_str(), nullptr);
        if (topic == nullptr) {
            throw std::runtime_error(
                std::string("Failed to create topic object: ") +
                rd_kafka_err2str(rd_kafka_last_error()));
        }

        // run forever
        ShapeType shape;
        std::vector<uint8_t> buffer;
        for (;;) {
            fill_random_shape(shape, color);
            const size_t payloadLen = shape.ByteSizeLong();
            buffer.resize(payloadLen);
            if (!shape.SerializeToArray(buffer.data(), (int)payloadLen)) {
                throw std::runtime_error("failed to serialize shape");
            }
            if (RD_KAFKA_RESP_ERR_NO_ERROR !=
                rd_kafka_produce(
                    topic,
                    RD_KAFKA_PARTITION_UA,
                    RD_KAFKA_MSG_F_COPY,
                    buffer.data(),
                    payloadLen,
                    nullptr,
                    0,
                    nullptr)) {
                throw std::runtime_error(
                    std::string("Failed to produce message: ") +
                    rd_kafka_err2str(rd_kafka_last_error()));
            }
            rd_kafka_poll(producer, 0);
            std::cout << "published shape: " << shape.ShortDebugString() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception &ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        rc = 1;
    }

    if (topic != nullptr) {
        rd_kafka_topic_destroy(topic);
        topic = nullptr;
    }
    if (producer != nullptr) {
        rd_kafka_flush(producer, 10 * 1000); // Wait for max 10 seconds
        if (rd_kafka_outq_len(producer) > 0) {
            std::cout << "Warning: " << rd_kafka_outq_len(producer)
                      << " message(s) were not delivered" << std::endl;
        }
        rd_kafka_destroy(producer);
        producer = nullptr;
    }

    return rc;
}
