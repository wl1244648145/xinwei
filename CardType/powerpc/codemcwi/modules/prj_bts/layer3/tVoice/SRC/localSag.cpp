/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSag.cpp
* Create Date    : 16-Sep-2009
* programmer     :
* description    :
和tVoice任务间有用户信息的交互(增删用户等消息)；
从tVCR任务接收信令消息；
发送信令消息时直接发送给tVOICE任务；
自己产生定时器消息；
需要支持放拨号音、回铃音、忙音；
语音数据的处理使用bts内部语音自交换功能；
* functions      : 
* Modify History :
*******************************************************************************/
#include "localSag.h"
#include "localSagCCB.h"
#include "localSagStruct.h"
#include "localSagCommon.h"
#include "localSagMsgID.h"
#include "localSagCfg.h"
#include "localSagFsmCfg.h"
#include "voiceToolFunc.h"
#include "cpe_signal_struct.h"

#include "log.h"
#include "tVoice.h"
#include "BtsVMsgId.h"
#include "OtherMsg.h"
#include "voiceTone.h"
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//此处和普通SAG一样保证srtp接口的L3地址为低2byte有效
#define M_MAX_L3ADDR (0x0000FF00)
#define M_MIN_L3ADDR (0x00000001)
UINT32 CSAG::m_L3AddrRes = M_MIN_L3ADDR;	
//信令统计表
UINT32	CSAG::RxSignalCounter[InvalidSignal_MSG+1]={0,};
UINT32	CSAG::TxSignalCounter[InvalidSignal_MSG+1]={0,};

SignalProcFuncPtr CSAG::signalProc[InvalidSignal_MSG+1]={};
timeoutProcFuncPtr CSAG::timeoutProc[TIMERID_SAG_COUNT]={};

void CSAG::initSignalHandlers()
{
	int i;
	for(i=0;i<InvalidSignal_MSG+1;i++)
	{
		signalProc[i] = NULL;
	}

	signalProc[LAPagingRsp_MSG] = &CSAG::handleSignalLAPagingRsp;//LAPagingRsp_MSG,
	signalProc[DELAPagingRsp_MSG] = &CSAG::handleSignalDeLAPagingRsp;//DELAPagingRsp_MSG,
	signalProc[AssignResReq_MSG] = &CSAG::handleSignalAssignResReq;//AssignResReq_MSG,
	signalProc[RlsResRsp_MSG] = &CSAG::handleSignalReleaseResRsp;//RlsResRsp_MSG,
	signalProc[ErrNotifyReq_MSG] = &CSAG::handleSignalErrNotiReq;//ErrNotifyReq_MSG,
	signalProc[ErrNotifyRsp_MSG] = &CSAG::handleSignalErrNotiRsp;//ErrNotifyRsp_MSG,

	signalProc[Setup_MSG] = &CSAG::handleSignalSetup;//Setup_MSG,
	signalProc[CallProc_MSG] = &CSAG::handleSignalSetupAck;//CallProc_MSG,
	signalProc[Alerting_MSG] = &CSAG::handleSignalAlerting;//Alerting_MSG,
	signalProc[Connect_MSG] = &CSAG::handleSignalConnect;//Connect_MSG,
	signalProc[ConnectAck_MSG] = &CSAG::handleSignalConnectAck;//ConnectAck_MSG,
	signalProc[Disconnect_MSG] = &CSAG::handleSignalDisconnect;//Disconnect_MSG,
	signalProc[ReleaseComplete_MSG] = &CSAG::handleSignalReleaseComplete;//ReleaseComplete_MSG,
	signalProc[Information_MSG] = &CSAG::handleSignalInformation;//Information_MSG,
	signalProc[ModiMediaReq_MSG] = &CSAG::handleSignalModifyMediaReq;//ModiMediaReq_MSG,
	signalProc[ModiMediaRsp_MSG] = &CSAG::handleSignalModifyMediaRsp;//ModiMediaRsp_MSG,

	signalProc[AuthCmdRsp_MSG] = &CSAG::handleSignalAuthCmdRsp;//AuthCmdRsp_MSG,
	signalProc[Login_MSG] = &CSAG::handleSignalLoginReq;//Login_MSG,
	signalProc[Logout_MSG] = &CSAG::handleSignalLogout;//Logout_MSG,

	signalProc[HandOverReq_MSG] = &CSAG::handleSignalHandOverReq;//HandOverReq_MSG,
	signalProc[HandOverComplete_MSG] = &CSAG::handleSignalHandOverComplete;//HandOverComplete_MSG,

	signalProc[MOSmsDataReq_MSG] = &CSAG::handleSignalMOSMSDataReq;//MOSmsDataReq_MSG,
	signalProc[MTSmsDataRsp_MSG] = &CSAG::handleSignalMTSMSDataRsp;//MTSmsDataRsp_MSG,
	signalProc[SMSMemAvailReq_MSG] = &CSAG::handleSignalSMSMemAvailReq;//SMSMemAvailReq_MSG,

	signalProc[Auth_Info_Req_MSG] = &CSAG::handleSignalAuthInfoReq;//Auth_Info_Req_MSG,

#ifdef M__SUPPORT__ENC_RYP_TION
	signalProc[OAMTransfer_Info_Req_MSG] = &CSAG::handleSignalOamTransferInfoReq;//OAMTransfer_Info_Req_MSG,
	signalProc[OAMTransfer_Info_Rsp_MSG] = &CSAG::handleSignalOamTransferInfoRsp;//OAMTransfer_Info_Rsp_MSG,
	signalProc[SecuriryCardCallPara_Req_MSG] = &CSAG::handleSignalSecuriryCardCallParaReq;//SecuriryCardCallPara_Req_MSG,
	signalProc[SecuriryCardCallPara_Rsp_MSG] = &CSAG::handleSignalSecuriryCardCallParaRsp;//SecuriryCardCallPara_Rsp_MSG,
#endif

	signalProc[LAGrpPagingRsp_MSG] = &CSAG::handleSignalLAGrpPagingRsp;
	signalProc[StatusReport_MSG] = &CSAG::handleSignalStatusReport;
	signalProc[GrpHandoverReq_MSG] = &CSAG::handleSignalGrpHandoverReq;
	signalProc[GrpResReq_MSG] = &CSAG::handleSignalGrpResReq; 
	signalProc[PTT_SetupReq_MSG] = &CSAG::handleSignalPttSetupReq; 
	signalProc[PTT_ConnectAck_MSG] = &CSAG::handleSignalPttConnectAck; 
	signalProc[Grp_Disconnect_MSG] = &CSAG::handleSignalGrpDisconnect; 
	signalProc[Group_CallingRlsComplete_MSG] = &CSAG::handleSignalGrpCallingRlsComplete; 
	signalProc[PTT_PressReq_MSG] = &CSAG::handleSignalPttPressReq; 
	signalProc[PTT_InterruptAck_MSG] = &CSAG::handleSignalPttInterruptAck; 
	signalProc[PTT_Rls_MSG] = &CSAG::handleSignalPttRls; 
	signalProc[PTT_PressCancel_MSG] = &CSAG::handleSignalPttPressCancel; 
	signalProc[PTT_PressApplyReq_MSG] = &CSAG::handleSignalPttPressApplyReq;

	signalProc[InvalidSignal_MSG] = &CSAG::handleSignal_Invalid;		//InvalidSignal_MSG,
	
}
void CSAG::initTimeoutHandlers()
{
	int i;
	for(i=0;i<TIMERID_SAG_COUNT;i++)
	{
		timeoutProc[i] = NULL;
	}
	//begin
	timeoutProc[TIMERID_SAG_O_SETUP]=&CSAG::handleTimeout_O_Setup;
	timeoutProc[TIMERID_SAG_O_DIALNUMBER]=&CSAG::handleTimeout_O_DialNumber;
	timeoutProc[TIMERID_SAG_O_ALERTING]=&CSAG::handleTimeout_O_Alerting;	
	timeoutProc[TIMERID_SAG_O_CONNECT]=&CSAG::handleTimeout_O_Connect;
	timeoutProc[TIMERID_SAG_O_CONNECTACK]=&CSAG::handleTimeout_O_ConnectAck;
	timeoutProc[TIMERID_SAG_T_SETUPACK]=&CSAG::handleTimeout_T_SetupAck;
	timeoutProc[TIMERID_SAG_T_ALERTING]=&CSAG::handleTimeout_T_Alerting;
	timeoutProc[TIMERID_SAG_T_CONNECT]=&CSAG::handleTimeout_T_Connect;
	timeoutProc[TIMERID_SAG_T_CONNECTACK]=&CSAG::handleTimeout_T_ConnectAck;
	timeoutProc[TIMERID_SAG_DISCONNECT]=&CSAG::handleTimeoutDisconnect;
	timeoutProc[TIMERID_SAG_RELEASE_COMPLETE]=&CSAG::handleTimeoutReleaseComplete;
	timeoutProc[TIMERID_SAG_T_LAPAGING]=&CSAG::handleTimeout_T_LaPaging;
	timeoutProc[TIMERID_SAG_T_ASSIGN_TRANS_RES]=&CSAG::handleTimeout_T_AssignTransRes;
	//grpTimers
	timeoutProc[TIMERID_SAG_GRP_PTTCONNECT]=&CSAG::handleTimeout_Grp_PttConnect;//组呼建立超时
	timeoutProc[TIMERID_SAG_GRP_LAGRPPAGING]=&CSAG::handleTimeout_Grp_LAGrpPaging;
	timeoutProc[TIMERID_SAG_GRP_ASSIGNRESREQ]=&CSAG::handleTimeout_Grp_AssignResReq;
	timeoutProc[TIMERID_SAG_GRP_PRESSINFO]=&CSAG::handleTimeout_Grp_PressInfo;
	timeoutProc[TIMERID_SAG_GRP_MAX_IDLE_TIME]=&CSAG::handleTimeout_Grp_MaxIdleTime;
	timeoutProc[TIMERID_SAG_GRP_TTL]=&CSAG::handleTimeout_Grp_TTL;
	timeoutProc[TIMERID_SAG_GRP_MAX_TALKING_TIME]=&CSAG::handleTimeout_Grp_TalkingTime;	

	//new timer add here
	//end	
}

