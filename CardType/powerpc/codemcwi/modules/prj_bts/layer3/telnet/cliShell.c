/*----------------------------------------------------------------------------------
    cliShell.c - 命令行模块用户输入解析 

版权所有 2004 -2006 信威公司深研所BSC项目组.

修改历史记录
--------------------

03.04.06e,  08-05-2004,     L.Y     创建
------------------------------------------------------------------------------------*/

/*
模块功能
该模块为解释用户在命令行上
所输入, 调用相应模块的处理函数
...
INCLUDE FILES: cliShell.h
*/

/* 
* 头文件 
*/

#include "sysos.h"
#include "msg.h"

#include "cliShell.h"
#include "cliTelnet.h"
#include "cliConfig.h"


typedef struct 
{
    char  user[40];
    char  password[40];
}T_BTSUserCfgEle;
extern T_BTSUserCfgEle gstNvUser;
#define THIS_FILE_ID FILE_CLISHELL_C

#define kCLI_HISTORY_BUFFER_SIZE    30

#define FID_CLI 1
#define FID_IP  2
#define FID_OAM 3
#define MAX_FID  4
/*
*外部变量定义
*/



/* 
* 全局变量 
*/

SYS_MSG_Q_ID  Msg_Q_Telnet;
SYS_SOCKET_ID CliSock = ERROR; // 记录临时输出Fd
SYS_SOCKET_ID PipeFd  = ERROR;
SYS_SOCKET_ID CliPipeFd = ERROR;
_INT IoChangeFlag = 0;
_INT RunShellFlag = 0;

_INT OrigGlobalIn = ERROR;
_INT OrigGlobalOut = ERROR;
_INT OrigGlobalErr = ERROR;
_INT HoldL2Flag=0;

_INT L2Shell = 0;
unsigned char ucOut2Telnet = 0;

/*lijinan 090108 for telnet all cmd*/
_UINT  uiAllCmdSupport = 0;

extern SYS_TASK_ID TelnetSvr;
/* 
* 本地变量 
*/

CLI_ENV    **gppCliSessions;

COM_CHAN gConsoleObject;


/* 命令解释节点*/

//CLI_SHELL_CMD cliShellCmd;

CLI_AP_CMDS    cliApCmds;

/*
* -----------------------------------------------------------------------------------
*  以下是命令结构树
*
*/

/* 根模式*/

CMD_DATAS   sysCmds[] = 
{
    {0, cmdHelp, "help", "help", "no parameter"},
    {0, cmdHelp, "?", "quick help", "no parameter"},
    {0, cmdCls, "cls", "clear screen", "no parameter"},
    {0, cmdExit, "exit", "quit", "no parameter"},
    {0, cmdEnterL2Shell, "l2sh", "enter the L2Shell", "no parameter"},
    {0, cmdShellParse, "pcishow", "Show PCI status", "verbose"},
    {0, cmdShellParse, "csiShow", "Show CSI status", "record"},
    {0, cmdShellParse, "ARPShow", "Show ARP information", "no parameter"},
    {0, cmdShellParse, "l3oamshowalarm", "Show l3 OAM alarm", "no parameter"},
    {0, cmdShellParse, "l3oamprintswversion", "Show SW version", "no parameter"},
    {0, cmdShellParse, "sysBootStateShow", "Show system boot state", "no parameter"},
    {0, cmdShellParse, "l3oamprintbtssysstatus", "Show system status", "no parameter"},
    {0, cmdDiagShow, "diag", "diag function", "N/A"},
    {0, cmdShellParse, "EBShow", "show EB information", \
    			" 1:basic information\n 2:forwarding table\n 3:performance information\
    			\n 4:MAC filtering table\n 5:QoS configuration"},
    {0, cmdShellParse, "SNShow", "show SN information", \
    			" 1:basic information\n 2:show CCBs in memory\n 3:show CCBs in NVRAM\
    			\n 4:show permance information\n 5:set PPPoE remote=false\n 6:set PPPoE remote=true"},	
    {0, cmdShellParse, "reboot", "reboot system", "no parameter"},
    {0, cmdDebugOpen, "dbgopen", "output dbg msg to telnet", "no parameter"},
    {0, cmdDebugClose, "dbgclose", "do not output dbg msg to telnet", "no parameter"},

     /*lijinan add 090108 for telnet all cmd output*/		
    {0, cmdOpen, "cmdOpen", "support all cmd for telnet 32", "no parameter"},
    {0, cmdClose, "cmdClose", "close all cmd for telnet 32", "no parameter"},
    {0, cmdShellParse, "cmd", "support all cmd", "no parameter"},

    /*命令次序不能错乱，新增命令在后面增加*/	
    {0,0,0,0,0}
};

//sunshanggu
CMD_DATAS L2SysCmds[] =
{
	 {0, cmdHelp, "help", "help'", "no parameter"},
	 {0, cmdHelp, "?", "help'", "no parameter"},
    {0, cmdCls, "cls", "clear screen", "no parameter"},
    {0, cmdL2Exit, "exit", "quit L2 shell", "no parameter"},
    {0, cmdL2Exit, "quit", "quit L2 shell", "no parameter"},
    {0, cmdL2Exit, "bye", "quit L2 shell", "no parameter"},
    {0, cmdL2ShellParse, "l2AlarmShow", "Show l2 alarm", "no parameter"},
    {0, cmdL2ShellParse, "l2statShow", "Show l2 state","no parameter"},
    {0, cmdL2ShellParse, "bmShow", "Show l2 system state", "no parameter"},
    {0, cmdL2ShellParse, "tddShow", "Show l1 tdd state", "no parameter"},
    {0, cmdL2ShellParse, "tddClear", "Clear l1 tdd state", "no paremeter"},
    {0, cmdL2ShellParse, "bmClear", "Clear l2 system state", "no paremeter"},
    {0, cmdL2ShellParse, "l2AlarmReset", "Reset l2 alarm", "no parameter"},
    {0, cmdL2ShellParse, "l2statReset", "Reset all l2 counters", "no parameter"},
    {0,0,0,0,0}



};



/* *************************************************
* 函数定义部分 */
_INT SendMsgTelnet(LONG_MESSAGE *pstMsg)
{
     _INT i, nResult;
    _UINT n;
	
    if(pstMsg == NULL)
    {
    	//SYS_ASSERT(0);
    	return FAIL;
    }




    if(pstMsg->stHeader.ucDstTaskNo == IP_TASK_ID)   /*发送给IP模块的消息*/
    {
        SYS_MsgSend2IP(pstMsg);
        return SUCC;
    }


    //printf("\n\r Send Msg To Telnet Task");
    nResult = SYS_MsgQSend(Msg_Q_Telnet, (_UCHAR *)pstMsg, \
            	   pstMsg->stHeader.usMsgLen+SYS_MSG_HEAD_LEN, NO_WAIT, MSG_PRI_NORMAL);
              
    return nResult;

}


/* 内部操作函数*/

RL_STATIC CLI_ENV * TELNETD_GetCliSession(_INT index)
{
    if (index > kCLI_MAX_CLI_TASK)
        return NULL;

    return gppCliSessions[index];
}

RL_STATIC COM_CHAN *TELNETD_GetConsoleChannel()
{
    return &gConsoleObject;
}


/*---------------------------------------------------------------------

 CLI_TELNETD_BroadcastMessage - 向所有客户端广播

 参数说明:

 返回值: 



------------------------------------------------------------------------*/

_INT CLI_TELNETD_BroadcastMessage(_CHAR *pMessage, _SHORT authLevel)
{
    _INT      index;
    CLI_ENV    *pCliDest;
    COM_CHAN *pChannel;

    for (index = 1;index < kCLI_MAX_CLI_TASK; index++)
    {
        pCliDest = TELNETD_GetCliSession(index);

        if (NULL == pCliDest)
            continue;

        pChannel = MMISC_GetChannel(pCliDest);

        if (kThreadWorking != pChannel->ThreadState)
            continue;

        if(kTELNET_USR_PROCESS != MMISC_GetLoginStat(pCliDest))
            continue;

        //CLI_EXT_WriteStr(pCliDest, pMessage);
        //CLI_EXT_PutStr(pCliDest, pMessage);
        CLI_EXT_WriteStrLine(pCliDest, pMessage); //sunshanggu	
    }
    return SUCC;
}


/*---------------------------------------------------------------------

 TELNETD_ConsoleRead - 从控制台读取一个字符


 参数说明:

 返回值: 


------------------------------------------------------------------------*/

RL_STATIC  _INT TELNETD_ConsoleRead(CLI_ENV *pEnv, _UCHAR charIn1, _CHAR *pBuf, _INT *bytesRead)
{
    _CHAR charIn = charIn1;
    
    *bytesRead = 0;

    if (kCLI_DOSKEY_ESC == charIn) 
    {
        charIn = GETCHAR();
        switch(charIn)
        {
            case kCLI_DOSKEY_UP:
                charIn = kKEY_MOVE_UP;
                break;
            case kCLI_DOSKEY_DN:  
                charIn = kKEY_MOVE_DOWN;
                break;
            case kCLI_DOSKEY_LT:  
                charIn = kKEY_MOVE_LEFT;
                break;
            case kCLI_DOSKEY_RT:
                charIn = kKEY_MOVE_RIGHT;
                break;
            case kCLI_DOSKEY_DEL:
                charIn = kKEY_DELETE_CHAR;
                break;
            default:
                break;
        }
    }

/* unix uses lf as end of line */
#ifndef WINDOWS
    if (kLF == charIn)
        charIn = kCR;
#endif

    pBuf[(*bytesRead)++] = charIn;

    return SUCC;
}

/*---------------------------------------------------------------------

 TELNETD_ConsoleWrite - 向控制台输出字符串


 参数说明:

 返回值: 


------------------------------------------------------------------------*/

RL_STATIC _INT TELNETD_ConsoleWrite(CLI_ENV *pEnv, _CHAR *pBuf, _INT BufSize)
{
	printf("\ncall output fun\n");
    fwrite(pBuf, 1, BufSize, stdout);
    fflush(stdout);
        
    return SUCC;
}



/*
* 为移植到新平台上,特增加2个主函数,
* 分别初始化和用户输入消息分发
*
*/

/*---------------------------------------------------------------------

 CLI_IniProc - 初始化
 
 参数说明：
 
 eStepId          － 输入，初始化的步骤

 返回值: OK, 函数操作失败返回ERROR


------------------------------------------------------------------------*/

_ULONG CLI_IniProc()
{
    _INT siIdx;
    CLI_ENV    *pCliEnv    = NULL;
    COM_CHAN *theConsole = TELNETD_GetConsoleChannel();

   
	
	initCmdTree();
	
	gppCliSessions = (CLI_ENV**) RC_CALLOC(1, sizeof(gppCliSessions) * kCLI_MAX_CLI_TASK);
	
	for(siIdx=0; siIdx <= kCLI_MAX_CLI_TASK; siIdx++)
	{        
		if (SUCC != CLI_DB_InitEnvironment(&pCliEnv, theConsole))
			return FAIL;   
		
		MMISC_SetFsmId(pCliEnv, siIdx);
		
		gppCliSessions[siIdx] = pCliEnv;                  
		
		if (SUCC != CLI_UTIL_Init(pCliEnv))
			return FAIL;                     
		
		if(!siIdx)
		{
			MCONN_SetReadHandle(pCliEnv, TELNETD_ConsoleRead);
			MCONN_SetWriteHandle(pCliEnv, TELNETD_ConsoleWrite);
			
			MCONN_SetConnType(pCliEnv,  kCONSOLE_CONNECTION);
			MSCRN_SetWidth(pCliEnv,          kCLI_DEFAULT_WIDTH);
			MSCRN_SetHeight(pCliEnv,         kCLI_DEFAULT_HEIGHT);
			
		}
		else
		{
			MCONN_SetReadHandle(pCliEnv, TELNET_RECV_FN);
			MCONN_SetWriteHandle(pCliEnv, TELNET_SEND_FN);
		}
		
		/* 命令树根节点*/
		
		CLIENV(pCliEnv)->cliShellCmd.cliMode = sysMd;
		CLIENV(pCliEnv)->cliShellCmd.pCmdLst = sysCmds;
		
		/* 修改提示符*/
		InitPrompt(pCliEnv);
		
		MMISC_SetLoginStat(pCliEnv, kTELNET_NOT_LOGIN);
		MEDIT_SetKeyStat(pCliEnv, KEY_STATE_DATA);
		
		CLI_HIST_InitHistInfo(pCliEnv);
	}

    if (FAIL==SYS_MsgQCreate(100, 0x200, &Msg_Q_Telnet))
    {
        printf("\n\rERR:InitMPUMsg SYS_MsgQCreate");
        return FAIL;
    }

    //初始化
     IoChangeFlag = 0;

     OrigGlobalIn = ERROR;
     OrigGlobalOut = ERROR;
     OrigGlobalErr = ERROR;
    //HoldIoFlag=0;
    return SUCC;
}

