/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    SAGSignal.h
*
* DESCRIPTION: 
*		ģ��SAG��SAbis+����ṹ����
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-12-06   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__SAGSIGNAL_H
#define	__SAGSIGNAL_H

#include "SAGCommon.h"
#include "SAbis1_1Struct.h"

#define M_MAX_SIGNAL_LENGTH		(1024)

typedef struct tagSigTable
{	
	UINT8		nMaxSigIndex;		//sigTypeArr�����кϷ����������Index
	SignalType	sigTypeArr[100];	//��EventIDΪ������������Ϣ�����
}sigTableT;

typedef struct tagSigIDS
{
	UINT8	EventGroupID;
	UINT16	EventID;
}SigIDST;
//////////////////////////////////////////////////////////////////////////
//VCR�ӿڵ�����
class CSAbisSignal
{
private:
	UINT8	m_DataBuffer[M_MAX_SIGNAL_LENGTH];
	UINT16	m_DataLen;
public:
    CSAbisSignal():m_DataLen(0){}
	CSAbisSignal(const CSAbisSignal& signal);
    ~CSAbisSignal(){}
	
	UINT16 GetDataLength(){ return m_DataLen;}
	void SetDataLength(UINT16 len){ m_DataLen = len;}
	UINT8* GetDataPtr(){ return m_DataBuffer;}

	SignalType  ParseMessageFromSAG();		//�����õ���SAG������Ϣ����Ϣ����
	SignalType	ParseMessageToSAG();		//�����õ�SAG������Ϣ����Ϣ����
	SignalType	ParseUTSAGMsgFromSAG();		//�����õ���SAG����͸����Ϣ����Ϣ����
	SignalType	ParseUTSAGMsgToSAG();		//�����õ�SAG����͸����Ϣ����Ϣ����
	
	//��������ͷ����BTSID��SAGID
	void SetBTSSAGID(UINT16 BTSID, UINT32 SAGID);
	//������Ϣ����������Ϣ��EVENT GROUP ID(net order)��EVENT ID,
	void SetSigIDS(SignalType sigType);
	//��������ͷ����Length�ֶ�
	void SetSigHeaderLengthField(UINT16 len);
	//��������ͷ��TcpPktHeader�ͽ������
	void setSigTcpPKTHeaderTail(UINT16 SPC_Code, UINT16 DPC_Code, UINT16 TcpPKTLen);
	//����SAG�յ���������Ϣ�Ǹ���L3Addr����UID����CCB
	bool SAGSignalHowToFindCCB(UINT8& how, UINT32& UID_SAG);
	
	static void ClearSignalCounters();
	void DoCountRxSignal();
	void DoCountTxSignal();
	
	static UINT32	m_SAGID;	//net order
	static UINT16	m_BTSID;	//net order
	
	//�����EVENT_ID��EVENTGROUP_ID���ձ�
	static SigIDST	m_SigIDS[InvalidSignal_MSG];	//host order
	//����EventID���ձ�
	static sigTableT	sigTabEventID[M_MSG_EVENT_GROUP_NUM+1];
	//͸������MessageType���ձ�
	static SignalType	sigTabMsgType[M_MSGTYPE_MAX_VALUE+1];
	
	//����ͳ�Ʊ�
	static UINT32	RxSignalCounter[InvalidSignal_MSG+1];
	static UINT32	TxSignalCounter[InvalidSignal_MSG+1];

	UINT16 getSPC();
	UINT16 getDPC();

};

#endif /* __TSAGSIGNAL_H */