CSAG* CSAG::s_pSagInst=NULL;
CSAG* CSAG::getSagInstance()
{
	if(s_pSagInst==NULL)
	{
		s_pSagInst = new CSAG;
	}
	return s_pSagInst;
}

bool CSAG::Init()
{
	UINT16 i;
	//初始化空闲CCB池
	m_freeCCBList.clear();
	for(i=0;i<M_MAX_CCB_NUM;i++)
	{
		m_CCBTable[i].setTabIndex(i);
		m_freeCCBList.push_back(i);
	}
	m_freeGrpCCBList.clear();
	for(i=0;i<M_MAX_GRPCCB_NUM;i++)
	{
		m_GrpCCBTable[i].setTableIndex(i);
		m_freeGrpCCBList.push_back(i);
	}
	//信令统计计数初始化
	clearSignalCounters();
	return true;
}
UINT32 CSAG::AllocateL3Addr()
{
	return m_L3AddrRes>M_MAX_L3ADDR ? M_MIN_L3ADDR:(m_L3AddrRes++);
}

CCCB* CSAG::AllocCCB(UINT32 uid)
{
	UINT16 id;
	CCCB* ret = NULL;
#if 0	
	ret = FindCCBByUID(uid);
	if(ret==NULL)
#endif		
	{
		if(!m_freeCCBList.empty())
		{
			id = *(m_freeCCBList.begin());
			ret = &m_CCBTable[id];
			m_freeCCBList.pop_front();
			ret->ClearCCBInfo();
		}
		else
		{
			LOG(LOG_CRITICAL, LOGNO(SAG, EC_L3VOICE_NORMAL), "ALL CCBs used up, can not allocate CCB!!!");
		}
	}
	return ret;
}
void CSAG::DeAllocCCB(UINT16 TabIndex)
{
	if(TabIndex<M_MAX_CCB_NUM)
	{
		m_CCBTable[TabIndex].ClearCCBInfo();
		m_freeCCBList.push_back(TabIndex);
	}
	else
	{
		LOG(LOG_SEVERE, 0, "CSAG::DeAllocCCB(UINT16 TabIndex), TabIndex out of range!!!");
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
	string str_num(number);
	PHONE_ADDRESS_BOOK::iterator it = m_Phone_Address_Book.find(str_num);
	return (it==m_Phone_Address_Book.end()) ? NULL: (*it).second;
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

void CSAG::AddPhoneAddressBook(char* number, CCCB* pCCB)
{
	if(NULL==number || 0==strlen(number) || NULL==pCCB)
		return;
	string strNumber(number);
	m_Phone_Address_Book.insert(PHONE_ADDRESS_BOOK::value_type(strNumber, pCCB));
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
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CSAG::sigleVoiceCallTimeoutProc(CMessage& msg)
{
	PSagTimerStructT pData = (PSagTimerStructT)msg.GetDataPtr();
	CCCB* pCCB = FindCCBByUID(pData->uid);
	if(pCCB!=NULL)
	{
		pCCB->deleteTimer(pData->type);
		injectFsm(msg);
	}
	else
	{
		LOG3(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"timeout Msg, UID[0x%08X] L3Addr[0x%08X] timerType[%s], cannot find CCB!!!", 
				pData->uid, pData->l3Addr, (int)sagTimerName[pData->type]);
	}
}
void CSAG::handleTimeout_O_Setup(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_O_DialNumber(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_O_Alerting(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_O_Connect(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_O_ConnectAck(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_T_SetupAck(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_T_Alerting(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_T_Connect(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_T_ConnectAck(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeoutDisconnect(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeoutReleaseComplete(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_T_LaPaging(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_T_AssignTransRes(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CSAG::handleSignal_Invalid(CSAbisSignal& signal)
{
	//LOG
}

void CSAG::handleSignalLAPagingRsp(CSAbisSignal& signal)
{
	//收到被叫寻呼回应发送DeLAPaging
	//只有本地一个bts无需考虑停止寻呼

	injectFsm(signal);
}

void CSAG::handleSignalDeLAPagingRsp(CSAbisSignal& signal)
{
	//do nothing
}
void CSAG::handleSignalAssignResReq(CSAbisSignal& signal)
{
	//切换应用则回失败的AssignResRsp,不支持切换
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
//20121115Modify by fengbing begin
	//区分VAC建链原因
	if(M_VACSETUPREASON_GRP_SETUP==signal.GetEID()||
		M_VACSETUPREASON_ENC_GRP_SETUP==signal.GetEID())
	{
		VSetU16BitVal(pSigBufRcv->sigPayload.AssignResReq.ServerOption , 
			(M_LOCALSAG_ASSIGNRES_REASON_GRP_SETUP));
	}
	if(M_VACSETUPREASON_GRP_TALKING==signal.GetEID() ||
		M_VACSETUPREASON_ENC_GRP_TALKING==signal.GetEID())
	{
		VSetU16BitVal(pSigBufRcv->sigPayload.AssignResReq.ServerOption , 
			(M_LOCALSAG_ASSIGNRES_REASON_GRP_TALKING));
	}
//20121115Modify by fengbing end
	UINT16 srvOpt = VGetU16BitVal(pSigBufRcv->sigPayload.AssignResReq.ServerOption);
	if(APPTYPE_VOICE_SWITCH==srvOpt)
	{
		CSAbisSignal AssgnResRsp;
		if ( !AssgnResRsp.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(AssignResRspT)) )
		{
			LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		}
		else
		{
			SAbisSignalT* pSigBuf = (SAbisSignalT*)AssgnResRsp.GetDataPtr();
			SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();

			setAllHeadTailFields(sizeof(AssignResRspT), AssignResRsp_MSG, AssgnResRsp);
			
			pSigBuf->sigPayload.AssignResRsp.AssignResult = 1;//fail
			VSetU32BitVal(pSigBuf->sigPayload.AssignResRsp.L3addr, NO_L3ADDR);
			memcpy(pSigBuf->sigPayload.AssignResRsp.UID ,
				pSigBufRcv->sigPayload.AssignResReq.UID, 4);

			if(sendSignalToLocalBTS(AssgnResRsp))
			{
				LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"AssignResRsp---->bts, UID[0x%08X] L3Addr[0x%08X] Result[0x%02X] ", 
					VGetU32BitVal(pSigBuf->sigPayload.AssignResRsp.UID), 
					VGetU32BitVal(pSigBuf->sigPayload.AssignResRsp.L3addr),
					pSigBuf->sigPayload.AssignResRsp.AssignResult);
			}
		}
	}
	else
	{
		injectFsm(signal);
	}
}

void CSAG::handleSignalReleaseResRsp(CSAbisSignal& signal)
{
	//do nothing
	//清除呼叫相关信息,l3地址作废这些工作在进入IDLE状态时做

}

void CSAG::handleSignalErrNotiReq(CSAbisSignal& signal)
{
	//回应ErrorNotiRsp
	CSAbisSignal sndSignal;
	if ( !sndSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(ErrNotifyRspT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		SAbisSignalT* pSigBufSnd = (SAbisSignalT*)sndSignal.GetDataPtr();
		SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();

		setAllHeadTailFields(sizeof(ErrNotifyRspT), ErrNotifyRsp_MSG, sndSignal);
		
		memcpy(pSigBufSnd->sigPayload.ErrNotifyRsp.Uid, pSigBufRcv->sigPayload.ErrNotifyReq.Uid, 4);
		memcpy(pSigBufSnd->sigPayload.ErrNotifyRsp.L3Addr, pSigBufRcv->sigPayload.ErrNotifyReq.L3Addr, 4);

		if(sendSignalToLocalBTS(sndSignal))
		{
			LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"ErrNotifyRsp---->bts, UID[0x%08X] L3Addr[0x%08X] ", 
				VGetU32BitVal(pSigBufSnd->sigPayload.AssignResRsp.UID), 
				VGetU32BitVal(pSigBufSnd->sigPayload.AssignResRsp.L3addr));
		}
	}
	//消息进状态机
	injectFsm(signal);
}
void CSAG::handleSignalErrNotiRsp(CSAbisSignal& signal)
{
	//do nothing
}

void CSAG::handleSignalSetup(CSAbisSignal& signal)
{
	injectFsm(signal);
}
void CSAG::handleSignalSetupAck(CSAbisSignal& signal)
{
	//不用转给另一方，消息进入状态机
	injectFsm(signal);
}
void CSAG::handleSignalAlerting(CSAbisSignal& signal)
{
	injectFsm(signal);
}
void CSAG::handleSignalConnect(CSAbisSignal& signal)
{
	injectFsm(signal);
}
void CSAG::handleSignalConnectAck(CSAbisSignal& signal)
{
	injectFsm(signal);
}
void CSAG::handleSignalDisconnect(CSAbisSignal& signal)
{
	injectFsm(signal);
}

void CSAG::handleSignalInformation(CSAbisSignal& signal)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(NULL==pCCB)
	{
		return;
	}
	//如果是主叫收号状态
	UINT16 curState = pCCB->getState();
	if(SAG_DIAL_STATE==curState)
	{
		injectFsm(signal);
	}
	else
	{
		if(SAG_CONNECT_STATE==curState)
		{
			//向对端终端发送information信令(后续号码)
			InformationT_CPE* pDataCPE = (InformationT_CPE*)(&pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
			if(0==pDataCPE->type)//拨号消息
			{
				CCCB* pPeerCCB = pCCB->getPeerCCB();
				if(pPeerCCB!=NULL)
				{
					if(SAG_CONNECT_STATE==pPeerCCB->getState())
					{
						//替换成对端的L3Addr，准备把信令转发给对端
						VSetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr, pPeerCCB->getL3Addr());
						if(sendSignalToLocalBTS(signal))
						{
							LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
								"Information---->bts, UID[0x%08X] L3Addr[0x%08X] Digit[0x%02X] ", 
								pPeerCCB->getUID(), 
								VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr),
								pDataCPE->digi_No.num);
						}
					}
				}				
			}
		}
	}
}


void CSAG::handleSignalReleaseComplete(CSAbisSignal& signal)
{
	injectFsm(signal);
}

void CSAG::handleSignalModifyMediaReq(CSAbisSignal& signal)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(NULL==pCCB)
	{
		return;
	}
	
	//转发给对端终端
	CCCB* pPeerCCB = pCCB->getPeerCCB();
	if(pPeerCCB!=NULL)
	{
		if(SAG_CONNECT_STATE==pPeerCCB->getState())
		{
			ModiMediaReqT_CPE* pDataCPE = (ModiMediaReqT_CPE*)(&pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
			//替换成对端的L3Addr，准备把信令转发给对端
			VSetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr, pPeerCCB->getL3Addr());
			if(sendSignalToLocalBTS(signal))
			{
				LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"ModifyMediaReq---->bts, UID[0x%08X] L3Addr[0x%08X] MediaType[0x%02X] ", 
					pPeerCCB->getUID(), 
					VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr),
					pDataCPE->mediaType);
			}
		}
	}				
}	

void CSAG::handleSignalModifyMediaRsp(CSAbisSignal& signal)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 L3Addr = VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr);
	CCCB* pCCB = FindCCBByL3Addr(L3Addr);
	if(NULL==pCCB)
	{
		return;
	}
	
	//转发给对端终端
	CCCB* pPeerCCB = pCCB->getPeerCCB();
	if(pPeerCCB!=NULL)
	{
		if(SAG_CONNECT_STATE==pPeerCCB->getState())
		{
			ModiMediaRspT_CPE* pDataCPE = (ModiMediaRspT_CPE*)(&pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
			//替换成对端的L3Addr，准备把信令转发给对端
			VSetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr, pPeerCCB->getL3Addr());
			if(sendSignalToLocalBTS(signal))
			{
				LOG3(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"ModifyMediaRsp---->bts, UID[0x%08X] L3Addr[0x%08X] Result[0x%02X] ", 
					pPeerCCB->getUID(), 
					VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr),
					pDataCPE->result);
			}
		}
	}				
}

void CSAG::handleSignalHandOverReq(CSAbisSignal& signal)
{
	//不支持切换，回应失败的HandOverRsp，
	//应该不会走到这个流程，因为切换时assignResReq时就已经拒绝了
	//do nothing
}

void CSAG::handleSignalHandOverComplete(CSAbisSignal& signal)
{
	//不支持切换
	//应该不会走到这个流程，因为切换时assignResReq时就已经拒绝了
	//do nothing
}

void CSAG::handleSignalSecuriryCardCallParaReq(CSAbisSignal & signal)
{
#ifdef M__SUPPORT__ENC_RYP_TION
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 l3Addr = VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr);
	UINT8 funcTag = pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.UTSAGPayload[0];

	LOG2(SAG_LOG_UL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"SecuriryCardCallParaReq<----bts, L3Addr[0x%08X] tag[0x%02X] ", 
		l3Addr, funcTag);
	
	CCCB* pCCB = FindCCBByL3Addr(l3Addr);
	if(!pCCB)
	{
		LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "cannot find CCB!!!");
		return;
	}

	if(funcTag==FUNCLIST_5)//脱网参数
	{
		//转发给对端终端
		CCCB* pPeerCCB = pCCB->getPeerCCB();
		if(pPeerCCB)
		{
			//替换成对端的L3Addr，准备把信令转发给对端
			VSetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr, 
				pPeerCCB->getL3Addr());
			if(sendSignalToLocalBTS(signal))
			{
				LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"SecuriryCardCallParaReq---->bts, L3Addr[0x%08X] tag[0x%02X] ", 
					pPeerCCB->getL3Addr(), funcTag);
			}
		}
		else
		{
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), 
				"peerCCB is NULL!!!");
		}
	}	
