#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>


void extract_error(char* fn, SQLHANDLE sql_handle, SQLSMALLINT sql_small_int)
{
	SQLCHAR sql_char_state[7];
	SQLCHAR sql_char_message[256];
	SQLSMALLINT sql_small_int_message_length;
	SQLINTEGER sql_integer_i;
	SQLINTEGER sql_integer_native_error;
	SQLRETURN sql_return;

	fprintf(stderr, "\nThe driver reported the following error %s\n", fn);
	do {
		sql_return = SQLGetDiagRec(sql_small_int, sql_handle, ++sql_integer_i, sql_char_state,
			&sql_integer_native_error, sql_char_message, sizeof(sql_char_message), &sql_small_int_message_length);
		if (SQL_SUCCEEDED(sql_return)) {
			printf("%s:%ld:%ld:%s\n", sql_char_state, (long)sql_integer_i, (long)sql_integer_native_error, sql_char_message);
		}
	} while (sql_return == SQL_SUCCESS);
}

int main()
{
	SQLHENV sql_henv = SQL_NULL_HENV;
	SQLHDBC sql_hdbc = SQL_NULL_HDBC;
	SQLHSTMT sql_hstmt = SQL_NULL_HSTMT;

	do {
		// Allocate environment handle.
		SQLRETURN sql_return = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql_henv);
		if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
			extract_error("SQLAllocHandle(SQL_HANDLE_ENV)", sql_henv, SQL_HANDLE_ENV);
			break;
		}

		// Set the ODBC version environment attribute.
		sql_return = SQLSetEnvAttr(sql_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
		if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
			extract_error("SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION)", sql_henv, SQL_HANDLE_ENV);
			break;
		}

		// Allocate connection handle.
		sql_return = SQLAllocHandle(SQL_HANDLE_DBC, sql_henv, &sql_hdbc);
		if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
			extract_error("SQLAllocHandle(SQL_HANDLE_DBC)", sql_hdbc, SQL_HANDLE_DBC);
			break;
		}

		// Set login timeout to 5 seconds.
		sql_return = SQLSetConnectAttr(sql_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
		if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
			extract_error("SQLSetConnectAttr(SQL_LOGIN_TIMEOUT)", sql_hdbc, SQL_HANDLE_DBC);
			break;
		}

		// Connect to data source.
		SQLCHAR sql_char[1024];
		SQLSMALLINT sql_small_int;
		sql_return = SQLDriverConnect(sql_hdbc, NULL, "Driver=ODBC Driver 17 for SQL Server;Server=172.30.95.206;Uid=SA;Pwd=SSlx&gw7!;database=master",
			SQL_NTS, sql_char, sizeof(sql_char), &sql_small_int, SQL_DRIVER_NOPROMPT);
		if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
			extract_error("SQLDriverConnect()", sql_hdbc, SQL_HANDLE_DBC);
			break;
		}

		// Allocate statement handle.
		sql_return = SQLAllocHandle(SQL_HANDLE_STMT, sql_hdbc, &sql_hstmt);
		if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
			extract_error("SQLAllocHandle(SQL_HANDLE_STMT)", sql_hstmt, SQL_HANDLE_STMT);
			break;
		}

		// Execute.
		sql_return = SQLExecDirect(sql_hstmt, (SQLCHAR*)"SELECT @@Version", SQL_NTS);
		if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
			extract_error("SQLExecDirect()", sql_hstmt, SQL_HANDLE_STMT);
			break;
		}

		// Bind columns.
		sql_return = SQLBindCol(sql_hstmt, 1, SQL_C_CHAR, &sql_char, sizeof(sql_char), 0);
		if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
			extract_error("SQLBindCol()", sql_hstmt, SQL_HANDLE_STMT);
			break;
		}

		// Fetch and print each row of data until SQL_NO_DATA returned.
		while (1) {
			sql_return = SQLFetch(sql_hstmt);
			if (sql_return != SQL_SUCCESS && sql_return != SQL_SUCCESS_WITH_INFO) {
				break;
			}
			printf("Result is: %s", sql_char);
		}

		if (sql_return != SQL_NO_DATA) {
			extract_error("SQLFetch()", sql_hstmt, SQL_HANDLE_STMT);
		}

	} while (0);


	if (sql_hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, sql_hstmt);
	}
	if (sql_hdbc != SQL_NULL_HDBC) {
		SQLDisconnect(sql_hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, sql_hdbc);
	}
	if (sql_henv != SQL_NULL_HENV) {
		SQLFreeHandle(SQL_HANDLE_ENV, sql_henv);
	}

	printf("\nComplete.\n");
	return 0;
}