/*---------------------------------------------------------------------

 CLI_MsgProc - 根据用户输入的消息(对每个字符)进行分发处理

 参数说明：
 pstMsgHeader       － 输入，用户输入的字符

 返回值: OK, 函数操作失败返回ERROR

------------------------------------------------------------------------*/
_INT CLI_MsgProc(CliCOMMON_HEADER* pstMsgHeader)
{
	CliCOMMON_MESSAGE* pCh;
	CLI_ENV* pCliEnv;
	_INT    status;
	
	//减去偏移量
	pstMsgHeader->stSender.usFsmId = pstMsgHeader->stSender.usFsmId - (IP_FSM_CLI_FIRST_CLIENT-1);
	//printf("\nusFsmId=%x",pstMsgHeader->stSender.usFsmId);
	if(pstMsgHeader->stSender.usFsmId >= kCLI_MAX_CLI_TASK)
	{
		return FAIL;
	}
	
	pCh = (CliCOMMON_MESSAGE*) pstMsgHeader;
	
	pCliEnv = TELNETD_GetCliSession(pstMsgHeader->stSender.usFsmId);
	if(NULL == pCliEnv)
	{
		sendCloseReq(pstMsgHeader->stSender.usFsmId);
		return FAIL;
	}
	if(MMISC_GetFsmId(pCliEnv) != pstMsgHeader->stSender.usFsmId)
	{
		return FAIL;
	}

	switch(pstMsgHeader->usMsgId)
	{
		case MSG_IP_DATAIND:                     
			CLI_EXT_ReadCh(pCliEnv, pCh->ucBuffer, pCh->stHeader.usMsgLen);
			if(MMISC_GetLoginStat(pCliEnv) != kTELNET_USR_PROCESS)
				break;
			if(MMISC_GetInput(pCliEnv) == 0)
			{
				
				if (0 < MEDIT_GetLength(pCliEnv))
				{
					CLI_HIST_AddHistLine(pCliEnv);
					
					//查找命令
					
					status = ParseCommand(pCliEnv);
					
					switch (status)
					{
					case SUCC:
						// 执行
						ExcuteCommand(pCliEnv);
						break;
					case STATUS_CLI_INTERNAL_COMMAND:
					case STATUS_CLI_NO_INTERMEDIATE:
					case ERROR_CLI_NO_INPUT_DATA:
						break;
					default:
						CLI_TASK_PrintError(pCliEnv, status);
						break;
					}
					
				}
				
				CLI_EXT_ResetOutput(pCliEnv);
				if(CLIENV(pCliEnv)->cliShellCmd.cliMode != ExitCliShellMd)
					CLI_EXT_WriteStr(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.pscCursor);
				CLI_EXT_InitCommandLine(pCliEnv);
				
				switch(pstMsgHeader->stSender.usFsmId)
				{
					//distinguish console and telnet
				case 0:
					
					//console
					
					if(CLIENV(pCliEnv)->cliShellCmd.cliMode==ExitCliShellMd)
					{
						//loop again
						
						if (SUCC != CLI_UTIL_Init(pCliEnv))
							return FAIL;
						
						MMISC_SetLoginStat(pCliEnv, kTELNET_NOT_LOGIN);
						MHIST_History(pCliEnv)->iCurHistCmd  = 0;
						MHIST_History(pCliEnv)->iNumCmds     = 0;
						
						
					}
					
					break;
					
				default:
					
					//distinguish each client
					if(CLIENV(pCliEnv)->cliShellCmd.cliMode==ExitCliShellMd)
					{
						sendCloseReq(MMISC_GetFsmId(pCliEnv));
						
						/* 命令树根节点*/
						
						CLIENV(pCliEnv)->cliShellCmd.cliMode = sysMd;
						CLIENV(pCliEnv)->cliShellCmd.pCmdLst = sysCmds;
						
						/* 修改提示符*/
						InitPrompt(pCliEnv);
						MMISC_SetLoginStat(pCliEnv, kTELNET_NOT_LOGIN);
						MEDIT_SetKeyStat(pCliEnv, KEY_STATE_DATA);
						MHIST_History(pCliEnv)->iCurHistCmd  = 0;
						MHIST_History(pCliEnv)->iNumCmds     = 0;
						
						
						
					}                
					break;
				}
				
				
			}
			break;
		case MSG_IP_CLOSEIND:
			// close by remote client
			/* 命令树根节点*/
                     //printf("TELNET Recv IP Close\n");
			CLIENV(pCliEnv)->cliShellCmd.cliMode = sysMd;
			CLIENV(pCliEnv)->cliShellCmd.pCmdLst = sysCmds;
			
			/* 修改提示符*/
			InitPrompt(pCliEnv);          
			MMISC_SetLoginStat(pCliEnv, kTELNET_NOT_LOGIN);
			MEDIT_SetKeyStat(pCliEnv, KEY_STATE_DATA);
			MHIST_History(pCliEnv)->iCurHistCmd  = 0;
			MHIST_History(pCliEnv)->iNumCmds     = 0;
			
			//将调试消息输出到新的telnet关闭lijinan 20080902
			ucOut2Telnet = 0;
			uiAllCmdSupport = 0;
			
			break;
		
		case MSG_IP_CONNECTIND:
                    
                     //printf("TELNET Recv IP CONNECT\n");  
			if (SUCC != CLI_TELNET_Init(pCliEnv))
			{
				COM_CHAN *pChannel;
				CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
				CLI_EXT_WriteStr(pCliEnv, kCLI_MSG_FAIL);
				pChannel = MMISC_GetChannel(pCliEnv);
				pChannel->ThreadState = kThreadDead;
				pChannel->InUse       = FALSE;
				
				//CLI_TASK_Cleanup(pCliEnv);
				//printf("\n\r CLI_TELNET_Init succ  ");
				return SUCC;
			}
                     //printf("\n\r CLI_TELNET_Init failed  ");
			break;
		case MSG_IP_DATAREQ:
			
		
		//打印提示符
		//CLI_EXT_WriteStr(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.pscCursor);
			break;
		default:
			break;
		
	}
		

	return SUCC;
}
/*---------------------------------------------------------------------

 CLI_MsgTask - 根据用户输入的消息(对每个字符)进行分发处理

 参数说明：
 pstMsgHeader       － 输入，用户输入的字符

 返回值: OK, 函数操作失败返回ERROR

------------------------------------------------------------------------*/
_INT CLI_MsgTask()
{
	_INT i,nLen;
	//_UCHAR szCLIRevBuf[MAX_MSG_LEN];
	LONG_MESSAGE Msg;
	CliCOMMON_MESSAGE  stCmnMsg;
	
	FOREVER
	{
		nLen=SYS_MsgQRecv(Msg_Q_Telnet, (_UCHAR*)&Msg, MAX_MSG_LEN, WAIT_FOREVER);
              		

#ifdef VXWORKS		
		if(nLen < SYS_MSG_HEAD_LEN)
		{
			//PRINT(MD(MOD_ID_UP, PL_ERR),"CLI_MsgTask len=%d error",nLen);
			continue;
		}
		if((Msg.stHeader.usMsgLen + SYS_MSG_HEAD_LEN) != nLen)    /*队列消息接收长度与消息长度不一致*/
		{
			//PRINT(MD(MOD_ID_UP, PL_ERR),"CLI_MsgTask len=%d error",nLen);
			continue;
		}
		if(Msg.stHeader.ucDstTaskNo != CLI_TASK_ID) /*目的任务号错*/
		{
			//PRINT(MD(MOD_ID_UP, PL_ERR),"CLI_MsgTask TaskNo=%d error",Msg.stHeader.ucDstTaskNo);			
			continue;
		}
		
		if(Msg.stHeader.usMsgLen >= (MAX_MSG_LEN - SYS_MSG_HEAD_LEN))
		{
			//PRINT(MD(MOD_ID_UP, PL_ERR),"CLI_MsgTask Msg len=%d error",Msg.stHeader.usMsgLen);
			continue;
		}
#endif	
		
		stCmnMsg.stHeader.stReceiver.ucModId    = MODID;
		stCmnMsg.stHeader.stReceiver.ucFId      = FID_CLI;
		stCmnMsg.stHeader.stReceiver.usFsmId    = IP_FSM_CLI_SERVER;
		
		stCmnMsg.stHeader.stSender.ucModId    = MODID;
		stCmnMsg.stHeader.stSender.ucFId      = FID_IP;
		stCmnMsg.stHeader.stSender.usFsmId    = Msg.ucBuffer[Msg.stHeader.usMsgLen-1];
		
		stCmnMsg.stHeader.usMsgId       = Msg.stHeader.usMsgId;
		stCmnMsg.stHeader.usMsgLen     = Msg.stHeader.usMsgLen - 1;
		SYS_MEMCPY(stCmnMsg.ucBuffer, Msg.ucBuffer, stCmnMsg.stHeader.usMsgLen);

               
		CLI_MsgProc((CliCOMMON_HEADER *)&stCmnMsg);
		

	}	
	return SUCC;
}


_VOID sendCloseReq(_INT connId)
{
        //send close req
        CliCOMMON_MESSAGE stMsg;

        stMsg.stHeader.stReceiver.ucModId = MODID; /* liwenyan 060920 for telnet */
        stMsg.stHeader.stReceiver.ucFId   = FID_IP;
        stMsg.stHeader.stReceiver.usFsmId = connId + (IP_FSM_CLI_FIRST_CLIENT - 1);;

        stMsg.stHeader.stSender.ucModId   = MODID;
        stMsg.stHeader.stSender.ucFId     = FID_OAM;
        stMsg.stHeader.stSender.usFsmId   = BLANK_USHORT;

        stMsg.stHeader.usMsgId    = MSG_IP_CLOSEREQ;
        
        stMsg.stHeader.usMsgLen   = 0;
        //memcpy(stMsg.ucBuffer, pBuf, BufLen);

        //if(stMsg.stHeader.stReceiver.usFsmId)
        //SYS_MSGSEND(&stMsg);
    
}

/*---------------------------------------------------------------------

 ExcuteCommand - 执行命令

 参数说明：
    - 
    
 返回值: SUCC

 该函数执行找到命令的处理函数

------------------------------------------------------------------------*/

_INT ExcuteCommand(CLI_ENV *pCliEnv)
{
    _INT i;
    _CHAR cache[CLI_MAX_CMD_HELP_LEN+10];
    
    _CHAR *pscArgument[CLI_MAX_ARG_NUM];
    
    _VOID (* func)(CLI_ENV *, _INT, _CHAR **);

    for(i=0; i < CLI_MAX_ARG_NUM; ++i)
    {
        if(CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[i]!=NULL)
        {
            // 保存参数,从1开始,0保存的是命令字
            
            pscArgument[i] = &CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[i][0];
        }
        else
        {
            // 参数结束
            break;
        }
    }

      // 参数帮助
      if(!strcmp(pscArgument[1],"?"))
      {
         CLI_EXT_WriteStrLine(pCliEnv, "参数说明");
         CLI_EXT_WriteStrLine(pCliEnv, "---------");
        
         sprintf(cache, "%s",CLIENV(pCliEnv)->cliShellCmd.pCmdLst[CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx].pscParaHelpStr);
         CLI_EXT_WriteStrLine(pCliEnv, cache);
         return 1;
      }
    
    func=(_VOID(*)(CLI_ENV *, _INT,_CHAR **))(CLIENV(pCliEnv)->cliShellCmd.pCmdLst[CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx].pCmdHandler);
    
    func(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt + 1, pscArgument);
    
    return 1;
}

/*---------------------------------------------------------------------

 ParseCommand - 从用户输入的一段完整字符串中分析命令
    
                            以及参数

 参数说明：无
    
 返回值：1 找到要执行的命令, 0未找到

要点说明:

 该函数获取用户输入的每一个字符,发现有换行符将其
放入命令字结构

------------------------------------------------------------------------*/

_INT ParseCommand(CLI_ENV *pCliEnv)
{
    _INT  i, j;

    _CHAR *pscTmpName;

    memset(&CLIENV(pCliEnv)->cliShellCmd.cmdArg, 0, sizeof(CMD_ARG));
    
    /* 保存字符串*/
      for (i = 0, j = 0;  i < CLI_MAX_INPUT_LEN; ++i)
      {
            
            //用户输入了空格或TAB

            if (MEDIT_BufPtr(pCliEnv)[i] == ' ' || MEDIT_BufPtr(pCliEnv)[i] == '\t')
            {
                   // 第一个字符串不是参数
                CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt][j]='\0';

                  // 下个字符是有效字符或逗号

                if (MEDIT_BufPtr(pCliEnv)[i+1] != '\0'
                    && MEDIT_BufPtr(pCliEnv)[i+1] != ' '
                    && MEDIT_BufPtr(pCliEnv)[i+1] != '\t')
                 {
                        // 下一个参数
                        CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt++;

                    j=0;
                 }  

                if(CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt == CLI_MAX_ARG_NUM)
                {
                    
                    return ERROR_CLI_AMBIGUOUS_PARAM;
                }
            }
            else
            {
                 // 按字符保存
                CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[CLIENV(pCliEnv)->cliShellCmd.cmdArg.siArgcnt][j] = MEDIT_BufPtr(pCliEnv)[i];
                 
            // 结尾
                if(MEDIT_BufPtr(pCliEnv)[i] == '\0')
                    break;
                
            ++j;
            
            if(j == CLI_MAX_ARG_LEN)
            {
                return ERROR_CLI_INVALID_PARAM;
            }
        }
     }
    
    /*分析命令和参数*/
	/*lijinan 090108 for telnet all cmd*/
	if(uiAllCmdSupport==0)
	{
    
        for( i = 0; ; ++i)
        {
        
        if(CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pCmdHandler == NULL)
        {
            CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx = 0;
            //printf("Command error, input again!!\n");
            //return SUCC;
            return ERROR_CLI_BAD_COMMAND;
        }

        pscTmpName = CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdName;
   #if 0
        //不完整命令比较查找符合
        if(strlen(pscTmpName) > 3 && strlen(CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[0])>=2)
        {
            if(strstr(pscTmpName, CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[0]) == pscTmpName)
            {
                //找到该命令
                CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx = i;
                break;
            }
        }
        // 完整命令比较查找符合
        else if(strlen(pscTmpName) <= 3)
        {
            if(!strcmp(pscTmpName,CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[0]))
            {
                CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx = i;
                break;
            }
        }   
 #endif

	 		if(!strcmp(pscTmpName,CLIENV(pCliEnv)->cliShellCmd.cmdArg.pscArgval[0]))
	            {
	                CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx = i;
	                break;
	            }
			
	 
	    }
	}
	else
	{
		CLIENV(pCliEnv)->cliShellCmd.cmdArg.siIdx = 20;
	}
    
        return SUCC;
}


