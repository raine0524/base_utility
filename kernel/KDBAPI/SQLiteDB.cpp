#include "stdafx.h"
#include "KDBAPI/SQLiteDB.h"
#include <cstdlib>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef HAS_SQLITE

#define MAX_DBMYSQL_SQLLENGTH	8192
#define SQLITE_BUSY_TIMEOUT		60000

//=======================================================================================
// SQLiteResultSet类实现
//=======================================================================================

//---------------------------------------------------------------------------------------
SQLiteResultSet::SQLiteResultSet()
:m_pResultSet(NULL)
,m_bEnd(true)
{
}

//---------------------------------------------------------------------------------------
SQLiteResultSet::SQLiteResultSet(const SQLiteResultSet& rs)
{
	m_pDB = rs.m_pDB;
	m_pResultSet = rs.m_pResultSet;
	const_cast<SQLiteResultSet&>(rs).m_pResultSet = 0;
	m_bEnd = rs.m_bEnd;
}

//---------------------------------------------------------------------------------------
SQLiteResultSet::SQLiteResultSet(sqlite3* pDB,sqlite3_stmt* pResultSet,bool bEnd)
{
	m_pDB = pDB;
	m_pResultSet = pResultSet;
	m_bEnd = bEnd;
}

//---------------------------------------------------------------------------------------
SQLiteResultSet::~SQLiteResultSet()
{
	Close();
}

//---------------------------------------------------------------------------------------
void SQLiteResultSet::Close()
{
	if(m_pResultSet)
	{
		int nRet = sqlite3_finalize(m_pResultSet);
		m_pResultSet = NULL;
		if(nRet != SQLITE_OK)
		{
			throw DBException(sqlite3_errmsg(m_pDB));
		}
	}
}

//---------------------------------------------------------------------------------------
bool SQLiteResultSet::End()
{
	return m_bEnd;
}

//---------------------------------------------------------------------------------------
void SQLiteResultSet::MoveNext()
{
	int nRet = sqlite3_step(m_pResultSet);
	if(nRet == SQLITE_DONE)
	{
		m_bEnd = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		return;
	}
	else
	{
		Close();
	}
}

//---------------------------------------------------------------------------------------
int SQLiteResultSet::GetFieldCols(void)
{
	return sqlite3_column_count(m_pResultSet);
}

//---------------------------------------------------------------------------------------
int SQLiteResultSet::GetFieldCol(const std::string& strFieldName)
{
	int nCols = sqlite3_column_count(m_pResultSet);
	for(int nCol = 0; nCol < nCols; nCol++)
	{
		std::string strTemp = sqlite3_column_name(m_pResultSet, nCol);
		if(strFieldName == strTemp)
		{
			return nCol;
		}
	}
	return 0;
}

//---------------------------------------------------------------------------------------
std::string SQLiteResultSet::GetStrField(int nCol)
{
	const unsigned char* pValue = sqlite3_column_text(m_pResultSet, nCol);
	if(pValue==NULL)
	{
		return "";
	}
	return (const char*)pValue;
}

//---------------------------------------------------------------------------------------
std::string SQLiteResultSet::GetStrField(const std::string& strFieldName)
{
	int nCol = GetFieldCol(strFieldName);
	const unsigned char* pValue = sqlite3_column_text(m_pResultSet, nCol);
	if(pValue==NULL)
	{
		return "";
	}
	return (const char*)pValue;
}

//---------------------------------------------------------------------------------------
int SQLiteResultSet::GetIntField(int nCol)
{
	return sqlite3_column_int(m_pResultSet, nCol);
}

//---------------------------------------------------------------------------------------
int SQLiteResultSet::GetIntField(const std::string& strFieldName)
{
	int nCol = GetFieldCol(strFieldName);
	return sqlite3_column_int(m_pResultSet, nCol);
}

//---------------------------------------------------------------------------------------
unsigned long SQLiteResultSet::GetInt64Field(int nCol)
{
	return sqlite3_column_int64(m_pResultSet, nCol);
}

//---------------------------------------------------------------------------------------
unsigned long SQLiteResultSet::GetInt64Field(const std::string& strFieldName)
{
	int nCol = GetFieldCol(strFieldName);
	return sqlite3_column_int64(m_pResultSet, nCol);
}

//---------------------------------------------------------------------------------------
double SQLiteResultSet::GetFloatField(int nCol)
{
	return sqlite3_column_double(m_pResultSet,nCol);
}

