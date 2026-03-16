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

#ifndef SQLTABLE_HPP
#define SQLTABLE_HPP

#include <dds/core/corefwd.hpp>
#include <rti/routing/PropertySet.hpp>
#include <dds/sub/SampleInfo.hpp>
#include <rti/routing/StreamInfo.hpp>
#include "Queries.hpp"
#include "odbc/OdbcStatementManager.hpp"
#include "OdbcStruct.hpp"
#include "TableFields.hpp"
#include "Utils.hpp"
#include "QueryConverter.hpp"

namespace rti { namespace adapter { namespace dis {

// Forward declaration
struct TopicProperties;
enum class TypeRepresentationFormat;

// Metadata field indices
constexpr int METADATA_INSTANCE_HANDLE_INDEX = 2;
constexpr int DEFAULT_UPDATE_PARAMATER_ID = 1;

/**
 * @brief SqlTable class. This class contains all logic necesary for performing
 * operations to databases needed by the DisSteamWritter. This functionality is 
 * provided by invoking odbc calls via an OdbcStatementManager object.
 */
class SqlTable
{
public:
    /**
     * Constructor. Creates a SqlTable object
     *
     * @param topic_properties TopicProperties struct containing topic-specific
     *      configuration from JSON files
     *
     * @param info StreamInfo object containing information related to the
     *		specific topic
     *
     * @param statement_manager OdbcStatementManager object providing access
     *		to odbc functionality
     *
     * @param query_converter Query converter type (mariadb, postgresql)
     * 
     * @param type_representation_format Format for type representation (XML or XCDR)
     * 
     * @param domain_id DDS Domain ID
     */
    SqlTable(
            const std::shared_ptr<TopicProperties>& topic_properties,
            const routing::StreamInfo& info,
            const std::shared_ptr<odbc::OdbcConnectionManager>& odbc_connection_manager,
            const QueryConverterFormat& query_converter,
            TypeRepresentationFormat type_representation_format,
            int domain_id);

    SqlTable(rti::adapter::dis::SqlTable&&) = default;

    ~SqlTable();

    odbc::OdbcStatementManager& get_statement_manager();

    bool delete_on_dispose() const;

    int32_t instance_history() const;

    /**
     * @brief Counts the number of rows for a specific instance in the database
     *
     * @param info SampleInfo object containing the instance handle
     * @return Number of rows for this instance
     */
    int32_t instance_samples(const dds::sub::SampleInfo *info);

    /**
     * @brief Gets the RTI_HISTORY_ID of the oldest sample for a specific instance
     *
     * @param info SampleInfo object containing the instance handle
     * @return RTI_HISTORY_ID of the oldest sample for this instance
     */
    int32_t old_sample(const dds::sub::SampleInfo *info);

    /**
     * @brief Inserts a sample its database
     *
     * @param sample DynamicData object as provided by the DisSteamWritter
     *
     * @param info SampleInfo object as provided by the DisSteamWritter
     */
    int insert_sample(
            const dds::core::xtypes::DynamicData *sample,
            const dds::sub::SampleInfo *info);

    /**
     * @brief Updates a sample in the database
     *
     * @param sample DynamicData object as provided by the DisSteamWritter
     *
     * @param info SampleInfo object as provided by the DisSteamWritter
     */
    int update_sample(
            const dds::core::xtypes::DynamicData *sample,
            const dds::sub::SampleInfo *info);

    /**
     * @brief Updates a sample in the database
     *
     * @param sample DynamicData object as provided by the DisSteamWritter
     *
     * @param info SampleInfo object as provided by the DisSteamWritter
     *
     * @param rti_history_id The RTI_HISTORY_ID of the record to update
     */
    int update_sample(
            const dds::core::xtypes::DynamicData *sample,
            const dds::sub::SampleInfo *info,
            int32_t rti_history_id);

    void remove_instance(const dds::sub::SampleInfo *sample_info);

private:
    std::shared_ptr<TopicProperties> topic_properties_;
    std::string table_name_;
    odbc::OdbcStatementManager odbc_statement_manager_;
    std::unique_ptr<Formatter> formatter_;
    Queries queries_;
    odbc::OdbcStructContainer odbc_struct_container_;
    routing::StreamInfo info_;
    int update_parameter_number_;
    TypeRepresentationFormat type_representation_format_;

    std::shared_ptr<odbc::OdbcConnectionManager> create_statement(
            const routing::PropertySet& properties);

    void initialize_components();

    void prepare_database();

    /**
     * Private method. Creates data table if not exists
     */
    void initialize_data_table(const TableFields&);

    /**
     * Private method. Creates types table if not exists and inserts topic
     *		type in it if not present
     */
    void initialize_type_table(const TableFields&);

    /**
     * Private method. Inserts type representation into types table
     */
    void insert_type_representation();

    /**
     * Private method. Allocates Odbc statements
     */
    void allocate_statements();

    /**
     * Private method. Prepares odbc insertion query
     */
    void prepare_data_insertion_statement(const TableFields&);

    void prepare_data_remove_statement();

    void prepare_data_update_statement(const TableFields&);

    void prepare_instance_samples_statement();

    void prepare_old_sample_statement();

    /**
     * Private method. Updates the content of the OdbcStructs linked to the
     *		current type
     *
     * @param sample DynamicData object as provided by the DisSteamWritter
     *
     * @param info SampleInfo object as provided by the DisSteamWritter
     */
    void update_odbc_structs(
            const dds::core::xtypes::DynamicData *sample,
            const dds::sub::SampleInfo *sample_info);

    void bind_structs();

    /**
     * Private method. Bind the content of the OdbcStructs linked to the
     *		current type to the Odbc statement parameters
     */
    void bind_odbc_structs(
            const std::vector<std::unique_ptr<odbc::OdbcStruct>>& odbc_structs);

    /**
     * Private method. Bind the content of the OdbcStructs for UPDATE statement,
     *		skipping the instance_handle field
     */
    void bind_odbc_structs_for_update(
            const std::vector<std::unique_ptr<odbc::OdbcStruct>>& odbc_structs);

    /**
     * Private method. Get the next parameter number for binding
     */
    int get_next_parameter_number() const;

    /**
     * Private method. OdbcStatementManager execution of a given query
     */
    void execute_query(const std::string&);
};

} } } // namespace rti::adapter::dis

#endif

