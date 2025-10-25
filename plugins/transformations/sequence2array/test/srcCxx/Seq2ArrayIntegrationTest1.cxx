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

#include "Seq2ArrayTester.hpp"

namespace rti {
namespace gateway {
namespace test {

class Seq2ArrayIntegrationTest1 : public Seq2ArrayTester<
    ::test::inttest1::PubType,
    ::test::inttest1::SubType> {
public:
    Seq2ArrayIntegrationTest1(const int32_t tester_id)
    : Seq2ArrayTester(tester_id, "Seq2ArrayIntTest1Seq", "Seq2ArrayIntTest1Array")
    {}

protected:
    void fill_message(::test::inttest1::PubType& message, const int32_t count) override {
        using namespace ::test;

        TestStruct struct_val;
        this->fill_test_struct(struct_val, count);

        TestUnionSeq union_val;
        this->fill_test_union_seq(union_val, count);

        // fill in member1
        std::vector<TestStruct> member1;
        member1.push_back(struct_val);
        member1.push_back(struct_val);
        member1.push_back(struct_val);

        message.member1(member1);

        // fill in member2
        std::vector<TestEnum> member2;
        member2.push_back(struct_val.member4());
        member2.push_back(struct_val.member4());
        member2.push_back(struct_val.member4());

        message.member2(member2);

        // fill in member3
        std::vector<TestUnionSeq> member3;
        member3.push_back(union_val);
        member3.push_back(union_val);
        member3.push_back(union_val);

        message.member3(member3);

        // fill in member4
        std::array<TestStruct, 2> member4;
        member4[0] = struct_val;
        member4[1] = struct_val;

        message.member4(member4);

        // fill in member5
        message.member5(struct_val);

        // fill in member6
        message.member6(static_cast<uint64_t>(count));

        // fill in member7
        message.member7(struct_val.member4());

        std::cout
            << "[" << tester_id_ << "] "
            << "Sending message " << count << ":" << std::endl;
        this->pretty_print(message);
        std::cout << std::endl << std::endl;
    }

    void verify_message(const ::test::inttest1::SubType& message, const int32_t count) override {
        using namespace ::test;

        std::cout
            << "[" << tester_id_ << "] "
            << "Verifying message " << count << ":" << std::endl;
        this->pretty_print(message);
        std::cout << std::endl << std::endl;

        std::string expected_str = "test" + std::to_string(count);
        TestEnum expected_enum;
        switch(count % 3) {
            case 0:
                expected_enum = TestEnum::TEST_ENUM_VALUE_1;
                break;
            case 1:
                expected_enum = TestEnum::TEST_ENUM_VALUE_2;
                break;
            default:
                expected_enum = TestEnum::TEST_ENUM_VALUE_3;
                break;
        }

        // Validate SubType::member1
        for (size_t i = 0; i < 5; i++) {
            const TestStruct& member1 = message.member1()[i];
            if (!this->verify_test_struct(member1, i < 3 ? count : -1)) {
                std::cerr <<
                    "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                    "count=" << count << ", "
                    "member1[" << i << "] " <<
                    "invalid value" << std::endl;
                throw std::runtime_error("invalid value for member1");
            }
        }

        // Validate SubType::member2
        for (size_t i = 0; i < 5; i++) {
            const TestEnum i_exp_enum = i < 3 ? expected_enum : TestEnum::TEST_ENUM_VALUE_1;
            if (message.member2()[i] != i_exp_enum) {
                std::cerr <<
                    "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                    "count=" << count << ", "
                    "member2[" << i << "] " <<
                    "expected=" << i_exp_enum << ", " <<
                    "found=" << message.member2()[i] << std::endl;
                throw std::runtime_error("invalid value for member2");
            }
        }

        // Validate SubType::member3
        for (size_t i = 0; i < 5; i++) {
            const TestUnionArray& member3 = message.member3()[i];
            if (i < 3) {
                TestEnum expected_d;
                switch(count % 3) {
                    case 0:
                        expected_d = TestEnum::TEST_ENUM_VALUE_1;
                        break;
                    case 1:
                        expected_d = TestEnum::TEST_ENUM_VALUE_2;
                        break;
                    default:
                        expected_d = TestEnum::TEST_ENUM_VALUE_3;
                        break;
                }
                if (member3._d() != expected_d) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member3[" << i << "] " <<
                        "expected discriminator=" << expected_d << ", " <<
                        "found=" << member3._d() << std::endl;
                    throw std::runtime_error("invalid discriminator for member3");
                }
                switch(count % 3) {
                    case 0:
                        if (member3.member1() != static_cast<uint16_t>(count)) {
                            std::cerr <<
                                "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                                "count=" << count << ", "
                                "member3[" << i << "].member1 " <<
                                "expected=" << static_cast<uint16_t>(count) << ", " <<
                                "found=" << member3.member1() << std::endl;
                            throw std::runtime_error("invalid value for member3.member1");
                        }
                        break;
                    case 1:
                        if (member3.member2() != (count % 2 != 0)) {
                            std::cerr <<
                                "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                                "count=" << count << ", "
                                "member3[" << i << "].member2 " <<
                                "expected=" << (count % 2 != 0) << ", " <<
                                "found=" << member3.member2() << std::endl;
                            throw std::runtime_error("invalid value for member3.member2");
                        }
                        break;
                    default:
                        for (size_t j = 0; j < 5; j++) {
                            const TestStruct& struct_val = member3.member3()[j];
                            if (!this->verify_test_struct(struct_val, j < 3 ? count : -1)) {
                                std::cerr <<
                                    "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                                    "count=" << count << ", "
                                    "member3[" << i << "].member3[" << j << "] " <<
                                    "invalid value" << std::endl;
                                throw std::runtime_error("invalid value for member3.member3");
                            }
                        }
                        break;
                }
            } else {
#if RTICONNEXTDDS_HAS_LEGACY_UNIONS
                if (member3._d() != TestEnum::TEST_ENUM_VALUE_3) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member3[" << i << "] " <<
                        "expected discriminator=" << TestEnum::TEST_ENUM_VALUE_1 << ", " <<
                        "found=" << member3._d() << std::endl;
                    throw std::runtime_error("invalid discriminator for member3");
                }

                for (size_t j = 0; j < 5; j++) {
                    const TestStruct& struct_val = member3.member3()[j];
                    if (!this->verify_test_struct(struct_val, -1)) {
                        std::cerr <<
                            "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                            "count=" << count << ", "
                            "member3[" << i << "].member3[" << j << "] " <<
                            "invalid value" << std::endl;
                        throw std::runtime_error("invalid value for member3.member3");
                    }
                }
#else
                if (member3._d() != TestEnum::TEST_ENUM_VALUE_1) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member3[" << i << "] " <<
                        "expected discriminator=" << TestEnum::TEST_ENUM_VALUE_1 << ", " <<
                        "found=" << member3._d() << std::endl;
                    throw std::runtime_error("invalid discriminator for member3");
                }

                if (member3.member1() != 0) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member3[" << i << "].member1 " <<
                        "expected=" << 0 << ", " <<
                        "found=" << member3.member1() << std::endl;
                    throw std::runtime_error("invalid value for member3.member1");
                }
#endif
            }
        }

