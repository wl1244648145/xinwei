/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   田静伟       Initial file creation.
 *   19/03/2006   刘剑峰       LN.757   M_BTS_EMS_UPGRADE_UT_SW_RSP   改为   M_BTS_EMS_BC_UPGRADE_UT_SW_RSP
 *                             LN.342   添加_REQ_DEBUG，停止状态变化
 *
 *---------------------------------------------------------------------------*/

#pragma warning (disable : 4786)
#ifdef  __WIN32_SIM__
#include <Winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#else
#include <vxWorks.h>
#include <stdio.h>
#include <ioLib.h>
#include <usrlib.h>
#include <ftpLib.h>
#include <string.h>
#include <inetLib.h>
#include <dirent.h>
#include "remLib.h"
#include "ramDrv.h"
#include "mcWill_bts.h"
#endif

#ifndef _INC_MSGQUEUE
#include "MsgQueue.h"
#endif

#ifndef _INC_L3OAMTEST
//#include "L3OamTest.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_L3OAMFILE
#include "L3OamFile.h"
#endif

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif


#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif

#ifndef _INC_ERRORCODEDEF
#include "ErrorCodeDef.h"
#endif

#ifndef _INC_L3OAMCOMMONRSP
#include "L3OamCommonRsp.h"
#endif

#ifndef _INC_L3OAMFILELOADNOTIFY
#include "L3OamFileLoadNotify.h"
#endif

#ifndef _INC_L3OAMFTPCLIENT
#include "L3OamFtpClient.h"
#endif
#include "l3oamcfg.h"
#include "l3oamcfgcommon.h"
#ifndef _INC_OAML3CPEM
#include "L3OamCpeM.h"
#endif

extern T_NvRamData *NvRamDataAddr;
extern "C"
{
    int bspGetLoadVersion_A();
    void bspSetLoadVersion_A(int);
    int bspGetLoadVersion_B();
    void bspSetLoadVersion_B(int);
    int  bspGetAtaState();
	#ifdef WBBU_CODE
	void LoadMcu();
	void LoadFpga();
	#endif
}
#ifdef WBBU_CODE
extern unsigned char Loading_Wrru_Code ;
#endif
CTaskFileMag* CTaskFileMag:: m_Instance = NULL;
//#define LJF_FILE_DL_RSP_TEST
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//文件管理部分:同EMS接口部分，文件升级部分
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void bspSetSMFlag(int plane)
{
    CTaskCfg::l3oambspNvRamWrite((char*)&(NvRamDataAddr->BTSCommonDataEle.BtsBootPlane), (char*)&plane, sizeof(int));
}

CTaskFileMag :: CTaskFileMag()
{
    strcpy(m_szName, "tFileM");
    m_uPriority   = M_TP_L3SM;
    m_uOptions    = 0;
    m_uStackSize  = SIZE_KBYTE * 50;
    m_iMsgQMax    = 100; 
    m_iMsgQOption = 0;

    m_SysStatus         = OAM_FILE_IDLE;
    m_CurUpdateCpeNum   = 0;
    m_SysDLReqSeqNum    = 0;

    memset(m_BCRcdA, 0, sizeof(m_BCRcdA));
    
    m_pcalFtpCTimer=NULL;//记录校准任务启动的定时器
    m_pdlFtpCTimer=NULL;//记录下载任务启动的定时器
    m_pUpdateCTimer = NULL;
    m_pFtpPMTimer = NULL;
    m_usDlType = 0xEEEE;
    m_usTransId = 0xEEEE;
    m_ftpPmTimeoutNum = 0;
    m_calFtpCTimeoutNum = 0;
    m_dlFtpCTimeoutNum = 0;
    m_ftpPmFlag = FALSE;
    m_calFtpCFlag = FALSE;
    m_dlFtpCFlag = FALSE;
    m_pPMTaskTimer = NULL;
    m_pFtpCTaskTimer = NULL;
    m_PmCreateFailNum = 0;
    m_ftpCCreateFailNum = 0;
}


bool CTaskFileMag :: Initialize()
{
    CBizTask :: Initialize();
    //m_bBCReqFlag = false;
    return true;
}

bool CTaskFileMag:: ProcessMessage(CMessage &rMsg)
{
    UINT16 MsgId;

    MsgId = rMsg.GetMessageId();
    OAM_LOGSTR2(LOG_DEBUG, L3SM_ERROR_REV_MSG, "l3oam file task receive msg = 0x%04x, eid = %x", MsgId, rMsg.GetEID());
    switch(MsgId)
    {
        case CALFTPCLIENT_TIMER:
            SM_calFtpClientTimeout();
            break;
        case DLFTPCLIENT_TIMER:
            SM_dlFtpClientTimeout();
            break;
        case M_EMS_BTS_DL_BTS_SW_REQ:
//        case M_EMS_BTS_DL_UT_SW_REQ:
//        case M_EMS_BTS_DL_Z_SW_REQ://Z SW
            SM_FileLoadReq(rMsg);
            break;
        
        case M_BTS_EMS_DL_BTS_SW_RESULT_NOTIFY://处理ftp client 返回的文件下载通知
        case M_BTS_EMS_DL_UT_SW_RESULT_NOTIFY: //处理ftp client 返回的文件下载通知
//        case M_BTS_EMS_DL_Z_SW_RESULT_NOTIFY: //Z SW
        {
            CL3OamCommonRsp rNotify(rMsg);
            SM_FileLoadResultNotify(rNotify);
            break;
        }

        case M_EMS_BTS_UPGRADE_BTS_SW_REQ:
        {
            CBtsSWUpdateReq rReq(rMsg);
            SM_BtsUpgradeReq(rReq);
            break;
        }
        
//        case M_EMS_BTS_UPGRADE_UT_SW_REQ://EMS -> CPE_UC请求 0x040A
//        case M_EMS_BTS_UPGRADE_Z_SW_REQ://EMS -> ZSW请求 0x0424
//        {
//            SM_CpeUpgradeReq(rMsg);
//            break;
//        }
        case M_EMS_BTS_DL_UT_SW_REQ_NEW:
        {
            SM_FileLoadReqNew(rMsg);
            break;
        }	
        case M_EMS_BTS_UPGRADE_UT_SW_REQ_NEW:
        case M_EMS_BTS_UPGRADE_UT_BTLDER_REQ_NEW:
        {
            SM_CpeUpgradeReqNew(rMsg);
            break;
        }	
#if 0
        case M_EMS_BTS_BC_UPGRADE_UT_SW_REQ_NEW:
        {
			CBCUTSWReqNew rReq(rMsg);
            SM_CpeBCUpgradeReqNew(rReq);
            break;
        }			
#endif        
        case M_OAM_UNICAST_UTSW_REQ_FAIL://0x2040  transaction超时是的处理函数
        {
            CL3OamUnicastUTSWReqFail FailMsg(rMsg);
            SM_CpeUpgradeReqFail(FailMsg);
            break;
        }  
        
        //case M_BTS_EMS_UPGRADE_UT_SW_RSP://0x040B  ???????????????????
        case M_CPE_L3_UPGRADE_SW_RSP://0x6402  ???????????????????
        case M_CPE_L3_UPGRADE_Z_SW_RSP: 
        {
            CL3OamCommonRspFromCpe rRsp(rMsg);
            SM_CpeZUpgradeRsp(rRsp, MsgId);
            break;
        }

        case M_CPE_L3_UPGRADE_SW_PACK_RSP://ox6405
        case M_CPE_L3_DL_Z_SW_PACK_RSP:
        {
            CL3CpeSWDLPackRsp rRsp(rMsg);
            SM_CpeZUpgradePackRsp(rRsp);
            break;
        }

        //case M_BTS_EMS_UPGRADE_UT_SW_NOTIFY://The Result Notify 0x040E  ???????????????????
        case M_CPE_L3_UPGRADE_SW_FINISH_NOTIFY:// 0x6407  ???????????????????
        case M_CPE_L3_UPGRADE_Z_SW_FINISH_NOTIFY:
        {
            CUpdateUTSWResultNotify rNotify(rMsg);
            SM_CpeUpgradeNotify(rNotify);
            break;
        }
#if 0
        case M_EMS_BTS_BC_UPGRADE_UT_SW_REQ:
        {
            CBCUTSWReq rReq(rMsg);
            SM_CpeBCUpgradeReq(rReq);
            break;
        }

        case M_OAM_BC_UTSW_TIMER:
        {
            CL3BCUTSWTimer rBCTimerMsg(rMsg);
            SM_CpeBCUpgradeTimer( rBCTimerMsg );
            break;
        }
		
        case M_OAM_BC_UTSW_TIMER_NEW:
        {
            CL3BCUTSWTimerNew rBCTimerMsg(rMsg);
            SM_CpeBCUpgradeTimerNew( rBCTimerMsg );
            break;
        }
#endif
     case UPDATE_TIMER:
	 	SM_UpdateTimeout(rMsg);
		break;
	case FTPPM_TIMER:
		SM_FtpPMTimeout();
		break;
	case PMFAIL_TIMER:
		SM_PMCreateTimeout();
		break;
	case FTPCFAIL_TIMER:
		SM_FtpCCreateTimeout();
		break;
	case M_UM_SM_UT_MOVE_AWAY_NOTIFY:
		SM_getUtUpgradeStatus(rMsg);
		break;	
	case M_EMS_BTS_UPGRADE_UT_RETRAN_REQ:
              SM_CpeUpgradeRetran(rMsg);
		break;
	 case DEL_INFO_TIMER:
	 	SM_delCpeUpdateInfo(rMsg);
	 	break;
	#ifdef WBBU_CODE 
	case M_EMS_BTS_RRU_SW_EXCHANGE_REQ:
		 T_WrruLoad *rHead;
		 rHead = (T_WrruLoad*)(rMsg.GetDataPtr());
		 if(rHead->LoadType == 1)//LOAD wrru mcu
		 {
		     if(Loading_Wrru_Code!=0)
		     	{
		     	      OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_REV_ERR_MSG, "l3oam file task Load Wrru last Time not End\n ");
		     	       SM_PostCommonRsp(M_TID_EMSAGENTTX,  rHead->TransId, M_BTS_EMS_RRU_SW_EXCHANGE_RSP, OAM_FAILURE);
		     	       break;
		     	}
		     else
		     	{
		      OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_REV_ERR_MSG, "l3oam file task Load Wrru MCU Code ");
		     LoadMcu();
		     	}
		 }
		 else if(rHead->LoadType == 2)// load wrru fpga
		 {
		       if(Loading_Wrru_Code!=0)
		     	{
		     	      OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_REV_ERR_MSG, "l3oam file task Load Wrru last Time not End\n ");
		     	       SM_PostCommonRsp(M_TID_EMSAGENTTX,  rHead->TransId, M_BTS_EMS_RRU_SW_EXCHANGE_RSP, OAM_FAILURE);
		     	       break;
		     	}
		       else
		       {
		     	 	OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_REV_ERR_MSG, "l3oam file task Load Wrru FPGACode");
		    		LoadFpga();
		       }
		 }
              SM_PostCommonRsp(M_TID_EMSAGENTTX,  rHead->TransId, M_BTS_EMS_RRU_SW_EXCHANGE_RSP, OAM_SUCCESS);
		break;
	#endif
        default:
            OAM_LOGSTR1(LOG_DEBUG, L3SM_ERROR_REV_ERR_MSG, "l3oam file task receive error msg = 0x%04x", MsgId);
            break;
    }
    
    return true;
}

TID CTaskFileMag :: GetEntityId() const
{
    return M_TID_SM;
}

CTaskFileMag* CTaskFileMag :: GetInstance()
{
    if ( NULL == m_Instance )
    {
        m_Instance = new CTaskFileMag;
    }

    return m_Instance;
}

void CTaskFileMag :: SetSysStatus(UINT32 Status )
{
    m_SysStatus = Status;
}

UINT32 CTaskFileMag :: GetSysStatus()
{
    return m_SysStatus;
}


UINT16 CTaskFileMag :: GetSysDLReqSeqNum()
{
    return m_SysDLReqSeqNum; 
}

void CTaskFileMag :: AddSysDLReqSeqNum()
{
    m_SysDLReqSeqNum++; 
}

bool CTaskFileMag :: SM_BtsUpgradeReq(CBtsSWUpdateReq &rMsg)
{
    SINT32  atastate = bspGetAtaState();
    if(1 != atastate)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, rMsg.GetTransactionId(), M_BTS_EMS_UPGRADE_BTS_SW_RSP, (UINT16)L3SM_ERROR_ATA_DEVICE_ERR);
        return true;
    }
    
    SM_PostCommonRsp(M_TID_EMSAGENTTX, rMsg.GetTransactionId(), M_BTS_EMS_UPGRADE_BTS_SW_RSP, OAM_SUCCESS);

    bspSetWorkFlag(LOAD_STATUS_TEST_RUN);   //设置试运行标志
    bspSetResetNum(0);   //设置试运行标志

    BOOT_PLANE plane = bspGetBootPlane();
    OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "l3oam file task bts current master boot plane[%d]!", plane);
    int Mplane; 
    if(BOOT_PLANE_B == plane)
    {    
        bspSetBootPlane(BOOT_PLANE_A);   
        Mplane = BOOT_PLANE_A;
        bspSetSMFlag(Mplane);
    }
    else
    {
        bspSetBootPlane(BOOT_PLANE_B);   
        Mplane = BOOT_PLANE_B;
        bspSetSMFlag(Mplane);
    }
    
    OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "l3oam file task bts set master boot plane[%d]!", Mplane);

    CBtsRstNotify NotifyMsg;
    NotifyMsg.CreateMessage(*this);
    NotifyMsg.SetDstTid(M_TID_SYS);
    NotifyMsg.SetSrcTid(M_TID_SM);
    NotifyMsg.SetTransactionId(OAM_DEFAUIT_TRANSID);
    NotifyMsg.SetResetReason(L3SYS_ERROR_BTS_RESET_CODE_UPDATE);
    NotifyMsg.SetMessageId(M_OAM_BTS_RESET_NOTIFY);
    if(true != NotifyMsg.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", NotifyMsg.GetMessageId());
        NotifyMsg.DeleteMessage();
    }

    OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "l3oam file task bts set bts_boot_data_source_ems!");
#ifdef WBBU_CODE
    bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
#endif
    return true;
}

//  1. Send response
//  2. Send message to taskFtpClient to dl the file
bool CTaskFileMag :: SM_FileLoadReq(CMessage &rMsg)
{
    CL3OamCommonReq ReqMsg(rMsg);

    UINT32 State = GetSysStatus(); 
    if(OAM_FILE_DL_SW == State)  // 若是下载状态，向ems返回系统忙消息
    {
//        if (M_EMS_BTS_DL_BTS_SW_REQ == rMsg.GetMessageId())
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             ReqMsg.GetTransactionId(), 
                             M_BTS_EMS_DL_BTS_SW_RSP, 
                             L3SM_ERROR_SYS_STATE_DL);
/*        else
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             ReqMsg.GetTransactionId(), 
                             M_BTS_EMS_DL_UT_SW_RSP, 
                             L3SM_ERROR_SYS_STATE_DL);
*/
        return true;
    }
    else //设置系统状态为下载软件状态 
    {
        SetSysStatus(OAM_FILE_DL_SW);
    }
    
    //BTS dl
    if (M_EMS_BTS_DL_BTS_SW_REQ == rMsg.GetMessageId())
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         ReqMsg.GetTransactionId(), 
                         M_BTS_EMS_DL_BTS_SW_RSP, 
                         OAM_SUCCESS);
        CBtsSWDLoadReq DLReqMsg(rMsg);
		
        CFileLoadNotify rNotify;  //向ftp client 任务发送文件下载指示消息 
        rNotify.CreateMessage(*this);
        rNotify.SetDstTid(M_TID_FTPCLIENT);
        rNotify.SetMessageId(M_EMS_BTS_DL_BTS_SW_REQ);
        rNotify.SetTransactionId(ReqMsg.GetTransactionId());
        rNotify.SetFtpServerIp(DLReqMsg.GetFtpServerIp());
        rNotify.SetFtpServerPort(DLReqMsg.GetFtpServerPort());
        
        SINT8 TempBuff[FILE_DIRECTORY_LEN];
        memset(TempBuff, 0, sizeof(TempBuff));
        DLReqMsg.GetUserName(TempBuff, DLReqMsg.GetUserNameLen());
        rNotify.SetUserName(TempBuff);
        
        memset(TempBuff, 0, sizeof(TempBuff));
        DLReqMsg.GetFtpPass(TempBuff, DLReqMsg.GetFtpPassLen());
        rNotify.SetFtpPass(TempBuff);
        
        memset(TempBuff, 0, sizeof(TempBuff));
        DLReqMsg.GetFtpDir(TempBuff, DLReqMsg.GetFtpDirLen());
        rNotify.SetFtpDir(TempBuff);
   
        memset(TempBuff, 0, sizeof(TempBuff));
        DLReqMsg.GetFileName(TempBuff);
        rNotify.SetFileName(TempBuff);
        if(true != rNotify.Post())
        {
            OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "L3OAM FILE TASK POST MSG[0X%04X] FAIL", rNotify.GetMessageId());
            rNotify.DeleteMessage();
        }
	 else
 	 {
   	     //启动定时器保护ftp任务jy081121
  	     CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
  	     taskSM->setFtpUsingFlag(M_TID_SM, TRUE);
 	 }
    }
