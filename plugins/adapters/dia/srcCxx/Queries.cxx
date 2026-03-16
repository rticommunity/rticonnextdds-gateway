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

#include "Queries.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <dds/core/xtypes/DynamicType.hpp>
#include <ndds/ndds_c.h>
#include "SqlTable.hpp"

namespace rti { namespace adapter { namespace dis {

using namespace dds::core::xtypes;
using namespace rti::routing;
using namespace rti::adapter::dis::odbc;

Queries::Queries(std::unique_ptr<QueryConverter>&& query_converter) :
	query_converter_(std::move(query_converter))
{}

Queries::~Queries()
{}

std::string Queries::create_data_table(
		const std::string& table_name,
		const TableFields& table_fields,
		int32_t instance_history)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly,
	// but safe if users filter malicious topic names
	std::string query;
	query += "CREATE TABLE IF NOT EXISTS " + query_converter_->quotes() +
		table_name + query_converter_->quotes() + "(";

    if (instance_history > 1 || instance_history == -1) {
        auto& field = table_fields.instance_history_field();
		query.append((*field).name)
				.append(" ")
				.append(query_converter_->transform_type((*field).type))
				.append(" ")
				.append((*field).qualifiers)
				.append(", ");
    }

    auto& fields = table_fields.fields();
	auto begin = fields.begin();
	auto end = fields.end()-1;
	for(auto field = begin; field < end; field++) {
		query.append((*field).name)
				.append(" ")
				.append(query_converter_->transform_type((*field).type))
				.append(" ")
				.append((*field).qualifiers)
				.append(", ");
	}

	auto last_field = (fields.end()-1);
	query.append((*last_field).name).append(" ")
		.append(query_converter_->transform_type((*last_field).type));
	
	// Add primary key constraint
	if (instance_history > 1 || instance_history == -1) {
		query.append(", PRIMARY KEY (RTI_HISTORY_ID, sampleInfo_instance_handle)");
	} else {
		// Single primary key: sampleInfo_instance_handle only
		query.append(", PRIMARY KEY (sampleInfo_instance_handle)");
	}
	
	query.append(")");

	return query;
}

std::string Queries::drop_data_table(const std::string& table_name)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly,
	// but safe if users filter malicious topic names
	std::string query;
	query.append("DROP TABLE ")
			.append(query_converter_->quotes())
			.append(table_name)
			.append(query_converter_->quotes());
	return query;
}

std::string Queries::create_types_table(TypeRepresentationFormat format)
{
	// SQL Injection Safety: SAFE - no user input is concatenated into the query
	std::string query;
	query.append("CREATE TABLE IF NOT EXISTS types(")
			.append("topic_name ")
			.append(query_converter_->transform_type(OdbcType::string))
			.append(", ")
			.append("type ")
			.append(query_converter_->transform_type(OdbcType::string))
			.append(", ")
			.append("type_representation ");
	
	// Choose column type based on format
    switch(format) {
    case TypeRepresentationFormat::XCDR:
		query.append(query_converter_->transform_type(OdbcType::large_binary));
    break;

    case TypeRepresentationFormat::XML:
		query.append(query_converter_->transform_type(OdbcType::string));
    break;

    default:
        throw std::runtime_error("unsupported type representation format");
    }

	query.append(" )");
	return query;
}

std::string Queries::insert_type(
        const StreamInfo& info, 
        TypeRepresentationFormat format)
{
	// SQL Injection Safety: PARTIALLY SAFE - topic_name and type are concatenated directly,
	// but safe if users filter malicious topic/type names (type_representation uses parameterized query)
	std::string query;
	auto topic_name = info.stream_name();
	auto type = info.type_info().type_name();
	
	query.append("INSERT INTO types(topic_name, type, type_representation) VALUES('")
			.append(topic_name)
			.append("', '")
			.append(type)
			.append("', ?)");
	return query;
}

std::string Queries::delete_type(const std::string& topic_name)
{
	// SQL Injection Safety: PARTIALLY SAFE - topic_name is concatenated directly,
	// but safe if users filter malicious topic names
	std::string query;
	query.append("DELETE FROM types WHERE topic_name = '")
			.append(topic_name)
			.append("'");
	return query;
}

std::string Queries::delete_instance(const std::string& table_name)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly (not safe),
	// but WHERE clause uses parameterized query (safe)
	std::string query;
	query.append("DELETE FROM ")
			.append(query_converter_->quotes())
			.append(table_name)
			.append(query_converter_->quotes())
            .append(" WHERE sampleInfo_instance_handle = ?");
	return query;
}

std::string Queries::prepare_insertion(
		const std::string& table_name,
		const TableFields& table_fields)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly (not safe),
	// but all data values use parameterized queries (safe)
	std::string query;
	query.append("INSERT INTO ")
			.append(query_converter_->quotes())
			.append(table_name).append(query_converter_->quotes())
			.append("(");
	std::string interrogation_marks = "(";

    auto& columns = table_fields.fields();
	for(auto element = columns.begin(); element < columns.end()-1; ++element) {
		query.append((*element).name).append(", ");
		interrogation_marks.append("?, ");
	}
	query.append((*(columns.end()-1)).name + ")");
	interrogation_marks.append("?)");

	query.append(" VALUES ").append(interrogation_marks);

	return query;
}

std::string Queries::prepare_upsert(
		const std::string& table_name,
		const TableFields& table_fields)
{
	return query_converter_->upsert_query(table_name, table_fields.fields());
}

std::string Queries::prepare_update(
		const std::string& table_name,
		const TableFields& table_fields)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly (not safe),
	// but all data values use parameterized queries (safe)
	std::string query;
	query.append("UPDATE ")
			.append(query_converter_->quotes())
			.append(table_name)
			.append(query_converter_->quotes())
			.append(" SET ");

	// Add SET clause for all columns except instance_handle
    auto& columns = table_fields.fields();
	bool first = true;
	for (const auto& column : columns) {
		// Skip the instance_handle field
		if (column.name == "sampleInfo_instance_handle") {
			continue;
		}
		if (!first) {
			query.append(", ");
		}
		query.append(column.name).append(" = ?");
		first = false;
	}

	// Add WHERE clause with RTI_HISTORY_ID and instance_handle
	query.append(" WHERE RTI_HISTORY_ID = ? AND sampleInfo_instance_handle = ?");

	return query;
}

std::string Queries::sample_instance_count(const std::string& table_name)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly (not safe),
	// but WHERE clause uses parameterized query (safe)
	std::string query = "SELECT COUNT(*) FROM " + 
					   query_converter_->quotes() + table_name + 
					   query_converter_->quotes() + 
					   " WHERE sampleInfo_instance_handle = ?";
	return query;
}

std::string Queries::oldest_sample(const std::string& table_name)
{
	// SQL Injection Safety: PARTIALLY SAFE - table_name is concatenated directly (not safe),
	// but WHERE clause uses parameterized query (safe)
	std::string query = "SELECT RTI_HISTORY_ID FROM " + 
					   query_converter_->quotes() + table_name + 
					   query_converter_->quotes() + 
					   " WHERE sampleInfo_instance_handle = ?" +
					   " ORDER BY RTI_HISTORY_ORDER ASC";
	return query;
}

} } } // namespace rti::adapter::dis