/*---------------------------------------------------------------------

 GetCommand - 从输入设备获得用户输入

 参数说明：
    
 返回值：用户输入了正确的字符串返回1, 其他情况返回0

要点说明:

 该函数获取用户输入的每一个字符,发现有换行符将其
放入命令字结构

------------------------------------------------------------------------*/

_INT GetCommand(_VOID)
{
    _INT i = 0 ;

       for(i = 0; i < CLI_MAX_INPUT_LEN - 1; ++i)
    {
        //cliShellCmd.pscUsrInput[i] = GETCHAR();//_getch()

            //if(cliShellCmd.pscUsrInput[i] == '\n')
            //{
            //  cliShellCmd.pscUsrInput[i] = '\0';
            //  break;
            //}

        
    }
        
     if(i == 0)
    {
          return 0;
     }

    if (i>=CLI_MAX_INPUT_LEN-1)
    {
        //CLI_EXT_WriteStr(pCliEnv, "命令字长度越界\n");

        return 0;
    }

    return 1;
}    

/*---------------------------------------------------------------------

 initCmdTree - 初始化用户命令结构

 参数说明:

 返回值: 

 该函数将静态的用户命令结构复制到系统动态内存中

------------------------------------------------------------------------*/

_VOID initCmdTree()
{
     _INT    i = 0, j = 0;

  
    /* 申请动态内存 */

    initCliMem();

    /* 初始化动态内存区*/

    initCliApCmd();
    
    return;
    
}

_VOID initCliMem()
{
    // 每条占内存268 * 50 = 13400 BYTES, 共134000BYTES
    
   
}

_VOID initCliApCmd()
{

}

_VOID initApCmd(CMD_DATAS* pDstCmd, CMD_DATAS* pSysCmd, CMD_DATAS* pApCmd)
{
     _INT    i = 0, j = 0;

     while(strlen(pSysCmd[i].pscCmdName))
    {
        if(i >= CLI_MAX_CMD_NUM - 1)
            break;
        
        memcpy(&pDstCmd[i], &pSysCmd[i], sizeof(struct cmdDatas));
        i++;
    }

    j = 0;
    
    while(strlen(pApCmd[j].pscCmdName))
    {
        if(i >= CLI_MAX_CMD_NUM - 1)
             break;

        memcpy(&pDstCmd[i], &pApCmd[j], sizeof(struct cmdDatas));

        i++;
        j++;

    }

    pDstCmd[i].siFlag = 0;
    pDstCmd[i].pscCmdName[0] = 0;
    pDstCmd[i].pCmdHandler = NULL;
    pDstCmd[i].pscCmdHelpStr    = NULL;
    pDstCmd[i].pscParaHelpStr   = NULL;

    return;

}

/*---------------------------------------------------------------------

 InitPrompt - 根据模式修改提示符

 参数说明：

 返回值：
 
该函数在每次模式跳转的时候调用
------------------------------------------------------------------------*/

_VOID InitPrompt(CLI_ENV* pCliEnv)
{  

    switch(CLIENV(pCliEnv)->cliShellCmd.cliMode)
    {
        case sysMd:
            strcpy(CLIENV(pCliEnv)->cliShellCmd.pscCursor, "\r\nL3->");
            break;
		 case sysL2:
		 	  strcpy(CLIENV(pCliEnv)->cliShellCmd.pscCursor, "\r\nL2->");
		 	  break;
        default:
            // 不作改动
            break;  
    }
    
    return;
}



/* 
*   命令树处理函数
* ---------------------------------------------------------------------------------
*/

/* 模式切换*/

_VOID modeSwitch(CLI_ENV* pCliEnv, _INT siCurMode,_INT siDstMode, _INT siPara0,_INT siPara1)
{
    CLIENV(pCliEnv)->cliShellCmd.cliMode = siDstMode;
    CLIENV(pCliEnv)->cliShellCmd.ulPara0 = siPara0;
    CLIENV(pCliEnv)->cliShellCmd.ulPara1 = siPara1;

    InitPrompt(pCliEnv);
    
    switch(siDstMode)
    {
          //系统模式
        case sysMd:
            CLIENV(pCliEnv)->cliShellCmd.pCmdLst= sysCmds;
                    break;
		 case sysL2:
			  CLIENV(pCliEnv)->cliShellCmd.pCmdLst= L2SysCmds;
			break;

        default:
            break;
    }       
    return;
}


/* 模式切换函数*/

_VOID cmdExit(CLI_ENV* pCliEnv, _INT siArgc , _CHAR **ppArgv)
{
    modeSwitch(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.cliMode, ExitCliShellMd, 0, 0);
    return;
}

_VOID cmdL2Exit(CLI_ENV* pCliEnv, _INT siArgc , _CHAR **ppArgv)
{
	L2Shell = 0;
	stopL2Shell();
	//RtnStdIO();
    modeSwitch(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.cliMode, sysMd, 0, 0);
	//changeShellStatus(pCliEnv, sysMd);

    return;
}

_VOID cmdQuit(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    switch(CLIENV(pCliEnv)->cliShellCmd.cliMode)
    {
        case sysMd:
            modeSwitch(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.cliMode, ExitCliShellMd, 0, 0);        
            break;
                               
        default:
            break;
    }       
    return;
}

_VOID cmdHelp(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    _INT i;
    _CHAR cache[CLI_MAX_CMD_HELP_LEN+10];

    if(siArgc>2)
        PARA_ERROR(pCliEnv)
        
    if(siArgc== 1)
    {
        CLI_EXT_WriteStrLine(pCliEnv, "命令列表:");
        CLI_EXT_WriteStrLine(pCliEnv, "---------");
        for(i= 0;strlen(CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdName)>0;i++)
        {
            sprintf(cache, "%-20s%-2s%-6s\r\n",
                CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdName,
                "- ",
                CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdHelpStr);

            CLI_EXT_WriteStr(pCliEnv, cache);
      
        }
                     CLI_EXT_WriteStrLine(pCliEnv, " ");
        
    }
    else if(siArgc == 2)
    {
            CLI_EXT_WriteStrLine(pCliEnv, "参数说明:");
              CLI_EXT_WriteStrLine(pCliEnv, "---------");

        for(i = 0;strlen(CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdName)>0;i++)
        {
            if(!strcmp(CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscCmdName,ppArgv[1]))
            {
                sprintf(cache, "%s",CLIENV(pCliEnv)->cliShellCmd.pCmdLst[i].pscParaHelpStr);
                CLI_EXT_WriteStrLine(pCliEnv, cache);
                          
            }
        }
    }
    
    switch(CLIENV(pCliEnv)->cliShellCmd.cliMode)
        {
        case sysMd:
            break;
        default:
            break;
        }       
    return;
}
_VOID cmdShow(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    CLI_EXT_WriteStrLine(pCliEnv, "cmdShow Called.");
}
_VOID cmdSet(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    CLI_EXT_WriteStrLine(pCliEnv, "cmdSet Called.");
    
}

_VOID cmdIPSet(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    if(siArgc < 2)
    {
		CLI_EXT_WriteStrLine(pCliEnv, "no argv input.");
		return;
    }

    if(strcmp(ppArgv[1], "mpu") == 0 )
    {
	   cmdMPUIPSet(pCliEnv, siArgc, ppArgv);
    }
    else if(strcmp(ppArgv[1], "rnms") == 0)
    {
	    cmdRnmsIPSet(pCliEnv, siArgc, ppArgv);
    }
    else
    {
	    CLI_EXT_WriteStrLine(pCliEnv, "Err argv input.");
    }
    return;     
   
}

_VOID cmdMPUIPSet(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
#if 0
    _UINT nMPUIP,nRnmsIP;
    _USHORT usMPUPort,usRnmsPort;
	
    switch(siArgc)
    {
	    case 3:  /*只输入了IP地址*/
	    case 4: /*输入了IP地址和端口号*/
		    GetSYSIPInfo(&nMPUIP, &usMPUPort, &nRnmsIP, &usRnmsPort);
		    if(SUCC== ip_to_int(ppArgv[2],&nMPUIP))
		    {
			    if(siArgc == 4)
			    {
				usMPUPort = (_USHORT)atoi(ppArgv[3]);
				if(usMPUPort < 1024|| usMPUPort > 20000) 
				{
					CLI_EXT_WriteStrLine(pCliEnv, "  Set  Port FAIL");
					return;
				}
			    }
			    
	    		    SetSYSIPInfoFromTelnet(nMPUIP, usMPUPort, nRnmsIP, usRnmsPort);
			    CLI_EXT_WriteStrLine(pCliEnv, "  Set SUCC!");
		    }
		    else
		    {
		        CLI_EXT_WriteStrLine(pCliEnv, "  Set Ip Or Port FAIL");
		    }
		    break;
			
	    case 1:
	    case 2:			
		    CLI_EXT_WriteStrLine(pCliEnv, " Nothing changed.");
		    break;
			
	    default:
		    CLI_EXT_WriteStrLine(pCliEnv, "  ERR:MPU IpSet ,too many Argv.");	
		    
    }
#endif
    return;
}

_VOID cmdRnmsIPSet(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
#if 0
    _UINT nMPUIP,nRnmsIP;
    _USHORT usMPUPort,usRnmsPort;
	
    switch(siArgc)
    {
	    case 3:  /*只输入了IP地址*/
	    case 4: /*输入了IP地址和端口号*/
		    GetSYSIPInfo(&nMPUIP, &usMPUPort, &nRnmsIP, &usRnmsPort);\
		    /*  ppArgv[0]:"ipset", ppArgv[1]:"rnms", ppArgv[2]:ip地址  */
		    if(SUCC== ip_to_int(ppArgv[2],&nRnmsIP)) 
		    {
			if(siArgc == 4)
			    {
				usRnmsPort = (_USHORT)atoi(ppArgv[3]);
				if(usRnmsPort < 1024 ||usRnmsPort > 20000) 
				{
					CLI_EXT_WriteStrLine(pCliEnv, "  Set  Port FAIL");
					return;
				}
			    }
    		    	SetSYSIPInfoFromTelnet(nMPUIP, usMPUPort, nRnmsIP, usRnmsPort);
			CLI_EXT_WriteStrLine(pCliEnv, "  Set SUCC!");
		    }
		    else
		    {
		        CLI_EXT_WriteStrLine(pCliEnv, "  Set Ip FAIL");
		    }
		    break;
			
	    		
	    case 1:
	    case 2:
		    CLI_EXT_WriteStrLine(pCliEnv, "Nothing changed.");
		    break;
			
	    default:
		    CLI_EXT_Printf(pCliEnv,"  ERR:Rnms IPSet ,too many Argv:%d.",siArgc);
    }

    return;
#endif
    
}

_VOID cmdIPShow(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
#if 0
    _UCHAR  ucRnmsIP[4], ucMPUIP[4]; 
    _USHORT usMPUPort,usRnmsPort;
    _UCHAR pBuf[100];

    GetSYSIPInfo((_UINT *)&ucMPUIP, &usMPUPort, (_UINT *)&ucRnmsIP, &usRnmsPort);

	
    sprintf(pBuf,"\n\rMPU : %d.%d.%d.%d  Port:%d\n\rRNMS: %d.%d.%d.%d  Port:%d\0", 
		ucMPUIP[0], ucMPUIP[1], ucMPUIP[2], ucMPUIP[3],usMPUPort,
		ucRnmsIP[0], ucRnmsIP[1], ucRnmsIP[2], ucRnmsIP[3],usRnmsPort); 
    
    CLI_EXT_WriteStrLine(pCliEnv, pBuf);
#endif
    
}