#endif
}

void CSAG::handleSignalSecuriryCardCallParaRsp(CSAbisSignal & signal)
{
#ifdef M__SUPPORT__ENC_RYP_TION
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 l3Addr = VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr);
	UINT8 funcTag = pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.UTSAGPayload[0];

	LOG2(SAG_LOG_UL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"SecuriryCardCallParaRsp<----bts, L3Addr[0x%08X] tag[0x%02X] ", 
		l3Addr, funcTag);
	
	CCCB* pCCB = FindCCBByL3Addr(l3Addr);
	if(!pCCB)
	{
		LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "cannot find CCB!!!");
		return;
	}

	if(funcTag==CONFIGRESULT_5)//脱网参数
	{
		//转发给对端终端
		CCCB* pPeerCCB = pCCB->getPeerCCB();
		if(pPeerCCB)
		{
			//替换成对端的L3Addr，准备把信令转发给对端
			VSetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.L3Addr, 
				pPeerCCB->getL3Addr());
			if(sendSignalToLocalBTS(signal))
			{
				LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"SecuriryCardCallParaRsp---->bts, L3Addr[0x%08X] tag[0x%02X] ", 
					pPeerCCB->getL3Addr(), funcTag);
			}
		}
		else
		{
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), 
				"peerCCB is NULL!!!");
		}
	}	
