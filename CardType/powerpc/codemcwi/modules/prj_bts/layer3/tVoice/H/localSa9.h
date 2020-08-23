/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSagStruct.h
* Create Date    : 19-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__LOCALSAGSTRUCT_H
#define	__LOCALSAGSTRUCT_H

typedef struct __PlayToneInfoT
{
	UINT8 CID;
	UINT8 UID[4];
	UINT8 toneID[2];
}PlayToneInfoT;
typedef struct __StopToneInfoT
{
	UINT8 CID;
	UINT8 UID[4];
	UINT8 toneID[2];
}StopToneInfoT;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//内部信令头，置于sabis信令头前
typedef struct __InnerSignalHeadT
{
	UINT8 sigType[2];
}InnerSignalHeadT, *PInnerSignalHeadT;

typedef struct __InnerSignal_CallArriveT
{
	UINT8 dstUID[4];
	UINT8 srcUID[4];
	UINT8 srcL3Addr[4];
}InnerSignal_CallArriveT;

typedef struct __InnerSignal_GrpMsgHeadT
{
	UINT8 UID[4];
	UINT8 GID[2];
}InnerSignal_GrpMsgHeadT;

typedef struct __InnerSignal_PttConnectT
{
	InnerSignal_GrpMsgHeadT head;
	UINT8	grant;
	UINT8	callOwnership;
	UINT8	callPriority;
	UINT8	EncryptFlag;
}InnerSignal_PttConnectT;

typedef struct __InnerSignal_GrpCallingRlsT
{
	InnerSignal_GrpMsgHeadT head;
	UINT8	reason;
}InnerSignal_GrpCallingRlsT;

typedef struct __InnerSignal_PttInterruptT
{
	InnerSignal_GrpMsgHeadT head;
	UINT8	grant;
	UINT8	encryptFlag;
	UINT8	reason;
	UINT8	telno[3];
}InnerSignal_PttInterruptT;

////////////////////////////////////////////////////////////////////////////////
//cpe向bts注册电话号码
typedef struct __TelNOT
{
	UINT8 cid;
	UINT8 uid[4];
	//UINT8 telTag;
	UINT8 telLen;
	UINT8 telNO[10];
}TelNOT;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAGSTRUCT_H */


