/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    CallSignalMsg.h
*
* DESCRIPTION: 
*		����������Ϣ�࣬����SabisA+������Ϣ�ӿ�
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2006-09-06  fengbing  ������������͸��������callticket,callticketAck,SRQ,SRP,SRC,
*                         ��Ӧ�Ķ�����ͳ�ƣ�����������ӿ���ʱ���䣬������������Ҳ���м���
*   2005-9-14   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef __CallSignalMsg_H__
#define __CallSignalMsg_H__

#include "Message.h"
#include "voiceCommon.h"
#include "voice_msgs_struct.h"
#include "cpe_msgs_struct.h"

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
class CMsg_Signal_VCR:public CMessage
{
public:
    CMsg_Signal_VCR(){}
    CMsg_Signal_VCR(const CMessage& Msg):CMessage( Msg ){}
	CMsg_Signal_VCR(const CMsg_Signal_VCR& Msg):CMessage( Msg ){}
    ~CMsg_Signal_VCR(){}
	
	bool CreateMessage(CComEntity& Entity, UINT32 uDataSize);
	bool Post();
	void SetPayloadLength(UINT32 len){ SetDataLength(len);}	//����payload�ĳ���

	SignalType  ParseMessageFromSAG();		//�����õ���SAG������Ϣ����Ϣ����
	SignalType	ParseMessageToSAG();		//�����õ���SAG������Ϣ����Ϣ����
	SignalType	ParseUTSAGMsgFromSAG();		//�����õ���SAG����͸����Ϣ����Ϣ����
	SignalType	ParseUTSAGMsgToSAG();		//�����õ���SAG����͸����Ϣ����Ϣ����
	bool isUTSAGMsg();	//�ж��Ƿ�Ϊ͸������

	//��������ͷ����BTSID��SAGID
	void SetBTSSAGID();
	//������Ϣ����������Ϣ��EVENT GROUP ID(net order)��EVENT ID,
	void SetSigIDS(SignalType sigType);
	//��������ͷ����Length�ֶ�
	void SetSigHeaderLengthField(UINT16 len);
	//������SAG�յ���������Ϣ�Ǹ���L3Addr����UID����CCB
	bool SAGSignalHowToFindCCB(UINT8& how, UINT32& UID_SAG);

	static void ClearSignalCounters();
	void DoCountRxSignal();
	void DoCountTxSignal();

	static UINT32	m_SAGID;
	static UINT16	m_BTSID;
	static char m_sigName[InvalidSignal_MSG+1][40];	//signal name

	//�����EVENT_ID��EVENTGROUP_ID���ձ�
	static SigIDST	m_SigIDS[InvalidSignal_MSG+1];	//host order
	//����EventID���ձ�
	static sigTableT	sigTabEventID[M_MSG_EVENT_GROUP_NUM+1];
	//͸������MessageType���ձ�
	static SignalType	sigTabMsgType[M_MSGTYPE_MAX_VALUE+1];

	//����ͳ�Ʊ�
#define MAX_PERF_COUNTER_SIGNAL    UTSAG_UID_MSG	
	static UINT32	RxSignalCounter[InvalidSignal_MSG+1];	//���һ���ǷǷ��������
	static UINT32	TxSignalCounter[InvalidSignal_MSG+1];	//���һ���ǷǷ��������
protected:
	UINT16 GetDefaultMsgId() const { return 1; }

};
//////////////////////////////////////////////////////////////////////////
//VAC�ӿ�͸������
class CMsg_UTSAGSignal_VAC : public CMessage
{
public:
    CMsg_UTSAGSignal_VAC(){}
    CMsg_UTSAGSignal_VAC(const CMessage& Msg):CMessage( Msg ){}
	CMsg_UTSAGSignal_VAC(const CMsg_UTSAGSignal_VAC& Msg):CMessage( Msg ){}
    ~CMsg_UTSAGSignal_VAC(){}
	bool Post();

	//����payload�ĳ���
	void SetPayloadLength(UINT32 len){ SetDataLength(len);}	

	UINT8 GetCID();
	void SetCID(UINT8 cid);

	//���ô�MessageType��ʼ������ɣ�lenΪ���ɳ���
	void SetSignalPayload(UINT8* pPayload, UINT16 len);		
	//�õ���MessageType��ʼ������ɣ�lenΪ���ɳ���
	void GetSignalPayload(UINT8*& pPayload, UINT16& len);
protected:
	UINT16 GetDefaultMsgId() const { return 1; }
};


//////////////////////////////////////////////////////////////////////////
//DAC�ӿ�͸������

class CMsg_UTSAGSignal_DAC : public CMessage
{
public:
    CMsg_UTSAGSignal_DAC(){}
    CMsg_UTSAGSignal_DAC(const CMessage& Msg):CMessage( Msg ){}
	CMsg_UTSAGSignal_DAC(const CMsg_UTSAGSignal_DAC& Msg):CMessage( Msg ){}
    ~CMsg_UTSAGSignal_DAC(){}
	bool Post();
	
	//����payload�ĳ���
	void SetPayloadLength(UINT32 len){ SetDataLength(len);}	
	
	UINT8 GetCID();
	void SetCID(UINT8 cid);
	
	//���ô�MessageType��ʼ������ɣ�lenΪ���ɳ���
	void SetSignalPayload(UINT8* pPayload, UINT16 len);	
	//�õ���MessageType��ʼ������ɣ�lenΪ���ɳ���
	void GetSignalPayload(UINT8*& pPayload, UINT16& len);	
protected:
	UINT16 GetDefaultMsgId() const { return 1; }
};



#endif  /*__CallSignalMsg_H__*/

