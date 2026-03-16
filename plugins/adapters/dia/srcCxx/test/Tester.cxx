/*
 * (c) Copyright, Real-Time Innovations, 2025.
 * All rights reserved.
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "rtitest/Tester.hpp"
#include "rtitest/TestAssertion.hpp"
#include "dds_c/dds_c_infrastructure.h"
#include "SqlTableTester.hpp" 
#include "AdapterTester.hpp"
#include "OdbcConnectionManagerTester.hpp"
#include "IntegrationTester.hpp"

namespace rti { namespace adapter { namespace dis { namespace test {

class DatabaseIntegrationServiceTester : public rti::test::TesterContainer {
public:
    DatabaseIntegrationServiceTester() 
            : rti::test::TesterContainer("DatabaseIntegrationServiceTester")
    {
        RTITestSetting_setupWithConnext();
        add_tester<SQLTableTester>();
        add_tester<AdapterTester>();
        add_tester<OdbcConnectionManagerTester>();
        add_tester<IntegrationTester>();
    }
};

}}}}  // namespace rti::adapter::dis::test

int main(int argc, char **argv)
{
    return rti::adapter::dis::test::DatabaseIntegrationServiceTester().run_tests(argc, argv);
}