#endif
}

void CSAG::handleSignalOamTransferInfoReq(CSAbisSignal & signal)
{
#ifdef M__SUPPORT__ENC_RYP_TION
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 UID = VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_Uid.Uid);
	UINT8 *pPID = &pSigBufRcv->sigPayload.UTSAG_Payload_Uid.UTSAGPayload[0];
	UINT8 funcTag = pSigBufRcv->sigPayload.UTSAG_Payload_Uid.UTSAGPayload[4];

	LOG2(SAG_LOG_UL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"OamTransferInfoReq<----bts, UID[0x%08X] tag[0x%02X] ", 
		UID, funcTag);
	
	CCCB* pCCB = FindCCBByUID(UID);
	if(!pCCB)
	{
		LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "cannot find CCB!!!");
		return;
	}

	if(funcTag==FUNCLIST_5)//脱网参数
	{
		//转发给对端终端
		CCCB* pPeerCCB = pCCB->getPeerCCB();
		if(pPeerCCB)
		{
			//替换成对端的UID/PID，准备把信令转发给对端
			VSetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_Uid.Uid, 
				pPeerCCB->getUID());
			VSetU32BitVal(pPID, pPeerCCB->getEID());
			if(sendSignalToLocalBTS(signal))
			{
				LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"OamTransferInfoReq---->bts, UID[0x%08X] tag[0x%02X] ", 
					pPeerCCB->getUID(), funcTag);
			}
		}
		else
		{
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), 
				"peerCCB is NULL!!!");
		}
	}	
#endif
}

void CSAG::handleSignalOamTransferInfoRsp(CSAbisSignal & signal)
{
#ifdef M__SUPPORT__ENC_RYP_TION
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 UID = VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_Uid.Uid);
	UINT8 *pPID = &pSigBufRcv->sigPayload.UTSAG_Payload_Uid.UTSAGPayload[0];
	UINT8 funcTag = pSigBufRcv->sigPayload.UTSAG_Payload_Uid.UTSAGPayload[4];

	LOG2(SAG_LOG_UL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"OamTransferInfoRsp<----bts, UID[0x%08X] tag[0x%02X] ", 
		UID, funcTag);
	
	CCCB* pCCB = FindCCBByUID(UID);
	if(!pCCB)
	{
		LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "cannot find CCB!!!");
		return;
	}

	if(funcTag==CONFIGRESULT_5)//脱网参数
	{
		//转发给对端终端
		CCCB* pPeerCCB = pCCB->getPeerCCB();
		if(pPeerCCB)
		{
			//替换成对端的UID/PID，准备把信令转发给对端
			VSetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_Uid.Uid, 
				pPeerCCB->getUID());
			VSetU32BitVal(pPID, pPeerCCB->getEID());
			if(sendSignalToLocalBTS(signal))
			{
				LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"OamTransferInfoRsp---->bts, UID[0x%08X] tag[0x%02X] ", 
					pPeerCCB->getUID(), funcTag);
			}
		}
		else
		{
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), 
				"peerCCB is NULL!!!");
		}
	}	
#endif
}