#if 0
    else
    {
        //CPE Z SW
        CCpeSWDLoadReq DLReqMsg(rMsg);
        CFileLoadNotify rNotify;  //向ftp client 任务发送文件下载指示消息 
        rNotify.CreateMessage(*this);
        rNotify.SetDstTid(M_TID_FTPCLIENT);
        rNotify.SetMessageId(rMsg.GetMessageId());
        rNotify.SetTransactionId(ReqMsg.GetTransactionId());
        rNotify.SetFtpServerIp(DLReqMsg.GetFtpServerIp());
        rNotify.SetFtpServerPort(DLReqMsg.GetFtpServerPort());

        SINT8 TempBuff[FILE_DIRECTORY_LEN];
        memset(TempBuff, 0, sizeof(TempBuff));
        DLReqMsg.GetUserName(TempBuff, DLReqMsg.GetUserNameLen());
        rNotify.SetUserName(TempBuff);

        memset(TempBuff, 0, sizeof(TempBuff));
        DLReqMsg.GetFtpPass(TempBuff, DLReqMsg.GetFtpPassLen());
        rNotify.SetFtpPass(TempBuff);

        memset(TempBuff, 0, sizeof(TempBuff));
        DLReqMsg.GetFtpDir(TempBuff, DLReqMsg.GetFtpDirLen());
        rNotify.SetFtpDir(TempBuff);

#ifdef MZ_2ND
        if( M_EMS_BTS_DL_UT_SW_REQ == rMsg.GetMessageId() )//CPE & MZ
        {
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             ReqMsg.GetTransactionId(), 
                             M_BTS_EMS_DL_UT_SW_RSP, 
                             OAM_SUCCESS);

            memset(TempBuff, 0, sizeof(TempBuff));
            DLReqMsg.GetFileName(TempBuff);
            rNotify.SetFileName(TempBuff);
            if(true != rNotify.Post())
            {
                OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", rNotify.GetMessageId());
                rNotify.DeleteMessage();
            }
            else
            {
                //启动定时器保护ftp任务jy081121
                CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
                taskSM->setFtpUsingFlag(M_TID_SM, TRUE);
            }
        }  
#else
        if( M_EMS_BTS_DL_UT_SW_REQ == rMsg.GetMessageId() )//CPE SW
        {
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             ReqMsg.GetTransactionId(), 
                             M_BTS_EMS_DL_UT_SW_RSP, 
                             OAM_SUCCESS);

            memset(TempBuff, 0, sizeof(TempBuff));
            DLReqMsg.GetFileName(TempBuff);
            rNotify.SetFileName(TempBuff);
            if(true != rNotify.Post())
            {
                OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", rNotify.GetMessageId());
                rNotify.DeleteMessage();
            }
            else
            {
                //启动定时器保护ftp任务jy081121
                CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
                taskSM->setFtpUsingFlag(M_TID_SM, TRUE);
            }
        }  
        else if( M_EMS_BTS_DL_Z_SW_REQ == rMsg.GetMessageId() )//Z SW
        {
            CZSWDLoadReq Req(rMsg);
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             ReqMsg.GetTransactionId(), 
                             M_BTS_EMS_DL_Z_SW_RSP, 
                             OAM_SUCCESS);

            memset(TempBuff, 0, sizeof(TempBuff));
            Req.GetFileName(TempBuff);
            rNotify.SetFileName(TempBuff);
            if(true != rNotify.Post())
            {
                OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", rNotify.GetMessageId());
                rNotify.DeleteMessage();
            }
            else
            {
                //启动定时器保护ftp任务jy081121
                CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
                taskSM->setFtpUsingFlag(M_TID_SM, TRUE);
            }
        }
    }
#endif
#endif //0
    m_usDlType = rMsg.GetMessageId() + 1;
    m_usTransId = rMsg.GetTransactionId();    
    return true;
}

bool CTaskFileMag :: SM_FileLoadResultNotify(CL3OamCommonRsp& rMsg)    //处理ftp client 返回的文件下载通知
{
	m_usDlType = FTPCLIENT_SUCCESS_FLAG;
    if( m_pdlFtpCTimer )
        m_pdlFtpCTimer->Stop();
	SetSysStatus(OAM_FILE_IDLE);//保证下次下载可以正确进行jiaying20100906
	UINT16 ulRst = rMsg.GetResult();
#ifdef LJF_FILE_DL_RSP_TEST
	switch (ulRst)
	{
		case (0x26):
            OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "Get BTS DL 1st OK [%04X]!!!/n", ulRst );    
			break;
		case (0x27):
            OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "Get BTS DL 2nd OK [%04X]!!!/n", ulRst );    
			break;
		case (0x28):
            OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "Get CPE DL OK [%04X]!!!/n", ulRst );    
			break;
	}
#endif
	if( 0x26 == ulRst )
	{
        OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "Get BTS DL 1st OK [%04X] a !!!/n", ulRst );    
    	return true;
	}
	else
	{
	    CL3OamCommonRsp RspMsg; //向ems返回成功应答消息
	    RspMsg.CreateMessage(*this);
	    RspMsg.SetMessageId(rMsg.GetMessageId());
	    RspMsg.SetDstTid(M_TID_EMSAGENTTX);
	    RspMsg.SetTransactionId(rMsg.GetTransactionId());
		if( (0x27==ulRst) || (0x28==ulRst) )
	    	RspMsg.SetResult( 0 );
		else
		    RspMsg.SetResult( ulRst );
	    if(true != RspMsg.Post())
	    {
	        OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", RspMsg.GetMessageId());
	        RspMsg.DeleteMessage();
	    }
	    
    	return true;
	}
}
bool CTaskFileMag :: DealCpeUpdateReqNew( CUpdateCpeSWReqNew& rMsg, T_CpeUpgradeRecord& CpeInfo)
{
    if( HW_TYPE_ZBOX_7C == rMsg.GetHWtype() )
    {
        CpeInfo.bZFlag = true;
        CpeInfo.ulZBoxPID = rMsg.getPid();
        OAM_LOGSTR(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "DealCpeUpdateReqNew() this is ZBox req" );
    }
    else
        CpeInfo.bZFlag = false;  

    SINT8 FileName[FILE_NAME_LEN];
    memset(FileName, 0, FILE_NAME_LEN);
    rMsg.GetFileName(FileName,rMsg.GetFileNameLen());
    memset( CpeInfo.FileDirName, 0, FILE_DIRECTORY_LEN + FILE_NAME_LEN );
    strcat( CpeInfo.FileDirName, SM_CPE_BOOT_DIR );
    strcat( CpeInfo.FileDirName, FileName );

    CpeInfo.EmsTransId   = rMsg.GetTransactionId();
    CpeInfo.CPEID        = rMsg.GetCPEID();
    return true;
}

#if 0
bool CTaskFileMag :: DealCpeUpdateReq( CUpdateCpeSWReq& rMsg, T_CpeUpgradeRecord& CpeInfo)
{
    CpeInfo.bZFlag = false;  

    SINT8 FileName[FILE_NAME_LEN];
    memset(FileName, 0, FILE_NAME_LEN);
    rMsg.GetFileName(FileName);
    memset( CpeInfo.FileDirName, 0, FILE_DIRECTORY_LEN + FILE_NAME_LEN );
    strcat( CpeInfo.FileDirName, SM_CPE_BOOT_DIR );
    strcat( CpeInfo.FileDirName, FileName );

    CpeInfo.EmsTransId   = rMsg.GetTransactionId();
    CpeInfo.CPEID        = rMsg.GetCPEID();
	return true;
}
bool CTaskFileMag :: DealZUpdateReq( CUpdateZSWReq& rMsg, T_CpeUpgradeRecord& CpeInfo) 
{
    CpeInfo.bZFlag = true;

    SINT8 FileName[FILE_NAME_LEN];
    memset(FileName, 0, FILE_NAME_LEN);
    rMsg.GetFileName(FileName);
    memset( CpeInfo.FileDirName, 0, FILE_DIRECTORY_LEN + FILE_NAME_LEN );
    strcat( CpeInfo.FileDirName, SM_CPE_BOOT_DIR );
    strcat( CpeInfo.FileDirName, FileName );

    CpeInfo.EmsTransId   = rMsg.GetTransactionId();
    CpeInfo.CPEID        = rMsg.GetCPEID();
    CpeInfo.UID          = rMsg.GetUid();
    return true;
}

bool CTaskFileMag :: SM_CpeUpgradeReq(CMessage& rMsg)
{
    OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[CTaskFileMag::SM_CpeUpgradeReq] Get Reques of Upgrade Z /CPE", 0);
    //如果已经达到系统允许的最大个数或CPE正在升级,向ems返回应答消息表明升级失败
    T_CpeUpgradeRecord CpeInfo;
    memset(&CpeInfo, 0, sizeof(T_CpeUpgradeRecord));
    UINT16 transID;
    
    if( M_EMS_BTS_UPGRADE_UT_SW_REQ == rMsg.GetMessageId() )
    {
        CUpdateCpeSWReq rReq(rMsg);
        transID = rReq.GetTransactionId();
        if ( ! DealCpeUpdateReq( rReq, CpeInfo ) )
            return false;
    }
    else
    {
        CUpdateZSWReq rReq(rMsg);
        transID = rReq.GetTransactionId();
        if ( ! DealZUpdateReq( rReq, CpeInfo ) )
            return false;
    }
    
    FILE *pFile = fopen( CpeInfo.FileDirName, "rb" );  
    if(NULL == pFile)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         transID, 
                         (M_EMS_BTS_UPGRADE_Z_SW_REQ == rMsg.GetMessageId()) ? M_BTS_EMS_UPGRADE_Z_SW_RSP : M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                         L3SM_ERROR_OPEN_FILE);
        return true;        
    }

    UINT32 FileLen;
    FileLen = GetFileLength(pFile);
    fclose( pFile );
    
    CpeInfo.DLReqSeqNum  = GetSysDLReqSeqNum();
    CpeInfo.Progress     = 0;
    CpeInfo.FileSize     = FileLen;
    CpeInfo.RetranNum = 0;
    UINT32 LeftChar = FileLen % MAX_SWPACK_UC_SIZE;
    if(0 == LeftChar)
    {
        CpeInfo.TotalPackNum = (UINT16)(FileLen/MAX_SWPACK_UC_SIZE);
    }
    else
    {
        CpeInfo.TotalPackNum = (UINT16)(FileLen/MAX_SWPACK_UC_SIZE + 1);    
    }

    CpeInfo.PackSize     = MAX_SWPACK_UC_SIZE;
    CpeInfo.CurPackNum   = 0;     //真正的数据包号从 NO.1 开始 
    CpeInfo.FileCurPtr   = pFile;
    //添加定时器,如果超时则清除升级列表中该eid信息
    CComMessage* pMsgTimer = new ( this, 0 ) CComMessage;    
    if (pMsgTimer!=NULL)
    {
    	pMsgTimer->SetDstTid( M_TID_SM  );
    	pMsgTimer->SetSrcTid( M_TID_SM  );
    	pMsgTimer->SetMessageId( DEL_INFO_TIMER);
	pMsgTimer->SetEID(CpeInfo.CPEID);
    }
    CpeInfo.delInfoCTimer = new CTimer( false, 120000, pMsgTimer /**pMsgTimer */);
    if(CpeInfo.delInfoCTimer==NULL)
    {
        pMsgTimer->Destroy();        
    }   
    else if(true!=CpeInfo.delInfoCTimer->Start())
    {
        delete CpeInfo.delInfoCTimer;
        CpeInfo.delInfoCTimer=NULL;
    }
    UINT16 Result = AddCpeUpgradeRecord(CpeInfo); 

    SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                     transID, 
                     (M_EMS_BTS_UPGRADE_Z_SW_REQ == rMsg.GetMessageId()) ? M_BTS_EMS_UPGRADE_Z_SW_RSP : M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                     Result);
        
    if(OAM_SUCCESS == Result)
    {   
#ifndef MZ_2ND
        if( CpeInfo.bZFlag )//目标ID应该是tUTV
            SM_SendZSWDLReqToCpe(rMsg, FileLen);
        else//向CPE发送文件升级请求消息
#endif
            SM_SendUNICASTSWDLReqToCpe(rMsg, FileLen);
	//ccb中记录升级状态  
        CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
        pTaskCpeM->CPE_SetDlFlag(CpeInfo.CPEID, 1); 
    }
    else
    {
        CpeInfo.delInfoCTimer->Stop();
	 delete CpeInfo.delInfoCTimer;
	 CpeInfo.delInfoCTimer = NULL;
    }
    
    return true;
}

bool CTaskFileMag :: SM_SendUNICASTSWDLReqToCpe(CMessage &ReqMsg, UINT32 FileLen)
{
    CUpdateCpeSWReq rMsg(ReqMsg);
    CL3CpeSWDLReq SWDLReq;
    SWDLReq.CreateMessage(*this);
    
    SWDLReq.SetMessageId(M_L3_CPE_UPGRADE_SW_REQ);
    SWDLReq.SetDstTid(M_TID_CPESM);
    SWDLReq.SetDLType(SWDLTYPE_UNICAST);  //0 - unicast
    SWDLReq.SetEID( ((CUpdateCpeSWReq*)&ReqMsg)->GetCPEID() );
    SWDLReq.SetDLReqSeqNum(GetSysDLReqSeqNum());

    SINT8 VerInfo[FILE_NAME_LEN];
    memset(VerInfo, 0, sizeof(VerInfo));
    rMsg.GetSWVer(VerInfo);
    UINT32 FileVer = inet_addr(VerInfo);
    SWDLReq.SetFileVersion(FileVer);
    SWDLReq.SetInterfaceType(rMsg.GetModelType());
    SWDLReq.SetFileSize(FileLen);
    SWDLReq.SetDLPackSize(MAX_SWPACK_UC_SIZE);
    SWDLReq.SetHWverListNum(0);
    SINT8 HwVerInfo[MAX_HWVER_LIST_NUM * DEVICE_HW_VER_SIZE];
    memset(HwVerInfo, 0, sizeof(HwVerInfo));
    SWDLReq.SetHWverList(HwVerInfo, sizeof(HwVerInfo));
    
    //构造配置失败消息
    CL3OamUnicastUTSWReqFail FailMsg;
    FailMsg.CreateMessage(*this);
    FailMsg.SetDstTid(M_TID_SM);
    FailMsg.SetTransactionId(rMsg.GetTransactionId());
    FailMsg.SetCPEID(rMsg.GetCPEID());

    //创建配置数据transaction
    CTransaction* pCfgTransaction = CreateTransact(SWDLReq, 
                                    FailMsg, 
                                    OAM_REQ_RESEND_CNT3, 
                                    OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        SWDLReq.DeleteMessage();
        FailMsg.DeleteMessage();
        return false;
    } 
    SWDLReq.SetTransactionId(pCfgTransaction->GetId());
    FailMsg.SetTransactionId(rMsg.GetTransactionId());
    if(!pCfgTransaction->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pCfgTransaction->EndTransact();
	 delete pCfgTransaction;
	 return false;
    }
    return true;
}
bool CTaskFileMag :: SM_SendZSWDLReqToCpe(CMessage &ReqMsg, UINT32 FileLen)
{
    CUpdateZSWReq rMsg(ReqMsg);
    CL3ZSWDLReq SWDLReq;
    SWDLReq.CreateMessage(*this);
    
    SWDLReq.SetMessageId(M_EMS_BTS_UPGRADE_Z_SW_REQ);
    SWDLReq.SetDstTid(M_TID_UTV);
    SWDLReq.SetEID( rMsg.GetCPEID() );

    SINT8 ucStr[FILEVER_STR_LEN];
    rMsg.GetSWVer(ucStr);
    SWDLReq.SetFileVersion( inet_addr(ucStr) );
    SWDLReq.SetFileSize( FileLen );
    rMsg.GetZProductType(ucStr);
    SWDLReq.SetProductType( inet_addr(ucStr) );
    SWDLReq.SetUid( rMsg.GetUid());
    
    SWDLReq.SetDLReqSeqNum( GetSysDLReqSeqNum() );
    SWDLReq.SetDLPackSize( MAX_SWPACK_UC_SIZE );
    SWDLReq.SetFileType( 0x0 );
    SWDLReq.SetUpgradeFlag( rMsg.GetUpdateType() );
    
    //构造配置失败消息
    CL3OamUnicastUTSWReqFail FailMsg;
    FailMsg.CreateMessage(*this,M_OAM_UNICAST_ZSW_REQ_FAIL);
    FailMsg.SetDstTid(M_TID_SM);
    FailMsg.SetTransactionId(rMsg.GetTransactionId());
    FailMsg.SetCPEID(rMsg.GetCPEID());

    //创建配置数据transaction
    CTransaction* pCfgTransaction = CreateTransact(SWDLReq, 
                                    FailMsg, 
                                    OAM_REQ_RESEND_CNT3, 
                                    OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        SWDLReq.DeleteMessage();
        FailMsg.DeleteMessage();
        return false;
    } 
    SWDLReq.SetTransactionId(pCfgTransaction->GetId());
    FailMsg.SetTransactionId(rMsg.GetTransactionId());
    if(!pCfgTransaction->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pCfgTransaction->EndTransact();
        delete pCfgTransaction;
        return false;
    }

    return true;
}

