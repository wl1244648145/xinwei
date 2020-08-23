/*******************************************************************************
* Copyright (c) 2010 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagGrpCCB.cpp
* Create Date    : 6-Jan-2010
* programmer     :fengbing
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "localSag.h"
#include "localSagStruct.h"
#include "voiceFsm.h"
#include "Cpe_signal_struct.h"
#include "tVoice.h"
#include "localSagCfg.h"
#include "localSagGrpCCB.h"
#include "voiceToolFunc.h"

void SxcGrpCCB::setEncryptKey(UINT8 * pKey)
{
	if(pKey)
	{
		memcpy(m_EncryptKey, pKey, M_ENCRYPT_KEY_LENGTH);
	}
}

void SxcGrpCCB::setEncryptCtrl(UINT8 EncryptCtrl)
{
	m_EncryptCtrl=EncryptCtrl;
#ifdef M__SUPPORT__ENC_RYP_TION
	if(EncryptCtrl)
	{
		//����ʽ��վ�͹�������ʱ����Ҫ�������ӵļ��ܷ�ʽ
		m_EncryptCtrl = ENCRYPT_CTRL_ENCRYPT_WITHOUT_KEY;
	}
#endif
}
SxcGrpCCB::SxcGrpCCB()
{
	setPagingUserFlag(false);
	setGID(M_INVALID_GID);
	setGrpL3Addr(M_INVALID_GRPL3ADDR);
	setTalkingUID(INVALID_UID);
	initGrpQue();
	m_pTmGrpPaging=NULL;
	m_pTmGrpAssignResReq=NULL;
	m_pTmGrpPressInfo=NULL;
	m_pTmMaxGrpAlive=NULL;
	m_pTmMaxGrpTalking=NULL;
	m_pTmMaxGrpIdle=NULL;
	m_grpVDataGuard.init(0, 5);
}

void SxcGrpCCB::initGrpQue()
{
	m_PttQueue.size = 0;
	m_PttQueue.QueHead.pNext = &m_PttQueue.QueHead;
	m_PttQueue.QueHead.pPrev= &m_PttQueue.QueHead;
}

void SxcGrpCCB::cleanGrpQue()
{
	GrpQueItemInfoT *pTmp;
	while(m_PttQueue.QueHead.pNext != &m_PttQueue.QueHead)
	{
		pTmp = m_PttQueue.QueHead.pNext->pNext;
		delete [] (UINT8*)m_PttQueue.QueHead.pNext;
		m_PttQueue.QueHead.pNext = pTmp;
	}
	initGrpQue();
}

void SxcGrpCCB::cleanGrpSrvInfo()
{
	setPagingUserFlag(false);
	setGID(M_INVALID_GID);
	setGrpL3Addr(M_INVALID_GRPL3ADDR);
	setTalkingUID(INVALID_UID);
	cleanGrpQue();
	deleteGrpTimer(TIMERID_SAG_COUNT, &m_pTmGrpPaging);
	deleteGrpTimer(TIMERID_SAG_COUNT, &m_pTmGrpAssignResReq);
	deleteGrpTimer(TIMERID_SAG_COUNT, &m_pTmGrpPressInfo);
	deleteGrpTimer(TIMERID_SAG_COUNT, &m_pTmMaxGrpAlive);
	deleteGrpTimer(TIMERID_SAG_COUNT, &m_pTmMaxGrpTalking);
	deleteGrpTimer(TIMERID_SAG_COUNT, &m_pTmMaxGrpIdle);
	m_grpVDataGuard.init();
}

bool SxcGrpCCB::startGrpTimer(UINT16 timerID, CTimer**ppTimer)
{
	return startSagTimer(timerID, ppTimer, getTalkingUID(), NO_L3ADDR, getGID(), getGrpL3Addr());
}
bool SxcGrpCCB::stopGrpTimer(UINT16 timerID, CTimer**ppTimer)
{
	if( stopSagTimer(ppTimer) )
	{
		if(timerID<TIMERID_SAG_COUNT)
		{
			if(sagTimerCfgTbl[timerID].blPeriodic)
			{
				LOG3(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTimer timer[%s] GID[0x%04X] GrpL3Addr[0x%08X]",
					(int)sagTimerName[timerID], getGID(), getGrpL3Addr());
			}
			else
			{
				LOG3(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTimer timer[%s] GID[0x%04X] GrpL3Addr[0x%08X]",
					(int)sagTimerName[timerID], getGID(), getGrpL3Addr());
			}
		}
		return true;
	}
	return false;
}
bool SxcGrpCCB::deleteGrpTimer(UINT16 timerID, CTimer**ppTimer)
{
	if( deleteSagTimer(ppTimer) )
	{
		if(timerID<TIMERID_SAG_COUNT)
		{
			if(sagTimerCfgTbl[timerID].blPeriodic)
			{
				LOG3(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTimer timer[%s] GID[0x%04X] GrpL3Addr[0x%08X]",
					(int)sagTimerName[timerID], getGID(), getGrpL3Addr());
			}
			else
			{
				LOG3(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTimer timer[%s] GID[0x%04X] GrpL3Addr[0x%08X]",
					(int)sagTimerName[timerID], getGID(), getGrpL3Addr());
			}
		}
		return true;
	}
	return false;
}
void SxcGrpCCB::grantPttToUser(UINT32 uid)
{
	deleteGrpTimer(TIMERID_SAG_GRP_MAX_IDLE_TIME, &m_pTmMaxGrpIdle);
	//���ڱ���ն�Ϊ������
	setTalkingUID(uid);
	//��������ŵ����͵�ǰ��������Ϣ
	sendDLSignalPttGranted(uid);
	//(MaybeFinishLater)������ʱ��TIMERID_SAG_GRP_MAX_TALKING_TIME��������ʱ���Ƿ�ʱ
}
void SxcGrpCCB::grantPttToUser(UINT32 uid, UINT16 sessionType, UINT8 prio, UINT8 encryptFlag)
{
	//find pCCB with UID
	CCCB *pCCB = CSAG::getSagInstance()->FindCCBByUID(uid);
	if(pCCB!=NULL)
	{
		pCCB->setGID(getGID());
	}
	deleteGrpTimer(TIMERID_SAG_GRP_MAX_IDLE_TIME, &m_pTmMaxGrpIdle);
	//���ڱ���ն�Ϊ������
	setTalkingUID(uid);
	//���ն˷������Ȩ������Ӧ��Ϣ���軰Ȩ
	sendDLSignalPttPressRsp(TRANSGRANT_PERMIT_TALK, uid, sessionType, prio, encryptFlag);
	//������ʱ��TIMERID_SAG_GRP_ASSIGNRESREQ�ȴ��û���������
	startGrpTimer(TIMERID_SAG_GRP_ASSIGNRESREQ, &m_pTmGrpAssignResReq);
	//��������ŵ����͵�ǰ��������Ϣ
	sendDLSignalPttGranted(uid);
	//(MaybeFinishLater)������ʱ��TIMERID_SAG_GRP_MAX_TALKING_TIME��������ʱ���Ƿ�ʱ
	
}
void SxcGrpCCB::putOneUserInQueue(UINT32 uid, UINT16 sessionType, UINT8 prio, UINT8 encryptFlag)
{
	UINT8 result;
	if(isPttQueueFull())
	{
		//����Ŷӽ��ʧ��
		result = TRANSGRANT_FORBID_TALK;
	}
	else
	{
		GrpQueItemInfoT *pNode = (GrpQueItemInfoT*) new UINT8[sizeof(GrpQueItemInfoT)];
		if(NULL==pNode)
		{
			result = TRANSGRANT_FORBID_TALK;
		}
		else
		{
			//����Ŷӽ���ɹ�
			result = TRANSGRANT_WAITING_TALK;
			pNode->uid = uid;
			pNode->sessionType = sessionType;
			pNode->prio = prio;
			pNode->encryptCtrl = encryptFlag;
			m_PttQueue.size++;
			//�����β
			GrpQueItemInfoT *pOldTail = m_PttQueue.QueHead.pPrev;
			pNode->pNext = pOldTail->pNext;
			pNode->pPrev = pOldTail;
			pOldTail->pNext = pNode;
			m_PttQueue.QueHead.pPrev = pNode;
			LOG2(SAG_LOG_VOICE_GRP_INFO2, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"Add user to PttPress Queue, GID[0x%04X] UID[0x%08X] ", 
				getGID(), uid);
			//֪ͨ�����������Ŷ�
			sendDLSignalPressInfo(uid);
		}
	}
	//���ն˻��Ŷ���Ӧ��Ϣ
	sendDLSignalPttPressRsp(result, uid, sessionType, prio, encryptFlag);
}
void SxcGrpCCB::delUserFromQueue(UINT32 uid)
{
	if(!isPttQueueEmpty())
	{
		GrpQueItemInfoT *pNode = NULL;
		GrpQueItemInfoT *pTmp = m_PttQueue.QueHead.pNext;
		while(pTmp != &m_PttQueue.QueHead)
		{
			if(pTmp->uid==uid)
			{
				pNode = pTmp;
				break;
			}
			else
			{
				pTmp = pTmp->pNext;
			}
		}		
		if(pNode!=NULL)
		{
			m_PttQueue.size--;
			//ɾ���ڵ�
			pNode->pPrev->pNext = pNode->pNext;
			pNode->pNext->pPrev = pNode->pPrev;
			LOG2(SAG_LOG_VOICE_GRP_INFO2, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"Del user from PttPress Queue, GID[0x%04X] UID[0x%08X] ", 
				getGID(), uid);
			delete [] (UINT8*)pNode;
		}
	}
}
GrpQueItemInfoT SxcGrpCCB::getOneUserOutofQueue()
{
	GrpQueItemInfoT pttUserInfo;
	pttUserInfo.uid = INVALID_UID;
	if(!isPttQueueEmpty())
	{
		GrpQueItemInfoT *pNode = m_PttQueue.QueHead.pNext;
		pttUserInfo = *(pNode);
		m_PttQueue.size--;
		//ɾ���ڵ�
		pNode->pPrev->pNext = pNode->pNext;
		pNode->pNext->pPrev = pNode->pPrev;
		delete [] (UINT8*)pNode;
	}
	return pttUserInfo;
}
void SxcGrpCCB::releaseGrpSrv(UINT8 RelCause)
{
	//����н�����
	if(isSomeoneTalking())
	{
		//�򽲻��������ڲ�����INSignal__GrpCallingRelease
		sendInnerSiganlGrpCallingRls(getTalkingUID(), RelCause);
	}
	//��bts����GrpRelease
	sendDLSignalGroupRelease(RelCause);
	//DeAllocGrpCCB
	CSAG::getSagInstance()->DeAllocGrpCCB(getTableIndex());
}
void SxcGrpCCB::releseGrpSrvByGrpMaker(UINT32 uid, UINT8 RelCause)
{
	if(isGrpFounder(uid))
	{
		//��bts����GrpRelease
		sendDLSignalGroupRelease(RelCause);
		//DeAllocGrpCCB
		CSAG::getSagInstance()->DeAllocGrpCCB(getTableIndex());
	}
}
void SxcGrpCCB::handlePttPressReq(CMessage& signal, UINT32 UID)
{
	if(!CSAG::getSagInstance()->isUserInGroup(UID, getGID()))
	{
		LOG2(SAG_LOG_VOICE_GRP_INFO3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"user not in group,  UID[0x%08X] GID[0x%04X]", UID, getGID());
		return;
	}
	//�����������ֻ�Ӧ��Ϣ������
	CSAbisSignal sabisSignal(signal);
	SAbisSignalT *pSignalData = (SAbisSignalT*)signal.GetDataPtr();
	UINT16 PttReqIfType;
	SignalType sigType;
	UINT8 prio, encryptFlag;
	if(sabisSignal.isUTSAGMsg())
	{
		sigType = sabisSignal.ParseUTSAGMsgToSAG();
	}
	else
	{
		sigType = sabisSignal.ParseMessageToSAG();
	}
	if(sigType==PTT_PressApplyReq_MSG)
	{
		PttReqIfType=VGetU16BitVal(pSignalData->sigPayload.PttPressApplyReq.sessionType);
		prio = pSignalData->sigPayload.PttPressApplyReq.prio;
		encryptFlag = pSignalData->sigPayload.PttPressApplyReq.EncryptCtrl;
	}
	else
	{
		if(sigType==PTT_PressReq_MSG)
		{
			PttReqIfType=M_USE_OLD_PTTREQ_IF;
			PTTPressReqT_CPE* pDataCpe = (PTTPressReqT_CPE*)(&pSignalData->sigPayload.UTSXC_Payload_GrpUid.msgType-1);
			prio = pDataCpe->CallPrioity;
			encryptFlag = pDataCpe->EncryptControl;
		}
		else
		{
			return;
		}
	}
	//������ڵ�ǰû�н�����
	if(!isSomeoneTalking())
	{
		grantPttToUser(UID, PttReqIfType, prio, encryptFlag);
	}
	//����������˽���
	else
	{
		//�ŶӴ���
		putOneUserInQueue(UID, PttReqIfType, prio, encryptFlag);
	}
}
void SxcGrpCCB::dispatchTalkersInGrpQueue()
{
	setTalkingUID(INVALID_UID);
	deleteGrpTimer(TIMERID_SAG_GRP_ASSIGNRESREQ, &m_pTmGrpAssignResReq);
	//�Ŷӵ���
	//����ŶӶ��зǿ�
	if(!isPttQueueEmpty())
	{
		deleteGrpTimer(TIMERID_SAG_GRP_MAX_IDLE_TIME, &m_pTmMaxGrpIdle);
		//��һ������Ϊ������
		GrpQueItemInfoT pttInfo = getOneUserOutofQueue();
		//���軰Ȩ
		grantPttToUser(pttInfo.uid, pttInfo.sessionType, pttInfo.prio, pttInfo.encryptCtrl);
	}
	//����ŶӶ���Ϊ��
	else
	{
		//��������ŵ��������������Ϣ
		sendDLSignalPttGranted(INVALID_UID);
		//������ʱ��TIMERID_SAG_GRP_MAX_IDLE_TIME
		deleteGrpTimer(TIMERID_SAG_GRP_MAX_IDLE_TIME, &m_pTmMaxGrpIdle);
		startGrpTimer(TIMERID_SAG_GRP_MAX_IDLE_TIME, &m_pTmMaxGrpIdle);
	}
}
void SxcGrpCCB::showInfo()
{
	VPRINT("\n====================================================");
	VPRINT("\nGID[0x%04X] GrpL3Addr[0x%08X] FounderUID[0x%08X] talkingUID[0x%08X]",
		getGID(), getGrpL3Addr(), getGrpSetupUID(), getTalkingUID());
	VPRINT("\nPttQue: ");
	GrpQueItemInfoT *pNode = m_PttQueue.QueHead.pNext;
	while(pNode!=&m_PttQueue.QueHead)
	{
		VPRINT(" UID[0x%08X]", pNode->uid);
		pNode = pNode->pNext;
	}
	VPRINT("\n====================================================");		
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool SxcGrpCCB::sendInnerSignalPttConnect(UINT32 uid)
{
	bool ret = false;
	InnerSignal_PttConnectT *pData;
	CSAbisSignal signal;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(InnerSignal_PttConnectT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		pData = (InnerSignal_PttConnectT*)signal.GetDataPtr();		
		CSAG::getSagInstance()->setInnerSignalHead(signal, PTT_Connect_MSG);
		VSetU32BitVal(pData->head.UID, uid);
		VSetU16BitVal(pData->head.GID, getGID());
		pData->grant = TRANSGRANT_PERMIT_TALK;
		pData->callOwnership = IS_CALL_OWNER;
		pData->callOwnership = getPrio();
		pData->EncryptFlag = getEncryptCtrl();

		ret = CSAG::getSagInstance()->sendInnerSignal(signal);
		if(ret)
		{
			LOG2(SAG_LOG_INNER_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"InnerSignal PttConnect  ----> UID[0x%08X] GID[0x%04X]", 
 				uid, getGID());
		}
	}
	return ret;	
}
bool SxcGrpCCB::sendInnerSiganlGrpCallingRls(UINT32 uid, UINT8 RelCause)
{
	bool ret = false;
	InnerSignal_GrpCallingRlsT *pData;
	CSAbisSignal signal;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(InnerSignal_GrpCallingRlsT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!!!!");
	}
	else
	{
		pData = (InnerSignal_GrpCallingRlsT*)signal.GetDataPtr();		
		CSAG::getSagInstance()->setInnerSignalHead(signal, Group_CallingRls_MSG);
		VSetU32BitVal(pData->head.UID, uid);
		VSetU16BitVal(pData->head.GID, getGID());
		pData->reason = RelCause;

		ret = CSAG::getSagInstance()->sendInnerSignal(signal);
		if(ret)
		{
			LOG3(SAG_LOG_INNER_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"InnerSignal GroupCallingRls  ----> UID[0x%08X] GID[0x%04X] reason[0x%02X]", 
 				uid, getGID(), RelCause);
		}
	}
	return ret;	
}
bool SxcGrpCCB::sendInnerSignalPttInterrupt(UINT32 uid, UINT8 reason)
{
	bool ret = false;
	InnerSignal_PttInterruptT *pData;
	CSAbisSignal signal;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(InnerSignal_PttInterruptT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		pData = (InnerSignal_PttInterruptT*)signal.GetDataPtr();		
		CSAG::getSagInstance()->setInnerSignalHead(signal, PTT_Interrupt_MSG);
		VSetU32BitVal(pData->head.UID, uid);
		VSetU16BitVal(pData->head.GID, getGID());
		pData->grant = TRANSGRANT_FORBID_TALK;
		pData->encryptFlag = getEncryptCtrl();
		pData->reason = reason;
		memset(pData->telno, 0, sizeof(pData->telno));

		ret = CSAG::getSagInstance()->sendInnerSignal(signal);
		if(ret)
		{
			LOG3(SAG_LOG_INNER_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"InnerSignal PttInterrupt  ----> UID[0x%08X] GID[0x%04X] reason[0x%02X]", 
 				uid, getGID(), reason);
		}
	}
	return ret;	
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool SxcGrpCCB::sendDLSignalLEPagingStart()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(LEGrpPagingT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(LEGrpPaging_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();

		VSetU16BitVal(pData->sigPayload.LEGrpPaging.GID, getGID());
		VSetU32BitVal(pData->sigPayload.LEGrpPaging.GrpL3Addr, getGrpL3Addr());
		VSetU16BitVal(pData->sigPayload.LEGrpPaging.GrpSize, getGrpSize());
		pData->sigPayload.LEGrpPaging.commType = getCommType();
		pData->sigPayload.LEGrpPaging.CallPrioty = getPrio();
		pData->sigPayload.LEGrpPaging.EncryptFlag = getEncryptCtrl();
		pData->sigPayload.LEGrpPaging.LEPeriod = g_GrpLePagingLoopTime;
		pData->sigPayload.LEGrpPaging.transID = getTransID();
#ifdef M__SUPPORT__ENC_RYP_TION
		memcpy(pData->sigPayload.LEGrpPaging.EncryptKey, getEncryptKey(), 
			sizeof(pData->sigPayload.LEGrpPaging.EncryptKey));
#endif
		signal.SetSigHeaderLengthField(sizeof(LEGrpPagingT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(LEGrpPagingT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG6(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"LEGrpPagingReq---->bts, GID[0x%04X] GrpL3Addr[0x%08X] TransID[0x%02X] CommType[0x%02X] CallPrioty[0x%02X] EncryptFlag[0x%02X]", 
				getGID(), getGrpL3Addr(), getTransID(), getCommType(), getPrio(), getEncryptCtrl());
		}
	}
	return ret;	
}
bool SxcGrpCCB::sendDLSignalGrpHandoverRsp(CSAbisSignal& ReqMsg, UINT8 result)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData, *pReqData;
	pReqData = (VoiceVCRCtrlMsgT*)ReqMsg.GetDataPtr();
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpHandoverRspT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(GrpHandoverRsp_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();

		VSetU16BitVal(pData->sigPayload.GrpHandoverRsp.GID, getGID());
		memcpy(pData->sigPayload.GrpHandoverRsp.UID, pReqData->sigPayload.GrpHandoverReq.UID, 4);
		pData->sigPayload.GrpHandoverRsp.Result = result;
		VSetU32BitVal(pData->sigPayload.GrpHandoverRsp.GrpL3Addr, getGrpL3Addr());
		memcpy(pData->sigPayload.GrpHandoverRsp.PID, pReqData->sigPayload.GrpHandoverReq.PID, 4);
		memcpy(pData->sigPayload.GrpHandoverRsp.curBTSID, pReqData->sigPayload.GrpHandoverReq.curBTSID, 4);
		pData->sigPayload.GrpHandoverRsp.transID = getTransID();

		signal.SetSigHeaderLengthField(sizeof(GrpHandoverRspT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpHandoverRspT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG6(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"GrpHandoverRsp---->bts, GID[0x%04X] UID[0x%08X] GrpL3Addr[0x%08X] PID[0x%08X] result[0x%02X] TransID[0x%02X]", 
				getGID(), VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.UID),
				VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.GrpL3Addr),
				VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.PID),
				result, getTransID());
		}
	}
	return ret;	
}
bool SxcGrpCCB::sendDLSignalGrpResRsp(CSAbisSignal& ReqMsg, UINT8 result)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData, *pReqData;
	pReqData = (VoiceVCRCtrlMsgT*)ReqMsg.GetDataPtr();
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpResRspT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(GrpResRsp_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();

		VSetU16BitVal(pData->sigPayload.GrpResRsp.GID, getGID());
		memcpy(pData->sigPayload.GrpResRsp.UID, pReqData->sigPayload.GrpResRsp.UID, 4);
		VSetU32BitVal(pData->sigPayload.GrpResRsp.GrpL3Addr, getGrpL3Addr());
		pData->sigPayload.GrpResRsp.Cause=result;
		memcpy(pData->sigPayload.GrpResRsp.PID, pReqData->sigPayload.GrpResRsp.PID, 4);
		pData->sigPayload.GrpResRsp.transID = getTransID();

		signal.SetSigHeaderLengthField(sizeof(GrpResRspT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpResRspT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG6(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"GrpResRsp---->bts, GID[0x%04X] UID[0x%08X] GrpL3Addr[0x%08X] PID[0x%08X] Cause[0x%02X] TransID[0x%02X]", 
				getGID(), VGetU32BitVal(pData->sigPayload.GrpResRsp.UID),
				VGetU32BitVal(pData->sigPayload.GrpResRsp.GrpL3Addr),
				VGetU32BitVal(pData->sigPayload.GrpResRsp.PID),
				pData->sigPayload.GrpResRsp.Cause,
				getTransID());
		}
	}
	return ret;	
}
bool SxcGrpCCB::sendDLSignalPttGranted(UINT32 uid)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	typedef struct
	{
		UINT8 msgType;//Message type	1
		UINT8 grant;//Call grant	1	M		0 ��Ȩ����1 ��Ȩռ�� 
		UINT8 encryptFlag;//Encryption Flag	1	M		�˵��˼��ܱ�־λ
		char telno[100];//������������ʾ	N	C		TLV��ʽ
	}tmp_PttGrantedT_payload;
	UINT16 sigHeadLenFieldVal = sizeof(tmp_PttGrantedT_payload) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CSAG::getSagInstance()->setInnerSignalHead(signal, PTT_Granted_MSG);
		signal.SetSigIDS(PTT_Granted_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSXC_Payload_GrpL3Addr.GrpL3Addr, getGrpL3Addr());
		tmp_PttGrantedT_payload* pDataPayload = (tmp_PttGrantedT_payload*)(&pData->sigPayload.UTSXC_Payload_GrpL3Addr.msgType);
		pDataPayload->msgType = M_MSGTYPE_PTT_GRANTED;
		pDataPayload->encryptFlag = getEncryptCtrl();
		sigHeadLenFieldVal = sizeof(UINT32) + ((UINT8*)&pDataPayload->telno[0] - &pDataPayload->msgType);
		if(INVALID_UID==uid)
		{
			pDataPayload->grant = CALLGRANT_FREE;
			pDataPayload->telno[0]=0;//������
			pDataPayload->telno[1]=0;//����0
			pDataPayload->telno[2]=0;//����0
			sigHeadLenFieldVal += 3;
		}
		else
		{
			CCCB* pCCB = CSAG::getSagInstance()->FindCCBByUID(uid);
			if(NULL!=pCCB)
			{
				UINT8 nTelNOLen = strlen(pCCB->getOwnNumber());
				pDataPayload->grant = CALLGRANT_INUSE;
				pDataPayload->telno[0]=1;//�绰����
				pDataPayload->telno[1]=nTelNOLen;//����
				strcpy(&pDataPayload->telno[2], pCCB->getOwnNumber());//����
				sigHeadLenFieldVal += (3+nTelNOLen);
			}
			else
			{
				pDataPayload->grant = CALLGRANT_INUSE;
				pDataPayload->telno[0]=1;//�绰����
				pDataPayload->telno[1]=0;//����0
				pDataPayload->telno[2]=0;//����0
				sigHeadLenFieldVal += 3;
			}
		}

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG5(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"PttGranted---->bts, GID[0x%04X] GrpL3Addr[0x%08X] Grant[0x%02X] EncryptFlag[0x%02X] UID[[0x%08X]]", 
				getGID(), getGrpL3Addr(), pDataPayload->grant, pDataPayload->encryptFlag, uid);
		}
	}
	return ret;	
}
bool SxcGrpCCB::sendDLSignalLAGrpPaging()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpLAPagingT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(LAGrpPagingReq_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();

		VSetU16BitVal(pData->sigPayload.GrpLAPaging.GID, getGID());
		VSetU16BitVal(pData->sigPayload.GrpLAPaging.GrpSize, getGrpSize());
		VSetU32BitVal(pData->sigPayload.GrpLAPaging.GrpL3Addr, getGrpL3Addr());
		pData->sigPayload.GrpLAPaging.CommType = getCommType();
		pData->sigPayload.GrpLAPaging.CallPrioty = getPrio();
		pData->sigPayload.GrpLAPaging.EncryptFlag = getEncryptCtrl();
		pData->sigPayload.GrpLAPaging.transID = getTransID();
#ifdef M__SUPPORT__ENC_RYP_TION
		memcpy(pData->sigPayload.GrpLAPaging.EncryptKey, getEncryptKey(), 
			sizeof(pData->sigPayload.GrpLAPaging.EncryptKey));
#endif
		signal.SetSigHeaderLengthField(sizeof(GrpLAPagingT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpLAPagingT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG6(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"LAGrpPagingReq---->bts, GID[0x%04X] GrpL3Addr[0x%08X] TransID[0x%02X] CommType[0x%02X] CallPrioty[0x%02X] EncryptFlag[0x%02X]", 
				getGID(), getGrpL3Addr(), getTransID(), getCommType(), getPrio(), getEncryptCtrl());
		}
	}
	return ret;	
}
bool SxcGrpCCB::sendDLSignalPressInfo(UINT32 UidInQue)
{
	bool ret = false;
	CCCB *pCCBTalking=NULL; 
	if(isSomeoneTalking())
	{
		pCCBTalking= CSAG::getSagInstance()->FindCCBByUID(getTalkingUID());
	}
	if(NULL==pCCBTalking)
	{
		return ret;
	}
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	typedef struct
	{
		UINT8 msgType;//Message type	1
		char telno[100];//������ʾ	N	C		TLV��ʽ
	}tmp_PttPressInfoT_payload;	
	UINT16 sigHeadLenFieldVal = sizeof(tmp_PttPressInfoT_payload) + sizeof(UINT32);
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(PTT_PressInfo_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, pCCBTalking->getL3Addr());
		tmp_PttPressInfoT_payload* pDataPayload = (tmp_PttPressInfoT_payload*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType);
		pDataPayload->msgType = M_MSGTYPE_PTT_PRESS_INFO;

		sigHeadLenFieldVal = sizeof(UINT32) + ((UINT8*)&pDataPayload->telno[0] - &pDataPayload->msgType);
		if(INVALID_UID==UidInQue)
		{
			pDataPayload->telno[0]=0;//������
			pDataPayload->telno[1]=0;//����0
			pDataPayload->telno[2]=0;//����0
			sigHeadLenFieldVal += 3;
		}
		else
		{
			CCCB* pCCBInQue = CSAG::getSagInstance()->FindCCBByUID(UidInQue);
			if(NULL!=pCCBInQue)
			{
				UINT8 nTelNOLen = strlen(pCCBInQue->getOwnNumber());
				pDataPayload->telno[0]=1;//�绰����
				pDataPayload->telno[1]=nTelNOLen;//����
				strcpy(&pDataPayload->telno[2], pCCBInQue->getOwnNumber());//����
				sigHeadLenFieldVal += (3+nTelNOLen);
			}
			else
			{
				pDataPayload->telno[0]=1;//�绰����
				pDataPayload->telno[1]=0;//����0
				pDataPayload->telno[2]=0;//����0
				sigHeadLenFieldVal += 3;
			}
		}
		
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"PttPressInfo---->bts, GID[0x%04X] L3Addr[0x%08X] UIDinQue[0x%08X]", 
				getGID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr), UidInQue);
		}
	}
	return ret;
}
bool SxcGrpCCB::sendDLSignalGroupRelease(UINT8 RelCause)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	typedef struct
	{
		UINT8 msgType;//Message type	1
		UINT8 relCause;//Release cause 	1
	}tmp_GrpReleaseT_payload;
	UINT16 sigHeadLenFieldVal = sizeof(tmp_GrpReleaseT_payload) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CSAG::getSagInstance()->setInnerSignalHead(signal, Grp_Release_MSG);
		signal.SetSigIDS(Grp_Release_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSXC_Payload_GrpL3Addr.GrpL3Addr, getGrpL3Addr());
		tmp_GrpReleaseT_payload* pDataPayload = (tmp_GrpReleaseT_payload*)(&pData->sigPayload.UTSXC_Payload_GrpL3Addr.msgType);
		pDataPayload->msgType = M_MSGTYPE_GRP_RLS;
		pDataPayload->relCause = RelCause;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"GroupRelease---->bts, GID[0x%04X] GrpL3Addr[0x%08X] RelCause[0x%02X]", 
				getGID(), getGrpL3Addr(), RelCause);
		}
	}
	return ret;	
}
bool SxcGrpCCB::sendDLSignalPttPressRsp(UINT8 result, UINT32 uid, UINT16 sessionType, UINT8 prio, UINT8 encryptFlag)
{
	return CSAG::getSagInstance()->sendPttPressRsp(getGID(), result, uid, sessionType, prio, encryptFlag);
}

