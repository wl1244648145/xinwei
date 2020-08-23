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
#include "tVoiceSignal.h"
#include "NatPi.h"
#include "log.h"
#include "NatAuth.h"
#include "stdio.h"
#ifndef __WIN32_SIM__

#include "sockLib.h"
#include "errnoLib.h"
#include "inetLib.h" 

#endif

#define LOG(a,b,c) printf(c) 
#define LOG1(a,b,c,d) printf(c,d)
#define LOG2(a,b,c,d,e) printf(c,d,e)
#define VOICE 1
#define LOGNO
#define EC_L3VOICE_NORMAL 2

void OutputSocketErrCode(char *p)
{
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

void NatPiSession::init(UINT32 localIP, UINT16 localPort,
						UINT32 peerIP, UINT16 peerPort,
						RegSuccessCallBack cbFunc)
{
    srand((unsigned int)time((time_t *)NULL));
	UINT32 initRandomVAL = rand();
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

}

void NatPiSession::start()
{
	sendRegMsg();
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
	int i;
	for(i=0;i<TIMER_NAT_COUNT;i++)
	{
		//new timeout msg
		g_timerCfgTbl[i].pTimeoutMsg = 
				new (CTask_VoiceSignal::GetInstance(),16) CComMessage;
		if(NULL==g_timerCfgTbl[i].pTimeoutMsg)
		{
			LOG1( LOG_CRITICAL, 
				 LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
				 "\nNATAP[%s]: New CComMessage error!!!",
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
				 	 "\nNATAP[%s]: New CTimer error!!!",
				 	 (int)getName());
			}		
		}
	}
	LOG1( LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		 "\nNATAP[%s]: initAllTimers()",
		 (int)getName());
}

void NatPiSession::clearAllTimers()
{
	int i;
	LOG1( LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		 "\nNATAP[%s]: clearAllTimers()",
		 (int)getName());
	for(i=0;i<TIMER_NAT_COUNT;i++)
	{
		g_timerCfgTbl[i].pTimeoutMsg = NULL;
		deleteTimer((NATTimerID)i);
	}
}

