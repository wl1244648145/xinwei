/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    VAC_session_interface.h
*
* DESCRIPTION: 
*		VAC session接口消息。
*		Usage:
*			CMsg_XXXX msg;
*			msg.CreateMessage(comEntity);
*			msg.SetDstTid(M_TID_VOICE);
*			msg.SetMessageId(msgid);
*			msg.SetCid(cid);
*			msg.SetRate() or SetResult or Donothing
*			msg.Post;
*
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2006-09-12 fengbing  add VAC release reason for VAC release notify
*   2006-08-14 fengbing  add DeleteMessage() when Post CMessage failed
*   2006-04-09 fengbing  增加VACSetupRsp的结果字段定义
*   2005-09-21 fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__VAC_SESSION_INTERFACE_H
#define	__VAC_SESSION_INTERFACE_H

#include "Message.h"

#define M_RATE_8K	(8)
#define M_RATE_64K	(64)

#define M_VACSETUPREASON_CALL		(0)
#define M_VACSETUPREASON_HANDOVER	(1)
#define M_VACSETUPREASON_GRP_SETUP	(2)////2：组呼建立建链---实现故障弱化集群功能时加入，已经实现
#define M_VACSETUPREASON_GRP_TALKING	(3)////3：组呼讲话方建链---实现故障弱化集群功能时加入，已经实现
#define M_VACSETUPREASON_ENC_CALL (4)////4：加密呼叫建链---加密项目需要，L2据此判断为加密应用时rrm做处理
#define M_VACSETUPREASON_ENC_HANDOVER (5)////5：加密切换建链---加密项目需要，L2据此判断为加密应用时rrm做处理
#define M_VACSETUPREASON_ENC_GRP_SETUP (6)////6：加密组呼建立建链---加密项目需要，L2据此判断为加密应用时rrm做处理
#define M_VACSETUPREASON_ENC_GRP_TALKING (8)////8：加密组呼讲话方建链---加密项目需要，L2据此判断为加密应用时rrm做处理

typedef enum
{
	VACSETUP_RESULT_OK=0x0,//0: ok

	VACSETUP_RESULT_PEER_VAC_REQ=0x01,//0x01: Peer VAC Request
	VACSETUP_RESULT_PEER_TMO,//0x02: Peer Setup TMO
	VACSETUP_RESULT_PEER_REJECT,//0x03: Peer Reject


	VACSETUP_RESULT_MAC_REJECT=0x10,//0x10: MAC Reject
	VACSETUP_RESULT_MAC_REL_INDICATION,//0x11: MAC Release Indication
	VACSETUP_RESULT_MAC_SETUP_TMO,//0x12: MAC Setup TMO
	VACSETUP_RESULT_MAC_REJECT1,//0x13: MAC Reject
	VACSETUP_RESULT_MAC_REL_INDICATION1,//0x14: MAC Release Indication
	VACSETUP_RESULT_MAC_FAIL,//0x15: MAC Fail

	VACSETUP_RESULT_L3_REJECT=0x20,//0x20: L3 Release Req	//当用户电话处于忙音＆拥塞时
	VACSETUP_RESULT_L3_TMO,//0x21: L3 TMO		//等待SAG的Assign transport resource Rsp超时时

	VACSETUP_RESULT_SAG_REJECT=0x30//0x30: SAG reject
	
}ENUM_VACSetupRspResultT;

//VAC session release reason
#define VAC_REL_REASON_MAC		(0)
#define VAC_REL_REASON_PEER		(1)
#define VAC_REL_REASON_ERROR	(2)

//////////////////////////////////////////////////////////////////////////
//VAC session interface msg structs
//VAC setup cmd Req
typedef struct tagVACSetupCmdReq
{
	UINT8	CID;
	UINT8	Rate;
// 	UINT32	L3Addr;
	UINT8	Reason;
#ifdef M__SUPPORT__ENC_RYP_TION	
	UINT8	sagStatus:1;
	UINT8	rsv:7;
#endif	
}VACSetupCmdReqT;
//VAC setup cmd Rsp
typedef struct tagVACSetupCmdRsp
{
	UINT8	CID;
	UINT8	Result;	
//	UINT32	L3Addr;
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8	sagStatus:1;
	UINT8	rsv:7;
#endif	
}VACSetupCmdRspT;
//VAC setup cmd Notify
typedef struct tagVACSetupCmdNotify
{
	UINT8	CID;
	UINT8	Rate;	
// 	UINT32	L3Addr;
	UINT8	Reason;
#ifdef M__SUPPORT__ENC_RYP_TION	
	UINT8	sagStatus:1;
	UINT8	rsv:7;
#endif	
}VACSetupCmdNotifyT;
//VAC release cmd Req
typedef struct tagVACRlsCmdReq
{
	UINT8	CID;
}VACRlsCmdReqT;
//VAC release cmd notify
typedef struct tagVACRlsCmdNotify
{
	UINT8	CID;
	UINT8	reason;//VAC_REL_REASON_MAC or VAC_REL_REASON_PEER
}VACRlsCmdNotifyT;
//VAC start cmd
typedef struct tagVACStart
{
	UINT8	CID;
}VACStartT;
//VAC stop cmd
typedef struct tagVACStop
{
	UINT8	CID;
}VACStopT;
//VAC modify cmd Req
typedef struct tagVACModiCmdReq
{
	UINT8	CID;
	UINT8	Rate;
}VACModiCmdReqT;
//VAC modify cmd Rsp
typedef struct tagVACModiCmdRsp
{
	UINT8	CID;
	UINT8	Result;	
}VACModiCmdRspT;
//VAC modify cmd Notify
typedef struct tagVACModiCmdNotify
{
	UINT8	CID;
	UINT8	Rate;	
}VACModiCmdNotifyT;


