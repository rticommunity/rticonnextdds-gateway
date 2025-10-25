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

#ifndef Seq2ArrayTester_hpp
#define Seq2ArrayTester_hpp

#include "BaseTester.hpp"

#include "tsfm_seq2array_test_message.hpp"

#ifndef RTICONNEXTDDS_HAS_LEGACY_UNIONS
#define RTICONNEXTDDS_HAS_LEGACY_UNIONS 0
#endif

#ifndef RTICONNEXTDDS_HAS_OSTREAM_OPERATORS
#define RTICONNEXTDDS_HAS_OSTREAM_OPERATORS 0
#endif

namespace rti {
namespace gateway {
namespace test {

template<typename PubType, typename SubType>
class Seq2ArrayTester : public BaseTester {
public:
    Seq2ArrayTester(
        const int32_t tester_id,
        const std::string& topic_name_out,
        const std::string& topic_name_in)
    : BaseTester(tester_id),
        topic_name_out_(topic_name_out),
        topic_name_in_(topic_name_in)
    {}

    virtual ~Seq2ArrayTester() = default;

    virtual void test_up(
        const int32_t domain_id) override
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

    virtual void run() override
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

        int32_t max_count = 100;

        for (int32_t count = 0; count < max_count; count++) {
            this->run_loop(count);
        }

        std::cout
            << "[" << tester_id_ << "] "
            << "All messages processed successfully." << std::endl;
    }

protected:
    virtual void run_loop(const int32_t count)
    {
        std::cout
            << "[" << tester_id_ << "] "
            << "Publishing message " << (count + 1) << "..." << std::endl;

        // Prepare PubType message
        this->fill_message(pub_message_, count);

        // Write message
        dds_writer_.write(pub_message_);

        // Wait to receive message back from RS.
        bool reader_data_available = false;
        while (!reader_data_available) {
            std::cout
                << "[" << tester_id_ << "] "
                << "Waiting to receive message back from RS..." << std::endl;
            dds::core::cond::WaitSet waitset;
            dds::sub::cond::ReadCondition reader_data_cond(
                dds_reader_,
                dds::sub::status::DataState::new_data(),
                [&reader_data_available]() {
                    reader_data_available = true;
                });
            waitset += reader_data_cond;
            waitset.dispatch();
        }

        // Verify message contents
        dds::sub::LoanedSamples<SubType> samples = dds_reader_.take();
        std::cout
            << "[" << tester_id_ << "] "
            << "Received " << samples.length() << " samples." << std::endl;
        if (samples.length() != 1) {
            throw std::runtime_error("Received unexpected number of samples");
        }
        if (samples[0].info().valid()) {
            this->verify_message(samples[0].data(), count);
        } else {
            throw std::runtime_error("Received invalid sample");
        }

        std::cout
            << "[" << tester_id_ << "] "
            << "Message " << (count + 1) << " processed successfully." << std::endl;
    }

    virtual void fill_message(PubType& message, const int32_t count) = 0;

    virtual void verify_message(const SubType& message, const int32_t count) = 0;

    virtual void fill_test_struct(::test::TestStruct& struct_val, const int32_t count)
    {
        using namespace ::test;

        struct_val.member1("test" + std::to_string(count));
        struct_val.member2(static_cast<double>(count));
        struct_val.member3(static_cast<int8_t>(count));
        switch(count % 3) {
            case 0:
                struct_val.member4(TestEnum::TEST_ENUM_VALUE_1);
                break;
            case 1:
                struct_val.member4(TestEnum::TEST_ENUM_VALUE_2);
                break;
            default:
                struct_val.member4(TestEnum::TEST_ENUM_VALUE_3);
                break;
        }
    }

    virtual void fill_test_union_seq(::test::TestUnionSeq& union_val, const int32_t count)
    {
        using namespace ::test;

        switch(count % 3) {
            case 0:
                union_val.member1(static_cast<uint16_t>(count));
                break;
            case 1:
                union_val.member2(count % 2 != 0);
                break;
            default: {
                TestStruct struct_val;
                this->fill_test_struct(struct_val, count);
                union_val._d(TestEnum::TEST_ENUM_VALUE_3);
                union_val.member3().clear();
                for (int i = 0; i < 3; i++) {
                    union_val.member3().push_back(struct_val);
                }
                break;
            }
        }
    }

