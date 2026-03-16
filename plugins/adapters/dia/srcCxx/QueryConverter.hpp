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

#ifndef QUERYCONVERTER_HPP
#define QUERYCONVERTER_HPP

#include <rti/routing/PropertySet.hpp>
#include "OdbcStruct.hpp"

namespace rti { namespace adapter { namespace dis {

enum class QueryConverterFormat {
    mariadb,
    postgresql
};

class QueryConverter
{
public:
	/**
	 * @brief Returns a string with the quote representation of the
	 *		character quotes in the current database.
	 */
	virtual std::string quotes() = 0;

	/**
	 * @brief Returns a string with the type representation of a
	 *		given type in the current database.
	 *
	 * @param type String object to be transfomed
	 */
	virtual std::string transform_type(
            const rti::adapter::dis::odbc::OdbcType& type) = 0;

	/**
	 * @brief Returns the upsert query for the current database.
	 *		Upsert inserts a row or updates it if the primary key already exists.
	 *
	 * @param table_name The name of the table
	 * @param columns Vector of column names to insert/update
	 */
	virtual std::string upsert_query(
			const std::string& table_name,
            const std::vector<rti::adapter::dis::odbc::OdbcField>& columns) = 0;
};


class QueryConverterFactory
{
public:
    std::unique_ptr<QueryConverter> create_query_converter(
            const QueryConverterFormat& query_converter) const;
};

class MariaDBQueryConverter : public QueryConverter
{
public:
    virtual std::string quotes() override;

    virtual std::string transform_type(
            const rti::adapter::dis::odbc::OdbcType& type) override;

    virtual std::string upsert_query(
            const std::string& table_name,
            const std::vector<
                    rti::adapter::dis::odbc::OdbcField>& columns) override;
};

class PostgreQueryConverter : public QueryConverter
{
public:
    virtual std::string quotes() override;

    virtual std::string transform_type(
            const rti::adapter::dis::odbc::OdbcType& type) override;

    virtual std::string upsert_query(
            const std::string& table_name,
            const std::vector<
                    rti::adapter::dis::odbc::OdbcField>& columns) override;
};

} } } // namespace rti::adapter::dis

#endif
