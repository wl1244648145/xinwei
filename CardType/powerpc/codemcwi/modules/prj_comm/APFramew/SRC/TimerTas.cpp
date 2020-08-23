#ifndef _INC_TIMERTASK
#include "TimerTask.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_PTRLIST
#include "PtrList.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifdef __WIN32_SIM__
#elif __NUCLEUS__
#include "NUCLEUS.h"
#else //VxWorks
//#include <intLib.h>//delete by huangjl
//#include <taskLib.h>//delete by huangjl
#include "Vxw_hdrs.h" //add by huangjl
#include "vxwk2pthread.h"
#endif
#include <stdio.h>
/////////////////////////////
//Platform related defs
#ifdef __WIN32_SIM__
#include <string.h>

#elif __NUCLEUS__

#ifndef NUCLEUS
#include "Nucleus.h"
#endif
#include <string.h>

#else //VxWorks

#ifndef __INCstringh
#include <string.h>
#endif

#endif

const UINT32 M_MAX_MINDELAYARRAY= (1<<(M_MAX_INDEX_BITS));

bool CTimerTask::s_bTimerTaskInited = false;

#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section(".oamStack")
#else
#pragma DATA_SECTION(".oamStack")
#endif
UINT8 CTimerTask_s_uStack[M_TT_STACKSIZE];
#endif

CPtrList** CTimerTask::s_pListArray = NULL;
UINT32 CTimerTask::s_uCurrentListArrayIndex = 0;
#if  (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
CMutex* CTimerTask::s_pmutexListArray = NULL;
#endif

CPtrList* CTimerTask::s_plstTimer = NULL;
UINT32 CTimerTask::s_ulstTimerSize = 0;

#if  (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
CMutex* CTimerTask::s_pmutexlstTimer = NULL;
#endif

#ifdef __WIN32_SIM__
HANDLE CTimerTask::s_semaphore;
UINT CTimerTask::s_Timer;
#elif __NUCLEUS__
NU_SEMAPHORE CTimerTask::s_semaphore;
NU_TIMER CTimerTask::s_Timer;
#ifdef SLEEP
NU_SEMAPHORE CTimerTask::tickInSleep_semaphore;
#endif
#else //VxWorks
SEM_ID CTimerTask::s_semaphore;
WDOG_ID CTimerTask::s_Timer;
#endif

#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section("p_init")
#else
#pragma CODE_SECTION("p_init")
#endif
#endif
bool CTimerTask::InitTimerTask()
{
    int i;

    if (s_bTimerTaskInited)
        return true;

    //Create the Semaphore
    if (!CreateTickSemaphore())
        return false;

    //Create Timing List Array
    s_pListArray = new CPtrList*[M_MAX_MINDELAYARRAY];
    if (s_pListArray==NULL)
        return false;

    for(i=0 ; i<M_MAX_MINDELAYARRAY ; ++i)
    {
        CPtrList* plist = new CPtrList();
        if (plist==NULL)
            return false;
        s_pListArray[i] = plist;

    }

#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    s_pmutexListArray = new CMutex;
    if (s_pmutexListArray==NULL)
        return false;
#endif

    //Create Timer Object Pool
    s_plstTimer = new CPtrList();
    if (s_plstTimer==NULL)
        return false;

#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    s_pmutexlstTimer = new CMutex;
    if (s_pmutexlstTimer==NULL)
        return false;
#endif

    WaitTimerPoolMutex();

    CTimer* pTimer = (CTimer*) new unsigned char[M_INIT_TIMER_NUM*sizeof(CTimer)];
    if (pTimer==NULL)
    	{
    	ReleaseTimerPoolMutex();
        return false;
    	}
    for (i=0; i<M_INIT_TIMER_NUM;++i)
    {
        s_plstTimer->push_back(pTimer);
        pTimer++;
    }
    s_ulstTimerSize = M_INIT_TIMER_NUM;

    ReleaseTimerPoolMutex();

#ifdef M_MONITOR_RESOURCE
	updateResPoolSize(RES_MONITOR_TIMER, M_INIT_TIMER_NUM);
#endif

    CTimerTask* pTask = new CTimerTask;
    
  
   
    if (pTask==NULL)
        return false;
    if (!pTask->Begin())
        return false;

    s_bTimerTaskInited = true;
    return true;
}