void CSAG::handleSignalAuthInfoReq(CSAbisSignal & signal)
{
/*
SAG ID	4	M		
BTS ID 	2	M		
EVENT GROUP ID	1	M		
EVENT ID	2	M		
LENGH	2	M		
UID	4	M		
PID	4	M		
Auth_res	1	M		
Ind	1	M		
result	1	C		当Ind为1或者2时携带
SID	16	C		当result为0x00时携带
NickName	18	C	当Ind为3时携带
CID	3	M	
*/
	typedef struct __tmpAuthResultT
	{
		UINT8 UID[4];
		UINT8 PID[4];
		UINT8 AuthRes;
#ifdef M__SUPPORT__ENC_RYP_TION
		UINT8 Ind;//should be 0x4 20110426 by fb
#else
		UINT8 Ind;//should be 0
#endif
		UINT8 CID[3];
	}tmpAuthResultT;

	//回应AuthResult
	CSAbisSignal sndSignal;
	if ( !sndSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(tmpAuthResultT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		SAbisSignalT* pSigBufSnd = (SAbisSignalT*)sndSignal.GetDataPtr();
		SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
		AuthInfoReqT* pDataRcv = (AuthInfoReqT*)&pSigBufRcv->sigPayload;
		tmpAuthResultT* pDataSnd = (tmpAuthResultT*)&pSigBufSnd->sigPayload;

		setAllHeadTailFields(sizeof(tmpAuthResultT), Auth_Result_MSG, sndSignal);
		
		memcpy(pDataSnd->UID, pDataRcv->Uid, 4);
		memcpy(pDataSnd->PID, pDataRcv->Pid, 4);
		pDataSnd->AuthRes = 0;//成功
#ifdef M__SUPPORT__ENC_RYP_TION		
		pDataSnd->Ind = 0x4;//should be 0x4 20110426 by fb
#else
		pDataSnd->Ind = 0;
#endif
		pDataSnd->CID[0] = pDataSnd->CID[1] = pDataSnd->CID[2] = 0xff;

		if(sendSignalToLocalBTS(sndSignal, M_TID_UM))
		{
			LOG2(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"AuthResult---->bts, UID[0x%08X] PID[0x%08X] ", 
				VGetU32BitVal(pDataSnd->UID), 
				VGetU32BitVal(pDataSnd->PID));
		}
	}
	
}

void CSAG::handleSignalLoginReq(CSAbisSignal& signal)
{
	if(g_blUseLocalUserInfoFile)
	{
		sendLoginRspWithUserSrvInfo(signal);
	}
	else
	{
		sendSuccessLoginRspWithoutUserSrvInfo(signal);
	}	
}

void CSAG::handleSignalLogout(CSAbisSignal& signal)
{
	//释放CCB
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = VGetU32BitVal(pSigBufRcv->sigPayload.UTSAG_Payload_Uid.Uid);
	CCCB* pCCB = FindCCBByUID(uid);
	//注销用户语音业务
	stopVoiceSrv(pCCB);
}

void CSAG::handleSignalAuthCmdRsp(CSAbisSignal& signal)
{
//local SAG暂时不支持鉴权	
#if 0	
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = ntohl(pSigBufRcv->sigPayload.AuthCmdRsp.containerHeader.uid_L3addr);
	CCCB* pCCB = FindCCBByUID(uid); 
	CCCB* pPeerCCB = pCCB->getPeerCCB();
	if(NULL==pCCB)
	{
		LOG(LOG_DEBUG3, 0, "cannot find CCB");
		return;
	}
	UINT8 authType = pCCB->getAuthReason();
	if(pSigBufRcv->sigPayload.AuthCmdRsp.result==0)	//鉴权失败
	{
		if(AUTH_REASON_MTCALL==authType)	//被叫鉴权
		{
			//给主叫发送Release
			if(pPeerCCB!=NULL)
			{
				sendDisconnect(pCCB->getPeerCCB(), pCCB->getPeerCCB()->getSrvBTS()->m_fdSignalSocket, REL_CAUSE_AUTHFAIL);
			}
		}
		if(AUTH_REASON_LOGIN==authType)		//注册鉴权
		{
			//发送LoginRsp;
			CSAbisSignal LoginRsp;
			SAbisSignalT* pSigLoginRsp = (SAbisSignalT*)LoginRsp.GetDataPtr();

			setAllHeadTailFields(sizeof(LoginRspT), UTSAG_UID_MSG, LoginRsp);

			pSigLoginRsp->sigPayload.LoginRsp.containerHeader.msgType = M_MSGTYPE_LOGINRSP;
			pSigLoginRsp->sigPayload.LoginRsp.containerHeader.uid_L3addr = htonl(uid);
			pSigLoginRsp->sigPayload.LoginRsp.LoginResult = LOGINRSP_NONUMBER;
			FillLAI(pSigLoginRsp->sigPayload.LoginRsp.LAI, pCCB->getSrvBTS()->getLAI());
			pSigLoginRsp->sigPayload.LoginRsp.RegPeriod = htons(90);

			sendSignalToLocalBTS(LoginRsp);
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

			setAllHeadTailFields(sizeof(AssignResReqT), AssignResReq_MSG, AssignResReq);
		
			pSigAssignResReq->sigPayload.AssignResReq.AssignReason = 0;
			pSigAssignResReq->sigPayload.AssignResReq.L3addr = htonl(pCCB->getL3Addr());
			pSigAssignResReq->sigPayload.AssignResReq.ReqRate = M_REQ_TRANS_RATE_DEFAULT;
			pSigAssignResReq->sigPayload.AssignResReq.UID = htonl(pCCB->getUID());

			sendSignalToLocalBTS(AssignResReq);
			
		}
		if(AUTH_REASON_LOGIN==authType)		//注册鉴权
		{
			//发送LoginRsp;
			CSAbisSignal LoginRsp;
			SAbisSignalT* pSigLoginRsp = (SAbisSignalT*)LoginRsp.GetDataPtr();

			setAllHeadTailFields(sizeof(LoginRspT), UTSAG_UID_MSG, LoginRsp);
		
			pSigLoginRsp->sigPayload.LoginRsp.containerHeader.msgType = M_MSGTYPE_LOGINRSP;
			pSigLoginRsp->sigPayload.LoginRsp.containerHeader.uid_L3addr = htonl(uid);
			pSigLoginRsp->sigPayload.LoginRsp.LoginResult = LOGINRSP_SUCCESS;
			FillLAI(pSigLoginRsp->sigPayload.LoginRsp.LAI, pCCB->getSrvBTS()->getLAI());
			pSigLoginRsp->sigPayload.LoginRsp.RegPeriod = htons(90);
			
			sendSignalToLocalBTS(LoginRsp);

			pCCB->setUID(uid);
			//如果没有CCB,申请CCB,更新相关查找表


		}
	}
#endif	
}

void CSAG::handleSignalMOSMSDataReq(CSAbisSignal& signal)
{
/*	
UID
Message type	1	M		
SequenceNo	1	M		
Payload	N	M		
	UID		4	
	SMS_DataLen		1	短消息内容长度
	SMS_BearerData			短消息内容
	SMS_CallNumLen		1	电话号码长度 
	SMS_CalledNo			短消息被叫(BCD码)
*/
typedef struct __tmpSMSDataReq
{
	UINT8 UID[4];
	UINT8 msgType;
	UINT8 seqNo;
	UINT8 UID_Payload[4];
	UINT8 SMS_DataLen;
	UINT8 SMS_BearerData;
}tmpSMSDataReqT;

	//check if is valid msg
	SAbisSignalT* pData = (SAbisSignalT*)signal.GetDataPtr();
	UINT16 nSigLen = signal.GetDataLength();
	UINT8 *pSigHead = (UINT8*)pData;
	tmpSMSDataReqT* pSMSPayload = (tmpSMSDataReqT*)pData->sigPayload.UTSAG_Payload_Uid.Uid;
	UINT8 numberLen = *(&pSMSPayload->SMS_BearerData + pSMSPayload->SMS_DataLen);
	UINT8 *pBCDNumber = &pSMSPayload->SMS_BearerData + pSMSPayload->SMS_DataLen +1;
	if(pSigHead+nSigLen<pBCDNumber+numberLen)
	{
		//消息错
		LOG1(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
						"MOSMSDataReq---->bts, UID[0x%08X]  invalid!!!!!!!", 
						VGetU32BitVal(pData->sigPayload.UTSAG_Payload_Uid.Uid));		
		return;
	}
	//获取短信的接收方号码MT
	

	//if(MT的号码存在)
	{
		//如果可以找到MT的CCB
		//MT记录MO的UID，用于收到MTSMSDataRsp后向MO回应MOSMSDataRsp
		//把短消息变为MTSMSDataReq发送给MT，携带发送者的号码
	}
	//else
	{
		//do nothing
	}
}

void CSAG::handleSignalMTSMSDataRsp(CSAbisSignal& signal)
{
	//转给MO方的CPE

}
void CSAG::handleSignalSMSMemAvailReq(CSAbisSignal& signal)
{
	//发送SMSMemAvailRsp

}

void CSAG::releaseCurCall(CCCB* pCCB)
{
	if(pCCB!=NULL)
	{
		if(NO_L3ADDR!=pCCB->getL3Addr())
		{
			//向bts发送release transport resource req
			pCCB->sendDLSignalRlsTransResReq(REL_CAUSE_UNKNOWN);
			//清除L3索引表条目
			DelL3AddrIndexTable(pCCB->getL3Addr());
		}
		//停止放音
		pCCB->stopTone(RINGBACK_TONE);
		//向peerCCB发Disconnect信令释放呼叫
		if((pCCB->getPeerCCB())!=NULL)
		{
			//向peerCCB发Disconnect信令释放呼叫
			pCCB->sendInnerSignalDisconnect2Peer(REL_CAUSE_UNKNOWN);
		}		
		pCCB->clearCCBCallInfo();
		pCCB->clearCCBGrpVoiceCallInfo();
	}
}

void CSAG::stopVoiceSrv(CCCB* pCCB)
{
	if(pCCB!=NULL)
	{
		releaseCurCall(pCCB);
		//清除所有的CCB相关的查找表
		DelUIDIndexTable(pCCB->getUID());
		//清除电话号码表相关表
		DelPhoneAddressBook(pCCB->getOwnNumber());
		pCCB->ClearCCBInfo();
		DeAllocCCB(pCCB->getTabIndex());
	}		
}

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
bool g_blLogRxSagSAbisSignal=false;
bool g_blLogTxSagSAbisSignal=false;
extern "C" void setLogRxSAbisSignalFlag(bool flag) 
{
	g_blLogRxSagSAbisSignal=flag;
}
extern "C" void setLogTxSAbisSignalFlag(bool flag) 
{
	g_blLogTxSagSAbisSignal=flag;
}
void ShowSignal(CSAbisSignal& signal)
{
	UINT16 i;
	SAbisSignalT* pData = (SAbisSignalT*)signal.GetDataPtr();
	UINT8* pU8;
#if 0
	//Tcp packet header
	//			pData->tcpPktHeader.HeadFlag,
	//			pData->tcpPktHeader.PktLen,
	//			pData->tcpPktHeader.DPC_Code,
	//			pData->tcpPktHeader.SPC_Code,
	//			pData->tcpPktHeader.UserType,
	//			pData->tcpPktHeader.TTL

	//Tcp packet end flag
	VPRINT("\n    Tcp pkt HeadFlag: ");
	VPRINT("%02x %02x",0x7e,0xa5);

	VPRINT("\n    TcpHeader: ");
	pU8 = (UINT8*)&pData->tcpPktHeader;
	for(i=0;i<sizeof(TcpPktHeaderT);i++)
	{
		if(0==i%2)
			VPRINT(" ");
		VPRINT(" %02x", pU8[i]);
	}
	
#endif
	//Sabis signal header
//			pData->sigHeader.SAG_ID
//			pData->sigHeader.BTS_ID
//			pData->sigHeader.EVENT_GROUP_ID
//			pData->sigHeader.Event_ID
//			pData->sigHeader.Length
	VPRINT("\n    SAbis Signal Header: ");
	pU8 = (UINT8*)&pData->sigHeader;
	for(i=0;i<sizeof(SigHeaderT);i++)
	{
		VPRINT(" %02x", pU8[i]);
	}

	//Sabis signal payload
	VPRINT("\n    SAbis Signal Payload: ");
	pU8 = (UINT8*)&pData->sigPayload;
	for(i=0;i<VGetU16BitVal(pData->sigHeader.Length);i++)
	{
		VPRINT(" %02x", pU8[i]);
	}
#if 0
	//Tcp packet end flag
	VPRINT("\n    Tcp pkt EndFlag: ");
	pU8 = ((UINT8*)&pData->sigPayload) + VGetU16BitVal(pData->sigHeader.Length);
	for(i=0;i<2;i++)
	{
		VPRINT(" %02x", pU8[i]);
	}
#endif
}
void CSAG::clearSignalCounters()
{
	for(int i=0;i<=InvalidSignal_MSG;i++)
		RxSignalCounter[i] = TxSignalCounter[i] = 0;
}
void CSAG::doTxSignalAcount(CSAbisSignal& signal)
{
	SignalType sigType = signal.ParseMessageFromSAG();
	++TxSignalCounter[sigType];
	if(InvalidSignal_MSG!=sigType)
	{
		VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		if(M_MSG_EVENT_GROUP_ID_UTSAG==pSigVCR->sigHeader.EVENT_GROUP_ID ||
			M_MSG_EVENT_GROUP_ID_CPEOAM==pSigVCR->sigHeader.EVENT_GROUP_ID)
		{
			sigType = signal.ParseUTSAGMsgFromSAG();
			++TxSignalCounter[sigType];
		}
	}	
}
void CSAG::doRxSignalAcount(CSAbisSignal& signal)
{
	SignalType sigType = signal.ParseMessageToSAG();
	++RxSignalCounter[sigType];
	if(InvalidSignal_MSG!=sigType)
	{
		VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		if(M_MSG_EVENT_GROUP_ID_UTSAG==pSigVCR->sigHeader.EVENT_GROUP_ID || 
			M_MSG_EVENT_GROUP_ID_CPEOAM==pSigVCR->sigHeader.EVENT_GROUP_ID)
		{
			sigType = signal.ParseUTSAGMsgToSAG();
			++RxSignalCounter[sigType];
		}
	}	
}
void CSAG::showSignalAccount()
{
	int i;
	VPRINT("\nLocalSag Signal Counters:");
	for(i=0;i<InvalidSignal_MSG;i++)
	{
		VPRINT("\n %30s : Rx[%10u] Tx[%10u]", CMsg_Signal_VCR::m_sigName[i],
					RxSignalCounter[i], TxSignalCounter[i]);
	}
	VPRINT("\n");
}

void CSAG::showPhoneBook(bool blDeatailFlag)
{
	VPRINT("\n----------Phone Book[%d]  Format[ telNO , UID , PID, CID, &CCB ]-------\n", m_Phone_Address_Book.size());
	map<string, CCCB*>::iterator itPhoneBook;
	if(blDeatailFlag)
	{
		for(itPhoneBook=m_Phone_Address_Book.begin();itPhoneBook!=m_Phone_Address_Book.end();itPhoneBook++)
		{
			CCCB* pCCB = (*itPhoneBook).second;
			if(pCCB!=NULL)
			{
				VPRINT("\n[%s] [0x%08X] [0x%08X] [0x%02X] [0x%08X]", 
					pCCB->getOwnNumber(), pCCB->getUID(), pCCB->getEID(), pCCB->getCID(), (UINT32)pCCB);
			}
			else
			{
				VPRINT("\n[%s] [0x%08X] [0x%08X] [0x%02X] [0x%08X] , error exist!!!", 
					(*itPhoneBook).first.c_str(), 0xffffffff, 0xffffffff, 0xff, NULL);
			}
		}	
	}
	VPRINT("\n-----------------------------------------------------------------------\n");	
	
}

void CSAG::showActiveCall(bool blDeatailFlag)
{
	VPRINT("\n----------Active Calls[%d]  Format[ telNO , UID , l3Addr, &CCB ]-------\n", m_L3Addr_index_Table.size());
	map<UINT32, CCCB*>::iterator itL3Addr;
	if(blDeatailFlag)
	{
		for(itL3Addr=m_L3Addr_index_Table.begin();itL3Addr!=m_L3Addr_index_Table.end();itL3Addr++)
		{
			CCCB* pCCB = (*itL3Addr).second;
			if(pCCB!=NULL)
			{
				VPRINT("\n[%s] [0x%08X] [0x%08X] [0x%08X]", 
					pCCB->getOwnNumber(), pCCB->getUID(), (*itL3Addr).first, (UINT32)(*itL3Addr).second);
			}
			else
			{
				VPRINT("\n[%s] [0x%08X] [0x%08X] [0x%08X] , error exist!!!", 
					"NULL", 0xffffffff, (*itL3Addr).first, (UINT32)(*itL3Addr).second);
			}
		}	
	}
	VPRINT("\n-----------------------------------------------------------------------\n");	
}

void CSAG::showUsers(bool blDeatailFlag)
{
	map<UINT32, CCCB*>::iterator itUID;
	
	VPRINT("\n====================================================================\n");
	VPRINT("\nCCB Pool Size[%d], Free[%d]\n", M_MAX_CCB_NUM, m_freeCCBList.size());
	VPRINT("\n----------UserList Size[%d]  Format[ UID , PhoneNumber, &CCB ]-------\n", m_Uid_index_Table.size());
	if(blDeatailFlag)
		for(itUID=m_Uid_index_Table.begin();itUID!=m_Uid_index_Table.end();itUID++)
		{
			CCCB* pCCB = (*itUID).second;
			if(pCCB!=NULL)
			{
				VPRINT(" [ 0x%08X , %s, 0x%08X ]", 
					(*itUID).first, pCCB->getOwnNumber(),
					(UINT32)(*itUID).second);
			}
			else
			{
				VPRINT(" [ 0x%08X , %s, 0x%08X ]", 
					(*itUID).first, "pCCB==NULL!!!",
					(UINT32)(*itUID).second);
			}
		}
	VPRINT("\n====================================================================\n");	
}

bool CSAG::sendSignalToLocalBTS(CSAbisSignal& signal, TID dstTid)
{
	bool ret;
	signal.SetMessageId(MSGID_VCR_VOICE_SIGNAL);
	//signal.SetSrcTid(M_TID_SAG);
	signal.SetSrcTid(M_TID_VCR);
	signal.SetDstTid(dstTid);
	VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();	
	if(pSigVCR)
	{
		VSetU32BitVal(pSigVCR->sigHeader.SAG_ID, 0xFFFFFFFF);
	}
	ret = signal.Post();
	if(!ret)
	{
		LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), "send msg to bts failed!!!");
	}
	else
	{
		doTxSignalAcount(signal);	
#if 1
		UINT8 signalType;
		if(signal.isUTSAGMsg())
		{
			signalType = signal.ParseUTSAGMsgFromSAG();
		}
		else
		{
			signalType = signal.ParseMessageFromSAG();
		}
		if(signalType!=BeatHeart_MSG && signalType!=BeatHeartAck_MSG)
		{
			LOG1(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL)," [%s] Signal -----> BTS", (int)CMsg_Signal_VCR::m_sigName[signalType]);
			if(g_blLogTxSagSAbisSignal)
			{
				ShowSignal(signal);
			}
		}