void NatPiSession::startTimer(NATTimerID timerID)
{
	if(m_pTimer[timerID]!=NULL)
	{
		//重新取得超时时间，因为也许更新了定时器时长
		m_pTimer[timerID]->SetInterval(g_timerCfgTbl[timerID].timeoutVal);
		m_pTimer[timerID]->Start();
		LOG2( LOG_DEBUG2, 
			  LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			  "\nNATAP[%s]: NATStartTimer[%s]", 
			  (int)getName(),
			  (int)g_natTimerName[timerID]);		
	}
}
void NatPiSession::stopTimer(NATTimerID timerID)
{
	if(m_pTimer[timerID]!=NULL)
	{
		m_pTimer[timerID]->Stop();
		LOG2( LOG_DEBUG2, 
			  LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			  "\nNATAP[%s]: NATStopTimer[%s]",
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
			  "\nNATAP[%s]: NATDeleteTimer[%s]", 
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
			UINT16* pHeadFlag = (UINT16*)bufToSend;
			UINT16* pEndFlag = (UINT16*)(bufToSend + 2 + nLenToSend);
			*pHeadFlag = htons(M_TCP_PKT_BEGIN_FLAG);
			*pEndFlag = htons(M_TCP_PKT_END_FLAG);
			do 
			{
				sent = send(m_fdSocket, pBufToSend, totalToSend, 0);
				if(sent<0)
				{
					LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
						"\nNATAP[%s]: sendNatPiPkt send error!!!",
						(int)getName());
					OutputSocketErrCode("send()");
					return sent;
				}
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
						"\nNATAP[%s]: sendNatPiPkt send error!!!",
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
	natRegMsg.AHHead.SPI = ((rand() % (NATAP_SPI_COUNT-1))+1);
	//长度包括认证字段
	nNatApPktLen = (UINT8*)natRegMsg.AuthResult - (UINT8*)&natRegMsg
				   +NATAPGetAuthResultLength(natRegMsg.AHHead.SPI);
	natRegMsg.AHHead.nLen = htons( nNatApPktLen );
	natRegMsg.AHHead.msgType = REG_MSG;
//AuthSN
	incLocalAHSN();
	natRegMsg.AuthSN = htonl( getLocalAHSN() );
//regData
	natRegMsg.NatRegData.transID = htons(getTransSN());
	natRegMsg.NatRegData.localIP = htonl(getLocalIP());
	natRegMsg.NatRegData.localPort = htons(getLocalPort());
//AuthResult
	//发送前对报文内容（从协议标识开始至报文内容截止，包括认证序号）
	//按加密算法加密，结果写入认证字段	
	NATAPComputeAuthResult( natRegMsg.AHHead.SPI, 
							(UINT8*) &natRegMsg, 
							(UINT8*) natRegMsg.AuthResult- (UINT8*)&natRegMsg,
							(UINT8*) natRegMsg.AuthResult);

//send to Peer
	sendNatPiPkt((UINT8*)&natRegMsg, nNatApPktLen);
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: send Register Msg",
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
	natHandShakeMsg.AHHead.nLen = htons(sizeof(NAT_HandShake_Pkt_T));
	natHandShakeMsg.AHHead.msgType = HANDSHAKE_MSG;
	//data
	natHandShakeMsg.NatHandShakeData.linkID = htons(getLinkID());
	natHandShakeMsg.NatHandShakeData.SN = htonl(getHandShakeSN());
	//send to Peer
	sendNatPiPkt((UINT8*)&natHandShakeMsg, sizeof(NAT_HandShake_Pkt_T));
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: send HandShake Msg",
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
	natHandShakeRspMsg.AHHead.nLen = htons(sizeof(NAT_HandShakeRsp_Pkt_T));
	natHandShakeRspMsg.AHHead.msgType = HANDSHAKE_RSP_MSG;
	//data
	natHandShakeRspMsg.NatHandShakeRspData.linkID = htons(getLinkID());
	natHandShakeRspMsg.NatHandShakeRspData.SN = htonl(peerSN);
	//send to Peer
	sendNatPiPkt((UINT8*)&natHandShakeRspMsg, sizeof(NAT_HandShakeRsp_Pkt_T));
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: send HandShakeRsp Msg",
		(int)getName());	
}

void NatPiSession::sendRegRspMsg(UINT16 transID)
{
	NAT_RegRsp_Pkt_T natRegRspMsg;
	UINT16 nNatApPktLen;
//AHHead
	natRegRspMsg.AHHead.protocolID = M_PROTOCOL_ID_NATAP;
	//random spi,[1,spicount-1]
	natRegRspMsg.AHHead.SPI = ((rand() % (NATAP_SPI_COUNT-1))+1);
	//长度包括认证字段
	nNatApPktLen = (UINT8*)natRegRspMsg.AuthResult - (UINT8*)&natRegRspMsg
				   +NATAPGetAuthResultLength(natRegRspMsg.AHHead.SPI);
	natRegRspMsg.AHHead.nLen = htons( nNatApPktLen );
	natRegRspMsg.AHHead.msgType = REG_RSP_MSG;
//AuthSN
	incLocalAHSN();
	natRegRspMsg.AuthSN = htonl( getLocalAHSN() );
//regRspData
	natRegRspMsg.NatRegRspData.transID = htons(transID);
	natRegRspMsg.NatRegRspData.result = NATAP_REG_SUCCESS;
	natRegRspMsg.NatRegRspData.linkID = htons(getLinkID());
	natRegRspMsg.NatRegRspData.pubIP = 5678;
	natRegRspMsg.NatRegRspData.pubPort = 5678;
//AuthResult
	//发送前对报文内容（从协议标识开始至报文内容截止，包括认证序号）
	//按加密算法加密，结果写入认证字段	
	NATAPComputeAuthResult( natRegRspMsg.AHHead.SPI, 
							(UINT8*) &natRegRspMsg, 
							(UINT8*) natRegRspMsg.AuthResult - (UINT8*)&natRegRspMsg,
							(UINT8*) natRegRspMsg.AuthResult);

//send to Peer
	sendNatPiPkt((UINT8*)&natRegRspMsg, nNatApPktLen);
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: send RegisterRsp Msg",
		(int)getName());

}

