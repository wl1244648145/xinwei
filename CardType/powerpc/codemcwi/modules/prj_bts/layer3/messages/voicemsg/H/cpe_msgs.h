/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    cpe_msgs_struct.h
*
* DESCRIPTION: 
*
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   10/17/05   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__CPE_MSGS_STRUCT_H
#define	__CPE_MSGS_STRUCT_H


//==============================================================================
//msg structs

//----------------------------------------
//voice control msgs

#define M_MAX_PAYLOAD_LEN	(1024)
typedef struct tagVoiceVACCtrlMsg
{
	UINT8	Cid;
	UINT8	sigPayload[M_MAX_PAYLOAD_LEN];		//����payload,��MessageType��ʼ
}VoiceVACCtrlMsgT;

typedef struct tagVoiceDACCtrlMsg
{
	UINT8	Cid;
	UINT8	sigPayload[M_MAX_PAYLOAD_LEN];		//����payload,��MessageType��ʼ
}VoiceDACCtrlMsgT;



#endif /* __CPE_MSGS_STRUCT_H */

