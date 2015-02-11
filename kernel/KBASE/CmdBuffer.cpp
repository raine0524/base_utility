#include "stdafx.h"
#include "KBASE/CmdBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KCmdBuffer::KCmdBuffer(void)
{
}

//---------------------------------------------------------------------------------------
KCmdBuffer::~KCmdBuffer(void)
{
}

//---------------------------------------------------------------------------------------
void KCmdBuffer::SetDomain(const std::string& strDomain)
{
	m_strDomain = strDomain;
}

//---------------------------------------------------------------------------------------
std::string KCmdBuffer::GetDomain(void)
{
	return m_strDomain;
}

//---------------------------------------------------------------------------------------
void KCmdBuffer::SetDomainID(unsigned int nDomainID)
{
	m_nDomainID = nDomainID;
}

//---------------------------------------------------------------------------------------
unsigned int KCmdBuffer::GetDomainID(void)
{
	return m_nDomainID;
}

//---------------------------------------------------------------------------------------
void KCmdBuffer::SetAgentID(unsigned int nAgentID)
{
	m_nAgentID = nAgentID;
}

//---------------------------------------------------------------------------------------
unsigned int KCmdBuffer::GetAgentID(void)
{
	return m_nAgentID;
}

//---------------------------------------------------------------------------------------
void KCmdBuffer::SetNodeID(const std::string& strNodeID)
{
	m_strNodeID = strNodeID;
}

//---------------------------------------------------------------------------------------
std::string KCmdBuffer::GetNodeID(void)
{
	return m_strNodeID;
}

//---------------------------------------------------------------------------------------
void KCmdBuffer::SetC2SID(unsigned int nC2SID)
{
	m_nC2SID = nC2SID;
}

//---------------------------------------------------------------------------------------
unsigned int KCmdBuffer::GetC2SID(void)
{
	return m_nC2SID;
}
