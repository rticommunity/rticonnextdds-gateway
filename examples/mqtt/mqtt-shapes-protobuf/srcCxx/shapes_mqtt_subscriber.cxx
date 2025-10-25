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

#include "shapes_mqtt_node.hxx"
#include "shape_type.pb.h"

class ShapesMqttSubscriber : public ShapesMqttNode {
public:
    ShapesMqttSubscriber(
        const std::string &clientId,
        const std::string &serverUrl,
        const std::string &topicIn,
        const int qos)
        : ShapesMqttNode(clientId, serverUrl)
    {
        subscribeData(topicIn, qos);
    }

protected:
    void onMessageArrived(const std::string & topicName, MQTTAsync_message * const message) override
    {
        ShapeType shape;
        if (!shape.ParseFromArray(message->payload, message->payloadlen)) {
            std::cout << "ERROR failed to parse shape from message on topic " << topicName << std::endl;
            return;
        }
        std::cout << "received shape on '" << topicName << "': "
                  << "color='" << shape.color() << "', "
                  << "x=" << shape.x() << ", "
                  << "y=" << shape.y() << ", "
                  << "size=" << shape.shapesize()
                  << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "usage: shapes_mqtt_subscriber <client_id> <server_url> <topic> <qos>" << std::endl;
        return 1;
    }

    const std::string clientId = argv[1];
    const std::string serverUrl = argv[2];
    const std::string topicIn = argv[3];
    const int qos = std::stol(argv[4]);

    try {
        ShapesMqttSubscriber node(clientId, serverUrl, topicIn, qos);

        // run forever
        for (;;) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception &ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
