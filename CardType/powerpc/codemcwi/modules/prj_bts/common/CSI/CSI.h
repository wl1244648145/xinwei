/*******************************************************************
*
*    DESCRIPTION: CSI.h - This file contains the Crash Scene Trace class.
*
*    AUTHOR: Yushu Shi
*
*    HISTORY:
*
*    DATE: 1/8/2006
*
*******************************************************************/
#ifndef __CSI_H__
#define __CSI_H__

#include <vxworks.h>
#include <wvLib.h>    
#include <taskLib.h>
#include <sysLib.h>
#include <arch/ppc/esfppc.h>
#include <arch/ppc/excppclib.h>
#include <taskHookLib.h>
#include <semLib.h>
#include <stdio.h>

#include <arch/ppc/fppPpcLib.h>
#include <arch/ppc/altivecPpcLib.h>

#include "dataType.h"
#include "logArea.h"

#ifdef M_TGT_L3
#include "sysBtsConfigData.h"
#include "MemOpReload.h"
#else
#include "sysL2BspCsi.h"
#endif

#define CSI_BSP_VERSION_SIZE            64
#define CSI_NVRAM_LATEST_VERSION        2

#define CSI_NVRAM_CSI_SAFE_PATTERN      "Mcwill CSI NVRAM Initialized" 
#define CSI_NVRAM_MAX_LOG_COUNT         32
#ifdef M_TGT_L3
#define CSI_NVRAM_MAX_CSI_COUNT         6 
#else
#define CSI_NVRAM_MAX_CSI_COUNT         4
#endif

#define CSI_NVRAM_ALL_CSI_RECORDS      -1
#define CSI_NVRAM_LAST_CSI_RECORD       0

#define CSI_IDLE_MIN_IDLE_COUNT         1
#define CSI_IDLE_MAX_BUSY_TIME          (SecondsToTicks(/*60*/240*3)/(CSI_SW_WDT_DELAY_IN_TICK))   /* in the uit of CSI_SW_WDT_DELAY_IN_TICK  wangwenhua change from 60 to 120*/

#define CSI_MAX_CONTEXT_SWITCH          32
#ifdef M_TGT_L3
#define CSI_MAX_TASK_SUMMARY            64
#define CSI_MAX_TASK_TRACE              32
#else
#define CSI_MAX_TASK_SUMMARY            32
#define CSI_MAX_TASK_TRACE              16
#endif

#define CSI_MAX_STACK_SIZE              512
#define CSI_STACKCHECK_MARGIN           512
#define CSI_STACKCHECK_INITIAL_DELAY    SecondsToTicks(5)
#define CSI_STACKCHECK_DELAY_IN_SECOND  60  // 60 seconds
#define CSI_SW_WDT_DELAY_IN_TICK        10   /* 10*10ms= 100ms */

#define CSI_TRACE_MAX_DEPTH             32
#define CSI_NAME_LEN                    11

#define CSI_MAX_DEADLOCK_TIDS_LOGGED    4
#define CSI_MAX_DEADLOCK_TIDS           CSI_MAX_TASK_TRACE
#define CSI_MAX_RESOURCE_TRACE          4

#ifdef M_TGT_L3
#define CSI_IMAGE_ADDRESS       NVRAM_CSI_BASE
#define CSI_IMAGE_SIZE          NVRAM_CSI_SIZE
//csi扩充部分
#define CSI_IMAGE_ADDRESS_NEW       NVRAM_CSI_BASE_NEW
#define CSI_IMAGE_SIZE_NEW          NVRAM_CSI_SIZE_NEW
#else
#define CSI_IMAGE_ADDRESS       MV_SRAM_CSI_IMAGE_ADRS
#define CSI_IMAGE_SIZE          MV_SRAM_CSI_IMAGE_SIZE
#endif

#define CSI_MEM_STATS_SAFE_PATTERN      0x12345678

#define M_CSI_TASK_OPTION        ( VX_FP_TASK )

#define PACKED __attribute__ ((packed))

typedef enum
{
    CSI_EMPTY_RECORD = 0,
    CSI_CLEAR_RECORD,
    CSI_START_RECORD,
    CSI_REBOOTHOOK_RECORD,
    CSI_EXCEPTION_RECORD,
    CSI_WATCHDOGTASK_RECORD,
    CSI_STACKCHECK_RECORD,
    CSI_DEADLOCK_RECORD,
    CSI_CRITICALALARM_RECORD,
    CSI_CRITICALASSERTION_RECORD,
    CSI_SW_MEMORY_LEAK,
	CSI_TASK_DEAD_RECORD,
    CSI_ABORT_RECORD
} CSI_RECORD_TYPE;

