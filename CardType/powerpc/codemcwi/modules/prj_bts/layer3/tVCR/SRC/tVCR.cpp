/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    tVCR.h
*
* DESCRIPTION: 
*		BTS上的tVCR任务类
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2006-12-19  fengbing  如果SAG的IP配置为0.0.0.0，则认为不使用语音业务
*   2006-10-18  fengbing  修改心跳流程，心跳周期内收到心跳或心跳应答时不影响下次心跳的发送时间
*   2006-10-13  fengbing  连接建立后立即发送beatHeart使SAbis AP进入InService状态
*   2006-04-19  fengbing  加入任务接口上信令＆语音数据的计数统计
*   2006-3-25   fengbing  modify running cfg
*	2006-3-20	fengbing  add more serious checking for abnormal routine
*   2005-9-19   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/


#include "tVCR.h"
#include "BtsVMsgId.h"
#include "commessage.h"
#include "log.h"
#include "mutex.h"
#include "NatpiApp.h"
#include "stdio.h"
#include "l3oammessageid.h"
#include "L3OamCfgCommon.h"
#include "sysBtsConfigData.h"
#include "voiceToolFunc.h"
#include "localSagCfg.h"
#include "localSagMsgID.h"

#ifdef WBBU_CODE
#include "sysBootLoad.h"
#endif
#ifdef DSP_BIOS
extern "C" RESET_REASON bspGetBtsResetReason();
extern  "C" unsigned int  getSysSec();
#define taskDelay(ticks) TSK_sleep(ticks)
#endif
typedef struct _vcrPipeCntT{
    UINT32 nPostCalled;
    UINT32 nPostNull;
    UINT32 nPostFail;
    UINT32 nPostWhenSagNotConnected;
    UINT32 nEnterPipe;
    UINT32 nOutofPipe;
    UINT32 nFlush;
    UINT32 nReadOK;
    UINT32 nReadError;
    UINT32 nReadLessBytes;
}vcrPipeCntsT;
vcrPipeCntsT vcrPipeCnt;
bool sagStatusFlag=false;
extern T_NvRamData *NvRamDataAddr;
extern "C" void showVcrPipeStatus();
extern "C" void clrVcrPipeStatus();
extern  unsigned int  getTimeNUll();
extern UINT8 g_TOS;
extern "C" bool isValidTosVal(UINT8 tos);

void clrVcrPipeStatus()
{
    memset( (void*)&vcrPipeCnt, 0, sizeof(vcrPipeCntsT));
}
void showVcrPipeStatus()
{
    VPRINT("\n========vcrPipeCnt========");
    VPRINT("\n nPostCalled[%d] \n nPostNull[%d] \n nPostFail[%d] \n nPostWhenSagNotConnected[%d] \n nEnterPipe[%d] \n nOutofPipe[%d] \n nFlush[%d] \n nReadOK[%d] \n nReadError[%d] \n nReadLessBytes[%d]",
        vcrPipeCnt.nPostCalled,
        vcrPipeCnt.nPostNull,
        vcrPipeCnt.nPostFail,
        vcrPipeCnt.nPostWhenSagNotConnected,
        vcrPipeCnt.nEnterPipe,
        vcrPipeCnt.nOutofPipe,
        vcrPipeCnt.nFlush,
        vcrPipeCnt.nReadOK,
        vcrPipeCnt.nReadError,
        vcrPipeCnt.nReadLessBytes
        );
    VPRINT("\n================\n");    
}

extern "C" void setVcrRcvBufTraceFlag(bool blShowInfo, bool blShowInfoDetail)
{	
	CVCR::GetInstance()->m_RcvBuf.setShowInfoFlag(blShowInfo);
	CVCR::GetInstance()->m_RcvBuf.setShowDetailFlag(blShowInfoDetail);
}
extern "C" void setBakVcrRcvBufTraceFlag(bool blShowInfo, bool blShowInfoDetail)
{	
	CVCR::GetBakInstance()->m_RcvBuf.setShowInfoFlag(blShowInfo);
	CVCR::GetBakInstance()->m_RcvBuf.setShowDetailFlag(blShowInfoDetail);
}

#ifdef __UNITTEST__
bool  InitL3Oam(void)
{
	return true;
}
bool  InitL3VoiceSvc(void)
{
	CVCR* pTaskVCR = CVCR::GetInstance();
	pTaskVCR->Begin();
	return true;
}
bool  InitL3DataSvc(void)
{
	return true;
}
#endif

//////////////////////////////////////////////////////////////////////////

//任务实例指针的初始化
CVCR* CVCR::s_ptaskTVCR = NULL;
CVCR* CVCR::s_ptaskTVCR1 = NULL;

void CVCR::setTosVal(UINT8 tos)
{
	if(m_fdTcpSag<0)
	{
	}
	else
	{
		int optval = tos;
		if(isValidTosVal( tos))
			setsockopt (m_fdTcpSag, IPPROTO_IP, IP_TOS, (char*)&optval, (int)sizeof(optval));
	}
}

UINT32 CVCR::getSAGID()
{
	return m_pBtsSagLinkCfg->SAGID;
}

void CVCR::setTaskOptions()
{
	if(GetEntityId()==M_TID_VCR1)
	{
		memcpy(m_szName, M_TASK_TVCR1_TASKNAME, strlen( M_TASK_TVCR1_TASKNAME ) );
		m_szName[strlen( M_TASK_TVCR1_TASKNAME )] = 0;
		strcpy(m_szPipeName, M_VCR1_PIPENAME);
		m_pBtsSagLinkCfg = &g_vSagBtsLinkCfg2;
		m_pNatpiSession = getVcr1NatpiSession();
	}
	else
	{
		memcpy(m_szName, M_TASK_TVCR_TASKNAME, strlen( M_TASK_TVCR_TASKNAME ) );
		m_szName[strlen( M_TASK_TVCR_TASKNAME )] = 0;
		strcpy(m_szPipeName, M_VCR_PIPENAME);
		m_pBtsSagLinkCfg = &g_vSagBtsLinkCfg1;
		m_pNatpiSession = getVcrNatpiSession();
	}

	m_uPriority = M_TP_L3VCR;
#ifndef DSP_BIOS
	m_uOptions = M_TASK_TVCR_OPTION;
#endif
	m_uStackSize = M_TASK_TVCR_STACKSIZE;
#ifndef DSP_BIOS
	m_lMaxMsgs = M_TASK_TVCR_MAXMSG;
	m_lMsgQOption = M_TASK_TVCR_MSGOPTION;
#else
    m_iMsgQMax = M_TASK_TVCR_MAXMSG;//lijinan 20091118 add
#endif
	m_fdPipe = 0;
	m_fdTcpSag = -1;

	m_LocalPort = 0xffff;
	m_SagPort = 0xffff;
	strcpy(m_SagIPAddr,"255.255.255.255");

	m_connected=false;
	m_blNetCfgChanged = false;	
	m_blNeedResetLink = false;
}


CVCR::CVCR()
{
#ifndef NDEBUG
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVCR::CVCR()");
	
	if (!Construct(CObject::M_OID_VCR))
	{
       	LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL),"ERROR!!!CVCR::CVCR()% Construct failed.");
	}
#endif	

#if 0
	//memset(m_szName, 0 , M_TASK_NAME_LEN );
	memcpy(m_szName, M_TASK_TVCR_TASKNAME, strlen( M_TASK_TVCR_TASKNAME ) );
	m_szName[strlen( M_TASK_TVCR_TASKNAME )] = 0;
	m_uPriority = M_TP_L3VCR;
	m_uOptions = M_TASK_TVCR_OPTION;
	m_uStackSize = M_TASK_TVCR_STACKSIZE;
	
	m_lMaxMsgs = M_TASK_TVCR_MAXMSG;
	m_lMsgQOption = M_TASK_TVCR_MSGOPTION;

	m_fdPipe = 0;
	m_fdTcpSag = ERROR;

	m_LocalPort = 0xffff;
	m_SagPort = 0xffff;
	strcpy(m_SagIPAddr,"255.255.255.255");

	m_connected=false;
	m_blNetCfgChanged = false;
	m_blNeedResetLink = false;
