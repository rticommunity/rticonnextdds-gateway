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

#include "TableFields.hpp"
#include "SqlTable.hpp"
#include "OdbcStruct.hpp"
#include "OdbcEnvironmentManager.hpp"
#include "MetadataFacade.hpp"
#include "SampledataFacade.hpp"
#include <rti/routing/Logger.hpp>

namespace rti { namespace adapter { namespace dis {

using namespace dds::core::xtypes;
using namespace dds::sub;
using namespace rti::adapter::dis::odbc;
using namespace rti::routing;

TableFields::TableFields(
        OdbcStructContainer *odbc_struct_container) :
    odbc_struct_container_(odbc_struct_container)
{

}

void TableFields::initialize_components(
        const routing::StreamInfo& info,
        const std::unique_ptr<Formatter>& formatter,
        const std::shared_ptr<TopicProperties>& topic_properties)
{
    initialize_metadata(topic_properties);
    formatter->initialize_sampledata(*this, info);
    initialize_other_data();
}

void TableFields::initialize_metadata(
        const std::shared_ptr<TopicProperties>& topic_properties)
{
    add_metadata(
            std::unique_ptr<OdbcStruct>(
                    new OdbcStructInt(
                        "sampleInfo_reception_timestamp_seconds",
                        &MetadataFacade::sampleinfo_reception_timestamp_seconds)),
            OdbcField("sampleInfo_reception_timestamp_seconds", OdbcType::integer));
    add_metadata(
            std::unique_ptr<OdbcStruct>(
                    new OdbcStructInt(
                        "sampleInfo_reception_timestamp_nanoseconds",
                        &MetadataFacade::
                        sampleinfo_reception_timestamp_nanoseconds)),
            OdbcField("sampleInfo_reception_timestamp_nanoseconds", OdbcType::integer));
    add_metadata(
            std::unique_ptr<OdbcStruct>(
                    new OdbcStructBinary(
                        "sampleInfo_instance_handle",
                        &MetadataFacade::
                        sampleinfo_instance_handle)),
            OdbcField("sampleInfo_instance_handle", OdbcType::binary));

    // Add RTI_HISTORY_ID as the first column only if instance_history > 1
    if (topic_properties->instance_history > 1 
                || topic_properties->instance_history == -1) {
        instance_history_key_.reset(
                new OdbcField("RTI_HISTORY_ID", OdbcType::auto_increment));

        add_metadata(
                std::unique_ptr<OdbcStruct>(
                        new OdbcStructUint64("RTI_HISTORY_ORDER")),
                OdbcField("RTI_HISTORY_ORDER", OdbcType::integer));
    }
}

void TableFields::initialize_other_data()
{}

void TableFields::add_metadata(
        std::unique_ptr<OdbcStruct>&& odbc_struct, OdbcField field)
{
    odbc_struct_container_->add_metadata_odbc_struct(std::move(odbc_struct));
    fields_.push_back(field);
}

void TableFields::add_data(
        std::unique_ptr<OdbcStruct>&& odbc_struct, OdbcField field)
{
    odbc_struct_container_->add_sample_odbc_struct(std::move(odbc_struct));
    fields_.push_back(field);
}

const std::vector<OdbcField>& TableFields::fields() const
{
    return fields_;
}

const std::unique_ptr<OdbcField>& TableFields::instance_history_field() const
{
    return instance_history_key_;
}

OdbcStructContainer * TableFields::odbc_struct_container() const
{
	return odbc_struct_container_;
}

std::unique_ptr<Formatter> FormatterFactory::create_formatter(
        const TopicStorageFormat& format,
        const routing::StreamInfo& info) const
{
    switch (format) {
    case TopicStorageFormat::json:
        return std::unique_ptr<Formatter>(new JsonFormatter());
    case TopicStorageFormat::sql:
        return std::unique_ptr<Formatter>(new SqlFormatter());
    default:
        throw std::runtime_error("Incorrect formatter type");
    }
}

void JsonFormatter::initialize_sampledata(
        TableFields& component_initializer,
        const routing::StreamInfo&) const
{
    component_initializer.add_data(
            std::unique_ptr<OdbcStruct>(
                new OdbcStructChar(
                    "json_sample",
                    &SampledataFacade::get_json_value)),
            OdbcField("json_sample", OdbcType::string));
}

void SqlFormatter::initialize_sampledata(
        TableFields& component_initializer,
        const StreamInfo& info) const
{
    // Iterate over stream info to get the topic structure and create a
    // specific odbc_struct for each field
    // for (auto field: info.type_info().dynamic_type.native()._data) {
    // }
}

} } } // namespace rti::adapter::dis