void NatPiSession::handleRegMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: Recv Register Msg",
		(int)getName());
	NAT_Reg_Pkt_T* pRegMsg = (NAT_Reg_Pkt_T*)pMsg;
	sendRegRspMsg(ntohs(pRegMsg->NatRegData.transID));
}

void NatPiSession::handleRegRspMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: Recv RegisterRsp Msg",
		(int)getName());
	
	stopTimer(TIMER_NAT_REGRSP);	//stop timer for regRsp timeout
	NAT_RegRsp_Pkt_T *pNatRegRspMsg = (NAT_RegRsp_Pkt_T*)pMsg;
	if(NATAP_REG_SUCCESS==pNatRegRspMsg->NatRegRspData.result)
	{//success
		setRegisteredFlag(true);
		setLinkID(ntohs(pNatRegRspMsg->NatRegRspData.linkID));
		stopTimer(TIMER_NAT_REGLOOP);	//stop timer for regLoop timeout
		clearHSLostCounter();
		startTimer(TIMER_NAT_HANDSHAKE);//start timer for handshake loop

		//run callback function
		if(NULL!=m_callbackFuncRegSuccess)
		{
			(*m_callbackFuncRegSuccess)(NULL);
		}

		LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"\nNATAP[%s]: Register Success##########################",
			(int)getName());
	}
	else
	{//fail
		//when timer for regLoop timeout,reg will be sent periodicly
		setRegisteredFlag(false);
		LOG2(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"\nNATAP[%s]: Register failed !!! Result[%d]",
			(int)getName(),
			pNatRegRspMsg->NatRegRspData.result);		
	}
}

void NatPiSession::handleHandShakeMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: Recv HandShake Msg",
		(int)getName());
	if(!IsServer())
	{
		if(!isRegistered())
		{
			//未注册状态下收到握手包不回应，尽快使远端失连，接收新注册
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"\nNATAP[%s]: HandShake received when not Registered, discard!!!",
				(int)getName());
			return;
		}
	}
	
	NAT_HandShake_Pkt_T* pHandShakeMsg = (NAT_HandShake_Pkt_T*)pMsg;
	UINT32 handshakeSN = ntohl(pHandShakeMsg->NatHandShakeData.SN);
	sendHandShakeRspMsg(handshakeSN);
}

void NatPiSession::handleHandShakeRspMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: Recv HandShakeRsp Msg",
		(int)getName());	
	stopTimer(TIMER_NAT_HANDSHAKERSP);
	clearHSLostCounter();	//清除握手未应答次数
}

//only for udp
void NatPiSession::handleCloseMsg(UINT8* pMsg, UINT16 len)
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: Recv Close Msg##########################",
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
	int i;
	//1)NATAP AHhead
	//2)LinkID
	UINT16 nNatApPktLen = sizeof(AH_T)+
						  sizeof(NATAP_LinkIDT);
	NAT_App_Pkt_T* pNat_AppPkt = (NAT_App_Pkt_T*)(pDataApp 
								 - nNatApPktLen);
	//AHHead
	pNat_AppPkt->AHHead.protocolID = M_PROTOCOL_ID_NATAP;
	pNat_AppPkt->AHHead.SPI = 0;
	pNat_AppPkt->AHHead.nLen = htons(nNatApPktLen+len);//包括Protocol和SPI
	pNat_AppPkt->AHHead.msgType = APP_PACKET;
	//LinkID
	pNat_AppPkt->linkID = htons(getLinkID());
	return nNatApPktLen;
}

void NatPiSession::handleTmoutRegRsp()
{
	stopTimer(TIMER_NAT_REGRSP);
	//reg fails,when timer regLoop expires, next reg will be sent
	setRegisteredFlag(false);
}

void NatPiSession::handleTmoutRegLoop()
{
	stopTimer(TIMER_NAT_REGLOOP);
	sendRegMsg();	//send reg msg
}

void NatPiSession::handleTmoutHandShakeRsp()
{
	LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"\nNATAP[%s]: wait HandShakeRsp timeout!!!",
		(int)getName());
	stopTimer(TIMER_NAT_HANDSHAKERSP);
	incHSLostCounter();	//握手未应答次数+1
	if(getHSLostCounter()>=g_MaxHandShakeLost)
	{
		stopTimer(TIMER_NAT_HANDSHAKE);	//结束握手流程
		setRegisteredFlag(false);		//标记为未注册
		sendRegMsg();					//转入注册流程
		LOG2(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			 "\nNATAP[%s]: %d or more HandShakes unreplyed, set UnRegistered!!!",
			 (int)getName(),
			 g_MaxHandShakeLost);
	}
}

