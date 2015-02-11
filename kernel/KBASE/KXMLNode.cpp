#include "stdafx.h"
#include "KBASE/KXMLNode.h"
#include "KBASE/KXMLAccumulator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KXMLNode::KXMLNode(const std::string& xns, XMLNODE_TYPE type)
:m_xns(xns)
,m_type(type)
{
}

//---------------------------------------------------------------------------------------
KXMLNode::~KXMLNode()
{
}

//---------------------------------------------------------------------------------------
void KXMLNode::SetXNS(const std::string& xns)
{
	m_xns=xns;
}

//---------------------------------------------------------------------------------------
std::string KXMLNode::GetXNS()
{
	return m_xns;
}

//---------------------------------------------------------------------------------------
XMLNODE_TYPE KXMLNode::GetType()
{
	return m_type;
}

