#include "stdafx.h"
#include "KBASE/KXMLPacket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
// 构造函数(发送包时构造)
KXMLPacket::KXMLPacket(const std::string& xns,int cmd,const std::string& userid,const std::string& fromid,const std::string& toid,unsigned long rsid,unsigned long csid)
:KXMLElement(xns)
{
	PutAttribUN("cmd",cmd);
	if(!userid.empty())
	{
		PutAttrib("userid",userid);
	}
	if(!fromid.empty())
	{
		PutAttrib("fromid",fromid);
	}
	if(!toid.empty())
	{
		PutAttrib("toid", toid);
	}
	PutAttribUL("rsid",rsid);
	PutAttribUL("csid",csid);
}

//---------------------------------------------------------------------------------------
void KXMLPacket::SetCMD(int cmd)
{
	DelAttrib("cmd");
	PutAttribUN("cmd",cmd);
}

//---------------------------------------------------------------------------------------
void KXMLPacket::SetUserID(const std::string& userid)
{
	DelAttrib("userid");
	PutAttrib("userid",userid);
}

//---------------------------------------------------------------------------------------
void KXMLPacket::SetFromID(const std::string& fromid)
{
	DelAttrib("fromid");
	PutAttrib("fromid",fromid);
}

//---------------------------------------------------------------------------------------
void KXMLPacket::SetToID(const std::string& toid)
{
	DelAttrib("toid");
	PutAttrib("toid",toid);
}

//---------------------------------------------------------------------------------------
void KXMLPacket::SetRSID(unsigned long rsid)
{
	DelAttrib("rsid");
	PutAttribUL("rsid",rsid);
}

//---------------------------------------------------------------------------------------
void KXMLPacket::SetCSID(unsigned long csid)
{
	DelAttrib("csid");
	PutAttribUL("csid",csid);
}

//---------------------------------------------------------------------------------------
int KXMLPacket::GetCMD(void)
{
	return STR2INT(GetAttrib("cmd"));
}

//---------------------------------------------------------------------------------------
std::string KXMLPacket::GetUserID(void)
{
	return GetAttrib("userid");
}

//---------------------------------------------------------------------------------------
std::string KXMLPacket::GetFromID(void)
{
	return GetAttrib("fromid");
}

//---------------------------------------------------------------------------------------
std::string KXMLPacket::GetToID(void)
{
	return GetAttrib("toid");
}

//---------------------------------------------------------------------------------------
unsigned long KXMLPacket::GetRSID(void)
{
	return GetAttribUL("rsid");
}

//---------------------------------------------------------------------------------------
unsigned long KXMLPacket::GetCSID(void)
{
	return GetAttribUL("csid");
}