typedef enum
{
    CSI_TASK_PEND_UNKNOWN = 0,
    CSI_TASK_PEND_NONE,   // not in a pending state
    CSI_TASK_PEND_MSG,
    CSI_TASK_PEND_SEM,
    CSI_TASK_PEND_EVENT,
    CSI_TASK_PEND_SELECT
}CSI_TASK_PEND_REASON;

typedef struct
{
    int                  tid;
    CSI_TASK_PEND_REASON reason;
    int                  pendResourceId;
    int                  ResourceHoldingTid;
}CSI_RESOURCE_TRACE;

typedef struct
{
    UINT32              resourceTraceDepth;
    CSI_RESOURCE_TRACE  resourceTrace[CSI_MAX_RESOURCE_TRACE];
}CSI_DEADLOCK_TRACE;



typedef struct {
    int     tid;
    UINT32  pc;
    INT8    name[CSI_NAME_LEN+1];
} PACKED CSI_TASK_INFO;

typedef struct {
    int     tid;
    UINT32  pc;
} PACKED CSI_CONTEXT_INFO;


typedef struct
{
    UINT32  addr1;
    UINT32  addr2;
} PACKED CSI_TRACE_INFO;

typedef struct
{
    UINT32 entry;
    UINT32 arg;
}PACKED CSI_TRACE_ARG_INFO;

typedef struct {
    int     tid;
    int	    taskStatus;	

    UINT32  priority;
    UINT32  pc;
    UINT32  errorStatus;

    UINT32  sp;
    UINT32  base;
    int     size;
    int     high;
    int     margin;
    INT8    name[CSI_NAME_LEN+1];

} PACKED CSI_TASK_STATUS;

typedef struct {
    int     tid;
    UINT32              traceCount;
    CSI_TRACE_INFO      trace[CSI_TRACE_MAX_DEPTH];
} PACKED CSI_TASK_TRACE;


typedef struct {
    CSI_TASK_INFO       task;
    ESFPPC              esf;

    BOOL                fppCtxIsValid;
    FP_CONTEXT          fppCtx;
    BOOL                vpuCtxIsValid;
    ALTIVEC_CONTEXT     vpuCtx;
} PACKED CSI_TASK_RECORD;



typedef struct
{
    char        nvRamSafePattern[128];
    UINT32      version;
    #ifdef      M_TGT_L3
    UINT32      btsID;
    #endif

    UINT32         nextCsiRecordIndex;
    UINT32         nextLogRecordIndex;
    UINT32         startCount;         // BTS start count since CSI image initialization
    UINT32         rebootCount;
    UINT32         crashCount;
    T_TimeDate     initTimeStamp;
    T_TimeDate     bootTimeStamp;   // last time BTS boot time
} PACKED CSI_NVRAM_HEADER;


typedef struct
{
    CSI_RECORD_TYPE     recordType;
    T_TimeDate          timeStamp;
    UINT32              csiRecordIndex;
    RESET_REASON        resetReason;
    char                loadVersion[CSI_BSP_VERSION_SIZE];
} PACKED CSI_NVRAM_LOG;



typedef struct 
{
    UINT32              version;
    bool                valid;
    CSI_RECORD_TYPE     recordType;

    // Software Version
    char                loadVersion[CSI_BSP_VERSION_SIZE];

    // Timestamp
    T_TimeDate             rtc;

    CSI_TASK_RECORD     troubleTask[CSI_MAX_DEADLOCK_TIDS_LOGGED];
    
    // Task Info
    UINT32              taskCount;
    CSI_TASK_STATUS     taskStatus[CSI_MAX_TASK_SUMMARY];

    UINT32              traceTaskCount;
    CSI_TASK_TRACE      taskTrace[CSI_MAX_TASK_TRACE];

    CSI_DEADLOCK_TRACE  deadlockTrace[CSI_MAX_DEADLOCK_TIDS_LOGGED];

    UINT32              rawStack[CSI_MAX_STACK_SIZE / sizeof(UINT32)];

    UINT32              memHeapStatsSafePattern;
    MEM_PART_STATS      memHeapStats;
    #ifdef M_TGT_L2
    UINT8               fileName[256];//record abort liwei 110427
    UINT32              lineNum;
    #endif

    struct 
    {
        // Context Switch Info
        UINT32            contextCount;
        CSI_CONTEXT_INFO  oldContext[CSI_MAX_CONTEXT_SWITCH];
        CSI_CONTEXT_INFO  newContext[CSI_MAX_CONTEXT_SWITCH];
    } contextSwitchHistory;

    #ifdef M_TGT_L3
    struct usrMemPart MemoryUsage[M_MAX_TASK_NUM];
    #endif

    struct 
    {
        // Critical Alarm Info
        UINT16              code;
        UINT16              entity;
        UINT16              instance;
    } alarmInfo;

    RESET_REASON            resetReason;
} PACKED CSI_CRITICAL_DATA;



