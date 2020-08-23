/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    NatPi.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   2006-11-27 fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/
#include <time.h>
#include "tVoice.h"
#include "NatPi.h"
#include "log.h"
#include "NatAuth.h"
#include  "voiceToolFunc.h"

#ifndef __WIN32_SIM__
#ifdef __VXWORKS__
#include "sockLib.h"
#include "errnoLib.h"
#include "inetLib.h" 
#endif
#endif

#ifndef M_TCP_PKT_BEGIN_FLAG
#define M_TCP_PKT_BEGIN_FLAG (0x7ea5)
#endif
#ifndef M_TCP_PKT_END_FLAG
#define M_TCP_PKT_END_FLAG (0x7e0d)
#endif

bool g_blHandShakeFlag = true;
void setNatApHandShakeFlag(bool flag)
{
	g_blHandShakeFlag = flag;
}

void OutputSocketErrCode(char *p)
{
#ifndef __USE_LWIP__
#ifdef __WIN32_SIM__
	int err_code = WSAGetLastError();
#else	//vxworks
	int err_code = errnoGet();
#endif
	if(NULL!=p)
    {
        LOG2(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), 
			"%s failed.errno=[%d]", (int)p, err_code);
    }
	else
    {
        LOG2(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), 
			"%s failed.errno=[%d]", (int)"socket operation", err_code);
    }
#endif
}

UINT8 g_MaxHandShakeLost = M_MAX_HANDSHAKE_UNREPLY;

char g_natTimerName[TIMER_NAT_COUNT][30]=
{
	"TIMER_NAT_REGLOOP",
	"TIMER_NAT_REGRSP",
	"TIMER_NAT_HANDSHAKE",
	"TIMER_NAT_HANDSHAKERSP"
};

TimerCfgItemT g_timerCfgTbl[]=
{
	{	
		TIMER_NAT_REGLOOP, 			MSGID_TMOUT_REG_LOOP, 
		M_TIMEOUT_NAT_REGLOOP, 		NULL
	},
	{	
		TIMER_NAT_REGRSP, 			MSGID_TMOUT_REG_RSP, 
		M_TIMEOUT_NAT_REGRSP, 		NULL
	},
	{	
		TIMER_NAT_HANDSHAKE, 		MSGID_TMOUT_HANDSHAKE, 
		M_TIMEOUT_NAT_HANDSHAKE,	NULL
	},
	{	
		TIMER_NAT_HANDSHAKERSP, 	MSGID_TMOUT_HANDSHAKERSP, 
		M_TIMEOUT_NAT_HANDSHAKERSP,	NULL
	}
};

void NatPiSession::setSPIVal(UINT8 spiV)
{
	if(spiV==0 || spiV>31)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
			"NatAP Key error!!! use default value[1]");
		m_nSPI = 1;
	}
	else
	{
		m_nSPI = spiV;
	}
}

void NatPiSession::init(UINT32 localIP, UINT16 localPort,
						UINT32 peerIP, UINT16 peerPort,
						RegSuccessCallBack cbFunc)
{
	srand((unsigned int)time((time_t *)NULL));
	//暂时初始值定为0x8023,今后可能根据OAM配置
	UINT32 initRandomVAL = 0x8923;	
	m_fdSocket = -1;

	clearCounters();
	clearAllTimers();
	initAllTimers();
	
	setRegisteredFlag(false);
	setLinkID(M_NAT_INVALID_LINKID);
	setLocalAHSN(initRandomVAL);	//认证序号初始化为随机数
	setPeerAHSN(0xffffffff);
	setTransSN(0);
	setHandShakeSN(0);
	
	setLocalIP(localIP);
	setLocalPort(localPort);
	setPeerIP(peerIP);			//peer ip
	setPeerPort(peerPort);		//peer port
	updateRemoteAddr();
	
	clearHSLostCounter();
	m_callbackFuncRegSuccess = cbFunc;

	m_nNoNatApActionTimes = 0;

}

