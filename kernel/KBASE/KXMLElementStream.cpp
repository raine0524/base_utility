#include "stdafx.h"
#include "KBASE/KXMLElementStream.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KXMLElementStream::KXMLElementStream(KXMLElementStreamEvent& rXMLElementStreamEvent)
:m_pXMLParser(NULL)
,m_rXMLElementStreamEvent(rXMLElementStreamEvent)
,m_bXMLDocumentStarted(false)
,m_bXMLDocumentEnded(false)
{
	m_bXMLDocumentStarted=false;
	m_bXMLDocumentEnded=false;
   
	m_pXMLParser=XML_ParserCreate(NULL);
    
    XML_SetUserData(m_pXMLParser, this);
	XML_SetStartElementHandler(m_pXMLParser,&KXMLElementStream::OnStartElement);
	XML_SetEndElementHandler(m_pXMLParser,&KXMLElementStream::OnEndElement);
    XML_SetCharacterDataHandler(m_pXMLParser,&KXMLElementStream::OnCDATA);
}

//---------------------------------------------------------------------------------------
KXMLElementStream::~KXMLElementStream()
{
	if(m_pXMLParser)
	{
		XML_ParserFree(m_pXMLParser);
		m_pXMLParser=NULL;
	}
	
	Clear();
}

//---------------------------------------------------------------------------------------
// 清空队列
void KXMLElementStream::Clear(void)
{
	if(m_listXMLElement.size()>0)
	{
		KXMLElement* e=m_listXMLElement.front();
		m_listXMLElement.pop_front();
		if(e)
		{
			delete e;
			e=NULL;
		}
	}
	m_listXMLElement.clear();
}

//---------------------------------------------------------------------------------------
//Push XML Element into XML Element List
void KXMLElementStream::Push(const std::string& data)
{
	Push(data.c_str(), (int)data.size());
}

//---------------------------------------------------------------------------------------
//Push XML Element into XML Element List
void KXMLElementStream::Push(const char* data, int datasz)
{
	Clear();

    if(XML_Parse(m_pXMLParser,data, datasz, false)==XML_STATUS_ERROR)
    {
#ifdef WIN32
		TRACE("KXMLElementStream::Push:%s\n",XML_ErrorString(XML_GetErrorCode(m_pXMLParser)));
#else
		printf("KXMLElementStream::Push:%s\n",XML_ErrorString(XML_GetErrorCode(m_pXMLParser)));
#endif
    }
}

//---------------------------------------------------------------------------------------
// OnXMLStartElement
void KXMLElementStream::OnStartElement(void *userdata,const char *name,const char **attribs)
{
	KXMLElementStream* ts = (KXMLElementStream*)userdata;

	if(!ts->m_bXMLDocumentStarted)
	{
		//如果流没有开始则需要生成一个事件
		KXMLElement e(name, attribs);
		ts->m_bXMLDocumentStarted = true;
		ts->m_rXMLElementStreamEvent.OnXMLDocumentStart(&e);	
		return;
	}

	//如果流已经开始则判断列表中是否有元素
	if(ts->m_listXMLElement.empty())
	{
		//如果元素栈为空则新建一个元素
		KXMLElement* e=new KXMLElement(name, attribs);
		ts->m_listXMLElement.push_back(e);
	}
	else
	{
		//如果元素栈不为空就修改上一个元素的属性并将指针保留到最后
		ts->m_listXMLElement.push_back(ts->m_listXMLElement.back()->AddElement(name, attribs));
	}
}

//---------------------------------------------------------------------------------------
// OnXMLEndElement
void KXMLElementStream::OnEndElement(void* userdata,const char* name)
{
	KXMLElementStream* ts = (KXMLElementStream*)userdata;

	switch(ts->m_listXMLElement.size())
	{
		case 1:
			{
				KXMLElement* e=ts->m_listXMLElement.back();
				ts->m_rXMLElementStreamEvent.OnXMLDocumentElement(e);
				ts->m_listXMLElement.pop_back();
				delete e;
				e=NULL;
			}
			break;
		case 0:
			{
				ts->m_rXMLElementStreamEvent.OnXMLDocumentEnd();
				ts->m_bXMLDocumentEnded = true;
			}
			break;
		default:
			{
				//如果是子元素结束则删除,此时父元素已经修改了属性
				ts->m_listXMLElement.pop_back();
			}
	}
}

//---------------------------------------------------------------------------------------
// OnXMLCDATA
void KXMLElementStream::OnCDATA(void* userdata,const char* cdata, int cdatasz)
{
	KXMLElementStream* ts = (KXMLElementStream*)userdata;

    if(!ts->m_listXMLElement.empty())
	{
		ts->m_listXMLElement.back()->AddCDATA(cdata, cdatasz, true);
	}
}

//---------------------------------------------------------------------------------------
// Parse File
bool KXMLElementStream::ParseFile(const char* filename)
{
    size_t len=0;
    char buf[1024];

	FILE* f=fopen(filename,"r");
    if(f == NULL)
    {
        return false;
    }

	for(;;)
    {
        len = fread(buf, 1, 1024, f);
        if(ferror(f))
        {
            fclose(f);
			return false;
        }
		Push(buf,(int)len);
		if(feof(f))
		{
           break;
		}
    }
    fclose(f);
	return true;
}