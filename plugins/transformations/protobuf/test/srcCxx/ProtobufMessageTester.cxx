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

#include "ProtobufMessageTester.hpp"
#include "ProtobufMessageUtils.hpp"

namespace rti {
namespace gateway {
namespace test {

void
ProtobufMessageTester::test_up(
    const int32_t domain_id)
{
    BaseTester::test_up(domain_id);

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

dds::topic::Topic<::test::SerializedMessage>
ProtobufMessageTester::create_topic_in()
{
    return dds::topic::Topic<::test::SerializedMessage>(
        dds_participant_,
        "serialized_test_messages");
}

dds::topic::Topic<::test::SerializedMessage>
ProtobufMessageTester::create_topic_out()
{
    return dds_topic_in_;
}

dds::pub::DataWriter<::test::SerializedMessage>
ProtobufMessageTester::create_writer()
{
    return dds::pub::DataWriter<::test::SerializedMessage>(
        dds_publisher_, dds_topic_in_);
}

dds::sub::DataReader<::test::SerializedMessage>
ProtobufMessageTester::create_reader()
{
    return dds::sub::DataReader<::test::SerializedMessage>(
        dds_subscriber_, dds_topic_out_);
}

void
ProtobufMessageTester::serialize_message(
    const ::test::Message& message,
    ::test::SerializedMessage& serialized_message)
{
    serialized_message.src(tester_id_);
    const int serialized_size = message.ByteSizeLong();
    serialized_message.data().resize(serialized_size);
    if (!message.SerializeToArray(&serialized_message.data()[0], serialized_size)) {
        throw std::runtime_error("Failed to serialize Protobuf message");
    }
}

void
ProtobufMessageTester::deserialize_message(
    const ::test::SerializedMessage& serialized_message,
    ::test::Message& message)
{
    if (!message.ParseFromArray(
            &serialized_message.data()[0],
            static_cast<int>(serialized_message.data().size()))) {
        throw std::runtime_error("Failed to deserialize Protobuf message");
    }
}

void
ProtobufMessageTester::run()
{
    // 1. Create a waitset to wait for DDS events
    dds::core::cond::WaitSet waitset;

    // 1.1 Create a status condition to intercept "subscription_matched" events
    //     on the input DataReader
    dds::core::status::SubscriptionMatchedStatus reader_status;
    dds::core::cond::StatusCondition reader_cond(dds_reader_);
    reader_cond.enabled_statuses(
        dds::core::status::StatusMask::subscription_matched());
    reader_cond->handler(
        [this, &reader_status]() {
            dds::core::status::StatusMask statuses = dds_reader_.status_changes();
            std::cout
                << "[" << tester_id_ << "] "
                << "DataReader status changed: " << statuses << std::endl;
            if ((statuses & dds::core::status::StatusMask::subscription_matched())
                    .any()) {
                reader_status = dds_reader_.subscription_matched_status();
            }
        });
    waitset += reader_cond;

    // 1.2 Create a read condition to wait for data to be available
    //     on the input DataReader
    bool reader_data_available = false;
    dds::sub::cond::ReadCondition reader_data_cond(
        dds_reader_,
        dds::sub::status::DataState::new_data(),
        [this, &reader_data_available]() {
            reader_data_available = true;
            std::cout
                << "[" << tester_id_ << "] "
                << "DataReader has data available." << std::endl;
        });
    waitset += reader_data_cond;

    // 1.3 Create a status condition to intercept "publication_matched" events
    //     on the output DataWriter
    dds::core::status::PublicationMatchedStatus writer_status;
    dds::core::cond::StatusCondition writer_cond(dds_writer_);
    writer_cond.enabled_statuses(
        dds::core::status::StatusMask::publication_matched());
    writer_cond->handler(
        [this, &writer_status]() {
            dds::core::status::StatusMask statuses = dds_writer_.status_changes();
            std::cout
                << "[" << tester_id_ << "] "
                << "DataWriter status changed: " << statuses << std::endl;
            if ((statuses & dds::core::status::StatusMask::publication_matched())
                    .any()) {
                writer_status = dds_writer_.publication_matched_status();
            }
        });
    waitset += writer_cond;

    // 2. Enable DomainParticipant
    dds_participant_.enable();
    dds::domain::ignore(dds_participant_, dds_participant_.instance_handle());

    // 3. Wait for both DataReader and DataWriter to match a remote endpoint
    while (reader_status.current_count() != 1 || writer_status.current_count() != 1) {
        std::cout 
            << "[" << tester_id_ << "] "
            << "Waiting for DataReader and DataWriter to match a remote endpoint..."
            << std::endl;
        waitset.dispatch();
        std::cout
            << "[" << tester_id_ << "] "
            << "  DataReader current_count: " << reader_status.current_count()
            << ", total_count: " << reader_status.total_count() << std::endl;
        std::cout
            << "[" << tester_id_ << "] "
            << "  DataWriter current_count: " << writer_status.current_count()
            << ", total_count: " << writer_status.total_count() << std::endl;
    }
    std::cout
        << "[" << tester_id_ << "] "
        << "DataReader and DataWriter matched." << std::endl;

    waitset -= reader_cond;
    waitset -= writer_cond;

    // 4. Iteratively, up to max_count:
    //    - Prepare protobuf test::<essage
    //    - Serialize test::Message into a test::SerializedMessage
    //    - Write message
    //    - Wait to receive a test::SerializedMessage back from RS.
    //    - Deserialize test::SerializedMessage into a protobuf test::Message
    //    - Verify message contents
    ::test::Message message;
    ::test::SerializedMessage serialized_message;
    int32_t max_count = 100;

    for (int32_t count = 0; count < max_count; count++) {
        std::cout
            << "[" << tester_id_ << "] "
            << "Publishing message " << (count + 1) << "..." << std::endl;

        // 2.1 Prepare protobuf test::Message
        ::test::generate_message(message, tester_id_, count);

        // 2.2 Serialize test::Message into a test::SerializedMessage
        serialize_message(message, serialized_message);

        // 2.3 Write message
        dds_writer_.write(serialized_message);

        // 2.3 Wait to receive message back from RS.
        reader_data_available = false;
        while (!reader_data_available) {
            std::cout
                << "[" << tester_id_ << "] "
                << "Waiting to receive message back from RS..." << std::endl;
            waitset.dispatch();
        }

        // 2.4 Verify message contents
        dds::sub::LoanedSamples<::test::SerializedMessage> samples = dds_reader_.take();
        std::cout
            << "[" << tester_id_ << "] "
            << "Received " << samples.length() << " samples." << std::endl;
        if (samples.length() != 1) {
            throw std::runtime_error("Received unexpected number of samples");
        }
        if (samples[0].info().valid()) {
            int32_t received_count;
            deserialize_message(samples[0].data(), message);
            ::test::verify_message(message, tester_id_, received_count);
            if (received_count != count) {
                throw std::runtime_error("Received message with unexpected count");
            }
        } else {
            throw std::runtime_error("Received invalid sample");
        }

        std::cout
            << "[" << tester_id_ << "] "
            << "Message " << (count + 1) << " processed successfully." << std::endl;
    }

    std::cout
        << "[" << tester_id_ << "] "
        << "All messages processed successfully." << std::endl;
}

} // namespace test
} // namespace gateway
} // namespace rti


int main(int argc, char *argv[])
{
    using namespace rti::gateway::test;
    return tester_main<ProtobufMessageTester>(argc, argv);
}
