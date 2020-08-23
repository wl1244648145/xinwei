/*******************************************************************************
* Copyright (c) 2009 by Beijing Ap Co.Ltd.All Rights Reserved   
* File Name      : localSagFsm.cpp
* Create Date    : 10-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/

#include "localSagFsm.h"
#include "localSagFsmCfg.h"
#include "localSag.h"
#include "localSagStruct.h"
#include "localSagTimer.h"
#include "localSagMsgID.h"
#include "callSignalMsg.h"
#include "voiceToolFunc.h"
#include "BtsVMsgId.h"

CCCB* findCCB4ULSignal(CMessage& msg)
{
	enum{ use_UID, use_L3Addr, use_nothing };
	CCCB* pCCB = NULL;
	UINT16 sigType;
	UINT8 findType = use_nothing;
	UINT32 tmpUid, tmpL3Addr;
	CSAbisSignal signal(msg);
	SAbisSignalT* pDataSignal = (SAbisSignalT*)msg.GetDataPtr();
	sigType = signal.ParseMessageToSAG();
	switch(sigType)
	{
	//个呼相关
		case LAPagingRsp_MSG:
			findType = use_UID;
			tmpUid = VGetU32BitVal(pDataSignal->sigPayload.LAPagingRsp.UID);
			break;
		case AssignResReq_MSG:
			findType = use_UID;
			tmpUid = VGetU32BitVal(pDataSignal->sigPayload.AssignResReq.UID);
			break;
		case UTSAG_L3Addr_MSG:
			findType = use_L3Addr;
			tmpL3Addr = VGetU32BitVal(pDataSignal->sigPayload.UTSAG_Payload_L3Addr.L3Addr);
			break;
		case ErrNotifyReq_MSG:
			findType = use_UID;
			tmpUid = VGetU32BitVal(pDataSignal->sigPayload.ErrNotifyReq.Uid);
			break;
			
	//其他透传消息，暂不处理	
		case UTSAG_UID_MSG:
			break;
		case UTSAG_UID_OAM_MSG:
			break;
		case UTSXC_GRPL3Addr_MSG:
			sigType = signal.ParseUTSAGMsgToSAG();
			if(Grp_Disconnect_MSG==sigType)
			{
				findType = use_UID;
				tmpUid = VGetU32BitVal(pDataSignal->sigPayload.UTSAG_Signal_XXX.UTSAGPayload);
			}
			break;
		case UTSXC_GRPUID_MSG:
		case Auth_Info_Req_MSG:
			break;
		default:
			break;
	}
	switch(findType)
	{
		case use_UID:
			pCCB=CSAG::getSagInstance()->FindCCBByUID(tmpUid);
			break;
		case use_L3Addr:
			pCCB=CSAG::getSagInstance()->FindCCBByL3Addr(tmpL3Addr);
			break;
		default:
			LOG1(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"findCCB4ULSignal, don't know how to find CCB for signal[%s]!!!", 
				(int)CMsg_Signal_VCR::m_sigName[sigType]);
			break;
	}	
	return pCCB;
}

CCCB* findCCB4InnerSignal(CMessage& msg)
{
	enum{ use_UID, use_L3Addr, use_nothing };
	CCCB* pCCB = NULL;
	UINT8 findType = use_nothing;
	UINT32 tmpUid, tmpL3Addr;
	UINT16 innerSigType = CSAG::getSagInstance()->getInnerSignalType(msg);
	if(innerSigType>InvalidSignal_MSG)
	{
		switch(innerSigType)
		{
			case SAG_INNER_CallArrive_Msg:
				findType = use_UID;
				tmpUid = VGetU32BitVal(((InnerSignal_CallArriveT*)msg.GetDataPtr())->dstUID);
				break;
			default:
				break;
		}
	}
	else
	{
		InnerSignal_GrpMsgHeadT* pDataGrpSignal = (InnerSignal_GrpMsgHeadT*)msg.GetDataPtr();
		SAbisSignalT* pDataSignal = (SAbisSignalT*)msg.GetDataPtr();
		switch(innerSigType)
		{
		//个呼相关begin----------------
			case CallProc_MSG:
			case Alerting_MSG:
			case Connect_MSG:
			case ConnectAck_MSG:
			case Disconnect_MSG:
				findType = use_L3Addr;
				tmpL3Addr = VGetU32BitVal(pDataSignal->sigPayload.UTSAG_Payload_L3Addr.L3Addr);
				break;
		//个呼相关end------------------
		//集群讲话方相关begin---------------------------
			case Group_CallingRls_MSG:
			case PTT_Connect_MSG:
			case PTT_Interrupt_MSG:
				findType = use_UID;
				tmpUid = VGetU32BitVal(pDataGrpSignal->UID);
				break;
		//集群讲话方相关end-----------------------------
		//其他透传消息，暂不处理	
			case UTSAG_UID_MSG:
				break;
			case UTSAG_UID_OAM_MSG:
				break;
			case UTSXC_GRPL3Addr_MSG:
			case UTSXC_GRPUID_MSG:
			case Auth_Info_Req_MSG:
				break;
			default:
				break;
		}
	}
	switch(findType)
	{
		case use_L3Addr:
			pCCB=CSAG::getSagInstance()->FindCCBByL3Addr(tmpL3Addr);
			break;
		case use_UID:
			pCCB=CSAG::getSagInstance()->FindCCBByUID(tmpUid);
			break;
		default:
			LOG1(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"findCCB4InnerSignal, don't know how to find CCB for signal[0x%04X]!!!", 
				innerSigType);
			break;
	}	
	return pCCB;	
}

CCCB* findCCB(CMessage& msg)
{
	CCCB* pCCB = NULL;
	UINT16 msgID = msg.GetMessageId();
	PSagTimerStructT pDataTimer;
	switch(msgID)
	{
		case MSGID_VOICE_VCR_SIGNAL:
			pCCB = findCCB4ULSignal(msg);
			break;
		case MSGID_TIMEOUT_LOCALSAG:
			pDataTimer = (PSagTimerStructT)msg.GetDataPtr();
			pCCB = CSAG::getSagInstance()->FindCCBByUID(pDataTimer->uid);
			break;
		case MSGID_SAG_INNER_SIGNAL:
			pCCB = findCCB4InnerSignal(msg);
		default:
			break;
	}
	return pCCB;
}

UINT16 parseEvent(CMessage& msg, UINT16 state)
{
	UINT16 i;
	CSAbisSignal signal(msg);
	PSagTimerStructT pDataTimer=NULL;
	UINT16 ret = SAG_CALL_FSM_END;
	bool blIsUTSAGMsg = false;
	UINT16 sigType=InvalidSignal_MSG;
	UINT16 timerType=M_INVALID_SAG_TIMERTYPE;
	UINT16 msgID = msg.GetMessageId();

	switch(msgID)
	{
		case MSGID_VOICE_VCR_SIGNAL:
			blIsUTSAGMsg = signal.isUTSAGMsg();
			if(blIsUTSAGMsg)
			{
				sigType = signal.ParseUTSAGMsgToSAG();
			}
			else
			{
				sigType = signal.ParseMessageToSAG();
			}		
			break;
		case MSGID_SAG_INNER_SIGNAL:
			sigType = CSAG::getSagInstance()->getInnerSignalType(msg);
			break;
		case MSGID_TIMEOUT_LOCALSAG:
			pDataTimer = (PSagTimerStructT)msg.GetDataPtr();
			timerType = pDataTimer->type;	
			break;
		
		default:
			//not use now
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), "inject msg not using in fsm!!!");
			break;
	}
	
	
	if(state<SAG_STATE_COUNT)
	{
		EventActionItemT* pEventActionCurState = (EventActionItemT*)pEventAction[state];
		for(i=0;pEventActionCurState[i].TransIndex!=SAG_CALL_FSM_END;i++)
		{
			if(msgID==pEventActionCurState[i].messageID)
			{
				switch(msgID)
				{
					case MSGID_VOICE_VCR_SIGNAL:
						if(sigType==pEventActionCurState[i].signalType)
						{
							return pEventActionCurState[i].TransIndex;
						}
						break;
					case MSGID_SAG_INNER_SIGNAL:
						if(sigType==pEventActionCurState[i].signalType)
						{
							return pEventActionCurState[i].TransIndex;
						}
						break;
					case MSGID_TIMEOUT_LOCALSAG:
						if(timerType==pEventActionCurState[i].timerType)
						{
							return pEventActionCurState[i].TransIndex;
						}
						break;
					
					default:
						break;
				}
			}
		}
	}
	return ret;
}

void injectFsm(CMessage& msg)
{
	UINT8 targetState, currentState;
	CCCB* pCCB = findCCB(msg);
	if(pCCB==NULL)
		return;
	int trans = parseEvent(msg, pCCB->getState());
	if(trans!=SAG_CALL_FSM_END)
	{
		currentState = pCCB->getState();
		LOG2(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			" CCB[UID=0x%08X] %s", pCCB->getUID(), (int)callFsmFuncName[trans]);

		targetState = (*pCallFsmFunc[trans])(msg, *pCCB);
		if(targetState!=currentState)
		{
			pCCB->exitState();
			LOG2(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				" CCB[UID=0x%08X] exit state[%s]", pCCB->getUID(), (int)callFsmStateName[currentState]);
			pCCB->setState(targetState);
			pCCB->enterState();
			LOG2(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				" CCB[UID=0x%08X] enter state[%s]", pCCB->getUID(), (int)callFsmStateName[targetState]);
		}
	}
}


