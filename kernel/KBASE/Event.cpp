#include "stdafx.h"
#include "KBASE/Event.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KEvent::KEvent(bool bManualReset)
{
#ifdef WIN32
	m_hEvent = CreateEvent(NULL, bManualReset, FALSE, NULL);
#else
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init(&m_cond,NULL);
	m_nEvent=0;
	m_bManualReset=bManualReset;
#endif
}

//---------------------------------------------------------------------------------------
KEvent::~KEvent(void)
{
#ifdef WIN32
	if(m_hEvent) 
	{
		CloseHandle(m_hEvent);
		m_hEvent=NULL;
	}	
#else
	pthread_mutex_destroy(&m_mutex);
	pthread_cond_destroy(&m_cond);
#endif
}

//---------------------------------------------------------------------------------------
//对于任何等待的线程设置事件可用(有信号)或不可用(无信号)。
void KEvent::Set(void)
{
#ifdef WIN32
	SetEvent(m_hEvent);
#else
	pthread_mutex_lock(&m_mutex);
    m_nEvent=1;
	pthread_cond_broadcast(&m_cond);
    pthread_mutex_unlock(&m_mutex);
#endif
}

//---------------------------------------------------------------------------------------
//等待事件有信号或超时，dwTimeout是毫秒，如果返回true，事件被设置，否则超时。
bool KEvent::Wait(unsigned long dwTimeout)
{
	if (dwTimeout==0)
		return false;

#ifdef WIN32
	return (WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0);
#else
	bool bRet=false;
	if (dwTimeout==0xFFFFFFFF || dwTimeout==0)
	{
		pthread_mutex_lock(&m_mutex);
		while (m_nEvent == 0)
		{
			pthread_cond_wait(&m_cond, &m_mutex);
		}
		if (!m_bManualReset)
			m_nEvent=0;
		pthread_mutex_unlock(&m_mutex);
		bRet=true;
	}
	else
	{
		struct timeval now;
		struct timespec timeout;
		int retcode;
		int	sec, usec;

		gettimeofday(&now,NULL);

		sec=dwTimeout/1000;
		dwTimeout=dwTimeout-sec*1000;
		usec=dwTimeout*1000;

		timeout.tv_sec = now.tv_sec + sec;
		timeout.tv_nsec = (now.tv_usec +usec) * 1000;
		if(timeout.tv_nsec > 999999999)
		{
			 timeout.tv_sec++;
			 timeout.tv_nsec -= 1000000000;
		}

		pthread_mutex_lock(&m_mutex);
		retcode = 0;
		while (m_nEvent == 0 && retcode != ETIMEDOUT)
		{
			  retcode = pthread_cond_timedwait(&m_cond, &m_mutex, &timeout);
		}
		if (retcode == ETIMEDOUT)
		{
			bRet=false;
		} 
		else 
		{
			bRet=true;
		}
		if (!m_bManualReset)
			m_nEvent=0;
		pthread_mutex_unlock(&m_mutex);
	}
	return bRet;
#endif
}

//---------------------------------------------------------------------------------------
//设置事件不可用(无信号)。
void KEvent::Reset(void)
{
#ifdef WIN32
	ResetEvent(m_hEvent); 
#else
	pthread_mutex_lock(&m_mutex);
    m_nEvent=0;
    pthread_mutex_unlock(&m_mutex);
#endif
}

