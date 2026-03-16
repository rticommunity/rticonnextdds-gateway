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

#ifndef ODBCSTATEMENTMANAGER_HPP
#define ODBCSTATEMENTMANAGER_HPP

#include <rti/routing/PropertySet.hpp>
#include "OdbcConnectionManager.hpp"
#include "Utils.hpp"
#include <sqlext.h>
#include <sqltypes.h>

namespace rti { namespace adapter { namespace dis { namespace odbc {

enum class OdbcStatementKind {
    custom = 0,
    insert = 1,
    remove = 2,
    update = 3,
    count_rows = 4,
    oldest_sample = 5,
    COUNT = 6 // Sentinel value for number of enum values
};

/**
 * @brief Helper function to convert OdbcStatementKind to array index
 */
inline constexpr std::size_t to_index(OdbcStatementKind kind) {
    return static_cast<std::size_t>(kind);
}

/**
 * @brief OdbcStatementManager class. This class acts proxy for the Odbc API
 * necessary for the DIS project providing extra functionality and error
 * management capabilities.
 */
class OdbcStatementManager {

public:
	OdbcStatementManager(
            const std::shared_ptr<OdbcConnectionManager>& connection);

	~OdbcStatementManager();

	// Delete copy constructor and copy assignment to prevent double-free
	OdbcStatementManager(const OdbcStatementManager&) = delete;
	OdbcStatementManager& operator=(const OdbcStatementManager&) = delete;

	// Allow move semantics
	OdbcStatementManager(OdbcStatementManager&&) = default;
	OdbcStatementManager& operator=(OdbcStatementManager&&) = default;

	SQLHSTMT get_statement(OdbcStatementKind kind) const;

    void allocate_statement(OdbcStatementKind kind);

	/**
	 * @brief Attempts to execute any given query
	 *
	 * @param query String object with the query to be executed
	 */
	SQLRETURN execute_query(const std::string&) const;

	/**
	 * @brief Prepares insertion query
	 *
	 * @param kind OdbcStatementKind specifying which statement to prepare
	 * @param query String object with the query to insertion query to be
	 *		prepared
	 */
	SQLRETURN prepare_query(
            OdbcStatementKind kind,
            const std::string& insertion_query) const;

	/**
	 * @brief Binds a parameter to a variable
	 *
	 * @param kind OdbcStatementKind specifying which statement to bind to
	 *
	 * @param parameter_number SQLUSMALLINT number of the parameter to be
	 *		binded
	 *
	 * @param value_type SQLSMALLINT C type of the value to be binded
	 *
	 * @param parameter_type SQLSMALLINT SQL type of the value to be binded
	 *
	 * @param column_size SQLULEN
	 *
	 * @param decimal_digits SQLSMALLINT in case of numeric type, number of
	 *		decimal digits of such number
	 *
	 * @param parameter_value_ptr SQLPOINTER(void pointer) to the variable
	 *		being binded
	 *
	 * @param buffer_length SQLLEN total size of the value contained by the
	 *		variable being binded
	 *
	 * @param strlen_or_indptr SQLLEN String terminator. It is only relevant
	 *		when the datatype of the variable being binded is String
	 */
	void bind_parameter(
            OdbcStatementKind kind,
			const SQLUSMALLINT& parameter_number,
			const SQLSMALLINT& value_type,
			const SQLSMALLINT& parameter_type,
			const SQLULEN& column_size,
			const SQLSMALLINT& decimal_digits,
			const SQLPOINTER& parameter_value_ptr,
			const SQLLEN& buffer_length,
			SQLLEN *strlen_or_indptr) const;

	/**
	 * @brief Resets the state of the SQLHSTMT. Necessary before
	 *		 executing every statement after the first
	 * 
	 * @param kind OdbcStatementKind specifying which statement to reset
	 */
	void reset_statement(OdbcStatementKind kind) const;

	/**
	 * @brief Attempt execution of insertion query
	 *
	 * @param kind OdbcStatementKind specifying which statement to execute
	 */
	SQLRETURN execute(OdbcStatementKind kind) const;

private:
    std::shared_ptr<OdbcConnectionManager> connection_;

    std::array<SQLHSTMT, to_index(OdbcStatementKind::COUNT)> statements_ {
            SQL_NULL_HSTMT, 
            SQL_NULL_HSTMT, 
            SQL_NULL_HSTMT,
            SQL_NULL_HSTMT,
            SQL_NULL_HSTMT,
            SQL_NULL_HSTMT};
};

} } } } // namespace rti::adapter::dis::odbc

#endif