typedef struct
{
    CSI_NVRAM_HEADER    nvHdr;
    CSI_NVRAM_LOG       nvLog[CSI_NVRAM_MAX_LOG_COUNT];
    CSI_CRITICAL_DATA   nvCsi[CSI_NVRAM_MAX_CSI_COUNT];
} PACKED CSI_IMAGE;
 
typedef struct
{
    FUNCPTR reset;
    FUNCPTR mach;
    FUNCPTR data;
    FUNCPTR inst;
    FUNCPTR align;
    FUNCPTR prog;
    FUNCPTR fpu;
    FUNCPTR res1;
    FUNCPTR res2;
    FUNCPTR syscall;
    FUNCPTR trace;
    FUNCPTR res3;

    FUNCPTR pm;
    FUNCPTR inst_bkpt;
    FUNCPTR smi;
} CSI_VEC_TBL;


typedef struct 
{
    UINT32 deadlockTid;      
    UINT32 switchCount;      
    UINT32 maxBlockedTime;   
    UINT32 remainBlockedTime;
}DEADLOCK_MON_TASK_INFO;

typedef struct // added for suspended task detection
{
    int  tid;
    int  TTL;
}CSI_SUSPENDED_TASK_ENTRY;
#define CSI_SUSPEND_TASK_NUM 4
#ifdef M_TGT_L3
//csi扩展部分
typedef struct
{
    UINT8 fileName[256];//record abort liwei 110427
    UINT32 lineNum;
    char rev[4096];//为了以后扩展使用
}CSI_DATA_RECORD_NEW;
typedef struct
{
    char nvRamSafeHead[128];
    CSI_DATA_RECORD_NEW csi_record_new[CSI_NVRAM_MAX_CSI_COUNT];
    char nvRamSafeTail[128];
    
}PACKED CSI_IMAGE_NEW;
#endif
class CSI
{
public:
    CSI();
    ~CSI();

    static CSI* GetInstance(void);
    bool        GetStatus(void) {return enabled;}
    STATUS      MonitorTaskDeadlock(UINT32 tid, UINT32 maxBlockedTimeInMSecs = 60);
    STATUS      RegisterTaskStackTrace(UINT32 tid);
    void        DisableDeadlockCheck(UINT32 tid);

    STATUS      ClearImage();
    STATUS      ShowCsiRecord(unsigned int record);
    STATUS      ShowLog();
    STATUS      ShowHdrAndCsiRecordSummary();
    STATUS      ShowResourceTrace(CSI_CRITICAL_DATA *);
    STATUS      ShowDeadlockTaskList();

	STATUS      IdlePerf(void);
    UINT32      GetIdleCounterAverage() { return currentIdleCountAverage; };
    UINT32      GetIdleCounterHigh()    { return idleCountHighWM; };
    UINT32      GetIdleCounterLow()     { return  idleCountLowWM; };
    UINT32      GetIdleCounterMax()     { return  idleCountMax; };

    void        SaveRebootAlarmInfo(UINT16 almCode, UINT16 almEntity, UINT16 almInst);
    void        RestoreExceptionISRs(void);

    void        ShowDeadlockTaskInfo();
    void        HookExceptionISRs(void);
    void        ShowSuspendedTaskInfo();    
    void         AbortRecord(char *fileName, int lineNum);
   
private:
    void         SaveRebootRecord();
    void         EnterShutdownFailSafeMode(void);
    inline void  IncrementIdleCountThisPass(){idleCountThisPass[idleCountPassIndex]++;};

