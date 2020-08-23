#include "log.h"
#include "SAGSignal.h"


UINT16 CSAbisSignal::m_BTSID = 0xffff;
UINT32 CSAbisSignal::m_SAGID = 0xffffffff;

SigIDST	CSAbisSignal::m_SigIDS[InvalidSignal_MSG]=
{
	//EventGroupID					//EventID
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_LAPAGING},			//LAPaging_MSG=0,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_LAPAGINGRSP},		//LAPagingRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DELAPAGING},			//DELAPagingReq_MSG,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DELAPAGINGRSP},		//DELAPagingRsp_MSG,

	{M_MSG_EVENT_GROUP_ID_RESMANAGE,M_MSG_EVENT_ID_ASSGNRESREQ},		//AssignResReq_MSG,
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,M_MSG_EVENT_ID_ASSGNRESRSP},		//AssignResRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,M_MSG_EVENT_ID_RLSRESREQ},			//RlsResReq_MSG,
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,M_MSG_EVENT_ID_RLSRESRSP},			//RlsResRsp_MSG,

	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_RESET},				//Reset_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_RESETACK},			//ResetAck_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_BEATHEART},			//BeatHeart_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_BEATHEARTACK},		//BeatHeartAck_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_CONGESTREQ},			//CongestionCtrlReq_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_CONGESTRSP},			//CongestionCtrlRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_ERRNOTIFYREQ},		//ErrNotifyReq_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_ERRNOTIFYRSP},		//ErrNotifyRsp_MSG,


																		
};

