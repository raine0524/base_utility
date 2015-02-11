// Zip.cpp: implementation of the CZip class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KBASE/Zip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef WIN32

#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8 
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif

#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

//---------------------------------------------------------------------------------------
KZip::KZip()
{
	m_pbBuffer				=	NULL;
	m_dwBufferSize			=	0;
	m_dwBufferLength		=	0;
	m_crc32					=	0;
}

//---------------------------------------------------------------------------------------
KZip::~KZip()
{
	if (m_list.GetCount())
	{
		POSITION pos = m_list.GetHeadPosition();
		while (pos)
			delete[] m_list.GetNext(pos);
		
	}
}

//---------------------------------------------------------------------------------------
bool KZip::Open(DWORD dwLen)
{
	Close();
	m_pbBuffer				=	new BYTE[dwLen];

	m_dwBufferSize			=	dwLen;
	m_crc32					=	0;
	m_ZIPStream.avail_in	=	(uInt)0;
    m_ZIPStream.avail_out	=	(uInt)m_dwBufferSize;
    m_ZIPStream.next_out	=	m_pbBuffer;
    m_ZIPStream.total_in	=	0;
    m_ZIPStream.total_out	=	0;
	m_dwBufferLength		=	0;

	m_ZIPStream.zalloc		=	(alloc_func)myalloc;
    m_ZIPStream.zfree		=	(free_func)myfree;
    m_ZIPStream.opaque		=	&m_list;

    int err = deflateInit2(&m_ZIPStream, Z_DEFAULT_COMPRESSION,Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
	return m_pbBuffer?true:false;
}

//---------------------------------------------------------------------------------------
void KZip::Close()
{
	if	(m_pbBuffer)
	{
		delete m_pbBuffer;
		m_pbBuffer	=	NULL;
	}
	m_dwBufferSize		=	0;
	m_dwBufferLength	=	0;
}

//---------------------------------------------------------------------------------------
bool KZip::Write(PBYTE pData, DWORD dwLength)
{
	m_ZIPStream.next_in		=	pData;
    m_ZIPStream.avail_in	=	dwLength;

	m_crc32					=	crc32(m_crc32, pData, dwLength);

	int err = Z_OK;

    while ((err == Z_OK) && (m_ZIPStream.avail_in > 0))
    {

        if (m_ZIPStream.avail_out == 0)//»º³åÇøÂú
        {
			return false;
        }
        else
        {
            uLong uTotalOutBefore = m_ZIPStream.total_out;

            err = deflate(&m_ZIPStream,  Z_NO_FLUSH);
            m_dwBufferLength += (uInt)(m_ZIPStream.total_out - uTotalOutBefore) ;
        }
    }
	return true;
}

//---------------------------------------------------------------------------------------
void* KZip::myalloc(void *opaque, UINT items, UINT size)
{
	void* p = new TCHAR[size * items];
	if (opaque)
	{
		CPtrList* list  = (CPtrList*) opaque;
		list->AddTail(p);
	}
	return p;
}

//---------------------------------------------------------------------------------------
void KZip::myfree(void *opaque, void *address)
{
	if (opaque)
	{
		CPtrList* list  = (CPtrList*) opaque;
		POSITION pos = list->Find(address);
		if (pos)
			list->RemoveAt(pos);
	}
	delete[] address;
}

//---------------------------------------------------------------------------------------
void KZip::Flash()
{
	int err = Z_OK;

    m_ZIPStream.avail_in = 0;
    
    while (err == Z_OK)
	{
		uLong uTotalOutBefore;
		if (m_ZIPStream.avail_out == 0)//»º³åÇøÂú
		{
			m_ZIPStream.avail_out	=	(uInt)m_dwBufferSize;
			m_ZIPStream.next_out	=	m_pbBuffer;
		}
		uTotalOutBefore = m_ZIPStream.total_out;
		err = deflate(&m_ZIPStream,  Z_FINISH);
		m_dwBufferLength += (uInt)(m_ZIPStream.total_out - uTotalOutBefore) ;
	}
    if (err == Z_STREAM_END)
        err = Z_OK; /* this is normal */

	err = deflateEnd(&m_ZIPStream);
}

//---------------------------------------------------------------------------------------
KUnZip::KUnZip()
{
	m_pbBuffer				=	NULL;
	m_dwBufferSize			=	0;
	m_dwReadPos				=	0;
	m_crc32					=	0;
	m_nLengthOfUnCompressed	=	0;
	m_pReadBuffer	=	new TCHAR[UNZ_BUFSIZE];
}

//---------------------------------------------------------------------------------------
KUnZip::~KUnZip()
{
	delete	m_pReadBuffer;
}

//---------------------------------------------------------------------------------------
bool KUnZip::Open(void *pData, int nLen,int nUnCompressed)
{
	m_pbBuffer		=	(PBYTE) pData;
	m_dwBufferSize	=	nLen;
	m_dwReadPos		=	0;
	m_crc32			=	0;

	m_ZIPStream.zalloc	= (alloc_func)KZip::myalloc;
	m_ZIPStream.zfree	= (free_func)KZip::myfree;
	m_ZIPStream.opaque	= &m_list;
	m_ZIPStream.total_out	=	0;

	int err = inflateInit2(&m_ZIPStream, -MAX_WBITS);

	m_ZIPStream.avail_in	=	(uInt)0;	

	m_nLengthOfUnCompressed	=	nUnCompressed;
	return (err==Z_OK)?true:false;
}

//---------------------------------------------------------------------------------------
int KUnZip::Read(void *buf, UINT len)
{
	if ((len == 0) || !buf)
		return 0;

	m_ZIPStream.next_out	=	(Bytef*)buf;
	m_ZIPStream.avail_out	=	(uInt)len;

	if (len > m_nLengthOfUnCompressed)
		m_ZIPStream.avail_out = (uInt)m_nLengthOfUnCompressed;

	uInt	iRead		=	0;
	uInt	uReadThis	=	UNZ_BUFSIZE;

	while (m_ZIPStream.avail_out>0)
	{
		if ((m_ZIPStream.avail_in==0) &&
            (m_dwReadPos<m_dwBufferSize))
		{			
			if ((m_dwBufferSize-m_dwReadPos)<uReadThis)
				uReadThis = (uInt)(m_dwBufferSize-m_dwReadPos);
			if (uReadThis == 0)
				return 0;

			memcpy(m_pReadBuffer,m_pbBuffer+m_dwReadPos,uReadThis);

			m_dwReadPos	+=	uReadThis;
			
			m_ZIPStream.next_in		=	(Bytef*)m_pReadBuffer;
			m_ZIPStream.avail_in	=	(uInt)uReadThis;
		}

		uLong uTotalOutBefore = m_ZIPStream.total_out;
		const Bytef *bufBefore = m_ZIPStream.next_out;
		int flush=Z_SYNC_FLUSH;

		int err = inflate(&m_ZIPStream,flush);

		uLong uTotalOutAfter = m_ZIPStream.total_out;
		uLong uOutThis = uTotalOutAfter-uTotalOutBefore;
		
		m_crc32		=     crc32(m_crc32,bufBefore,(uInt)(uOutThis));

		iRead += uOutThis;
		m_nLengthOfUnCompressed	-= uOutThis;

		if (err==Z_STREAM_END)
		{
			break;
		}
		else if(err!=0)
		{
			iRead	=	0;
			break;
		}
	}
	return iRead;
}

//---------------------------------------------------------------------------------------
void KUnZip::Close()
{
	if(m_pbBuffer)
		inflateEnd(&m_ZIPStream);
}

#endif