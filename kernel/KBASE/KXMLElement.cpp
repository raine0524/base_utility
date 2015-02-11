#include "stdafx.h"
#include "KBASE/KXMLElement.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
//构造函数
KXMLElement::KXMLElement(const std::string& xns, const char** attribs)
:KXMLNode(xns,XMLNODE_TYPE_ELEMENT)
{
	// Process expat attribs
	if(attribs != NULL)
	{
		int i = 0;
		while(attribs[i] != '\0')
		{
			m_mapAttribs.insert(make_pair(std::string(attribs[i]), std::string(attribs[i+1])));
			i += 2;
		}
	}
}

//---------------------------------------------------------------------------------------
//构造函数
KXMLElement::KXMLElement(KXMLElement& e)
:KXMLNode(e.GetXNS(), XMLNODE_TYPE_ELEMENT)
,m_mapAttribs(e.m_mapAttribs)
{
	XMLELEMENT_CHILD_LIST::iterator it = e.m_listChildren.begin();
	while(it!=e.m_listChildren.end())
	{
		KXMLNode* n=(*it);
	    if (n->GetType() == XMLNODE_TYPE_ELEMENT)
		{
			KXMLElement* t=new KXMLElement(*static_cast<KXMLElement*>(n));
			AppendChild(t);
		}
	    else
		{
			KXMLCData* t=new KXMLCData(*static_cast<KXMLCData*>(n));
			AppendChild(t);
		}
		++it;
	}
}

//---------------------------------------------------------------------------------------
//析构函数
KXMLElement::~KXMLElement()
{
    ClearElement();
}

//---------------------------------------------------------------------------------------
//清空Element
void KXMLElement::ClearElement(void)
{
	m_mapAttribs.clear();
	while(m_listChildren.size())
	{
		KXMLNode* n=m_listChildren.front();
		m_listChildren.pop_front();
		delete n;
		n=NULL;
	}
}

//---------------------------------------------------------------------------------------
KXMLElement& KXMLElement::operator=(KXMLElement& e)
{
	// 如果是自己就返回自己的指针
	if (this == &e)
		return *this;

    // 拷贝以下成员
	KXMLNode::operator=(e);

	// 释放以前的子项
	while(!m_listChildren.size())
	{
		KXMLNode* t=m_listChildren.front();
		m_listChildren.pop_front();
		delete t;
		t=NULL;
	}

	// 释放以前的属性
    if(!m_mapAttribs.empty())
	{
		m_mapAttribs.clear();
	}
	
	//从t中拷贝子项
	XMLELEMENT_CHILD_LIST::iterator it = e.m_listChildren.begin();
	while(it!=e.m_listChildren.end())
	{
		KXMLNode* n=(*it);
		if (n->GetType() == XMLNODE_TYPE_ELEMENT)
		{
			KXMLElement* t=new KXMLElement(*static_cast<KXMLElement*>(n));
			AppendChild(t);
		}
		else
		{
			KXMLCData* t=new KXMLCData(*static_cast<KXMLCData*>(n));
			AppendChild(t);
		}
		++it;
	}

	//从t中拷贝属性
	if(!e.m_mapAttribs.empty())
	{
		m_mapAttribs = e.m_mapAttribs;
	}

	return *this;
}

//---------------------------------------------------------------------------------------
// 添加新的子元素
KXMLElement* KXMLElement::AddElement(const std::string& xns, const char** attribs)
{
    KXMLElement* e = new KXMLElement(xns, attribs);
    m_listChildren.push_back(e);
    return e;
}

//---------------------------------------------------------------------------------------
// 添加新的子元素
KXMLElement* KXMLElement::AddElement(const std::string& xns, const std::string& value, bool escaped)
{
    KXMLElement* e = AddElement(xns);
    e->AddCDATA(value.c_str(), (int)value.size(), escaped);
    return e;
}

//---------------------------------------------------------------------------------------
// 添加内容数据
KXMLCData* KXMLElement::AddCDATA(const char* data, int datasz, bool escaped)
{
    KXMLCData* e;
    if(!m_listChildren.empty() && m_listChildren.back()->GetType() == XMLNODE_TYPE_CDATA)
    {
		e = static_cast<KXMLCData*>(m_listChildren.back());
		e->AppendText(data, datasz, escaped);
    }
    else
    {
		e = new KXMLCData(data, datasz, escaped);
		m_listChildren.push_back(e);
    }
    return e;
}

