#include "stdafx.h"
#include "KBASE/KUID.h"
#include "KBASE/Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=======================================================================================
// KUID
//=======================================================================================

//---------------------------------------------------------------------------------------
KUID::KUID(const std::string& strUID)
:m_strUID(strUID)
{
	STRING_MAP m_mapUID = StrSplit(m_strUID,"@");
	m_strUserID = m_mapUID[0];
	m_strDomain = m_mapUID[1];

	STRING_MAP m_mapDomain = StrSplit(m_strDomain,"/");
	if(m_mapDomain.size()>=2)
	{
		m_strDomain = m_mapDomain[0];
		m_strResource = m_mapDomain[1];
	}
}

//---------------------------------------------------------------------------------------
KUID::KUID(const std::string& strUserID,const std::string& strDomain,const std::string& strResource)
:m_strUserID(strUserID)
,m_strDomain(strDomain)
,m_strResource(strResource)
{
	if(m_strResource.empty())
	{
		m_strUID = m_strUserID + "@" + m_strDomain;
	}
	else
	{
		m_strUID = m_strUserID + "@" + m_strDomain + "/" + m_strResource;
	}
}

//---------------------------------------------------------------------------------------
KUID::~KUID()
{
}

//---------------------------------------------------------------------------------------
std::string KUID::GetUserID(void)
{
	return m_strUserID;
}

//---------------------------------------------------------------------------------------
std::string KUID::GetDomain(void)
{
	return m_strDomain;
}

//---------------------------------------------------------------------------------------
std::string KUID::GetResource(void)
{
	return m_strResource;
}

//---------------------------------------------------------------------------------------
std::string KUID::GetUID(void)
{
	return m_strUID;
}

//=======================================================================================
// KDomain
//=======================================================================================

//---------------------------------------------------------------------------------------
KDomain::KDomain(const std::string& strLocalDomain)
:m_strLocalDomain(strLocalDomain)
{
	STRING_MAP mapString = StrSplit(m_strLocalDomain,".");
	m_nLocalDomainLevel = mapString.size();
	m_nParentDomainLevel = m_nLocalDomainLevel - 1;
	
	size_t pos = m_strLocalDomain.find('.');
	if(pos == std::string::npos)
	{
		m_strParentDomain = "";
		return;
	}
	size_t nCount = m_strLocalDomain.size() - pos - 1;
	m_strParentDomain =  m_strLocalDomain.substr(pos+1,nCount);
}

//---------------------------------------------------------------------------------------
KDomain::~KDomain()
{
}

//---------------------------------------------------------------------------------------
std::string KDomain::GetParentDomain(void)
{
	return m_strParentDomain;
}

//---------------------------------------------------------------------------------------
int KDomain::GetParentDomainLevel(void)
{
	return m_nParentDomainLevel;
}

//---------------------------------------------------------------------------------------
std::string KDomain::GetLocalDomain(void)
{
	return m_strLocalDomain;
}

//---------------------------------------------------------------------------------------
int KDomain::GetLocalDomainLevel(void)
{
	return m_nLocalDomainLevel;
}

//---------------------------------------------------------------------------------------
void KDomain::CompareDomainLevel(const std::string strDomain,int& nLevel)
{
	if (GetDomainLevel(strDomain) == GetDomainLevel(m_strLocalDomain))
	{
		nLevel = 0; //同级
	}
	else if (GetDomainLevel(strDomain) < GetDomainLevel(m_strLocalDomain))
	{
		nLevel = 1; //上级
	}	
	else if (GetDomainLevel(strDomain) > GetDomainLevel(m_strLocalDomain))
	{
		nLevel = -1; //下级
	}
}

//---------------------------------------------------------------------------------------
STRING_MAP& KDomain::GetFullDomain(void)
{
	std::string strLocalDomain = m_strLocalDomain;
	m_mapFullDomain.insert(STRING_MAP::value_type(0,strLocalDomain));

	int i=1;
	while(true)
	{
		size_t pos = strLocalDomain.find('.');
		if(pos == std::string::npos)
		{
			break;
		}
		strLocalDomain =  strLocalDomain.substr(pos+1,strLocalDomain.size()-pos-1);
		m_mapFullDomain.insert(STRING_MAP::value_type(i,strLocalDomain));
		i++;
	}
	return m_mapFullDomain;
}