sigTableT	CSAbisSignal::sigTabEventID[M_MSG_EVENT_GROUP_NUM+1]=
{
//���п�����Ϣ
	{0x04,
		{
			InvalidSignal_MSG,
			LAPaging_MSG,		//LA Paging				0x01	���п�����Ϣ	0x00
			LAPagingRsp_MSG,	//LA Paging Response	0x02	���п�����Ϣ	0x00
			DELAPagingReq_MSG,	//De-LA Paging			0x03	���п�����Ϣ	0x00
			DELAPagingRsp_MSG,	//De-LA Paging Response	0x04		
		}
	},
//��Դ������Ϣ
	{0x04,
		{
			InvalidSignal_MSG,
			AssignResReq_MSG,	//Assignmengt transport resource req	0x01	0x01
			AssignResRsp_MSG,	//Assignmengt transport resource rsp	0x02	0x01
			RlsResReq_MSG,		//Release transport resource req		0x03	0x01
			RlsResRsp_MSG,		//Release transport resource rsp		0x04	0x01
		}
	},
//��������Ϣ
	{0x08,
		{
			InvalidSignal_MSG,
			Reset_MSG,				//Reset						0x01		0x02
			ResetAck_MSG,			//Reset ack					0x02		0x02
			ErrNotifyReq_MSG,		//Error notification req	0x03		0x02
			ErrNotifyRsp_MSG,		//Error notification rsp	0x04		0x02
			BeatHeart_MSG,			//beatheart					0x05		0x02
			BeatHeartAck_MSG,		//Beatheart ack				0x06		0x02
			CongestionCtrlReq_MSG,	//Congestion control req	0x07		0x02
			CongestionCtrlRsp_MSG,	//Congestion control req	0x08		0x02
		}
	},
//UE-SAGֱ�Ӵ�����Ϣ
	{0x02,	
		{
			InvalidSignal_MSG,
			UTSAG_L3Addr_MSG,	//L3��ַѰַ��UE-SAG message	0x01		0x03
			UTSAG_UID_MSG,		//UIDѰַ��UE-SAG message		0x02		0x03
		}
	}
};
SignalType	CSAbisSignal::sigTabMsgType[M_MSGTYPE_MAX_VALUE+1]=
{
						//��Ϣ����				��Ϣ����	��Ϣ���	ע��
	InvalidSignal_MSG,	//0x00	reserved

//���п�����Ϣ
	Setup_MSG,			//Setup					0x01	���п�����Ϣ	L3��ַѰַ
	CallProc_MSG,		//Setup ack				0x02	���п�����Ϣ	L3��ַѰַ
	Alerting_MSG,		//Alerting				0x03	���п�����Ϣ	L3��ַѰַ
	Connect_MSG,		//Connect				0x04	���п�����Ϣ	L3��ַѰַ
	ConnectAck_MSG,		//Connect Ack			0x05	���п�����Ϣ	L3��ַѰַ
	Disconnect_MSG,		//Disconnect			0x06	���п�����Ϣ	L3��ַѰַ
	Information_MSG,	//Information			0x07	���п�����Ϣ	L3��ַѰַ
	Release_MSG,		//Release				0x08	���п�����Ϣ	L3��ַѰַ
	ReleaseComplete_MSG,//Release complete		0x09	���п�����Ϣ	L3��ַѰַ
	ModiMediaReq_MSG,	//Modify media type req	0x0a	���п�����Ϣ	L3��ַѰַ
	ModiMediaRsp_MSG,	//Modify media type rsp	0x0b	���п�����Ϣ	L3��ַѰַ
	
	//0x0c--0x0f	reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	
//�л���Ϣ
	HandOverReq_MSG,		//Handover req		0x10	�л���Ϣ	L3��ַѰַ
	HandOverRsp_MSG,		//Handover rsp		0x11	�л���Ϣ	L3��ַѰַ
	HandOverComplete_MSG,	//Handover complete	0x12	�л���Ϣ	L3��ַѰַ

	//0x13--0x1f	reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG,

//�ƶ��Թ�����Ϣ
	Login_MSG,		//Login req					0x20	�ƶ��Թ�����Ϣ	UIDѰַ
	LoginRsp_MSG,	//Login rsp					0x21	�ƶ��Թ�����Ϣ	UIDѰַ
	Logout_MSG,		//Logout					0x22	�ƶ��Թ�����Ϣ	UIDѰַ
	AuthCmdReq_MSG,	//Authentication command	0x23	�ƶ��Թ�����Ϣ	UIDѰַ
	AuthCmdRsp_MSG,	//Authentication rsp		0x24	�ƶ��Թ�����Ϣ	UIDѰַ

	//0x25--0x2f	reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,

//����Ϣ
	MOSmsDataReq_MSG,//MO Sms data req			0x30	����Ϣ	UIDѰַ
	MOSmsDataRsp_MSG,//MO Sms data rsp			0x31	����Ϣ	UIDѰַ
	MTSmsDataReq_MSG,//MT Sms data req			0x32	����Ϣ	UIDѰַ
	MTSmsDataRsp_MSG,//MT Sms data rsp			0x33	����Ϣ	UIDѰַ
	SMSMemAvailReq_MSG,//SMS memory available req	0x34	����Ϣ	UIDѰַ
	SMSMemAvailRsp_MSG,//SMS memory available rsp	0x35	����Ϣ	UIDѰַ
	
};

//����ͳ�Ʊ�
UINT32	CSAbisSignal::RxSignalCounter[InvalidSignal_MSG+1]={0,};
UINT32	CSAbisSignal::TxSignalCounter[InvalidSignal_MSG+1]={0,};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


/*
 *	fill BTSID&SAGID for signal message, 
 *  called when constructing messages sending to tVCR
 */
void CSAbisSignal::SetBTSSAGID(UINT16 BTSID, UINT32 SAGID)
{
	SAbisSignalT* pData = (SAbisSignalT*)GetDataPtr();
	pData->sigHeader.BTS_ID = BTSID;
	pData->sigHeader.SAG_ID = SAGID;
}

/*
 *	�����õ���SAG������Ϣ����Ϣ����
 *	͸����Ϣ����UTSAG_L3Addr_MSG or UTSAG_UID_MSG
 *	�Ƿ���Ϣ����InvalidSignal_MSG
 */
