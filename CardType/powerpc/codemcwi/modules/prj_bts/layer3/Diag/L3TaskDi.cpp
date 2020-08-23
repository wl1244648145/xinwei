/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: L3TaskDiag.cpp
 *
 * DESCRIPTION:
 *     Implementation of the L3 Diagnostic task, accept commands from shell, send 
 *     diag messages to each CPU's diag agent and display the result
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 03/04/2006  Yushu Shi   Initial file creation. 
 *---------------------------------------------------------------------------*/
#include "L3TaskDiag.h"

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif
#include  <stdio.h>
#include <iostream.h>
#ifndef WBBU_CODE
#include "loadVersion.h"
#else
#include "loadVersion_WBBU.h"
#endif
#ifndef __BTS_DSPCDI_H__
#include "Dspcdi.h"
#endif
#ifdef WBBU_CODE
#include "L3OamAlmInfo.h"
extern   unsigned int    g_ResetDSP1_NOUser ;
#endif

extern void L2AirLinkCfgRandomBCH();
extern void L2AirLinkCfg();

CL3TaskDiag* CL3TaskDiag::Instance = NULL;
extern "C" 
{
DIAG_CPU_NAME  l3    = CPU_NAME_L3PPC;
DIAG_CPU_NAME  l2    = CPU_NAME_L2PPC;
DIAG_CPU_NAME  aux   = CPU_NAME_AUX;
DIAG_CPU_NAME  mcp0  = CPU_NAME_MCP0;
DIAG_CPU_NAME  mcp1  = CPU_NAME_MCP1;
DIAG_CPU_NAME  mcp2  = CPU_NAME_MCP2;
DIAG_CPU_NAME  mcp3  = CPU_NAME_MCP3;
DIAG_CPU_NAME  mcp4  = CPU_NAME_MCP4;
DIAG_CPU_NAME  mcp5  = CPU_NAME_MCP5;
DIAG_CPU_NAME  mcp6  = CPU_NAME_MCP6;
DIAG_CPU_NAME  mcp7  = CPU_NAME_MCP7;
DIAG_CPU_NAME  fep0  = CPU_NAME_FEP0;
DIAG_CPU_NAME  fep1  = CPU_NAME_FEP1;
DIAG_CPU_NAME  all   = CPU_NAME_ALL;
DIAG_CPU_NAME  mcpall = CPU_NAME_MCPALL;
DIAG_CPU_NAME  fepall = CPU_NAME_FEPALL;
#ifdef WBBU_CODE
DIAG_CPU_NAME  core9 = CPU_NAME_CORE9;//
#endif
DIAG_SHELL_CMD probe = DIAG_SHELL_CMD_PROBE;
DIAG_SHELL_CMD peek  = DIAG_SHELL_CMD_MEM_PEEK;
DIAG_SHELL_CMD poke  = DIAG_SHELL_CMD_MEM_POKE;
DIAG_SHELL_CMD ver   = DIAG_SHELL_CMD_VER_QUERY;
DIAG_SHELL_CMD stats = DIAG_SHELL_CMD_STATE_QUERY;
DIAG_SHELL_CMD rpc   = DIAG_SHELL_CMD_PRC;
DIAG_SHELL_CMD hlp   = DIAG_SHELL_CMD_HELP;
}

extern "C"
{
int rebootMCP(int);
int rebootAUX();
int rebootFEP(int);
}
//lijinan 20081013

SEM_ID             ShellCmdSem;
SEM_ID             ShellCmdCompleteSem;
#ifdef WBBU_CODE
 extern "C"  void   Reset_All_DSP();
 extern "C" unsigned char  Reset_Dsp(unsigned char index,unsigned char flag);
 extern "C" void Send_Data_Cfg_2_CM( UINT16 MsgId);
extern "C" unsigned int Read_Fpga_Version_Soft();
 extern bool AlarmReport(UINT8   Flag, UINT16  EntityType,
                 UINT16  EntityIndex, UINT16  AlarmCode,      
                 UINT8   Severity, const char format[],...);
 
extern "C" void L3_L2_Ether_Packet(unsigned char* ptr,unsigned short len);
  extern "C"  unsigned char  IfDSP2Protect();
  unsigned char g_dsp2_protect = 0;
extern "C"  void  SetDsp4SaveInfo();
  extern "C" void ResetAifSerdies();
  extern "C" void  Reset_ALL();
unsigned char g_send_2_DSP4 = 1;//是否需要通知dsp4保存AIF的信息当基站起来后，先发送一次。
//如果中途复位了DSP4，则需要重新配置，配置的前提是取到了版本后进行配置 
#endif
const T_DiagCpuInfo CpuInfo[CPU_INDEX_MAX] = 
{ 
    {CPU_L2,  0, "L2 PPC"},
    {CPU_AUX, 0, "AUX"},
    {CPU_MCP, 0, "MCP 0"},
    {CPU_MCP, 1, "MCP 1"},
    {CPU_MCP, 2, "MCP 2"},
    {CPU_MCP, 3, "MCP 3"},
    {CPU_MCP, 4, "MCP 4"},
    {CPU_MCP, 5, "MCP 5"},
    {CPU_MCP, 6, "MCP 6"},
    {CPU_MCP, 7, "MCP 7"},
    {CPU_FEP, 0, "FEP 0"},
#ifndef WBBU_CODE
    {CPU_FEP, 1, "FEP 1"}
#else
    {CPU_FEP, 1, "FEP 1"},
    {CPU_CORE9, 0 ,"CORE9"}
#endif
};
#ifdef WBBU_CODE
T_DSPErrCount     DSP_Err[CPU_INDEX_MAX];
unsigned int          DSP_Reset_Time[MAX_DSP_NUM];
unsigned char Need_Load_Cfg = 0;
int randombch = 0; //liuweidong send random bch info to l1 while get dsp version no response 
#endif
T_SerialNumRsp serialNumRsp;
         
CL3TaskDiag::CL3TaskDiag()
{
    strcpy( m_szName, M_TASK_L3DIAG_TASKNAME);
    m_uPriority     = M_TP_L3DIAGM;
    m_uOptions      = M_TASK_L3DIAG_OPTION;
    m_uStackSize    = M_TASK_L3DIAG_STACKSIZE+1024;

    m_iMsgQMax      = M_TASK_L3DIAG_MAXMSG;
    m_iMsgQOption   = M_TASK_L3DIAG_MSGOPTION;

    ShellCmdSem = NULL;
    DiagCommandTransactionID = 0;
    OneMinQuryVerFlag = false;
    for (int i=0; i<M_NUM_OF_CPU  ; i++)
    {
        WaitingForCpuResponse[i] = false;
	 CpuNoResponseCount[i] = 0;//lijinan 20081013
    }
#ifdef WBBU_CODE
    for(int j =0 ; j< 12; j++)
    	{
    	   DSP_Err[j].Rest_Time = 0;
    	   DSP_Err[j].VersionNoRsp = 0;
    	}
    for(int k = 0; k< 5; k++)
    	{
    	    DSP_Reset_Time[k] = 0;
    	}
#endif
    DiagResponseTimer = NULL;
    ShellCmdCompleteSem = NULL;
}


CL3TaskDiag* CL3TaskDiag::GetInstance()
{
    if (NULL == Instance)
    {
        Instance = new CL3TaskDiag;
    }
    return Instance;
}


bool CL3TaskDiag::Initialize()
{
    // create the mutex semaphore to allow only one shell command in process
    ShellCmdSem = semBCreate(SEM_Q_FIFO, SEM_FULL);
    ShellCmdCompleteSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);   // initial value of the semaphore

    bool ucInit = CBizTask::Initialize();
    if ( false == ucInit )
    {
        return false;
    }
#ifdef WBBU_CODE
    g_dsp2_protect = IfDSP2Protect();
#endif
   return true;
}

bool CL3TaskDiag::DiagCommandParse(DIAG_SHELL_CMD cmd, DIAG_CPU_NAME cpu,  int arg1, int arg2, int arg3, int arg4)
{
    taskPrioritySet(0, M_TP_DIAG_SHELL);

    if (DIAG_SHELL_CMD_PROBE > cmd || DIAG_SHELL_CMD_HELP < cmd)
    {
        printf( "Unknown Diag shell command\n");
    }

    if ( DIAG_SHELL_CMD_HELP == cmd )
    {
		printf(" Diag Command List: (case sensitive)\n");
        printf("     diag probe, cpu    -- check if the CPU is still alive\n");
        printf("     diag peek,  cpu,  <addr>, <number of 32bit word>   -- memory peek\n");
        printf("     diag poke,  cpu,  <addr>, <value>   -- modify the memory at <addr> to <value>\n");
        printf("     diag ver,   cpu    -- query the software version \n");
        printf("     diag stats, cpu    -- query the current status\n");
        printf("     diag rpc,   cpu,  <rpc function index or address>, [arg1], [arg2] -- invoke a funciton\n\n");
        printf(" CPU Name List: (case sensitive)\n");
        printf("     l3   -- L3 PPC \n");
        printf("     l2   -- L2 PPC \n");
        printf("     aux  -- AUX\n");
        printf("     mcp0 -- MCP 0\n");
        printf("     mcp1 -- MCP 1\n");
        printf("     mcp2 -- MCP 2\n");
        printf("     mcp3 -- MCP 3\n");
        printf("     mcp4 -- MCP 4\n");
        printf("     mcp5 -- MCP 5\n");
        printf("     mcp6 -- MCP 6\n");
        printf("     mcp7 -- MCP 7\n");
        printf("     fep0 -- FEP 0\n");
        printf("     fep1 -- FEP 1\n");
        printf("     all  -- all the CPUs\n");
        printf("     mcpall  -- all the 8 MCPs\n");
        printf("     fepall  -- all the 2 FEPs\n");
#ifdef WBBU_CODE
        printf("     core9----CORE 9\n");
#endif
        return true;
    }
#ifdef WBBU_CODE
    if ( CPU_NAME_L3PPC > cpu || CPU_NAME_CORE9 < cpu)
    {
        printf(" Unknown CPU instance\n");
        return false;
    }
#else

    if ( CPU_NAME_L3PPC > cpu || CPU_NAME_FEPALL < cpu)
    {
        printf(" Unknown CPU instance\n");
        return false;
    }
#endif

    if ( OK == semTake(ShellCmdSem, SecondsToTicks(3)))
    {
        CComMessage *pComMsg = new(this, sizeof(T_DiagShellCommand))CComMessage;
        if ( pComMsg )
        {
            pComMsg->SetDstTid (M_TID_DIAGM);
            pComMsg->SetSrcTid (M_TID_DIAGM);
            pComMsg->SetMessageId( MSGID_DIAG_SHELL_CMD);
            T_DiagShellCommand *cmdPtr = (T_DiagShellCommand*)pComMsg->GetDataPtr();
            cmdPtr->diagCmd = cmd;
            cmdPtr->cpuName = cpu;
            cmdPtr->arg1 = arg1;
            cmdPtr->arg2 = arg2;
            cmdPtr->arg3 = arg3;
            cmdPtr->arg4 = arg4;

            if ( false == CComEntity::PostEntityMessage(pComMsg) )
            {
                pComMsg->Destroy();
                LOG_STR(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, " Failed to execute the diag command, failure in L3 system" );
                semGive(ShellCmdSem);
                return false;
            }
            semTake(ShellCmdCompleteSem, WAIT_FOREVER);  // wait for the command to either complete or time out
        }
    }
    else
    {
        LOG_STR(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, "Failed to execute the diag command,  another diag command is not finished\n");
        return false;
    }
}
#ifdef WBBU_CODE
 bool CL3TaskDiag::DiagCommandParse_MAC(DIAG_SHELL_CMD cmd, DIAG_CPU_NAME cpu,  int arg1, int arg2, int arg3, int arg4)
  {
    taskPrioritySet(0, M_TP_DIAG_SHELL);

    if (DIAG_SHELL_CMD_PROBE > cmd || DIAG_SHELL_CMD_HELP < cmd)
    {
        printf( "Unknown Diag shell command\n");
    }

    if ( DIAG_SHELL_CMD_HELP == cmd )
    {
		printf(" Diag Command List: (case sensitive)\n");
        printf("     diag probe, cpu    -- check if the CPU is still alive\n");
        printf("     diag peek,  cpu,  <addr>, <number of 32bit word>   -- memory peek\n");
        printf("     diag poke,  cpu,  <addr>, <value>   -- modify the memory at <addr> to <value>\n");
        printf("     diag ver,   cpu    -- query the software version \n");
        printf("     diag stats, cpu    -- query the current status\n");
        printf("     diag rpc,   cpu,  <rpc function index or address>, [arg1], [arg2] -- invoke a funciton\n\n");
        printf(" CPU Name List: (case sensitive)\n");
        printf("     l3   -- L3 PPC \n");
        printf("     l2   -- L2 PPC \n");
        printf("     aux  -- AUX\n");
        printf("     mcp0 -- MCP 0\n");
        printf("     mcp1 -- MCP 1\n");
        printf("     mcp2 -- MCP 2\n");
        printf("     mcp3 -- MCP 3\n");
        printf("     mcp4 -- MCP 4\n");
        printf("     mcp5 -- MCP 5\n");
        printf("     mcp6 -- MCP 6\n");
        printf("     mcp7 -- MCP 7\n");
        printf("     fep0 -- FEP 0\n");
        printf("     fep1 -- FEP 1\n");
        printf("     all  -- all the CPUs\n");
        printf("     mcpall  -- all the 8 MCPs\n");
        printf("     fepall  -- all the 2 FEPs\n");
#ifdef WBBU_CODE
        printf("     core9----CORE 9\n");
#endif
        return true;
    }
#ifdef WBBU_CODE
    if ( CPU_NAME_L3PPC > cpu || CPU_NAME_CORE9 < cpu)
    {
        printf(" Unknown CPU instance\n");
        return false;
    }
#else

    if ( CPU_NAME_L3PPC > cpu || CPU_NAME_FEPALL < cpu)
    {
        printf(" Unknown CPU instance\n");
        return false;
    }
#endif

    if ( OK == semTake(ShellCmdSem, SecondsToTicks(3)))
    {
        CComMessage *pComMsg = new(this, sizeof(T_DiagShellCommand))CComMessage;
        if ( pComMsg )
        {
            pComMsg->SetDstTid (M_TID_DIAGM);
            pComMsg->SetSrcTid (M_TID_DIAGM);
            pComMsg->SetMessageId( MSGID_DIAG_SHELL_CMD_MAC);
          //  pComMsg->SetEID(0x87654321);
            T_DiagShellCommand *cmdPtr = (T_DiagShellCommand*)pComMsg->GetDataPtr();
            cmdPtr->diagCmd = cmd;
            cmdPtr->cpuName = cpu;
            cmdPtr->arg1 = arg1;
            cmdPtr->arg2 = arg2;
            cmdPtr->arg3 = arg3;
            cmdPtr->arg4 = arg4;

            if ( false == CComEntity::PostEntityMessage(pComMsg) )
            {
                pComMsg->Destroy();
                LOG_STR(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, " Failed to execute the diag command, failure in L3 system" );
                semGive(ShellCmdSem);
                return false;
            }
            semTake(ShellCmdCompleteSem, WAIT_FOREVER);  // wait for the command to either complete or time out
        }
    }
    else
    {
        LOG_STR(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, "Failed to execute the diag command,  another diag command is not finished\n");
        return false;
    }
}
#endif