void NatPiSession::start()
{
	sendRegMsg();

	m_nNoNatApActionTimes = 0;
}

void NatPiSession::stop()
{
	int i;
	for(i=0;i<TIMER_NAT_COUNT;i++)
		stopTimer((NATTimerID)i);
	setRegisteredFlag(false);
	clearHSLostCounter();
}

void NatPiSession::initAllTimers()
{
#if 0
	int i;
	for(i=0;i<TIMER_NAT_COUNT;i++)
	{
		//new timeout msg
		g_timerCfgTbl[i].pTimeoutMsg = 
				new (CTVoice::GetInstance(),16) CComMessage;
		if(NULL==g_timerCfgTbl[i].pTimeoutMsg)
		{
			LOG1( LOG_CRITICAL, 
				 LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
				 "NATAP[%s]: New CComMessage error!!!",
				 (int)getName());
			m_pTimer[i] = NULL;
		}
		else
		{
			g_timerCfgTbl[i].pTimeoutMsg->SetSrcTid(M_TID_VOICE);
			g_timerCfgTbl[i].pTimeoutMsg->SetDstTid(m_dstTaskID);
			g_timerCfgTbl[i].pTimeoutMsg->SetMessageId(g_timerCfgTbl[i].msgID);


			//new timer			
			m_pTimer[i] = new CTimer( false, 
								  		g_timerCfgTbl[i].timeoutVal, 
								  		g_timerCfgTbl[i].pTimeoutMsg );
			if(NULL==m_pTimer[i])
			{
				LOG1( LOG_CRITICAL, 
				 	 LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
				 	 "NATAP[%s]: New CTimer error!!!",
				 	 (int)getName());
			}	
			
		}
	}
	LOG1( LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		 "NATAP[%s]: initAllTimers()",
		 (int)getName());

#endif
	
}

void NatPiSession::clearAllTimers()
{
	int i;
	LOG1( LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		 "NATAP[%s]: clearAllTimers()",
		 (int)getName());
	for(i=0;i<TIMER_NAT_COUNT;i++)
	{
		g_timerCfgTbl[i].pTimeoutMsg = NULL;
		deleteTimer((NATTimerID)i);
	}
}

void NatPiSession::startTimer(NATTimerID timerID)
{
	deleteTimer(timerID);
	//new timeout msg
	CComMessage* pTmoutMsg = new (CTVoice::GetInstance(),16) CComMessage;
	if(NULL==pTmoutMsg)
	{
		LOG1( LOG_CRITICAL, 
			 LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
			 "NATAP[%s]: New CComMessage error!!!",
			 (int)getName());
		m_pTimer[timerID] = NULL;
		return;
	}
	else
	{
		pTmoutMsg->SetSrcTid(m_dstTaskID);
		pTmoutMsg->SetDstTid(M_TID_VOICE);
		pTmoutMsg->SetMessageId(g_timerCfgTbl[timerID].msgID);
	}
	
	//new timer			
	m_pTimer[timerID] = new CTimer( false, 
					  		g_timerCfgTbl[timerID].timeoutVal, 
					  		pTmoutMsg );
	if(NULL==m_pTimer[timerID])
	{
		LOG1( LOG_CRITICAL, 
		 	 LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
		 	 "NATAP[%s]: New CTimer error!!!",
		 	 (int)getName());
	}	

	if(m_pTimer[timerID]!=NULL)
	{
		//重新取得超时时间，因为也许更新了定时器时长
		m_pTimer[timerID]->SetInterval(g_timerCfgTbl[timerID].timeoutVal);
		bool ret = m_pTimer[timerID]->Start();
		if(!ret)
		{
			LOG2( LOG_SEVERE, 
				  LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
				  "NATAP[%s]: startTimer[%s] fail!!!!!!!!", 
				  (int)getName(),
				  (int)g_natTimerName[timerID]);
		}
		LOG2( LOG_DEBUG2, 
			  LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			  "NATAP[%s]: NATStartTimer[%s]", 
			  (int)getName(),
			  (int)g_natTimerName[timerID]);		
	}
}
void NatPiSession::stopTimer(NATTimerID timerID)
{
	if(m_pTimer[timerID]!=NULL)
	{
		m_pTimer[timerID]->Stop();
		delete m_pTimer[timerID];
		m_pTimer[timerID] = NULL;
		LOG2( LOG_DEBUG2, 
			  LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			  "NATAP[%s]: NATStopTimer[%s]",
			  (int)getName(),
			  (int)g_natTimerName[timerID]);			
	}
}

