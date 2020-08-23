/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: BtsLog.cpp
 *
 * DESCRIPTION:
 *     Implementation of the FWKLIB's Log Utility.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 10/31/2005  Liu Qun     Added CPE log support interface.
 * 09/04/2005  Liu Qun     Initial file creation.
 * 02/14/2006  Yushu Shi   Change this file for BTS LOG implementation only
 * 03/23/2006  Yushu Shi   Add destination and source task name to comMsg display info
 *---------------------------------------------------------------------------*/
#ifndef __WIN32_SIM__
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <iomanip.h>
#endif

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

#include "Log.h"
#include "ComMessage.h"
#include "LogArea.h"
#include "DebugLevel.h"
#include "vxwk2pthread.h"
#include "Vxw_hdrs.h"

extern CDebugLevel g_debugLevel[LOG_AI_MAX];
extern UINT8 EntityMsgLogEnableTable[M_TID_MAX][M_TID_MAX];
extern char *EntityNameStr[M_TID_MAX];
#ifdef WBBU_CODE
extern "C" unsigned char   print_flag_WBBU = 0;
#endif
///////////////////////////////////////////////////////////////////////////
CLog* CLog::Instance = NULL;

CLog* CLog::GetInstance()
{
    if ( NULL == Instance)
    {
        Instance = new CLog();
    }
    return Instance;
}


CLog::CLog()      
{

    LogErrorCount = 0;
    // initialize the node list
    ::lstInit(&FreeLogNodeList);

}