bool CL3TaskDiag::ProcessComMessage(CComMessage *pComMsg)
{
    switch( pComMsg->GetMessageId() )
    {
        case MSGID_DIAG_SHELL_CMD:
            ProcessShellCommand(pComMsg);
            break;
        #ifdef WBBU_CODE
        case MSGID_DIAG_SHELL_CMD_MAC:
        	  ProcessShellCommand_MAC(pComMsg);
        	break;
       #endif
        case MSGID_DIAG_MEMORY_PEEK_RESPONSE :
            ProcessMemoryPeekResponse(pComMsg);
            break;
        case MSGID_DIAG_MEMORY_POKE_RESPONSE :
            ProcessMemoryPokeResponse(pComMsg);
            break;
        case MSGID_DIAG_SW_VER_QUERY_RESPONSE:
            ProcessSwVerQueryResponse(pComMsg);
            break;
        case MSGID_DIAG_STATUS_QUERY_RESPONSE:
            ProcessStatusQueryResponse(pComMsg);
            break;
        case MSGID_DIAG_CPU_PING_RESPONSE:
            ProcessCpuPingResponse(pComMsg);
            break;
        case MSGID_DIAG_RPC_RESPONSE:
            ProcessRpcResponse(pComMsg);
            break;
        case MSGID_DIAG_INFO_DISPLAY:
            ProcessInfoDisplay(pComMsg);
            break;
        case MSGID_DIAG_TIME_OUT:
            ProcessTimeOut();
            break;	
	 case SERIAL_NUM_GET_REQ://ems来的查询序列号请求，由cm模块转发
	      ProcessSerialNumReq(pComMsg);
	      break;
	 case MSGID_DIAG_SERIAL_NUM_TIME_OUT:
	 	ProcessSerialNumTimeout();
		break;
	case MSGID_DIAG_1MIN_TIME_OUT:
	     Process1MinTimeOut();//lijinan 20081013
		break;
	case MSGID_DIAG_RPC_CFG:
	     processRpcCfg(pComMsg);
		break;
#ifdef WBBU_CODE
	case MSGID_DIAG_PRINT:
		ProcessDebugMsg(pComMsg);

		break;
#endif
        default:
            LOG1(LOG_SEVERE, M_DIAG_INVALID_CMD, "Unknown ComMsg ID: 0x%x", pComMsg->GetMessageId());
            break;
    }

    pComMsg->Destroy();
    //return false;
    return true;
}

void CL3TaskDiag::PostDiagGeneralMsg(DIAG_CPU_INDEX cpuIndex, UINT16 msgId)
{
    CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
 
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagGeneral))CComMessage;

    if ( diagMsg)
    {
        T_DiagGeneral* dataPtr = (T_DiagGeneral*)diagMsg->GetDataPtr();
        dataPtr->TransactionId = taskObj->GetDiagCommandTransactionID();
        dataPtr->cpuType = CpuInfo[cpuIndex].CpuType;
        dataPtr->cpuInstance = CpuInfo[cpuIndex].CpuIndex;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(msgId);
        if (CComEntity::PostEntityMessage(diagMsg))
        {
            taskObj->SetWaitingFlag(cpuIndex);
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}

void CL3TaskDiag::PostDiagProbeMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    PostDiagGeneralMsg(cpuIndex, MSGID_DIAG_CPU_PING);
}


void CL3TaskDiag::PostDiagMemoryPeekMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagMemoryPeekCmd))CComMessage;

    if ( diagMsg)
    {
        T_DiagMemoryPeekCmd* dataPtr = (T_DiagMemoryPeekCmd*)diagMsg->GetDataPtr();
        dataPtr->cpuInfo.TransactionId = taskObj->GetDiagCommandTransactionID();
        dataPtr->cpuInfo.cpuType = CpuInfo[cpuIndex].CpuType;
        dataPtr->cpuInfo.cpuInstance = CpuInfo[cpuIndex].CpuIndex;
        dataPtr->peekAddress = arg1;
        dataPtr->peekLength = arg2;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(MSGID_DIAG_MEMORY_PEEK_CMD);
        if (CComEntity::PostEntityMessage(diagMsg))
        {
            taskObj->SetWaitingFlag(cpuIndex);
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}


void CL3TaskDiag::PostDiagMemoryPokeMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{

    CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagMemoryPoke))CComMessage;

    if ( diagMsg)
    {
        T_DiagMemoryPoke* dataPtr = (T_DiagMemoryPoke*)diagMsg->GetDataPtr();
        dataPtr->cpuInfo.TransactionId = taskObj->GetDiagCommandTransactionID();
        dataPtr->cpuInfo.cpuType = CpuInfo[cpuIndex].CpuType;
        dataPtr->cpuInfo.cpuInstance = CpuInfo[cpuIndex].CpuIndex;
        dataPtr->pokeAddress = arg1;
        dataPtr->setValue = arg2;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(MSGID_DIAG_MEMORY_POKE_CMD);
        if (CComEntity::PostEntityMessage(diagMsg))
        {
            taskObj->SetWaitingFlag(cpuIndex);
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}


void CL3TaskDiag::PostDiagVerQueryMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    PostDiagGeneralMsg(cpuIndex, MSGID_DIAG_SW_VER_QUERY);
}


void CL3TaskDiag::PostDiagStatusQueryMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    PostDiagGeneralMsg(cpuIndex, MSGID_DIAG_STATUS_QUERY);
}

void CL3TaskDiag::PostDiagRpcMsg(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagRpcReq))CComMessage;

    if ( diagMsg)
    {
        T_DiagRpcReq* dataPtr = (T_DiagRpcReq*)diagMsg->GetDataPtr();
        dataPtr->cpuInfo.TransactionId = taskObj->GetDiagCommandTransactionID();
        dataPtr->cpuInfo.cpuType = CpuInfo[cpuIndex].CpuType;
        dataPtr->cpuInfo.cpuInstance = CpuInfo[cpuIndex].CpuIndex;
        dataPtr->RpcIndex = arg1;
        dataPtr->arg0 = arg2;
        dataPtr->arg1 = arg3;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(MSGID_DIAG_RPC_CMD);
        if (CComEntity::PostEntityMessage(diagMsg))
        {
            taskObj->SetWaitingFlag(cpuIndex);
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}

#ifdef WBBU_CODE
void CL3TaskDiag::PostDiagGeneralMsg_MAC(DIAG_CPU_INDEX cpuIndex, UINT16 msgId)
{
    CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
 
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagGeneral))CComMessage;

    if ( diagMsg)
    {
        T_DiagGeneral* dataPtr = (T_DiagGeneral*)diagMsg->GetDataPtr();
        dataPtr->TransactionId = taskObj->GetDiagCommandTransactionID();
        dataPtr->cpuType = CpuInfo[cpuIndex].CpuType;
        dataPtr->cpuInstance = CpuInfo[cpuIndex].CpuIndex;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(msgId);
          if((CpuInfo[cpuIndex].CpuType==CPU_AUX)||(CpuInfo[cpuIndex].CpuType==CPU_FEP))
        	{
        	    //dsp 1
        	    diagMsg->SetMoudlue(2);
        	}
        else if(CpuInfo[cpuIndex].CpuType==CPU_L2)
        	{
        	   //dsp 5
        	   diagMsg->SetMoudlue(0);
        	}
        else if((CpuInfo[cpuIndex].CpuType==CPU_CORE9))
        	{
        	     //dsp 4 
        	      diagMsg->SetMoudlue(5);
        	}
        else if((CpuInfo[cpuIndex].CpuType==CPU_MCP))
        	{
        	     if(CpuInfo[cpuIndex].CpuIndex<3)
        	     	{
        	     	   //dsp 2
        	     	   diagMsg->SetMoudlue(3);
        	     	}
        	     else if((CpuInfo[cpuIndex].CpuIndex<6)&&(CpuInfo[cpuIndex].CpuIndex>2))
        	     	{
        	     	   //dsp 3
        	     	   diagMsg->SetMoudlue(4);
        	     	}
        	     else if(((CpuInfo[cpuIndex].CpuIndex<8)&&(CpuInfo[cpuIndex].CpuIndex>5)))
        	     	{
        	     	    //dsp 4
        	     	    diagMsg->SetMoudlue(5);
        	     	}
        	}
        if (CComEntity::PostEntityMessage(diagMsg))
        {
            taskObj->SetWaitingFlag(cpuIndex);
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}

void CL3TaskDiag::PostDiagProbeMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    PostDiagGeneralMsg_MAC(cpuIndex, MSGID_DIAG_CPU_PING);
}


void CL3TaskDiag::PostDiagMemoryPeekMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagMemoryPeekCmd))CComMessage;

    if ( diagMsg)
    {
        T_DiagMemoryPeekCmd* dataPtr = (T_DiagMemoryPeekCmd*)diagMsg->GetDataPtr();
        dataPtr->cpuInfo.TransactionId = taskObj->GetDiagCommandTransactionID();
        dataPtr->cpuInfo.cpuType = CpuInfo[cpuIndex].CpuType;
        dataPtr->cpuInfo.cpuInstance = CpuInfo[cpuIndex].CpuIndex;
        dataPtr->peekAddress = arg1;
        dataPtr->peekLength = arg2;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(MSGID_DIAG_MEMORY_PEEK_CMD);
              if((CpuInfo[cpuIndex].CpuType==CPU_AUX)||(CpuInfo[cpuIndex].CpuType==CPU_FEP))
        	{
        	    //dsp 1
        	    diagMsg->SetMoudlue(2);
        	}
        else if(CpuInfo[cpuIndex].CpuType==CPU_L2)
        	{
        	   //dsp 5
        	   diagMsg->SetMoudlue(0);
        	}
        else if((CpuInfo[cpuIndex].CpuType==CPU_CORE9))
        	{
        	     //dsp 4 
        	      diagMsg->SetMoudlue(5);
        	}
        else if((CpuInfo[cpuIndex].CpuType==CPU_MCP))
        	{
        	     if(CpuInfo[cpuIndex].CpuIndex<3)
        	     	{
        	     	   //dsp 2
        	     	   diagMsg->SetMoudlue(3);
        	     	}
        	     else if((CpuInfo[cpuIndex].CpuIndex<6)&&(CpuInfo[cpuIndex].CpuIndex>2))
        	     	{
        	     	   //dsp 3
        	     	   diagMsg->SetMoudlue(4);
        	     	}
        	     else if(((CpuInfo[cpuIndex].CpuIndex<8)&&(CpuInfo[cpuIndex].CpuIndex>5)))
        	     	{
        	     	    //dsp 4
        	     	    diagMsg->SetMoudlue(5);
        	     	}
        	}
        if (CComEntity::PostEntityMessage(diagMsg))
        {
            taskObj->SetWaitingFlag(cpuIndex);
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}


void CL3TaskDiag::PostDiagMemoryPokeMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{

    CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagMemoryPoke))CComMessage;

    if ( diagMsg)
    {
        T_DiagMemoryPoke* dataPtr = (T_DiagMemoryPoke*)diagMsg->GetDataPtr();
        dataPtr->cpuInfo.TransactionId = taskObj->GetDiagCommandTransactionID();
        dataPtr->cpuInfo.cpuType = CpuInfo[cpuIndex].CpuType;
        dataPtr->cpuInfo.cpuInstance = CpuInfo[cpuIndex].CpuIndex;
        dataPtr->pokeAddress = arg1;
        dataPtr->setValue = arg2;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(MSGID_DIAG_MEMORY_POKE_CMD);
              if((CpuInfo[cpuIndex].CpuType==CPU_AUX)||(CpuInfo[cpuIndex].CpuType==CPU_FEP))
        	{
        	    //dsp 1
        	    diagMsg->SetMoudlue(2);
        	}
        else if(CpuInfo[cpuIndex].CpuType==CPU_L2)
        	{
        	   //dsp 5
        	   diagMsg->SetMoudlue(0);
        	}
        else if((CpuInfo[cpuIndex].CpuType==CPU_CORE9))
        	{
        	     //dsp 4 
        	      diagMsg->SetMoudlue(5);
        	}
        else if((CpuInfo[cpuIndex].CpuType==CPU_MCP))
        	{
        	     if(CpuInfo[cpuIndex].CpuIndex<3)
        	     	{
        	     	   //dsp 2
        	     	   diagMsg->SetMoudlue(3);
        	     	}
        	     else if((CpuInfo[cpuIndex].CpuIndex<6)&&(CpuInfo[cpuIndex].CpuIndex>2))
        	     	{
        	     	   //dsp 3
        	     	   diagMsg->SetMoudlue(4);
        	     	}
        	     else if(((CpuInfo[cpuIndex].CpuIndex<8)&&(CpuInfo[cpuIndex].CpuIndex>5)))
        	     	{
        	     	    //dsp 4
        	     	    diagMsg->SetMoudlue(5);
        	     	}
        	}
        if (CComEntity::PostEntityMessage(diagMsg))
        {
            taskObj->SetWaitingFlag(cpuIndex);
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}


void CL3TaskDiag::PostDiagVerQueryMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    PostDiagGeneralMsg_MAC(cpuIndex, MSGID_DIAG_SW_VER_QUERY);
}


void CL3TaskDiag::PostDiagStatusQueryMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    PostDiagGeneralMsg_MAC(cpuIndex, MSGID_DIAG_STATUS_QUERY);
}

