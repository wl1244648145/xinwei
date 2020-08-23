/*******************************************************************************
* Copyright (c) 2010 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagGrpSrv.cpp
* Create Date    : 6-Jan-2010
* programmer     :fengbing
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "localSag.h"
#include "localSagGrpCCB.h"
#include "localSagFsmCfg.h"
#include "localSagCfg.h"
#include "voiceData.h"
#include "Vac_voice_data.h"
#include "VDR_voice_data.h"
#include "voiceFsm.h"
#include "voiceToolFunc.h"
#include "tVoice.h"
#include "BtsVMsgId.h"
#include "cpe_signal_struct.h"

UINT32 CSAG::m_GrpTransIDRes=0;
UINT8 CSAG::AllocateGrpTransID()
{
	return (m_GrpTransIDRes++ & 0x1F );//between 0--31
}
SxcGrpCCB* CSAG::AllocGrpCCB(UINT16 gid)
{
	UINT16 id;
	SxcGrpCCB* ret = NULL;
#if 0	
	ret = FindGrpCCBByGID(gid);
	if(ret==NULL)
#endif		
	{
		if(!m_freeGrpCCBList.empty())
		{
			id = *(m_freeGrpCCBList.begin());
			ret = &m_GrpCCBTable[id];
			m_freeGrpCCBList.pop_front();
			ret->cleanGrpSrvInfo();
			ret->setGID(gid);
			//无移动性管理这里默认当前组大小10
			ret->setGrpSize(10);
			ret->setTransID(AllocateGrpTransID());
			UINT32 grpL3Addr = AllocateGrpL3Addr();
			ret->setGrpL3Addr(grpL3Addr);
			AddGIDIndexTable(gid, ret);
			AddGrpL3AddrIndexTable(grpL3Addr, ret);
		}
		else
		{
			LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"ALL GrpCCBs used up, can not allocate GrpCCB!!!");
		}
	}
	return ret;
}

void CSAG::DeAllocGrpCCB(UINT16 TabIndex)
{
	if(TabIndex<M_MAX_GRPCCB_NUM)
	{
		DelGIDIndexTable(m_GrpCCBTable[TabIndex].getGID());
		DelGrpL3AddrIndexTable(m_GrpCCBTable[TabIndex].getGrpL3Addr());
		m_GrpCCBTable[TabIndex].cleanGrpSrvInfo();
		m_freeGrpCCBList.push_back(TabIndex);
	}
	else
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), 
			"CSAG::DeAllocGrpCCB(UINT16 TabIndex), TabIndex out of range!!!");
	}
}

SxcGrpCCB* CSAG::FindGrpCCBByGID(UINT16 gid)
{
	map<UINT16,SxcGrpCCB*,less<UINT16> >::iterator it = m_Gid_index_Table.find(gid);
	return (it==m_Gid_index_Table.end()) ? NULL: (*it).second;
}

SxcGrpCCB* CSAG::FindGrpCCBByGrpL3Addr(UINT32 grpL3Addr)
{
	map<UINT32,SxcGrpCCB*,less<UINT32> >::iterator it = m_GrpL3Addr_index_Table.find(grpL3Addr);
	return (it==m_GrpL3Addr_index_Table.end()) ? NULL: (*it).second;
}

void CSAG::AddGIDIndexTable(UINT16 gid, SxcGrpCCB* pGrpCCB)
{
	if(M_INVALID_GID==gid|| NULL==pGrpCCB)
		return;
	m_Gid_index_Table.insert(map<UINT16,SxcGrpCCB*,less<UINT16> >::value_type(gid, pGrpCCB));
}

void CSAG::DelGIDIndexTable(UINT32 gid)
{
	if(M_INVALID_GID==gid)
		return;
	map<UINT16,SxcGrpCCB*,less<UINT16> >::iterator it = m_Gid_index_Table.find(gid);
	if(m_Gid_index_Table.end()!=it)
		m_Gid_index_Table.erase(it);
}

void CSAG::AddGrpL3AddrIndexTable(UINT32 grpL3Addr, SxcGrpCCB* pGrpCCB)
{
	if(M_INVALID_GRPL3ADDR==grpL3Addr || NULL==pGrpCCB)
		return;
	m_GrpL3Addr_index_Table.insert(map<UINT32,SxcGrpCCB*,less<UINT32> >::value_type(grpL3Addr, pGrpCCB));
}

void CSAG::DelGrpL3AddrIndexTable(UINT32 grpL3Addr)
{
	if(M_INVALID_GRPL3ADDR==grpL3Addr)
		return;
	map<UINT32,SxcGrpCCB*,less<UINT32> >::iterator it = m_GrpL3Addr_index_Table.find(grpL3Addr);
	if(m_GrpL3Addr_index_Table.end()!=it)
		m_GrpL3Addr_index_Table.erase(it);
}

void CSAG::handleVoiceDataFromBTS(CMessage& msg)
{
	UINT16 nVDataLen, tmpVDataLen;
	UINT16 nVacVDataLen = 1;
	UINT16 nVacVDataCount = 0;
	UINT16 nVac10msCount = 0;
	UINT8 *VACBuf = NULL;
	CComMessage* pComMsg = NULL;
	VACVoiceDataIE_T *pCurVacVData = NULL;
	int i,j;
	bool blSpyGidVData = g_SpyVData.isSpyGidVoiceData();
	UINT8	*pUdp = (UINT8*)msg.GetDataPtr();
	UINT16 nDataPkt = ((DMUXHeadT*)pUdp)->nFrameNum;	//UDP中语音数据个数
	DMUXVoiDataCommonT* pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+sizeof(DMUXHeadT));	//指向第一个语音数据帧
	for(i=0;i<nDataPkt;i++)
	{
		switch(pVoiDataCommon->Codec)
		{
			case CODEC_G729A:
				nVDataLen = M_G729_10MS_DATALEN;
				break;
			case CODEC_ENCRYPT_G729A:
				nVDataLen = M_ENC_VDATA_PKT_LEN;
				break;
			case CODEC_G711A:
				nVDataLen = M_G711_10MS_DATALEN;
				break;
			default:
				LOG1(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"localSag uplink voiceData codec[0x%02X] not support!!!",
					pVoiDataCommon->Codec);
				if(pComMsg!=NULL)
				{
					pComMsg->Destroy();
				}
				return;				
		}
		//根据L3地址找到CCB
		UINT32 l3Addr = VGetU16BitVal(pVoiDataCommon->CallID);
		CCCB *pCCB = FindCCBByL3Addr(l3Addr);
		if(pCCB!=NULL)
		{
			//如果是集群讲话状态,则确定为集群语音包,需要下发给bts,否则丢弃不处理
			if(SAG_GRP_TALKING_STATE==pCCB->getState())
			{
				//new message if necessary
				if(NULL==pCurVacVData)
				{
					pComMsg = new (CTVoice::GetInstance(), 2000) CComMessage;
					if(pComMsg==NULL)
					{
						LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL),
							"new ComMessage failed!!!");
						return;
					}
					else
					{
						nVacVDataLen = 1;
						nVacVDataCount = 0;
						nVac10msCount = 0;
						VACBuf = (UINT8*)pComMsg->GetDataPtr();
						pCurVacVData = (VACVoiceDataIE_T*)(&VACBuf[1]);
					}					
				}
				//根据CCB找到GID
				UINT16 gid = pCCB->getGID();
				SxcGrpCCB *pGrpCCB = CSAG::getSagInstance()->FindGrpCCBByGID(gid);
				if(CODEC_G729A==pVoiDataCommon->Codec || CODEC_ENCRYPT_G729A==pVoiDataCommon->Codec)
				{
					//构造语音包格式，减少中间环节提高效率,此处直接发送给btsL2
					for(j=0;j<pVoiDataCommon->frameNum;j++)
					{
						VSetU32BitVal(pCurVacVData->header.Eid, gid);
						pCurVacVData->header.Cid = GRP_TONE_FLAG_CID;
						pCurVacVData->header.SN = pVoiDataCommon->timeStamp+j;
						pCurVacVData->header.Type = pVoiDataCommon->Codec;

						memcpy((void*)pCurVacVData->VoiceData, 
							(void*)((UINT8*)pVoiDataCommon->CallID+sizeof(DMUXVoiDataCommonT)+j*nVDataLen), 
							nVDataLen);

						nVacVDataCount++;
						nVacVDataLen += (sizeof(VACVoiceDataHeaderT)+nVDataLen);
						pCurVacVData = (VACVoiceDataIE_T*)&VACBuf[nVacVDataLen];

						if(CODEC_ENCRYPT_G729A==pVoiDataCommon->Codec)
						{
							if(pGrpCCB)
							{
								pGrpCCB->m_grpVDataGuard.valAdd(4);
							}
							nVac10msCount += 4;
							//加密语音打包为10byte加密信息帧+40msG.729语音帧
							break;
						}
						else
						{
							if(pGrpCCB)
							{
								pGrpCCB->m_grpVDataGuard.valAdd(1);
							}
							nVac10msCount++;
						}
					}
					if(pVoiDataCommon->blG729B)
					{
						VSetU32BitVal(pCurVacVData->header.Eid, gid);
						pCurVacVData->header.Cid = GRP_TONE_FLAG_CID;
						pCurVacVData->header.SN = pVoiDataCommon->timeStamp+j;
						pCurVacVData->header.Type = CODEC_G729B_SID;
						memcpy(pCurVacVData->VoiceData, 
							(void*)((UINT8*)pVoiDataCommon->CallID+sizeof(DMUXVoiDataCommonT)+pVoiDataCommon->frameNum*nVDataLen), 
							M_G729B_SRTP_DATALEN);
						
						nVacVDataCount++;
						nVac10msCount++;
						nVacVDataLen += (sizeof(VACVoiceDataHeaderT)+M_G729B_SRTP_DATALEN);
						pCurVacVData = (VACVoiceDataIE_T*)&VACBuf[nVacVDataLen];
					}
					if(blSpyGidVData)
					{			
						if(gid == g_SpyVData.getDiagGID())
						{
							if(CODEC_ENCRYPT_G729A==pVoiDataCommon->Codec)
							{
								//加密语音固定40ms打包
								g_SpyVData.uCntGID10msVDataToVAC += 4;
							}
							else
							{
								g_SpyVData.uCntGID10msVDataToVAC += pVoiDataCommon->frameNum;
							}						
							if(pVoiDataCommon->blG729B)
							{
								++g_SpyVData.uCntGID10ms729BToVAC;
							}
						}
					}
				}
				else
				{
					LOG1(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
						"localSag uplink voiceData codec[0x%02X] not support!!!",
						pVoiDataCommon->Codec);
					pComMsg->Destroy();
					return;
				}
			}
		}
		//下一语音包
		if(CODEC_ENCRYPT_G729A==pVoiDataCommon->Codec)
		{
			tmpVDataLen = nVDataLen;
		}
		else
		{
			tmpVDataLen = pVoiDataCommon->frameNum*nVDataLen;
		}
		pVoiDataCommon = (DMUXVoiDataCommonT*)((UINT8*)pVoiDataCommon + 
										sizeof(DMUXVoiDataCommonT) + 
										tmpVDataLen +
										M_G729B_SRTP_DATALEN*pVoiDataCommon->blG729B);
	}
	//下发给bts
	if(pComMsg!=NULL)
	{
		if(nVacVDataCount>0)
		{
			VACBuf[0] = nVacVDataCount;
			pComMsg->SetDataLength(nVacVDataLen);
			pComMsg->SetDstTid(M_TID_VAC);
			pComMsg->SetSrcTid(M_TID_VOICE);
			pComMsg->SetMessageId(MSGID_VOICE_VAC_DATA);
			if(postComMsg(pComMsg))
			{
				Counters.nVoiDataToVAC++;
				Counters.n10msPktToVAC += nVac10msCount;
			}
		}
		else
		{
			pComMsg->Destroy();
		}
	}
}

void CSAG::handleSignalLAGrpPagingRsp(CSAbisSignal& signal)
{
	SAbisSignalT *pSigData = (SAbisSignalT*)signal.GetDataPtr();
	LOG4(SAG_LOG_VOICE_GRP_INFO3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"LAGrpPagingRsp<----BTS UID[0x%08X] GrpL3Addr[0x%08X] commType[0x%02X] cause[0x%02X]",
		VGetU32BitVal(pSigData->sigPayload.GrpLAPagingRsp.UID),
		VGetU32BitVal(pSigData->sigPayload.GrpLAPagingRsp.GrpL3Addr),
		pSigData->sigPayload.GrpLAPagingRsp.commType,
		pSigData->sigPayload.GrpLAPagingRsp.Cause);
	UINT32 grpL3Addr = VGetU32BitVal(pSigData->sigPayload.GrpLAPagingRsp.GrpL3Addr);
	SxcGrpCCB *pGrpCCB = FindGrpCCBByGrpL3Addr(grpL3Addr);
	if(pGrpCCB!=NULL)
	{
		if(pGrpCCB->isPagingUserNow())
		{
			pGrpCCB->setPagingUserFlag(false);
			//停止组寻呼响应超时定时器
			pGrpCCB->deleteGrpTimer(TIMERID_SAG_GRP_LAGRPPAGING, &pGrpCCB->m_pTmGrpPaging);
			//向组呼发起者发送内部信令PTT_Connect_MSG
			pGrpCCB->sendInnerSignalPttConnect(pGrpCCB->getGrpSetupUID());
			//记录组呼发起者为讲话方
			pGrpCCB->grantPttToUser(pGrpCCB->getGrpSetupUID());
			//向bts发送LEGrpPaging_MSG
			pGrpCCB->sendDLSignalLEPagingStart();
			//(MaybeFinishLater)启动定时器TIMERID_SAG_GRP_TTL监控组呼是否超时
			//(MaybeFinishLater)启动定时器TIMERID_SAG_GRP_MAX_TALKING_TIME检测最长讲话时间是否超时
			//(MaybeFinishLater)启动定时器TIMERID_SAG_GRP_PRESSINFO用于PressInfo周期性通知
		}
		else
		{
			//do nothing now
		}
	}
	else
	{
		LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"cannot find GrpCCB!");
	}
}
void CSAG::handleSignalStatusReport(CSAbisSignal& signal)
{
	//do nothing at present
}
void CSAG::handleSignalGrpHandoverReq(CSAbisSignal& signal)
{
	SAbisSignalT *pSigData = (SAbisSignalT*)signal.GetDataPtr();
	UINT16 gid = VGetU16BitVal(pSigData->sigPayload.GrpHandoverReq.GID);
	LOG4(SAG_LOG_VOICE_GRP_INFO3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"GrpHandoverReq<----BTS GID[0x%04X] UID[0x%08X] PID[0x%08X] curBTSID[0x%08X]",
		gid,
		VGetU32BitVal(pSigData->sigPayload.GrpHandoverReq.UID),
		VGetU32BitVal(pSigData->sigPayload.GrpHandoverReq.PID),
		VGetU32BitVal(pSigData->sigPayload.GrpHandoverReq.curBTSID));
	//本地sac不支持集群业务切换，回失败响应
	sendDLSignalGrpHandoverRsp(signal, 1, M_INVALID_GRPL3ADDR, 0);
#if 0	
	//如果根据GID找到了GrpCCB，则回成功响应
	SxcGrpCCB *pGrpCCB = FindGrpCCBByGID(gid);
	if(pGrpCCB!=NULL)
	{
		pGrpCCB->sendDLSignalGrpHandoverRsp(signal);
	}
#endif
}
void CSAG::handleSignalGrpResReq(CSAbisSignal& signal)
{
	SAbisSignalT *pSigData = (SAbisSignalT*)signal.GetDataPtr();
	UINT16 gid = VGetU16BitVal(pSigData->sigPayload.GrpResReq.GID);
	UINT32 uid = VGetU32BitVal(pSigData->sigPayload.GrpResReq.UID);
	LOG3(SAG_LOG_VOICE_GRP_INFO3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"GrpResReq<----BTS GID[0x%04X] UID[0x%08X] PID[0x%08X] ",
		gid, uid,
		VGetU32BitVal(pSigData->sigPayload.GrpResReq.PID));

	UINT8 result = M_SABIS_SUCCESS;
	if(!CSAG::getSagInstance()->isUserInGroup(uid, gid))
	{
		LOG2(SAG_LOG_VOICE_GRP_INFO3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"user not in group,  UID[0x%08X] GID[0x%04X]", uid, gid);
		result = M_SABIS_FAIL;
	}
	
	//如果根据GID找到了GrpCCB，则回响应
	SxcGrpCCB *pGrpCCB = FindGrpCCBByGID(gid);
	if(pGrpCCB!=NULL)
	{
		pGrpCCB->sendDLSignalGrpResRsp(signal, result);
	}
	else
	{
		LOG(SAG_LOG_VOICE_GRP_INFO3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"GrpResReq<----BTS when Grp doesn't exist!");
	}
}
void CSAG::handleSignalPttSetupReq(CSAbisSignal& signal)
{
	SAbisSignalT *pSigData = (SAbisSignalT*)signal.GetDataPtr();
	PTTSetupT_CPE *pDataCpe = (PTTSetupT_CPE*)(&pSigData->sigPayload.UTSAG_Payload_L3Addr.msgType-1);
	LOG5(SAG_LOG_VOICE_GRP_INFO1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"PttSetupReq<----BTS GID[0x%04X] L3Addr[0x%08X] prio[0x%02X] CommType[0x%02X] EncryptFlag[0x%02X] ",
		VGetU16BitVal(pDataCpe->GID), 
		VGetU32BitVal(pSigData->sigPayload.UTSAG_Payload_L3Addr.L3Addr),
		pDataCpe->CallPriority, 
		pDataCpe->CommType, pDataCpe->EncryptFlag);
	//消息进入状态机
	injectFsm(signal);
}
void CSAG::handleSignalPttConnectAck(CSAbisSignal& signal)
{
	//do nothing at present
}
void CSAG::handleSignalGrpDisconnect(CSAbisSignal& signal)
{
	SAbisSignalT *pSigData = (SAbisSignalT*)signal.GetDataPtr();
	GroupDiscntT_CPE *pDataCpe = (GroupDiscntT_CPE*)(&pSigData->sigPayload.UTSXC_Payload_GrpL3Addr.msgType-1);
	LOG2(SAG_LOG_VOICE_GRP_INFO1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"GrpDisconnect<----BTS GID[0x%04X] UID[0x%08X] ",
		VGetU16BitVal(pDataCpe->GID), VGetU32BitVal(pDataCpe->UID));	
	//(MaybeFinishLater)暂时不支持组呼建立成功后组呼建立者释放组呼
	
	//消息进入状态机
	injectFsm(signal);
}
void CSAG::handleSignalGrpCallingRlsComplete(CSAbisSignal& signal)
{
	//do nothing at present
}
void CSAG::handleSignalPttPressReq(CSAbisSignal& signal)
{
	SAbisSignalT *pSignalData = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = VGetU32BitVal(pSignalData->sigPayload.UTSXC_Payload_GrpUid.GrpUid);
	PTTPressReqT_CPE *pDataCpe = (PTTPressReqT_CPE*)(&pSignalData->sigPayload.UTSXC_Payload_GrpUid.msgType-1);
	UINT16 gid = VGetU16BitVal(pDataCpe->Gid);
	LOG4(SAG_LOG_VOICE_GRP_INFO2, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"PttPressReq<----BTS GID[0x%04X] UID[0x%08X] prio[0x%02X] encryptFlag[0x%02X]",
		gid, uid, pDataCpe->CallPrioity, pDataCpe->EncryptControl);
	SxcGrpCCB *pGrpCCB = FindGrpCCBByGID(gid);
	//如果组呼存在
	if(NULL!=pGrpCCB)
	{
		//handlePttPressReq
		pGrpCCB->handlePttPressReq(signal, uid);
	}
	else
	{
		//组呼不存在向终端结果为回组呼不存在的话权响应消息
		sendPttPressRsp(gid, TRANSGRANT_GRPSRV_NOT_EXIST, uid, M_USE_OLD_PTTREQ_IF, 
			pDataCpe->CallPrioity, pDataCpe->EncryptControl);
	}
}
void CSAG::handleSignalPttPressApplyReq(CSAbisSignal& signal)
{
	SAbisSignalT *pSignalData = (SAbisSignalT*)signal.GetDataPtr();
	UINT32 uid = VGetU32BitVal(pSignalData->sigPayload.PttPressApplyReq.UID);
	UINT16 gid = VGetU16BitVal(pSignalData->sigPayload.PttPressApplyReq.GID);
	UINT16 sessionType = VGetU16BitVal(pSignalData->sigPayload.PttPressApplyReq.sessionType);
	LOG5(SAG_LOG_VOICE_GRP_INFO2, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"PttPressApplyReq<----BTS GID[0x%04X] UID[0x%08X] prio[0x%02X] encryptFlag[0x%02X] sessionType[0x%02X]",
		gid, uid, pSignalData->sigPayload.PttPressApplyReq.prio, 
		pSignalData->sigPayload.PttPressApplyReq.EncryptCtrl,
		sessionType);
	SxcGrpCCB *pGrpCCB = FindGrpCCBByGID(gid);
	//如果组呼存在
	if(NULL!=pGrpCCB)
	{
		//handlePttPressReq
		pGrpCCB->handlePttPressReq(signal, uid);
	}
	else
	{
		//组呼不存在向终端结果为回组呼不存在的话权响应消息
		sendPttPressRsp(gid, TRANSGRANT_GRPSRV_NOT_EXIST, uid, sessionType, 
			pSignalData->sigPayload.PttPressApplyReq.prio, 
			pSignalData->sigPayload.PttPressApplyReq.EncryptCtrl);
	}
}
void CSAG::handleSignalPttInterruptAck(CSAbisSignal& signal)
{
	//do nothing at present
}
void CSAG::handleSignalPttRls(CSAbisSignal& signal)
{
	SAbisSignalT *pSigData = (SAbisSignalT*)signal.GetDataPtr();
	LOG1(SAG_LOG_VOICE_GRP_INFO2, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"PttRelease<----BTS L3Addr[0x%08X] ", 
		VGetU32BitVal(pSigData->sigPayload.UTSAG_Payload_L3Addr.L3Addr));
	//消息进入状态机
	injectFsm(signal);
}
void CSAG::handleSignalPttPressCancel(CSAbisSignal& signal)
{
	SAbisSignalT *pSigData = (SAbisSignalT*)signal.GetDataPtr();
	PTTPressCancleT_CPE* pDataCpe = (PTTPressCancleT_CPE*)(&pSigData->sigPayload.UTSXC_Payload_GrpUid.msgType-1);
	UINT32 uid = VGetU32BitVal(pSigData->sigPayload.UTSXC_Payload_GrpUid.GrpUid);
	UINT16 gid = VGetU16BitVal(pDataCpe->GID);
	LOG2(SAG_LOG_VOICE_GRP_INFO2, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"PttPressCancel<----BTS UID[0x%08X] GID[0x%04X]", uid, gid);
	//根据消息找到GrpCCB和CCB
	SxcGrpCCB *pGrpCCB = FindGrpCCBByGID(gid);
	if(NULL!=pGrpCCB)
	{
		//如果talkingUID==UID
		if(pGrpCCB->getTalkingUID()==uid)
		{
			//向bts发送releaseResReq
			CCCB *pCCB = FindCCBByUID(uid);
			if(NULL!=pCCB)
			{
				sendRlsTransResReq(pCCB, REL_CAUSE_NORMAL);
			}
			//话权调度
			pGrpCCB->dispatchTalkersInGrpQueue();
		}
		else
		{
			//如果UID在queue中，取消排队
			pGrpCCB->delUserFromQueue(uid);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CSAG::handleTimeout_Grp_PttConnect(CMessage& msg)
{
	sigleVoiceCallTimeoutProc(msg);
}
void CSAG::handleTimeout_Grp_LAGrpPaging(CMessage& msg)
{
	SagTimerStructT* pData = (SagTimerStructT*)msg.GetDataPtr();
	LOG1(SAG_LOG_VOICE_GRP_INFO1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"Timeout_Grp_LAGrpPaging: GID[0x%04X]", pData->gid);
	//find GrpCCB
	SxcGrpCCB *pGrpCCB = (SxcGrpCCB*)FindGrpCCBByGID(pData->gid);
	if(NULL!=pGrpCCB)
	{
		//deleteGrpTimer
		pGrpCCB->deleteGrpTimer(TIMERID_SAG_GRP_LAGRPPAGING, &pGrpCCB->m_pTmGrpPaging);
		//releaseGrpSrv(向bts发送GrpRelease和GrpCallingRls)
		pGrpCCB->releaseGrpSrv(REL_CAUSE_TIMERTIMEOUT);
	}
}
void CSAG::handleTimeout_Grp_AssignResReq(CMessage& msg)
{
	SagTimerStructT* pData = (SagTimerStructT*)msg.GetDataPtr();
	LOG2(SAG_LOG_VOICE_GRP_INFO2, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"Timeout_Grp_AssignResReq: GID[0x%04X] UID[0x%08X]", pData->gid, pData->uid);
	//find GrpCCB
	SxcGrpCCB *pGrpCCB = (SxcGrpCCB*)FindGrpCCBByGID(pData->gid);
	//deleteGrpTimer
	if(NULL!=pGrpCCB)
	{
		pGrpCCB->deleteGrpTimer(TIMERID_SAG_GRP_ASSIGNRESREQ, &pGrpCCB->m_pTmGrpAssignResReq);
		//话权调度
		pGrpCCB->dispatchTalkersInGrpQueue();
	}
}
void CSAG::handleTimeout_Grp_PressInfo(CMessage& msg)
{
	SagTimerStructT* pData = (SagTimerStructT*)msg.GetDataPtr();
	LOG1(SAG_LOG_VOICE_GRP_INFO3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"Timeout_Grp_PressInfo: GID[0x%04X] ", pData->gid);
	//find GrpCCB
	SxcGrpCCB *pGrpCCB = (SxcGrpCCB*)FindGrpCCBByGID(pData->gid);
	if(NULL!=pGrpCCB)
	{
		//deleteGrpTimer
		pGrpCCB->deleteGrpTimer(TIMERID_SAG_GRP_PRESSINFO, &pGrpCCB->m_pTmGrpPressInfo);
		//PttGranted
		pGrpCCB->sendDLSignalPttGranted(pGrpCCB->getTalkingUID());
		//startGrpTimer
		pGrpCCB->startGrpTimer(TIMERID_SAG_GRP_PRESSINFO, &pGrpCCB->m_pTmGrpPressInfo);
		//check grpVData
		pGrpCCB->m_grpVDataGuard.checkIfValChanged();
		//if no grpVData for a long time, then ....
		if(pGrpCCB->m_grpVDataGuard.getTimeUnchanged()>(g_maxGrpIdleTime+6))
		{
			LOG1(SAG_LOG_VOICE_GRP_INFO1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"No GrpVData for a long time: GID[0x%04X], release talker and dispatchTalkersInGrpQueue ", 
				pData->gid);
		
			//monitor grpVData again
			pGrpCCB->m_grpVDataGuard.init();
			//if someone talking, release 
			if(pGrpCCB->isSomeoneTalking())
			{
				CCCB *pCCB = CSAG::getSagInstance()->FindCCBByUID(pGrpCCB->getTalkingUID());
				if(pCCB)
				{
					//pCCB->sendDLSignalRlsTransResReq(REL_CAUSE_PTT_INTERRUPT);
					pGrpCCB->sendInnerSignalPttInterrupt(
						pGrpCCB->getTalkingUID(), 
						REL_CAUSE_PTT_INTERRUPT);
				}
			}
			//dispatchTalkersInGrpQueue
			pGrpCCB->dispatchTalkersInGrpQueue();
		}
	}
}
void CSAG::handleTimeout_Grp_MaxIdleTime(CMessage& msg)
{
	SagTimerStructT* pData = (SagTimerStructT*)msg.GetDataPtr();
	LOG1(SAG_LOG_VOICE_GRP_INFO1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"Timeout_Grp_MaxIdleTime: GID[0x%04X] ", pData->gid);
	//find GrpCCB
	SxcGrpCCB *pGrpCCB = (SxcGrpCCB*)FindGrpCCBByGID(pData->gid);
	if(NULL!=pGrpCCB)
	{
		//deleteGrpTimer
		pGrpCCB->deleteGrpTimer(TIMERID_SAG_GRP_MAX_IDLE_TIME, &pGrpCCB->m_pTmMaxGrpIdle);
		//releaseGrpSrv(向bts发送GrpRelease和GrpCallingRls)
		pGrpCCB->releaseGrpSrv(REL_CAUSE_IDLE_TIMEOUT);
	}
}
void CSAG::handleTimeout_Grp_TTL(CMessage& msg)
{
	SagTimerStructT* pData = (SagTimerStructT*)msg.GetDataPtr();
	LOG1(SAG_LOG_VOICE_GRP_INFO1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"Timeout_Grp_TTL: GID[0x%04X] ", pData->gid);
	//find GrpCCB
	SxcGrpCCB *pGrpCCB = (SxcGrpCCB*)FindGrpCCBByGID(pData->gid);
	if(NULL!=pGrpCCB)
	{
		//deleteGrpTimer
		pGrpCCB->deleteGrpTimer(TIMERID_SAG_GRP_TTL, &pGrpCCB->m_pTmMaxGrpAlive);
		//releaseGrpSrv(向bts发送GrpRelease和GrpCallingRls)
		pGrpCCB->releaseGrpSrv(REL_CAUSE_TALK_TOOLONG);
	}
}
void CSAG::handleTimeout_Grp_TalkingTime(CMessage& msg)
{
	SagTimerStructT* pData = (SagTimerStructT*)msg.GetDataPtr();
	LOG1(SAG_LOG_VOICE_GRP_INFO1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
		"Timeout_Grp_TalkingTime: GID[0x%04X] ", pData->gid);
	//find GrpCCB
	SxcGrpCCB *pGrpCCB = (SxcGrpCCB*)FindGrpCCBByGID(pData->gid);
	if(NULL!=pGrpCCB)
	{
		//deleteGrpTimer
		pGrpCCB->deleteGrpTimer(TIMERID_SAG_GRP_MAX_TALKING_TIME, &pGrpCCB->m_pTmMaxGrpTalking);
		//releaseGrpSrv(向bts发送GrpRelease和GrpCallingRls)
		pGrpCCB->releaseGrpSrv(REL_CAUSE_TALK_TOOLONG);
	}
}
////////////////////////////////////////////////////////////////////////////////
bool CSAG::sendLoginRspWithUserSrvInfo(CSAbisSignal& signal)
{
	//不支持鉴权，直接回应注册结果
	//带集群组信息
	//参见LoginRspT_CPE和AttachSubT_CPE
	
	typedef struct __tmpLoginRspT
	{
		UINT8	UID[4];
		UINT8	msgType;
		UINT8	RegPeriod[2];	//Periodical Update timer value, 0x02(hour) 0x01(1 hour)
		UINT8	LAI[3];
		UINT8	LoginResult;	//login result,always be 0 here
		
		UINT8	Ind;//must be 0
#if 0		
		UINT8	result;
		UINT8	EID[4];
		UINT8	port;
#endif		
		UINT8	AttachInd;////0 不携带1 携带组附属信息（开机注册、周期注册、位置更新注册）
		UINT8	Num;
		UINT8	EmergencyCallInd;
		UINT8	Content;	
	}tmpLoginRspT;

	//发送LoginRsp;
	bool ret = false;
	CSAbisSignal sndSignal;
	VoiceVCRCtrlMsgT *pData, *pDataRcv;
	pDataRcv = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
	UINT32 uid=VGetU32BitVal(pDataRcv->sigPayload.UTSAG_Payload_Uid.Uid);
	map< UINT32, userInfo, less < UINT32 > >::iterator  itFound;
	UINT8 loginResult = LOGINRSP_SUCCESS;
	if(!findUserInfoByUID(uid, itFound))
	{
		loginResult = LOGINRSP_USRNOTEXIST;
	}
	
	if ( !sndSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(tmpLoginRspT)+1000) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		sndSignal.SetSigIDS(LoginRsp_MSG);
		pData = (VoiceVCRCtrlMsgT*)sndSignal.GetDataPtr();
		tmpLoginRspT* pSigPayload = (tmpLoginRspT*)pData->sigPayload.UTSAG_Payload_Uid.Uid;
		
		memcpy(pSigPayload->UID, pDataRcv->sigPayload.UTSAG_Payload_Uid.Uid, 4);
		pSigPayload->msgType = M_MSGTYPE_LOGINRSP;
		pSigPayload->RegPeriod[0]=2;//hour
		pSigPayload->RegPeriod[1]=1;//one hour
		pSigPayload->LAI[0]=pSigPayload->LAI[1]=pSigPayload->LAI[2]=0;
		pSigPayload->LoginResult=loginResult;
		UINT16 nPayloadLen;
		
		pSigPayload->Ind=0;
		pSigPayload->EmergencyCallInd=0;
		pSigPayload->Content=0;
		
		if(LOGINRSP_SUCCESS!=loginResult)
		{
			pSigPayload->AttachInd=0;
			pSigPayload->Num=0;
			nPayloadLen = sizeof(tmpLoginRspT);
		}
		else
		{
			UINT16 len;
			UINT8 grpNum;
			if(formatUserGrpListInfo((char*)&pSigPayload->Content, len, grpNum, uid))
			{
				nPayloadLen = sizeof(tmpLoginRspT) - 1 + len;
			}
			else
			{
				nPayloadLen = sizeof(tmpLoginRspT);
			}
			pSigPayload->AttachInd=1;
			pSigPayload->Num = grpNum;
		}

		sndSignal.SetSigHeaderLengthField(nPayloadLen);
		sndSignal.SetPayloadLength(sizeof(SigHeaderT)+nPayloadLen);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(sndSignal);
		if(ret)
		{
			LOG2(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"LoginRsp---->bts, UID[0x%08X] LoginResult[0x%02X]", 
				VGetU32BitVal(pData->sigPayload.UTSAG_Payload_Uid.Uid),
				pSigPayload->LoginResult);
		}
	}	
	return ret;
}

bool CSAG::sendSuccessLoginRspWithoutUserSrvInfo(CSAbisSignal& signal)
{
	//不支持鉴权，直接回应注册成功
	//(不带集群组信息和其他业务功能(用户数据下载更新等))
	//参见LoginRspT_CPE和AttachSubT_CPE
	
	typedef struct __tmpLoginRspT
	{
		UINT8	UID[4];
		UINT8	msgType;
		UINT8	RegPeriod[2];	//Periodical Update timer value, 0x02(hour) 0x01(1 hour)
		UINT8	LAI[3];
		UINT8	LoginResult;	//login result,always be 0 here
		
		UINT8	Ind;//must be 0
#if 0		
		UINT8	result;
		UINT8	EID[4];
		UINT8	port;
#endif		
		UINT8	AttachInd;//must be 0
		UINT8	Num;
		UINT8	EmergencyCallInd;
		UINT8	Content;	
	}tmpLoginRspT;

	//发送LoginRsp;
	bool ret = false;
	CSAbisSignal sndSignal;
	VoiceVCRCtrlMsgT *pData, *pDataRcv;
	if ( !sndSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(tmpLoginRspT)) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		sndSignal.SetSigIDS(LoginRsp_MSG);
		pData = (VoiceVCRCtrlMsgT*)sndSignal.GetDataPtr();
		tmpLoginRspT* pSigPayload = (tmpLoginRspT*)pData->sigPayload.UTSAG_Payload_Uid.Uid;
		pDataRcv = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
		
		memcpy(pSigPayload->UID, pDataRcv->sigPayload.UTSAG_Payload_Uid.Uid, 4);
		pSigPayload->msgType = M_MSGTYPE_LOGINRSP;
		pSigPayload->RegPeriod[0]=2;//hour
		pSigPayload->RegPeriod[1]=1;//one hour
		pSigPayload->LAI[0]=pSigPayload->LAI[1]=pSigPayload->LAI[2]=0;
		pSigPayload->LoginResult=LOGINRSP_SUCCESS;//success

		pSigPayload->Ind=0;
		pSigPayload->AttachInd=0;
		pSigPayload->Num=0;
		pSigPayload->EmergencyCallInd=0;
		pSigPayload->Content=0;		

		sndSignal.SetSigHeaderLengthField(sizeof(tmpLoginRspT));
		sndSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(tmpLoginRspT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(sndSignal);
		if(ret)
		{
			LOG2(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"LoginRsp---->bts, UID[0x%08X] LoginResult[0x%02X]", 
				VGetU32BitVal(pData->sigPayload.UTSAG_Payload_Uid.Uid),
				pSigPayload->LoginResult);
		}
	}	
	return ret;
}

bool CSAG::sendDLSignalGrpHandoverRsp(CSAbisSignal& ReqMsg, UINT8 result, UINT32 grpL3Addr, UINT8 transID)
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

		memcpy(pData->sigPayload.GrpHandoverRsp.GID, pReqData->sigPayload.GrpHandoverReq.GID, 2);
		memcpy(pData->sigPayload.GrpHandoverRsp.UID, pReqData->sigPayload.GrpHandoverReq.UID, 4);
		pData->sigPayload.GrpHandoverRsp.Result = result;
		VSetU32BitVal(pData->sigPayload.GrpHandoverRsp.GrpL3Addr, grpL3Addr);
		memcpy(pData->sigPayload.GrpHandoverRsp.PID, pReqData->sigPayload.GrpHandoverReq.PID, 4);
		memcpy(pData->sigPayload.GrpHandoverRsp.curBTSID, pReqData->sigPayload.GrpHandoverReq.curBTSID, 4);
		pData->sigPayload.GrpHandoverRsp.transID = transID;

		signal.SetSigHeaderLengthField(sizeof(GrpHandoverRspT));
		signal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpHandoverRspT));
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			LOG6(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"GrpHandoverRsp---->bts, GID[0x%04X] UID[0x%08X] GrpL3Addr[0x%08X] PID[0x%08X] curBTSID[0x%08X] TransID[0x%02X]", 
				VGetU16BitVal(pData->sigPayload.GrpHandoverRsp.GID), 
				VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.UID),
				VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.GrpL3Addr),
				VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.PID),
				VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.curBTSID),
				transID);
		}
	}
	return ret;	
}

bool CSAG::sendPttPressRsp(UINT16 gid, UINT8 result, UINT32 uid, UINT16 sessionType, UINT8 prio, UINT8 encryptFlag)
{
	bool ret = false;
	CSAbisSignal signal;
	VoiceVCRCtrlMsgT *pData;
	UINT16 sigHeadLenFieldVal;
	char signalName[20];

	if(M_USE_OLD_PTTREQ_IF==sessionType)
	{
		sigHeadLenFieldVal = sizeof(PTTPressRspT_CPE) - sizeof(UINT8) + sizeof(UINT32);
		strcpy(signalName, "PttPressRsp");
	}
	else
	{
		sigHeadLenFieldVal = sizeof(PttPressApplyRspT);
		strcpy(signalName, "PttPressApplyRsp");
	}	
	
	if ( !signal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sigHeadLenFieldVal) )
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		if(M_USE_OLD_PTTREQ_IF==sessionType)	
		{//PttPressReq
			signal.SetSigIDS(PTT_PressRsp_MSG);
			pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
			VSetU32BitVal(pData->sigPayload.UTSXC_Payload_GrpUid.GrpUid, uid);
			PTTPressRspT_CPE* pDataCPE = (PTTPressRspT_CPE*)(&pData->sigPayload.UTSXC_Payload_GrpUid.msgType-1);
			pDataCPE->msgType = M_MSGTYPE_PTT_PRESS_RSP;
			VSetU16BitVal(pDataCPE->GID, gid);
			pDataCPE->Grant = result;
			pDataCPE->EryptFlag = encryptFlag;
		}
		else
		{//PttPressApplyReq
			signal.SetSigIDS(PTT_PressApplyRsp_MSG);
			pData = (VoiceVCRCtrlMsgT*)signal.GetDataPtr();
			VSetU32BitVal(pData->sigPayload.PttPressApplyRsp.UID, uid);
			VSetU16BitVal(pData->sigPayload.PttPressApplyRsp.GID, gid);
			pData->sigPayload.PttPressApplyRsp.TransmissionGrant = result;
			pData->sigPayload.PttPressApplyRsp.EncryptCtrl = encryptFlag;
			VSetU16BitVal(pData->sigPayload.PttPressApplyRsp.sessionType, sessionType);
		}
		signal.SetSigHeaderLengthField(sigHeadLenFieldVal);
		signal.SetPayloadLength(sizeof(SigHeaderT)+sigHeadLenFieldVal);
		ret = CSAG::getSagInstance()->sendSignalToLocalBTS(signal);
		if(ret)
		{
			if(M_USE_OLD_PTTREQ_IF==sessionType)
			{
				LOG5(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"PttPressRsp---->bts, UID[0x%08X] GID[0x%08X] result[%d] encryptFlag[%d] prio[%d]", 
					uid, gid, result, encryptFlag, prio);
			}
			else
			{
				LOG5(SAG_LOG_DL_SIGNAL, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"PttPressApplyRsp---->bts, UID[0x%08X] GID[0x%08X] result[%d] sessionType[%d] encryptFlag[%d]", 
					uid, gid, result, sessionType, encryptFlag);
			}
		}
	}
	return ret;
	
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