    static void  CsiIsrHandler(ESFPPC *pEsf);
           void _CsiIsrHandler(ESFPPC *pEsf);
    static void  ContextSwitchHandler(WIND_TCB *pOldTcb, WIND_TCB *pNewTcb);
    static int   WdtExceptionHandler(ESFPPC *pEsf);
    static void  trcPrint(UINT32 addr1, UINT32 addr2, UINT32 numArgs, UINT32 *args);
           void _trcPrint(UINT32 addr1, UINT32 addr2, UINT32 numArgs, UINT32 *args);

    static void  detailTrcPrint(UINT32 addr1, UINT32 addr2, UINT32 numArgs, UINT32 *args);
           void _detailTrcPrint(UINT32 addr1, UINT32 addr2, UINT32 numArgs, UINT32 *args);

    static void  CpuUsageSupervisor(CSI *csi);   // middle priority task housekeeping the idle counters periodically
    static void  DeadlockCheck(CSI *csi);
    static void  PeriodCheckTask(CSI *csi);
    static void  IdleTask();  // lowest priority task that keeps the idle count

           void  GetTaskInfo();
           void  DeadlockResourceTrace();
           int   TraceOneTaskPendingResource(int tid, int entryIndex);
           void  UpdateCsiRecord(CSI_CRITICAL_DATA *);
           void  GetTaskStackTrace(int taskIndex);

    // Task IDs
    int idleTaskId;
    int idleTaskSupervisorId;
    int periodicCheckTaskId;

    static bool     idleTaskEnabled;

    // Idle Counters
    UINT32  idleCounter;
    UINT32  prevCounter;
    UINT32  busyCounter;
    UINT32  lastIdleCountIndex;
    UINT32  lastPassIdleCount;

    UINT32  idleCountMax;
    UINT32  idleCountHighWM;
    UINT32  idleCountLowWM;
    UINT32  idleCountThisPass[2];
    UINT32  idleCountPassIndex;
    UINT32  currentIdleCountAverage;

    static CSI_SUSPENDED_TASK_ENTRY  suspendedTaskTable[CSI_SUSPEND_TASK_NUM];

    // Outage Footprint data
    static CSI_CRITICAL_DATA criticalData;

    CSI_VEC_TBL   oldVecs;
    static CSI*   Instance;
    bool          enabled;
    static bool   shutdown;


    // Deadlock Detection data 
    static UINT8  deadTidCount;
    static DEADLOCK_MON_TASK_INFO DeadlockMonTaskInfo[CSI_MAX_DEADLOCK_TIDS];

    UINT32             traceBufferSize;
    CSI_TRACE_INFO     *traceBuffer;
    UINT32             *traceCount;

    UINT32             deadlockTraceCount;
    CSI_TRACE_ARG_INFO deadlockTraceBuffer[CSI_TRACE_MAX_DEPTH];

    CSI_NVRAM_LOG*     GetNextLogRecord();
    void               InitImage(CSI_IMAGE* nvImage, T_TimeDate* initTimeStamp);
    bool               StackCheck(int &id);

    static void        ShutdownHook();

    char*              GetTaskNameFromTid(CSI_CRITICAL_DATA *nvOfp, int tid);
    #ifdef M_TGT_L3
    void InitImageNew();
    #endif
};


extern STATUS csiInit(void);
extern STATUS csiTerm(void);

extern "C"
{
extern STATUS csiDisable(void);
extern STATUS csiEnable(void);
extern BOOL   csiIsEnabled(void);
}

#define LOG_ERR_CSI_INIT               LOGNO(CSI, 0)
#define LOG_ERR_CSI_NVRAM_TOO_SMALL    LOGNO(CSI, 1)
#define LOG_ERR_CSI_TASKHOOKADD_ERROR  LOGNO(CSI, 2)
#define LOG_ERR_CSI_TASKSPAWN_ERROR    LOGNO(CSI, 3)
#define LOG_ERR_CSI_DEADLOOP_MON_REG   LOGNO(CSI, 4)
#define LOG_ERR_CSI_TRACE_REG          LOGNO(CSI, 5)
#define LOG_ERR_CSI_INVALID_IMAGE      LOGNO(CSI, 6)
#define LOG_ERR_CSI_INVALID_RECORD     LOGNO(CSI, 7)


#define DM_MAX_BLOCKED_TIME_IN_10ms_TICK (200)
        

#endif   /* <__CSI_H__> */
