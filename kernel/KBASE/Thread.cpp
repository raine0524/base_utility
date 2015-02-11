#include "stdafx.h"
#include "KBASE/Thread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KThread::KThread()
{
#ifndef WIN32
	sem_init(&m_semWaitForStop,0,1);
	m_thread = -1;
	m_bThreadStopped=true;
#else
	m_dwThreadID=0;
	m_hThread=NULL;
	m_evStop = CreateEvent(NULL,TRUE,TRUE,NULL);
	SetEvent(m_evStop);
#endif
}

//---------------------------------------------------------------------------------------
KThread::~KThread()
{
#ifndef WIN32
	sem_destroy(&m_semWaitForStop);
#else
	CloseHandle(m_evStop);
#endif
}

//---------------------------------------------------------------------------------------
bool KThread::StartThread()
{
#ifndef WIN32
	if (pthread_create(&m_thread,NULL,InitThreadProc,(void*)this)!=0)
		return false;
	sem_wait(&m_semWaitForStop);
	m_bThreadStopped=false;
	return true;
#else
	//����߳��Ѿ�����������Ҫ�ٴ���
	if (!m_hThread){ 
		//�����������̺߳���
		m_hThread = CreateThread(
					NULL,			//the handle cannot be inherited. 
                    0,				//the default Thread Stack Size is set
                    InitThreadProc,	//�̺߳���
                    this,			//�̺߳����Ĳ���
                    0,				//ʹ�̺߳������������������
                    &m_dwThreadID	//receives the thread identifier
					);
                
        }//end if (!m_hThread...

	if (m_hThread)
		ResetEvent(m_evStop);

	return	m_hThread != NULL;
#endif
}

//---------------------------------------------------------------------------------------
void KThread::WaitForStop()
{
#ifndef WIN32
	if (m_bThreadStopped==false)
	{
		sem_wait(&m_semWaitForStop);
		m_bThreadStopped=true;
		pthread_join(m_thread,NULL);
	}
#else
		
	WaitForSingleObject(m_evStop,INFINITE);

	// �����߳̾��
	HANDLE hThread = (HANDLE)InterlockedExchange((LONG *)&m_hThread, 0);
	if (hThread) {
		// �ȴ��߳���ֹ
		WaitForSingleObject(hThread, INFINITE);
		// �ر��߳̾��
		CloseHandle(hThread);
	}// end if (hThread...
#endif
}

//---------------------------------------------------------------------------------------
unsigned long  KThread::ThreadProc()
{
	ThreadProcMain();
#ifndef WIN32
	sem_post(&m_semWaitForStop);
#else
	SetEvent(m_evStop);
#endif
	return 0;
}