#endif	
}

CVCR::~CVCR()
{
#ifndef NDEBUG
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVCR::~CVCR");
	if (!Destruct(CObject::M_OID_VCR))
	{
       	LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "ERROR!!!CVCR::~CVCR failed.");
	}
#endif
}

TID CVCR::GetEntityId() const
{ 
	return m_selfTID; 
}

#ifdef NATAP_FUNCTION
int OnVcrNatApRegSuccess(void* para)
{
	CVCR::GetInstance()->SendResetMsgToSAG();
	return 1;
}
#endif

CVCR* CVCR::GetInstance()
{
	if ( NULL == s_ptaskTVCR)
	{
		s_ptaskTVCR = new CVCR;
		if(s_ptaskTVCR!=NULL)
		{
			s_ptaskTVCR->SetEntityId(M_TID_VCR);
			s_ptaskTVCR->setTaskOptions();
		}
	}
	return s_ptaskTVCR;
}

CVCR* CVCR::GetBakInstance()
{
	if ( NULL == s_ptaskTVCR1)
	{
		s_ptaskTVCR1 = new CVCR;
		if(s_ptaskTVCR1!=NULL)
		{
			s_ptaskTVCR1->SetEntityId(M_TID_VCR1);
			s_ptaskTVCR1->setTaskOptions();
		}	
	}
	return s_ptaskTVCR1;
}

bool CVCR::Initialize()
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVCR::Initialize");
	//convert reset reason
	RESET_REASON reason = bspGetBtsResetReason();
	switch(reason)
	{
		case RESET_REASON_HW_WDT:
			m_ResetReason = RESETCAUSE_HARDWARE_ERROR;
			break;
		case RESET_REASON_SW_WDT:
			m_ResetReason = RESETCAUSE_SOFTWARE_ERROR;
			break;
		case RESET_REASON_POWER_ON:
			m_ResetReason = RESETCAUSE_POWEROFF;
			break;
		case RESET_REASON_SW_NORMAL:
			m_ResetReason = RESETCAUSE_ONREQUEST;
			break;
		case RESET_REASON_SW_ALARM:
			m_ResetReason = RESETCAUSE_SOFTWARE_ERROR;
			break;
		case RESET_REASON_SW_ABNORMAL:
	       case RESET_REASON_ARPNOT_GTAEWAY:
		case RESET_REASON_PM_CREATE_FAIL:
              case  RESET_REASON_FTPC_CREATE_FAIL:
              case RESET_REASON_NETMBUFFER_FULL:
		case RESET_REASON_L3BOOTLINE_DIFF:
			m_ResetReason = RESETCAUSE_SOFTWARE_ERROR;
			break;
		case RESET_REASON_SW_INIT_FAIL:
		case RESET_REASON_SW_INIT_DOWNFPGA_FAIL://wangwenhua modify 20080801
    		case RESET_REASON_SW_INIT_EMSTIMEOUT_FAIL:
   		case  RESET_REASON_SW_INIT_LOADCODETIMEOUT_FAIL:
   		case  RESET_REASON_SW_INIT_NVRAMBOOT_FAIL:
			m_ResetReason = RESETCAUSE_SOFTWARE_ERROR;
			break;
		case RESET_REASON_EMS:
			m_ResetReason = RESETCAUSE_ONREQUEST;
			break;
		default:
			m_ResetReason = RESETCAUSE_UNKONWREASON;
	}
#ifdef DSP_BIOS
	TSK_Attrs tskAttr;
	tskAttr.stack = NULL;
	tskAttr.stacksize = 0x4000;
	tskAttr.stackseg = TSK->STACKSEG;
	tskAttr.environ = NULL;
	tskAttr.exitflag = TRUE;
	tskAttr.initstackflag = TRUE;	
	tskAttr.priority = M_TP_L3VCR;
	//tskAttr.name = "TSK_vcrRx";
	if(m_selfTID==M_TID_VCR)
		tskAttr.name = "TSK_vcrRx";
	else
		tskAttr.name = "TSK_vcr1Rx";
	if((m_selfTID==M_TID_VCR))
	{
		if((TSK_create((Fxn)RunVcrSocketRx, &tskAttr, (int)this))==NULL)
		{
			LOG(LOG_SEVERE,0,"TSK_socketRx taskSpawm failed.");
			return false;
		}
	}
	else
	{
		if((TSK_create((Fxn)RunVcr1SocketRx, &tskAttr, (int)this))==NULL)
		{
			LOG(LOG_SEVERE,0,"TSK_socketRx taskSpawm failed.");
			return false;
		}
	}
	if (!CBizTask::Initialize())
		return false;
#else
	//create pipe
	if(!CreatePipe())
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreatePipe() failed.");
		return false;
	}
	//open pipe
	if(!OpenPipe())
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "OpenPipe() failed.");
		DeletePipe();
		return false;
	}
#endif
	return true;
}

bool CVCR::IsNetCfgChanged()
{
	char tmpIpAddr[16];
	//IP address of SAG
	UINT32 IP_UINT32 = m_pBtsSagLinkCfg->SAGSignalIP;
	UINT8* pIPAddr = (UINT8*)&IP_UINT32;
	sprintf(tmpIpAddr, "%d.%d.%d.%d", pIPAddr[0], pIPAddr[1], pIPAddr[2], pIPAddr[3]);

	if( strcmp(tmpIpAddr, m_SagIPAddr)==0 && 
		m_SagPort == m_pBtsSagLinkCfg->SAGRxPortS &&
		m_LocalPort == m_pBtsSagLinkCfg->SAGTxPortS &&
		m_pBtsSagLinkCfg->NatAPKey == m_pNatpiSession->getSPIVal() )
	{
		return false;
	}
	else
	{
		LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "tVCR netcfg changed!!!");
		return true;
	}
}

