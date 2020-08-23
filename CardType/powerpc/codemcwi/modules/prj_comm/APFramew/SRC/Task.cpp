/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: Task.cpp
 *
 * DESCRIPTION:
 *     Implementation of the FWKLIB's Generic Task object.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 07/12/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_TASK
#include "Task.h"
#include "vxwk2pthread.h"//add by haungjl
#include <unistd.h>
#endif
#include <stdio.h>
#if !( defined(__WIN32_SIM__) || defined(__NUCLEUS__) )
#ifndef __INCtaskLibh
#include "Vxw_hdrs.h"   //add by huangjl
#endif

#endif

#ifdef __NUCLEUS__

#include <string.h>
#include <tc_defs.h>
#endif


#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


//extern "C" void CsiMonitorDeadlock(UINT32 tid, UINT32 maxBlockedTime); delete 
// by  huangjl
extern "C" void CsiEnableStackTrace(UINT32 tid);
extern void Csi_MonitorEnable(UINT16 tid);
                        
#ifdef __WIN32_SIM__
DWORD WINAPI CTask::TaskProc(void* pTask)
#elif __NUCLEUS__
VOID CTask::TaskProc(UNSIGNED argc, VOID *pTask)
#else
STATUS CTask::TaskProc(CTask* pTask)
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::TaskProc");
	
    if (pTask==NULL)
    {
        LOG(LOG_CRITICAL,0,"pTask==NULL.");

#ifdef __NUCLEUS__
		return;
#else
		return 0;
#endif
        
    }
#endif
      printf("\n##################################TaskProc#######################\n");
    ((CTask*)pTask)->Run();

#ifndef __NUCLEUS__
	return 0;
#endif
    
}

#ifndef __NUCLEUS__
bool CTask::TaskLock()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::TaskLock");
#endif

#ifdef __WIN32_SIM__
    return false;
#elif __NUCLEUS__
    ::NU_Change_Preemption(NU_NO_PREEMPT);
	return true;
#else //VxWorks
    return ::taskLock()==OK;
#endif
}

bool CTask::TaskUnlock()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::TaskUnlock");
#endif

#ifdef __WIN32_SIM__
    return false;
#elif __NUCLEUS__
    ::NU_Change_Preemption(NU_PREEMPT);
	return true;
#else
    return ::taskUnlock()==OK;
#endif
}
#endif 

#ifdef __WIN32_SIM__
TID CTask::GetCurrentTaskId()
{
    DWORD id = ::GetCurrentThreadId();
    return CComEntity::LookupTid(id);
}
#else
#include <string.h>
void CTask::GetCurrentTaskName(char *name)
{
#ifdef __NUCLEUS__
	// use ptr to TCB
    NU_TASK* task = ::NU_Current_Task_Pointer();
    ::memcpy(name, ((TC_TCB *)task)->tc_name, NU_MAX_NAME);
#else //VxWorks
    INT32 tid = ::taskIdSelf();
    char *tName = ::taskName(tid);
    ::memcpy(name, tName, 8);
#endif
}
#endif

CTask::CTask()
:m_uPriority(100),
m_uStackSize(2000),
m_uFlags(0x00000000)
#ifdef __NUCLEUS__
,m_ptrStack(NULL)
#else
,m_uMaxBlockedTime(WAIT_FOREVER)
#endif
{
    #ifndef __NUCLEUS__
    m_bNeedDeadlockMonitor = false;
    #endif

#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::CTask");

    if (!Construct(CObject::M_OID_TASK))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif

    m_szName[0]=0;

#ifdef __NUCLEUS__
    ::memset(&m_TCB, 0, sizeof(UNSIGNED)*NU_TASK_SIZE);
#else
    m_uOptions = 0;
#endif
}

