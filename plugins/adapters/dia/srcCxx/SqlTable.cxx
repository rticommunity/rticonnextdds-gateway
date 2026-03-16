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

#include "SqlTable.hpp"
#include "DiaConnection.hpp"
#include "OdbcStruct.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "MetadataFacade.hpp"
#include "SampledataFacade.hpp"
#include "Utils.hpp"
#include <rti/routing/Logger.hpp>
#include <sql.h>
#include <sqlext.h>

namespace rti { namespace adapter { namespace dis {

using namespace dds::core::xtypes;
using namespace dds::sub;
using namespace rti::adapter::dis::odbc;
using namespace rti::routing;

SqlTable::SqlTable(
        const std::shared_ptr<TopicProperties>& topic_properties,
        const StreamInfo& info,
        const std::shared_ptr<odbc::OdbcConnectionManager>& odbc_connection_manager,
        const QueryConverterFormat& query_converter,
        TypeRepresentationFormat type_representation_format,
        int domain_id) 
    : topic_properties_(topic_properties),
      table_name_(info.stream_name() + "@" + std::to_string(domain_id)),
      odbc_statement_manager_(odbc_connection_manager),
      info_(info),
      queries_(Queries(
            (QueryConverterFactory()).create_query_converter(query_converter))),
      formatter_((FormatterFactory()).create_formatter(topic_properties->format, info)),
      type_representation_format_(type_representation_format),
      update_parameter_number_(DEFAULT_UPDATE_PARAMATER_ID)
{
    prepare_database();
}

SqlTable::~SqlTable()
{
    try {
        if(topic_properties_->drop_table_on_dispose){
            execute_query(queries_.drop_data_table(table_name_));
        }
        if(topic_properties_->delete_type_on_dispose){
            execute_query(queries_.delete_type(info_.stream_name()));
        }
    } catch (const std::exception& e) {
        // Log error but don't throw from destructor
        rti::routing::Logger::instance().error(
            std::string("Error in SqlTable destructor: ") + e.what());
    }
}

std::shared_ptr<OdbcConnectionManager> SqlTable::create_statement(
        const PropertySet& properties)
{
    std::shared_ptr<OdbcConnectionManager> connection_manager = 
            std::make_shared<OdbcConnectionManager>(OdbcEnvironmentManager());

    connection_manager->connect_to_database(properties);
    return connection_manager;
}


void SqlTable::prepare_database()
{
    TableFields component_initializer(&odbc_struct_container_);
    component_initializer.initialize_components(info_, formatter_, topic_properties_);

    allocate_statements();
    initialize_data_table(component_initializer);
    initialize_type_table(component_initializer);
    prepare_data_insertion_statement(component_initializer);
    if (topic_properties_->delete_on_dispose) {
        prepare_data_remove_statement();
    }
    if (topic_properties_->instance_history > 1 
            || topic_properties_->instance_history == -1) {
        prepare_data_update_statement(component_initializer);
        prepare_instance_samples_statement();
        prepare_old_sample_statement();
    }
}

void SqlTable::initialize_data_table(
        const TableFields& component_initializer)
{
    execute_query(
            queries_.create_data_table(
                    table_name_,
                    component_initializer,
                    topic_properties_->instance_history));
}

void SqlTable::initialize_type_table(
        const TableFields& component_initializer)
{
    // Create types table
    execute_query(queries_.create_types_table(type_representation_format_));
    
    // Prepare and execute insert statement with type representation
    insert_type_representation();
}

void SqlTable::insert_type_representation()
{
    SQLLEN type_repr_length = SQL_NULL_DATA;
    void* data_ptr = nullptr;
    SQLLEN data_size = 0;

    std::vector<uint8_t> xcdr_data;
    std::string xml_data;
    SQLSMALLINT value_type = SQL_NULL_DATA;
    SQLSMALLINT param_type = SQL_NULL_DATA;

    switch(type_representation_format_) {
    case TypeRepresentationFormat::XCDR:
    {
        // Get XCDR representation using MetadataFacade
        xcdr_data = odbc::MetadataFacade::extract_type_xcdr(&info_);
        data_size = xcdr_data.size();
        if (data_size > 0) {
            type_repr_length = data_size;
            data_ptr = xcdr_data.data();
        }
        value_type = SQL_C_BINARY;
        param_type = SQL_VARBINARY;
    }
    break;

    case TypeRepresentationFormat::XML:
    {
        // Get XML representation using MetadataFacade
        xml_data = odbc::MetadataFacade::extract_type_xml(&info_);
        data_size = xml_data.length();

        if (data_size > 0) {
            type_repr_length = data_size;
            data_ptr = const_cast<char*>(xml_data.c_str());
        }

        value_type = SQL_C_CHAR;
        param_type = SQL_VARCHAR;
    }
    break;

    default:
        throw std::runtime_error("unsupported type representation format");
    }

    // Prepare the insert query
    std::string query = queries_.insert_type(
            info_, 
            type_representation_format_);
    odbc_statement_manager_.prepare_query(OdbcStatementKind::custom, query);

    // Bind the type representation parameter
    odbc_statement_manager_.bind_parameter(
            OdbcStatementKind::custom,
            1,  // Parameter number
            value_type,
            param_type,
            data_size,  // Column size
            0,  // Decimal digits
            data_ptr,  // Parameter value
            data_size,  // Buffer length
            &type_repr_length);  // String length or indicator

    // Execute the insert
    odbc_statement_manager_.execute(OdbcStatementKind::custom);
}

void SqlTable::allocate_statements() 
{
    odbc_statement_manager_.allocate_statement(OdbcStatementKind::custom);
    odbc_statement_manager_.allocate_statement(OdbcStatementKind::insert);

    if (topic_properties_->delete_on_dispose) {
        odbc_statement_manager_.allocate_statement(OdbcStatementKind::remove);
    }

    if (topic_properties_->instance_history > 1 
            || topic_properties_->instance_history == -1) {
        odbc_statement_manager_.allocate_statement(OdbcStatementKind::update);
        odbc_statement_manager_.allocate_statement(
                OdbcStatementKind::count_rows);
        odbc_statement_manager_.allocate_statement(
                OdbcStatementKind::oldest_sample);
    }
}
void SqlTable::prepare_data_insertion_statement(
        const TableFields& component_initializer)
{
    // Use UPSERT when instance_history is 0 or 1 to replace existing rows
    if (topic_properties_->instance_history == 0 || topic_properties_->instance_history == 1) {
        odbc_statement_manager_.prepare_query(
                OdbcStatementKind::insert,
                queries_.prepare_upsert(
                        table_name_,
                        component_initializer));
    } else {
        odbc_statement_manager_.prepare_query(
                OdbcStatementKind::insert,
                queries_.prepare_insertion(
                        table_name_,
                        component_initializer));
    }
}

void SqlTable::prepare_data_remove_statement()
{
    odbc_statement_manager_.prepare_query(
            OdbcStatementKind::remove,
            queries_.delete_instance(table_name_));
}

void SqlTable::prepare_data_update_statement(
        const TableFields& component_initializer)
{
    odbc_statement_manager_.prepare_query(
            OdbcStatementKind::update,
            queries_.prepare_update(
                    table_name_,
                    component_initializer));
}

void SqlTable::prepare_instance_samples_statement()
{
    odbc_statement_manager_.prepare_query(
            OdbcStatementKind::count_rows,
            queries_.sample_instance_count(table_name_));
}

void SqlTable::prepare_old_sample_statement()
{
    odbc_statement_manager_.prepare_query(
            OdbcStatementKind::oldest_sample,
            queries_.oldest_sample(table_name_));
}

OdbcStatementManager& SqlTable::get_statement_manager()
{
    return odbc_statement_manager_;
}

bool SqlTable::delete_on_dispose() const
{
    return topic_properties_->delete_on_dispose;
}

int32_t SqlTable::instance_history() const
{
    return topic_properties_->instance_history;
}

int32_t SqlTable::instance_samples(const dds::sub::SampleInfo *sample_info)
{
    // Reset statement to clear previous execution state
    odbc_statement_manager_.reset_statement(OdbcStatementKind::count_rows);

    // Get instance handle value
    odbc_struct_container_.update_metadata_odbc_structs(sample_info);
    auto& odbc_structs = odbc_struct_container_.get_metadata_odbc_structs();
    auto& instance_handle_struct = odbc_structs[METADATA_INSTANCE_HANDLE_INDEX];

    // Bind the instance handle parameter
    odbc_statement_manager_.bind_parameter(
            OdbcStatementKind::count_rows,
            1,
            instance_handle_struct->get_value_type(),
            instance_handle_struct->get_parameter_type(),
            instance_handle_struct->get_column_size(),
            instance_handle_struct->get_decimal_digits(),
            instance_handle_struct->get_parameter_value_ptr(),
            instance_handle_struct->get_buffer_length(),
            instance_handle_struct->get_str_len_or_ind_ptr());

    // Execute the prepared statement
    odbc_statement_manager_.execute(OdbcStatementKind::count_rows);

    // Fetch the count result
    auto statement = 
            odbc_statement_manager_.get_statement(OdbcStatementKind::count_rows);
    int32_t count = 0;

    SQLRETURN retcode = SQLFetch(statement);
    if (SQL_SUCCEEDED(retcode)) {
        SQLLEN indicator;
        SQLGetData(statement, 1, SQL_C_SLONG, &count, sizeof(count), &indicator);
    } else {
        auto diagnostics = get_handle_diagnostics(statement, SQL_HANDLE_STMT);
        rti::routing::Logger::instance().warn(
                "Failed to fetch parameters: " + diagnostics);
    }

    return count;
}

int32_t SqlTable::old_sample(const dds::sub::SampleInfo *sample_info)
{
    // Reset statement to clear previous execution state
    odbc_statement_manager_.reset_statement(OdbcStatementKind::oldest_sample);

    // Get instance handle value
    odbc_struct_container_.update_metadata_odbc_structs(sample_info);
    auto& odbc_structs = odbc_struct_container_.get_metadata_odbc_structs();
    auto& instance_handle_struct = odbc_structs[METADATA_INSTANCE_HANDLE_INDEX];

    // Bind the instance handle parameter
    odbc_statement_manager_.bind_parameter(
            OdbcStatementKind::oldest_sample,
            1,
            instance_handle_struct->get_value_type(),
            instance_handle_struct->get_parameter_type(),
            instance_handle_struct->get_column_size(),
            instance_handle_struct->get_decimal_digits(),
            instance_handle_struct->get_parameter_value_ptr(),
            instance_handle_struct->get_buffer_length(),
            instance_handle_struct->get_str_len_or_ind_ptr());

    // Execute the prepared statement
    odbc_statement_manager_.execute(OdbcStatementKind::oldest_sample);

    // Fetch the RTI_HISTORY_ID result
    auto statement = 
            odbc_statement_manager_.get_statement(
                    OdbcStatementKind::oldest_sample);
    int32_t row_id = -1;

    SQLRETURN retcode = SQLFetch(statement);
    if (SQL_SUCCEEDED(retcode)) {
        SQLLEN indicator;
        SQLGetData(statement, 1, SQL_C_SLONG, &row_id, sizeof(row_id), &indicator);
    } else {
        auto diagnostics = get_handle_diagnostics(statement, SQL_HANDLE_STMT);
        rti::routing::Logger::instance().warn(
                "Failed to fetch parameters: " + diagnostics);
    }


    return row_id;
}


void SqlTable::execute_query(const std::string& query)
{
    odbc_statement_manager_.execute_query(query);
}

int SqlTable::insert_sample(
        const DynamicData *sample,
        const dds::sub::SampleInfo *sample_info)
{
    update_odbc_structs(sample, sample_info);
    bind_structs();
    return odbc_statement_manager_.execute(OdbcStatementKind::insert);
}

int SqlTable::update_sample(
		const DynamicData *sample,
		const dds::sub::SampleInfo *sample_info)
{
    update_odbc_structs(sample, sample_info);

    // Reset parameter counter for UPDATE statement
    update_parameter_number_ = DEFAULT_UPDATE_PARAMATER_ID;

    // Bind all fields except instance_handle for the SET clause
    bind_odbc_structs_for_update(
            odbc_struct_container_.get_metadata_odbc_structs());
    bind_odbc_structs_for_update(
            odbc_struct_container_.get_sample_odbc_structs());

    int32_t rti_history_id = 0;
    // Bind RTI_HISTORY_ID for the WHERE clauseRE clause
    SQLLEN row_id_indicator = 0;
    odbc_statement_manager_.bind_parameter(
            OdbcStatementKind::update,
            update_parameter_number_,
            SQL_C_SLONG,
            SQL_INTEGER,
            0,
            0,
            &rti_history_id,
            sizeof(rti_history_id),
            &row_id_indicator);

    return odbc_statement_manager_.execute(OdbcStatementKind::update);
}

int SqlTable::update_sample(
        const DynamicData *sample,
        const dds::sub::SampleInfo *sample_info,
        int32_t rti_history_id)
{
    update_odbc_structs(sample, sample_info);

    // Reset parameter counter for UPDATE statement
    update_parameter_number_ = DEFAULT_UPDATE_PARAMATER_ID;

    // Bind all fields except instance_handle for the SET clause
    bind_odbc_structs_for_update(
            odbc_struct_container_.get_metadata_odbc_structs());
    bind_odbc_structs_for_update(
            odbc_struct_container_.get_sample_odbc_structs());

    // Bind RTI_HISTORY_ID for the WHERE clause
    SQLLEN row_id_indicator = 0;
    odbc_statement_manager_.bind_parameter(
            OdbcStatementKind::update,
            update_parameter_number_++,
            SQL_C_SLONG,
            SQL_INTEGER,
            0,
            0,
            &rti_history_id,
            sizeof(rti_history_id),
            &row_id_indicator);

    // Bind instance_handle for the WHERE clause
    auto& odbc_structs = odbc_struct_container_.get_metadata_odbc_structs();
    auto& instance_handle_struct = odbc_structs[METADATA_INSTANCE_HANDLE_INDEX];
    odbc_statement_manager_.bind_parameter(
            OdbcStatementKind::update,
            update_parameter_number_,
            instance_handle_struct->get_value_type(),
            instance_handle_struct->get_parameter_type(),
            instance_handle_struct->get_column_size(),
            instance_handle_struct->get_decimal_digits(),
            instance_handle_struct->get_parameter_value_ptr(),
            instance_handle_struct->get_buffer_length(),
            instance_handle_struct->get_str_len_or_ind_ptr());

    return odbc_statement_manager_.execute(OdbcStatementKind::update);
}

void SqlTable::remove_instance(const dds::sub::SampleInfo *sample_info)
{
    odbc_statement_manager_.reset_statement(OdbcStatementKind::remove);

    // Get instance handle value
    odbc_struct_container_.update_metadata_odbc_structs(sample_info);
    auto& odbc_structs = odbc_struct_container_.get_metadata_odbc_structs();
    auto& odbc_struct = odbc_structs[METADATA_INSTANCE_HANDLE_INDEX];

    // Bind the instance handle parameter
    odbc_statement_manager_.bind_parameter(
            OdbcStatementKind::remove,
            1,
            odbc_struct->get_value_type(),
            odbc_struct->get_parameter_type(),
            odbc_struct->get_column_size(),
            odbc_struct->get_decimal_digits(),
            odbc_struct->get_parameter_value_ptr(),
            odbc_struct->get_buffer_length(),
            odbc_struct->get_str_len_or_ind_ptr());

    // Execute the prepared statement
    odbc_statement_manager_.execute(OdbcStatementKind::remove);
}

void SqlTable::update_odbc_structs(
        const DynamicData *sample,
        const dds::sub::SampleInfo *sample_info)
{
    odbc_struct_container_.update_metadata_odbc_structs(sample_info);
    odbc_struct_container_.update_sample_odbc_structs(sample);
}

void SqlTable::bind_structs()
{
    bind_odbc_structs(odbc_struct_container_.get_metadata_odbc_structs());
    bind_odbc_structs(odbc_struct_container_.get_sample_odbc_structs());
}

void SqlTable::bind_odbc_structs(
        const std::vector<std::unique_ptr<OdbcStruct>>& odbc_structs)
{
    for (const auto& odbc_struct: odbc_structs) {
        odbc_statement_manager_.bind_parameter(
                OdbcStatementKind::insert,
                odbc_struct->get_parameter_number(),
                odbc_struct->get_value_type(),
                odbc_struct->get_parameter_type(),
                odbc_struct->get_column_size(),
                odbc_struct->get_decimal_digits(),
                odbc_struct->get_parameter_value_ptr(),
                odbc_struct->get_buffer_length(),
                odbc_struct->get_str_len_or_ind_ptr());
    }
}

void SqlTable::bind_odbc_structs_for_update(
        const std::vector<std::unique_ptr<OdbcStruct>>& odbc_structs)
{
    for (const auto& odbc_struct: odbc_structs) {
        // Skip instance_handle field for UPDATE statement
        if (odbc_struct->get_parameter_number() == METADATA_INSTANCE_HANDLE_INDEX + 1) {
            continue;
        }

        odbc_statement_manager_.bind_parameter(
                OdbcStatementKind::update,
                update_parameter_number_++,
                odbc_struct->get_value_type(),
                odbc_struct->get_parameter_type(),
                odbc_struct->get_column_size(),
                odbc_struct->get_decimal_digits(),
                odbc_struct->get_parameter_value_ptr(),
                odbc_struct->get_buffer_length(),
                odbc_struct->get_str_len_or_ind_ptr());
    }
}

int SqlTable::get_next_parameter_number() const
{
    // Calculate total parameters: metadata + sample data - instance_handle + 1
    auto metadata_count = odbc_struct_container_.get_metadata_odbc_structs().size();
    auto sample_count = odbc_struct_container_.get_sample_odbc_structs().size();
    // -1 for skipped instance_handle, +1 for the next parameter
    return metadata_count + sample_count;
}

} } } // namespace rti::adapter::dis
