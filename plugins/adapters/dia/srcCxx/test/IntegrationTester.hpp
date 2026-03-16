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

#ifndef INTEGRATION_TESTER_HPP
#define INTEGRATION_TESTER_HPP

#include <rtitest/Tester.hpp>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace rti::adapter::dis;

class IntegrationTester :
        public rti::test::Tester,
        public rti::test::Singleton<IntegrationTester> {
public:

    friend class rti::test::Singleton<IntegrationTester>;

    /**
     * Tests routing service published samples are properly inserted in the database.
     */
    void test_sample_insertion();

    /**
     * Tests two different topics in different domains are routed to tables
     * with the correct domain suffix in their names.
     */
    void test_sample_insertion_multiple_domains();

    /**
     * Tests routing service disposed instances are properly removed from the database.
     */
    void test_dispose_instance();

    /**
     * Tests routing service disposed instances are not removed from the database.
     * when delete_on_dispose property is not set.
     */
    void test_no_dispose_instance();

    /**
     * Tests instance_history feature limits samples per instance to configured value.
     * With instance_history=3, inserting 5 samples should result in 3 samples (oldest updated).
     */
    void test_instance_history();

    /**
     * Tests instance_history batch filtering with instance_history=2.
     * Writes 5 samples in a single batch per instance, verifies only the newest 2 
     * are processed per instance.
     */
    void test_instance_history_batch();

    /**
     * Tests instance_history with two different topics having different history limits.
     * Topic1 with instance_history=3, Topic2 with instance_history=2.
     * Writes 5 samples to each topic and verifies correct sample counts.
     */
    void test_instance_history_two_topics();

    /**
     * Tests instance_history with depth=-1 (unlimited history).
     * Writes 10 samples for one instance and verifies all are stored.
     */
    void test_instance_history_unlimited();
private:

    IntegrationTester();
};

}}}}  // namespace rti::adapter::dis::test

#endif // INTEGRATION_TESTER_HPP
