/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: L3TaskDiag.h
 *
 * DESCRIPTION:
 *     Declaration of the L3 Diagnostic task, accept commands from shell, send 
 *     diag messages to each CPU's diag agent and display the result
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 03/04/2006  Yushu Shi   Initial file creation. 
 *---------------------------------------------------------------------------*/


#ifndef __DIAG_TASK_INC__
#define __DIAG_TASK_INC__

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#include "TaskDef.h"
#include "BtsConfig.h"
#include "DiagDef.h"
#include "Timer.h"
#include "logArea.h"
#include "log.h"

#include <semLib.h>
#include <taskLib.h>


//UT Bridge任务参数定义
#define M_TASK_L3DIAG_TASKNAME      "tL3Diag"
#define M_TASK_L3DIAG_OPTION        ( VX_FP_TASK )
#define M_TASK_L3DIAG_MSGOPTION     ( MSG_Q_FIFO )
#define M_TASK_L3DIAG_STACKSIZE     ( 4096 )
#define M_TASK_L3DIAG_MAXMSG        ( 20 )

#define M_DIAG_DEBUG_INFO_CODE      LOGNO(DIAG, 0)
#define M_DIAG_INVALID_CMD          LOGNO(DIAG, 1)

#define MSGID_DIAG_TIME_OUT         0x3900
#define SERIAL_NUM_GET_REQ         0x0710//CM转发邋ems请求
#define SERIAL_NUM_GET_RSP         0x0711
#define MSGID_DIAG_SERIAL_NUM_TIME_OUT         0x3901

#define MSGID_DIAG_1MIN_TIME_OUT         0x3902  //lijinan 20081013
#define MSGID_DIAG_RPC_CFG 0x0065 //ems配置,将snr和ranging offset发给rpc
#ifndef WBBU_CODE
const UINT8 M_NUM_OF_CPU =  (M_NumberOfDSP + M_NumberOfFEP + 1);   // +1 for L2, no need to wait for L3's response 
#else
const UINT8 M_NUM_OF_CPU =  (M_NumberOfDSP + M_NumberOfFEP + 1+1);   // +1 for L2, no need to wait for L3's response 1-core9
#define MAX_DSP_NUM 5
#endif
typedef enum
{
    CPU_NAME_L3PPC = 1000,
    CPU_NAME_L2PPC,
    CPU_NAME_AUX,
    CPU_NAME_MCP0,
    CPU_NAME_MCP1,
    CPU_NAME_MCP2,
    CPU_NAME_MCP3,
    CPU_NAME_MCP4,
    CPU_NAME_MCP5,
    CPU_NAME_MCP6,
    CPU_NAME_MCP7,
    CPU_NAME_FEP0,
    CPU_NAME_FEP1,
    CPU_NAME_ALL,
    CPU_NAME_MCPALL,
    CPU_NAME_FEPALL,
#ifdef WBBU_CODE
    CPU_NAME_CORE9//wangwenhua add 20090908
#endif
}DIAG_CPU_NAME;


typedef enum
{
    CPU_INDEX_L2 = 0,
    CPU_INDEX_AUX,
    CPU_INDEX_MCP0,
    CPU_INDEX_MCP1,
    CPU_INDEX_MCP2,
    CPU_INDEX_MCP3,
    CPU_INDEX_MCP4,
    CPU_INDEX_MCP5,
    CPU_INDEX_MCP6,
    CPU_INDEX_MCP7,
    CPU_INDEX_FEP0,
    CPU_INDEX_FEP1,
#ifdef WBBU_CODE
    CPU_INDEX_CORE9 = 12,
#endif
    CPU_INDEX_MAX
}DIAG_CPU_INDEX;

typedef void (*SEND_DIAG_CMD_FUNCPTR) (DIAG_CPU_INDEX, int, int, int, int);	/* pfunction returning int */
       
typedef struct
{
    DIAG_CPU_TYPE  CpuType;
    UINT16         CpuIndex;
    char           CpuName[8];
}T_DiagCpuInfo;

typedef enum
{
    DIAG_SHELL_CMD_PROBE = 2000,
    DIAG_SHELL_CMD_MEM_PEEK,
    DIAG_SHELL_CMD_MEM_POKE,
    DIAG_SHELL_CMD_VER_QUERY,
    DIAG_SHELL_CMD_STATE_QUERY,
    DIAG_SHELL_CMD_PRC,
    DIAG_SHELL_CMD_HELP
}DIAG_SHELL_CMD;


