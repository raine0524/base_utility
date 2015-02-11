/* KBufferPool.cpp */

#include "stdafx.h"
#include "KBASE/BufferPool.h"
#include "KBASE/AutoLock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KBufferPool::KBufferPool(void)
{
}

//---------------------------------------------------------------------------------------
KBufferPool::~KBufferPool(void)
{
	Clear();
}

//---------------------------------------------------------------------------------------
void KBufferPool::Push(char* pData,int nLen)
{
	KBuffer* pBuffer=new KBuffer();
	if(pBuffer->SetBuffer(pData,nLen))
	{
		Push(pBuffer);
	}
	else
	{
		delete pBuffer;
		pBuffer=NULL;
	}
}

//---------------------------------------------------------------------------------------
void KBufferPool::Push(KBuffer* pBuffer)
{
	KAutoLock l(m_CritSec);
	m_listBuffer.push(pBuffer);
}

//---------------------------------------------------------------------------------------
KBuffer* KBufferPool::Pop(void)
{
	KAutoLock l(m_CritSec);

	if (m_listBuffer.size())
	{
		KBuffer* pBuffer=(KBuffer*)m_listBuffer.front();
		m_listBuffer.pop();
		return pBuffer;
	}
	return NULL;
}

//---------------------------------------------------------------------------------------
int KBufferPool::GetSize(void)
{
	KAutoLock l(m_CritSec);
	return (int)m_listBuffer.size();
}

//---------------------------------------------------------------------------------------
void KBufferPool::Clear(void)
{
	KAutoLock l(m_CritSec);

	while (m_listBuffer.size())
	{
		KBuffer* pBuffer=(KBuffer*)m_listBuffer.front();
		m_listBuffer.pop();
		delete pBuffer;
		pBuffer=NULL;
	}
}

