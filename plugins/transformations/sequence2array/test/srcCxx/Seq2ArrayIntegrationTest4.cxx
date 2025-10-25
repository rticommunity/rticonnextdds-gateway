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

class Seq2ArrayIntegrationTest4 : public Seq2ArrayTester<
    ::test::inttest4::PubType,
    ::test::inttest4::SubType> {
public:
    Seq2ArrayIntegrationTest4(const int32_t tester_id)
    : Seq2ArrayTester(tester_id, "Seq2ArrayIntTest4Array", "Seq2ArrayIntTest4Seq")
    {}

protected:
    void fill_message(::test::inttest4::PubType& message, const int32_t count) override {
        using namespace ::test;

        TArr_String letters_zero;
        std::fill(letters_zero.begin(), letters_zero.end(), '\0');

        TArr_String letters;
        letters[0] = 'a';
        letters[1] = 'b';
        letters[2] = 'c';
        std::fill(letters.begin() + 3, letters.end(), '\0');

        std::array<TArr_String, 5> member1;
        member1[0] = letters;
        member1[1] = letters;
        member1[2] = letters;
        member1[3] = letters_zero;
        member1[4] = letters_zero;

        message.member1(member1);

        TArr_List numbers_zero;
        std::fill(numbers_zero.begin(), numbers_zero.end(), 0);

        TArr_List numbers;
        numbers[0] = 1;
        numbers[1] = 2;
        numbers[2] = 3;
        std::fill(numbers.begin() + 3, numbers.end(), 0);

        std::array<TArr_List, 5> member2;
        member2[0] = numbers;
        member2[1] = numbers;
        member2[2] = numbers;
        member2[3] = numbers_zero;
        member2[4] = numbers_zero;
        
        message.member2(member2);

        if (count % 2) {
            TArr_StringArr strs;
            strs[0] = letters;
            strs[1] = letters;
            strs[2] = letters;
            strs[3] = letters_zero;
            strs[4] = letters_zero;
    
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

    bool verify_string_seq(const ::test::TSeq_String& str_val, const bool set = true) {
        if (str_val.size() != 10) {
            return false;
        }

        if (set) {
            for (size_t i = 0; i < 3; i++) {
                if (str_val[i] != static_cast<char>('a' + i)) {
                    return false;
                }
            }
            for (size_t i = 3; i < 10; i++) {
                if (str_val[i] != '\0') {
                    return false;
                }
            }
        } else {
            for (size_t i = 0; i < 10; i++) {
                if (str_val[i] != '\0') {
                    return false;
                }
            }
        }
        return true;
    }

    void verify_message(const ::test::inttest4::SubType& message, const int32_t count) override {
        using namespace ::test;

        std::cout
            << "[" << tester_id_ << "] "
            << "Verifying message " << count << ":" << std::endl;
        this->pretty_print(message);
        std::cout << std::endl << std::endl;

        // Validate SubType::member1
        if (message.member1().size() != 5) {
            std::cerr <<
                "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                "count=" << count << ", "
                "member1 " <<
                "expected size=5, " <<
                "found size=" << message.member1().size() << std::endl;
            throw std::runtime_error("invalid size for member1");
        }

        for (size_t i = 0; i < 5; i++) {
            const TSeq_String& member1 = message.member1()[i];
            if (i < 3) {
                if (!verify_string_seq(member1)) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member1[" << i << "]" << std::endl;
                    throw std::runtime_error("invalid value for member1");
                }
            } else {
                if (!verify_string_seq(member1, false)) {
                    std::cerr <<
                        "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                        "count=" << count << ", "
                        "member1[" << i << "]" << std::endl;
                    throw std::runtime_error("invalid value for member1");
                }
            }
        }

        // Validate SubType::member2
        for (size_t i = 0; i < 5; i++) {
            const TSeq_List& member2 = message.member2()[i];
            if (member2.size() != 10) {
                std::cerr <<
                    "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                    "count=" << count << ", "
                    "member2[" << i << "] " <<
                    "expected size=10, " <<
                    "found size=" << member2.size() << std::endl;
                throw std::runtime_error("invalid size for member2");
            }
            if (i < 3) {
                for (size_t j = 0; j < 3; j++) {
                    if (member2[j] != static_cast<long>(j + 1)) {
                        std::cerr <<
                            "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                            "count=" << count << ", "
                            "member2[" << i << "][" << j << "] " <<
                            "expected=" << static_cast<long>(j + 1) << ", " <<
                            "found=" << member2[j] << std::endl;
                        throw std::runtime_error("invalid value for member2");
                    }
                }
                for (size_t j = 3; j < 10; j++) {
                    if (member2[j] != 0) {
                        std::cerr <<
                            "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                            "count=" << count << ", "
                            "member2[" << i << "][" << j << "] " <<
                            "expected=0, " <<
                            "found=" << member2[j] << std::endl;
                        throw std::runtime_error("invalid value for member2");
                    }
                }
            } else {
                for (size_t j = 0; j < 10; j++) {
                    if (member2[j] != 0) {
                        std::cerr <<
                            "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                            "count=" << count << ", "
                            "member2[" << i << "][" << j << "] " <<
                            "expected=0, " <<
                            "found=" << member2[j] << std::endl;
                        throw std::runtime_error("invalid value for member2");
                    }
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

            if (message.member3()->size() != 5) {
                std::cerr <<
                    "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                    "count=" << count << ", "
                    "member3 " <<
                    "expected size=5, " <<
                    "found size=" << message.member3()->size() << std::endl;
                throw std::runtime_error("invalid size for member3");
            }

            for (size_t i = 0; i < 5; i++) {
                const TSeq_String& member3 = (*message.member3())[i];
                if (i < 3) {
                    if (!verify_string_seq(member3)) {
                        std::cerr <<
                            "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                            "count=" << count << ", "
                            "member3[" << i << "]" << std::endl;
                        throw std::runtime_error("invalid value for member3");
                    }
                } else {
                    if (!verify_string_seq(member3, false)) {
                        std::cerr <<
                            "[" << tester_id_ << "] INVALID VALUE RECEIVED " <<
                            "count=" << count << ", "
                            "member3[" << i << "]" << std::endl;
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
    return tester_main<Seq2ArrayIntegrationTest4>(argc, argv);
}
