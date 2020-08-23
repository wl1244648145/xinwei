/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    tVDR.cpp
 *
 * DESCRIPTION: 
 *   VDR task, voice data snd&rcv function
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   2006-12-19 fengbing  如果SAG的IP配置为0.0.0.0，则认为不使用语音业务
 *   2006-04-19 fengbing  加入任务接口上信令＆语音数据的计数统计 
 *   2006-04-16 fengbing  fix bugs in CVDR::PostMessage();
 *   2006-3-29  fengbing  change DMUX format, delete BTSID comparation, only recv data from SAG cfg Ipaddr
 *   2006-3-25  fengbing  modify running cfg
 *   11/08/05   fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/


#include "tVCR.h"
#include "tVDR.h"
#include "log.h"
#include "BtsVMsgId.h"
#include "stdio.h"
#include "NatpiApp.h"
#include "sysBtsConfigData.h"
#include "l3OamCfgCommon.h"
#include "voiceToolFunc.h"
#include "localSagCfg.h"

extern T_NvRamData *NvRamDataAddr;
extern void OutputSocketErrCode(char *p);

typedef struct _vdrPipeCntT{
    UINT32 nPostCalled;
    UINT32 nPostNull;
    UINT32 nPostFail;
//    UINT32 nPostWhenSagNotConnected;
    UINT32 nEnterPipe;
    UINT32 nOutofPipe;
//    UINT32 nFlush;
    UINT32 nReadOK;
    UINT32 nReadError;
    UINT32 nReadLessBytes;
}vdrPipeCntsT;
vdrPipeCntsT vdrPipeCnt;
extern "C" void showVdrPipeStatus();
extern "C" void clrVdrPipeStatus();
void clrVdrPipeStatus()
{
    memset( (void*)&vdrPipeCnt, 0, sizeof(vdrPipeCntsT));
}
void showVdrPipeStatus()
{
    VPRINT("\n========vdrPipeCnt========");
    VPRINT("\n nPostCalled[%d] \n nPostNull[%d] \n nPostFail[%d] \n nEnterPipe[%d] \n nOutofPipe[%d] \n nReadOK[%d] \n nReadError[%d] \n nReadLessBytes[%d]",
        vdrPipeCnt.nPostCalled,
        vdrPipeCnt.nPostNull,
        vdrPipeCnt.nPostFail,
        vdrPipeCnt.nEnterPipe,
        vdrPipeCnt.nOutofPipe,
        vdrPipeCnt.nReadOK,
        vdrPipeCnt.nReadError,
        vdrPipeCnt.nReadLessBytes
        );
    VPRINT("\n================\n");    
}
#if 0
#ifdef DSP_BIOS
static Void error(Void)
{
    LOG_VPRINT(&trace, "VDR Error: copy signal falsely triggered!");
    
    for (;;) {
        ;       /* loop for ever */
    }
}
#endif
#endif
#ifdef __WIN32_SIM__
//#include "winsock2.h"
#define getErrNO	WSAGetLastError

#else	//vxworks
#define getErrNO	errnoGet
#endif
//任务实例指针的初始化
CVDR* CVDR::s_ptaskTVDR = NULL;
CVDR* CVDR::s_ptaskTVDR1 = NULL;

UINT8 g_TOS = 0;//IPTOS_LOWDELAY;
extern "C" bool isValidTosVal(UINT8 tos)
{
	return (tos<64);
}
extern "C" void SetTosVal(UINT8 tos)
{
	if(isValidTosVal(tos))
	{
		g_TOS = tos;
		CVDR::GetInstance()->setTosVal(tos);
		CVDR::GetBakInstance()->setTosVal(tos);
		CVCR::GetInstance()->setTosVal(tos);
		CVCR::GetBakInstance()->setTosVal(tos);
		if(tos!=NvRamDataAddr->SagTos.ucSagTosVoice)
		{
			writeData2NvRam((char*)&NvRamDataAddr->SagTos.ucSagTosVoice, 
				(char*)&g_TOS, 
				sizeof(NvRamDataAddr->SagTos.ucSagTosVoice));
		}
	}
}
void CVDR::setTosVal(UINT8 tos)
{
	if(m_UdpSocket<0)
	{
	}
	else
	{
		int optval = tos;
		if(isValidTosVal( tos))
			setsockopt (m_UdpSocket, IPPROTO_IP, IP_TOS, (char*)&optval, (int)sizeof(optval));
	}
}

