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

#ifndef SAMPLEDATAFACADE_HPP
#define SAMPLEDATAFACADE_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace rti { namespace adapter { namespace dis { namespace odbc {

/**
 * @brief SampledataFacade class. Composed of static methods, it is a collection.
 * of functions necesary to retrieve sampledata information from a DynamicData
 * structure.
 */
class SampledataFacade {

public:
	static std::pair<std::string, long> get_json_value(const void *sample);

	static std::string transform_sample_to_json(
			const dds::core::xtypes::DynamicData *sample);
};

} } } } // namespace rti::adapter::dis::odbc

#endif
