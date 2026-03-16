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

#include <sqlext.h>
#include <sqltypes.h>
#include <dds/core/corefwd.hpp>
#include "SampledataFacade.hpp"
#include "Utils.hpp"

using namespace rti::adapter::dis::odbc;
using namespace dds::core::xtypes;

std::pair<std::string, long> SampledataFacade::get_json_value(const void *sample)
{
	auto value = transform_sample_to_json((const DynamicData *)sample);
	auto value_size = sizeof(SQLCHAR) * sizeof(value);
	return std::pair<std::string, long>(value, value_size);
};

std::string SampledataFacade::transform_sample_to_json(const DynamicData *sample)
{
	DDS_UnsignedLong jsonStrLen = 0;
	DDS_DynamicDataFormatter_to_json(
			&(sample->native()), NULL, &jsonStrLen, 0);
	auto jsonStr = DDS_String_alloc(jsonStrLen);
	if(jsonStr == NULL){
		std::runtime_error("Json Converter error");
	}
	DDS_DynamicDataFormatter_to_json(
			&(sample->native()), jsonStr, &jsonStrLen, 0);
	std::string output = std::string(jsonStr);
	DDS_String_free(jsonStr);

	return output;
}
