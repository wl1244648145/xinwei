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
*    HISTORY:
*    08/02/2010   Cao Huawei 移植到tci6487平台上
*******************************************************************/
#include <iostream.h>
#include <stdio.h>
#include <string.h>

#include "Csi.h"
#include "dataType.h"
#include "Log.h"
#include "loadVersion.h"
#include "btsTypes.h"
#include "DebugLevel.h"

#define MSR_RI                      0x00000002  /* Recoverable-Interrupt */
#define MAX_TASK_NUM                 20

CSI_CRITICAL_DATA   CSI::criticalData = {0};
CSI*                CSI::Instance = NULL;

UINT8               CSI::deadTidCount = 0;
DEADLOCK_MON_TASK_INFO  CSI::DeadlockMonTaskInfo[CSI_MAX_DEADLOCK_TIDS];

bool                CSI::idleTaskEnabled;
bool                CSI::shutdown = false;

CSI_SUSPENDED_TASK_ENTRY  CSI::suspendedTaskTable[CSI_SUSPEND_TASK_NUM];

#define CSI_DIR_NAME "csi"
#define FILENAME  "/RAMDL2/csi/l2-csi.log"

TSK_Handle tskArray[MAX_TASK_NUM] = {NULL};
int tskArrayIndex = 0;

extern cregister volatile unsigned int ICR;
extern cregister volatile unsigned int IER;
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

    if (nvImage == NULL)
    {
        LOG(LOG_SEVERE, LOG_ERR_CSI_INIT, "CSI::InitImage fail 0");
        return;
    }
    // Enable NVRAM writes
    if(bspEnableNvRamWrite((char *)nvImage, nvSize)==TRUE)
    {    
         // Clear the whole area and set the default initial values
         memset((char *)nvImage, 0, nvSize);
         memcpy((char *)&nvHdr->initTimeStamp, (char *)initTimeStamp, sizeof(nvHdr->initTimeStamp));
         nvHdr->version     = CSI_NVRAM_LATEST_VERSION;
         
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
    CSI_IMAGE         *nvImage = (CSI_IMAGE *)CSI_IMAGE_ADDRESS;
    CSI_NVRAM_HEADER  *nvHdr   = &nvImage->nvHdr;
    UINT32             nvSize  = CSI_IMAGE_SIZE;

    if (Instance != NULL)
    {
        return;
    }
    enabled = true;

    DiscoverLastResetReason();

    if (sizeof(*nvImage) > nvSize)
    {
        LOG(LOG_SEVERE, LOG_ERR_CSI_NVRAM_TOO_SMALL, "CSI::NVRAM is too small to hold CSI record");
        return;
    }
 
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
    if (TSK_isTSK())
    {
        TSK_disable();
    }
    int oldLevel = HWI_disable();
    
    // If NVRAM has not been intialized
    if (   (0 != strncmp((char *)&nvHdr->nvRamSafePattern[0], (char *)CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN)) ) 
        || (nvHdr->version != CSI_NVRAM_LATEST_VERSION) 
        || (nvHdr->nextCsiRecordIndex > SIZEOF(nvImage->nvCsi)) 
        || (nvHdr->nextLogRecordIndex > SIZEOF(nvImage->nvLog))
       )
    {
        nvHdr->nvRamSafePattern[sizeof(CSI_NVRAM_CSI_SAFE_PATTERN)] = '\0';
        LOG(LOG_SEVERE,LOG_ERR_CSI_INIT, "CSI::init");
        LOG1(LOG_SEVERE, LOG_ERR_CSI_INIT, "%s", nvHdr->nvRamSafePattern);
        LOG3(LOG_SEVERE,LOG_ERR_CSI_INIT, "%d;%d;%d",  nvHdr->version, nvHdr->nextCsiRecordIndex, nvHdr->nextLogRecordIndex);
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
        nvLog->resetReason     = bspGetBtsResetReason();
        nvLog->recordType      = CSI_START_RECORD;
        bspDisableNvRamWrite((char *)nvHdr, sizeof(*nvHdr));
    }
    // End critical section
    HWI_restore(oldLevel);
    if (TSK_isTSK())
    {
        TSK_enable();
    }
    
    //Clear Idle Counts
    idleCountMax=1;
    idleCountHighWM=0;
    idleCountLowWM=0xffffffff;
    idleCountThisPass[0]=0;
    idleCountThisPass[1]=0;
    idleCountPassIndex = 0;
    currentIdleCountAverage=0;

    for(int i=0; i<MAX_TASK_NUM; ++i)
    {
        tskArray[i] = NULL;
    }
    
    _MPC_userHook = (void (*)(void))CsiIsrHandler;
    
    // Start the Idle Task
    TSK_Attrs tskAttr;
    tskAttr.stack = NULL;
    tskAttr.stacksize = 2048;
    tskAttr.stackseg = TSK->STACKSEG;
    tskAttr.environ = NULL;
    tskAttr.exitflag = TRUE;
    tskAttr.initstackflag = TRUE;
    tskAttr.priority = 1;//to be considered
    tskAttr.name = "csiCheck";
    TSK_Handle tsk = TSK_create((Fxn)PeriodCheckTask, &tskAttr, this);
    csiRegTask(tsk);
    
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

    if (TSK_isTSK())
    {
        TSK_disable();
    }
    
    IER = 0; 
    ICR = 0xFFFF;
    while(1);
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
	int i;
	if (TSK_isTSK())
    {
        TSK_disable();
    }
    int oldLevel = HWI_disable();

    for (i=0; i<tskArrayIndex; ++i)
    {
        if ((UINT32)tskArray[i] == tid)
        {
            break;
        }
    }
    if (i >= MAX_TASK_NUM)
    {
        LOG(LOG_SEVERE, LOG_ERR_CSI_TRACE_REG, "Invalid tid");
        HWI_restore(oldLevel);
        if (TSK_isTSK())
        {
            TSK_enable();
        }
        return ERROR;
    }
   
    if (deadTidCount >= SIZEOF(DeadlockMonTaskInfo))
    {
        LOG(LOG_SEVERE, LOG_ERR_CSI_DEADLOOP_MON_REG, "CSI max deadlock TIDs exceeded");
        HWI_restore(oldLevel);
        if (TSK_isTSK())
        {
            TSK_enable();
        }
        return ERROR;
    }

    // Round up to next higher integral number of seconds.
    DeadlockMonTaskInfo[deadTidCount].maxBlockedTime = 8* maxBlockedTimeInTicks / CSI_SW_WDT_DELAY_IN_TICK;
    DeadlockMonTaskInfo[deadTidCount].remainBlockedTime = DeadlockMonTaskInfo[deadTidCount].maxBlockedTime;
    DeadlockMonTaskInfo[deadTidCount].switchCount = 1;
    TSK_setenv((TSK_Handle)tid, (Ptr)&DeadlockMonTaskInfo[deadTidCount].switchCount);
    DeadlockMonTaskInfo[deadTidCount].deadlockTid = tid;

    deadTidCount++;

	HWI_restore(oldLevel);
    if (TSK_isTSK())
    {
        TSK_enable();
    }

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
	int i;
	if (TSK_isTSK())
    {
        TSK_disable();
    }
    int oldLevel = HWI_disable();

    for (i=0; i<tskArrayIndex; ++i)
    {
        if ((UINT32)tskArray[i] == tid)
        {
            break;
        }
    }
    if (i >= MAX_TASK_NUM)
    {
        LOG(LOG_SEVERE, LOG_ERR_CSI_TRACE_REG, "Invalid tid");
        HWI_restore(oldLevel);
        if (TSK_isTSK())
        {
            TSK_enable();
        }
        return ERROR;
    }
    if (criticalData.traceTaskCount >= SIZEOF(criticalData.taskTrace))
    {
        LOG(LOG_SEVERE, LOG_ERR_CSI_TRACE_REG, "CSI max stack trace TIDs exceeded");
        HWI_restore(oldLevel);
        if (TSK_isTSK())
        {
            TSK_enable();
        }
        return ERROR;
    }

    criticalData.taskTrace[criticalData.traceTaskCount].tid = tid;
    criticalData.traceTaskCount ++;
    
	HWI_restore(oldLevel);
    if (TSK_isTSK())
    {
        TSK_enable();
    }

    return(OK);
}

