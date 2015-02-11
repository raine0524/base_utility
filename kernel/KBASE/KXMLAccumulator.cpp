#include "stdafx.h"
#include "KBASE/KXMLAccumulator.h"
#include "KBASE/KXMLNode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

KXMLAccumulator::KXMLAccumulator(std::string& s)
:m_result(s)
{
}

void KXMLAccumulator::operator()(KXMLNode* n)
{
	n->Accumulate(*this);
}

void KXMLAccumulator::operator()(const std::pair<const std::string, std::string> p)
{
	m_result += " " + p.first + "='" + XMLEscape(p.second) + "'";
}

//---------------------------------------------------------------------------------------
void XMLUnEscape(const char* src, unsigned int srcLen, std::string& dest, bool append)
{
    unsigned int i, j;
    int len;

    // Setup string size
    if (append)
    {
		len = j = (int)dest.length();
		dest.resize(len + srcLen);
    }
    else
    {
		j = 0;
		len = 0;
		dest.resize(srcLen);
    }

    // Walk the input text, unescaping as we go..
    for (i = 0; i < srcLen; i++)
    {
		// See if this is an KXMLEscape character
		if (src[i] == '&')
		{
			if (strncmp(&src[i+1],"amp;",4)==0)
			{
				dest[j] = '&';
				i += 4;
			}
			else if (strncmp(&src[i+1],"quot;",5)==0)
			{
				dest[j] = '\"';
				i += 5;
			}
			else if (strncmp(&src[i+1],"apos;",5)==0)
			{
				dest[j] = '\'';
				i += 5;
			}
			else if (strncmp(&src[i+1],"lt;",3)==0)
			{
				dest[j] = '<';
				i += 3;
			}
			else if (strncmp(&src[i+1],"gt;",3)==0)
			{
				dest[j] = '>';
				i += 3;
			}
			else
			{
				dest[j] = src[i];
			}
		}
		else
		{
			// Not an KXMLEscape character, so just copy the 
			// exact character
			dest[j] = src[i];
		}
		j++;
		len++;
    }
    // Cleanup
    dest.resize(len);
}

//---------------------------------------------------------------------------------------
std::string XMLEscape(const std::string& src)
{
    int i,j,oldlen,newlen;

    if(src.empty())
	{
		return std::string(src);
	}

    oldlen = newlen = (int)src.length();
    for(i = 0; i < oldlen; i++)
    {
		switch(src[i])
		{
			case '<':
			case '>':
				newlen+=4; break;
			case '&' : 
				newlen+=5; break;
			case '\'': 
			case '\"': 
				newlen+=6; break;
		}
    }

    if(oldlen == newlen) 
		return std::string(src);
	
	std::string result;
    result.reserve(newlen);
	
    for(i = j = 0; i < oldlen; i++)
    {
		switch(src[i])
		{
			case '&':
				result += "&amp;"; break;
			case '\'':
				result += "&apos;"; break;
			case '\"':
				result += "&quot;"; break;
			case '<':
				result += "&lt;"; break;
			case '>':
				result += "&gt;"; break;
			default:
				result += src[i];
		}
    }
    return result;
}