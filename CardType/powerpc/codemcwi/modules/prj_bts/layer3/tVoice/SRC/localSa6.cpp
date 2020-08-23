/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSagFsmProc.cpp
* Create Date    : 14-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "voiceTone.h"
#include "cpe_signal_struct.h"
#include "localSagDialPlan.h"
#include "localSagFsmCfg.h"
#include "localSag.h"
#include "localSagCfg.h"
#include "localSagStruct.h"
#include "voiceToolFunc.h"

SAG_CPE_STATE IDLE__ULSignal__AssignResReq__Proc(CMessage& msg, CCCB& ccb)
{
	//切换应用不进入状态机,在handleXXX函数中处理
	//分配主叫L3Addr或讲话方L3Addr
	UINT32 L3Addr = CSAG::getSagInstance()->AllocateL3Addr();
	ccb.setL3Addr(L3Addr);
	CSAG::getSagInstance()->AddL3AddrIndexTable(L3Addr, &ccb);
	//回应成功的AssignResRsp
	ccb.sendDLSignalAssignResRsp(msg);
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)msg.GetDataPtr();
	UINT16 srvOpt = VGetU16BitVal(pSigBufRcv->sigPayload.AssignResReq.ServerOption);
	if(M_LOCALSAG_ASSIGNRES_REASON_GRP_TALKING==srvOpt)
	{
		//find GrpCCB
		SxcGrpCCB *pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
		if(NULL!=pGrpCCB)
		{
			//GrpCCB stopTimer TIMERID_SAG_GRP_ASSIGNRESREQ
			pGrpCCB->stopGrpTimer(TIMERID_SAG_GRP_ASSIGNRESREQ, &pGrpCCB->m_pTmGrpAssignResReq);
		}
		//组呼讲话方直接进入讲话状态
		return SAG_GRP_TALKING_STATE;
	}
	//启动定时器TOSetup,等待上行setup信令或PttSetup信令
	ccb.startTimer(TIMERID_SAG_O_SETUP);
	//返回SAG_O_SETUP_STATE
	return SAG_O_SETUP_STATE;
}
SAG_CPE_STATE IDLE__INSignal__Setup__Proc(CMessage& msg, CCCB& ccb)
{
	//保存呼叫相关信息对端ccb
	InnerSignal_CallArriveT* pData = (InnerSignal_CallArriveT*)msg.GetDataPtr();
	UINT32 peerUID = VGetU32BitVal(pData->srcUID);
	CCCB* pPeerCCB = CSAG::getSagInstance()->FindCCBByUID(peerUID);
	if(pPeerCCB==NULL)
	{
		LOG3(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"cannot find src CCB!!!InnerSignal CallArrive  , dstUID[0x%08X] srcUID[0x%08X] L3Addr[0x%08X] ", 
			VGetU32BitVal(pData->dstUID),
			VGetU32BitVal(pData->srcUID),
			VGetU32BitVal(pData->srcL3Addr));
		return SAG_IDLE_STATE;
	}
	//是否加密呼叫
	ccb.setEncryptCtrl(pPeerCCB->getEncryptCtrl());
	ccb.setPeerCCB(pPeerCCB);
	ccb.setOrigCall(false);
	//分配被叫L3Addr
	UINT32 L3Addr = CSAG::getSagInstance()->AllocateL3Addr();
	ccb.setL3Addr(L3Addr);
	CSAG::getSagInstance()->AddL3AddrIndexTable(L3Addr, &ccb);
	//发送寻呼给bts
	UINT16 appType;
	if(ENCRYPT_CTRL_NOTUSE==ccb.getEncryptCtrl())
	{
		appType = APPTYPE_VOICE_QCELP;
	}
	else
	{
		appType = APPTYPE_ENCRYPT_VOICE;
	}
	ccb.sendDLSignalLAPaging(appType);
	//启动寻呼超时定时器TlapagingRsp等待上行信令LaPagingRsp
	ccb.startTimer(TIMERID_SAG_T_LAPAGING);
	//返回SAG_PAGING_STATE;
	return SAG_PAGING_STATE;
}
SAG_CPE_STATE O_SETUP__ULSignal__PttSetupReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	SAbisSignalT *pSigData = (SAbisSignalT*)msg.GetDataPtr();
	PTTSetupT_CPE *pDataCpe = (PTTSetupT_CPE*)(&pSigData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
	UINT16 gid = VGetU16BitVal(pDataCpe->GID);
	ccb.setGID(gid);
	//向bts回应PttSetupAck
	ccb.sendDLSignalPttSetupAck(pDataCpe->EncryptFlag, pDataCpe->CallPriority);
	ccb.setEncryptCtrl(pDataCpe->EncryptFlag);
	ccb.setPrio(pDataCpe->CallPriority);
	SxcGrpCCB *pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(gid);
	//判断用户是否有权限建立组呼
	if(!CSAG::getSagInstance()->ifAllowGrpSetup(gid, ccb.getUID()))
	{
		ccb.sendDLSignalGrpCallingRls(REL_CAUSE_USRSRVNOTALLOW);
		ccb.sendDLSignalRlsTransResReq(REL_CAUSE_NORMAL);
		return SAG_IDLE_STATE;
	}
	//如果组存在
	if(pGrpCCB!=NULL)
	{
#ifdef M__SUPPORT__ENC_RYP_TION	
		//如果涉及到JM则拒绝此次组建立
		if(pDataCpe->EncryptFlag || pGrpCCB->getEncryptCtrl())
		{
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"refuse GrpCalling...");
			//向终端发送GrpCallingRls信令释放组呼主叫，原因为用户忙
			ccb.sendDLSignalGrpCallingRls(REL_CAUSE_PEER_BUSY);
			//向bts发送releaseResReq消息
			CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
			//返回状态SAG_IDLE_STATE
			return SAG_IDLE_STATE;
		}
#endif
		//如果组空闲
		if(!pGrpCCB->isSomeoneTalking())
		{
			//给予话权
			pGrpCCB->grantPttToUser(ccb.getUID());
			//向终端发送PttConnect信令
			ccb.sendDLSignalPttConnect(NOT_CALL_OWNER, pGrpCCB->getEncryptCtrl(), pGrpCCB->getPrio());
			//返回状态SAG_GRP_TALKING_STATE
			return SAG_GRP_TALKING_STATE;
		}
		//有讲话方
		else
		{
			//向终端发送GrpCallingRls信令释放组呼主叫，原因为用户忙
			ccb.sendDLSignalGrpCallingRls(REL_CAUSE_PEER_BUSY);
			//向bts发送releaseResReq消息
			CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
			//返回状态SAG_IDLE_STATE
			return SAG_IDLE_STATE;
		}
	}
	//组不存在
	else
	{
		//AllocGrpCCB
		pGrpCCB = CSAG::getSagInstance()->AllocGrpCCB(gid);
		if(NULL==pGrpCCB)
		{
			ccb.sendDLSignalGrpCallingRls(REL_CAUSE_UNKNOWN);
			ccb.sendDLSignalRlsTransResReq(REL_CAUSE_NORMAL);
			return SAG_IDLE_STATE;
		}
		//记录commType
		pGrpCCB->setCommType(pDataCpe->CommType);
		//记录是否加密
		pGrpCCB->setEncryptCtrl(pDataCpe->EncryptFlag);
#ifdef M__SUPPORT__ENC_RYP_TION
		//JMKEY
		if(pDataCpe->EncryptFlag)
		{
			pGrpCCB->setEncryptKey(pDataCpe->EncryptKey);
		}
#endif
		//记录优先级
		pGrpCCB->setPrio(pDataCpe->CallPriority);
		//MaybeFinishLater,也许以后这里会填写其他优先级
		//标记组呼建立者
		pGrpCCB->setGrpSetupUID(ccb.getUID());
		//向bts发送LAGrpPaging
		pGrpCCB->sendDLSignalLAGrpPaging();
		pGrpCCB->setPagingUserFlag(true);
		//启动定时器TIMERID_SAG_GRP_LAGRPPAGING
		pGrpCCB->startGrpTimer(TIMERID_SAG_GRP_LAGRPPAGING, &pGrpCCB->m_pTmGrpPaging);
		//启动定时器TIMERID_SAG_GRP_PRESSINFO
		pGrpCCB->startGrpTimer(TIMERID_SAG_GRP_PRESSINFO, &pGrpCCB->m_pTmGrpPressInfo);
		//启动定时器TIMERID_SAG_GRP_PTTCONNECT
		ccb.startTimer(TIMERID_SAG_GRP_PTTCONNECT);
		//返回状态SAG_GRP_CALLSETUP_STATE
		return SAG_GRP_CALLSETUP_STATE;
	}
}
SAG_CPE_STATE O_SETUP__ULSignal__Setup__Proc(CMessage& msg, CCCB& ccb)
{
	CCCB* pCCBCalled = NULL;
	//回应信令SetupAck给bts
	ccb.sendDLSignalSetupAck();
	//停止定时器TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	ccb.setOrigCall(true);
	//计费相关
	T_TimeDate dt = bspGetDateTime();
	ccb.m_cdr.setStartDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
	//如果setup信令中携带了被叫号码
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)msg.GetDataPtr();
	SetupUTT_CPE* pDataSetup = (SetupUTT_CPE*)(&pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
	//是否加密
	if(SETUPCAUSE_ENCRYPT_VOICE==pDataSetup->SetupCause)
	{
		ccb.setEncryptCtrl(ENCRYPT_VOICE);
	}
	else
	{
		ccb.setEncryptCtrl(ENCRYPT_CTRL_NOTUSE);
	}
	//setupCause
	ccb.setSetupCause(pDataSetup->SetupCause);
	UINT8* pDataBegin = (UINT8*)pSigBufRcv;
	bool blWithCalledNumber=false;
	if( (pDataSetup->Codeclist+pDataSetup->Codeclist[0]+1 ) < 
		(pDataBegin+msg.GetDataLength()) )
	{
		blWithCalledNumber = true;
	}
	if(blWithCalledNumber)
	{
		void *pNO=pDataSetup->Codeclist+pDataSetup->Codeclist[0]+1;
		//保存被叫号码
		ccb.setDialedNumberLen(convertDigitNO2Str(pNO, ccb.getDialedNumber()));
		bool blFound=false;
		UINT8 bFetId=0;
		UINT8 bAddDgts=0;
		UINT8 bParseResult = parseNumberDialed(ccb.getDialedNumberLen(), ccb.getDialedNumber(), bFetId, bAddDgts);
		//如果被叫号码符合拨号计划并且被叫在本sag(bts)，
		if(bParseResult==DIAL_COMPLETE)
		{
			//查找被叫是否在本bts
			char *pCalledNumber = ccb.getDialedNumber();
			char pCalledNumberWithAreaCode[M_MAX_PHONE_NUMBER_LEN*2];
			pCCBCalled = CSAG::getSagInstance()->FindCCBByLocalNumber(pCalledNumber);
			//没找到
			if(pCCBCalled==NULL)
			{
				 if(pCalledNumber[0]=='0')
				 {
				 	//被叫号码带区号，不用在重新找了
				 	//本地存储号码如果带区号则不用加区号再找(无线市话类编号)
				 	//如果本地存储号码不带区号，拨打被叫时也不应该加区号(移动手机类编号)
				 }
				 else
				 {
				 	//被叫号码不带区号，加上区号再找一次
				 	//本地存储号码如果带区号则加区号再找一次有可能找到(无线市话类编号)
				 	//如果本地存储号码不带区号，再找一次肯定找不到，浪费一点儿时间罢了(移动手机类编号)
				 	strcpy(pCalledNumberWithAreaCode, getAreaCode());
					//区号有效
					if(pCalledNumber[0]!=0)
					{
						strcat(pCalledNumberWithAreaCode, ccb.getDialedNumber());
						pCCBCalled = CSAG::getSagInstance()->FindCCBByLocalNumber(pCalledNumberWithAreaCode);
						if(pCCBCalled!=NULL)
						{
							blFound = true;
						}
					}
				 }
			}
			else
			{
				blFound = true;
			}
		}

		if(blFound)
		{
			//如果被叫不是SAG_IDLE_STATE
			if(SAG_IDLE_STATE!=pCCBCalled->getState())
			{
				//开始放忙音
				//向终端发送Disconnect信令
				ccb.sendDLSignalDisconnect(REL_CAUSE_PEER_BUSY);
				//启动等待用户挂机定时器TOnhook
				ccb.startTimer(TIMERID_SAG_DISCONNECT);
				//返回SAG_WAITONHOOK_STATE	
				return SAG_WAIT_ONHOOK_STATE;
			}
			else
			{
				//如果被叫空闲则发送内部信令CallArrive给被叫
				ccb.setPeerCCB(pCCBCalled);
				ccb.sendInnerSignalCallArrive(pCCBCalled->getUID());
				//启动定时器TOAlerting等待被叫回Alerting
				ccb.startTimer(TIMERID_SAG_O_ALERTING);
				//返回SAG_O_ALERTING_STATE
				return SAG_O_ALERTING_STATE;
			}
		}
		else
		{
			//如果被叫号码不符合拨号计划或被叫不在本sag(bts)，则开始放忙音，
			ccb.sendDLSignalDisconnect(REL_CAUSE_SAG);
			//启动等待用户挂机定时器TOnhook
			ccb.startTimer(TIMERID_SAG_DISCONNECT);
			//返回SAG_WAITONHOOK_STATE
			return SAG_WAIT_ONHOOK_STATE;
		}
	}
	else
	{
		//setup信令中未携带被叫号码
		//给终端放拨号音
		ccb.playTone(DIAL_TONE);
		ccb.setDialToneStoppedFlag(false);
		//启动拨号超时定时器TDial,
		ccb.startTimer(TIMERID_SAG_O_DIALNUMBER);
		//返回SAG_DIAL_STATE
		return SAG_DIAL_STATE;
	}	
}
SAG_CPE_STATE O_SETUP__Timeout__OSetup__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	//这种情况不放忙音直接通知bts拆除空口链路(对于组呼讲话方和组呼建立者暂不支持，支持需要增加状态)
	//向bts发送release transport resource req
	ccb.sendDLSignalRlsTransResReq(REL_CAUSE_TIMERTIMEOUT);
	//返回SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE O_SETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	//向bts回应errNotiRsp信令,进入状态机前已经自动回复了响应
	//返回SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE O_SETUP__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOSetup
	ccb.deleteTimer(TIMERID_SAG_O_SETUP);
	//向bts发送release transport resource req
	ccb.sendDLSignalRlsTransResReq(REL_CAUSE_SAG);
	//返回SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE DIAL__ULSignal__DialNumber__Proc(CMessage& msg, CCCB& ccb)
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)msg.GetDataPtr();
	InformationT_CPE* pDataCPE = (InformationT_CPE*)(&pSigBufRcv->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
	if(0==pDataCPE->type)//拨号消息
	{
		//停止定时器TDial
		ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
		//首次拨号停拨号音
		if(!ccb.isDialToneStopped())
		{
			ccb.stopTone(DIAL_TONE);
			ccb.setDialToneStoppedFlag(true);
		}
		//保存号码
		ccb.SaveDialedNumber(pDataCPE->digi_No.num & 0x0f);	
		CCCB* pCCBCalled = NULL;
		bool blFound=false;
		UINT8 bFetId=0;
		UINT8 bAddDgts=0;
		UINT8 bParseResult = parseNumberDialed(ccb.getDialedNumberLen(), ccb.getDialedNumber(), bFetId, bAddDgts);
		//如果被叫号码符合拨号计划并且被叫在本sag(bts)，
		if(bParseResult==DIAL_COMPLETE)
		{
			//查找被叫是否在本bts
			char *pCalledNumber = ccb.getDialedNumber();
			char pCalledNumberWithAreaCode[M_MAX_PHONE_NUMBER_LEN*2];
			pCCBCalled = CSAG::getSagInstance()->FindCCBByLocalNumber(pCalledNumber);
			//没找到
			if(pCCBCalled==NULL)
			{
				 if(pCalledNumber[0]=='0')
				 {
				 	//被叫号码带区号，不用在重新找了
				 	//本地存储号码如果带区号则不用加区号再找(无线市话类编号)
				 	//如果本地存储号码不带区号，拨打被叫时也不应该加区号(移动手机类编号)
				 }
				 else
				 {
				 	//被叫号码不带区号，加上区号再找一次
				 	//本地存储号码如果带区号则加区号再找一次有可能找到(无线市话类编号)
				 	//如果本地存储号码不带区号，再找一次肯定找不到，浪费一点儿时间罢了(移动手机类编号)
				 	strcpy(pCalledNumberWithAreaCode, getAreaCode());
					//区号有效
					if(pCalledNumber[0]!=0)
					{
						strcat(pCalledNumberWithAreaCode, ccb.getDialedNumber());
						pCCBCalled = CSAG::getSagInstance()->FindCCBByLocalNumber(pCalledNumberWithAreaCode);
						if(pCCBCalled!=NULL)
						{
							blFound = true;
						}
					}
				 }
			}
			else
			{
				blFound = true;
			}
		}
		else
		{
			if(bParseResult==DIAL_CONTINUE ||
				bParseResult==DIAL_VALID)
			{
				//如果可以继续拨号
				//启动定时器TDial
				ccb.startTimer(TIMERID_SAG_O_DIALNUMBER);
				//返回状态SAG_DIAL_STATE
				return SAG_DIAL_STATE;
			}
		}

		if(blFound)
		{
			//如果被叫不是SAG_IDLE_STATE
			if(SAG_IDLE_STATE!=pCCBCalled->getState())
			{
				//开始放忙音
				//向终端发送Disconnect信令
				ccb.sendDLSignalDisconnect(REL_CAUSE_PEER_BUSY);
				//启动等待用户挂机定时器TOnhook
				ccb.startTimer(TIMERID_SAG_DISCONNECT);
				//返回SAG_WAITONHOOK_STATE	
				return SAG_WAIT_ONHOOK_STATE;
			}
			else
			{
				//如果被叫空闲则发送内部信令CallArrive给被叫
				ccb.setPeerCCB(pCCBCalled);
				ccb.sendInnerSignalCallArrive(pCCBCalled->getUID());
				//启动定时器TOAlerting等待被叫回Alerting
				ccb.startTimer(TIMERID_SAG_O_ALERTING);
				//返回SAG_O_ALERTING_STATE
				return SAG_O_ALERTING_STATE;
			}
		}
		else
		{
			//如果被叫号码不符合拨号计划或被叫不在本sag(bts)，则开始放忙音，
			ccb.sendDLSignalDisconnect(REL_CAUSE_SAG);
			//启动等待用户挂机定时器TOnhook
			ccb.startTimer(TIMERID_SAG_DISCONNECT);
			//返回SAG_WAITONHOOK_STATE
			return SAG_WAIT_ONHOOK_STATE;
		}		

	}
	else
	{
		//拍插簧,不处理
		return SAG_DIAL_STATE;
	}	
}
SAG_CPE_STATE DIAL__Timeout__DialNumber__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TDial
	ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
	//放忙音
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//发送Disconnect信令给终端
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE DIAL__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TDial
	ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
	//向用户发送release信令
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE DIAL__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TDial
	ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
	//回应信令errNotiRsp
	//返回SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE DIAL__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TDial
	ccb.deleteTimer(TIMERID_SAG_O_DIALNUMBER);
	//开始放忙音
	//向终端发送Disconnect信令
	ccb.sendDLSignalDisconnect(ccb.getDisconnectReason(msg));
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//返回SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE O_ALERTING__INSignal__Alerting__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//启动定时器TOConnect
	ccb.startTimer(TIMERID_SAG_O_CONNECT);
	//向终端发送Alerting
	ccb.sendDLSignalAlerting();
	//开始放回铃音
	ccb.playTone(RINGBACK_TONE);
	//返回状态SAG_RINGBACK_STATE
	return SAG_RINGBACK_STATE;
}
SAG_CPE_STATE O_ALERTING__Timeout__Alerting__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//开始放忙音
	//向终端发送Disconnect
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//向被叫终端发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回SAG_WAITONHOOK_STATE	
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE O_ALERTING__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//向终端发送release信令
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//向被叫终端发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_NORMAL);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE O_ALERTING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//回应信令errNotiRsp
	//向被叫终端发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE O_ALERTING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOAlerting
	ccb.deleteTimer(TIMERID_SAG_O_ALERTING);
	//开始放忙音
	//向终端发送Disconnect信令
	ccb.sendDLSignalDisconnect(ccb.getDisconnectReason(msg));
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//返回SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE RINGBACK__INSignal__Connect__Proc(CMessage& msg, CCCB& ccb)
{
	//停定时器TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//向终端发送信令Connect
	ccb.sendDLSignalConnect();
	//启动定时器TOConnectAck
	ccb.startTimer(TIMERID_SAG_O_CONNECTACK);
	//计费相关操作
	T_TimeDate dt = bspGetDateTime();
	ccb.m_cdr.setConnectDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);	
	//返回状态SAG_CONNECT_STATE
	return SAG_CONNECT_STATE;
}
SAG_CPE_STATE RINGBACK__Timeout__Connect__Proc(CMessage& msg, CCCB& ccb)
{
	//停定时器TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//开始放忙音
	//向终端发送信令Disconnect
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//向被叫终端发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE RINGBACK__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停定时器TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//向终端发送信令Release
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//向被叫终端发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_NORMAL);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RINGBACK__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停定时器TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//回应信令errNotiRsp
	//向被叫终端发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE RINGBACK__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停定时器TOConnect
	ccb.deleteTimer(TIMERID_SAG_O_CONNECT);
	//开始放忙音
	//向终端发送Disconnect
	ccb.sendDLSignalDisconnect(ccb.getDisconnectReason(msg));
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//返回SAG_WAITONHOOK_STATE	
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE PAGING__ULSignal__LapagingRsp__Proc(CMessage& msg, CCCB& ccb)
{
	//停定时器TlapagingRsp
	ccb.deleteTimer(TIMERID_SAG_T_LAPAGING);
	SAbisSignalT* pData = (SAbisSignalT*)msg.GetDataPtr();
	if(M_LAPAGING_CAUSE_SUCCESS==pData->sigPayload.LAPagingRsp.Cause)
	{
		//启动定时器TassignResReq等待被叫申请资源
		ccb.startTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
		//返回状态SAG_T_ASSIGNRES_STATE
		return SAG_T_ASSIGNRES_STATE;
	}
	else
	{
		//向被叫终端发送Disconnect内部信令,让对端开始释放呼叫
		ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_PEER_BUSY);
		//返回状态SAG_IDLE_STATE
		return SAG_IDLE_STATE;
	}
}
SAG_CPE_STATE PAGING__Timeout__LapagingRsp__Proc(CMessage& msg, CCCB& ccb)
{
	//停定时器TlapagingRsp
	ccb.deleteTimer(TIMERID_SAG_T_LAPAGING);
	//向对端终端发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE PAGING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停定时器TlapagingRsp
	ccb.deleteTimer(TIMERID_SAG_T_LAPAGING);
	//向bts发送DeLaPaging信令
	ccb.sendDLSignalDeLaPaging();
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_ASSIGNRES__ULSignal__AssignResReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TassignResReq
	ccb.deleteTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
	//向bts回应AssignResRsp消息
	ccb.sendDLSignalAssignResRsp(msg);
	//向被叫UT发送Setup信令
	ccb.sendDLSignalSetup();
	//启动TTSetupAck定时器等待SetupAck信令
	ccb.startTimer(TIMERID_SAG_T_SETUPACK);
	//返回状态SAG_T_SETUP_STATE
	return SAG_T_SETUP_STATE;
}
SAG_CPE_STATE T_ASSIGNRES__Timeout__AssignResReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TassignResReq
	ccb.deleteTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
	//发送ErrNotiReq给bts
	ccb.sendDLSignalErrNotiReq();
	//向主叫发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_ASSIGNRES__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TassignResReq
	ccb.deleteTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
	//发送ErrNotiRsp给bts
	//向主叫发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_ASSIGNRES__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TassignResReq
	ccb.deleteTimer(TIMERID_SAG_T_ASSIGN_TRANS_RES);
	//发送ErrNotiReq给bts
	ccb.sendDLSignalErrNotiReq();
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_SETUP__ULSignal__SetupAck__Proc(CMessage& msg, CCCB& ccb)
{
	//停止TTSetupAck定时器
	ccb.deleteTimer(TIMERID_SAG_T_SETUPACK);
	//启动定时器TTAlerting等待信令Alerting
	ccb.startTimer(TIMERID_SAG_T_ALERTING);
	//返回状态SAG_T_ALERTING_STATE
	return SAG_T_ALERTING_STATE;
}
SAG_CPE_STATE T_SETUP__Timeout__SetupAck__Proc(CMessage& msg, CCCB& ccb)
{
	//停止TTSetupAck定时器
	ccb.deleteTimer(TIMERID_SAG_T_SETUPACK);
	//向终端发送Release信令
	ccb.sendDLSignalRelease(REL_CAUSE_TIMERTIMEOUT);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//向主叫发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE T_SETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止TTSetupAck定时器
	ccb.deleteTimer(TIMERID_SAG_T_SETUPACK);
	//回应errNotiRsp给bts
	//向主叫发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_SETUP__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止TTSetupAck定时器
	ccb.deleteTimer(TIMERID_SAG_T_SETUPACK);
	//向终端发送Release信令
	ccb.sendDLSignalRelease(ccb.getDisconnectReason(msg));
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE T_ALERTING__ULSignal__Alerting__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTAlerting
	ccb.deleteTimer(TIMERID_SAG_T_ALERTING);
	//向主叫发送内部信令Alerting
	ccb.sendInnerSignalAlerting();
	//启动定时器TTConnect，等待被叫摘机
	ccb.startTimer(TIMERID_SAG_T_CONNECT);
	//返回状态SAG_RING_STATE
	return SAG_RING_STATE;
}
SAG_CPE_STATE T_ALERTING__Timeout__Alerting__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTAlerting
	ccb.deleteTimer(TIMERID_SAG_T_ALERTING);
	//向终端发送Release信令
	ccb.sendDLSignalRelease(REL_CAUSE_TIMERTIMEOUT);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//向主叫发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE T_ALERTING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTAlerting
	ccb.deleteTimer(TIMERID_SAG_T_ALERTING);
	//回应errNotiRsp给bts
	//向主叫发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE T_ALERTING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTAlerting
	ccb.deleteTimer(TIMERID_SAG_T_ALERTING);
	//向终端发送Release信令
	ccb.sendDLSignalRelease(ccb.getDisconnectReason(msg));
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RING__ULSignal__Connect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//向主叫发送内部信令Connect
	ccb.sendInnerSignalConnect();
	//向bts发送内部交换语音信令
	ccb.sendDLSignalDVoiceReq(ENABLE_BTS_VDATA_INNER_SWITCH);
	//启动定时器TTConnectAck，等待主叫回应connectAck
	ccb.startTimer(TIMERID_SAG_T_CONNECTACK);
	//返回状态SAG_CONNECT_STATE
	return SAG_CONNECT_STATE;
}
SAG_CPE_STATE RING__Timeout__Connect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//向终端发送Release信令
	ccb.sendDLSignalRelease(REL_CAUSE_TIMERTIMEOUT);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//向主叫发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//回应errNotiRsp给bts
	//向主叫发送Disconnect内部信令,让对端开始释放呼叫
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE RING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//向终端发送Release信令
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RING__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTConnect
	ccb.deleteTimer(TIMERID_SAG_T_CONNECT);
	//向主叫发送disconnect	
	ccb.sendInnerSignalDisconnect2Peer(ccb.getDisconnectReason(msg));
	//向终端发送Release信令
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE CONNECT__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOConnectAck/TTConnectAck
	if(ccb.isOrigCall())
	{
		ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	}
	else
	{
		ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	}
	//向终端发送Release信令
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//向对端终端发送Disconnect内部信令，让对端开始释放
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_NORMAL);
	//返回SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE CONNECT__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOConnectAck/TTConnectAck
	if(ccb.isOrigCall())
	{
		ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	}
	else
	{
		ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	}
	//回应errNotiRsp给bts
	//向对端终端发送Disconnect内部信令，让对端开始释放
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_AIRLINKFAIL);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE CONNECT__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOConnectAck/TTConnectAck
	if(ccb.isOrigCall())
	{
		ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	}
	else
	{
		ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	}
	//开始放忙音
	//向终端发送Disconnect信令
	ccb.sendDLSignalDisconnect(ccb.getDisconnectReason(msg));
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//返回状态SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE CONNECT__INSignal__TConnectAck__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTConnectAck
	ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	//发送信令ConnectAck给被叫终端
	ccb.sendDLSignalConnectAck();
	//返回状态SAG_CONNECT_STATE
	return SAG_CONNECT_STATE;
}
SAG_CPE_STATE CONNECT__Timeout__TConnectAck__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TTConnectAck
	ccb.deleteTimer(TIMERID_SAG_T_CONNECTACK);
	//开始放忙音
	//向被叫终端发送Disconnect信令
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//向主叫终端发送Disconnect内部信令，让对端开始释放
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE CONNECT__ULSignal__OConnectAck__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOConnectAck
	ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	//发送内部信令ConnectAck给被叫
	ccb.sendInnerSignalConnectAck();
	//返回状态SAG_CONNECT_STATE
	return SAG_CONNECT_STATE;
}
SAG_CPE_STATE CONNECT__Timeout__OConnectAck__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOConnectAck
	ccb.deleteTimer(TIMERID_SAG_O_CONNECTACK);
	//开始放忙音
	//向主叫终端发送Disconnect信令
	ccb.sendDLSignalDisconnect(REL_CAUSE_TIMERTIMEOUT);
	//启动等待用户挂机定时器TOnhook
	ccb.startTimer(TIMERID_SAG_DISCONNECT);
	//向被叫终端发送Disconnect内部信令，让对端开始释放
	ccb.sendInnerSignalDisconnect2Peer(REL_CAUSE_TIMERTIMEOUT);
	//返回状态SAG_WAITONHOOK_STATE
	return SAG_WAIT_ONHOOK_STATE;
}
SAG_CPE_STATE WAITONHOOK__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOnhook
	ccb.deleteTimer(TIMERID_SAG_DISCONNECT);
	//停止放忙音
	//向终端发送信令Release
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE WAITONHOOK__Timeout__Disconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOnhook
	ccb.deleteTimer(TIMERID_SAG_DISCONNECT);
	//停止放忙音
	//向终端发送信令Release
	ccb.sendDLSignalRelease(REL_CAUSE_NORMAL);
	//启动定时器Treleasecomplete
	ccb.startTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//返回状态SAG_RELEASE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE WAITONHOOK__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TOnhook
	ccb.deleteTimer(TIMERID_SAG_DISCONNECT);
	//回应errNotiRsp给bts
	//停止放忙音
	//返回状态SAG_IDLE_STATE
	return SAG_RELEASE_STATE;
}
SAG_CPE_STATE RELEASE__ULSignal__ReleaseComplete__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器Treleasecomplete
	ccb.deleteTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//发送releaseTransResReq给bts释放资源
	ccb.sendDLSignalRlsTransResReq(REL_CAUSE_NORMAL);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE RELEASE__Timeout__ReleaseComplete__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器Treleasecomplete
	ccb.deleteTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//发送releaseTransResReq给bts释放资源
	ccb.sendDLSignalRlsTransResReq(REL_CAUSE_NORMAL);
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE RELEASE__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器Treleasecomplete
	ccb.deleteTimer(TIMERID_SAG_RELEASE_COMPLETE);
	//回应errNotiRsp给bts
	//返回状态SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__ULSignal__GrpDisconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//releseGrpSrvByGrpMaker
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	pGrpCCB->releseGrpSrvByGrpMaker(ccb.getUID(), REL_CAUSE_NORMAL);
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//releseGrpSrvByGrpMaker
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	pGrpCCB->releseGrpSrvByGrpMaker(ccb.getUID(), REL_CAUSE_AIRLINKFAIL);
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__Timeout__PttConnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//releseGrpSrvByGrpMaker
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	pGrpCCB->releseGrpSrvByGrpMaker(ccb.getUID(), REL_CAUSE_TIMERTIMEOUT);
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__INSignal__PttConnect__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//向bts发送PTT_Connect_MSG
	ccb.sendDLSignalPttConnect(IS_CALL_OWNER, ccb.getEncryptCtrl(), ccb.getPrio());
	//返回SAG_GRP_TALKING_STATE
	return SAG_GRP_TALKING_STATE;
}
SAG_CPE_STATE GRP_CALLSETUP__INSignal__GrpCallingRelease__Proc(CMessage& msg, CCCB& ccb)
{
	//停止定时器TIMERID_SAG_GRP_PTTCONNECT
	ccb.deleteTimer(TIMERID_SAG_GRP_PTTCONNECT);
	//向终端发送GrpCallingRelease
	ccb.sendDLSignalGrpCallingRls(ccb.getGrpCallingRlsReason(msg));
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__ULSignal__GrpDisconnect__Proc(CMessage& msg, CCCB& ccb)
{
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	//话权调度
	if(pGrpCCB!=NULL)
	{
		pGrpCCB->dispatchTalkersInGrpQueue();
	}
	//返回SAG_IDLE_STATE	
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__ULSignal__PttRls__Proc(CMessage& msg, CCCB& ccb)
{
	//向终端回应PttRlsAck
	ccb.sendDLSignalPttRlsAck();
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//话权调度
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	if(pGrpCCB!=NULL)
	{
		pGrpCCB->dispatchTalkersInGrpQueue();
	}
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb)
{
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//话权调度
	SxcGrpCCB* pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(ccb.getGID());
	if(pGrpCCB!=NULL)
	{
		pGrpCCB->dispatchTalkersInGrpQueue();
	}
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__INSignal__GrpCallingRelease__Proc(CMessage& msg, CCCB& ccb)
{
	//向终端发送GrpCallingRelease
	ccb.sendDLSignalGrpCallingRls(ccb.getGrpCallingRlsReason(msg));
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}
SAG_CPE_STATE GRP_TALKING__INSignal__PttInterrupt__Proc(CMessage& msg, CCCB& ccb)
{
	//向终端发送PttInterrupt
	ccb.sendDLSignalPttInterrupt(ccb.getPttInterruptReason(msg));
	//向bts发送releaseResReq
	CSAG::getSagInstance()->sendRlsTransResReq(&ccb, REL_CAUSE_NORMAL);
	//返回SAG_IDLE_STATE
	return SAG_IDLE_STATE;
}


