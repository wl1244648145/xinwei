/*******************************************************************
*
*    DESCRIPTION: CSI.cpp - This file implements the Crash Scene Trace class.
*
*    AUTHOR: Yushu Shi
*
*    HISTORY:
*
*    DATE: 1/08/2007
*
*******************************************************************/
#include <intLib.h>
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <taskLib.h>
#include <arch/ppc/excPpcLib.h>
#include <arch/ppc/regsPpc.h>
#include <arch/ppc/vxPpcLib.h>
#include <arch/ppc/ivPpc.h>
#include <arch/ppc/fppPpcLib.h>
#include <arch/ppc/altivecPpcLib.h>

#include <limits.h>
#include <vmLib.h>
#include <trcLib.h>
#include <tickLib.h>
#include <semLib.h>

#include <private/taskLibP.h>
#include <private/semLibP.h>
#include <selectLib.h>
#include <eventLib.h>

#include <dosFsLib.h>
#include <stat.h>
#include <ramDrv.h>

#include "Csi.h"
#include "dataType.h"

#include "Log.h"
#include "ApAssert.h"
#include "mcwill_Bts.h"
#include <dbgLib.h >
#ifndef WBBU_CODE
#include "loadVersion.h"
#else
#include "loadVersion_WBBU.h"
#include <netBufLib.h >
#endif
#ifndef min
#define min(x, y)	(((x) < (y)) ? (x) : (y))
#endif /* !min */

#define MSR_RI                      0x00000002  /* Recoverable-Interrupt */

static FILE* csiFd = 0;

CSI_CRITICAL_DATA   CSI::criticalData = {0};
CSI*                CSI::Instance = NULL;

UINT8               CSI::deadTidCount = 0;
DEADLOCK_MON_TASK_INFO  CSI::DeadlockMonTaskInfo[CSI_MAX_DEADLOCK_TIDS];

bool                CSI::idleTaskEnabled;
bool                CSI::shutdown = false;

CSI_SUSPENDED_TASK_ENTRY  CSI::suspendedTaskTable[CSI_SUSPEND_TASK_NUM];


#define CSI_DIR_NAME "csi"
#ifdef  M_TGT_L3
#ifdef WBBU_CODE
#define DEVICE_RAMDISK		"/RAMD:0/"   /*  "/RAMDISK/"   */

#define FILENAME  "/RAMD:0/csi/l3-csi.log"
#else
#define FILENAME  "/RAMD/csi/l3-csi.log"
#endif
extern "C" void sysWdtCallbackInstall(FUNCPTR func, UINT32 arg,UINT32 period);
extern "C" void PetHwWatchdog();
#ifndef WBBU_CODE
extern "C" usrMemPart g_mem_by_task[M_MAX_TASK_NUM];
#endif
#else
#define FILENAME  "/RAMDL2/csi/l2-csi.log"
#endif
#ifndef WBBU_CODE
extern "C" int frkCheckTaskStatus(UINT32 *systid);
#endif
int g_blStartNewTelnet=0;
static UINT32 DLfirstTime = 3000;   // 3000*100ms = 300s

//--------------------------------------------------------------------------
//  Method: CSI
//
//  Description:  This is the constructor for the CSI class.
//              
//  Inputs: NONE
//
//  Return: NONE
//
//--------------------------------------------------------------------------
void CSI::InitImage(CSI_IMAGE* nvImage, T_TimeDate* initTimeStamp)
{
    CSI_NVRAM_HEADER  *nvHdr   = &nvImage->nvHdr;
    UINT32             nvSize  = CSI_IMAGE_SIZE;

//    ApAssertRtn((nvImage != NULL), LOG_SEVERE, LOG_ERR_CSI_INIT, "CSI::InitImage fail 0", ;);

    // Enable NVRAM writes
    if(bspEnableNvRamWrite((char *)nvImage, nvSize)==TRUE)
    {
         // Clear the whole area and set the default initial values
         memset((char *)nvImage, 0, nvSize);
         memcpy((char *)&nvHdr->initTimeStamp, (char *)initTimeStamp, sizeof(nvHdr->initTimeStamp));
         nvHdr->version     = CSI_NVRAM_LATEST_VERSION;
         #ifdef M_TGT_L3
         nvHdr->btsID       = bspGetBtsID();
         #endif
         
         memcpy((char *)&nvHdr->nvRamSafePattern[0], CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN));
         
         // Disable NVRAM writes
         bspDisableNvRamWrite((char *)nvImage, nvSize);
    }
}

CSI_NVRAM_LOG* CSI::GetNextLogRecord()
{
    CSI_IMAGE         *nvImage = (CSI_IMAGE *)CSI_IMAGE_ADDRESS;
    CSI_NVRAM_HEADER  *nvHdr   = &nvImage->nvHdr;

    // Update header.
    UINT32 logRecordIndex = nvHdr->nextLogRecordIndex;
    if(bspEnableNvRamWrite((char *)nvHdr, sizeof(*nvHdr))==TRUE)
    {
        nvHdr->nextLogRecordIndex = ((logRecordIndex + 1) >= SIZEOF(nvImage->nvLog)) ? 0 : (logRecordIndex + 1);
        bspDisableNvRamWrite((char *)nvHdr, sizeof(*nvHdr));
    }
    // Initialize next log record to null
    CSI_NVRAM_LOG* nvNextLog     = &nvImage->nvLog[nvHdr->nextLogRecordIndex];
    if(bspEnableNvRamWrite((char *)nvNextLog, sizeof(*nvNextLog))==TRUE)
    {
        nvNextLog->recordType = CSI_EMPTY_RECORD;
        bspDisableNvRamWrite((char *)nvNextLog, sizeof(*nvNextLog));
    }
    return &nvImage->nvLog[logRecordIndex];
}

CSI::CSI() 
{
    STATUS             rc;
    CSI_IMAGE         *nvImage = (CSI_IMAGE *)CSI_IMAGE_ADDRESS;
    CSI_NVRAM_HEADER  *nvHdr   = &nvImage->nvHdr;
    UINT32             nvSize  = CSI_IMAGE_SIZE;
    #ifdef M_TGT_L3
    CSI_IMAGE_NEW *nvImage_new = (CSI_IMAGE_NEW*)CSI_IMAGE_ADDRESS_NEW;
    char *nvHdrNew   = nvImage_new->nvRamSafeHead;
    char *nvTailNew   = nvImage_new->nvRamSafeTail;    
    #endif
    //DLfirstTime = 3000; 

    if (Instance != NULL)
    {
        return;
    }
    enabled = true;

    #ifdef M_TGT_L2
    DiscoverLastResetReason();
    #endif

    printf("size of CSI image is %d, nvSize=%d\n", sizeof(CSI_IMAGE), nvSize);
   /* ApAssertRtn(sizeof(*nvImage) <= nvSize, LOG_SEVERE, LOG_ERR_CSI_NVRAM_TOO_SMALL, 
                   "CSI::NVRAM is too small to hold CSI record", ;);*/
    // Initialize current runtime critical data section
    memset((char *)&criticalData, 0, sizeof(criticalData));
    criticalData.version       = CSI_NVRAM_LATEST_VERSION;
    criticalData.recordType    = CSI_EXCEPTION_RECORD;
    criticalData.resetReason   = RESET_REASON_SW_ABNORMAL;

    // Initialize current boot time
    T_TimeDate timeDate = bspGetDateTime();
    memcpy(&criticalData.rtc, &timeDate, sizeof(T_TimeDate));

    // Initialize deadlock data structures
    deadTidCount = 0;
    memset((char *)&DeadlockMonTaskInfo[0], 0, sizeof(DeadlockMonTaskInfo));

    memset((char *)suspendedTaskTable, 0, sizeof(suspendedTaskTable));

    // Setup NVRAM log record in a critical section
	::taskLock();
    int oldLevel  =::intLock();

    // If NVRAM has not been intialized
    if (   (0 != strncmp((char *)&nvHdr->nvRamSafePattern[0], (char *)CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN)) ) 
        || (nvHdr->version != CSI_NVRAM_LATEST_VERSION) 
        || (nvHdr->nextCsiRecordIndex > SIZEOF(nvImage->nvCsi)) 
        || (nvHdr->nextLogRecordIndex > SIZEOF(nvImage->nvLog))
        #ifdef M_TGT_L3
        || (nvHdr->btsID != bspGetBtsID())
        #endif
       )
    {
        InitImage(nvImage, &criticalData.rtc);
    }

    // Initialize and update header.
    if(bspEnableNvRamWrite((char *)nvHdr, sizeof(*nvHdr))==TRUE)
    {
        memcpy((char *)&nvHdr->bootTimeStamp, (char *)&timeDate, sizeof(nvHdr->bootTimeStamp));
        nvHdr->startCount += 1;
        bspDisableNvRamWrite((char *)nvHdr, sizeof(*nvHdr));
    }

    // Initialize current log record information with a special START event
    CSI_NVRAM_LOG* nvLog  = GetNextLogRecord();
    if(bspEnableNvRamWrite((char *)nvLog, sizeof(*nvLog))==TRUE)
    {
         memcpy((char *)&nvLog->timeStamp, (char *)&timeDate, sizeof(nvLog->timeStamp));
         
         char verStr[100];
         sprintf(verStr, "%s [%s, %s]", VERSION, __DATE__, __TIME__ );
         memcpy(&nvLog->loadVersion[0], verStr, sizeof(nvLog->loadVersion)-1);
         nvLog->loadVersion[sizeof(nvLog->loadVersion)-1] = 0;
         
         nvLog->csiRecordIndex  = 0;
         #ifndef WBBU_CODE
         nvLog->resetReason     = bspGetBtsResetReason();
         #else
          if(bspGetResetFlag()!=RESET_SYSTEM_SW_RESET)
            nvLog->resetReason = RESET_REASON_SW_NORMAL;
        else
            nvLog->resetReason     = bspGetBtsResetReason();
        #endif
         nvLog->recordType      = CSI_START_RECORD;
         bspDisableNvRamWrite((char *)nvLog, sizeof(*nvLog));
	}
    
    #ifdef WBBU_CODE    
   bspSetResetFlag(RESET_SYSTEM_HW_RESET);
   bspSetBtsResetReason(RESET_REASON_SW_ABNORMAL);
   #endif
    #ifdef M_TGT_L3
    // If NVRAM_new has not been intialized
    if ((0 != strncmp(nvHdrNew, (char *)CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN))) \
        ||(0 != strncmp(nvTailNew, (char *)CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN))) )       
    {
        InitImageNew();
    }
    #endif
    // End critical section
	::intUnlock(oldLevel);
    ::taskUnlock();

    //Clear Idle Counts
    idleCountMax=1;
    idleCountHighWM=0;
    idleCountLowWM=0xffffffff;
    idleCountThisPass[0]=0;
    idleCountThisPass[1]=0;
    idleCountPassIndex = 0;
    currentIdleCountAverage=0;
    
    HookExceptionISRs();

    // add hook after normal vxWorks exception processing
    excHookAdd((FUNCPTR) CsiIsrHandler);

    // Add hook so that the contextSwitchHandler is called on every context switch 
 /*   ApAssertRtn(!taskSwitchHookAdd((FUNCPTR)ContextSwitchHandler), LOG_CRITICAL, LOG_ERR_CSI_TASKHOOKADD_ERROR, 
                  "Failed to add Task Hook for CSI", ;);*/

    // Start the Idle Task
    idleCounter = prevCounter = busyCounter = 0;

    #ifdef M_TGT_L3
    // because L2 has trace task as the lowest priority task, can not start this task
    idleTaskId = taskSpawn("tCsiIdle", M_TP_CSI_IDLE, M_CSI_TASK_OPTION,
                          4096, (FUNCPTR)IdleTask,
                          (int)this, 0,0,0,0,0,0,0,0,0);

   /* ApAssertRtn(idleTaskId != 0, LOG_CRITICAL, LOG_ERR_CSI_TASKSPAWN_ERROR,
                  "Failed to spawn CSI Idle Task", ;);*/
    #else
    idleTaskId = NULL;
    #endif 

    periodicCheckTaskId = taskSpawn("tPrdChk", M_TP_CSI_STACK_CHECK, M_CSI_TASK_OPTION,
                          4096, (FUNCPTR)PeriodCheckTask,
                          (int)this, 0,0,0,0,0,0,0,0,0);

    /*ApAssertRtn(0 != periodicCheckTaskId, LOG_CRITICAL, LOG_ERR_CSI_TASKSPAWN_ERROR,
                  "Failed to spawn CSI Periodid Check Task", ;);*/

    idleTaskSupervisorId = taskSpawn("tIdleSupervisor", M_TP_IDLE_PERF, M_CSI_TASK_OPTION,
                          4096, (FUNCPTR)CpuUsageSupervisor,
                          (int) this, 0,0,0,0,0,0,0,0,0);

  /*  ApAssertRtn(idleTaskSupervisorId != NULL, LOG_CRITICAL, LOG_ERR_CSI_TASKSPAWN_ERROR,
                  "Failed to spawn CSI Idle Perf Task", ;);*/

    // register a call back function to watchdog task
    sysWdtCallbackInstall((FUNCPTR)DeadlockCheck, (UINT32)this,CSI_SW_WDT_DELAY_IN_TICK);

    // register routines to be called when reset_BTS is called, 
    // reset_BTS will be called when reboot() is invoked
    RegisterRebootCallbackFunc((FUNCPTR)ShutdownHook);

    char path[100];
    strcpy(path, DEVICE_RAMDISK);
    strcat(path, CSI_DIR_NAME);
    mkdir(path);

    Instance = this;

}

//--------------------------------------------------------------------------
//  Method: CSI
//
//  Description:  This is the destructor for the CSI class.
//              
//  Inputs: NONE
//
//  Return: NONE
//
//--------------------------------------------------------------------------
CSI::~CSI()
{
}


//--------------------------------------------------------------------------
//  Method: ShutdownHook
//
//  Description:  This is the routine hooked with reset_BTS()
//              
//  Inputs: NONE
//
//  Return: NONE
//
//--------------------------------------------------------------------------
void CSI::ShutdownHook()
{
    if(!Instance) return;

    Instance->EnterShutdownFailSafeMode();

    Instance->SaveRebootRecord();

    taskLock();
    while(1);
}



//--------------------------------------------------------------------------
//  Method: CSI::IdleTask()
//
//  Description:  This method is the lowest priority task routine which runs in 
//                an infinite loop to increment an idle counter to help detect
//                any deadloop in higher priority tasks, and also for CPU usage 
//                statistics
//              
//  Inputs: NONE
//
//  Return: STATUS
//
//--------------------------------------------------------------------------
void CSI::IdleTask()
{
    CSI *csiObj = CSI::GetInstance();
    for (;;)
    {
        csiObj->IncrementIdleCountThisPass();
    }
}



