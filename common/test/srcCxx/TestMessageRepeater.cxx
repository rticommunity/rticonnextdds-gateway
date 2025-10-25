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
#include "TestMessageRepeater.hpp"

namespace rti {
namespace gateway {
namespace test {

void
TestMessageRepeater::test_up(
    const int32_t domain_id)
{
    BaseTester::test_up(domain_id);

    // Re-register test::Message because it has a custom default name
    rti::domain::register_type<::test::Message>("test::Message");

    dds_topic_in_ = this->create_topic_in();
    if (nullptr == dds_topic_in_) {
        throw std::runtime_error("Failed to create the input topic");
    }

    dds_topic_out_ = this->create_topic_out();
    if (nullptr == dds_topic_out_) {
        throw std::runtime_error("Failed to create the output topic");
    }

    dds_writer_ = this->create_writer();
    if (nullptr == dds_writer_) {
        throw std::runtime_error("Failed to create the DataWriter");
    }

    dds_reader_ = this->create_reader();
    if (nullptr == dds_reader_) {
        throw std::runtime_error("Failed to create the DataReader");
    }
}

dds::topic::Topic<::test::Message>
TestMessageRepeater::create_topic_in()
{
    return dds::topic::Topic<::test::Message>(
        dds_participant_,
        "test_messages",
        "test::Message");
}

dds::topic::Topic<::test::Message>
TestMessageRepeater::create_topic_out()
{
    return dds_topic_in_;
}

dds::pub::DataWriter<::test::Message>
TestMessageRepeater::create_writer()
{
    return dds::pub::DataWriter<::test::Message>(
        dds_publisher_, dds_topic_in_);
}

dds::sub::DataReader<::test::Message>
TestMessageRepeater::create_reader()
{
    return dds::sub::DataReader<::test::Message>(
        dds_subscriber_, dds_topic_out_);
}

void
TestMessageRepeater::run()
{
    // 1. Create a WaitSet to wait for DDS events
    dds::core::cond::WaitSet waitset;

    // 1.1. Create a read condition to wait for data to be available
    //      on the input DataReader
    bool reader_data_available = false;
    dds::sub::cond::ReadCondition reader_data_cond(
        dds_reader_,
        dds::sub::status::DataState::new_data(),
        [this, &reader_data_available]() {
            reader_data_available = true;
        });
    waitset += reader_data_cond;

    // 2. Enable DomainParticipant
    dds_participant_.enable();
    dds::domain::ignore(dds_participant_, dds_participant_.instance_handle());

    // 3. Wait indefinitely for data to be available.
    //    When a message is received, write it back on the output DataWriter.
    int32_t count = 0;
    while (true) {
        reader_data_available = false;
        std::cout
            << "[" << tester_id_ << "] "
            << "Waiting for data to be available..."
            << std::endl;
        waitset.dispatch();
        if (reader_data_available) {
            dds::sub::LoanedSamples<::test::Message> samples = dds_reader_.take();
            std::cout
                << "[" << tester_id_ << "] "
                << "Received " << samples.length() << " samples."
                << std::endl;
            for (const auto &sample : samples) {
                if (sample.info().valid()) {
                    dds_writer_.write(sample.data());
                    count += 1;
                    std::cout
                        << "[" << tester_id_ << "] "
                        << "Wrote sample back, "
                        << "count=" << count
                        << std::endl;
                }
            }
        }
    }
}

} // namespace test
} // namespace gateway
} // namespace rti

