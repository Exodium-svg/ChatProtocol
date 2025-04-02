#include "DbConn.h"

DbConn::DbConn(Env& env)
{
    SQLRETURN result = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hContext);
    if (!SQL_SUCCEEDED(result))
        throw DBFunc::CheckError(result, SQL_NULL_HANDLE, SQL_HANDLE_ENV);

    result = SQLSetEnvAttr(m_hContext, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(result)) {
        SQLFreeHandle(SQL_HANDLE_ENV, m_hContext);
        throw DBFunc::CheckError(result, m_hContext, SQL_HANDLE_ENV);
    }

    result = SQLAllocHandle(SQL_HANDLE_DBC, m_hContext, &m_hConnection);
    if (!SQL_SUCCEEDED(result)) {
        SQLFreeHandle(SQL_HANDLE_ENV, m_hContext);
        throw DBFunc::CheckError(result, m_hContext, SQL_HANDLE_DBC);
    }

    // Connection string


    std::string address = env.GetString("db.address", "51.15.15.33");
    std::string port = env.GetString("db.port", "");
    std::string database = env.GetString("db.name", "chat_app");

    std::stringstream connSstream{};
    
    connSstream << "DRIVER={ODBC Driver 17 for SQL Server};";
    connSstream << std::format("SERVER={},{};", address, port);
    connSstream << std::format("DATABASE={};", database);

    SQLCHAR connStr[] = "DRIVER={ODBC Driver 17 for SQL Server};"
        "SERVER=51.15.15.33,1433;"
        "DATABASE=your_database;"
        "UID=sa;"
        "PWD=Redemption92554477;";

    SQLCHAR outConnStr[1024];
    SQLSMALLINT outConnStrLen;

    result = SQLDriverConnectA(m_hConnection, NULL, connStr, SQL_NTS, outConnStr, sizeof(outConnStr), &outConnStrLen, SQL_DRIVER_NOPROMPT);

    if (!SQL_SUCCEEDED(result)) {
        SQLFreeHandle(SQL_HANDLE_DBC, m_hConnection);
        SQLFreeHandle(SQL_HANDLE_ENV, m_hContext);
        throw DBFunc::CheckError(result, m_hConnection, SQL_HANDLE_DBC);
    }
}

std::string DBFunc::CheckError(SQLRETURN ret, SQLHANDLE handle, SQLSMALLINT handleType)
{
	SQLINTEGER iterator{};
	SQLCHAR state[7];
	SQLINTEGER native;
	SQLCHAR text[256];
	SQLSMALLINT errorMsg;

	std::stringstream sstream;
	do {
		SQLRETURN ret = SQLGetDiagRecA(handleType, handle, iterator, state, &native, (SQLCHAR*) & text, sizeof(text), &errorMsg);

		if (!SQL_SUCCEEDED(ret))
			throw "Failed to diagnose issue...";

		sstream << text;
	} while (ret == SQL_SUCCESS);


	return sstream.str();
}