void CL3TaskDiag::PostDiagRpcMsg_MAC(DIAG_CPU_INDEX cpuIndex, int arg1, int arg2, int arg3, int arg4)
{
    CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagRpcReq))CComMessage;

    if ( diagMsg)
    {
        T_DiagRpcReq* dataPtr = (T_DiagRpcReq*)diagMsg->GetDataPtr();
        dataPtr->cpuInfo.TransactionId = taskObj->GetDiagCommandTransactionID();
        dataPtr->cpuInfo.cpuType = CpuInfo[cpuIndex].CpuType;
        dataPtr->cpuInfo.cpuInstance = CpuInfo[cpuIndex].CpuIndex;
        dataPtr->RpcIndex = arg1;
        dataPtr->arg0 = arg2;
        dataPtr->arg1 = arg3;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(MSGID_DIAG_RPC_CMD);
            if((CpuInfo[cpuIndex].CpuType==CPU_AUX)||(CpuInfo[cpuIndex].CpuType==CPU_FEP))
        	{
        	    //dsp 1
        	    diagMsg->SetMoudlue(2);
        	}
        else if(CpuInfo[cpuIndex].CpuType==CPU_L2)
        	{
        	   //dsp 5
        	   diagMsg->SetMoudlue(0);
        	}
        else if((CpuInfo[cpuIndex].CpuType==CPU_CORE9))
        	{
        	     //dsp 4 
        	      diagMsg->SetMoudlue(5);
        	}
        else if((CpuInfo[cpuIndex].CpuType==CPU_MCP))
        	{
        	     if(CpuInfo[cpuIndex].CpuIndex<3)
        	     	{
        	     	   //dsp 2
        	     	   diagMsg->SetMoudlue(3);
        	     	}
        	     else if((CpuInfo[cpuIndex].CpuIndex<6)&&(CpuInfo[cpuIndex].CpuIndex>2))
        	     	{
        	     	   //dsp 3
        	     	   diagMsg->SetMoudlue(4);
        	     	}
        	     else if(((CpuInfo[cpuIndex].CpuIndex<8)&&(CpuInfo[cpuIndex].CpuIndex>5)))
        	     	{
        	     	    //dsp 4
        	     	    diagMsg->SetMoudlue(5);
        	     	}
        	}
        if (CComEntity::PostEntityMessage(diagMsg))
        {
            taskObj->SetWaitingFlag(cpuIndex);
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}
LOCAL unsigned int debug_count = 0; 
 void CL3TaskDiag::ProcessDebugMsg(CComMessage *pComMsg)
 {
     char *rxBuff = (char *)pComMsg->GetDataPtr();
    int lineLen = pComMsg->GetDataLength();
   debug_count++;
    if ( '\n' == rxBuff[lineLen-1] )
    {
        rxBuff[lineLen-1] = '\0';
    }
	else
	{
		rxBuff[lineLen-1] = '\0';
       }

       LOG_STR(LOG_CRITICAL, 0, rxBuff);
 }

#endif
void  CL3TaskDiag::PostDiagMsg(T_DiagShellCommand *diagCmd, SEND_DIAG_CMD_FUNCPTR sendFunc)
{
    DiagCommandTransactionID ++;

    if ( CPU_NAME_L2PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
    {
        sendFunc(CPU_INDEX_L2, diagCmd->arg1, diagCmd->arg2, diagCmd->arg3, diagCmd->arg4);
    }
    if ( CPU_NAME_AUX == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName)
    {
        sendFunc(CPU_INDEX_AUX, diagCmd->arg1, diagCmd->arg2, diagCmd->arg3, diagCmd->arg4);
    }
    if ( CPU_NAME_MCP0 <= diagCmd->cpuName && CPU_NAME_MCP7 >= diagCmd->cpuName )
    {
        sendFunc((DIAG_CPU_INDEX)(CPU_INDEX_MCP0 + diagCmd->cpuName - CPU_NAME_MCP0), diagCmd->arg1, diagCmd->arg2, diagCmd->arg3, diagCmd->arg4);
    }
    if ( CPU_NAME_MCPALL == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName)
    {
        for (int i=0; i<M_NumberOfMCP; i++)
        {
            sendFunc((DIAG_CPU_INDEX)(CPU_INDEX_MCP0 + i), diagCmd->arg1, diagCmd->arg2, diagCmd->arg3, diagCmd->arg4);
        }
    }
    if ( CPU_NAME_FEP0 == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName || CPU_NAME_FEPALL == diagCmd->cpuName)
    {
        sendFunc(CPU_INDEX_FEP0, diagCmd->arg1, diagCmd->arg2, diagCmd->arg3, diagCmd->arg4);
    }
    if ( CPU_NAME_FEP1 == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName || CPU_NAME_FEPALL == diagCmd->cpuName)
    {
        sendFunc(CPU_INDEX_FEP1, diagCmd->arg1, diagCmd->arg2, diagCmd->arg3, diagCmd->arg4);
    }
#ifdef WBBU_CODE
        if ( CPU_NAME_CORE9== diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
    {
        sendFunc(CPU_INDEX_CORE9, diagCmd->arg1, diagCmd->arg2, diagCmd->arg3, diagCmd->arg4);
    }
#endif
}


extern "C" unsigned int bspGetCPLDVersion();
#ifndef WBBU_CODE
void BspDisplaySwVersion()
{
#if 1
    char verStr[100];
	char tmpStr[50] = "BWA ";
	strcpy( verStr, VERSION );
	strcat( tmpStr+4, verStr+7 );
	memset( verStr, 0, 100 );
    sprintf(verStr, "%s [%s, %s]", tmpStr, __DATE__, __TIME__ );
	printf(" BTS L3 Version : %s\n", verStr);
    printf(" BTS L3 system CPLD version is 0x%4x\n", bspGetCPLDVersion());
#else
    char verStr[100];
    sprintf(verStr, "%s [%s, %s]", VERSION, __DATE__, __TIME__ );
	printf(" BTS L3 Version : %s\n", verStr);
    printf(" BTS L3 system CPLD version is 0x%4x\n", bspGetCPLDVersion());
#endif
}
#else
void BspDisplaySwVersion()
{
    char verStr[100];
    unsigned int ver;
   unsigned char a,b,c,d;
    sprintf(verStr, "%s [%s, %s]", VERSION, __DATE__, __TIME__ );
	printf(" WBBU L3 Version : %s\n", verStr);
   ver = Read_Fpga_Version_Soft();
   a = ver>>24;
   b = ver>> 16;
   c = ver>> 8;
   d =ver;
    printf( "WBBU FPGA Version:%02x.%02x.%02x.%02x\n",a,b,c,d);
	#ifdef WBBU_NVRAM_
    printf(" BTS L3 system CPLD version is 0x%4x\n", bspGetCPLDVersion());
	#endif
}
#endif
void L3PpcStateShow()
{
}
void CL3TaskDiag::ProcessShellCommand(CComMessage *pComMsg)
{
    T_DiagShellCommand *diagCmd = (T_DiagShellCommand*)pComMsg->GetDataPtr();

    switch (diagCmd->diagCmd)
    {
        case DIAG_SHELL_CMD_PROBE:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
            	printf("L3 PPC is alive\n");
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagProbeMsg);
            }
            break;

        case DIAG_SHELL_CMD_MEM_PEEK:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
                UINT32 *addr = (UINT32*)diagCmd->arg1;
                printf( "L3 PPC memory at Address 0x%x is: \n", diagCmd->arg1);
                for (int i=0; i<diagCmd->arg2; i++)
                {
                    printf("0x%08x ", *addr++);
                }
                printf("\n");
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagMemoryPeekMsg);
            }
            break;

            break;
        case DIAG_SHELL_CMD_MEM_POKE:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
                UINT32 *addr = (UINT32 *)diagCmd->arg1;
                *addr = diagCmd->arg2;
                printf( "L3 PPC memory at Address 0x%x is: changed to 0x%08x\n", addr, *addr);
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagMemoryPokeMsg);
            }
            break;

        case DIAG_SHELL_CMD_VER_QUERY:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
            	 if(OneMinQuryVerFlag==false)//lijinan 20081014
            	 	{
            	 	     printf(" \n");
                BspDisplaySwVersion();
            			printf(" \n");
            	 	}
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagVerQueryMsg);
            }
            break;

        case DIAG_SHELL_CMD_STATE_QUERY:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
                L3PpcStateShow();
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagStatusQueryMsg);
            }
            break;
        case DIAG_SHELL_CMD_PRC:
            if (CPU_NAME_L3PPC == diagCmd->cpuName )
            {
				printf(" L3 does not support RPC\n");
            }
            else
            {
                PostDiagMsg(diagCmd, (SEND_DIAG_CMD_FUNCPTR)PostDiagRpcMsg);
            }
            break;
        default:
            printf("Unknown Diag command\n");
            return;//wangwenhua add 2012-3-21
            break;
    }

    bool responsePending = false;
    for (int i=0; i<CPU_INDEX_MAX; i++)
    {
        if (WaitingForCpuResponse[i])
        {
            responsePending = true;
            break;
        }
    }

    if (responsePending)
    {
        CComMessage *tmoMsg = new(this, 0)CComMessage;
        if (tmoMsg)
        {
            tmoMsg->SetDstTid(M_TID_DIAGM);
            tmoMsg->SetSrcTid(M_TID_DIAGM);
            tmoMsg->SetMessageId(MSGID_DIAG_TIME_OUT);
            DiagResponseTimer = new CTimer(false, 2000, tmoMsg);
            if (DiagResponseTimer)
            {
                DiagResponseTimer->Start();
                return;
            }
        }
    }
    if(OneMinQuryVerFlag == false)
    	semGive(ShellCmdCompleteSem);   // let the diag shell task finish if no response is pending
    semGive(ShellCmdSem);  // no need to wait for any response
    OneMinQuryVerFlag = false;
}
#ifdef WBBU_CODE
  void CL3TaskDiag::ProcessShellCommand_MAC(CComMessage *pComMsg)
  {
        T_DiagShellCommand *diagCmd = (T_DiagShellCommand*)pComMsg->GetDataPtr();

    switch (diagCmd->diagCmd)
    {
        case DIAG_SHELL_CMD_PROBE:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
            	printf("L3 PPC is alive\n");
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagProbeMsg_MAC);
            }
            break;

        case DIAG_SHELL_CMD_MEM_PEEK:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
                UINT32 *addr = (UINT32*)diagCmd->arg1;
                printf( "L3 PPC memory at Address 0x%x is: \n", diagCmd->arg1);
                for (int i=0; i<diagCmd->arg2; i++)
                {
                    printf("0x%08x ", *addr++);
                }
                printf("\n");
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagMemoryPeekMsg_MAC);
            }
            break;

            break;
        case DIAG_SHELL_CMD_MEM_POKE:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
                UINT32 *addr = (UINT32 *)diagCmd->arg1;
                *addr = diagCmd->arg2;
                printf( "L3 PPC memory at Address 0x%x is: changed to 0x%08x\n", addr, *addr);
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagMemoryPokeMsg_MAC);
            }
            break;

        case DIAG_SHELL_CMD_VER_QUERY:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
            	 if(OneMinQuryVerFlag==false)//lijinan 20081014
                BspDisplaySwVersion();
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagVerQueryMsg_MAC);
            }
            break;

        case DIAG_SHELL_CMD_STATE_QUERY:
            if ( CPU_NAME_L3PPC == diagCmd->cpuName || CPU_NAME_ALL == diagCmd->cpuName )
            {
                L3PpcStateShow();
            }
            if ( CPU_NAME_L3PPC != diagCmd->cpuName )
            {
                PostDiagMsg(diagCmd,(SEND_DIAG_CMD_FUNCPTR)PostDiagStatusQueryMsg_MAC);
            }
            break;
        case DIAG_SHELL_CMD_PRC:
            if (CPU_NAME_L3PPC == diagCmd->cpuName )
            {
				printf(" L3 does not support RPC\n");
            }
            else
            {
                PostDiagMsg(diagCmd, (SEND_DIAG_CMD_FUNCPTR)PostDiagRpcMsg_MAC);
            }
            break;
        default:
            printf("Unknown Diag command\n");
            return;//wangwenhua add 2012-3-21
            break;
    }

    bool responsePending = false;
    for (int i=0; i<CPU_INDEX_MAX; i++)
    {
        if (WaitingForCpuResponse[i])
        {
            responsePending = true;
            break;
        }
    }

    if (responsePending)
    {
        CComMessage *tmoMsg = new(this, 0)CComMessage;
        if (tmoMsg)
        {
            tmoMsg->SetDstTid(M_TID_DIAGM);
            tmoMsg->SetSrcTid(M_TID_DIAGM);
            tmoMsg->SetMessageId(MSGID_DIAG_TIME_OUT);
            DiagResponseTimer = new CTimer(false, 2000, tmoMsg);
            if (DiagResponseTimer)
            {
                DiagResponseTimer->Start();
                return;
            }
        }
    }
    if(OneMinQuryVerFlag == false)
    	semGive(ShellCmdCompleteSem);   // let the diag shell task finish if no response is pending
    semGive(ShellCmdSem);  // no need to wait for any response
    OneMinQuryVerFlag = false;
  }
#endif
DIAG_CPU_INDEX GetCpuIndex(UINT16 cpuType, UINT16 cpuInstance)
{
    DIAG_CPU_INDEX cpuIndex =(DIAG_CPU_INDEX)( CPU_INDEX_MAX - 1);
    switch (cpuType)
    {
        case CPU_L2:
            cpuIndex = CPU_INDEX_L2;
            break;
        case CPU_AUX:
            cpuIndex = CPU_INDEX_AUX;
            break;
        case CPU_MCP:
            if (cpuInstance < 8)
            {
                cpuIndex = (DIAG_CPU_INDEX)(CPU_INDEX_MCP0 + cpuInstance);
            }
            break;
        case CPU_FEP:
            if (cpuInstance < 2)
            {
                cpuIndex = (DIAG_CPU_INDEX)(CPU_INDEX_FEP0 + cpuInstance);
            }
            break;
#ifdef WBBU_CODE
          case CPU_CORE9:
          	cpuIndex = CPU_INDEX_CORE9;
          	break;
#endif
    }

    return cpuIndex;

}

void CL3TaskDiag::CommandCompleteCheck(DIAG_CPU_INDEX cpuIndex)
{
    int i;
    WaitingForCpuResponse[cpuIndex] = false;

    for (i=0; i<CPU_INDEX_MAX; i++)
    {
        if (WaitingForCpuResponse[i])
        {
            break;
        }
    }

    if (i >= CPU_INDEX_MAX )
    {   // all response needed are received
        if (DiagResponseTimer)
        {
            DiagResponseTimer->Stop();
            delete DiagResponseTimer;
            DiagResponseTimer = NULL;
        }
#ifdef WBBU_CODE
        if(Need_Load_Cfg==1)
        {
             Send_Data_Cfg_2_CM(0x2037);
             Need_Load_Cfg = 0;
        }
#endif
	 if(OneMinQuryVerFlag == false)
       	 semGive(ShellCmdCompleteSem);   // let the command task finish
        semGive(ShellCmdSem);   // give the mutex semaphore to allow next command
        OneMinQuryVerFlag = false;
    }
}



