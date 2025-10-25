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

#ifndef MqttMessageTester_hpp
#define MqttMessageTester_hpp

#include "BaseTester.hpp"

#include "rtiadapt_mqtt_types_message.hpp"

namespace rti {
namespace gateway {
namespace test {

class MqttMessageTester : public BaseTester {
public:
    MqttMessageTester(const int32_t tester_id)
    : BaseTester(tester_id)
    {}

    virtual ~MqttMessageTester() = default;

    virtual void test_up(
        const int32_t domain_id) override;

    virtual void run() override;

protected:
    virtual dds::topic::Topic<::RTI::MQTT::KeyedMessage> create_topic_in();

    virtual dds::topic::Topic<::RTI::MQTT::KeyedMessage> create_topic_out();

    virtual dds::pub::DataWriter<::RTI::MQTT::KeyedMessage> create_writer();

    virtual dds::sub::DataReader<::RTI::MQTT::KeyedMessage> create_reader();

    dds::topic::Topic<::RTI::MQTT::KeyedMessage> dds_topic_in_{nullptr};
    dds::topic::Topic<::RTI::MQTT::KeyedMessage> dds_topic_out_{nullptr};
    dds::pub::DataWriter<::RTI::MQTT::KeyedMessage> dds_writer_{nullptr};
    dds::sub::DataReader<::RTI::MQTT::KeyedMessage> dds_reader_{nullptr};
};

} // namespace test
} // namespace gateway
} // namespace rti

#endif // MqttMessageTester_hpp
