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
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <taskLib.h>
#include <ioLib.h>
#include <string.h>
#include <ledLib.h>
#include <semLib.h>

#include "L2PciShell.h"
#include "log.h"


static int L2ShellCount = 0;//used for count the number of l2shell for new telent --sunshanggu 080627
int  L2ShellLineEditorID = 0;
char L2ShellInputBuffer[MAX_CONSOLE_LINE_LEN];

CL2Shell* CL2Shell::Instance = NULL;

extern "C" int CLI_TELNETD_BroadcastMessage(char *pMessage, short authLevel);
extern int L2Shell;


/*****************************************************************************
 *
 *   Method:     CL2Shell::CL2Shell()
 *
 *   Description:  Constructor, initialize super class data members and private data members
 *
 *   Parameters:  None
 *
 *   Returns:  None
 *
 *****************************************************************************
 */
CL2Shell::CL2Shell()
{
    
    strcpy(m_szName, "tL2SioRx");
    m_uPriority   = M_TP_L2_SHELL_RX;
    m_uOptions    = 0;
    m_uStackSize  = 1024 * 10;
    m_iMsgQMax       = MAX_MSG_FROM_L2;
    m_iMsgQOption    = MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY ;

    CommandToL2Count = 0;
    MsgFromL2Count = 0;

}


/*****************************************************************************
 *
 *   Method:     CL2Shell::GetInstance()
 *
 *   Description:  Singleton
 *                
 *   Parameters:  none
 *
 *   Returns:  None
 *
 *****************************************************************************
 */
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

    if ( (QID_CmdToL2 = ::msgQCreate(MAX_CMD_TO_L2_BUF, MAX_CONSOLE_LINE_LEN, MSG_Q_FIFO))==NULL)
    {
        LOG(LOG_SEVERE,0,"ERROR!!! CL2Shell msgQCreate failed.");
        return false;
    }

    if ( (TID_L2ShellInput=::taskSpawn("tL2SioTx", M_TP_L2_SHELL_TX, 0, 10*1024,
                            (FUNCPTR)TxMainLoop, (int)this, 0, 0, 0, 0, 0,
                            0, 0, 0, 0)) == ERROR )
    {
        LOG(LOG_SEVERE,0,"ERROR!!! CL2Shell taskSpawn tL2SioTx failed.");
        return false;
    }

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

void CL2Shell::TxMainLoop(CL2Shell* taskObj)
{
    char inputLineBuf[MAX_CONSOLE_LINE_LEN];
    int  lineLen;

    FOREVER
    {
        lineLen =::msgQReceive(taskObj->QID_CmdToL2,(char*)inputLineBuf, MAX_CONSOLE_LINE_LEN-1,WAIT_FOREVER);
        if ( 0 != lineLen )
        {
            CComMessage *pComMsg = new (taskObj, lineLen+2) CComMessage;
            if (pComMsg)
            {
                pComMsg->SetDstTid(M_TID_PCISIO);
                pComMsg->SetSrcTid(M_TID_L2SHELL);
                pComMsg->SetMessageId(0x1234);
                if ( inputLineBuf[lineLen-1] != '\n')
                {
                      inputLineBuf[lineLen] = '\n';
                      lineLen ++;
                }
                inputLineBuf[lineLen] = '\0';
                pComMsg->SetDataLength(lineLen+1);
               
                memcpy(pComMsg->GetDataPtr(), inputLineBuf, lineLen+1);
                if ( false == CComEntity::PostEntityMessage(pComMsg))
                {
                    pComMsg->Destroy();
                }
                else
                {
                }
            }
        }
    }
}

char L2OutputBuffer[MAX_CONSOLE_LINE_LEN + 20];
const char *L2ShellHeader = "<L2Shell> ";
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

	//rxBuff -= strlen(L2ShellHeader);
	//memcpy(rxBuff, L2ShellHeader, strlen(L2ShellHeader));
//if(L2Shell)
   // CLI_TELNETD_BroadcastMessage(rxBuff, 0);
//else
    LOG_STR(LOG_CRITICAL, 0, rxBuff);

    pComMsg->Destroy();
    return true;
}



void CL2Shell::ShowStatus()
{
    printf("CommandToL2Count = %d\n",CommandToL2Count);
    printf("MsgFromL2Count = %d\n", MsgFromL2Count);
};


