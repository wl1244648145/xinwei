#include "tVoiceData.h"
#include "tVoiceSignal.h"
#include "SAGSim.h"
#include "log.h"
//////////////////////////////////////////////////////////////////////////

void FillLAI(UINT8* pLAI, UINT32 LAIval)
{
	pLAI[0] = LAIval & 0x00ff0000;
	pLAI[1] = LAIval & 0x0000ff00;
	pLAI[2] = LAIval & 0x000000ff;
}



bool  InitL3Oam(void)
{
	//printf("===========================================");
	return true;
}

CSAG sag;

bool  InitL3VoiceSvc(void)
{
	sag.Init();
	
	
	CTask_VoiceSignal* pTaskVoiceSignal = CTask_VoiceSignal::GetInstance();
	pTaskVoiceSignal->Begin();

	Sleep(2000);

	CTask_VoiceData* pTaskVoiceData = CTask_VoiceData::GetInstance();
	pTaskVoiceData->Begin();

/*	
	char *p = "aaa";
	string str1(p);
	string str2("bbb");
	string str3 = p;

	printf("\n%s", str1.c_str());
	printf("\n%s", str2.c_str());
	printf("\n%s", str3.c_str());
*/
	return true;
}
bool  InitL3DataSvc(void)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CCCB::clearCCBCallInfo()
{
	setCodec(CODEC_G729A);
	setL3Addr(NO_L3ADDR);
	setRemoteCCB(NULL);
	setDialedNumber("");
	setState(IDLE_STATE);
}