SignalType  CSAbisSignal::ParseMessageFromSAG()
{
	UINT16 msgLen = GetDataLength();
	SAbisSignalT *pSigVCR;

	//��Ϣ���ȼ��,���VCR�ӿ������
	if(msgLen<(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT))) 
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseMessageFromSAG(), VCR ctrl msgLen error!!!");
		return InvalidSignal_MSG;
	}
	
	pSigVCR = (SAbisSignalT*)GetDataPtr();
	UINT8 eventGroutID = pSigVCR->sigHeader.EVENT_GROUP_ID;
	if(eventGroutID>M_MSG_EVENT_GROUP_NUM)
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseMessageFromSAG(), invalid EventGroupID!!!");
		return InvalidSignal_MSG;
	}
	//net order to host oder
	UINT16 eventID = ntohs(pSigVCR->sigHeader.Event_ID);
	if(eventID>sigTabEventID[eventGroutID].nMaxSigIndex)
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseMessageFromSAG(), invalid EVENT_ID!!!");
		return InvalidSignal_MSG;
	}
	else
		return sigTabEventID[eventGroutID].sigTypeArr[eventID];
}

/*
 *	�����õ���SAG������Ϣ����Ϣ����
 *	͸����Ϣ����UTSAG_L3Addr_MSG or UTSAG_UID_MSG
 *	�Ƿ���Ϣ����InvalidSignal_MSG
 */
SignalType CSAbisSignal::ParseMessageToSAG()
{
	UINT16 msgLen = GetDataLength();
	SAbisSignalT *pSigVCR;
	
	//��Ϣ���ȼ��,���VCR�ӿ������
	if(msgLen<(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT))) 
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseMessageToSAG(), VCR ctrl msgLen error!!!");
		return InvalidSignal_MSG;
	}
	
	pSigVCR = (SAbisSignalT*)GetDataPtr();
	UINT8 eventGroutID = pSigVCR->sigHeader.EVENT_GROUP_ID;
	if(eventGroutID>M_MSG_EVENT_GROUP_NUM)
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseMessageToSAG(), invalid EventGroupID!!!");
		return InvalidSignal_MSG;
	}
	//net order to host oder
	UINT16 eventID = ntohs(pSigVCR->sigHeader.Event_ID);
	if(eventID>sigTabEventID[eventGroutID].nMaxSigIndex)
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseMessageToSAG(), invalid EVENT_ID!!!");
		return InvalidSignal_MSG;
	}
	else
		return sigTabEventID[eventGroutID].sigTypeArr[eventID];
}

/*
 *	�����õ���SAG����͸����Ϣ����Ϣ����
 */
SignalType	CSAbisSignal::ParseUTSAGMsgFromSAG()
{
	UINT16 msgLen = GetDataLength();
	SAbisSignalT *pSigVCR;

	//��Ϣ����Ӧ�ò�С�� ����ͨ��ͷ�����ȣ�UID/L3Addr���ȣ�MessageType����
	if(msgLen<(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(UINT32)+sizeof(UINT8)))
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseUTSAGMsgFromSAG(), msg len error!!!");
		return InvalidSignal_MSG;
	}

	//EVENT_ID
	pSigVCR = (SAbisSignalT*)GetDataPtr();
	UINT16 eventID = ntohs(pSigVCR->sigHeader.Event_ID);	//net order to host oder
	if(M_MSG_EVENT_ID_UTSAG_L3ADDR!=eventID && M_MSG_EVENT_ID_UTSAG_UID!=eventID)
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseUTSAGMsgFromSAG(), invalid EVENT_ID!!!");
		return InvalidSignal_MSG;
	}

	//MessageType
	UINT8 msgType = ( (M_MSG_EVENT_ID_UTSAG_L3ADDR==eventID) ?
					pSigVCR->sigPayload.UTSAG_Payload_L3Addr.msgType: 
					pSigVCR->sigPayload.UTSAG_Payload_Uid.msgType );
	if(msgType>M_MSGTYPE_MAX_VALUE)
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseUTSAGMsgFromSAG(), invalid MessageType!!!");
		return InvalidSignal_MSG;		
	}
	return sigTabMsgType[msgType];
}

/*
 *	�����õ���SAG����͸����Ϣ����Ϣ����
 */
