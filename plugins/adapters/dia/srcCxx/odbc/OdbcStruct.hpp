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

#ifndef ODBCSTRUCT_HPP
#define ODBCSTRUCT_HPP

#include <dds/sub/SampleInfo.hpp>
#include <rti/core/InstanceHandle.hpp>
#include <rti/core/xtypes/DynamicDataImpl.hpp>
#include <sqlext.h>
#include <sqltypes.h>
#include <string>

namespace rti { namespace adapter { namespace dis { namespace odbc {

enum class OdbcType {
    integer,
    string,
    binary,
    large_binary,
    auto_increment
};

/**
 * @brief Fields class. Used to link a name and its type.
 */
struct OdbcField
{
public:
	std::string name;
	OdbcType type;
	std::string qualifiers;

	OdbcField(std::string name, OdbcType type) : name(name), type(type){};
	OdbcField(std::string name, OdbcType type, std::string qualifiers):
		name(name), type(type), qualifiers(qualifiers){};
};

/**
 * @brief OdbcStruct class. Contains all necessary fields to perform a binding
 * operation required to insert samples in the database.
 */
class OdbcStruct
{
public:
	std::string get_name();
	SQLUSMALLINT get_parameter_number();
	void set_parameter_number(SQLUSMALLINT parameter_number);
	SQLSMALLINT get_value_type();
	SQLSMALLINT get_parameter_type();
	SQLULEN  get_column_size();
	SQLSMALLINT get_decimal_digits();
	SQLPOINTER get_parameter_value_ptr();
	SQLLEN get_buffer_length();
	SQLLEN *get_str_len_or_ind_ptr();

	virtual void update(const void *) = 0;

	virtual ~OdbcStruct() = default;

	OdbcStruct(const OdbcStruct &) = default;

protected:
	std::string name_;
	SQLUSMALLINT parameter_number_;
	SQLSMALLINT value_type_;
	SQLSMALLINT parameter_type_;
	SQLULEN  column_size_;
	SQLSMALLINT decimal_digits_;
	SQLPOINTER parameter_value_ptr_;
	SQLLEN buffer_length_;
	SQLLEN *str_len_or_ind_ptr_;

	void update_length(long length);

	OdbcStruct(std::string field_name);
};

class OdbcStructInt : public OdbcStruct
{
public:
	OdbcStructInt(
			const std::string& name,
			std::pair<long long, long> (*get_value)(const void *));

	virtual void update(const void *) override;

private:
	long value_;

	std::pair<long long, long> (*get_value_)(const void *);
};

class OdbcStructBinary : public OdbcStruct
{
public:
	OdbcStructBinary(
			const std::string& name,
			std::pair<std::vector<uint8_t>, long>
			(*get_value)(const void *));

	virtual void update(const void *) override;

private:
	std::vector<uint8_t> value_;
	long length_;
	std::pair<std::vector<uint8_t>, long> (*get_value_)(const void *);
};

class OdbcStructChar : public OdbcStruct
{
public:
	OdbcStructChar(
			const std::string& name,
			std::pair<std::string, long>(*get_value)(const void *));

	virtual void update(const void *) override;

private:
	std::string value_;
	SQLLEN string_length_;

	std::pair<std::string, long> (*get_value_)(const void *);
};

class OdbcStructUint64 : public OdbcStruct
{
public:
	OdbcStructUint64(const std::string& name);

	virtual void update(const void *) override;

private:
	uint64_t value_;
};

/**
 * @brief OdbcStructContainer class. Manages a collection of metadata and
 * sample odbc structs asociated to a SqlTable object.
 */
class OdbcStructContainer
{
public:
	OdbcStructContainer();

	OdbcStructContainer(OdbcStructContainer&& rhs) = default;

	~OdbcStructContainer();

	const std::vector<std::unique_ptr<OdbcStruct>>& get_sample_odbc_structs() const;
	const std::vector<std::unique_ptr<OdbcStruct>>& get_metadata_odbc_structs() const;
	
	void add_sample_odbc_struct(std::unique_ptr<OdbcStruct>&&);
	void add_metadata_odbc_struct(std::unique_ptr<OdbcStruct>&&);

	void update_sample_odbc_structs(const dds::core::xtypes::DynamicData *sample);
	void update_metadata_odbc_structs(const dds::sub::SampleInfo *sample_info);

private:
	std::vector<std::unique_ptr<OdbcStruct>> sample_odbc_structs_;
	std::vector<std::unique_ptr<OdbcStruct>> metadata_odbc_structs_;
	unsigned int current_parameter_number_;
};

} } } } // namespace rti::adapter::dis::odbc

#endif