typedef struct 
{
    DIAG_CPU_NAME    cpuName;
    DIAG_SHELL_CMD   diagCmd;
    int              arg1;
    int              arg2;
    int              arg3;
    int              arg4;
}T_DiagShellCommand;
typedef struct 
    {
         UINT16 TranId;
         UINT16 result;
         UINT8 l3_serialnum[20];// 基带板序列号 
         UINT8 syn_RF_num[180];
    }T_SerialNumRsp;
#ifdef WBBU_CODE
typedef struct
{
      UINT32  VersionNoRsp;
      UINT32  Rest_Time;
}T_DSPErrCount;
#endif
class CL3TaskDiag : public CBizTask
{
public:
    static CL3TaskDiag* GetInstance();
    inline TID GetEntityId() const  { return  M_TID_DIAGM;}
    bool   DiagCommandParse(DIAG_SHELL_CMD, DIAG_CPU_NAME, int arg1, int arg2, int arg3, int arg4);
    #ifdef WBBU_CODE

   bool DiagCommandParse_MAC(DIAG_SHELL_CMD, DIAG_CPU_NAME, int arg1, int arg2, int arg3, int arg4);
    #endif
    UINT16 GetDiagCommandTransactionID() { return DiagCommandTransactionID; }
    void GetSynSerialNum();
    UINT16 SetWaitingFlag(DIAG_CPU_INDEX cpuIndex)
    {
        WaitingForCpuResponse[cpuIndex] = true;
    }

private:
    CL3TaskDiag();
    ~CL3TaskDiag();

    bool Initialize();
    bool ProcessComMessage(CComMessage*);
    inline bool IsNeedTransaction() { return false;}

    #define DIAG_MAX_BLOCKED_TIME_IN_10ms_TICK (500)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return DIAG_MAX_BLOCKED_TIME_IN_10ms_TICK ;};


    void ProcessShellCommand(CComMessage *);
    void ProcessMemoryPeekResponse(CComMessage *);
    void ProcessMemoryPokeResponse(CComMessage *);
    void ProcessSwVerQueryResponse(CComMessage *);
    void ProcessStatusQueryResponse(CComMessage *);
    void ProcessCpuPingResponse(CComMessage *);
    void ProcessRpcResponse(CComMessage *);
    void ProcessInfoDisplay(CComMessage *);
    void ProcessTimeOut();
    int Process1MinTimeOut();//lijinan 20081013
    void CommandCompleteCheck(DIAG_CPU_INDEX);
    void ProcessSerialNumReq(CComMessage *);
    void ProcessSerialNumTimeout();
    void processRpcCfg(CComMessage *);
    void PostDiagMsg(T_DiagShellCommand *diagCmd, SEND_DIAG_CMD_FUNCPTR sendFunc);

    static void PostDiagGeneralMsg(DIAG_CPU_INDEX cpuIndex, UINT16 msgId);
    static void PostDiagProbeMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
    static void PostDiagMemoryPeekMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
    static void PostDiagMemoryPokeMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
    static void PostDiagVerQueryMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
    static void PostDiagStatusQueryMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
    static void PostDiagRpcMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
    #ifdef WBBU_CODE
          void ProcessShellCommand_MAC(CComMessage *);
          static void PostDiagGeneralMsg_MAC(DIAG_CPU_INDEX cpuIndex, UINT16 msgId);
	    static void PostDiagProbeMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
	    static void PostDiagMemoryPeekMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
	    static void PostDiagMemoryPokeMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
	    static void PostDiagVerQueryMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
	    static void PostDiagStatusQueryMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
	    static void PostDiagRpcMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4);
	    void ProcessDebugMsg(CComMessage *);

    #endif


    static CL3TaskDiag *Instance;
    //SEM_ID             ShellCmdSem;
    //SEM_ID             ShellCmdCompleteSem;
    UINT16             DiagCommandTransactionID;
    bool               WaitingForCpuResponse[M_NUM_OF_CPU];
    UINT16           CpuNoResponseCount[M_NUM_OF_CPU];//lijinan 20081013
    UINT16           OneMinQuryVerFlag;
    CTimer             *DiagResponseTimer;
    CTimer  *serialNumReqTimer;
};


#endif