void CL3TaskDiag::ProcessMemoryPeekResponse(CComMessage *pComMsg)
{
    int i, j;
    bool flag=FALSE;
    T_DiagMemoryPeekResp *resp = (T_DiagMemoryPeekResp*)pComMsg->GetDataPtr();

    if (resp->cpuInfo.TransactionId != DiagCommandTransactionID)
    {
        return;
    }

    UINT32 *addr = (UINT32 *)resp->peekAddress;
    DIAG_CPU_INDEX cpuIndex = GetCpuIndex(resp->cpuInfo.cpuType, resp->cpuInfo.cpuInstance);
	if(resp->peekAddress==0xffff0020)//查询序列号应答
	{
		memcpy(serialNumRsp.syn_RF_num, &resp->MemroyValue[0], 180);
		if(serialNumReqTimer)//网管下发的消息
		{
			flag = TRUE;
			delete serialNumReqTimer;
            		serialNumReqTimer = NULL;	
			
			CComMessage *pComMsg1 = new(this, sizeof(T_SerialNumRsp))CComMessage;
			if ( pComMsg1 )
			{
				pComMsg1->SetDstTid (M_TID_CM);
				pComMsg1->SetSrcTid (M_TID_DIAGM);
				pComMsg1->SetMessageId( SERIAL_NUM_GET_RSP);
				memcpy((UINT8*)pComMsg1->GetDataPtr(), &serialNumRsp, sizeof(T_SerialNumRsp));
				
				if ( false == CComEntity::PostEntityMessage(pComMsg1) )
				{
					pComMsg1->Destroy();			
					return;
				}
			}
			return;
		}		
		j=0;
		printf("\nSyn serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}
		printf("\nRF0 serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}
		printf("\nRF1 serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}
		printf("\nRF2 serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}
		printf("\nRF3 serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}
		printf("\nRF4 serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}
		printf("\nRF5 serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}
		printf("\nRF6 serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}
		printf("\nRF7 serial num is: ");
		for(i=0; i<20;i++)
		{
			printf("%x ", serialNumRsp.syn_RF_num[j]);
			j++;
		}	
		printf("\n");
		if((serialNumReqTimer == NULL)&&(flag==FALSE))
			CommandCompleteCheck(cpuIndex);
		return;
	}
#ifdef WBBU_CODE
	if(resp->peekAddress==0xfffffffe)
	{
	   unsigned char *pp =(unsigned char*) pComMsg->GetDataPtr();
          unsigned int *addr_print;
	   addr_print = (unsigned int*)(pp+14);
	printf("\r\n Frame_Counter:%d",*addr_print);
	addr_print++;
	printf("\r\nSend_FEP_Counter:%d",*addr_print);
	addr_print++;
	printf("\r\nSend_CORE9_Counter:%d",*addr_print);
	addr_print++;
	printf("\r\nUPLINK_Proc_Counter:%d",*addr_print);
	addr_print++;
	printf("\r\n DOWNLINK_Proc_Counter:%d",*addr_print);
	addr_print++;
	
	printf("\r\nRecv_DSP1_GPIO_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DSP1_AIF_Get_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DSP1_AIF_Lost_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DSP1_EDMA_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DSP1_AIF_Link_Error_counter:%d",*addr_print);
	addr_print++;

	printf("\r\n Recv_FEP0_CheckSum_Error:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_FEP1_CheckSum_Error:%d",*addr_print);
	addr_print++;

	printf("\r\nRecv_DSP4_GPIO_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DSP4_AIF_Get_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DSP4_AIF_Lost_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DSP4_EDMA_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DSP4_AIF_Link_Error_counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_CORE9_CheckSum_Error:%d",*addr_print);
	addr_print++;

	printf("\r\n ReConfig_Counter:%d",*addr_print);
	addr_print++;
	printf("\r\n Recv_UlProfile_Counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_DlProfile_Counter:%d",*addr_print);
	addr_print++;
	printf("\r\nRecv_Config_Profile_Counter:%d",*addr_print);
	addr_print++;
	printf("\r\n MCP_UL_Not_Over_Counter:%d",*addr_print);
	
     printf("\n");
	   CommandCompleteCheck(cpuIndex);
	   return;
	}
#endif
    printf( "%s  memory at Address 0x%x is: \n", CpuInfo[cpuIndex].CpuName, resp->peekAddress);
	//头信息改格式打印了jy080804
	if((resp->peekAddress==0xffff000f)&&(resp->peekLength == 32))
	{
		int16_t  tmp[64];
		int16_t *addr16 = (int16_t *)&resp->MemroyValue[0]; 

		for(i=0; i<64;)
		{
			tmp[i+1] = *addr16;
			addr16++;
			tmp[i] = *addr16;
			i = i+2;
			addr16++;
		}
		addr16 = tmp;
		printf("version: %04d\n", *addr16++);
		printf("Head size: %04d\n", *addr16++);
		printf("Band ID: %04d\n", *addr16++);
		printf("Dtmax: %04d\n", *addr16++);
		printf("Dtmin: %04d\n", *addr16++);
		printf("Dtstep: %04d\n", *addr16++);
		printf("Drmax: %04d\n", *addr16++);
		printf("Drmin: %04d\n", *addr16++);
		printf("Drstep: %04d\n", *addr16++);
		printf("Tmax: %04d\n", *addr16++);
		printf("Tmin: %04d\n", *addr16++);
		printf("Tstep: %04d\n", *addr16++);
		printf("Fmax: %04d\n", *addr16++);
		printf("Fmin: %04d\n", *addr16++);
		printf("Fstep: %04d\n", *addr16++);
		printf("PLLVersion: %04d\n", *addr16++);
		printf("PLLFormat: %04d\n", *addr16++);
		printf("PLLCnt: %04d\n", *addr16++);
		printf("PLLmin: %04d\n", *addr16++);
		printf("PLLmax: %04d\n", *addr16++);
		printf("PLLstep: %04d\n", *addr16++);
		printf("Gmax: %04d\n", *addr16++);
		printf("Pmax: %04d\n", *addr16++);
		printf("TTh1: %04d\n", *addr16++);
		printf("RTh1: %04d\n", *addr16++);
		printf("rsv:\n ");
		for(i=0; i<38; i++)
		{
			if(i%10==0)
				printf("\n");
			printf("%04d  ", *addr16++);
		}
		printf("\n");
		printf("Chksum: %04d\n", *addr16++);		
		
	}
	else
	{
		addr = &resp->MemroyValue[0];
		for (i=0; i<resp->peekLength; i++)
		{
			printf("0x%08x ", *addr++);
		}
    }
    printf("\n");

    CommandCompleteCheck(cpuIndex);

}



void CL3TaskDiag::ProcessMemoryPokeResponse(CComMessage *pComMsg)
{
    T_DiagMemoryPoke *resp = (T_DiagMemoryPoke*)pComMsg->GetDataPtr();

    if (resp->cpuInfo.TransactionId != DiagCommandTransactionID)
    {
        return;
    }

  //  UINT32 *addr = (UINT32*)resp->pokeAddress;
    DIAG_CPU_INDEX cpuIndex = GetCpuIndex(resp->cpuInfo.cpuType, resp->cpuInfo.cpuInstance);
    printf( "%s  memory at Address 0x%x is: set to 0x%08x\n", CpuInfo[cpuIndex].CpuName, resp->pokeAddress, resp->setValue);

    CommandCompleteCheck(cpuIndex);
}


void CL3TaskDiag::ProcessSwVerQueryResponse(CComMessage *pComMsg)
{
    T_DiagSwVersion *resp = (T_DiagSwVersion*)pComMsg->GetDataPtr();

    if (resp->cpuInfo.TransactionId != DiagCommandTransactionID)
    {
        return;
    }

	#ifndef WBBU_CODE
	    char *verStr = new char[resp->verStrLen+1-7+4];//del "MCWill ", add "BWA "	    
	    DIAG_CPU_INDEX cpuIndex = GetCpuIndex(resp->cpuInfo.cpuType, resp->cpuInfo.cpuInstance);
	    if (verStr)
	    {
	    	memcpy( verStr, "BWA ", 4 );
	        memcpy( verStr+4, (&resp->verStr[0])+7, resp->verStrLen-7);
	        verStr[resp->verStrLen-7+4] = '\0';
		if(OneMinQuryVerFlag==false)//lijinan 20081014
    		{
	        	printf( "%s  SW version is %s [%d]\n", CpuInfo[cpuIndex].CpuName,verStr,resp->verStrLen );
		}
	        delete[] verStr;
	    }
	#else
	    char *verStr = new char[resp->verStrLen+1];
	    DIAG_CPU_INDEX cpuIndex = GetCpuIndex(resp->cpuInfo.cpuType, resp->cpuInfo.cpuInstance);
	    if (verStr)
	    {
	        memcpy( verStr, &resp->verStr[0], resp->verStrLen);
	        verStr[resp->verStrLen] = '\0';
		if(OneMinQuryVerFlag==false)//lijinan 20081014
    		{
	        	printf( "%s  SW version is %s\n", CpuInfo[cpuIndex].CpuName,verStr);
		}
	        delete[] verStr;
	    }
		if((cpuIndex==CPU_INDEX_MCP6)||(cpuIndex==CPU_INDEX_MCP7)||(cpuIndex==CPU_INDEX_CORE9))
		{
			  if(g_send_2_DSP4==1)
		  	{
		  	     SetDsp4SaveInfo();
				 g_send_2_DSP4 = 0;
		  	}
		 }
   #endif
    //lijinan 20081013
    CpuNoResponseCount[cpuIndex] = 0;
   
    CommandCompleteCheck(cpuIndex);

}


void DisplayCpuAUXStatus(UINT32* dataPtr)
{
#ifndef WBBU_CODE
    AUXCDIBufType * paux = (AUXCDIBufType*)dataPtr;
    printf("fpgaVer[%08X]\r\nbtsSeqId[%08X]\r\nTDDtimeslot[%08X]\r\nDnlktimeslot[%08X]\r\nsyncSRC[%08X]\r\nGps_offset[%08X]\r\nantennaMask[%08X]\r\nrfPoweron[%08X]\n",
		    paux->fpgaVer,    paux->btsSeqId,  paux->TDDtimeslot,  paux->Dnlktimeslot,paux->syncSRC, paux->Gps_offset, paux->antennaMask, paux->rfPoweron);
    printf("W0 real: ");
	for(int i = 0; i<8; i++) printf("%04X  ", paux->w0[i].real);
	printf("");
    printf("\nW0 imag: ");
	for(int j = 0; j<8; j++) printf("%04X  ", paux->w0[j].imag);
	
	printf("\nRfRxGain: ");
	for(int k = 0; k<8; k++) printf("%08X  ", paux->RfRxGain[k]);
	printf("\nRfTxGain: ");
	for(int l = 0; l<8; l++) printf("%08X  ", paux->RfTxGain[l]);

    printf("\nSynRxGain[%08X]\r\nSynTxGain[%08X]\n",paux->SynRxGain, paux->SynTxGain);
	printf("L2msgCnt: ");
	for(int m = 0; m<5; m++) printf("%08X    ", paux->L2msgCnt[m]);

	printf("\nL2TimeCnt: ");
	for(int n= 0; n<3; n++) printf("%08X    ", paux->L2TimeCnt[n]);

	printf("\nfepChkErr: ");
	for(int o= 0; o<4; o++) printf("%08X    ", paux->fepChkErr[o]);

	printf("\nspiChkErr: ");
	for(int p= 0; p<11; p++) printf("%08X    ", paux->spiChkErr[p]);
    printf("\ncalibErrflag[%08X]\r\nemifaNotEmpty[%08X]\r\nemifbNotEmpty[%08X]\n",paux->calibErrflag, paux->emifaNotEmpty, paux->emifbNotEmpty);
	#else
	AUXStatusBuffer * paux = (AUXStatusBuffer*)dataPtr;
     int i;

	printf("btsSeqId:%x,TDDtimeslot:%x,Dnlktimeslot:%x,syncSRC:%x,antennaMask:%x\n",paux->btsSeqId,
		paux->TDDtimeslot,paux->Dnlktimeslot,paux->syncSRC,paux->antennaMask);
	printf("W0:");
	for( i =0 ;i < 8; i++)
	{
	   printf("%08x,",paux->w0[i]);
	}
	printf("\n");
	printf("RfRxGain:");
	for( i =0 ;i < 8; i++)
	{
	   printf("%08x,",paux->RfRxGain[i]);
	}
	printf("\n");
	printf("RfTxGain:");
	for( i =0 ;i < 8; i++)
	{
	   printf("%08x,",paux->RfTxGain[i]);
	}
	printf("\n");
	printf("SynRxGain:%x,SynTxGain:%x\n",paux->SynRxGain,paux->SynTxGain);
	printf("L2msgCnt:%x,%x,%x\n",paux->L2msgCnt[0],paux->L2msgCnt[1],paux->L2msgCnt[2]);
	printf("fepChkErr:%x,%x\n",paux->fepChkErr[0],paux->fepChkErr[1]);
	printf("calibErrflag:%x,SynParaFlag:%x\n",paux->calibErrflag,paux->SynParaFlag);
	printf("RfCalTxGain[0-4]:%x,%x,%x,%x,%x\n",paux->RfCalTxGain[0],paux->RfCalTxGain[1],paux->RfCalTxGain[2],paux->RfCalTxGain[3],paux->RfCalTxGain[4]);
	printf("RfCalTxGain[5-9]:%x,%x,%x,%x,%x\n",paux->RfCalTxGain[5],paux->RfCalTxGain[6],paux->RfCalTxGain[7],paux->RfCalTxGain[8],paux->RfCalTxGain[9]);
	printf("RfCalRxGain[0-5]:%x,%x,%x,%x,%x,%x\n",paux->RfCalRxGain[0],paux->RfCalRxGain[1],paux->RfCalRxGain[2],paux->RfCalRxGain[3],paux->RfCalRxGain[4],paux->RfCalRxGain[5]);
	printf("RfCalRxGain[6-11]:%x,%x,%x,%x,%x,%x\n",paux->RfCalRxGain[6],paux->RfCalRxGain[7],paux->RfCalRxGain[8],paux->RfCalRxGain[9],paux->RfCalRxGain[10],paux->RfCalRxGain[11]);
       printf("SynCalRxGain:%x,%x\n",paux->SynCalRxGain[0],paux->SynCalRxGain[1]);

	printf("SynCalTxGain[0-5]:%x,%x,%x,%x,%x,%x\n",paux->SynCalTxGain[0],paux->SynCalTxGain[1],paux->SynCalTxGain[2],paux->SynCalTxGain[3],paux->SynCalTxGain[4],paux->SynCalTxGain[5]);
	printf("SynCalTxGain[6-12]:%x,%x,%x,%x,%x,%x,%x\n",paux->SynCalTxGain[6],paux->SynCalTxGain[7],paux->SynCalTxGain[8],paux->SynCalTxGain[9],paux->SynCalTxGain[10],paux->SynCalTxGain[11],paux->SynCalTxGain[12]);
       printf("GpioRevCount:%x,%x,%x\n",paux->GpioRevCount[0],paux->GpioRevCount[1],paux->GpioRevCount[2]);

	printf("GpioDataHeadMissCount:%x,%x,%x\n",paux->GpioDataHeadMissCount[0],paux->GpioDataHeadMissCount[1],paux->GpioDataHeadMissCount[2]);
	printf("GpioDataHeadOkCount:%x,%x,%x\n",paux->GpioDataHeadOkCount[0],paux->GpioDataHeadOkCount[1],paux->GpioDataHeadOkCount[2]);
	printf("link_state_counter:%x,%x,%x\n",paux->link_state_counter[0],paux->link_state_counter[1],paux->link_state_counter[2]);
	#endif
}



void DisplayCpuFEPStatus(UINT32* dataPtr)
{
#ifndef WBBU_CODE
    FEPCDIBufType * pFep = (FEPCDIBufType*)dataPtr;
    printf("FepId[%08X]\r\nfpgaVer[%08X]\r\nbtsSeqId[%08X]\r\nTDDtimeslot[%08X]\r\nDnlktimeslot[%08X]\r\npreambleScale[%08X]\r\nantennaMask[%08X]\r\nSCGMask[%08X]\n",
		    pFep->FepId,    pFep->fpgaVer,  pFep->btsSeqId,  pFep->TDDtimeslot,pFep->Dnlktimeslot, pFep->preambleScale, pFep->antennaMask, pFep->SCGMask);
    printf("W0 real: ");
	for(int i = 0; i<8; i++) printf("%04X    ", pFep->w0[i].real);
    printf("\nW0 imag: ");
	for(int j = 0; j<8; j++) printf("%04X    ", pFep->w0[j].imag);

	printf("\nauxmsgCnt: ");
	for(int m = 0; m<4; m++) printf("%08X    ", pFep->auxmsgCnt[m]);
    printf("\nauxChkErr[%08X]\r\ncPreUplk[%08X]\r\ncUplk[%08X]\r\ncPreDnlk[%08X]\r\ncDnlk[%08X]\r\ncDefault[%08X]\n",
		    pFep->auxChkErr,    pFep->cPreUplk,  pFep->cUplk,  pFep->cPreDnlk,pFep->cDnlk, pFep->cDefault);

	printf("\ndnlkCount: ");
	for(int m = 0; m<8; m++) printf("%08X    ", pFep->dnlkCount[m]);

	printf("\ndnlkISymbErrCount: ");
	for(int m = 0; m<8; m++) printf("%08X    ", pFep->dnlkISymbErrCount[m]);

	printf("\ndnlkStatusErrCount: ");
	for(int m = 0; m<8; m++) printf("%08X    ", pFep->dnlkStatusErrCount[m]);

	printf("\ndnlkErrCount: ");
	for(int m = 0; m<8; m++) printf("%08X    ", pFep->dnlkErrCount[m]);

	printf("\ncErrChkSymb[00-07]: ");
	for(int m = 0; m<8; m++)   printf("%08X    ", pFep->cErrChkSymb[m]);
	printf("\ncErrChkSymb[08-15]: ");
	for(int m = 8; m<16; m++)  printf("%08X    ", pFep->cErrChkSymb[m]);
	printf("\ncErrChkSymb[16-23]: ");
	for(int m = 16; m<24; m++) printf("%08X    ", pFep->cErrChkSymb[m]);
	printf("\ncErrChkSymb[24-33]: ");
	for(int m = 24; m<34; m++) printf("%08X    ", pFep->cErrChkSymb[m]);

    printf("\nbMpTest[%08X]\r\nbRevTest[%08X]\r\nRevType[%08X]\n",
		    pFep->bMpTest,    pFep->bRevTest,  pFep->RevType);
	#else
	int i;
	FEPStatusBuffer * pFep = (FEPStatusBuffer*)dataPtr;
       printf("FepId:%08x,btsSeqId:%08x,TDDtimeslot:%x,Dnlktimeslot:%x\n",pFep->FepId,pFep->btsSeqId,pFep->TDDtimeslot,pFep->Dnlktimeslot);
       printf("preambleScale:%08x,antennaMask:%08x,SCGMask:%x\n",pFep->preambleScale,pFep->antennaMask,pFep->SCGMask);
	printf("W0:");
	for( i =0 ;i < 8; i++)
	{
	   printf("%08x,",pFep->W0[i]);
	}
	printf("\n");
	printf("auxmagCnt[0] =%08x,auxmagCnt[1] = %08x\n",pFep->auxmagCnt[0],pFep->auxmagCnt[1]);
	printf("auxChkErr:%x,cUplk:%x,cDnlk:%x\n",pFep->auxChkErr,pFep->cUplk,pFep->cDnlk);
      	printf("dnlkCount:");
	for( i =0 ;i < 8; i++)
	{
	   printf("%08x,",pFep->dnlkCount[i]);
	}
	printf("\n");
	  printf("dnlkISymbErrCount:");
	for(i =0 ;i < 8; i++)
	{
	   printf("%08x,",pFep->dnlkISymbErrCount[i]);
	}
	printf("\n");
	  printf("dnlkErrCount:");
	for( i =0 ;i < 8; i++)
	{
	   printf("%08x,",pFep->dnlkErrCount[i]);
	}
	printf("\n");
      	 printf("cErrChkSymb[1-17]:");
	for( i =0 ;i < 17; i++)
	{
	   printf("%x,",pFep->cErrChkSymb[i]);
	}
	printf("\n");
       printf("cErrChkSymb[18-34]:");
	for( i =0 ;i < 17; i++)
	{
	   printf("%x,",pFep->cErrChkSymb[i+17]);
	}
	printf("\n");

	printf("bMpTest:%x,bRevTest:%x,RevType:%x,FlagTestFpgaUp:%x,FlagTestFpgaDn:%x\n",pFep->bMpTest,pFep->bRevTest,pFep->RevType,pFep->FlagTestFpgaUp,pFep->FlagTestFpgaDn);

	printf("UpDataErrCnt[0]:%x,UpDataErrCnt[1]:%x\n",pFep->UpDataErrCnt[0],pFep->UpDataErrCnt[1]);
	#endif
}


void DisplayCpuMCPStatus(UINT32* dataPtr)
{
    MCPCDIBufType * pMcp = (MCPCDIBufType*)dataPtr;
    #if 0
    printf( "\r\ntest_uplink_report_tsn_err           [0X%08x]"
		    "\r\ntest_ppc_length_zeros                [0X%08x]"
			"\r\ntest_ppc_status_empty                [0X%08x]"
			"\r\ntest_ppc2mcp_checksum_err            [0X%08x]"          
			"\r\ntest_ppc_status_invalid              [0X%08x]"
			"\r\ntest_ppc2mcp_int_counter_err         [0X%08x]"
			"\r\ntest_ppc2mcp_tsn_err                 [0X%08x]"
			"\r\ntest_downlink_proc_err_times         [0X%08x]"
			"\r\ntest_TDD_EDMA_not_over_counter       [0X%08x]"
			"\r\ntest_PPC2MCPint_EDMA_not_over_counter[0X%08x]"
			"\r\ntest_ppc_length_too_long             [0X%08x]"
			"\r\ntest_mcp2ppc_length_too_long         [0X%08x]"	
			"\r\ntest_mcp2ppc_EDMA_not_over_counter   [0X%08x]"
			"\r\ntest_mcp2ppc_status_not_empty_err    [0X%08x]"	
			"\r\ntest_fep0_checksum_err               [0X%08x]"
			"\r\ntest_fep1_checksum_err               [0X%08x]"	
			"\r\ntest_fep01_status_len_err            [0X%08x]"
			"\r\ntest_fep0_status_len_err             [0X%08x]"
			"\r\ntest_fep1_status_len_err             [0X%08x]"
			"\r\ntest_symb_idx_err                    [0X%08x]"
			"\r\ntest_mcpId_err_counter               [0X%08x]"
			"\r\ntest_TDD                             [0X%08x]"
			"\r\ntest_uplink_deal_times               [0X%08x]"
			"\r\ntest_ppc2mcp_int                     [0X%08x]"
			"\r\ntest_edma_over                       [0X%08x]"       	
			"\r\ntest_deal_with_ppc_data              [0X%08x]"	
			"\r\ntest_before_start_mcp2ppc_dma_counter[0X%08x]"
			"\r\ntest_start_ppc2mcp_dma_counter       [0X%08x]"
			"\r\ntest_downlink_proc_times             [0X%08x]"
			"\r\ntest_fep0_read                       [0X%08x]"
			"\r\ntest_fep1_read                       [0X%08x]"
			"\r\ntest_fep_read                        [0X%08x]"
			"\r\ntest_start_mcp2ppc_dma_counter       [0X%08x]"
			"\r\ntest_shutdown_nulling                [0X%08x]"
			"\r\nmcpId                                [0X%08x]"
			"\r\nsch_num                              [0X%08x]"
			"\r\nschIdx_begin                         [0X%08x]"
			"\r\nschIdx_end                           [0X%08x]"
			"\r\nDownlink_DC_sch_posi                 [0X%08x]"
			"\r\nDownlink_DC_sca_posi                 [0X%08x]"
			"\r\nuplink_DC_sch_posi                   [0X%08x]"
			"\r\nuplink_DC_sca_posi                   [0X%08x]"
			"\r\nuplink_DC_sch_posi2                  [0X%08x]"
			"\r\nuplink_DC_sca_posi2                  [0X%08x]"
			"\r\ntwo_SCG                              [0X%08x]"
			"\r\nsch_in_first_SCG                     [0X%08x]"
			"\r\nBTS_sensitivity                      [0X%08x]"
			"\r\nBTS_power                            [0X%08x]"			
			"\r\nmcp2fep_dma_size                     [0X%08x]",

			pMcp->test_uplink_report_tsn_err,
			pMcp->test_ppc_length_zeros,
			pMcp->test_ppc_status_empty,
			pMcp->test_ppc2mcp_checksum_err,          
			pMcp->test_ppc_status_invalid,
			pMcp->test_ppc2mcp_int_counter_err,
			pMcp->test_ppc2mcp_tsn_err,
			pMcp->test_downlink_proc_err_times,
			pMcp->test_TDD_EDMA_not_over_counter,
			pMcp->test_PPC2MCPint_EDMA_not_over_counter,
			pMcp->test_ppc_length_too_long,
			pMcp->test_mcp2ppc_length_too_long,	
			pMcp->test_mcp2ppc_EDMA_not_over_counter,
			pMcp->test_mcp2ppc_status_not_empty_err,	
			pMcp->test_fep0_checksum_err,
			pMcp->test_fep1_checksum_err,	
			pMcp->test_fep01_status_len_err,
			pMcp->test_fep0_status_len_err,
			pMcp->test_fep1_status_len_err,
			pMcp->test_symb_idx_err,
			pMcp->test_mcpId_err_counter,
			pMcp->test_TDD,
			pMcp->test_uplink_deal_times,
			pMcp->test_ppc2mcp_int,
			pMcp->test_edma_over,       	
			pMcp->test_deal_with_ppc_data,	
			pMcp->test_before_start_mcp2ppc_dma_counter,
			pMcp->test_start_ppc2mcp_dma_counter,
			pMcp->test_downlink_proc_times,
			pMcp->test_fep0_read,
			pMcp->test_fep1_read,
			pMcp->test_fep_read,
			pMcp->test_start_mcp2ppc_dma_counter,
			pMcp->test_shutdown_nulling,
			pMcp->mcpId,
			pMcp->sch_num,
			pMcp->schIdx_begin,
			pMcp->schIdx_end,
			pMcp->Downlink_DC_sch_posi,
			pMcp->Downlink_DC_sca_posi,
			pMcp->uplink_DC_sch_posi,
			pMcp->uplink_DC_sca_posi,
			pMcp->uplink_DC_sch_posi2,
			pMcp->uplink_DC_sca_posi2,
			pMcp->two_SCG,
			pMcp->sch_in_first_SCG,
			pMcp->BTS_sensitivity,
			pMcp->BTS_power,			
			pMcp->mcp2fep_dma_size
			);
	#endif
	 printf( "\r\ntest_uplink_report_tsn_err           [0X%08x]",pMcp->test_uplink_report_tsn_err);
	 printf( "\r\ntest_ppc_length_zeros          [0X%08x]",pMcp->test_ppc_length_zeros);
	 printf( "\r\ntest_ppc_status_empty           [0X%08x]",pMcp->test_ppc_status_empty);
	 printf( "\r\ntest_ppc2mcp_checksum_err           [0X%08x]",pMcp->test_ppc2mcp_checksum_err);
	 printf( "\r\ntest_ppc_status_invalid          [0X%08x]",pMcp->test_ppc_status_invalid);
	 printf( "\r\ntest_ppc2mcp_int_counter_err          [0X%08x]",pMcp->test_ppc2mcp_int_counter_err);
	 printf( "\r\ntest_ppc2mcp_tsn_err          [0X%08x]",pMcp->test_ppc2mcp_tsn_err);
	 printf( "\r\ntest_downlink_proc_err_times           [0X%08x]",pMcp->test_downlink_proc_err_times);
	 printf( "\r\ntest_TDD_EDMA_not_over_counter          [0X%08x]",pMcp->test_TDD_EDMA_not_over_counter);
	 printf( "\r\ntest_PPC2MCPint_EDMA_not_over_counter           [0X%08x]",pMcp->test_PPC2MCPint_EDMA_not_over_counter);
	 printf( "\r\ntest_ppc_length_too_long           [0X%08x]",pMcp->test_ppc_length_too_long);
	 printf( "\r\ntest_mcp2ppc_length_too_long           [0X%08x]",pMcp->test_mcp2ppc_length_too_long);
	 printf( "\r\ntest_mcp2ppc_EDMA_not_over_counter           [0X%08x]",pMcp->test_mcp2ppc_EDMA_not_over_counter);
	 printf( "\r\ntest_mcp2ppc_status_not_empty_err           [0X%08x]",pMcp->test_mcp2ppc_status_not_empty_err);
	 printf( "\r\ntest_fep0_checksum_err           [0X%08x]",pMcp->test_fep0_checksum_err);
	 printf( "\r\ntest_fep1_checksum_err           [0X%08x]",pMcp->test_fep1_checksum_err);
	 printf( "\r\ntest_fep01_status_len_err           [0X%08x]",pMcp->test_fep01_status_len_err);
	 printf( "\r\ntest_fep0_status_len_err           [0X%08x]",pMcp->test_fep0_status_len_err);
	 printf( "\r\ntest_fep1_status_len_err           [0X%08x]",pMcp->test_fep1_status_len_err);
	 printf( "\r\ntest_symb_idx_err           [0X%08x]",pMcp->test_symb_idx_err);
	 printf( "\r\ntest_mcpId_err_counter           [0X%08x]",pMcp->test_mcpId_err_counter);
	 printf( "\r\ntest_TDD           [0X%08x]",pMcp->test_TDD);
	 printf( "\r\ntest_uplink_deal_times           [0X%08x]",pMcp->test_uplink_deal_times);
	 printf( "\r\ntest_ppc2mcp_int           [0X%08x]",pMcp->test_ppc2mcp_int);
	 printf( "\r\ntest_edma_over           [0X%08x]",pMcp->test_edma_over);
	 printf( "\r\ntest_deal_with_ppc_data           [0X%08x]",pMcp->test_deal_with_ppc_data);
	 printf( "\r\ntest_before_start_mcp2ppc_dma_counter           [0X%08x]",pMcp->test_before_start_mcp2ppc_dma_counter);
	 printf( "\r\ntest_start_ppc2mcp_dma_counter           [0X%08x]",pMcp->test_start_ppc2mcp_dma_counter);
	 printf( "\r\ntest_downlink_proc_times           [0X%08x]",pMcp->test_downlink_proc_times);
	 printf( "\r\ntest_fep0_read           [0X%08x]",pMcp->test_fep0_read);
	 printf( "\r\ntest_fep1_read           [0X%08x]",pMcp->test_fep1_read);
	 printf( "\r\ntest_fep_read           [0X%08x]",pMcp->test_fep_read);
	 printf( "\r\ntest_start_mcp2ppc_dma_counter           [0X%08x]",pMcp->test_start_mcp2ppc_dma_counter);
	 printf( "\r\ntest_shutdown_nulling           [0X%08x]",pMcp->test_shutdown_nulling);
	 printf( "\r\nmcpId           [0X%08x]",pMcp->mcpId);
	 printf( "\r\nsch_num           [0X%08x]",pMcp->sch_num);
	 printf( "\r\nschIdx_begin           [0X%08x]",pMcp->schIdx_begin);
	 printf( "\r\nschIdx_end           [0X%08x]",pMcp->schIdx_end);
	 printf( "\r\nDownlink_DC_sch_posi           [0X%08x]",pMcp->Downlink_DC_sch_posi);
	 printf( "\r\nDownlink_DC_sca_posi           [0X%08x]",pMcp->Downlink_DC_sca_posi);
	 printf( "\r\nuplink_DC_sch_posi           [0X%08x]",pMcp->uplink_DC_sch_posi);
	 printf( "\r\nuplink_DC_sca_posi           [0X%08x]",pMcp->uplink_DC_sca_posi);
	 printf( "\r\nuplink_DC_sch_posi2           [0X%08x]",pMcp->uplink_DC_sch_posi2);
	 printf( "\r\nuplink_DC_sca_posi2           [0X%08x]",pMcp->uplink_DC_sca_posi2);
	 printf( "\r\ntwo_SCG           [0X%08x]",pMcp->two_SCG);
	 printf( "\r\nsch_in_first_SCG           [0X%08x]",pMcp->sch_in_first_SCG);
	 printf( "\r\nBTS_sensitivity           [0X%08x]",pMcp->BTS_sensitivity);
	 printf( "\r\nBTS_power           [0X%08x]",pMcp->BTS_power);
	 printf( "\r\nmcp2fep_dma_size           [0X%08x]",pMcp->mcp2fep_dma_size);
	

}
#ifdef WBBU_CODE
void DisplayCore9Status(UINT32* dataPtr)
{
 Core9TestCounterControl * pCore9= (Core9TestCounterControl*)dataPtr;

	printf(" \r\nFrame_Counter:%d",pCore9->Frame_Counter);
	
	printf(" \r\nRecv_AUX_GPIO_counter:%d",pCore9->Recv_AUX_GPIO_counter);
	printf(" \r\nRecv_AUX_AIF_Get_counter:%d",pCore9->Recv_AUX_AIF_Get_counter);
	printf(" \r\nRecv_AUX_AIF_Lost_counter:%d",pCore9->Recv_AUX_AIF_Lost_counter);
	printf("  \r\nRecv_AUX_EDMA_counter:%d",pCore9->Recv_AUX_EDMA_counter);
	printf(" \r\nRecv_AUX_AIF_Link_Error_counter:%d",pCore9->Recv_AUX_AIF_Link_Error_counter);

	printf("  \r\nRecv_DSP2_GPIO_counter:%d",pCore9->Recv_DSP2_GPIO_counter);
	printf(" \r\nRecv_DSP2_AIF_Get_counter:%d",pCore9->Recv_DSP2_AIF_Get_counter);
	printf(" \r\nRecv_DSP2_AIF_Lost_counter:%d",pCore9->Recv_DSP2_AIF_Lost_counter);
	printf("  \r\nRecv_DSP2_EDMA_counter:%d",pCore9->Recv_DSP2_EDMA_counter);
	printf(" \r\nRecv_DSP2_AIF_Link_Error_counter:%d",pCore9->Recv_DSP2_AIF_Link_Error_counter);

	printf(" \r\nRecv_DSP3_GPIO_counter:%d",pCore9->Recv_DSP3_GPIO_counter);
	printf("  \r\nRecv_DSP3_AIF_Get_counter:%d",pCore9->Recv_DSP3_AIF_Get_counter);
	printf(" \r\nRecv_DSP3_AIF_Lost_counter:%d",pCore9->Recv_DSP3_AIF_Lost_counter);
	printf("  \r\nRecv_DSP3_EDMA_counter:%d",pCore9->Recv_DSP3_EDMA_counter);
	printf("  \r\nRecv_DSP3_AIF_Link_Error_counter:%d",pCore9->Recv_DSP3_AIF_Link_Error_counter);
	
	printf(" \r\nRecv_DSP4_EDMA_counter:%d",pCore9->Recv_DSP4_EDMA_counter);
	
	printf("  \r\nRecv_MAC_GPIO_counter:%d",pCore9->Recv_MAC_GPIO_counter);
	printf(" \r\nRecv_MAC_AIF_Get_counter:%d",pCore9->Recv_MAC_AIF_Get_counter);
	printf(" \r\nRecv_MAC_AIF_Lost_counter:%d",pCore9->Recv_MAC_AIF_Lost_counter);
	printf(" \r\nRecv_MAC_EDMA_counter:%d",pCore9->Recv_MAC_EDMA_counter);
	printf("  \r\nRecv_MAC_AIF_Link_Error_counter:%d",pCore9->Recv_MAC_AIF_Link_Error_counter);

	printf(" \r\nRecv_MCP0_Error_Counter:%d",pCore9->Recv_MCP0_Error_Counter);
	printf("  \r\nRecv_MCP1_Error_Counter:%d",pCore9->Recv_MCP1_Error_Counter);	
	printf(" \r\n Recv_MCP2_Error_Counter:%d",pCore9->Recv_MCP2_Error_Counter);	
	printf(" \r\nRecv_MCP3_Error_Counter:%d",pCore9->Recv_MCP3_Error_Counter);	
	printf("  \r\nRecv_MCP4_Error_Counter:%d",pCore9->Recv_MCP4_Error_Counter);	
	printf(" \r\nRecv_MCP5_Error_Counter:%d",pCore9->Recv_MCP5_Error_Counter);
	printf(" \r\nRecv_MCP6_Error_Counter:%d",pCore9->Recv_MCP6_Error_Counter);	
	printf(" \r\nRecv_MCP7_Error_Counter:%d",pCore9->Recv_MCP7_Error_Counter);	
	printf("  \r\nRecv_MAC_Error_Counter:%d",pCore9->Recv_MAC_Error_Counter);

	printf(" \r\nSend_MCP_Counter:%d",pCore9->Send_MCP_Counter);
	printf("  \r\nSend_AUX_Counter:%d",pCore9->Send_AUX_Counter);
	printf(" \r\nSend_MAC_Counter:%d",pCore9->Send_MAC_Counter);

	printf("  \r\nMCP_Proc_Counter:%d",pCore9->MCP_Proc_Counter);
	printf("  \r\nMAC_Proc_Counter:%d",pCore9->MAC_Proc_Counter);
	printf(" \r\nAUX_Proc_Counter:%d",pCore9->AUX_Proc_Counter);

	printf("  \r\nAIF_Reconfig_Counter:%d\n",pCore9->AIF_Reconfig_Counter);

}
#endif

void DisplayCpuStatus( DIAG_CPU_TYPE cpuType, UINT32* dataPtr)
{
    switch(cpuType)
    { 
		case CPU_AUX:
			DisplayCpuAUXStatus(dataPtr);
			break;
        case CPU_FEP:
			DisplayCpuFEPStatus(dataPtr);
			break;
        case CPU_MCP:
			DisplayCpuMCPStatus(dataPtr);
			break;
#ifdef WBBU_CODE
	case  CPU_CORE9:
		      DisplayCore9Status(dataPtr);
			break;
#endif
		default:
            printf("\r\nError CPU TYPE!\r\n");
			break;
	}
}

void CL3TaskDiag::ProcessStatusQueryResponse(CComMessage *pComMsg)
{
    T_DiagStatusQueryResp *resp = (T_DiagStatusQueryResp*)pComMsg->GetDataPtr();

    if (resp->cpuInfo.TransactionId != DiagCommandTransactionID)
    {
        return;
    }

    DIAG_CPU_INDEX cpuIndex = GetCpuIndex(resp->cpuInfo.cpuType, resp->cpuInfo.cpuInstance);
    printf( "%s Status \n", CpuInfo[cpuIndex].CpuName);

    DisplayCpuStatus((DIAG_CPU_TYPE)resp->cpuInfo.cpuType, &resp->statusData[0]);
    printf( "%s Status Infor End.\n", CpuInfo[cpuIndex].CpuName);
    CommandCompleteCheck(cpuIndex);
}


void CL3TaskDiag::ProcessCpuPingResponse(CComMessage *pComMsg)
{
    T_DiagGeneral *resp = (T_DiagGeneral*)pComMsg->GetDataPtr();

    if (resp->TransactionId != DiagCommandTransactionID)
    {
        return;
    }

    DIAG_CPU_INDEX cpuIndex = GetCpuIndex(resp->cpuType, resp->cpuInstance);
    printf( "%s is alive\n", CpuInfo[cpuIndex].CpuName);

    CommandCompleteCheck(cpuIndex);
}


void CL3TaskDiag::ProcessRpcResponse(CComMessage *pComMsg)
{
    T_DiagRpcResult *resp = (T_DiagRpcResult*)pComMsg->GetDataPtr();

    if (resp->cpuInfo.TransactionId != DiagCommandTransactionID)
    {
        return;
    }

    DIAG_CPU_INDEX cpuIndex = GetCpuIndex(resp->cpuInfo.cpuType, resp->cpuInfo.cpuInstance);

    CommandCompleteCheck(cpuIndex);

}


void CL3TaskDiag::ProcessInfoDisplay(CComMessage *pComMsg)
{
    T_DiagDisplayInfo *dispInfo = (T_DiagDisplayInfo*)pComMsg->GetDataPtr();

    DIAG_CPU_INDEX cpuIndex = GetCpuIndex(dispInfo->cpuInfo.cpuType, dispInfo->cpuInfo.cpuInstance);
    printf( "<%s> : %s \n", CpuInfo[cpuIndex].CpuName, &dispInfo->displayInfo[0]);
    printf("\n");

    char *infoStr = new char[dispInfo->displayInfoLen+1];
    if (infoStr)
    {
        memcpy( infoStr, &dispInfo->displayInfo[0], dispInfo->displayInfoLen);
        infoStr[dispInfo->displayInfoLen] = '\0';
        DIAG_CPU_INDEX cpuIndex = GetCpuIndex(dispInfo->cpuInfo.cpuType, dispInfo->cpuInfo.cpuInstance);
        printf( "<%s>  : %s \n", CpuInfo[cpuIndex].CpuName,infoStr);

        delete [] infoStr;
    }
}

#ifndef WBBU_CODE
extern void resetExcepL3();
void CL3TaskDiag::ProcessTimeOut()
{
    for (int i=0; i<CPU_INDEX_MAX; i++)
    {
        if (WaitingForCpuResponse[i])
        {
            if(OneMinQuryVerFlag==false)//lijinan 20081014
            	printf("    no response from %s\n", CpuInfo[i].CpuName);
	     else
		LOG1(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, "    no response from %s\n", (int)CpuInfo[i].CpuName);
            WaitingForCpuResponse[i] = false;
			
			//lijinan 20081013
	     CpuNoResponseCount[i]++;
	     if(CpuNoResponseCount[i]>=3)
	     {
			if(i==0)//L2	
			{
				resetExcepL3();	//复位三层以下
				printf("\nCannt get L2 version in 3mins,reboot Bts except L3\n");
			}
			else if(i==1)//AUX
			{
				rebootAUX();
				printf("\nCannt get AUX version in 3mins,reboot AUX\n");
			}
			else if(i<10)//MCP
			{
				rebootMCP((i-CPU_INDEX_MCP0));
				printf("\nCannt get MCP%d version in 3mins,reboot MCP%d\n",(i-CPU_INDEX_MCP0),(i-CPU_INDEX_MCP0));
			}
			else if(i<CPU_INDEX_MAX)//FEP
			{
				printf("\nCannt get FEP%d version in 3mins,reboot FEP%d\n",(i-CPU_INDEX_FEP0),(i-CPU_INDEX_FEP0));
			       rebootFEP((i-CPU_INDEX_FEP0));
				
			}
			 CpuNoResponseCount[i] = 0;
	     }
		 //end
        }
    }
   if(DiagResponseTimer)
   {
        DiagResponseTimer->Stop();
        delete DiagResponseTimer;
        DiagResponseTimer = NULL;
   }
    semGive(ShellCmdSem);
    if(OneMinQuryVerFlag == false)
   	 semGive(ShellCmdCompleteSem);
    OneMinQuryVerFlag = false;
}

extern "C" void bspGetDeviceID(UINT8 *device);
#else
void CL3TaskDiag::ProcessTimeOut()
{
#ifndef WBBU_CODE
    for (int i=0; i<CPU_INDEX_MAX; i++)
    {
        if (/*WaitingForCpuResponse[i]*/)
        {
            if(OneMinQuryVerFlag==false)//lijinan 20081014
            	printf("    1no response from %s\n", CpuInfo[i].CpuName);
	     else
	     	{
		LOG1(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, "    no response from %s\n", (int)CpuInfo[i].CpuName);
	     	}
            WaitingForCpuResponse[i] = false;
			
			//lijinan 20081013
	     CpuNoResponseCount[i]++;
	     if(CpuNoResponseCount[i]>=3)
	     {
			if(i==CPU_INDEX_L2)//L2	
			{
				resetExcepL3();	//复位三层以下
				printf("\nCannt get L2 version in 3mins,reboot Bts except L3\n");
			}
			else if(i==CPU_INDEX_AUX)//AUX
			{
				rebootAUX();
				printf("\nCannt get AUX version in 3mins,reboot AUX\n");
			}
			else if(i<10)//MCP
			{
				rebootMCP((i-CPU_INDEX_MCP0));
				printf("\nCannt get MCP%d version in 3mins,reboot MCP%d\n",(i-CPU_INDEX_MCP0),(i-CPU_INDEX_MCP0));
			}
			else if(i<CPU_INDEX_MAX)//FEP
			{
				printf("\nCannt get FEP%d version in 3mins,reboot FEP%d\n",(i-CPU_INDEX_FEP0),(i-CPU_INDEX_FEP0));
			       rebootFEP((i-CPU_INDEX_FEP0));
				
			}
			 CpuNoResponseCount[i] = 0;
	     }
		 //end
        }
    }


#else
  unsigned char flag = 0;
  unsigned char need_flag_All = 0;
  unsigned char dsp1_reset_flag = 0;
  unsigned char dsp2_reset_flag = 0;
  unsigned char dsp3_reset_flag = 0;
  unsigned char dsp4_reset_flag = 0;
  unsigned char dsp5_reset_flag = 0;
   unsigned char rest_flag = 0;
    for (int i=0; i<CPU_INDEX_MAX; i++)
    {
        if (WaitingForCpuResponse[i])
        {
            if(OneMinQuryVerFlag==false)//lijinan 20081014
            	{
               	printf("    no response from %s\n", CpuInfo[i].CpuName);
            	}
	     else
	     	{
		LOG1(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, "    no response from %s\n", (int)CpuInfo[i].CpuName);
	     	}
            WaitingForCpuResponse[i] = false;
			
			//lijinan 20081013
	     CpuNoResponseCount[i]++;
	    DSP_Err[i].VersionNoRsp++;
	     if(CpuNoResponseCount[i]>=4)
	     {
			if(i==CPU_INDEX_L2)//L2
			{
			#ifdef NEW_DSP_BOOT
			dsp5_reset_flag = 1;
			rest_flag = Reset_Dsp(5,2);
			DSP_Reset_Time[4]++;
			CpuNoResponseCount[i] = 0;
			taskDelay(100);
			if(rest_flag==1)
			{
			AlarmReport(ALM_FLAG_SET,ALM_ENT_L3PPC, ALM_ENT_INDEX0, ALM_ID_DSP5_RESET, ALM_CLASS_INFO,STR_L3_Reset_DSP5_Warm); 
			}
			#else
				Reset_Dsp(5,0);
				Reset_Dsp(1,0);
				DSP_Reset_Time[4]++;
				DSP_Reset_Time[0]++;
			#endif
				 LOG_STR(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nCannt get L2version in 15s,reboot DSP 5\n");
			      break;
			}//	;		//暂时不复位二层
			else if(i==CPU_INDEX_AUX)//AUX
			{
				//rebootAUX();
				rest_flag= Reset_Dsp (1,2);
				dsp1_reset_flag = 1;
				DSP_Reset_Time[0]++;
				if((DSP_Reset_Time[0]%5)==0)
				{
				    LOG_STR(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nreboot DSP 1 over 5 times ,Reset Serdies\n");
				    ResetAifSerdies();
				}
				CpuNoResponseCount[i] = 0;
				 CpuNoResponseCount[CPU_INDEX_FEP0] = 0;
				  CpuNoResponseCount[CPU_INDEX_FEP1] = 0;
				 
				taskDelay(20);
#ifdef NEW_DSP_BOOT
                         	if(rest_flag==1)
                         	{
				LOG_STR(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nCannt get AUX version in 15s,reboot DSP 1\n");
                         		}
#else
			//	printf("\nCannt get AUX version in 15s,reboot DSP 11\n");	
#endif
                          	  AlarmReport(ALM_FLAG_SET,ALM_ENT_L3PPC,  ALM_ENT_INDEX0, ALM_ID_DSP1_RESET,ALM_CLASS_INFO,STR_L3_Reset_DSP1_Warm); 
				Need_Load_Cfg = 1;
							    if(randombch ==0)
								{

								if(g_dsp2_protect==1)
									{
										   L2AirLinkCfgRandomBCH();
										   randombch=5;
									}
							    }
							    else
							    {
							       //L2AirLinkCfg();	
							       //randombch=0;
							    }
			}
			else if(i<10)//MCP
			{
							    if(randombch ==0)
								{
								  if(g_dsp2_protect==1)
								  	{
									   L2AirLinkCfgRandomBCH();
									   randombch=5;
								  	}
							    }
							    else
							    {
							      // L2AirLinkCfg();	
							       //randombch=0;
							    }

			#ifdef NEW_DSP_BOOT
			    if((i == CPU_INDEX_MCP0)||(i ==CPU_INDEX_MCP1)||(i==CPU_INDEX_MCP2))
			    	{
			    	 if(dsp2_reset_flag==0)
			    	 	{
			    	   rest_flag= Reset_Dsp (2,2);
					   DSP_Reset_Time[1]++;
					   	if((DSP_Reset_Time[1]%5)==0)
						{
						  LOG_STR(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nreboot DSP 2 over 5 times ,Reset Serdies\n");
						    ResetAifSerdies();
						}
					   dsp2_reset_flag = 1;
				//	   i = CPU_INDEX_MCP3;
				LOG_STR1(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nCannt get MCP%d version in 15s,reboot DSP 2\n",(i-CPU_INDEX_MCP0));
				 CpuNoResponseCount[CPU_INDEX_MCP0] = 0;
				 CpuNoResponseCount[CPU_INDEX_MCP1] = 0;
				  CpuNoResponseCount[CPU_INDEX_MCP2] = 0;
				  if(rest_flag==1)
				  	{
				   		AlarmReport(ALM_FLAG_SET,ALM_ENT_L3PPC, ALM_ENT_INDEX0,  ALM_ID_DSP2_RESET,ALM_CLASS_INFO,STR_L3_Reset_DSP2_Warm); 
				  	}
					   taskDelay(20);
			    	 	}
			    	}
				else if((i == CPU_INDEX_MCP3)||(i ==CPU_INDEX_MCP4)||(i==CPU_INDEX_MCP5))
				{
				     if(dsp3_reset_flag==0)
				     	{
				     	  rest_flag= Reset_Dsp (3,2);
					   DSP_Reset_Time[2]++;
					   	if((DSP_Reset_Time[2]%5)==0)
						{
						   LOG_STR(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nreboot DSP 3 over 5 times ,Reset Serdies\n");
						    ResetAifSerdies();
						}
				//	   i = CPU_INDEX_MCP6;
					   dsp3_reset_flag  = 1;
					    CpuNoResponseCount[CPU_INDEX_MCP3] = 0;
				 CpuNoResponseCount[CPU_INDEX_MCP4] = 0;
				  CpuNoResponseCount[CPU_INDEX_MCP5] = 0;
				 LOG_STR1(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nCannt get MCP%d version in 15s,reboot DSP 3\n",(i-CPU_INDEX_MCP0));
                                  if(rest_flag==1)
                                  	{
					     AlarmReport(ALM_FLAG_SET, ALM_ENT_L3PPC, ALM_ENT_INDEX0, ALM_ID_DSP3_RESET, ALM_CLASS_INFO,STR_L3_Reset_DSP3_Warm); 
                                  	}
					   taskDelay(20);
				     	}
				}
				else if((i == CPU_INDEX_MCP6)||(i ==CPU_INDEX_MCP7))
				{
				        if(dsp4_reset_flag == 0)
				        {
				         CpuNoResponseCount[CPU_INDEX_MCP6] = 0;
				         CpuNoResponseCount[CPU_INDEX_MCP7] = 0;
				          CpuNoResponseCount[CPU_INDEX_CORE9] = 0;
				     	  rest_flag= Reset_Dsp (4,2);
					   DSP_Reset_Time[3]++;
					   	if((DSP_Reset_Time[3]%5)==0)
						{
						    LOG_STR(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nreboot DSP 4 over 5 times ,Reset Serdies\n");
						    ResetAifSerdies();
						}
					   g_send_2_DSP4 = 1;
					   dsp4_reset_flag = 1;
					   
					   LOG_STR1(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nCannt get MCP%d version in 15s,reboot DSP 4\n",(i-CPU_INDEX_MCP0));
					   taskDelay(20);
					   if(rest_flag==1)
					   	{
					     AlarmReport(ALM_FLAG_SET, ALM_ENT_L3PPC, ALM_ENT_INDEX0, ALM_ID_DSP4_RESET, ALM_CLASS_INFO,STR_L3_Reset_DSP4_Warm); 
					   	}
					   }
				}
			#else
				Reset_All_DSP();
				DSP_Reset_Time[0]++;
				DSP_Reset_Time[1]++;
				DSP_Reset_Time[2]++;
				DSP_Reset_Time[3]++;
				DSP_Reset_Time[4]++;
			
				//printf("\nCannt get MCP%d version in 15s,reboot all DSP \n",(i-CPU_INDEX_MCP0));
				flag = 0x55;
			#endif
				
				break;
			}
			else if(i<CPU_INDEX_CORE9)//FEP
			{
				if(dsp1_reset_flag == 0)
				{
			     //  rebootFEP((i-CPU_INDEX_FEP0));
			       rest_flag= Reset_Dsp (1,2);
			      DSP_Reset_Time[0]++;
				if((DSP_Reset_Time[0]%5)==0)
				{
				      LOG_STR(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nreboot DSP 1 over 5 times ,Reset Serdies\n");
					ResetAifSerdies();
				}
			       Need_Load_Cfg = 1;
				  CpuNoResponseCount[CPU_INDEX_AUX] = 0; 
				 CpuNoResponseCount[CPU_INDEX_FEP0] = 0;
				  CpuNoResponseCount[CPU_INDEX_FEP1] = 0;
				  if(rest_flag==1)
				  {
				  		AlarmReport(ALM_FLAG_SET,ALM_ENT_L3PPC,  ALM_ENT_INDEX0, ALM_ID_DSP1_RESET, ALM_CLASS_INFO,STR_L3_Reset_DSP1_Warm); 
				//   i = CPU_INDEX_CORE9;
				  }
				}
				LOG_STR1(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nCannt get FEP%d version in 15s,reboot DSP 1\n",(i-CPU_INDEX_FEP0));
				taskDelay(20);
			}
			else if(i<CPU_INDEX_MAX)
			{
			    	
				#ifdef NEW_DSP_BOOT
				if(dsp4_reset_flag==0)
				{
			     //  rebootFEP((i-CPU_INDEX_FEP0));
			       rest_flag= Reset_Dsp (4,2);			      
			       DSP_Reset_Time[3]++;	
				if((DSP_Reset_Time[3]%5)==0)
				{
				       LOG_STR(LOG_SEVERE, M_DIAG_DEBUG_INFO_CODE,"\nreboot DSP 4 over 5 times ,Reset Serdies\n");
					ResetAifSerdies();
				}
				 CpuNoResponseCount[i] = 0;
	                     g_send_2_DSP4 = 1;
	
			       taskDelay(20);
			       if(rest_flag==1)
			       {
			       	AlarmReport(ALM_FLAG_SET,ALM_ENT_L3PPC,  ALM_ENT_INDEX0, ALM_ID_DSP4_RESET,ALM_CLASS_INFO,STR_L3_Reset_DSP4_Warm); 
			       }
			       }
				 #else
			       rebootFEP((i-CPU_INDEX_FEP0));
			       Reset_Dsp (4,0);
			       Reset_Dsp (1,0);
			       DSP_Reset_Time[3]++;
			      DSP_Reset_Time[0]++;
				 #endif
			       Need_Load_Cfg = 1;
				LOG_STR(LOG_MAJOR, M_DIAG_DEBUG_INFO_CODE,"\nCannt get core 9 version in 15s,reboot DSP 4 \n");
			}
			#ifndef NEW_DSP_BOOT
			 CpuNoResponseCount[i] = 0;
			#endif
	     }
		 //end
        }
    }
#endif
    delete DiagResponseTimer;
    DiagResponseTimer = NULL;
    semGive(ShellCmdSem);
    if(OneMinQuryVerFlag == false)
   	 semGive(ShellCmdCompleteSem);
    OneMinQuryVerFlag = false;
    if(flag == 0x55)
    	{
    	       for (int j=0; j<CPU_INDEX_MAX; j++)
    	       	{
    	       	   CpuNoResponseCount[j]  = 0;
    	       	   WaitingForCpuResponse[j] =0;
    	       	}
    	}
	//增加保护，如果长时间不能恢复的话，复位
	for(int k = 0 ;k<4; k++)
	{
	      if(((DSP_Reset_Time[k] %10)==0)&&(DSP_Reset_Time[k]>0))
	      	{
	      	        LOG_STR1(LOG_MAJOR, M_DIAG_DEBUG_INFO_CODE,"\nDSP[%d] Reset Over 10 times Reset All Cpu except L3 \n",k+1);
	      	     need_flag_All = 1;
			
	      	}
	}
	if(need_flag_All==1)
	{
	  
	       Reset_ALL();
	}
}
#endif
void CL3TaskDiag::ProcessSerialNumReq(CComMessage *pComMsg)
{
#ifndef WBBU_CODE
	UINT16 ReqTranid = *(UINT16*)(pComMsg->GetDataPtr());

	//记录tranid
	serialNumRsp.TranId = ReqTranid;
	serialNumRsp.result = 0;
	//得到基带板序列号
	bspGetDeviceID(&serialNumRsp.l3_serialnum[0]);	
       //构造消息取得syn和rf板序列号       
	PostDiagMemoryPeekMsg(CPU_INDEX_AUX, 0xffff0020, 1,0,0);
	CComMessage *tmoMsg = new(this, 0)CComMessage;
	if (tmoMsg)
	{
		tmoMsg->SetDstTid(M_TID_DIAGM);
		tmoMsg->SetSrcTid(M_TID_DIAGM);
		tmoMsg->SetMessageId(MSGID_DIAG_SERIAL_NUM_TIME_OUT);
		 
		serialNumReqTimer = new CTimer(false, 1000, tmoMsg);
		if (serialNumReqTimer)
		{
			serialNumReqTimer->Start();
			return;
		}
	}
   #endif 
}
void CL3TaskDiag::ProcessSerialNumTimeout()
{
       printf("\naux no response\n");
	delete serialNumReqTimer;
	serialNumReqTimer = NULL;			
		
	CComMessage *pComMsg1 = new(this, sizeof(T_SerialNumRsp))CComMessage;
	if ( pComMsg1 )
	{
		pComMsg1->SetDstTid (M_TID_CM);
		pComMsg1->SetSrcTid (M_TID_DIAGM);
		pComMsg1->SetMessageId( SERIAL_NUM_GET_RSP);
		memcpy((UINT8*)pComMsg1->GetDataPtr(), &serialNumRsp, sizeof(T_SerialNumRsp));
		
		if ( false == CComEntity::PostEntityMessage(pComMsg1) )
		{
			pComMsg1->Destroy();			
			return;
		}
	}	
}
void CL3TaskDiag::GetSynSerialNum()
{
#ifndef WBBU_CODE
	PostDiagMemoryPeekMsg(CPU_INDEX_AUX, 0xffff0020, 1,0,0);
#endif
}

int OneMinPrintFlag = false;
int SetOneMinPrintFlag(int flag)
{
	OneMinPrintFlag = flag;
}
#ifndef WBBU_CODE
//增加对DSP版本的1分钟定时查询
int CL3TaskDiag::Process1MinTimeOut()
{
    if ( OK == semTake(ShellCmdSem, 1))
    {
        CComMessage *pComMsg = new(this, sizeof(T_DiagShellCommand))CComMessage;
        if ( pComMsg )
        {
            pComMsg->SetDstTid (M_TID_DIAGM);
            pComMsg->SetSrcTid (M_TID_DIAGM);
            pComMsg->SetMessageId( MSGID_DIAG_SHELL_CMD);
            T_DiagShellCommand *cmdPtr = (T_DiagShellCommand*)pComMsg->GetDataPtr();
            cmdPtr->diagCmd = DIAG_SHELL_CMD_VER_QUERY;
            cmdPtr->cpuName = CPU_NAME_ALL;
            cmdPtr->arg1 = 0;
            cmdPtr->arg2 = 0;
            cmdPtr->arg3 = 0;
            cmdPtr->arg4 = 0;

            if ( false == CComEntity::PostEntityMessage(pComMsg) )
            {
                pComMsg->Destroy();
                LOG_STR(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, " Failed to execute the diag command, failure in L3 system" );
                semGive(ShellCmdSem);
                return false;
            }
	     if(OneMinPrintFlag==false)
	     	OneMinQuryVerFlag = true;
	  
        }
    }
    else
    {
        LOG_STR(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, "Failed to execute the diag command,  another diag command is not finished\n");
        return false;
    }
  
}

#else
extern unsigned char Loading_Wrru_Code ;
unsigned char Version_Times =0;
unsigned char no_diag_flag = 0;
void set_diag_flag(unsigned char flag)
{
    no_diag_flag= flag;
}
//增加对DSP版本的1分钟定时查询
int CL3TaskDiag::Process1MinTimeOut()
{
     if(g_dsp2_protect==1)
     	{
 	    if(randombch ==0)
 		{
 	    }
 	    else
 	    {
 	    	randombch--;
 	    	if(randombch==0)
 	           L2AirLinkCfg();	
 	       //randombch=0;
 	    }
     	}
     if(no_diag_flag==1)
     	{
     	   return 0;
     	}
if(Version_Times>20)
{
//如果代码加载状态在100s内还没有复位的话，则继续响应取版本的流程
    Version_Times = 0;
    Loading_Wrru_Code = 0;
}
 if(Loading_Wrru_Code==0)//如果没有在代码加载状态，则取版本
 {
    Version_Times = 0;
    if ( OK == semTake(ShellCmdSem, 1))
    {
        CComMessage *pComMsg = new(this, sizeof(T_DiagShellCommand))CComMessage;
        if ( pComMsg )
        {
            pComMsg->SetDstTid (M_TID_DIAGM);
            pComMsg->SetSrcTid (M_TID_DIAGM);
            pComMsg->SetMessageId( MSGID_DIAG_SHELL_CMD);
            T_DiagShellCommand *cmdPtr = (T_DiagShellCommand*)pComMsg->GetDataPtr();
            cmdPtr->diagCmd = DIAG_SHELL_CMD_VER_QUERY;
            cmdPtr->cpuName = CPU_NAME_ALL;
            cmdPtr->arg1 = 0;
            cmdPtr->arg2 = 0;
            cmdPtr->arg3 = 0;
            cmdPtr->arg4 = 0;

            if ( false == CComEntity::PostEntityMessage(pComMsg) )
            {
                pComMsg->Destroy();
                LOG_STR(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, " Failed to execute the diag command1, failure in L3 system" );
                semGive(ShellCmdSem);
                return false;
            }
	     if(OneMinPrintFlag==false)
	     	OneMinQuryVerFlag = true;
	  
        }
    }
    else
    {
        LOG_STR(LOG_DEBUG, M_DIAG_DEBUG_INFO_CODE, "Failed to execute the diag command1,  another diag command is not finished\n");
	semGive(ShellCmdSem);
        return false;
    }
 }
 else
 	{
 	    Version_Times++;
 	}
  
}
//lijinan 20081013
#endif
void CL3TaskDiag::processRpcCfg(CComMessage *rMsg)
{
    int snr;
    int rangingOffset;
    int i;
    snr = *(UINT8*)(rMsg->GetDataPtr());
    rangingOffset = *((UINT8*)rMsg->GetDataPtr()+1);
    for(i=CPU_INDEX_MCP0; i<=CPU_INDEX_MCP7; i++)
        PostDiagRpcMsg((DIAG_CPU_INDEX)i, 0x500, (int)snr, (int)rangingOffset, 0);
    
}
extern "C"
STATUS diag(DIAG_SHELL_CMD cmd, DIAG_CPU_NAME cpu, int arg1, int arg2, int arg3, int arg4)
{
      CL3TaskDiag::GetInstance()->DiagCommandParse(cmd, cpu,arg1 ,arg2,arg3,arg4);
	  return 0;
}

#ifdef WBBU_CODE
extern "C"
STATUS diag1(DIAG_SHELL_CMD cmd, DIAG_CPU_NAME cpu, int arg1, int arg2, int arg3, int arg4)
{
       CL3TaskDiag::GetInstance()->DiagCommandParse_MAC(cmd, cpu,arg1 ,arg2,arg3,arg4);
	  return 0;
}

#endif
extern "C"
STATUS getSerialNum()
{
     #ifndef WBBU_CODE
	bspGetDeviceID(&serialNumRsp.l3_serialnum[0]);
	printf("\nBasebank serial num is: ");
	for(int i=0; i<20;i++)
	{
		printf("%x ", serialNumRsp.l3_serialnum[i]);
	}
	
	CL3TaskDiag::GetInstance()->GetSynSerialNum();
#endif
}
#ifdef WBBU_CODE
void Send_Data_Cfg_2_CM( UINT16 MsgId)
{

/*************************************
            MsgId = M_OAMSYS_CFG_INIT_L2DATA_NOTIFY; 
        }
        else if(BTS_CPU_TYPE_AUX == CpuType)
        {
            MsgId = M_OAMSYS_CFG_INIT_AUXDATA_NOTIFY; 
        }
        else if(BTS_CPU_TYPE_FEP == CpuType)
        {
            MsgId = M_OAMSYS_CFG_INIT_FEPDATA_NOTIFY; 


****************************/
   CComMessage *pComMsg = NULL;
   unsigned short *p;
   pComMsg = new (CL3TaskDiag::GetInstance(), 2) CComMessage;
   p =(unsigned short*) pComMsg->GetDataPtr();
   pComMsg->SetDstTid(M_TID_CM);
  pComMsg->SetSrcTid(M_TID_SYS);
   pComMsg->SetMoudlue(0);
   pComMsg->SetMessageId(MsgId);
   pComMsg->SetEID(0x12345678);
   p[0] = 0xffff;//transid;
   
   if(false==CComEntity::PostEntityMessage(pComMsg))
     {
     	     	printf("hello2\n",0,1,2,3,4,5);
     }
}

void print_dsp_err()
{
//  int i,j;
    printf("CPU INDEX Version No Response Count:\n");
   printf("CPU_INDEX_L2 :%x\n",DSP_Err[0].VersionNoRsp);
    printf("CPU_INDEX_AUX:%x\n",DSP_Err[1].VersionNoRsp);
    printf("CPU_INDEX_MCP0:%x\n",DSP_Err[2].VersionNoRsp);
    printf("CPU_INDEX_MCP1:%x\n",DSP_Err[3].VersionNoRsp);
    printf("CPU_INDEX_MCP2:%x\n",DSP_Err[4].VersionNoRsp);
    printf("CPU_INDEX_MCP3:%x\n",DSP_Err[5].VersionNoRsp);
    printf("CPU_INDEX_MCP4:%x\n",DSP_Err[6].VersionNoRsp);
    printf("CPU_INDEX_MCP5:%x\n",DSP_Err[7].VersionNoRsp);
    printf( "CPU_INDEX_MCP6:%x\n",DSP_Err[8].VersionNoRsp);
    printf("CPU_INDEX_MCP7:%x\n",DSP_Err[9].VersionNoRsp);
    printf("CPU_INDEX_FEP0:%x\n",DSP_Err[10].VersionNoRsp);
    printf("CPU_INDEX_FEP1:%x\n",DSP_Err[11].VersionNoRsp);
    printf("CPU_INDEX_CORE9:%x\n",DSP_Err[12].VersionNoRsp);
   
    printf("DSP Reset Count:\n");

    printf("AUX DSP 0:%x\n",DSP_Reset_Time[0]);
    printf("L1 DSP 1:%x\n",DSP_Reset_Time[1]);
    printf("L1 DSP 2:%x\n",DSP_Reset_Time[2]);
    printf("L1 DSP 3:%x\n",DSP_Reset_Time[3]);
    printf("L2 DSP 4:%x\n",DSP_Reset_Time[4]);
    printf("Reset DSP1 for no active user Times:%d\n",g_ResetDSP1_NOUser);
   
}
void clear_dsp_err()
{
    int i,j;
    for(i = 0;i<CPU_INDEX_MAX;i++)
    	{
     		 DSP_Err[i].Rest_Time = 0;
     		 DSP_Err[i].VersionNoRsp = 0;
    	}
    for(j = 0; j<MAX_DSP_NUM; j++)
    	{
    	
     	DSP_Reset_Time[j] = 0;
    	}
}



void Reset_Test()
{
	for(;;)
	{
		Reset_Dsp(5,2);
		taskDelay(1500);
		
		Reset_Dsp(4,2);
		taskDelay(500);
		Reset_Dsp(3,2);
		taskDelay(500);
		Reset_Dsp(2,2);
		taskDelay(500);
		Reset_Dsp(1,2);
		diag(ver,all,0,0,0,0);
		taskDelay(5000);
	}
}

extern  int  WRRU_Temperature;

extern "C" void Send_Temp_2Aux()
{
    printf("WRRU_Temperature:%d\n",WRRU_Temperature);
   diag(rpc,aux,0x200,1,WRRU_Temperature,0);
}

extern "C"  void Set_DSP4_info(unsigned int flag)
{
	 CL3TaskDiag *taskObj = CL3TaskDiag::GetInstance();
    CComMessage *diagMsg = new (taskObj, sizeof(T_DiagRpcReq))CComMessage;

    if ( diagMsg)
    {
        T_DiagRpcReq* dataPtr = (T_DiagRpcReq*)diagMsg->GetDataPtr();
        dataPtr->cpuInfo.TransactionId = 0xffff;
        dataPtr->cpuInfo.cpuType = 4;
        dataPtr->cpuInfo.cpuInstance = 0;
        if(flag==1)
        {
	        dataPtr->RpcIndex = 0x500;
	   }
        else
        {
        	      dataPtr->RpcIndex = 0x501;
        }
        dataPtr->arg0 = 1;
        dataPtr->arg1 = 1;
        diagMsg->SetDstTid(M_TID_L2MAIN);
        diagMsg->SetSrcTid(M_TID_DIAGM);
        diagMsg->SetMessageId(MSGID_DIAG_RPC_CMD);
	diagMsg->SetMoudlue(0);
        if (CComEntity::PostEntityMessage(diagMsg))
        {
           
        }
        else
        {
            diagMsg->Destroy();
        }
    }
}

void send_b(unsigned char flag)
{
       unsigned char ptr[64];
       int i = 0;
       for(i = 0; i< 64; i++)
       {
             ptr[i] = i;
       }
       for(i = 0; i<6 ;i++)
       {
           ptr[i] = 0xff;
       }
       if(flag==1)
      {
	       ptr[0] = 0x30;
	       ptr[1] = 0x31;
	       ptr[2] = 0x32;
	       ptr[3] = 0x33;
	       ptr[4] = 0x34;
	       ptr[5] = 0x35;
      }
       ptr[6] = 0x00;
       ptr[7] = 0xa0;
       ptr[8] = 0x1e;
       ptr[9] = 0x01;
       ptr[10] = 0x01;
      ptr[11] = 0x01;
      ptr[12] = 0xcc;
      ptr[13] = 0xcc;
      L3_L2_Ether_Packet(ptr,64);
      
}

struct ethmsghdr_t {
	UINT8 dst[6];
	UINT8 src[6];
	UINT16 proto_type;
	UINT16 info;
};

UINT8  dsp_mac_dsp1[]={0x30, 0x31, 0x32, 0x33, 0x34, 0x35};
UINT8 dsp_mac_dsp2[]={0x30, 0x31, 0x32, 0x33, 0x34, 0x36};
UINT8 dsp_mac_dsp3[]={0x30, 0x31, 0x32, 0x33, 0x34, 0x37};
UINT8 dsp_mac_dsp4[]={0x30, 0x31, 0x32, 0x33, 0x34, 0x38};
UINT8 dsp_mac_dsp5_core0[] ={0x0,0x1,0x2,0x3,0x4,0x5};
UINT8 dsp_mac_dsp5_core1[] ={0x10,0x11,0x12,0x13,0x14,0x15};
UINT8 self_mac[]={0x00, 0xa0, 0x1e, 1,1,1};
#include <sockLib.h>
#include <inetLib.h>
 UINT8 sbuf[400];
LOCAL unsigned char  print_cmd_flag = 0;

void PrintCMD(unsigned char flag)
{
    print_cmd_flag = flag;
}
/****************************************

a ----表示第几个dsp1         -1
                                      dsp2          2
                                      dsp3           3
                                      dsp4            4
 b--------------表示输入的内容

**********************************************/
void DSPCmd(int a,int b)
{
    int i = 0;
	typedef struct
{
    UINT16 DestTid;
    UINT16 SrcTid;
    UINT32 EID;
    UINT16 MsgId;
    UINT16 MsgLen;
    UINT16 Reserved;
    UINT16 UID;
} L2L3_MSG_HEADER;

	ethmsghdr_t *pmh;
	L2L3_MSG_HEADER *l2l3hdr;
	
	char *cp;
	int l;
	
	pmh=(ethmsghdr_t *)sbuf;
	if(a==1)
	{
		memcpy(pmh->dst, dsp_mac_dsp1,6);
	}
	else if(a==2)
	{
	      	memcpy(pmh->dst, dsp_mac_dsp2,6);
	}
	else if(a==3)
	{
	     	memcpy(pmh->dst, dsp_mac_dsp3,6);
	}
	else if(a==4)
	{
	     	memcpy(pmh->dst, dsp_mac_dsp4,6);
	}
	else
	{
	   printf("input dsp index(1-4)err\n");
	    return;
	}
		
	memcpy(pmh->src, self_mac,6);
	pmh->proto_type=htons(0xaaaa);
	pmh->info=htons(2);
	l2l3hdr=(L2L3_MSG_HEADER*)(pmh+1);
	l2l3hdr->DestTid=htons(57);
	l2l3hdr->SrcTid=htons(5);
	l2l3hdr->EID=0;
	l2l3hdr->MsgId=htons(MSGID_DIAG_COMMAND);
	l2l3hdr->Reserved=0;
	l2l3hdr->UID=0;
	cp=(char*)(l2l3hdr+1);
	strcpy(cp, (char*)b);
	l2l3hdr->MsgLen=strlen(cp)+1;

	l=sizeof(*pmh)+sizeof(*l2l3hdr)+l2l3hdr->MsgLen;
	if(print_cmd_flag==1)
	{
    for(i= 0; i<l;i++)
	{
		
		
			printf("%02x,",sbuf[i]);
		
		if((i!=0)&&(i%10==0))
		{
		   printf("\n");
		}
	}
	}
	L3_L2_Ether_Packet(sbuf, l);
	
}

/****************************************

core ----0  表示dsp5 的core0,1----表示dsp5 的core1
 b--------------表示输入的内容

 如L2Cmd 0,"showSession 0"
 表示看L2层showSession命令

**********************************************/
void L2Cmd(int core,int ptr)
{
    	typedef struct
	{
	    UINT16 DestTid;
	    UINT16 SrcTid;
	    UINT32 EID;
	    UINT16 MsgId;
	    UINT16 MsgLen;
	    UINT16 Reserved;
	    UINT16 UID;
	} L2L3_MSG_HEADER;
       int i = 0;
	ethmsghdr_t *pmh;
	L2L3_MSG_HEADER *l2l3hdr;
	
	char *cp;
	int l;
	
	pmh=(ethmsghdr_t *)sbuf;
	if(core==0)
	{
		memcpy(pmh->dst, dsp_mac_dsp5_core0,6);
	}
	else
	{
		 memcpy(pmh->dst, dsp_mac_dsp5_core1,6);
	}
	memcpy(pmh->src, self_mac,6);
	pmh->proto_type=htons(0xaaaa);
	pmh->info=htons(2);
	l2l3hdr=(L2L3_MSG_HEADER*)(pmh+1);
	l2l3hdr->DestTid=htons(57);
	l2l3hdr->SrcTid=htons(5);
	l2l3hdr->EID=0;
	l2l3hdr->MsgId=htons(MSGID_DIAG_COMMAND);
	l2l3hdr->Reserved=0;
	l2l3hdr->UID=0;
	cp=(char*)(l2l3hdr+1);
	strcpy(cp, (char*)ptr);
	l2l3hdr->MsgLen=strlen(cp)+1;

	l=sizeof(*pmh)+sizeof(*l2l3hdr)+l2l3hdr->MsgLen;
    if(print_cmd_flag==1)
    {
	for(i= 0; i<l;i++)
	{
		printf("%02x,",sbuf[i]);
		
		if((i!=0)&&(i%10==0))
		{
		   printf("\n");
		}
	}
    }

	L3_L2_Ether_Packet(sbuf, l);
}
#endif
