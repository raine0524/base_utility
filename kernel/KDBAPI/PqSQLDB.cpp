#include "stdafx.h"
#include "KDBAPI/PqSQLDB.h"
#include <stdarg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef HAS_PQSQL

#define MAX_DBPQSQL_SQLLENGTH	65535

//---------------------------------------------------------------------------------------
// 构造函数
PqSQLDB::PqSQLDB(void)
:m_pDB(NULL)
{
}

//---------------------------------------------------------------------------------------
// 析构函数
PqSQLDB::~PqSQLDB(void)
{
}

//---------------------------------------------------------------------------------------
// 连接数据库
bool PqSQLDB::Connect(const std::string& strDBHost,unsigned short usDBPort,const std::string& strDBName,const std::string& strDBUser,const std::string& strDBPass)
{
	if (m_pDB)
	{
		return true;
	}
	char szConnectInfo[1024]={0};
	sprintf(szConnectInfo,"hostaddr=%s port=%d dbname=%s user=%s password=%s",strDBHost.c_str(),usDBPort,strDBName.c_str(),strDBUser.c_str(),strDBPass.c_str());
	m_strConnectInfo=szConnectInfo;

	m_pDB = PQconnectdb(szConnectInfo);

	if(PQstatus(m_pDB) != CONNECTION_OK)
	{
		ShowLastError();
		Disconnect();
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------------------
// 断开数据库连接
void PqSQLDB::Disconnect(void)
{
	PQfinish(m_pDB);
	m_pDB=NULL;
}

//---------------------------------------------------------------------------------------
// 检查数据库
bool PqSQLDB::CheckDB(void)
{
	if(PQstatus(m_pDB) == CONNECTION_OK)
	{
		return true;
	}

	Disconnect();
	m_pDB = PQconnectdb(m_strConnectInfo.c_str());
	if(PQstatus(m_pDB) == CONNECTION_OK)
	{
		return true;
	}

	ShowLastError();
	return false;
}

//---------------------------------------------------------------------------------------
// 创建SQL语句
std::string PqSQLDB::CreateSQL(const char* cszSQL,...)
{
	char szSQL[MAX_DBPQSQL_SQLLENGTH]={0};
	va_list args;
	va_start(args,cszSQL);
#ifdef WIN32
	_vsnprintf(szSQL,sizeof(szSQL),cszSQL,args);
#else
	vsnprintf(szSQL,sizeof(szSQL),cszSQL,args);
#endif
	va_end(args);
	return szSQL;
}

//---------------------------------------------------------------------------------------
// 执行SQL语句
bool PqSQLDB::SQLExecute(const std::string& strSQL)
{
	if(CheckDB()==false)
	{
		return false;
	}

	//Execute
	PGresult* res = PQexec(m_pDB,strSQL.c_str());
	if(!res || PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		ShowLastError();
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;
}

//---------------------------------------------------------------------------------------
PqSQLResultSet PqSQLDB::SQLQuery(const std::string& strSQL)
{
	PqSQLResultSet rres;
	if(CheckDB()==false)
	{
		return rres;
	}

	//Query
	PGresult* res = PQexec(m_pDB,strSQL.c_str());
	if(!res || PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		ShowLastError();
		PQclear(res);
		return rres;
	}

	unsigned int field_num = PQnfields(res);
	unsigned long row_num   = PQntuples(res);

	for(int i=0;i<row_num;i++)
	{
		PqSQLRow row;
		for(unsigned int j=0;j<field_num;j++)
		{
			const char* field_name = PQfname(res,j);
			const char* row_value = PQgetvalue(res,i,j);
			row[field_name]=row_value?row_value:"";	//字段有值取值,无值取空
		}
		rres.insert(PqSQLResultSet::value_type(i,row));
	}
	PQclear(res);
	return rres;
}

//---------------------------------------------------------------------------------------
const char* PqSQLDB::Escape(const std::string& strValue)
{
	char szToValue[MAX_DBPQSQL_SQLLENGTH]={0};
	int nErrNo=0;
	size_t ret=PQescapeStringConn (m_pDB,szToValue,strValue.c_str(),strValue.length(),&nErrNo);
	return szToValue;
}

//---------------------------------------------------------------------------------------
unsigned long PqSQLDB::GetServerVersion(void)
{
	if(!CheckDB())
	{
		return 0;
	}
	return PQserverVersion(m_pDB);
}

//---------------------------------------------------------------------------------------
unsigned long PqSQLDB::GetProtocolVersion(void)
{
	if(!CheckDB())
	{
		return 0;
	}
	return PQprotocolVersion(m_pDB);
}

//---------------------------------------------------------------------------------------
void PqSQLDB::ShowLastError(void)
{
	printf("[DBERR] %s.\n",PQerrorMessage(m_pDB));
}

#endif //HAS_PQSQL