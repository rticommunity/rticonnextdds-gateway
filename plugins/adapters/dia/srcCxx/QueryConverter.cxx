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

#include "QueryConverter.hpp"
#include <iostream>
#include <sql.h>
#include <stdexcept>

namespace rti { namespace adapter { namespace dis {

using namespace rti::routing;
using namespace rti::adapter::dis::odbc;

std::unique_ptr<QueryConverter> QueryConverterFactory::create_query_converter(
        const QueryConverterFormat& query_converter) const
{
    switch(query_converter) {
    case QueryConverterFormat::mariadb:
        return std::unique_ptr<QueryConverter>(new MariaDBQueryConverter());
    case QueryConverterFormat::postgresql:
        return std::unique_ptr<QueryConverter>(new PostgreQueryConverter());
    default:
        throw std::runtime_error("Incorrect database");
    }
}


std::string MariaDBQueryConverter::quotes()
{
	return "`";
}

std::string MariaDBQueryConverter::transform_type(const OdbcType& type)
{
    switch(type) {
    case OdbcType::integer:
		return "INT UNSIGNED";
    break;

    case OdbcType::string:
		return "VARCHAR(1000)";
    break;

    case OdbcType::binary:
		return "BINARY(16)";
    break;

    case OdbcType::large_binary:
		return "LONGBLOB";
    break;

    case OdbcType::auto_increment:
		return "INT UNSIGNED AUTO_INCREMENT";
    break;

    default:
        throw std::runtime_error("Wrong SQL type");
    }
}

std::string MariaDBQueryConverter::upsert_query(
		const std::string& table_name,
        const std::vector<rti::adapter::dis::odbc::OdbcField>& columns)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly,
	// but safe if users filter malicious topic names (all data values use parameterized queries)
	std::string query;
	query.append("REPLACE INTO ")
			.append(quotes())
			.append(table_name)
			.append(quotes())
			.append("(");
	
	std::string interrogation_marks = "(";
	
	for(auto element = columns.begin(); element < columns.end()-1; element++) {
		query.append((*element).name).append(", ");
		interrogation_marks.append("?, ");
	}
	query.append((*(columns.end()-1)).name + ")");
	interrogation_marks.append("?)");
	
	query.append(" VALUES ").append(interrogation_marks);
	
	return query;
}

std::string PostgreQueryConverter::quotes()
{
	return "\"";
}

std::string PostgreQueryConverter::transform_type(const OdbcType& type)
{
    switch(type) {
    case OdbcType::integer:
		return "INT";
    break;

    case OdbcType::string:
		return "VARCHAR(1000)";
    break;

    case OdbcType::binary:
		return "BYTEA";
    break;

    case OdbcType::large_binary:
		return "BYTEA";
    break;

    case OdbcType::auto_increment:
		return "SERIAL";
    break;

    default:
        throw std::runtime_error("Wrong SQL type");
    }
}

std::string PostgreQueryConverter::upsert_query(
		const std::string& table_name,
        const std::vector<rti::adapter::dis::odbc::OdbcField>& columns)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly,
	// but safe if users filter malicious topic names (all data values use parameterized queries)
	std::string query;
	query.append("INSERT INTO ")
			.append(quotes())
			.append(table_name)
			.append(quotes())
			.append("(");
	
	std::string interrogation_marks = "(";
	std::string update_clause = "";
	
	for(auto element = columns.begin(); element < columns.end()-1; element++) {
		query.append((*element).name).append(", ");
		interrogation_marks.append("?, ");
		// Skip sampleInfo_instance_handle in UPDATE clause since it's the key
		if ((*element).name != "sampleInfo_instance_handle") {
			if (!update_clause.empty()) {
				update_clause.append(", ");
			}
			update_clause.append((*element).name).append(" = EXCLUDED.").append((*element).name);
		}
	}
	auto last_col = (*(columns.end()-1)).name;
	query.append(last_col).append(")");
	interrogation_marks.append("?)");
	
	// Add UPDATE clause for non-key columns
	if (last_col != "sampleInfo_instance_handle") {
		if (!update_clause.empty()) {
			update_clause.append(", ");
		}
		update_clause.append(last_col).append(" = EXCLUDED.").append(last_col);
	}
	
	query.append(" VALUES ").append(interrogation_marks)
			.append(" ON CONFLICT (sampleInfo_instance_handle) DO UPDATE SET ")
			.append(update_clause);
	
	return query;
}

} } } // namespace rti::adapter::dis
