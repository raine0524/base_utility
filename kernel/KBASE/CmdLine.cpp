#include "stdafx.h"
#include "KBASE/CmdLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
#ifdef WIN32
KCmdLine::KCmdLine(int argc, TCHAR* argv[])
#else
KCmdLine::KCmdLine(int argc, char* argv[])
#endif
{
	for(int i=0;i<argc;i++)
	{
		std::string strArgValue;
		#ifdef WIN32
		strArgValue = U2A(argv[i]);
		#else
		strArgValue = argv[i];
		#endif
		m_mapCmdLine.insert(STRING_MAP::value_type(i,strArgValue));
	}
}

//---------------------------------------------------------------------------------------
KCmdLine::~KCmdLine(void)
{
	m_mapCmdLine.clear();
}

//---------------------------------------------------------------------------------------
int KCmdLine::GetParamCount(void)
{
	return m_mapCmdLine.size();
}

//---------------------------------------------------------------------------------------
std::string KCmdLine::GetParamValue(int nIndex)
{
	STRING_MAP::iterator it = m_mapCmdLine.find(nIndex);
	if(it!=m_mapCmdLine.end())
	{
		return it->second;
	}
	else
	{
		return "";
	}
}

//---------------------------------------------------------------------------------------
std::string KCmdLine::GetOptionParamValue(const std::string& strOptionParamName)
{
	int nIndex=-1;
	for(STRING_MAP::iterator it=m_mapCmdLine.begin();it!=m_mapCmdLine.end();it++)
	{
		std::string strParamName = it->second;
		if(strOptionParamName == strParamName)
		{
			nIndex = it->first;
			break;
		}
	}
	if(nIndex == -1)
	{
		return "";
	}
	else
	{
		return GetParamValue(++nIndex);
	}
}

//---------------------------------------------------------------------------------------
bool KCmdLine::IsExistParam(const std::string& strOptionParamName)
{
	for(STRING_MAP::iterator it=m_mapCmdLine.begin();it!=m_mapCmdLine.end();it++)
	{
		std::string strParamName = it->second;
		if(strOptionParamName == strParamName)
		{
			return true;
		}
	}
	return false;
}