//---------------------------------------------------------------------------------------
// 是否有属性
bool KXMLElement::HasAttrib(const std::string& xns)
{
    return m_mapAttribs.count(xns) > 0;
}

//---------------------------------------------------------------------------------------
// 删除属性
void KXMLElement::DelAttrib(const std::string& xns)
{
    m_mapAttribs.erase(xns);
}

//---------------------------------------------------------------------------------------
// 属性比较
bool KXMLElement::CmpAttrib(const std::string& xns, const std::string& value)
{
    XMLELEMENT_ATTRIB_MAP::iterator it = m_mapAttribs.find(xns);
    if (it != m_mapAttribs.end())
	{
		return it->second == value;
	}
    else
	{
		return false;
	}
}

//---------------------------------------------------------------------------------------
// 写属性(std::string)
void KXMLElement::PutAttrib(const std::string& xns, const std::string& value)
{
    m_mapAttribs[xns] = value;
}

//---------------------------------------------------------------------------------------
// 写属性(unsigned long)
void KXMLElement::PutAttribUL(const std::string& xns,unsigned long value)
{
	PutAttrib(xns,ULONG2STR(value));
}

//---------------------------------------------------------------------------------------
// 写属性(unsigned int)
void KXMLElement::PutAttribUN(const std::string& xns,unsigned int value)
{
	PutAttrib(xns,UINT2STR(value));
}

//---------------------------------------------------------------------------------------
// 写属性(unsigned short)
void KXMLElement::PutAttribUS(const std::string& xns,unsigned short value)
{
	PutAttrib(xns,USHORT2STR(value));
}

//---------------------------------------------------------------------------------------
// 写属性(unsigned char)
void KXMLElement::PutAttribUC(const std::string& xns,unsigned char value)
{
	PutAttrib(xns,BYTE2STR(value));
}

//---------------------------------------------------------------------------------------
// 写属性(bool)
void KXMLElement::PutAttribBL(const std::string& xns,bool value)
{
	PutAttrib(xns,BOOL2STR(value));
}

//---------------------------------------------------------------------------------------
// 读属性(std::string)
std::string KXMLElement::GetAttrib(const std::string& xns)
{
    XMLELEMENT_ATTRIB_MAP::iterator it = m_mapAttribs.find(xns);
    if(it!= m_mapAttribs.end())
	{
		return it->second;
	}
    else
	{
		return "";
	}
}

//---------------------------------------------------------------------------------------
// 读属性(unsigned long)
unsigned long KXMLElement::GetAttribUL(const std::string& xns)
{
	std::string InBuffer = GetAttrib(xns);
	return STR2ULONG(InBuffer.c_str());
}

//---------------------------------------------------------------------------------------
// 读属性(unsigned int)
unsigned int KXMLElement::GetAttribUN(const std::string& xns)
{
	std::string InBuffer = GetAttrib(xns);
	return STR2UINT(InBuffer.c_str());
}

//---------------------------------------------------------------------------------------
// 读属性(unsigned short)
unsigned short KXMLElement::GetAttribUS(const std::string& xns)
{
	std::string InBuffer = GetAttrib(xns);
	return STR2USHORT(InBuffer.c_str());
}

//---------------------------------------------------------------------------------------
// 读属性(unsigned char)
unsigned char KXMLElement::GetAttribUC(const std::string& xns)
{
	std::string InBuffer = GetAttrib(xns);
	return STR2BYTE(InBuffer.c_str());
}

//---------------------------------------------------------------------------------------
// 读属性(bool)
bool KXMLElement::GetAttribBL(const std::string& xns)
{
	std::string InBuffer = GetAttrib(xns);
	return STR2BOOL(InBuffer.c_str());
}

//---------------------------------------------------------------------------------------
// 获得属性MAP
XMLELEMENT_ATTRIB_MAP KXMLElement::GetAttribMap()
{
	return m_mapAttribs;
}

//---------------------------------------------------------------------------------------
// 获得项列表
XMLELEMENT_CHILD_LIST KXMLElement::GetChildrenList()
{
	return m_listChildren;
}

//---------------------------------------------------------------------------------------
// 获取XML语句
std::string KXMLElement::XMLString()
{
	std::string result;
    KXMLAccumulator acc(result);
    Accumulate(acc);
    return result;
}