#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section("p_init")
#else
#pragma CODE_SECTION("p_init")
#endif
#endif
bool CTimerTask::CreateTickSemaphore()
{
#ifdef __WIN32_SIM__
    s_semaphore = ::CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
    return (s_semaphore!=NULL);
#elif __NUCLEUS__
#ifdef SLEEP
    NU_Create_Semaphore(&tickInSleep_semaphore, "",0, NU_FIFO);
#endif
    return (NU_SUCCESS==::NU_Create_Semaphore(&s_semaphore, "",0, NU_FIFO));
#else //VxWorks
    s_semaphore = ::semCCreate(SEM_Q_FIFO, 0);
    return (s_semaphore!=NULL);
#endif
}

#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section("p_init")
#else
#pragma CODE_SECTION("p_init")
#endif
#endif
bool CTimerTask::CreateTickTimer()
{
#ifdef __WIN32_SIM__
    s_Timer = ::timeSetEvent(M_TICKS_PER_MINDELAY,1,OnTick,0,TIME_PERIODIC|TIME_CALLBACK_FUNCTION);
    return (s_Timer!=NULL);
#elif __NUCLEUS__
	#ifdef COMIP
	::memset(&s_Timer, 0, sizeof(s_Timer));
	#else
    ::memset(&s_Timer, 0, sizeof(UNSIGNED)*NU_TIMER_SIZE);
	#endif
    return (NU_SUCCESS==::NU_Create_Timer(&s_Timer, "TTTimer", OnTick, 0, 1, M_TICKS_PER_MINDELAY, NU_ENABLE_TIMER));
#else //VxWorks
    s_Timer = ::wdCreate();
    if (s_Timer==NULL)
        return false;

	printf("CreateTickTimer\n");
    return (OK==::wdStart(s_Timer,100/* M_TICKS_PER_MINDELAY*/, (FUNCPTR)OnTick, 0));
#endif
}

#ifdef __WIN32_SIM__
void CALLBACK CTimerTask::OnTick(UINT uID, UINT, DWORD dwUser, DWORD, DWORD)
{
#ifndef NDEBUG
    //static UINT32 tickcount;
    //LOG(LOG_DEBUG3, 0, "CTimerTask::OnTick %d", ++tickcount);
#endif
    ::ReleaseSemaphore(s_semaphore, 1, NULL);
}
#elif __NUCLEUS__
#ifndef COMIP
#ifdef SLEEP
extern "C" void OnTickFunc(BOOL tickInSleep);
#else
extern "C" void OnTickFunc();
#endif
#else
extern "C" void OnTickFunc();
#endif

void CTimerTask::OnTick(UNSIGNED)
{

    ::NU_Release_Semaphore(&s_semaphore);
}
#else //VxWorks
int CTimerTask::OnTick(UINT32)
{
   printf("OnTick\n");
    ::semGive(s_semaphore);

    if (OK!=::wdStart(s_Timer, M_TICKS_PER_MINDELAY, (FUNCPTR)OnTick, 0))
        LOG(LOG_CRITICAL, 0, "Start WatchDog failed.");
}
#endif

bool CTimerTask::WaitTimerPoolMutex()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CTimerTask::WaitTimerPoolMutex");
#endif

#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    return (s_pmutexlstTimer!=NULL && s_pmutexlstTimer->Wait());
#else //VxWorks
    ::taskLock();
    return true;
#endif
}

bool CTimerTask::ReleaseTimerPoolMutex()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CTimerTask::ReleaseTimerPoolMutex");
#endif

#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    return (s_pmutexlstTimer!=NULL && s_pmutexlstTimer->Release());
#else //VxWorks
    ::taskUnlock();
    return true;
#endif
}