//--------------------------------------------------------------------------
//  Method: CSI::MonitorTaskDeadlock()
//
//  Description:  API function called by other tasks to register for 
//                deadloop monitoring.
//              
//  Inputs:     maxBlockedTimeInMSecs -- maximum time in non-running state, 
//                    if the task stays in non-running state longer than this
//                    time, there must be some other higher priority tasks stays in 
//                    a deadloop
//
//  Return: STATUS
//
//--------------------------------------------------------------------------
STATUS CSI::MonitorTaskDeadlock(UINT32 tid, UINT32 maxBlockedTimeInTicks)
{
	::taskLock();
    int oldLevel = ::intLock();

 /*   ApAssertRtnV(OK == ::taskIdVerify(tid), LOG_SEVERE, LOG_ERR_CSI_DEADLOOP_MON_REG, 
              "Invalid tid", {::intUnlock(oldLevel);::taskUnlock();}, ERROR);*/

    WIND_TCB* tcb = ::taskTcb(tid);

 /*   ApAssertRtnV(NULL != tcb, LOG_SEVERE, LOG_ERR_CSI_DEADLOOP_MON_REG, 
              "Cannot retrieve TCB",  {::intUnlock(oldLevel);::taskUnlock();}, ERROR);

    ApAssertRtnV((deadTidCount < SIZEOF(DeadlockMonTaskInfo)), LOG_SEVERE, LOG_ERR_CSI_DEADLOOP_MON_REG, 
              "CSI max deadlock TIDs exceeded", {::intUnlock(oldLevel);::taskUnlock();}, ERROR);*/

    // Round up to next higher integral number of seconds.
    DeadlockMonTaskInfo[deadTidCount].maxBlockedTime = 10* maxBlockedTimeInTicks / CSI_SW_WDT_DELAY_IN_TICK;
    DeadlockMonTaskInfo[deadTidCount].remainBlockedTime = DeadlockMonTaskInfo[deadTidCount].maxBlockedTime;
    DeadlockMonTaskInfo[deadTidCount].switchCount = 1;
    tcb->reserved1 = (int)&DeadlockMonTaskInfo[deadTidCount].switchCount;
    DeadlockMonTaskInfo[deadTidCount].deadlockTid = tid;

    deadTidCount++;

	::intUnlock(oldLevel);
    ::taskUnlock();

//    printf("Task %s is enabled for Deadlock Monitoring, switchCountAddr = 0x%x\n", taskName(tid), tcb->reserved1);

    return(OK);
}

//--------------------------------------------------------------------------
//  Method: CSI::RegisterTaskStackTrace()
//
//  Description:  API function called by other tasks to register for 
//                stack trace recordig in CSI record.
//              
//  Inputs:     
//  Return: STATUS
//
//--------------------------------------------------------------------------
STATUS CSI::RegisterTaskStackTrace(UINT32 tid)
{
	::taskLock();
    int oldLevel = ::intLock();
    for(int i=0; i<CSI_MAX_TASK_TRACE; i++)
    {
        if(criticalData.taskTrace[i].tid == tid)
        {
            printf("RegisterTaskStackTrace, tid(%x) has been added\n", tid);
	     ::intUnlock(oldLevel);
            ::taskUnlock();
            return OK;
        }
    }
  /*  ApAssertRtnV(OK == ::taskIdVerify(tid), LOG_SEVERE, LOG_ERR_CSI_TRACE_REG, 
              "Invalid tid",  {::intUnlock(oldLevel);::taskUnlock();}, ERROR);

    ApAssertRtnV((criticalData.traceTaskCount < SIZEOF(criticalData.taskTrace)),
              LOG_SEVERE, LOG_ERR_CSI_TRACE_REG, 
              "CSI max stack trace TIDs exceeded", 
               {::intUnlock(oldLevel);::taskUnlock();}, ERROR);*/

    criticalData.taskTrace[criticalData.traceTaskCount].tid = tid;
    criticalData.traceTaskCount ++;
    
	::intUnlock(oldLevel);
    ::taskUnlock();

    return(OK);
}




// fill the stack trace to tempotary traceBuffer 
void CSI::_trcPrint(UINT32 addr1, UINT32 addr2, UINT32 numArgs, UINT32 *args)
{
    if (*traceCount < traceBufferSize)
    {
        traceBuffer[*traceCount].addr1 = addr1;
        traceBuffer[*traceCount].addr2 = addr2;
        (*traceCount)++;
    }
}

// stack function that can be called to fill in CSI trace buffer
void CSI::trcPrint(UINT32 addr1, UINT32 addr2, UINT32 numArgs, UINT32 *args)
{
    CSI::GetInstance()->_trcPrint(addr1, addr2, numArgs, args);
}


// stack function calling CSI member function to fill in stack trace with argument info
void CSI::detailTrcPrint(UINT32 addr1, UINT32 addr2, UINT32 numArgs, UINT32 *args)
{
    CSI::GetInstance()->_detailTrcPrint(addr1, addr2, numArgs, args);
}


// fill in stack trace with the first argument
void CSI::_detailTrcPrint(UINT32 addr1, UINT32 addr2, UINT32 numArgs, UINT32 *args)
{
    if (deadlockTraceCount < SIZEOF(deadlockTraceBuffer))
    {
        deadlockTraceBuffer[deadlockTraceCount].entry = addr2;
        if (numArgs > 0)
        {
            deadlockTraceBuffer[deadlockTraceCount].arg = args[0];
        }
        else
        {
            deadlockTraceBuffer[deadlockTraceCount].arg = args[0];
        }
        deadlockTraceCount++;
    }
}



//save the task status information of all the system tasks
void CSI::GetTaskInfo()
{
    TASK_DESC   taskDesc;
    int         numIds;
    int         listId[SIZEOF(criticalData.taskStatus)];

    // Get a list of all task IDs
    numIds = taskIdListGet(listId, SIZEOF(criticalData.taskStatus));

    // Loop through all of the task IDs
    for (int id = 0; id < numIds; id++)
    {
        // Get the task info
        if (taskInfoGet(listId[id], &taskDesc) == OK)
        {
            criticalData.taskCount ++;

            memcpy(&criticalData.taskStatus[id].name[0], taskDesc.td_name, CSI_NAME_LEN);
            criticalData.taskStatus[id].name[CSI_NAME_LEN] = 0;
            criticalData.taskStatus[id].tid = taskDesc.td_id;
            criticalData.taskStatus[id].sp = (UINT32)taskDesc.td_sp;
            criticalData.taskStatus[id].base = (UINT32)taskDesc.td_pStackBase;
            criticalData.taskStatus[id].size = taskDesc.td_stackSize;

            // Some general task info
            criticalData.taskStatus[id].priority    = taskDesc.td_priority;
            criticalData.taskStatus[id].errorStatus = taskDesc.td_errorStatus;
            criticalData.taskStatus[id].pc          = ::taskTcb(taskDesc.td_id) ? ::taskTcb(taskDesc.td_id)->regs.pc : 0;
            criticalData.taskStatus[id].taskStatus      = taskDesc.td_status;

            // If VxWorks stack fill option is not used
            if (taskDesc.td_options & VX_NO_STACK_FILL)
            {
                // Not able to check stack high water mark
                criticalData.taskStatus[id].high = 0;
                criticalData.taskStatus[id].margin = 0;
            }
            else
            {
                // Record high water mark based on stack fill
                criticalData.taskStatus[id].high = taskDesc.td_stackHigh;
                criticalData.taskStatus[id].margin = taskDesc.td_stackMargin;
            }
        }
    }
}


// save task stack trace of the specific task
void CSI::GetTaskStackTrace(int taskIndex)
{
    REG_SET localRegSet;

    int tid = criticalData.taskTrace[taskIndex].tid;
    WIND_TCB* tcb = ::taskTcb(tid);

    // Registers
    memcpy(&localRegSet, &tcb->regs, sizeof(localRegSet));

    // Stack Trace
    traceCount      = &criticalData.taskTrace[taskIndex].traceCount;
    traceBuffer     = &criticalData.taskTrace[taskIndex].trace[0];
    traceBufferSize = SIZEOF(criticalData.taskTrace[taskIndex].trace);
    *traceCount     = 0;

    //call the system routine to save the trace into data member buffer
    trcStack(&localRegSet, (FUNCPTR)trcPrint, tid);
}



// save the resource trace of one specific task
// return the task ID holding the resource pended by tid
int CSI::TraceOneTaskPendingResource(int tid, int taskIndex)
{
    WIND_TCB    *tcb;
    REG_SET	    regSet;		/* register set */
    int         depth;
    CSI_RESOURCE_TRACE *trace;
    int holdingTid = 0;

    depth = criticalData.deadlockTrace[taskIndex].resourceTraceDepth;
    if (depth == CSI_MAX_RESOURCE_TRACE-1)
    {
        return 0;
    }

    tcb = taskTcb(tid);
    if (tcb != NULL)
    {
        // initialize the entry in resource trace table
        trace = &criticalData.deadlockTrace[taskIndex].resourceTrace[depth];
        trace->tid = tid;
        trace->reason = CSI_TASK_PEND_UNKNOWN;
        criticalData.deadlockTrace[taskIndex].resourceTraceDepth ++;

        if ((tcb->status & WIND_PEND)==WIND_PEND)
        {   // the task is pending on something

            // get Stack Trace
            memcpy(&regSet, &tcb->regs, sizeof(tcb->regs));
            deadlockTraceCount = 0;   // will be changed in detailedTrcPrint()

            trcStack(&regSet, (FUNCPTR)detailTrcPrint, tid);

            holdingTid = 0;

            // search for recognizable routine entry, then for resource ID
            for (UINT32 i=0; i<deadlockTraceCount; i++)
            {
                if (deadlockTraceBuffer[i].entry == (UINT32)msgQReceive)
                {
                    trace->reason = CSI_TASK_PEND_MSG;
                    trace->pendResourceId = deadlockTraceBuffer[i].arg;
                    trace->ResourceHoldingTid = 0;
                    break;
                }

                if (deadlockTraceBuffer[i].entry  == (UINT32)semTake)
                {
                    // only continue to trace if pended on semaphore
                    trace->reason = CSI_TASK_PEND_SEM;
                    trace->pendResourceId = deadlockTraceBuffer[i].arg;
#if 0
                    SEM_ID sem = (SEM_ID)trace->pendResourceId;
                    holdingTid = taskNameToId(sem->state.owner->name);
                    cout<<"owner="<<(int)sem->state.owner<< "tid="<<holdingTid<<endl;
                    if (holdingTid == ERROR)
                    {
                        holdingTid = 0;
                    }
                    trace->ResourceHoldingTid = holdingTid;
#else
trace->ResourceHoldingTid = 0;
holdingTid = 0;
#endif
                    break;
                }

                if (deadlockTraceBuffer[i].entry == (UINT32)select)
                {
                    trace->reason = CSI_TASK_PEND_SELECT;
                    trace->pendResourceId = 0;
                    trace->ResourceHoldingTid = 0;
                    break;
                }

                if (deadlockTraceBuffer[i].entry == (UINT32)eventReceive)
                {
                    trace->reason = CSI_TASK_PEND_EVENT;
                    trace->pendResourceId = deadlockTraceBuffer[i].arg;
                    trace->ResourceHoldingTid = 0;
                    break;
                }
            }
        }
    }

    return holdingTid;
}

// get the deadlock tasks resource holding trace
void CSI::DeadlockResourceTrace()
{
    int task;
    int tid;
    for (task = 0; task < (int)SIZEOF(criticalData.troubleTask); task++)
    {
        tid = criticalData.troubleTask[task].task.tid;
        if ( tid == 0)
        {
            break;
        }
        while (tid != 0)
        {
            tid = TraceOneTaskPendingResource(tid,task);
        }
    }
}


// write the critical data to NVRAM
// parameter nvCsi -- address of the CSI record in NVRAM
void CSI::UpdateCsiRecord(CSI_CRITICAL_DATA *nvCsi)
{
    PetHwWatchdog();

    if (nvCsi != NULL)
    {
        if(bspEnableNvRamWrite((char *)nvCsi, sizeof(*nvCsi))==TRUE)
    	{
            memcpy((char *)nvCsi, (char *)&criticalData, sizeof(criticalData));
            bspDisableNvRamWrite((char *)nvCsi, sizeof(*nvCsi));
    	}
    }
}


//--------------------------------------------------------------------------
//  Method: ExceptionHandler
//
//  Description:  This function handles all exceptions that are not normally
//               expected to occur.
//              
//  Inputs: pEsf - pointer to the Exception Stack Frame
//
//  Return: NONE
//
//--------------------------------------------------------------------------
void CSI::CsiIsrHandler(ESFPPC *pEsf)
{
    CSI::GetInstance()->_CsiIsrHandler(pEsf);
}