void CVCR::reloadVcrCfg()
{
	m_SPCCode = m_pBtsSagLinkCfg->BTSSPC;	//本端信令点编码
	m_DPCCode = m_pBtsSagLinkCfg->SAGSPC;	//远端信令点编码

	if(IsNetCfgChanged())
	{
		//IP address of SAG
		UINT32 IP_UINT32 = m_pBtsSagLinkCfg->SAGSignalIP;
		UINT8* pIPAddr = (UINT8*)&IP_UINT32;
		sprintf(m_SagIPAddr, "%d.%d.%d.%d", pIPAddr[0], pIPAddr[1], pIPAddr[2], pIPAddr[3]);
		
		m_SagPort = m_pBtsSagLinkCfg->SAGRxPortS;	//TCP port of SAG  
		m_LocalPort = m_pBtsSagLinkCfg->SAGTxPortS;  //local TCP port
		
#ifdef NATAP_FUNCTION
		//In Effct or not
		//m_pNatpiSession->setEffect(true or false);
		if(m_pNatpiSession->IsInEffect())
		{
			m_pNatpiSession->init(GetBtsSagIpAddr(), 
							m_pBtsSagLinkCfg->SAGTxPortS, 
							m_pBtsSagLinkCfg->SAGSignalIP, 
							m_pBtsSagLinkCfg->SAGRxPortS,
							OnVcrNatApRegSuccess);
			m_pNatpiSession->setSPIVal(m_pBtsSagLinkCfg->NatAPKey);
		}
#endif
	}
}
#ifdef DSP_BIOS
void CVCR::tryToConnectToSAG()
{
	//如果SAG的IP配置为0.0.0.0，则认为不使用语音业务
	while(0==m_pBtsSagLinkCfg->SAGSignalIP)
	{
		if(isMasterInstance())
		{
			LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"m_pBtsSagLinkCfg[0x%08X], &g_vSagBtsLinkCfg1[0x%08X]",
				m_pBtsSagLinkCfg, &g_vSagBtsLinkCfg1);
			LOG3(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"SAGSignalIP value: nvRam[0x%08X] g_vSagBtsLinkCfg1[0x%08X] m_pBtsSagLinkCfg[0x%08X]",
				VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGSignalIP),
				g_vSagBtsLinkCfg1.SAGSignalIP,
				m_pBtsSagLinkCfg->SAGSignalIP);
			
			if(m_pBtsSagLinkCfg!=&g_vSagBtsLinkCfg1)
			{
				LOG2(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"m_pBtsSagLinkCfg[0x%08X], &g_vSagBtsLinkCfg1[0x%08X], not same, error!!!",
					m_pBtsSagLinkCfg, &g_vSagBtsLinkCfg1);
				LOG3(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"SAGSignalIP value: nvRam[0x%08X] g_vSagBtsLinkCfg1[0x%08X] m_pBtsSagLinkCfg[0x%08X]",
					VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGSignalIP),
					g_vSagBtsLinkCfg1.SAGSignalIP,
					m_pBtsSagLinkCfg->SAGSignalIP);				
			}
			
		}
		TSK_sleep(1000);
	}

	reloadVcrCfg();
	//Tcp connection
	while(!ConnectSAG())
	{
		if(!m_AlarmExist)
			SendSAGAlarmToOAM(1);	//alarm occur
		LOG2(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "ConnectSAG() failed,sagIp:0x%x,port:%d.",m_pBtsSagLinkCfg->SAGSignalIP,m_pBtsSagLinkCfg->SAGTxPortS);
		TSK_sleep(1000);
		reloadVcrCfg();
	}
	SetSAGConnected(true);
	if(m_AlarmExist)
	{
		SendSAGAlarmToOAM(0);	//alarm clear
	}
		
#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		m_pNatpiSession->setSocket(m_fdTcpSag);
		m_pNatpiSession->start();
	}
	else
	{
		SendResetMsgToSAG();		//RESETCAUSE_LINK_ERROR or other
	}
#else
	SendResetMsgToSAG();		//RESETCAUSE_LINK_ERROR or other	
#endif

//		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "ConnectSAG() Success.");

}


STATUS CVCR::RunVcrSocketRx(CVCR *pTask)
{
    ( (CVCR*)pTask )->SocketRxMainLoop();
    return 0;
}


STATUS CVCR::RunVcr1SocketRx(CVCR *pTask)
{
    ( (CVCR*)pTask )->SocketRxMainLoop();
    return 0;
}
void CVCR::SocketRxMainLoop()
{
	FD_SET ReadFDs;
	int ret;
	struct timeval timer = { TVCR_MAX_BLOCKED_TIME_IN_10ms_TICK/SecondsToTicks(1), 0} ;
	m_AlarmExist = 0;
	tryToConnectToSAG();
		
	//select消息接收处理循环
	while(1)	
	{
		if(m_fdTcpSag==-1)
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "m_fdTcpSag==-1!!!");
			handleTcpError();
			tryToConnectToSAG();
			continue;
		}
		
		FD_ZERO(&ReadFDs);
		//FD_SET(m_fdPipe, &ReadFDs);		
		FD_SET(m_fdTcpSag, &ReadFDs);
		int maxFD = m_fdTcpSag;//max(m_fdPipe,m_fdTcpSag);
		ret = select(maxFD+1, &ReadFDs, NULL, NULL, &timer);

		if(ret==-1)	//error occured
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VCR select() return ERROR!!!");
			handleTcpError();
			tryToConnectToSAG();
			continue;
		}
		//netcfg changed,need to reconnect sag
		if(m_blNetCfgChanged)
		{
			m_blNetCfgChanged = false;
			handleTcpError();
			tryToConnectToSAG();
			continue;
		}
		//when resetLink command received
		if(m_blNeedResetLink)
		{
			m_blNeedResetLink = false;
			handleTcpError();
			tryToConnectToSAG();
			continue;
		}

		if(ret==0)
		{
			LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VCR select() return 0,m_fdTcpSag = %d!!!",m_fdTcpSag);
			continue;
		}
		else 
		{
			LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VCR select() return %d,m_fdTcpSag = %d!!!", ret, m_fdTcpSag);
			//int a=1;
		}

		//when sag not connected
		if(!IsSAGConnected())
		{
			m_blNetCfgChanged = false;
			m_blNeedResetLink = false;
			handleTcpError();
			tryToConnectToSAG();
			continue;
		}		
		
		//处理tcp接收的消息,tcp接收如果出错,清除管道中的数据handleTcpError
		if(FD_ISSET(m_fdTcpSag, &ReadFDs))
		{
			ret = m_RcvBuf.recvData2Buf();
			if(ret<0)
			{
				handleTcpError();
				tryToConnectToSAG();
				continue;
			}
			CComMessage* pComMsg = NULL;
			while( (ret = m_RcvBuf.getOneMsgOutofBuf(&pComMsg))!= M_CONTINUE_RECV)
			{
				if(M_GET_ONE_MSG==ret)
				{
					processOneMsgFromPeer(pComMsg);
				}
			}
		}
	}

}
#endif//#ifdef DSP_BIOS