//save the task status information of all the system tasks
void CSI::GetTaskInfo()
{
    TSK_Handle tcb;
    TSK_Stat statbuf;
    // Loop through all of the task IDs
    for (int id = 0; id < tskArrayIndex; id++)
    {
        // Get the task info
        if (tskArray[id] != 0)
        {
            tcb = tskArray[id];
            TSK_stat(tcb, &statbuf);
            
            criticalData.taskCount ++;

            memcpy(&criticalData.taskStatus[id].name[0], tcb->name, CSI_NAME_LEN);
            criticalData.taskStatus[id].name[CSI_NAME_LEN] = 0;
            criticalData.taskStatus[id].tid = (int)tcb;
            criticalData.taskStatus[id].sp = (UINT32)tcb->kobj.sp;
            criticalData.taskStatus[id].base = (UINT32)tcb->stack+tcb->stacksize;
            criticalData.taskStatus[id].size = tcb->stacksize;

            // Some general task info
            criticalData.taskStatus[id].priority    = tcb->kobj.priority;
            criticalData.taskStatus[id].errorStatus = tcb->errno;
            criticalData.taskStatus[id].pc          = (UINT32)tcb->environ;
            criticalData.taskStatus[id].taskStatus      = tcb->kobj.mode;

            // If VxWorks stack fill option is not used
            //if (statbuf.attrs.initstackflag)
            {
                criticalData.taskStatus[id].high = statbuf.used;
                criticalData.taskStatus[id].margin = tcb->stacksize - statbuf.used;
            }
        }
    }
}

// save task stack trace of the specific task
void CSI::GetTaskStackTrace(int taskIndex)
{
    int tid = criticalData.taskTrace[taskIndex].tid;
    TSK_Handle tcb = (TSK_Handle)tid;

    // Stack Trace
    traceCount      = &criticalData.taskTrace[taskIndex].traceCount;
    traceBuffer     = &criticalData.taskTrace[taskIndex].trace[0];
    traceBufferSize = SIZEOF(criticalData.taskTrace[taskIndex].trace);
    *traceCount     = 0;
    
    UINT32* stackBase = (UINT32*)((UINT8*)tcb->stack+tcb->stacksize);
    UINT32  stackSize = (UINT32)stackBase - (UINT32)tcb->kobj.sp;
   
    stackSize = min(stackSize, sizeof(criticalData.taskTrace[taskIndex].trace));
    memcpy((UINT8*)&criticalData.taskTrace[taskIndex].trace[0], 
                    (UINT8*)tcb->kobj.sp, stackSize);
    criticalData.taskTrace[taskIndex].traceCount = stackSize;
}

