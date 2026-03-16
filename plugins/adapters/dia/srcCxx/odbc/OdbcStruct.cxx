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

#include "OdbcStruct.hpp"

namespace rti { namespace adapter { namespace dis { namespace odbc {

OdbcStruct::OdbcStruct(std::string name)
	: name_(name),
	decimal_digits_(0),
	buffer_length_(0),
	str_len_or_ind_ptr_(NULL)
{
}

std::string OdbcStruct::get_name()
{
	return name_;
}

SQLUSMALLINT OdbcStruct::get_parameter_number()
{
	return parameter_number_;
}

void OdbcStruct::set_parameter_number(SQLUSMALLINT parameter_number)
{
	parameter_number_ = parameter_number;
}

SQLSMALLINT OdbcStruct::get_value_type()
{
	return value_type_;
}

SQLSMALLINT OdbcStruct::get_parameter_type()
{
	return parameter_type_;
}

SQLULEN OdbcStruct::get_column_size()
{
	return column_size_;
}

SQLSMALLINT OdbcStruct::get_decimal_digits()
{
	return decimal_digits_;
}

SQLPOINTER OdbcStruct::get_parameter_value_ptr()
{
	return parameter_value_ptr_;
}

SQLLEN OdbcStruct::get_buffer_length()
{
	return buffer_length_;
}

SQLLEN *OdbcStruct::get_str_len_or_ind_ptr()
{
	return str_len_or_ind_ptr_;
}

OdbcStructInt::OdbcStructInt(
		const std::string &name,
		std::pair<long long, long> (*get_value)(const void *))
	: OdbcStruct(name), get_value_(get_value)
{
	value_type_ = SQL_C_LONG;
	parameter_type_ = SQL_BIGINT;
	decimal_digits_ = 0;
}


void OdbcStructInt::update(const void *data)
{
	auto data_and_size = get_value_(data);
	value_ = data_and_size.first;
	parameter_value_ptr_ = &value_;
	auto length = data_and_size.second;
	column_size_ = length;
	buffer_length_ = length;
	str_len_or_ind_ptr_ = &length;
}

OdbcStructBinary::OdbcStructBinary(
		const std::string &name,
		std::pair<std::vector<uint8_t>, long> (*get_value)(const void *))
	: OdbcStruct(name), get_value_(get_value)
{
	value_type_ = SQL_C_BINARY;
	parameter_type_ = SQL_BINARY;
	decimal_digits_ = 0;
}

void OdbcStructBinary::update(const void *data)
{
	std::pair<std::vector<uint8_t>, long> data_and_size = get_value_(data);
	value_ = data_and_size.first;
	parameter_value_ptr_ = value_.data();
	length_ = data_and_size.second;
	column_size_ = length_;
	buffer_length_ = length_;
	str_len_or_ind_ptr_ = &length_;
}

OdbcStructChar::OdbcStructChar(
		const std::string &name,
		std::pair<std::string, long> (*get_value)(const void *))
	: OdbcStruct(name), get_value_(get_value)
{
	value_type_ = SQL_C_CHAR;
	parameter_type_ = SQL_VARCHAR;
	decimal_digits_ = 0;
}

void OdbcStructChar::update(const void *data)
{
	auto data_and_size = get_value_(data);
	value_ = data_and_size.first;
	parameter_value_ptr_ = reinterpret_cast<unsigned char *>(&value_[0]);
	auto length = data_and_size.second;
	column_size_ = length;
	buffer_length_ = length;
	string_length_ = SQL_NTS;
	str_len_or_ind_ptr_ = &string_length_;
}

OdbcStructUint64::OdbcStructUint64(const std::string& name)
	: OdbcStruct(name), value_(0)
{
	value_type_ = SQL_C_UBIGINT;
	parameter_type_ = SQL_BIGINT;
	decimal_digits_ = 0;
	column_size_ = sizeof(uint64_t);
	buffer_length_ = sizeof(uint64_t);
	parameter_value_ptr_ = &value_;
}

void OdbcStructUint64::update(const void *)
{
	// Increment the counter on each update
	value_++;
	str_len_or_ind_ptr_ = &buffer_length_;
}

OdbcStructContainer::OdbcStructContainer() : current_parameter_number_(1)
{
}

OdbcStructContainer::~OdbcStructContainer()
{}

const std::vector<std::unique_ptr<OdbcStruct>>&
OdbcStructContainer::get_metadata_odbc_structs() const
{
	return metadata_odbc_structs_;
}

const std::vector<std::unique_ptr<OdbcStruct>>&
OdbcStructContainer::get_sample_odbc_structs() const
{
	return sample_odbc_structs_;
}

void OdbcStructContainer::add_metadata_odbc_struct(
		std::unique_ptr<OdbcStruct>&& metadata_odbc_struct)
{
	metadata_odbc_struct->set_parameter_number(current_parameter_number_);
	current_parameter_number_++;

	metadata_odbc_structs_.push_back(std::move(metadata_odbc_struct));
}

void OdbcStructContainer::add_sample_odbc_struct(
		std::unique_ptr<OdbcStruct>&& sample_odbc_struct)
{
	sample_odbc_struct->set_parameter_number(current_parameter_number_);
	current_parameter_number_++;

	sample_odbc_structs_.push_back(std::move(sample_odbc_struct));
}

void OdbcStructContainer::update_metadata_odbc_structs(
		const dds::sub::SampleInfo *sample_info)
{
		for (auto& metadata_odbc_struct : metadata_odbc_structs_) {
			metadata_odbc_struct->update(sample_info);
		}
}

void OdbcStructContainer::update_sample_odbc_structs(
		const dds::core::xtypes::DynamicData *sample)
{
		for (auto& sample_odbc_struct : sample_odbc_structs_) {
			sample_odbc_struct->update(sample);
		}
}

} } } } // namespace rti::adapter::dis::odbc