#ifdef DSP_BIOS
extern "C" void mySwitchFxn(int tid,unsigned int inOut,void* pMsg);
#endif
//任务循环
void CVCR::MainLoop()
{
#ifdef DSP_BIOS
    while (1)
    {
    	mySwitchFxn(GetEntityId(),0,NULL);
        CComMessage* pComMsg = GetMessage();
	mySwitchFxn(GetEntityId(),1,(void*)pComMsg);
		UINT16 msgID=0;
		TID srcTid;
		if(pComMsg!=NULL)
		{
			msgID = pComMsg->GetMessageId();
			srcTid = pComMsg->GetSrcTid();
#ifdef NATAP_FUNCTION
    			if(MSGID_TMOUT_REG_LOOP<=msgID && 
    					msgID<=MSGID_TMOUT_HANDSHAKERSP)
    			{
    				if(m_pNatpiSession->IsInEffect())
    				{
    					m_pNatpiSession->handleNATAPTmoutMsg(msgID);
    				}
				pComMsg->Destroy();
				pComMsg = NULL;
    			}
				else
#endif		
			if(msgID==MSGID_VOICE_SET_CFG && M_TID_VOICE==srcTid)
			{
				bool blNetCfgChanged = IsNetCfgChanged();//true;
				reloadVcrCfg();
				if(blNetCfgChanged)
				{
					m_blNetCfgChanged = true;
				}
				else
				{
				}
				pComMsg->Destroy();
				pComMsg = NULL;
			}
			else if(msgID==MSGID_VOICE_VCR_RESETLINK && M_TID_VOICE==srcTid)
			{
			#if 1 //by fengbing tmp test 20090922
				m_blNeedResetLink = true;
            #endif 
				pComMsg->Destroy();
				pComMsg = NULL;
			}
			else
			{
				Counters.nSigToVCR++;
				if(M_TID_VOICE==srcTid)
				{
					Counters.nSigFromTvoice++;
				}
				else
				{
					Counters.nSigFromUm++;
				}
			}
		}
	
        if ( NULL == pComMsg )
        {
            continue;
        }
#ifndef NDEBUG
    LOG1(LOG_DEBUG3,0,"Received Message ID=0X%x",pComMsg->GetMessageId());
#endif
        if(0>sendOneSignalToSAG(pComMsg))
		{
			if(pComMsg!=NULL)
				pComMsg->Destroy();
			handleTcpError();
			//tryToConnectToSAG();//del by fengbing,20091222,最好保证尽量只有一个任务负责socket的连接，本任务只pend在消息队列上
			continue;
		}
		else
		{
			if(pComMsg!=NULL)
				pComMsg->Destroy();
		}
    }
#else//#ifdef DSP_BIOS
	FD_SET ReadFDs;
	int ret;
	m_AlarmExist = 0;
	while (1) 
	{
    	//如果SAG的IP配置为0.0.0.0，则认为不使用语音业务
    	while(0==m_pBtsSagLinkCfg->SAGSignalIP)
    	{
    		taskDelay(300);
    	}

		reloadVcrCfg();
		//Tcp connection
		while(!ConnectSAG())
		{
			if(!m_AlarmExist)
				SendSAGAlarmToOAM(1);	//alarm occur
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "ConnectSAG() failed.");
			taskDelay(1000);
			reloadVcrCfg();
		}
		SetSAGConnected(true);
		if(m_AlarmExist)
		{
			SendSAGAlarmToOAM(0);	//alarm clear
		}
		
#ifdef NATAP_FUNCTION
		if(m_pNatpiSession->IsInEffect())
		{
			m_pNatpiSession->setSocket(m_fdTcpSag);
			m_pNatpiSession->start();
		}
		else
		{
			SendResetMsgToSAG();		//RESETCAUSE_LINK_ERROR or other
		}
#else
			SendResetMsgToSAG();		//RESETCAUSE_LINK_ERROR or other	
#endif

		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "ConnectSAG() Success.");
		
		//select消息接收处理循环
		while(1)	
		{
			struct timeval timer = { TVCR_MAX_BLOCKED_TIME_IN_10ms_TICK/SecondsToTicks(1), 0} ;
			FD_ZERO(&ReadFDs);
			FD_SET(m_fdPipe, &ReadFDs);		
			FD_SET(m_fdTcpSag, &ReadFDs);
			int maxFD = max(m_fdPipe,m_fdTcpSag);
			ret = select(maxFD+1, &ReadFDs, NULL, NULL, &timer);
			if(ret==-1)	//error occured
			{
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VCR select() return ERROR!!!");
				handleTcpError();
				break;
			}
			//先处理管道中的数据,tcp发送如果出错,清除管道中的消息handleTcpError
			if(FD_ISSET(m_fdPipe, &ReadFDs))
			{
				CComMessage *pComMsg = readOneSignalFromPipe();
				
				//netcfg changed,need to reconnect sag
				if(m_blNetCfgChanged)
				{
					m_blNetCfgChanged = false;
					handleTcpError();
					break;
				}
				//when resetLink command received
				if(m_blNeedResetLink)
				{
					m_blNeedResetLink = false;
					handleTcpError();
					break;
				}
				
				if(0>sendOneSignalToSAG(pComMsg))
				{
					if(pComMsg!=NULL)
						pComMsg->Destroy();
					handleTcpError();
					break;
				}
				else
				{
					if(pComMsg!=NULL)
						pComMsg->Destroy();
				}
			}
			//处理tcp接收的消息,tcp接收如果出错,清除管道中的数据handleTcpError
			if(FD_ISSET(m_fdTcpSag, &ReadFDs))
			{
				ret = m_RcvBuf.recvData2Buf();
				if(ret<0)
				{
					handleTcpError();
					break;
				}
				CComMessage* pComMsg = NULL;
				while( (ret = m_RcvBuf.getOneMsgOutofBuf(&pComMsg))!= M_CONTINUE_RECV)
				{
					if(M_GET_ONE_MSG==ret)
					{
						processOneMsgFromPeer(pComMsg);
					}
				}
			}
		}
	}
#endif//#ifdef DSP_BIOS
}

void CVCR::sendOneSignalToUm(CComMessage* pComMsg)
{
	if(NULL==pComMsg)
		return;
	pComMsg->SetDstTid(M_TID_UM);
	pComMsg->SetSrcTid(M_TID_VCR);

	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_MSG_SND_FAIL), "send SAbis1 signal to tVoice failed");
		pComMsg->Destroy();
	}
	else
	{
		Counters.nSigToUm++;
	}
	
}
void CVCR::processOneSignalFromSAG(CComMessage* pComMsg)
{
	Counters.nSigFromSAG++;

	CMessage tmpMsg(pComMsg);
	CMsg_Signal_VCR signalMsgVCR(tmpMsg);
	signalMsgVCR.DoCountRxSignal(); 	//从SAG接收信令计数统计

	SignalType sigType = signalMsgVCR.ParseMessageFromSAG();

	//如果主SAG连通，则从备用SAG收到的消息只有心跳类消息转发给tVoice
	if(isBackupInstance())
	{
		if(CVCR::GetInstance()->IsSAGConnected())
		{
			if(sigType!=BeatHeart_MSG && sigType!=BeatHeartAck_MSG)
			{
				//20090905 fengbing,need to destroy pComMsg
				if(pComMsg!=NULL)
				{
					pComMsg->Destroy();
				}
				return;
			}
		}
	}

	VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)pComMsg->GetDataPtr();
	UINT8 eventGroupID = pSigVCR->sigHeader.EVENT_GROUP_ID;
	UINT16 eventID = VGetU16BitVal(pSigVCR->sigHeader.Event_ID);
	switch(sigType)
	{
		case Auth_Cmd_MSG:
			pComMsg->SetMessageId(MSGID_AUTH_CMD);
			break;
		case Auth_Result_MSG:
			pComMsg->SetMessageId(MSGID_AUTHDEV_RESULT);
			break;
		case BWInfo_Rsp_MSG:
			pComMsg->SetMessageId(MSGID_BWINFO_RSP);
			break;
		case BWInfo_Del_Req_MSG:
			pComMsg->SetMessageId(MSGID_BWINFO_DEL_REQ);
			break;
		case BWInfo_Modify_Req_MSG:
			pComMsg->SetMessageId(MSGID_BWINFO_UPDATE_REQ);
			break;
		default:
			if(M_MSG_EVENT_GROUP_ID_UTSAGOAM==eventGroupID)
			{
				pComMsg->SetMessageId(MSGID_AUTH_CMD - 1 + eventID);
				break;
			}
			else
			{
				sendOneSignalToTVoice(pComMsg);
				return;
			}
	};	
	sendOneSignalToUm(pComMsg);
}

