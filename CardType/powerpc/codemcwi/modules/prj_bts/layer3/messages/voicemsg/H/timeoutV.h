/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    timeoutVoiceMsg.h
*
* DESCRIPTION: 
*		Voice模块定时器超时消息，所有SabisA+信令消息接口。
*		Usage:
*			CVoiceTimeoutMsg msg;
*			msg.CreateMessage(comEntity);
*			msg.SetDstTid(M_TID_VOICE);
*			msg.SetMessageId(timerID);
*			msg.SetCid(cid);
*
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-9-21   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__TIMEOUTVOICEMSG_H
#define	__TIMEOUTVOICEMSG_H

#include "Message.h"


typedef struct tagTimerStruct
{
	UINT8 Cid;
	UINT8 TimerType;
	UINT16 GID;
}TimerStructT;


class CMsg_VoiceTimeout:public CMessage
{
public:
    CMsg_VoiceTimeout(){}
    CMsg_VoiceTimeout(const CMessage& Msg):CMessage( Msg ){}
    ~CMsg_VoiceTimeout(){}
	
    //bool CreateMessage(CComEntity&);

	void SetCid(UINT8 cid){ ((TimerStructT*)GetDataPtr())->Cid = cid; }
	UINT8 GetCid(){ return ((TimerStructT*)GetDataPtr())->Cid; }
	void SetTimerType(UINT8 timerType){ ((TimerStructT*)GetDataPtr())->TimerType = timerType; }
	UINT8 GetTimerType(){ return ((TimerStructT*)GetDataPtr())->TimerType;}
	void SetGid(UINT16 gid){ ((TimerStructT*)GetDataPtr())->GID = gid; }
	UINT8 GetGid(){ return ((TimerStructT*)GetDataPtr())->GID; }
protected:
    UINT32 GetDefaultDataLen() const{ return sizeof(TimerStructT);}
	UINT16 GetDefaultMsgId() const { return 1; }
};

#endif /* __TIMEOUTVOICEMSG_H */

