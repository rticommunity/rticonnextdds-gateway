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

#include "DiaStreamWriter.hpp"
#include "DiaConnection.hpp"
#include "SqlTable.hpp"
#include <sql.h>
#include <set>
#include <iostream>
#include <iomanip>

using namespace rti::routing;
using namespace rti::routing::adapter;
using namespace rti::adapter::dis;

DiaStreamWriter::DiaStreamWriter(
        const routing::PropertySet& properties,
        const std::shared_ptr<TopicProperties>& topic_properties,
        const routing::StreamInfo& info,
        const std::shared_ptr<odbc::OdbcConnectionManager>& connection_manager,
        const QueryConverterFormat& query_converter,
        TypeRepresentationFormat type_representation_format)
    : sql_table_(
            topic_properties,
            info,
            connection_manager,
            query_converter,
            type_representation_format,    
            extract_domain_id(properties))
{}

int DiaStreamWriter::extract_domain_id(const routing::PropertySet& properties)
{
    auto it = properties.find("dds.domain_id");
    if (it != properties.end()) {
        return std::stoi(it->second);
    }
    return 0; // Default domain ID
}

int32_t DiaStreamWriter::instance_history() const
{
    return sql_table_.instance_history();
}

bool DiaStreamWriter::delete_on_dispose() const
{
    return sql_table_.delete_on_dispose();
}

DiaStreamWriter::~DiaStreamWriter() {}

int DiaStreamWriter::write(
        const std::vector<dds::core::xtypes::DynamicData *>& samples,
        const std::vector<dds::sub::SampleInfo *>& infos)
{
    int samples_number = samples.size();
    int insertions = 0;
    
    // If instance_history > 1, filter samples to keep only the newest ones per instance
    if (sql_table_.instance_history() > 1 || sql_table_.instance_history() == -1) {
        // Track indices of samples to skip
        std::set<int> samples_to_skip;
        
        // Samples are sorted by source timestamp and instance
        // For each instance, count samples and mark older ones to skip
        for (int i = 0; i < samples_number; i++) {
            if (!infos[i]->valid()) {
                continue;
            }
            
            // Count how many valid samples exist for this instance after current position
            int samples_after = 0;
            for (int j = i + 1; j < samples_number; j++) {
                if (infos[j]->valid() && 
                    infos[j]->instance_handle() == infos[i]->instance_handle()) {
                    samples_after++;
                }
            }
            
            // If there are instance_history or more samples after this one,
            // skip this sample as it will be superseded
            if (samples_after >= sql_table_.instance_history() && sql_table_.instance_history() != -1) {
                samples_to_skip.insert(i);
            }
        }
        
        // Process filtered samples
        for (int i = 0; i < samples_number; i++) {
            if (samples_to_skip.count(i) > 0) {
                continue; // Skip this sample
            }
            
            if (infos[i]->valid()) {
                // Limited history: check how many rows exist for this instance
                int32_t row_count = sql_table_.instance_samples(infos[i]);
                if (row_count < sql_table_.instance_history() 
                        || sql_table_.instance_history() == -1) {
                    // Haven't reached limit yet, insert new sample
                    insertions += sql_table_.insert_sample(samples[i], infos[i]);
                    continue;
                }

                // Limit reached, update oldest sample
                int32_t old_row_id = sql_table_.old_sample(infos[i]);

                insertions += sql_table_.update_sample(
                        samples[i], infos[i], old_row_id);
            } else {
                // Check for dispose
                if (sql_table_.delete_on_dispose() && infos[i]->state().instance_state()
                        == dds::sub::status::InstanceState::not_alive_disposed()) {
                    sql_table_.remove_instance(infos[i]);
                }
            }
        }
    } else {
        // instance_history <= 1, process all samples normally
        for (int i = 0; i < samples_number; i++) {
            if (infos[i]->valid()) {
                insertions += sql_table_.insert_sample(samples[i], infos[i]);
            } else {
                // Check for dispose
                if (sql_table_.delete_on_dispose() && infos[i]->state().instance_state()
                        == dds::sub::status::InstanceState::not_alive_disposed()) {
                    sql_table_.remove_instance(infos[i]);
                }
            }
        }
    }

    return insertions;
}