void CVCR::processOneMsgFromPeer(CComMessage* pMsg)
{
	if(!pMsg)
	{
		return;
	}
	
	UINT8 *bufNatPkt = (UINT8*)pMsg->GetDataPtr();
	UINT16 nNatApLen = pMsg->GetDataLength();
	
	NATAP_PKT_TYPE NatApPktType = 
		m_pNatpiSession->authenticateNATAPMsg(bufNatPkt, nNatApLen);
	if(INVALID_NATAP_PKT==NatApPktType)
	{
		pMsg->Destroy();
		return;
	}
	if(APP_PACKET<NatApPktType && NatApPktType<=CLOSE_MSG)
	{
		m_pNatpiSession->handleNATAPMsg(bufNatPkt, nNatApLen);
		pMsg->Destroy();
		return;
	}

	//App packet

	//tcp header len
	TcpPktHeaderT* pTcpPktHeader = (TcpPktHeaderT*)(bufNatPkt+
									sizeof(AH_T)+sizeof(NATAP_LinkIDT));
	UINT16 nTcpPktLen = VGetU16BitVal(pTcpPktHeader->PktLen);
	UINT16 nTCPEUserType = VGetU16BitVal(pTcpPktHeader->UserType);

	if( nTcpPktLen+sizeof(NATAP_LinkIDT)+sizeof(AH_T)!=nNatApLen )
	{
		LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_INVALID_SIGNAL), 
			"invalid packet, tcp header len error!!!");
		pMsg->Destroy();
		return;
	}
	//如果是TCP连接认证和握手消息，则返回应答
	if( M_TCP_PKT_TEST_USERTYPE== nTCPEUserType)
	{
		UINT8 *pData = (UINT8*)(bufNatPkt+
								sizeof(AH_T)+
								sizeof(NATAP_LinkIDT)+
								sizeof(TcpPktHeaderT));
		UINT8 MsgType = pData[0];
		UINT32 RAND = 0xFFFFFFFF;
		if(M_TCP_PKT_TEST_AUTH_REQ==MsgType)	//认证请求
		{
			TcpPktTestAuthReqT* pPayload = (TcpPktTestAuthReqT*)pData;
			RAND = VGetU32BitVal(pPayload->RAND);
		}
		send_TcpFuncTestPkt_Rsp(MsgType, RAND);
		pMsg->Destroy();
		return;
	}

	//如果不是本信令点或者用户类型错，丢弃
	if( VGetU16BitVal(pTcpPktHeader->DPC_Code)!=m_pBtsSagLinkCfg->BTSSPC || 
		nTCPEUserType!=(M_TCP_PKT_SABIS1_USERTYPE) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), 
			"invalid packet, DPCCode or userType error!!!");
		pMsg->Destroy();
		return;
	}
	
	//sig header len
	SigHeaderT* pSigHeader = (SigHeaderT*)((UINT8*)pTcpPktHeader+
											sizeof(TcpPktHeaderT));
	UINT16 nSigLen = VGetU16BitVal(pSigHeader->Length);
	if( nSigLen+sizeof(SigHeaderT)+sizeof(TcpPktHeaderT)!=nTcpPktLen )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), 
			"invalid packet, signal header len error!!!");
		pMsg->Destroy();
		return;
	}
	
	//合法信令
	UINT16 sigDataLen = nNatApLen
						-sizeof(AH_T)
						-sizeof(NATAP_LinkIDT)-sizeof(TcpPktHeaderT);
	pMsg->SetDataPtr((void*)pSigHeader);
	pMsg->SetDataLength(sigDataLen);
	pMsg->SetMessageId(MSGID_VCR_VOICE_SIGNAL);

	processOneSignalFromSAG(pMsg);
}

//往任务的管道中发送消息
bool CVCR::PostMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent)
{
	vcrPipeCnt.nPostCalled++;
	if(NULL==pMsg)
	{
		vcrPipeCnt.nPostNull++;
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "write null msg to pipe.");
		return false;
	}

	//信令消息
	if(MSGID_VOICE_VCR_SIGNAL==pMsg->GetMessageId())
	{
		//与对应的sag未连接
		if(!IsSAGConnected())
		{
			CMessage msg(pMsg);
			CMsg_Signal_VCR signalMsgVCR(msg);
			SignalType sigType = signalMsgVCR.ParseMessageToSAG();
			//心跳类消息丢弃
			if(BeatHeart_MSG==sigType || BeatHeartAck_MSG==sigType)
			{
				return false;
			}
			else
			{
				//主sag断开时发送给备sag
				if(isMasterInstance())
				{
					//to backup sag
					pMsg->SetDstTid(M_TID_VCR1);
					//change sagid
					VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)pMsg->GetDataPtr();
					VSetU32BitVal(pSigVCR->sigHeader.SAG_ID, CVCR::GetBakInstance()->getSAGID());
					return CComEntity::PostEntityMessage(pMsg);
				}
				//备sag断开时发送给本地sag任务
				if(isBackupInstance())
				{
					if(g_blUseLocalSag)
					{
						//to local sag
						pMsg->SetDstTid(M_TID_SAG);
						return CComEntity::PostEntityMessage(pMsg);					
					}
					else
					{
						//discard msg
						LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
						"no SAG connected, discard msg.");
						return false;
					}
					
				}
			}
		}
	}

	if (!IsSAGConnected()) 
	{
		vcrPipeCnt.nPostWhenSagNotConnected++;
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "write msg to pipe when SAG disconnected, discard msg.");
		pMsg->AddRef();
		return false;
	}
#ifdef DSP_BIOS
	return CBizTask::PostMessage( pMsg, timeout, isUrgent);
#else//#ifdef DSP_BIOS
	if (false == pMsg->AddRef())
	{
		return false;
	}
	//往管道中发送pMsg
#ifdef __WIN32_SIM__
	struct sockaddr_in fsin;
	fsin.sin_family = AF_INET;
	fsin.sin_port= htons(M_WIN32_VCR_PIPEPORT);
	fsin.sin_addr.s_addr=inet_addr("127.0.0.1");

	int ret = sendto (m_fdUdpSndToPipe, (char *)&pMsg, sizeof(pMsg), 0,
		(struct sockaddr *)&fsin,
		sizeof(struct sockaddr));
#else
	int ret = write(m_fdPipe, (char*)&pMsg, sizeof(pMsg));
#endif
	if(ret==sizeof(pMsg))
	{
		vcrPipeCnt.nEnterPipe++;
		return true;
	}
	else
	{
		vcrPipeCnt.nPostFail++;
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "writing msg to pipe failed.");
		//pMsg->Destroy();
		return false;
	}
#endif//#ifdef DSP_BIOS
}
//创建socket
bool CVCR::CreateSocket()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVCR::CreateSocket");
#endif
    sockaddr_in server;
	
    m_fdTcpSag = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_fdTcpSag<0)
    {
        LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "Create socket failed.");
        return false;
    }
#ifndef __USE_LWIP__
	int ret;
	struct linger Linger={1,0};
	int yes = 1;	
	UINT32 on = 1;
	ret = setsockopt(m_fdTcpSag, SOL_SOCKET, SO_LINGER, (char *)&Linger, sizeof(Linger));
	//tcp no delay option
	ret = setsockopt(m_fdTcpSag, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(yes));
	//keep alive option
	ret = setsockopt(m_fdTcpSag, SOL_SOCKET, SO_KEEPALIVE, (char *)&yes, sizeof(yes));
	
#ifdef __WIN32_SIM__
//	if(ioctlsocket(m_fdTcpSag, FIONBIO, (u_long*)&on) < 0) 
#else
	if(ioctl(m_fdTcpSag, FIONBIO, (int)&on) < 0) 
	{ 
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "make socket unblock fail.");
		OutputSocketErrCode("ioctl(m_fdTcpSag, FIONBIO, (u_long*)&on)");
	} 
#endif


	//addr reuse
	//ret = setsockopt (m_fdTcpSag, SOL_SOCKET, SO_REUSEADDR, (char *)0, 0); 	
	ret = setsockopt (m_fdTcpSag, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)); 	
#endif//#ifndef __USE_LWIP__
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(m_LocalPort);//bts的tcp端口绑定指定的端口	
    if (::bind(m_fdTcpSag, (sockaddr*)&server, sizeof(server))<0)
    {
        LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "bind socket failed.");
		OutputSocketErrCode("bind(m_fdTcpSag, (sockaddr*)&server, sizeof(server))");
        DisconnectSAG();
        return false;
    }
	this->setTosVal(g_TOS);
    return true;
}
//和SAG建立TCP连接,阻塞调用保证知道是否连接成功
bool CVCR::ConnectSAG()
{
	LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "ConnectSAG() ,sagIp:0x%x,port:%d.",m_pBtsSagLinkCfg->SAGSignalIP,m_pBtsSagLinkCfg->SAGTxPortS);
	DisconnectSAG();
	CreateSocket();
	struct sockaddr_in fsin;
	fsin.sin_family = AF_INET;
	fsin.sin_port= htons(m_SagPort);
	fsin.sin_addr.s_addr=inet_addr(m_SagIPAddr);
