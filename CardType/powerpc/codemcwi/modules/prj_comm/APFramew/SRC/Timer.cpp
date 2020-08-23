#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef _INC_TIMERTASK
#include "TimerTask.h"
#endif
#include <stdio.h>
void* CTimer::operator new(size_t)
{
    CTimer* pTimer;
    void* nullptr = NULL;

#ifndef NDEBUG
    if (!CTimerTask::WaitTimerPoolMutex())
    {
        LOG(LOG_SEVERE, 0, "Wait Timer pool mutex failed.");
        return nullptr;
    }
    if ((pTimer = CTimerTask::AllocateTimer())==NULL)
    {
        LOG(LOG_SEVERE, 0, "AllocateTimer failed.");
        return nullptr;
    }
    if (!CTimerTask::ReleaseTimerPoolMutex())
        LOG(LOG_SEVERE, 0, "Release Timer pool mutex failed.");
    return pTimer;
#else
    if (!CTimerTask::WaitTimerPoolMutex())
        return nullptr;
    if ((pTimer = CTimerTask::AllocateTimer())==NULL)
    {
        return nullptr;
    }
    CTimerTask::ReleaseTimerPoolMutex();
    return pTimer;
#endif
}

void CTimer::operator delete(void* p)
{
    CTimer* pTimer = (CTimer*)p;
#ifndef NDEBUG
    if (!CTimerTask::WaitTimerPoolMutex())
    {
        LOG(LOG_SEVERE, 0, "Wait Timer pool mutex failed.");
        return;
    }
    if (!CTimerTask::DeallocateTimer(pTimer))
        LOG(LOG_SEVERE, 0, "Deallocate pTimer failed.");
    if (!CTimerTask::ReleaseTimerPoolMutex())
        LOG(LOG_SEVERE, 0, "Release Timer pool mutex failed.");
#else
    if (!CTimerTask::WaitTimerPoolMutex())
        return;
    CTimerTask::DeallocateTimer(pTimer);
    CTimerTask::ReleaseTimerPoolMutex();
#endif
}

CTimer::CTimer(bool bPeriodic, UINT32 uInterval, const CMessage& Msg)
{
	 m_uMsgId =NULL;
        m_tidDst=NULL;
        m_tidSrc=NULL;
    m_bPeriodic = bPeriodic;
    m_pComMsg = Msg.m_pMsg;
    m_uExpireCount = 0;
    init(uInterval);
#ifndef NDEBUG
    if (ASSERT_VALID(m_pComMsg))
    {
        m_pComMsg->AddRef();
	 m_uMsgId =m_pComMsg->GetMessageId();
        m_tidDst=m_pComMsg->GetDstTid();
        m_tidSrc=m_pComMsg->GetSrcTid();
    }
    else
        m_pComMsg = NULL;
#else
    m_pComMsg->AddRef();
    m_uMsgId =m_pComMsg->GetMessageId();
    m_tidDst=m_pComMsg->GetDstTid();
    m_tidSrc=m_pComMsg->GetSrcTid();
#endif

#ifdef M_MONITOR_RESOURCE
	//new timer
	TID tmp = getMsgCreatorTID(m_pComMsg);
	if(M_TID_MAX>tmp)
	{	
		m_releatedTID = tmp;
	    if (!CTimerTask::WaitTimerPoolMutex())
	        return;
		updateResStat(RES_MONITOR_TIMER, RES_OP_ALLOC, 1, tmp);
		CTimerTask::ReleaseTimerPoolMutex();
	}
#endif

}

