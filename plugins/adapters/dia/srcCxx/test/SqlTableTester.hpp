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

#ifndef SQL_TABLE_TESTER_HPP
#define SQL_TABLE_TESTER_HPP

#include <rtitest/Tester.hpp>

namespace rti { namespace adapter { namespace dis { namespace test {

using namespace rti::adapter::dis;

class SQLTableTester :
        public rti::test::Tester,
        public rti::test::Singleton<SQLTableTester> {
public:

    friend class rti::test::Singleton<SQLTableTester>;

    /**
     * @brief Tests correct data_table creation, correct types creation and
     *		type insertion into types. Also tests data_table is
     *		dropped on type disposal but types is not dropped and
     *		still contains the proper type.
     */
    void test_table_creation_deletion();
    /**
     * @brief Tests correct data insertion to database. A sample and its sample info
     *		are mocked and after they are inserted using method
     *		insert_sample. It is checked the fetched value is conincident
     *		with the JSON value related to such sample.
     */
    void test_sample_insertion();

    /**
     * @brief Tests correct instance disposal from database. A sample info
     *		is mocked with disposed instance state and after calling
     *		remove_instance it is checked the instance is no longer
     *		present in the database.
     */
    void test_dispose_instance();

    /**
     * @brief Tests that type representation in XCDR format is correctly stored
     *		and retrieved from the types. Compares the original
     *		TypeCode serialization with the retrieved binary data.
     */
    void test_type_representation_xcdr();

    /**
     * @brief Tests that type representation in XML format is correctly stored
     *		and retrieved from the types. Compares the original
     *		IDL string with the retrieved text data.
     */
    void test_type_representation_xml();

private:

    SQLTableTester();
};

}}}}  // namespace rti::adapter::dis::test

#endif // SQL_TABLE_TESTER_HPP