#ifndef WBBU_CODE //其他代码
#ifdef __USE_LWIP__
	if( connect(m_fdTcpSag, (struct sockaddr *) &fsin ,sizeof(fsin))<0 )
#else
#ifdef __VXWORKS__
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	if( connectWithTimeout(m_fdTcpSag, (struct sockaddr *) &fsin , sizeof(fsin), &timeout)<0 )
#endif
#endif
	{
		return false;
	}	 
#else //WBBU代码
	int result =0;
	int err;
	char connect_ok = 0;
	result = ( connect(m_fdTcpSag, (struct sockaddr *) &fsin ,sizeof(fsin)));
	if( result!=OK )
	{
		err = errno;
		if (err != EINPROGRESS)
		{
			LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), 
					"ConnectSAG() error!!!err[%d], err != EINPROGRESS", err);
		}
		else
		{
			time_t tmBegin = time(NULL);
			while(1)
			{
				connect(m_fdTcpSag, (struct sockaddr *) &fsin ,sizeof(fsin));
				err = errno;                   
				switch (err)
				{
					case EISCONN:   /* connect ok */
						connect_ok = 1;
						break;
					case EALREADY:  /* is connecting, need to check again */
						connect_ok = 2;
						break;
					default:   /* failed, retry again ? */
						connect_ok =0;
						break;
				}
				LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
						"ConnectSAG() normal! err[%d] connect_ok[%d]", 
						err, connect_ok);
				if(1==connect_ok)
				{
					return true;
				}
				time_t tmNow = time(NULL);
				if((tmNow-tmBegin)>15)
				{
					return false;
				}
				taskDelay(100);
			}
		}
		return false;
	}	 
#endif
	return true;
}
//和SAG断开TCP连接
bool CVCR::DisconnectSAG()
{
#ifndef __USE_LWIP__
	shutdown(m_fdTcpSag,2);
#endif
	if(-1!=m_fdTcpSag) {
#ifdef __WIN32_SIM__
	closesocket(m_fdTcpSag);
#else
		close(m_fdTcpSag);
#endif	
	}
	m_fdTcpSag = -1;

#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		m_pNatpiSession->setSocket(m_fdTcpSag);
		m_pNatpiSession->stop();
	}
#endif

	return true;
}

//创建Pipe
bool CVCR::CreatePipe()
{
#ifndef DSP_BIOS
#ifdef __WIN32_SIM__
	m_fdUdpSndToPipe = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(INVALID_SOCKET==m_fdUdpSndToPipe)
	{
		OutputSocketErrCode("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)");
		return false;
	}
	m_fdPipe = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(INVALID_SOCKET==m_fdPipe)
	{
		OutputSocketErrCode("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)");
		return false;
	}
	sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(M_WIN32_VCR_PIPEPORT);
    if (::bind(m_fdPipe, (sockaddr*)&server, sizeof(server))==SOCKET_ERROR)
    {
        OutputSocketErrCode("bind(m_fdPipe, (sockaddr*)&server, sizeof(server))");
        return false;
    }
	return true;
#else		//vxworks
	return pipeDevCreate(m_szPipeName, m_lMaxMsgs, sizeof(int))==OK ? true:false;
#endif	
#else
	return true;
#endif
}
//打开Pipe
bool CVCR::OpenPipe()
{
#ifndef DSP_BIOS
    clrVcrPipeStatus();
#ifdef __WIN32_SIM__
	//return true;
#else		//vxworks
	m_fdPipe = open(m_szPipeName, O_RDWR, 0644);
	if(ERROR==m_fdPipe)
    {
        LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "VCR open pipe error!!!");
        return false;
    }
#endif	

	//set pipe unblock mode
#ifdef __WIN32_SIM__
	UINT32 on = TRUE;
	if ( ioctlsocket(m_fdPipe, FIONBIO, (u_long*)&on) < 0 ) // make socket non-block 
#else	//vxworks
	//if ( ioctl(m_fdPipe, FIONBIO, (int)&on) < 0 ) // make handle non-block 
	if(0)
#endif
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "make pipe unblock failed!!!");
		return false;
	}

    return true;
#else
	return true;
#endif
}
//删除Pipe
bool CVCR::DeletePipe()
{
#ifndef DSP_BIOS
#ifdef __WIN32_SIM__
	closesocket(m_fdPipe);
	closesocket(m_fdUdpSndToPipe);
	return true;
#else		//vxworks
	return pipeDevDelete(m_szPipeName, true)==OK;
#endif
#else
	return true;
#endif
}
//关闭管道
bool CVCR::ClosePipe()
{
#ifndef DSP_BIOS
#ifdef __WIN32_SIM__
	return true;
#else		//vxworks
	return close(m_fdPipe)!=ERROR;
#endif	
#else
	return true;
#endif	
}

bool CVCR::IsSAGConnected()
{
	bool ret;
	//lock
	m_lock.Wait();
	ret = m_connected;
	//unlock
	m_lock.Release();
	return ret;
}
void CVCR::SetSAGConnected(bool connected)
{
	if(connected)
	{
		m_RcvBuf.resetRcvBuf();
		m_RcvBuf.setFd(m_fdTcpSag);
	}
	//lock
	m_lock.Wait();
	m_connected = connected;
	//unlock
	m_lock.Release();
#if 0	
	if(isMasterInstance())
	{
		sagStatusFlag = (connected || CVCR::GetBakInstance()->IsSAGConnected() );
	}
	if(isBackupInstance())
	{
		sagStatusFlag = (connected || CVCR::GetInstance()->IsSAGConnected() );
	}
//20091120 add by fengbing begin
	if(connected)
	{
		//向tSag发送退出服务消息
		sendNopayloadMsg(CTVoice::GetInstance(), M_TID_SAG, GetEntityId(), MSGID_BTS_SAG_STOP_SRV);
	}
//20091120 add by fengbing end

	//判断是否停止语音业务
	if(!connected)
	{
		if(isMasterInstance() ||
			( isBackupInstance() && !CVCR::GetInstance()->IsSAGConnected() ) )
		{
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, GetEntityId(), MSGID_VCR_VOICE_RELEASE_VOICE_SRV);
		}
	}
	else
	{
		if(isMasterInstance() ||
			( isBackupInstance() && !CVCR::GetInstance()->IsSAGConnected() ) )
		{
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, GetEntityId(), MSGID_VCR_VOICE_RELEASE_VOICE_SRV);
		}		
	}
	//判断是否需要指令登记
	if(!connected)
	{
		if(isMasterInstance() && CVCR::GetBakInstance()->IsSAGConnected())
		{
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, GetEntityId(), MSGID_VCR_VOICE_FORCE_UT_REGISTER);
		}
	}
	else
	{
		if(isMasterInstance())
		{
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, GetEntityId(), MSGID_VCR_VOICE_FORCE_UT_REGISTER);
		}
		if(isBackupInstance() && !CVCR::GetInstance()->IsSAGConnected())
		{
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, GetEntityId(), MSGID_VCR_VOICE_FORCE_UT_REGISTER);
		}
	}
#ifdef M__SUPPORT__ENC_RYP_TION
	tellBtsL2SrvStatus(GetEntityId());
#endif
#endif
	sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, GetEntityId(), MSGID_VCR_VOICE_SABIS1_RESET);
}

