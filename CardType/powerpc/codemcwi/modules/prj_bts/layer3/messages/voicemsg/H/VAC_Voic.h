/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    VAC_Voice_Data.h
*
* DESCRIPTION: 
*		Voiceģ����VAC֮���������ݰ���Ϣ�ӿڡ�
*		Usage:
*			CVoiceTimeoutMsg msg;
*			msg.CreateMessage(comEntity);
*			msg.SetDstTid(M_TID_VOICE);
*			msg.SetMessageId(msgid);
*			msg.SetPayloadLen(payload);
*			VAC��tVoice������Ϣ��ʹ�ô���Ϣ�ࡣ
*			tVoice����ʹ�ô���Ϣ����VAC���ͺͽ�����Ϣ
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
	//��VAC���յ�����������Ϣʹ��GetDataLength()��ȷ����Ϣ����
	//ֻ��Voice��VAC������Ϣ���õ�����GetDefaultDataLen()��ÿ��UDP����VAC��һ����Ϣ
    //�շ���Ϣ��ʹ��GetDataPtr()���õ���Ϣpayload��������
	UINT32 GetDefaultDataLen() const{ return M_MAX_VOICE_TO_VAC_DATASIZE;}

	UINT16 GetDefaultMsgId() const { return 1; }
};

#endif /* __VAC_VOICE_DATA_H */

