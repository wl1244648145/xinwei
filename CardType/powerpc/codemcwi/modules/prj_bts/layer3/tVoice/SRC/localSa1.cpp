/*******************************************************************************
* Copyright (c) 2009 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagCCB.cpp
* Create Date    : 13-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "localSagCCB.h"
#include "localSagTimer.h"
#include "localSagStruct.h"
#include "localSag.h"
#include "localSagFsmCfg.h"
#include "localSagMsgID.h"
#include "voiceToolFunc.h"
#include "tVoice.h"
#include "voiceTone.h"
#include "cpe_signal_struct.h"
#include "time.h"
////////////////////////////////////////////////////////////////////////////////
void LocalSagCdr::setCalling(char* calling)
{
	UINT16 n;
	UINT16 sz = sizeof(m_CdrRecord.CALLING);
	for(n=0;n<sz;n++)
	{
		m_CdrRecord.CALLING[n] = calling[n];
		if('\0'==calling[n])
			break;
	}
	m_CdrRecord.CALLING[sz-1] = '\0';
}
void LocalSagCdr::setCalled(char* called)
{
	UINT16 n;
	UINT16 sz = sizeof(m_CdrRecord.CALLED);
	for(n=0;n<sz;n++)
	{
		m_CdrRecord.CALLED[n] = called[n];
		if('\0'==called[n])
			break;
	}
	m_CdrRecord.CALLED[sz-1] = '\0';
}

time_t LocalSagCdr::getTimeFromDateTime(LocalSagDateTimeT *pDT)
{
	time_t ret = 0;
	if(pDT!=NULL)
	{
		struct tm time_s;
		time_s.tm_sec = pDT->second;
		time_s.tm_min = pDT->minute;
		time_s.tm_hour = pDT->hour;

		time_s.tm_mday = pDT->day;
		time_s.tm_mon  = pDT->month - 1;
		time_s.tm_year = pDT->year - 1900;
		time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */
		ret =  mktime(&time_s);
	}
	return ret;
}

void LocalSagCdr::setDateTime(LocalSagDateTimeT *pDT, UINT16 year, UINT8 month, UINT8 day, UINT8 hour, UINT8 minute, UINT8 second)
{
	if(pDT!=NULL)
	{
		pDT->year = year;
		pDT->month = month;
		pDT->day = day;
		pDT->hour = hour;
		pDT->minute = minute;
		pDT->second = second;
	}
}