CTimer::CTimer(bool bPeriodic, UINT32 uInterval, CComMessage* pComMsg)
{
    m_uMsgId =NULL;
    m_tidDst=NULL;
    m_tidSrc=NULL;
    m_bPeriodic = bPeriodic;
    m_pComMsg = pComMsg;
    m_uExpireCount = 0;
    init(uInterval);
#ifndef NDEBUG
    if (ASSERT_VALID(m_pComMsg))
    {
        m_pComMsg->AddRef();
	 m_uMsgId =m_pComMsg->GetMessageId();
        m_tidDst=m_pComMsg->GetDstTid();
        m_tidSrc=m_pComMsg->GetSrcTid();
    }
    else
        m_pComMsg = NULL;
#else
    m_pComMsg->AddRef();
    m_uMsgId =m_pComMsg->GetMessageId();
    m_tidDst=m_pComMsg->GetDstTid();
    m_tidSrc=m_pComMsg->GetSrcTid();
#endif

#ifdef M_MONITOR_RESOURCE
	//new timer
	TID tmp = getMsgCreatorTID(m_pComMsg);
	if(M_TID_MAX>tmp)
	{
		m_releatedTID = tmp;
	    if (!CTimerTask::WaitTimerPoolMutex())
	        return;
		updateResStat(RES_MONITOR_TIMER, RES_OP_ALLOC, 1, tmp);
		CTimerTask::ReleaseTimerPoolMutex();		
	}
#endif

}

#ifdef __NUCLEUS__
static const UINT32 msPerTick = 1000 / M_TICKS_PER_SECOND;
#endif

void CTimer::init(UINT32 uInterval)
{
#ifdef __NUCLEUS__
    m_uInterval = ( ((uInterval + msPerTick -1 )/msPerTick ) + (M_TICKS_PER_MINDELAY-1)
                    )/M_TICKS_PER_MINDELAY;
#else                        
    TIMERINTERVALTYPE uInterval64 = (TIMERINTERVALTYPE)uInterval;
    m_uInterval = (UINT32)( (((uInterval64*M_TICKS_PER_SECOND + 999)/1000) 
                             + (M_TICKS_PER_MINDELAY-1)
                            )
                            /M_TICKS_PER_MINDELAY );
#endif

    m_bActive = false;
    m_uRemaining = m_uInterval;
}
    
bool CTimer::Start()
{
#ifndef NDEBUG
    if (!ASSERT_VALID(this))
        return false;
#endif

    //if Interval==0, only expire once
    if (m_uInterval==0)
    {
       if((m_uMsgId ==m_pComMsg->GetMessageId()) &&
        (m_tidDst==m_pComMsg->GetDstTid())&&
        (m_tidSrc==m_pComMsg->GetSrcTid()))
	        CComEntity::PostEntityMessage(m_pComMsg);
	  else
	  {
	  	  #if (M_TARGET==M_TGT_L3)
	  	 LOG6(LOG_SEVERE, 0, "CTimer::Start orign dst =%x,sr=%x,msgid=%x, try post dst =%x,sr=%x,msgid=%x",
	  	 m_tidDst,m_tidSrc,m_uMsgId,
	  	 m_pComMsg->GetDstTid(),m_pComMsg->GetSrcTid(),m_pComMsg->GetMessageId());
	  	 #endif
	  }
        return true;
    }

    return CTimerTask::StartTimer(this);
}

bool CTimer::Stop()
{
#ifndef NDEBUG
    if (!ASSERT_VALID(this))
        return false;
#endif

    if (!m_bActive)
        return true;

    return CTimerTask::StopTimer(this);
}

bool CTimer::SetInterval(UINT32 uInterval)
{
#ifndef NDEBUG
    if (!ASSERT_VALID(this))
        return false;
#endif

    if (m_bActive)
        return false;

    //m_pInfo->m_uInterval = uInterval;
    init( uInterval );
    return true;
}

UINT32 CTimer::GetExpireCount() const
{
#ifndef NDEBUG
    if (!ASSERT_VALID(this))
        return 0;
#endif

    return m_uExpireCount;
}

CTimer::~CTimer()
{
    Stop();
    if (m_pComMsg!=NULL)
   	{
        m_pComMsg->Destroy();
#ifdef WBBU_CODE
    	      m_pComMsg = NULL;
#endif
#ifdef M_MONITOR_RESOURCE
		//delete timer
		if(M_TID_MAX>m_releatedTID)
		{		
			if (!CTimerTask::WaitTimerPoolMutex())
				return;
			updateResStat(RES_MONITOR_TIMER, RES_OP_FREE, -1, m_releatedTID);
			CTimerTask::ReleaseTimerPoolMutex();		
		}
#endif
   	}
}

