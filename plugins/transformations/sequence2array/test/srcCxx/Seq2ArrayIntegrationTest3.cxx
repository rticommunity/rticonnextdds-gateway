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

class Seq2ArrayIntegrationTest3 : public Seq2ArrayTester<
    ::test::inttest3::PubType,
    ::test::inttest3::SubType> {
public:
    Seq2ArrayIntegrationTest3(const int32_t tester_id)
    : Seq2ArrayTester(tester_id, "Seq2ArrayIntTest3Seq", "Seq2ArrayIntTest3Array")
    {}

protected:
    void fill_message(::test::inttest3::PubType& message, const int32_t count) override {
        using namespace ::test;

        TSeq_String letters;
        letters.push_back('a');
        letters.push_back('b');
        letters.push_back('c');

        std::vector<TSeq_String> member1;
        member1.push_back(letters);
        member1.push_back(letters);
        member1.push_back(letters);

        message.member1(member1);

        TSeq_List numbers;
        numbers.push_back(1);
        numbers.push_back(2);
        numbers.push_back(3);

        std::vector<TSeq_List> member2;
        member2.push_back(numbers);
        member2.push_back(numbers);
        member2.push_back(numbers);
        
        message.member2(member2);

        if (count % 2) {
            TSeq_StringSeq strs;
            strs.push_back(letters);
            strs.push_back(letters);
            strs.push_back(letters);
    
            message.member3(strs);
        } else {
            message.member3().reset();
        }


        std::cout
            << "[" << tester_id_ << "] "
            << "Sending message " << count << ":" << std::endl;
        this->pretty_print(message);
        std::cout << std::endl << std::endl;
    }

    void verify_message(const ::test::inttest3::SubType& message, const int32_t count) override {
        using namespace ::test;

        static const TArr_String exp_string_set = {'a', 'b', 'c', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
        static const TArr_String exp_string_unset = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
        static const TArr_List exp_list_set = {1, 2, 3, 0, 0, 0, 0, 0, 0, 0};
        static const TArr_List exp_list_unset = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        std::cout
            << "[" << tester_id_ << "] "
            << "Verifying message " << count << ":" << std::endl;
        this->pretty_print(message);
        std::cout << std::endl << std::endl;

        // Validate SubType::member1

        for (size_t i = 0; i < 5; i++) {
            const TArr_String& member1 = message.member1()[i];
            if (i < 3) {
                if (member1 != exp_string_set) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member1[" << i << "] " <<
#if RTICONNEXTDDS_HAS_OSTREAM_OPERATORS
                        "expected=" << exp_string_set << ", " <<
                        "found=" << member1 <<
#endif
                        std::endl;
                    throw std::runtime_error("invalid value for member1");
                }
            } else {
                if (member1 != exp_string_unset) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member1[" << i << "] " <<
#if RTICONNEXTDDS_HAS_OSTREAM_OPERATORS
                        "expected=" << exp_string_unset << ", " <<
                        "found=" << member1 <<
#endif
                        std::endl;
                    throw std::runtime_error("invalid value for member1");
                }
            }
        }

        // Validate SubType::member2
        for (size_t i = 0; i < 5; i++) {
            const TArr_List& member2 = message.member2()[i];
            if (i < 3) {
                if (member2 != exp_list_set) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member2[" << i << "] " <<
#if RTICONNEXTDDS_HAS_OSTREAM_OPERATORS
                        "expected=" << exp_list_set << ", " <<
                        "found=" << member2 <<
#endif
                        std::endl;
                    throw std::runtime_error("invalid value for member2");
                }
            } else {
                if (member2 != exp_list_unset) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member2[" << i << "] " <<
#if RTICONNEXTDDS_HAS_OSTREAM_OPERATORS
                        "expected=" << exp_list_unset << ", " <<
                        "found=" << member2 <<
#endif
                        std::endl;
                    throw std::runtime_error("invalid value for member2");
                }
            }
        }

        // Validate SubType::member3
        if (count % 2) {
            if (!message.member3().is_set()) {
                std::cerr <<
                    "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                    "count=" << count << ", "
                    "member3 " <<
                    "expected not null, " <<
                    "found null" << std::endl;
                throw std::runtime_error("invalid value for member3");
            }

            for (size_t i = 0; i < 5; i++) {
                const TArr_String& member3 = (*message.member3())[i];
                if (i < 3) {
                    if (member3 != exp_string_set) {
                        std::cerr <<
                            "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                            "count=" << count << ", "
                            "member3[" << i << "] " <<
#if RTICONNEXTDDS_HAS_OSTREAM_OPERATORS
                            "expected=" << exp_string_set << ", " <<
                            "found=" << member3 <<
#endif
                            std::endl;
                        throw std::runtime_error("invalid value for member3");
                    }
                } else {
                    if (member3 != exp_string_unset) {
                        std::cerr <<
                            "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                            "count=" << count << ", "
                            "member3[" << i << "] " <<
#if RTICONNEXTDDS_HAS_OSTREAM_OPERATORS
                            "expected=" << exp_string_unset << ", " <<
                            "found=" << member3 <<
#endif
                            std::endl;
                        throw std::runtime_error("invalid value for member3");
                    }
                }
            }
        } else {
            if (message.member3().is_set()) {
                std::cerr <<
                    "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                    "count=" << count << ", "
                    "member3 " <<
                    "expected null, " <<
                    "found not null" << std::endl;
                throw std::runtime_error("invalid value for member3");
            }
        }
    }
};

} // namespace test
} // namespace gateway
} // namespace rti

int main(int argc, char *argv[])
{
    using namespace rti::gateway::test;
    return tester_main<Seq2ArrayIntegrationTest3>(argc, argv);
}
