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

#ifndef ADAPTER_TESTER_HPP
#define ADAPTER_TESTER_HPP

#include <rtitest/Tester.hpp>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace rti::adapter::dis;

/**
 * @brief Test class for DIS Adapter functionality
 * 
 * This class contains unit tests for the Database Integration Service (DIS) adapter,
 * testing connection creation, topic properties parsing, and stream writer configuration.
 */
class AdapterTester :
        public rti::test::Tester,
        public rti::test::Singleton<AdapterTester> {
public:

    friend class rti::test::Singleton<AdapterTester>;

    /**
     * @brief Tests correct JSON parsing for topic properties configuration.
     *        Creates a DiaConnection with a JSON file, creates a StreamWriter,
     *        and verifies the properties are correctly parsed and applied.
     */
    void test_json_topic_properties_parsing();

private:
    AdapterTester();
};

} } } } // namespace rti::adapter::dis::test

#endif