bool CLog::StartLogTask()
{
    /* Create the message queue*/
    LogMsgQueId = msgQCreate (MAX_LOG_QUEUE_DEPTH, sizeof(T_LogMsg), MSG_Q_FIFO);

    printf("LogMsgQueId is 0x%x\n", LogMsgQueId);
    LOG_NODE *node = (LOG_NODE*) new UINT8[sizeof(LOG_NODE)*MAX_LOG_QUEUE_DEPTH];
    if (!node)
    {
        printf("failed to create tLog task node list\n");
        return false;
    }
    ::taskLock();
//    UINT32 oldLevel = ::intLock();//delete by huangjl
    for (int i = 0; i < MAX_LOG_QUEUE_DEPTH; ++i)
    {
        memset(node, 0, sizeof(LOG_NODE));
        ::lstAdd(&FreeLogNodeList, &node->lstNode);
        node ++;
    }
//    ::intUnlock(oldLevel);//delete by huangjl
    ::taskUnlock();
    // create the log task
    int _tid = taskSpawn ("tLog", M_TP_L3LOG, 0, 0, (FUNCPTR)_LogMain/*task11 _LogMain*/, 
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    if (_tid)
    {
    	printf("StartLogTask:%x\n",_tid);
        return true;
    }
    else
    {
        printf("failed to create tLog task\n");
        return false;
    }

}


void CLog::_LogMain()
{
	printf("_LogMain\n");
    CLog::GetInstance()->LogMain();
}

void CLog::DisplayMsg(T_LogMsg& msg, char* buff)
{
    int len = strlen(buff);
    if (len)
    {
        if ('\n' == buff[len - 1])
            buff[len - 1] = '\0'; // Chop off CR
         else
            buff[len] = '\0';
    }

	printf("%09d %12s:%04x %20s(%4d):%s\n",
			msg.logTime,
			::taskName(msg.tid),
			msg.errCode,
			msg.fileName,
			msg.line,
			buff
            );
}


void CLog::LogMain()
{

    INT32 errCntSnapshot = 0;
    INT32 errCntDelta;
    T_LogMsg msg;
    char buff[MAX_STRING_LEN+100];
    int rc;
    for (;;)
    {
        printf("before msgQReceive\n");
        rc = msgQReceive (LogMsgQueId, (char *) &msg, sizeof(msg), WAIT_FOREVER);/*1000*/
        printf("after msgQReceive\n");
        if (OK == rc)//            if (sizeof(T_LogMsg) == rc)
        {
            errCntDelta    = LogErrorCount - errCntSnapshot;
            errCntSnapshot = LogErrorCount;

            if (errCntDelta > 0)
            {
                T_LogMsg dropMsg;

                dropMsg.tid = ::taskIdSelf();
                dropMsg.logTime  = time(NULL);
                dropMsg.errCode  = 0;
                dropMsg.fileName = __FILE__;
                dropMsg.line     = __LINE__;

                sprintf(buff, "Failed to log %d messages", errCntDelta);
                DisplayMsg(dropMsg, buff);
            }
            switch (msg.type)
            {
                case LOG_TEXT_MSG:
                    sprintf(buff, msg.node->text, msg.arg0, msg.arg1, msg.arg2, msg.arg3,msg.arg4,msg.arg5);
                    DisplayMsg(msg, buff);
                    break;

                case LOG_COM_MSG:
                    {
                        printf("in LOG_COM_MSG\n");
                        UINT16 dstTid = (msg.arg0>>16)&0xffff;
                        UINT16 srcTid = msg.arg0&0xffff;
                        if (EntityMsgLogEnableTable[srcTid][dstTid])
                        {
                            DisplayMsg(msg, &msg.node->text[0]);
                            ::sprintf(buff,"    Dst:%d(%s), Src:%d(%s), ID:0x%.4X, Length:%d, EID:%.8X, FLAG:%.8X",
                                      dstTid, EntityNameStr[dstTid], srcTid, EntityNameStr[srcTid],
                                      (msg.arg1>>16)&0xffff, msg.arg1&0xffff, msg.arg2, msg.arg3);
                            printf("%s\n", buff);
                            if (msg.comMsgData)
                            {
                                int i=0, j;
                                sprintf(buff, "                ");
                                for (j=10; i<(msg.arg1&0xffff); j+=3)
                                {
                                    sprintf(buff+j,"%.2X ",msg.comMsgData[i++]);
                                    if (!(i%20))
                                    {
                                        buff[j+3]= '\0';
                                        printf("%s\n", buff);
                                        sprintf(buff, "                ");
                                        j=7;
                                    }
                                }
                                if (i%20)
                                {
                                    buff[j+3] = '\0';
                                    printf("%s\n", buff);
                                }
                                delete [] msg.comMsgData;
                            }
                            else
                            {
                                printf("Empty Com Message\n");
                            }
                        }
                        else
                        {
                        	if (msg.comMsgData)
                        	{
                        		 delete [] msg.comMsgData;
                        	}
                        }
                    }
                    break;

                case LOG_STRING:
                    sprintf(buff, &msg.node->text[0], msg.arg0, msg.arg1, msg.arg2, msg.arg3);
                    buff[MAX_STRING_LEN] = '\0';
                    printf("%s\n", buff);
                    break;
            }
	    ::taskLock();	
 //           UINT32 oldLevel = ::intLock();//delete by huangjl
            ::lstAdd(&FreeLogNodeList, &msg.node->lstNode);
 //           ::intUnlock(oldLevel);//delete by huangjl
		::taskUnlock();
        }
        else
        {
            printf("error receive que msg %d\n", rc);
            taskDelay(500);
        }
    }
}



///////////////////////////////////////////////////////////////////////////
#ifndef DMBT
UINT32 sysFrameNum = 0;
#endif
void CLog::LogDebugMsg(const char* lpszFileName,int nLine,LOGLEVEL level,UINT32 errcode,const char *text, 
                       int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, UINT32 eid)//modified by maqiang
{
#ifdef WBBU_CODE
  if(print_flag_WBBU==1)
  {
    printf("print_flag_WBBU flag should be set first\n");
   	return;
  }
#endif
    printf("\nin LogDebugMsg\n");
    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        printf("log level is not right\n");
        return;
    }
    else
    {
        printf("log level:%d, g_debugLevel:%d\n",level, g_debugLevel[errcode>>16].GetDebugLevel());
    }
    
    if (text)
    {
        T_LogMsg logMsg;

        // try to get a free node form the pool
        ::taskLock();
//        UINT32 oldLevel = ::intLock();//delete by huangjl
        logMsg.node = (LOG_NODE*)::lstGet(&FreeLogNodeList);
//        ::intUnlock(oldLevel);//delete by huangjl
	 ::taskUnlock();
        if (logMsg.node)
        {
            logMsg.type     = LOG_TEXT_MSG;
            logMsg.tid      = ::taskIdSelf();
//            #ifdef M_TGT_L2
            logMsg.logTime  = sysFrameNum;
/*
            #else
            logMsg.logTime  = time(NULL);
            #endif
*/            
            logMsg.errCode  = errcode;
            logMsg.fileName = lpszFileName;
            logMsg.line     = nLine;
            logMsg.arg0     = arg1;
            logMsg.arg1     = arg2;
            logMsg.arg2     = arg3;
            logMsg.arg3     = arg4;
            logMsg.arg4     = arg5;//added by maqiang
            logMsg.arg5     = arg6;//added by maqiang

            if (eid==0)//have not Eid info, Print it directly   
            {
            
                printf(" in eid==0\n");
                 strncpy(logMsg.node->text, text, sizeof(logMsg.node->text) - sizeof('\0'));            
            }
            else
            {
            
                //对比Eid 跟踪表
                if(EidTbl4LOG.Find(eid))
                {
                    sprintf(logMsg.node->text,"EID=[%08x], ",eid);
                    strncat(logMsg.node->text, text, sizeof(logMsg.node->text) - 20);
                    //strncpy(logMsg.node->text, text, sizeof(logMsg.node->text) - sizeof('\0'));            

                }
                else
                {
                    // don't need to send, drop msg
                    ::taskLock();
   //                 UINT32 oldLevel = ::intLock();//delete by huangjl
                    ::lstAdd(&FreeLogNodeList, &logMsg.node->lstNode);
   //                 ::intUnlock(oldLevel);//delete by huangjl
								::taskUnlock();
                    printf(" i have return\n");
                    return;
                }
                    
            }

            if (ERROR == msgQSend (LogMsgQueId, (char *) &logMsg, sizeof(logMsg), NO_WAIT,  MSG_PRI_NORMAL))
            {
            
                printf(" in msgQSend\n");
                ::taskLock();
 //               UINT32 oldLevel = ::intLock();//delete by huangjl
                ::lstAdd(&FreeLogNodeList, &logMsg.node->lstNode);
//				::intUnlock(oldLevel);//delete by huangjl
						::taskUnlock();
                ++LogErrorCount;
            }
        }
        else
        {
            ++LogErrorCount;
        }
    }
}

