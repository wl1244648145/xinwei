/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    CallSignalMsg.h
*
* DESCRIPTION: 
*		呼叫信令消息类，所有SabisA+信令消息接口
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2006-09-06  fengbing  信令种类增加透传类信令callticket,callticketAck,SRQ,SRP,SRC,
*                         相应改动信令统计，与其他任务接口暂时不变，但是新增信令也进行计数
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
	UINT8		nMaxSigIndex;		//sigTypeArr数组中合法的最大数组Index
	SignalType	sigTypeArr[100];	//以EventID为索引的信令消息翻译表
}sigTableT;


typedef struct tagSigIDS
{
	UINT8	EventGroupID;
	UINT16	EventID;
}SigIDST;
//////////////////////////////////////////////////////////////////////////
//VCR接口的信令
class CMsg_Signal_VCR:public CMessage
{
public:
    CMsg_Signal_VCR(){}
    CMsg_Signal_VCR(const CMessage& Msg):CMessage( Msg ){}
	CMsg_Signal_VCR(const CMsg_Signal_VCR& Msg):CMessage( Msg ){}
    ~CMsg_Signal_VCR(){}
	
	bool CreateMessage(CComEntity& Entity, UINT32 uDataSize);
	bool Post();
	void SetPayloadLength(UINT32 len){ SetDataLength(len);}	//设置payload的长度

	SignalType  ParseMessageFromSAG();		//分析得到从SAG接收消息的消息类型
	SignalType	ParseMessageToSAG();		//分析得到向SAG发送消息的消息类型
	SignalType	ParseUTSAGMsgFromSAG();		//分析得到从SAG接收透传消息的消息类型
	SignalType	ParseUTSAGMsgToSAG();		//分析得到向SAG发送透传消息的消息类型
	bool isUTSAGMsg();	//判断是否为透传信令

	//设置信令头部的BTSID和SAGID
	void SetBTSSAGID();
	//根据消息类型设置消息的EVENT GROUP ID(net order)和EVENT ID,
	void SetSigIDS(SignalType sigType);
	//设置信令头部的Length字段
	void SetSigHeaderLengthField(UINT16 len);
	//分析从SAG收到的信令消息是根据L3Addr还是UID来找CCB
	bool SAGSignalHowToFindCCB(UINT8& how, UINT32& UID_SAG);

	static void ClearSignalCounters();
	void DoCountRxSignal();
	void DoCountTxSignal();

	static UINT32	m_SAGID;
	static UINT16	m_BTSID;
	static char m_sigName[InvalidSignal_MSG+1][40];	//signal name

	//信令的EVENT_ID和EVENTGROUP_ID对照表
	static SigIDST	m_SigIDS[InvalidSignal_MSG+1];	//host order
	//信令EventID对照表
	static sigTableT	sigTabEventID[M_MSG_EVENT_GROUP_NUM+1];
	//透传信令MessageType对照表
	static SignalType	sigTabMsgType[M_MSGTYPE_MAX_VALUE+1];

	//信令统计表
#define MAX_PERF_COUNTER_SIGNAL    UTSAG_UID_MSG	
	static UINT32	RxSignalCounter[InvalidSignal_MSG+1];	//最后一项是非法信令计数
	static UINT32	TxSignalCounter[InvalidSignal_MSG+1];	//最后一项是非法信令计数
protected:
	UINT16 GetDefaultMsgId() const { return 1; }

};
//////////////////////////////////////////////////////////////////////////
//VAC接口透传信令
class CMsg_UTSAGSignal_VAC : public CMessage
{
public:
    CMsg_UTSAGSignal_VAC(){}
    CMsg_UTSAGSignal_VAC(const CMessage& Msg):CMessage( Msg ){}
	CMsg_UTSAGSignal_VAC(const CMsg_UTSAGSignal_VAC& Msg):CMessage( Msg ){}
    ~CMsg_UTSAGSignal_VAC(){}
	bool Post();

	//设置payload的长度
	void SetPayloadLength(UINT32 len){ SetDataLength(len);}	

	UINT8 GetCID();
	void SetCID(UINT8 cid);

	//设置从MessageType开始的信令静荷，len为静荷长度
	void SetSignalPayload(UINT8* pPayload, UINT16 len);		
	//得到从MessageType开始的信令静荷，len为静荷长度
	void GetSignalPayload(UINT8*& pPayload, UINT16& len);
protected:
	UINT16 GetDefaultMsgId() const { return 1; }
};


//////////////////////////////////////////////////////////////////////////
//DAC接口透传信令

class CMsg_UTSAGSignal_DAC : public CMessage
{
public:
    CMsg_UTSAGSignal_DAC(){}
    CMsg_UTSAGSignal_DAC(const CMessage& Msg):CMessage( Msg ){}
	CMsg_UTSAGSignal_DAC(const CMsg_UTSAGSignal_DAC& Msg):CMessage( Msg ){}
    ~CMsg_UTSAGSignal_DAC(){}
	bool Post();
	
	//设置payload的长度
	void SetPayloadLength(UINT32 len){ SetDataLength(len);}	
	
	UINT8 GetCID();
	void SetCID(UINT8 cid);
	
	//设置从MessageType开始的信令静荷，len为静荷长度
	void SetSignalPayload(UINT8* pPayload, UINT16 len);	
	//得到从MessageType开始的信令静荷，len为静荷长度
	void GetSignalPayload(UINT8*& pPayload, UINT16& len);	
protected:
	UINT16 GetDefaultMsgId() const { return 1; }
};



#endif  /*__CallSignalMsg_H__*/

