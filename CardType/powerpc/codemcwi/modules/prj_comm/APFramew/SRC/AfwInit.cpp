#ifdef __WIN32_SIM__

#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif

#endif

#ifndef _INC_AFWINIT
#include "AfwInit.h"
#endif

#ifndef _INC_AFWMAIN
#include "AfwMain.h"
#endif

#ifndef _INC_OBJECT
#include "Object.h"
#endif

#ifndef _INC_COMENTITY
#include "ComEntity.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif
#ifndef _INC_TIMER
#include "TimerTask.h"
#endif

#ifndef __NUCLEUS__
#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif
#endif

#ifdef __WIN32_SIM__
#ifndef _INC_PERSONATOR
#include "Personator.h"
#endif
#endif

#if (defined __NUCLEUS__) && (defined AP_MSG_QUEUE)
#include "MsgQueue.h"
#endif

#ifndef _INC_PTRLIST
#include "PtrList.h"
#endif

#if (defined M_TGT_L3) && (!defined __NUCLEUS__)
extern bool BtsL3MemInit();
#endif


#ifdef WBBU_CODE
extern void  startTaskInforRecord();
#endif

#if 1
bool AfwInit()
{
#ifdef __WIN32_SIM__
    WSADATA wsadata;
    if (::WSAStartup(MAKEWORD(2,2), &wsadata))
        return false;
#endif

    #if (defined M_TGT_L3) && (!defined __NUCLEUS__)
        #ifdef RELOAD_MEM_OPERATOR
		//BtsL3MemInit();
        #endif
    #endif

#if(!defined M_TGT_CPE) ||(defined ENABLE_UT_CSI_SERVICE)
  //  csiInit();
    #endif

    if (!InitLogTask())
    {
    
        return false;
     }

#ifndef NDEBUG
    if (!CObject::InitObjectClass())
        return false;
#endif

    if(!CPtrList::InitPtrListClass())
    {
    	return false;
    }
	
    #if (defined __NUCLEUS__) && (defined AP_MSG_QUEUE)
    if ( !CMsgQueue::InitMsgQueueClass())
        return false;
    #endif
    
    if (!CComEntity::InitComEntityClass())
        return false;
    LOG(LOG_CRITICAL,0,"##################I can log here0 $$$$$$$$$$$$$$$$$$$\n");
    LOG2(LOG_CRITICAL,0,"##################I can log here %d:$$%d$$$$$$$$$$$$$$$$$\n",888,456);
    if (!CTimerTask::InitTimerTask())
        return false;
    else
        LOG(LOG_CRITICAL,0,"init TimerTask::InitTimerTask SUCC\n");

#if 1
#ifndef __NUCLEUS__
    if (!CTransaction::InitTransactionClass())
        return false;
#endif

#endif 

    return AfwMain();
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "vxwk2pthread.h"
#include "Vxw_hdrs.h"

static vxwk2pthread_cb_t root_tcb;
int main( int argc, char **argv )
{
    int max_priority;
    taskInit( &root_tcb, "tUsrRoot", 0, 0, 0, 0, (FUNCPTR)root_task,0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
    max_priority = sched_get_priority_max( SCHED_FIFO );
    (root_tcb.prv_priority).sched_priority = max_priority;
    pthread_attr_setschedparam( &(root_tcb.attr), &(root_tcb.prv_priority) );
    taskActivate( root_tcb.taskid );
    errno = 0;

    AfwInit();
    pthread_join(getLogpid(), 0);
    exit( 0 );
}

#endif
