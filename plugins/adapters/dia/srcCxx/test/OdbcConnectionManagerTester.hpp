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

#ifndef ODBC_CONNECTION_TESTER_HPP
#define ODBC_CONNECTION_TESTER_HPP

#include <rtitest/Tester.hpp>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace rti::adapter::dis;

class OdbcConnectionManagerTester :
        public rti::test::Tester,
        public rti::test::Singleton<OdbcConnectionManagerTester> {
public:

    friend class rti::test::Singleton<OdbcConnectionManagerTester>;

    /**
     * @brief Tests correct connection to mariadb related DSN can be performed as well
     *		as an error is thrown when not
     */
    void test_database_connection();

private:

    OdbcConnectionManagerTester();
};

}}}}  // namespace rti::adapter::dis::test

#endif // ODBC_CONNECTION_TESTER_HPP