void CVDR::setTaskOptions()
{
	if(GetEntityId()==M_TID_VDR1)
	{
		memcpy(m_szName, M_TASK_TVDR1_TASKNAME, strlen( M_TASK_TVDR1_TASKNAME ) );
		m_szName[strlen( M_TASK_TVDR1_TASKNAME )] = 0;
		strcpy(m_szPipeName, M_VDR1_PIPENAME);
		m_pBtsSagLinkCfg = &g_vSagBtsLinkCfg2;
		m_pNatpiSession = getVdr1NatpiSession();
	}
	else
	{
		memcpy(m_szName, M_TASK_TVDR_TASKNAME, strlen( M_TASK_TVDR_TASKNAME ) );
		m_szName[strlen( M_TASK_TVDR_TASKNAME )] = 0;
		strcpy(m_szPipeName, M_VDR_PIPENAME);
		m_pBtsSagLinkCfg = &g_vSagBtsLinkCfg1;
		m_pNatpiSession = getVdrNatpiSession();
	}
	m_uPriority = M_TP_L3VDR;
#ifdef DSP_BIOS
	m_iMsgQMax = 1024;//lijinan 20091118 add
#else 	
	m_uOptions = M_TASK_TVDR_OPTION;
#endif
	m_uStackSize = M_TASK_TVDR_STACKSIZE + 10240;
	m_LocalPort = 0xffff;
	m_MediaPort = 0xffff;
	strcpy(m_MediaServerIPAddr,"255.255.255.255");
	
	m_UdpSocket = -1;
	m_blEffect = false;
	m_blNetCfgChanged = false;	
	m_blNatApSessionNeedRestart = false;
}

CVDR::CVDR()
{
#ifndef NDEBUG
	LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVDR::CVDR()");
	
	if (!Construct(CObject::M_OID_VDR))
	{
       	LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "ERROR!!!CVDR::CVDR()% Construct failed.");
	}
#endif	
#if 0
	//memset(m_szName, 0 , M_TASK_NAME_LEN );
	memcpy(m_szName, M_TASK_TVDR_TASKNAME, strlen( M_TASK_TVDR_TASKNAME ) );
	m_szName[strlen( M_TASK_TVDR_TASKNAME )] = 0;
	m_uPriority = M_TP_L3VDR;
	m_uOptions = M_TASK_TVDR_OPTION;
	m_uStackSize = M_TASK_TVDR_STACKSIZE + 10240;

	m_LocalPort = 0xffff;
	m_MediaPort = 0xffff;
	strcpy(m_MediaServerIPAddr,"255.255.255.255");
	
	m_UdpSocket = -1;
	m_blEffect = false;
	m_blNetCfgChanged = false;
	m_blNatApSessionNeedRestart = false;
#endif	
}

CVDR::~CVDR()
{
#ifndef NDEBUG
	LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVDR::~CVDR");
	if (!Destruct(CObject::M_OID_VDR))
	{
       	LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "ERROR!!!CVDR::~CVDR failed.");
	}
#endif
	
#ifdef __WIN32_SIM__
	WSACleanup();
#endif	
}

TID CVDR::GetEntityId() const
{ 
	return m_selfTID; 
}

CVDR* CVDR::GetInstance()
{
	if ( NULL == s_ptaskTVDR)
	{
		s_ptaskTVDR = new CVDR;
		if(s_ptaskTVDR!=NULL)
		{
			s_ptaskTVDR->SetEntityId(M_TID_VDR);
			s_ptaskTVDR->setTaskOptions();
		}
	}
	return s_ptaskTVDR;
}

