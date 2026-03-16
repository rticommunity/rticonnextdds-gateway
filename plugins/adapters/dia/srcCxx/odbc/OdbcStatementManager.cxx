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

#include "OdbcStatementManager.hpp"
#include <sql.h>
#include <rti/routing/Logger.hpp>
#include <sqltypes.h>

using namespace rti::adapter::dis::odbc;

OdbcStatementManager::OdbcStatementManager(
        const std::shared_ptr<OdbcConnectionManager>& connection)
    : connection_ (connection)
{
}

OdbcStatementManager::~OdbcStatementManager()
{
    for (auto& statement : statements_) {
        if (statement != SQL_NULL_HSTMT) {
            SQLFreeHandle(SQL_HANDLE_STMT, statement);
        }
    }
}

SQLHSTMT OdbcStatementManager::get_statement(OdbcStatementKind kind) const
{
	return statements_[to_index(kind)];
}

void OdbcStatementManager::allocate_statement(OdbcStatementKind kind)
{
    auto& statement = statements_[to_index(kind)];

	if (statement == SQL_NULL_HSTMT) {
        statement = connection_->create_statement();
    }
}

SQLRETURN OdbcStatementManager::execute_query(
		const std::string& statement_text) const
{
    auto& statement = statements_[to_index(OdbcStatementKind::custom)];
	
	// Check if statement is allocated, if not throw an error
	if (statement == SQL_NULL_HSTMT) {
		std::string error_text = "Statement not allocated. Cannot execute query: ";
		error_text.append(statement_text);
		throw std::runtime_error(error_text);
	}
	
	reset_statement(OdbcStatementKind::custom);

	try {
		SQLCHAR *char_statement_text =
				reinterpret_cast<unsigned char *>(
						const_cast<char *>(statement_text.c_str()));
		SQLINTEGER text_length = statement_text.length();
		SQLRETURN retcode = SQLExecDirect(
				statement,
				char_statement_text,
				text_length);

		auto diagnostics = get_handle_diagnostics(statement, SQL_HANDLE_ENV);
		if(!SQL_SUCCEEDED(retcode)) {
			std::string error_text;
			error_text.append("Unable to execute not prepared query: ")
					.append(statement_text);
			auto error_message = build_diagnostics(
					error_text,
					std::to_string(retcode),
					diagnostics);
			// Rollback on failure
			connection_->rollback_transaction();
			throw std::runtime_error(error_message);
		}

		// Commit on success
		connection_->commit_transaction();
		return retcode;
	} catch (...) {
		// Rollback on any exception
		connection_->rollback_transaction();
		throw;
	}
}

SQLRETURN OdbcStatementManager::prepare_query(
        OdbcStatementKind kind,
		const std::string& insertion_query) const
{
	auto char_insertion_query = 
		reinterpret_cast<unsigned char *>(
				const_cast<char *>(insertion_query.c_str()));
    auto& statement = statements_[to_index(kind)];

	auto retcode = SQLPrepare(statement, char_insertion_query , SQL_NTS);
	auto diagnostics = get_handle_diagnostics(statement, SQL_HANDLE_ENV);
	if (!SQL_SUCCEEDED(retcode)) {
		std::string error_text;
		error_text.append("Unable to prepare query: ")
                .append(insertion_query);
		auto error_message = build_diagnostics(
				error_text,
				std::to_string(retcode),
				diagnostics);
		throw std::runtime_error(error_message);
	}

	return retcode;
}

void OdbcStatementManager::bind_parameter(
        OdbcStatementKind kind,
		const SQLUSMALLINT& parameter_number,
		const SQLSMALLINT& value_type,
		const SQLSMALLINT& parameter_type,
		const SQLULEN& column_size,
		const SQLSMALLINT& decimal_digits,
		const SQLPOINTER& parameter_value_ptr,
		const SQLLEN& buffer_length,
		SQLLEN *strlen_or_indptr) const
{
    auto retcode = SQLBindParameter(
            statements_[to_index(kind)],
            parameter_number,
            SQL_PARAM_INPUT,
            value_type,
			parameter_type,
            column_size,
            decimal_digits, 
            parameter_value_ptr,
			buffer_length,
            strlen_or_indptr);

	auto diagnostics = get_handle_diagnostics(
            statements_[to_index(kind)],
            SQL_HANDLE_ENV);
	if (!SQL_SUCCEEDED(retcode)) {
		std::string error_text;
		error_text.append("Unable to bind parameter");
		auto error_message = build_diagnostics(
                error_text,
                std::to_string(retcode),
                diagnostics);
		throw std::runtime_error(error_message);
	}
}

void OdbcStatementManager::reset_statement(OdbcStatementKind kind) const
{
	SQLRETURN retcode;
    auto& statement = statements_[to_index(kind)];

    // Check if statement is allocated before attempting to reset
    if (statement == SQL_NULL_HSTMT) {
        return;
    }

    retcode = SQLFreeStmt(statement, SQL_RESET_PARAMS);
    if (!SQL_SUCCEEDED(retcode)) {
        auto diagnostics = get_handle_diagnostics(statement, SQL_HANDLE_STMT);
        rti::routing::Logger::instance().warn(
                "Failed to reset parameters: " + diagnostics);
    }

    retcode = SQLFreeStmt(statement, SQL_CLOSE);
    if (!SQL_SUCCEEDED(retcode)) {
        auto diagnostics = get_handle_diagnostics(statement, SQL_HANDLE_STMT);
        rti::routing::Logger::instance().warn(
                "Failed to close statement: " + diagnostics);
    }
}

SQLRETURN OdbcStatementManager::execute(OdbcStatementKind kind) const
{
    auto& statement = statements_[to_index(kind)];

	try {
		SQLRETURN retcode = SQLExecute(statement);
		if (!SQL_SUCCEEDED(retcode)) {
			std::string error_message;
			error_message += (std::string)"Unable to execute query:\n";
			error_message += "Return Code: " + std::to_string(retcode) + "\n";
			error_message += "Diagnostics: " +
					get_handle_diagnostics(statement, SQL_HANDLE_STMT);
			rti::routing::Logger::instance().error(error_message);
			// Rollback on failure
			connection_->rollback_transaction();
			return false;
		}

		// Commit on success
		connection_->commit_transaction();
		return true;
	} catch (...) {
		// Rollback on any exception
		connection_->rollback_transaction();
		throw;
	}
}