#else 
bool CTaskFileMag :: SM_SendZSWDLReqToCpeNew(CMessage &ReqMsg, UINT32 FileLen)
{
    CUpdateCpeSWReqNew req(ReqMsg);
    UINT32 VerInfo = 0;
	UINT8 modelType = 0;
	SINT8 filename[FILE_NAME_LEN];
	UINT8 filenameLen = req.GetFileNameLen();
	UINT8 isComipApp = 0; //1 - comipapp
	req.GetFileName(filename, filenameLen);
    if(false == getHardwareInfo(filename, filenameLen, &modelType, &VerInfo, &isComipApp))
	{
		OAM_LOGSTR(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task getHardwareInfo fail");
		return false;
	}
		OAM_LOGSTR3(LOG_DEBUG, L3FM_ERROR_REV_MSG, "getHardwareInfo filename = %s ,modelType = 0x%X, isComipApp = 0x%X", (int)filename,modelType,isComipApp);

    CL3ZSWDLReq SWDLReq;
    SWDLReq.CreateMessage(*this);
    
    SWDLReq.SetMessageId( M_L3_CPE_UPGRADE_Z_SW_REQ );
    SWDLReq.SetDstTid(M_TID_UTV);
    SWDLReq.SetEID( req.GetCPEID() );

    SWDLReq.SetTransactionId( req.GetTransactionId() );
    SWDLReq.SetVersion( 0 );
    SWDLReq.SetFileVersion( VerInfo );
    SWDLReq.SetFileSize( FileLen );
    SWDLReq.SetProductType( 0 );
    SWDLReq.SetPid( req.getPid() );
    SWDLReq.SetDLReqSeqNum( GetSysDLReqSeqNum() );
    SWDLReq.SetDLPackSize( MAX_SWPACK_UC_SIZE );
    SWDLReq.SetFileType( 0x0 );
    SWDLReq.SetUpgradeFlag( 0x0000/*强制升级*/ );
//    SINT8 ucStr[FILEVER_STR_LEN];
//    rMsg.GetSWVer(ucStr);
//    SWDLReq.SetFileVersion( inet_addr(ucStr) );
//    rMsg.GetZProductType(ucStr);
//    SWDLReq.SetProductType( inet_addr(ucStr) );
//    SWDLReq.SetUid( rMsg.GetUid());
    
    
    //构造配置失败消息
    CL3OamUnicastUTSWReqFail FailMsg;
    FailMsg.CreateMessage(*this,M_OAM_UNICAST_UTSW_REQ_FAIL);
    FailMsg.SetDstTid(M_TID_SM);
    FailMsg.SetTransactionId(req.GetTransactionId());
    FailMsg.SetCPEID(req.GetCPEID());

    //创建配置数据transaction
    CTransaction* pCfgTransaction = CreateTransact(SWDLReq, 
                                    FailMsg, 
                                    OAM_REQ_RESEND_CNT3, 
                                    OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        SWDLReq.DeleteMessage();
        FailMsg.DeleteMessage();
        return false;
    } 
    SWDLReq.SetTransactionId(pCfgTransaction->GetId());
    FailMsg.SetTransactionId(req.GetTransactionId());
    if(!pCfgTransaction->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pCfgTransaction->EndTransact();
        delete pCfgTransaction;
        return false;
    }

    return true;
}

#endif
bool CTaskFileMag :: SM_CpeZUpgradeRsp(CL3OamCommonRspFromCpe &rMsg, UINT16 MsgId)
{
    UINT32 ulMsgEid = rMsg.GetEID();
    UINT16 usMsgTransID = rMsg.GetTransactionId();
    UINT16 usMsgResult = rMsg.GetResult();
    //获取升级记录信息
    list<T_CpeUpgradeRecord> :: iterator Iter;
    Iter = FindCpeUpgradeRecord( ulMsgEid );

    //停掉 transaction 
    CTransaction * pTransaction = FindTransact( usMsgTransID );
    if(pTransaction!=NULL)
    {
        pTransaction->EndTransact();
        delete pTransaction;
    }
    //CL3OamCommonRsp CommonRsp(rMsg);
    if(Iter==NULL)
    {
        OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_REV_DATA_SEQ_ERR, "SM_CpeZUpgradeRsp find eid failed, eid:%x", ulMsgEid );
    	return false;	
    }
    if(Iter->delInfoCTimer!=NULL)		
    {  
        Iter->delInfoCTimer->Stop();
        if(true!=Iter->delInfoCTimer->Start())
    	 {
            delete Iter->delInfoCTimer;
            Iter->delInfoCTimer=NULL;
    	 }
    }

#if 0
    CUpdateUTSWResultNotify ResultNotify;
    ResultNotify.CreateMessage(*this);
    ResultNotify.SetMessageId(M_BTS_EMS_UPGRADE_UT_SW_RSP/*M_BTS_EMS_UPGRADE_UT_SW_RSP*/);
    ResultNotify.SetDstTid(M_TID_EMSAGENTTX);
    ResultNotify.SetTransactionId(Iter->EmsTransId);
    if( M_CPE_L3_UPGRADE_Z_SW_RSP == MsgId )
        ResultNotify.SetCPEID(Iter->ulZBoxPID);
    else
        ResultNotify.SetCPEID(ulMsgEid);
    ResultNotify.SetResult(usMsgResult);
    if(true != ResultNotify.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", ResultNotify.GetMessageId());
        ResultNotify.DeleteMessage();
    }
#else
    SM_PostCommonRsp(M_TID_EMSAGENTTX, 
             Iter->EmsTransId, 
             M_BTS_EMS_UPGRADE_UT_SW_RSP, 
             usMsgResult );

#endif

    if(OAM_SUCCESS != usMsgResult )
    {
        OAM_LOGSTR2(LOG_SEVERE, L3SM_ERROR_REV_DATA_SEQ_ERR, "SM_CpeZUpgradeRsp get rsp eid:%08x, result[%04x]", ulMsgEid, usMsgResult );
        DelCpeUpgradeRrcord( ulMsgEid );
        return false;
    }
    //发送第一个数据包
    SINT8 Buff[MAX_SWPACK_UC_SIZE];
    memset(Buff, 0, MAX_SWPACK_UC_SIZE);
    FILE *pFile = fopen( Iter->FileDirName, "rb" );
    UINT16 ReadCnt = (UINT16)fread(Buff, 1, MAX_SWPACK_UC_SIZE, pFile);
    fclose( pFile );

    
    //构造配置失败消息
    CL3OamUnicastUTSWReqFail FailMsg;
    FailMsg.CreateMessage(*this,M_OAM_UNICAST_UTSW_REQ_FAIL);
    FailMsg.SetDstTid(M_TID_SM);
    FailMsg.SetTransactionId( usMsgTransID );
    FailMsg.SetCPEID( ulMsgEid );
    
    //向CPE发送软件数据包请求消息
    if( !Iter->bZFlag )
    {
        SendCpePack( FailMsg, Iter, Buff ,ReadCnt, usMsgTransID );
    }
    else
    {
        SendZPack( FailMsg, Iter, Buff ,ReadCnt, usMsgTransID );
    }

    //修改升级记录
    Iter->Progress   = (UINT8)(Iter->CurPackNum * 100/ Iter->TotalPackNum) ; 
    Iter->CurPackNum = Iter->CurPackNum + 1;

    OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_REV_DATA_SEQ_ERR, "l3oam file uc_pack_seq_num = 0001, eid:%x", ulMsgEid );


    //向EMS发送进度指示
    CUpdateCpeSWRateNotify Notify;
    Notify.CreateMessage(*this);
    Notify.SetMessageId(M_BTS_EMS_UPGRADE_UT_SW_PROGRESS);
    Notify.SetDstTid(M_TID_EMSAGENTTX);
    Notify.SetTransactionId(Iter->EmsTransId);
    Notify.SetCPEID( ulMsgEid );
    Notify.SetProgress(Iter->Progress);
    if(true!=Notify.Post())
    {
       Notify.DeleteMessage();
    }
    return true;
}


bool CTaskFileMag :: SendZPack( CMessage& rFailMsg, list<CTaskFileMag :: T_CpeUpgradeRecord> :: iterator pRecord, SINT8* Buff, UINT16 usLen, UINT16 usTransId )
{
    CL3ZSWUCDLPackReq SWDLPackReq; 
    SWDLPackReq.CreateMessage(*this);
    SWDLPackReq.SetDstTid(M_TID_UTV);
    SWDLPackReq.SetMessageId(M_L3_CPE_DL_Z_SW_PACK_REQ);
    SWDLPackReq.SetDLReqSeqNum(pRecord->DLReqSeqNum);
    SWDLPackReq.SetSWPackSeqNum(pRecord->CurPackNum + 1);
    SWDLPackReq.SetEID( pRecord->CPEID );
    SWDLPackReq.SetPID( pRecord->ulZBoxPID );
    SWDLPackReq.SetSWPackData(Buff, MAX_SWPACK_UC_SIZE);

    //创建配置数据transaction
    CTransaction* pCfgTransaction = CreateTransact( SWDLPackReq, 
                                                    rFailMsg, 
                                                    OAM_REQ_RESEND_CNT3, 
                                                    OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        SWDLPackReq.DeleteMessage();
        rFailMsg.DeleteMessage();
        return false;
    } 
    OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_REV_DATA_SEQ_ERR, "******ZZZ Send Pack Seq = [%04d]", pRecord->CurPackNum + 1 );

    SWDLPackReq.SetTransactionId(pCfgTransaction->GetId());
    rFailMsg.SetTransactionId(usTransId);
    if(!pCfgTransaction->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pCfgTransaction->EndTransact();
        delete pCfgTransaction;
        return false;
    }
    return true;
}
bool CTaskFileMag :: SendCpePack( CMessage& rFailMsg, list<CTaskFileMag :: T_CpeUpgradeRecord> :: iterator pRecord, SINT8* Buff, UINT16 usLen, UINT16 usTransId )
{
    CL3CpeSWUCDLPackReq SWDLPackReq; 
    SWDLPackReq.CreateMessage(*this);
    SWDLPackReq.SetDstTid(M_TID_CPESM);
    SWDLPackReq.SetMessageId(M_L3_CPE_UPGRADE_SW_PACK_REQ);
    SWDLPackReq.SetDLReqSeqNum(pRecord->DLReqSeqNum);
    SWDLPackReq.SetSWPackSeqNum(pRecord->CurPackNum + 1);
    SWDLPackReq.SetEID( pRecord->CPEID );
    SWDLPackReq.SetSWPackData(Buff, usLen);

    //创建配置数据transaction
    CTransaction* pCfgTransaction = CreateTransact( SWDLPackReq, 
                                                    rFailMsg, 
                                                    OAM_REQ_RESEND_CNT3, 
                                                    OAM_REQ_RSP_PERIOD);//OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )
    {            
        SWDLPackReq.DeleteMessage();
        rFailMsg.DeleteMessage();
        return false;
    }   
    SWDLPackReq.SetTransactionId(pCfgTransaction->GetId());
    rFailMsg.SetTransactionId(usTransId);
    if(!pCfgTransaction->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pCfgTransaction->EndTransact();
        delete pCfgTransaction;
        return false;
    }
    return true;
}

//if(feof(tlIter->FileCurPtr)){}  //文件结束不用判断
//因为没有文件下载结束指示消息,所以不必计算文件是否结束,下载结束
//由cpe自己根据接收到的数据(当前序号,总的数据包个数)自行判断.
bool CTaskFileMag :: SM_CpeZUpgradePackRsp(CL3CpeSWDLPackRsp &rMsg)
{
    int count;
    CTransaction * pTransaction = FindTransact(rMsg.GetTransactionId());
    if(pTransaction!=NULL)
    {    
        pTransaction->EndTransact();
        delete pTransaction;
    }    
    
    //获取升级记录信息
    list<T_CpeUpgradeRecord> :: iterator tlIter ;
    tlIter = FindCpeUpgradeRecord(rMsg.GetEID());
    if(tlIter==NULL)
        return false;
    if(tlIter->delInfoCTimer!=NULL)		
    {  
        tlIter->delInfoCTimer->Stop();
        if(true!=tlIter->delInfoCTimer->Start())
    	 {
             delete tlIter->delInfoCTimer;
	      tlIter->delInfoCTimer=NULL;
    	 }
    }
    CUpdateCpeSWRateNotify Notify;  //进度指示消息
    Notify.CreateMessage(*this);
    Notify.SetMessageId( /*(tlIter->bZFlag) ? M_BTS_EMS_UPGRADE_Z_SW_PROGRESS : */M_BTS_EMS_UPGRADE_UT_SW_PROGRESS );
    Notify.SetDstTid(M_TID_EMSAGENTTX);
    Notify.SetTransactionId(tlIter->EmsTransId);
    Notify.SetCPEID(rMsg.GetEID());

    OAM_LOGSTR3(LOG_DEBUG, L3SM_ERROR_REV_DATA_SEQ_ERR, "SM_CpeZUpgradePackRsp TotalPackNum[%02X],CurPackNum[%02x],MsgPackSeqNum[%02x]", 
        tlIter->TotalPackNum, tlIter->CurPackNum, rMsg.GetSWPackSeqNum() );
            
    if((rMsg.GetSWPackSeqNum() > tlIter->CurPackNum) ||\
		((rMsg.GetSWPackSeqNum() < tlIter->CurPackNum)&&(tlIter->RetranNum>5)))//失败,
    {
        DelCpeUpgradeRrcord(rMsg.GetEID());

        //向ems返回应答消息
        CL3OamCommonRsp CommonRsp(rMsg);
        CommonRsp.SetTransactionId(tlIter->EmsTransId);
        CommonRsp.SetMessageId( /*(tlIter->bZFlag) ? M_BTS_EMS_UPGRADE_Z_SW_RSP : */M_BTS_EMS_UPGRADE_UT_SW_RSP );
        CommonRsp.SetDstTid(M_TID_EMSAGENTTX);
        CommonRsp.SetResult(L3SM_ERROR_REV_DATA_SEQ_ERR);
        CommonRsp.Post();
        OAM_LOGSTR2(LOG_SEVERE, L3SM_ERROR_REV_DATA_SEQ_ERR, "l3oam file CPE_Z Pack DL ERR = %04d, eid: %x", rMsg.GetSWPackSeqNum(),rMsg.GetEID());
        return false;
    }
    else if(rMsg.GetSWPackSeqNum() < tlIter->CurPackNum)
    {
        tlIter->CurPackNum = rMsg.GetSWPackSeqNum();
        tlIter->RetranNum++;
    }
    else
    {
        if( rMsg.GetSWPackSeqNum() == tlIter->TotalPackNum )//升级完毕
        {
            //向EMS发送进度指示
            Notify.SetProgress(100);
            if(true!=Notify.Post())
        	{
        	    Notify.DeleteMessage();
        	}
	
	     //////失败没有处理
            OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_REV_DATA_SEQ_ERR, "l3oam file uc_dl_finish, eid: %x", rMsg.GetEID() );
            StartUpdateCTimer(rMsg.GetEID());
            return true;
        }
    }

    SINT8 Buff[MAX_SWPACK_UC_SIZE];
    memset(Buff, 0, MAX_SWPACK_UC_SIZE);
    FILE *pFile = fopen( tlIter->FileDirName, "rb" );
    fseek( pFile, tlIter->CurPackNum * MAX_SWPACK_UC_SIZE, SEEK_SET );
    UINT16 ReadCnt;
    if( tlIter->CurPackNum != tlIter->TotalPackNum - 1 )
        ReadCnt = (UINT16)fread(Buff, 1, MAX_SWPACK_UC_SIZE, pFile);
    else
    {
        if( 0 == tlIter->FileSize % MAX_SWPACK_UC_SIZE )
            ReadCnt = (UINT16)fread(Buff, 1, MAX_SWPACK_UC_SIZE, pFile);
        else
            ReadCnt = (UINT16)fread(Buff, 1, (tlIter->FileSize % MAX_SWPACK_UC_SIZE), pFile);
	    ReadCnt = MAX_SWPACK_UC_SIZE;
    }
    fclose( pFile );

    //构造配置失败消息
    CL3OamUnicastUTSWReqFail FailMsg;
    FailMsg.CreateMessage(*this,M_OAM_UNICAST_UTSW_REQ_FAIL);
    FailMsg.SetDstTid(M_TID_SM);
    FailMsg.SetTransactionId(rMsg.GetTransactionId());
    FailMsg.SetCPEID(rMsg.GetEID());
    
    //向CPE发送软件数据包请求消息
    if( !tlIter->bZFlag )
    {
        SendCpePack( FailMsg, tlIter, Buff ,ReadCnt, rMsg.GetTransactionId() );
    }
    else
    {
        SendZPack( FailMsg, tlIter, Buff ,ReadCnt, rMsg.GetTransactionId() );
    }

    //修改升级记录
    UINT16 usTmp = tlIter->CurPackNum * 100;
    tlIter->Progress   = usTmp / tlIter->TotalPackNum ;
    tlIter->CurPackNum = tlIter->CurPackNum + 1;
    tlIter->FileCurPtr = tlIter->FileCurPtr + ReadCnt;

    count = tlIter->TotalPackNum/5;
    if(tlIter->CurPackNum%count==0)
    {
         //向EMS发送进度指示
         Notify.SetProgress(tlIter->Progress);
         if(true!=Notify.Post())
         {
            Notify.DeleteMessage();
         }
    }
   else
   	{
   	    Notify.DeleteMessage();
   	}
    return true;
}

