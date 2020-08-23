/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:  implementation of VAC simulator module on CPE/BTS
 *
 *
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/02/2006   Fengbing      Initial file creation.
 *---------------------------------------------------------------------------*/
#pragma warning(disable:4786)

#include <stdio.h>

#include "L3L2MessageID.h"
#include "BTSVACSim.h"

extern UINT16 BtsMacAddr[3];

typedef struct
{
	UINT16 CPEMacAddr[3];
	UINT16 BtsMacAddr[3];
	UINT16 Protocol;
	UINT16 DstTID;
	UINT16 SrcTID;
	UINT16 MsgID;
	UINT16 Length;
}T_SecondHeader;

typedef struct
{
	T_SecondHeader	hdr;
	UINT8	VoiceData[10];
}EthNet_Snd_BufT;
/*
list<UINT16> lst_FreeEthNetBuf;

EthNet_Snd_BufT	EthNet_Snd_Buf[300];	//��̫�������������ڴ滺����
*/
CBTSVACSim * CBTSVACSim::Instance = NULL;


/*****************************************************************************
 *
 *   Method:     CBTSVACSim::CBTSVACSim()
 *
 *   Description:  Constructor, register to ComEntity for VAC task
 *
 *   Parameters:  None
 *
 *   Returns:  None
 *
 *****************************************************************************
 */
CBTSVACSim::CBTSVACSim()
{
	RegisterEntity(false);
/*
	memcpy(m_szName, M_TASK_VACSIM_TASKNAME, strlen( M_TASK_VACSIM_TASKNAME ) );
	m_szName[strlen( M_TASK_VACSIM_TASKNAME )] = 0;

	m_uPriority = M_TASK_VACSIM_PRIORITY;
	m_uOptions = M_TASK_VACSIM_OPTION;
	m_uStackSize = M_TASK_VACSIM_STACKSIZE;

	m_iMsgQMax = M_TASK_VACSIM_MAXMSG;
	m_iMsgQOption = M_TASK_VACSIM_MSGOPTION;
*/
}


/*****************************************************************************
 *
 *   Method:     CBTSVACSim::GetInstance()
 *
 *   Description:  Singleton
 *                
 *   Parameters:  none
 *
 *   Returns:  None
 *
 *****************************************************************************
 */
CBTSVACSim *CBTSVACSim::GetInstance()
{
    if ( NULL == Instance )
    {
        Instance = new CBTSVACSim;
    }
    return Instance;
}

TID CBTSVACSim::GetEntityId() const
{ 
	return M_TID_VAC;
}

/*
 *	��UTV�յ�VACSetupReq��ΪVACSetupNoti����BTS
 *	��UTV�յ�VACSetupRsp����BTS
 *	��UTV�յ�VACReleaseCMD��ΪVACReleaseNoti����BTS
 *	��UTV�յ�VACModifyReq��ΪVACModifyNoti����BTS
 *	��UTV�յ�VACModifyRsp����BTS
 *	��UTV�յ�VACStart����CPEL1
 *	��UTV�յ�VACStop����CPEL1
 *	
 *	��CPEL1�յ����������͸�BTS��VAC
 *	��BTSBridge�յ������������͸�CPEL1
 *
 *	��UTV�յ���������Ϣ���͸�BTS
 *	��BTSBridge�յ���������Ϣ���͸�UTV
 *	��BTSBridge�յ���VAC������Ϣ���͸�UTV
 */ 



/*
bool CBTSVACSim::Initialize()
{
	
	int i;

	//��ʼ����̫�����ͻ�����
	for(i=0;i<300;i++)
	{
		lst_FreeEthNetBuf.push_back(i);
		
		EthNet_Snd_Buf[i].hdr.BtsMacAddr[0] = BtsMacAddr[0];
		EthNet_Snd_Buf[i].hdr.BtsMacAddr[1] = BtsMacAddr[1];
		EthNet_Snd_Buf[i].hdr.BtsMacAddr[2] = BtsMacAddr[2];
		EthNet_Snd_Buf[i].hdr.DstTID = M_TID_UTVAC;
		EthNet_Snd_Buf[i].hdr.SrcTID = M_TID_VAC;
		EthNet_Snd_Buf[i].hdr.Length = 0;
		EthNet_Snd_Buf[i].hdr.Protocol = 0x9998;
		EthNet_Snd_Buf[i].hdr.Length = 10;

	}

	//��ʼ��VAC
	m_SN = 0;
	//�����������ݰ�����,�����������ݰ�ÿ10ms���͸�tVoice����
	UplinkVoiceDataCount=0;
	//�����������ݰ�����,�����������ݰ�ÿ10msÿ�������˿�ֻ����һ���������ݰ���CPE
	for(i=0;i<100;i++)
	{
		lstFreeDownklinkVACSession.push_back(i);
		DownlinkVACSession[i].curTx = DownlinkVACSession[i].curFree = 0;
	}
	BTreeByTuple.clear();
	
	//framework init
	UINT8 ucInit = CBizTask::Initialize();
	if ( false == ucInit)
	{
		//Delete MsgQueue
		delete m_pMsgQueue;
	       return false;
	}
	return true;
}
*/

