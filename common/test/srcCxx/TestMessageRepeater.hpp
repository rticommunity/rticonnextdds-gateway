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
#ifndef TestMessageRepeater_hpp
#define TestMessageRepeater_hpp

#include "BaseTester.hpp"

#include "message.hpp"

namespace rti {
namespace gateway {
namespace test {

class TestMessageRepeater : public BaseTester {
public:
    TestMessageRepeater(const int32_t tester_id)
    : BaseTester(tester_id)
    {}

    virtual ~TestMessageRepeater() = default;

    virtual void test_up(
        const int32_t domain_id) override;
    
    virtual void run() override;

protected:
    virtual dds::topic::Topic<::test::Message> create_topic_in();

    virtual dds::topic::Topic<::test::Message> create_topic_out();

    virtual dds::pub::DataWriter<::test::Message> create_writer();

    virtual dds::sub::DataReader<::test::Message> create_reader();

    dds::topic::Topic<::test::Message> dds_topic_in_{nullptr};
    dds::topic::Topic<::test::Message> dds_topic_out_{nullptr};
    dds::pub::DataWriter<::test::Message> dds_writer_{nullptr};
    dds::sub::DataReader<::test::Message> dds_reader_{nullptr};
};

} // namespace test
} // namespace gateway
} // namespace rti

#endif // TestMessageRepeater_hpp