CVDR* CVDR::GetBakInstance()
{
	if ( NULL == s_ptaskTVDR1)
	{
		s_ptaskTVDR1 = new CVDR;
		if(s_ptaskTVDR!=NULL)
		{
			s_ptaskTVDR1->SetEntityId(M_TID_VDR1);
			s_ptaskTVDR1->setTaskOptions();
		}
	}
	return s_ptaskTVDR1;
}

bool CVDR::Initialize()
{
	LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVDR::Initialize");

#ifdef DSP_BIOS
	semHandle = SEM_create(0,NULL);
	if(semHandle==NULL)
	{
		LOG(LOG_CRITICAL,0,"l3datasocket create sem err!");
		return false;
	}
	TSK_Attrs tskAttr;
	tskAttr.stack = NULL;
	tskAttr.stacksize = 0x4000;
	tskAttr.stackseg = TSK->STACKSEG;
	tskAttr.environ = NULL;
	tskAttr.exitflag = TRUE;
	tskAttr.initstackflag = TRUE;	
	tskAttr.priority = M_TP_L3VDR;
	if(m_selfTID==M_TID_VDR)
		tskAttr.name = "TSK_vdrRx";
	else
		tskAttr.name = "TSK_vdr1Rx";
	if((m_selfTID==M_TID_VDR))
	{
		if((TSK_create((Fxn)RunVdrSocketRx, &tskAttr, (int)this))==NULL)
		{
			LOG(LOG_SEVERE,0,"TSK_socketRx taskSpawm failed.");
			return false;
		}
	}
	else
	{
		if((TSK_create((Fxn)RunVdr1SocketRx, &tskAttr, (int)this))==NULL)
		{
			LOG(LOG_SEVERE,0,"TSK_socketRx taskSpawm failed.");
			return false;
		}
	}
	if (!CBizTask::Initialize())
		return false;
#else//DSP_BIOS
#ifdef __WIN32_SIM__
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	
	wVersionRequested = MAKEWORD( 2, 2 );
	
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return false;
	}
#endif	

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
	
#endif//DSP_BIOS
	return true;
}
//接收并处理从SAG收到的语音包
bool CVDR::RecvAndHandleVoiceDataFromSAG()
{
	int nRcvBytes = 0;
	CComMessage *pComMsg = NULL;
	sockaddr_in from;
	int fromlen = sizeof(from);

	//接收语音包并转发给tVoice任务
	pComMsg = new (this, M_MAX_VDR_PDU+1) CComMessage;

	if(pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "Allocate empty commessage object failed.");
        return false;
	}
	char *buf = (char*)pComMsg->GetDataPtr();
#ifdef __USE_LWIP__	
	nRcvBytes = recvfrom(m_UdpSocket, buf, M_MAX_VDR_PDU+1, 0, (sockaddr*)&from, (unsigned int *)&fromlen);
#else
	nRcvBytes = recvfrom(m_UdpSocket, buf, M_MAX_VDR_PDU+1, 0, (sockaddr*)&from, &fromlen);
#endif
	if(nRcvBytes<0)	//socket error
	{
#ifdef __USE_LWIP__	
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "recvfrom failed. ");
#else
		LOG1(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "recvfrom failed. errcode[%d]", getErrNO());
#endif
		CloseSocket();
		CreateSocket();
		pComMsg->Destroy();
		return false;
	}
	else if(nRcvBytes>M_MAX_VDR_PDU)	//收多了,非法包，丢弃
	{
		LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_INVALID_VOICEDATA), "recvfrom() recv bytes exceeds MAXPDU, packet invalid!!!");
		pComMsg->Destroy();
		return false;
	}
	else
	{
		if( (m_RemoteAddr.sin_port!=from.sin_port) || 
			(m_RemoteAddr.sin_addr.s_addr!=from.sin_addr.s_addr) )
		{
			LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_INVALID_VOICEDATA), "recv data from invalid ipAddr!!!!.");
			pComMsg->Destroy();
			return false;
		}