void LocalSagCdr::setStartDateTime(UINT16 year, UINT8 month, UINT8 day, UINT8 hour, UINT8 minute, UINT8 second)
{
	setDateTime(&m_dtStart, year, month, day, hour, minute, second);
}
void LocalSagCdr::setConnectDateTime(UINT16 year, UINT8 month, UINT8 day, UINT8 hour, UINT8 minute, UINT8 second)
{
	setDateTime(&m_dtConnect, year, month, day, hour, minute, second);
}
void LocalSagCdr::setEndDateTime(UINT16 year, UINT8 month, UINT8 day, UINT8 hour, UINT8 minute, UINT8 second)
{
	setDateTime(&m_dtEnd, year, month, day, hour, minute, second);
}
UINT32 LocalSagCdr::computeDuration()
{
	time_t tmConnect = getTimeFromDateTime(&m_dtConnect);
	time_t tmEnd = getTimeFromDateTime(&m_dtEnd);
	if(tmEnd>=tmConnect)
	{
		return(tmEnd-tmConnect);
	}
	else
	{
		LOG2(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"local sag cdr error!tmEnd[%d] < tmConnect[%d]",
			tmEnd, tmConnect);
		return 0;
	}
}
void LocalSagCdr::finishCdrRecord(char *pCalling, char *pCalled)
{
	memset(&m_CdrRecord, 0, sizeof(LocalSagCdrStructT));
	//calling
	if(pCalling!=NULL)
	{
		setCalling(pCalling);
	}
	//called
	if(pCalled!=NULL)
	{
		setCalled(pCalled);
	}
	char *pDateTimeStr;
	LocalSagDateTimeT *pDT;
	//start datetime
	pDateTimeStr = (char*)&m_CdrRecord.START_DATE;
	pDT = &m_dtStart;
	sprintf(pDateTimeStr, "%04d%02d%02d%02d%02d%02d", 
		pDT->year, pDT->month, pDT->day, 
		pDT->hour, pDT->minute, pDT->second);
	//answer datetime
	pDateTimeStr = (char*)&m_CdrRecord.ANSWER_DA;
	pDT = &m_dtConnect;
	sprintf(pDateTimeStr, "%04d%02d%02d%02d%02d%02d", 
		pDT->year, pDT->month, pDT->day, 
		pDT->hour, pDT->minute, pDT->second);
	//end datetime
	pDateTimeStr = (char*)&m_CdrRecord.END_DATE;
	pDT = &m_dtEnd;
	sprintf(pDateTimeStr, "%04d%02d%02d%02d%02d%02d", 
		pDT->year, pDT->month, pDT->day, 
		pDT->hour, pDT->minute, pDT->second);
	//duration
	UINT32 nDuration = computeDuration();
	sprintf((char*)&m_CdrRecord.DURATION_T, "%d", nDuration);
	//connected flag
	strcpy((char*)m_CdrRecord.CALL_STATU, "connected");
}
#include "l3OAMMessageID.h"
bool LocalSagCdr::reportCdr()
{
	bool ret = false;
	CComMessage *pMsg = new (CTVoice::GetInstance(), sizeof(LocalSagCdrStructT)) CComMessage;
	if(pMsg!=NULL)
	{
		pMsg->SetMessageId(M_VOICE_EB_BILLING_NOTIFY);
		pMsg->SetDstTid(M_TID_EB);
		pMsg->SetSrcTid(M_TID_SAG);
		pMsg->SetDataLength(sizeof(LocalSagCdrStructT));
		memcpy(pMsg->GetDataPtr(), (UINT8*)&m_CdrRecord, sizeof(LocalSagCdrStructT));
		ret = postComMsg(pMsg);
	}
	else
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), 
			"new ComMessage failed!!!");
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////

