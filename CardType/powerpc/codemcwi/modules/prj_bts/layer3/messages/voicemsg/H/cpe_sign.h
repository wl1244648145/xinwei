/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    cpe_signal_struct.h
*
* DESCRIPTION: 
*
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2006-09-04 fengbing  select codec info field in handover Req changed to NotTLV format.
*   03/21/06   fengbign  delete UT_CAP field in handover req signal
*   10/28/05   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__CPE_SIGNAL_STRUCT_H
#define	__CPE_SIGNAL_STRUCT_H

#include "voiceCommon.h"

#ifndef M_CLUSTER_EN
#define M_CLUSTER_EN
#endif

#define M_SMS_MSG_LENGTH	(140)
#define M_MAX_SIGNAL_LENGTH	(M_SMS_MSG_LENGTH+2)
//==============================================================================
//msg structs

//----------------------------------------
//voice control msgs

//setup signal from UT
typedef struct tagStruct_TLV
{
	UINT8 type;
	UINT8 lenth;
	UINT8 value;
}Struct_TLV;
typedef struct tagSetupUT_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	SetupCause;
	UINT8   Codeclist[2]; //now 729A only;
	UINT8     Digit_Type;
	UINT8	Digit_Len;	//UT不用，手机使用
	UINT8	Digits[30];	//UT不用，手机使用
}SetupUTT_CPE;

//setup signal from SAG
typedef struct tagSetupSAG_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	SetupCause;
	UINT8	codecList[100];
}SetupSAGT_CPE;

typedef struct tagCallProc_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	prio;
	tagStruct_TLV selcodeinfo;
}CallProcT_CPE;

typedef struct tagAlerting_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	tagStruct_TLV selcodeinfo;
}AlertingT_CPE;

typedef struct tagConnectUT_CPE
{
	UINT8	Cid;
	UINT8	msgType;	
	tagStruct_TLV selcodeinfo;
}ConnectUTT_CPE;

typedef struct tagConnectSAG_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	tagStruct_TLV selcodeinfo;
}ConnectSAGT_CPE;

typedef struct tagConnectAck_CPE
{
	UINT8	Cid;
	UINT8	msgType;
}ConnectAckT_CPE;

typedef struct tagDisconnect_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	RelCause;
}DisconnectT_CPE;

typedef struct tagRelease_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	RelCause;		
}ReleaseT_CPE;

typedef struct tagReleaseComplete_CPE
{
	UINT8	Cid;
	UINT8	msgType;
}ReleaseCompleteT_CPE;

typedef struct tagModiMediaReq_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	mediaType;	
}ModiMediaReqT_CPE;

typedef struct tagModiMediaRsp_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	result;	
}ModiMediaRspT_CPE;



typedef struct tagInformation_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	type;		//0:dial number;1:flash hook
	struct tagDigi_No
		{
		UINT8 type;
		UINT8 lenth;
		UINT8 num;
		}digi_No;
}InformationT_CPE;

typedef struct tagAuthCmdReq_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	Rand[16];	
}AuthCmdReqT_CPE;

typedef struct tagAuthCmdRsp_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	result[4];	
}AuthCmdRspT_CPE;

typedef struct tagLogin_CPE
{
	UINT8	Cid;
	UINT8	msgType;	
	UINT8	LAI[3];
	UINT8	VersionInfo[2];
	UINT8	REG_TYPE;
}LoginT_CPE;

typedef struct tagLoginRsp_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	RegPeriod[2];	//Periodical Update timer value
	UINT8	LAI[3];
	UINT8	LoginResult;	//login result
#ifdef M_CLUSTER_EN
	UINT8	Ind;
	UINT8	result;
	UINT8	EID[4];
	UINT8	port;
#endif

}LoginRspT_CPE;

typedef struct tagLogout_CPE
{
	UINT8	Cid;
	UINT8	msgType;
}LogoutT_CPE;

typedef struct tagHandOverReq_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	UID[4];
	UINT8	LAI[3];
	UINT8	HO_Type;
	UINT8	ServiceOption[2];
	UINT8	ServiceAppType;
	//tagStruct_TLV selcodeinfo;
	UINT8	selcodeinfo;
	UINT8	VersionInfo[2];		//Version info 
	
}HandOverReqT_CPE;

typedef struct tagHandOverRsp_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	UID[4];				//UID
	UINT8	LAI[3];
	UINT8	Result;
}HandOverRspT_CPE;

typedef struct tagHandOverComplete_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	UID[4];
	UINT8	Result;				//Result
}HandOverCompleteT_CPE;

//-------------------------------------
//sms msgs
typedef struct tagMOSmsDataReq_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	payload[M_SMS_MSG_LENGTH];
}MOSmsDataReqT_CPE;