        // Validate SubType::member4
        for (size_t i = 0; i < 2; i++) {
            const TestStruct& member4 = message.member4()[i];
            if (!this->verify_test_struct(member4, count)) {
                std::cerr <<
                    "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                    "count=" << count << ", "
                    "member4[" << i << "] " <<
                    "invalid value" << std::endl;
                throw std::runtime_error("invalid value for member4");
            }
        }

        // Validate SubType::member5
        const TestStruct& member5 = message.member5();
        if (!this->verify_test_struct(member5, count)) {
            std::cerr <<
                "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                "count=" << count << ", "
                "member5 " <<
                "invalid value" << std::endl;
            throw std::runtime_error("invalid value for member5");
        }

        // Validate SubType::member6
        if (message.member6() != static_cast<uint64_t>(count)) {
            std::cerr <<
                "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                "count=" << count << ", "
                "member6 " <<
                "expected=" << static_cast<uint64_t>(count) << ", " <<
                "found=" << message.member6() << std::endl;
            throw std::runtime_error("invalid value for member6");
        }

        // Validate SubType::member7
        if (message.member7() != expected_enum) {
            std::cerr <<
                "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                "count=" << count << ", "
                "member7 " <<
                "expected=" << expected_enum << ", " <<
                "found=" << message.member7() << std::endl;
            throw std::runtime_error("invalid value for member7");
        }
    }
};

} // namespace test
} // namespace gateway
} // namespace rti

int main(int argc, char *argv[])
{
    using namespace rti::gateway::test;
    return tester_main<Seq2ArrayIntegrationTest1>(argc, argv);
}