void CCCB::ClearCCBInfo()
{
	setCodec(CODEC_G729A);
	setUID(INVALID_UID);
	setL3Addr(NO_L3ADDR);
	setRemoteCCB(NULL);
	setLocalNumber("");
	setDialedNumber("");
}
UINT32 CCCB::getUID()
{
	return m_Uid;
}
void CCCB::setUID(UINT32 uid)
{
	m_Uid = uid;
}
UINT32 CCCB::getL3Addr()
{
	return m_L3Addr;
}
void CCCB::setL3Addr(UINT32 l3Addr)
{
	m_L3Addr = l3Addr;
}
/*
UINT16 CCCB::getBTSID()
{
	return m_BTSID;
}
void CCCB::setBTSID(UINT16 BTSID)
{
	m_BTSID = BTSID;
}
UINT16 CCCB::getLAI()
{
	return m_LAI;
}
void CCCB::setLAI(UINT16 LAI)
{
	m_LAI = LAI;
}
*/
CCCB* CCCB::getRemoteCCB()
{
	return m_pRemoteCCB;
}
void CCCB::setRemoteCCB(CCCB* pCCB)
{
	m_pRemoteCCB = pCCB;
}
char* CCCB::getLocalNumber()
{
	return m_LocalNumber;
}
void CCCB::setLocalNumber(char* LocalNumber)
{
	if(strlen(LocalNumber)<sizeof(m_LocalNumber))
		strcpy(m_LocalNumber, LocalNumber);
	else
	{
		LOG(LOG_DEBUG3, 0, "Local Number too long!!!");
		strncpy(m_LocalNumber, LocalNumber, sizeof(m_LocalNumber)-1);
		m_LocalNumber[sizeof(m_LocalNumber)-1] = 0;
	}
}
void CCCB::SaveDialedNumber(char Digit)
{
	m_DialedNumber[m_DialedNumberLen++] = Digit;
	m_DialedNumber[m_DialedNumberLen] = 0;
}
char* CCCB::getDialedNumber()
{
	return m_DialedNumber;
}
void CCCB::setDialedNumber(char* number)
{
	if(strlen(number)<sizeof(m_DialedNumber))
		strcpy(m_DialedNumber, number);
	else
	{
		LOG(LOG_DEBUG3, 0, "CCCB::setDialedNumber(number), number len too long!!!");
		strncpy(m_DialedNumber, number, sizeof(m_DialedNumber)-1);
		m_LocalNumber[sizeof(m_DialedNumber)-1] = 0;
	}
	m_DialedNumberLen = strlen(m_DialedNumber);
}
UINT16 CCCB::getCCBTableIndex()
{
	return m_TabIndex;
}
void CCCB::setCCBTableIndex(UINT16 TabIndex)
{
	m_TabIndex = TabIndex;
}
UINT8 CCCB::getCodec()
{
	return m_Codec;
}
void CCCB::setCodec(UINT8 codec)
{
	m_Codec = codec;
}
UINT8 CCCB::getDialedNumberLen()
{
	return m_DialedNumberLen;
}
void CCCB::setDialedNumberLen(UINT8 len)
{
	m_DialedNumberLen = len;
}
UINT16 CCCB::getTabIndex()
{
	return m_TabIndex;
}
void CCCB::setTabIndex(UINT16 index)
{
	m_TabIndex = index;
}
UINT8 CCCB::getState()
{
	return m_State;
}
void CCCB::setState(UINT8 state)
{
	m_State = state;
}
bool CCCB::isOrigCall()
{
	return m_blOrigCall;
}
void CCCB::setOrigCall(bool blOrigCall)
{
	m_blOrigCall = blOrigCall;
}
UINT8 CCCB::getAuthReason()
{
	return m_AuthReason;
}
void CCCB::setAuthReason(UINT8 reason)
{
	m_AuthReason = reason;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
UINT32 CSAG::m_L3AddrRes = 1;

void CSAG::Init()
{
	int i;
	//SAG configuration
	setSAGID(1);
	setPC(100);
	//配置BTSID,LAI,BTS的语音数据接收地址
	btsTbl[0].setBTSID(17);
	btsTbl[0].setPC(201);
	btsTbl[0].setLAI(1);
	btsTbl[0].m_destAddr.sin_family = AF_INET;
	btsTbl[0].m_destAddr.sin_addr.s_addr = inet_addr("192.168.2.247");
	btsTbl[0].m_destAddr.sin_port = htons(7777);
	
	btsTbl[1].setBTSID(18);
	btsTbl[1].setPC(1002);
	btsTbl[1].setLAI(2);
	btsTbl[1].m_destAddr.sin_family = AF_INET;
	btsTbl[1].m_destAddr.sin_addr.s_addr = inet_addr("192.168.2.254");
	btsTbl[1].m_destAddr.sin_port = htons(7777);
	
	//init UID list
	//m_UidList.push_back(0x12345620);
	//m_UidList.push_back(0x12345621);
	//根据配置文件初始化合法用户列表
	m_valid_User_Table.insert(VALID_USER_TABLE::value_type(0x12345620,"31"));
	m_valid_User_Table.insert(VALID_USER_TABLE::value_type(0x12345621,"32"));
	//根据配置文件初始化合法用户电话号码本
	m_Phone_Address_Book.insert(PHONE_ADDRESS_BOOK::value_type("31", 0x12345620));
	m_Phone_Address_Book.insert(PHONE_ADDRESS_BOOK::value_type("32", 0x12345621));
	//根据配置文件初始化拨号计划
	
	//初始化空闲CCB池
	for(i=0;i<M_MAX_CCB_NUM;i++)
	{
		m_CCBTable[i].setTabIndex(i);
	}
}
UINT32 CSAG::AllocateL3Addr()
{
	return m_L3AddrRes>0xffffff00 ? 1:(m_L3AddrRes++);
}

CCCB* CSAG::AllocCCB(UINT32 uid)
{
	int i;
	CCCB * pCCB = FindCCBByUID(uid);
	if(pCCB!=NULL)
		return pCCB;
	for(i=0;i<M_MAX_CCB_NUM;i++)
	{
		if(INVALID_UID==m_CCBTable[i].getUID())
		{
			return &m_CCBTable[i];
		}		
	}
	LOG(LOG_DEBUG3, 0, "ALL CCBs used up!!!");
	return NULL;
}
void CSAG::DeAllocCCB(UINT16 TabIndex)
{
	if(TabIndex<M_MAX_CCB_NUM)
	{
		m_CCBTable[TabIndex].ClearCCBInfo();
	}
	else
	{
		LOG(LOG_DEBUG3, 0, "CSAG::DeAllocCCB(UINT16 TabIndex), TabIndex out of range!!!");
	}
}

CCCB* CSAG::FindCCBByUID(UINT32 uid)
{
	UID_INTDEX_TABLE::iterator it = m_Uid_index_Table.find(uid);
	return (it==m_Uid_index_Table.end()) ? NULL: (*it).second;	
}

CCCB* CSAG::FindCCBByL3Addr(UINT32 l3Addr)
{
	L3ADDR_INDEX_TABLE::iterator it = m_L3Addr_index_Table.find(l3Addr);
	return (it==m_L3Addr_index_Table.end()) ? NULL: (*it).second;	
}

//查找被叫CPE时使用
CCCB* CSAG::FindCCBByLocalNumber(char* number)
{
	//根据LocalNumber找到UID
	UINT32 Uid;
	string str_num(number);
	PHONE_ADDRESS_BOOK::iterator it = m_Phone_Address_Book.find(str_num);
	if(m_Phone_Address_Book.end()==it)
	{
		LOG(LOG_DEBUG3, 0, "Number[%s] Dialed does not exist!!!", number);
		return NULL;
	}
	else
	{
		Uid = (*it).second;
	}
	//验证是否合法用户
	if(!IsValidUser(Uid))
		return NULL;
	//根据UID找到CCB
	return FindCCBByUID(Uid);
}

//验证用户是否合法，在配置文件中有登记，收到login请求时需要调用
bool CSAG::IsValidUser(UINT32 uid)
{
	VALID_USER_TABLE::iterator it = m_valid_User_Table.find(uid);
	return (m_valid_User_Table.end()==it) ? false:true;
}

void CSAG::AddUIDIndexTable(UINT32 uid, CCCB* pCCB)
{
	if(INVALID_UID==uid || NULL==pCCB)
		return;
	m_Uid_index_Table.insert(UID_INTDEX_TABLE::value_type(uid,pCCB));
}

void CSAG::DelUIDIndexTable(UINT32 uid)
{
	if(INVALID_UID==uid)
		return;
	UID_INTDEX_TABLE::iterator it = m_Uid_index_Table.find(uid);
	if(m_Uid_index_Table.end()!=it)
		m_Uid_index_Table.erase(it);
}

void CSAG::AddPhoneAddressBook(char* number, UINT32 uid)
{
	if(NULL==number || 0==strlen(number) || INVALID_UID==uid)
		return;
	string strNumber(number);
	m_Phone_Address_Book.insert(PHONE_ADDRESS_BOOK::value_type(strNumber, uid));
}

void CSAG::DelPhoneAddressBook(char* number)
{
	if(NULL==number || 0==strlen(number))
		return;
	string strNumber(number);
	PHONE_ADDRESS_BOOK::iterator it = m_Phone_Address_Book.find(strNumber);
	if(m_Phone_Address_Book.end()!=it)
		m_Phone_Address_Book.erase(it);
}

void CSAG::AddL3AddrIndexTable(UINT32 l3Addr, CCCB* pCCB)
{
	if(NO_L3ADDR==l3Addr || SMS_L3ADDR==l3Addr || NULL==pCCB)
		return;

	L3ADDR_INDEX_TABLE::iterator it = m_L3Addr_index_Table.find(l3Addr);
	if(m_L3Addr_index_Table.end()!=it)
		m_L3Addr_index_Table.erase(it);

	m_L3Addr_index_Table.insert(L3ADDR_INDEX_TABLE::value_type(l3Addr, pCCB));
}

void CSAG::DelL3AddrIndexTable(UINT32 l3Addr)
{
	if(NO_L3ADDR==l3Addr || SMS_L3ADDR==l3Addr)
		return;
	L3ADDR_INDEX_TABLE::iterator it = m_L3Addr_index_Table.find(l3Addr);
	if(m_L3Addr_index_Table.end()!=it)
		m_L3Addr_index_Table.erase(it);
}

void CSAG::AddValidUserList(UINT32 uid, char* number)
{
	if(INVALID_UID==uid || NULL==number || 0==strlen(number))
		return;
	string strNumber(number);
	m_valid_User_Table.insert(VALID_USER_TABLE::value_type(uid, strNumber));
}

void CSAG::DelValidUserList(UINT32 uid)
{
	if(INVALID_UID==uid)
		return;
	VALID_USER_TABLE::iterator it = m_valid_User_Table.find(uid);
	if(m_valid_User_Table.end()!=it)
		m_valid_User_Table.erase(it);
}

//void CSAG::handleSignalLAPagingReq(CSAbisSignal& signal, int fdSocket);
void CSAG::handleSignalLAPagingRsp(CSAbisSignal& signal, int fdSocket)
{
	//收到被叫寻呼回应发送DeLAPaging
	//+++

	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT8 PagingResult = pSigBufRcv->sigPayload.LAPagingRsp.Cause ;
	UINT16 AppType = ntohs(pSigBufRcv->sigPayload.LAPagingRsp.App_Type);
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.LAPagingRsp.UID);
	CCCB* pCCB = FindCCBByUID(uid);
	if(NULL==pCCB)
		return;

	//语音呼叫
	if(APPTYPE_VOICE_QCELP==AppType || APPTYPE_VOICE_G729==AppType)
	{
		if(0==PagingResult)	//成功
		{
			//发送AuthCmdReq,标记鉴权原因为被叫
			CSAbisSignal AuthCmdReq;
			SAbisSignalT* pSigBuf = (SAbisSignalT*)AuthCmdReq.GetDataPtr();
			setAllHeadTailFields(pCCB->getSrvBTS(),
								 this,
								 sizeof(AuthCmdReqT),
								 UTSAG_UID_MSG,
								 AuthCmdReq);
								 
/*
			AuthCmdReq.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
			AuthCmdReq.SetSigIDS(UTSAG_UID_MSG);
			AuthCmdReq.SetSigHeaderLengthField(sizeof(AuthCmdReqT));
			AuthCmdReq.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(AuthCmdReqT)+sizeof(EndFlagT));
			AuthCmdReq.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(AuthCmdReqT)-sizeof(HeadFlagT));
*/			
			pSigBuf->sigPayload.AuthCmdReq.containerHeader.msgType = M_MSGTYPE_AUTHCMD;
			pSigBuf->sigPayload.AuthCmdReq.containerHeader.uid_L3addr = htonl(pCCB->getUID());
			for(int ss=0;ss<16;ss++)
				pSigBuf->sigPayload.AuthCmdReq.Rand[ss] = ss;

			sendSignalToBTS(AuthCmdReq, fdSocket);
			
			pCCB->setAuthReason(AUTH_REASON_MTCALL);
		}
		else	//失败
		{
			//发送Disconnect给主叫
			if(NULL!=pCCB->getRemoteCCB())
			{
				sendDisconnect(pCCB->getRemoteCCB(), pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket, REL_CAUSE_UNKNOWN);
			}
		}
	}
	
	//SMS
	if(APPTYPE_SMS==AppType)
	{
		//发送所有缓存的SMS;
		//+++

	}

	//+++PagingNow = false;

}
//void CSAG::handleSignalDeLAPagingReq(CSAbisSignal& signal, int fdSocket); 
void CSAG::handleSignalDeLAPagingRsp(CSAbisSignal& signal, int fdSocket)
{
	//do nothing
}
void CSAG::handleSignalAssignResReq(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.AssignResReq.UID);
	CCCB* pCCB = FindCCBByUID(uid);
	if(NULL==pCCB)
	{
		return;
	}
	//分配L3Addr,更新CCB的L3Addr和SAG中L3Addr相关的查找表
	UINT32 L3Addr = AllocateL3Addr();
	pCCB->setL3Addr(L3Addr);
	AddL3AddrIndexTable(L3Addr, pCCB);
	//回应AssignResRsp
	CSAbisSignal AssgnResRsp;
	SAbisSignalT* pSigBuf = (SAbisSignalT*)AssgnResRsp.GetDataPtr();

	setAllHeadTailFields(pCCB->getSrvBTS(),
						 this,
						 sizeof(AssignResRspT),
						 AssignResRsp_MSG,
						 AssgnResRsp);