void NatPiSession::deleteTimer(NATTimerID timerID)
{
	if(m_pTimer[timerID]!=NULL)
	{
		delete (m_pTimer[timerID]);
		m_pTimer[timerID] = NULL;
		LOG2( LOG_DEBUG2, 
			  LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			  "NATAP[%s]: NATDeleteTimer[%s]", 
			  (int)getName(),
			  (int)g_natTimerName[timerID]);			
	}	
}

void NatPiSession::updateRemoteAddr()
{
	//准备好UDP发送地址
	m_RemoteAddr.sin_family = AF_INET;
	m_RemoteAddr.sin_port= htons(m_peerPort);
	m_RemoteAddr.sin_addr.s_addr=m_peerIP;
}
int NatPiSession::sendNatPiPkt(UINT8 *pDataToSend, UINT16 nLenToSend)
{
	#ifdef DSP_BIOS
		int fail_cnt = 0;
	#endif

	AH_T  *pAHHead = (AH_T*)pDataToSend;
	int sent=0;
	if(m_fdSocket!=-1 && nLenToSend>0 && pDataToSend!=NULL)
	{
		if(isTcpApp())	//tcp
		{
			//发送给SAG
			int totalToSend=nLenToSend+2+2;
			char bufToSend[2000];
			char *pBufToSend = bufToSend;
			memcpy( (void*)(bufToSend+2), (void*)pDataToSend, nLenToSend);
			UINT8* pHeadFlag = (UINT8*)bufToSend;
			UINT8* pEndFlag = (UINT8*)(bufToSend + 2 + nLenToSend);
			VSetU16BitVal(pHeadFlag, M_TCP_PKT_BEGIN_FLAG);
			VSetU16BitVal(pEndFlag, M_TCP_PKT_END_FLAG);
			do 
			{
				sent = send(m_fdSocket, pBufToSend, totalToSend, 0);
				if(sent<0)
				{
					LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
						"NATAP[%s]: sendNatPiPkt send error!!!",
						(int)getName());
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

				pBufToSend+=sent;
				totalToSend-=sent;
			} while(0!=totalToSend);
			m_nSndNatApPktCounters[pAHHead->msgType]++;
			return nLenToSend;
		}
		else	//udp
		{
			sent = sendto( m_fdSocket, (char*)pDataToSend, nLenToSend, 0, 
							(struct sockaddr *)&m_RemoteAddr, 
							sizeof(m_RemoteAddr));
			if(sent<0)
			{
				LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
						"NATAP[%s]: sendNatPiPkt send error!!!",
						(int)getName());
				OutputSocketErrCode("sendto()");
			}
			else
			{
				m_nSndNatApPktCounters[pAHHead->msgType]++;
			}
			return sent;
		}			
	}
	else
	{
		return 0;
	}
}

