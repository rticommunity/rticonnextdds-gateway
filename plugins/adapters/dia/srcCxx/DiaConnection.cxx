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

#include "DiaConnection.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "DiaStreamWriter.hpp"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <rti/routing/Logger.hpp>
#include <reda/reda_string.h>

using namespace rti::routing;
using namespace rti::routing::adapter;
using namespace rti::adapter::dis;
using namespace rti::adapter::dis::odbc;

DiaConnection::DiaConnection(const PropertySet& properties,
        const OdbcEnvironmentManager& odbc_environment_manager)
    : odbc_connection_manager_(new OdbcConnectionManager(odbc_environment_manager)),
      default_properties_(std::make_shared<TopicProperties>()),
      type_representation_format_(TypeRepresentationFormat::XCDR)
{
    odbc_connection_manager_->connect_to_database(properties);

    // Extract query_converter from connection properties
    auto query_converter_it = properties.find("query_converter");
    if (query_converter_it != properties.end()) {
        if (query_converter_it->second == "mariadb") {
            query_converter_ = QueryConverterFormat::mariadb;
        } else if (query_converter_it->second == "postgresql") {
            query_converter_ = QueryConverterFormat::postgresql;
        } else {
            throw std::runtime_error(
                    "Invalid 'query_converter' format: must be set to a valid value (mariadb, postgresql)");
        }
    }

    // Parse type_representation_format from connection properties
	auto type_repr_format = properties.find("type_representation_format");
	if (type_repr_format != properties.end()) {
		if (type_repr_format->second == "XCDR" || type_repr_format->second == "xcdr") {
			type_representation_format_ = TypeRepresentationFormat::XCDR;
		} else if (type_repr_format->second == "XML" || type_repr_format->second == "xml") {
			type_representation_format_ = TypeRepresentationFormat::XML;
		} else {
			throw std::runtime_error(
					"Invalid value for type_representation_format property: " +
					type_repr_format->second);
		}
	}

    // Read and parse topic_properties JSON file if provided
    auto topic_properties_it = properties.find("topic_properties");
    if (topic_properties_it != properties.end()) {
        parse_topic_properties_json(topic_properties_it->second);
    }
}

DiaConnection::~DiaConnection()
{}

void DiaConnection::parse_topic_properties_json(const std::string& json_file_path)
{
    std::ifstream json_file(json_file_path);

    if (!json_file.is_open()) {
        throw std::runtime_error(
            "Failed to open topic_properties file: " + json_file_path);
    }

    std::stringstream buffer;
    buffer << json_file.rdbuf();
    std::string json_content = buffer.str();
    json_file.close();

    try {
        rti::json::json_document json_doc;
        json_doc.parse(json_content.c_str());

        json_value *root = json_doc.first_node();
        if (!root || root->type != json_object) {
            throw std::runtime_error("Invalid JSON format: root must be an object");
        }

        // Find topic_properties array
        json_value *topic_props_value = nullptr;
        for (size_t i = 0; i < root->u.object.length; i++) {
            if (std::string(root->u.object.values[i].name) == "topic_properties") {
                topic_props_value = root->u.object.values[i].value;
                break;
            }
        }

        if (!topic_props_value || topic_props_value->type != json_array) {
            throw std::runtime_error("Invalid JSON format: 'topic_properties' must be an array");
        }

        size_t array_count = topic_props_value->u.array.length;

        for (size_t i = 0; i < array_count; i++) {
            json_value *config_value = topic_props_value->u.array.values[i];
            if (!config_value || config_value->type != json_object) {
                continue;
            }

            TopicConfiguration topic_config;
            topic_config.properties = std::make_shared<TopicProperties>();

            // Parse all properties in the configuration object
            for (size_t j = 0; j < config_value->u.object.length; j++) {
                std::string key_str(config_value->u.object.values[j].name);
                json_value *value = config_value->u.object.values[j].value;

                if (key_str == "topic_name_expression") {
                    if (value->type != json_string) {
                        throw std::runtime_error(
                                "Invalid JSON format: 'topic_name_expression' must be a string");
                    }
                    if (value->u.string.ptr == nullptr || strlen(value->u.string.ptr) == 0) {
                        throw std::runtime_error(
                                "Invalid JSON value: 'topic_name_expression' can't be empty");
                    }

                    topic_config.topic_name_expression = value->u.string.ptr;
                } else if (key_str == "delete_on_dispose") {
                    if (value->type != json_boolean) {
                        throw std::runtime_error(
                                "Invalid JSON format: 'delete_on_dispose' must be a bolean");
                    }

                    topic_config.properties->delete_on_dispose = value->u.boolean;
                } else if (key_str == "drop_table_on_dispose") {
                    if (value->type != json_boolean) {
                        throw std::runtime_error(
                                "Invalid JSON format: 'drop_table_on_dispose' must be a bolean");
                    }

                    topic_config.properties->drop_table_on_dispose = value->u.boolean;
                } else if (key_str == "delete_type_on_dispose") {
                    if (value->type != json_boolean) {
                        throw std::runtime_error(
                                "Invalid JSON format: 'delete_type_on_dispose' must be a bolean");
                    }

                    topic_config.properties->delete_type_on_dispose = value->u.boolean;
                } else if (key_str == "instance_history") {
                    if (value->type != json_integer) {
                        throw std::runtime_error(
                                "Invalid JSON format: 'instance_history' must be an integer");
                    }

                    if (value->u.integer < -1) {
                        throw std::runtime_error(
                                "Invalid JSON value: 'instance_history' can't be lower than -1");
                    }

                    topic_config.properties->instance_history = value->u.integer;
                } else if (key_str == "format") {
                    if (value->type != json_string) {
                        throw std::runtime_error(
                                "Invalid JSON format: 'format' must be a string");
                    }

                    if (strcmp(value->u.string.ptr, "SQL") == 0) {
                        throw std::runtime_error(
                                "Invalid JSON value: SQL format is not supported");
                    } else if (strcmp(value->u.string.ptr, "JSON") == 0) {
                        topic_config.properties->format = TopicStorageFormat::json;
                    } else {
                        throw std::runtime_error(
                                "Invalid JSON value: 'format' must be set to a valid value (JSON, SQL)");
                    }
                }
            }

            topic_configurations_.push_back(topic_config);
        }

        Logger::instance().local(
            "Loaded " + std::to_string(topic_configurations_.size()) + 
            " topic configuration(s) from " + json_file_path);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Failed to parse topic_properties JSON file: " + 
            json_file_path + " - " + e.what());
    }
}

bool DiaConnection::match_topic_pattern(const std::string& pattern, const std::string& topic_name) const
{
    return (0 == REDAString_fnmatch(pattern.c_str(), topic_name.c_str(), 0));
}

std::shared_ptr<TopicProperties> DiaConnection::get_properties_for_topic(const std::string& topic_name) const
{
    // Find the first matching configuration and use it
    for (const auto& config : topic_configurations_) {
        if (match_topic_pattern(config.topic_name_expression, topic_name)) {
            return config.properties;
        }
    }
    // Return default properties if no match found
    return default_properties_;
}

TypeRepresentationFormat DiaConnection::type_representation_format() const
{
	return type_representation_format_;
}

StreamWriter * DiaConnection::create_stream_writer(
        Session *session,
        const StreamInfo& info,
        const PropertySet& properties)
{
    return new DiaStreamWriter(
            properties,
            get_properties_for_topic(info.stream_name()),
            info,
            odbc_connection_manager_,
            query_converter_,
            type_representation_format_);
}

void DiaConnection::delete_stream_writer(StreamWriter *writer)
{
    delete writer;
}