#endif		
	}
	return ret;
}

bool CSAG::sendInnerSignal(CMessage& signal)
{
	bool ret;
	signal.SetMessageId(MSGID_SAG_INNER_SIGNAL);
	signal.SetSrcTid(M_TID_SAG);
	signal.SetDstTid(M_TID_SAG);
	ret = signal.Post();
	if(!ret)
	{
		LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), "send msg to bts failed!!!");
	}
	return ret;
}

void CSAG::handleCpeTelNOReg(CMessage& msg)
{
	if(!g_blUseLocalUserInfoFile)
	{
		char szTelNO[30];
		memset(szTelNO, 0, sizeof(szTelNO));
		TelNOT* pData = (TelNOT*)msg.GetDataPtr();
		//取得UID和电话号码
		UINT32 uid = VGetU32BitVal(pData->uid);
		if(0<convertDigitNO2Str((void*)(&pData->telLen-1), szTelNO))
		{
			//本机号码在szTelNO中
		}
		else
		{
			//本机号码错误
			LOG1(SAG_LOG_UT_ACTION, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"CpeTelNOReg telephone number error!!! UID[0x%08X] ", uid);
		}
		CCCB *pCCB = FindCCBByUID(uid);
		if(pCCB!=NULL)
		{
			//如果电话号码信息不同
			if(strcmp(szTelNO, pCCB->getOwnNumber())!=0)
			{
				LOG1(SAG_LOG_UT_ACTION, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"UT Enable, update TELNO... UID[0x%08X] ", pCCB->getUID());
				//释放旧呼叫
				releaseCurCall(pCCB);
				//删除旧电话号码索引表条目
				DelPhoneAddressBook(pCCB->getOwnNumber());
				//更新电话号码
				pCCB->setOwnNumber(szTelNO);
				//增加新电话号码索引表条目
				if(szTelNO[0])
				{
					AddPhoneAddressBook(szTelNO, pCCB);
				}
				//EID&CID
				if(pCCB->getEID()!=msg.GetEID() || pCCB->getCID()!= pData->cid)
				{
					pCCB->setEID(msg.GetEID());
					pCCB->setCID(pData->cid);
				}
			}
		}
		else
		{
			pCCB = AllocCCB(uid);
			if(pCCB==NULL)
			{
				return;
			}
			//设置UID和电话号码
			pCCB->setUID(uid);
			pCCB->setOwnNumber(szTelNO);
			//更新各种索引表
			AddUIDIndexTable(uid, pCCB);
			if(szTelNO[0])
			{
				AddPhoneAddressBook(szTelNO, pCCB);
			}
			LOG2(SAG_LOG_UT_ACTION, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"UT Enable... UID[0x%08X] TELNO[%s] ",
				pCCB->getUID(), (int)pCCB->getOwnNumber());	
			
			pCCB->setEID(msg.GetEID());
			pCCB->setCID(pData->cid);
		}
	}
}