void NatPiSession::sendRegMsg()
{
	NAT_Reg_Pkt_T natRegMsg;
	UINT16 nNatApPktLen;
//AHHead
	natRegMsg.AHHead.protocolID = M_PROTOCOL_ID_NATAP;
	//random spi,[1,spicount-1]
	//natRegMsg.AHHead.SPI = ((rand() % (NATAP_SPI_COUNT-1))+1);
	natRegMsg.AHHead.SPI = getSPIVal();
	//长度包括认证字段
	nNatApPktLen = (UINT8*)natRegMsg.AuthResult - (UINT8*)&natRegMsg
				   +NATAPGetAuthResultLength(natRegMsg.AHHead.SPI);
	VSetU16BitVal(natRegMsg.AHHead.nLen, nNatApPktLen);
	natRegMsg.AHHead.msgType = REG_MSG;
//AuthSN
	//incLocalAHSN();	//目前实际只用到初始值
	VSetU32BitVal(natRegMsg.AuthSN, getLocalAHSN());
//regData
	VSetU16BitVal(natRegMsg.NatRegData.transID, getTransSN());
	VSetU32BitVal(natRegMsg.NatRegData.localIP, getLocalIP());
	VSetU16BitVal(natRegMsg.NatRegData.localPort, getLocalPort());
//AuthResult
	//发送前对报文内容（从协议标识开始至报文内容截止，包括认证序号）
	//按加密算法加密，结果写入认证字段	
	NATAPComputeAuthResult( natRegMsg.AHHead.SPI, 
							(UINT8*) &natRegMsg, 
							//(UINT8*)&natRegMsg.NatRegData - (UINT8*)&natRegMsg,
							(UINT8*) natRegMsg.AuthResult- (UINT8*)&natRegMsg,
							(UINT8*) natRegMsg.AuthResult);

//send to Peer
	sendNatPiPkt((UINT8*)&natRegMsg, nNatApPktLen);
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: send Register Msg",
		(int)getName());
//start timers
	startTimer(TIMER_NAT_REGRSP);
	startTimer(TIMER_NAT_REGLOOP);
}

void NatPiSession::sendHandShakeMsg()
{
	NAT_HandShake_Pkt_T natHandShakeMsg;
	//AHHead
	natHandShakeMsg.AHHead.protocolID = M_PROTOCOL_ID_NATAP;
	natHandShakeMsg.AHHead.SPI = 0;
	VSetU16BitVal(natHandShakeMsg.AHHead.nLen, sizeof(NAT_HandShake_Pkt_T));
	natHandShakeMsg.AHHead.msgType = HANDSHAKE_MSG;
	//data
	VSetU16BitVal(natHandShakeMsg.NatHandShakeData.linkID, getLinkID());
	VSetU32BitVal(natHandShakeMsg.NatHandShakeData.SN, getHandShakeSN());
	//send to Peer
	sendNatPiPkt((UINT8*)&natHandShakeMsg, sizeof(NAT_HandShake_Pkt_T));
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: send HandShake Msg",
		(int)getName());	
	//start timer for response timeout and timer for the next handshake
	startTimer(TIMER_NAT_HANDSHAKERSP);
	startTimer(TIMER_NAT_HANDSHAKE);
}

void NatPiSession::sendHandShakeRspMsg(UINT32 peerSN)
{
	NAT_HandShakeRsp_Pkt_T natHandShakeRspMsg;
	//AHHead
	natHandShakeRspMsg.AHHead.protocolID = M_PROTOCOL_ID_NATAP;
	natHandShakeRspMsg.AHHead.SPI = 0;
	VSetU16BitVal(natHandShakeRspMsg.AHHead.nLen, sizeof(NAT_HandShakeRsp_Pkt_T));
	natHandShakeRspMsg.AHHead.msgType = HANDSHAKE_RSP_MSG;
	//data
	VSetU16BitVal(natHandShakeRspMsg.NatHandShakeRspData.linkID, getLinkID());
	VSetU32BitVal(natHandShakeRspMsg.NatHandShakeRspData.SN, peerSN);
	//send to Peer
	sendNatPiPkt((UINT8*)&natHandShakeRspMsg, sizeof(NAT_HandShakeRsp_Pkt_T));
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: send HandShakeRsp Msg",
		(int)getName());	
}