#if 0		
		//判断BTSID是否合法，否则丢弃
		UINT32	BTSID = ntohs( *((UINT16*)buf) );
		if(BTSID!=m_BTSID)
		{
			
			LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_INVALID_VOICEDATA), "BTSID error, packet invalid!!!!.");
			pComMsg->Destroy();
			return false;
		}
#endif		

#ifdef NATAP_FUNCTION
		if(m_pNatpiSession->IsInEffect())
		{
			//如果是NATAP包
			AH_T* pAHHead = (AH_T*)buf;
			if(M_PROTOCOL_ID_NATAP==pAHHead->protocolID)
			{
				m_pNatpiSession->handleNATAPMsg( (UINT8*)buf, nRcvBytes );
				pComMsg->Destroy();
				return true;
			}		
		}
#endif
		//20090905 fengbing,when master SAG is Connected, discard voice data packet from backup SAG
		if(isBackupInstance())
		{
			if( CVCR::GetInstance()->IsSAGConnected() )
			{
				pComMsg->Destroy();
				return true;
			}
		}

		Counters.nVoiDataFromMG++;

		pComMsg->SetDstTid(M_TID_VOICE);
		pComMsg->SetSrcTid(M_TID_VDR);
		pComMsg->SetMessageId(MSGID_VDR_VOICE_DATA);
		pComMsg->SetDataLength(nRcvBytes);
		
		if(!CComEntity::PostEntityMessage(pComMsg))
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_MSG_SND_FAIL), "Send Voice Data Message to tVoice error!!!");
			pComMsg->Destroy();		
		}
		else
		{
			Counters.nVoiDataToTvoice++;
		}
	}
	
	return true;
}
//接收并处理从tVoice任务收到的语音包
#ifdef DSP_BIOS
int CVDR::RecvAndHandleVoiceDataFromVoice(CComMessage *pComMsg)
#else
int CVDR::RecvAndHandleVoiceDataFromVoice()
#endif
{
	UINT16 msgID;

#ifndef DSP_BIOS	
	CComMessage *pComMsg = NULL;
	int nRcvLen;
#ifdef __WIN32_SIM__
	nRcvLen = recvfrom(m_fdPipe, (char*)&pComMsg, 4, 0, NULL, NULL);
#else	//vxworks
	nRcvLen = read (m_fdPipe, (char*)&pComMsg, 4);
#endif
	if(nRcvLen!=4)
	{
	    if(nRcvLen<0)
        {
	        vdrPipeCnt.nReadError++;
	        LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_MSG_EXCEPTION), "VDR read from pipe error!!!");
        }
	    else
        {
            vdrPipeCnt.nReadLessBytes++;
            vdrPipeCnt.nOutofPipe++;
            LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_MSG_EXCEPTION), "VDR read less than 4 bytes from pipe !!!");
        }
		return -1;
	}
    else
    {
        vdrPipeCnt.nReadOK++;
        vdrPipeCnt.nOutofPipe++;
    }