//---------------------------------------------------------------------------------------
double SQLiteResultSet::GetFloatField(const std::string& strFieldName)
{
	int nCol = GetFieldCol(strFieldName);
	return sqlite3_column_double(m_pResultSet, nCol);
}

//---------------------------------------------------------------------------------------
const unsigned char* SQLiteResultSet::GetBlobField(int nCol, int& nLen)
{
	nLen = sqlite3_column_bytes(m_pResultSet, nCol);
	return (const unsigned char*)sqlite3_column_blob(m_pResultSet, nCol);
}

//---------------------------------------------------------------------------------------
const unsigned char* SQLiteResultSet::GetBlobField(const std::string& strFieldName, int& nLen)
{
	int nCol = GetFieldCol(strFieldName);
	return GetBlobField(nCol, nLen);
}

//=======================================================================================
// SQLiteDB类实现
//=======================================================================================

//---------------------------------------------------------------------------------------
SQLiteDB::SQLiteDB()
:m_pDB(NULL)
{
}

//---------------------------------------------------------------------------------------
SQLiteDB::~SQLiteDB()
{
}

//---------------------------------------------------------------------------------------
bool SQLiteDB::Connect(const std::string& strDBName)
{
	if (CheckDB())
	{
		return true;
	}
	int nRet = sqlite3_open(strDBName.c_str(), &m_pDB);
	if(nRet != SQLITE_OK)
	{
		printf("%s.\n",sqlite3_errmsg(m_pDB));
		return false;
	}
	sqlite3_busy_timeout(m_pDB, SQLITE_BUSY_TIMEOUT);
	return true;
}

//---------------------------------------------------------------------------------------
void SQLiteDB::Disconnect()
{
	sqlite3_close(m_pDB);
	m_pDB = NULL;
}

//---------------------------------------------------------------------------------------
bool SQLiteDB::CheckDB(void)
{
	if(m_pDB)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//---------------------------------------------------------------------------------------
// 创建SQL语句
std::string SQLiteDB::CreateSQL(const char* cszSQL,...)
{
	char szSQL[MAX_DBMYSQL_SQLLENGTH];
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
unsigned long SQLiteDB::SQLExecute(const std::string& strSQL)
{
	if(CheckDB()==false)
	{
		throw DBException("database not connected.");
	}

	//Execute
	int nRet = sqlite3_exec(m_pDB, strSQL.c_str(), NULL, NULL, NULL);
	if(nRet!=SQLITE_OK)
	{
		sqlite3_exec(m_pDB,"ROLLBACK;", NULL, NULL, NULL);
		throw DBException(sqlite3_errmsg(m_pDB));
	}
	return sqlite3_changes(m_pDB);
}

//---------------------------------------------------------------------------------------
SQLiteResultSet SQLiteDB::SQLQuery(const std::string& strSQL)
{
	sqlite3_stmt* pResultSet;
	int nRet = sqlite3_prepare(m_pDB, strSQL.c_str(), -1, &pResultSet, NULL);
	if(nRet != SQLITE_OK)
	{
		throw DBException(sqlite3_errmsg(m_pDB));
	}
	if(!pResultSet)
	{
		throw DBException("invalid result set.");
	}
	nRet = sqlite3_step(pResultSet);
	if(nRet == SQLITE_DONE)
	{
		return SQLiteResultSet(m_pDB, pResultSet, true);	//结果集为空
	}
	else if(nRet == SQLITE_ROW)
	{
		return SQLiteResultSet(m_pDB, pResultSet, false);	//结果集不为空
	}
	else
	{
		sqlite3_finalize(pResultSet);
		pResultSet=NULL;
		throw DBException(sqlite3_errmsg(m_pDB));
	}
}

//---------------------------------------------------------------------------------------
unsigned long SQLiteDB::GetLastInsertID()
{
	return sqlite3_last_insert_rowid(m_pDB);
}

//---------------------------------------------------------------------------------------
bool SQLiteDB::HasTable(const std::string& strTableName)
{
	char szSQL[1024];
	sprintf(szSQL,"select count(*) from sqlite_master where type='table' and name='%s'",strTableName.c_str());

	SQLiteResultSet res=SQLQuery(szSQL);
	if(res.End() || res.GetFieldCols() < 1)
	{
		return false;
	}
	int nRet = res.GetIntField(0);
	return (nRet > 0);
}

//---------------------------------------------------------------------------------------
const char* SQLiteDB::GetVersion()
{
	return SQLITE_VERSION;
}

#endif //HAS_SQLITE