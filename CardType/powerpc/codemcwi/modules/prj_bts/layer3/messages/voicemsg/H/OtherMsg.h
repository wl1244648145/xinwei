/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    OtherMsg.h
*
* DESCRIPTION: 
*		Voice模块和呼叫无关的消息。包括与tCM之间的配置注册等消息

*
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*	2006-3-29	fengbing  fix SetCid() error
*   2005-9-26   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__OTHERMSG_H
#define	__OTHERMSG_H

#include "Message.h"
#include "voiceToolFunc.h"



typedef struct tagCMVoiceRegMsg
{
	UINT8 Cid;
	UINT8 Uid[4];
	UINT8 blIsCPEZ;
}CMVoiceRegMsgT;


//////////////////////////////////////////////////////////////////////////
//Voice Port Register Message
class CMsg_VoicePortReg:public CMessage
{
public:
    CMsg_VoicePortReg(){}
    CMsg_VoicePortReg(const CMessage& Msg):CMessage( Msg ){}
    ~CMsg_VoicePortReg(){}
	
    //bool CreateMessage(CComEntity&);

	void SetCid(UINT8 cid){ ((CMVoiceRegMsgT*)GetDataPtr())->Cid = cid; }
	UINT8 GetCid(){ return ((CMVoiceRegMsgT*)GetDataPtr())->Cid; }

	void SetUid(UINT32 uid){ VSetU32BitVal(((CMVoiceRegMsgT*)GetDataPtr())->Uid, uid); }
	UINT32 GetUid(){ return VGetU32BitVal(((CMVoiceRegMsgT*)GetDataPtr())->Uid); }		

	UINT8 isCPEZ(){return ((CMVoiceRegMsgT*)GetDataPtr())->blIsCPEZ; }
	
protected:
    UINT32 GetDefaultDataLen() const{ return sizeof(CMVoiceRegMsgT);}
	UINT16 GetDefaultMsgId() const { return 1; }
};

//Voice Port UnRegister Message
typedef CMsg_VoicePortReg CMsg_VoicePortUnReg;

//////////////////////////////////////////////////////////////////////////
//probe request
/*
 *	ProbeReq :cid
 *	ProbeRsp :cid,result
 */
class CMsg_ProbeReq:public CMessage
{
public:
    CMsg_ProbeReq(){}
    CMsg_ProbeReq(const CMessage& Msg):CMessage( Msg ){}
    ~CMsg_ProbeReq(){}	

	UINT8 GetCid(){ return ((UINT8*)GetDataPtr())[0]; }
	void SetCid(UINT8 cid){ ((UINT8*)GetDataPtr())[0] = cid;}
protected:
	UINT32 GetDefaultDataLen() const{ return 1;}
	UINT16 GetDefaultMsgId() const { return 1; }
};
//probe response
class CMsg_ProbeRsp:public CMessage
{
public:
    CMsg_ProbeRsp(){}
    CMsg_ProbeRsp(const CMessage& Msg):CMessage( Msg ){}
    ~CMsg_ProbeRsp(){}	

	UINT8 GetCid(){ return ((UINT8*)GetDataPtr())[0]; }
	void SetCid(UINT8 cid){ ((UINT8*)GetDataPtr())[0] = cid;}

	UINT8 GetProbeResult(){ return ((UINT8*)GetDataPtr())[1]; };
protected:
	UINT32 GetDefaultDataLen() const{ return 2;}
	UINT16 GetDefaultMsgId() const { return 1; }
private:
};
//Congestion request to L2
class CMsg_CongestReqtoL2:public CMessage
{
public:
    CMsg_CongestReqtoL2(){}
    CMsg_CongestReqtoL2(const CMessage& Msg):CMessage( Msg ){}
    ~CMsg_CongestReqtoL2(){}
protected:
	UINT32 GetDefaultDataLen() const{ return 0;}
	UINT16 GetDefaultMsgId() const { return 1; }
};
#endif /* __OTHERMSG_H */