#endif//DSP_BIOS
    if(pComMsg==NULL)
        return 0;
    
	msgID = pComMsg->GetMessageId();
	TID srcTID = pComMsg->GetSrcTid();
	if(MSGID_VOICE_VDR_DATA==msgID)
	{
		Counters.nVoiDataFromTvoice++;
		sendto(m_UdpSocket, (char*)pComMsg->GetDataPtr(), pComMsg->GetDataLength(), 0, (struct sockaddr *)&m_RemoteAddr, sizeof(m_RemoteAddr));
		Counters.nVoiDataToMG++;
		pComMsg->Destroy();
		return 1;
	}
	else
	{
#ifdef M_SYNC_BROADCAST
		if(MSGID_GRP_L2L3_MBMS_MEDIA_DELAY_RSP==msgID)
		{
			sendto(m_UdpSocket, (char*)pComMsg->GetDataPtr(), pComMsg->GetDataLength(), 0, (struct sockaddr *)&m_RemoteAddr, sizeof(m_RemoteAddr));
			pComMsg->Destroy();
			return 6;	
		}
#endif//M_SYNC_BROADCAST
        if(MSGID_VOICE_SET_CFG==msgID && M_TID_VOICE==srcTID)
        {
            pComMsg->Destroy();
            bool blNetCfgChanged = IsNetCfgChanged();//true;
            reloadVdrCfg();
            if(blNetCfgChanged)
            {
                m_blNetCfgChanged = true;                
                return 2;
            }
            else
            {
                return 3;
            }
            
        }
        else
        {
		pComMsg->Destroy();
 #ifdef NATAP_FUNCTION	
		if(m_pNatpiSession->IsInEffect())
		{
			if(MSGID_TMOUT_REG_LOOP<=msgID && msgID<=MSGID_TMOUT_HANDSHAKERSP)
			{
				m_pNatpiSession->handleNATAPTmoutMsg(msgID);
				return 4;
			}
			if(MSGID_NATAP_RESTART==msgID && M_TID_VOICE==srcTID)
			{
				m_pNatpiSession->stop();
				m_blNatApSessionNeedRestart = true;
				return 5;
			}
		}
#endif       
        }
	}
	
	return 1;
}
#ifdef __VXWORKS__
#ifndef WBBU_CODE
extern ULONG dwSagIp;
extern struct sockaddr_in stSagAddr;
extern ULONG dwSagIp1;
extern struct sockaddr_in stSagAddr1;
#endif
#endif
#ifdef DSP_BIOS
extern "C"int bspGetBtsID();
#endif
void CVDR::reloadVdrCfg()
{
    //如果SAG的IP配置为0.0.0.0，则认为不使用语音业务
	m_blEffect = (m_pBtsSagLinkCfg->SAGSignalIP!=0);

	//得到运行配置信息
	m_BTSID = bspGetBtsID();
	m_BTSID &= 0x0fff;		//BTSID只取低16位
	SetTosVal(g_vSrvCfg.nTosVal);

	if(IsNetCfgChanged())
	{
		UINT32 IP_UINT32 = m_pBtsSagLinkCfg->SAGVoiceIP;
		UINT8* pIPAddr = (UINT8*)&IP_UINT32;
		sprintf(m_MediaServerIPAddr, "%d.%d.%d.%d", pIPAddr[0], pIPAddr[1], pIPAddr[2], pIPAddr[3]);
		m_MediaPort = m_pBtsSagLinkCfg->SAGTxPortV;
		m_LocalPort = m_pBtsSagLinkCfg->SAGRxPortV;
	
#ifdef NATAP_FUNCTION
		//In Effct or not
		//m_pNatpiSession->setEffect(true or false);
		if(m_pNatpiSession->IsInEffect())
		{
			m_pNatpiSession->init(GetBtsSagIpAddr(),
								m_pBtsSagLinkCfg->SAGRxPortV, 
								m_pBtsSagLinkCfg->SAGVoiceIP, 
								m_pBtsSagLinkCfg->SAGTxPortV);
			m_pNatpiSession->setSPIVal(m_pBtsSagLinkCfg->NatAPKey);
		}
#endif
		//准备好UDP发送地址
		m_RemoteAddr.sin_family = AF_INET;
		m_RemoteAddr.sin_port= htons(m_MediaPort);
		m_RemoteAddr.sin_addr.s_addr=inet_addr(m_MediaServerIPAddr);
#ifdef __VXWORKS__
#ifndef WBBU_CODE
		if(isMasterInstance())
		{
			stSagAddr.sin_port = m_RemoteAddr.sin_port;
			stSagAddr.sin_addr.s_addr = m_RemoteAddr.sin_addr.s_addr;
			dwSagIp = m_RemoteAddr.sin_addr.s_addr;
		}
		else
		{
			stSagAddr1.sin_port = m_RemoteAddr.sin_port;
			stSagAddr1.sin_addr.s_addr = m_RemoteAddr.sin_addr.s_addr;
			dwSagIp1 = m_RemoteAddr.sin_addr.s_addr;
		}
#endif
#endif
	}

}