void CSAG::handleUtReg(CMessage& msg)
{
	//使用本地用户业务信息配置文件时处理此消息作为用户接入起点
	if(g_blUseLocalUserInfoFile)
	{
		CMsg_VoicePortReg regMsg(msg);				
		//取得UID和电话号码
		UINT32 uid = regMsg.GetUid();
		char szTelNO[30];
		memset(szTelNO, 0, sizeof(szTelNO));
		map< UINT32, userInfo, less<UINT32> >::iterator itFound;
		if(!findUserInfoByUID(uid, itFound))
		{
			LOG1(SAG_LOG_UT_ACTION, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"User Info not in UserInfo Table!!! UID[0x%08X] ", uid);
			return;
		}
		else
		{
			strcpy(szTelNO, (*itFound).second.telNO.c_str());
		}

		CCCB *pCCB = FindCCBByUID(uid);
		if(pCCB!=NULL)
		{
			//如果电话号码信息不同
			if(strcmp(szTelNO, pCCB->getOwnNumber())!=0)
			{
				LOG1(SAG_LOG_UT_ACTION, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"UT Enable, update TELNO... UID[0x%08X] ", pCCB->getUID());
				//释放旧呼叫
				releaseCurCall(pCCB);
				//删除旧电话号码索引表条目
				DelPhoneAddressBook(pCCB->getOwnNumber());
				//更新电话号码
				pCCB->setOwnNumber(szTelNO);
				//增加新电话号码索引表条目
				if(szTelNO[0])
				{
					AddPhoneAddressBook(szTelNO, pCCB);
				}
				//EID&CID
				if(pCCB->getEID()!=msg.GetEID() || pCCB->getCID()!= regMsg.GetCid())
				{
					pCCB->setEID(msg.GetEID());
					pCCB->setCID(regMsg.GetCid());
				}
			}
		}
		else
		{
			pCCB = AllocCCB(uid);
			if(pCCB==NULL)
			{
				return;
			}
			//设置UID和电话号码
			pCCB->setUID(uid);
			pCCB->setOwnNumber(szTelNO);
			//更新各种索引表
			AddUIDIndexTable(uid, pCCB);
			if(szTelNO[0])
			{
				AddPhoneAddressBook(szTelNO, pCCB);
			}
			LOG2(SAG_LOG_UT_ACTION, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"UT Enable... UID[0x%08X] TELNO[%s] ",
				pCCB->getUID(), (int)pCCB->getOwnNumber());	
			
			pCCB->setEID(msg.GetEID());
			pCCB->setCID(regMsg.GetCid());
		}		
	}
	else
	{
		//不使用基站本地业务配置文件时tvoice不转发这个消息给localSAG，依赖于终端上报电话号码的消息
		//不会运行到此
	}
}

void CSAG::handleUtUnReg(CMessage& msg)
{
	//取得UID
	CMsg_VoicePortUnReg unRegMsg(msg);
	UINT32 uid = unRegMsg.GetUid();
	//根据UID找到CCB
	CCCB* pCCB = FindCCBByUID(uid);
	//清除CCB
	if(pCCB!=NULL)
	{
		LOG2(SAG_LOG_UT_ACTION, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"UT Disable... UID[0x%08X] TELNO[%s] ",
			pCCB->getUID(), (int)pCCB->getOwnNumber());
		stopVoiceSrv(pCCB);
	}
}

void CSAG::handleInnerSignalCallArrive(CMessage& msg)
{
	UINT32 tmpUid = VGetU32BitVal(((InnerSignal_CallArriveT*)msg.GetDataPtr())->dstUID);
	CCCB* pCCB = FindCCBByUID(tmpUid);
	if(NULL!=pCCB)
	{
		if(SAG_IDLE_STATE==pCCB->getState())
		{
			injectFsm(msg);
		}
		else
		{
			//处理掉业务冲突
			pCCB->sendInnerSignalDisconnect2Peer(REL_CAUSE_PEER_BUSY);
			LOG2(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"UT not in IDLE when called ... UID[0x%08X] TELNO[%s] ",
			pCCB->getUID(), (int)pCCB->getOwnNumber());
		}			
	}
}

void CSAG::releaseVoiceSrv()
{
	LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"notify local SAG stopping voice service...");
	//遍历当前通话，拆除呼叫
	map<UINT32, CCCB*>::iterator itL3Addr;
	for(itL3Addr=m_L3Addr_index_Table.begin();itL3Addr!=m_L3Addr_index_Table.end();itL3Addr++)
	{
		CCCB* pCCB = (*itL3Addr).second;
		if(pCCB!=NULL)
		{
			releaseCurCall(pCCB);
			LOG2(SAG_LOG_UT_ACTION, LOGNO(SAG, EC_L3VOICE_NORMAL), "UID[0x%08X] l3Addr[0x%08X], release current call", 
				pCCB->getUID(), (*itL3Addr).first);
		}
		else
		{
			LOG1(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), " l3Addr[0x%08X], release current call , pCCB is NULL!!!!", 
				(*itL3Addr).first);
		}
	}
	//释放组呼
	while(m_Gid_index_Table.size())
	{
		map<UINT16,SxcGrpCCB*,less<UINT16> >::iterator itGid;
		itGid = m_Gid_index_Table.begin();
		SxcGrpCCB* pSxcGrpCCB = (*itGid).second;
		if(pSxcGrpCCB)
		{
			pSxcGrpCCB->releaseGrpSrv(REL_CAUSE_SAG);
		}
		else
		{
			//there are errors!!!
			m_Gid_index_Table.erase(itGid);
		}
	}
	m_Gid_index_Table.clear();
	m_GrpL3Addr_index_Table.clear();
}