SignalType	CSAbisSignal::ParseUTSAGMsgToSAG()
{
	UINT16 msgLen = GetDataLength();
	SAbisSignalT *pSigVCR;

	//��Ϣ����Ӧ�ò�С�� ����ͨ��ͷ�����ȣ�UID/L3Addr���ȣ�MessageType����
	if(msgLen<(sizeof(TcpPktHeaderT)+sizeof(SigHeaderT)+sizeof(UINT32)+sizeof(UINT8)))
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseUTSAGMsgToSAG(), msg len error!!!");
		return InvalidSignal_MSG;
	}
	
	//EVENT_ID
	pSigVCR = (SAbisSignalT*)GetDataPtr();
	UINT16 eventID = ntohs(pSigVCR->sigHeader.Event_ID);	//net order to host oder
	if(M_MSG_EVENT_ID_UTSAG_L3ADDR!=eventID && M_MSG_EVENT_ID_UTSAG_UID!=eventID)
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseUTSAGMsgToSAG(), invalid EVENT_ID!!!");
		return InvalidSignal_MSG;
	}
	
	//MessageType
	UINT8 msgType = ( (M_MSG_EVENT_ID_UTSAG_L3ADDR==eventID) ?
		pSigVCR->sigPayload.UTSAG_Payload_L3Addr.msgType: 
	pSigVCR->sigPayload.UTSAG_Payload_Uid.msgType );
	if(msgType>M_MSGTYPE_MAX_VALUE)
	{
		LOG(LOG_DEBUG3, 0, "CSAbisSignal::ParseUTSAGMsgToSAG(), invalid MessageType!!!");
		return InvalidSignal_MSG;		
	}
	return sigTabMsgType[msgType];	
}

/*
 *	������Ϣ����������Ϣ��EVENT GROUP ID(net order)��EVENT ID
 */
void CSAbisSignal::SetSigIDS(SignalType sigType)
{
	SAbisSignalT *pSigVCR = (SAbisSignalT*)GetDataPtr();
	
	if(UTSAG_L3Addr_MSG==sigType)
	{
		pSigVCR->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_UTSAG;
		pSigVCR->sigHeader.Event_ID = htons(M_MSG_EVENT_ID_UTSAG_L3ADDR);
		return;
	}

	if(UTSAG_UID_MSG==sigType)
	{
		pSigVCR->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_UTSAG;
		pSigVCR->sigHeader.Event_ID = htons(M_MSG_EVENT_ID_UTSAG_UID);
		return;
	}
	
	pSigVCR->sigHeader.EVENT_GROUP_ID = m_SigIDS[sigType].EventGroupID;
	pSigVCR->sigHeader.Event_ID = htons(m_SigIDS[sigType].EventID);
}

/*
 *	����������Ϣͷ�ĳ����ֶ�
 */
void CSAbisSignal::SetSigHeaderLengthField(UINT16 len)
{
	SAbisSignalT *pSigVCR = (SAbisSignalT*)GetDataPtr();
	pSigVCR->sigHeader.Length = htons(len);
}

void CSAbisSignal::setSigTcpPKTHeaderTail(UINT16 SPC_Code, UINT16 DPC_Code, UINT16 TcpPKTLen)
{
	SAbisSignalT* pSigBuf = (SAbisSignalT*)GetDataPtr();
	pSigBuf->tcpPktHeader.DPC_Code = htons(DPC_Code);
	pSigBuf->tcpPktHeader.SPC_Code = htons(SPC_Code);
//	pSigBuf->tcpPktHeader.HeadFlag = htons(M_TCP_PKT_BEGIN_FLAG);
	pSigBuf->tcpPktHeader.PktLen = htons(TcpPKTLen);
	pSigBuf->tcpPktHeader.UserType = htons(M_TCP_PKT_SABIS1_USERTYPE);
	pSigBuf->tcpPktHeader.TTL = htons(32);
	//����Tcp Packet �������
	UINT8* ptmp = (UINT8*)pSigBuf + TcpPKTLen;
	*((UINT16*)ptmp) = htons(M_TCP_PKT_END_FLAG);
}

/*
 *	����SAG�յ���������Ϣ�Ǹ���L3Addr����UID����CCB
 *	return value: true when success, false when fail
 *	Parameters:
 *	how--output parameter,0:use UID,1:use L3Addr,2:need not find CCB
 *	Uid_L3addr--output paramter, when how==0, Uid_L3addr = UID, when how==1, Uid_L3addr = L3Addr
 */