bool CVDR::IsNetCfgChanged()
{
	//如果SAG的IP配置为0.0.0.0，则认为不使用语音业务
	//这时认为配置发生了变化,fengbing 20091225,保证将来启用语音业务时不用重启bts
	if(m_pBtsSagLinkCfg->SAGSignalIP==0)
	{
		LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "not using voice service,tVDR netcfg changed!!!");
		return true;
	}
	
	//得到运行配置信息
	UINT32 IP_UINT32 = m_pBtsSagLinkCfg->SAGVoiceIP;
	UINT8* pIPAddr = (UINT8*)&IP_UINT32;
    char tmpIpAddr[16];
	sprintf(tmpIpAddr, "%d.%d.%d.%d", pIPAddr[0], pIPAddr[1], pIPAddr[2], pIPAddr[3]);
	if( strcmp(m_MediaServerIPAddr, tmpIpAddr)==0 &&
	    m_MediaPort == m_pBtsSagLinkCfg->SAGTxPortV &&
	    m_LocalPort == m_pBtsSagLinkCfg->SAGRxPortV &&
	    m_pBtsSagLinkCfg->NatAPKey == m_pNatpiSession->getSPIVal())
    {
        return false;
    }
	else
    {
        LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "tVDR netcfg changed!!!");
        return true;
    }
}


#ifdef DSP_BIOS
STATUS CVDR::RunVdrSocketRx(CVDR *pTask)
{
    ( (CVDR*)pTask )->SocketRxMainLoop();
    return 0;
}

STATUS CVDR::RunVdr1SocketRx(CVDR *pTask)
{
    ( (CVDR*)pTask )->SocketRxMainLoop();
    return 0;
}
extern "C" void mySwitchFxn(int tid,unsigned int inOut,void* pMsg);
void CVDR::SocketRxMainLoop()
#else//DSP_BIOS
//任务循环
void CVDR::MainLoop()
#endif//DSP_BIOS
{
    while(1)
    {
    	//如果SAG的IP配置为0.0.0.0，则认为不使用语音业务
    	while((0==m_pBtsSagLinkCfg->SAGSignalIP))
    	{
#ifdef DSP_BIOS    	
    		TSK_sleep(200);
#else
    		taskDelay(200);
#endif
    	}
    	//使用语音业务
        reloadVdrCfg();
        if(!CreateSocket())
        {
            
        }

    	FD_SET ReadFDs;
    	int ret;
    	struct timeval timer = { TVDR_MAX_BLOCKED_TIME_IN_10ms_TICK/SecondsToTicks(1), 0} ;
    	while (1) 
    	{
    		FD_ZERO(&ReadFDs);
    		FD_SET(m_UdpSocket, &ReadFDs);
#ifndef DSP_BIOS			
    		FD_SET(m_fdPipe, &ReadFDs);
			int maxFD = max(m_fdPipe, m_UdpSocket);
#else
			mySwitchFxn(GetEntityId(),0,NULL);
			int maxFD = m_UdpSocket;
#endif    	
    		ret = select(maxFD+1, &ReadFDs, NULL, NULL, &timer);
    		if(ret==-1)	//error occured, 假设管道不会发生错误
    		{
				LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "select() returned -1.");
    			CloseSocket();
    			break;
    		}
			
#ifndef DSP_BIOS
		//先处理管道中的数据
		if(FD_ISSET(m_fdPipe, &ReadFDs))
		{
			ret = RecvAndHandleVoiceDataFromVoice();
			//if net cfg changed
			if(m_blNetCfgChanged)
			{
				CloseSocket();
				m_blNetCfgChanged = false;
				break;
			}
#ifdef NATAP_FUNCTION	
			if(m_blNatApSessionNeedRestart)
			{
				LOG(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), 
					"VDR NATAP session DEAD!!!");
				CloseSocket();
				m_blNatApSessionNeedRestart = false;
				break;
			}
#endif				
		}		
#endif

#ifdef DSP_BIOS	
		mySwitchFxn(GetEntityId(),1,NULL);
#endif

    		//接收并处理从SAG的语音数据
    		if(FD_ISSET(m_UdpSocket, &ReadFDs))
    		{
    			RecvAndHandleVoiceDataFromSAG();
    		}
#ifdef DSP_BIOS			
			//if net cfg changed
			if(m_blNetCfgChanged)
			{
				CloseSocket();
				m_blNetCfgChanged = false;
				break;
			}
			//if natap is dead
#ifdef NATAP_FUNCTION	
			if(m_blNatApSessionNeedRestart)
			{
				LOG(LOG_DEBUG2, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), 
					"VDR NATAP session DEAD!!!");
				CloseSocket();
				m_blNatApSessionNeedRestart = false;
				break;
			}