//=======================================================================================
std::string GetUserDomain(const std::string& strUserID)
{
	KUID uid(strUserID);
	return uid.GetDomain();
}

std::string GetLowestCommonParentDomain(const std::string& strDomain1,const std::string& strDomain2)
{
	KDomain kDomain1(strDomain1);
	STRING_MAP mapFullDomain1 = kDomain1.GetFullDomain();
	KDomain kDomain2(strDomain2);
	STRING_MAP mapFullDomain2 = kDomain2.GetFullDomain();
	STRING_MAP mapHighFullDomain;
	STRING_MAP mapLowFullDomain;
	int nLevel;
	kDomain1.CompareDomainLevel(strDomain2,nLevel);
	if (nLevel >= 0)
	{
		mapHighFullDomain = mapFullDomain2;
		mapLowFullDomain  = mapFullDomain1;
	}
	else
	{
		mapHighFullDomain = mapFullDomain1;
		mapLowFullDomain  = mapFullDomain2;
	}

	
	int  nCountHighFullDomain = mapHighFullDomain.size();
	for (int i=0; i<nCountHighFullDomain; i++)
	{
		std::string strDomain = mapHighFullDomain[i];
		STRING_MAP::iterator iter = mapLowFullDomain.begin();
		while (iter != mapLowFullDomain.end())
		{
			if (strDomain == iter->second)
			{
				return strDomain;
			}

			iter++;
		}
	}

	return "";
}

std::string GetUserResource(const std::string& strUserID)
{
	KUID uid(strUserID);
	return uid.GetResource();
}


//=======================================================================================
std::string GetUserIDWithDomain(const std::string& strUserID,const std::string& strDomain,const std::string& strResource)
{
	std::string strTempUserID = strUserID;
	int nFirst = strUserID.find("@");
	int nFind = strUserID.find("/edu");
	if(nFirst == std::string::npos && !strDomain.empty())
	{
		if (strResource.empty())
		{
			strTempUserID = strUserID + "@" + strDomain;
		}
		else
		{
			strTempUserID = strUserID + "@" + strDomain + "/" + strResource;
		}
	}
	else if (nFind == std::string::npos)
	{
		if(!strResource.empty())
		{
			strTempUserID = strUserID + "/edu";
		}
	}
	return strTempUserID;
}

//---------------------------------------------------------------------------------------
int GetDomainLevel(const std::string strDomain)
{
	std::string strDomainTemp = strDomain;
	int nPos = strDomain.find('.');
	int n = 1;
	while (nPos > 0)
	{
		n++;
		strDomainTemp = strDomainTemp.substr(nPos + 1, strDomainTemp.length() - nPos - 1);
		nPos = strDomainTemp.find('.');
		if (nPos <= 0)
		{
			break;
		}
	}
	return n;
}

//---------------------------------------------------------------------------------------
void SplitThreeDomain(const std::string strDomainFull,std::string& strDomain1,std::string& strDomain2,std::string& strDomain3)
{
	STRING_MAP mapDomain = StrSplit(strDomainFull,".");
	if (mapDomain.size() >= 3)
	{
		strDomain1 = mapDomain[0];
		strDomain2 = mapDomain[1];
		strDomain3 = mapDomain[2];
	}
	else if (mapDomain.size() == 2)
	{
		strDomain1 = mapDomain[0];
		strDomain2 = mapDomain[1];
		strDomain3 = "";
	}
	else if (mapDomain.size() == 1)
	{
		strDomain1 = mapDomain[0];
		strDomain2 = "";
		strDomain3 = "";
	}
	else
	{
		strDomain1 = "";
		strDomain2 = "";
		strDomain3 = "";
	}
}
