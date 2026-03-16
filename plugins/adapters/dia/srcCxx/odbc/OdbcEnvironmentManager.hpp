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

#ifndef ODBCENVIRONMENTMANAGER_HPP
#define ODBCENVIRONMENTMANAGER_HPP

#include <rti/routing/PropertySet.hpp>
#include <sqlext.h>
#include <sqltypes.h>
#include "Utils.hpp"

/**
 * @brief OdbcEnvironmentManager class. This class acts as a factory for
 * OdbcConnectionManager objects.
 */
namespace rti { namespace adapter { namespace dis { namespace odbc {

class OdbcEnvironmentManager {

public:
	OdbcEnvironmentManager();

	~OdbcEnvironmentManager();

	SQLHDBC create_connection() const;

private:
	SQLHENV environment_;
};

} } } } // namespace rti::adapter::dis::odbc

#endif