void NatPiSession::handleRegRspMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: Recv RegisterRsp Msg",
		(int)getName());
	
	stopTimer(TIMER_NAT_REGRSP);	//stop timer for regRsp timeout
	NAT_RegRsp_Pkt_T *pNatRegRspMsg = (NAT_RegRsp_Pkt_T*)pMsg;
	if(NATAP_REG_SUCCESS==pNatRegRspMsg->NatRegRspData.result)
	{//success
		setRegisteredFlag(true);
		setLinkID(VGetU16BitVal(pNatRegRspMsg->NatRegRspData.linkID));
		stopTimer(TIMER_NAT_REGLOOP);	//stop timer for regLoop timeout
		clearHSLostCounter();
		startTimer(TIMER_NAT_HANDSHAKE);//start timer for handshake loop

		//run callback function
		if(NULL!=m_callbackFuncRegSuccess)
		{
			(*m_callbackFuncRegSuccess)(NULL);
		}

		LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"NATAP[%s]: Register Success##########################",
			(int)getName());
	}
	else
	{//fail
		//when timer for regLoop timeout,reg will be sent periodicly
		setRegisteredFlag(false);
		LOG2(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"NATAP[%s]: Register failed !!! Result[%d]",
			(int)getName(),
			pNatRegRspMsg->NatRegRspData.result);		
	}
}

void NatPiSession::handleHandShakeMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: Recv HandShake Msg",
		(int)getName());
	if(!IsServer())
	{
		if(!isRegistered())
		{
			//未注册状态下收到握手包不回应，尽快使远端失连，接收新注册
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"NATAP[%s]: HandShake received when not Registered, discard!!!",
				(int)getName());
			return;
		}	
	}

	
	NAT_HandShake_Pkt_T* pHandShakeMsg = (NAT_HandShake_Pkt_T*)pMsg;
	UINT32 handshakeSN = VGetU32BitVal(pHandShakeMsg->NatHandShakeData.SN);
	sendHandShakeRspMsg(handshakeSN);
}

void NatPiSession::handleHandShakeRspMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: Recv HandShakeRsp Msg",
		(int)getName());	
	stopTimer(TIMER_NAT_HANDSHAKERSP);
	clearHSLostCounter();	//清除握手未应答次数
}

//only for udp
void NatPiSession::handleCloseMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: Recv Close Msg##########################",
		(int)getName());
	
	setRegisteredFlag(false);
	stopTimer(TIMER_NAT_HANDSHAKERSP);
	stopTimer(TIMER_NAT_HANDSHAKE);
	stopTimer(TIMER_NAT_REGRSP);
	stopTimer(TIMER_NAT_REGLOOP);

	sendRegMsg();					//转入注册流程	
}

//now only for tcp, return the length of the NATAP packet, include protocol ID
//pDataApp point to the beginning of APP layer data
UINT8 NatPiSession::packNATAPMsg4AppPkt(UINT8* pDataApp, UINT16 len)
{
	//1)NATAP AHhead
	//2)LinkID
	UINT16 nNatApPktLen = sizeof(AH_T)+
						  sizeof(NATAP_LinkIDT);
	NAT_App_Pkt_T* pNat_AppPkt = (NAT_App_Pkt_T*)(pDataApp 
								 - nNatApPktLen);
	//AHHead
	pNat_AppPkt->AHHead.protocolID = M_PROTOCOL_ID_NATAP;
	pNat_AppPkt->AHHead.SPI = 0;
	VSetU16BitVal(pNat_AppPkt->AHHead.nLen, nNatApPktLen+len);//包括Protocol和SPI
	pNat_AppPkt->AHHead.msgType = APP_PACKET;
	//LinkID
	VSetU16BitVal(pNat_AppPkt->linkID, getLinkID());
	m_nSndNatApPktCounters[APP_PACKET]++;
	return nNatApPktLen;
}

void NatPiSession::handleTmoutRegRsp()
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: wait RegRsp timeout!!!",
		(int)getName());
	stopTimer(TIMER_NAT_REGRSP);
	//reg fails,when timer regLoop expires, next reg will be sent
	setRegisteredFlag(false);
}

void NatPiSession::handleTmoutRegLoop()
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: Reg period timeout!!!",
		(int)getName());
	stopTimer(TIMER_NAT_REGLOOP);
	sendRegMsg();	//send reg msg
}

