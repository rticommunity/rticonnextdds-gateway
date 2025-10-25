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
#ifndef BaseTester_hpp
#define BaseTester_hpp

#include <dds/dds.hpp>
#include <rti/topic/to_string.hpp>

#include "rti/test/AbstractDomainTester.hpp"
#include "rtitest/Tester.hpp"
#include "rti/test/generic_tests.hpp"

namespace rti {
namespace gateway {
namespace test {

class BaseTester {
public:
    BaseTester(
        const int32_t tester_id)
    : tester_id_(tester_id)
    {}

    virtual ~BaseTester() = default;

    virtual void test_up(const int32_t domain_id);

    virtual void test_down();

    virtual void run();

protected:
    virtual dds::domain::DomainParticipant create_dds_participant(
        const int32_t domain_id);

    virtual dds::pub::Publisher create_dds_publisher();

    virtual dds::sub::Subscriber create_dds_subscriber();

    template<typename M> void pretty_print(const M& message) {
        rti::topic::PrintFormatProperty format;
        format.pretty_print(true); // Enable pretty printing (indentation, newlines)
        format.enum_as_int(false); // Print enums as strings (optional)
        format.include_root_elements(true); // Include root elements (optional)

        std::cout << rti::topic::to_string(message, format);
    }

    const int32_t tester_id_;
    dds::domain::DomainParticipant dds_participant_{nullptr};
    dds::pub::Publisher dds_publisher_{nullptr};
    dds::sub::Subscriber dds_subscriber_{nullptr};
};

template <typename Tester>
int
tester_main(int argc, char **argv)
{
    int32_t tester_id = 0;
    int32_t domain_id = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--id") == 0) {
            tester_id = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--domain") == 0) {
            domain_id = atoi(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            return -1;
        }
    }
    
    try {
        Tester tester(tester_id);
        tester.test_up(domain_id);
        tester.run();
        tester.test_down();
    } catch (const std::exception &ex) {
        std::cerr << "Exception caught in tester_main: " << ex.what()
                  << std::endl;
        return 1;
    }

    return 0;
}

} // namespace test
} // namespace gateway
} // namespace rti


#endif // BaseTester_hpp