_VOID cmdDebugLevelShow(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{

    
}


_VOID cmdDebugLevelSet(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{

    return;
}

_VOID cmdDebugOpen(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    ucOut2Telnet = 1;	
    return;
}

_VOID cmdDebugClose(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    ucOut2Telnet = 0;	
    return;
}

/*lijinan 090108 for telnet all cmd*/
_VOID cmdOpen(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    uiAllCmdSupport = 1;	
    return;
}

_VOID cmdClose(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    uiAllCmdSupport = 0;	
    return;
}



_VOID changeIO()
{
	if (ioGlobalStdGet(STD_IN)==CliPipeFd)
	{
		printf("\nchangeIO useless!\n");
		return;        
	}
		
    OrigGlobalIn   = ioGlobalStdGet(STD_IN);
    OrigGlobalOut = ioGlobalStdGet(STD_OUT); /*保存原来的输出句柄*/
    OrigGlobalErr  = ioGlobalStdGet(STD_ERR); /*保存原来的ERR句柄 */

    ioGlobalStdSet(STD_IN,   CliPipeFd); // CliSock
    ioGlobalStdSet(STD_OUT, PipeFd);
    ioGlobalStdSet(STD_ERR, PipeFd);
    //HoldIoFlag=1;
    IoChangeFlag = 1;

	printf("\nchangeIO Succ, in:%d<--%d, out:%d<--%d, err:%d<--%d!\n", CliPipeFd, OrigGlobalIn, PipeFd, OrigGlobalOut, PipeFd, OrigGlobalErr);
        
}

_VOID RtnStdIO()
{
	if(ioGlobalStdGet(STD_IN)!=CliPipeFd)
	{
		printf("\nRtnStdIO useless!\n");
		return;
	}
	
    if ((OrigGlobalIn!=ERROR) && (OrigGlobalIn!=ERROR) && (OrigGlobalIn!=ERROR)) 
    {
        IoChangeFlag = 0;
        ioGlobalStdSet(STD_IN,    OrigGlobalIn);
        ioGlobalStdSet(STD_OUT, OrigGlobalOut);
        ioGlobalStdSet(STD_ERR, OrigGlobalErr);

		printf("\nRtnStdIO Succ to, in:%d, out:%d, err:%d!\n", OrigGlobalIn , OrigGlobalOut, OrigGlobalErr);

		//printf("\nRtnStdIO Succ!\n");

        OrigGlobalIn = ERROR;
        OrigGlobalOut = ERROR;
        OrigGlobalErr = ERROR;
        
    }
    else
    {
        printf("\nNot be init, Can't to Back\n");
    }
}



_VOID cmdDiagShow(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    _CHAR cmdBuf[256];
    _CHAR outbuf[256];
    
    _USHORT i, usPos;
    _INT OrigInPutFd = ERROR;
    _INT OrigOutPutFd = ERROR;
    _INT OrigErrPutFd = ERROR; /*2004-9-29*/
    _INT TempOutFd = ERROR;
     _INT nTaskId = ERROR;

    
    SYS_MEMSET(cmdBuf, 0, 256);    
    for (i=0, usPos = 0; i < siArgc; i++)
    {
        SYS_SPRINTF((cmdBuf+usPos),"%s ",ppArgv[i]);
        usPos+=strlen(ppArgv[i])+1;
    }
		
    OrigInPutFd   = ioTaskStdGet(0,STD_IN);
    OrigOutPutFd = ioTaskStdGet(0,STD_OUT); /*保存原来的输出句柄*/
    OrigErrPutFd = ioTaskStdGet(0,STD_ERR); /*保存原来的ERR句柄 */

    
    ioTaskStdSet(0, STD_IN,   CliPipeFd);//CliPipeFd
    ioTaskStdSet(0, STD_OUT, PipeFd);
    ioTaskStdSet(0, STD_ERR, PipeFd);
    taskDelay(2);
    nTaskId = taskNameToId("tL3Diag");
    if (nTaskId != ERROR)
    {
        TempOutFd = ioTaskStdGet(nTaskId, STD_OUT);
        ioTaskStdSet(nTaskId, STD_OUT, PipeFd);
    }

        
    RunShellFlag = 1;
	#ifndef WBBU_CODE
    execute(cmdBuf);
	#endif
    RunShellFlag = 0;


    taskDelay(100);
    if (nTaskId != ERROR)
    {
        ioTaskStdSet(taskNameToId("tL3Diag"), STD_OUT, TempOutFd);
        CLI_EXT_WriteStrLine(pCliEnv, "\n");
    }

    
    ioTaskStdSet(0, STD_IN,    OrigInPutFd);
    ioTaskStdSet(0, STD_OUT, OrigOutPutFd);
    ioTaskStdSet(0, STD_ERR, OrigErrPutFd);

       
    
        
   //CLI_EXT_WriteStrLine(pCliEnv, cmdBuf);
    
}

_VOID cmdShellParse(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    _CHAR cmdBuf[256];
    _CHAR outbuf[256];
    
    _USHORT i, usPos;
    _INT OrigInPutFd = ERROR;
    _INT OrigOutPutFd = ERROR;
    _INT OrigErrPutFd = ERROR; /*2004-9-29*/
    _INT TempOutFd = ERROR;


    
    SYS_MEMSET(cmdBuf, 0, 256);    
    for (i=0, usPos = 0; i < siArgc; i++)
    {
        SYS_SPRINTF((cmdBuf+usPos),"%s ",ppArgv[i]);
        usPos+=strlen(ppArgv[i])+1;
    }
		
    OrigInPutFd   = ioTaskStdGet(0,STD_IN);
    OrigOutPutFd = ioTaskStdGet(0,STD_OUT); /*保存原来的输出句柄*/
    OrigErrPutFd = ioTaskStdGet(0,STD_ERR); /*保存原来的ERR句柄 */

    
    ioTaskStdSet(0, STD_IN,   CliPipeFd);//CliPipeFd
    ioTaskStdSet(0, STD_OUT, PipeFd);
    ioTaskStdSet(0, STD_ERR, PipeFd);
    taskDelay(2);

        
    RunShellFlag = 1;
   #ifndef WBBU_CODE
    execute(cmdBuf);
	#endif
    RunShellFlag = 0;
    
    ioTaskStdSet(0, STD_IN,    OrigInPutFd);
    ioTaskStdSet(0, STD_OUT, OrigOutPutFd);
    ioTaskStdSet(0, STD_ERR, OrigErrPutFd);

       
    
        
   //CLI_EXT_WriteStrLine(pCliEnv, cmdBuf);
    
}

_VOID changeShellStatus(CLI_ENV* pCliEnv, int l2ShellStatus)
{
	switch(l2ShellStatus)
	{
		case sysMd:
			CLIENV(pCliEnv)->cliShellCmd.cliMode = sysMd;
			CLIENV(pCliEnv)->cliShellCmd.pCmdLst = sysCmds;
			InitPrompt(pCliEnv);
			break;
		case sysL2:
			CLIENV(pCliEnv)->cliShellCmd.cliMode = sysL2;
			CLIENV(pCliEnv)->cliShellCmd.pCmdLst = L2SysCmds;
			InitPrompt(pCliEnv);
			break;
		default:
			break;
	}
	return;
}

_VOID cmdEnterL2Shell(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{

		L2Shell = 1;
		if(startL2Shell())
		{	
				CLI_EXT_WriteStrLine(pCliEnv, "can not enter L2 shell.");
				return;
		}
		modeSwitch(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.cliMode, sysL2, 0, 0);
		//changeShellStatus(pCliEnv, sysL2);
		//startL2Shell();
		//l2sh();
		//HoldL2Flag=1;
       //RunShellFlag=1;
       //changeIO();
		//modeSwitch(pCliEnv, CLIENV(pCliEnv)->cliShellCmd.cliMode, sysMd, 0, 0);
		//changeShellStatus(pCliEnv, sysMd);

}


_VOID cmdL2ShellParse(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    _CHAR cmdBuf[256];
    _CHAR outbuf[256];
    
    _USHORT i, usPos;
     _INT nTaskId = ERROR;
	 _INT nBytes = 0;

    
    SYS_MEMSET(cmdBuf, 0, 256);    
    for (i=0, usPos = 0; i < siArgc; i++)
    {
        SYS_SPRINTF((cmdBuf+usPos),"%s ",ppArgv[i]);
        usPos+=strlen(ppArgv[i])+1;
    }

	nBytes = strlen(&cmdBuf);

	 SendCommandToL2(&cmdBuf, nBytes);
    
}


_VOID cmdCls(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
    CLI_CMD_Clear(pCliEnv);    
}

// 输入输出函数--------------------------------------------------------------------

RL_STATIC _SHORT EXT_GetCursorX(CMD_EDITINFO *edit)
{
    return edit->termX;
}

/*-----------------------------------------------------------------------*/

RL_STATIC _SHORT EXT_GetCursorY(CMD_EDITINFO *edit)
{
    return edit->termY;
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_SetCursorX(CMD_EDITINFO *edit, _SHORT pos)
{
    edit->termX = pos;
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_SetCursorY(CMD_EDITINFO *edit, _SHORT pos)
{
    edit->termY = pos;
}

/*-----------------------------------------------------------------------*/
RL_STATIC _VOID CLI_EXT_CurrCoord(CLI_ENV *pCliEnv, _SHORT *xPos, _SHORT *yPos) 
{
    _CHAR    *pBuf        = MEDIT_BufPtr(pCliEnv);
    _SHORT  cursorPos   = MEDIT_GetCursor(pCliEnv);
    _SHORT  width       = (_SHORT) MSCRN_GetWidth(pCliEnv);
    _SHORT  X           = (_SHORT) MEDIT_PromptLen(pCliEnv);
    _SHORT  Y           = 0;
    
    if (0 >= width)
        width = kCLI_DEFAULT_WIDTH;

    while (0 < cursorPos--)
    {
        switch (*(pBuf++))
        {
        case kCR:
            X = 0;
            break;
        case kLF:
            Y++;
            break;
        default:         
            if (++X >= width)
            {
                Y++;
                X = 0;
            }
            break;
        }
    }
    *xPos = X;
    *yPos = Y;
}

/*-----------------------------------------------------------------------*/

/*
 *  Movement functions
 */

  _VOID CLI_EXT_LineStart(CLI_ENV *pCliEnv)
{
    CLI_EXT_SetCursor(pCliEnv, 0);
}

/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_LineEnd(CLI_ENV *pCliEnv)
{
    CLI_EXT_SetCursor(pCliEnv, MEDIT_GetLength(pCliEnv));
}

/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_EraseLine(CLI_ENV *pCliEnv)
{
    _CHAR    *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT  lineLength = MEDIT_GetLength(pCliEnv);
    _SHORT  index;

#ifdef kKEY_CONTINUE
    for (index = 0; index < lineLength; index++)
    {
        if ((kCR == pBuf[index]) || (kLF == pBuf[index]))
            continue;

        pBuf[index] = ' ';
    }
#else
    for (index = 0; index < lineLength; index++)
        pBuf[index] = ' ';
#endif

    pBuf[index] = '\0';

    CLI_EXT_LineStart(pCliEnv);
    CLI_EXT_Write(pCliEnv, pBuf, index);

    index     = -index;

    CLI_EXT_MoveCursor(pCliEnv, index);
    CLI_EXT_InitCommandLine(pCliEnv);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID CLI_EXT_MoveTTYCursor(CLI_ENV *pCliEnv, _SHORT xPos, _SHORT yPos)
{
    _CHAR   buffer[32];

#ifdef __DISABLE_VT_ESCAPES__
    return;
#endif

    if (0 != xPos)
    {
        if (xPos < 0)
            sprintf(buffer, kCLI_VTTERM_LT, -xPos);
        else if (xPos > 0)
            sprintf(buffer, kCLI_VTTERM_RT, xPos);

        CLI_EXT_PutStr(pCliEnv, buffer);
    }

    if (0 != yPos)
    {
        if (yPos < 0)
            sprintf(buffer, kCLI_VTTERM_UP, -yPos);
        else if (yPos > 0)
            sprintf(buffer, kCLI_VTTERM_DN, yPos);

        CLI_EXT_PutStr(pCliEnv, buffer);
    }
}

/*-----------------------------------------------------------------------*/

#ifdef WINDOWS

RL_STATIC _VOID CLI_EXT_MoveDOSCursor(CLI_ENV *pCliEnv, _SHORT xPos, _SHORT yPos)
{
    CONSOLE_SCREEN_BUFFER_INFO screenBuffer;
    HANDLE                     console;
    
    
    if (xPos >= MSCRN_GetWidth(pCliEnv))
        xPos--;

    console = GetStdHandle(STD_OUTPUT_HANDLE);
        
    if (INVALID_HANDLE_VALUE == console)
        return;

    if (0 == GetConsoleScreenBufferInfo(console, &screenBuffer))
            return;

    screenBuffer.dwCursorPosition.X += xPos;
    screenBuffer.dwCursorPosition.Y += yPos;

    SetConsoleCursorPosition(console, screenBuffer.dwCursorPosition);
    
}

#endif /* WINDOWS */

/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_SetCursor(CLI_ENV *pCliEnv, _SHORT position)
{
    _SHORT  lineLength = MEDIT_GetLength(pCliEnv);
    _SHORT  new_X      = 0;
    _SHORT  new_Y      = 0;
    _SHORT  old_X      = 0;
    _SHORT  old_Y      = 0;
    CMD_EDITINFO *pEdit   = MEDIT_EditInfoPtr(pCliEnv);

    /* no point moving around text not on screen */
    if (CLI_NotEnabled(pCliEnv, kCLI_FLAG_ECHO))
        return;

    CLI_EXT_CurrCoord(pCliEnv, &old_X, &old_Y); 

    if (position < 0)
        position = 0;

    if (position > lineLength)
        position = lineLength;

    MEDIT_SetCursor(pCliEnv, position);

    CLI_EXT_CurrCoord(pCliEnv, &new_X, &new_Y);

    /* save position */
    EXT_SetCursorX(pEdit, new_X);
    EXT_SetCursorY(pEdit, new_Y);
    
    new_X -= old_X;
    new_Y -= old_Y;

    /* don't bother moving if it's nowhere */
    if ((0 == new_X) && (0 == new_Y))
        return;

    if (kCONSOLE_CONNECTION == MCONN_GetConnType(pCliEnv))
    {
#ifdef WINDOWS
        CLI_EXT_MoveDOSCursor(pCliEnv, new_X, new_Y);
#else        
        CLI_EXT_MoveTTYCursor(pCliEnv, new_X, new_Y);
#endif
    }
    else
    {
        CLI_EXT_MoveTTYCursor(pCliEnv, new_X, new_Y);
    }
}

/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_MoveCursor(CLI_ENV *pCliEnv, _SHORT offset)
{
    _SHORT  cursorPos = MEDIT_GetCursor(pCliEnv);
    _CHAR    *pBuf      = MEDIT_BufPtr(pCliEnv);

    if (0 == offset)
        return;

    /* no point moving around text not on screen */
    if (CLI_NotEnabled(pCliEnv, kCLI_FLAG_ECHO))
        return;

    cursorPos += offset;
    pBuf      += cursorPos;

    /* if moving to a EOL char we have to scoot
       past to the other side */
    while ((kCR == *pBuf) || (kLF == *pBuf)) 
    {
        if (0 > offset)
        {
            cursorPos--;
            pBuf--;
        }
        else
        {
            cursorPos++;
            pBuf++;
        }
    }

    CLI_EXT_SetCursor(pCliEnv, cursorPos);
}

/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_ResetOutput(CLI_ENV *pCliEnv) 
{
    LINE_OUT      *output = MMISC_OutputPtr(pCliEnv);

    output->flags      = 0;
    output->lineCount  = 0;
    output->length     = 0;

    if (NULL != output->pOutput)
        output->pOutput[0]  = '\0';
}

/*-----------------------------------------------------------------------*/

  _INT CLI_EXT_InitOutput(CLI_ENV *pEnv, _INT size)
{
    LINE_OUT  *output = MMISC_OutputPtr(pEnv);

    if (NULL == (output->pOutput = RC_MALLOC(size)))
        return ERROR_MEMMGR_NO_MEMORY;

    memset(output->pOutput, 0, size);
    output->maxSize = size;

    return SUCC;
}

/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_FreeOutput(CLI_ENV *pEnv)
{
    LINE_OUT  *output = MMISC_OutputPtr(pEnv);

    RC_FREE(output->pOutput);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_More(CLI_ENV *pEnv)
{
#ifdef __BSC_TASK_CLI__
    _CHAR        pBuf      = 0;
    _CHAR        moreBuf[] = kCLI_MORE_TEXT;
    _INT       moreSize  = sizeof(moreBuf);
    WriteHandle *Writer    = MCONN_GetWriteHandle(pEnv);
    COM_CHAN  *pChannel  = MMISC_GetChannel(pEnv);
    LINE_OUT     *output    = MMISC_OutputPtr(pEnv);
    _INT       time      = OS_SPECIFIC_GET_SECS();

    Writer(pEnv, moreBuf, moreSize);
    CLI_EXT_ResetOutput(pEnv);

    switch (MCONN_GetConnType(pEnv))
    {
        case kCONSOLE_CONNECTION:
            pBuf = GETCHAR();
            break;
        case kTELNET_CONNECTION:
            while (0 == pBuf)
            {
                OS_SPECIFIC_SOCKET_READ(pChannel->sock, &pBuf, 1);
                if (0 == OS_SPECIFIC_GET_SECS() - time)
                    pBuf = 0;
            }
            break;       
    }
    /* clear more message */
    Writer(pEnv, "\r", 1);
    memset(moreBuf, ' ', moreSize);
    Writer(pEnv, moreBuf, moreSize);
    Writer(pEnv, "\r", 1);

#ifdef kCLI_PRINT_CANCEL
    if (kCLI_PRINT_CANCEL == TOUPPER(pBuf))       
        SET_PRINT_FLAG(output, kPRINT_FLAG_NOPRINT);
#endif

#endif
}


/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_EnablePrint(CLI_ENV *pCliEnv, _CHAR enable)
{
    LINE_OUT  *pOutput   = MMISC_OutputPtr(pCliEnv);

    if (enable)
        CLR_PRINT_FLAG(pOutput, kPRINT_FLAG_NOPRINT);
    else
        SET_PRINT_FLAG(pOutput, kPRINT_FLAG_NOPRINT);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _INT EXT_Update(CLI_ENV *pCliEnv, _CHAR *pBuf, _INT length, _CHAR *more)
{
    _INT        index    = 0;
    _CHAR       hardWrap = (_CHAR) CLI_IsEnabled(pCliEnv, kCLI_FLAG_HARDWRAP);
    CMD_EDITINFO  *pEdit    = MEDIT_EditInfoPtr(pCliEnv);
    LINE_OUT      *pOutput  = MMISC_OutputPtr(pCliEnv);
    _SHORT      xPos     = EXT_GetCursorX(pEdit);
    _SHORT      yPos     = EXT_GetCursorY(pEdit);
    _SHORT      maxCol   = MSCRN_GetWidth(pCliEnv);
#ifndef __DISABLE_PAGED_OUTPUT__
    _SHORT      maxRow   = MSCRN_GetHeight(pCliEnv);
#endif

    *more = FALSE;

    while (length-- > 0)
    {
        if (GET_PRINT_FLAG(pOutput, kPRINT_FLAG_NOPRINT))
            return -1;

        index++;
        pEdit->cursorPos++;

        switch (*pBuf) 
        {
        case kCR:
            xPos = 0;
            break;
        case kLF:
            yPos++;
            pOutput->lineCount++;
            break;
        default:
            xPos++;
            break;
        }

        if (maxCol == xPos)
        {
            if (hardWrap)
            {
                CLI_EXT_Put(pCliEnv, kEOL, kEOL_SIZE);
            }
            xPos = 0;
            yPos++;
            pOutput->lineCount++;
        }

        EXT_SetCursorX(pEdit, xPos);
        EXT_SetCursorY(pEdit, yPos);
    
#ifndef __DISABLE_PAGED_OUTPUT__
        if (CLI_IsEnabled(pCliEnv, kCLI_FLAG_MORE))
        {
            if (pOutput->lineCount >= maxRow - 1)
            {
                *more = TRUE;
                break;
            }
        }
#endif /* __DISABLE_PAGED_OUTPUT__ */

        pBuf++;
    }
    
    return index;
}

/*-----------------------------------------------------------------------*/

  _INT CLI_EXT_Write(CLI_ENV *pCliEnv, _CHAR *pBuf, _INT bufLen)
{
    _INT      status   = SUCC;
    _INT        outSize  = 0;
    _CHAR       more     = FALSE;
    WriteHandle  *Writer   = MCONN_GetWriteHandle(pCliEnv);

    /* this should be an assert eventually */
    if (NULL == pBuf)
        return CLI_ERROR;

    /* skip it all if echo disabled */
    if ( CLI_IsEnabled(pCliEnv, kCLI_FLAG_INPUT) &&
         CLI_NotEnabled(pCliEnv, kCLI_FLAG_ECHO))
        return SUCC;
            
    while (1)
    {
        outSize = EXT_Update(pCliEnv, pBuf, bufLen, &more);
        if (0 > outSize)
            break;

        status  = Writer(pCliEnv, pBuf, outSize);

#ifndef __DISABLE_PAGED_OUTPUT__
        if (more)
            EXT_More(pCliEnv);
#endif

        bufLen -= outSize;

        if ( (0 >= bufLen)  || 
             (status != SUCC) || 
             (0 > outSize) )
            break;
    }

    return status;
}

/*-----------------------------------------------------------------------*/

/* 
 * Similar to CLI_EXT_Write but doesn't update buffer 
 */
  _INT CLI_EXT_Put(CLI_ENV *pCliEnv, _CHAR *pBuf, _INT bufLen)
{
    WriteHandle  *Writer = MCONN_GetWriteHandle(pCliEnv);

    /* this should be an assert eventually */
    if (NULL == pBuf)
        return CLI_ERROR;

    return Writer(pCliEnv, pBuf, bufLen);
}

/*-----------------------------------------------------------------------*/

/* 
 * Similar to CLI_EXT_WriteStr but doesn't update buffer 
 */
  _INT CLI_EXT_PutStr(CLI_ENV *pCliEnv, _CHAR *pBuf)
{
    /* this should be an assert eventually */
    if (NULL == pBuf)
        return CLI_ERROR;

    return CLI_EXT_Put(pCliEnv, pBuf, strlen(pBuf));
}

/*-----------------------------------------------------------------------*/

  _INT CLI_EXT_WriteStr(CLI_ENV *pCliEnv, _CHAR *pBuf)
{
    /* this should be an assert eventually */
    if (NULL == pBuf)
        return SUCC;

    return CLI_EXT_Write(pCliEnv, pBuf, strlen(pBuf));
}


  _INT CLI_EXT_WriteStrLine(CLI_ENV *pCliEnv, _CHAR *pBuf)
{
    _INT status = SUCC;

    if (SUCC == (status = CLI_EXT_WriteStr(pCliEnv, pBuf)))
          return CLI_EXT_Write(pCliEnv, kEOL, kEOL_SIZE);

    return status;
}

/*
*   带格式的输出函数2004-10-22
*/
_INT CLI_EXT_Printf(CLI_ENV* pCliEnv, _CHAR* pFmt, ...)
{
    _INT status = SUCC;
    va_list pvar;
    _CHAR buf[1024];

    va_start(pvar, pFmt);
    vsprintf(buf, pFmt, pvar);
    va_end(pvar);

    return CLI_EXT_Write(pCliEnv, buf, strlen(buf));
}


  _VOID CLI_EXT_PrintString(CLI_ENV *pCliEnv, _CHAR *pString, _INT length, _CHAR fill)
{
    _INT  limit, size;

    if (0 == length)
    {
        CLI_EXT_WriteStr(pCliEnv, pString);
        return;
    }

    size  = (_INT) strlen(pString);
    limit = CLI_MIN(length, size);
    CLI_EXT_Write(pCliEnv, pString, limit);

    limit = length - limit;
    while (0 < limit--)
        CLI_EXT_Write(pCliEnv, &fill, 1);

}

/*-----------------------------------------------------------------------*/

/*
 *  Line editing functions
 */

  _VOID
CLI_EXT_InsertText(CLI_ENV *pCliEnv, _CHAR *pText, _INT length)
{
    _SHORT    offset;
    _SHORT    cursorPos   = MEDIT_GetCursor(pCliEnv);
    _SHORT    lineLength  = MEDIT_GetLength(pCliEnv);
    _CHAR      *pBuf        = MEDIT_BufPtr(pCliEnv);
    _INT      index;

    if (CLI_MAX_CMD_LEN <= (lineLength + length))
        return;

    /* are we just appending text? */
    if (cursorPos == lineLength)
    {
        pBuf       += lineLength;
        cursorPos  += length;

	strncpy(pBuf, pText, length);

	  if(MMISC_GetLoginStat(pCliEnv) == kTELNET_LOGIN_PROCESS)
	 {
		/*输入密码,则返回星号*/
	 	SYS_MEMSET(pText, '*', length);
	 }
	
        CLI_EXT_Write(pCliEnv, pText, length);

        MEDIT_SetCursor(pCliEnv, cursorPos);
        MEDIT_SetLength(pCliEnv, cursorPos);
        return;
    }

    /* inserting in the middle... */
    for (index = lineLength; index >= cursorPos; index--)
    {
        pBuf[index + length] = pBuf[index];
    }

    pBuf += cursorPos;
    memcpy(pBuf, pText, length);
    CLI_EXT_WriteStr(pCliEnv, pBuf);

    lineLength += length;
    offset      = cursorPos - lineLength + length;

    MEDIT_SetCursor(pCliEnv, lineLength);
    MEDIT_SetLength(pCliEnv, lineLength);
    CLI_EXT_MoveCursor(pCliEnv, offset);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_Refresh(CLI_ENV *pCliEnv, _SHORT offset, _SHORT length)
{
    _CHAR     *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT   cursorPos;

    if (0 > length)
        return;

    CLI_EXT_MoveCursor(pCliEnv, offset);
    cursorPos = MEDIT_GetCursor(pCliEnv);
    pBuf += cursorPos;

    CLI_EXT_Write(pCliEnv, pBuf, length);
    length = -length;
    CLI_EXT_MoveCursor(pCliEnv, length);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_DeleteText(CLI_ENV *pCliEnv, _SHORT start, _SHORT length)
{
    _CHAR     *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT   lineLength = MEDIT_GetLength(pCliEnv);
    _SHORT   index;
    _SHORT   remainder;
    _SHORT   updated[kCLI_DEFAULT_HEIGHT];
    _SHORT   lineWidth    = 0;
    _SHORT   lineCount    = 0;
    _CHAR    linesDeleted = FALSE;
    _CHAR     *pTemp;

    if (0 >= length)
        return;


    if (0 == strncmp(kEOL, &pBuf[start + 1], kEOL_SIZE))
    {
        if (1 < kEOL_SIZE)
            length++;

        start++;
    }

    /* delete mid string */
    if ( (start + length) < lineLength)
    {

        /* if shifting lines up we need to know what to erase */
        for (index = start; index < (start + length); index++)
        {
            if (kCR == pBuf[index])
            {
                linesDeleted = TRUE;
                break;
            }
        }

        if (linesDeleted)
        {
            pTemp = &pBuf[start];
            for (index = start; index < lineLength; index++, pTemp++)
            {
                if (kCR == *pTemp)
                {
                    updated[lineCount] = lineWidth;
                    lineCount++;
                    lineWidth = 0;
                }
                if ((kCR != *pTemp) && (kLF != *pTemp))
                    lineWidth++;
            }
            if (0 < lineCount)
                updated[lineCount] = lineWidth;

            /* erase extra lines */
            for (index = 0; index <= lineCount; index++)
            {
                CLI_EXT_PrintString(pCliEnv, "", updated[index], ' ');
                if (index < lineCount)
                    CLI_EXT_WriteStrLine(pCliEnv, "");
            }
            CLI_EXT_SetCursor(pCliEnv, start);
        }

        /* shift remaining text left */
        for (index = start + length; index < lineLength; index++)
        {
            pBuf[index - length] = pBuf[index];
            pBuf[index] = ' ';
        }

        /* erase rest of line */
        for (index = lineLength - length; index < lineLength; index++)
            pBuf[index] = ' ';
    }
    else /* end of string */
    {
        for (index = start; index < lineLength; index++)
            pBuf[index] = ' ';
    }

    /* display revised text */
    remainder         = lineLength - start;
    start            -= MEDIT_GetCursor(pCliEnv);

    EXT_Refresh(pCliEnv, start, remainder);

    lineLength       -= length;
    pBuf[lineLength]  = kCHAR_NULL;

    MEDIT_SetLength(pCliEnv, lineLength);
}

/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_DeleteChar(CLI_ENV *pCliEnv)
{
    _SHORT lineLength = MEDIT_GetLength(pCliEnv);
    _SHORT cursorPos  = MEDIT_GetCursor(pCliEnv);

    if (lineLength <= 0)
        return;

    /* trying to delete past end of line? */
    if (cursorPos >= lineLength)
        return;

    EXT_DeleteText(pCliEnv, cursorPos, 1);
}

/*-----------------------------------------------------------------------*/

  _VOID CLI_EXT_InitCommandLine(CLI_ENV *pCliEnv)
{
    _CHAR    *pBuf = MEDIT_BufPtr(pCliEnv);

    MEDIT_SetLength(pCliEnv, 0);
    MEDIT_SetCursor(pCliEnv, 0);
    memset(pBuf, kCHAR_NULL, CLI_MAX_CMD_LEN);
}

/*-----------------------------------------------------------------------*/

  _CHAR CLI_EXT_GetPriorChar(CLI_ENV *pCliEnv)
{
    _SHORT  cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT  lineLength = MEDIT_GetLength(pCliEnv);
    _CHAR    *pBuf       = MEDIT_BufPtr(pCliEnv);

    if (0 >= lineLength)
        return '\0';

    return pBuf[cursorPos - 1];
}

/*-----------------------------------------------------------------------*/

_VOID CLI_EXT_DiscardPrevChar(CLI_ENV *pCliEnv)
{
    _SHORT  lineLength = MEDIT_GetLength(pCliEnv);
    _CHAR    *pBuf       = MEDIT_BufPtr(pCliEnv);

    if (0 >=  lineLength)
        return;

    lineLength--;
    pBuf[lineLength] = '\0';
    MEDIT_SetLength(pCliEnv, lineLength);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_DeleteFromStart(CLI_ENV *pCliEnv)
{
    _SHORT cursorPos  = MEDIT_GetCursor(pCliEnv);

    CLI_EXT_LineStart(pCliEnv);
    EXT_DeleteText(pCliEnv, 0, cursorPos);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_DeleteToEnd(CLI_ENV *pCliEnv)
{
    _SHORT lineLength = MEDIT_GetLength(pCliEnv);
    _SHORT cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT length     = lineLength - cursorPos;

    EXT_DeleteText(pCliEnv, cursorPos, length);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_WordPrevious(CLI_ENV *pCliEnv)
{
    _CHAR   newWord    = FALSE;
    _CHAR   blank      = FALSE;
    _CHAR    *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT  cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT  index;
    
    index  = cursorPos;
    pBuf  += cursorPos;

    while (0 < --index)
    {
        pBuf--;

        blank = blank || WHITE_SPACE(*pBuf);

        if (newWord && WHITE_SPACE(*pBuf))
            break;
        else
            newWord = (blank && (! WHITE_SPACE(*pBuf)));

    }

    if (0 < index)
        index++;

    CLI_EXT_SetCursor(pCliEnv, index);
}

/*-----------------------------------------------------------------------*/

_VOID CLI_EXT_WordNext(CLI_ENV *pCliEnv)
{
    _CHAR    *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT  cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT  lineLength = MEDIT_GetLength(pCliEnv);
    _SHORT  index      = 0;
    _CHAR   blank      = FALSE;
    
    index  = cursorPos;
    pBuf  += cursorPos;
    while (lineLength > index++)
    {
        if (blank && (!WHITE_SPACE(*pBuf)))
            break;
        else
            blank = WHITE_SPACE(*pBuf);

        pBuf++;
    }
    if (blank)
        CLI_EXT_SetCursor(pCliEnv, --index);
}

/*-----------------------------------------------------------------------*/

_VOID CLI_EXT_WordChangeCase(CLI_ENV *pCliEnv, _CHAR upperCase)
{
    _CHAR    *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT  cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT  lineLength = MEDIT_GetLength(pCliEnv);
    _SHORT  count      = 0;

    pBuf += cursorPos;

    if (WHITE_SPACE(*pBuf))
        return;

    while ((cursorPos < lineLength) && !(WHITE_SPACE(*pBuf)))
    {
        if (upperCase)
            *pBuf = TOUPPER(*pBuf);
        else
            *pBuf = TOLOWER(*pBuf);

        count++;
        cursorPos++;
        pBuf++;
    }

    EXT_Refresh(pCliEnv, 0, count);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_WordDeleteStart(CLI_ENV *pCliEnv)
{
    _CHAR     *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT   cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT   length     = 0;
    
    if (0 >= cursorPos)
        return;

    pBuf += cursorPos - 1;

    while (0 < cursorPos)
    {
        cursorPos--;
        pBuf--;
        length++;

        if (WHITE_SPACE(*pBuf))
            break;
    }

    EXT_DeleteText(pCliEnv, cursorPos, length);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_WordDeleteEnd(CLI_ENV *pCliEnv)
{
    _CHAR     *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT   cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT   lineLength = MEDIT_GetLength(pCliEnv);
    _SHORT   length     = 0;
    _SHORT   index      = 0;

    pBuf += cursorPos;

    /* make sure in a word */
    if (WHITE_SPACE(*pBuf))
        return;

    index = cursorPos;
    while ((index < lineLength) && !WHITE_SPACE(*pBuf))
    {
        index++;
        length++;
        pBuf++;
    }

    EXT_DeleteText(pCliEnv, cursorPos, length);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_Transpose(CLI_ENV *pCliEnv)
{
    _CHAR      tempChar;
    _CHAR     *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT   cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT   lineLength = MEDIT_GetLength(pCliEnv);
    
    if ((cursorPos <= 0) || (cursorPos == lineLength))
        return;

    tempChar            = pBuf[cursorPos];
    pBuf[cursorPos]     = pBuf[cursorPos - 1];
    pBuf[cursorPos - 1] = tempChar;

    EXT_Refresh(pCliEnv, -1, 2);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID EXT_Backspace(CLI_ENV *pCliEnv)
{
    _CHAR       *pBuf       = MEDIT_BufPtr(pCliEnv);
    _SHORT     cursorPos  = MEDIT_GetCursor(pCliEnv);
    _SHORT     lineLength = MEDIT_GetLength(pCliEnv);
    CMD_EDITINFO *pEdit      = MEDIT_EditInfoPtr(pCliEnv);
    _SHORT     xPos       = EXT_GetCursorX(pEdit);

    if (0 >= cursorPos)
        return;

    /* just remove from buffer w/o screen update */
    if ( (CLI_NotEnabled(pCliEnv, kCLI_FLAG_ECHO)) &&
         (cursorPos == lineLength))
    {
        MEDIT_SetCursor(pCliEnv, --cursorPos);
        MEDIT_SetLength(pCliEnv, --lineLength);
        pBuf[lineLength] = kCHAR_NULL;
        return;
    }

    if ((kTELNET_CONNECTION == MCONN_GetConnType(pCliEnv)) &&
        (cursorPos == lineLength))
    {
        /* telnet is dumb and won't go to previous line */
        if (0 < xPos--)
        {
            EXT_SetCursorX(pEdit, xPos);
            CLI_EXT_PutStr(pCliEnv, kCLI_TELNET_BACKSPACE);
            MEDIT_SetCursor(pCliEnv, --cursorPos);
            MEDIT_SetLength(pCliEnv, --lineLength);
            pBuf[lineLength] = kCHAR_NULL;
            return;
        }

#ifdef __DISABLE_VT_ESCAPES__
        return;
#endif

    }
    CLI_EXT_MoveCursor(pCliEnv, -1);
    CLI_EXT_DeleteChar(pCliEnv);
}

// 清屏

#ifdef WINDOWS
/* clear DOS screen */
_VOID CLI_CMD_DosClear(CLI_ENV *pCliEnv)
{
    CONSOLE_SCREEN_BUFFER_INFO lpScreenBufferInfo;
    DWORD                      nLength;
    COORD                      dwWriteCoord;
    DWORD                      NumberOfCharsWritten;

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE == console)
        return;

    if (0 == GetConsoleScreenBufferInfo(console, &lpScreenBufferInfo))
        return;

    dwWriteCoord.X = 0;
    dwWriteCoord.Y = 0;
    nLength = lpScreenBufferInfo.dwSize.X * lpScreenBufferInfo.dwSize.Y;
    if (! FillConsoleOutputCharacter(
            console,                // handle to screen buffer
            ' ',                    // character
            nLength,                // number of cells
            dwWriteCoord,           // first coordinates
            &NumberOfCharsWritten   // number of cells written
    ))
        return;

    SetConsoleCursorPosition(console, dwWriteCoord);
}
#endif /* __WIN32_OS__ */

/*-----------------------------------------------------------------------*/

/* clear screen */
_INT CLI_CMD_Clear(CLI_ENV *pCliEnv)
{
    if (kTELNET_CONNECTION == MCONN_GetConnType(pCliEnv))
        CLI_EXT_WriteStr(pCliEnv, kCLI_TELNET_CLEAR);
    else
    {
        /* Console */
#ifdef WINDOWS
        CLI_CMD_DosClear(pCliEnv);
#else
        /* assume that non-Windows console is really telnet */
        CLI_EXT_WriteStr(pCliEnv, kCLI_TELNET_CLEAR);
#endif /* __WIN32_OS__ */        
    }
    return SUCC;
}

/*
* 历史记录相关
*
*/

_INT CLI_HIST_InitHistInfo(CLI_ENV *pCliEnv)
{
    HistInfo *history;
    _ULONG    histSize = kCLI_HISTORY_BUFFER_SIZE * sizeof(CmdHistBuff);

    if (NULL == pCliEnv)
        return CLI_ERROR;

    history = MHIST_History(pCliEnv);

    history->iMaxHistCmds = kCLI_HISTORY_BUFFER_SIZE;
    history->iCurHistCmd  = 0;
    history->iNumCmds     = 0;

    if (NULL == (history->pHistBuff = RC_MALLOC(histSize)))
        return ERROR_MEMMGR_NO_MEMORY;

    return SUCC;
}

_VOID CLI_HIST_AddHistLine (CLI_ENV *pCliEnv)
{
    _CHAR       *pSrc  = NULL;
    _CHAR       *pDest = NULL;
    HistInfo    *pHistory;

    if (NULL == pCliEnv)
        return;

    pHistory = MHIST_History(pCliEnv);
    pSrc     = MEDIT_BufPtr(pCliEnv);

    if (0 < pHistory->iNumCmds) {
        if (HIST_BUFFER_FULL(pHistory))
            pHistory->bufferIndex = 0;
        else
            pHistory->bufferIndex++;
    }

    pDest = HistBuffPtr(pHistory, pHistory->bufferIndex);
    strcpy(pDest, pSrc);

    pHistory->iCurHistCmd = ++(pHistory->iNumCmds) + 1;
}

_INT CLI_HIST_Hist2Buff(HistInfo *pHistory, _INT index)
{
    _INT offset;
    _INT least;

    /* empty buffer? */
    if (0 >= pHistory->iNumCmds)
        return -1;

    least = LEAST_RECENT_HIST(pHistory);

    /* out of range? */
    if ((least > index) || (index > pHistory->iNumCmds))
        return -1;

    /* buffer not wrapped yet? */
    if (pHistory->iNumCmds < pHistory->iMaxHistCmds)
        return index - 1;

    offset = pHistory->bufferIndex - (pHistory->iNumCmds - index);
    if (0 > offset)
        offset += pHistory->iMaxHistCmds;

    return offset;
}


_CHAR * GetHistoryCmd(HistInfo *pHistory)
{
    _CHAR       *pBuf;
    _INT       offset;
    CmdHistBuff *histCmdBuff;
    _CHAR       *pTemp = MHIST_TempBuf(pHistory);
    
    if (pHistory->iCurHistCmd > pHistory->iNumCmds)
        return pTemp;

    offset = CLI_HIST_Hist2Buff(pHistory, pHistory->iCurHistCmd);
    if (0 > offset)
        return NULL;

    histCmdBuff = &(pHistory->pHistBuff[offset]);
    pBuf = histCmdBuff->histCmd;
    return pBuf;
}

void ChangeHistory(HistInfo *pHistory, _INT  offset)
{
    _INT newHistory;
    
    newHistory = pHistory->iCurHistCmd + offset;

    if ((1 > newHistory) || (newHistory > (pHistory->iNumCmds + 1)))
        return;
    
    if (newHistory < LEAST_RECENT_HIST(pHistory))
        return;

    pHistory->iCurHistCmd = newHistory;
}


_VOID CLI_HIST_Scroll(CLI_ENV *pCliEnv, _INT offset)
{
    HistInfo  *pHistory = MHIST_History(pCliEnv);
    _CHAR     *pBuf     = MEDIT_BufPtr(pCliEnv);
    _CHAR     *pTemp    = MHIST_TempBuf(pHistory);
    _CHAR     *pHistoryCmd;
    
    if (pHistory->iCurHistCmd > pHistory->iNumCmds) 
        strcpy(pTemp, pBuf);

    ChangeHistory(pHistory, offset);
    pHistoryCmd = GetHistoryCmd(pHistory);
    if (NULL == pHistoryCmd)
        return;

    CLI_EXT_EraseLine(pCliEnv);
    //RCC_DB_SetCommand(pCliEnv, pHistoryCmd);
    
    CLI_EXT_WriteStr(pCliEnv, pHistoryCmd);

    strcpy(pBuf, pHistoryCmd);
    MEDIT_SetCursor(pCliEnv, strlen(pHistoryCmd));
    MEDIT_SetLength(pCliEnv, strlen(pHistoryCmd));
    CLI_EXT_MoveCursor(pCliEnv, (_SHORT) strlen(pHistoryCmd));

}



/*-----------------------------------------------------------------------*/

/* 
 * 读取命令字
*/
  _INT CLI_EXT_ReadCmd(CLI_ENV *pCliEnv)
{

    return 0;
}

  _VOID CLI_EXT_LocalEcho(CLI_ENV *pCliEnv, _CHAR enable)
{
    if (enable)
        CLI_DisableFeature(pCliEnv, kCLI_FLAG_ECHO);
    else
        CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
}

/* --------------------------------------------------------------- */
_VOID *RC_CALLOC(_UCHAR num, _ULONG size)
{
    _ULONG  memSize;
    _VOID    *pMem;

    memSize = num * size;
    pMem    = malloc( memSize );

    if ( NULL == pMem )
        return pMem;

    memset( pMem, 0, memSize );

    return pMem;
}

_VOID *RC_MALLOC(_ULONG memSize)
{
    return malloc( memSize );
}

_VOID RC_FREE(_VOID *pBuffer)
{   
    free( pBuffer );
}


  _VOID LogWrite(FILE *fd, char *text)
{
    if (NULL == text)
        return;

    if (NULL == fd)
        return;

    fprintf(fd, "%s", text);
    fflush(fd);
}

CLI_LIST *List_Construct(void)
{
    CLI_LIST *pTemp = (CLI_LIST *) RC_MALLOC (sizeof(CLI_LIST));

    if (NULL == pTemp)
        return NULL;

    pTemp->pObject  = NULL;
    pTemp->pNext    = NULL;

    return pTemp;
}

/*
*
*
*
/
/*-----------------------------------------------------------------------*/

/* 
 * Read one char and process
*/
_INT CLI_EXT_ReadCh(CLI_ENV *pCliEnv, _UCHAR* pOrgCh, _USHORT orgLen)
{
    KEY_STATE    keyState;
    _INT        idx = 0;
    _INT       bytesRead   = 0;
    _INT     status      = SUCC;
    _CHAR      loop        = TRUE;
    _CHAR       charIn;

    _CHAR       *pBuffer     = MEDIT_BufPtr(pCliEnv);
    ReadHandle  *Reader;
    

    CLI_EnableFeature(pCliEnv, kCLI_FLAG_INPUT);


        //CLI_EXT_ResetOutput(pCliEnv);

        if (CLI_IsEnabled(pCliEnv, kCLI_FLAG_KILL))
        {
            status = STATUS_CLI_KILL;
            loop   = FALSE;
            MMISC_SetInput(pCliEnv, 0);

            return ERROR_CLI_WRONG_MODE;
        }

    //TELNET 字符处理

    while( idx < orgLen)
    {
            
            Reader      = MCONN_GetReadHandle(pCliEnv);
            Reader(pCliEnv, pOrgCh[idx], &charIn, &bytesRead);
            
            if(bytesRead == 0)
            {
                idx++;
                continue;
            }

        keyState    = MEDIT_GetKeyStat(pCliEnv);
        /* pre-process escape codes */
        switch (keyState)
        {
        case KEY_STATE_ESC:
            switch ( TOLOWER(charIn) ) 
            {
            case kCLI_ESC_CURSOR:
                MEDIT_SetKeyStat(pCliEnv, KEY_STATE_CURSOR);
                idx++;
               continue;
            case kCLI_WORD_PREV:
                charIn = (_CHAR) kKEY_WORD_PREV;
                break;
            case kCLI_WORD_NEXT:
                charIn = (_CHAR) kKEY_WORD_NEXT;
                break;
            case kCLI_WORD_UPPERCASE_TO_END:
                charIn = (_CHAR) kKEY_UPPERCASE;
                break;
            case kCLI_WORD_DELETE_TO_END:
                charIn = (_CHAR) kKEY_DELETE_WORD_END;
                break;
            case kCLI_WORD_LOWERCASE_TO_END:
                charIn = (_CHAR) kKEY_LOWERCASE;
                break;
            }
             MEDIT_SetKeyStat(pCliEnv, KEY_STATE_DATA);
            break;

        case KEY_STATE_CURSOR:
            switch(charIn)
            {
            case kCLI_CURSOR_UP:
                charIn = (_CHAR) kKEY_MOVE_UP;
                break;
            case kCLI_CURSOR_DOWN:  
                charIn = (_CHAR) kKEY_MOVE_DOWN;
                break;
            case kCLI_CURSOR_LEFT:  
                charIn = (_CHAR) kKEY_MOVE_LEFT;
                break;
            case kCLI_CURSOR_RIGHT:
                charIn = (_CHAR) kKEY_MOVE_RIGHT;
                break;
            default:
                break;
            }
             MEDIT_SetKeyStat(pCliEnv, KEY_STATE_DATA);
            break;
            
        default:
            break;

        }

    switch ( charIn ) 
    {
            case kESC:
                 MEDIT_SetKeyStat(pCliEnv, KEY_STATE_ESC);
                break;

            case kCR: /* 字符结尾? */
	     case kLF:
		 if (SUCC != CLI_EXT_WriteStr(pCliEnv, kEOL))
                        return ERROR_CLI_WRITE;

                loop = FALSE;
                break;
            case kBS:
                EXT_Backspace(pCliEnv);
                break;
            case kKEY_BREAK:
                 if (CLI_IsEnabled(pCliEnv, kCLI_FLAG_LOGON))
                {
                    CLI_EXT_WriteStrLine(pCliEnv, "");
                    CLI_EXT_InitCommandLine(pCliEnv);
                    //RCC_TASK_PrintPrompt(pCliEnv);
                }
                break;
            case kTAB:
                break;
            case kKEY_DELETE:
            case kKEY_DELETE_CHAR:
                CLI_EXT_DeleteChar(pCliEnv);
                break;
            case kKEY_DELETE_FROM_START:
                break;
            case kKEY_DELETE_TO_END:
                break;
            case kKEY_LINE_START:
                break;
            case kKEY_LINE_END:
                break;
            case kKEY_MOVE_UP:
                CLI_HIST_Scroll(pCliEnv, -1);
                break;
            case kKEY_MOVE_DOWN:
                CLI_HIST_Scroll(pCliEnv, 1);
                break;
            case kKEY_MOVE_LEFT:
                CLI_EXT_MoveCursor(pCliEnv, -1);
                break;
            case kKEY_MOVE_RIGHT:
                CLI_EXT_MoveCursor(pCliEnv, 1);
                break;
            case kKEY_WORD_PREV:
                break;
            case kKEY_WORD_NEXT:
                break;
            case kKEY_UPPERCASE:
                break;
            case kKEY_LOWERCASE:
                break;
            case kKEY_DELETE_WORD_END:
                break;
            case kKEY_DELETE_WORD_START:
                break;
            case kKEY_TRANSPOSE:
                break;
            case kKEY_END_OF_ENTRY:
          CLI_EXT_WriteStrLine(pCliEnv, kEOL);
                status = STATUS_CLI_EXIT_TO_ROOT;
                loop   = FALSE;
                break;
            case 25: /* Ctrl-y */
                break;
            case kKEY_HELP:

                // 即时显示帮助
             CLI_EXT_InsertText(pCliEnv, (_CHAR *) &charIn, 1);
                
              if (SUCC != CLI_EXT_WriteStr(pCliEnv, kEOL))
                        return ERROR_CLI_WRITE;

                loop = FALSE;
                
                break; /* 帮助 */

         default:
              /* 实体字符 */
              if ((' ' <= charIn) && (charIn <= 127))
              {
                  CLI_EXT_InsertText(pCliEnv, (_CHAR *) &charIn, 1);
		   // if (SUCC != CLI_EXT_WriteStr(pCliEnv, &charIn))
                 //      return ERROR_CLI_WRITE;
              }
          break;
        }  /* switch */

    idx++;
    
    }
        

 //   CLI_DisableFeature(pCliEnv, kCLI_FLAG_INPUT);

 if(loop == FALSE)
    MMISC_SetInput(pCliEnv, 0);
 else
    MMISC_SetInput(pCliEnv, 1);

//用户鉴权
if(MMISC_GetLoginStat(pCliEnv) != kTELNET_USR_PROCESS)
    if(SUCC != CLI_TASK_Login(pCliEnv))
    {
            CLI_EXT_ResetOutput(pCliEnv);
            CLI_EXT_InitCommandLine(pCliEnv);          
    }

    return status;
}


_VOID CLI_EXT_NEWLINE(CLI_ENV* pCliEnv)
{
        CLI_EXT_Write(pCliEnv, kEOL, kEOL_SIZE);
        CLI_EXT_ResetOutput(pCliEnv);
        CLI_EXT_InitCommandLine(pCliEnv);

}

/*
* 部分功能函数
*
*/

/*-----------------------------------------------------------------------*/

_INT CLI_TASK_Login(CLI_ENV *pCliEnv)
{


    switch(MMISC_GetLoginStat(pCliEnv))
    {  
        case kTELNET_NOT_LOGIN:
        {
            memset(MMISC_Login(pCliEnv), 0, kCLI_MAX_LOGIN_LEN);
            memset(MMISC_Passwd(pCliEnv), 0, kCLI_MAX_PASSWORD_LEN);
            
            CLI_DisableFeature(pCliEnv, kCLI_FLAG_LOGON);

            CLI_EXT_WriteStrLine(pCliEnv, " ");
            CLI_EXT_WriteStrLine(pCliEnv, "                                                                    ");
            CLI_EXT_WriteStrLine(pCliEnv, "----------   欢迎使用 R5 Telnet 配置管理系统----------                   ");
            CLI_EXT_WriteStrLine(pCliEnv, "                                                                    ");
            CLI_EXT_WriteStrLine(pCliEnv, " ");    
            
            CLI_EXT_WriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
            
            CLI_EXT_ResetOutput(pCliEnv);
            CLI_EXT_InitCommandLine(pCliEnv);         

            //CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);  

            

            MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);

        }
        break;
        case kTELNET_USRNAME_LOGIN:
        {
            CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);  

            CLI_EXT_WriteStr(pCliEnv, kCLI_LOGIN_PROMPT);

            MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);

            CLI_EXT_ResetOutput(pCliEnv);
            CLI_EXT_InitCommandLine(pCliEnv);   
        }
        break;
        case kTELNET_USRNAME_INPUT:
        {
            if(MMISC_GetInput(pCliEnv) == 0)
            {
                if (kCLI_MAX_LOGIN_LEN <= MEDIT_GetLength(pCliEnv))
               {

                    CLI_EXT_WriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
                    return ERROR_CLI_RETRY;
               }

              if ( 0 == MEDIT_GetLength(pCliEnv))
              {
                    CLI_EXT_WriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);

                    return ERROR_CLI_RETRY;
              }

                MEDIT_CopyFromInput(pCliEnv, CLIENV(pCliEnv)->login);

                CLI_EXT_WriteStr(pCliEnv, kCLI_PASSWORD_PROMPT);
                //CLI_DisableFeature(pCliEnv, kCLI_FLAG_ECHO);  

                MMISC_SetLoginStat(pCliEnv, kTELNET_LOGIN_PROCESS);

               //CLI_EXT_NEWLINE (pCliEnv);
                CLI_EXT_ResetOutput(pCliEnv);
                CLI_EXT_InitCommandLine(pCliEnv);
                
            }
                
        }
        break;        
        case kTELNET_LOGIN_PROCESS:
        {
            if(MMISC_GetInput(pCliEnv) == 0)
            {
                if (kCLI_MAX_PASSWORD_LEN <= MEDIT_GetLength(pCliEnv))
                {
                    CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);  

                    //PASSWD DISABLE ECHO KEOL NOT DISPLAYED.
                    CLI_EXT_WriteStrLine(pCliEnv, "");
    
                    CLI_EXT_WriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
                    
                    return ERROR_CLI_RETRY;
                }

              if ( 0 == MEDIT_GetLength(pCliEnv))
                  {
                    CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);  

                    //PASSWD DISABLE ECHO KEOL NOT DISPLAYED.
                    CLI_EXT_WriteStrLine(pCliEnv, "");

                    CLI_EXT_WriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);
                      
                      return ERROR_CLI_RETRY;
              }

                MEDIT_CopyFromInput(pCliEnv, CLIENV(pCliEnv)->passwd);

                if (! CLI_AUTH_CALLBACK_FN(MMISC_Login(pCliEnv), MMISC_Passwd(pCliEnv), &(MMISC_GetAccess(pCliEnv))))
            {              
                    CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);  
                    CLI_EXT_WriteStrLine(pCliEnv, "");
                    
                    CLI_EXT_WriteStr(pCliEnv, kCLI_LOGIN_PROMPT);
                    MMISC_SetLoginStat(pCliEnv, kTELNET_USRNAME_INPUT);

                      return ERROR_CLI_RETRY;
               }

              CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);
              MMISC_SetLoginStat(pCliEnv, kTELNET_USR_PROCESS);

                CLI_EXT_NEWLINE(pCliEnv);       


              CLIENV(pCliEnv)->cliShellCmd.cliMode = sysMd;
              CLIENV(pCliEnv)->cliShellCmd.pCmdLst = sysCmds;

                /* 修改提示符*/
                InitPrompt(pCliEnv);


            }
            
            
        }
        break;
        default:
            break;
    }

    
    return SUCC;
}



_VOID CLI_TASK_PrintError(CLI_ENV *pCliEnv, _INT status)
{
    _CHAR      *errorText = MMISC_GetErrorText(pCliEnv);               

    CLI_EXT_WriteStr(pCliEnv, kCLI_MSG_ERROR_PREFIX);

    switch (status)
    {
        case ERROR_CLI_BAD_COMMAND:
            CLI_EXT_WriteStr(pCliEnv, kMSG_ERROR_CLI_BAD_COMMAND);
            break;
        case ERROR_CLI_INVALID_PARAM:
            CLI_EXT_WriteStr(pCliEnv, kMSG_ERROR_CLI_INVALID_PARAM);
            break;
        case ERROR_CLI_AMBIGUOUS_PARAM:
            CLI_EXT_WriteStr(pCliEnv, kMSG_ERROR_CLI_AMBIGUOUS_PARAM);
            break;
        default:
            CLI_EXT_WriteStr(pCliEnv, kMSG_ERROR_CLI_DEFAULT);
    }
    
    if (! NULL_STRING(errorText))
    {
        CLI_EXT_WriteStr(pCliEnv, errorText);
        MMISC_ClearErrorText(pCliEnv);
    }

    CLI_EXT_WriteStrLine(pCliEnv, "");
}


_CHAR CLI_TASK_ValidateLogin(_CHAR *pLogin, _CHAR *pPassword, Access *pAccLvl/* 用户级别暂未使用*/)
{

   /* if ( (0 == strcmp(kCLI_DEFAULT_LOGIN,    pLogin)) &&
         (0 == strcmp(kCLI_DEFAULT_PASSWORD, pPassword)) )*/
    if( ( (0 == strcmp(gstNvUser.user,    pLogin)) &&
         (0 == strcmp(gstNvUser.password, pPassword)) )||( (0 == strcmp(kCLI_DEFAULT_LOGIN,    pLogin)) &&
         (0 == strcmp(kCLI_DEFAULT_PASSWORD, pPassword)) ))
    {
        return TRUE;
    }
    printf("\nuser or pswd err,right:%s,%s,but in :%s,%s\n",gstNvUser.user,gstNvUser.password,pLogin,pPassword);
    return FALSE;
}


 _INT CLI_UTIL_Init(CLI_ENV *pCliEnv)
{
    //_INT status;

    /* enable intermediate mode */
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_MODE);
    /* expanded error warnings */
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_WARN);
    /* show help for child nodes */
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_HELPCHILD);
    /* write to output updates cursor position*/
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_UPDATE);
    /* enable paging of output */
    /* make sure output is initially visible */
    CLI_EnableFeature(pCliEnv, kCLI_FLAG_ECHO);


    return SUCC;
}


/* ------------------------------------------------------------------------------*/

_INT CLI_DB_InitEnvironment(CLI_ENV **ppCliEnv, COM_CHAN *pChannel)
{
    _INT      status;
    CLI_INFO     *pCliEnv = NULL;
    GEN_ENVIRON  *pNewEnv = NULL;
    CLI_ENV     **pCli    = &pNewEnv;
    int si;

    if (SUCC != (status = ENVIRONMENT_Construct(&pNewEnv)))
        return status;

    si = sizeof(CLI_INFO);
    if (NULL == (pCliEnv =(CLI_INFO*) RC_CALLOC(1, sizeof(*pCliEnv))))
    {
        ENVIRONMENT_Destruct(&pNewEnv);
        return SYS_ERROR_NO_MEMORY;
    }

    pNewEnv->pCli         = pCliEnv;
    pCliEnv->pChannel     = pChannel;
    pCliEnv->pEnvironment = pNewEnv;
    pCliEnv->chMore    = 0;


#ifndef __DISABLE_STRUCTURES__
    status = Cache_Construct(&pNewEnv->phCacheHandle, K_CACHE_READWRITE);
    if( SUCC > status )
    {
        ENVIRONMENT_Destruct(&pNewEnv);
        FREEMEM(pCliEnv);
        return status;
    }
#endif

    pChannel->env = pNewEnv;
    *ppCliEnv     = *pCli;

    return status;
}

_VOID CLI_DB_DestroyEnvironment(CLI_ENV *pCliEnv)
{
    
    FREEMEM(pCliEnv->pCli);
    ENVIRONMENT_Destruct(&pCliEnv);
}

_INT ENVIRONMENT_Construct(GEN_ENVIRON **pp_envInit)
{
    if (NULL == (*pp_envInit = RC_MALLOC(sizeof(GEN_ENVIRON))))
        return ERROR_MEMMGR_NO_MEMORY;

    memset(*pp_envInit, 0x00, sizeof(GEN_ENVIRON));


    (*pp_envInit)->PostValid            = TRUE;
    (*pp_envInit)->PostBuffer           = NULL;
    (*pp_envInit)->PostBufferLen        = 0;
    (*pp_envInit)->UserLevel            = 0;
    (*pp_envInit)->clientIndex          = -1;
    

//    for (index = 0; index < K_MAX_ENV_SIZE; index++)
//        (*pp_envInit)->variables[index] = NULL;


    return SUCC;

}   /* ENVIRONMENT_Construct */

_INT ENVIRONMENT_Destruct(GEN_ENVIRON **pp_envTemp)
{
    GEN_ENVIRON *p_envTemp;

    if (NULL == pp_envTemp)
        return ERROR_GENERAL_NO_DATA;

    p_envTemp = *pp_envTemp;

    if (NULL == p_envTemp)
        return ERROR_GENERAL_NO_DATA;

    if (NULL != p_envTemp->phCacheHandle)
    {
        p_envTemp->phCacheHandle = NULL;
    }

    if (NULL != p_envTemp->PostBuffer)
    {
        RC_FREE(p_envTemp->PostBuffer);
        p_envTemp->PostBuffer = NULL;
    }

//    for (index = 0; index < K_MAX_ENV_SIZE; index++)
//        if (NULL != p_envTemp->variables[index])
//        {
//            RC_FREE(p_envTemp->variables[index]);
//            p_envTemp->variables[index] = NULL;
//        }

    RC_FREE(p_envTemp);
    *pp_envTemp = NULL;

    return SUCC;
}

_INT Cache_Construct(CACHE_HANDLE **pph_cacHandle, Access AccessType)
{
    
    if (NULL == pph_cacHandle)
        return ERROR_GENERAL_ACCESS_DENIED;

    if (NULL == (*pph_cacHandle = (CACHE_HANDLE *) RC_MALLOC (sizeof(CACHE_HANDLE))))
        return ERROR_MEMMGR_NO_MEMORY;

    if (NULL == ((*pph_cacHandle)->p_lstCacheObjects = List_Construct()))
    {
        RC_FREE(*pph_cacHandle);
        *pph_cacHandle = NULL;

        return ERROR_MEMMGR_NO_MEMORY;
    }

    (*pph_cacHandle)->AccessType = AccessType;

    return SUCC;
}



_VOID int_to_ip(_UINT addr, _CHAR *ip)
{
    _CHAR ip_buffer[16];
    _INT curr_num, curr_dig;
    _UINT num;
    _CHAR *p = &(ip_buffer[14]);
    
    strcpy(ip_buffer, "xxx.xxx.xxx.xxx");    
    for (curr_num = 0; curr_num <= 3; curr_num++)
    {
        num = addr & 0xFF; 
        addr >>= 8;
        for (curr_dig = 0; curr_dig <= 2; curr_dig++)
        {
            *p = (_CHAR)(((_CHAR)(num % 10) + '0'));
            num /= 10;
            p--;
            if (num == 0)
                break;
        }
        if (curr_num < 3)
            *p-- = '.';
    }
    strcpy(ip,(p+1));
}

_VOID StrToUp(_CHAR * s)
{
    _INT i=0;
    for(i = 0; i < (_INT)strlen(s); i++)
        if(s[i] >= 'a' && s[i] <= 'z' )
            s[i] -= 0x20;
    
}

_UINT ip_to_int(const _CHAR *cp, _UINT *pnet_addr)
{
#if 0
    _CHAR *p=cp;
    _UCHAR ucAddr[4]={0,0,0,0};
    _UINT  nIpLen=0;
    _UINT i;
	
    if (cp==NULL)	
    {
	    return FAIL;
    }
    
    for (i=0;i<4;i++)
    {
        if ((*p==0) || (*p=='.')) return FAIL;
		
        do 
        {
	     //非法字符
            if (!((*p>='0') && (*p<='9')))  return FAIL;
            
            ucAddr[i]*=10;
            ucAddr[i]+=(*p - '0');
            if (ucAddr[i]>0xff) return FAIL;

	     p++;
            if (*p == 0)
	     { 
		  i=4;
		  break;
	     }
			
        } while (*p != '.');
		
	if (i<4)
		p++;
	nIpLen++;
    } 
    
    //完成性判断
    if ((nIpLen!=4) || (*p!=0)) return FAIL;
	
	*pnet_addr=ucAddr[3];
	*pnet_addr+=(ucAddr[2] <<  8);
	*pnet_addr+=(ucAddr[1] << 16);
	*pnet_addr+=(ucAddr[0] << 24);
#endif	
	return SUCC;
}


_INT TelnetPrintf(const char *_format, ...)
{
	va_list ap;
	_CHAR  ucStr[500] = {0};
	unsigned short usBuffLen = 0xFFFF;
	unsigned int  ulTickVal = 0xFFFFFFFF;

	/*---------------------FOR TEST SunShanggu----------------------------------*/
	va_start( ap, _format );
	usBuffLen = (unsigned short)vsprintf(ucStr, _format, ap);
	if( 485 < usBuffLen )
	{
		sprintf(ucStr, "\r\n PRINT INFO IS TOO LONG TO PRINT ");
	}
	va_end( ap );

	//广播到所有TELNET终端
	if(ucOut2Telnet == 1)
	{
		CLI_TELNETD_BroadcastMessage(ucStr, 0);
	}
	else
	{
		printf("%s",ucStr);
	}

	return 0;
}