void CLog::LogComMessage(const char* lpszFileName,int nLine,LOGLEVEL level,UINT32 errcode,const CComMessage* pComMsg,const char *text)
{

    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    if (text)
    {
        T_LogMsg logMsg;

        // try to get a free node form the pool
        ::taskLock();
//        UINT32 oldLevel = ::intLock();//delete by huangjl
        logMsg.node = (LOG_NODE*)::lstGet(&FreeLogNodeList);
 //       ::intUnlock(oldLevel);//delete by huangjl
				::taskUnlock();

        if (logMsg.node)
        {
            logMsg.type     = LOG_COM_MSG;
            logMsg.tid      = ::taskIdSelf();
            logMsg.logTime  = time(NULL);
            logMsg.errCode  = errcode;
            logMsg.fileName = lpszFileName;
            logMsg.line     = nLine;
            logMsg.arg0     = (pComMsg->GetDstTid() <<16 ) | pComMsg->GetSrcTid();
            logMsg.arg1     = (pComMsg->GetMessageId() << 16) | pComMsg->GetDataLength();
            logMsg.arg2     = pComMsg->GetEID();
            logMsg.arg3     = pComMsg->GetFlag();

            strncpy(logMsg.node->text, text, sizeof(logMsg.node->text) - sizeof('\0'));

            logMsg.comMsgData = new UINT8[pComMsg->GetDataLength()];
            if (logMsg.comMsgData)
            {
                memcpy(logMsg.comMsgData, pComMsg->GetDataPtr(), pComMsg->GetDataLength());
            }
printf("have send already0\n");
            if (ERROR == msgQSend (LogMsgQueId, (char *) &logMsg, sizeof(logMsg), NO_WAIT,  MSG_PRI_NORMAL))
            {
                ::taskLock();
      //          UINT32 oldLevel = ::intLock();//delete by huangjl
                ::lstAdd(&FreeLogNodeList, &logMsg.node->lstNode);
     //           ::intUnlock(oldLevel);//delete by huangjl
                ::taskUnlock();
                ++LogErrorCount;
            }
        }
        else
        {
            ++LogErrorCount;
        }
    }
}



void CLog::LogDbgString(LOGLEVEL level, UINT32 errcode, const char* text, int arg1, int arg2, int arg3, int arg4)
{
#ifdef WBBU_CODE
   if(print_flag_WBBU==1)
   	return;
#endif
    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    if (text)
    {
        T_LogMsg logMsg;

        // try to get a free node form the pool
        ::taskLock();
  //      UINT32 oldLevel = ::intLock();//delete by huangjl
        logMsg.node = (LOG_NODE*)::lstGet(&FreeLogNodeList);
  //      ::intUnlock(oldLevel);//delete by huangjl
        ::taskUnlock();

        if (logMsg.node)
        {
            logMsg.type     = LOG_STRING;
            logMsg.arg0     = arg1;
            logMsg.arg1     = arg2;
            logMsg.arg2     = arg3;
            logMsg.arg3     = arg4;

            strncpy(logMsg.node->text, text, sizeof(logMsg.node->text) - sizeof('\0'));

            if (ERROR == msgQSend (LogMsgQueId, (char *) &logMsg, sizeof(logMsg), NO_WAIT,  MSG_PRI_NORMAL))
            {
                ::taskLock();
  //              UINT32 oldLevel = ::intLock();//delete by huangjl
                ::lstAdd(&FreeLogNodeList, &logMsg.node->lstNode);
  //              ::intUnlock(oldLevel);//delete by huangjl
                ::taskUnlock();			
                ++LogErrorCount;
            }
        }
        else
        {
            ++LogErrorCount;
        }
    }
}