STATUS CL2Shell::SendCommandToL2(char *bufPtr, int lineLen)
{
    if (lineLen < MAX_CONSOLE_LINE_LEN)
    {
        CommandToL2Count ++;
        return msgQSend(QID_CmdToL2, bufPtr, lineLen, NO_WAIT, MSG_PRI_NORMAL);
    }
    else
    {
        printf("Command to Line too long, exceeds %d characters\n", MAX_CONSOLE_LINE_LEN);
    }

    return ERROR;
}


STATUS CL2Shell::SendControlCommandToL2(bool enable)
{
	CComMessage *pComMsg = new (this, 1) CComMessage;
	if (pComMsg)
	{
		pComMsg->SetDstTid(M_TID_PCISIO);
		pComMsg->SetSrcTid(M_TID_L2SHELL);
		pComMsg->SetMessageId(0x5678);
		pComMsg->SetDataLength(1);
		UINT8 *mode = (UINT8*)pComMsg->GetDataPtr();
		*mode = (enable == true)?1:0;
		if ( false == CComEntity::PostEntityMessage(pComMsg))
		{
			pComMsg->Destroy();
			return ERROR;
		}
	}
	return OK;
}

extern "C"
STATUS SendCommandToL2(char *bufPtr, int lineLen)
{
#ifndef WBBU_CODE
    return CL2Shell::GetInstance()->SendCommandToL2(bufPtr, lineLen);
#else
    return 0;
#endif
}

#ifndef WBBU_CODE
extern "C"
STATUS l2sh()
{
    STATUS rc = ERROR;
    char* exitStr[] =
    {
        "exit",
        "quit",
        "bye",
    };
    int nBytes;

#ifdef WBBU_CODE
    L2ShellLineEditorID =(int) ledOpen(STD_IN, STD_OUT, (int)MAX_CONSOLE_LINE_LEN);
#else
    L2ShellLineEditorID = ledOpen(STD_IN, STD_OUT, MAX_CONSOLE_LINE_LEN);
#endif    
    

    //L2ShellLineEditorID = ledOpen(STD_IN, STD_OUT, MAX_CONSOLE_LINE_LEN);

    if(L2ShellLineEditorID)
    {
        bool done = false;
        do
        {
	     printf("L2->");

#ifdef WBBU_CODE
          nBytes = ledRead((LED_ID )L2ShellLineEditorID, L2ShellInputBuffer, sizeof(L2ShellInputBuffer) - sizeof('\0'));
#else
            nBytes = ledRead(L2ShellLineEditorID, L2ShellInputBuffer, sizeof(L2ShellInputBuffer) - sizeof('\0'));
#endif
	     //printf("\nL2-recv:%s\n",L2ShellInputBuffer);
            if(EOF != nBytes)
            {
                for(unsigned int index = 0; index < SIZEOF(exitStr); ++index)
                {
                    if(!strncmp(L2ShellInputBuffer, exitStr[index], strlen(exitStr[index])))
                    {
                        done = true;
                    }
                }

                if(!done)
                {
                    SendCommandToL2(L2ShellInputBuffer, nBytes);
                    taskDelay(2);
                }
            }
            else
            {
                done = true;
            }
        } while(!done);
#ifndef WBBU_CODE
        rc = ledClose(L2ShellLineEditorID);
#else
        rc = ledClose((LED_ID )L2ShellLineEditorID);
#endif
        L2ShellLineEditorID = 0;

        printf("exit from L2 shell!\n");
    }

    return rc;
}


#endif

extern "C" STATUS stopL2Shell()
{
	/*::semTake (CL2Shell::GetInstance()->GetL2ShellSEM(), WAIT_FOREVER);
	L2ShellCount--;
	::semGive (CL2Shell::GetInstance()->GetL2ShellSEM());
	
	if(L2ShellCount==0)*/
	#ifndef WBBU_CODE
		return CL2Shell::GetInstance()->SendControlCommandToL2(false);
	#else
            printf("not support stopL2Shell routine\n ");
	     return 0;		
	#endif
	/*else
		return 0;			*/													 	     
}

extern "C" STATUS startL2Shell()
{
/*
	::semTake (CL2Shell::GetInstance()->GetL2ShellSEM(), WAIT_FOREVER);
	L2ShellCount++;
	::semGive (CL2Shell::GetInstance()->GetL2ShellSEM());
	
	if(L2ShellCount==1)*/
	#ifndef WBBU_CODE
		return CL2Shell::GetInstance()->SendControlCommandToL2(true);
     #else

          printf("not support startL2Shell routine\n ");
	 return 0;		
	 #endif
/*	else
		return 0;*/
}

