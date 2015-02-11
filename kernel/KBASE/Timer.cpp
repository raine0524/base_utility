#include "stdafx.h"
#include "KBASE/Timer.h"
#include "KBASE/AutoLock.h"
#include "KBASE/Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
KTimer::KTimer()
:m_bWantToStop(false)
{
}

//---------------------------------------------------------------------------------------
KTimer::~KTimer()
{
}

//---------------------------------------------------------------------------------------
bool KTimer::StartTimer(void)
{
	return StartThread();
}

//---------------------------------------------------------------------------------------
void KTimer::StopTimer(void)
{
	m_bWantToStop = true;
	WaitForStop();

	{
		KAutoLock l(m_secTimerEvent);
		m_mapTimerEvent.clear();
	}
}

//---------------------------------------------------------------------------------------
void KTimer::SetTimerEvent(unsigned int nEventID,unsigned long ulElapse)
{
	KAutoLock l(m_secTimerEvent);
	TIMER_EVENT_MAP::iterator it = m_mapTimerEvent.find(nEventID);
	if(it!=m_mapTimerEvent.end())
	{
		it->second.elapse= ulElapse;
		it->second.timestamp = GetTimeStamp();
	}
	else
	{
		TIMER_EVENT tTimerEvent;
		tTimerEvent.elapse = ulElapse;
		tTimerEvent.timestamp = GetTimeStamp();
		m_mapTimerEvent[nEventID]=tTimerEvent;
	}
}

//---------------------------------------------------------------------------------------
void KTimer::KillTimerEvent(unsigned int nEventID)
{
	KAutoLock l(m_secTimerEvent);
	m_mapTimerEvent.erase(nEventID);
}

//---------------------------------------------------------------------------------------
void KTimer::ThreadProcMain(void)
{
	while(!m_bWantToStop)
	{
		ProcessTimerEvent();
		Pause(1);
	}
}

//---------------------------------------------------------------------------------------
bool KTimer::ProcessTimerEvent(void)
{
	typedef std::map<unsigned long,unsigned long> MapEventID;
	MapEventID tMapEventID;
	{
		KAutoLock l(m_secTimerEvent);
		if(m_mapTimerEvent.size()==0)
		{
			return false;
		}
		
		unsigned long ulCurrentTimeStamp = GetTimeStamp();
		for(TIMER_EVENT_MAP::iterator it = m_mapTimerEvent.begin();it!=m_mapTimerEvent.end();++it)
		{
			unsigned int nEventID = (unsigned int)it->first;
			if(ulCurrentTimeStamp - it->second.timestamp > it->second.elapse)
			{
				it->second.timestamp = ulCurrentTimeStamp;

				tMapEventID[nEventID] = nEventID;
			}
		}
	}

	MapEventID::iterator iter;
	for (iter=tMapEventID.begin();iter!=tMapEventID.end();++iter)
	{
		OnTimerEvent(iter->second);
	}

	return true;
}