void CSI::_CsiIsrHandler(ESFPPC *pEsf)
{
    int                  tid;
    WIND_TCB            *tcb;
    CSI_IMAGE           *nvImage = (CSI_IMAGE *)(CSI_IMAGE *)CSI_IMAGE_ADDRESS;;
    CSI_NVRAM_HEADER    *nvHdr   = &nvImage->nvHdr;
    static bool          csiLogged = false;

    if (false == enabled)
    {
        return;
    }
    shutdown = true;
    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////

    // Lockout other tasks (this function is called at task level in some cases)
	::taskLock();
    int oldLevel = ::intLock();
#ifndef WBBU_CODE
    oldLevel |= _PPC_MSR_BIT_FP;
    vxMsrSet(vxMsrGet() | _PPC_MSR_BIT_FP);
#endif
    /* feed the hardware watchdog if any */
    PetHwWatchdog();
    #ifdef M_TGT_L3
    bspSetResetFlag(RESET_SYSTEM_SW_RESET);
    #else
    bspSetResetFlag(RESET_SYSTEM_L2_SELF_RESET);
    #endif
    
    // Only record the first exception in the CSI. Secondary exceptions during recovery are not logged
    //   and fall through to hardware watchdog recovery.
    if (!csiLogged)
    {

        csiLogged = true;
        T_TimeDate timeDate = bspGetDateTime();

        /* windview - level 3 event logging */
        EVT_CTX_1(EVENT_EXCEPTION, pEsf->vecOffset);

        //////////////////////////////////////////////////////////////////////////////////////////
        //  mark record as valid
        //////////////////////////////////////////////////////////////////////////////////////////
        criticalData.valid          = true;

        //////////////////////////////////////////////////////////////////////////////////////////
        //   Update Csi Image header, Initialize and update header.
        //////////////////////////////////////////////////////////////////////////////////////////
        CSI_NVRAM_LOG*     nvLog = GetNextLogRecord();
        UINT32             csiRecordIndex = SIZEOF(nvImage->nvCsi);
        CSI_CRITICAL_DATA* nvCsi     = NULL;
        RESET_REASON       resetReason = criticalData.resetReason;

        bspSetBtsResetReason(resetReason);

        switch (criticalData.recordType)
        {
            case CSI_REBOOTHOOK_RECORD:
                if(bspEnableNvRamWrite((char *)nvHdr, sizeof(*nvHdr))==TRUE)
                {
                    nvHdr->rebootCount += 1;
                    bspDisableNvRamWrite((char *)nvHdr, sizeof(*nvHdr));
		  }
                break;

            case CSI_CRITICALALARM_RECORD:
                // fall through to next case
            case CSI_CRITICALASSERTION_RECORD:
                // fall through to next case
            default:
                csiRecordIndex = nvHdr->nextCsiRecordIndex;
                nvCsi = &nvImage->nvCsi[csiRecordIndex];
                if(bspEnableNvRamWrite((char *)nvHdr, sizeof(*nvHdr))==TRUE)
                {
                    nvHdr->crashCount += 1;
                    nvHdr->nextCsiRecordIndex   = (csiRecordIndex >= SIZEOF(nvImage->nvCsi)) ? 0 : (csiRecordIndex + 1);
                    bspDisableNvRamWrite((char *)nvHdr, sizeof(*nvHdr));
            	  }
                break;
        }

        ///////////////////////////////////////////////
        // fill one NVLog entry, for exception cases, still needs
        // to fill in detail reset reason ,
        // leave the csi record to be invalid if CSI quits now
         char verStr[100];
        if(bspEnableNvRamWrite((char *)nvLog, sizeof(*nvLog))==TRUE)
        {
             nvLog->timeStamp = timeDate;
           //  char verStr[100];
             sprintf(verStr, "%s [%s, %s]", VERSION, __DATE__, __TIME__ );
             memcpy(&nvLog->loadVersion[0], &verStr, sizeof(nvLog->loadVersion)-1);
             nvLog->loadVersion[sizeof(nvLog->loadVersion)-1] = 0;
             nvLog->recordType      = criticalData.recordType;
             nvLog->resetReason     = resetReason;
             if (csiRecordIndex < SIZEOF(nvImage->nvCsi))
             {
                 nvLog->csiRecordIndex  = csiRecordIndex + 1;  // 0 is used for last csi record in display
             }
             else
             {
                 nvLog->csiRecordIndex  = 0;  // 0 is used for last csi record in display
             }
             bspDisableNvRamWrite((char *)nvLog, sizeof(*nvLog));
        }
        ///////////////////////
        // prepare critical data 

        //////////////////////////////////////////////////////////////////////////////////////////
        //   1. Log general info
        //////////////////////////////////////////////////////////////////////////////////////////
        criticalData.rtc = timeDate;
        memcpy(&criticalData.loadVersion[0], &verStr, sizeof(criticalData.loadVersion)-1);
        criticalData.loadVersion[sizeof(criticalData.loadVersion)-1] = 0;

        //////////////////////////////////////////////////////////////////////////////////////////
        //   Log trouble task info
        //////////////////////////////////////////////////////////////////////////////////////////
        int  task;
        switch ( criticalData.recordType )
        {
            case CSI_REBOOTHOOK_RECORD:
            case CSI_SW_MEMORY_LEAK:
                criticalData.troubleTask[0].task.tid = 0;
                break;
            case CSI_WATCHDOGTASK_RECORD:
            case CSI_STACKCHECK_RECORD:
            case CSI_TASK_DEAD_RECORD:
                criticalData.troubleTask[0].task.tid = pEsf->pad1;
                break;
            case CSI_DEADLOCK_RECORD:
                task = 0;   // gpr in regSet is used to save deadlock tids
                while (((tid = pEsf->regSet.gpr[task]) != 0) && 
                         (task < GREG_NUM) && (task < (int)SIZEOF(criticalData.troubleTask)))
                {
                    criticalData.troubleTask[task].task.tid = tid;
                    task++;
                }
                break;
            default:
                criticalData.troubleTask[0].task.tid = taskIdSelf();
                break;
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////
        //   Log register info 
        //////////////////////////////////////////////////////////////////////////////////////////
        memcpy((char *)&criticalData.troubleTask[0].esf, (char *)pEsf, sizeof(ESFPPC));

        tcb = NULL;
        for (task = 0; task < (int)SIZEOF(criticalData.troubleTask); task++)
        {
            tid = criticalData.troubleTask[task].task.tid;
            if ( tid != 0)
            {
                int loopCounter;
                for (loopCounter = 0; loopCounter < criticalData.traceTaskCount; loopCounter++)
                {
                    if (tid == criticalData.taskTrace[loopCounter].tid)
                    {
                        break;
                    }
                }
                if ( loopCounter == criticalData.traceTaskCount)
                {   // the task is not in the list, add it as the last one
                    if (criticalData.traceTaskCount < CSI_MAX_TASK_TRACE )
                    {
                        criticalData.taskTrace[criticalData.traceTaskCount++].tid = tid;
                    }
                }

                if(  (ERROR != ::taskIdVerify(tid))
                  && ((tcb = ::taskTcb(tid)) != NULL)
                  )
                {
                    // Basic Task Info
                    criticalData.troubleTask[task].task.pc = (UINT32)tcb->entry;
                    memcpy(&criticalData.troubleTask[task].task.name[0], taskName(tid), CSI_NAME_LEN);
                    criticalData.troubleTask[task].task.name[CSI_NAME_LEN] = 0;

                    // Copy the ESF into the Critical data buffer for DEADLOCK record
                    if (criticalData.recordType == CSI_DEADLOCK_RECORD)
                    {
                        memcpy(&criticalData.troubleTask[task].esf.regSet, &tcb->regs, sizeof(tcb->regs));
                    }

                }
                else
                {
                    strncpy((char*)&criticalData.troubleTask[task].task.name[0], "INVALID", CSI_NAME_LEN);
                    criticalData.troubleTask[task].task.name[CSI_NAME_LEN] = 0;
                }
            }
            else
            {
                break;   
            }
        }

        UpdateCsiRecord(nvCsi);  //save partial critical data, in case the CPU resets before
                                 // next sanitory point
        
        //////////////////////////////////////////////////////////////////////////////////////////
        //  Log Raw stack data for exception outages, which has only one abnormal task
        //////////////////////////////////////////////////////////////////////////////////////////
        if( (criticalData.recordType == CSI_EXCEPTION_RECORD)
             || (criticalData.recordType == CSI_WATCHDOGTASK_RECORD)
             || (criticalData.recordType == CSI_STACKCHECK_RECORD)
             || (criticalData.recordType == CSI_TASK_DEAD_RECORD)
            )
        {
            if ( tcb!= NULL)
            {
                UINT32* stackPointer = (UINT32*)criticalData.troubleTask[0].esf.regSet.gpr[1];
                UINT32  stackSize    = (UINT32)tcb->pStackBase - (UINT32)stackPointer;
                   // tcb is the result of previous code execution

                stackSize = min(stackSize, sizeof(criticalData.rawStack));

                criticalData.troubleTask[0].esf.pad1 = (UINT32)stackPointer;
                memcpy((UINT8*)&criticalData.rawStack[0], (UINT8*)stackPointer, stackSize);
            }
            else
            {
                criticalData.troubleTask[0].esf.pad1 = 0;
            }
        }

        UpdateCsiRecord(nvCsi);
        //////////////////////////////////////////////////////////////////////////////////////////
        //   Log status and trace info of all  tasks
        //   Log stack info. Do deep trace for all tasks
        //////////////////////////////////////////////////////////////////////////////////////////
        GetTaskInfo();
        UpdateCsiRecord(nvCsi);

        //////////////////////////////////////////////////////////////////////////////////////////
        // save the stack trace of all the tasks
        //////////////////////////////////////////////////////////////////////////////////////////
        for (int loopCounter = 0; loopCounter < criticalData.traceTaskCount; loopCounter++)
        {
            tid = criticalData.taskTrace[loopCounter].tid;
            if ( tid != 0  )
            {
                GetTaskStackTrace(loopCounter);
            }
        }
        UpdateCsiRecord(nvCsi);
        
        memPartInfoGet(memSysPartId, &criticalData.memHeapStats);
        criticalData.memHeapStatsSafePattern = CSI_MEM_STATS_SAFE_PATTERN;
        UpdateCsiRecord(nvCsi);

        if (criticalData.recordType == CSI_DEADLOCK_RECORD)
        {
            DeadlockResourceTrace();
            UpdateCsiRecord(nvCsi);
        }

        #ifdef M_TGT_L3
	 #ifndef WBBU_CODE
        memcpy(criticalData.MemoryUsage, g_mem_by_task, sizeof(criticalData.MemoryUsage));
	 #endif
        UpdateCsiRecord(nvCsi);
        #endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // Reset the processor
        //////////////////////////////////////////////////////////////////////////////////////////
        //printf("CSI try to reset BTS, reason = %d\n",criticalData.recordType);
		// should not print here, in case the serial port is already dead the task will be 
		// suspended

        #ifdef M_TGT_L3
        ResetBTSViaCPLD();
        #else
        RequestL3ToResetL2(L2_PCI_IF_L2_REQUEST_REBOOT_CSI);
        #endif

        while(1);   // let HW watchdog save us if reset fails 
        // For gracefull shutdowns, resume processing after CSI log captured.
    }
    else
    {
        // Double fault. Spin, let the hardware watchdog save us.
        //printf("Double fault detected by CSI, reason = %d using HW watchdog to reboot\n",criticalData.recordType );
		// should not call printf here, in case the serial port is dead the task will be 
		// suspended

        // force the BTS to reboot if gets to this point
        #ifdef M_TGT_L3
        ResetBTSViaCPLD();
        #else
        RequestL3ToResetL2(L2_PCI_IF_L2_REQUEST_REBOOT_CSI);
        #endif

        while(1);
    }

    return;
}


//--------------------------------------------------------------------------
//  Method: ContextSwitchHandler
//
//  Description:  This function is called to save the context switch info.
//              
//  Inputs: pOldTcb - pointer to the old TCB
//  Inputs: pNewTcb - pointer to the new TCB
//
//  Return: NONE
//
//--------------------------------------------------------------------------
void CSI::ContextSwitchHandler(WIND_TCB *pOldTcb, WIND_TCB *pNewTcb)
{
    CSI_CONTEXT_INFO* context;
    UINT32         contextCount = criticalData.contextSwitchHistory.contextCount;

    // Save TID and PC at each context switch for history in CSI crash records
    context      = &criticalData.contextSwitchHistory.oldContext[contextCount];
    context->tid = (int)pOldTcb;
    context->pc  = pOldTcb->regs.pc;

    context      = &criticalData.contextSwitchHistory.newContext[contextCount];
    context->tid = (int)pNewTcb;
    context->pc  = pNewTcb->regs.pc;

    if (++criticalData.contextSwitchHistory.contextCount >= SIZEOF(criticalData.contextSwitchHistory.newContext))
    {
        criticalData.contextSwitchHistory.contextCount = 0;
    }

    // Deadlock monitoring
    if(pNewTcb->reserved1)   // reserved1 saves the address of switchCount
    {
        (*(UINT32*)(pNewTcb->reserved1))++;
    }
}


int  CSI::WdtExceptionHandler(ESFPPC *pEsf)
{
    CsiIsrHandler(pEsf);
    return 0;
}




//--------------------------------------------------------------------------
//  Method:  DeadlockCheck
//
//  Description:  This routine check if any deadlock condition has happened
//                called by SW watchdog task every 10 ticks
//
//  Inputs: NONE
//
//  Return: NONE
//
//--------------------------------------------------------------------------
int csiWdtCnt = 0;
int csiDeadlockCnt = 0;
#define  CSI_MAX_TASK_NUM 100
int csiTaskList[CSI_MAX_TASK_NUM];
#ifdef M_TGT_L3
extern "C" void ToggleRunningLed();
#include <memLib.h>
#endif
#ifndef WBBU_CODE
extern "C" STATUS exitTelnetSession();
#endif
UINT8 enableExit = 1;
UINT8 times_count = 0;
#ifdef WBBU_CODE
UINT8 memLowSize = 0;
#endif
void CSI::DeadlockCheck(CSI *csi)
{
    unsigned int    i, j;
    static bool     deadlock = false;


    #ifdef M_TGT_L3
    static int toggleLedInterval = 0;
    if (++toggleLedInterval>= 10)
    {
        ToggleRunningLed();
        toggleLedInterval = 0;
    }
    #endif

    if (DLfirstTime > 0) 
    {   // do not check deadlock for the first 2 minutes
        DLfirstTime--;
        csi->lastIdleCountIndex = csi->idleCountPassIndex;
        csi->lastPassIdleCount = csi->idleCountThisPass[csi->idleCountPassIndex];
        return;
    }
#ifndef WBBU_CODE
//for task dead record
  static int __cnt=0;
  __cnt++;

   if((__cnt%100) == 0)  
    {   
        UINT32 tid = 0;
        
        if(-1==frkCheckTaskStatus(&tid))
        {      
            memset((char *)&criticalData, 0, sizeof(criticalData));
            
            if(csi->enabled)
            {
                ESFPPC     esf;
                WIND_TCB    *tcb = NULL;

                memset(&esf, 0, sizeof(esf));

                if(ERROR != ::taskIdVerify(tid))
                {
                    tcb = ::taskTcb(tid);
                }

                if(NULL != tcb)
                {
                    // Copy the registers from TCB of the subject task
                    memcpy(&esf.regSet, &tcb->regs, sizeof(esf.regSet));
                    esf.pad1  = tid;    // use pad1 field to save the TID of the deadloop task
                }
                else
                {
                    esf.pad1  = 0;   // did not find the trouble causing task
                }

                criticalData.recordType = CSI_TASK_DEAD_RECORD;
                criticalData.resetReason = RESET_REASON_SW_ABNORMAL;
                WdtExceptionHandler(&esf);
            }
        }
    }
   #endif
//end
    #ifdef M_TGT_L3
    
    
    int maxMemSize = memFindMax();
    if (maxMemSize < 2000000 )
    {   /* reboot the BTS if the maximum free memory block size is below 2MB */
        printf("Reboot by memory leak, maximum block size = %d\n", maxMemSize);
        #ifdef WBBU_CODE
        memLowSize++;
        #endif
        #ifdef WBBU_CODE
        if(memLowSize>3)
        {
        #endif
        ESFPPC       esf;
        memset(&esf,0,sizeof(esf));
        criticalData.recordType = CSI_SW_MEMORY_LEAK;
        criticalData.resetReason = RESET_REASON_SW_ABNORMAL;

        if(csi->enabled)
        {
            WdtExceptionHandler(&esf);
        }
       #ifdef WBBU_CODE
       }
       #endif
    }
    
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////
    // because L2 has trace task as the lowest priority task, can not start this task
    //////////////////////////////////////////////////////////////////////////////////////////
    // Check software Idle counter (considering rollover on the idleCounter)
    // to see if any task is in deadloop -- assume the lowest priorty task in context
    // switch history is the culprit
    if (csi->lastIdleCountIndex == csi->idleCountPassIndex)
    {
        csi->idleCounter += csi->idleCountThisPass[csi->idleCountPassIndex] - csi->lastPassIdleCount;
    }
    else
    {
        csi->idleCounter += csi->idleCountThisPass[csi->idleCountPassIndex];
    }

    if (  (csi->idleCounter >= csi->prevCounter)
       && ((csi->idleCounter - csi->prevCounter) < CSI_IDLE_MIN_IDLE_COUNT)
       )
    {
        csi->busyCounter++;
    }
    else
    {
        csi->busyCounter = 0;
    }

    csi->prevCounter = csi->idleCounter;
    csi->lastIdleCountIndex = csi->idleCountPassIndex;
    csi->lastPassIdleCount = csi->idleCountThisPass[csi->idleCountPassIndex];

    // Check CPU busy status (Idle task blocked)
    if ( csi->busyCounter > (CSI_IDLE_MAX_BUSY_TIME) )
    {
        csi->busyCounter = 0;

        ::taskLock(); // Shut down all tasking as a software watchdog is in progress
        UINT32 oldLevel = ::intLock();

        ESFPPC       esf;
        WIND_TCB    *tcb = NULL;
        unsigned int count;
        UINT8        pri;
        int          tid       = 0;
        UINT8        lowestPri = 0;
        int          lowestTid = 0;
        WIND_TCB    *lowestTcb = NULL;

        memset(&esf, 0, sizeof(esf));

        for(count = 0; count < SIZEOF(criticalData.contextSwitchHistory.oldContext); count++)
        {
            tid = criticalData.contextSwitchHistory.oldContext[count].tid;

            if(ERROR != ::taskIdVerify(tid))
            {
                tcb = ::taskTcb(tid);
            }

            if(NULL != tcb)
            {
                pri = tcb->priority;

                if(pri >= lowestPri)
                {
                    lowestPri = pri;
                    lowestTid = tid;
                    lowestTcb = tcb;
                }
            }
        }
        
        if (NULL != lowestTcb)
        {
            tcb = lowestTcb;
            tid = lowestTid;
        }

        if(NULL != tcb)
        {
            // Copy the registers from TCB of the subject task
            memcpy(&esf.regSet, &tcb->regs, sizeof(esf.regSet));
            esf.pad1  = tid;    // use pad1 field to save the TID of the deadloop task
        }
        else
        {
            esf.pad1  = 0;   // did not find the trouble causing task
        }

        criticalData.recordType = CSI_WATCHDOGTASK_RECORD;
        criticalData.resetReason = RESET_REASON_SW_WDT;

        if(csi->enabled)
        {
            WdtExceptionHandler(&esf);
        }

        ::intUnlock(oldLevel);
        ::taskUnlock();

        csiWdtCnt++;
    }

    #endif

    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    // Check for deadlocked tasks (only those registered)
    if (!deadlock && !shutdown)
    {
        UINT32 taskId[SIZEOF(DeadlockMonTaskInfo)];

        memset(taskId, 0 ,sizeof(taskId));

        for (i = 0; i < deadTidCount; i++)
        {
            if(DeadlockMonTaskInfo[i].deadlockTid != 0)
            {
                if (   (DeadlockMonTaskInfo[i].switchCount == 0) 
                    && (taskIsReady(DeadlockMonTaskInfo[i].deadlockTid) == FALSE))
                {
                    if ( --DeadlockMonTaskInfo[i].remainBlockedTime == 0)
                    {
                        deadlock = true;
                        taskId[i] = DeadlockMonTaskInfo[i].deadlockTid;
printf("Task Deadlock detected, tid = 0x%x\n", taskId[i]);
                         tt(taskId[i]);//wangwenhua add 20111116
                    }
                }
                else
                {
                    DeadlockMonTaskInfo[i].remainBlockedTime = DeadlockMonTaskInfo[i].maxBlockedTime;
                }

                // prepare for next round checking
                DeadlockMonTaskInfo[i].switchCount = 0;
            }
        }

        // If one or more tasks are deadlocked
        if (deadlock)
        {
            ::taskLock(); // Shut down all tasking as a software watchdog is in progress
            UINT32 oldLevel = ::intLock();

            ESFPPC     esf;

            memset(&esf, 0, sizeof(esf));

            // Send list of deadlocked task IDs in gpr regs
            for (i = 0, j = 0; i < SIZEOF(taskId) && j < GREG_NUM; i++)
            {
                if (taskId[i] != 0)
                {
                    esf.regSet.gpr[j++] = taskId[i];
                }
            }

            criticalData.recordType = CSI_DEADLOCK_RECORD;
            criticalData.resetReason = RESET_REASON_SW_ABNORMAL;
            
            if(csi->enabled)
            {
                WdtExceptionHandler(&esf);
            }

            ::intUnlock(oldLevel);
            ::taskUnlock();

            csiDeadlockCnt++;
        }
    }

    // reboot BTS if any task got suspended for more than 120 seconds
    if (!deadlock && !shutdown)
    {
        bool suspendedInThisRound[CSI_SUSPEND_TASK_NUM];
        for (int tmpIndex=0; tmpIndex<CSI_SUSPEND_TASK_NUM; tmpIndex++)
        {
            suspendedInThisRound[tmpIndex] = false;
        }
        int taskNum = taskIdListGet(csiTaskList,CSI_MAX_TASK_NUM);
        for (int taskIndex=0; taskIndex<taskNum; taskIndex++)
        {
            if ( taskIsSuspended(csiTaskList[taskIndex]) )
            { //wangwenhua add 20080811
     
		   TASK_DESC   taskDesc;
		   UINT8 result = 0;
                  if(taskInfoGet(csiTaskList[taskIndex], &taskDesc) ==0)
		      {
                  	
		    //if the task is telnet,then passed ;
		           UINT8 taskname[] = {"tTelnetIn"};
			   if(memcmp(taskDesc.td_name,taskname,9) == 0)
			   {
			       result = 1;
			   }
            	     }
		    if(result == 1)
		    {
		       if(enableExit==1)
		      {
			if((times_count%20)==0)
			{
		         printf("Telnet task suspend!");
			}
			times_count++;
#ifndef WBBU_CODE
		         exitTelnetSession();
#endif
			// taskDelete(csiTaskList[taskIndex]);
			 //taskDelete(csiTaskList[taskIndex - 1]);
			  continue;
		       }
		    }
                int entryIndex=0;
                for (entryIndex=0; entryIndex<CSI_SUSPEND_TASK_NUM; entryIndex++)
                {  // try to find the task in suspended task table
                    if (suspendedTaskTable[entryIndex].tid == csiTaskList[taskIndex] )
                    {  // found, decrement TTL, reach 0 then reboot BTS
                        suspendedInThisRound[entryIndex] = true;
                        if( suspendedTaskTable[entryIndex].TTL>0)
                        	{
                        suspendedTaskTable[entryIndex].TTL --;
                        	}
                        if ( 0 == suspendedTaskTable[entryIndex].TTL)
                        {
                            ::taskLock(); // Shut down all tasking as a software watchdog is in progress
                            UINT32 oldLevel = ::intLock();

                            ESFPPC     esf;

                            memset(&esf, 0, sizeof(esf));
                            esf.regSet.gpr[0] = csiTaskList[taskIndex];

                            criticalData.recordType = CSI_DEADLOCK_RECORD;
                            criticalData.resetReason = RESET_REASON_SW_ABNORMAL;

                            if(csi->enabled && !g_blStartNewTelnet)
                            {
                            	//del the line below for new telnet test -fengbing 20080417
                                WdtExceptionHandler(&esf);
                            }

                            ::intUnlock(oldLevel);
                            ::taskUnlock();

                            csiDeadlockCnt++;
                        }
                        break;
                    }
                }
                if (CSI_SUSPEND_TASK_NUM == entryIndex)
                {  // found a new suspended task, need to put into the table
                    for (entryIndex = 0; entryIndex<CSI_SUSPEND_TASK_NUM; entryIndex++)
                    {
                        if ( 0 == suspendedTaskTable[entryIndex].tid)
                        {
printf("Task 0x%x is put into deadlock list\n", csiTaskList[taskIndex]);
                            suspendedTaskTable[entryIndex].tid = csiTaskList[taskIndex];
                            suspendedTaskTable[entryIndex].TTL = 1200;  // 2 minutes
                            suspendedInThisRound[entryIndex] = true;
                            break;
                        }
                    }
                }
            }
        }
        for (int tmpIndex=0; tmpIndex<CSI_SUSPEND_TASK_NUM; tmpIndex++)
        {
            if ( false == suspendedInThisRound[tmpIndex])
            {
                suspendedTaskTable[tmpIndex].tid = 0;
            }
        }

    }
}

void CSI::ShowSuspendedTaskInfo()
{
    for (int i=0; i<CSI_SUSPEND_TASK_NUM; i++)
    {
        if (suspendedTaskTable[i].tid)
        {
            printf("TID 0x%0x  TTL: %d\n", suspendedTaskTable[i].tid, 
                   suspendedTaskTable[i].TTL);
        }
    }
}

STATUS csiDeadTaskShow()
{
    CSI::GetInstance()->ShowSuspendedTaskInfo();
    return OK;
}

//--------------------------------------------------------------------------
//  Method: DisableDeadlockCheck
//
//  Description:  This method forces the CSI deadlock check to
//                abandon checks on any registered tasks.
//              
//  Inputs: NONE
//
//  Return: NONE
//
//--------------------------------------------------------------------------
void CSI::DisableDeadlockCheck(UINT32 tid)
{
    for (unsigned int i = 0; i < SIZEOF(DeadlockMonTaskInfo); i++)
    {
        if(DeadlockMonTaskInfo[i].deadlockTid == tid)
        {
            DeadlockMonTaskInfo[i].deadlockTid = 0;
            if(deadTidCount>0)
            	{
            deadTidCount --;
            	}
        }
    }
}



//--------------------------------------------------------------------------
//  Method: CSI::IdlePerf()
//
//  Description:  This method calculates average idle counter as well has high and low idle count water marks.
//				  It is called at a higher priority than application threads and lower than executive threads.  
//				  Thread IdleCountSupervisor calls this method once every second.
//              
//  Inputs: NONE
//
//  Return: STATUS
//
//--------------------------------------------------------------------------
unsigned int idleCountPublic = 0;
STATUS CSI::IdlePerf(void)
{
	static UINT32 firstTime = 180;
    if (firstTime > 0) 
    {
        idleCountThisPass[idleCountPassIndex] = 0;
        firstTime--;
        return OK;
    }
 
    // swap to use the other counter and reset it before using it
    // to avoid the idle task increment the same counter on the previous value (race condition and non-atomic operation)
    UINT32 idleCounterTmp = idleCountThisPass[idleCountPassIndex];
    idleCountPassIndex = 1 - idleCountPassIndex;
    idleCountThisPass[idleCountPassIndex] = 0;
    
    //Calculate an Idle Count Average.
    // The math is currentIdleCountAveraye = (1/8)*idleCountThisPass + (7/8)* currentIdleCountAverage
    currentIdleCountAverage = (idleCounterTmp >> 3) + currentIdleCountAverage - (currentIdleCountAverage >> 3);

	//Store max idle count
	if (idleCountMax < idleCounterTmp) 
	{
		idleCountMax = idleCounterTmp;
	}

    // Update HWM
    if((idleCounterTmp) > idleCountHighWM)
	{
		idleCountHighWM = idleCounterTmp;
	}
	else
	{
		//Reduce water mark gradually,  Quick divide by 512 using shift.
		idleCountHighWM = idleCountHighWM - (idleCountHighWM >> 10);
	}

	// Update LWM
	if(idleCounterTmp < idleCountLowWM)
	{
		idleCountLowWM = idleCounterTmp;
	}
	else
	{
		//Increase water mark gradually,  Quick divide by 512 using shift.
		idleCountLowWM = idleCountLowWM + (((idleCountLowWM >> 10) > 0) ? (idleCountLowWM >> 10) : (idleCountMax >> 6));
	}
	//Clear This idle count for next pass
	idleCountPublic = idleCounterTmp;
	
}


//--------------------------------------------------------------------------
//  Method: CpuUsageSupervisor
//
//  Description:  This task processes idle counters.
//              
//  Inputs: csi - pointer to the CSI Object
//
//  Return: NONE
//
//--------------------------------------------------------------------------
void CSI::CpuUsageSupervisor(CSI *csi)
{
    FOREVER
    {
        csi->IdlePerf();
        taskDelay(SecondsToTicks(1));
    }
}


//--------------------------------------------------------------------------
//  Method: EnterShutdownFailSafeMode
//
//  Description:  This method places CSI in a mode which guarantees reboot
//                after the busy-idle time expires. This mode is used during
//                shutdown to insure no deadlock occurs while shutting down.
//              
//  Inputs: None
//
//  Return: NONE
//
//--------------------------------------------------------------------------
void CSI::EnterShutdownFailSafeMode(void)
{
    enabled = true;   // make sure the reboot will happen
   // Delete the Idle task
    if(0 != idleTaskId)
    {
        idleCounter++; // Pet idle counter once before we begin shutdown to make
                       //  sure shutdown isn't prematurely aborted.

        int theTaskId = idleTaskId;
        idleTaskId = 0;

        idleTaskEnabled = FALSE;

        if(theTaskId != taskIdSelf())
        {
            taskDelete(theTaskId);
        }
    }
}


//--------------------------------------------------------------------------
//  Method: PeriodCheckTask
//
//  Description:  This task runs periodically to check the stacks of all tasks.
//              
//  Inputs: csi - pointer to the CSI Object
//
//  Return: NONE
//
//--------------------------------------------------------------------------
bool CSI::StackCheck(int &errorTid)
{
    TASK_DESC   taskDesc; 
    bool        stackError;
    int         numIds;
    int         listId[CSI_MAX_TASK_SUMMARY];

    stackError = false;
    errorTid = 0;

    // Get a list of all task IDs
    numIds = taskIdListGet(listId, CSI_MAX_TASK_SUMMARY);

    // Loop through all of the task IDs
    for (int id = 0; id < numIds; id++)
    {
        // Get the task info
        if (taskInfoGet(listId[id], &taskDesc) == OK)
        {
            // If VxWorks stack fill option is not used
            if (taskDesc.td_options & VX_NO_STACK_FILL)
            {
                // Check for stack overrun based on current stack pointer
                if ((taskDesc.td_stackSize - taskDesc.td_stackCurrent) < CSI_STACKCHECK_MARGIN)
                {
                    stackError = true;
                    errorTid = listId[id];
                    break;
                }
            }
            else
            {
                // Check for stack overrun based on high water mark
                if (taskDesc.td_stackMargin < CSI_STACKCHECK_MARGIN)
                {
                    stackError = true;
                    errorTid = listId[id];
                    break;
                }
            }
        }
    }

    return stackError;
}



void CSI::PeriodCheckTask(CSI *csi)
{
    static UINT32 periodCounter = 0;
    for (;;)
    {
        periodCounter++;
        if (periodCounter >= CSI_STACKCHECK_DELAY_IN_SECOND)  // run stack check every minute
        {
            csi->IncrementIdleCountThisPass();   // in case this function runs for some time too long

            if(csi->enabled)
            {
                bool        stackError;
                int         tid;

                stackError = csi->StackCheck(tid);

                // If any stack was overrun
                if (stackError)
                {
                    ESFPPC esf;

                    memset(&esf, 0, sizeof(esf));

                    criticalData.recordType = CSI_STACKCHECK_RECORD;
                    criticalData.resetReason = RESET_REASON_SW_ABNORMAL;

                    esf.pad1  = tid;
                    // Cause an exception (exception handler will log data and reset board)
                    CsiIsrHandler(&esf);
                }
            }
            periodCounter = 0;
        }

        taskDelay(SecondsToTicks(1));
    }
}                   


CSI* CSI::GetInstance(void)
{
    return Instance;
}



int csiRstAddr=0x10100;
void SresetToHreset(void)
{
#ifndef WBBU_CODE
    void (*hrstVec) (void) = (VOIDFUNCPTR)csiRstAddr;

    vxMsrSet(0);

    hrstVec();
#endif
}

void CSI::HookExceptionISRs(void)
{
#ifndef WBBU_CODE
    // Save the current ISR routines to the buffer
    // _EXC_OFF_RES0 not applicable on PPC
    oldVecs.reset       = excVecGet((FUNCPTR *) _EXC_OFF_RESET);
    oldVecs.mach        = excVecGet((FUNCPTR *) _EXC_OFF_MACH);
    oldVecs.data        = excVecGet((FUNCPTR *) _EXC_OFF_DATA);
    oldVecs.inst        = excVecGet((FUNCPTR *) _EXC_OFF_INST);
    oldVecs.align       = excVecGet((FUNCPTR *) _EXC_OFF_ALIGN);
    oldVecs.prog        = excVecGet((FUNCPTR *) _EXC_OFF_PROG);
    oldVecs.fpu         = excVecGet((FUNCPTR *) _EXC_OFF_FPU);
    oldVecs.res1        = excVecGet((FUNCPTR *) _EXC_OFF_RES1);
    oldVecs.res2        = excVecGet((FUNCPTR *) _EXC_OFF_RES2);
    oldVecs.syscall     = excVecGet((FUNCPTR *) _EXC_OFF_SYSCALL);
    oldVecs.trace       = excVecGet((FUNCPTR *) _EXC_OFF_TRACE);
    oldVecs.res3        = excVecGet((FUNCPTR *) _EXC_OFF_RES3);

    oldVecs.pm              = excVecGet((FUNCPTR *) _EXC_NEW_OFF_PERF);  //Used by performance pack.
    oldVecs.inst_bkpt       = excVecGet((FUNCPTR *) _EXC_OFF_INST_BRK);
    oldVecs.smi             = excVecGet((FUNCPTR *) _EXC_OFF_SYS_MNG);


    // Connect CSI ISR to all exceptions 
    // Connect NMI interrupt exception
    excConnect((VOIDFUNCPTR *) _EXC_OFF_RESET,       (VOIDFUNCPTR) SresetToHreset);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_MACH,        (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_DATA,        (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_INST,        (VOIDFUNCPTR) CsiIsrHandler);
    // _EXC_OFF_INTR is used for external interrupts
    excConnect((VOIDFUNCPTR *) _EXC_OFF_ALIGN,       (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_PROG,        (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_FPU,         (VOIDFUNCPTR) CsiIsrHandler);
    // _EXC_OFF_DECR is used for timing
    excConnect((VOIDFUNCPTR *) _EXC_OFF_RES1,        (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_RES2,        (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_SYSCALL,     (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_TRACE,       (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_RES3,        (VOIDFUNCPTR) CsiIsrHandler);
    //excVecGet((FUNCPTR *) _EXC_NEW_OFF_PERF,   (FUNCPTR) CsiIsrHandler);  Used by performance pack.
    excConnect((VOIDFUNCPTR *) _EXC_OFF_INST_BRK,   (VOIDFUNCPTR) CsiIsrHandler);
    excConnect((VOIDFUNCPTR *) _EXC_OFF_SYS_MNG,    (VOIDFUNCPTR) CsiIsrHandler);
#endif
    enabled = true;
}


void CSI::RestoreExceptionISRs()
{ 
#ifndef WBBU_CODE
    excVecSet((FUNCPTR*)_EXC_OFF_RESET,       oldVecs.reset);
    excVecSet((FUNCPTR*)_EXC_OFF_MACH,        oldVecs.mach);
    excVecSet((FUNCPTR*)_EXC_OFF_DATA,        oldVecs.data);
    excVecSet((FUNCPTR*)_EXC_OFF_INST,        oldVecs.inst);
    excVecSet((FUNCPTR*)_EXC_OFF_ALIGN,       oldVecs.align);
    excVecSet((FUNCPTR*)_EXC_OFF_PROG,        oldVecs.prog);
    excVecSet((FUNCPTR*)_EXC_OFF_FPU,         oldVecs.fpu);
    excVecSet((FUNCPTR*)_EXC_OFF_RES1,        oldVecs.res1);
    excVecSet((FUNCPTR*)_EXC_OFF_RES2,        oldVecs.res2);
    excVecSet((FUNCPTR*)_EXC_OFF_SYSCALL,     oldVecs.syscall);
    excVecSet((FUNCPTR*)_EXC_OFF_TRACE,       oldVecs.trace);
    excVecSet((FUNCPTR*)_EXC_OFF_RES3,        oldVecs.res3);

    //excVecGet((FUNCPTR *) _EXC_NEW_OFF_PERF,   oldVecs.pm);  Used by performance pack.
    excVecSet((FUNCPTR *) _EXC_OFF_INST_BRK,   oldVecs.inst_bkpt);
    excVecSet((FUNCPTR *) _EXC_OFF_SYS_MNG,    oldVecs.smi);
#endif  
    enabled = false;
}

void CSI::SaveRebootAlarmInfo(UINT16 almCode, UINT16 almEntity, UINT16 almInst)
{
    criticalData.alarmInfo.code = almCode;
    criticalData.alarmInfo.entity = almEntity;
    criticalData.alarmInfo.instance = almInst;
}


void CSI::SaveRebootRecord()
{
    ESFPPC     esf;
    memset(&esf, 0, sizeof(esf));

    bspSetBtsResetReason(RESET_REASON_SW_NORMAL);

    RESET_REASON  restartCode = bspGetBtsResetReason();  /* save the first record reset type */

    criticalData.resetReason = restartCode;

    #ifdef M_TGT_L3
    if( RESET_REASON_SW_NORMAL == restartCode || RESET_REASON_EMS == restartCode)
    #else
    if( RESET_REASON_SW_NORMAL == restartCode )
    #endif
    {
        criticalData.recordType = CSI_REBOOTHOOK_RECORD;
    }
    else
    {
        criticalData.recordType = CSI_CRITICALALARM_RECORD;
    }

    _CsiIsrHandler(&esf);

}
#ifdef M_TGT_L2
void CSI::AbortRecord(char *fileName, int lineNum)
{
    ESFPPC     esf;
    memset(&esf, 0, sizeof(esf));
    
    bspSetBtsResetReason(RESET_REASON_SW_NORMAL);

    RESET_REASON  restartCode = bspGetBtsResetReason();  /* save the first record reset type */

    criticalData.resetReason = restartCode;
    
   
    criticalData.recordType = CSI_ABORT_RECORD;
  

    int len = strlen(fileName) > sizeof(criticalData.fileName)-1? 
                    sizeof(criticalData.fileName)-1 : strlen(fileName);

    memcpy(&criticalData.fileName[0], fileName, len);
    criticalData.fileName[len] = '\0';
    criticalData.lineNum = lineNum; 
    
    _CsiIsrHandler(&esf);
}
#endif
static char *csiExcString(UINT32 vector)
{
    static char *strings[] = {"",            /* 0x0000 */
                              "RESET ",      /* 0x0100 */
                              "MACH ",       /* 0x0200 */
                              "DATA ",       /* 0x0300 */
                              "INST ",       /* 0x0400 */
                              "INTR ",       /* 0x0500 */
                              "ALIGN ",      /* 0x0600 */
                              "PROG ",       /* 0x0700 */
                              "FPU ",        /* 0x0800 */
                              "DECR ",       /* 0x0900 */
                              "RES1 ",       /* 0x0a00 */
                              "RES2 ",       /* 0x0b00 */
                              "SYSCALL ",    /* 0x0c00 */
                              "TRACE ",      /* 0x0d00 */
                              "RES3 ",       /* 0x0e00 */
                              "PM",          /* 0x0f00 */
                              "INST MISS",   /* 0x1000 */
                              "LOAD MISS ",  /* 0x1100 */
                              "STORE MISS",  /* 0x1200 */
                              "INST BKPT",   /* 0x1300 */
                              "SMI",         /* 0x1400 */
                              "",            /* 0x1500 */
                              "VPA",         /* 0x1600 */
                              "",            /* 0x1700 */
                              "",            /* 0x1800 */
                              "",            /* 0x1900 */
                              "",            /* 0x1a00 */
                              "",            /* 0x1b00 */
                              "",            /* 0x1c00 */
                              "",            /* 0x1d00 */
                              "",            /* 0x1e00 */
                              "VPU",         /* 0x1f00 Relocated */
    };
#ifndef WBBU_CODE
    if (_EXC_NEW_OFF_PERF == vector) vector = _EXC_OFF_PERF;
#endif  
    if ((vector / 0x100) >= SIZEOF(strings)) vector = 0;

    return (strings[(vector / 0x100)]);
}

static char *csiTypeString(CSI_RECORD_TYPE recordType)
{
    static char *strings[] = {"UNKNOWN        ",
                              "CLEAR          ",
                              "START          ",
                              "REBOOT         ",
                              "EXCEPTION      ",
                              "WATCHDOG       ",
                              "STACK CHECK    ",
                              "DEADLOCK       ",
                              "CRITICAL ALARM ",
                              "CRITICAL ASSERT",
                              "MEMORY LEAK    ",
                              "TASK DEAD      ",
							  "ABORT_RECORD   "};

    if ((unsigned int)recordType >= SIZEOF(strings)) recordType = CSI_EMPTY_RECORD;

    return (strings[recordType]);
}

 #ifndef WBBU_CODE
static char *resetReasonString(RESET_REASON resetReason)
{
    static char *resetStr[] = {"HW WATCHDOG",
                               "SW WATCHDOG",
                               "POWER ON   ",
                               "SW NORMAL  ",
                               "SW ALARM   ",
                               "SW ABNORMAL",
                               "BOOTUP FAIL",
                               #ifdef M_TGT_L3
                               "EMS REQUEST",
                               #else
                               "L3 REBOOT  ",
                               #endif
				  "BOOT DOWNFPGA_FAIL",//wangwenhua add 20080801
                               "BOOT EMSTIMEOUT",
                               "BOOT LOADCODETIMEOUT",
                               "BOOT NVRAMDATA_FAIL",
                               "ARPNOT_GATEWAY",
                               "RESET_REASON_PM_CREATE_FAIL",
                                "RESET_REASON_FTPC_CREATE_FAIL",
                               " RESET_REASON_NETMBUFFER_FULL",
                               "RESET_REASON_L3BOOTLINE_DIFF",
                               "RESET_REASON_DSPERR_IN5MIN    "
                             };

    #ifdef M_TGT_L3
    if (resetReason >= RESET_REASON_HW_WDT && resetReason<= RESET_REASON_DSPERR_IN5MIN)
    #else
    if (resetReason >= RESET_REASON_HW_WDT && resetReason<= RESET_REASON_DSPERR_IN5MIN)
    #endif
    {
        return resetStr[resetReason - RESET_REASON_HW_WDT];
    }
    else
    {
        return resetStr[SIZEOF(resetStr)-1];
    }
}
 #else

static char *resetReasonString(RESET_REASON resetReason)
{
    static char *resetStr[] = {"HW WATCHDOG",
                               "SW WATCHDOG",
                               "POWER ON   ",
                               "SW NORMAL  ",
                               "SW ALARM   ",
                               "SW ABNORMAL",
                               "BOOTUP FAIL",
                               #ifdef M_TGT_L3
                               "EMS REQUEST",
                               #else
                               "L3 REBOOT  ",
                               #endif
				  "BOOT DOWNFPGA_FAIL",//wangwenhua add 20080801
                               "BOOT EMSTIMEOUT",
                               "BOOT LOADCODETIMEOUT",
                               "BOOT NVRAMDATA_FAIL",
                               "ARPNOT_GATEWAY",
                               "RESET_REASON_PM_CREATE_FAIL",
                                "RESET_REASON_FTPC_CREATE_FAIL",
                               " RESET_REASON_NETMBUFFER_FULL",
                               "RESET_REASON_L3BOOTLINE_DIFF",
                               "RESET_REASON_L3_IMAGE_ERROR    ",
                              " RESET_REASON_FILESYSTEM",
                               "RESET_REASON_RRU_NOWORKING"};

    #ifdef M_TGT_L3
    if (resetReason >= RESET_REASON_HW_WDT && resetReason<= RESET_REASON_RRU_NOWORKING)
    #else
    if (resetReason >= RESET_REASON_HW_WDT && resetReason<= RESET_REASON_RRU_NOWORKING)
    #endif
    {
        return resetStr[resetReason - RESET_REASON_HW_WDT];
    }
    else
    {
        return resetStr[SIZEOF(resetStr)-1];
    }
}
#endif
/*#define WIND_READY		0x00 
#define WIND_SUSPEND		0x01 
#define WIND_PEND		0x02	
#define WIND_DELAY		0x04	
#define WIND_DEAD		0x08	
*/  // defined in taskLibP.h

static char *csiTaskStatusName(int taskStatus)
{
    static char *strings[] = {"READY   ",
                              "SUSPEND ",
                              "PEND    ",
                              "PEND+S  ",
                              "DELAY   ",
                              "DELAY+S ",
                              "PEND+T  ",
                              "PEND+S+T",
                             };
    static char *unknown =    "UNKNOWN    ";

    if ((unsigned int)taskStatus >= SIZEOF(strings))
        return unknown;
    else
        return (strings[taskStatus]);
}


char* CSI::GetTaskNameFromTid(CSI_CRITICAL_DATA *nvCsi, int tid)
{
    static char* unknown="UNKNOWN";
    char *rc = unknown;
    
    for (UINT32 loopCounter=0; loopCounter<nvCsi->taskCount; loopCounter++)
    {
        if (nvCsi->taskStatus[loopCounter].tid == tid )
        {
            rc = (char *)nvCsi->taskStatus[loopCounter].name;
            break;
        }
    }

    return rc;
}


STATUS CSI::ShowResourceTrace(CSI_CRITICAL_DATA *nvCsi)
{
    for (UINT32 i=0; i<SIZEOF(nvCsi->deadlockTrace); i++)
    {
        if (nvCsi->deadlockTrace[i].resourceTraceDepth == 0)
        {
            break;
        }
        
        fprintf(csiFd, " Resource Trace of Task %s\n", 
                GetTaskNameFromTid(nvCsi, nvCsi->deadlockTrace[i].resourceTrace[0].tid));
        fprintf(csiFd, " level    taskName     PendReason   ResourceId  HoldingTask\n");
        fprintf(csiFd, " -----  ------------  ------------ ------------ -----------\n");

        for (UINT32 j=0; j<nvCsi->deadlockTrace[i].resourceTraceDepth; j++)
        {
            switch (nvCsi->deadlockTrace[i].resourceTrace[j].reason)
            {
                case CSI_TASK_PEND_NONE:
                    fprintf(csiFd, " %05d  %-11s    NOT PENDING\n", j+1,
                            GetTaskNameFromTid(nvCsi,nvCsi->deadlockTrace[i].resourceTrace[j].tid));
                    break;
                case CSI_TASK_PEND_MSG:
                    fprintf(csiFd, " %05d  %-11s    Msg Queue      %08x\n", j,
                            GetTaskNameFromTid(nvCsi,nvCsi->deadlockTrace[i].resourceTrace[j].tid),
                            nvCsi->deadlockTrace[i].resourceTrace[j].pendResourceId
                            );
                    break;
                case CSI_TASK_PEND_SEM:
                    fprintf(csiFd, " %05d  %-11s    SEMAPHORE      %08x  %-11s\n", j,
                            GetTaskNameFromTid(nvCsi,nvCsi->deadlockTrace[i].resourceTrace[j].tid),
                            nvCsi->deadlockTrace[i].resourceTrace[j].pendResourceId,
                            GetTaskNameFromTid(nvCsi,nvCsi->deadlockTrace[i].resourceTrace[j].ResourceHoldingTid)
                            );
                    break;
                case CSI_TASK_PEND_EVENT:
                    fprintf(csiFd, " %05d  %-11s    Event          %08x\n", j,
                            GetTaskNameFromTid(nvCsi,nvCsi->deadlockTrace[i].resourceTrace[j].tid),
                            nvCsi->deadlockTrace[i].resourceTrace[j].pendResourceId
                            );
                    break;
                case CSI_TASK_PEND_SELECT:
                    fprintf(csiFd, " %05d  %-11s    SELECT         %08x\n", j,
                            GetTaskNameFromTid(nvCsi,nvCsi->deadlockTrace[i].resourceTrace[j].tid),
                            nvCsi->deadlockTrace[i].resourceTrace[j].pendResourceId
                            );
                    break;
                default:
                    fprintf(csiFd, " %05d  %-11s    UNKNOWN\n", j, 
                            GetTaskNameFromTid(nvCsi,nvCsi->deadlockTrace[i].resourceTrace[j].tid));
                    break;
            }
        }
    }
    return(OK);
}

STATUS CSI::ShowCsiRecord(unsigned int record)
{
    unsigned int        i, j, n;
    unsigned int        task;
    UINT32              min = record;
    UINT32              max = record;
    CSI_TASK_RECORD    *taskData;
    CSI_IMAGE          *nvImage = (CSI_IMAGE *)CSI_IMAGE_ADDRESS;;
    CSI_NVRAM_HEADER   *nvHdr   = &nvImage->nvHdr;
    CSI_CRITICAL_DATA  *nvCsi;
    #ifdef M_TGT_L3
    CSI_IMAGE_NEW *nvImage_new = (CSI_IMAGE_NEW*)CSI_IMAGE_ADDRESS_NEW;
    #endif

    if (   (0 != strncmp((char *)&nvHdr->nvRamSafePattern[0], (char *)CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN)) ) 
        || (nvHdr->version != CSI_NVRAM_LATEST_VERSION) 
        || (nvHdr->nextCsiRecordIndex > SIZEOF(nvImage->nvCsi)) 
        || (nvHdr->nextLogRecordIndex > SIZEOF(nvImage->nvLog))
       )
    {
      // ApAssertRtnV(false, LOG_SEVERE, LOG_ERR_CSI_INVALID_IMAGE, "CSI NVRAM not initialized!", ;, ERROR);
    }

    if (record == (unsigned int)CSI_NVRAM_ALL_CSI_RECORDS)
    {
        min = 0;
        max = SIZEOF(nvImage->nvCsi) - 1;
    }
    else if(record == CSI_NVRAM_LAST_CSI_RECORD)
    {
        min = max =  ((nvHdr->nextCsiRecordIndex == 0) ? SIZEOF(nvImage->nvCsi) - 1 : (nvHdr->nextCsiRecordIndex - 1));
    }
    else if(record <= SIZEOF(nvImage->nvCsi))
    {
        min = max = record - 1;
    }
    else
    {
//        ApAssertRtnV(false, LOG_SEVERE, LOG_ERR_CSI_INVALID_RECORD, "Invalid Record ID!", ;,ERROR);
    }

    for (i = min; i <= max; i++)
    {
        nvCsi = &nvImage->nvCsi[i];

        fprintf(csiFd, "\n");
        fprintf(csiFd, "RECORD %d", i + 1);

        if (!nvCsi->valid)
        {
            fprintf(csiFd, "   (EMPTY)\n");
            fprintf(csiFd, "\n");
        }
        else
        {
            fprintf(csiFd, "\n");
            fprintf(csiFd, "\n");
            fprintf(csiFd, "  CSI Version: %d\n", nvCsi->version);

            fprintf(csiFd, "  CSI Timestamp: %02d/%02d/%04d %02d:%02d:%02d\n",
                   nvCsi->rtc.month, nvCsi->rtc.day, nvCsi->rtc.year,
                   nvCsi->rtc.hour, nvCsi->rtc.minute, nvCsi->rtc.second);
            fprintf(csiFd, "  CSI Record Type: %s%s\n",
                   ((nvCsi->recordType == CSI_EXCEPTION_RECORD) ? 
                     csiExcString(nvCsi->troubleTask[0].esf.vecOffset) : ""),
                   csiTypeString(nvCsi->recordType));
            fprintf(csiFd, "  CSI Reset Reason: %d\n", nvCsi->resetReason);
            fprintf(csiFd, "\n");
            
            fprintf(csiFd, "  BSP Version: %s\n", nvCsi->loadVersion);
            fprintf(csiFd, "\n");

            taskData = nvCsi->troubleTask;
            switch (nvCsi->recordType)
            {
                case  CSI_CRITICALALARM_RECORD:
                    
                    fprintf(csiFd, "  Critical Alarm Code(%04x)\n", nvCsi->alarmInfo.code);
                    fprintf(csiFd, "           Alarm Entiry(%04d)\n", nvCsi->alarmInfo.entity);
                    fprintf(csiFd, "           Alarm Instance(%04d)\n", nvCsi->alarmInfo.instance);
                    break;

                case CSI_CRITICALASSERTION_RECORD:
                    fprintf(csiFd, "  Critical Assertion Code(%04x)\n", nvCsi->alarmInfo.code & ~0x4000);
                    if (taskData[0].task.tid != 0)
                    {
                        fprintf(csiFd, "\n  Task Info:\n");
                        fprintf(csiFd, "  Tid=0x%08x, TaskEntry=0x%08x Name=%-15s\n", taskData[0].task.tid, taskData[0].task.pc, taskData[0].task.name);
                        fprintf(csiFd, "\n");
                    }
                    break;
                
                case CSI_ABORT_RECORD:
                    #ifdef M_TGT_L2
                    fprintf(csiFd, "  %s %d lines cause abort reboot\n", nvCsi->fileName, nvCsi->lineNum);
                     // fall through to next case
                    #else                    
                    fprintf(csiFd, "  %s %d lines cause abort reboot\n", nvImage_new->csi_record_new[i].fileName,\
                        nvImage_new->csi_record_new[i].lineNum);
                     // fall through to next case, no break;
                    #endif
                default:
                    task = 0;
                    for (task = 0; task < SIZEOF(nvCsi->troubleTask); task++)
                    {
                        if (taskData[task].task.tid != 0)
                        {
                            fprintf(csiFd, "\n  Trouble Task Info:\n");
                            fprintf(csiFd, "  Tid=0x%08x, TaskEntry=0x%08x Name=%-15s\n", taskData[task].task.tid, taskData[task].task.pc, taskData[task].task.name);
                            fprintf(csiFd, "\n");

                            fprintf(csiFd, "  PPC Register Data:\n");
                            fprintf(csiFd, "\n");
                            fprintf(csiFd, "  vec   =0x%08x\n", taskData[task].esf.vecOffset);
                            fprintf(csiFd, "  errno =0x%08x\n", taskData[task].esf._errno);
#ifndef WBBU_CODE
                            fprintf(csiFd, "  dar   =0x%08x\n", taskData[task].esf.dar);
                            fprintf(csiFd, "  dsisr =0x%08x\n", taskData[task].esf.dsisr);
                            fprintf(csiFd, "  fpcsr =0x%08x\n", taskData[task].esf.fpcsr);
#endif
                            for (j = 0; j < GREG_NUM; j++)
                            {
                                if ((j % 4) == 0) fprintf(csiFd, "\n");
                                fprintf(csiFd, "  gpr%-2d =0x%08x", j, taskData[task].esf.regSet.gpr[j]);
                            }
                            fprintf(csiFd, "\n");
                            fprintf(csiFd, "\n");
                            fprintf(csiFd, "  msr   =0x%08x\n", taskData[task].esf.regSet.msr);
                            fprintf(csiFd, "  lr    =0x%08x\n", taskData[task].esf.regSet.lr);
                            fprintf(csiFd, "  ctr   =0x%08x\n", taskData[task].esf.regSet.ctr);
                            fprintf(csiFd, "  pc    =0x%08x\n", taskData[task].esf.regSet.pc);
                            if (nvCsi->recordType != CSI_DEADLOCK_RECORD)
                                fprintf(csiFd, "  sp    =0x%08x\n", taskData[task].esf.pad1);
                            fprintf(csiFd, "  cr    =0x%08x\n", taskData[task].esf.regSet.cr);
                            fprintf(csiFd, "  xer   =0x%08x\n", taskData[task].esf.regSet.xer);
                            fprintf(csiFd, "\n");

                        }
                        else
                        {
                            break;
                        }
                    }

                    /////////////////////////////
                    // print memory heap status 
                    fprintf(csiFd, "  Heap Stats:\n");
                    fprintf(csiFd, "  ------------------\n");
                    if (nvCsi->memHeapStatsSafePattern != CSI_MEM_STATS_SAFE_PATTERN)
                    {
                        fprintf(csiFd, "  Not available\n\n\n");
                    }
                    else
                    {
                        fprintf(csiFd, "  numBytesFree = %d\n", nvCsi->memHeapStats.numBytesFree);
                        fprintf(csiFd, "  numBlocksFree = %d\n", nvCsi->memHeapStats.numBlocksFree);
                        fprintf(csiFd, "  maxBlockSizeFree = %d\n", nvCsi->memHeapStats.maxBlockSizeFree);
                        fprintf(csiFd, "  numBytesAlloc = %d\n", nvCsi->memHeapStats.numBytesAlloc);
                        fprintf(csiFd, "  numBlocksAlloc = %d\n\n\n\n", nvCsi->memHeapStats.numBlocksAlloc);
                    }

                    #ifdef M_TGT_L3
                    fprintf(csiFd,"    %10s %15s %10s %10s %10s\n","[taskID]", "[taskName]", "[alloc]", "[free]", "[hold]");
                    fprintf(csiFd,"    -----------------------------------------------------------\n");
                    for (int idx = 0; idx < M_MAX_TASK_NUM; ++idx)
                    {
                        int tid = nvCsi->MemoryUsage[idx].taskID;
                        if (0 != tid)
                        {
                            fprintf(csiFd,"    %10x %15s %10d %10d %10d\n", tid, GetTaskNameFromTid(nvCsi,tid), 
                                nvCsi->MemoryUsage[idx].allocate, 
                                nvCsi->MemoryUsage[idx].free, 
                                nvCsi->MemoryUsage[idx].allocate - nvCsi->MemoryUsage[idx].free);
                        }
                    }
                    #endif


                    // print raw stack data
                    if(  (nvCsi->recordType == CSI_EXCEPTION_RECORD)
                         || (nvCsi->recordType == CSI_WATCHDOGTASK_RECORD )
                         || (nvCsi->recordType == CSI_STACKCHECK_RECORD) 
                         || (nvCsi->recordType == CSI_TASK_DEAD_RECORD) 
                         )
                   {
                        UINT32    sp = nvCsi->troubleTask[0].esf.pad1;

                        const int    rowSize = 4;
                        unsigned int columnSize = SIZEOF(nvCsi->rawStack) / rowSize;

                        fprintf(csiFd, "  Stack Data:\n\n");
                        fprintf(csiFd, "  ------------------  ------------------  ------------------  ------------------");
                        for (j = 0; j < columnSize; j++)
                        {
                            fprintf(csiFd, "\n");

                            fprintf(csiFd, "  %08x: %08x  %08x: %08x  %08x: %08x  %08x: %08x",

                                    sp +          ((SIZEOF(nvCsi->rawStack) - (0*columnSize + j + 1)) * sizeof(sp)),
                                    nvCsi->rawStack[SIZEOF(nvCsi->rawStack) - (0*columnSize + j) - 1],

                                    sp +          ((SIZEOF(nvCsi->rawStack) - (1*columnSize + j + 1)) * sizeof(sp)),
                                    nvCsi->rawStack[SIZEOF(nvCsi->rawStack) - (1*columnSize + j) - 1],

                                    sp +          ((SIZEOF(nvCsi->rawStack) - (2*columnSize + j + 1)) * sizeof(sp)),
                                    nvCsi->rawStack[SIZEOF(nvCsi->rawStack) - (2*columnSize + j) - 1],

                                    sp +          ((SIZEOF(nvCsi->rawStack) - (3*columnSize + j + 1)) * sizeof(sp)),
                                    nvCsi->rawStack[SIZEOF(nvCsi->rawStack) - (3*columnSize + j) - 1]
                                    );
                        }

                        fprintf(csiFd, "<-sp\n\n\n");
                    }


                    break;
            }

            fprintf(csiFd, "  Context Switch Data:\n");
            fprintf(csiFd, "\n");
            fprintf(csiFd, "           OLD-TASK    OLD-TID      OLD-PC       NEW-TASK      NEW-TID      NEW-PC\n");  
            fprintf(csiFd, "      --------------- ----------  ---------- --------------- ----------  ----------\n");
            for (j = 0, n = nvCsi->contextSwitchHistory.contextCount; 
                 j < SIZEOF(nvCsi->contextSwitchHistory.newContext); 
                 j++, n = ((n == (SIZEOF(nvCsi->contextSwitchHistory.newContext) - 1)) ? 0 : (n + 1)))
            {
                char *oldname = GetTaskNameFromTid(nvCsi, nvCsi->contextSwitchHistory .oldContext [n].tid);
                char *newname = GetTaskNameFromTid(nvCsi, nvCsi->contextSwitchHistory .newContext [n].tid);
                fprintf(csiFd, "  %02d: %-15s 0x%08x  0x%08x %-15s 0x%08x  0x%08x\n", (j+1),
                       oldname, nvCsi->contextSwitchHistory.oldContext[n].tid, 
                        nvCsi->contextSwitchHistory.oldContext[n].pc,
                       newname, nvCsi->contextSwitchHistory.newContext[n].tid, 
                        nvCsi->contextSwitchHistory.newContext[n].pc
                       );
            }
            fprintf(csiFd, "\n");

            //////////////////////////////////////////////////////
            // print task status
            if (nvCsi->taskCount > 0)
            {
                int entryId = 0;

                fprintf(csiFd, "  Task Info:\n");
                fprintf(csiFd, "\n");
                fprintf(csiFd, "        TASK          TID      Pri   Status      PC      ERRNO       SP       BASE     SIZE    HIGH   MARGIN\n");
                fprintf(csiFd, "      ------------ ----------  ---  --------  --------  --------  --------  --------  ------  ------  ------\n");

                for(unsigned int pri = 0; pri < 0x100; pri++)
                {
                    for (j = 0; ((j < nvCsi->taskCount) && (j<SIZEOF(nvCsi->taskStatus))); j++)
                    {
                        if(pri == nvCsi->taskStatus[j].priority)
                        {
                            fprintf(csiFd, "  %02d: %-12s 0x%08x  %3u  %-8s  %08x  %08x  %08x  %08x  %6u  %6u  %6u", 
                                   ++entryId, 
                                   nvCsi->taskStatus[j].name,
                                   nvCsi->taskStatus[j].tid, 
                                   nvCsi->taskStatus[j].priority, 
                                   csiTaskStatusName(nvCsi->taskStatus[j].taskStatus),
                                   nvCsi->taskStatus[j].pc,
                                   nvCsi->taskStatus[j].errorStatus,
                                   nvCsi->taskStatus[j].sp,
                                   nvCsi->taskStatus[j].base,
                                   nvCsi->taskStatus[j].size, 
                                   nvCsi->taskStatus[j].high, 
                                   nvCsi->taskStatus[j].margin);

                            if(nvCsi->taskStatus[j].margin < (CSI_STACKCHECK_MARGIN * 2))
                            {
                                fprintf(csiFd, "******\n");
                            }
                            else
                            {
                                fprintf(csiFd, "\n");
                            }
                        }
                    }
                }
                fprintf(csiFd, "\n");
            }

            /////////////////////////////
            // print task stack trace information
            if (nvCsi->traceTaskCount > 0)
            {
                for (task = 0; ((task < nvCsi->traceTaskCount) && (task<SIZEOF(nvCsi->taskTrace))); task++)
                {
                    char *name = GetTaskNameFromTid(nvCsi, nvCsi->taskTrace[task].tid);

                    fprintf(csiFd, "\n  %s (tid 0x%x) Stack Traceback Data:\n", name, nvCsi->taskTrace[task].tid);
                    fprintf(csiFd, "\n");
                    fprintf(csiFd, "                            Return       Entry\n");
                    fprintf(csiFd, "                           --------     --------\n");

                    for (j = 0; ((j < nvCsi->taskTrace[task].traceCount) && 
                                  (j < SIZEOF(nvCsi->taskTrace[task].trace))); j++)
                    {
                        if(0 == j) fprintf(csiFd, "                     top->"); 
                        else  fprintf(csiFd,      "\n                          ");
                        fprintf(csiFd, "0x%08x : 0x%08x", 
                                nvCsi->taskTrace[task].trace[j].addr1, 
                                nvCsi->taskTrace[task].trace[j].addr2);
                    }
                    if(nvCsi->taskTrace[task].traceCount
                         < (SIZEOF(nvCsi->taskTrace[task].trace) - 1))
                    {
                        fprintf(csiFd, "<-current\n\n\n");
                    }
                    else
                    {
                        fprintf(csiFd, "\n\n");
                    }
                }
            }

            if (nvCsi->recordType == CSI_DEADLOCK_RECORD)
            {
                ShowResourceTrace(nvCsi);
            }

        }
    }

    return(OK);
}

STATUS CSI::ShowLog()
{
    unsigned int        i;
    CSI_IMAGE          *nvImage = (CSI_IMAGE *)CSI_IMAGE_ADDRESS;
    CSI_NVRAM_HEADER   *nvHdr   = &nvImage->nvHdr;
    CSI_NVRAM_LOG      *nvLog;

    if (   (0 != strncmp((char *)&nvHdr->nvRamSafePattern[0], (char *)CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN)) ) 
        || (nvHdr->version != CSI_NVRAM_LATEST_VERSION) 
        || (nvHdr->nextCsiRecordIndex > SIZEOF(nvImage->nvCsi)) 
        || (nvHdr->nextLogRecordIndex > SIZEOF(nvImage->nvLog))
       )
    {
//        ApAssertRtnV(false, LOG_SEVERE, LOG_ERR_CSI_INVALID_IMAGE, "CSI NVRAM not initialized!", ;,ERROR);
    }

    fprintf(csiFd, "\n");
    fprintf(csiFd, "Rec      Type         CSI  ResetReason      Timestamp                             BSP Version\n");
    fprintf(csiFd, "---  ---------------  ---  -----------  -------------------  -----------------------------------------------------\n");

    for (i = 0; i < SIZEOF(nvImage->nvLog); i++)
    {
        nvLog = &nvImage->nvLog[i];

        if (nvLog->recordType == CSI_EMPTY_RECORD)
        {
            fprintf(csiFd, "%03d  (EMPTY)\n", i+1);
        }
        else
        {
            fprintf(csiFd, "%03d  %s  (%d)  %s  %02d/%02d/%04d %02d:%02d:%02d  %s\n", i + 1, csiTypeString(nvLog->recordType),
                   nvLog->csiRecordIndex,
                   resetReasonString(nvLog->resetReason),
                   nvLog->timeStamp.month, nvLog->timeStamp.day, nvLog->timeStamp.year,
                   nvLog->timeStamp.hour, nvLog->timeStamp.minute, nvLog->timeStamp.second,
                   nvLog->loadVersion);
        }
    }

    fprintf(csiFd, "\n");

    return OK;
}

STATUS CSI::ShowHdrAndCsiRecordSummary()
{
    unsigned int        i;
    unsigned int        count;
    CSI_IMAGE          *nvImage = (CSI_IMAGE *)CSI_IMAGE_ADDRESS;
    CSI_NVRAM_HEADER   *nvHdr   = &nvImage->nvHdr;
    CSI_CRITICAL_DATA  *nvCsi;

    if (   (0 != strncmp((char *)&nvHdr->nvRamSafePattern[0], (char *)CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN)) ) 
        || (nvHdr->version != CSI_NVRAM_LATEST_VERSION) 
        || (nvHdr->nextCsiRecordIndex > SIZEOF(nvImage->nvCsi)) 
        || (nvHdr->nextLogRecordIndex > SIZEOF(nvImage->nvLog))
       )
    {
      //  ApAssertRtnV(false, LOG_SEVERE, LOG_ERR_CSI_INVALID_IMAGE, "CSI NVRAM not initialized!",;,ERROR);
    }

    fprintf(csiFd, "\n");
    #ifdef M_TGT_L3
    fprintf(csiFd, "    BTS %d CSI L3 Stats \n", nvHdr->btsID);
    #else
    fprintf(csiFd, "       L2 CSI Stats\n");
    #endif
    fprintf(csiFd, "===================================\n");
    fprintf(csiFd, "Init Timestamp: %02d/%02d/%04d %02d:%02d:%02d\n",
           nvHdr->initTimeStamp.month, nvHdr->initTimeStamp.day, nvHdr->initTimeStamp.year,
           nvHdr->initTimeStamp.hour, nvHdr->initTimeStamp.minute, nvHdr->initTimeStamp.second);
    fprintf(csiFd, "Boot Timestamp: %02d/%02d/%04d %02d:%02d:%02d\n",
           nvHdr->bootTimeStamp.month, nvHdr->bootTimeStamp.day, nvHdr->bootTimeStamp.year,
           nvHdr->bootTimeStamp.hour, nvHdr->bootTimeStamp.minute, nvHdr->bootTimeStamp.second);
    fprintf(csiFd, "\n");
    fprintf(csiFd, "Format Version:  %10u\n", nvHdr->version);
    fprintf(csiFd, "Start Count:     %10u\n", nvHdr->startCount);
    fprintf(csiFd, "Reboot Count:    %10u\n", nvHdr->rebootCount);
    fprintf(csiFd, "Crash Count:     %10u\n", nvHdr->crashCount);
    fprintf(csiFd, "Next CSI Record: %10u\n", nvHdr->nextCsiRecordIndex + 1);
    fprintf(csiFd, "Next Log Record: %10u\n", nvHdr->nextLogRecordIndex + 1);


    for (i = 0; i < SIZEOF(nvImage->nvCsi); i++)
    {
        nvCsi = &nvImage->nvCsi[i];
        fprintf(csiFd, "\n");

        if (!nvCsi->valid)
        {
            fprintf(csiFd, "Record #%d  (EMPTY)\n", i + 1);
        }
        else
        {
            fprintf(csiFd, "Record #%d  %02d/%02d/%04d %02d:%02d:%02d\n", i + 1, nvCsi->rtc.month,
                   nvCsi->rtc.day, nvCsi->rtc.year, nvCsi->rtc.hour, nvCsi->rtc.minute, nvCsi->rtc.second);
            fprintf(csiFd, "  Record Type: ");
            switch(nvCsi->recordType)
            {
            case CSI_EXCEPTION_RECORD:
#ifndef WBBU_CODE
                fprintf(csiFd, "%sEXCEPTION", csiExcString(nvCsi->troubleTask[0].esf.vecOffset));
                if ((nvCsi->troubleTask[0].esf.vecOffset == _EXC_OFF_MACH) ||
                    (nvCsi->troubleTask[0].esf.vecOffset == _EXC_OFF_DATA) ||
                    (nvCsi->troubleTask[0].esf.vecOffset == _EXC_OFF_ALIGN))
                {
                    fprintf(csiFd, " dar=0x%08x\n", nvCsi->troubleTask[0].esf.dar);
                }
                else
                {
                    fprintf(csiFd, "\n");
                }
#endif
                fprintf(csiFd, "  Task Name: %s\n", nvCsi->troubleTask[0].task.name);
                break;
            case CSI_WATCHDOGTASK_RECORD:
                fprintf(csiFd, "WATCHDOG\n");
                count = nvCsi->contextSwitchHistory.contextCount;
                if (count < SIZEOF(nvCsi->contextSwitchHistory.newContext))
                {
                    fprintf(csiFd, "  Task Name: %s\n", nvCsi->troubleTask[0].task.name);
                }
                break;
            case CSI_STACKCHECK_RECORD:
                fprintf(csiFd, "STACKCHECK\n");
                fprintf(csiFd, "  Task Name: %s\n", nvCsi->troubleTask[0].task.name);
                break;
            case CSI_REBOOTHOOK_RECORD:
                fprintf(csiFd, "REBOOT\n");
                break;
            case CSI_DEADLOCK_RECORD:
                fprintf(csiFd, "DEADLOCK\n");
                break;
            case CSI_CRITICALALARM_RECORD:
                fprintf(csiFd, "CRITICAL ALARM[%x]\n", nvCsi->alarmInfo.code);
                break;
            case CSI_CRITICALASSERTION_RECORD:
                fprintf(csiFd, "CRITICAL ASSERTION[%04x]\n", nvCsi->alarmInfo.code & ~0x4000);
                break;
            case CSI_SW_MEMORY_LEAK:
                fprintf(csiFd, "MEMORY LEAK\n");
                break;
//for task dead record
            case CSI_TASK_DEAD_RECORD:
                fprintf(csiFd, "TASK DEAD\n");
                break;
            case CSI_ABORT_RECORD:
                fprintf(csiFd, "APP ABORT\n");
                break;            
            default:
                fprintf(csiFd, "UNKNOWN\n");
                break;
            }
        }
    }

    fprintf(csiFd, "===================================\n");
    fprintf(csiFd, "\n");

    return(OK);
}


STATUS CSI::ClearImage()
{

    T_TimeDate         bootTime;
    CSI_IMAGE          *nvImage = (CSI_IMAGE *)CSI_IMAGE_ADDRESS;
    CSI_NVRAM_HEADER   *nvHdr   = &nvImage->nvHdr;

    // Get current time and save boot time
    T_TimeDate currTime = bspGetDateTime();
    memcpy((char *)&bootTime, (char *)&nvHdr->bootTimeStamp, sizeof(bootTime));

    // Reinitialize CSI image as a critical section
    // Clear the whole area and set the default initial values
    InitImage(nvImage, &currTime);

    // Initialize and update header.
    if(bspEnableNvRamWrite((char *)nvHdr, sizeof(*nvHdr))==TRUE)
    {
        memcpy((char *)&nvHdr->bootTimeStamp, (char *)&bootTime, sizeof(nvHdr->bootTimeStamp));
        nvHdr->startCount += 1;
        bspDisableNvRamWrite((char *)nvHdr, sizeof(*nvHdr));
    }
    // Initialize current log record information with a special START event
    CSI_NVRAM_LOG* nvLog     = GetNextLogRecord();
    char verStr[100];
    if(bspEnableNvRamWrite((char *)nvLog, sizeof(*nvLog))==TRUE)
    {
        memcpy((char *)&nvLog->timeStamp, (char *)&criticalData.rtc, sizeof(nvLog->timeStamp)); 
        sprintf(verStr, "%s [%s, %s]", VERSION, __DATE__, __TIME__ );
        memcpy(&nvLog->loadVersion[0], &verStr, sizeof(nvLog->loadVersion)-1);
        nvLog->loadVersion[sizeof(nvLog->loadVersion)-1] = 0;
    
        nvLog->loadVersion[sizeof(nvLog->loadVersion)-1] = 0;
        nvLog->csiRecordIndex = 0;
        nvLog->resetReason        = bspGetBtsResetReason();
        nvLog->recordType = CSI_START_RECORD;
        bspDisableNvRamWrite((char *)nvLog, sizeof(*nvLog));
    }

    // Initialize current log record information with a special CLEAR event
    nvLog     = GetNextLogRecord();
    if(bspEnableNvRamWrite((char *)nvLog, sizeof(*nvLog))==TRUE)
    {
        memcpy((char *)&nvLog->timeStamp, (char *)&currTime, sizeof(nvLog->timeStamp));
        memcpy(&nvLog->loadVersion[0], &verStr, sizeof(nvLog->loadVersion)-1);
        nvLog->loadVersion[sizeof(nvLog->loadVersion)-1] = 0;
    
        nvLog->csiRecordIndex = 0;
        nvLog->recordType = CSI_CLEAR_RECORD;
        nvLog->resetReason = bspGetBtsResetReason();
        bspDisableNvRamWrite((char *)nvHdr, sizeof(*nvHdr));
    }
    return(OK);
}

STATUS CSI::ShowDeadlockTaskList()
{
    int   tid;

    fprintf(csiFd, "\n");
    fprintf(csiFd, "  CSI Deadlock Task IDs\n");
    fprintf(csiFd, "=========================\n");

    for (unsigned int i = 0; i < SIZEOF(DeadlockMonTaskInfo); i++)
    {
        tid = (int)DeadlockMonTaskInfo[i].deadlockTid;

        if (tid != 0)
        {
            fprintf(csiFd, "0x%08x  %s\n", tid, taskName(tid));
        }
    }

    fprintf(csiFd, "\n");
    return(OK);
}


void CSI::ShowDeadlockTaskInfo()
{
    printf("\n");
    printf("  CSI Deadlock Task Info\n");
    printf("=========================\n");

    int tid;
    for (unsigned int i = 0; i < SIZEOF(DeadlockMonTaskInfo); i++)
    {
        tid = (int)DeadlockMonTaskInfo[i].deadlockTid;

        if (tid != 0)
        {
            printf("tid = 0x%08x  %s, maxBlockedTime = %d, remainBlockedTime = %d, switchCount= %d\n", 
                   tid, taskName(tid), DeadlockMonTaskInfo[i].maxBlockedTime,
                   DeadlockMonTaskInfo[i].remainBlockedTime, 
                   DeadlockMonTaskInfo[i].switchCount);
        }
    }
}

extern "C" STATUS csiShowDeadlock()
{
    CSI* csiObj = CSI::GetInstance();
    if (csiObj)
    {
        csiObj->ShowDeadlockTaskInfo();
    }
    return OK;
}

extern "C" STATUS csiHelp(void)
{
    if(!csiFd) csiFd = stdout;

    printf("csiClear                    - Erases all CSI logs and starts from scrath.\n");
    printf("\n");
    printf("csiDisable                  - Disables all CSI protection mechanisms. The BTS is not protected from faults.\n");
    printf("\n");
    printf("csiEnable <verbose>         - Reenables the CSI protection mechanisms after being disabled.\n");
    printf("                            - <verbose> ['0' | blank]  - Standard, non-verbose logging. No run time impact.\n");
    printf("                            - <verbose> ['1']          - Verbose logging. May encounter run time performance impacts.\n");
    printf("\n");
    printf("csiShow <crashId>           - This command provides a full interpretation from each of the\n");
    printf("                              CSI data stores. The entry in the verbose crash data store can be\n");
    printf("                              specified through the <crashId> parameter. The display results are\n");
    printf("                              also automatically copied to the file '/dev0/bts-csi.log'.\n");
    printf("                            - <crashId> ['-1' | 'all'] - Display all verbose crash records.\n");
    printf("                            - <crashId> ['0' | blank]  - Display most recent crash record.\n");
    printf("                            - <crashId> [> '1']        - Display specified crash record.\n");
    printf("\n");
    printf("csiUpload                   - Captures a binary copy of the entire CSI core with configuration to\n");
    printf("                              the log directory '/dev0/log'.\n");
    return OK;
}

extern "C" STATUS csi(void)
{
    if(!csiFd) csiFd = stdout;
    return csiHelp();
}

extern "C" STATUS csiClear(void)
{
    CSI* csiObj = CSI::GetInstance();
    if (csiObj)
    {
        if(!csiFd) csiFd = stdout;
        return CSI::GetInstance()->ClearImage();
    }
    else
    {
        printf( " CSI not started \n");
    }
}

extern "C" STATUS csiDisable(void)
{
    CSI* csiObj = CSI::GetInstance();
    if (csiObj)
    {
        if(!csiFd) csiFd = stdout;
        CSI::GetInstance()->RestoreExceptionISRs();
    }
    else
    {
        printf( " CSI not started \n");
    }
    return OK;
}


extern "C" STATUS showIdleCounters(void)
{
    #ifdef M_TGT_L3
    CSI* csiObj = CSI::GetInstance();
    if (csiObj)
    {
        if(!csiFd) csiFd = stdout;
        printf("CPU Occupancy Average = %2.2f Percent\n", (1-(((float)(CSI::GetInstance()->GetIdleCounterAverage()))/CSI::GetInstance()->GetIdleCounterMax()))*100);
        printf("CPU Occupancy LWM     = %2.2f Percent\n", (1-(((float)(CSI::GetInstance()->GetIdleCounterHigh()))/CSI::GetInstance()->GetIdleCounterMax()))*100);
        printf("CPU Occupancy HWM     = %2.2f Percent\n", (1-(((float)(CSI::GetInstance()->GetIdleCounterLow()))/CSI::GetInstance()->GetIdleCounterMax()))*100);

        printf("Idle Counter Average  = %u\n", CSI::GetInstance()->GetIdleCounterAverage());
        printf("High Water Mark       = %u\n", CSI::GetInstance()->GetIdleCounterHigh());
        printf("Low Water Mark        = %u\n", CSI::GetInstance()->GetIdleCounterLow());
        printf("Idle Counter Max      = %u\n", CSI::GetInstance()->GetIdleCounterMax());
    }
    else
    {
        printf("CSI not started\n");
    }
    #else
    printf("This feature is not provided for L2 system\n");
    #endif
	return OK;
}

extern "C" STATUS csiEnable(void)
{
    CSI* csiObj = CSI::GetInstance();
    if (csiObj)
    {
        if (!csiIsEnabled())
        {
            csiObj->HookExceptionISRs();
        }
        return OK;
    }
    else
    {
        return csiInit();
    }
}

extern "C" BOOL csiIsEnabled(void)
{
    return (BOOL)CSI::GetInstance()->GetStatus();
}

#ifndef M_TGT_L3
extern "C" int uploadCSIfileToL3(const char *filename)
{
    //copy to l3 csi directory. dstfile = "/FTPSERVER/RAMD/csi/l2-csi.log"
    struct stat filestat;
    char dstfile[40] = {0};
    char disp[200];

    /* stat the file and figure out it's size */  
    for (char i=0; i<4; i++)
    {
        if (stat((char *)filename, &filestat) != 0)
        {
            sprintf(disp, "unable to open target binary `%s' for getting file size.\n", (int)filename);
            LOG(LOG_CRITICAL, 0, disp);     
            taskDelay(10);
        }
        else
        {
            break;
        } 
    }
    /* open the program for reading */
    FILE *readfp = fopen(filename, "r");
    if (!readfp)
    {
        sprintf(disp, "unable to open target binary `%s' for reading", (int)filename);
        LOG(LOG_CRITICAL, 0, disp);
        return -1;
    }
    /* about 1k more than reqd is malloced to read file into memory */
    char *readbuf = (char *) malloc((filestat.st_size + 0x200) & ~(0x200));
    if(readbuf == NULL)
    {
        fclose(readfp);
        sprintf(disp, "readbuf = NULL `%s'", (int)filename);
        LOG(LOG_CRITICAL, 0, disp);
        return -1;
    }
    /* load the program into memory */
    if (fread(readbuf, 1, filestat.st_size, readfp) < 1)
    {
        fclose(readfp);
        free(readbuf);
        sprintf(disp, "unable to read from target binary `%s'", (int)filename);
        LOG(LOG_CRITICAL, 0, disp);
        return -1;
    }

    /* done with the executable, close it */
    if (fclose(readfp))
    {
        free(readbuf);
        sprintf(disp, "unable to close target binary '%s'", (int)filename);
        LOG(LOG_CRITICAL, 0, disp );
        return -1;
    }

    strcat(dstfile, "/FTP:/RAMD/csi/l2-csi.log");

    FILE *writefp = fopen(dstfile, "w+");
    if (!writefp)
    {
        free(readbuf);
        sprintf(disp, "unable to open target binary `%s' for write", (int)dstfile);
        LOG(LOG_CRITICAL, 0, disp);
        return -1;
    }
    if (fwrite(readbuf, 1, filestat.st_size, writefp) < filestat.st_size)
    {
        free(readbuf);
        fclose(writefp);
        sprintf(disp, "unable to write target binary `%s'", (int)dstfile);
        LOG(LOG_CRITICAL, 0, disp);
        return -1;
    }
    if (fclose(writefp))
    {
        free(readbuf);
        sprintf(disp, "unable to close target binary '%s'", (int)dstfile);
        LOG(LOG_CRITICAL, 0, disp );
        return -1;
    }
    free(readbuf);
    return 0;
}
#endif

extern "C" STATUS csiShow(int record)
{
    FILE* dstFile = 0;
    STATUS rc = OK;

    dstFile = fopen(FILENAME, "w+");

    if(NULL == dstFile)
    {
        rc = ERROR;
    }
    else
    {
        csiFd = dstFile;

        rc |= CSI::GetInstance()->ShowHdrAndCsiRecordSummary();
        rc |= CSI::GetInstance()->ShowLog();
        rc |= CSI::GetInstance()->ShowCsiRecord(-1);

        rc |= fclose(dstFile); 
    }

    if(-1 != record)
    {
        csiFd = stdout;

        CSI::GetInstance()->ShowHdrAndCsiRecordSummary();
        CSI::GetInstance()->ShowLog();
        CSI::GetInstance()->ShowCsiRecord(record);

       // display current registered deadlock monitor task list
        //CSI::GetInstance()->ShowDeadlockTaskList();  
        fflush(csiFd);
    }

#ifndef M_TGT_L3
    uploadCSIfileToL3(FILENAME);
#endif

    return rc;
}



static CSI* csiObj = NULL;

STATUS csiInit(void)
{
    if (CSI::GetInstance() == NULL)
    {
        csiObj = new CSI;
    }

//    _csiWvrInit();


    return OK;
}

STATUS csiTerm(void)
{
    return OK;

}


void SaveRebootAlarmInfo(UINT16 almCode, UINT16 almEntity, UINT16 almInst)
{
    CSI *csiObj = CSI::GetInstance();

    if (csiObj)
    {
        csiObj->SaveRebootAlarmInfo(almCode,almEntity,almInst );
    }
}


extern "C" void CsiMonitorDeadlock(UINT32 tid, UINT32 maxBlockedTime)
{
    CSI *csiObj = CSI::GetInstance();
    if ( csiObj )
    {
        csiObj->MonitorTaskDeadlock(tid, maxBlockedTime);
    }
}
extern "C" void CsiEnableStackTrace(UINT32 tid)
{
    CSI *csiObj = CSI::GetInstance();
    if ( csiObj  )
    {
        csiObj->RegisterTaskStackTrace(tid);
    }
}

#ifndef WBBU_CODE
extern int startcsitest();
extern "C"
int startcsi()
{
    startcsitest();
}
#endif

extern "C"
void abortReboot(char *fileName, UINT32 lineNum)
{    
    if (CSI::GetInstance() != NULL)
    {
        CSI::GetInstance()->AbortRecord(fileName, lineNum);
    }    
}
#ifdef M_TGT_L3
/*初始化csi扩展部分*/
void CSI::InitImageNew()
{
    CSI_IMAGE_NEW *nvImage_new = (CSI_IMAGE_NEW*)CSI_IMAGE_ADDRESS_NEW;
    char *nvHdrNew   = nvImage_new->nvRamSafeHead;
    char *nvTailNew   = nvImage_new->nvRamSafeTail;  
    UINT32 nvSize_new  = CSI_IMAGE_SIZE_NEW;

//    ApAssertRtn((nvImage_new != NULL), LOG_SEVERE, LOG_ERR_CSI_INIT, "CSI::InitImage fail 0", ;);

    // Enable NVRAM writes
    if(bspEnableNvRamWrite((char *)nvImage_new, nvSize_new)==TRUE)
    {
         // Clear the whole area and set the default initial values
         memset((char *)nvImage_new, 0, nvSize_new);
         memcpy(nvHdrNew, CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN));
         memcpy(nvTailNew, CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN));
         // Disable NVRAM writes
         bspDisableNvRamWrite((char *)nvImage_new, nvSize_new);
    }
}
/*记录二层在三层部分上报的数据*/
void CSI::AbortRecord(char *fileName, int lineNum)
{
    CSI_IMAGE_NEW *nvImage_new = (CSI_IMAGE_NEW*)CSI_IMAGE_ADDRESS_NEW;
    UINT32 nvSize_new  = CSI_IMAGE_SIZE_NEW;
    ESFPPC     esf;
    CSI_IMAGE           *nvImage = (CSI_IMAGE *)(CSI_IMAGE *)CSI_IMAGE_ADDRESS;;
    CSI_NVRAM_HEADER    *nvHdr   = &nvImage->nvHdr;
    UINT32             csiRecordIndex = SIZEOF(nvImage->nvCsi);
    
    memset(&esf, 0, sizeof(esf));
    
    bspSetBtsResetReason(RESET_REASON_SW_NORMAL);

    RESET_REASON  restartCode = bspGetBtsResetReason();  /* save the first record reset type */

    criticalData.resetReason = restartCode;   
    criticalData.recordType = CSI_ABORT_RECORD;
  

    int len = strlen(fileName) > 255? 255 : strlen(fileName);
    if(bspEnableNvRamWrite((char *)nvImage_new->csi_record_new[csiRecordIndex].fileName, 260/*filename+linenum*/)==TRUE)
    {
        csiRecordIndex = nvHdr->nextCsiRecordIndex;
        memcpy(nvImage_new->csi_record_new[csiRecordIndex].fileName, fileName, len);
        nvImage_new->csi_record_new[csiRecordIndex].fileName[len] = '\0';
        nvImage_new->csi_record_new[csiRecordIndex].lineNum = lineNum; 
        // Disable NVRAM writes
        bspDisableNvRamWrite((char *)nvImage_new->csi_record_new[csiRecordIndex].fileName, 260/*filename+linenum*/);
    }
    _CsiIsrHandler(&esf);
}
#endif



