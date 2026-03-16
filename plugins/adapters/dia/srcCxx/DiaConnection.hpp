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

#ifndef DIACONNECTION_HPP
#define DIACONNECTION_HPP

#include <rti/routing/adapter/AdapterPlugin.hpp>
#include <rti/routing/adapter/Connection.hpp>
#include "OdbcEnvironmentManager.hpp"
#include "OdbcConnectionManager.hpp"
#include "QueryConverter.hpp"
#include <vector>
#include <string>

namespace rti { namespace adapter { namespace dis {

enum class TopicStorageFormat {
    sql,
    json
};

/**
 * @brief Structure to hold topic-specific properties from JSON configuration
 */
struct TopicProperties {
    bool delete_on_dispose = false;
    bool drop_table_on_dispose = false;
    bool delete_type_on_dispose = false;
    int64_t instance_history = 0;
    TopicStorageFormat format = TopicStorageFormat::json;
};

/**
 * @brief Structure to hold topic pattern and its properties
 */
struct TopicConfiguration {
    std::string topic_name_expression;
    std::shared_ptr<TopicProperties> properties;
};

/**
 * @brief Type representation format for storing in database
 */
enum class TypeRepresentationFormat
{
	XML,   // IDL/XML string representation
	XCDR   // Binary XCDR serialized TypeCode
};

/**
 * @brief Connection class. It acts as a factory for DiaStreamWriter objects.
 * Upon creation DiaStreamWriter's are passed a OdbcStatementManager provided
 * by the odbc_connection_manager_ manager provided by the DiaAdapter.
 */
class DiaConnection : public rti::routing::adapter::Connection {

public:
	DiaConnection(
		    const rti::routing::PropertySet&,
		    const odbc::OdbcEnvironmentManager&);

	~DiaConnection();

	rti::routing::adapter::StreamWriter * create_stream_writer(
			rti::routing::adapter::Session *session,
			const rti::routing::StreamInfo& info,
			const rti::routing::PropertySet& properties) final;

	void delete_stream_writer(rti::routing::adapter::StreamWriter *writer) final;

	TypeRepresentationFormat type_representation_format() const;

private:
	void parse_topic_properties_json(const std::string& json_file_path);
	bool match_topic_pattern(const std::string& pattern, const std::string& topic_name) const;
	std::shared_ptr<TopicProperties> get_properties_for_topic(const std::string& topic_name) const;

    std::shared_ptr<odbc::OdbcConnectionManager> odbc_connection_manager_;
    std::vector<TopicConfiguration> topic_configurations_;
    std::shared_ptr<TopicProperties> default_properties_;
    QueryConverterFormat query_converter_;
	TypeRepresentationFormat type_representation_format_;
};

} } } // namespace rti::adapter::dis

#endif