//所有单播过程中超时失败消息都有此函数处理
bool CTaskFileMag :: SM_CpeUpgradeReqFail(CL3OamUnicastUTSWReqFail &rMsg)
{
    UINT32 eid;
    UINT16 transId = rMsg.GetTransactionId();
    if(transId!=0)
    {
        CTransaction * pTransaction = FindTransact(transId);
        if(pTransaction)
        {
            pTransaction->EndTransact();
            delete pTransaction;
        }		    
    }
    eid = rMsg.GetCPEID();
    //获取升级记录信息
    list<T_CpeUpgradeRecord> :: iterator tlIter ;
    tlIter = FindCpeUpgradeRecord(eid);
    BOOL bZBoxUpdateFlag = false;
    if(tlIter!=NULL)
    {
        if( 0 == tlIter->CurPackNum )
        {//包序号为0，是软件升级请求超时，发送响应，否则发送通知
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
            tlIter->EmsTransId, 
            M_BTS_EMS_UPGRADE_UT_SW_RSP, 
            L3SM_ERROR_UPDATE_TIMEOUT );

        }
        else
        {
            bZBoxUpdateFlag = tlIter->bZFlag;
            //向EMS发送升级结果指示消息
            CUpdateUTSWResultNotify ResultNotify;
            ResultNotify.CreateMessage(*this);
            ResultNotify.SetMessageId(M_BTS_EMS_UPGRADE_UT_SW_NOTIFY);
            ResultNotify.SetDstTid(M_TID_EMSAGENTTX);
            ResultNotify.SetTransactionId(tlIter->EmsTransId);
    		if( bZBoxUpdateFlag )
                ResultNotify.SetCPEID(tlIter->ulZBoxPID);
            else
                ResultNotify.SetCPEID(eid);
            ResultNotify.SetResult(L3SM_ERROR_UPDATE_TIMEOUT);
           // ResultNotify.Post();
            if(true != ResultNotify.Post())
            {
                OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", ResultNotify.GetMessageId());
                ResultNotify.DeleteMessage();
            }
        }
        DelCpeUpgradeRrcord(eid); 
    }

    if( ! bZBoxUpdateFlag )
    {
        //ccb中记录升级状态
        CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
        pTaskCpeM->CPE_SetDlFlag(eid, 0); 
    }
    return true;
}

bool CTaskFileMag :: SM_CpeUpgradeNotify(CUpdateUTSWResultNotify &rMsg)
{
   if( m_pUpdateCTimer )
        m_pUpdateCTimer->Stop();
   
    //获取升级记录信息
    list<T_CpeUpgradeRecord> :: iterator tlIter ;
    tlIter = FindCpeUpgradeRecord(rMsg.GetEID());
    BOOL bZBoxUpdateFlag = false;
    if(tlIter!=NULL)
    {
        bZBoxUpdateFlag = tlIter->bZFlag;
	    CUpdateUTSWResultNotify emsMsg; //向ems返回成功应答消息
	    emsMsg.CreateMessage(*this);
	    emsMsg.SetMessageId( M_BTS_EMS_UPGRADE_UT_SW_NOTIFY );
	    emsMsg.SetDstTid(M_TID_EMSAGENTTX);
        emsMsg.SetTransactionId( tlIter->EmsTransId );
        emsMsg.SetCPEID( rMsg.GetCPEID() );
        emsMsg.SetResult( rMsg.GetResult() );
        
        OAM_LOGSTR3(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3oam get notify trans[%04X] eid/z[%08X] result[%04X]", 
            rMsg.GetTransactionId(), rMsg.GetCPEID(), rMsg.GetResult() );
        //ResultNotify.Post();
        if(true != emsMsg.Post())
        {
            OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", rMsg.GetMessageId());
            emsMsg.DeleteMessage();
        }
        DelCpeUpgradeRrcord(rMsg.GetEID());
    }
    else
    {
        OAM_LOGSTR3(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3oam get notify trans[%04X] eid/z[%08X] result[%04X] find rcd fail", 
            rMsg.GetTransactionId(), rMsg.GetCPEID(), rMsg.GetResult() );
    }
    if( ! bZBoxUpdateFlag )
    {//ccb中记录升级状态
        CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
        pTaskCpeM->CPE_SetDlFlag(rMsg.GetEID(), 0); 
    }
    return true;
}

#if 0
UINT8 CTaskFileMag :: SM_GetBCArrayIndex(UINT8 hwtype)
{
    UINT8 flagZBOX    = (hwtype & 0x80)>>7;
    UINT8 flagRsv     = (hwtype & 0x60)>>5;
    UINT8 flagRDA     = (hwtype & 0x10)>>4;
    UINT8 flagHwType  = (hwtype & 0x0f);
    UINT8 index = 0;
    /****************************************/
    /* 1 - Maxim Handset
       2 - maxim PCMCIA
       3 - maxim 1Mhz CPE
       4 - maxim 5Mhz SuperCPE
       5 - RDA Handset
       6 - RDA PCMCIA
       7 - RDA 1M CPE
       8 - RDA 5M SuperCPE
       9 - maxim 1Mhz CPE_Z
       10- maxim 5Mhz CPE_Z
       11- RDA 1Mhz CPE_Z
       12- RDA 5Mhz CPE_Z
                                            */
    /****************************************/
    if((UTHW_TYPE_NZBOX == flagZBOX) &&
       (UTHW_TYPE_MAX   == flagRDA) &&
       (0               == flagRsv))
    {
        index = flagHwType;
    }
    else if((UTHW_TYPE_NZBOX == flagZBOX) &&
            (UTHW_TYPE_RDA   == flagRDA))
    {
        index = flagHwType+4;
    }
    else if((UTHW_TYPE_ZBOX == flagZBOX) &&
            (UTHW_TYPE_RDA  == flagRDA))
    {
        if(UTHW_TYPE_CPE_1M == flagHwType)
        {
            index = 11;
        }
        else if(UTHW_TYPE_CPE_5M == flagHwType)
        {
            index = 12;
        }
    }
    else if((UTHW_TYPE_ZBOX == flagZBOX) &&
            (UTHW_TYPE_MAX  == flagRDA))
    {
        if(UTHW_TYPE_CPE_1M == flagHwType)
        {
            index = 9;
        }
        else if(UTHW_TYPE_CPE_5M == flagHwType)
        {
            index = 10;
        }     
    }

//test start
//printf("\r\n SM_GetBCArrayIndex:  hwtype = 0x%x <--> %d = index\r\n",hwtype,index);
//test stop
    return index;
}

bool CTaskFileMag :: SM_CpeBCUpgradeReq(CBCUTSWReq &rMsg)
{
    SINT8 FileName[FILE_NAME_LEN];
    memset(FileName, 0, FILE_NAME_LEN);
    rMsg.GetFileName(FileName);
    
    SINT8 FileDirName[FILE_DIRECTORY_LEN + FILE_NAME_LEN];
    memset(FileDirName, 0, sizeof(FileDirName));
    strcat(FileDirName, SM_CPE_BOOT_DIR);
    strcat(FileDirName, FileName);
    FILE *pFile = fopen(FileDirName, "rb");  

    if(NULL == pFile)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rMsg.GetTransactionId(), 
                         M_BTS_EMS_BC_UPGRADE_UT_SW_RSP, 
                         L3SM_ERROR_OPEN_FILE);
        return true;        
    }
    
    UINT32 FileLen;
    FileLen = GetFileLength(pFile);
    UINT16 HwType = rMsg.GetModelType();
    fclose( pFile );

    //获取版本信息
    SINT8 VerInfo[FILE_NAME_LEN];
    memset(VerInfo, 0, sizeof(VerInfo));
    rMsg.GetSWVer(VerInfo);
    UINT32 FileVer = inet_addr(VerInfo);

    UINT8 flagZBOX    = (HwType & 0x80)>>7;
    UINT8 flagRsv     = (HwType & 0x60)>>5;
    UINT8 flagRDA     = (HwType & 0x10)>>4;
    UINT8 flagHwType  = (HwType & 0x0f);
    SINT8 hardware[50];
    memset(hardware, 0, sizeof(hardware));
    switch(flagHwType)
    {
    case UTHW_TYPE_HANDSET:
        strcpy(hardware, "handset");
        break;
    case UTHW_TYPE_PCMCIA:
        strcpy(hardware, "pcmcia");
        break;  
    case UTHW_TYPE_CPE_1M:
        strcpy(hardware, "cpe(1m)");
        break;  
    case UTHW_TYPE_CPE_5M:
        strcpy(hardware, "cpe(5m)");
        break;     
    }
    if(UTHW_TYPE_RDA == flagRDA)
    {
        strcat(hardware, " RDA");
    }
    else
    {
        strcat(hardware, " MAX");
    }
    if(UTHW_TYPE_ZBOX == flagZBOX)
    {
        strcat(hardware, " with ZBOX");
    }

	OAM_LOGSTR1(LOG_CRITICAL, L3FM_ERROR_REV_MSG, " broadcast software of %s", (int)hardware);
    //printf("\r\n broadcast software of %s\r\n",hardware);


    UINT8 bcindex = SM_GetBCArrayIndex(HwType);
    if(0 == bcindex)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rMsg.GetTransactionId(), 
                         M_BTS_EMS_BC_UPGRADE_UT_SW_RSP, 
                         L3SM_ERROR_CPE_HW_TYPE);
        return true;        
    }

    if((0 == flagRsv) &&
       ((UTHW_TYPE_HANDSET == flagHwType) ||
        (UTHW_TYPE_PCMCIA  == flagHwType) ||
        (UTHW_TYPE_CPE_1M  == flagHwType) ||
        (UTHW_TYPE_CPE_5M  == flagHwType)))
        
    {
        //此处应该增加RetryTimes的限制1-3
        if((0 >= rMsg.GetRetryTimes()) || 
           (MAX_REQ_SEND_CNT < rMsg.GetRetryTimes()))
        {
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             rMsg.GetTransactionId(), 
                             M_BTS_EMS_BC_UPGRADE_UT_SW_RSP, 
                             L3SM_ERROR_RETRY_TIMES);
            return true;         
        }    
        m_BCRcdA[bcindex].EmsTransId  = rMsg.GetTransactionId();
        m_BCRcdA[bcindex].SendReqCnt  = 1;
        m_BCRcdA[bcindex].CurRetryTimes = 0;
        m_BCRcdA[bcindex].DLReqSeqNum = GetSysDLReqSeqNum();
        m_BCRcdA[bcindex].FileSize    = FileLen;
        m_BCRcdA[bcindex].SWVer       = FileVer;
        m_BCRcdA[bcindex].InterType   = HwType;
        UINT32  LeftChar = FileLen % MAX_SWPACK_BC_SIZE;
        if(0 == LeftChar)
        {
            m_BCRcdA[bcindex].TotalPackNum = FileLen/MAX_SWPACK_BC_SIZE;
        }
        else
        {
            m_BCRcdA[bcindex].TotalPackNum = FileLen/MAX_SWPACK_BC_SIZE + 1;    
        }

        m_BCRcdA[bcindex].PackSize     = MAX_SWPACK_BC_SIZE;
        m_BCRcdA[bcindex].CurPackNum   = 0;     //真正的数据包号从 NO.1 开始 
        //m_BCRcdA[HwType].FileCurPtr   = pFile;
        memcpy( m_BCRcdA[bcindex].chFileName, FileDirName, FILE_DIRECTORY_LEN + FILE_NAME_LEN);
        m_BCRcdA[bcindex].Progress     = 0;
        m_BCRcdA[bcindex].RetryTimes   = rMsg.GetRetryTimes();
    }
    else
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rMsg.GetTransactionId(), 
                         M_BTS_EMS_BC_UPGRADE_UT_SW_RSP, 
                         L3SM_ERROR_CPE_HW_TYPE);
        return true;
    }

    bool IsNeedBC = true;  
    
    if(NULL == m_BCRcdA[bcindex].pBcSwTimer)
    {
        CL3BCUTSWTimer BCUTSWTimerMsg;
        BCUTSWTimerMsg.CreateMessage(*this);
        BCUTSWTimerMsg.SetDstTid(M_TID_SM);
        BCUTSWTimerMsg.SetTransactionId(OAM_TIMEOUT_ERR);
        BCUTSWTimerMsg.SetHWType(HwType);
        m_BCRcdA[bcindex].pBcSwTimer = new CTimer(M_TIMER_PERIOD_YES, BC_UT_DATAPACK_PERIOD, BCUTSWTimerMsg);
        m_BCRcdA[bcindex].pBcSwTimer->Start();
    }
    else
    {
        IsNeedBC = false;
    }
    
    if(false == IsNeedBC)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                 rMsg.GetTransactionId(), 
                 M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                 L3SM_ERROR_CPE_BC_UPDATING);
        return true;
    }
    //向ems返回成功应答消息
    SM_PostCommonRsp(M_TID_EMSAGENTTX, 
             rMsg.GetTransactionId(), 
             M_BTS_EMS_UPGRADE_UT_SW_RSP, 
             OAM_SUCCESS);


    //向CPE发送文件升级请求消息
    SM_SendBCSWDLReqToCpe(m_BCRcdA[bcindex]);

    //向EMS发送进度指示,因为是还未进行传送数据,所以进度值为零 
    CBCUTSWRateNotify RateNotify;
    RateNotify.CreateMessage(*this);
    RateNotify.SetMessageId(M_BTS_EMS_BC_UPGRADE_UT_SW_PROGRESS);
    RateNotify.SetDstTid(M_TID_EMSAGENTTX);
    RateNotify.SetTransactionId(rMsg.GetTransactionId());
    RateNotify.SetCpeHWType(rMsg.GetModelType());
    
    RateNotify.SetProgress(0);

    if(true != RateNotify.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", RateNotify.GetMessageId());
        RateNotify.DeleteMessage();
    }
    
    
    return true;
}


bool CTaskFileMag :: SM_SendBCSWDLReqToCpe(T_BCUGRcd &RcdInfo)
{
    CL3CpeSWDLReq SWDLReq;
    SWDLReq.CreateMessage(*this);
    
    SWDLReq.SetMessageId(M_L3_CPE_UPGRADE_SW_REQ);
    SWDLReq.SetDstTid(M_TID_CPESM);
    SWDLReq.SetDLType(SWDLTYPE_BROADCAST);  //0 - unicast
    SWDLReq.SetEID( 0xFFFFFFFE );
    SWDLReq.SetDLReqSeqNum(GetSysDLReqSeqNum());
    SWDLReq.SetFileVersion(RcdInfo.SWVer);
    SWDLReq.SetInterfaceType(RcdInfo.InterType);
    SWDLReq.SetFileSize(RcdInfo.FileSize);
    SWDLReq.SetDLPackSize(MAX_SWPACK_BC_SIZE);
    SWDLReq.SetHWverListNum(0);
    SINT8 HwVerInfo[MAX_HWVER_LIST_NUM * DEVICE_HW_VER_SIZE];
    memset(HwVerInfo, 0, sizeof(HwVerInfo));
    SWDLReq.SetHWverList(HwVerInfo, sizeof(HwVerInfo));
    if(true != SWDLReq.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", SWDLReq.GetMessageId());
        SWDLReq.DeleteMessage();
    }
    return true;
}

bool CTaskFileMag :: SM_CpeBCUpgradeTimer(CL3BCUTSWTimer & TimerMsg)     
{
    UINT16 HwType = TimerMsg.GetHWType();
    UINT8 bcindex = SM_GetBCArrayIndex(HwType);
    if(0 == bcindex)
    {
        return true;     
    }
    if( m_bBCReqFlag )
    {
        m_bBCReqFlag = false;
        SM_SendBCSWDLReqToCpe(m_BCRcdA[bcindex]);
        return true;        
    }
    
    SINT8 Buff[MAX_SWPACK_BC_SIZE];
    memset(Buff, 0, MAX_SWPACK_BC_SIZE);
    FILE *pFile = fopen( m_BCRcdA[bcindex].chFileName, "rb" );
    if( NULL == pFile )
    	return true;
    UINT16 ReadCnt;
    fseek( pFile, m_BCRcdA[bcindex].CurPackNum * MAX_SWPACK_BC_SIZE, SEEK_SET );
    if( m_BCRcdA[bcindex].CurPackNum != m_BCRcdA[bcindex].TotalPackNum - 1 )
        ReadCnt = (UINT16)fread(Buff, 1, MAX_SWPACK_BC_SIZE, pFile);
    else
        ReadCnt = (UINT16)fread(Buff, 1, (m_BCRcdA[bcindex].FileSize % MAX_SWPACK_BC_SIZE), pFile);
    fclose( pFile );

    //向CPE发送软件数据包请求消息
    CL3CpeSWBCDLPackReq SWDLPackReq; 
    SWDLPackReq.CreateMessage(*this);
    SWDLPackReq.SetMessageId(M_L3_CPE_UPGRADE_SW_PACK_REQ);
    SWDLPackReq.SetDstTid(M_TID_CPESM);
    SWDLPackReq.SetEID( 0xFFFFFFFE );
    SWDLPackReq.SetDLReqSeqNum(m_BCRcdA[bcindex].DLReqSeqNum);
    SWDLPackReq.SetSWPackSeqNum((UINT16)m_BCRcdA[bcindex].CurPackNum + 1);
    SWDLPackReq.SetSWPackData(Buff, MAX_SWPACK_BC_SIZE);
    if(true != SWDLPackReq.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", SWDLPackReq.GetMessageId());
        SWDLPackReq.DeleteMessage();
    }

    //修改升级记录
    m_BCRcdA[bcindex].CurPackNum = m_BCRcdA[bcindex].CurPackNum + 1;
    m_BCRcdA[bcindex].FileCurPtr = m_BCRcdA[bcindex].FileCurPtr + ReadCnt;

    //恢复出progress
    UINT16 progress = m_BCRcdA[bcindex].Progress;
    //printf("\r\n process = %d\r\n",progress);
    //升级完一遍,向ems返回进度通知消息
    CBCUTSWRateNotify RateNotify;
    RateNotify.CreateMessage(*this);
    RateNotify.SetMessageId(M_BTS_EMS_BC_UPGRADE_UT_SW_PROGRESS);
    RateNotify.SetDstTid(M_TID_EMSAGENTTX);
    RateNotify.SetTransactionId(m_BCRcdA[bcindex].EmsTransId);
    RateNotify.SetCpeHWType(HwType);
    UINT16 x1 = m_BCRcdA[bcindex].CurPackNum;
    UINT16 x3 = m_BCRcdA[bcindex].TotalPackNum*
                m_BCRcdA[bcindex].RetryTimes;
    UINT8 rate = 0;
        rate = progress + x1*100/x3;
        
    
    RateNotify.SetProgress(rate);
        if(true != RateNotify.Post())
        {
            OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", RateNotify.GetMessageId());
            RateNotify.DeleteMessage();
        }

    //判断是否文件下载结束     
    if(m_BCRcdA[bcindex].CurPackNum == m_BCRcdA[bcindex].TotalPackNum) 
    {
        m_BCRcdA[bcindex].CurRetryTimes = m_BCRcdA[bcindex].CurRetryTimes + 1;                  
        m_BCRcdA[bcindex].CurPackNum = 0;
        m_BCRcdA[bcindex].Progress = rate;    
  
        //判断是否升级结束
        if(m_BCRcdA[bcindex].CurRetryTimes >= m_BCRcdA[bcindex].RetryTimes)
        {
            //通知刷新进度条进度
            CBCUTSWRateNotify RateNotify;
            RateNotify.CreateMessage(*this);
            RateNotify.SetMessageId(M_BTS_EMS_BC_UPGRADE_UT_SW_PROGRESS);
            RateNotify.SetDstTid(M_TID_EMSAGENTTX);
            RateNotify.SetTransactionId(m_BCRcdA[bcindex].EmsTransId);
            RateNotify.SetCpeHWType(HwType);
            RateNotify.SetProgress(100);
            if(true != RateNotify.Post())
            {
                OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", RateNotify.GetMessageId());
                RateNotify.DeleteMessage();
            }

            //升级结束,向后台返回升级成功通知消息
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                     m_BCRcdA[bcindex].EmsTransId, 
                     M_BTS_EMS_BC_UPGRADE_UT_SW_NOTIFY, 
                     OAM_SUCCESS);
            
            if( NULL != m_BCRcdA[bcindex].pBcSwTimer ) 
            {  
                m_BCRcdA[bcindex].pBcSwTimer->Stop();
                delete m_BCRcdA[bcindex].pBcSwTimer;
                m_BCRcdA[bcindex].pBcSwTimer = NULL;
                memset((UINT8*)&(m_BCRcdA[bcindex].chFileName),0,sizeof(T_BCUGRcd));
            }
        }
        else //继续升级过程
            m_bBCReqFlag = true;
    }
    
    return true;
}
#endif //0