#endif
#endif//	DSP_BIOS	
    	}    
    }
}

#ifdef DSP_BIOS
bool CVDR::ProcessComMessage(CComMessage* pComMsg)
{
    RecvAndHandleVoiceDataFromVoice(pComMsg);
	return true;
}
#endif

//创建socket
bool CVDR::CreateSocket()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CVDR::CreateSocket");
#endif
    sockaddr_in server;
    m_UdpSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_UdpSocket<0)
    {
#ifdef __USE_LWIP__
        LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "TVDR Create socket failed. ");
#else
        LOG1(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "Create socket failed. errcode[%d]", getErrNO());
#endif
        return false;
    }
#ifndef __USE_LWIP__
	UINT32 on=1;
	if(ioctl(m_UdpSocket, FIONBIO, (int)&on) < 0) 
	{ 
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "make socket unblock fail.");
		OutputSocketErrCode("ioctl(m_UdpSocket, FIONBIO, (u_long*)&on)");
	} 
#endif	
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(m_LocalPort);
    if (::bind(m_UdpSocket, (sockaddr*)&server, sizeof(server))<0)
    {
        LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), "bind socket failed.");
        return false;
    }

	this->setTosVal(g_TOS);
	
#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		m_pNatpiSession->setSocket(m_UdpSocket);
		m_pNatpiSession->start();	
	}
#endif	
    return true;
}
//关闭socket
void CVDR::CloseSocket()
{
#ifdef NATAP_FUNCTION
	if(m_pNatpiSession->IsInEffect())
	{
		m_pNatpiSession->setSocket(-1);
		m_pNatpiSession->stop();
	}
#endif

#ifdef __WIN32_SIM__
	closesocket(m_UdpSocket);
#else	//vxworks
	close(m_UdpSocket);
#endif
	m_UdpSocket = -1;

}

///////////////////////////////////////////////////////////////////////////////

//创建Pipe
bool CVDR::CreatePipe()
{
#ifndef DSP_BIOS
#ifdef __WIN32_SIM__
	return true;
#else		//vxworks
	return pipeDevCreate(m_szPipeName, M_TASK_TVDR_MAXMSG, sizeof(int))==OK ? true:false;
#endif	
#else
	return true;
#endif
}
//打开Pipe
bool CVDR::OpenPipe()
{
#ifndef DSP_BIOS
    clrVdrPipeStatus();
#ifdef __WIN32_SIM__
    sockaddr_in server;
    m_fdPipe = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_fdPipe<0)
    {
        LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "Create socket failed. errcode[%d]", getErrNO());
        return false;
    }
	
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(M_WIN32_VDR_PIPEPORT);
    if (::bind(m_fdPipe, (sockaddr*)&server, sizeof(server))<0)
    {
        LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "bind socket failed.");
        return false;
    }
    	
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
	//int ret = ioctl(m_fdPipe, FIONBIO, (int)&on);
	//if ( ret < 0 ) // make handle non-block 
	if(0)
