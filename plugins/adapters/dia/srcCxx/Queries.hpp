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

#ifndef QUERIES_HPP
#define QUERIES_HPP

#include <string>
#include <rti/routing/StreamInfo.hpp>
#include "QueryConverter.hpp"
#include "TableFields.hpp"
#include "OdbcStruct.hpp"
#include "DiaConnection.hpp"

namespace rti { namespace adapter { namespace dis {

/**
 * @brief Queries class. Responsable for creating necesary queries for SqlTable
 * operations.
 */
class Queries
{
public:
	/**
	 * Constructor. Creates a Queries object
	 *
	 * @param query_converter Pointer to QueryConverter object that resolves
	 *		differences in the Data Manipulation Language(DML) and
	 *		Data Definition Language(DDL) so that any Database Model
	 *		can be used if a proper QueryConverter for it has been
	 *		defined
	 */
	Queries(std::unique_ptr<QueryConverter>&& query_converter);

	Queries(Queries&&) = default;

	~Queries();

	/**
	 * @brief Returns the query needed to create the data table
	 *
	 * @param table_name String object with the data table name
	 *
	 * @param fields Vector of OdbcFields that correspond to the columns of the
	 *		data table
	 *
	 * @param instance_history Instance history configuration value
	 */
	std::string create_data_table(
			const std::string& table_name,
            const TableFields& table_fields,
			int32_t instance_history);

	/**
	 * @brief Returns the query needed to drop the data table
	 *
	 * @param table_name String object with the data table name
	 */
	std::string drop_data_table(const std::string& table_name);

	/**
	 * @brief Returns the query needed to create the types table if
	 *		it does not exist
	 *
	 * @param format Type representation format (XML or XCDR)
	 */
	std::string create_types_table(TypeRepresentationFormat format);

	/**
	 * @brief Returns the query needed to insert the topic type in
	 *		the types table
	 *
	 * @param info StreamInfo containing type information
	 * @param format Type representation format (XML or XCDR)
	 */
	std::string insert_type(const routing::StreamInfo& info, TypeRepresentationFormat format);

	/**
	 * @brief Returns the query needed to delete the topic type from
	 *		the types table
	 */
	std::string delete_type(const std::string& type_name);

	/**
	 * @brief Returns the query needed to delete an instance from
	 *		the data table
	 */
    std::string delete_instance(const std::string& table_name);

	/**
	 * @brief Returns the query needed to execute insertions by
	 *		means of the OdbcStatementManager
	 */
	std::string prepare_insertion(
			const std::string& table_name,
			const TableFields& table_fields);

	/**
	 * @brief Returns the query needed to execute upsert (insert or replace)
	 *		operations when instance_history is 0
	 *
	 * @param table_name String object with the data table name
	 * @param columns Vector of column names to upsert
	 */
	std::string prepare_upsert(
			const std::string& table_name,
			const TableFields& table_fields);

	/**
	 * @brief Returns the query needed to update all fields except
	 *		instance_handle by means of the OdbcStatementManager
	 *
	 * @param table_name String object with the data table name
	 * @param columns Vector of column names to update
	 */
	std::string prepare_update(
			const std::string& table_name,
			const TableFields& table_fields);

	/**
	 * @brief Returns the query to count rows for a specific instance handle
	 *
	 * @param table_name String object with the data table name
	 */
	std::string sample_instance_count(const std::string& table_name);

	/**
	 * @brief Returns the query to get the oldest RTI_HISTORY_ID for a specific instance
	 *
	 * @param table_name String object with the data table name
	 */
	std::string oldest_sample(const std::string& table_name);

private:
	std::unique_ptr<QueryConverter> query_converter_;

};

} } } // namespace rti::adapter::dis

#endif