//////////////////////////////////////////////////////
UINT16 CTaskFileMag :: AddCpeUpgradeRecord(T_CpeUpgradeRecord & CpeInfo)
{
    list<T_CpeUpgradeRecord> :: iterator tlIter ;

    if(m_CurUpdateCpeNum >= MAX_UPDATE_CPE_NUM)
    {
        OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_REV_DATA_SEQ_ERR, "l3oam file uc_req max_num", 0 );
        return (UINT16)L3SM_ERROR_MAX_CPE_NUM; // 表示已经达到系统极限，不能接收请求
        
    }
    
    for(tlIter = m_CpeUpgradeList.begin(); tlIter != m_CpeUpgradeList.end(); tlIter ++ )
    {
        if(tlIter->CPEID == CpeInfo.CPEID)
        {
            OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_REV_DATA_SEQ_ERR, "l3oam file uc_req updating", 0 );
            return (UINT16)L3SM_ERROR_CPE_UPDATING; // 表示此CPE正在升级中，拒绝再次升级
            
        }
    }

    if(tlIter == m_CpeUpgradeList.end())
    {
        m_CpeUpgradeList.push_back(CpeInfo);// 表示此CPE未在升级中，可以升级
        m_CurUpdateCpeNum++;
        return OAM_SUCCESS;
    }

     return OAM_SUCCESS;
}

bool CTaskFileMag :: DelCpeUpgradeRrcord( UINT32 Cpeid)
{
    list<T_CpeUpgradeRecord> :: iterator tlIter ;
    for(tlIter = m_CpeUpgradeList.begin(); tlIter != m_CpeUpgradeList.end(); tlIter ++ )
    {
        if(tlIter->CPEID == Cpeid)
        {
            //停止用来删除cpeinfo的定时器
            if(tlIter->delInfoCTimer!=NULL)
            {
                 tlIter->delInfoCTimer->Stop();
	          delete tlIter->delInfoCTimer;
	          tlIter->delInfoCTimer = NULL;
    	     }
            m_CpeUpgradeList.erase(tlIter);
	     if(m_CurUpdateCpeNum>0)
	     	{
            m_CurUpdateCpeNum--;
	     	}
            return true;
        }
    }

    return false;
}

list<CTaskFileMag :: T_CpeUpgradeRecord> :: iterator CTaskFileMag :: FindCpeUpgradeRecord(UINT32 Cpeid)  //此处CpeInfo是输出参数 
{
    list<T_CpeUpgradeRecord> :: iterator tlIter ;
    for(tlIter = m_CpeUpgradeList.begin(); tlIter != m_CpeUpgradeList.end(); tlIter ++ )
    {
        if(tlIter->CPEID == Cpeid)
        {
            return tlIter;
        }
    }

    return NULL;
}

SINT32 CTaskFileMag :: GetFileLength(FILE* pFile)
{
    SINT32 filelength;

    fseek(pFile, 0, SEEK_END);
    filelength = ftell(pFile);

    
    return filelength;
}


void  CTaskFileMag :: SM_PostCommonRsp(TID tid, UINT16 transid, UINT16 msgid, UINT16 result)
{
    CL3OamCommonRsp CommonRsp;
    CommonRsp.CreateMessage(*this);
    CommonRsp.SetDstTid(tid);
    CommonRsp.SetTransactionId(transid);
    CommonRsp.SetMessageId(msgid);
    CommonRsp.SetResult(result);
    if(true != CommonRsp.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", CommonRsp.GetMessageId());
        CommonRsp.DeleteMessage();
    }
}


UINT32 bspGetActiveVersion()
{
    if(BOOT_PLANE_A == bspGetBootPlane())
    {    
        return bspGetLoadVersion_A();
    }
    else
    {    
        return bspGetLoadVersion_B();
    }
}

UINT32 bspGetStandbyVersion()
{
    if(BOOT_PLANE_A != bspGetBootPlane())
    {    
        return bspGetLoadVersion_A();
    }
    else
    {    
        return bspGetLoadVersion_B();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//test start
//  1. Send response
//  2. Send message to taskFtpClient to dl the file
void CTaskFileMag :: showDL()
{
	for(int i=0; i<UT_HWTYPE_CNT; i++)
	{
		if(m_BCRcdA[i].FileSize != 0)
		{
			printf("\r\n *******************************************************************************************************");
			printf("\r\n chFileName = %s, \r\n",m_BCRcdA[i].chFileName);
			printf("\r\n EmsTransId = 0x%x, \r\n",m_BCRcdA[i].EmsTransId);
			printf("\r\n SendReqCnt = %d, \r\n",m_BCRcdA[i].SendReqCnt);
			printf("\r\n DLReqSeqNum = 0x%x, \r\n",m_BCRcdA[i].DLReqSeqNum);
			printf("\r\n FileSize = 0x%x, \r\n",m_BCRcdA[i].FileSize);
			printf("\r\n SWVer = 0x%x, \r\n",m_BCRcdA[i].SWVer);
			printf("\r\n InterType = 0x%x, \r\n",m_BCRcdA[i].InterType);			
			printf("\r\n TotalPackNum = 0x%x, \r\n",m_BCRcdA[i].TotalPackNum);
			printf("\r\n PackSize = 0x%x, \r\n",m_BCRcdA[i].PackSize);
			printf("\r\n CurPackNum = 0x%x, \r\n",m_BCRcdA[i].CurPackNum);
			printf("\r\n FileCurPtr = 0x%x, \r\n",(UINT8*)(m_BCRcdA[i].FileCurPtr));
			printf("\r\n Progress = 0x%x, \r\n",m_BCRcdA[i].Progress);
			printf("\r\n CurRetryTimes = %d, \r\n",m_BCRcdA[i].CurRetryTimes);			
		}
			
	}
}
extern "C" STATUS showdl()
{
    CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
    taskSM->showDL();
	return OK;
}
//终止软件下载
bool stopFlag = false;
extern "C" STATUS stopdl()
{
	stopFlag = true;
	return OK;
}

bool CTaskFileMag :: SM_FileLoadReqNew(CMessage &rMsg)
{
	if(true == stopFlag)
	{
		m_SysStatus = OAM_FILE_IDLE;
		stopFlag = false;
		return true;
	}
    CL3OamCommonReq ReqMsg(rMsg);

    UINT32 State = GetSysStatus(); 
    if(OAM_FILE_DL_SW == State)  // 若是下载状态，向ems返回系统忙消息
    {
        if (M_EMS_BTS_DL_UT_SW_REQ_NEW != rMsg.GetMessageId())
    	{
    		OAM_LOGSTR1(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task receive error msg(0x%x) from EMS !!!", rMsg.GetMessageId());
    		return false;
    	}

        else
        {
            OAM_LOGSTR(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task is in downloading status,can't serve for a new request!!!");
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             ReqMsg.GetTransactionId(), 
                             M_BTS_EMS_DL_UT_SW_RSP, 
                             L3SM_ERROR_SYS_STATE_DL);
        }
        return true;
    }
    else //设置系统状态为下载软件状态 
    {
        SetSysStatus(OAM_FILE_DL_SW);
    }
	
    CCpeSWDLoadReqNew DLReqMsg(rMsg);
    CFileLoadNotify rNotify;  //向ftp client 任务发送文件下载指示消息 
    rNotify.CreateMessage(*this);
    rNotify.SetDstTid(M_TID_FTPCLIENT);
    rNotify.SetMessageId(rMsg.GetMessageId());
    rNotify.SetTransactionId(ReqMsg.GetTransactionId());
    rNotify.SetFtpServerIp(DLReqMsg.GetFtpServerIp());
    rNotify.SetFtpServerPort(DLReqMsg.GetFtpServerPort());

    SINT8 TempBuff[FILE_DIRECTORY_LEN];
    memset(TempBuff, 0, sizeof(TempBuff));
    DLReqMsg.GetUserName(TempBuff, DLReqMsg.GetUserNameLen());
    rNotify.SetUserName(TempBuff);

    memset(TempBuff, 0, sizeof(TempBuff));
    DLReqMsg.GetFtpPass(TempBuff, DLReqMsg.GetFtpPassLen());
    rNotify.SetFtpPass(TempBuff);

    memset(TempBuff, 0, sizeof(TempBuff));
    DLReqMsg.GetFtpDir(TempBuff, DLReqMsg.GetFtpDirLen());
	
#ifdef _SMDEBUG
	printf("\r\n GetFtpDir.TempBuff  =  %s\r\n",TempBuff);
#endif

    rNotify.SetFtpDir(TempBuff);
	
    if( M_EMS_BTS_DL_UT_SW_REQ_NEW == rMsg.GetMessageId() )//CPE SW NEW
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         ReqMsg.GetTransactionId(), 
                         M_BTS_EMS_DL_UT_SW_RSP, 
                         OAM_SUCCESS);

        memset(TempBuff, 0, sizeof(TempBuff));
        DLReqMsg.GetFileName(TempBuff, DLReqMsg.GetFileNameLen());
        rNotify.SetFileName(TempBuff);

        if(true != rNotify.Post())
        {
            OAM_LOGSTR1(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", rNotify.GetMessageId());
            rNotify.DeleteMessage();
        }
        else
        {
            //启动定时器保护ftp任务jy081121
            CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
            taskSM->setFtpUsingFlag(M_TID_SM, TRUE);
 	     }
    }  
    m_usDlType = M_BTS_EMS_DL_UT_SW_RSP;
    m_usTransId = rMsg.GetTransactionId();    
    return true;
}

bool CTaskFileMag :: SM_CpeUpgradeReqNew(CMessage& rMsg)
{
	if(true == stopFlag)
	{
		m_SysStatus = OAM_FILE_IDLE;
		stopFlag = false;
		return true;
	}

    //如果已经达到系统允许的最大个数或CPE正在升级,向ems返回应答消息表明升级失败
    T_CpeUpgradeRecord CpeInfo;
    memset(&CpeInfo, 0, sizeof(T_CpeUpgradeRecord));

    CUpdateCpeSWReqNew rReq(rMsg);
    /*if(( M_EMS_BTS_UPGRADE_UT_SW_REQ_NEW == rMsg.GetMessageId() )||\
		( M_EMS_BTS_UPGRADE_UT_BTLDER_REQ_NEW == rMsg.GetMessageId() ))
    {
        if ( ! DealCpeUpdateReqNew( rReq, CpeInfo ) )
    	{
    		OAM_LOGSTR1(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "l3oam file task DealCpeUpdateReqNew fail,eid:%x", rReq.GetCPEID());
			return false;
    	}
    }*/
    DealCpeUpdateReqNew( rReq, CpeInfo );
    FILE *pFile = fopen( CpeInfo.FileDirName, "rb" );  
    if(NULL == pFile)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rReq.GetTransactionId(), 
                         M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                         L3SM_ERROR_OPEN_FILE);
        OAM_LOGSTR2(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "l3oam file task open file[%s] fail! eid[%04x]", (int)(CpeInfo.FileDirName), CpeInfo.CPEID);
        return true;        
    }

    UINT32 FileLen;
    FileLen = GetFileLength(pFile);
    fclose( pFile );
    CpeInfo.DLReqSeqNum  = GetSysDLReqSeqNum();
    CpeInfo.Progress     = 0;
    CpeInfo.FileSize     = FileLen;
    CpeInfo.RetranNum = 0;
    CpeInfo.TotalPackNum = (UINT16)( (FileLen+MAX_SWPACK_UC_SIZE-1) / MAX_SWPACK_UC_SIZE );
    CpeInfo.PackSize     = MAX_SWPACK_UC_SIZE;
    CpeInfo.CurPackNum   = 0;     //真正的数据包号从 NO.1 开始 
    //添加定时器,如果超时则清除升级列表中该eid信息
    CComMessage* pMsgTimer = new ( this, 0 ) CComMessage;    
    if (pMsgTimer!=NULL)
    {
    	pMsgTimer->SetDstTid( M_TID_SM  );
    	pMsgTimer->SetSrcTid( M_TID_SM  );
    	pMsgTimer->SetMessageId( DEL_INFO_TIMER);
    	pMsgTimer->SetEID(rReq.GetCPEID());
    }
    else
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                     rReq.GetTransactionId(), 
                     M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                     L3SM_ERROR_BTS_DEAL_ERR );
        OAM_LOGSTR(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "SM_CpeUpgradeReqNew create pMsgTimer fail!");
        return false;
    }
    CpeInfo.delInfoCTimer = new CTimer( false, 120000,  pMsgTimer );
    if(CpeInfo.delInfoCTimer==NULL)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                     rReq.GetTransactionId(), 
                     M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                     L3SM_ERROR_BTS_DEAL_ERR );
        OAM_LOGSTR(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "SM_CpeUpgradeReqNew create pMsgTimer fail!");
        pMsgTimer->Destroy();        
        OAM_LOGSTR(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "SM_CpeUpgradeReqNew create delInfoCTimer fail!");
        return false;
    }   
    else if(true!=CpeInfo.delInfoCTimer->Start())
    {
        pMsgTimer->Destroy();        
        delete CpeInfo.delInfoCTimer;
        CpeInfo.delInfoCTimer = NULL;
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                     rReq.GetTransactionId(), 
                     M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                     L3SM_ERROR_BTS_DEAL_ERR );
        OAM_LOGSTR(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "SM_CpeUpgradeReqNew start pMsgTimer fail!");
        return false;
    }
    UINT16 Result = AddCpeUpgradeRecord(CpeInfo); 
    if(OAM_SUCCESS == Result)
    {   
        if( CpeInfo.bZFlag )
        {
            if( ! SM_SendZSWDLReqToCpeNew(rMsg,FileLen) )
            {
                CpeInfo.delInfoCTimer->Stop();
                pMsgTimer->Destroy();        
                delete CpeInfo.delInfoCTimer;
                CpeInfo.delInfoCTimer = NULL;
                SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             rReq.GetTransactionId(), 
                             M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                             L3SM_ERROR_BTS_DEAL_ERR );
                DelCpeUpgradeRrcord( CpeInfo.CPEID );
                OAM_LOGSTR(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "SM_CpeUpgradeReqNew SM_SendZSWDLReqToCpeNew fail!");
            }
        }
        else
        {
            if( ! SM_SendUNICASTSWDLReqToCpeNew(rMsg, FileLen) )
            {
                CpeInfo.delInfoCTimer->Stop();
                pMsgTimer->Destroy();        
                delete CpeInfo.delInfoCTimer;
                CpeInfo.delInfoCTimer = NULL;
                SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                             rReq.GetTransactionId(), 
                             M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                             L3SM_ERROR_BTS_DEAL_ERR );
                DelCpeUpgradeRrcord( CpeInfo.CPEID );
                OAM_LOGSTR(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "SM_CpeUpgradeReqNew SM_SendUNICASTSWDLReqToCpeNew fail!");
            }
            //ccb中记录升级状态
            CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
            pTaskCpeM->CPE_SetDlFlag(rReq.GetCPEID(), 1); 
        }
    }
    else
    {
        CpeInfo.delInfoCTimer->Stop();
        delete CpeInfo.delInfoCTimer;
        CpeInfo.delInfoCTimer = NULL;
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                     rReq.GetTransactionId(), 
                     M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                     Result);
        OAM_LOGSTR(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "SM_CpeUpgradeReqNew AddCpeUpgradeRecord fail!");
        return false;
    }
   
    return true;
}

