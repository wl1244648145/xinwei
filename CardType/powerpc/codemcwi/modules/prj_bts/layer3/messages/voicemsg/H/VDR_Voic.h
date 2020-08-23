/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    VDR_Voice_Data.h
*
* DESCRIPTION: 
*		Voice模块与VDR之间语音数据包消息接口。
*		Usage:
*			CVoiceTimeoutMsg msg;
*			只用于tVoice从tVDR接收消息，VDR任务使用CComMessage给tVoice发送消息
*			所以不用实现GetDefaultDataLen()函数，不用重新继承CMessage类实现。
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2006-04-18  fengbing  modify bit fields definition.
*	2006-3-29	fengbing  modify DMUX format
*   2005-9-21   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__VDR_VOICE_DATA_H
#define	__VDR_VOICE_DATA_H



typedef struct tagDMUXHead
{
	UINT8	FormID:3;
	UINT8	Prio:3;
	UINT8	Reserved1:2;
	UINT8	nFrameNum;
	UINT8	Reserved2[4];

}DMUXHeadT;

typedef struct tagDMUXVoiDataCommon
{
	UINT8	CallID[2];		//low 2 bytes of L3Addr,now hight 2 bytes of L3Addr are zeros.
	UINT8	Codec:4;
	UINT8	frameNum:3;
	UINT8	blG729B:1;
	UINT8	SN;
	UINT8	timeStamp;
	
}DMUXVoiDataCommonT;

typedef struct tagDMUXVoiDataPkt729
{
	DMUXVoiDataCommonT head;
	UINT8	Data[10]; 

}DMUXVoiDataPkt729T;

typedef struct tagDMUXVoiDataPkt711
{
	DMUXVoiDataCommonT head;
	UINT8	Data[80]; 

}DMUXVoiDataPkt711T;


#endif /* __VDR_VOICE_DATA_H */

