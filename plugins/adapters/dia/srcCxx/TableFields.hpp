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

#ifndef TABLEFIELDS_HPP
#define TABLEFIELDS_HPP

#include <dds/core/corefwd.hpp>
#include <rti/routing/PropertySet.hpp>
#include <dds/sub/SampleInfo.hpp>
#include <rti/routing/StreamInfo.hpp>
#include "OdbcStruct.hpp"
#include "DiaConnection.hpp"

namespace rti { namespace adapter { namespace dis {

class Formatter;

/**
 * @brief TableFields class. Used in order to prepare the database
 * and program for its normal behaviour, i.d, perform regular insertions.
 */
class TableFields
{
public:
	TableFields(odbc::OdbcStructContainer *);

	/**
	 * @brief
	 *
	 * @param info StreamInfo object that initilizes access to specific
	 *		metadata
	 *
	 * @param formatter pointer to Formatter object that initilizes access
	 *		to sample data in the way given by the formatter
	 *
	 * @param topic_properties Topic configuration properties
	 */
	void initialize_components(
			const routing::StreamInfo& info,
			const std::unique_ptr<Formatter>& formatter,
            const std::shared_ptr<TopicProperties>& topic_properties);

	/**
	 * @brief Adds a field to the sample data fields
	 *
	 * @param odbc_struct pointer to OdbcStruct object to be added to the
	 *		list of sample odbc_structs
	 *
	 * @param field OdbcFields object to be added to the fields list
	 */
	void add_data(
            std::unique_ptr<rti::adapter::dis::odbc::OdbcStruct>&& odbc_struct,
            rti::adapter::dis::odbc::OdbcField field);

	/**
	 * @brief Adds a field to the metadata fields
	 *
	 * @param odbc_struct pointer to OdbcStruct object to be added to the
	 *		list of metadata odbc_structs
	 *
	 * @param field OdbcFields object to be added to the fields list
	 */
	void add_metadata(
			std::unique_ptr<rti::adapter::dis::odbc::OdbcStruct>&& odbc_struct,
		    rti::adapter::dis::odbc::OdbcField field);

	const std::vector<rti::adapter::dis::odbc::OdbcField>& fields() const;
	const std::unique_ptr<rti::adapter::dis::odbc::OdbcField>& instance_history_field() const;

	odbc::OdbcStructContainer * odbc_struct_container() const;

private:
	std::vector<rti::adapter::dis::odbc::OdbcField> fields_;
    std::unique_ptr<rti::adapter::dis::odbc::OdbcField> instance_history_key_;
	odbc::OdbcStructContainer *odbc_struct_container_;

	void initialize_metadata(
            const std::shared_ptr<TopicProperties>& topic_properties);
	void initialize_other_data();
};

/**
 * @brief Formatter class. This class contains all the logic necessary to
 * process samples depending on the formatting option provided by the DIS
 * configuration
 */
class Formatter {
public:
	virtual void initialize_sampledata(
			TableFields&,
			const routing::StreamInfo&) const = 0;
};


/**
 * @brief FormatterFactory class. A factory for Formatters.
 */
class FormatterFactory {

public:
	std::unique_ptr<Formatter> create_formatter(
			const TopicStorageFormat& format,
			const routing::StreamInfo&) const;
};


/**
 * @brief JsonFormatter class. Manages samples to be formatted in Json format.
 */
class JsonFormatter: public Formatter{

public:
	explicit JsonFormatter() {};

	void initialize_sampledata(
			TableFields&,
			const routing::StreamInfo&) const;
};

/**
 * @brief SqlFormatter class. Manages samples to be formatted in Sql serialized
 * format.
 */
class SqlFormatter: public Formatter{

public:
	explicit SqlFormatter() {};

	void initialize_sampledata(
			TableFields&,
			const routing::StreamInfo&) const;
};

} } } // namespace rti::adapter::dis

#endif // TABLEFIELDS_HPP