#if 0
bool CTaskFileMag :: SM_CpeBCUpgradeReqNew(CBCUTSWReqNew &rMsg)
{
	if(true == stopFlag)
	{
		m_SysStatus = OAM_FILE_IDLE;
		stopFlag = false;
		return true;
	}
	
//printf("\r\n <1>\r\n");
    SINT8 FileName[FILE_NAME_LEN];
    memset(FileName, 0, FILE_NAME_LEN);
    rMsg.GetFileName(FileName,rMsg.GetFileNameLen());
//printf("\r\n <2>\r\n"); 
#ifdef _SMDEBUG
	printf("\r\nFileNameLen = %d\r\n",rMsg.GetFileNameLen());
#endif
    SINT8 FileDirName[FILE_DIRECTORY_LEN + FILE_NAME_LEN];
    memset(FileDirName, 0, sizeof(FileDirName));
    strcat(FileDirName, SM_CPE_BOOT_DIR);
    strcat(FileDirName, FileName);
//printf("\r\n <3>\r\n");	
#ifdef _SMDEBUG
	printf("\r\n FileDirName = %s\r\n",FileDirName);
#endif
    FILE *pFile = fopen(FileDirName, "rb");  
//printf("\r\n <4>\r\n");	  
    if(NULL == pFile)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rMsg.GetTransactionId(), 
                         M_BTS_EMS_BC_UPGRADE_UT_SW_RSP, 
                         L3SM_ERROR_OPEN_FILE);
//test start
		OAM_LOGSTR1(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "L3SM_ERROR_OPEN_FILE: FileDirName = %s ", (int)FileDirName);
		//printf("\r\n L3SM_ERROR_OPEN_FILE: FileDirName = %s \r\n",FileDirName);
//test stop
        return true;        
    }
//printf("\r\n <5>\r\n");	   
    UINT32 FileLen;
    FileLen = GetFileLength(pFile);
    fclose( pFile );


    //SINT8 VerInfo[FILE_NAME_LEN];
    //memset(VerInfo, 0, sizeof(VerInfo));
    UINT32 VerInfo = 0;
	UINT8 modelType = 0;
//	SINT8 filename[FILE_NAME_LEN];
//	UINT8 filenameLen = rMsg.GetFileNameLen();
//	rMsg.GetFileName(filename, filenameLen);
//printf("\r\n <2>\r\n");	
//printf("\r\nFileName = [%s]\r\n",FileName);
    UINT8 isComipApp = 0;
    if(false == getHardwareInfo(FileName, (UINT8)rMsg.GetFileNameLen(), &modelType, &VerInfo, &isComipApp))
	{
		OAM_LOGSTR(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task getHardwareInfo fail");
		return false;
	}
//test start	
	OAM_LOGSTR3(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "getHardwareInfo: filename = %s ,modelType = 0x%X, isComipApp = 0x%X",(int)FileName,modelType,isComipApp);
		//printf("\r\n getHardwareInfo filename = %s ,modelType = 0x%X, VerInfo = %s, isComipApp = 0x%X\r\n", FileName,modelType,VerInfo,isComipApp);
//test stop 	

    //获取版本信息
//    SINT8 VerInfo[FILE_NAME_LEN];
//    memset(VerInfo, 0, sizeof(VerInfo));
	
   // UINT32 FileVer = inet_addr(VerInfo);

//    SINT8 hardware[50];
//    memset(hardware, 0, sizeof(hardware));
//    printf("\r\n broadcast software of %s\r\n",hardware);
//printf("\r\n <6>\r\n");	
    //UINT8 bcindex = SM_GetBCArrayIndex(modelType);
    UINT8 bcindex = modelType;
/*    if(0 == bcindex)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rMsg.GetTransactionId(), 
                         M_BTS_EMS_BC_UPGRADE_UT_SW_RSP, 
                         L3SM_ERROR_CPE_HW_TYPE);
        return true;        
    }
*/
//printf("\r\n <7>\r\n");	

    //此处应该增加RetryTimes的限制1-3
    if((0 >= rMsg.GetRetryTimes()) || 
       (MAX_REQ_SEND_CNT < rMsg.GetRetryTimes()))
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rMsg.GetTransactionId(), 
                         M_BTS_EMS_BC_UPGRADE_UT_SW_RSP, 
                         L3SM_ERROR_RETRY_TIMES);
        return true;         
    }   
//printf("\r\n <8>\r\n");		
    m_BCRcdA[bcindex].EmsTransId  = rMsg.GetTransactionId();
    m_BCRcdA[bcindex].SendReqCnt  = 1;
    m_BCRcdA[bcindex].CurRetryTimes = 0;
    m_BCRcdA[bcindex].DLReqSeqNum = GetSysDLReqSeqNum();
    m_BCRcdA[bcindex].FileSize    = FileLen;
    m_BCRcdA[bcindex].SWVer       = VerInfo;	
    m_BCRcdA[bcindex].InterType   = modelType;
    UINT32  LeftChar = FileLen % MAX_SWPACK_BC_SIZE;
    if(0 == LeftChar)
    {
//printf("\r\n <9>\r\n");	    
        m_BCRcdA[bcindex].TotalPackNum = FileLen/MAX_SWPACK_BC_SIZE;
    }
    else
    {
//printf("\r\n <10>\r\n");	    
        m_BCRcdA[bcindex].TotalPackNum = FileLen/MAX_SWPACK_BC_SIZE + 1;    
    }

    m_BCRcdA[bcindex].PackSize     = MAX_SWPACK_BC_SIZE;
    m_BCRcdA[bcindex].CurPackNum   = 0;     //真正的数据包号从 NO.1 开始 
    //m_BCRcdA[bcindex].FileCurPtr   = pFile;
    memcpy( m_BCRcdA[bcindex].chFileName, FileDirName, FILE_DIRECTORY_LEN + FILE_NAME_LEN);
    m_BCRcdA[bcindex].Progress     = 0;
    m_BCRcdA[bcindex].RetryTimes   = rMsg.GetRetryTimes();

    bool IsNeedBC = true;  
//printf("\r\n <11>\r\n");	 
    if(NULL == m_BCRcdA[bcindex].pBcSwTimer)
    {
//printf("\r\n <12>\r\n");    
        CL3BCUTSWTimerNew BCUTSWTimerMsg;
        BCUTSWTimerMsg.CreateMessage(*this);
        BCUTSWTimerMsg.SetDstTid(M_TID_SM);
        BCUTSWTimerMsg.SetTransactionId(OAM_TIMEOUT_ERR);
        BCUTSWTimerMsg.SetHWType(modelType);
        m_BCRcdA[bcindex].pBcSwTimer = new CTimer(M_TIMER_PERIOD_YES, BC_UT_DATAPACK_PERIOD, BCUTSWTimerMsg);
        m_BCRcdA[bcindex].pBcSwTimer->Start();
    }
    else
    {
//printf("\r\n <13>\r\n");     
        IsNeedBC = false;
    }
    
    if(false == IsNeedBC)
    {
//printf("\r\n <14>\r\n");      
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                 rMsg.GetTransactionId(), 
                 M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                 L3SM_ERROR_CPE_BC_UPDATING);
//test start
//printf("\r\n L3SM_ERROR_CPE_BC_UPDATING :modelType = 0x%x , bcindex = %d \r\n",modelType, bcindex);
		OAM_LOGSTR2(LOG_CRITICAL, L3SM_ERROR_REV_MSG, " L3SM_ERROR_CPE_BC_UPDATING :modelType = 0x%x , bcindex = %d ", modelType, bcindex);
//test stop

        return true;
    }
    //向ems返回成功应答消息
    SM_PostCommonRsp(M_TID_EMSAGENTTX, 
             rMsg.GetTransactionId(), 
             M_BTS_EMS_UPGRADE_UT_SW_RSP, 
             OAM_SUCCESS);


    //向CPE发送文件升级请求消息
    SM_SendBCSWDLReqToCpe(m_BCRcdA[bcindex]);

    //向EMS发送进度指示,因为是还未进行传送数据,所以进度值为零 
    CBCUTSWRateNotify RateNotify;
    RateNotify.CreateMessage(*this);
    RateNotify.SetMessageId(M_BTS_EMS_BC_UPGRADE_UT_SW_PROGRESS);
    RateNotify.SetDstTid(M_TID_EMSAGENTTX);
    RateNotify.SetTransactionId(rMsg.GetTransactionId());
    RateNotify.SetCpeHWType(modelType);
    
    RateNotify.SetProgress(0);

//test start
//printf("\r\n RateNotify post Rate(2) = %d \r\n",0);
//test stop
    if(true != RateNotify.Post())
    {
//test start
//printf("\r\n RateNotify post failed Rate(2) = %d \r\n",0);
//test stop    
        OAM_LOGSTR1(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", RateNotify.GetMessageId());
        RateNotify.DeleteMessage();
    }
    
    
    return true;
}
//test stop
#endif //0

