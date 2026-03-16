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

#ifndef ODBCUTILS_HPP
#define ODBCUTILS_HPP

#include <sqlext.h>
#include <string>
#include <rti/core/xtypes/DynamicDataImpl.hpp>
#include <rti/routing/PropertySet.hpp>

namespace rti { namespace adapter { namespace dis { namespace odbc {

/**
 * Function. Builds a diagnostics string
 *
 * @param text String object with the problem description
 *
 * @param code String object with the error code related to the problem
 *
 * @param diagnostics String object with the ODBC related diagnostics message
 */
std::string build_diagnostics(
		const std::string& message,
		const std::string& code,
		const std::string& diagnostics);

/**
 * Function. Odtains a diagnostics string
 *
 * @param handle SQLHANDLE object from which diagnostics need to be extracted
 *
 * @param handle SQLSMALLINT object that indicates the type of handle from which
 *		diagnostics need to be extracted
 */
std::string get_handle_diagnostics(const SQLHANDLE& handle, const SQLSMALLINT& type);

} } } } // namespace rti::adapter::dis::odbc

#endif