//////////////////////////////////////////////////////////////////////////
//base class
class CMsg_VACSessionIFMsg:public CMessage
{
public:
	CMsg_VACSessionIFMsg(){}
	CMsg_VACSessionIFMsg(const CMessage& Msg):CMessage( Msg ){}
	~CMsg_VACSessionIFMsg(){}

	bool Post()
	{
		bool ret = CMessage::Post();
		if(!ret)
			DeleteMessage();
		return ret;
	};
	void SetCid(UINT8 cid){ *((UINT8*)GetDataPtr()) = cid; }
	UINT8 GetCid(){ return *((UINT8*)GetDataPtr()); }
	void SetPayloadLen(UINT8 DataLen){ SetDataLength(DataLen);}
protected:
	UINT32 GetDefaultDataLen() const{ return sizeof(VACSetupCmdReqT);}
	UINT16 GetDefaultMsgId() const { return 1; }
};

//////////////////////////////////////////////////////////////////////////
//VAC setup
class CMsg_VACSetupReq:public CMsg_VACSessionIFMsg
{
public:
	CMsg_VACSetupReq(){};
	CMsg_VACSetupReq(const CMessage& Msg):CMsg_VACSessionIFMsg( Msg ){}
	~CMsg_VACSetupReq(){}

	void SetRate(UINT8 rate){ ((VACSetupCmdReqT*)GetDataPtr())->Rate = rate; }
	UINT8 GetRate(){ return ((VACSetupCmdReqT*)GetDataPtr())->Rate; }

	void SetReason(UINT8 reason){((VACSetupCmdReqT*)GetDataPtr())->Reason = reason;}
	UINT8 GetReason(){ return ((VACSetupCmdReqT*)GetDataPtr())->Reason; }
#ifdef M__SUPPORT__ENC_RYP_TION	
	void SetSagStatus(UINT8 flag){((VACSetupCmdReqT*)GetDataPtr())->sagStatus=flag;};
	UINT8 GetSagStatus(){return ((VACSetupCmdReqT*)GetDataPtr())->sagStatus;};
	void SetRsv(){((VACSetupCmdReqT*)GetDataPtr())->rsv=0;}
#endif	
};
class CMsg_VACSetupRsp:public CMsg_VACSessionIFMsg
{
public:
	CMsg_VACSetupRsp(){};
	CMsg_VACSetupRsp(const CMessage& Msg):CMsg_VACSessionIFMsg( Msg ){}
	~CMsg_VACSetupRsp(){}

	void SetResult(UINT8 result){ ((VACSetupCmdRspT*)GetDataPtr())->Result = result; }
	UINT8 GetResult(){ return ((VACSetupCmdRspT*)GetDataPtr())->Result; }
#ifdef M__SUPPORT__ENC_RYP_TION		
	void SetSagStatus(UINT8 flag){((VACSetupCmdRspT*)GetDataPtr())->sagStatus=flag;};
	UINT8 GetSagStatus(){return ((VACSetupCmdRspT*)GetDataPtr())->sagStatus;};
	void SetRsv(){((VACSetupCmdRspT*)GetDataPtr())->rsv=0;}
#endif
};
#define 	CMsg_VACSetupNotify		CMsg_VACSetupReq

//////////////////////////////////////////////////////////////////////////
//VAC start stop release 
#define CMsg_VACStart			CMsg_VACSessionIFMsg
#define CMsg_VACStop			CMsg_VACSessionIFMsg
#define CMsg_VACRelease			CMsg_VACSessionIFMsg
#define CMsg_VACReleaseNotify	CMsg_VACSessionIFMsg

//////////////////////////////////////////////////////////////////////////
//VAC modify
#define CMsg_VACModifyReq		CMsg_VACSetupReq
#define CMsg_VACModifyRsp		CMsg_VACSetupRsp
#define CMsg_VACModifyNotify	CMsg_VACSetupNotify

#endif /* __VAC_SESSION_INTERFACE_H */