bool CTaskFileMag :: SM_SendUNICASTSWDLReqToCpeNew(CMessage &ReqMsg, UINT32 FileLen)
{
	if(true == stopFlag)
	{
		m_SysStatus = OAM_FILE_IDLE;
		stopFlag = false;
		return true;
	}

    CUpdateCpeSWReqNew rMsg(ReqMsg);
    CL3CpeSWDLReq SWDLReq;

    SWDLReq.CreateMessage(*this);
    
    SWDLReq.SetMessageId(M_L3_CPE_UPGRADE_SW_REQ);
    SWDLReq.SetDstTid(M_TID_CPESM);
    if( M_EMS_BTS_UPGRADE_UT_SW_REQ_NEW == ReqMsg.GetMessageId() )
        SWDLReq.SetDLType(SWDLTYPE_UNICAST);  //0 - unicast
    else if( M_EMS_BTS_UPGRADE_UT_BTLDER_REQ_NEW == ReqMsg.GetMessageId() )
    	SWDLReq.SetDLType(SWDLTYPE_BOOTLOADER); 
    SWDLReq.SetEID( ((CUpdateCpeSWReqNew*)&ReqMsg)->GetCPEID() );
    SWDLReq.SetDLReqSeqNum(GetSysDLReqSeqNum());

    UINT32 VerInfo = 0;
	UINT8 modelType = 0;
	SINT8 filename[FILE_NAME_LEN];
	UINT8 filenameLen = rMsg.GetFileNameLen();
	UINT8 isComipApp = 0; //1 - comipapp
	rMsg.GetFileName(filename, filenameLen);
    if(false == getHardwareInfo(filename, filenameLen, &modelType, &VerInfo, &isComipApp))
	{
		OAM_LOGSTR(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task getHardwareInfo fail");
		return false;
	}
		OAM_LOGSTR3(LOG_DEBUG, L3FM_ERROR_REV_MSG, "getHardwareInfo filename = %s ,modelType = 0x%X, isComipApp = 0x%X", (int)filename,modelType,isComipApp);

    SWDLReq.SetFileVersion(VerInfo);
    SWDLReq.SetInterfaceType(modelType);
    SWDLReq.SetIsComipApp(isComipApp);
    SWDLReq.SetFileSize(FileLen);
    SWDLReq.SetDLPackSize(MAX_SWPACK_UC_SIZE);
    SWDLReq.SetHWverListNum(0);
    SINT8 HwVerInfo[MAX_HWVER_LIST_NUM * DEVICE_HW_VER_SIZE];
    memset(HwVerInfo, 0, sizeof(HwVerInfo));
    SWDLReq.SetHWverList(HwVerInfo, sizeof(HwVerInfo));

    //构造配置失败消息
    CL3OamUnicastUTSWReqFail FailMsg;
    FailMsg.CreateMessage(*this);
    FailMsg.SetDstTid(M_TID_SM);
    FailMsg.SetTransactionId(rMsg.GetTransactionId());
    FailMsg.SetCPEID(rMsg.GetCPEID());

    //创建配置数据transaction
    CTransaction* pCfgTransaction = CreateTransact(SWDLReq, 
                                    FailMsg, 
                                    OAM_REQ_RESEND_CNT3, 
                                    OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )
    {            
        SWDLReq.DeleteMessage();
        FailMsg.DeleteMessage();
        return false;
    }   

    SWDLReq.SetTransactionId(pCfgTransaction->GetId());
    FailMsg.SetTransactionId(rMsg.GetTransactionId());
    if(!pCfgTransaction->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pCfgTransaction->EndTransact();
        delete pCfgTransaction;
        return false;
    }

    return true;
}

bool CTaskFileMag :: getModelTypeByFilename(SINT8* filename, UINT8* modelType)
{
	char tmp[FILE_NAME_LEN];
    char* ptr = NULL;
	ptr = filename;
	int cnt = 0,i=0;
    while(*(ptr) != '\0')
	{
		if(*ptr == '.')
		{
			cnt++;
		}
		tmp[i++] = *ptr;
		ptr++;
		if(2 == cnt)
		{
			tmp[i] = '\0';
			break;
		}
	}
	
	printf("\r\n getModelTypeByFilename tmp : %s\r\n",tmp);

	if(0 == strcmp(tmp, "hs.om."))//1
		*modelType = 0x1;
	else if(0 == strcmp(tmp, "pcmcia.om."))//2
		*modelType = 0x2;
	else if(0 == strcmp(tmp, "cpe.om."))//3
		*modelType =  0x3;
	else if(0 == strcmp(tmp, "cpe.bm."))//4
		*modelType = 0x4;
	else if(0 == strcmp(tmp, "hs.or."))//5
		*modelType = 0x11;
	else if(0 == strcmp(tmp, "pcmcia.or."))//6
		*modelType = 0x12;
	else if(0 == strcmp(tmp, "cpe.or."))//7
		*modelType = 0x13;
	else if(0 == strcmp(tmp, "cpe.br."))//8
		*modelType = 0x14;
	else if(0 == strcmp(tmp, "cpez.om."))//9
		*modelType = 0x83;
	else if(0 == strcmp(tmp, "cpez.bm."))//10
		*modelType = 0x84;
	else if(0 == strcmp(tmp, "cpez.or."))//11
		*modelType = 0x93;
	else if(0 == strcmp(tmp, "cpez.br."))//12
		*modelType = 0x94;
	else
		return false;
	return true;
}

UINT16 SWAP16( UINT16* uValue )
{
	if( NULL == uValue )
		return 0;
	UINT8 uTmp = * (UINT8*)uValue;
	* (UINT8*)uValue = * ((UINT8*)uValue + 1);
	* ((UINT8*)uValue + 1) = uTmp;
	return *uValue;
}
bool CTaskFileMag :: getHardwareInfo(SINT8* filename, UINT8 len, UINT8* modelType, UINT32* VerInfo, UINT8* isComipApp)
{
    DIR* pdir = opendir( SM_CPE_BOOT_DIR );
    if( NULL == pdir )
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFile] opendir( /ata0a/cpe/ ) error");
		return false;
    }
    else
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFile] opendir( /ata0a/cpe/ ) success");
        if(OK != closedir( pdir ))
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFile] closedir() error");
			return false;
		}
    }
    
    if(OK != chdir( SM_CPE_BOOT_DIR ))
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFile] chdir( /ata0a/cpe/ ) error");
		return false;
	}

    OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFile] chdir( /ata0a/cpe/ ) success");


	FILE* dstFile = 0;
	char fileName[FILE_NAME_LEN];
	memset(fileName,0,sizeof(fileName));
	
	strncpy(fileName,filename,len);
    dstFile = fopen(fileName, "rb");
    if(NULL == dstFile)
    {
    	OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "open file [%s] failed!!!\r\n",(int)fileName);
		fclose(dstFile); 
		return false;
    }
	SINT8 buff[512];
	memset(buff,0,sizeof(buff));
	UINT16 readCount = (UINT16)fread(buff, 1,sizeof(CPE_BIN_HEADER), dstFile);
	CPE_BIN_HEADER* pHeader = (CPE_BIN_HEADER*)buff;
	*modelType = (pHeader->HardwareType)>>8;
	//如果HwType=0,根据文件名发送类型码
	if(0 == *modelType) 
	{
		if(!getModelTypeByFilename(fileName,modelType))
		{
			OAM_LOGSTR(LOG_CRITICAL, L3SM_ERROR_PRINT_SM_INFO, "bin file is very old ,hardware type is 0x0, getModelTypeByFilename failed!!!\r\n");
			fclose(dstFile); 
			return false;
		}
		OAM_LOGSTR1(LOG_CRITICAL, L3SM_ERROR_PRINT_SM_INFO, "bin file is very old ,hardware type is 0x0, change it to 0x%X\r\n",*modelType);
	}	
	memcpy((void*)VerInfo,(void*)pHeader,sizeof(UINT32));
	SWAP16((UINT16*)VerInfo);
	SWAP16(((UINT16*)VerInfo+1));
	//根据SetType类型判断是否为ComipApp数据包
	*isComipApp = ((pHeader->SetType) == SETTYPE_COMIPAPP) ? 1 : 0;
    OAM_LOGSTR2(LOG_DEBUG3, L3SM_ERROR_PRINT_SM_INFO, "[tFile] getHardwareInfo() pHeader->SetType[%08x] isComipApp[%1d]", pHeader->SetType, *isComipApp);
	fclose(dstFile); 
	return true;
	
}
#if 0
bool CTaskFileMag :: SM_CpeBCUpgradeTimerNew(CL3BCUTSWTimerNew & TimerMsg)     
{

    UINT16 HwType = TimerMsg.GetHWType();
    UINT8 bcindex = HwType;

if(stopFlag == true)
{
   m_BCRcdA[bcindex].pBcSwTimer->Stop();
   delete m_BCRcdA[bcindex].pBcSwTimer;
   memset((UINT8*)&(m_BCRcdA[bcindex].chFileName[0]),0,sizeof(T_BCUGRcd));
   m_BCRcdA[bcindex].pBcSwTimer = NULL;
   stopFlag = false;
   return true;
}

//printf("\r\n <11>\r\n");
    if( m_bBCReqFlag )
    {
//printf("\r\n <12>\r\n");    
        m_bBCReqFlag = false;
        SM_SendBCSWDLReqToCpe(m_BCRcdA[bcindex]);
        return true;        
    }
    
    SINT8 Buff[MAX_SWPACK_BC_SIZE];
    memset(Buff, 0, MAX_SWPACK_BC_SIZE);
    FILE *pFile = fopen( m_BCRcdA[bcindex].chFileName, "rb" );
    UINT16 ReadCnt;
    if( NULL == pFile )
    	return false;
    fseek( pFile, m_BCRcdA[bcindex].CurPackNum * MAX_SWPACK_BC_SIZE, SEEK_SET );
    if( m_BCRcdA[bcindex].CurPackNum != m_BCRcdA[bcindex].TotalPackNum - 1 )
        ReadCnt = (UINT16)fread(Buff, 1, MAX_SWPACK_BC_SIZE, pFile);
    else
        ReadCnt = (UINT16)fread(Buff, 1, (m_BCRcdA[bcindex].FileSize % MAX_SWPACK_BC_SIZE), pFile);
    fclose( pFile );
//printf("\r\n <13>\r\n");  
    //向CPE发送软件数据包请求消息
    CL3CpeSWBCDLPackReq SWDLPackReq; 
    SWDLPackReq.CreateMessage(*this);
    SWDLPackReq.SetMessageId(M_L3_CPE_UPGRADE_SW_PACK_REQ);
    SWDLPackReq.SetDstTid(M_TID_CPESM);
    SWDLPackReq.SetEID( 0xFFFFFFFE );
    SWDLPackReq.SetDLReqSeqNum(m_BCRcdA[bcindex].DLReqSeqNum);
    SWDLPackReq.SetSWPackSeqNum((UINT16)m_BCRcdA[bcindex].CurPackNum + 1);
    SWDLPackReq.SetSWPackData(Buff, MAX_SWPACK_BC_SIZE);

//test start
	OAM_LOGSTR3(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "broadcasting software [0x%x] ,seqnum = %4d:%4d ", (int)HwType,m_BCRcdA[bcindex].CurPackNum + 1,m_BCRcdA[bcindex].TotalPackNum);
//printf("\r\n broadcasting software [0x%x] ,seqnum = %4d:%4d \n",HwType,m_BCRcdA[bcindex].CurPackNum + 1,m_BCRcdA[bcindex].TotalPackNum); 
//test stop
 
    if(true != SWDLPackReq.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3SM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", SWDLPackReq.GetMessageId());
        SWDLPackReq.DeleteMessage();
    }

    //修改升级记录
    m_BCRcdA[bcindex].CurPackNum = m_BCRcdA[bcindex].CurPackNum + 1;
    m_BCRcdA[bcindex].FileCurPtr = m_BCRcdA[bcindex].FileCurPtr + ReadCnt;

    //恢复出progress
    UINT16 progress = m_BCRcdA[bcindex].Progress;
    //printf("\r\n process = %d\r\n",progress);
    //升级完一遍,向ems返回进度通知消息
    CBCUTSWRateNotify RateNotify;
    RateNotify.CreateMessage(*this);
    RateNotify.SetMessageId(M_BTS_EMS_BC_UPGRADE_UT_SW_PROGRESS);
    RateNotify.SetDstTid(M_TID_EMSAGENTTX);
    RateNotify.SetTransactionId(m_BCRcdA[bcindex].EmsTransId);
    RateNotify.SetCpeHWType(HwType);

    UINT16 x1 = m_BCRcdA[bcindex].CurPackNum;
    UINT16 x3 = m_BCRcdA[bcindex].TotalPackNum*
                m_BCRcdA[bcindex].RetryTimes;
    UINT8 rate = 0;
    rate = progress + x1*100/x3;
    RateNotify.SetProgress(rate);
//test start
#ifdef _SMDEBUG
	int aaa=0;
	if(rate >=100)
	{
	    for(aaa=0;aaa<10;aaa++)
	    {
	        printf("\r\n @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\r\n");
	    }
	}
	printf("\r\n RateNotify post Rate(1) = %d \r\n",rate);
#endif	

//test stop        
        if(true != RateNotify.Post())
        {
//test start
//printf("\r\n RateNotify post failed Rate(1) = %d \r\n",rate);
//test stop
            OAM_LOGSTR2(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail,rate = ", RateNotify.GetMessageId(),rate);
            RateNotify.DeleteMessage();
        }

    //判断是否文件下载结束     
    if(m_BCRcdA[bcindex].CurPackNum == m_BCRcdA[bcindex].TotalPackNum) 
    {
        m_BCRcdA[bcindex].CurRetryTimes = m_BCRcdA[bcindex].CurRetryTimes + 1;                  
        m_BCRcdA[bcindex].CurPackNum = 0;
        m_BCRcdA[bcindex].Progress = rate;    
  
        //判断是否升级结束
        if(m_BCRcdA[bcindex].CurRetryTimes >= m_BCRcdA[bcindex].RetryTimes)
        {
            //通知刷新进度条进度
            CBCUTSWRateNotify RateNotify;
            RateNotify.CreateMessage(*this);
            RateNotify.SetMessageId(M_BTS_EMS_BC_UPGRADE_UT_SW_PROGRESS);
            RateNotify.SetDstTid(M_TID_EMSAGENTTX);
            RateNotify.SetTransactionId(m_BCRcdA[bcindex].EmsTransId);
            RateNotify.SetCpeHWType(HwType);
            RateNotify.SetProgress(100);
//test start
#ifdef _SMDEBUG
			printf("\r\n RateNotify post Rate(3) = %d \r\n",rate);
#endif
//test stop        
            if(true != RateNotify.Post())
            {
//test start
#ifdef _SMDEBUG
				printf("\r\n RateNotify post failed Rate(3) = %d \r\n",rate);
#endif
//test stop
                OAM_LOGSTR2(LOG_CRITICAL, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail,rate = %d", RateNotify.GetMessageId(),rate);
                RateNotify.DeleteMessage();
            }

            //升级结束,向后台返回升级成功通知消息
            SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                     m_BCRcdA[bcindex].EmsTransId, 
                     M_BTS_EMS_BC_UPGRADE_UT_SW_NOTIFY, 
                     OAM_SUCCESS);
            
//test start
#if 0
int i =0;
for(i =0;i<10;i++)
{
printf("\r\n %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n");
}
m_BCRcdA[bcindex].SWVer = 0xFF0000FF;
#endif
//test stop
            if( NULL != m_BCRcdA[bcindex].pBcSwTimer ) 
            {  
                m_BCRcdA[bcindex].pBcSwTimer->Stop();
                delete m_BCRcdA[bcindex].pBcSwTimer;
                m_BCRcdA[bcindex].pBcSwTimer = NULL;
                memset((UINT8*)&(m_BCRcdA[bcindex].chFileName),0,sizeof(T_BCUGRcd));
            }
        }
        else //继续升级过程
            m_bBCReqFlag = true;
    }
    
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////
#endif //0


void  CTaskFileMag :: SM_calFtpClientTimeout()
{
    if(m_calFtpCFlag == FALSE)// ftp 已经结束
    {
	m_calFtpCTimeoutNum = 0;	
	return;
    }
    m_calFtpCTimeoutNum++;
    if(m_calFtpCTimeoutNum<3)//pm ftp没有结束任务,但超时次数小于3
    {    	
	StartCalFtpCTimer();
	return;
    }    
    bool Rst = false;
    //taskSafe();
    CTaskFfpClient *pTaskFtpClient;
	pTaskFtpClient = CTaskFfpClient::GetInstance();    
    //CTaskFfpClient::GetInstance()->Ftp_SetInstance();
    delete pTaskFtpClient;
    if( taskDelete(m_iFtpCTid) == OK )
    {
        CComEntity::setEntityNull(M_TID_FTPCLIENT);
        m_calFtpCFlag = FALSE;
        m_dlFtpCFlag = FALSE;
        m_pdlFtpCTimer->Stop();
        m_pcalFtpCTimer->Stop();
        pTaskFtpClient = CTaskFfpClient::GetInstance();
        Rst = pTaskFtpClient->Begin();
    }
    if( ! Rst )
    {
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3oam ftpclent restart fail" );
	 StartFtpCCreateTimer();
    }
    else
    {        
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3oam ftpclent restart success, addr[0x%08x]", (UINT32)pTaskFtpClient );
    }
    //taskUnsafe();
}
void  CTaskFileMag :: SM_dlFtpClientTimeout()
{
    if(m_dlFtpCFlag == FALSE)// ftp 已经结束
    {
	m_dlFtpCTimeoutNum = 0;	
	return;
    }
    m_dlFtpCTimeoutNum++;
    if(m_dlFtpCTimeoutNum<3)//pm ftp没有结束任务,但超时次数小于3
    {    	
	StartDlFtpCTimer();
	return;
    }
     UINT32 State = GetSysStatus(); 
    if(OAM_FILE_DL_SW == State)//如果是下载状态,给ems回失败应答
    {
     	CL3OamCommonRsp RspMsg; //向ems返回成功应答消息
        RspMsg.CreateMessage(*this);
        RspMsg.SetMessageId( m_usDlType );
        RspMsg.SetDstTid(M_TID_EMSAGENTTX);
        RspMsg.SetTransactionId( m_usTransId );
        RspMsg.SetResult( L3SM_ERROR_CANT_LINKTO_SERVER );
        if(true != RspMsg.Post())
        {
            OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", RspMsg.GetMessageId());
            RspMsg.DeleteMessage();
        }

    }
    bool Rst = false;
    //taskSafe();
    CTaskFfpClient *pTaskFtpClient;
	pTaskFtpClient = CTaskFfpClient::GetInstance();    
    //CTaskFfpClient::GetInstance()->Ftp_SetInstance();
    delete pTaskFtpClient;
    if( taskDelete(m_iFtpCTid) == OK )
    {
        CComEntity::setEntityNull(M_TID_FTPCLIENT);
        m_calFtpCFlag = FALSE;
        m_dlFtpCFlag = FALSE;
        m_pdlFtpCTimer->Stop();
        m_pcalFtpCTimer->Stop();
        pTaskFtpClient = CTaskFfpClient::GetInstance();
        Rst = pTaskFtpClient->Begin();
	if(!Rst)
	{
	    delete pTaskFtpClient;
	}
		
    }
    if( ! Rst )
    {
        SetSysStatus(OAM_FILE_DL_SW);
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3oam ftpclent restart fail" );
	 StartFtpCCreateTimer();
    }
    else
    {
        SetSysStatus(OAM_FILE_IDLE);
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3oam ftpclent restart success, addr[0x%08x]", (UINT32)pTaskFtpClient );
    }
    //taskUnsafe();
}


void  CTaskFileMag :: StartCalFtpCTimer()
{
    OAM_LOGSTR1(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "StartCalFtpCTimer:%d", m_calFtpCTimeoutNum );
    if( NULL == m_pcalFtpCTimer )
    {
    	CComMessage* pMsgTimer = new ( this, 0 ) CComMessage;        
        if (pMsgTimer!=NULL)
        {
        	pMsgTimer->SetDstTid( M_TID_SM  );
        	pMsgTimer->SetSrcTid( M_TID_SM  );
        	pMsgTimer->SetMessageId( CALFTPCLIENT_TIMER );
        }
    	m_pcalFtpCTimer = new CTimer( false, 120000/*1000*/,  pMsgTimer );
	if(NULL==m_pcalFtpCTimer)
	{
	    pMsgTimer->Destroy();	    
	    return;
	}
    }
    if(true!=m_pcalFtpCTimer->Start())
    	{
    	    delete m_pcalFtpCTimer;

    	}
}
void  CTaskFileMag :: StartDlFtpCTimer()
{
    OAM_LOGSTR1(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "StartDlFtpCTimer:%d", m_dlFtpCTimeoutNum );
    if( NULL == m_pdlFtpCTimer )
    {
    	CComMessage* pMsgTimer = new ( this, 0 ) CComMessage;        
        if (pMsgTimer!=NULL)
        {
        	pMsgTimer->SetDstTid( M_TID_SM  );
        	pMsgTimer->SetSrcTid( M_TID_SM  );
        	pMsgTimer->SetMessageId( DLFTPCLIENT_TIMER );
        }
    	m_pdlFtpCTimer = new CTimer( false, 120000/*1000*/,  pMsgTimer );
	if(NULL==m_pdlFtpCTimer)
	{
	    pMsgTimer->Destroy();	    
	    return;
	}
    }
    if(true!=m_pdlFtpCTimer->Start())
    	{
    	    delete m_pdlFtpCTimer;

    	}
}
/*启动定时器,超时后重新再创建FTPC任务
*/
void CTaskFileMag ::StartFtpCCreateTimer()
{
    OAM_LOGSTR(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "StartFtpCCreateTimer" );
    if( NULL == m_pFtpCTaskTimer )
    {
    	CComMessage* pMsgTimer = new ( this, 0 ) CComMessage;        
        if (pMsgTimer!=NULL)
        {
        	pMsgTimer->SetDstTid( M_TID_SM  );
        	pMsgTimer->SetSrcTid( M_TID_SM  );
        	pMsgTimer->SetMessageId( FTPCFAIL_TIMER );
        }
    	m_pFtpCTaskTimer = new CTimer( false, 120000/*1000*/,  pMsgTimer );
	if(NULL==m_pFtpCTaskTimer)
	{
	    pMsgTimer->Destroy();	    
	    return;
	}
    }
    if(true!=m_pFtpCTaskTimer->Start())
    	{
    	   delete m_pFtpCTaskTimer;
    	}
}
extern "C" bool bspGetIsRebootBtsIfFtpDownEnable();
/*如果3次定时器超时创建ftpc任务都失败，则复位基站
*/
extern  void sendCdrForReboot();/*lijinan 20100916 for bill*/
void CTaskFileMag ::SM_FtpCCreateTimeout()
{
    m_ftpCCreateFailNum++;
    if(m_ftpCCreateFailNum<3)//pm ftp没有结束任务,但超时次数小于3
    {    	
        CTaskFfpClient *pTaskFtpClient;
	 bool Rst = false;
	 pTaskFtpClient = CTaskFfpClient::GetInstance();   
        Rst = pTaskFtpClient->Begin();        
        if( ! Rst )
        {        
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 FtpC task restart fail" );
	    delete pTaskFtpClient;
	     StartFtpCCreateTimer();
        }
        else
        {        
            OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 FtpC restart success, addr[0x%08x]", (UINT32)pTaskFtpClient );
	     m_ftpCCreateFailNum = 0;
        }       
        return;
    }	
    if(bspGetIsRebootBtsIfFtpDownEnable())
    {
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 ftpc task create 4 times fail, reboot bts" );
        bspSetBtsResetReason(RESET_REASON_FTPC_CREATE_FAIL);
	 sendCdrForReboot();
        taskDelay(50);
        rebootBTS(BOOT_CLEAR);
    }
    else
    {
    	StartFtpCCreateTimer();
    }
}
//防止下载时最后一条结果消息没有收到,启动定时器保护
void  CTaskFileMag :: StartUpdateCTimer(UINT32 eid)
{
    if( NULL == m_pUpdateCTimer )
    {
    	CComMessage* pMsgTimer = new ( this, 0 ) CComMessage;        
        if (pMsgTimer!=NULL)
        {
        	pMsgTimer->SetDstTid( M_TID_SM  );
        	pMsgTimer->SetSrcTid( M_TID_SM  );
        	pMsgTimer->SetMessageId( UPDATE_TIMER );
		pMsgTimer->SetEID(eid);
        }
    	m_pUpdateCTimer = new CTimer( false, 10000/*1000*/,  pMsgTimer );
	if(m_pUpdateCTimer==NULL)
	{
	   pMsgTimer->Destroy();	   
	   return;
	}
    }
    if(true!=m_pUpdateCTimer->Start())
    	{
    	    delete m_pUpdateCTimer;
    	}
}
void  CTaskFileMag :: SM_UpdateTimeout(CMessage &rMsg)
{
    UINT32 eid;

    eid = rMsg.GetEID();
	
    //获取升级记录信息
    list<T_CpeUpgradeRecord> :: iterator tlIter ;
    tlIter = FindCpeUpgradeRecord(eid);    
    if(tlIter!=NULL)
    {
        //向EMS发送升级结果指示消息
        CUpdateUTSWResultNotify ResultNotify;
        ResultNotify.CreateMessage(*this);
        ResultNotify.SetMessageId(M_BTS_EMS_UPGRADE_UT_SW_NOTIFY);
        ResultNotify.SetDstTid(M_TID_EMSAGENTTX);
        ResultNotify.SetTransactionId(tlIter->EmsTransId);
        ResultNotify.SetCPEID(eid);
        ResultNotify.SetResult(L3SM_ERROR_UPDATE_TIMEOUT);
       // ResultNotify.Post();
        if(true != ResultNotify.Post())
        {
            OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam file task post msg[0x%04x] fail", ResultNotify.GetMessageId());
            ResultNotify.DeleteMessage();
        }
        DelCpeUpgradeRrcord(eid);
    }
    //ccb中记录升级状态
    CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    pTaskCpeM->CPE_SetDlFlag(eid, 0); 
}
#include "L3_BTS_PM.h"
/*启动对pm任务监测,超时3次则重启pm任务*/
void  CTaskFileMag :: SM_FtpPMTimeout()
{
    if(m_ftpPmFlag == FALSE)//pm ftp 已经结束
    {
	m_ftpPmTimeoutNum = 0;	
	return;
    }
    m_ftpPmTimeoutNum++;
    if(m_ftpPmTimeoutNum<3)//pm ftp没有结束任务,但超时次数小于3
    {    	
	StartFtpPMTimer();
	return;
    }

    CTaskPM *pCTaskPM;   
    bool Rst = false;
    pCTaskPM = CTaskPM::GetInstance();
    pCTaskPM->DelPerfFile();    
    pCTaskPM->DelPerfFile_new();    
    OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3PM task download fail, addr[0x%08x]", (UINT32)pCTaskPM );    
    delete pCTaskPM;
    if( taskDelete(m_iFtpPMTid) == OK )
    {
        m_ftpPmFlag = FALSE;
	 CComEntity::setEntityNull(M_TID_PM);
        pCTaskPM = CTaskPM::GetInstance();
        Rst = pCTaskPM->Begin();
	 pCTaskPM->reStartPeriodRpt();
	 pCTaskPM->InitRTMonitor();
        pCTaskPM->PM_StartRTMonitor();
	 OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM task delete ok" );
    }
    if( ! Rst )
    {        
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM task restart fail" );
	delete pCTaskPM;
	 StartPMCreateTimer();
    }
    else
    {        
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM restart success, addr[0x%08x]", (UINT32)pCTaskPM );
    }    
}
/*
启动对PM任务监测的定时器
*/
void  CTaskFileMag :: StartFtpPMTimer()
{
    OAM_LOGSTR(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "StartFtpPMTimer" );
    if( NULL == m_pFtpPMTimer )
    {
    	CComMessage* pMsgTimer = new ( this, 0 ) CComMessage;       
        if (pMsgTimer!=NULL)
        {
        	pMsgTimer->SetDstTid( M_TID_SM  );
        	pMsgTimer->SetSrcTid( M_TID_SM  );
        	pMsgTimer->SetMessageId( FTPPM_TIMER );
        }
    	m_pFtpPMTimer = new CTimer( false, 120000/*1000*/,  pMsgTimer );
	if(m_pFtpPMTimer==NULL)
	{
	   pMsgTimer->Destroy();	   
	   return;
	}
    }
    if(true!=m_pFtpPMTimer->Start())
    {
        delete 	m_pFtpPMTimer;
    }
}
/*启动定时器,超时后重新再创建pm任务
*/
void CTaskFileMag ::StartPMCreateTimer()
{
    OAM_LOGSTR(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "StartPMCreateTimer" );
    if( NULL == m_pPMTaskTimer )
    {
    	CComMessage* pMsgTimer = new ( this, 0 ) CComMessage;        
        if (pMsgTimer!=NULL)
        {
        	pMsgTimer->SetDstTid( M_TID_SM  );
        	pMsgTimer->SetSrcTid( M_TID_SM  );
        	pMsgTimer->SetMessageId( PMFAIL_TIMER );
        }
    	m_pPMTaskTimer = new CTimer( false, 120000/*1000*/,  pMsgTimer );
	if(m_pPMTaskTimer==NULL)
	{
	   pMsgTimer->Destroy();	   
	   return;
	}
    }
    if(true!=m_pPMTaskTimer->Start())
    	{
    	    delete m_pPMTaskTimer;
    	}
}
/*如果3次定时器超时创建pm任务都失败，则复位基站
*/
void CTaskFileMag ::SM_PMCreateTimeout()
{
    m_PmCreateFailNum++;
    if(m_PmCreateFailNum<3)//pm ftp没有结束任务,但超时次数小于3
    {    	
        CTaskPM *pCTaskPM;   
        bool Rst = false;
        pCTaskPM = CTaskPM::GetInstance();
        Rst = pCTaskPM->Begin();        
        if( ! Rst )
        {        
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM task restart fail" );
	     StartPMCreateTimer();
	      delete pCTaskPM;
        }
        else
        {        
            OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM restart success, addr[0x%08x]", (UINT32)pCTaskPM );
	     m_PmCreateFailNum = 0;
        }       
        return;
    }	
    if(bspGetIsRebootBtsIfFtpDownEnable())
    {
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM task create 4 times fail, reboot bts" );
        bspSetBtsResetReason(RESET_REASON_PM_CREATE_FAIL);
	 sendCdrForReboot();
        taskDelay(1000);
        rebootBTS(BOOT_CLEAR);
    }
    else
    {
        StartPMCreateTimer();
    }
}
/*启动对ftp任务的监测:
如果是pm模块启动的,则启动1分钟定时器,
如果是sm任务启的,则启动2分钟定时器
如果关闭则关闭相关定时器
*/
void CTaskFileMag ::setFtpUsingFlag(UINT32 tid, bool flag)
{
	if(tid == M_TID_PM)
	{
		m_ftpPmFlag = flag;		
		if(flag == TRUE)
		{		    
			StartFtpPMTimer();
			m_iFtpPMTid = CTaskPM::GetInstance()->PM_GetTid();
		}
		else
		{
            if( NULL != m_pFtpPMTimer )
			    m_pFtpPMTimer->Stop();
			OAM_LOGSTR(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "StopFtpPMTimer" );
		}
	}
	else if(tid == M_TID_SM)
	{
		m_dlFtpCFlag = flag;
		if(flag == TRUE)
		{
			StartDlFtpCTimer();
			m_iFtpCTid = CTaskFfpClient::GetInstance()->Ftp_GetTid();
		}
		else
		{
            if( NULL != m_pdlFtpCTimer )
			    m_pdlFtpCTimer->Stop();
			OAM_LOGSTR(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "StopdlFtpCTimer" );
		}
	}
	else if(tid == M_TID_CM)
	{
		m_calFtpCFlag = flag;
		if(flag == TRUE)
		{
			StartCalFtpCTimer();
			m_iFtpCTid = CTaskFfpClient::GetInstance()->Ftp_GetTid();
		}
		else
		{
            if(NULL!=m_pcalFtpCTimer)
			    m_pcalFtpCTimer->Stop();
			OAM_LOGSTR(LOG_DEBUG3, L3FM_ERROR_REV_MSG, "StopcalFtpCTimer" );
		}
	}
}
void  CTaskFileMag :: SM_delCpeUpdateInfo(CMessage &rMsg)
{
    UINT32 eid;
    eid = rMsg.GetEID();
	
    //获取升级记录信息
    list<T_CpeUpgradeRecord> :: iterator tlIter ;
    tlIter = FindCpeUpgradeRecord(eid);    
    if(tlIter!=NULL)
    {
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "eid: %x update timeout, del it", eid );
        DelCpeUpgradeRrcord(eid);
    }
    //ccb中记录升级状态
    CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    pTaskCpeM->CPE_SetDlFlag(eid, 0); 
}
/**********************************************************************
*
*  NAME:          setTaskNull
*  FUNTION:     设置注册队列为空
*  INPUT:          tid, 删除任务
*  OUTPUT:        无
  OTHERS:        jiaying20100820
*******************************************************************/
void CTaskFileMag :: setTaskNull(TID tid)
{
    CComEntity::setEntityNull(tid);
}
extern "C" STATUS restartFtp0()
{
     CTaskPM *pCTaskPM;   
    bool Rst = false;
    int m_iFtpPMTid;
    CTaskFileMag *pCTaskFileMag;
	
    pCTaskFileMag = CTaskFileMag::GetInstance();
    m_iFtpPMTid = CTaskPM::GetInstance()->PM_GetTid();
    pCTaskPM = CTaskPM::GetInstance(); 
    pCTaskPM->DelPerfFile();
    pCTaskPM->DelPerfFile_new();
    delete pCTaskPM;
    if( taskDelete(m_iFtpPMTid) == OK )
    {
        pCTaskFileMag->setTaskNull(M_TID_PM);
        pCTaskPM = CTaskPM::GetInstance();	 
        Rst = pCTaskPM->Begin();
	 pCTaskPM->reStartPeriodRpt();
	 pCTaskPM->InitRTMonitor();
        pCTaskPM->PM_StartRTMonitor();
	 OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM task delete ok" );
    }
    if( ! Rst )
    {        
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM task restart fail" );	 
	 pCTaskFileMag->StartPMCreateTimer();
    }
    else
    {        
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3 PM restart success, addr[0x%08x]", (UINT32)pCTaskPM );
    }    
	return OK;
}
extern "C" STATUS restartFtp1()
{
	bool Rst = false;
	int m_iFtpCTid;
       CTaskFileMag *pCTaskFileMag;
	   
       pCTaskFileMag = CTaskFileMag::GetInstance();
	m_iFtpCTid = CTaskFfpClient::GetInstance()->Ftp_GetTid();	
	CTaskFfpClient *pTaskFtpClient;
	pTaskFtpClient = CTaskFfpClient::GetInstance();	
	delete pTaskFtpClient;
	if( taskDelete(m_iFtpCTid) == OK )
	{
	       pCTaskFileMag->setTaskNull(M_TID_FTPCLIENT);
		pTaskFtpClient = CTaskFfpClient::GetInstance();
		Rst = pTaskFtpClient->Begin();
		stopdl();
	}
	if( ! Rst )
	{
		OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3oam ftpclent restart fail" );		
              pCTaskFileMag->StartFtpCCreateTimer();
	}
	else
	{		
		OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_MSG, "l3oam ftpclent restart success, addr[0x%08x]", (UINT32)pTaskFtpClient );
        }
}
//um收到move away 消息后如果ut正在加载,给sm发送
//此消息,收到后上报终端加载信息,清除终端
bool   CTaskFileMag ::SM_getUtUpgradeStatus(CMessage& rMsg)
{
    T_UtMoveaway *rMoveaway;
    T_UtDlGuageNotify UtDlGuageNotify;
    T_UtDlGuageNotify_tail UtDlGuageTail;

    rMoveaway = (T_UtMoveaway*)rMsg.GetDataPtr();
    
    UtDlGuageNotify.TransId = rMoveaway->TransId;
    UtDlGuageNotify.EID = rMoveaway->EID;
    
     //获取升级记录信息
    list<T_CpeUpgradeRecord> :: iterator Iter;
    Iter = FindCpeUpgradeRecord(UtDlGuageNotify.EID);

    if(Iter==NULL)
	return false;
    else
    {	
	UtDlGuageTail.Seq = Iter->CurPackNum-1;
	UtDlGuageNotify.EmsTransId = Iter->EmsTransId;
	UtDlGuageTail.guage = Iter->Progress;
	UtDlGuageNotify.UT_Type = Iter->bZFlag;
	UtDlGuageTail.newBtsId = rMoveaway->BtsId;	
	UtDlGuageNotify.File_name_length = strlen(Iter->FileDirName);
	memcpy(UtDlGuageNotify.File_name, (UINT8*)Iter->FileDirName, UtDlGuageNotify.File_name_length);
	
    }
    //printf("seq:%x, guage:%d\n", UtDlGuageTail.Seq, UtDlGuageTail.guage);
    //printf("btsid:%d\n", UtDlGuageTail.newBtsId);
    CComMessage* pComMsg;    
    pComMsg = new (this, 17+UtDlGuageNotify.File_name_length) CComMessage;    
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_BtsLayer3DataRsp.");
    	return false;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_SM);    
    pComMsg->SetMessageId(M_BTS_EMS_UPGRADE_UT_GUAGE_NOTIFY);
    memcpy((UINT8*)pComMsg->GetDataPtr(), (UINT8*)&UtDlGuageNotify, 10+UtDlGuageNotify.File_name_length);
    memcpy((UINT8*)((UINT8*)pComMsg->GetDataPtr()+10+UtDlGuageNotify.File_name_length), (UINT8*)&UtDlGuageTail, 7);
        
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }    
    //清除加载信息
    DelCpeUpgradeRrcord(UtDlGuageNotify.EID); 
}
bool   CTaskFileMag ::SM_CpeUpgradeRetran(CMessage& rMsg)
{    
    T_UtRetranHead *rHead = (T_UtRetranHead*)rMsg.GetDataPtr();
    T_UtRetranTail *rTail = (T_UtRetranTail*)((UINT8*)rMsg.GetDataPtr()+9+rHead->File_name_length);

    //如果在升级列表中找到该终端,则拒绝断点升级
    list<T_CpeUpgradeRecord> :: iterator Iter;
    Iter = FindCpeUpgradeRecord(rHead->EID);
    if(Iter!=NULL)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rHead->EmsTransId, 
                         M_BTS_EMS_UPGRADE_UT_RETRAN_RSP, 
                         L3SM_ERROR_CPE_UPDATING);
        OAM_LOGSTR1(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "eid:%x is updating, error!", rHead->EID);
	 return false;
    }
    
    if(true == stopFlag)
    {
        m_SysStatus = OAM_FILE_IDLE;
        stopFlag = false;
        return true;
    }    
    //ccb中记录升级状态
    CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    pTaskCpeM->CPE_SetDlFlag(rHead->EID, 1); 
    T_CpeUpgradeRecord CpeInfo;
    memset(&CpeInfo, 0, sizeof(T_CpeUpgradeRecord));   
    //printf("\r\n FileName len = %d\r\n",rHead->File_name_length);    
    memcpy( CpeInfo.FileDirName, rHead->File_name, rHead->File_name_length);  
    //printf("\r\n FileName = %s\r\n", CpeInfo.FileDirName);
    CpeInfo.EmsTransId   = rHead->EmsTransId;
    CpeInfo.CPEID        = rHead->EID;