CEidTBL::CEidTBL()
{
     m_Status=ALL_OPEN;
     ClearTbl();
}

void CEidTBL::ClearTbl()
{
    m_Index = 0;
    memset(m_EidTable, 0, sizeof(m_EidTable));
    
}


#define EIDALLENABLE 0xffffffff


bool CEidTBL::Find(UINT32 eid)
{   
    switch (m_Status)
    {
        case ALL_OPEN:
            return true;

        case ALL_CLOSE:
            return false;
            
        case SELECTED:                      
            for ( short i=0; i<EIDTBL_DEPTH; i++)
            {
                if (m_EidTable[i]==eid)  
                {
                    m_Index=i;
                    return true;
                }
            }
            break;

        default:
            break;
                
    }
    
    return false;   
}

void CEidTBL::Add(UINT32 eid)
{
    if (eid == EIDALLENABLE)
    {
        m_Status = ALL_OPEN;
        ClearTbl();
        printf("Log open for all Eid!\n");
    }
    else 
    {
        m_Status = SELECTED;
        if (Find(eid))
        {
            printf("Eid 0x%08x aready open for LOG\n",eid);
            return;
        }        

        if (Find(EMPTY_EID))   //find for Empty Element
        {
            printf("Log open for Eid:0x%08x !\n",eid);
            m_EidTable[m_Index] = eid;                
            return;
        }
        else
            printf("EidTable is Full, Add Failed!\n");

    }
}

void CEidTBL::Del(UINT32 eid)
{
    
    if (eid == EIDALLENABLE)
    {
        m_Status = ALL_CLOSE;
        ClearTbl();
        printf("Log close for all Eid!\n");
    }
    else 
    {
        if(m_Status != SELECTED)
        {
            printf("Eid:[0x%08x] not finded, TblStatus=[%d],Del Failed!\n",eid, m_Status);
            return;
        }
        if(Find(eid))
        {
            printf("Log close for Eid:0x%08x !\n",eid);
            m_EidTable[m_Index] = 0;  
        }
        else
        {
            printf("Eid:[0x%08x] not finded in EidTable, Del Failed!\n",eid);
        }

    }
}

void CEidTBL::Show()
{
    switch (m_Status)
    {
        case ALL_OPEN  :
            printf("All Eid is Allowed  for  LOG\n");
            break;
            
        case ALL_CLOSE :
            printf("All Eid is Forbidden for LOG\n");
            break;
            
        case SELECTED:
            for (int i=0;i<EIDTBL_DEPTH;i++)
            {
                if (m_EidTable[i]!=EMPTY_EID)
                {
                    printf ("Eid: [0x%08x] Allowed for LOG. \n",m_EidTable[i]);
                }
            }
            break;

        default:
            printf ("Table Status=%d,  ERR!\n",m_Status);
    }
}

bool InitLogTask()
{
    CLog *logger = CLog::GetInstance();
    if (logger)
    {
    	printf("InitLogTask\n");
        return logger->StartLogTask();
    }
    else
    {
        return false;
    }
}
#ifdef WBBU_CODE
extern "C" void LOG_STR_F(LOGLEVEL level, UINT32 errcode, const char* text)
{
	CLog::GetInstance()->LogDbgString(level, errcode, text, 0, 0, 0, 0) ;
}

// 0-打开，1-关闭
extern "C" void CloseMsg(unsigned char flag)
{
   print_flag_WBBU = flag;
}
#endif
#ifndef WBBU_CODE
/******************************************
*
*二层某些类存在成员函数getEid
*可根据Eid 进行选择性打印
*
*此函数是对上述功能的补充
*
*
******************************************/
UINT32 getEid()
{
    return 0;
}

extern "C" void LOGEidAdd(UINT32 eid)
{
    if(eid==0) 
    {
        printf("Err eid inputed!\n");
        return;
    }
    
    CLog::GetInstance()->EidTbl4LOG.Add(eid);
}

extern "C" void LOGEidDel(UINT32 eid)
{
    if(eid==0) 
    {
        printf("Err eid inputed!\n");
        return;
    }    
    CLog::GetInstance()->EidTbl4LOG.Del(eid);
}

extern "C" void LOGEidAddAll()
{
    CLog::GetInstance()->EidTbl4LOG.Add(EIDALLENABLE);
}

extern "C" void LOGEidDelAll()
{
     CLog::GetInstance()->EidTbl4LOG.Del(EIDALLENABLE);
}

extern "C" void ShowEidTBL()
{
     CLog::GetInstance()->EidTbl4LOG.Show();
  
}
#endif
