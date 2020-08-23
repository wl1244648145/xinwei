/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    SAGSignal.h
*
* DESCRIPTION: 
*		模拟SAG的SAbis+信令结构定义
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

	SignalType  ParseMessageFromSAG();		//分析得到从SAG发送消息的消息类型
	SignalType	ParseMessageToSAG();		//分析得到SAG接收消息的消息类型
	SignalType	ParseUTSAGMsgFromSAG();		//分析得到从SAG发送透传消息的消息类型
	SignalType	ParseUTSAGMsgToSAG();		//分析得到SAG接收透传消息的消息类型
	
	//设置信令头部的BTSID和SAGID
	void SetBTSSAGID(UINT16 BTSID, UINT32 SAGID);
	//根据消息类型设置消息的EVENT GROUP ID(net order)和EVENT ID,
	void SetSigIDS(SignalType sigType);
	//设置信令头部的Length字段
	void SetSigHeaderLengthField(UINT16 len);
	//设置信令头部TcpPktHeader和结束标记
	void setSigTcpPKTHeaderTail(UINT16 SPC_Code, UINT16 DPC_Code, UINT16 TcpPKTLen);
	//分析SAG收到的信令消息是根据L3Addr还是UID来找CCB
	bool SAGSignalHowToFindCCB(UINT8& how, UINT32& UID_SAG);
	
	static void ClearSignalCounters();
	void DoCountRxSignal();
	void DoCountTxSignal();
	
	static UINT32	m_SAGID;	//net order
	static UINT16	m_BTSID;	//net order
	
	//信令的EVENT_ID和EVENTGROUP_ID对照表
	static SigIDST	m_SigIDS[InvalidSignal_MSG];	//host order
	//信令EventID对照表
	static sigTableT	sigTabEventID[M_MSG_EVENT_GROUP_NUM+1];
	//透传信令MessageType对照表
	static SignalType	sigTabMsgType[M_MSGTYPE_MAX_VALUE+1];
	
	//信令统计表
	static UINT32	RxSignalCounter[InvalidSignal_MSG+1];
	static UINT32	TxSignalCounter[InvalidSignal_MSG+1];

	UINT16 getSPC();
	UINT16 getDPC();

};

#endif /* __TSAGSIGNAL_H */