//printf("\r\n EmsTransId = %d\r\n", rHead->EmsTransId);
//printf("\r\n CPEID = 0x%x\r\n", rHead->EID);
   
    FILE *pFile = fopen( CpeInfo.FileDirName, "rb" );  
    if(NULL == pFile)
    {
        SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                         rHead->EmsTransId, 
                         M_BTS_EMS_UPGRADE_UT_RETRAN_RSP, 
                         L3SM_ERROR_OPEN_FILE);

        OAM_LOGSTR2(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "l3oam file task open file[%s] fail! trans[%04x]", (int)(CpeInfo.FileDirName), rHead->EmsTransId );
        return true;        
    }

    UINT32 FileLen;
    FileLen = GetFileLength(pFile);        
    CpeInfo.DLReqSeqNum  = GetSysDLReqSeqNum();
    CpeInfo.CurPackNum = rTail->Seq;    
    CpeInfo.FileSize     = FileLen;    
    
    UINT32 LeftChar = FileLen % MAX_SWPACK_UC_SIZE;
    if(0 == LeftChar)
    {
        CpeInfo.TotalPackNum = (UINT16)(FileLen/MAX_SWPACK_UC_SIZE);
    }
    else
    {
        CpeInfo.TotalPackNum = (UINT16)(FileLen/MAX_SWPACK_UC_SIZE + 1);    
    }
    if(CpeInfo.CurPackNum>=CpeInfo.TotalPackNum)
    {
         OAM_LOGSTR1(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "eid:%x curPackNum>=totalnum!", rHead->EID);
         return false;
    }
    CpeInfo.Progress     = (UINT8)(CpeInfo.CurPackNum * 100/ CpeInfo.TotalPackNum);
    CpeInfo.PackSize     = MAX_SWPACK_UC_SIZE;    
    CpeInfo.FileCurPtr   = pFile;
    CpeInfo.bZFlag = rTail->flag;
    CpeInfo.RetranNum = 0;
    //printf("seq:%x, bzflag:%d\n", CpeInfo.CurPackNum, CpeInfo.bZFlag);
    UINT16 Result = AddCpeUpgradeRecord(CpeInfo); 

    SM_PostCommonRsp(M_TID_EMSAGENTTX, 
                     rHead->EmsTransId, 
                     M_BTS_EMS_UPGRADE_UT_RETRAN_RSP, 
                     Result);
    if(Result!=OAM_SUCCESS)
    {    
       OAM_LOGSTR1(LOG_CRITICAL, L3SM_ERROR_REV_MSG, "eid:%x add cpeinfo error!", rHead->EID);
	return false;
    }
    Iter = FindCpeUpgradeRecord(CpeInfo.CPEID);
    SINT8 Buff[MAX_SWPACK_UC_SIZE];
    memset(Buff, 0, MAX_SWPACK_UC_SIZE);
    UINT16 ReadCnt;
    fseek( pFile, CpeInfo.CurPackNum * MAX_SWPACK_UC_SIZE, SEEK_SET );
   if( Iter->CurPackNum < Iter->TotalPackNum - 1 )
        ReadCnt = (UINT16)fread(Buff, 1, MAX_SWPACK_UC_SIZE, pFile);
    else
    {
        ReadCnt = (UINT16)fread(Buff, 1, (Iter->FileSize % MAX_SWPACK_UC_SIZE), pFile);
	 ReadCnt = MAX_SWPACK_UC_SIZE;
    }
    fclose( pFile );
   
    //构造配置失败消息
    CL3OamUnicastUTSWReqFail FailMsg;
    FailMsg.CreateMessage(*this,M_OAM_UNICAST_UTSW_REQ_FAIL);
    FailMsg.SetDstTid(M_TID_SM);    
    FailMsg.SetCPEID(CpeInfo.CPEID);  
	
    
    //向CPE发送软件数据包请求消息
    if( !Iter->bZFlag )
    {  
        SendCpePack( FailMsg, Iter, Buff ,ReadCnt, rMsg.GetTransactionId() );
    }
    else
    {
        SendZPack( FailMsg, Iter, Buff ,ReadCnt, rMsg.GetTransactionId() );
    }

    //修改升级记录
    Iter->Progress   = (UINT8)(Iter->CurPackNum * 100/ Iter->TotalPackNum) ; 
    Iter->CurPackNum = Iter->CurPackNum + 1;
    
    return true;
}