void CSAG::parseAndHandleMsg(CMessage& msg)
{
	UINT16 msgID = msg.GetMessageId();
	switch(msgID)
	{
		case MSGID_VOICE_VDR_DATA:
			handleVoiceDataFromBTS(msg);
			break;
		case MSGID_VOICE_VCR_SIGNAL:
			parseAndHandleSignal(msg);
			break;
		case MSGID_TIMEOUT_LOCALSAG:
			parseAndHandleTimeoutMsg(msg);
			break;
		case MSGID_SAG_INNER_SIGNAL:
			parseAndHandleInnerSignal(msg);
			break;
		case MSGID_DAC_UL_CPE_TELNO_MSG:
			handleCpeTelNOReg(msg);
			break;
		case MSGID_VOICE_UT_REG:
			handleUtReg(msg);
			break;
		case MSGID_VOICE_UT_UNREG:
			//终端不在该bts
			handleUtUnReg(msg);
			break;
		case MSGID_BTS_SAG_STOP_SRV:
			releaseVoiceSrv();
			break;
		default:
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), "received unknown msg!!!");
			break;
	}
}

void CSAG::parseAndHandleInnerSignal(CMessage& msg)
{
	UINT16 innerSigType = getInnerSignalType(msg);
	if(innerSigType>InvalidSignal_MSG)
	{
		switch(innerSigType)
		{
			case SAG_INNER_CallArrive_Msg:
				handleInnerSignalCallArrive(msg);
				break;
			default:
				break;
		}
	}
	else
	{
		switch(innerSigType)
		{
		//single voice call	begin=====================
			case Alerting_MSG:
			case Connect_MSG:
			case Disconnect_MSG:
			case ConnectAck_MSG:
				//inject fsm
				injectFsm(msg);
				break;
			case Setup_MSG:
			case Information_MSG:
			case Release_MSG:
		//single voice call	end=====================		
		//group voice call begin=====================
			case PTT_Connect_MSG:
			case Group_CallingRls_MSG:
			case PTT_Interrupt_MSG:
				injectFsm(msg);
				break;
		//group voice call end=====================
			default:
				
				break;
		}
	}
	
}

void CSAG::parseAndHandleTimeoutMsg(CMessage& msg)
{
	PSagTimerStructT pData = (PSagTimerStructT)msg.GetDataPtr();
	UINT16 timerType = pData->type;
	if(timerType<TIMERID_SAG_COUNT)
	{
		if(NULL!=timeoutProc[timerType])
		{
			(this->*timeoutProc[timerType])(msg);
		}		
	}
	else
	{
		LOG3(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"timeout Msg, UID[0x%08X] L3Addr[0x%08X] timerType[0x%04X], invalid timerType!!!", 
				pData->uid, pData->l3Addr, pData->type);
	}
}

void CSAG::parseAndHandleSignal(CMessage& msg)
{
	CSAbisSignal signal(msg);
	doRxSignalAcount(signal);
	UINT8 signalType;
	if(signal.isUTSAGMsg())
	{
		signalType = signal.ParseUTSAGMsgToSAG();
	}
	else
	{
		signalType = signal.ParseMessageToSAG();
	}
#if 1
	if(signalType!=BeatHeart_MSG && signalType!=BeatHeartAck_MSG)
	{
		LOG1(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL)," [%s] Signal <----- BTS", (int)CMsg_Signal_VCR::m_sigName[signalType]);
		if(g_blLogRxSagSAbisSignal)
		{
			ShowSignal(signal);
		}
	}
#endif
	if(NULL!=signalProc[signalType])
	{
		(this->*signalProc[signalType])(signal);
	}
}

void CSAG::setAllHeadTailFields(UINT16 nPayloadLen, SignalType sigType, CSAbisSignal& signal)
{
	//signal.setSigTcpPKTHeaderTail(pSAG->getPC(), pBTS->getPC(), sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+nPayloadLen);
	//signal.SetBTSSAGID(htons(pBTS->getBTSID()), htonl(pSAG->getSAGID()));
	signal.SetSigIDS(sigType);
	signal.SetSigHeaderLengthField(nPayloadLen);
	signal.SetDataLength(sizeof(SigHeaderT)+nPayloadLen);
}

void CSAG::sendDisconnect(CCCB* pCCB, UINT8 RelCause)
{
	CSAbisSignal sabisSignal_Sag;
	if ( !sabisSignal_Sag.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(DisconnectT_CPE)-1+sizeof(UINT32)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		SAbisSignalT* pData_SAG = (SAbisSignalT*)sabisSignal_Sag.GetDataPtr();

		setAllHeadTailFields(sizeof(DisconnectT_CPE)-1+sizeof(UINT32), UTSAG_L3Addr_MSG, sabisSignal_Sag);

		VSetU32BitVal(pData_SAG->sigPayload.UTSAG_Payload_L3Addr.L3Addr, pCCB->getL3Addr());

		DisconnectT_CPE* pData_CPE = (DisconnectT_CPE*)(&pData_SAG->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pData_CPE->msgType = M_MSGTYPE_DISCONNECT;
		pData_CPE->RelCause = RelCause;
		
		sendSignalToLocalBTS(sabisSignal_Sag);
	}	
}
void CSAG::sendRelease(CCCB* pCCB, UINT8 RelCause)
{
	CSAbisSignal sabisSignal_Sag;
	if ( !sabisSignal_Sag.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(ReleaseT_CPE)-1+sizeof(UINT32)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		SAbisSignalT* pData_SAG = (SAbisSignalT*)sabisSignal_Sag.GetDataPtr();

		setAllHeadTailFields(sizeof(ReleaseT_CPE)-1+sizeof(UINT32), UTSAG_L3Addr_MSG, sabisSignal_Sag);

		VSetU32BitVal(pData_SAG->sigPayload.UTSAG_Payload_L3Addr.L3Addr, pCCB->getL3Addr());

		ReleaseT_CPE* pData_CPE = (ReleaseT_CPE*)(&pData_SAG->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
		pData_CPE->msgType = M_MSGTYPE_RELEASE;
		pData_CPE->RelCause = RelCause;

		sendSignalToLocalBTS(sabisSignal_Sag);
	}
}

void CSAG::sendRlsTransResReq(CCCB* pCCB, UINT8 RelCause)
{
	CSAbisSignal sabisSignal_Sag;
	if ( !sabisSignal_Sag.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(RlsResReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		SAbisSignalT* pData_SAG = (SAbisSignalT*)sabisSignal_Sag.GetDataPtr();

		setAllHeadTailFields(sizeof(RlsResReqT), RlsResReq_MSG, sabisSignal_Sag);

		VSetU32BitVal(pData_SAG->sigPayload.RlsResReq.L3addr, pCCB->getL3Addr());
		pData_SAG->sigPayload.RlsResReq.RlsCause = RelCause;

		sendSignalToLocalBTS(sabisSignal_Sag);
	}
}

void CSAG::setInnerSignalHead(CMessage& msg, UINT16 sigType)
{
	InnerSignalHeadT* pInnerHead = (InnerSignalHeadT*)((UINT8*)msg.GetDataPtr()-sizeof(InnerSignalHeadT));
	VSetU16BitVal(pInnerHead->sigType, sigType);
}

UINT16 CSAG::getInnerSignalType(CMessage& msg)
{
	InnerSignalHeadT* pInnerHead = (InnerSignalHeadT*)((UINT8*)msg.GetDataPtr()-sizeof(InnerSignalHeadT));
	return VGetU16BitVal(pInnerHead->sigType);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//all test functions bellow
#include "localSAGTask.h"
void showSagSignalCounters()
{
	CSAG* pSAG = CSAG::getSagInstance();
	if(pSAG!=NULL)
	{
		pSAG->showSignalAccount();
	}
}
void clearSagSignalCounters()
{
	CSAG* pSAG = CSAG::getSagInstance();
	if(pSAG!=NULL)
	{
		pSAG->clearSignalCounters();
	}
}
extern "C" void showSagUsers(bool blDeatailFlag=false)
{
	CSAG::getSagInstance()->showUsers(blDeatailFlag);
}

extern "C" void showPhoneBook(bool blDeatailFlag=false)
{
	CSAG::getSagInstance()->showPhoneBook(blDeatailFlag);
}

extern "C" void showActiveCall(bool blDeatailFlag=false)
{
	CSAG::getSagInstance()->showActiveCall(blDeatailFlag);
}