void NatPiSession::handleTmoutHandShakeRsp()
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: wait HandShakeRsp timeout!!!",
		(int)getName());
	stopTimer(TIMER_NAT_HANDSHAKERSP);
	if(g_blHandShakeFlag)
	{
		incHSLostCounter(); //握手未应答次数+1
	}
	if(getHSLostCounter()>=g_MaxHandShakeLost)
	{
		stopTimer(TIMER_NAT_HANDSHAKE);	//结束握手流程
		setRegisteredFlag(false);		//标记为未注册
		sendRegMsg();					//转入注册流程
		LOG2(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			 "NATAP[%s]: %d or more HandShakes unreplyed, set UnRegistered!!!",
			 (int)getName(),
			 g_MaxHandShakeLost);
	}
}

void NatPiSession::handleTmoutHandShakeLoop()
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"NATAP[%s]: HandShake period timeout!!!",
		(int)getName());
	stopTimer(TIMER_NAT_HANDSHAKE);
	if(g_blHandShakeFlag)	
	{
		sendHandShakeMsg();//send handshake msg
	}
}

//pMsg:input buffer pointer, len:the length of input msg
NATAP_PKT_TYPE NatPiSession::authenticateNATAPMsg(UINT8* pMsg, UINT16 len)
{
	NAT_App_Pkt_T *pAppPkt;
	NAT_RegRsp_Pkt_T *pRegRspPkt;
	NAT_HandShake_Pkt_T *pHandShakePkt;
	NAT_HandShakeRsp_Pkt_T *pHandShakeRspPkt;
	NAT_Close_Pkt_T *pClosePkt;

	UINT8 Digest[20];
	UINT8 *pInput;
	UINT8 spi;
	UINT16 inputLen,digestLen;
	
	if(NULL==pMsg)
	{
		LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"NATAP[%s]: authenticateNATAPMsg, NULL pointer error!!!",
			(int)getName());		
		return INVALID_NATAP_PKT;
	}
	AH_T* pAHhead = (AH_T*)pMsg;
	if(pAHhead->protocolID!=M_PROTOCOL_ID_NATAP)
	{
		LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"NATAP[%s]: authenticateNATAPMsg, NATAP protocolID error!!!",
			(int)getName());
		return INVALID_NATAP_PKT;
	}
	UINT16 nLenFieldOfAHHead = len;//isTcpApp() ? len : (len-1);
	if(VGetU16BitVal(pAHhead->nLen)!=nLenFieldOfAHHead)	
	{
		LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"NATAP[%s]: authenticateNATAPMsg, NATAP packet length error!!!",
			(int)getName());
		return INVALID_NATAP_PKT;
	}
	
	UINT16 linkID;
	NATAP_PKT_TYPE pktType = (NATAP_PKT_TYPE)pAHhead->msgType;
	switch(pktType)
	{
		case APP_PACKET:
			//linkID
			pAppPkt = (NAT_App_Pkt_T*)pMsg;
			linkID = VGetU16BitVal(pAppPkt->linkID);
			break;
		case REG_MSG:
			//不可能走到
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"NATAP[%s]: authenticateNATAPMsg, REG msgType, invalid!!!",
				(int)getName());
			return INVALID_NATAP_PKT;
			//break;
		case REG_RSP_MSG:
			pRegRspPkt = (NAT_RegRsp_Pkt_T*)pMsg;
			spi = pRegRspPkt->AHHead.SPI;
			pInput = (UINT8*)pMsg;
			inputLen = (UINT8*)pRegRspPkt->AuthResult- pMsg;
			digestLen = NATAPComputeAuthResult(spi, pInput, inputLen, Digest);
			if(0==digestLen) 
			{
				LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"NATAP[%s]: authenticateNATAPMsg, SPI or AuthAlgo error!!!",
				(int)getName());
				return INVALID_NATAP_PKT;
			}
			else
			{
				if(0!=memcmp(Digest,pRegRspPkt->AuthResult,digestLen))
				{
					LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"NATAP[%s]: authenticateNATAPMsg, AuthResult not same!!!",
					(int)getName());
					//return INVALID_NATAP_PKT;鉴权结果比对错误也允许正常使用
					m_nRcvNatApPktCounters[pktType]++;
					return pktType;
				}
				else
				{
					m_nRcvNatApPktCounters[pktType]++;
					return pktType;
				}
			}
			//break;
		case HANDSHAKE_MSG:
			//linkID
			pHandShakePkt = (NAT_HandShake_Pkt_T*)pMsg;
			linkID = VGetU16BitVal(pHandShakePkt->NatHandShakeData.linkID);
			break;
		case HANDSHAKE_RSP_MSG:
			//linkID
			pHandShakeRspPkt = (NAT_HandShakeRsp_Pkt_T*)pMsg;
			linkID = VGetU16BitVal(pHandShakeRspPkt->NatHandShakeRspData.linkID);
			break;
		case CLOSE_MSG:
			//linkID
			pClosePkt = (NAT_Close_Pkt_T*)pMsg;
			linkID = VGetU16BitVal(pClosePkt->NatCloseData.linkID);
			break;
		default:
			//错误消息类型
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"NATAP[%s]: authenticateNATAPMsg, invalid msgType!!!",
				(int)getName());
			return INVALID_NATAP_PKT;
	}

	if(linkID==getLinkID())
	{
		m_nRcvNatApPktCounters[pktType]++;
		return pktType;
	}
	else
	{
		LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"NATAP[%s]: authenticateNATAPMsg, NATAP LinkID error!!!",
			(int)getName());
		return INVALID_NATAP_PKT;
	}
}