void NatPiSession::handleTmoutHandShakeLoop()
{
	stopTimer(TIMER_NAT_HANDSHAKE);
	sendHandShakeMsg();//send handshake msg
}

NATAP_PKT_TYPE NatPiSession::authenticateNATAPMsg(UINT8* pMsg, UINT16 len)
{
	NAT_App_Pkt_T *pAppPkt;
	NAT_Reg_Pkt_T *pRegPkt;
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
			"\nNATAP[%s]: authenticateNATAPMsg, NULL pointer error!!!",
			(int)getName());		
		return INVALID_NATAP_PKT;
	}
	AH_T* pAHhead = (AH_T*)pMsg;
	if(pAHhead->protocolID!=M_PROTOCOL_ID_NATAP)
	{
		LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"\nNATAP[%s]: authenticateNATAPMsg, NATAP protocolID error!!!",
			(int)getName());
		return INVALID_NATAP_PKT;
	}	

	UINT16 nLenFieldOfAHHead = len;//isTcpApp() ? len : (len-1);
	if(ntohs(pAHhead->nLen)!=nLenFieldOfAHHead)
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
			linkID = ntohs(pAppPkt->linkID);
			break;
		case REG_MSG:
			pRegPkt = (NAT_Reg_Pkt_T*)pMsg;
			spi = pRegPkt->AHHead.SPI;
			pInput = (UINT8*)pMsg;
			inputLen = (UINT8*)pRegPkt->AuthResult - pMsg;
			digestLen = NATAPComputeAuthResult(spi, pInput, inputLen, Digest);
			if(0==digestLen) 
			{
				LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"\nNATAP[%s]: authenticateNATAPMsg, SPI or AuthAlgo error!!!",
				(int)getName());
				return INVALID_NATAP_PKT;
			}
			else
			{
				if(0!=memcmp(Digest,pRegPkt->AuthResult,digestLen))
				{
					LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"\nNATAP[%s]: authenticateNATAPMsg, AuthResult not same!!!",
					(int)getName());
					return INVALID_NATAP_PKT;
				}
				else
				{
					m_nRcvNatApPktCounters[pktType]++;
					return pktType;
				}
			}
			break;
		case REG_RSP_MSG:
			pRegRspPkt = (NAT_RegRsp_Pkt_T*)pMsg;
			spi = pRegRspPkt->AHHead.SPI;
			pInput = (UINT8*)pMsg;
			inputLen = (UINT8*)pRegRspPkt->AuthResult - pMsg;
			digestLen = NATAPComputeAuthResult(spi, pInput, inputLen, Digest);
			if(0==digestLen) 
			{
				LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"\nNATAP[%s]: authenticateNATAPMsg, SPI or AuthAlgo error!!!",
				(int)getName());
				return INVALID_NATAP_PKT;
			}
			else
			{
				if(0!=memcmp(Digest,pRegRspPkt->AuthResult,digestLen))
				{
					LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"\nNATAP[%s]: authenticateNATAPMsg, AuthResult not same!!!",
					(int)getName());
					return INVALID_NATAP_PKT;
				}
				else
				{
					m_nRcvNatApPktCounters[pktType]++;
					return pktType;
				}
			}
			break;
		case HANDSHAKE_MSG:
			//linkID
			pHandShakePkt = (NAT_HandShake_Pkt_T*)pMsg;
			linkID = ntohs(pHandShakePkt->NatHandShakeData.linkID);
			break;
		case HANDSHAKE_RSP_MSG:
			//linkID
			pHandShakeRspPkt = (NAT_HandShakeRsp_Pkt_T*)pMsg;
			linkID = ntohs(pHandShakeRspPkt->NatHandShakeRspData.linkID);
			break;
		case CLOSE_MSG:
			//linkID
			pClosePkt = (NAT_Close_Pkt_T*)pMsg;
			linkID = ntohs(pClosePkt->NatCloseData.linkID);
			break;
		default:
			//错误消息类型
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"\nNATAP[%s]: authenticateNATAPMsg, invalid msgType!!!",
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
			"\nNATAP[%s]: authenticateNATAPMsg, NATAP LinkID error!!!",
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
	if(INVALID_NATAP_PKT==authenticateNATAPMsg(pMsg, len))
		return;
	
	AH_T* pAHhead = (AH_T*)pMsg;
	NATAP_PKT_TYPE pktType = (NATAP_PKT_TYPE)pAHhead->msgType;
	switch(pktType)
	{
		case APP_PACKET:
			//不应该走到
			LOG1(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"\nNATAP[%s]: handleNATAPMsg, should not run here!!!",
				(int)getName());
			break;
		case REG_MSG:
			handleRegMsg(pMsg, len);
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
				"\nNATAP[%s]: handleNATAPMsg, invalid msgType!!!",
				(int)getName());
	}
		
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
	printf("\n[%s] NatAP session info===============\n", getName());
	printf("\nType:[%s] InUse:[%s] socket:[%08d] Registered[%s] linkID[%08d]",
			isTcpApp() ? "TCP" : "UDP",
			IsInEffect() ? "YES" : "NO",
			m_fdSocket,
			isRegistered() ? "YES" : "NO",
			m_LinkID);
	printf("\nlocalIP[0x%08x] localPort[%08d] peerIP[0x%08x] peerPort[%08d]",
			getLocalIP(), getLocalPort(), getPeerIP(), getPeerPort());
	printf("\nnHandShakeLost[%8d]", getHSLostCounter());
	printf("\nRecv Counters\n");
	for(i=APP_PACKET;i<M_MAX_NATAP_PKTTYPE;i++)
		printf("     %s : %d", m_PktName[i], m_nRcvNatApPktCounters[i]);
	printf("\nSend Counters\n");
	for(i=APP_PACKET;i<M_MAX_NATAP_PKTTYPE;i++)
		printf("     %s : %d", m_PktName[i], m_nSndNatApPktCounters[i]);	
	printf("\n=====================================================\n");
}

