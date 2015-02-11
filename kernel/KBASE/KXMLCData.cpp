#include "stdafx.h"
#include "KBASE/KXMLCData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KXMLCData::KXMLCData(const char* text, unsigned int textsz, bool escaped)
:KXMLNode("#CDATA", XMLNODE_TYPE_CDATA)
{
	if (escaped)
	{
		XMLUnEscape(text, textsz, m_text);
	}
	else
	{
		m_text.assign(text, textsz);
	}
}

//---------------------------------------------------------------------------------------
void KXMLCData::SetText(const char* text, unsigned int textsz, bool escaped)
{ 
	if(escaped)
	{
		XMLUnEscape(text, textsz, m_text); 
	}
	else
	{
		m_text.assign(text, textsz);
	}
}

//---------------------------------------------------------------------------------------
void KXMLCData::AppendText(const char* text, unsigned int textsz, bool escaped)
{ 
	if(escaped)
	{
		XMLUnEscape(text, textsz, m_text, true); 
	}
	else
	{
		m_text.append(text, textsz);
	}
}

//---------------------------------------------------------------------------------------
const std::string& KXMLCData::GetText()
{
	return m_text;
}

//---------------------------------------------------------------------------------------
std::string KXMLCData::XMLString()
{
	return XMLEscape(m_text);
}

//---------------------------------------------------------------------------------------
void KXMLCData::Accumulate(KXMLAccumulator& acc)
{
	acc << XMLEscape(m_text);
}