CTimer* CTimerTask::AllocateTimer()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CTimerTask::AllocateTimer");
#endif
    CTimer* pTimer;

    if (s_plstTimer->empty())
    {
        int grow = M_TIMER_GROW_NUM;
        pTimer = (CTimer*) new UINT8[grow * sizeof(CTimer)];
        if (pTimer==NULL)
            return pTimer;
        s_ulstTimerSize += grow;
        for(;grow>0;--grow)
        {
            s_plstTimer->push_front(pTimer);
            ++pTimer;
        }
#ifdef M_MONITOR_RESOURCE
		//pool added
		addResPool(RES_MONITOR_TIMER, grow);
#endif
    }

    pTimer = (CTimer*)s_plstTimer->front();
    s_plstTimer->pop_front();

    return pTimer;
}

bool CTimerTask::DeallocateTimer(CTimer* pTimer)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CTimerTask::DeallocateTimer");
#endif
    s_plstTimer->push_front(pTimer);
    return true;
}


bool CTimerTask::StartTimer(CTimer* pTimer)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CTimerTask::StartTimer");
#endif
    UINT32 index;

#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    if (!s_pmutexListArray->Wait())
        return false;
#else //VxWorks
    ::taskLock();
//    UINT32 oldlevel = ::intLock();//delete by huangjl
#endif

    if (!pTimer->m_bActive)
    {
        pTimer->m_uRemaining = pTimer->m_uInterval;

        index = s_uCurrentListArrayIndex + pTimer->m_uRemaining;
        index &= (M_MAX_MINDELAYARRAY - 1);

        pTimer->m_bActive = true;
        pTimer->m_uListArrayIndex = index;

        s_pListArray[index]->push_front(pTimer);
    }

#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    s_pmutexListArray->Release();
#else //VxWorks
//    ::intUnlock(oldlevel);//delete by huangjl
    ::taskUnlock();
#endif
    return true;
}

bool CTimerTask::StopTimer(CTimer* pTimer)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CTimerTask::StopTimer");
#endif

    CPtrList::iterator first;
    CPtrList::iterator last;
    CPtrList::iterator iter;

    if ( ! pTimer->m_bActive )
    {
    	return true;
    }

	if ( pTimer->m_uListArrayIndex >= M_MAX_MINDELAYARRAY)
	{
	    return false;
	}
#ifdef WBBU_CODE
pTimer->m_bActive = false;
#endif
#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    if (!s_pmutexListArray->Wait())
        return false;
#else //VxWorks
    ::taskLock();
//    UINT32 oldlevel = ::intLock();//delete by huangjl
#endif

    
    first = s_pListArray[pTimer->m_uListArrayIndex]->begin();
    last = s_pListArray[pTimer->m_uListArrayIndex]->end();
    iter = s_pListArray[pTimer->m_uListArrayIndex]->find(first, last, pTimer);
	if ( iter != last )
	{
	    s_pListArray[pTimer->m_uListArrayIndex]->erase(iter);
	}

#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    s_pmutexListArray->Release();
#else //VxWorks
//    ::intUnlock(oldlevel);//delete by huangjl
    ::taskUnlock();
#endif

    pTimer->m_bActive = false;
    return true;
}

#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section("p_init")
#else
#pragma CODE_SECTION("p_init")
#endif
#endif
CTimerTask::CTimerTask()
{
    ::memcpy(m_szName, "tTimer", 6);
    m_szName[6] = '\0';
    m_uPriority = M_TP_TT;
#ifdef WBBU_CODE
 m_uStackSize = M_TT_STACKSIZE*5;

#else
    m_uStackSize = M_TT_STACKSIZE;

#endif
#ifdef __NUCLEUS__
    m_ptrStack = CTimerTask_s_uStack;
#else
    m_uOptions = 0;
#endif
}

#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section("p_init")
#else
#pragma CODE_SECTION("p_init")
#endif
#endif
#ifdef WBBU_CODE
unsigned char test_timer = 0;


extern "C" void test_timer_f(unsigned char flag)
{
     test_timer = flag;
}
#endif
bool CTimerTask::Initialize()
{
    //start timer interrupt
    return CreateTickTimer();
}

extern void Csi_ContextSwitchRecord(UINT32 Tid);
extern void Csi_TimerReport();