/*	
	AssgnResRsp.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
	AssgnResRsp.SetSigIDS(AssignResRsp_MSG);
	AssgnResRsp.SetSigHeaderLengthField(sizeof(AssignResRspT));
	AssgnResRsp.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(AssignResRspT)+sizeof(EndFlagT));
	AssgnResRsp.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(AssignResRspT)-sizeof(HeadFlagT));
*/	
	pSigBuf->sigPayload.AssignResRsp.AssignResult = 0;
	pSigBuf->sigPayload.AssignResRsp.L3addr = htonl(L3Addr);
	pSigBuf->sigPayload.AssignResRsp.UID = htonl(pCCB->getUID());

	sendSignalToBTS(AssgnResRsp, fdSocket);	

	//如果是切换的分配资源，则
	//记录为CCB服务的BTS
	if(1==pSigBufRcv->sigPayload.AssignResReq.AssignReason)
	{
		CBTS* pBTS = findBTSByPC( ntohs(pSigBufRcv->tcpPktHeader.SPC_Code) );
		pCCB->setOldBTS(pCCB->getSrvBTS());
		pCCB->setSrvBTS(pBTS);
	}

}
void CSAG::handleSignalAssignResRsp(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.AssignResRsp.UID);
	CCCB* pCCB = FindCCBByUID(uid);
	if(NULL==pCCB)
	{
		return;
	}
	//如果NAK,发送Disconnect给主叫,释放被叫的L3地址
	if(0!=pSigBufRcv->sigPayload.AssignResRsp.AssignResult)
	{
		if(NULL!=pCCB->getRemoteCCB())
		{
			sendDisconnect(pCCB->getRemoteCCB(), pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket, REL_CAUSE_TRANSPORTRESFAIL);
			
			DelL3AddrIndexTable(pCCB->getL3Addr());
			pCCB->setL3Addr(NO_L3ADDR);
		}
	}
	//如果ACK,发送Setup给被叫,更新被叫CCB的L3地址和L3Addr相关查找表,标记被叫CCB为被叫
	else
	{
		CSAbisSignal Setup;
		SAbisSignalT* pSigSetup = (SAbisSignalT*)Setup.GetDataPtr();
		
		setAllHeadTailFields(pCCB->getSrvBTS(),
							 this,
							 sizeof(SetupSAGT),
							 UTSAG_L3Addr_MSG,
							 Setup);		
/*		
		Setup.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
		Setup.SetSigIDS(UTSAG_L3Addr_MSG);
		Setup.SetSigHeaderLengthField(sizeof(SetupSAGT));
		Setup.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(SetupSAGT)+sizeof(EndFlagT));
		Setup.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(SetupSAGT)-sizeof(HeadFlagT));
*/		
		pSigSetup->sigPayload.SetupSAG.containerHeader.msgType = M_MSGTYPE_SETUP;
		pSigSetup->sigPayload.SetupSAG.containerHeader.uid_L3addr = htonl(pCCB->getL3Addr());
		pSigSetup->sigPayload.SetupSAG.SelCodecinfo = CODEC_G729A;
		pSigSetup->sigPayload.SetupSAG.Digit_Num = 2;
		pSigSetup->sigPayload.SetupSAG.Digits[0] = 0xA3;	//主叫号码固定送30
		
		sendSignalToBTS(Setup, fdSocket);

		pCCB->setOrigCall(false);
	}
}
//void CSAG::handleSignalReleaseResReq(CSAbisSignal& signal, int fdSocket); 
void CSAG::handleSignalReleaseResRsp(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = ntohl(pSigBufRcv->sigPayload.RlsResRsp.L3addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(NULL==pCCB)
	{
		return;
	}
	//清除呼叫相关信息,l3地址作废
	DelL3AddrIndexTable(pCCB->getL3Addr());
	pCCB->clearCCBCallInfo();
}

void CSAG::handleSignalReset(CSAbisSignal& signal, int fdSocket)
{
	//回应ResetAck
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();

	//更新BTS成员的信息
	UINT16 btsid = ntohs(pSigBufRcv->tcpPktHeader.SPC_Code);
	CBTS* pBTS = findBTSByPC(btsid);
	if(NULL!=pBTS)
	{
		pBTS->m_fdSignalSocket = fdSocket;
	}

	CSAbisSignal ResetAck;
	
	setAllHeadTailFields(pBTS,
						 this,
						 0,
						 ResetAck_MSG,
						 ResetAck);	
/*
	ResetAck.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
	ResetAck.SetSigIDS(ResetAck_MSG);
	//ResetAck信令没有内容
	ResetAck.SetSigHeaderLengthField(0);
	ResetAck.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ 0 +sizeof(EndFlagT));
	ResetAck.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ 0 -sizeof(HeadFlagT));
*/	
	sendSignalToBTS(ResetAck, fdSocket);

	//向新连接的BTS发送CPE指令登记
	CSAbisSignal LAPagingReq;
	SAbisSignalT* pSigPagingReq = (SAbisSignalT*)LAPagingReq.GetDataPtr();

	setAllHeadTailFields(pBTS,
		this,
		sizeof(LAPagingT),
		LAPaging_MSG,
		LAPagingReq);

	pSigPagingReq->sigPayload.LAPaging.App_Type = htons(APPTYPE_CMD_REG);
	pSigPagingReq->sigPayload.LAPaging.L3addr = htonl(NO_L3ADDR);
	//			pSigPagingReq->sigPayload.LAPaging.UT_Capability = 0;
	pSigPagingReq->sigPayload.LAPaging.VersionInfo[0] = 0;
	pSigPagingReq->sigPayload.LAPaging.VersionInfo[1] = 0;

	for (UINT i=0;i<m_UidList.size();i++) 
	{
		pSigPagingReq->sigPayload.LAPaging.UID = htonl(m_UidList[i]);
		sendSignalToBTS(LAPagingReq, fdSocket);
	}

}
void CSAG::handleSignalResetAck(CSAbisSignal& signal, int fdSocket)
{
	//do nothing
}
void CSAG::handleSignalErrNotiReq(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.ErrNotifyReq.Uid);
	CCCB* pCCB = FindCCBByUID(uid);
	if(NULL==pCCB)
	{
		return;
	}	
	//回应ErrorNotiRsp
	CSAbisSignal ErrorNotiRsp;
	SAbisSignalT* pSigErrRsp = (SAbisSignalT*)ErrorNotiRsp.GetDataPtr();

	setAllHeadTailFields(pCCB->getSrvBTS(),
						 this,
						 sizeof(ErrNotifyRspT),
						 ErrNotifyRsp_MSG,
						 ErrorNotiRsp);
/*	
	ErrorNotiRsp.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
	ErrorNotiRsp.SetSigIDS(ErrNotifyRsp_MSG);
	ErrorNotiRsp.SetSigHeaderLengthField(sizeof(ErrNotifyRspT));
	ErrorNotiRsp.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(ErrNotifyRspT)+sizeof(EndFlagT));
	ErrorNotiRsp.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(ErrNotifyRspT)-sizeof(HeadFlagT));
*/	
	pSigErrRsp->sigPayload.ErrNotifyRsp.L3Addr = htonl(pCCB->getL3Addr());
	pSigErrRsp->sigPayload.ErrNotifyRsp.Uid = htonl(pCCB->getUID());
	
	sendSignalToBTS(ErrorNotiRsp, fdSocket);
	
	//发送Disconnect给主叫和被叫
	sendDisconnect(pCCB, fdSocket, REL_CAUSE_UNKNOWN);

	if(NULL!=pCCB->getRemoteCCB())
	{
		sendDisconnect(pCCB->getRemoteCCB(), pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket, REL_CAUSE_UNKNOWN);
	}

}
void CSAG::handleSignalErrNotiRsp(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.ErrNotifyRsp.Uid);
	CCCB* pCCB = FindCCBByUID(uid);
	if(NULL==pCCB)
	{
		return;
	}
	//发送Disconnect给主叫和被叫
	sendDisconnect(pCCB, fdSocket, REL_CAUSE_UNKNOWN);
	if(NULL!=pCCB->getRemoteCCB())
	{
		sendDisconnect(pCCB->getRemoteCCB(), pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket, REL_CAUSE_UNKNOWN);
	}
}
void CSAG::handleSignalBeatHeart(CSAbisSignal& signal, int fdSocket)
{
	//回应BeatHeartAck
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	
	CSAbisSignal BeatHeartAck;
	SAbisSignalT* pData = (SAbisSignalT*)BeatHeartAck.GetDataPtr();

	UINT16 btspc = ntohs(pSigBufRcv->tcpPktHeader.SPC_Code);
	CBTS* pBTS = findBTSByPC(btspc);
	
	setAllHeadTailFields(pBTS,
						 this,
						 sizeof(BeatHeartAckT),
						 BeatHeartAck_MSG,
						 BeatHeartAck);	
/*	
	BeatHeartAck.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
	BeatHeartAck.SetSigIDS(BeatHeartAck_MSG);
	BeatHeartAck.SetSigHeaderLengthField(sizeof(BeatHeartAckT));
	BeatHeartAck.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ sizeof(BeatHeartAckT) +sizeof(EndFlagT));
	BeatHeartAck.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ sizeof(BeatHeartAckT) -sizeof(HeadFlagT));
*/	
	pData->sigPayload.BeatHeartAck.sequence = pSigBufRcv->sigPayload.BeatHeart.sequence;
	sendSignalToBTS(BeatHeartAck, fdSocket);
}
void CSAG::handleSignalBeatHeartAck(CSAbisSignal& signal, int fdSocket)
{
	//do nothing
}
void CSAG::handleSignalCongestReq(CSAbisSignal& signal, int fdSocket)
{
	//回应CongestRsp
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	
	CSAbisSignal CongestRsp;
	SAbisSignalT* pData = (SAbisSignalT*)CongestRsp.GetDataPtr();

	UINT16 btspc = ntohs(pSigBufRcv->tcpPktHeader.SPC_Code);
	CBTS* pBTS = findBTSByPC(btspc);
	setAllHeadTailFields(pBTS,
						 this,
						 sizeof(CongestionCtrlRspT),
						 CongestionCtrlRsp_MSG,
						 CongestRsp);
/*	
	CongestRsp.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
	CongestRsp.SetSigIDS(CongestionCtrlRsp_MSG);
	CongestRsp.SetSigHeaderLengthField(sizeof(CongestionCtrlRspT));
	CongestRsp.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ sizeof(CongestionCtrlRspT) +sizeof(EndFlagT));
	CongestRsp.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ sizeof(CongestionCtrlRspT) -sizeof(HeadFlagT));
*/	
	pData->sigPayload.CongestionCtrlRsp.level = 0;
	sendSignalToBTS(CongestRsp, fdSocket);
}
void CSAG::handleSignalCongestRsp(CSAbisSignal& signal, int fdSocket)
{
	//do nothing
}