#endif
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "make pipe unblock failed!!!");
		return false;
	}	

    return true;
#else//#ifndef DSP_BIOS
	return true;
#endif//#ifndef DSP_BIOS
}
//删除Pipe
bool CVDR::DeletePipe()
{
#ifndef DSP_BIOS
#ifdef __WIN32_SIM__
	return true;
#else		//vxworks
	return pipeDevDelete(m_szPipeName, true)==OK;
#endif
#else
	return true;
#endif
}
//关闭管道
bool CVDR::ClosePipe()
{
#ifndef DSP_BIOS
#ifdef __WIN32_SIM__
	closesocket(m_fdPipe);
	return true;
#else		//vxworks
	return close(m_fdPipe)!=ERROR;
#endif
#else
	return true;
#endif
}
//向本任务发送消息
bool CVDR::PostMessage(CComMessage* pMsg,	SINT32 timeout, bool isUrgent)
{
	vdrPipeCnt.nPostCalled++;
	if(NULL==pMsg)
	{
		vdrPipeCnt.nPostNull++;
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL),"POST NULL MSG!!!!");
		return false;
	}
	//如果没有主备sag连接
	if(!sagStatusFlag)
	{
		//如果使用本地sag故障弱化
		if(g_blUseLocalSag)
		{
			//语音包发送到本地sag任务
			if(MSGID_VOICE_VDR_DATA==pMsg->GetMessageId())
			{
				//to local sag
				pMsg->SetDstTid(M_TID_SAG);
				return CComEntity::PostEntityMessage(pMsg);	
			}
		}
	}

	//如果本实例连接主sag
	if( isMasterInstance() )
	{
		//并且主sag断开，语音包发送给备用sag，其他消息照旧
		if( !CVCR::GetInstance()->IsSAGConnected() )
		{
			if(MSGID_VOICE_VDR_DATA==pMsg->GetMessageId())
			{
				pMsg->SetDstTid(M_TID_VDR1);
				return CComEntity::PostEntityMessage(pMsg);
			}
		}
	}
	
	if(!m_blEffect)
	{
		pMsg->AddRef();
		return false;
	}
#ifdef DSP_BIOS
	if(m_UdpSocket==-1)
	{
		pMsg->AddRef();
		return false;
	}
	return CBizTask::PostMessage( pMsg, timeout, isUrgent);
#else//#ifdef DSP_BIOS
	if (false == pMsg->AddRef())
	{
		return false;
	}
    
	int ret;
	//往管道中发送pMsg
#ifdef __WIN32_SIM__
	sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1");
	dest.sin_port = htons(M_WIN32_VDR_PIPEPORT);
	ret = sendto(m_fdPipe, (char*)&pMsg, 4, 0, (sockaddr*)&dest, sizeof(dest));
#else
	ret = write(m_fdPipe, (char*)&pMsg, sizeof(pMsg));
#endif	
	if(sizeof(pMsg)==ret)
	{
#if 0
		LOG3(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL),
			"\nVDR POST Msg[MSGID(%x) SRCTID(%d) DSTTID(%d)]\n",
			pMsg->GetMessageId(), 
			pMsg->GetSrcTid(), 
			pMsg->GetDstTid());
#endif
		vdrPipeCnt.nEnterPipe++;
		return true;
	}
	else
	{
#if 1	
		LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL),
			"\nVDR POST MSG[%x] ERR!!!!!\n",pMsg->GetMessageId());
#endif
		vdrPipeCnt.nPostFail++;

		return false;
	}
#endif//#ifdef DSP_BIOS
}
#ifdef DSP_BIOS
extern "C" UINT32 GetBtsIpAddr();
#endif
UINT32 CVDR::GetBtsSagIpAddr()
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