/* 
����VAC͸������͸�tVoice
�����������ݻ�������,�ȴ�10ms��ʱ����ʱ���͸�tVoice
�����յ���VAC������Ϣ���͸�tVoice

�յ�10ms��ʱ����ʱ��Ϣ��tVoice����һ�η��Ͷ��������,��CPE����һ��������

�����������ݰ����ݷֳɸ���������10ms�������ݰ�����дEID���͸�UTVAC
����VAC͸������͸�UTVAC
��tVoice�յ���VAC������Ϣ�任���͸�UTVAC
*/
#define MSGID_10MSTIMER_TIMEOUT (0x8999)


//bool CBTSVACSim::ProcessComMessage(CComMessage *msg)
bool CBTSVACSim::PostMessage(CComMessage* msg, SINT32 timeout, bool isUrgent)
{
	CComMessage* pMsgRsp = NULL;
	UINT8* pDataRsp = NULL;
	UINT8* pData;
	UINT16 msgID = msg->GetMessageId();
/*
	if(MSGID_10MSTIMER_TIMEOUT==msgID)	//10ms timer timeout
	{
		//�����������ݰ�10ms��ʱ���ʹ���
		DownLinkVoiceDataTX();
		//�����������ݰ�10ms��ʱ���ʹ���
		UplinkVoiceDataTX();
		m_SN++;
		UplinkVoiceDataCount = 0;
	}

	VoiceTuple tuple;
	if(MSGID_VAC_RLS_CMD==msgID || MSGID_VAC_RLS_NOTIFY==msgID)
	{
		//deallocate vac session for downlink
		tuple.Eid = msg->GetEID();
		tuple.Cid = 0;			//+++Ŀǰֻ��һ���˿�
		DeAllocDownVACSession(tuple);
	}
*/

	UINT16 srcTID = msg->GetSrcTid();
	//if(M_TID_VOICE==srcTID || M_TID_VCR==srcTID || M_TID_VDR==srcTID)
	if(M_TID_VOICE==srcTID)
	{	//������Ϣ
		switch(msgID) 
		{
//=========================================
		case MSGID_VOICE_VAC_DATA:		//Downlink Voice Data Message:
			msg->Destroy();
			//ģ��VAC�������������ݰ�
			return true;
//=========================================
		case MSGID_VOICE_MAC_PROBEREQ: //(0x3A20)	//probe req,paging only
			pMsgRsp = new (this, 2) CComMessage;
			pMsgRsp->SetDstTid(M_TID_VOICE);
			pMsgRsp->SetSrcTid(M_TID_VAC);
			pMsgRsp->SetMessageId(MSGID_MAC_VOICE_PROBERSP);
			pMsgRsp->SetDataLength(2);
			pMsgRsp->SetEID(msg->GetEID());
			pDataRsp = (UINT8*)pMsgRsp->GetDataPtr();
			pData = (UINT8*)msg->GetDataPtr();
			pDataRsp[0] = pData[0];
			pDataRsp[1] = 0;
			if ( ! CComEntity::PostEntityMessage(pMsgRsp))
			{
				pMsgRsp->Destroy();
			}
			msg->Destroy();
			return true;
			break;
//#define MSGID_MAC_VOICE_PROBERSP (0x3A21)	//probe rsp,
//#define MSGID_CONGESTION_REPORT	 (0x3A23)	//congestion report, answer of congestion request from L2
		case MSGID_CONGESTION_REQUEST: //(0x3A22)	//congestion request to L2
			pMsgRsp = new (this, 1) CComMessage;
			pMsgRsp->SetDstTid(M_TID_VOICE);
			pMsgRsp->SetSrcTid(M_TID_VAC);
			pMsgRsp->SetMessageId(MSGID_CONGESTION_REPORT);
			pMsgRsp->SetDataLength(1);
			pDataRsp = (UINT8*)pMsgRsp->GetDataPtr();
			pDataRsp[0] = 0;
			if ( ! CComEntity::PostEntityMessage(pMsgRsp))
			{
				pMsgRsp->Destroy();
			}
			msg->Destroy();
			return true;
			break;
//=========================================			
//������ϢID�任
		case MSGID_VAC_SETUP_CMD:		//Setup VAC Session setup Command
			msg->SetMessageId(MSGID_VAC_SETUP_NOTIFY);
			break;
		case MSGID_VAC_RLS_CMD:			//VAC Session Release Command
			msg->SetMessageId(MSGID_VAC_RLS_NOTIFY);
			break;
		case MSGID_VAC_MODIFY_CMD:		//VAC session Modify Command
			msg->SetMessageId(MSGID_VAC_MODIFY_NOTIFY);
			break;
		case MSGID_L3_VAC_SETUPRSP:
			msg->SetMessageId(MSGID_VAC_SETUP_RSP);
			break;
//=========================================		
		default:
			;
		}
		//VAC������Ϣ��VAC͸������͸�CPE
		msg->SetSrcTid(M_TID_VAC);
		msg->SetDstTid(M_TID_UTVAC);
/*
		if(MSGID_VAC_SETUP_CMD==msgID || MSGID_VAC_SETUP_RSP==msgID)
		{
			//allocate vac session for downlink
			tuple.Eid = msg->GetEID();
			tuple.Cid = 0;			//+++Ŀǰֻ��һ���˿�
			AllocDownVACSession(tuple);
		}
*/		
	}
	else
	{	//������Ϣ
		if(MSGID_VAC_VOICE_DATA==msgID)
		{
			//ģ��VAC����������������Ϣ
			//handleUpLinkVoiData(msg);
			msg->Destroy();
			return true;
		}
		else
		{ //VAC������Ϣ��VAC͸����������Ϣ
			msg->SetSrcTid(M_TID_VAC);
			msg->SetDstTid(M_TID_VOICE);
		}
	}

	if ( ! CComEntity::PostEntityMessage(msg, timeout, isUrgent))
	{
		msg->Destroy();
	}

	return true;
}
/*
bool CBTSVACSim::AllocDownVACSession(VoiceTuple tuple)
{
	map<VoiceTuple, UINT32>::iterator it;
	if((it=BTreeByTuple.find(tuple))==BTreeByTuple.end())
	{
		int j;
		if(lstFreeDownklinkVACSession.empty())
		{
			printf("\nCannot Alloc free VAC session!!!\n");
			return false;
		}
		else
		{
			j = *(lstFreeDownklinkVACSession.begin());
			lstFreeDownklinkVACSession.pop_front();
			
			DownlinkVACSession[j].curFree = DownlinkVACSession[j].curTx = 0;
			DownlinkVACSession[j].EID = tuple.Eid;
			DownlinkVACSession[j].CID = tuple.Cid;
			BTreeByTuple.insert(map<VoiceTuple, UINT32>::value_type(tuple, j));
		}
	}
	else
	{
		UINT32 index = (*it).second;
		DownlinkVACSession[index].curFree = DownlinkVACSession[index].curTx = 0;
	}
	
	return true;
}

bool CBTSVACSim::DeAllocDownVACSession(VoiceTuple tuple)
{
	map<VoiceTuple, UINT32>::iterator it;
	if((it=BTreeByTuple.find(tuple))!=BTreeByTuple.end())
	{
		UINT32 index = (*it).second;
		DownlinkVACSession[index].curFree = DownlinkVACSession[index].curTx = 0;
		BTreeByTuple.erase(it);
		lstFreeDownklinkVACSession.push_back(index);
	}
	return true;
}

void CBTSVACSim::handleUpLinkVoiData(CComMessage* msg)
{
	//�յ�����������еȴ�10ms��ʱ����ʱ���͸�tVoice
	UplinkVoiceDataBuf[UplinkVoiceDataCount].EID = htonl(msg->GetEID());	//net order
	UplinkVoiceDataBuf[UplinkVoiceDataCount].CID = 0;		//+++Ŀǰֻ��һ���˿�
	UplinkVoiceDataBuf[UplinkVoiceDataCount].SN = m_SN;		//SN
	UplinkVoiceDataBuf[UplinkVoiceDataCount].Length = 10;	//ֻ֧��729
	//��������
	memcpy( UplinkVoiceDataBuf[UplinkVoiceDataCount].voiceData,
			msg->GetDataPtr(),
			10 );
	++UplinkVoiceDataCount;
}

void CBTSVACSim::handleDownLinkVoiData(CComMessage* msg)
{
	//����Ϣ�еĶ���������ֽ�洢����������session�Ļ����У��ȴ�10ms��ʱ����ʱ���͸�CPE
	UINT8* pData = (UINT8*)msg->GetDataPtr();
	UINT8 nCount = pData[0];
	if ( (nCount*sizeof(VACVoiceDataT)+1) != msg->GetDataLength() ) 
	{
		printf("\nCBTSVACSim::handleDownLinkVoiData, msg DataLength error!!!\n");
		return;
	}
	VACVoiceDataT* pVoiceIE = (VACVoiceDataT*)&pData[1];
	VoiceTuple tuple;
	int i, index;
	map<VoiceTuple, UINT32>::iterator it;
	for(i=0;i<nCount;i++)
	{
		tuple.Eid = ntohl(pVoiceIE->EID);
		tuple.Cid = pVoiceIE->CID;
		it = BTreeByTuple.find(tuple);
		if(BTreeByTuple.end()==it)
			continue;
		else
		{
			index = (*it).second;
			DownlinkVACSessionT* pDownSession = &DownlinkVACSession[index];
			//�洢
			memcpy( (void*)pDownSession->DownlinkVoiceDataBuf[pDownSession->curFree].voiceData,
					(void*)pVoiceIE->voiceData,
					10 );
			pDownSession->DownlinkVoiceDataBuf[pDownSession->curFree].SN = pVoiceIE->SN;
			//�ƶ�freeָ��
			++pDownSession->curFree;
			if(JITTER_BUF_SIZE==pDownSession->curFree)
				pDownSession->curFree = 0;
		}

		pVoiceIE++;
	}
}

//�����������ݰ�10ms��ʱ���ʹ���
void CBTSVACSim::UplinkVoiceDataTX()
{
	int toSend, sent;
	CComMessage* pComMsg;

	toSend = UplinkVoiceDataCount;
	sent = 0;
	while(toSend>80)
	{
		pComMsg = new (this, 2+sizeof(VACVoiceDataT)*80) CComMessage;
		if(pComMsg==NULL)
		{
			printf("\nCBTSVACSim::UplinkVoiceDataTX, new CComMessage failed, error!!!!\n");
			return;
		}
		
		pComMsg->SetSrcTid(M_TID_VAC);
		pComMsg->SetDstTid(M_TID_VOICE);
		pComMsg->SetMessageId(MSGID_VAC_VOICE_DATA);
		pComMsg->SetDataLength(2+sizeof(VACVoiceDataT)*80);

		UINT8* pData = (UINT8*)pComMsg->GetDataPtr();
		pData[0] = 1;
		pData[1] = 80;
		memcpy( (void*)&pData[2], 
				(void*)&UplinkVoiceDataBuf[sent],
				sizeof(VACVoiceDataT)*80 );

		toSend -= 80;
		sent += 80;
		
		if ( ! CComEntity::PostEntityMessage(pComMsg))
		{
			pComMsg->Destroy();
		}

	}

	pComMsg = new (this, 2+sizeof(VACVoiceDataT)*toSend) CComMessage;
	if(pComMsg==NULL)
	{
		printf("\nCBTSVACSim::UplinkVoiceDataTX, new CComMessage failed, error!!!!\n");
		return;
	}
	
	pComMsg->SetSrcTid(M_TID_VAC);
	pComMsg->SetDstTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_VAC_VOICE_DATA);
	pComMsg->SetDataLength(2+sizeof(VACVoiceDataT)*toSend);
	
	UINT8* pData = (UINT8*)pComMsg->GetDataPtr();
	pData[0] = 1;
	pData[1] = toSend;
	memcpy( (void*)&pData[2], 
		(void*)&UplinkVoiceDataBuf[sent],
		sizeof(VACVoiceDataT)*toSend );
	
	if ( ! CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
	}

}
//�����������ݰ�10ms��ʱ���ʹ���
void CBTSVACSim::DownLinkVoiceDataTX()
{
	if(BTreeByTuple.empty())
		return;
	map<VoiceTuple, UINT32>::iterator start, end, it;
	start = BTreeByTuple.begin();
	end = BTreeByTuple.end();
	for(it=start;it!=end;it++)
	{
		UINT16 index = (*it).second;
		if(DownlinkVACSession[index].curFree!=DownlinkVACSession[index].curTx)
		{ //���������ݰ�����

		}
	}
}
*/

/*

  typedef struct
  {
  UINT16 CPEMacAddr[3];
  UINT16 BtsMacAddr[3];
  UINT16 Protocol;
  UINT16 DstTID;
  UINT16 SrcTID;
  UINT16 MsgID;
  UINT16 Length;
  }T_SecondHeader;

typedef struct
{
	T_SecondHeader	hdr;
	UINT8	VoiceData[10];
}EthNet_Snd_BufT;


typedef struct tagDownlinkVACSession
{
	UINT32	EID;
	UINT8	CID;
	DownlinkVoiceDataT	DownlinkVoiceDataBuf[JITTER_BUF_SIZE];	//�����������ݰ��������ÿ��(EID,CID)������60ms���������ݰ�
	UINT8	curTx;
	UINT8	curFree;
}DownlinkVACSessionT;


  msg->SetDataLength(msg->GetDataLength() + sizeof(T_SecondHeader));
  
	if (! mv643xxRecvMsgFromEB ( (char*)msg->GetDataPtr(),       //Data to send
								(UINT16)msg->GetDataLength(),   //Data length
								CUTBridge::EBFreeMsgCallBack,        //function.
								(UINT32)msg                     //ComMessage ptr.
								))
	{
		msg->Destroy();
	}
	
	  break;

*/