bool CTask::Begin()
{
#ifndef NDEBUG
   // LOG(LOG_DEBUG3,0,"CTask::Begin");
    
     LOG(M_LL_CRITICAL,0,"CTask::Begin");
    if (!ASSERT_VALID(this))
        return false;
#endif

    if ((m_uFlags & M_TF_STATEALL)!=M_TF_STOPPED)
        return false;

#ifdef __WIN32_SIM__
    if ( (m_hThread = ::CreateThread(NULL,m_uStackSize,TaskProc,(void*)this,NULL,(DWORD*)&m_idsys)) == NULL)
    {
        LOG(LOG_SEVERE,0,"CreateThread failed.");
#elif __NUCLEUS__
	// allocate mem for task
	//m_ptrStack = NULL;
    if (m_uStackSize<NU_MIN_STACK_SIZE)
        m_uStackSize = NU_MIN_STACK_SIZE;

/*
	if(NU_SUCCESS!=NU_Allocate_Memory(&System_Memory, &m_ptrStack, 
			m_uStackSize, NU_SUSPEND))
    if ((m_ptrStack = new UINT8[m_uStackSize]) == NULL );
	{
		LOG(LOG_SEVERE,0,"NU_Allocate_Memory for CTask failed.");
		return false;
	}
*/
    //m_ptrStack = new UINT8[m_uStackSize];
    if (m_ptrStack==NULL)
        return false;
	// create task
    m_szName[NU_MAX_NAME-1] = '\0';
    memset(m_ptrStack, 0xee, m_uStackSize);
	if(NU_SUCCESS!=NU_Create_Task(&m_TCB, m_szName, TaskProc, 1, (VOID*)this, 
								  m_ptrStack, m_uStackSize, (OPTION)m_uPriority,
								  1, NU_PREEMPT, NU_START))
	{
		LOG(LOG_SEVERE,0,"NU_Create_Task failed.");
#else //VxWorks
    if (IsMonitoredForDeadlock() )
    {
        m_bNeedDeadlockMonitor = true;
    }
    m_uMaxBlockedTime = GetMaxBlockedTime();

    if ( (m_idsys=::taskSpawn(m_szName, m_uPriority, 0, 0,
                            (FUNCPTR)TaskProc, (int)this, 0, 0, 0, 0, 0,
                            0, 0, 0, 0)) == ERROR )
    {
        LOG(LOG_SEVERE,0,"taskSpawm failed.");
#endif
        return false;
    }
    else
    {
    	printf("m_idsys:%d\n",m_idsys);
 //   	sleep(1);
    }

#ifdef __NUCLEUS__
	m_idsys = (UINT32)&m_TCB;
#endif

    while ((m_uFlags & M_TF_INITED) == 0)
    {
#ifdef __WIN32_SIM__
        ::Sleep(1);
#elif __NUCLEUS__
        ::NU_Sleep(1);
#else //VxWorks

      //  ::taskDelay(1);
#endif
    }

#ifndef __WIN32_SIM__
#ifndef __NUCLEUS__
    if (m_bNeedDeadlockMonitor )
    {
//        CsiMonitorDeadlock(m_idsys, m_uMaxBlockedTime);//delete by huangjl
    }

//    CsiEnableStackTrace(m_idsys);
#endif
#endif

    return (m_uFlags & M_TF_RUNNING);
}

#ifndef __NUCLEUS__
bool CTask::Resume()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::Resume");

    if (!ASSERT_VALID(this))
        return false;
#endif

    UINT32 state = m_uFlags & M_TF_STATEALL;
    if (state==M_TF_STOPPED||state==M_TF_STATEALL)
        return false;
    else if (state==M_TF_RUNNING)
        return true;
    
#ifdef __WIN32_SIM__
	if (m_hThread==NULL)
    {
        LOG(LOG_SEVERE,0,"m_hThread==NULL.");
#elif __NUCLEUS__
	if(m_ptrStack==NULL)
	{
		LOG(LOG_SEVERE,0,"m_ptrStack==NULL.");
#else //VxWorks
    if (m_idsys==ERROR)
    {
        LOG(LOG_SEVERE,0,"m_idsys==ERROR.");
#endif
        return false;
    }

#ifdef __WIN32_SIM__
    if (::ResumeThread(m_hThread) != (DWORD)-1)
#elif __NUCLEUS__
	if (NU_Resume_Task((NU_TASK*)&m_TCB)==NU_SUCCESS)
#else //VxWorks
    if (::taskResume(m_idsys)==OK)
#endif
    {
        m_uFlags &= ~M_TF_STATEALL;
        m_uFlags |= M_TF_RUNNING;
        return true;
    }
    return false;
}

bool CTask::Suspend()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::Suspend");

    if (!ASSERT_VALID(this))
        return false;
#endif

    UINT32 state = m_uFlags & M_TF_STATEALL;

    if (state==M_TF_STOPPED || state==M_TF_STATEALL)
        return false;
    else if (state == M_TF_SUSPEND)
        return true;


#ifdef __WIN32_SIM__
    if (m_hThread==NULL)
    {
        LOG(LOG_SEVERE,0,"m_hThread==NULL.");
#elif __NUCLEUS__
	if(m_ptrStack==NULL)
	{
		LOG(LOG_SEVERE,0,"m_ptrStack==NULL.");
#else //VxWorks
    if (m_idsys==ERROR)
    {
        LOG(LOG_SEVERE,0,"m_idsys==ERROR.");
#endif
        return false;
    }

#ifdef __WIN32_SIM__
    if (::SuspendThread(m_hThread) != (DWORD)-1)
#elif __NUCLEUS__
	if (NU_Suspend_Task((NU_TASK*)&m_TCB)==NU_SUCCESS)
#else //VxWorks
    if (::taskSuspend(m_idsys)==OK)
#endif
    {
        m_uFlags &= M_TF_STATEALL;
        m_uFlags |= M_TF_SUSPEND;
        return true;
    }
    return false;
}


bool CTask::Restart()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::Restart");

    if (!ASSERT_VALID(this))
        return false;
#endif

#ifdef __WIN32_SIM__
    if (m_hThread==NULL)
    {
        LOG(LOG_SEVERE,0,"m_hThread==NULL");
#elif __NUCLEUS__
	if(m_ptrStack==NULL)
	{
		LOG(LOG_SEVERE,0,"m_ptrStack==NULL");
#else //VxWorks
    if (m_idsys==ERROR)
    {
        LOG(LOG_SEVERE,0,"m_idsys==ERROR");
#endif
        return false;
    }

#ifdef __WIN32_SIM__
    return false;
#elif __NUCLEUS__
	STATUS ret;
	ret = NU_Terminate_Task((NU_TASK*)&m_TCB);
	if(ret!=NU_SUCCESS)
		return false;
	ret = NU_Reset_Task((NU_TASK*)&m_TCB, 1, (VOID*)this);
	if(ret!=NU_SUCCESS)
		return false;
	return NU_Resume_Task((NU_TASK*)&m_TCB)==NU_SUCCESS;
#else //VxWorks
    return ::taskRestart(m_idsys)==OK;
#endif
}

bool CTask::SetPriority(UINT8 uPriority) const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::SetPriority");

    if (!ASSERT_VALID(this))
        return false;
#endif

#ifdef __WIN32_SIM__
    if (m_hThread==NULL)
    {
        LOG(LOG_SEVERE,0,"m_hThread==NULL.");
#elif __NUCLEUS__
	if(m_ptrStack==NULL)
	{
		LOG(LOG_SEVERE,0,"m_ptrStack==NULL.");
#else //VxWorks
    if (m_idsys==ERROR)
    {
        LOG(LOG_SEVERE,0,"m_idsys==ERROR.");
#endif
        return false;
    }

#ifdef __WIN32_SIM__
    return false;
#elif __NUCLEUS__
	return NU_Change_Priority((NU_TASK*)&m_TCB, (OPTION)uPriority)==NU_SUCCESS;
#else //VxWorks
    return ::taskPrioritySet(m_idsys, uPriority)==OK;
#endif
}

bool CTask::GetPriority(SINT32* pPriority) const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::GetPriority");

    if (!ASSERT_VALID(this))
        return false;
#endif

#ifdef __WIN32_SIM__
    if (m_hThread==NULL)
    {
        LOG(LOG_SEVERE,0,"m_hThread==NULL.");
#elif __NUCLEUS__
	if(m_ptrStack==NULL)
	{
		LOG(LOG_SEVERE,0,"m_ptrStack==NULL.");
#else //VxWorks
    if (m_idsys==ERROR)
    {
        LOG(LOG_SEVERE,0,"m_idsys==ERROR.");
#endif
        return false;
    }

    if (pPriority==NULL)
    {
        LOG(LOG_SEVERE,0,"pPriority==NULL.");
        return false;
    }

#ifdef __WIN32_SIM__
    return false;
#elif __NUCLEUS__
	char tName[10];
	DATA_ELEMENT tStatus;
	UNSIGNED tSche_count;
	OPTION tPrio, tPreempt;
	UNSIGNED tTime_slice;
	VOID *tStack_base;
	UNSIGNED tStack_size;
	UNSIGNED tMin_stack;
	if(NU_Task_Information((NU_TASK*)&m_TCB, tName, &tStatus, &tSche_count, 
							&tPrio, &tPreempt, &tTime_slice, &tStack_base, 
							&tStack_size, &tMin_stack) == NU_SUCCESS)
	{
		*pPriority = tPrio;
		return true;
	}
	return false;
#else //VxWorks
    return ::taskPriorityGet(m_idsys, pPriority)==OK;
#endif
}
#endif

void CTask::Run()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::Run");

    if (!ASSERT_VALID(this))
        return;
#endif
   printf("before Run\n");
   RegisterEntity(true);
   
  //  return;
    bool bInit = Initialize();
    if (bInit)
    {
        m_uFlags |= M_TF_RUNNING;
    }
    m_uFlags |= M_TF_INITED;


    if (!bInit)
    {
        LOG(LOG_CRITICAL,0,"Task Initialize failed.");
        return;
    }

#ifdef ENABLE_UT_CSI_SERVICE 
   Csi_MonitorEnable((UINT16)GetEntityId());
#endif
    RegisterEntity(false);
     printf("Run\n");
 //  return;
    MainLoop();
}

/*
bool CTask::Initialize()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::Initialize");
#endif

    LOG(LOG_CRITICAL,0,"Should not call this function.");
    ASSERT_VALID(this);
    return false;
}
*/

/*
void CTask::MainLoop()
{
    for(;;)
    {
        CComMessage* pComMsg = GetMessage();
        if (!ASSERT_VALID(pComMsg))
        {
            LOG(LOG_SEVERE,0,"GetMessage return Invalid Message.");
            continue;
        }

#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"Received Message ID=0X%x",pComMsg->GetMessageId());
#endif

        if (!ProcessComMessage(pComMsg))
        {
            LOG(LOG_WARN,0,"ProcessComMessage return false.");
            PostProcess();
        }
    }
}
*/

/*
CComMessage* CTask::GetMessage()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::GetMessage");

    LOG(LOG_CRITICAL,0,"Should not call this function.");
    ASSERT_VALID(this);
#endif
    return NULL;
}
*/

bool CTask::ProcessComMessage(CComMessage* pComMsg)
{
    bool bRet;
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::ProcessComMessage");

    if (!ASSERT_VALID(this))
        return false;

    if (!ASSERT_VALID(pComMsg))
        return false;
#endif

    CMessage msg(pComMsg);
    bRet = ProcessMessage(msg);
    pComMsg->Destroy();
    return bRet;
}


bool CTask::PostMessage(CComMessage*, SINT32, bool )
{
    return false;
}

bool CTask::ProcessMessage(CMessage& Msg)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::ProcessMessage");

    LOG(LOG_CRITICAL,0,"Should not call this function.");
    ASSERT_VALID(this);
#endif
    return false;
}

void CTask::PostProcess()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::PostProcess");

    ASSERT_VALID(this);
#endif
    return;
}

#ifndef __NUCLEUS__
bool CTask::End()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTask::End");

    if (!ASSERT_VALID(this))
        return false;
#endif

    if ((m_uFlags & M_TF_INITED)==0)
    {
        LOG(LOG_SEVERE,0,"task not initialized.");
        return false;
    }
    UINT32 state = m_uFlags & M_TF_STATEALL;
    if (state==M_TF_STOPPED)
    {
        LOG(LOG_SEVERE, 0, "Task already stopped.");
        return false;
    }
#ifdef __WIN32_SIM__
    if (!::TerminateThread(m_hThread,0))
    {
        LOG(LOG_SEVERE,0,"TerminateThread failed.");
        return false;
    }
#elif __NUCLEUS__
	// terminate task first, then delete, finally deallocate mem
	if(NU_SUCCESS!=NU_Terminate_Task(&m_TCB))
	{
		LOG(LOG_SEVERE,0,"NU_Delete_Task failed.");
		return false;
	}
	if(NU_SUCCESS!=NU_Delete_Task(&m_TCB))
	{
		LOG(LOG_SEVERE,0,"NU_Delete_Task failed.");
		return false;
	}

	//delete [] m_ptrStack;
	/*
	if(NU_SUCCESS!=NU_Deallocate_Memory(m_ptrStack))
	{
		LOG(LOG_SEVERE,0,"NU_Deallocate_Memory failed.");
		return false;
	}
	*/
#else //VxWorks
    if (::taskDelete(m_idsys)==ERROR)
    {
        LOG(LOG_SEVERE,0,"taskDelete failed.");
        return false;
    }
    while (taskIdVerify(m_idsys)==OK)
        ;
#endif
    m_uFlags &= ~M_TF_STATEALL;
    m_uFlags |= M_TF_STOPPED;
    return true;
}
#endif

#ifndef NDEBUG
bool CTask::AssertValid(const char* lpszFileName, UINT32 nLine) const
{
#ifdef __WIN32_SIM__
    if (::IsBadReadPtr(this,sizeof(CTask)) || ::IsBadWritePtr((void*)this,sizeof(CTask)) )
    {
        CLog::LogAdd(lpszFileName,nLine,M_LL_CRITICAL,0,"Invalid CTask pointer.");
        return false;
    }
#endif
    return true;
}
#endif