bool CCCB::startTimer(UINT16 timerID)
{
	if(m_timerID!=TIMERID_SAG_COUNT)
	{
		LOG2(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"when startTimer, find old timer[ timerName(%s) pTimer(0x%08X)] not stop !!!",
			(int)sagTimerName[m_timerID], (int)m_pTimer);
	}
	m_timerID=timerID;
	return startSagTimer(timerID, &m_pTimer, getUID(), getL3Addr());
}
bool CCCB::stopTimer(UINT16 timerID)
{
	m_timerID = TIMERID_SAG_COUNT;
	if( stopSagTimer(&m_pTimer) )
	{
		if(timerID<TIMERID_SAG_COUNT)
		{
			if(sagTimerCfgTbl[timerID].blPeriodic)
			{
				LOG3(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTimer timer[%s] UID[0x%08X] L3Addr[0x%08X]",
					(int)sagTimerName[timerID], getUID(), getL3Addr());
			}
			else
			{
				LOG3(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTimer timer[%s] UID[0x%08X] L3Addr[0x%08X]",
					(int)sagTimerName[timerID], getUID(), getL3Addr());
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}
bool CCCB::deleteTimer(UINT16 timerID)
{
	m_timerID = TIMERID_SAG_COUNT;
	if( deleteSagTimer(&m_pTimer) )
	{
		if(timerID<TIMERID_SAG_COUNT)
		{
			if(sagTimerCfgTbl[timerID].blPeriodic)
			{
				LOG3(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTimer timer[%s] UID[0x%08X] L3Addr[0x%08X]",
					(int)sagTimerName[timerID], getUID(), getL3Addr());
			}
			else
			{
				LOG3(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTimer timer[%s] UID[0x%08X] L3Addr[0x%08X]",
					(int)sagTimerName[timerID], getUID(), getL3Addr());
			}			
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool CCCB::playTone(UINT16 idTone)
{
	bool ret = false;
	//向tVoice任务发送消息通知向终端UID开始放音
	CComMessage *pMsg = new (CTVoice::GetInstance(), sizeof(PlayToneInfoT)) CComMessage;
	if(pMsg!=NULL)
	{
		pMsg->SetMessageId(MSGID_SAG_BTS_PLAYTONE);
		pMsg->SetDstTid(M_TID_VOICE);
		pMsg->SetSrcTid(M_TID_SAG);
		pMsg->SetDataLength(sizeof(PlayToneInfoT));
		PlayToneInfoT *pData = (PlayToneInfoT*)pMsg->GetDataPtr();
		VSetU16BitVal(pData->toneID , idTone);
		VSetU32BitVal(pData->UID , getUID());
		pData->CID = getCID();
		pMsg->SetEID(getEID());
		ret = postComMsg(pMsg);
	}
	if(ret)
	{
		LOG3(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"playTone tone[%s] UID[0x%08X] L3Addr[0x%08X]",
					(int)g_ToneTbl[idTone].toneName, getUID(), getL3Addr());	
	}
	else
	{
		LOG3(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"playTone tone[%s] UID[0x%08X] L3Addr[0x%08X] error!!!",
					(int)g_ToneTbl[idTone].toneName, getUID(), getL3Addr());	
	}
	return ret;
}
bool CCCB::stopTone(UINT16 idTone)
{
	bool ret = false;
	//向tVoice任务发送消息通知向终端UID停止放音
	CComMessage *pMsg = new (CTVoice::GetInstance(), sizeof(StopToneInfoT)) CComMessage;
	if(pMsg!=NULL)
	{
		pMsg->SetMessageId(MSGID_SAG_BTS_STOPTONE);
		pMsg->SetDstTid(M_TID_VOICE);
		pMsg->SetSrcTid(M_TID_SAG);
		pMsg->SetDataLength(sizeof(StopToneInfoT));
		StopToneInfoT *pData = (StopToneInfoT*)pMsg->GetDataPtr();
		VSetU16BitVal(pData->toneID , idTone);
		VSetU32BitVal(pData->UID , getUID());
		pData->CID = getCID();
		pMsg->SetEID(getEID());
		ret = postComMsg(pMsg);
	}
	if(ret)
	{
		LOG3(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTone tone[%s] UID[0x%08X] L3Addr[0x%08X]",
					(int)g_ToneTbl[idTone].toneName, getUID(), getL3Addr());	
	}
	else
	{
		LOG3(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"stopTone tone[%s] UID[0x%08X] L3Addr[0x%08X] error!!!",
					(int)g_ToneTbl[idTone].toneName, getUID(), getL3Addr());	
	}
	return ret;
}
void CCCB::enterState()
{
	UINT8 curState = getState();
	if(SAG_IDLE_STATE==curState)
	{
		//清除呼叫相关信息,l3地址作废
		CSAG::getSagInstance()->DelL3AddrIndexTable(getL3Addr());
		clearCCBCallInfo();
		clearCCBGrpVoiceCallInfo();
	}
	if(SAG_WAIT_ONHOOK_STATE==curState)
	{
		//放忙音
		playTone(BUSY_TONE);
	}
}
void CCCB::exitState()
{
	UINT8 curState = getState();
	if(SAG_WAIT_ONHOOK_STATE==curState)
	{
		//停忙音
		stopTone(BUSY_TONE);
	}
	if(SAG_RINGBACK_STATE==curState)
	{
		//停回铃音
		stopTone(RINGBACK_TONE);
	}
	if(SAG_DIAL_STATE==curState)
	{
		//停拨号音
		if(!isDialToneStopped())
		{
			stopTone(DIAL_TONE);
		}
	}
	if(SAG_CONNECT_STATE==curState)
	{
		if(isOrigCall())
		{
			//计费相关
			T_TimeDate dt = bspGetDateTime();
			m_cdr.setEndDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);	
			//完成话单
			m_cdr.finishCdrRecord(getOwnNumber(), getDialedNumber());
			//发送话单消息给话单任务
			m_cdr.reportCdr();	
		}
	}
}


CCCB::CCCB()
{
	m_pTimer = NULL;
	ClearCCBInfo();
	clearCCBGrpVoiceCallInfo();
}

void CCCB::clearCCBGrpVoiceCallInfo()
{
	setGID(M_INVALID_GID);
}

void CCCB::clearCCBCallInfo()
{
	if(SAG_IDLE_STATE!=getState())
	{
		exitState();
		LOG2(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			" CCB[UID=0x%08X] exit state[%s]", getUID(), (int)callFsmStateName[getState()]);
		setState(SAG_IDLE_STATE);
		enterState();
		LOG2(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			" CCB[UID=0x%08X] enter state[%s]", getUID(), (int)callFsmStateName[getState()]);
	}	
	setCodec(CODEC_G729A);
	setL3Addr(NO_L3ADDR);
	setPeerCCB(NULL);
	setDialedNumber("");
	this->deleteTimer();
}

void CCCB::ClearCCBInfo()
{
	setState(SAG_IDLE_STATE);
	setCodec(CODEC_G729A);
	setUID(INVALID_UID);
	setEID(0xffffffff);
	setCID(0xff);
	setL3Addr(NO_L3ADDR);
	setPeerCCB(NULL);
	setOwnNumber("");
	setDialedNumber("");
	this->deleteTimer();
	m_timerID = TIMERID_SAG_COUNT;
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

 CCCB* CCCB::getPeerCCB()
{
	return m_pPeerCCB;
}
 void CCCB::setPeerCCB(CCCB* pCCB)
{
	m_pPeerCCB = pCCB;
}
 char* CCCB::getOwnNumber()
{
	return m_OwnNumber;
}
void CCCB::setOwnNumber(char* number)
{
	if(strlen(number)<sizeof(m_OwnNumber))
		strcpy(m_OwnNumber, number);
	else
	{
		LOG(LOG_DEBUG3, 0, "Local Number too long!!!");
		strncpy(m_OwnNumber, number, sizeof(m_OwnNumber)-1);
		m_OwnNumber[sizeof(m_OwnNumber)-1] = 0;
	}
}
void CCCB::SaveDialedNumber(char Digit)
{
	char dictionary[]="D1234567890*#ABC";
	if(Digit<=0x0F)//dtmf
	{
		m_DialedNumber[m_DialedNumberLen++] = dictionary[(UINT8)Digit];
	}
	else	//asc code
	{
		m_DialedNumber[m_DialedNumberLen++] = Digit;
	}
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
		m_DialedNumber[sizeof(m_DialedNumber)-1] = 0;
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
#if 0
 UINT8 CCCB::getAuthReason()
{
	return m_AuthReason;
}
 void CCCB::setAuthReason(UINT8 reason)
{
	m_AuthReason = reason;
}
#endif

UINT8 CCCB::getDisconnectReason(CMessage& msg)
{
	SAbisSignalT* pData = (SAbisSignalT*)msg.GetDataPtr();
	DisconnectT_CPE* pDataCPE = (DisconnectT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
	return pDataCPE->RelCause;
}

//bool sendDLSignalErrNotiRsp(CMessage& errNotiReq);收到后自动回应
bool CCCB::sendDLSignalLAPaging(UINT16 appType)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(LAPagingT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(LAPaging_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.LAPaging.UID, getUID());
		VSetU32BitVal(pData->sigPayload.LAPaging.L3addr, getL3Addr());
		VSetU16BitVal(pData->sigPayload.LAPaging.App_Type, appType);
		pData->sigPayload.LAPaging.VersionInfo[0]=0;
		pData->sigPayload.LAPaging.VersionInfo[1]=1;

		signal.SetSigHeaderLengthField(sizeof(LAPagingT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(LAPagingT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG4(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"LAPagingReq---->bts, UID[0x%08X]  L3Addr[0x%08X] AppType[0x%04X] version[0x%04X]", 
				VGetU32BitVal(pData->sigPayload.LAPaging.UID),
				VGetU32BitVal(pData->sigPayload.LAPaging.L3addr),
				VGetU16BitVal(pData->sigPayload.LAPaging.App_Type),
				VGetU16BitVal(pData->sigPayload.LAPaging.VersionInfo));
		}
	}
	return ret;
}
bool CCCB::sendDLSignalDeLaPaging()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(DELAPagingReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(DELAPagingReq_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.DELAPagingReq.UID, getUID());

		signal.SetSigHeaderLengthField(sizeof(DELAPagingReqT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(DELAPagingReqT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG1(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"DELAPagingReq---->bts, UID[0x%08X]  ", 
				VGetU32BitVal(pData->sigPayload.DELAPagingReq.UID));
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalSetup()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(SetupSAGT_CPE)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(Setup_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();

		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		SetupSAGT_CPE* pDataCPE = (SetupSAGT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_SETUP;
#if 0		
		if(ENCRYPT_VOICE==getEncryptCtrl())
		{
			pDataCPE->SetupCause = SETUPCAUSE_ENCRYPT_VOICE;
		}
		else
		{
			pDataCPE->SetupCause = SETUPCAUSE_VOICE;
		}
#else
		pDataCPE->SetupCause = getPeerCCB()->getSetupCause();
#endif
		pDataCPE->codecList[0] = 1;
		pDataCPE->codecList[1] = CODEC_G729A;
		UINT8 TlvNOLen = convertStr2DigitNO((void*)&pDataCPE->codecList[2], getPeerCCB()->getOwnNumber());
		UINT16 sigHeadLenFieldVal = pData->sigPayload.UTSAG_Payload_L3Addr.UTSAGPayload - 
			pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr +
			sizeof(pDataCPE->SetupCause) + 2 + TlvNOLen ;
		
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"Setup---->bts, UID[0x%08X] L3Addr[0x%08X] ", 
				getUID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr));
		}
	}
	return ret;
}
bool CCCB::sendDLSignalSetupAck()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(CallProcT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(CallProc_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		CallProcT_CPE* pDataCPE = (CallProcT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_CALLPROC;
		pDataCPE->prio = M_DEFAULT_VOICE_PRIORITY;
		pDataCPE->selcodeinfo.type=0x02;
		pDataCPE->selcodeinfo.lenth=1;
		pDataCPE->selcodeinfo.value=CODEC_G729A;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"SetupAck---->bts, UID[0x%08X] L3Addr[0x%08X] ", 
				getUID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr));
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalAlerting()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(AlertingT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(Alerting_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		AlertingT_CPE* pDataCPE = (AlertingT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_ALERTING;
		pDataCPE->selcodeinfo.type=0x02;
		pDataCPE->selcodeinfo.lenth=1;
		pDataCPE->selcodeinfo.value=CODEC_G729A;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"Alerting---->bts, UID[0x%08X] L3Addr[0x%08X] ", 
 				getUID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr));
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalConnect()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(ConnectSAGT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(Connect_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		ConnectSAGT_CPE* pDataCPE = (ConnectSAGT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_CONNECT;
		pDataCPE->selcodeinfo.type=0x02;
		pDataCPE->selcodeinfo.lenth=1;
		pDataCPE->selcodeinfo.value=CODEC_G729A;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"Connect---->bts, UID[0x%08X] L3Addr[0x%08X] ", 
				getUID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr));
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalConnectAck()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(ConnectAckT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(ConnectAck_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		ConnectAckT_CPE* pDataCPE = (ConnectAckT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_CONNECTACK;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"ConnectAck---->bts, UID[0x%08X] L3Addr[0x%08X] ", 
				getUID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr));
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalDisconnect(UINT8 reason)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(DisconnectT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(Disconnect_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		DisconnectT_CPE* pDataCPE = (DisconnectT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_DISCONNECT;
		pDataCPE->RelCause = reason;
		
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"Disconnect---->bts, UID[0x%08X] L3Addr[0x%08X] RelCause[0x%02X]", 
				getUID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr), reason);
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalRelease(UINT8 reason)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(ReleaseT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(Release_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		ReleaseT_CPE* pDataCPE = (ReleaseT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_RELEASE;
		pDataCPE->RelCause = reason;
		
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"Release---->bts, UID[0x%08X] L3Addr[0x%08X] RelCause[0x%02X]", 
				getUID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr), reason);
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalAssignResRsp(CMessage& assignResReq)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(AssignResRspT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(AssignResRsp_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.AssignResRsp.UID, getUID());
		VSetU32BitVal(pData->sigPayload.AssignResRsp.L3addr, getL3Addr());
		pData->sigPayload.AssignResRsp.AssignResult = M_SABIS_SUCCESS;

		signal.SetSigHeaderLengthField(sizeof(AssignResRspT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(AssignResRspT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"AssignResRsp---->bts, UID[0x%08X] L3Addr[0x%08X] Result[0x%02X] ", 
				VGetU32BitVal(pData->sigPayload.AssignResRsp.UID), 
				VGetU32BitVal(pData->sigPayload.AssignResRsp.L3addr),
				pData->sigPayload.AssignResRsp.AssignResult);
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalRlsTransResReq(UINT8 reason)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(RlsResReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(RlsResReq_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.RlsResReq.L3addr, getL3Addr());
		pData->sigPayload.RlsResReq.RlsCause = reason;

		signal.SetSigHeaderLengthField(sizeof(RlsResReqT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(RlsResReqT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"RlsResReq---->bts, UID[0x%08X] L3Addr[0x%08X] RelCause[0x%02X] ", 
				getUID(), VGetU32BitVal(pData->sigPayload.RlsResReq.L3addr),
				pData->sigPayload.RlsResReq.RlsCause);
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalErrNotiReq()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(ErrNotifyReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(ErrNotifyReq_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.ErrNotifyReq.Uid, getUID());
		VSetU32BitVal(pData->sigPayload.ErrNotifyReq.L3Addr, getL3Addr());
		pData->sigPayload.ErrNotifyReq.ErrCause = M_ERRNOTIFY_ERR_CAUSE_SAG_FOUND_ERROR;

		signal.SetSigHeaderLengthField(sizeof(ErrNotifyReqT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(ErrNotifyReqT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"ErrorNotifyReq---->bts, UID[0x%08X] L3Addr[0x%08X] ErrCause[0x%02X] ", 
				VGetU32BitVal(pData->sigPayload.ErrNotifyReq.Uid), 
				VGetU32BitVal(pData->sigPayload.ErrNotifyReq.L3Addr),
				pData->sigPayload.ErrNotifyReq.ErrCause);
		}
	}
	return ret;
}
bool CCCB::sendDLSignalDVoiceReq(UINT8 DVoice)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(DVoiceCfgReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(DVoiceConfigReg_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.DVoiceCfgReq.L3Addr1, getL3Addr());
		VSetU32BitVal(pData->sigPayload.DVoiceCfgReq.L3Addr2, getPeerCCB()->getL3Addr());
		pData->sigPayload.DVoiceCfgReq.DVoice = DVoice;

		signal.SetSigHeaderLengthField(sizeof(DVoiceCfgReqT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(DVoiceCfgReqT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG5(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"DvoiceCfgReq---->bts, UID1[0x%08X] UID2[0x%08X] L3Addr1[0x%08X] L3Addr2[0x%08X] SelfSwitch[0x%02X] ", 
				getUID(), getPeerCCB()->getUID(),
				VGetU32BitVal(pData->sigPayload.DVoiceCfgReq.L3Addr1), 
				VGetU32BitVal(pData->sigPayload.DVoiceCfgReq.L3Addr2),
				pData->sigPayload.DVoiceCfgReq.DVoice);
		}
	}
	return ret;
}

bool CCCB::sendDLSignalPttSetupAck(UINT8 EncryptCtrl, UINT8 prio)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(PTTSetupAckT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(PTT_SetupAck_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		PTTSetupAckT_CPE* pDataCPE = (PTTSetupAckT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_PTT_SETUP_ACK;
		pDataCPE->CallPriority = EncryptCtrl;
		pDataCPE->EncryptFlag = prio;
		
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG5(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"PttSetupAck---->bts, UID[0x%08X] GID[0x%04X] L3Addr[0x%08X] prio[0x%02X] encryptFlag[0x%02X]", 
				getUID(), getGID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr), 
				pDataCPE->CallPriority, pDataCPE->EncryptFlag);
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalPttConnect(UINT8 GrpOwnerFlag, UINT8 EncryptCtrl, UINT8 prio)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(GroupPTTConnectT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(PTT_Connect_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		GroupPTTConnectT_CPE* pDataCPE = (GroupPTTConnectT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_PTT_CONNECT;
		pDataCPE->grant = TRANSGRANT_PERMIT_TALK;
		pDataCPE->callOwnership = GrpOwnerFlag;
		pDataCPE->EncryptFlag = EncryptCtrl;
		pDataCPE->callPriority = prio;
#ifdef M__SUPPORT__ENC_RYP_TION
		memset(pDataCPE->EncryptKey, 0, sizeof(pDataCPE->EncryptKey));
		SxcGrpCCB *pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(getGID());
		if(pGrpCCB)
		{
			memcpy(pDataCPE->EncryptKey,
				pGrpCCB->getEncryptKey(),
				sizeof(pDataCPE->EncryptKey));
		}
#endif
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG6(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"PttConnect---->bts, UID[0x%08X] GID[0x%04X] L3Addr[0x%08X] OwnerShip[0x%02X] EncryptCtrl[0x%02X] prio[0x%02X] ", 
				getUID(), getGID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr), 
				pDataCPE->callOwnership, pDataCPE->EncryptFlag, pDataCPE->callPriority);
		}
	}
	return ret;	
}
bool CCCB::sendDLSignalPttRlsAck()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(PTTReleaseAckT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(PTT_RlsAck_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		PTTReleaseAckT_CPE* pDataCPE = (PTTReleaseAckT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_PTT_RLS_ACK;
		
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"PttReleaseAck---->bts, UID[0x%08X] GID[0x%04X] L3Addr[0x%08X]", 
				getUID(), getGID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr));
		}
	}
	return ret;	
}
UINT8 CCCB::getGrpCallingRlsReason(CMessage& msg)
{
	InnerSignal_GrpCallingRlsT* pSigBuf = (InnerSignal_GrpCallingRlsT*)msg.GetDataPtr();
	return pSigBuf->reason;
}
bool CCCB::sendDLSignalGrpCallingRls(UINT8 relReason)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(GroupCallingRlsT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(Group_CallingRls_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		GroupCallingRlsT_CPE* pDataCPE = (GroupCallingRlsT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_GRP_CALLING_RLS;
		pDataCPE->reason= relReason;
		
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG4(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"GrpCallingRls---->bts, UID[0x%08X] GID[0x%04X] L3Addr[0x%08X] RelCause[0x%02X]", 
				getUID(), getGID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr), 
				relReason);
		}
	}
	return ret;	
}
UINT8 CCCB::getPttInterruptReason(CMessage& msg)
{
	InnerSignal_PttInterruptT* pSigBuf = (InnerSignal_PttInterruptT*)msg.GetDataPtr();
	return pSigBuf->reason;
}
bool CCCB::sendDLSignalPttInterrupt(UINT8 relReason)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(TalkInterruptT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		signal.SetSigIDS(PTT_Interrupt_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getL3Addr());
		TalkInterruptT_CPE* pDataCPE = (TalkInterruptT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_PTT_INTERRUPT;
		pDataCPE->grant = TRANSGRANT_FORBID_TALK;
		pDataCPE->encryptFlag = getEncryptCtrl();
		pDataCPE->reason = relReason;
		pDataCPE->telno[0]=pDataCPE->telno[1]=pDataCPE->telno[2]=0;
		
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG4(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"PttInterrupt---->bts, UID[0x%08X] GID[0x%04X] L3Addr[0x%08X] reason[0x%02X]", 
				getUID(), getGID(), VGetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr), 
				relReason);
		}
	}
	return ret;	
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
bool CCCB::sendInnerSignalCallArrive(UINT32 dstUID)
{
	bool ret = false;
	CSAbisSignal innerSignal;
	if ( !innerSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(InnerSignal_CallArriveT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CSAG::getSagInstance()->setInnerSignalHead(innerSignal, SAG_INNER_CallArrive_Msg);
		InnerSignal_CallArriveT* pData = (InnerSignal_CallArriveT*)innerSignal.GetDataPtr();
		VSetU32BitVal(pData->dstUID, dstUID);
		VSetU32BitVal(pData->srcUID, getUID());
		VSetU32BitVal(pData->srcL3Addr, getL3Addr());
		ret = CSAG::getSagInstance()->sendInnerSignal(innerSignal);
		if(ret)
		{
			LOG5(SAG_LOG_INNER_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"InnerSignal CallArrive  , UID[0x%08X] L3Addr[0x%08X] ----> UID[0x%02X],  caller[%s] called[%s]", 
				getUID(), getL3Addr(), dstUID, (int)getOwnNumber(), (int)getDialedNumber());
		}
	}
	return ret;
}
bool CCCB::sendInnerSignalAlerting()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(AlertingT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CSAG::getSagInstance()->setInnerSignalHead(signal, Alerting_MSG);
		signal.SetSigIDS(Alerting_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getPeerCCB()->getL3Addr());
		AlertingT_CPE* pDataCPE = (AlertingT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_ALERTING;
		pDataCPE->selcodeinfo.type=0x02;
		pDataCPE->selcodeinfo.lenth=1;
		pDataCPE->selcodeinfo.value=CODEC_G729A;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendInnerSignal(signal);
		if(ret)
		{
			LOG6(SAG_LOG_INNER_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"InnerSignal Alerting  , UID[0x%08X] L3Addr[0x%08X] ----> UID[0x%08X] L3Addr[0x%08X] caller[%s] called[%s]", 
 				getUID(), getL3Addr(), getPeerCCB()->getUID(), getPeerCCB()->getL3Addr(), 
 				(int)getPeerCCB()->getOwnNumber(), (int)getPeerCCB()->getDialedNumber());
		}
	}
	return ret;	
}
bool CCCB::sendInnerSignalConnect()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(ConnectSAGT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CSAG::getSagInstance()->setInnerSignalHead(signal, Connect_MSG);
		signal.SetSigIDS(Connect_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getPeerCCB()->getL3Addr());
		ConnectSAGT_CPE* pDataCPE = (ConnectSAGT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_CONNECT;
		pDataCPE->selcodeinfo.type=0x02;
		pDataCPE->selcodeinfo.lenth=1;
		pDataCPE->selcodeinfo.value=CODEC_G729A;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendInnerSignal(signal);
		if(ret)
		{
			LOG6(SAG_LOG_INNER_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"InnerSignal Connect  , UID[0x%08X] L3Addr[0x%08X] ----> UID[0x%08X] L3Addr[0x%08X] caller[%s] called[%s]", 
 				getUID(), getL3Addr(), getPeerCCB()->getUID(), getPeerCCB()->getL3Addr(), 
 				(int)getPeerCCB()->getOwnNumber(), (int)getPeerCCB()->getDialedNumber());
		}
	}
	return ret;	
}
bool CCCB::sendInnerSignalConnectAck()
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(ConnectAckT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CSAG::getSagInstance()->setInnerSignalHead(signal, ConnectAck_MSG);
		signal.SetSigIDS(ConnectAck_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getPeerCCB()->getL3Addr());
		ConnectAckT_CPE* pDataCPE = (ConnectAckT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_CONNECTACK;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendInnerSignal(signal);
		if(ret)
		{
			LOG6(SAG_LOG_INNER_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"InnerSignal ConnectAck  , UID[0x%08X] L3Addr[0x%08X] ----> UID[0x%08X] L3Addr[0x%08X] caller[%s] called[%s]", 
 				getUID(), getL3Addr(), getPeerCCB()->getUID(), getPeerCCB()->getL3Addr(), 
 				(int)getOwnNumber(), (int)getDialedNumber());
		}
	}
	return ret;	
}
bool CCCB::sendInnerSignalDisconnect2Peer(UINT8 reason)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal = sizeof(DisconnectT_CPE) - sizeof(UINT8) + sizeof(UINT32);
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CSAG::getSagInstance()->setInnerSignalHead(signal, Disconnect_MSG);
		signal.SetSigIDS(Disconnect_MSG);
		pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		VSetU32BitVal(pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr, getPeerCCB()->getL3Addr());
		DisconnectT_CPE* pDataCPE = (DisconnectT_CPE*)(&pData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pDataCPE->msgType = M_MSGTYPE_DISCONNECT;
		pDataCPE->RelCause = reason;

		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendInnerSignal(signal);
		if(ret)
		{
			LOG6(SAG_LOG_INNER_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"InnerSignal Disconnect  , UID[0x%08X] L3Addr[0x%08X] ----> UID[0x%08X] L3Addr[0x%08X] onhook[%s] peer[%s]", 
 				getUID(), getL3Addr(), getPeerCCB()->getUID(), getPeerCCB()->getL3Addr(), 
 				(int)getOwnNumber(), (int)getPeerCCB()->getOwnNumber());
		}
	}
	return ret;	
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


