#ifndef _INC_TIMERTASK
#define _INC_TIMERTASK

#ifndef _INC_TASK
#include "Task.h"
#endif

#ifdef __WIN32_SIM__
#elif __NUCLEUS__
#else //VxWorks
//#include <wdLib.h> //delete by huangjl
#include "Vxw_hdrs.h"
#endif
#include "Mutex.h"

#ifdef __WIN32_SIM__

#define M_TICKS_PER_MINDELAY  100
#define M_TICKS_PER_SECOND    1000
#define M_MAX_INDEX_BITS      16
#define M_INIT_TIMER_NUM      200
#define M_TIMER_GROW_NUM      100

#elif __NUCLEUS__

#define M_TICKS_PER_MINDELAY  10
#define M_TICKS_PER_SECOND    100
#define M_MAX_INDEX_BITS      2
#define M_INIT_TIMER_NUM      4
#define M_TIMER_GROW_NUM      4

#else

#define M_TICKS_PER_MINDELAY (MSToTicks(100))   // 100 ms per mindelay
#define M_TICKS_PER_SECOND    (SecondsToTicks(1)) //200
#define M_MAX_INDEX_BITS      8 //16
#define M_INIT_TIMER_NUM      500 //200
#define M_TIMER_GROW_NUM      50

#endif

////////////////////////////////////////////
//Declare all classes
class CTimer;
class CTimerTask;
class CPtrList;

/////////////////////////////////////////////
//Real Info Definition of Timer Object
class CTimerInfo
{
    friend class CTimer;
    friend class CTimerTask;
private:
    bool m_bActive;
    bool m_bPeriodic;
    UINT32 m_uRemaining;
    UINT32 m_uInterval;
    CComMessage* m_pComMsg;
    UINT32 m_uExpireCount;

    UINT32 m_uListArrayIndex;
};

#ifdef __WIN32_SIM__
#define M_TT_STACKSIZE 0
#elif __NUCLEUS__
#ifdef COMIP
#define M_TT_STACKSIZE 2048
#elif BF_NU_L2
#define M_TT_STACKSIZE 2000
#else
#define M_TT_STACKSIZE 500
#endif
#else //VxWorks
#define M_TT_STACKSIZE 10240
#endif

/////////////////////////////////////////////
//Timing Task Definition
class CTimerTask : public CTask
{
public:
    TID GetEntityId() const;
    static bool InitTimerTask();

#ifdef __NUCLEUS__
    static void OnTick(UNSIGNED);
#endif

    friend class CTimer;
private:
    static bool s_bTimerTaskInited;

    //Timing container
    static CPtrList** s_pListArray;
    static UINT32 s_uCurrentListArrayIndex;
#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    static CMutex* s_pmutexListArray;
#endif

    //CTimer Object Pool
    static CPtrList* s_plstTimer;
    static UINT32 s_ulstTimerSize;
#if (defined  __WIN32_SIM__) || (defined __NUCLEUS__)
    static CMutex* s_pmutexlstTimer;
#endif

    //system timer control block
#ifdef __WIN32_SIM__
    static HANDLE s_semaphore;
    static UINT s_Timer;
    static void CALLBACK OnTick(UINT uID,
                                UINT,
                                DWORD dwUser,
                                DWORD,
                                DWORD);
#elif __NUCLEUS__
    static NU_SEMAPHORE s_semaphore;
    static NU_TIMER s_Timer;
#else //VxWorks
    static SEM_ID s_semaphore;
    static WDOG_ID s_Timer;
    static int  OnTick(UINT32);
#endif



private:
    //Implementation of CTimer::InitTimerClass()
    static bool CreateTickSemaphore();
    static bool CreateTickTimer();

    #ifndef __NUCLEUS__
    bool IsMonitoredForDeadlock()  { return false; };
    int  GetMaxBlockedTime() { return m_uMaxBlockedTime = WAIT_FOREVER ;};
    #endif

    //CTimer Object Pool interface
    static bool WaitTimerPoolMutex();
    static bool ReleaseTimerPoolMutex();

    static CTimer* AllocateTimer();
    static bool DeallocateTimer(CTimer* pTimer);

    //Implementation of CTimer::Start() and CTimer::Stop()
    static bool StartTimer(CTimer *);
    static bool StopTimer(CTimer*);

    //task object constructor
    CTimerTask();


    //Initialize system ticking
    virtual bool Initialize();

    //Blocking on semaphore and trig timers
    virtual void MainLoop();

    CTimerTask(CTimerTask&);//forbid
    CTimerTask(CTask&);
    CTimerTask(CComEntity&);
public:
#if defined SLEEP && defined __NUCLEUS__
	static NU_SEMAPHORE tickInSleep_semaphore;
#endif
};

#endif
