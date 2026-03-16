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

#include "Utils.hpp"
#include <sql.h>

namespace rti { namespace adapter { namespace dis { namespace odbc {

using namespace dds::core::xtypes;

std::string build_diagnostics(
		const std::string& text,
		const std::string& code,
		const std::string& diagnostics)
{
	std::string message;
	return message.append("Message: ").append(text).append("\n")
            .append("Code: ").append(code).append("\n")
            .append("Diagnostics: ").append(diagnostics).append("\n\n");
}

std::string get_handle_diagnostics(const SQLHANDLE& handle, const SQLSMALLINT& type)
{
	SQLINTEGER i = 0;
	SQLINTEGER native = 0;
	SQLCHAR state[7] = "";
	SQLCHAR text[256] = "";
	SQLSMALLINT len = 0;
	SQLRETURN ret = SQL_SUCCESS;
	std::string diagnostics;

	while (ret == SQL_SUCCESS) {
		ret = SQLGetDiagRec(type, handle, ++i, state, &native, text,
			sizeof(text), &len);
		if (SQL_SUCCEEDED(ret)){
			diagnostics.append((char *)state);
			diagnostics.append(std::to_string(i));
			diagnostics.append(std::to_string(native));
			diagnostics.append((char *)text);
			diagnostics.append("\n");
		}
	}

	return diagnostics;
}

} } } } // namespace rti::adapter::dis::odbc