//处理掉管道中已有的消息
bool CVCR::FlushPipe()
{
	//处理掉管道中已有的消息
#ifndef DSP_BIOS
	int nBytesUnread = 0;
	int ret;
	CComMessage *pComMsg;
	while(1)
	{
		//how many bytes available in the first msg
		ret = ioctl (m_fdPipe, FIONREAD, (int)&nBytesUnread);
		if(ERROR!=ret && nBytesUnread>0)
		{
			if(sizeof(UINT32)==nBytesUnread)
	    		{
	    			//清除管道中的消息，释放内存
	    			pComMsg=readOneSignalFromPipe();
	    			if(pComMsg!=NULL)
	    			{
					vcrPipeCnt.nFlush++;
	    				pComMsg->Destroy();
	    			}
	    		}	
			else
	    		{
				//read this invalid msg
				readOneSignalFromPipe();
	    		}
    		}	
    		else
		{
			if(0==nBytesUnread)
			{
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Flush pipe , pipe empty!!!");
			}
			else if(ret==ERROR)
			{
				return false;
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "ioctl (m_fdPipe, FIONREAD, (int)&nBytesUnread) return ERROR!!!");
			}
			else//nBytesUnread<0
			{
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "ioctl (m_fdPipe, FIONREAD, (int)&nBytesUnread) nBytesUnread<0!!!");
			}
			return true;
		}
	}
#else
	return true;
#endif

}

void CVCR::handleTcpError()
{
	//不再往管道中发送消息
	SetSAGConnected(false);
	//处理掉管道中已有的消息
	FlushPipe();
	//断开TCP连接
	DisconnectSAG();
	if(!m_AlarmExist)
		SendSAGAlarmToOAM(1);	//alarm occur
	m_ResetReason = RESETCAUSE_LINK_ERROR;

	//避免配置错误时对SAG形成连接风暴
	taskDelay(1000);	
}

/*
 *	read one signal from SAG
 *  success return 1, fail return 0, return -1 when TCP error
 */

//send one signal to task tVoice
void CVCR::sendOneSignalToTVoice(CComMessage* pComMsg)
{
	if(NULL==pComMsg)
		return;
	pComMsg->SetDstTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_VCR_VOICE_SIGNAL);
	pComMsg->SetSrcTid(GetEntityId());

	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_MSG_SND_FAIL), "send SAbis1 signal to tVoice failed");
		pComMsg->Destroy();
	}
	else
	{
		Counters.nSigToTvoice++;
	}
}

//send one signal to SAG, return true when success, otherwise return false
int CVCR::sendOneSignalToSAG(CComMessage* pComMsg)
{
#ifdef DSP_BIOS
	int fail_cnt =0;
#endif
	if(NULL==pComMsg)
		return 0;
	if(pComMsg->GetMessageId()!=MSGID_VOICE_VCR_SIGNAL)	//非信令不发给sag
		return 0;
	
	//构造头部和尾部
	UINT8* pData = (UINT8*)pComMsg->GetDataPtr();
	TcpPktHeaderT* pTcpPktHeader = (TcpPktHeaderT*)(pData-sizeof(TcpPktHeaderT));
	UINT16 SigDataLen = pComMsg->GetDataLength();
	//填写Packet头部
	VSetU16BitVal(pTcpPktHeader->SPC_Code , (m_SPCCode));					//源信令点
	VSetU16BitVal(pTcpPktHeader->DPC_Code , (m_DPCCode));					//目的信令点
	VSetU16BitVal(pTcpPktHeader->UserType , (M_TCP_PKT_SABIS1_USERTYPE));	//用户类型
	VSetU16BitVal(pTcpPktHeader->TTL , (M_TCP_PKT_DEFAULT_TTL));			//路由计数器，默认值32
	VSetU16BitVal(pTcpPktHeader->PktLen , (SigDataLen+sizeof(TcpPktHeaderT)));

	UINT8* pBeginFlag = (UINT8*)((UINT8*)pTcpPktHeader-sizeof(HeadFlagT));

#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		UINT8 NatApPktLen = 
			m_pNatpiSession->packNATAPMsg4AppPkt((UINT8*)pTcpPktHeader,
											SigDataLen+sizeof(TcpPktHeaderT));

		pBeginFlag = (UINT8*)((UINT8*)pBeginFlag-NatApPktLen);
	}
#endif

	//Packet开始标志
	VSetU16BitVal(pBeginFlag, M_TCP_PKT_BEGIN_FLAG);
	//填写Packet尾部Packet结束标志
	UINT8* pEndFlag = (UINT8*)(&pData[SigDataLen]);
	VSetU16BitVal(pEndFlag, M_TCP_PKT_END_FLAG);
	//发送给SAG
	int sent, totalToSend, ret;
	char *bufToSend = (char*)pBeginFlag;
	ret = totalToSend = (UINT8*)pEndFlag - (UINT8*)pBeginFlag
						+sizeof(EndFlagT);
	do 
	{
		sent = send(m_fdTcpSag, bufToSend, totalToSend, 0);
		if(sent<0)
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "CVCR::sendOneSignalToSAG() send error!!!");
			OutputSocketErrCode("send()");
			return sent;
		}
				#ifdef DSP_BIOS
		if(sent==0) {
			TSK_sleep(10);
			fail_cnt++;
			if(fail_cnt==10)
				return -1;
			continue;
		}
		fail_cnt = 0;
		#endif

		bufToSend+=sent;
		totalToSend-=sent;
	} while(0!=totalToSend);
	Counters.nSigToSAG++;

	CMessage tmpMsg(pComMsg);
	CMsg_Signal_VCR SAbis1Msg(tmpMsg);
	SAbis1Msg.DoCountTxSignal();
	
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVCR::sendOneSignalToSAG()");	
	return ret;
}
#ifndef DSP_BIOS
CComMessage* CVCR::readOneSignalFromPipe()
{
	CComMessage* pComMsg = NULL;
	int ret;
#ifdef __WIN32_SIM__
	ret = recvfrom(m_fdPipe, (char*)&pComMsg, 4, 0, NULL, NULL);
#else	//vxworks
	ret = read(m_fdPipe, (char*)&pComMsg, 4);
#endif	
	if(ret<0)
	{
		vcrPipeCnt.nReadError++;
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_MSG_EXCEPTION), "Read from Pipe error!!!");
		pComMsg = NULL;
	}
	else
	{
		if(4!=ret)
		{
			vcrPipeCnt.nOutofPipe++;
			vcrPipeCnt.nReadLessBytes++;
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_MSG_EXCEPTION), "Read less than 4 bytes from Pipe, error!!!");
			pComMsg = NULL;
		}
		else
		{
			vcrPipeCnt.nOutofPipe++;
			vcrPipeCnt.nReadOK++;
			UINT16 msgID=0;
			TID srcTid;
			if(pComMsg!=NULL)
			{
				msgID = pComMsg->GetMessageId();
				srcTid = pComMsg->GetSrcTid();
    #ifdef NATAP_FUNCTION
	    			if(MSGID_TMOUT_REG_LOOP<=msgID && 
	    					msgID<=MSGID_TMOUT_HANDSHAKERSP)
	    			{
	    				if(m_pNatpiSession->IsInEffect())
	    				{
	    					m_pNatpiSession->handleNATAPTmoutMsg(msgID);
	    				}
					pComMsg->Destroy();
					pComMsg = NULL;
	    			}
    				else
    #endif		
				if(msgID==MSGID_VOICE_SET_CFG && M_TID_VOICE==srcTid)
				{
					bool blNetCfgChanged = IsNetCfgChanged();//true;
					reloadVcrCfg();
					if(blNetCfgChanged)
					{
						m_blNetCfgChanged = true;
					}
					else
					{
					}
					pComMsg->Destroy();
					pComMsg = NULL;
				}
				else if(msgID==MSGID_VOICE_VCR_RESETLINK && M_TID_VOICE==srcTid)
				{
					m_blNeedResetLink = true;
					pComMsg->Destroy();
					pComMsg = NULL;
				}
				else
				{
					Counters.nSigToVCR++;
					if(M_TID_VOICE==srcTid)
					{
						Counters.nSigFromTvoice++;
					}
					else
					{
						Counters.nSigFromUm++;
					}
				}
			}
		}
	}
	return pComMsg;
}
#endif

