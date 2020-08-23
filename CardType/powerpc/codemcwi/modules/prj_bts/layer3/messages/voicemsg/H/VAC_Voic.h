/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    VAC_Voice_Data.h
*
* DESCRIPTION: 
*		Voice模块与VAC之间语音数据包消息接口。
*		Usage:
*			CVoiceTimeoutMsg msg;
*			msg.CreateMessage(comEntity);
*			msg.SetDstTid(M_TID_VOICE);
*			msg.SetMessageId(msgid);
*			msg.SetPayloadLen(payload);
*			VAC向tVoice发送消息不使用此消息类。
*			tVoice可以使用此消息类向VAC发送和接收消息
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-9-21   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__VAC_VOICE_DATA_H
#define	__VAC_VOICE_DATA_H

#include "Message.h"



typedef struct tagVACVoiceDataHeader
{
	UINT8	Eid[4];
	UINT8	Cid;
	UINT8	SN;
	UINT8	Type;
}VACVoiceDataHeaderT;

typedef struct tagVACVoiceDataIE
{
	VACVoiceDataHeaderT header;
	UINT8	VoiceData[80];
}VACVoiceDataIE_T;



#define M_MAX_VOICE_TO_VAC_DATASIZE	(8000)

class CMsg_VACVoiceData:public CMessage
{
public:
    CMsg_VACVoiceData(){}
    CMsg_VACVoiceData(const CMessage& Msg):CMessage( Msg ){}
    ~CMsg_VACVoiceData(){}
	void SetPayloadLen(UINT32 len){SetDataLength(len);};
	bool Post()
	{
		bool ret = CMessage::Post();
		if(!ret)
			DeleteMessage();
		return ret;
	};
protected:
	//从VAC接收到的语音包消息使用GetDataLength()来确定消息长度
	//只有Voice给VAC发送消息才用到函数GetDefaultDataLen()，每个UDP包给VAC送一次消息
    //收发消息都使用GetDataPtr()来得到消息payload缓冲区。
	UINT32 GetDefaultDataLen() const{ return M_MAX_VOICE_TO_VAC_DATASIZE;}

	UINT16 GetDefaultMsgId() const { return 1; }
};

#endif /* __VAC_VOICE_DATA_H */