//处理收到的NATAP定时器消息
void NatPiSession::handleNATAPTmoutMsg(UINT16 msgID)
{
	switch(msgID)
	{
		case MSGID_TMOUT_REG_LOOP:
			handleTmoutRegLoop();
			break;
		case MSGID_TMOUT_REG_RSP:
			handleTmoutRegRsp();
			break;
		case MSGID_TMOUT_HANDSHAKE:
			handleTmoutHandShakeLoop();
			break;
		case MSGID_TMOUT_HANDSHAKERSP:
			handleTmoutHandShakeRsp();
			break;
		default:
			;
	}
}

//处理远端收到的NATAP的包
void NatPiSession::handleNATAPMsg(UINT8* pMsg, UINT16 len)
{
	//透传消息不进入此函数,目前仅tcp的NATAP层封装透传消息
	//非透传消息
	if(!isTcpApp())
	{
		if(INVALID_NATAP_PKT==authenticateNATAPMsg(pMsg, len))
			return;
	}
	
	AH_T* pAHhead = (AH_T*)pMsg;
	NATAP_PKT_TYPE pktType = (NATAP_PKT_TYPE)pAHhead->msgType;
	switch(pktType)
	{
		case APP_PACKET:
			//不应该走到
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"NATAP[%s]: handleNATAPMsg, should not run here!!!",
				(int)getName());
			break;
		case REG_MSG:
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"NATAP[%s]: handleNATAPMsg, REGISTER msgType, invalid!!!",
				(int)getName());
			//不可能走到
			break;
		case REG_RSP_MSG:
			handleRegRspMsg(pMsg, len);
			break;
		case HANDSHAKE_MSG:
			handleHandShakeMsg(pMsg, len);
			break;
		case HANDSHAKE_RSP_MSG:
			handleHandShakeRspMsg(pMsg, len);
			break;
		case CLOSE_MSG:
			handleCloseMsg(pMsg, len);
			break;
		default:
			//错误消息类型
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"NATAP[%s]: handleNATAPMsg, invalid msgType!!!",
				(int)getName());
	}
		
}

