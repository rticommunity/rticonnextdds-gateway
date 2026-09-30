#include "BaseTester.hpp"
#include "tsfm_unbounded2bounded_test_message.hpp"

#include <chrono>

using rti::gateway::test::BaseTester;

class Unbounded2BoundedTester : public BaseTester {
public:
    explicit Unbounded2BoundedTester(int32_t tester_id) : BaseTester(tester_id) {}

    void run() override
    {
        dds_participant_.enable();
        test_route<processor::unbounddata, processor::BoundData>(
                "UnboundedData", "BoundedData", 1000, 1.0);
        test_route<processor::UnboundedFloatData, processor::BoundedFloatData>(
                "UnboundedFloatData", "BoundedFloatData", 3, 0.25);
    }

private:
    template<typename Input, typename Output>
    void test_route(const std::string &input_topic, const std::string &output_topic,
            size_t bound, double step)
    {
        dds::topic::Topic<Input> source(dds_participant_, input_topic);
        dds::topic::Topic<Output> destination(dds_participant_, output_topic);
        dds::pub::DataWriter<Input> writer(dds_publisher_, source);
        dds::sub::DataReader<Output> reader(dds_subscriber_, destination);
        dds::core::cond::StatusCondition writer_status(writer);
        writer_status.enabled_statuses(dds::core::status::StatusMask::publication_matched());
        dds::core::cond::StatusCondition reader_status(reader);
        reader_status.enabled_statuses(dds::core::status::StatusMask::subscription_matched());
        dds::core::cond::WaitSet discovery;
        discovery += writer_status;
        discovery += reader_status;
        const auto discovery_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
        while (writer.publication_matched_status().current_count() == 0
                || reader.subscription_matched_status().current_count() == 0) {
            if (std::chrono::steady_clock::now() >= discovery_deadline) {
                throw std::runtime_error(input_topic + ": discovery timeout");
            }
            discovery.wait(dds::core::Duration::from_millisecs(100));
        }
        std::vector<Input> messages;
        for (size_t length : {size_t(0), size_t(1), bound - 1, bound, bound + 1,
                    2 * bound, 2 * bound + 1, size_t(0), size_t(1)}) {
            Input message;
            for (size_t index = 0; index < length; ++index) {
                message.data().push_back((index + messages.size() * 10) * step);
            }
            messages.push_back(message);
            writer.write(message);
        }
        size_t message_index = 0;
        size_t offset = 0;
        size_t received_chunks = 0;
        dds::sub::cond::ReadCondition available(reader, dds::sub::status::DataState::new_data());
        dds::core::cond::WaitSet incoming;
        incoming += available;
        const auto receive_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
        while (message_index < messages.size()) {
            if (std::chrono::steady_clock::now() >= receive_deadline) {
                throw std::runtime_error(input_topic + ": receive timeout at input "
                        + std::to_string(message_index) + ", offset " + std::to_string(offset)
                        + ", chunks " + std::to_string(received_chunks));
            }
            incoming.wait(dds::core::Duration::from_millisecs(100));
            for (const auto &sample : reader.take()) {
                if (!sample.info().valid()) {
                    continue;
                }
                if (message_index == messages.size()) {
                    throw std::runtime_error(input_topic + ": extra chunk");
                }
                const auto &expected = messages[message_index].data();
                const auto &actual = sample.data();
                const size_t length = std::min(bound, expected.size() - offset);
                if (actual.data().size() != length) {
                    throw std::runtime_error(input_topic + ": incorrect chunk length");
                }
                for (size_t index = 0; index < length; ++index) {
                    if (actual.data()[index] != expected[offset + index]) {
                        throw std::runtime_error(input_topic + ": incorrect chunk value/order");
                    }
                }
                offset += length;
                ++received_chunks;
                if (actual.more_data() != (offset < expected.size())) {
                    throw std::runtime_error(input_topic + ": incorrect completion flag");
                }
                if (!actual.more_data()) {
                    ++message_index;
                    offset = 0;
                }
            }
        }
        incoming.wait(dds::core::Duration::from_millisecs(200));
        for (const auto &sample : reader.take()) {
            if (sample.info().valid()) {
                throw std::runtime_error(input_topic + ": unexpected trailing chunk");
            }
        }
        std::cout << input_topic << ": verified " << messages.size()
                  << " inputs, " << received_chunks << " chunks" << std::endl;
    }
};

int main(int argc, char **argv)
{
    return rti::gateway::test::tester_main<Unbounded2BoundedTester>(argc, argv);
}