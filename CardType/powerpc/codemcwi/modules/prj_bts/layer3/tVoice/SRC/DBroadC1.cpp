/*******************************************************************************
* Copyright (c) 2010 by Beijing Jiaxun Feihong Electrical Co.Ltd.All Rights Reserved   
* File Name      : DBroadCastSrv.cpp
* Create Date    : 5-Aug-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "DBroadCastStruct.h"
#include "DBroadCastSrv.h"
#include "voiceFsm.h"
#include "tcpE.h"
#include "voiceToolFunc.h"
#include "DBroadCastMsg.h"
#include "BtsVMsgId.h"

/*******************************************************************************
* Prototype      : 
* Parameters     :msg from DCS, include signal and data, begin with tcpEhead
* Return Value   :
* Global Variable: 
* Description    : distinguish data and signals , then handle them separately
*******************************************************************************/
void VoiceFSM::handle_Msg_frmDCS(CMessage& msg)
{
	TcpPktHeaderT* pTcpHead = (TcpPktHeaderT*)msg.GetDataPtr();
	UINT16 pktType = VGetU16BitVal(pTcpHead->UserType);
	if(M_TCP_PKT_DGRP_DATA_USERTYPE==pktType)
	{
		handle_DB_Data_frmDCS(msg);
	}
	else
	{
		if(M_TCP_PKT_DGRP_CTRL_USERTYPE==pktType)
		{
			handle_DB_Signal_frmDCS(msg);
		}
	}
}

/*******************************************************************************
* Prototype      : 
* Parameters     :control msg from DCS, begin with tcpEhead
* Return Value   :
* Global Variable: 
* Description    : 
*******************************************************************************/
void VoiceFSM::handle_DB_Signal_frmDCS(CMessage& msg)
{
	DB_BtsDcsCtrlMsgHeadT* pSigHead = 
		(DB_BtsDcsCtrlMsgHeadT*)(((UINT8*)msg.GetDataPtr())+sizeof(TcpPktHeaderT));
	switch(pSigHead->DBMessagetype)
	{
		case MSGTYPE_DB_UIDMsg:
			handle_DB_UIDMsg_frmDCS(msg);
			break;
		case MSGTYPE_DB_Ready:
			break;
		case MSGTYPE_DB_DataReq:
			break;
		case MSGTYPE_DB_CReady:
			break;
		default:
			break;
	}
}

/*******************************************************************************
* Prototype      : 
* Parameters     :data msg from DCS, begin with tcpEhead
* Return Value   :
* Global Variable: 
* Description    : handle msg separately
*******************************************************************************/
void VoiceFSM::handle_DB_Data_frmDCS(CMessage& msg)
{
	bool blUidData = false;
	bool blGidData = false;
	DB_BtsDcsDataMsgHeadT* pDataHead = 
		(DB_BtsDcsDataMsgHeadT*)(((UINT8*)msg.GetDataPtr())+sizeof(TcpPktHeaderT));

	UINT8 *pDataNew;
	UINT16 nDataLenNew;
	if(DataType_GrpBroadcastData==pDataHead->DBDatatype)
	{
		blGidData = true;
		dcsCounters.cntGidDataFromDCS++;

		
	}
	else
	{
		if(DataType_UnicastData==pDataHead->DBDatatype)
		{
			blUidData = true;
			dcsCounters.cntUidDataFromDCS++;
			
			pDataNew = ((UINT8*)pDataHead) - 1;
			nDataLenNew = msg.GetDataLength() - sizeof(TcpPktHeaderT) + 1;
			DB_UnicastDataT *pUnicastData = (DB_UnicastDataT*)pDataHead;
			UINT32 uid = VGetU32BitVal(pUnicastData->UID);
			VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByUID(uid);
			if(pCCB==NULL)
			{
				LOG1(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), 
					"handle_DB_Data_frmDCS(), cannot find CCB, UID[0x%08X]!!!", uid);
				return;
			}
			VoiceTuple tuple = pCCB->getVoiceTuple();
			pDataNew[0] = tuple.Cid; // CID
			msg.SetEID(tuple.Eid); //EID
			msg.SetDataPtr((void*)pDataNew);
			msg.SetDataLength(nDataLenNew);
			msg.SetSrcTid(M_TID_VOICE);
			msg.SetDstTid(M_TID_UTDM);
			msg.SetMessageId(MSGID_DCS_CPE_DATA_DOWNLINK);
		}
		else
		{
			//error
			LOG1(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), 
				"handle_DB_Data_frmDCS(), wrong datatype[%d]!!!", 
				pDataHead->DBDatatype);
			return;			
		}
	}
	
	if(!msg.Post())
	{
		LOG(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), 
			"handle_DB_Data_frmDCS(), send msg error!!!");
		msg.DeleteMessage();
	}
	else
	{
		if(blGidData)
		{
			dcsCounters.cntGidDataToL2++;
		}
		if(blUidData)
		{
			dcsCounters.cntUidDataToCPE++;
		}
	}
}

