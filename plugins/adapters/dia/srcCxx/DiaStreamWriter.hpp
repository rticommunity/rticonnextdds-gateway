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

#ifndef DIASTREAMWRITER_HPP
#define DIASTREAMWRITER_HPP

#include "SqlTable.hpp"
#include "QueryConverter.hpp"
#include "DiaConnection.hpp"
#include <rti/routing/adapter/AdapterPlugin.hpp>
#include <rti/routing/adapter/StreamWriter.hpp>
#include <sqltypes.h>

namespace rti { namespace adapter { namespace dis {

// Forward declaration
struct TopicProperties;

/**
 * @brief StreamWriter class. It performs DIS operations such as writting samples
 * to the given database by making use of a SqlTable object which gets instantiated
 * using the OdbcStatementManager provided by DiaConnection.
 */
class DiaStreamWriter : public rti::routing::adapter::DynamicDataStreamWriter {

public:
    explicit DiaStreamWriter(
            const routing::PropertySet& properties,
            const std::shared_ptr<TopicProperties>& topic_properties,
            const routing::StreamInfo& info,
            const std::shared_ptr<odbc::OdbcConnectionManager>& statement_manager,
            const QueryConverterFormat& query_converter,
            TypeRepresentationFormat type_representation_format);

    ~DiaStreamWriter();

private:
    static int extract_domain_id(const routing::PropertySet& properties);

public:
    int write(
            const std::vector<dds::core::xtypes::DynamicData *>&,
            const std::vector<dds::sub::SampleInfo *>&) final;

    // Public accessors for testing
    int32_t instance_history() const;
    bool delete_on_dispose() const;

private:
    SqlTable sql_table_;
};

} } } // namespace rti::adapter::dis

#endif