//针对测试中出现的VDR任务没有死，但再也不发送NATAP消息和再也没有收到NATAP的定时器消息的现象
bool NatPiSession::isSessionAlive()
{
	//如果连续监测M_MAX_NATAP_NOACTION_TIMES次HandShake和RegisterReq包的发送数目没有变化，则认为NATAP session 死掉了
	if(m_nNoNatApActionTimes>M_MAX_NATAP_NOACTION_TIMES)
	{
		return false;
	}
	if(m_nSndNatApPktCounters_bak[REG_MSG]==m_nSndNatApPktCounters[REG_MSG] &&
		m_nSndNatApPktCounters_bak[HANDSHAKE_MSG]==m_nSndNatApPktCounters[HANDSHAKE_MSG])
	{
		m_nNoNatApActionTimes++;
	}
	else
	{
		m_nSndNatApPktCounters_bak[REG_MSG] = m_nSndNatApPktCounters[REG_MSG];
		m_nSndNatApPktCounters_bak[HANDSHAKE_MSG] = m_nSndNatApPktCounters[HANDSHAKE_MSG];
		m_nNoNatApActionTimes = 0;	//发送次数发生了变化，认为session still alive
	}
	return true;
}


void NatPiSession::clearCounters()
{
	memset((void*)m_nRcvNatApPktCounters, 0, sizeof(m_nRcvNatApPktCounters));
	memset((void*)m_nSndNatApPktCounters, 0, sizeof(m_nSndNatApPktCounters));
}

char NatPiSession::m_PktName[M_MAX_NATAP_PKTTYPE][20]=
{
	"NatApPkt",
	"NatApAppPkt",
	"Register",
	"RegisterRsp",
	"HandShake",
	"HandShakeRsp",
	"Close"
};

void NatPiSession::showStatus()
{	
	int i;
	VPRINT("\n[%s] NatAP session info===============\n", getName());
	VPRINT("\nType:[%s] InUse:[%s] socket:[%08d] Registered[%s] linkID[%08d] NATAP AUTH SPI:[%d] ",
			isTcpApp() ? "TCP" : "UDP",
			IsInEffect() ? "YES" : "NO",
			m_fdSocket,
			isRegistered() ? "YES" : "NO",
			m_LinkID, 
			getSPIVal());
	VPRINT("\nlocalIP[0x%08x] localPort[%08d] peerIP[0x%08x] peerPort[%08d]",
			getLocalIP(), getLocalPort(), getPeerIP(), getPeerPort());
	VPRINT("\nnHandShakeLost[%8d]", getHSLostCounter());
	VPRINT("\nRecv Counters\n");
	for(i=APP_PACKET;i<M_MAX_NATAP_PKTTYPE;i++)
		VPRINT("     %s : %d", m_PktName[i], m_nRcvNatApPktCounters[i]);
	VPRINT("\nSend Counters\n");
	for(i=APP_PACKET;i<M_MAX_NATAP_PKTTYPE;i++)
		VPRINT("     %s : %d", m_PktName[i], m_nSndNatApPktCounters[i]);	
	VPRINT("\n=====================================================\n");
}


void showNatApCfg()
{
	int i;
	VPRINT("\nNATAP timers Cfg==================\n");
	for(i=TIMER_NAT_REGLOOP;i<TIMER_NAT_COUNT;i++)
	{
		VPRINT("\n %20s, [%03d]seconds", 
			   g_natTimerName[i], 
			   g_timerCfgTbl[i].timeoutVal / 1000);
	}
	VPRINT("\n nMaxHandShakeLost[%d]", g_MaxHandShakeLost);
	VPRINT("\n==================================\n");
}

void setNatApTimer(NATTimerID timerID, UINT8 nSeconds)
{
	if(nSeconds==0)
	{
		VPRINT("Invalid timeout value!!!");
		return;
	}
	if(timerID<TIMER_NAT_COUNT)
		g_timerCfgTbl[timerID].timeoutVal = nSeconds * 1000;
	else
	{
		VPRINT("Invalid timerID!!!");
	}
}

void setMaxHandShakeLostNum(UINT8 nTimes)
{
	if(1<nTimes && nTimes<255)
		g_MaxHandShakeLost = nTimes;
	else
		VPRINT("\nInvalid value!!!\n");
}