void CSAG::handleSignalSetup(CSAbisSignal& signal, int fdSocket)
{
	//给主叫回应SetupAck
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = ntohl(pSigBufRcv->sigPayload.SetupUT.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(pCCB!=NULL)
	{
		CSAbisSignal SetupAck;
		SAbisSignalT* pSigSetupAck = (SAbisSignalT*)SetupAck.GetDataPtr();

		setAllHeadTailFields(pCCB->getSrvBTS(),
							 this,
							 sizeof(CallProcT),
							 UTSAG_L3Addr_MSG,
							 SetupAck);
/*		
		SetupAck.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
		SetupAck.SetSigIDS(UTSAG_L3Addr_MSG);
		SetupAck.SetSigHeaderLengthField(sizeof(CallProcT));
		SetupAck.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(CallProcT)+sizeof(EndFlagT));
		SetupAck.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(CallProcT)-sizeof(HeadFlagT));
*/		
		pSigSetupAck->sigPayload.CallProc.containerHeader.msgType = M_MSGTYPE_CALLPROC;
		pSigSetupAck->sigPayload.CallProc.containerHeader.uid_L3addr = htonl(pCCB->getL3Addr());
		pSigSetupAck->sigPayload.CallProc.prio = M_DEFAULT_VOICE_PRIORITY;
		
		sendSignalToBTS(SetupAck, fdSocket);
		
		pCCB->setOrigCall(true);
		pCCB->setDialedNumber("");
		pCCB->setState(U3_STATE);		//主叫进入拨号状态，开始收号
	}
//+++暂不考虑下面的情况
/*
	if(Setup中带了被叫号码)
	{
		保存被叫号码;
		if(可以根据被叫号码找到被叫)
		{
			if(被叫非法用户)
			{
				//放提示音
			}
			if(被叫没有注册)
			{
				//放提示音
			}
			else
			{
				if(被叫空闲)
				{
					//给被叫分配L3Addr,发送LAPaging,PagingNow=true;
				}
				else
				{
					//发release给主叫
				}
			}
		}

	}
*/	
}
void CSAG::handleSignalSetupAck(CSAbisSignal& signal, int fdSocket)
{
	//不用转给另一方，不用处理

}
void CSAG::handleSignalAlerting(CSAbisSignal& signal, int fdSocket)
{
	//转给另一方
	SAbisSignalT* pData = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = ntohl(pData->sigPayload.Alerting.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(pCCB!=NULL)
	{
		if(pCCB->getRemoteCCB()!=NULL)
		{
			pData->sigPayload.Alerting.containerHeader.uid_L3addr = htonl(pCCB->getRemoteCCB()->getL3Addr());

			setAllHeadTailFields(pCCB->getRemoteCCB()->getSrvBTS(),
								 this,
								 sizeof(AlertingT),
								 UTSAG_L3Addr_MSG,
								 signal);
/*
			UINT16 bts_pc = signal.getSPC();
			UINT16 sag_pc = signal.getDPC();
			
			signal.setSigTcpPKTHeaderTail(sag_pc, bts_pc, sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ sizeof(AlertingT) -sizeof(HeadFlagT));
*/
			sendSignalToBTS(signal, pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket);
		}
	}
}
void CSAG::handleSignalConnect(CSAbisSignal& signal, int fdSocket)
{
	//转给另一方
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = ntohl(pSigBufRcv->sigPayload.ConnectUT.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(pCCB!=NULL && pCCB->getRemoteCCB()!=NULL)
	{
		CSAbisSignal ConnectSAG;
		SAbisSignalT* pData = (SAbisSignalT*)ConnectSAG.GetDataPtr();

		setAllHeadTailFields(pCCB->getRemoteCCB()->getSrvBTS(),
							 this,
							 sizeof(ConnectSAGT),
							 UTSAG_L3Addr_MSG,
							 ConnectSAG);
/*
		ConnectSAG.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
		ConnectSAG.SetSigIDS(UTSAG_L3Addr_MSG);
		ConnectSAG.SetSigHeaderLengthField(sizeof(ConnectSAGT));
		ConnectSAG.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ sizeof(ConnectSAGT) +sizeof(EndFlagT));
		ConnectSAG.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ sizeof(ConnectSAGT) -sizeof(HeadFlagT));
*/		
		pData->sigPayload.ConnectSAG.containerHeader.uid_L3addr = htonl(pCCB->getRemoteCCB()->getL3Addr());
		pData->sigPayload.ConnectSAG.containerHeader.msgType = M_MSGTYPE_CONNECT;
		pData->sigPayload.ConnectSAG.SelCodecInfo = CODEC_G729A;
		sendSignalToBTS(ConnectSAG, pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket);
	}	
}
void CSAG::handleSignalConnectAck(CSAbisSignal& signal, int fdSocket)
{
	//转给另一方
	SAbisSignalT* pData = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = ntohl(pData->sigPayload.ConnectAck.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(pCCB!=NULL)
	{
		if(pCCB->getRemoteCCB()!=NULL)
		{
			pData->sigPayload.Alerting.containerHeader.uid_L3addr = htonl(pCCB->getRemoteCCB()->getL3Addr());

			setAllHeadTailFields(pCCB->getRemoteCCB()->getSrvBTS(),
								 this,
								 sizeof(ConnectAckT),
								 UTSAG_L3Addr_MSG,
								 signal);
/*
			UINT16 bts_pc = signal.getSPC();
			UINT16 sag_pc = signal.getDPC();
			
			signal.setSigTcpPKTHeaderTail(sag_pc, bts_pc, sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+ sizeof(ConnectAckT) -sizeof(HeadFlagT));
*/			
			sendSignalToBTS(signal, pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket);
		}
	}	
}
/*
void CSAG::handleSignalDisconnect(CSAbisSignal& signal, int fdSocket)
{
	if()	//如果是先挂方
	{	
	//给先挂方发Release

	//给后挂方发Disconnect
	}
	else	//如果是后挂方
	{
	//给后挂方发送Release
	}

}*/
void CSAG::handleSignalDisconnect(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = ntohl(pSigBufRcv->sigPayload.Disconnect.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(NULL==pCCB)
	{
		return;
	}
	//以下是为了简便处理，某些情况下不用向远端发Disconnect
	//给本方发送Release
	sendRelease(pCCB, fdSocket, REL_CAUSE_NORMAL);
	//给远端发送Disconnect
	if(NULL!=pCCB->getRemoteCCB())
	{
		sendDisconnect(pCCB->getRemoteCCB(), pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket, REL_CAUSE_NORMAL);
	}	
}
void CSAG::handleSignalInformation(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = ntohl(pSigBufRcv->sigPayload.Information.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(NULL==pCCB)
	{
		return;
	}	
	//如果是主叫收号状态
	if(U3_STATE==pCCB->getState() && pCCB->isOrigCall())
	{
		//保存拨叫的号码
		if(0==pSigBufRcv->sigPayload.Information.type)	//拨号
		{
			char NumTab[17] = "D1234567890*#ABC";
			UINT8 DTMFDigit = pSigBufRcv->sigPayload.Information.num;
			if(DTMFDigit<16)
			{
				pCCB->SaveDialedNumber(NumTab[DTMFDigit]);
			}
		}
		//如果不符合拨号计划,回应Disconnect,结束主叫拨号状态,正常情况下应该是放语音等用户挂机
		int ret = checkDialPlan(pCCB->getDialedNumber(), pCCB->getUID());
		if( ret==-1 || ret==-2 )
		{
			sendDisconnect(pCCB, fdSocket, REL_CAUSE_UNKNOWN);

			pCCB->setState(IDLE_STATE);
			return;
		}
		if(ret==1)
		{
			//继续收号
			return;
		}
		//如果找到了被叫,给被叫分配L3Addr,向被叫发送LA Paging(voice call),结束主叫拨号状态,SetRemoteCCB
		if(ret==0)
		{
			CCCB* pCalledCCB = FindCCBByLocalNumber(pCCB->getDialedNumber());
			//++++暂时不考虑被叫正在进行呼叫和通话的情况
			UINT32 L3Addrcalled = AllocateL3Addr();
			pCalledCCB->setL3Addr(L3Addrcalled);
			AddL3AddrIndexTable(L3Addrcalled, pCalledCCB);
			pCCB->setRemoteCCB(pCalledCCB);
			pCalledCCB->setRemoteCCB(pCCB);
			pCalledCCB->setOrigCall(false);

			CSAbisSignal LAPagingReq;
			SAbisSignalT* pSigPagingReq = (SAbisSignalT*)LAPagingReq.GetDataPtr();

			setAllHeadTailFields(pCalledCCB->getSrvBTS(),
								 this,
								 sizeof(LAPagingT),
								 LAPaging_MSG,
								 LAPagingReq);
/*			
			LAPagingReq.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
			LAPagingReq.SetSigIDS(LAPaging_MSG);
			LAPagingReq.SetSigHeaderLengthField(sizeof(LAPagingT));
			LAPagingReq.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(LAPagingT)+sizeof(EndFlagT));
			LAPagingReq.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(LAPagingT)-sizeof(HeadFlagT));
*/			
			pSigPagingReq->sigPayload.LAPaging.App_Type = htons(APPTYPE_VOICE_QCELP);
			pSigPagingReq->sigPayload.LAPaging.L3addr = htonl(pCalledCCB->getL3Addr());
			pSigPagingReq->sigPayload.LAPaging.UID = htonl(pCalledCCB->getUID());
//			pSigPagingReq->sigPayload.LAPaging.UT_Capability = 0;
			pSigPagingReq->sigPayload.LAPaging.VersionInfo[0] = 0;
			pSigPagingReq->sigPayload.LAPaging.VersionInfo[1] = 0;

			sendSignalToBTS(LAPagingReq, pCalledCCB->getSrvBTS()->m_fdSignalSocket);	
			
			pCCB->setState(U4_STATE);
		}
	}
	
	
/*
	if(可以根据号码确定被叫)
	{
		if(被叫没有注册)
		{
			//放提示音
		}
		else
		{
			if(被叫空闲)
			{
				//给被叫分配L3Addr,发送LAPaging,PagingNow=true;
			}
			else
			{
				//发release给主叫
			}
		}
	}
*/
}


void CSAG::handleSignalReleaseComplete(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();

	UINT32 L3Addr = ntohl(pSigBufRcv->sigPayload.ReleaseComplete.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(NULL==pCCB)
	{
		LOG(LOG_DEBUG3, 0, "CSAG::handleSignalReleaseComplete, Cannot find CCB");
		return;
	}

	//发送ReleaseResReq
	CSAbisSignal ReleaseResReq;
	SAbisSignalT* pSigBuf = (SAbisSignalT*)ReleaseResReq.GetDataPtr();

	setAllHeadTailFields(pCCB->getSrvBTS(),
						 this,
						 sizeof(RlsResReqT),
						 RlsResReq_MSG,
						 ReleaseResReq);
/*	
	ReleaseResReq.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
	ReleaseResReq.SetSigIDS(RlsResReq_MSG);
	ReleaseResReq.SetSigHeaderLengthField(sizeof(RlsResReqT));
	ReleaseResReq.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(RlsResReqT)+sizeof(EndFlagT));
	ReleaseResReq.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(RlsResReqT)-sizeof(HeadFlagT));
*/	
	pSigBuf->sigPayload.RlsResReq.L3addr = htonl(pCCB->getL3Addr());
	pSigBuf->sigPayload.RlsResReq.RlsCause = REL_CAUSE_NORMAL;
	
	sendSignalToBTS(ReleaseResReq, fdSocket);
}
void CSAG::handleSignalModifyMediaTypeReq(CSAbisSignal& signal, int fdSocket)
{
	//转给另一方
	
}
void CSAG::handleSignalModifyMediaTypeRsp(CSAbisSignal& signal, int fdSocket)
{
	//转给另一方
	
}

void CSAG::handleSignalHandOverReq(CSAbisSignal& signal, int fdSocket)
{
	//回应HandOverRsp

	//记录为CCB的BTS
	//+++

}
//void CSAG::handleSignalHandOverRsp(CSAbisSignal& signal, int fdSocket);
void CSAG::handleSignalHandOverComplete(CSAbisSignal& signal, int fdSocket)
{
	//向原来的BTS发送ReleaseResReq

}

void CSAG::handleSignalLoginReq(CSAbisSignal& signal, int fdSocket)
{
	CCCB* pCCB = NULL;
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.Login.containerHeader.uid_L3addr);
	if(!IsValidUser(uid))
	{
		LOG(LOG_DEBUG3, 0, "Invalid User.UID[%d]", uid);
		//发送LoginRsp;
		CSAbisSignal LoginRsp;
		SAbisSignalT* pSigLoginRsp = (SAbisSignalT*)LoginRsp.GetDataPtr();

		UINT16 btspc = ntohs(pSigBufRcv->tcpPktHeader.SPC_Code);
		CBTS* pBTS = findBTSByPC(btspc);
		setAllHeadTailFields(pBTS,
							 this,
							 sizeof(LoginRspT),
							 UTSAG_UID_MSG,
							 LoginRsp);
/*		
		LoginRsp.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
		LoginRsp.SetSigIDS(UTSAG_UID_MSG);
		LoginRsp.SetSigHeaderLengthField(sizeof(LoginRspT));
		LoginRsp.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(LoginRspT)+sizeof(EndFlagT));
		LoginRsp.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(LoginRspT)-sizeof(HeadFlagT));
*/		
		pSigLoginRsp->sigPayload.LoginRsp.containerHeader.msgType = M_MSGTYPE_LOGINRSP;
		pSigLoginRsp->sigPayload.LoginRsp.containerHeader.uid_L3addr = htonl(uid);
		pSigLoginRsp->sigPayload.LoginRsp.LoginResult = LOGINRSP_NONUMBER;
		FillLAI(pSigLoginRsp->sigPayload.LoginRsp.LAI, pBTS->getLAI());
		pSigLoginRsp->sigPayload.LoginRsp.RegPeriod = htons(90);
		
		sendSignalToBTS(LoginRsp, fdSocket);
		return;
	}
	if(REGTYPE_POWERON==pSigBufRcv->sigPayload.Login.REG_TYPE || 
	   NULL == FindCCBByUID(uid) )
	{
		pCCB = AllocCCB(uid);
		if(pCCB!=NULL)
		{
			AddUIDIndexTable(uid, pCCB);
			//记录为CCB服务的BTS
			CBTS* pBTS = findBTSByPC( ntohs(pSigBufRcv->tcpPktHeader.SPC_Code) );
			pCCB->setSrvBTS(pBTS);
			pCCB->setOldBTS(NULL);
		}
		else
		{
			LOG(LOG_DEBUG3, 0, "Cannot Allocate CCB");
			return;
		}
	}

//		REGTYPE_POWERON=0,			//00H	开机登记
//		REGTYPE_NEWLOCATION,		//01H	位置更新
//		REGTYPE_PERIOD,				//02H	周期注册
//		REGTYPE_CMDREG,				//03H	指令登记


	pCCB = FindCCBByUID(uid);
	if(NULL==pCCB)
	{
		LOG(LOG_DEBUG3, 0, "Cannot find CCB");
		return;
	}
	//发送AuthCmdReq
	CSAbisSignal AuthCmdReq;
	SAbisSignalT* pSigBuf = (SAbisSignalT*)AuthCmdReq.GetDataPtr();

	setAllHeadTailFields(pCCB->getSrvBTS(),
						 this,
						 sizeof(AuthCmdReqT),
						 UTSAG_UID_MSG,
						 AuthCmdReq);
/*	
	AuthCmdReq.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
	AuthCmdReq.SetSigIDS(UTSAG_UID_MSG);
	AuthCmdReq.SetSigHeaderLengthField(sizeof(AuthCmdReqT));
	AuthCmdReq.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(AuthCmdReqT)+sizeof(EndFlagT));
	AuthCmdReq.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(AuthCmdReqT)-sizeof(HeadFlagT));
*/
	
	pSigBuf->sigPayload.AuthCmdReq.containerHeader.msgType = M_MSGTYPE_AUTHCMD;
	pSigBuf->sigPayload.AuthCmdReq.containerHeader.uid_L3addr = htonl(uid);
	for(int i=0;i<16;i++)
		pSigBuf->sigPayload.AuthCmdReq.Rand[i] = i;

	sendSignalToBTS(AuthCmdReq, fdSocket);

	//标记鉴权原因为login
	pCCB->setAuthReason(AUTH_REASON_LOGIN);
	
}
//void CSAG::handleSignalLoginRsp(CSAbisSignal& signal, int fdSocket);
void CSAG::handleSignalLogout(CSAbisSignal& signal, int fdSocket)
{
	//释放CCB
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.Logout.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByUID(uid);
	CCCB* pRemoteCCB = NULL;
	if(pCCB!=NULL)
	{
		//清除所有的CCB相关的查找表
		DelL3AddrIndexTable(pCCB->getL3Addr());
		DelUIDIndexTable(pCCB->getUID());
		if((pRemoteCCB=pCCB->getRemoteCCB())!=NULL)
		{
			pRemoteCCB->setRemoteCCB(NULL);
			
		}
		DeAllocCCB(pCCB->getTabIndex());
	}
}
//void CSAG::handleSignalAuthCmdReq(CSAbisSignal& signal, int fdSocket);
void CSAG::handleSignalAuthCmdRsp(CSAbisSignal& signal, int fdSocket)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.AuthCmdRsp.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByUID(uid); 
	CCCB* pRemoteCCB = pCCB->getRemoteCCB();
	if(NULL==pCCB)
	{
		LOG(LOG_DEBUG3, 0, "cannot find CCB");
		return;
	}
	UINT8 authType = pCCB->getAuthReason();
	if(pSigBufRcv->sigPayload.AuthCmdRsp.result==0)	//鉴权失败++++
	{
		if(AUTH_REASON_MTCALL==authType)	//被叫鉴权
		{
			//给主叫发送Release
			if(pRemoteCCB!=NULL)
			{
				sendDisconnect(pCCB->getRemoteCCB(), pCCB->getRemoteCCB()->getSrvBTS()->m_fdSignalSocket, REL_CAUSE_AUTHFAIL);
			}
		}
		if(AUTH_REASON_LOGIN==authType)		//注册鉴权
		{
			//发送LoginRsp;
			CSAbisSignal LoginRsp;
			SAbisSignalT* pSigLoginRsp = (SAbisSignalT*)LoginRsp.GetDataPtr();

			setAllHeadTailFields(pCCB->getSrvBTS(),
								 this,
								 sizeof(LoginRspT),
								 UTSAG_UID_MSG,
								 LoginRsp);
/*			
			LoginRsp.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
			LoginRsp.SetSigIDS(UTSAG_UID_MSG);
			LoginRsp.SetSigHeaderLengthField(sizeof(LoginRspT));
			LoginRsp.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(LoginRspT)+sizeof(EndFlagT));
			LoginRsp.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(LoginRspT)-sizeof(HeadFlagT));
*/
			pSigLoginRsp->sigPayload.LoginRsp.containerHeader.msgType = M_MSGTYPE_LOGINRSP;
			pSigLoginRsp->sigPayload.LoginRsp.containerHeader.uid_L3addr = htonl(uid);
			pSigLoginRsp->sigPayload.LoginRsp.LoginResult = LOGINRSP_NONUMBER;
			FillLAI(pSigLoginRsp->sigPayload.LoginRsp.LAI, pCCB->getSrvBTS()->getLAI());
			pSigLoginRsp->sigPayload.LoginRsp.RegPeriod = htons(90);

			sendSignalToBTS(LoginRsp, fdSocket);
			//清除CCB和CCB相关查找表
			DelUIDIndexTable(pCCB->getUID());
			DeAllocCCB(pCCB->getTabIndex());
		}
	}
	else//鉴权成功
	{
		if(AUTH_REASON_MTCALL==authType)	//被叫鉴权
		{
			//发送AssignResReq给被叫;
			CSAbisSignal AssignResReq;
			SAbisSignalT* pSigAssignResReq = (SAbisSignalT*)AssignResReq.GetDataPtr();

			setAllHeadTailFields(pCCB->getSrvBTS(),
								 this,
								 sizeof(AssignResReqT),
								 AssignResReq_MSG,
								 AssignResReq);
/*			
			AssignResReq.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
			AssignResReq.SetSigIDS(AssignResReq_MSG);
			AssignResReq.SetSigHeaderLengthField(sizeof(AssignResReqT));
			AssignResReq.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(AssignResReqT)+sizeof(EndFlagT));
			AssignResReq.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(AssignResReqT)-sizeof(HeadFlagT));
*/			
			pSigAssignResReq->sigPayload.AssignResReq.AssignReason = 0;
			pSigAssignResReq->sigPayload.AssignResReq.L3addr = htonl(pCCB->getL3Addr());
			pSigAssignResReq->sigPayload.AssignResReq.ReqRate = M_REQ_TRANS_RATE_DEFAULT;
			pSigAssignResReq->sigPayload.AssignResReq.UID = htonl(pCCB->getUID());

			sendSignalToBTS(AssignResReq, fdSocket);
			
		}
		if(AUTH_REASON_LOGIN==authType)		//注册鉴权
		{
			//发送LoginRsp;
			CSAbisSignal LoginRsp;
			SAbisSignalT* pSigLoginRsp = (SAbisSignalT*)LoginRsp.GetDataPtr();

			setAllHeadTailFields(pCCB->getSrvBTS(),
								 this,
								 sizeof(LoginRspT),
								 UTSAG_UID_MSG,
								 LoginRsp);
/*			
			LoginRsp.SetBTSSAGID(pSigBufRcv->sigHeader.BTS_ID, pSigBufRcv->sigHeader.SAG_ID);
			LoginRsp.SetSigIDS(UTSAG_UID_MSG);
			LoginRsp.SetSigHeaderLengthField(sizeof(LoginRspT));
			LoginRsp.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(LoginRspT)+sizeof(EndFlagT));
			LoginRsp.setSigTcpPKTHeaderTail(signal.getDPC(), signal.getSPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(LoginRspT)-sizeof(HeadFlagT));
*/			
			pSigLoginRsp->sigPayload.LoginRsp.containerHeader.msgType = M_MSGTYPE_LOGINRSP;
			pSigLoginRsp->sigPayload.LoginRsp.containerHeader.uid_L3addr = htonl(uid);
			pSigLoginRsp->sigPayload.LoginRsp.LoginResult = LOGINRSP_SUCCESS;
			FillLAI(pSigLoginRsp->sigPayload.LoginRsp.LAI, pCCB->getSrvBTS()->getLAI());
			pSigLoginRsp->sigPayload.LoginRsp.RegPeriod = htons(90);
			
			sendSignalToBTS(LoginRsp, fdSocket);

			pCCB->setUID(uid);
			//如果没有CCB,申请CCB,更新相关查找表


		}
	}
}

void CSAG::handleSignalMOSMSDataReq(CSAbisSignal& signal, int fdSocket)
{
/*
	if(MT为非法用户)
	{
		回应失败
		return;
	}
	if(MT未注册)
	{
		回应失败
		return;
	}
	if(MT在呼叫进行中)
	{
		转给MTCPE
	}
	else
	{

		//MTCCB保存此SMS

		if(!PagingNow)
		{
			//发送LAPaging，置PagingNow标记

		}

	}
*/	
}
//void CSAG::handleSignalMOSMSDataRsp(CSAbisSignal& signal, int fdSocket);
//void CSAG::handleSignalMTSMSDataReq(CSAbisSignal& signal, int fdSocket);
void CSAG::handleSignalMTSMSDataRsp(CSAbisSignal& signal, int fdSocket)
{
	//转给MO方的CPE

}
void CSAG::handleSignalSMSMemAvailReq(CSAbisSignal& signal, int fdSocket)
{
	//发送SMSMemAvailRsp

}
//void CSAG::handleSignalSMSMemAvailRsp(CSAbisSignal& signal, int fdSocket);







//0表示符合拨号计划，号码完整匹配,-1表示不符合,-2表示呼叫自己,1表示符合拨号计划,但需要继续收号
int CSAG::checkDialPlan(char *calledNumber, UINT32 UIDcalling)
{
	if(0==strlen(calledNumber))
	{
		return 1;
	}
	else if(1==strlen(calledNumber))
	{
		if(calledNumber[0]!='3')
			return -1;
		else
			return 1;
	}
	else if(2==strlen(calledNumber))
	{
		CCCB* pCCB = FindCCBByLocalNumber(calledNumber);
		if(pCCB!=NULL)
		{
			if(pCCB->getUID()==UIDcalling)
				return -2;
			else
				return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

}
char signalName[InvalidSignal_MSG+1][40]=
{
	"LAPaging_MSG",
	"LAPagingRsp_MSG",
	"DELAPagingReq_MSG",
	"DELAPagingRsp_MSG",
	"AssignResReq_MSG",
	"AssignResRsp_MSG",
	"RlsResReq_MSG",
	"RlsResRsp_MSG",
	"Reset_MSG",
	"ResetAck_MSG",
	"BeatHeart_MSG",
	"BeatHeartAck_MSG",
	"CongestionCtrlReq_MSG",
	"CongestionCtrlRsp_MSG",
	"ErrNotifyReq_MSG",
	"ErrNotifyRsp_MSG",
	
	"Setup_MSG",
	"CallProc_MSG",
	"Alerting_MSG",

	"Connect_MSG",
	"ConnectAck_MSG",
	"Disconnect_MSG",
	"Release_MSG",
	"ReleaseComplete_MSG",
	"ModiMediaReq_MSG",
	"ModiMediaRsp_MSG",
	"Information_MSG",
	"AuthCmdReq_MSG",
	"AuthCmdRsp_MSG",
	"Login_MSG",
	"LoginRsp_MSG",
	"Logout_MSG",
	"HandOverReq_MSG",
	"HandOverRsp_MSG",
	"HandOverComplete_MSG",
	"StatusQry_MSG",
	"Status_MSG",
	
	"MOSmsDataReq_MSG",
	"MOSmsDataRsp_MSG",
	"MTSmsDataReq_MSG",
	"MTSmsDataRsp_MSG",
	"SMSMemAvailReq_MSG",
	"SMSMemAvailRsp_MSG",
	
	"UTSAG_L3Addr_MSG",
	"UTSAG_UID_MSG",
	"InvalidSignal_MSG"
		
};
void ShowSignal(CSAbisSignal& signal)
{
	int i;
	SAbisSignalT* pData = (SAbisSignalT*)signal.GetDataPtr();
	UINT8* pU8;

	//Tcp packet header
	//			pData->tcpPktHeader.HeadFlag,
	//			pData->tcpPktHeader.PktLen,
	//			pData->tcpPktHeader.DPC_Code,
	//			pData->tcpPktHeader.SPC_Code,
	//			pData->tcpPktHeader.UserType,
	//			pData->tcpPktHeader.TTL
	
	printf("\n    TcpHeader: ");
	pU8 = (UINT8*)&pData->tcpPktHeader;
	for(i=0;i<sizeof(TcpPktHeaderT);i++)
	{
		if(0==i%2)
			printf(" ");
		printf(" %02x", pU8[i]);
	}
	

	//Sabis signal header
//			pData->sigHeader.SAG_ID
//			pData->sigHeader.BTS_ID
//			pData->sigHeader.EVENT_GROUP_ID
//			pData->sigHeader.Event_ID
//			pData->sigHeader.Length
	printf("\n    SAbis Signal Header: ");
	pU8 = (UINT8*)&pData->sigHeader;
	for(i=0;i<sizeof(SigHeaderT);i++)
	{
		printf(" %02x", pU8[i]);
	}

	//Sabis signal payload
	printf("\n    SAbis Signal Payload: ");
	pU8 = (UINT8*)&pData->sigPayload;
	for(i=0;i<ntohs(pData->sigHeader.Length);i++)
	{
		printf(" %02x", pU8[i]);
	}

	//Tcp packet end flag
	printf("\n    Tcp pkt EndFlag: ");
	pU8 = ((UINT8*)&pData->sigPayload) + ntohs(pData->sigHeader.Length);
	for(i=0;i<2;i++)
	{
		printf(" %02x", pU8[i]);
	}

}

void CSAG::sendSignalToBTS(CSAbisSignal& signal, int fdSocket)
{
	if (0==signal.GetDataLength())
	{
		printf("\nsignal DataLen==0, error");
		return;
	}
	CTask_VoiceSignal* pTask_signal = CTask_VoiceSignal::GetInstance();
	pTask_signal->SendASignal(fdSocket, signal);

	UINT8 signalType = signal.ParseMessageToSAG();
	if(UTSAG_L3Addr_MSG==signalType || UTSAG_UID_MSG==signalType)
	{
		signalType = signal.ParseUTSAGMsgToSAG();
	}
	if(signalType!=BeatHeart_MSG && signalType!=BeatHeartAck_MSG)
	{
		printf("\nSend [%s] Signal to BTS", signalName[signalType]);
		ShowSignal(signal);
	}
	
}

typedef void (CSAG::*FuncPtr)(CSAbisSignal&, int);
FuncPtr FuncTable[InvalidSignal_MSG+1]=
{
	NULL,//LAPaging_MSG=0,
		&CSAG::handleSignalLAPagingRsp,//LAPagingRsp_MSG,
		NULL,//DELAPagingReq_MSG,
		&CSAG::handleSignalDeLAPagingRsp,//DELAPagingRsp_MSG,
		&CSAG::handleSignalAssignResReq,//AssignResReq_MSG,
		&CSAG::handleSignalAssignResRsp,//AssignResRsp_MSG,
		NULL,//RlsResReq_MSG,
		&CSAG::handleSignalReleaseResRsp,//RlsResRsp_MSG,
		&CSAG::handleSignalReset,//Reset_MSG,
		&CSAG::handleSignalResetAck,//ResetAck_MSG,
		&CSAG::handleSignalBeatHeart,//BeatHeart_MSG,
		&CSAG::handleSignalBeatHeartAck,//BeatHeartAck_MSG,
		&CSAG::handleSignalCongestReq,//CongestionCtrlReq_MSG,
		&CSAG::handleSignalCongestRsp,//CongestionCtrlRsp_MSG,
		&CSAG::handleSignalErrNotiReq,//ErrNotifyReq_MSG,
		&CSAG::handleSignalErrNotiRsp,//ErrNotifyRsp_MSG,
		//
		&CSAG::handleSignalSetup,//Setup_MSG,
		&CSAG::handleSignalSetupAck,//CallProc_MSG,
		&CSAG::handleSignalAlerting,//Alerting_MSG,
		//
		&CSAG::handleSignalConnect,//Connect_MSG,
		&CSAG::handleSignalConnectAck,//ConnectAck_MSG,
		&CSAG::handleSignalDisconnect,//Disconnect_MSG,
		NULL,//Release_MSG,
		&CSAG::handleSignalReleaseComplete,//ReleaseComplete_MSG,
		&CSAG::handleSignalModifyMediaTypeReq,//ModiMediaReq_MSG,
		&CSAG::handleSignalModifyMediaTypeRsp,//ModiMediaRsp_MSG,
		&CSAG::handleSignalInformation,//Information_MSG,
		NULL,//AuthCmdReq_MSG,
		&CSAG::handleSignalAuthCmdRsp,//AuthCmdRsp_MSG,
		&CSAG::handleSignalLoginReq,//Login_MSG,
		NULL,//LoginRsp_MSG,
		&CSAG::handleSignalLogout,//Logout_MSG,
		&CSAG::handleSignalHandOverReq,//HandOverReq_MSG,
		NULL,//HandOverRsp_MSG,
		&CSAG::handleSignalHandOverComplete,//HandOverComplete_MSG,
		NULL,//StatusQry_MSG,
		NULL,//Status_MSG,
		//
		&CSAG::handleSignalMOSMSDataReq,//MOSmsDataReq_MSG,
		NULL,//MOSmsDataRsp_MSG,
		NULL,//MTSmsDataReq_MSG,
		&CSAG::handleSignalMTSMSDataRsp,//MTSmsDataRsp_MSG,
		&CSAG::handleSignalSMSMemAvailReq,//SMSMemAvailReq_MSG,
		NULL,		//SMSMemAvailRsp_MSG,
		//
		NULL,		//UTSAG_L3Addr_MSG,
		NULL,		//UTSAG_UID_MSG,
		NULL		//InvalidSignal_MSG,
};


void CSAG::ParseAndHandleSignal(CSAbisSignal& signal, int fdSocket)
{
	UINT8 signalType = signal.ParseMessageToSAG();
	if(UTSAG_L3Addr_MSG==signalType || UTSAG_UID_MSG==signalType)
	{
		signalType = signal.ParseUTSAGMsgToSAG();
	}

	if(signalType!=BeatHeart_MSG && signalType!=BeatHeartAck_MSG)
	{
		printf("\nReceived [%s] Signal from BTS", signalName[signalType]);
		ShowSignal(signal);
	}

	if(NULL!=FuncTable[signalType])
	{
		(this->*FuncTable[signalType])(signal, fdSocket);
	}
}

CBTS* CSAG::findBTSByPC(UINT16 pc)
{
	int i;
	for(i=0;i<M_MAX_BTS_NUM;i++)
	{
		if(btsTbl[i].getPC()==pc)
			return &btsTbl[i];
	}
	return NULL;
}
CBTS* CSAG::findBTSByBTSID(UINT16 btsid)
{
	int i;
	for(i=0;i<M_MAX_BTS_NUM;i++)
	{
		if(btsTbl[i].getBTSID()==btsid)
			return &btsTbl[i];
	}
	return NULL;
}

void CSAG::setAllHeadTailFields(CBTS* pBTS,
							  CSAG* pSAG,
							  UINT16 nPayloadLen,
							  SignalType sigType,
							  CSAbisSignal& signal)
{
	if(!pBTS || !pSAG)
	{
		printf("\nCannot find Serving BTS, error");
		return;
	}
		

	signal.SetBTSSAGID(htons(pBTS->getBTSID()), htonl(pSAG->getSAGID()));
	signal.SetSigIDS(sigType);
	signal.SetSigHeaderLengthField(nPayloadLen);
	signal.SetDataLength(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+nPayloadLen+sizeof(EndFlagT));
	signal.setSigTcpPKTHeaderTail(pSAG->getPC(), pBTS->getPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+nPayloadLen-sizeof(HeadFlagT));

	
}
//目前测试中一个bts只带一个cpe（或者只有一个BTS带两个CPE），所以从某个bts收到的包只需要变换L3地址后，发送到另一个bts即可
void CSAG::handleVoiceData(char* buf, int bufLen, int fdUdpSocket)
{

	int i;
	UINT32 srcL3Addr, dstL3Addr;
	CCCB *pCCB, *pRemoteCCB;
	CBTS* pDstBTS = NULL;
	DMUXHeadT *pDMUX_Head = (DMUXHeadT*)buf;
	DMUXVoiDataPkt729T *pDMUX_G729 = (DMUXVoiDataPkt729T*)(buf+sizeof(DMUXHeadT));

	//变换l3addr
	for(i=0;i<pDMUX_Head->nFrameNum;i++)
	{
		srcL3Addr = ntohl(pDMUX_G729->head.L3Addr);
		pCCB = FindCCBByL3Addr(srcL3Addr);
		if(pCCB!=NULL)
		{
			if((pRemoteCCB=pCCB->getRemoteCCB())!=NULL)
			{
				dstL3Addr = pRemoteCCB->getL3Addr();
				pDMUX_G729->head.L3Addr = htonl(dstL3Addr);
			}
			//find target bts
			if(pDstBTS==NULL)
			{
				if(pRemoteCCB!=NULL)
				{
					pDstBTS = pRemoteCCB->getSrvBTS();
				}
			}
		}
		else
		{
			pDMUX_G729->head.L3Addr = htonl(NO_L3ADDR);
		}

		pDMUX_G729++;
	}
	
	if(pDstBTS!=NULL)
	{
		//发送给另一个bts
		::sendto(fdUdpSocket, buf, bufLen, 0, (sockaddr*)&pDstBTS->m_destAddr, sizeof(pDstBTS->m_destAddr));
	}
}


void CSAG::sendDisconnect(CCCB* pCCB, int fdSocket, UINT8 RelCause)
{
	if(NULL==pCCB || -1==fdSocket)
	{
		return;
	}

	CSAbisSignal Disconnect;
	SAbisSignalT* pSigDisconnect = (SAbisSignalT*)Disconnect.GetDataPtr();

	setAllHeadTailFields(pCCB->getSrvBTS(),
		this,
		sizeof(DisconnectT),
		UTSAG_L3Addr_MSG,
		Disconnect);

	pSigDisconnect->sigPayload.Disconnect.containerHeader.uid_L3addr = htonl(pCCB->getL3Addr());
	pSigDisconnect->sigPayload.Disconnect.containerHeader.msgType = M_MSGTYPE_DISCONNECT;
	pSigDisconnect->sigPayload.Disconnect.RelCause = RelCause;
	
	sendSignalToBTS(Disconnect, fdSocket);	
}
void CSAG::sendRelease(CCCB* pCCB, int fdSocket, UINT8 RelCause)
{
	if(NULL==pCCB || -1==fdSocket)
	{
		return;
	}

	CSAbisSignal Release;
	SAbisSignalT* pSigRelease = (SAbisSignalT*)Release.GetDataPtr();

	setAllHeadTailFields(pCCB->getSrvBTS(),
		this,
		sizeof(ReleaseT),
		UTSAG_L3Addr_MSG,
		Release);
			
	pSigRelease->sigPayload.Release.containerHeader.uid_L3addr = htonl(pCCB->getL3Addr());
	pSigRelease->sigPayload.Release.containerHeader.msgType = M_MSGTYPE_RELEASE;
	pSigRelease->sigPayload.Release.RelCause = RelCause;

	sendSignalToBTS(Release, fdSocket);	

}