    virtual void fill_test_union_array(::test::TestUnionArray& union_val, const int32_t count)
    {
        using namespace ::test;

        switch(count % 3) {
            case 0:
                union_val.member1(static_cast<uint16_t>(count));
                break;
            case 1:
                union_val.member2(count % 2 != 0);
                break;
            default: {
                TestStruct struct_val;
                this->fill_test_struct(struct_val, count);
                union_val._d(TestEnum::TEST_ENUM_VALUE_3);
                union_val.member3()[0] = struct_val;
                union_val.member3()[1] = struct_val;
                union_val.member3()[2] = struct_val;
                break;
            }
        }
    }

    virtual bool verify_test_struct(
        const ::test::TestStruct& struct_val,
        const int32_t count)
    {
        using namespace ::test;

        std::string expected_str =
            count < 0 ? "" : "test" + std::to_string(count);
        const double expected_double =
            count < 0 ? 0.0 : static_cast<double>(count);
        const int8_t expected_int8 =
            count < 0 ? 0 : static_cast<int8_t>(count);
        TestEnum expected_enum;
        switch(count % 3) {
            case 1:
                expected_enum = TestEnum::TEST_ENUM_VALUE_2;
                break;
            case 2:
                expected_enum = TestEnum::TEST_ENUM_VALUE_3;
                break;
            default:
                expected_enum = TestEnum::TEST_ENUM_VALUE_1;
                break;
        }

        if (struct_val.member1() != expected_str) {
            std::cout
                << "[" << tester_id_ << "] "
                << "INVALID VALUE RECEIVED "
                << "count=" << count << ", "
                << "member1 "
                << "expected=" << expected_str << ", "
                << "found=" << struct_val.member1() << std::endl;
            return false;
        }

        if (struct_val.member2() != expected_double) {
            std::cout
                << "[" << tester_id_ << "] "
                << "INVALID VALUE RECEIVED "
                << "count=" << count << ", "
                << "member2 "
                << "expected=" << expected_double << ", "
                << "found=" << struct_val.member2() << std::endl;
            return false;
        }

        if (struct_val.member3() != expected_int8) {
            std::cout
                << "[" << tester_id_ << "] "
                << "INVALID VALUE RECEIVED "
                << "count=" << count << ", "
                << "member3 "
                << "expected=" << expected_int8 << ", "
                << "found=" << struct_val.member3() << std::endl;
            return false;
        }

        if (struct_val.member4() != expected_enum) {
            std::cout
                << "[" << tester_id_ << "] "
                << "INVALID VALUE RECEIVED "
                << "count=" << count << ", "
                << "member4 "
                << "expected=" << expected_enum << ", "
                << "found=" << struct_val.member4() << std::endl;
            return false;
        }
        return true;
    }

    virtual dds::topic::Topic<SubType> create_topic_in()
    {
        return dds::topic::Topic<SubType>(
            dds_participant_,
            topic_name_in_);
    }

    virtual dds::topic::Topic<PubType> create_topic_out()
    {
        return dds::topic::Topic<PubType>(
            dds_participant_,
            topic_name_out_);
    }

    virtual dds::pub::DataWriter<PubType> create_writer()
    {
        return dds::pub::DataWriter<PubType>(
            dds_publisher_,
            dds_topic_out_);
    }

    virtual dds::sub::DataReader<SubType> create_reader()
    {
        return dds::sub::DataReader<SubType>(
            dds_subscriber_,
            dds_topic_in_);
    }

    std::string topic_name_out_;
    std::string topic_name_in_;
    dds::topic::Topic<SubType> dds_topic_in_{nullptr};
    dds::topic::Topic<PubType> dds_topic_out_{nullptr};
    dds::pub::DataWriter<PubType> dds_writer_{nullptr};
    dds::sub::DataReader<SubType> dds_reader_{nullptr};
    PubType pub_message_;
};

} // namespace test
} // namespace gateway
} // namespace rti

#endif // Seq2ArrayTester_hpp