bool CSAbisSignal::SAGSignalHowToFindCCB(UINT8& how, UINT32& Uid_L3addr)
{
	enum{ USE_UID, USE_L3ADDR, NEEDNOTFINDCCB };
	
	SAbisSignalT* pData = (SAbisSignalT*)GetDataPtr();

	SignalType sigType = ParseMessageFromSAG();
	switch(sigType) 
	{

	//================================================================
	case LAPaging_MSG:
		how = USE_UID;
		Uid_L3addr = pData->sigPayload.LAPaging.UID;
		break;
	//case LAPagingRsp_MSG:�������յ�
	//case DELAPagingReq_MSG:��ע��״̬��,��͸�������Բ�����CCB
	//case DELAPagingRsp_MSG:�������յ�
	case AssignResReq_MSG:
		how = USE_UID;
		Uid_L3addr = pData->sigPayload.AssignResReq.UID;
		break;
	case AssignResRsp_MSG:
		how = USE_UID;
		Uid_L3addr = pData->sigPayload.AssignResRsp.UID;
		break;
	case RlsResReq_MSG:
		how = USE_L3ADDR;
		Uid_L3addr = pData->sigPayload.RlsResReq.L3addr;
		break;
	case RlsResRsp_MSG:
		how = USE_L3ADDR;
		Uid_L3addr = pData->sigPayload.RlsResRsp.L3addr;
		break;
	//case Reset_MSG:��ע��״̬������͸�������Բ�����CCB
	//case ResetAck_MSG:��ע��״̬������͸�������Բ�����CCB
	//case BeatHeart_MSG:��ע��״̬������͸�������Բ�����CCB
	//case BeatHeartAck_MSG:��ע��״̬������͸�������Բ�����CCB
	//case CongestionCtrlReq_MSG:��ע��״̬������͸�������Բ�����CCB
	//case CongestionCtrlRsp_MSG:��ע��״̬������͸�������Բ�����CCB
	//case ErrNotifyReq_MSG:��ע��״̬������͸�������Բ�����CCB
	case ErrNotifyRsp_MSG:
		how = USE_UID;
		Uid_L3addr = pData->sigPayload.ErrNotifyRsp.Uid;
		break;

	//===========================================================
/*
	͸������Ϣ������뱾����
	case UTSAG_L3Addr_MSG:
		how = USE_L3ADDR;
		Uid_L3addr = pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr;
		break;
	case UTSAG_UID_MSG:
		how = USE_UID;
		Uid_L3addr = pData->sigPayload.UTSAG_Payload_Uid.Uid;
		break;
*/
	//===========================================================

	case InvalidSignal_MSG:
	default:
		LOG(LOG_DEBUG3, 0, "invalid signal");
		how = NEEDNOTFINDCCB;
		Uid_L3addr = NO_L3ADDR;
		return false;
	}
	Uid_L3addr = ntohl(Uid_L3addr);
	return true;
}

//�������ͳ�Ƽ�����
void CSAbisSignal::ClearSignalCounters()
{
	for(int i=0;i<=InvalidSignal_MSG;i++)
		RxSignalCounter[i] = TxSignalCounter[i] = 0;
}

//��������ͳ��
void CSAbisSignal::DoCountRxSignal()
{
	SignalType sigType = ParseMessageFromSAG();
	++RxSignalCounter[sigType];
	if(UTSAG_L3Addr_MSG==sigType || UTSAG_UID_MSG==sigType)
	{
		sigType = ParseUTSAGMsgFromSAG();
		++RxSignalCounter[sigType];
	}
}

//��������ͳ��
void CSAbisSignal::DoCountTxSignal()
{
	SignalType sigType = ParseMessageToSAG();
	++TxSignalCounter[sigType];
	if(UTSAG_L3Addr_MSG==sigType || UTSAG_UID_MSG==sigType)
	{
		sigType = ParseUTSAGMsgToSAG();
		++TxSignalCounter[sigType];
	}
}

UINT16 CSAbisSignal::getSPC()
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)GetDataPtr();
	return ntohs(pSigBufRcv->tcpPktHeader.SPC_Code);	 
}

UINT16 CSAbisSignal::getDPC()
{
	SAbisSignalT* pSigBufRcv = (SAbisSignalT*)GetDataPtr();
	return ntohs(pSigBufRcv->tcpPktHeader.DPC_Code);	
}


