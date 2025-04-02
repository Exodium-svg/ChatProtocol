#pragma once
#include <sstream>
#include <string>
#include <format>
#include <Env.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>


namespace DBFunc {
	std::string CheckError(SQLRETURN ret, SQLHANDLE handle, SQLSMALLINT type);
}


class DbConn
{
private:
	SQLHENV m_hContext;
	SQLHDBC m_hConnection;

public:
	DbConn(Env& env);

};