// save the resource trace of one specific task
// return the task ID holding the resource pended by tid
int CSI::TraceOneTaskPendingResource(int tid, int taskIndex)
{
    TSK_Handle  tcb = (TSK_Handle)tid;
    int         depth;
    CSI_RESOURCE_TRACE *trace;
    int holdingTid = 0;

    depth = criticalData.deadlockTrace[taskIndex].resourceTraceDepth;
    if (depth == CSI_MAX_RESOURCE_TRACE-1)
    {
        return 0;
    }
    
    if (tcb != NULL)
    {
        // initialize the entry in resource trace table
        trace = &criticalData.deadlockTrace[taskIndex].resourceTrace[depth];
        trace->tid = tid;
        trace->reason = CSI_TASK_PEND_UNKNOWN;
        criticalData.deadlockTrace[taskIndex].resourceTraceDepth ++;   
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
void CSI::CsiIsrHandler(UINT32 tidArray[])
{
    CSI::GetInstance()->_CsiIsrHandler(tidArray);
}

void CSI::_CsiIsrHandler(UINT32 tidArray[])
{
    int                  tid=0;
    TSK_Handle           tcb;
    CSI_IMAGE           *nvImage = (CSI_IMAGE *)CSI_IMAGE_ADDRESS;
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
    if (TSK_isTSK())
    {
        TSK_disable();
    }
    int oldLevel = HWI_disable();
    
    /* feed the hardware watchdog if any */
    bspSetResetFlag(RESET_SYSTEM_L2_SELF_RESET);
    
    // Only record the first exception in the CSI. Secondary exceptions during recovery are not logged
    //   and fall through to hardware watchdog recovery.
    if (!csiLogged)
    {

        csiLogged = true;
        T_TimeDate timeDate = bspGetDateTime();

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
            case CSI_ABORT_RECORD:
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
                criticalData.troubleTask[0].task.tid = (int)tidArray[0];
                break;
            case CSI_DEADLOCK_RECORD:
                task = 0;   // gpr in regSet is used to save deadlock tids
                while (((tid = tidArray[task]) != 0) && 
                         (task < MAX_TASK_NUM) && (task < (int)SIZEOF(criticalData.troubleTask)))
                {
                    criticalData.troubleTask[task].task.tid = tid;
                    task++;
                }
                break;
            default:
                criticalData.troubleTask[0].task.tid = (int)TSK_self();
                break;
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////
        //   Log register info 
        //////////////////////////////////////////////////////////////////////////////////////////
        getCpuRegister(&criticalData.troubleTask[0].cpuReg);
        if (criticalData.recordType == CSI_EXCEPTION_RECORD)
        {
            getExcRegister(&criticalData.troubleTask[0].excReg);
        }
            
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

                if((tcb = (TSK_Handle)tid) != NULL)
                {
                    // Basic Task Info
                    criticalData.troubleTask[task].task.pc = (UINT32)tcb->environ;
                    memcpy(&criticalData.troubleTask[task].task.name[0], TSK_getname(tcb), CSI_NAME_LEN);
                    criticalData.troubleTask[task].task.name[CSI_NAME_LEN] = 0;

                    // Copy the ESF into the Critical data buffer for DEADLOCK record
                    if (criticalData.recordType == CSI_DEADLOCK_RECORD)
                    {
                        //memcpy(&criticalData.troubleTask[task].esf.regSet, &tcb->regs, sizeof(tcb->regs));
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
             || (criticalData.recordType == CSI_STACKCHECK_RECORD
             || (criticalData.recordType == CSI_ABORT_RECORD))
            )
        {
            if ( tcb!= NULL)
            {
                UINT32* stackBase = (UINT32*)((UINT8*)tcb->stack+tcb->stacksize);
                UINT32  stackSize = (UINT32)stackBase - (UINT32)tcb->kobj.sp;
                // tcb is the result of previous code execution

                stackSize = min(stackSize, sizeof(criticalData.rawStack));

                criticalData.troubleTask[0].tskSp = (UINT32)tcb->kobj.sp;
                memcpy((UINT8*)&criticalData.rawStack[0], (UINT8*)tcb->kobj.sp, stackSize);
            }
            else
            {
                //criticalData.troubleTask[0].esf.pad1 = 0;
                criticalData.troubleTask[0].tcb = 0;
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

        criticalData.memHeapStatsSafePattern = CSI_MEM_STATS_SAFE_PATTERN;  
        UpdateCsiRecord(nvCsi);

        if (criticalData.recordType == CSI_DEADLOCK_RECORD)
        {
            DeadlockResourceTrace();
            UpdateCsiRecord(nvCsi);
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////
        // Reset the processor
        //////////////////////////////////////////////////////////////////////////////////////////
        //printf("CSI try to reset BTS, reason = %d\n",criticalData.recordType);
		// should not print here, in case the serial port is already dead the task will be 
		// suspended	
	    IER = 0; 
        ICR = 0xFFFF;
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
	    IER = 0;
        ICR = 0xFFFF;
        while(1);
    }
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
void CSI::ContextSwitchHandler(TSK_Handle pOldTcb, TSK_Handle pNewTcb)
{
    CSI_CONTEXT_INFO* context;
    UINT32         contextCount = criticalData.contextSwitchHistory.contextCount;

    // Save TID and PC at each context switch for history in CSI crash records
    context      = &criticalData.contextSwitchHistory.oldContext[contextCount];
    context->tid = (int)pOldTcb;
    //context->pc  = pOldTcb->regs.pc xxx;

    context      = &criticalData.contextSwitchHistory.newContext[contextCount];
    context->tid = (int)pNewTcb;
    //context->pc  = pNewTcb->regs.pc xxx;

    if (++criticalData.contextSwitchHistory.contextCount >= SIZEOF(criticalData.contextSwitchHistory.newContext))
    {
        criticalData.contextSwitchHistory.contextCount = 0;
    }

    // Deadlock monitoring  
    Ptr p = TSK_getenv(pNewTcb);
    if(p != NULL)   // reserved1 saves the address of switchCount
    {
        (*(UINT32*)p)++;
    }
}

int  CSI::WdtExceptionHandler(UINT32 tid[])
{
    CsiIsrHandler(tid);
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
extern "C" STATUS exitTelnetSession();
UINT8 enableExit = 1;
UINT8 times_count = 0;
void CSI::DeadlockCheck(CSI *csi)
{
    unsigned int    i, j;
    static bool     deadlock = false;
    static UINT32 firstTime = 3000;   // 3000*100ms = 300s

    if (firstTime > 0) 
    {   // do not check deadlock for the first 2 minutes
        firstTime--;
        csi->lastIdleCountIndex = csi->idleCountPassIndex;
        csi->lastPassIdleCount = csi->idleCountThisPass[csi->idleCountPassIndex];
        return;
    }

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
                if ((DeadlockMonTaskInfo[i].switchCount == 0) 
                    && (((TSK_Handle)DeadlockMonTaskInfo[i].deadlockTid)->kobj.mode != KNL_READY))
                {
                    if ( --DeadlockMonTaskInfo[i].remainBlockedTime == 0)
                    {
                        deadlock = true;
                        taskId[i] = DeadlockMonTaskInfo[i].deadlockTid;
                        printf("Task Deadlock detected, tid = 0x%x\n", taskId[i]);
                    }
                }
                else
                {
                    DeadlockMonTaskInfo[i].remainBlockedTime = DeadlockMonTaskInfo[i].maxBlockedTime;
					if (DeadlockMonTaskInfo[i].deadlockTid == 0x847843F4)
					{
						asm(" NOP");
					}
                }

                // prepare for next round checking
                DeadlockMonTaskInfo[i].switchCount = 0;
            }
        }

        // If one or more tasks are deadlocked
        if (deadlock)
        {
            if (TSK_isTSK())
            {
                TSK_disable();
            }
            int oldLevel = HWI_disable();

            UINT32 taskArray[SIZEOF(DeadlockMonTaskInfo)];

            memset(taskArray, 0 ,sizeof(taskArray));
        
            for(i=0, j=0; i< SIZEOF(taskId) && i<MAX_TASK_NUM; ++i)
            {
                if (taskId[i] != 0)
                {
                    taskArray[j++] = taskId[i];
                }
            }
            criticalData.recordType = CSI_DEADLOCK_RECORD;
            criticalData.resetReason = RESET_REASON_SW_ABNORMAL;

            if(csi->enabled)
            {
                WdtExceptionHandler(taskArray);
            }

            HWI_restore(oldLevel);
            if (TSK_isTSK())
            {
                TSK_enable();
            }

            csiDeadlockCnt++;
        }
    }

    // reboot BTS if any task got suspended for more than 120 seconds
#if 0 // can not be used on dsp_bios
    if (!deadlock && !shutdown)
    {
        UINT32 taskId[SIZEOF(DeadlockMonTaskInfo)];
        memset(taskId, 0 ,sizeof(taskId));
        
        bool suspendedInThisRound[CSI_SUSPEND_TASK_NUM];
        for (int tmpIndex=0; tmpIndex<CSI_SUSPEND_TASK_NUM; tmpIndex++)
        {
            suspendedInThisRound[tmpIndex] = false;
        }

        /*modify by caohuawei:can't monitor all task suspend*/
        for (int taskIndex=0; taskIndex<deadTidCount/*tskArrayIndex*/; taskIndex++)
        {
            if (((TSK_Handle)/*tskArray[taskIndex]*/DeadlockMonTaskInfo[taskIndex].deadlockTid)->kobj.mode == KNL_BLOCKED)
            {
                int entryIndex=0;
                for (entryIndex=0; entryIndex<CSI_SUSPEND_TASK_NUM; entryIndex++)
                {  // try to find the task in suspended task table
                    if (suspendedTaskTable[entryIndex].tid == (int)/*tskArray[taskIndex]*/DeadlockMonTaskInfo[taskIndex].deadlockTid)
                    {  // found, decrement TTL, reach 0 then reboot BTS
                        suspendedInThisRound[entryIndex] = true;
                        suspendedTaskTable[entryIndex].TTL --;
                        if ( 0 == suspendedTaskTable[entryIndex].TTL)
                        {
                            taskId[0] = (UINT32)/*tskArray[taskIndex]*/DeadlockMonTaskInfo[taskIndex].deadlockTid;
                            if (TSK_isTSK())// Shut down all tasking as a software watchdog is in progress
                            {
                                TSK_disable();
                            }
                            int oldLevel = HWI_disable();
                           
                            criticalData.recordType = CSI_DEADLOCK_RECORD;
                            criticalData.resetReason = RESET_REASON_SW_ABNORMAL;

                            if(csi->enabled)
                            {
                            	//del the line below for new telnet test -fengbing 20080417
                                WdtExceptionHandler(taskId);
                            }

                            HWI_restore(oldLevel);
                            if (TSK_isTSK())
                            {
                                TSK_enable();
                            }
                            
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
                            printf("Task 0x%x is put into deadlock list\n", (UINT32)/*tskArray[taskIndex]*/DeadlockMonTaskInfo[taskIndex].deadlockTid);
                            suspendedTaskTable[entryIndex].tid = (int)/*tskArray[taskIndex]*/DeadlockMonTaskInfo[taskIndex].deadlockTid;
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
                printf("Task 0x%x is remove from deadlock list\n", suspendedTaskTable[tmpIndex].tid);
                suspendedTaskTable[tmpIndex].tid = 0;
            }
        }

    }
#endif    
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
            deadTidCount --;
        }
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
    bool        stackError;

    stackError = false;
    errorTid = 0;

    TSK_Handle tcb;
    TSK_Stat statbuf;
    
    // Loop through all of the task IDs
    for (int id = 0; id < tskArrayIndex; id++)
    {
        // Get the task info
        if (tskArray[id] != 0)
        {
            tcb = (TSK_Handle)tskArray[id];
            TSK_stat(tcb, &statbuf);
            
            // If VxWorks stack fill option is not used
            //if (statbuf.attrs.initstackflag)
            {
                // Check for stack overrun based on current stack pointer
                if ((statbuf.attrs.stacksize - statbuf.used) < CSI_STACKCHECK_MARGIN)
                {
                    stackError = true;
                    errorTid = (int)tskArray[id];
                    break;
                }
            }
        }
    }

    return stackError;
}

extern "C" int g_malloc_sz;

void CSI::PeriodCheckTask(CSI *csi)
{
    static UINT32 periodCounter = 0;
    for (;;)
    {
        periodCounter++;
        if (periodCounter >= CSI_STACKCHECK_DELAY_IN_SECOND)  // run stack check every minute
        {
            //csi->IncrementIdleCountThisPass();   // in case this function runs for some time too long

            if(csi->enabled)
            {
                bool        stackError;
                int         tid;

                stackError = csi->StackCheck(tid);

                // If any stack was overrun
                if (stackError)
                {
                    criticalData.recordType = CSI_STACKCHECK_RECORD;
                    criticalData.resetReason = RESET_REASON_SW_ABNORMAL;

                    // Cause an exception (exception handler will log data and reset board)
                    CsiIsrHandler((UINT32*)&tid);
                }
            }
            periodCounter = 0;
        }

#if 0       
        MEM_stat(0, &criticalData.memHeapStats);
        int maxMemSize = criticalData.memHeapStats.length;
        if (maxMemSize < 0x100000 )
#else
		if(g_malloc_sz>48*0x100000)
#endif//0
        {   /* reboot the BTS if the maximum free memory block size is below 2MB */
//            printf("Reboot by memory leak, maximum block size = %d\n", maxMemSize);
            
            criticalData.recordType = CSI_SW_MEMORY_LEAK;
            criticalData.resetReason = RESET_REASON_SW_ABNORMAL;

            if(csi->enabled)
            {
                WdtExceptionHandler(NULL);
            }
        }

        TSK_sleep(100); // delay 1s
    }
}                   


CSI* CSI::GetInstance(void)
{
    return Instance;
}

void CSI::HookExceptionISRs(void)
{
    _MPC_userHook = (void (*)(void))CSI::CsiIsrHandler;
}

void CSI::RestoreExceptionISRs()
{
    _MPC_userHook = NULL;
}

void CSI::SaveRebootRecord()
{
    bspSetBtsResetReason(RESET_REASON_SW_NORMAL);

    RESET_REASON  restartCode = bspGetBtsResetReason();  /* save the first record reset type */

    criticalData.resetReason = restartCode;
    
    if( RESET_REASON_SW_NORMAL == restartCode )
    {
        criticalData.recordType = CSI_REBOOTHOOK_RECORD;
    }
    else
    {
        criticalData.recordType = CSI_CRITICALALARM_RECORD;
    }

    _CsiIsrHandler(NULL);

}

void CSI::AbortRecord(char *fileName, int lineNum)
{
    bspSetBtsResetReason(RESET_REASON_SW_NORMAL);

    RESET_REASON  restartCode = bspGetBtsResetReason();  /* save the first record reset type */

    criticalData.resetReason = restartCode;
    
   
    criticalData.recordType = CSI_ABORT_RECORD;
  

    int len = strlen(fileName) > sizeof(criticalData.fileName)-1? 
                    sizeof(criticalData.fileName)-1 : strlen(fileName);

    memcpy(&criticalData.fileName[0], fileName, len);
    criticalData.fileName[len] = '\0';
    criticalData.lineNum = lineNum; 
    
    _CsiIsrHandler(NULL);
}

static char *csiExcString(UINT32 mpFsr)
{
    static char *strings[] = {"",            /* 0x0000 */
                              "User Execute violation ",      /* 0x00000001 */
                              "User Write violation ",       /* 0x00000002 */
                              "User Read violation ",       /* 0x00000004 */
                              "Supervisor Execute violation ",       /* 0x00000008 */
                              "Supervisor Write violation ",       /* 0x00000010 */
                              "Supervisor Read violation ",      /* 0x00000020 */
    };
	int i;

    mpFsr &= 0x0000003F;
    for(i=0;i<8;i++)
    {
        if ((1<<i) == mpFsr)
        {
            break;
        }
    }
    if (i<SIZEOF(strings)-1)
    {
        return strings[i+1];
    }
    else
    {
        return strings[0];
    }
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
                              "ABORT REBOOT   "};

    if ((unsigned int)recordType >= SIZEOF(strings)) recordType = CSI_EMPTY_RECORD;

    return (strings[recordType]);
}


static char *resetReasonString(RESET_REASON resetReason)
{
    static char *resetStr[] = {"HW WATCHDOG",
                               "SW WATCHDOG",
                               "POWER ON   ",
                               "SW NORMAL  ",
                               "SW ALARM   ",
                               "SW ABNORMAL",
                               "BOOTUP FAIL",
                               "L3 REBOOT  ",
                			   "BOOT DOWNFPGA_FAIL",//wangwenhua add 20080801
                               "BOOT EMSTIMEOUT",
                               "BOOT LOADCODETIMEOUT",
                               "BOOT NVRAMDATA_FAIL",
                               "ARPNOT_GATEWAY",
                               "RESET_REASON_PM_CREATE_FAIL",
                                "RESET_REASON_FTPC_CREATE_FAIL",
                               " RESET_REASON_NETMBUFFER_FULL",
                               "RESET_REASON_L3BOOTLINE_DIFF",
                               "UNKNOWN    "};

    if (resetReason >= RESET_REASON_HW_WDT && resetReason<= RESET_REASON_L3BOOTLINE_DIFF)
    {
        return resetStr[resetReason - RESET_REASON_HW_WDT];
    }
    else
    {
        return resetStr[SIZEOF(resetStr)-1];
    }
}
/*#define WIND_READY		0x00 
#define WIND_SUSPEND		0x01 
#define WIND_PEND		0x02	
#define WIND_DELAY		0x04	
#define WIND_DEAD		0x08	
*/  // defined in taskLibP.h

static char *csiTaskStatusName(int taskStatus)
{
    static char *strings[] = {"TSK_RUNNING   ",
                              "TSK_READY ",
                              "TSK_BLOCKED    ",
                              "TSK_TERMINATED  ",
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
            rc = (char*)nvCsi->taskStatus[loopCounter].name;
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
        
        printf(" Resource Trace of Task %s\n", 
                GetTaskNameFromTid(nvCsi, nvCsi->deadlockTrace[i].resourceTrace[0].tid));
        printf(" level    taskName     PendReason   ResourceId  HoldingTask\n");
        printf(" -----  ------------  ------------ ------------ -----------\n"); 
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

    if (   (0 != strncmp((char *)&nvHdr->nvRamSafePattern[0], (char *)CSI_NVRAM_CSI_SAFE_PATTERN, sizeof(CSI_NVRAM_CSI_SAFE_PATTERN)) ) 
        || (nvHdr->version != CSI_NVRAM_LATEST_VERSION) 
        || (nvHdr->nextCsiRecordIndex > SIZEOF(nvImage->nvCsi)) 
        || (nvHdr->nextLogRecordIndex > SIZEOF(nvImage->nvLog))
       )
    {
        printf("CSI NVRAM not initialized!\n");
        return ERROR;
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
        printf("Invalid Record ID!\n");
        return ERROR;
    }

    for (i = min; i <= max; i++)
    {
        nvCsi = &nvImage->nvCsi[i];

        printf("\n");
        printf("RECORD %d", i + 1);

        if (!nvCsi->valid)
        {
            printf("   (EMPTY)\n");
            printf("\n");
        }
        else
        {
            printf("\n");
            printf("\n");
            printf("  CSI Version: %d\n", nvCsi->version);

            printf("  CSI Timestamp: %02d/%02d/%04d %02d:%02d:%02d\n",
                   nvCsi->rtc.month, nvCsi->rtc.day, nvCsi->rtc.year,
                   nvCsi->rtc.hour, nvCsi->rtc.minute, nvCsi->rtc.second);
            if (nvCsi->recordType == CSI_EXCEPTION_RECORD)
            {
                for(j=0; j<3; ++j)
                {
                    if (nvCsi->troubleTask[0].excReg.mpFsr[j] != 0)
                    {
                        break;
                    }
                }
                if (j >= 3)
                {
                    j = 0;
                }
            }
            printf("  CSI Record Type: %s%s\n",
                   ((nvCsi->recordType == CSI_EXCEPTION_RECORD) ? 
                     csiExcString(nvCsi->troubleTask[0].excReg.mpFsr[j]) : ""),
                   csiTypeString(nvCsi->recordType));
            
            printf("  CSI Reset Reason: %d\n", nvCsi->resetReason);
            printf("\n");
            
            printf("  BSP Version: %s\n", nvCsi->loadVersion);
            printf("\n");

            taskData = nvCsi->troubleTask;
            switch (nvCsi->recordType)
            {
                case  CSI_CRITICALALARM_RECORD:
                    
                    printf("  Critical Alarm Code(%04x)\n", nvCsi->alarmInfo.code);
                    printf("           Alarm Entiry(%04d)\n", nvCsi->alarmInfo.entity);
                    printf("           Alarm Instance(%04d)\n", nvCsi->alarmInfo.instance);
                    break;

                case CSI_CRITICALASSERTION_RECORD:
                    printf("  Critical Assertion Code(%04x)\n", nvCsi->alarmInfo.code & ~0x4000);
                    if (taskData[0].task.tid != 0)
                    {
                        printf("\n  Task Info:\n");
                        printf("  Tid=0x%08x, TaskEntry=0x%08x Name=%-15s\n", taskData[0].task.tid, taskData[0].task.pc, taskData[0].task.name);
                        printf("\n");
                    }
                    break;
                case CSI_ABORT_RECORD:
                    printf("  %s %d lines cause abort reboot\n", nvCsi->fileName, nvCsi->lineNum);
                     // fall through to next case
                default:
                    task = 0;
                    for (task = 0; task < SIZEOF(nvCsi->troubleTask); task++)
                    {
                        if (taskData[task].task.tid != 0)
                        {
                            printf("\n  Trouble Task Info:\n");
                            printf("  Tid=0x%08x, TaskEntry=0x%08x Name=%-15s\n", taskData[task].task.tid, taskData[task].task.pc, taskData[task].task.name);
                            printf("\n");

                            printf("  DSP Register Data:\n");
                            printf("\n");                            
                            //printf("  pc    =0x%08x\n", taskData[task].reg.pc);    
                            printf("  ier      =0x%08x\n", taskData[task].cpuReg.ier);
                            if (nvCsi->recordType != CSI_DEADLOCK_RECORD)
                                printf("  sp       =0x%08x\n", taskData[task].tskSp);   
                            if (nvCsi->recordType == CSI_EXCEPTION_RECORD)
                            {
                                printf("  nrp      =0x%08x\n", taskData[task].excReg.nrp);
                                printf("  efr      =0x%08x\n", taskData[task].excReg.efr);
                                printf("  L1PmpFar =0x%08x\n", taskData[task].excReg.mpFar[0]);
                                printf("  L1PmpFsr =0x%08x\n", taskData[task].excReg.mpFsr[0]);
                                printf("  L1DmpFar =0x%08x\n", taskData[task].excReg.mpFar[1]);
                                printf("  L1DmpFsr =0x%08x\n", taskData[task].excReg.mpFsr[1]);
                                printf("  L2mpFar  =0x%08x\n", taskData[task].excReg.mpFar[2]);
                                printf("  L2mpFsr  =0x%08x\n", taskData[task].excReg.mpFsr[2]);
                            }
                            printf("\n");
                        }
                        else
                        {
                            break;
                        }
                    }

                    /////////////////////////////
                    // print memory heap status                    
                    printf("  Heap Stats:\n");
                    printf("  ------------------\n");
                    if (nvCsi->memHeapStatsSafePattern != CSI_MEM_STATS_SAFE_PATTERN)
                    {
                        printf("  Not available\n\n\n");
                    }
                    else
                    {
                        printf("  size:        = %d\n", nvCsi->memHeapStats.size);
                        printf("  numBytesFree = %d\n", nvCsi->memHeapStats.size
                                                    - nvCsi->memHeapStats.used);
                        printf("  numBlocksFree = %d\n", nvCsi->memHeapStats.space);
                        printf("  maxBlockSizeFree = %d\n", nvCsi->memHeapStats.length);
                        printf("  numBytesAlloc = %d\n", nvCsi->memHeapStats.used);
                    }

                    // print raw stack data                   
                    if(  (nvCsi->recordType == CSI_EXCEPTION_RECORD)
                         || (nvCsi->recordType == CSI_WATCHDOGTASK_RECORD )
                         || (nvCsi->recordType == CSI_STACKCHECK_RECORD) 
                         )
                   {
                        UINT32    sp = nvCsi->troubleTask[0].tskSp;

                        const int    rowSize = 4;
                        unsigned int columnSize = SIZEOF(nvCsi->rawStack) / rowSize;

                        printf("  Stack Data:\n\n");
                        printf("  ------------------  ------------------  ------------------  ------------------");
                        for (j = 0; j < columnSize; j++)
                        {
                            printf("\n");

                            printf("  %08x: %08x  %08x: %08x  %08x: %08x  %08x: %08x",

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

                        printf("<-sp\n\n\n");
                    }


                    break;
            }
           
            printf("  Context Switch Data:\n");
            printf("\n");
            printf("           OLD-TASK    OLD-TID      OLD-PC       NEW-TASK      NEW-TID      NEW-PC\n");  
            printf("      --------------- ----------  ---------- --------------- ----------  ----------\n");
            for (j = 0, n = nvCsi->contextSwitchHistory.contextCount; 
                 j < SIZEOF(nvCsi->contextSwitchHistory.newContext); 
                 j++, n = ((n == (SIZEOF(nvCsi->contextSwitchHistory.newContext) - 1)) ? 0 : (n + 1)))
            {
                
                char *oldname = GetTaskNameFromTid(nvCsi, nvCsi->contextSwitchHistory .oldContext [n].tid);
                char *newname = GetTaskNameFromTid(nvCsi, nvCsi->contextSwitchHistory .newContext [n].tid);
                printf("  %02d: %-15s 0x%08x  0x%08x %-15s 0x%08x  0x%08x\n", (j+1),
                       oldname, nvCsi->contextSwitchHistory.oldContext[n].tid, 
                        nvCsi->contextSwitchHistory.oldContext[n].pc,
                       newname, nvCsi->contextSwitchHistory.newContext[n].tid, 
                        nvCsi->contextSwitchHistory.newContext[n].pc
                       );
            }
            printf("\n");

            //////////////////////////////////////////////////////
            // print task status
            if (nvCsi->taskCount > 0)
            {
                int entryId = 0;

                printf("  Task Info:\n");
                printf("\n");
                printf("        TASK          TID      Pri     Status       PC       ERRNO      SP       BASE     SIZE    HIGH   MARGIN\n");
                printf("      ------------ ----------  ---  -----------  --------  --------  --------  --------  ------  ------  ------\n");

                for(unsigned int pri = TSK_MAXPRI; pri >0; --pri)
                {
                    for (j = 0; ((j < nvCsi->taskCount) && (j<SIZEOF(nvCsi->taskStatus))); j++)
                    {
                        if(pri == nvCsi->taskStatus[j].priority)
                        {
                            printf("  %02d: %-12s 0x%08x  %3u  %-8s  %08x  %08x  %08x  %08x  %6u  %6u  %6u", 
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
                                printf("******\n");
                            }
                            else
                            {
                                printf("\n");
                            }
                        }
                    }
                }
                printf("\n");
            }

            /////////////////////////////
            // print task stack trace information
            if (nvCsi->traceTaskCount > 0)
            {
                for (task = 0; ((task < nvCsi->traceTaskCount) && (task<SIZEOF(nvCsi->taskTrace))); task++)
                {
                    char *name = GetTaskNameFromTid(nvCsi, nvCsi->taskTrace[task].tid);

                    printf("\n  %s (tid 0x%x) Stack Traceback Data:\n", name, nvCsi->taskTrace[task].tid);
                    printf("\n");
                    printf("                            Return       Entry\n");
                    printf("                           --------     --------\n");

                    for (j = 0; ((j < nvCsi->taskTrace[task].traceCount) && 
                                  (j < SIZEOF(nvCsi->taskTrace[task].trace))); j++)
                    {
                        if(0 == j) printf("                 current->"); 
                        else  printf("\n                          ");
                        printf("0x%08x : 0x%08x", 
                                nvCsi->taskTrace[task].trace[j].addr1, 
                                nvCsi->taskTrace[task].trace[j].addr2);
                    }
                    if(nvCsi->taskTrace[task].traceCount
                         < (SIZEOF(nvCsi->taskTrace[task].trace) - 1))
                    {
                        printf("<-bottom\n\n\n");
                    }
                    else
                    {
                        printf("\n\n");
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
        //ApAssertRtnV(false, LOG_SEVERE, LOG_ERR_CSI_INVALID_IMAGE, "CSI NVRAM not initialized!", ;,ERROR);
        printf("CSI NVRAM not initialized!\n");
        return ERROR;
    }

    printf("\n");
    printf("Rec      Type         CSI  ResetReason      Timestamp                             BSP Version\n");
    printf("---  ---------------  ---  -----------  -------------------  -----------------------------------------------------\n");

    for (i = 0; i < SIZEOF(nvImage->nvLog); i++)
    {
        nvLog = &nvImage->nvLog[i];

        if (nvLog->recordType == CSI_EMPTY_RECORD)
        {
            printf("%03d  (EMPTY)\n", i+1);
        }
        else
        {
            printf("%03d  %s  (%d)  %s  %02d/%02d/%04d %02d:%02d:%02d  %s\n", i + 1, csiTypeString(nvLog->recordType),
                   nvLog->csiRecordIndex,
                   resetReasonString(nvLog->resetReason),
                   nvLog->timeStamp.month, nvLog->timeStamp.day, nvLog->timeStamp.year,
                   nvLog->timeStamp.hour, nvLog->timeStamp.minute, nvLog->timeStamp.second,
                   nvLog->loadVersion);
        }
    }

    printf("\n");

    return OK;
}

STATUS CSI::ShowHdrAndCsiRecordSummary()
{
    unsigned int        i, j;
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
        //ApAssertRtnV(false, LOG_SEVERE, LOG_ERR_CSI_INVALID_IMAGE, "CSI NVRAM not initialized!",;,ERROR);
        printf("CSI NVRAM not initialized!\n");
        return ERROR;
    }

    printf("\n");
    printf("       L2 CSI Stats\n");
    printf("===================================\n");
    printf("Init Timestamp: %02d/%02d/%04d %02d:%02d:%02d\n",
           nvHdr->initTimeStamp.month, nvHdr->initTimeStamp.day, nvHdr->initTimeStamp.year,
           nvHdr->initTimeStamp.hour, nvHdr->initTimeStamp.minute, nvHdr->initTimeStamp.second);
    printf("Boot Timestamp: %02d/%02d/%04d %02d:%02d:%02d\n",
           nvHdr->bootTimeStamp.month, nvHdr->bootTimeStamp.day, nvHdr->bootTimeStamp.year,
           nvHdr->bootTimeStamp.hour, nvHdr->bootTimeStamp.minute, nvHdr->bootTimeStamp.second);
    printf("\n");
    printf("Format Version:  %10u\n", nvHdr->version);
    printf("Start Count:     %10u\n", nvHdr->startCount);
    printf("Reboot Count:    %10u\n", nvHdr->rebootCount);
    printf("Crash Count:     %10u\n", nvHdr->crashCount);
    printf("Next CSI Record: %10u\n", nvHdr->nextCsiRecordIndex + 1);
    printf("Next Log Record: %10u\n", nvHdr->nextLogRecordIndex + 1);


    for (i = 0; i < SIZEOF(nvImage->nvCsi); i++)
    {
        nvCsi = &nvImage->nvCsi[i];
        printf("\n");

        if (!nvCsi->valid)
        {
            printf("Record #%d  (EMPTY)\n", i + 1);
        }
        else
        {
            printf("Record #%d  %02d/%02d/%04d %02d:%02d:%02d\n", i + 1, nvCsi->rtc.month,
                   nvCsi->rtc.day, nvCsi->rtc.year, nvCsi->rtc.hour, nvCsi->rtc.minute, nvCsi->rtc.second);
            printf("  Record Type: ");
            switch(nvCsi->recordType)
            {
            case CSI_EXCEPTION_RECORD:
                for (j=0; j<3; ++j)
                {
                    if (nvCsi->troubleTask[0].excReg.mpFsr[j] != 0)
                    {
                        break;
                    }
                }
                if (j<3)
                {
                    printf("%sEXCEPTION", csiExcString(nvCsi->troubleTask[0].excReg.mpFsr[j]));
                    printf(" dar=0x%08x\n", nvCsi->troubleTask[0].excReg.mpFar[j]);
                }
                else
                {
                    printf("UNKNOWN EXCEPTION");
                }
                printf("  Task Name: %s\n", nvCsi->troubleTask[0].task.name);
                break;
            case CSI_WATCHDOGTASK_RECORD:
                printf("WATCHDOG\n");
                count = nvCsi->contextSwitchHistory.contextCount;
                if (count < SIZEOF(nvCsi->contextSwitchHistory.newContext))
                {
                    printf("  Task Name: %s\n", nvCsi->troubleTask[0].task.name);
                }
                break;
            case CSI_STACKCHECK_RECORD:
                printf("STACKCHECK\n");
                printf("  Task Name: %s\n", nvCsi->troubleTask[0].task.name);
                break;
            case CSI_REBOOTHOOK_RECORD:
                printf("REBOOT\n");
                break;
            case CSI_DEADLOCK_RECORD:
                printf("DEADLOCK\n");
                break;
            case CSI_CRITICALALARM_RECORD:
                printf("CRITICAL ALARM[%x]\n", nvCsi->alarmInfo.code);
                break;
            case CSI_CRITICALASSERTION_RECORD:
                printf("CRITICAL ASSERTION[%04x]\n", nvCsi->alarmInfo.code & ~0x4000);
                break;
            case CSI_SW_MEMORY_LEAK:
                printf("MEMORY LEAK\n");
                break;
            case CSI_ABORT_RECORD:
                printf("ABORT REBOOT\n");
                break;
            default:
                printf("UNKNOWN\n");
                break;
            }
        }
    }

    printf("===================================\n");
    printf("\n");

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

    printf("\n");
    printf("  CSI Deadlock Task IDs\n");
    printf("=========================\n");

    for (unsigned int i = 0; i < SIZEOF(DeadlockMonTaskInfo); i++)
    {
        tid = (int)DeadlockMonTaskInfo[i].deadlockTid;

        if (tid != 0)
        {
            printf("0x%08x  %s\n", tid, TSK_getname((TSK_Handle)tid));
        }
    }

    printf("\n");
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
                   tid, TSK_getname((TSK_Handle)tid), DeadlockMonTaskInfo[i].maxBlockedTime,
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
    return csiHelp();
}

extern "C" STATUS csiClear(void)
{
    CSI* csiObj = CSI::GetInstance();
    if (csiObj)
    {
        return CSI::GetInstance()->ClearImage();
    }
    else
    {
        printf( " CSI not started \n");
    }

    return OK;
}

extern "C" STATUS csiDisable(void)
{
    CSI* csiObj = CSI::GetInstance();
    if (csiObj)
    {
        CSI::GetInstance()->RestoreExceptionISRs();
    }
    else
    {
        printf( " CSI not started \n");
    }
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

extern "C" bool csiIsEnabled(void)
{
    return (bool)CSI::GetInstance()->GetStatus();
}

extern "C" int uploadCSIfileToL3(const char *filename)
{
    return 0;
}

extern "C" STATUS csiShow(int record)
{
    STATUS rc = OK;
   
    if(-1 != record)
    {
        CSI::GetInstance()->ShowHdrAndCsiRecordSummary();
        CSI::GetInstance()->ShowLog();
        CSI::GetInstance()->ShowCsiRecord(record);

       // display current registered deadlock monitor task list
        CSI::GetInstance()->ShowDeadlockTaskList();  
    }
    else
    {
        rc |= CSI::GetInstance()->ShowHdrAndCsiRecordSummary();
        rc |= CSI::GetInstance()->ShowLog();
        rc |= CSI::GetInstance()->ShowCsiRecord((unsigned int)-1);
    }

    uploadCSIfileToL3(FILENAME);
    return rc;
}

STATUS csiInit(void)
{
    if (CSI::GetInstance() == NULL)
    {
        CSI *csiObj = new CSI;
    }

//    _csiWvrInit();


    return OK;
}

STATUS csiTerm(void)
{
    return OK;

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

extern "C"
void contextSwith(TSK_Handle pOldTcb, TSK_Handle pNewTcb)
{
    if (CSI::GetInstance() != NULL)
    {
        CSI::GetInstance()->ContextSwitchHandler(pOldTcb, pNewTcb);
    }
}

extern "C"
void deadLockCheck()
{
    if (CSI::GetInstance() != NULL)
    {
        CSI::GetInstance()->DeadlockCheck(CSI::GetInstance());
    }
}

extern "C"
void csiRegTask(TSK_Handle tsk)
{
    int i;

    for (i=0; i<tskArrayIndex; ++i)
    {
        if ((UINT32)tskArray[i] == (UINT32)tsk)
        {
            break;
        }
    }

    if (i >= tskArrayIndex && tskArrayIndex < MAX_TASK_NUM)
    {
        tskArray[tskArrayIndex++] = tsk;
    }
}

extern "C"
void abortReboot(char *fileName, UINT32 lineNum)
{
    if (CSI::GetInstance() != NULL)
    {
        CSI::GetInstance()->AbortRecord(fileName, lineNum);
    }
}

extern "C"
void csiExcTest()
{
    volatile UINT32 a = *((UINT32*)0x5678ABCD);
}

extern "C"
void csiDeadLockTest()
{
    IER = 0x0010;
    ICR = 0xFFFF;
    while(1);
}

extern "C"
void csiStackCheckTest(int a)
{
    //testXXXXX = a;
}

extern "C"
void csiAbortTest()
{
    abortReboot(__FILE__, __LINE__);
}