typedef struct tagMOSmsDataRsp_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	SMSCause;
}MOSmsDataRspT_CPE;

typedef struct tagMTSmsDataReq_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	payload[M_SMS_MSG_LENGTH];
}MTSmsDataReqT_CPE;

typedef struct tagMTSmsDataRsp_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	SMSCause;
}MTSmsDataRspT_CPE;

typedef struct tagSMSMemAvailReq_CPE
{
	UINT8	Cid;
	UINT8	msgType;	
}SMSMemAvailReqT_CPE;

typedef struct tagSMSMemAvailRsp_CPE
{
	UINT8	Cid;
	UINT8	msgType;	
}SMSMemAvailRspT_CPE;

typedef struct tagCallTicket_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	uid[4];
	UINT8	buf[100];
}CallTicketT_CPE;

typedef struct tagCallTicketAck_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	uid[4];
	Struct_TLV call_ticket_rsp;
}CallTicketAckT_CPE;

typedef struct tagChangePasswdReq_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	newPasswdEncrypt[8];
	UINT8     MIC[20];
}ChangePasswdReqT_CPE;

typedef struct tagChangePasswdRsp_CPE
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	result;
}ChangePasswdRspT_CPE;

typedef struct tagChangePasswdAck_CPE
{
	UINT8	Cid;
	UINT8	msgType;
}ChangePasswdAckT_CPE;

//for softphone, 此处去掉CPE_WITH_MMI宏20081218
//#ifdef CPE_WITH_MMI

typedef struct 
{
	UINT8	Cid;
	char	Number[32];
}CallingSetUpA_U;

typedef struct 
{
	UINT8	Cid;
}RingbackU_A;

typedef struct 
{
	UINT8	Cid;
}ConnectU_A;

typedef struct 
{
	UINT8	Cid;
	UINT8     Reason;
}ReleaseU_A;
typedef struct 
{
	UINT8	Cid;
	char     Number[32];
}CalledSetupU_A;

typedef struct 
{
	UINT8	Cid;
}AlertingA_U;

typedef struct 
{
	UINT8	Cid;
}OffhookA_U;
typedef struct 
{
	UINT8	Cid;
}OnhookA_U;

typedef struct 
{
	UINT8	Cid;
}RefuseA_U;

typedef struct 
{
	UINT8	Cid;
	UINT8     Digit;
}InformationA_U;

typedef struct 
{
	UINT8	Cid;
}FlashhookA_U;
//for softphone, 此处去掉CPE_WITH_MMI宏20081218
//#endif//End CPE_WITH_MMI

#ifdef COMIP

typedef struct
{
	UINT8	Cid;
	UINT8	DtmfCode;
}DownlinkDTMFU_A;


#endif//end COMIP

#ifdef M_CLUSTER_EN
#define RESOURCE_LEN 14
//AT->L3 PTTSetup
typedef struct
{
	UINT8	Cid;
	UINT8	CommType;
	UINT16	GID;
	UINT8	CallPriorty;	
	UINT8	EncryptFlag;
}PTTSetupA_U;

//L3->AT PTTSetupRsp
typedef struct
{
	UINT8	Cid;
	UINT16	GID;
	UINT8	RspType;
	UINT8	Result;
	UINT8	CallPriority;
}PTTSetupRspU_A;

//L3->AT GroupRelease or TalkRelease
typedef struct
{
	UINT8	Cid;
	UINT16	GID;
	UINT8	Reason;	
}TrunkRealseU_A;

//L3->AT GroupPagingReq
typedef struct
{
	UINT8	Cid;
	UINT16	Gid;
	UINT8	CommType;
	UINT8	CallPriority;
	UINT8	LateEntryInd;
	UINT8	TransId;
}GroupPagingReqU_A;

//MMI回应给l3寻呼应答
typedef struct
{
	UINT8	Cid;
	UINT8	Ind;
	UINT16	GID;
	UINT8	CommType;
}GroupPagingRspA_U;


//L3->AT GID Info  
typedef struct
{
	UINT8	Cid;
	UINT8	Num;
	UINT8	EndFlag;
	UINT8	Content;
}GIDInfoU_A;

//L3->AT Current Talk Indication  
typedef struct
{
	UINT8	Cid;
	UINT16	GID;
	UINT8	Ind;
}CurTalkIndU_A;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
	UINT8	CommType;
	UINT8	GID[2];
	UINT8	EncryptFlag;
	UINT8	CallPriority;
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8 EncryptKey[M_ENCRYPT_KEY_LENGTH];
#endif
}PTTSetupT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
	UINT8	CallPriority;//Call priority	1	M		
	UINT8	EncryptFlag;//Encryption Flag	1	M		加密标识	
}PTTSetupAckT_CPE;

