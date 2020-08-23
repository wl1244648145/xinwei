/*******************************************************************************
* Copyright (c) 2010 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : dataGrpLink.cpp
* Create Date    : 22-Mar-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "DBroadCastMsg.h"
#include "DBroadCastSrv.h"
#include "dataGrpLink.h"
#include "BtsVMsgId.h"
#include "commessage.h"
#include "log.h"
#include "mutex.h"
#include "NatpiApp.h"
#include "stdio.h"
#include "l3oammessageid.h"
#include "sysBtsConfigData.h"
#include "voiceToolFunc.h"
#include "localSagCfg.h"
#include "localSagMsgID.h"

#ifdef DSP_BIOS
extern "C" RESET_REASON bspGetBtsResetReason();
extern  "C" unsigned int  getSysSec();
#define taskDelay(ticks) TSK_sleep(ticks)
#endif
typedef struct _dcsPipeCntT{
    UINT32 nPostCalled;
    UINT32 nPostNull;
    UINT32 nPostFail;
    UINT32 nPostWhenPeerNotConnected;
    UINT32 nEnterPipe;
    UINT32 nOutofPipe;
    UINT32 nFlush;
    UINT32 nReadOK;
    UINT32 nReadError;
    UINT32 nReadLessBytes;
}dcsPipeCntsT;
dcsPipeCntsT dcsPipeCnt;

extern "C" void setDcsRcvBufTraceFlag(bool blShowInfo, bool blShowInfoDetail)
{	
	CDGrpLink::GetInstance()->m_DcsRcvBuf.setShowInfoFlag(blShowInfo);
	CDGrpLink::GetInstance()->m_DcsRcvBuf.setShowDetailFlag(blShowInfoDetail);
}

extern "C" void showDcsPipeStatus();
extern "C" void clrDcsPipeStatus();
extern  unsigned int  getTimeNUll();
void clrDcsPipeStatus()
{
    memset( (void*)&dcsPipeCnt, 0, sizeof(dcsPipeCntsT));
}
void showDcsPipeStatus()
{
    VPRINT("\n========dcsPipeCnt========");
    VPRINT("\n nPostCalled[%d] \n nPostNull[%d] \n nPostFail[%d] \n nPostWhenPeerNotConnected[%d] \n nEnterPipe[%d] \n nOutofPipe[%d] \n nFlush[%d] \n nReadOK[%d] \n nReadError[%d] \n nReadLessBytes[%d]",
        dcsPipeCnt.nPostCalled,
        dcsPipeCnt.nPostNull,
        dcsPipeCnt.nPostFail,
        dcsPipeCnt.nPostWhenPeerNotConnected,
        dcsPipeCnt.nEnterPipe,
        dcsPipeCnt.nOutofPipe,
        dcsPipeCnt.nFlush,
        dcsPipeCnt.nReadOK,
        dcsPipeCnt.nReadError,
        dcsPipeCnt.nReadLessBytes
        );
    VPRINT("\n================\n");    
}


//////////////////////////////////////////////////////////////////////////

//任务实例指针的初始化
CDGrpLink* CDGrpLink::s_ptaskTDGrpLink = NULL;

void CDGrpLink::setTaskOptions()
{
	memcpy(m_szName, M_TASK_DGRP_TASKNAME, strlen( M_TASK_DGRP_TASKNAME ) );
	m_szName[strlen( M_TASK_DGRP_TASKNAME )] = 0;
	strcpy(m_szPipeName, M_DGRP_PIPENAME);
	m_pBtsDcsLinkCfg = &g_vDcsBtsLinkCfg;
	m_pNatpiSession = getDcsNatpiSession();

	m_uPriority = M_TP_L3VCR;
#ifndef DSP_BIOS
	m_uOptions = M_TASK_DGRP_OPTION;
#endif
	m_uStackSize = M_TASK_DGRP_STACKSIZE;
#ifndef DSP_BIOS
	m_lMaxMsgs = M_TASK_DGRP_MAXMSG;
	m_lMsgQOption = M_TASK_DGRP_MSGOPTION;
#else
	m_iMsgQMax = M_TASK_DGRP_MAXMSG;
#endif
	m_fdPipe = 0;
	m_fdTcpPeer = -1;

	m_LocalPort = 0xffff;
	m_PeerPort = 0xffff;
	strcpy(m_PeerIPAddr,"255.255.255.255");

	m_connected=false;
	m_blNetCfgChanged = false;	
	m_blNeedResetLink = false;
}


CDGrpLink::CDGrpLink()
{
}

CDGrpLink::~CDGrpLink()
{
}

TID CDGrpLink::GetEntityId() const
{ 
	return m_selfTID; 
}

#ifdef NATAP_FUNCTION
int OnDcsNatApRegSuccess(void* para)
{
	return 1;
}
#endif

CDGrpLink* CDGrpLink::GetInstance()
{
	if ( NULL == s_ptaskTDGrpLink)
	{
		s_ptaskTDGrpLink = new CDGrpLink;
		if(s_ptaskTDGrpLink!=NULL)
		{
			s_ptaskTDGrpLink->SetEntityId(M_TID_DGRV_LINK);
			s_ptaskTDGrpLink->setTaskOptions();
		}
	}
	return s_ptaskTDGrpLink;
}

bool CDGrpLink::Initialize()
{
	LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "CDGrpLink::Initialize");
	
#ifdef DSP_BIOS
	TSK_Attrs tskAttr;
	tskAttr.stack = NULL;
	tskAttr.stacksize = 0x4000;
	tskAttr.stackseg = TSK->STACKSEG;
	tskAttr.environ = NULL;
	tskAttr.exitflag = TRUE;
	tskAttr.initstackflag = TRUE;	
	tskAttr.priority = M_TP_L3VCR;
	tskAttr.name = "TSK_DcsRx";
	if((TSK_create((Fxn)RunDcsSocketRx, &tskAttr, (int)this))==NULL)
	{
		LOG(LOG_SEVERE,0,"TSK_socketRx taskSpawm failed.");
		return false;
	}

	if (!CBizTask::Initialize())
		return false;
#else
	//create pipe
	if(!CreatePipe())
	{
		LOG(LOG_CRITICAL, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), "CreatePipe() failed.");
		return false;
	}
	//open pipe
	if(!OpenPipe())
	{
		LOG(LOG_CRITICAL, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), "OpenPipe() failed.");
		DeletePipe();
		return false;
	}
#endif
	return true;
}

bool CDGrpLink::IsNetCfgChanged()
{
	char tmpIpAddr[16];
	//IP address of SAG
	UINT32 IP_UINT32 = m_pBtsDcsLinkCfg->DCS_IP;
	UINT8* pIPAddr = (UINT8*)&IP_UINT32;
	sprintf(tmpIpAddr, "%d.%d.%d.%d", pIPAddr[0], pIPAddr[1], pIPAddr[2], pIPAddr[3]);

	if( strcmp(tmpIpAddr, m_PeerIPAddr)==0 && 
		m_PeerPort == m_pBtsDcsLinkCfg->DCS_Port &&
		m_LocalPort == m_pBtsDcsLinkCfg->BTS_Port &&
		m_pBtsDcsLinkCfg->NatApKey == m_pNatpiSession->getSPIVal() )
	{
		return false;
	}
	else
	{
		LOG(LOG_DEBUG, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "tDGrpLink netcfg changed!!!");
		return true;
	}
}

void CDGrpLink::reloadDcsCfg()
{
	m_SPCCode = M_TCP_DEFAULT_SPC_CODE;	//本端信令点编码
	m_DPCCode = M_TCP_DEFAULT_DPC_CODE;	//远端信令点编码

	if(IsNetCfgChanged())
	{
		//IP address of DCS
		UINT32 IP_UINT32 = m_pBtsDcsLinkCfg->DCS_IP;
		UINT8* pIPAddr = (UINT8*)&IP_UINT32;
		sprintf(m_PeerIPAddr, "%d.%d.%d.%d", pIPAddr[0], pIPAddr[1], pIPAddr[2], pIPAddr[3]);
		
		m_PeerPort = m_pBtsDcsLinkCfg->DCS_Port;	//TCP port of DCS  
		m_LocalPort = m_pBtsDcsLinkCfg->BTS_Port;  //local TCP port
		
#ifdef NATAP_FUNCTION
		//In Effct or not
		//m_pNatpiSession->setEffect(true or false);
		if(m_pNatpiSession->IsInEffect())
		{
			m_pNatpiSession->init(GetBtsDcsIpAddr(), 
							m_pBtsDcsLinkCfg->BTS_Port, 
							m_pBtsDcsLinkCfg->DCS_IP, 
							m_pBtsDcsLinkCfg->DCS_Port,
							OnDcsNatApRegSuccess);
			m_pNatpiSession->setSPIVal(m_pBtsDcsLinkCfg->NatApKey);
		}
#endif
	}
}
#ifdef DSP_BIOS
void CDGrpLink::tryToConnectToPeer()
{
	//如果SAG的IP配置为0.0.0.0，则认为不使用语音业务
	while(0==m_pBtsDcsLinkCfg->DCS_IP)
	{
		TSK_sleep(1000);
	}

	reloadDcsCfg();
	//Tcp connection
	while(!ConnectPeer())
	{
		if(!m_AlarmExist)
			SendDcsAlarmToOAM(1);	//alarm occur
		LOG2(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), 
			"ConnectPeer() failed,peerIp:0x%x,port:%d.",
			m_pBtsDcsLinkCfg->DCS_IP,m_pBtsDcsLinkCfg->DCS_Port);
		TSK_sleep(1000);
		reloadDcsCfg();
	}
	SetConnected(true);
	if(m_AlarmExist)
	{
		SendDcsAlarmToOAM(0);	//alarm clear
	}
		
#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		m_pNatpiSession->setSocket(m_fdTcpPeer);
		m_pNatpiSession->start();
	}
	else
	{
		//first msg may send here
	}
#else
	//first msg may send here
#endif

}


STATUS CDGrpLink::RunDcsSocketRx(CDGrpLink *pTask)
{
    ( (CDGrpLink*)pTask )->SocketRxMainLoop();
    return 0;
}

void CDGrpLink::SocketRxMainLoop()
{
	FD_SET ReadFDs;
	int ret;
	struct timeval timer = { TDGrpLink_MAX_BLOCKED_TIME_IN_10ms_TICK/SecondsToTicks(1), 0} ;
	m_AlarmExist = 0;
	tryToConnectToPeer();
		
	//select消息接收处理循环
	while(1)	
	{	
		if(m_fdTcpPeer==-1)
		{
			LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "m_fdTcpPeer==-1!!!");
			handleTcpError();
			tryToConnectToPeer();
			continue;
		}
				
		FD_ZERO(&ReadFDs);		
		FD_SET(m_fdTcpPeer, &ReadFDs);
		int maxFD = m_fdTcpPeer;
		ret = select(maxFD+1, &ReadFDs, NULL, NULL, &timer);

		if(ret==-1)	//error occured
		{
			LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "DGrpLink select() return ERROR!!!");
			handleTcpError();
			tryToConnectToPeer();
			continue;
		}
		//netcfg changed,need to reconnect sag
		if(m_blNetCfgChanged)
		{
			m_blNetCfgChanged = false;
			handleTcpError();
			tryToConnectToPeer();
			continue;
		}
		//when resetLink command received
		if(m_blNeedResetLink)
		{
			m_blNeedResetLink = false;
			handleTcpError();
			tryToConnectToPeer();
			continue;
		}

		if(ret==0)
		{
			LOG1(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "DCS select() return 0,m_fdTcpPeer = %d!!!",m_fdTcpPeer);
			continue;
		}
		else 
		{
			LOG2(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "DCS select() return %d,m_fdTcpPeer = %d!!!", ret, m_fdTcpPeer);
			//int a=1;
		}

		//when peer not connected
		if(!IsConnected())
		{
			m_blNetCfgChanged = false;
			m_blNeedResetLink = false;
			handleTcpError();
			tryToConnectToPeer();
			continue;
		}		
		
		//处理tcp接收的消息,tcp接收如果出错,清除管道中的数据handleTcpError
		if(FD_ISSET(m_fdTcpPeer, &ReadFDs))
		{
			ret = m_DcsRcvBuf.recvData2Buf();
			if(ret<0)
			{
				handleTcpError();
				tryToConnectToPeer();
				continue;
			}
			CComMessage* pComMsg = NULL;
			while( (ret = m_DcsRcvBuf.getOneMsgOutofBuf(&pComMsg))!= M_CONTINUE_RECV)
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

int CDGrpLink::processOneMsgFromPeer(CComMessage *pComMsg)
{
	UINT8 *bufNatPkt;
	UINT16 nNatApLen;
	if(pComMsg==NULL)
	{
		return 0;
	}
	else
	{
		bufNatPkt = (UINT8*)pComMsg->GetDataPtr();
		nNatApLen = pComMsg->GetDataLength();
	}
	NATAP_PKT_TYPE NatApPktType = 
		m_pNatpiSession->authenticateNATAPMsg(bufNatPkt, nNatApLen);
	if(INVALID_NATAP_PKT==NatApPktType)
	{
		pComMsg->Destroy();
		return 0;
	}
	if(APP_PACKET<NatApPktType && NatApPktType<=CLOSE_MSG)
	{
		m_pNatpiSession->handleNATAPMsg(bufNatPkt, nNatApLen);
		pComMsg->Destroy();
		return 0;
	}
	else	//App packet
	{
		//tcp header len
		TcpPktHeaderT* pTcpPktHeader = (TcpPktHeaderT*)(bufNatPkt+
										sizeof(AH_T)+sizeof(NATAP_LinkIDT));
		UINT16 nTcpPktLen = VGetU16BitVal(pTcpPktHeader->PktLen);
		UINT16 nTCPEUserType = VGetU16BitVal(pTcpPktHeader->UserType);

		if(	nTcpPktLen+sizeof(NATAP_LinkIDT)+sizeof(AH_T)!=nNatApLen )
		{
			LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_INVALID_SIGNAL), 
				"invalid packet, tcp header len error!!!");
			pComMsg->Destroy();
			return 0;
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
			pComMsg->Destroy();
			return 0;
		}
		//如果用户类型错，丢弃
		if( nTCPEUserType!=(M_TCP_PKT_DGRP_CTRL_USERTYPE) &&
			nTCPEUserType!=(M_TCP_PKT_DGRP_DATA_USERTYPE) )
		{
			LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_INVALID_SIGNAL), 
				"invalid packet, TCPEUserType error!!!");
			pComMsg->Destroy();
			return 0;
		}

		//合法信令或用户数据
		pComMsg->SetMessageId(MSGID_DCS_VOICE_MSG);
		pComMsg->SetSrcTid(M_TID_DGRV_LINK);
		pComMsg->SetDstTid(M_TID_VOICE);
		pComMsg->SetDataPtr((void*)pTcpPktHeader);
		pComMsg->SetDataLength(nTcpPktLen);
		
		//接收消息统计
		dcsCounters.cntRxSignal[parseSignal(pComMsg)]++;
		
 		return postComMsg(pComMsg);
	}
}

#ifdef DSP_BIOS
extern "C" void mySwitchFxn(int tid,unsigned int inOut,void* pMsg);
#endif
//任务循环
void CDGrpLink::MainLoop()
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
			if(msgID==MSGID_VOICE_SET_CFG)
			{
				bool blNetCfgChanged = IsNetCfgChanged();//true;
				reloadDcsCfg();
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
			else if(msgID==MSGID_VOICE_DCS_RESETLINK && M_TID_VOICE==srcTid)
			{
			#if 1 //by fengbing tmp test 20090922
				m_blNeedResetLink = true;
			#endif 
				pComMsg->Destroy();
				pComMsg = NULL;
			}
			else
			{

			}
		}
	
        if ( NULL == pComMsg )
        {
            continue;
        }
#ifndef NDEBUG
    LOG1(LOG_DEBUG3,0,"Received Message ID=0X%x",pComMsg->GetMessageId());
#endif
        if(0>sendOneMsgToPeer(pComMsg))
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
	while (1) 
	{
		//20091121,fengbing, for test localSag function begin
		#if 0	//should be "#if 0" when create formal version !!!
		while(1)
		{
			taskDelay(1000);
		}
		#endif
		//20091121,fengbing, for test localSag function end
	    	//如果DCS的IP配置为0.0.0.0，则认为不使用数据集群业务
	    	while(0==m_pBtsDcsLinkCfg->DCS_IP)
	    	{
	    		taskDelay(300);
	    	}

		m_AlarmExist = 0;
		reloadDcsCfg();
		//Tcp connection
		while(!ConnectPeer())
		{
			if(!m_AlarmExist)
				SendDcsAlarmToOAM(1);	//alarm occur
			LOG(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), "ConnectPeer() failed.");
			taskDelay(1000);
			reloadDcsCfg();
		}
		SetConnected(true);
		if(m_AlarmExist)
		{
			SendDcsAlarmToOAM(0);	//alarm clear
		}
		
#ifdef NATAP_FUNCTION
		if(m_pNatpiSession->IsInEffect())
		{
			m_pNatpiSession->setSocket(m_fdTcpPeer);
			m_pNatpiSession->start();
		}
		else
		{
			//first msg may send here
		}
#else
			//first msg may send here
#endif

		LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "ConnectPeer() Success.");
		
		//select消息接收处理循环
		while(1)	
		{
			struct timeval timer = { TDGrpLink_MAX_BLOCKED_TIME_IN_10ms_TICK/SecondsToTicks(1), 0} ;
			FD_ZERO(&ReadFDs);
			FD_SET(m_fdPipe, &ReadFDs);		
			FD_SET(m_fdTcpPeer, &ReadFDs);
			int maxFD = max(m_fdPipe,m_fdTcpPeer);
			ret = select(maxFD+1, &ReadFDs, NULL, NULL, &timer);
			if(ret==-1)	//error occured
			{
				LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "DGrpLink select() return ERROR!!!");
				handleTcpError();
				break;
			}
			//先处理管道中的数据,tcp发送如果出错,清除管道中的消息handleTcpError
			if(FD_ISSET(m_fdPipe, &ReadFDs))
			{
				CComMessage *pComMsg = readOneMsgFromPipe();
				
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
				
				if(0>sendOneMsgToPeer(pComMsg))
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
			if(FD_ISSET(m_fdTcpPeer, &ReadFDs))
			{
				ret = m_DcsRcvBuf.recvData2Buf();
				if(ret<0)
				{
					handleTcpError();
					break;
				}
				CComMessage* pComMsg = NULL;
				while( (ret = m_DcsRcvBuf.getOneMsgOutofBuf(&pComMsg))!= M_CONTINUE_RECV)
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

//往任务的管道中发送消息
bool CDGrpLink::PostMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent)
{
	dcsPipeCnt.nPostCalled++;
	if(NULL==pMsg)
	{
		dcsPipeCnt.nPostNull++;
		LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "write null msg to pipe.");
		return false;
	}

	if (!IsConnected()) 
	{
		dcsPipeCnt.nPostWhenPeerNotConnected++;
		LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "write msg to pipe when DCS disconnected, discard msg.");
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
	fsin.sin_port= htons(M_WIN32_DGRP_PIPEPORT);
	fsin.sin_addr.s_addr=inet_addr("127.0.0.1");

	int ret = sendto (m_fdUdpSndToPipe, (char *)&pMsg, sizeof(pMsg), 0,
		(struct sockaddr *)&fsin,
		sizeof(struct sockaddr));
#else
	int ret = write(m_fdPipe, (char*)&pMsg, sizeof(pMsg));
#endif
	if(ret==sizeof(pMsg))
	{
		dcsPipeCnt.nEnterPipe++;
		return true;
	}
	else
	{
		dcsPipeCnt.nPostFail++;
		LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "writing msg to pipe failed.");
		//pMsg->Destroy();
		return false;
	}
#endif//#ifdef DSP_BIOS
}
//创建socket
bool CDGrpLink::CreateSocket()
{
    sockaddr_in server;
	
    m_fdTcpPeer= ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_fdTcpPeer<0)
    {
        LOG(LOG_CRITICAL, LOGNO(DGRPSRV, EC_L3VOICE_SOCKET_ERR), "Create socket failed.");
        return false;
    }
#ifndef __USE_LWIP__
	int ret;
	struct linger Linger={1,0};
	int yes = 1;	
	UINT32 on = 1;
	ret = setsockopt(m_fdTcpPeer, SOL_SOCKET, SO_LINGER, (char *)&Linger, sizeof(Linger));
	//tcp no delay option
	ret = setsockopt(m_fdTcpPeer, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(yes));
	//keep alive option
	ret = setsockopt(m_fdTcpPeer, SOL_SOCKET, SO_KEEPALIVE, (char *)&yes, sizeof(yes));
	
#ifdef __WIN32_SIM__
//	if(ioctlsocket(m_fdTcpPeer, FIONBIO, (u_long*)&on) < 0) 
#else
	if(ioctl(m_fdTcpPeer, FIONBIO, (int)&on) < 0) 
	{ 
		LOG(LOG_CRITICAL, LOGNO(DGRPSRV, EC_L3VOICE_SOCKET_ERR), "make socket unblock fail.");
		OutputSocketErrCode("ioctl(m_fdTcpPeer, FIONBIO, (u_long*)&on)");
	} 
#endif


	//addr reuse
	//ret = setsockopt (m_fdTcpPeer, SOL_SOCKET, SO_REUSEADDR, (char *)0, 0); 	
	ret = setsockopt (m_fdTcpPeer, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)); 	
#endif//#ifndef __USE_LWIP__
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(m_LocalPort);//bts的tcp端口绑定指定的端口	
    if (::bind(m_fdTcpPeer, (sockaddr*)&server, sizeof(server))<0)
    {
        LOG(LOG_CRITICAL, LOGNO(DGRPSRV, EC_L3VOICE_SOCKET_ERR), "bind socket failed.");
		OutputSocketErrCode("bind(m_fdTcpPeer, (sockaddr*)&server, sizeof(server))");
        DisconnectPeer();
        return false;
    }

    return true;
}
//建立TCP连接,阻塞调用保证知道是否连接成功
bool CDGrpLink::ConnectPeer()
{
	DisconnectPeer();
	CreateSocket();
	LOG2(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), 
		"ConnectPeer() ,peerIp:0x%x,port:%d.",
		m_pBtsDcsLinkCfg->DCS_IP,m_pBtsDcsLinkCfg->DCS_Port);
	
	struct sockaddr_in fsin;
	fsin.sin_family = AF_INET;
	fsin.sin_port= htons(m_PeerPort);
	fsin.sin_addr.s_addr=inet_addr(m_PeerIPAddr);
#ifndef WBBU_CODE //其他代码
#ifdef __USE_LWIP__
	if( connect(m_fdTcpPeer, (struct sockaddr *) &fsin ,sizeof(fsin))<0 )
#else
#ifdef __VXWORKS__
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	if( connectWithTimeout(m_fdTcpPeer, (struct sockaddr *) &fsin , sizeof(fsin), &timeout)<0 )
#endif
#endif
	{
		//LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_SOCKET_ERR), "ConnectPeer() error!!!");
		//OutputSocketErrCode("ConnectPeer()");				
		return false;
	}	 
#else //WBBU代码
	int result =0;
	int err;
	char connect_ok = 0;
	result = ( connect(m_fdTcpPeer, (struct sockaddr *) &fsin ,sizeof(fsin)));
	if( result!=OK )
	{
		err = errno;
		if (err != EINPROGRESS)
		{
			LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), 
					"ConnectPeer() error!!!err[%d], err != EINPROGRESS", err);
		}
		else
		{
			time_t tmBegin = time(NULL);
			while(1)
			{
				connect(m_fdTcpPeer, (struct sockaddr *) &fsin ,sizeof(fsin));
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
						"ConnectPeer() normal! err[%d] connect_ok[%d]", 
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
//断开TCP连接
bool CDGrpLink::DisconnectPeer()
{
#ifndef __USE_LWIP__
	shutdown(m_fdTcpPeer,2);
#endif
	if(-1!=m_fdTcpPeer) 
	{
#ifdef __WIN32_SIM__
		closesocket(m_fdTcpPeer);
#else
		close(m_fdTcpPeer);
#endif	
	}
	m_fdTcpPeer= -1;

#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		m_pNatpiSession->setSocket(m_fdTcpPeer);
		m_pNatpiSession->stop();
	}
#endif

	return true;
}

//创建Pipe
bool CDGrpLink::CreatePipe()
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
    server.sin_port = htons(M_WIN32_DGRP_PIPEPORT);
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
bool CDGrpLink::OpenPipe()
{
#ifndef DSP_BIOS
    clrDcsPipeStatus();
#ifdef __WIN32_SIM__
	//return true;
#else		//vxworks
	m_fdPipe = open(m_szPipeName, O_RDWR, 0644);
	if(ERROR==m_fdPipe)
    {
        LOG(LOG_CRITICAL, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), "DGrpLink open pipe error!!!");
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
		LOG(LOG_CRITICAL, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), "make pipe unblock failed!!!");
		return false;
	}

    return true;
#else
	return true;
#endif
}
//删除Pipe
bool CDGrpLink::DeletePipe()
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
bool CDGrpLink::ClosePipe()
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

bool CDGrpLink::IsConnected()
{
	bool ret;
	//lock
	m_lock.Wait();
	ret = m_connected;
	//unlock
	m_lock.Release();
	return ret;
}
void CDGrpLink::SetConnected(bool connected)
{
	//lock
	m_lock.Wait();
	m_connected = connected;
	//unlock
	m_lock.Release();

	if(connected)
	{
		//向tDCS(tSag)发送退出服务消息
		sendNopayloadMsg(CTVoice::GetInstance(), M_TID_SAG, GetEntityId(), MSGID_BTS_DCS_STOP_SRV);
		m_DcsRcvBuf.resetRcvBuf();
		m_DcsRcvBuf.setFd(m_fdTcpPeer);
	}
	else
	{
		//停止数据集群业务
		sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, GetEntityId(), MSGID_DGRPLINK_VOICE_RELEASE_DGRP_SRV);
	}
}

//处理掉管道中已有的消息
bool CDGrpLink::FlushPipe()
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
	    			pComMsg=readOneMsgFromPipe();
	    			if(pComMsg!=NULL)
	    			{
					dcsPipeCnt.nFlush++;
	    				pComMsg->Destroy();
	    			}
	    		}	
			else
	    		{
				//read this invalid msg
				readOneMsgFromPipe();
	    		}
    		}	
    		else
		{
			if(0==nBytesUnread)
			{
				LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "Flush pipe , pipe empty!!!");
			}
			else if(ret==ERROR)
			{
				return false;
				LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "ioctl (m_fdPipe, FIONREAD, (int)&nBytesUnread) return ERROR!!!");
			}
			else//nBytesUnread<0
			{
				LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "ioctl (m_fdPipe, FIONREAD, (int)&nBytesUnread) nBytesUnread<0!!!");
			}
			return true;
		}
	}
#else
	return true;
#endif

}

void CDGrpLink::handleTcpError()
{
	//不再往管道中发送消息
	SetConnected(false);
	//处理掉管道中已有的消息
	FlushPipe();
	//断开TCP连接
	DisconnectPeer();
	if(!m_AlarmExist)
		SendDcsAlarmToOAM(1);	//alarm occur

	//避免配置错误时对DCS形成连接风暴
	taskDelay(1000);	
}

//send one msg to Peer, return true when success, otherwise return false
int CDGrpLink::sendOneMsgToPeer(CComMessage* pComMsg)
{
	#ifdef DSP_BIOS
	int fail_cnt =0;
	#endif
	if(NULL==pComMsg)
		return 0;
	if(pComMsg->GetMessageId()!=MSGID_VOICE_DCS_MSG)	
		return 0;
	
	//构造头部和尾部
	UINT8* pData = (UINT8*)pComMsg->GetDataPtr();
	TcpPktHeaderT* pTcpPktHeader = (TcpPktHeaderT*)(pData);
	UINT16 SigDataLen = pComMsg->GetDataLength();
	//填写Packet头部
	VSetU16BitVal(pTcpPktHeader->SPC_Code , (m_SPCCode));					//源信令点
	VSetU16BitVal(pTcpPktHeader->DPC_Code , (m_DPCCode));					//目的信令点
	//VSetU16BitVal(pTcpPktHeader->UserType , (M_TCP_PKT_XXXXXX));	//用户类型
	VSetU16BitVal(pTcpPktHeader->TTL , (M_TCP_PKT_DEFAULT_TTL));			//路由计数器，默认值32
	//VSetU16BitVal(pTcpPktHeader->PktLen , (XXXXX));

	UINT8* pBeginFlag = (UINT8*)((UINT8*)pTcpPktHeader-sizeof(HeadFlagT));

#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		UINT8 NatApPktLen = 
			m_pNatpiSession->packNATAPMsg4AppPkt((UINT8*)pTcpPktHeader, SigDataLen);

		pBeginFlag = (UINT8*)((UINT8*)pBeginFlag-NatApPktLen);
	}
#endif

	//Packet开始标志
	VSetU16BitVal(pBeginFlag, M_TCP_PKT_BEGIN_FLAG);
	//填写Packet尾部Packet结束标志
	UINT8* pEndFlag = (UINT8*)(&pData[SigDataLen]);
	VSetU16BitVal(pEndFlag, M_TCP_PKT_END_FLAG);
	//发送给Peer
	int sent, totalToSend, ret;
	char *bufToSend = (char*)pBeginFlag;
	ret = totalToSend = (UINT8*)pEndFlag - (UINT8*)pBeginFlag
						+sizeof(EndFlagT);
	do 
	{
		sent = send(m_fdTcpPeer, bufToSend, totalToSend, 0);
		if(sent<0)
		{
			LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_SOCKET_ERR), "CDGrpLink::sendOneMsgToPeer() send error!!!");
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

	//发送消息统计
	dcsCounters.cntTxSignal[parseSignal(pComMsg)]++;
	
	LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "CDGrpLink::sendOneMsgToPeer()");	
	return ret;
}
#ifndef DSP_BIOS
CComMessage* CDGrpLink::readOneMsgFromPipe()
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
		dcsPipeCnt.nReadError++;
		LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_MSG_EXCEPTION), "Read from Pipe error!!!");
		pComMsg = NULL;
	}
	else
	{
		if(4!=ret)
		{
			dcsPipeCnt.nOutofPipe++;
			dcsPipeCnt.nReadLessBytes++;
			LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_MSG_EXCEPTION), "Read less than 4 bytes from Pipe, error!!!");
			pComMsg = NULL;
		}
		else
		{
			dcsPipeCnt.nOutofPipe++;
			dcsPipeCnt.nReadOK++;
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
				if(msgID==MSGID_VOICE_SET_CFG)
				{
					bool blNetCfgChanged = IsNetCfgChanged();//true;
					reloadDcsCfg();
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
				else if(msgID==MSGID_VOICE_DCS_RESETLINK && M_TID_VOICE==srcTid)
				{
					m_blNeedResetLink = true;
					pComMsg->Destroy();
					pComMsg = NULL;
				}
				else
				{
					
				}
			}
		}
	}
	return pComMsg;
}
#endif

bool CDGrpLink::send_TcpFuncTestPkt_Rsp(UINT8 msgType, UINT32 RAND)
{
	#ifdef DSP_BIOS
	int fail_cnt =0;
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
	//发送给Peer
	int sent, totalToSend;
	char *bufToSend = (char*)pHeadFlag;
	totalToSend = (UINT8*)pEndFlag - (UINT8*)pHeadFlag + sizeof(EndFlagT);
	do 
	{
		sent = send(m_fdTcpPeer, bufToSend, totalToSend, 0);
		if(sent<0)
		{
			LOG(LOG_DEBUG3, LOGNO(DGRPSRV, EC_L3VOICE_SOCKET_ERR), "CDGrpLink::send_TcpFuncTestPkt_Rsp() send error!!!");
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

void CDGrpLink::SendDcsAlarmToOAM(UINT8 SetAlarm)
{
#if 0	
	if(SetAlarm)
	{
		LOG(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "Connection to DCS lost!!!");
	}
	else
	{
		LOG(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_NORMAL), "ReConnected to DCS!!!");
	}
	CComMessage* pAlarm = new (this, 4) CComMessage;
	if(NULL!=pAlarm)
	{
		pAlarm->SetDstTid(M_TID_SYS);
		pAlarm->SetSrcTid(M_TID_DGRV_LINK);
		pAlarm->SetMessageId(XXXXXX);
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
#endif	
}
#ifdef DSP_BIOS
extern "C" UINT32 GetBtsIpAddr();
#endif
UINT32 CDGrpLink::GetBtsDcsIpAddr()
{
	return GetBtsIpAddr();
}

///////////////////////////////////////////////////////////////////////////////


