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

#ifndef JsonMessageTester_hpp
#define JsonMessageTester_hpp

#include "BaseTester.hpp"

#include "tsfm_json_test_message.hpp"

namespace rti {
namespace gateway {
namespace test {

class JsonMessageTester : public BaseTester {
public:
    JsonMessageTester(const int32_t tester_id)
    : BaseTester(tester_id)
    {}

    virtual ~JsonMessageTester() = default;

    virtual void test_up(
        const int32_t domain_id) override;

    virtual void run() override;

protected:
    virtual dds::topic::Topic<::test::SerializedMessage> create_topic_in();

    virtual dds::topic::Topic<::test::SerializedMessage> create_topic_out();

    virtual dds::pub::DataWriter<::test::SerializedMessage> create_writer();

    virtual dds::sub::DataReader<::test::SerializedMessage> create_reader();

    virtual void serialize_message(
        const ::test::Message& message,
        ::test::SerializedMessage& serialized_message);
    
    virtual void deserialize_message(
        const ::test::SerializedMessage& serialized_message,
        ::test::Message& message);

    dds::topic::Topic<::test::SerializedMessage> dds_topic_in_{nullptr};
    dds::topic::Topic<::test::SerializedMessage> dds_topic_out_{nullptr};
    dds::pub::DataWriter<::test::SerializedMessage> dds_writer_{nullptr};
    dds::sub::DataReader<::test::SerializedMessage> dds_reader_{nullptr};
};

} // namespace test
} // namespace gateway
} // namespace rti

#endif // JsonMessageTester_hpp
