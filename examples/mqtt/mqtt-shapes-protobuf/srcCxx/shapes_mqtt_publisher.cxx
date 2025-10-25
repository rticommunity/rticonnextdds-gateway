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

#include <vector>
#include "shapes_mqtt_node.hxx"
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
    if (argc != 6) {
        std::cerr << "usage: shapes_mqtt_node <client_id> <server_url> <color> <topic> <qos>" << std::endl;
        return 1;
    }

    const std::string clientId = argv[1];
    const std::string serverUrl = argv[2];
    const std::string color = argv[3];
    const std::string topic = argv[4];
    const int qos = std::stol(argv[5]);

    try {
        ShapesMqttNode node(clientId, serverUrl);

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
            node.publishData(topic, qos, buffer.data(), payloadLen);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception &ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}