void CTimerTask::MainLoop()
{
    CPtrList::iterator first;
    CPtrList::iterator last;
#ifdef ENABLE_UT_CSI_SERVICE         
    UINT32 delayCount = 0;
#endif
    printf("MainLoop\n");
    for(;;)
    {
        //wait the semaphore
        if (OK==::semTake(s_semaphore, WAIT_FOREVER))
        {

             printf("s_semaphore\n");

            ::taskLock();

			    printf("after s_semaphore In MainLoop\n");
                ++s_uCurrentListArrayIndex;
                s_uCurrentListArrayIndex &= (M_MAX_MINDELAYARRAY-1);

                if (!s_pListArray[s_uCurrentListArrayIndex]->empty())
                {
                
			        printf("In MainLoop0\n");
                    first = s_pListArray[s_uCurrentListArrayIndex]->begin();
                    last  = s_pListArray[s_uCurrentListArrayIndex]->end();

                    while (first!=last)
                    {
                        CTimer* pTimer = (CTimer*)(*first);
                        printf("In MainLoop1\n");

                        //protection added by fengbing 20090305
                        if (pTimer==NULL)
                        {               
                            //printf("\n!!!!!!!!!!pTImer==NULL!!!!!!!!!!\n");
                            //taskSuspend(taskIdSelf());
                            first = s_pListArray[s_uCurrentListArrayIndex]->erase(first);
                            continue;
                        }

                        if (pTimer->m_bActive==false)
                        {
                            first = s_pListArray[s_uCurrentListArrayIndex]->erase(first);
                            continue;
                        }
                        printf("In MainLoop2\n");
                        if (pTimer->m_uRemaining > M_MAX_MINDELAYARRAY)
                        {
                            pTimer->m_uRemaining -= M_MAX_MINDELAYARRAY;
                            ++first;
                        }
                        else
                        {
                            ++(pTimer->m_uExpireCount);
                            if (pTimer->m_bPeriodic) //Periodic timer, restore its setting.
                            {
                                pTimer->m_uRemaining = pTimer->m_uInterval;
                                if (pTimer->m_uRemaining==M_MAX_MINDELAYARRAY)
                                    ++first;
                                else
                                {
                                    UINT32 newIndex = (s_uCurrentListArrayIndex+pTimer->m_uRemaining) & (M_MAX_MINDELAYARRAY - 1);
                                    s_pListArray[newIndex]->push_back(pTimer);
                                    pTimer->m_uListArrayIndex = newIndex;
                                    first = s_pListArray[s_uCurrentListArrayIndex]->erase(first);
                                }
                            }
                            else
                            {
                                pTimer->m_bActive = false;
                                first = s_pListArray[s_uCurrentListArrayIndex]->erase(first);
                            }
			       if((pTimer->m_uMsgId ==pTimer->m_pComMsg->GetMessageId()) &&
				   (pTimer->m_tidDst==pTimer->m_pComMsg->GetDstTid())&&
				    (pTimer->m_tidSrc==pTimer->m_pComMsg->GetSrcTid()))			
                            		CComEntity::PostEntityMessage(pTimer->m_pComMsg);
				   else
                               {         
                                           StopTimer(pTimer);
                                         #if (M_TARGET==M_TGT_L3)
		   	       	  	 LOG6(LOG_DEBUG1, 0, "CTimerTask::MainLoop() orign dst =%x,sr=%x,msgid=%x, try post dst =%x,sr=%x,msgid=%x",
								 pTimer->m_tidDst,pTimer->m_tidSrc,pTimer->m_uMsgId,
								 pTimer->m_pComMsg->GetDstTid(),pTimer->m_pComMsg->GetSrcTid(),pTimer->m_pComMsg->GetMessageId());
					#endif
                               }
                        }
                    }
                }
                printf("In MainLoop out\n");

            ::taskUnlock();
        }
}
}

TID CTimerTask::GetEntityId() const
{
    return M_TID_TT;
}

#ifdef __NUCLEUS__
void PostTimeTickInSleepMode(UINT16 frameNo)
{
    UINT16 tickNum = (frameNo+5)/10;   // 100 ms per tick
    for (int i=0; i<tickNum; i++)
    {
        CTimerTask::OnTick(0);    
#ifdef SLEEP        
        ::NU_Release_Semaphore(&CTimerTask::tickInSleep_semaphore);
#endif
    }
}
#endif