//global obj for voice SAbis1 tcp NATAP
NatPiSession g_VCR_NatAPc(true, NAT_APP_TCP, M_TID_VCR, "VCR");
//global obj for voice DMUX udp NATAP
NatPiSession g_VDR_NatAPc(true, NAT_APP_UDP, M_TID_VDR, "VDR");

void showNatApInfo()
{
	int i;
	printf("\nNATAP timers Cfg==================\n");
	for(i=TIMER_NAT_REGLOOP;i<TIMER_NAT_COUNT;i++)
	{
		printf("\n %20s, [%03d]seconds", 
			   g_natTimerName[i], 
			   g_timerCfgTbl[i].timeoutVal / 1000);
	}
	printf("\n nMaxHandShakeLost[%d]", g_MaxHandShakeLost);
	printf("\n==================================\n");
	g_VCR_NatAPc.showStatus();
	g_VDR_NatAPc.showStatus();
}

void setNatApTimer(NATTimerID timerID, UINT8 nSeconds)
{
	if(nSeconds==0)
	{
		printf("Invalid timeout value!!!");
		return;
	}
	if(timerID<TIMER_NAT_COUNT)
		g_timerCfgTbl[timerID].timeoutVal = nSeconds * 1000;
	else
	{
		printf("Invalid timerID!!!");
	}
}

void setMaxHandShakeLostNum(UINT8 nTimes)
{
	if(1<nTimes && nTimes<255)
		g_MaxHandShakeLost = nTimes;
	else
		printf("\nInvalid value!!!\n");
}

void clearNatApPktCounters()
{
	g_VCR_NatAPc.clearCounters();
	g_VDR_NatAPc.clearCounters();
}

void closeNatApFun()
{
	g_VCR_NatAPc.stop();
	g_VCR_NatAPc.setEffect(false);
	g_VDR_NatAPc.stop();
	g_VDR_NatAPc.setEffect(false);
}

void closeVDRNatApFunc()
{
	g_VDR_NatAPc.stop();
	g_VDR_NatAPc.setEffect(false);
}

void closeVCRNatApFunc()
{
	g_VCR_NatAPc.stop();
	g_VCR_NatAPc.setEffect(false);
}