/*******************************************************************************
* Prototype      : 
* Parameters     :UIDMsg from DCS, begin with tcpEHead
* Return Value   :
* Global Variable: 
* Description    : send to cpe
*******************************************************************************/
void VoiceFSM::handle_DB_UIDMsg_frmDCS(CMessage& msg)
{
	DB_UtDcs_CtrlMsgHeadT* pUidHead = 
		(DB_UtDcs_CtrlMsgHeadT*)(((UINT8*)msg.GetDataPtr())+sizeof(TcpPktHeaderT));
	UINT8 *pDataNew = (UINT8*) (((UINT8*)pUidHead)-1);
	UINT16 nDataLenNew = msg.GetDataLength() - sizeof(TcpPktHeaderT) + 1;

	UINT32 uid = VGetU32BitVal(pUidHead->UID);
	VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByUID(uid);
	if(pCCB==NULL)
	{
		LOG1(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), 
			"handle_DB_UIDMsg_frmDCS(), cannot find CCB with UID[0x%08X]!!!", uid);
		return;
	}
	VoiceTuple tuple = pCCB->getVoiceTuple();
	pDataNew[0] = tuple.Cid; // CID
	msg.SetEID(tuple.Eid); //EID
	msg.SetDataPtr((void*)pDataNew);
	msg.SetDataLength(nDataLenNew);
	msg.SetSrcTid(M_TID_VOICE);
	msg.SetDstTid(M_TID_UTDM);
	msg.SetMessageId(MSGID_DCS_CPE_DOWNLINK_SIGNAL);
	if(!msg.Post())
	{
		LOG(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), 
			"handle_DB_UIDMsg_frmDCS(), send msg error!!!");
		msg.DeleteMessage();
	}
}

/*******************************************************************************
* Prototype      : 
* Parameters     :msg from CPE, begin with CID(default 0) and DBMessagetype
* Return Value   :
* Global Variable: 
* Description    : send to DCS without CID
*******************************************************************************/
void VoiceFSM::handle_DB_Signal_frmCPE(CMessage& msg)
{
	TcpPktHeaderT *pTcpEHead = 
		(TcpPktHeaderT*)  (((UINT8*)msg.GetDataPtr())+1-sizeof(TcpPktHeaderT));
	UINT16 nDataLenNew = msg.GetDataLength() - 1 + sizeof(TcpPktHeaderT);

	VSetU16BitVal(pTcpEHead->PktLen, nDataLenNew);
	VSetU16BitVal(pTcpEHead->UserType, M_TCP_PKT_DGRP_CTRL_USERTYPE);

	msg.SetDataPtr((void*)pTcpEHead);
	msg.SetDataLength(nDataLenNew);
	msg.SetSrcTid(M_TID_VOICE);
	msg.SetDstTid(M_TID_DGRV_LINK);
	msg.SetMessageId(MSGID_VOICE_DCS_MSG);
	if(!msg.Post())
	{
		LOG(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), 
			"handle_DB_Signal_frmCPE(), send msg error!!!");
		msg.DeleteMessage();
	}
}

/*******************************************************************************
* Prototype      : 
* Parameters     :msg from CPE, begin with CID(default 0) 
* Return Value   :
* Global Variable: 
* Description    : send to DCS without CID
*******************************************************************************/
void VoiceFSM::handle_DB_Data_frmCPE(CMessage& msg)
{
	dcsCounters.cntUidDataFromCPE++;
	
	TcpPktHeaderT *pTcpEHead = 
		(TcpPktHeaderT*)  (((UINT8*)msg.GetDataPtr())+1-sizeof(TcpPktHeaderT));
	UINT16 nDataLenNew = msg.GetDataLength() - 1 + sizeof(TcpPktHeaderT);

	VSetU16BitVal(pTcpEHead->PktLen, nDataLenNew);
	VSetU16BitVal(pTcpEHead->UserType, M_TCP_PKT_DGRP_DATA_USERTYPE);

	msg.SetDataPtr((void*)pTcpEHead);
	msg.SetDataLength(nDataLenNew);
	msg.SetSrcTid(M_TID_VOICE);
	msg.SetDstTid(M_TID_DGRV_LINK);
	msg.SetMessageId(MSGID_VOICE_DCS_MSG);
	if(!msg.Post())
	{
		LOG(LOG_WARN, LOGNO(DGRPSRV, EC_L3VOICE_SYS_FAIL), 
			"handle_DB_Data_frmCPE(), send msg error!!!");
		msg.DeleteMessage();
	}
	else
	{
		dcsCounters.cntUidDataToDCS++;
	}
}

DcsCountersT dcsCounters;
/*******************************************************************************
* Prototype      : 
* Parameters     :
* Return Value   :
* Global Variable: 
* Description    : show all counters
*******************************************************************************/
void showDcsCounters()
{
	int i;
	VPRINT("\n-------------------------------------------");
	for(i=0;i<DB_signalMax;i++)
	{
		VPRINT("\n%30s    Rx[%10d]    Tx[%10d]", 
			DcsSignalDictionary[i].name, 
			dcsCounters.cntRxSignal[i],
			dcsCounters.cntTxSignal[i]);
	}
	VPRINT("\n cntUidDataFromDCS[%10d]\t cntUidDataToCPE[%10d]",
		dcsCounters.cntUidDataFromDCS, dcsCounters.cntUidDataToCPE);
	VPRINT("\n cntUidDataFromCPE[%10d]\t cntUidDataToDCS[%10d]",
		dcsCounters.cntUidDataFromCPE, dcsCounters.cntUidDataToDCS);
	VPRINT("\n cntGidDataFromDCS[%10d]\t cntGidDataToL2[%10d]",
		dcsCounters.cntGidDataFromDCS, dcsCounters.cntGidDataToL2);
	VPRINT("\n-------------------------------------------");
}

/*******************************************************************************
* Prototype      : 
* Parameters     :
* Return Value   :
* Global Variable: 
* Description    : reset all counters with zeros
*******************************************************************************/
void clearDcsCounters()
{
	memset(&dcsCounters, 0, sizeof(DcsCountersT));
}