//l3->l2寻呼响应
typedef struct 
{
	UINT8	Cid;
	UINT8	GID[2];
	UINT8	result;
	UINT8	PagingType;
	UINT8	Late_Entry_Ind;
	UINT8	Response_Ind;
	UINT8	Resource[RESOURCE_LEN-4];
}GroupPagingRspT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	Resource[RESOURCE_LEN];
}CMResrcIndT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8 	GID[2];
	UINT8	UID[4];
}GroupDiscntT_CPE;


typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
}GroupCntAckT_CPE;


typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	grant;
	UINT8	callOwnership;
	UINT8	callPriority;
	UINT8	EncryptFlag;
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8 EncryptKey[M_ENCRYPT_KEY_LENGTH];
#endif
}GroupPTTConnectT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	Gid[2];
	UINT8	msgType;	
	UINT8	reason;
}GroupRlsT_CPE;

typedef struct
{
	UINT8	Cid;
	UINT8	GID[2];
	UINT8	EID[4];
	UINT8	UID[4];
	UINT8	CommType;
}GetShareResrcT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
}PTTReleaseT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
}PTTReleaseAckT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
}TalkInterruptAckT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
	UINT8	grant;
	UINT8	encryptFlag;
	UINT8	reason;
	UINT8	telno[3];
}TalkInterruptT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
}GroupCallingCompleteT_CPE;


typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
	UINT8	reason;
}GroupCallingRlsT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	Gid[2];
	UINT8	CallPrioity;
	UINT8	EncryptControl;
}PTTPressReqT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	Gid[2];
	UINT8	Result;
	UINT8	Resource[RESOURCE_LEN];
}ResourceRspT_CPE;


typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
	UINT8	GID[2];
}PTTPressCancleT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	msgType;	
	UINT8	GID[2];
	UINT8	Grant;
	UINT8	EryptFlag;
}PTTPressRspT_CPE;

typedef struct
{
	UINT8	Cid;
	UINT8	Gid[2];
	UINT8	Type;
	UINT8	RelayInd;
	UINT8	RspInd;
	UINT8	TransId;
	UINT8	CallPrioity;
	UINT8   SCG_INDEX;
    UINT8   DL_DSCH_SLOT_INDEX;
    UINT8   DL_DSCH_SCH_INDEX;
    UINT8   UL_SLOT_INDEX;
    UINT8   UL_SCH_INDEX;
    UINT8  	Report_Period[2] ;
    UINT8  	Backoff_Win[2] ;
    UINT8   EncryptionFlag ;//端到端加密
}GroupSetupIndT_CPE;


typedef struct
{
	UINT8	Cid;
	UINT8	Gid[2];
	UINT8	CallPrioity;
	UINT8	Type;
	UINT8	RelayInd;
	UINT8	RspInd;
	UINT8	TransId;
}GroupPagingActiveT_CPE;

typedef struct
{
	UINT8	Cid;
	UINT8	Gid[2];
	UINT8	MsgType;
}GroupSignalIndT_CPE;

typedef struct
{
	UINT8	Cid;
	UINT8	MsgType;
	UINT8 	AttachType;
	UINT8	Num;
	UINT8	EmergencyCallInd;
	UINT8 	Content;
}AttachGIDReqT_CPE;

typedef struct
{
	UINT8	Cid;
	UINT8	MsgType;
	UINT8	Result;
}AttachGIDRspT_CPE;

typedef struct
{
	UINT8	AttachInd;
	UINT8	Num;
	UINT8	EmergencyCallInd;
	UINT8	Content;
}AttachSubT_CPE;

typedef struct
{
	UINT8	GroupInd;
	UINT8	PermitInd;
	UINT8	TalkInd;
	UINT8	CommType;
	UINT16	GID;
}CMHandOverRspT_CPE;

typedef struct
{
	UINT8	Cid;
	UINT8	SessionType;
	UINT8	Reason;
	UINT8	SCGIndex;
}MacSessionRlsReqT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	Result;
}MacSessionRlsConfirmT_CPE;

typedef struct 
{
	UINT8	Cid;
	UINT8	Ind;
}GroupStatusReportReqT_CPE;

//L3->AT GID Info  
typedef struct
{
	UINT8	Cid;
	UINT8	GidNum;
	UINT16	Content;
}GroupGIDInfoIndT_CPE;

typedef struct
{
	UINT8	Cid;
	UINT8	ReportIndexInd[2];
}GroupReprotIndexIndT_CPE;
#endif
#endif /* __CPE_SIGNAL_STRUCT_H */  

