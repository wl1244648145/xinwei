/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L2PciShell.cpp
 *
 * DESCRIPTION:  implementation L2 shell redirection functions.
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   8/4/2006   Yushu Shi      Initial file creation.
 *   9/10/2012  huangjunlin    618 project:telnet L3 and send message to l2 by share memory
 *---------------------------------------------------------------------------*/
#include <stdio.h>
//#include <taskLib.h>
//#include <ioLib.h>
#include <string.h>
//#include <ledLib.h>
//#include <semLib.h>
#include <ctype.h>
#include "L2PciShell.h"
#include "Log.h"

#ifndef DIAG_COMMAND
	#define  DIAG_COMMAND        	  0x3810
#endif
#define     ENTER_COMMAND           0x3820
#define     EXIT_COMMAND            0x3821

#define		TO_RX  					  0
#define		TO_TX  					  1
#define		TO_RX_TX				  2
#define		L3_RX_TX				  3

int  		L2ShellLineEditorID 	= 0;
char 		L2ShellInputBuffer[MAX_CONSOLE_LINE_LEN];
static  	char strFrt[7];					   //命令提示符字串
int 		g_coreID = L3_RX_TX;				   //当前命令所在的核标记

CL2Shell* CL2Shell::Instance 	= NULL;

CL2Shell::CL2Shell()
{
    strcpy(m_szName, "tL2Shell");
    m_uPriority   		= M_TP_L2_SHELL_RX;
    m_uOptions    		= 0;
    m_uStackSize  		= 1024 * 10;
    m_iMsgQMax       	= MAX_MSG_FROM_L2*3;
    m_iMsgQOption    	= MSG_Q_FIFO;

    CommandToL2Count 	= 0;
    MsgFromL2Count 		= 0;
}

CL2Shell *CL2Shell::GetInstance()
{
    if ( NULL == Instance )
    {
        Instance = new CL2Shell;
    }
    return Instance;
}

bool CL2Shell::Initialize()
{

    if ( !CBizTask::Initialize() )
    {
        LOG(LOG_CRITICAL,0,"L2Shell Initialize failed.");
        return false;
    }

    SEM_L2ShellCount = ::semMCreate(SEM_Q_PRIORITY |SEM_INVERSION_SAFE|SEM_DELETE_SAFE);
	
    if (!SEM_L2ShellCount )
    {
        LOG(LOG_CRITICAL,0,"SEM L2ShellCount Create failed.");
        return false;
    }
}


/*****************************************************************************
 *   Description: 从L2接收消息并将消息打印出来
 *
 *   Parameters:  
 			pComMsg: 经过L3封装之后的来自L2的消息包格式
 *
 *   Returns:  true
 *****************************************************************************/  
bool CL2Shell::ProcessComMessage(CComMessage* pComMsg)
{
    char *rxBuff = (char *)pComMsg->GetDataPtr();
    int lineLen = pComMsg->GetDataLength();
    if ( '\n' == rxBuff[lineLen-1] )
    {
        rxBuff[lineLen-1] = '\0';
    }
    else
    {
	 	rxBuff[lineLen-1] = '\0';
    }
    printf("%s", rxBuff);
    
    pComMsg->Destroy();

    return true;
}



void CL2Shell::ShowStatus()
{
    printf("CommandToL2Count = %d\n",CommandToL2Count);
    printf("MsgFromL2Count = %d\n", MsgFromL2Count);
}


STATUS CL2Shell::SendCommandToL2(char *bufPtr, int lineLen)
{
    return 0;
}


STATUS CL2Shell::SendControlCommandToL2(bool enable)
{
	return 0;
};

extern "C" STATUS SendCommandToL2(char *bufPtr, int lineLen)
{
    return CL2Shell::GetInstance()->SendCommandToL2(bufPtr, lineLen);
}

#if 0
STATUS GetCommandListFromL2()
{}


static int __cmp_str(char *a, char *b)
{
	char *ps1,*ps2;
	//
	ps1=a;
	ps2=b;
	return strcmp(ps1,ps2);
}

/*************************************************************
*description:  g_str_tbl store the information of command string that come from L2
*
*
*
*************************************************************/
char **g_str_tbl;
#define COMMAND_LIST_SIZE    	80
#define COMMAND_MAX_SIZE		15

void CommandList_Init(char *str)
{
	for(int i =0 ; i< COMMAND_LIST_SIZE; i++)
		g_str_tbl[i] = new char[COMMAND_MAX_SIZE]();
}
#endif

void  CL2Shell::send_Notify_MessageToL2(int coreID, int messageID, char *str)
{
#if 0
	CComMessage *diagMsg = new(this, (strlen(str)+1))CComMessage;
    if(diagMsg)
    {
        	diagMsg->SetDstTid(M_TID_PCISIO);
        	diagMsg->SetSrcTid(M_TID_L2SHELL);
			diagMsg->SetMoudlue(coreID);

        	diagMsg->SetMessageId(messageID);
			strcpy((char*)diagMsg->GetDataPtr(), str);
        	if (CComEntity::PostEntityMessage(diagMsg))
        	{
				CommandToL2Count++;
        	}
        	else
        	{
            	diagMsg->Destroy();
        	}
    }
	else
           taskDelay(2);
    #endif
 }

/*****************************************************************************
 *
 *
 *   Description: 从标准telnet中获取用户输入并直接通过共享内存接口发送给L2
 *
 *   Parameters:  
 			coreID: 表示消息发往L2的哪个核
 *
 *   Returns:  rc
 *
 *****************************************************************************/        
STATUS CL2Shell::l2shell(int coreID)
{
    
    return 0;
}

/*****************************************************************************
 *   Description:
 *      l2sh0: 进入RX核模式，在该模式下，只输出RX核的输出信息
 *		l2sh1: 进入TX核模式，在该模式下，只输出RX核的输出信息
 *		l2sh:  进入RX, TX核模式，在该模式下，同时只输出RX,TX核的信息
 *		sh:	   进入RX, TX核模式，同时输出L2,RX,TX三者信息
 *   Parameters:  
 *			无
 *
 *   Returns:  true
 *****************************************************************************/  
extern "C" STATUS l2sh0()
{
    int coreID = TO_RX;
    return CL2Shell::GetInstance()->l2shell(coreID);
}

extern "C" STATUS l2sh1()
{
    int coreID = TO_TX;
	return CL2Shell::GetInstance()->l2shell(coreID);
}

extern "C" STATUS l2sh()
{
    int coreID = TO_RX_TX;
	return CL2Shell::GetInstance()->l2shell(coreID);
}

extern "C" STATUS sh()
{
    int coreID = L3_RX_TX;
	return CL2Shell::GetInstance()->l2shell(coreID);
}

/*下面两个函数接口功能已经无效， 但考虑到其他模块可能会调用,暂时留下，防止其他模块造成编译或者链接错误
stopL2Shell: 如果用户关闭了telnet客户端，重新telnet上去建议执行stopL2Shell命令。
*/
extern "C" STATUS stopL2Shell()
{
       CL2Shell::GetInstance()->send_Notify_MessageToL2(TO_RX, EXIT_COMMAND, "Exit_Command");
	   CL2Shell::GetInstance()->send_Notify_MessageToL2(TO_TX, EXIT_COMMAND, "Exit_Command");
       g_coreID = L3_RX_TX;	
       return 0;							 	     
}

extern "C" STATUS startL2Shell()
{
 	return 0;
}