bool CVCR::send_TcpFuncTestPkt_Rsp(UINT8 msgType, UINT32 RAND)
{
	#ifdef DSP_BIOS
	int fail_cnt = 0;
	#endif
	UINT8 DataBuf[200];
	UINT8 *buf = &DataBuf[100];
	UINT8 *pHeadFlag = (UINT8*)(buf-sizeof(HeadFlagT));
	
	//构造头部和尾部
	TcpPktHeaderT* pTcpPktHeader = (TcpPktHeaderT*)buf;
	//填写Packet头部
	VSetU16BitVal(pTcpPktHeader->SPC_Code , (m_SPCCode));					//源信令点
	VSetU16BitVal(pTcpPktHeader->DPC_Code , (m_DPCCode));					//目的信令点
	VSetU16BitVal(pTcpPktHeader->UserType , (M_TCP_PKT_TEST_USERTYPE));	//用户类型
	VSetU16BitVal(pTcpPktHeader->TTL , (M_TCP_PKT_DEFAULT_TTL));			//路由计数器，默认值32
	UINT8 DataLen = (M_TCP_PKT_TEST_AUTH_REQ==msgType) ? 
		sizeof(TcpPktTestAuthRspT) : sizeof(TcpPktTestBearHeartRspT);
	VSetU16BitVal(pTcpPktHeader->PktLen , (DataLen+sizeof(TcpPktHeaderT)));
	//填写数据部分
	UINT8* pData = buf + sizeof(TcpPktHeaderT);
	pData[0] = (M_TCP_PKT_TEST_AUTH_REQ==msgType) ? 
		M_TCP_PKT_TEST_AUTH_RSP : M_TCP_PKT_TEST_HEARTBEAT_RSP;

#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		UINT8 NatApLen = m_pNatpiSession->packNATAPMsg4AppPkt(buf,
									DataLen+sizeof(TcpPktHeaderT));
		pHeadFlag = (UINT8*)((UINT8*)pHeadFlag-NatApLen);
	}
#endif

	//Packet开始标志
	VSetU16BitVal(pHeadFlag, M_TCP_PKT_BEGIN_FLAG);		
	//填写Packet尾部Packet结束标志
	UINT8* pEndFlag = (UINT8*)(&pData[DataLen]);
	VSetU16BitVal(pEndFlag, M_TCP_PKT_END_FLAG);
	//发送给SAG
	int sent, totalToSend;
	char *bufToSend = (char*)pHeadFlag;
	totalToSend = (UINT8*)pEndFlag - (UINT8*)pHeadFlag + sizeof(EndFlagT);
	do 
	{
		sent = send(m_fdTcpSag, bufToSend, totalToSend, 0);
		if(sent<0)
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "CVCR::send_TcpFuncTestPkt_Rsp() send error!!!");
			OutputSocketErrCode("send()");
			return false;
		}
		#ifdef DSP_BIOS
		if(sent==0) {
			TSK_sleep(10);
			fail_cnt++;
			if(fail_cnt==10)
				return false;
			continue;
		}
		fail_cnt = 0;
		#endif

		bufToSend+=sent;
		totalToSend-=sent;
	} while(0!=totalToSend);
	return true;
	
}

void CVCR::SendResetMsgToSAG()
{
#ifdef DSP_BIOS
       UINT32 ttemp = getTimeNUll();   
	time_t timer = (time_t) ttemp;
#else
	time_t timer = time(NULL);
#endif
	struct tm TimeStru;
	localtime_r(&timer, &TimeStru);
	
	VoiceVCRCtrlMsgT* pData;
	CMsg_Signal_VCR ResetMsg;
	if(!ResetMsg.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(ResetT)))
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage error!!!");
		return;
	}	
	ResetMsg.SetDstTid(M_TID_VCR);
	ResetMsg.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
	ResetMsg.SetBTSSAGID();
	ResetMsg.SetSigIDS(Reset_MSG);	
	
	pData = (VoiceVCRCtrlMsgT*)ResetMsg.GetDataPtr();
	pData->sigPayload.Reset.RestCause = m_ResetReason;
	VSetU16BitVal(pData->sigPayload.Reset.year , (TimeStru.tm_year+1900));
	pData->sigPayload.Reset.month = TimeStru.tm_mon+1;
	pData->sigPayload.Reset.day = TimeStru.tm_mday;
	pData->sigPayload.Reset.hour = TimeStru.tm_hour;
	pData->sigPayload.Reset.minute = TimeStru.tm_min;
	pData->sigPayload.Reset.second = TimeStru.tm_sec;

	ResetMsg.SetSigHeaderLengthField(sizeof(ResetT));
	ResetMsg.SetPayloadLength(sizeof(SigHeaderT)+sizeof(ResetT));
	if(!ResetMsg.Post())
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Sending Reset to Pipe failed");
	}
	else
	{
//		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Send Reset to Pipe");
	}
	//连接建立后立即发送beatHeart使SAbis AP进入InService状态
	CMsg_Signal_VCR beatHeartMsg;
	if(!beatHeartMsg.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(BeatHeartT)))
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage error!!!");
		return;
	}	
	beatHeartMsg.SetDstTid(M_TID_VCR);
	beatHeartMsg.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
	beatHeartMsg.SetBTSSAGID();
	beatHeartMsg.SetSigIDS(BeatHeart_MSG);	
	
	pData = (VoiceVCRCtrlMsgT*)beatHeartMsg.GetDataPtr();
	pData->sigPayload.BeatHeart.sequence = 0;

	beatHeartMsg.SetSigHeaderLengthField(sizeof(BeatHeartT));
	beatHeartMsg.SetPayloadLength(sizeof(SigHeaderT)+sizeof(BeatHeartT));
	if(!beatHeartMsg.Post())
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Sending HeartBeat to Pipe failed");
	}
	else
	{
//		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Send HeartBeat to Pipe");
	}
}

void CVCR::SendSAGAlarmToOAM(UINT8 SetAlarm)
{
	if(SetAlarm)
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Connection to SAG lost!!!");
	}
	else
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "ReConnected to SAG!!!");
	}
	CComMessage* pAlarm = new (this, 4) CComMessage;
	if(NULL!=pAlarm)
	{
		pAlarm->SetDstTid(M_TID_SYS);
		pAlarm->SetSrcTid(M_TID_VCR);
		pAlarm->SetMessageId(M_OAM_VOICE_SAG_LINK_ALM);
		pAlarm->SetDataLength(4);
		UINT8* pData = (UINT8*)pAlarm->GetDataPtr();
		pData[2] = SetAlarm;
		m_AlarmExist = SetAlarm;
		if(!CComEntity::PostEntityMessage(pAlarm))
		{
			pAlarm->Destroy();
			m_AlarmExist = 0;	//使得下次继续发送告警
		}
	}
}
#ifdef DSP_BIOS
extern "C" UINT32 GetBtsIpAddr();
#endif
UINT32 CVCR::GetBtsSagIpAddr()
{
	UINT32 tmp;
	if(true == m_pBtsSagLinkCfg->SAGVlanUsage)
	{
		tmp = m_pBtsSagLinkCfg->BtsIPAddr;
		LOG1(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "sag vlan  in use, use bts ip : 0x%x",tmp);
	}
	else
	{
		//若EMS配置成不使用SAG VLAN 组网方式则使用bootline里配置的BTS　IP
		tmp = GetBtsIpAddr();
		LOG1(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "sag vlan not in use, use bts sag ip : 0x%x",tmp);
	}
	return tmp;
}

///////////////////////////////////////////////////////////////////////////////