//---------------------------------------------------------------------------------------
// Custom version of KXMLElement::XMLString. Permits non-recursive
// serialization of the KXMLElement to a properly escaped XML string
// representation. 
// @param recursive Determines if the KXMLElement is recursively serialization. Default is false.
// @param closetag Determines if the KXMLElement is closed after serialization. Default is false. 
std::string KXMLElement::XMLStringEx(bool recursive, bool closetag)
{
    std::string result;

    KXMLAccumulator acc(result);
    
    acc << "<" << GetXNS();
    for_each(m_mapAttribs.begin(), m_mapAttribs.end(), acc);

    if (recursive && !m_listChildren.empty())
    {
	acc << ">";
	for_each(m_listChildren.begin(), m_listChildren.end(), acc);
	if (closetag)
	    acc << "</" << GetXNS() << ">";
    }
    else
    {
	if (closetag)
	    acc << "/>";
	else
	    acc << ">";
    }
    return result;
}

//---------------------------------------------------------------------------------------
// Accumulate a properly escaped XML string representation of this object
void KXMLElement::Accumulate(KXMLAccumulator& acc)
{
    acc << "<" << GetXNS();
    for_each(m_mapAttribs.begin(), m_mapAttribs.end(), acc);
    if (!m_listChildren.empty())
    {
		acc << ">";
		for_each(m_listChildren.begin(), m_listChildren.end(), acc);
		acc << "</" << GetXNS() << ">";
    }
    else
    {
		acc << "/>";
    }
}

//---------------------------------------------------------------------------------------
// 获得CData
std::string KXMLElement::GetCDATA()
{
	XMLELEMENT_CHILD_LIST::iterator it=m_listChildren.begin();
	while(it!=m_listChildren.end())
    {
		if((*it)->GetType() == XMLNODE_TYPE_CDATA)
		{
			break;
		}
		++it;
    }
    if(it!=m_listChildren.end())
	{
		return static_cast<KXMLCData*>(*it)->GetText();
	}
    else
	{
		return std::string("");
	}
}

//---------------------------------------------------------------------------------------
// 追加子项
void KXMLElement::AppendChild(KXMLNode* n)
{
	m_listChildren.push_back(n);
}

//---------------------------------------------------------------------------------------
// 查找Element
KXMLElement* KXMLElement::FindElement(const std::string& xns)
{
	XMLELEMENT_CHILD_LIST::iterator it=m_listChildren.begin();
	while(it!=m_listChildren.end())
    {
		if((*it)->GetXNS() == xns && (*it)->GetType() == XMLNODE_TYPE_ELEMENT)
		{
			break;
		}
		++it;
    }

	if(it!=m_listChildren.end())
	{
		return static_cast<KXMLElement*>(*it);
	}
    else
	{
		return NULL;
	}
}

//---------------------------------------------------------------------------------------
// 删除元素
void KXMLElement::EraseElement(const std::string& xns)
{
    for(XMLELEMENT_CHILD_LIST::iterator it = m_listChildren.begin(); it != m_listChildren.end(); ++it)
    {
		if((*it)->GetXNS() == xns && (*it)->GetType() == XMLNODE_TYPE_ELEMENT)
		{
			m_listChildren.erase(it);
		}
    }
}

//---------------------------------------------------------------------------------------
// 获取子项数据
std::string KXMLElement::GetChildCDATA(const std::string& xns)
{
    KXMLElement* child = FindElement(xns);
    return (child != NULL) ? child->GetCDATA() : std::string();
}

//---------------------------------------------------------------------------------------
// Locate the first child node which has the specified xns and extract the KXMLCData from it
// ,converting the value into an integer
// @param xns Name of the child KXMLElement to find
// @param default Default value of the integer (in case conversion fails)
// @returns Integer value of the KXMLCData value within the child KXMLElement, or the default
// value if no child was found or integer conversion failed
int KXMLElement::GetChildCDATA(const std::string& xns, int defaultvalue)
{
    KXMLElement* child = FindElement(xns);
    if (child == NULL)
		return defaultvalue;

	std::string svalue = child->GetCDATA();
    if (svalue.empty())
		return defaultvalue;

    char* endptr = NULL;
    int result = strtol(svalue.c_str(), &endptr, 10);
    // If endptr is the same as the original string, no valid digits were found
    if (endptr == svalue.c_str())
		return defaultvalue;

    return result;
}
