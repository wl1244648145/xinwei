/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    SAbis1_1Struct.h
*
* DESCRIPTION: 
*
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   09/13/05   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__SABIS1_1STRUCT_H
#define	__SABIS1_1STRUCT_H

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif


//==============================================================================
//struct for common use
//

typedef UINT16 HeadFlagT;
typedef UINT16 EndFlagT;

typedef UINT16 HeadFlagT;
typedef UINT16 EndFlagT;

typedef struct tagTcpPktHeader
{
	HeadFlagT	HeadFlag;	//HeadFlag
	UINT16		PktLen;		//TCP报文长度，包括PktLen本身，不包含HeadFlag和EndFlag
	UINT16		DPC_Code;	//目的信令点编码
	UINT16		SPC_Code;	//源信令点编码
	UINT16		UserType;	//用户类型，SAbis1接口固定为M_TCP_PKT_SABIS1_USERTYPE
	UINT16		TTL;		//路由计数器：为防止TCP封装层用户路由错误出现循环路由，此计数器每经过一个节点减一，若为0则此包将被丢弃。此计数器的默认值为32。
}TcpPktHeaderT;

typedef struct tagSigHeader
{
	UINT32	SAG_ID;
	UINT16	BTS_ID;
	UINT8	EVENT_GROUP_ID;
	UINT16	Event_ID;
	UINT16	Length;
}SigHeaderT;

typedef struct tagUT_SAG_ContainerHeader
{
	UINT32	uid_L3addr;
	UINT8	msgType;
}UT_SAG_ContainerHeaderT;


//==============================================================================
//msg structs
//----------------------------------------


//----------------------------------------
//voice control msgs

typedef struct tagLAPaging
{
	UINT32			UID;
	UINT32			L3addr;
	UINT16			App_Type;
//	UINT8			UT_Capability;
	UINT8			VersionInfo[2];
}LAPagingT;

typedef struct tagLAPagingRsp
{
	UINT32			UID;
	UINT32			L3addr;
	UINT16			App_Type;
	UINT8			Cause;	
}LAPagingRspT;

typedef struct tagDELAPagingReq
{
	UINT32			UID;
}DELAPagingReqT;

typedef struct tagDELAPagingRsp
{
	UINT32			UID;
	UINT8			Cause;
}DELAPagingRspT;

typedef struct tagAssignResReq
{
	UINT32			UID;
	UINT32			L3addr;
	UINT8			ReqRate;
	UINT16			AssignReason;	//0:voice call 1:handover 2:add other:reserved
}AssignResReqT;

typedef struct tagAssignResRsp
{
	UINT32			UID;
	UINT32			L3addr;
	UINT8			AssignResult;
}AssignResRspT;

typedef struct tagRlsResReq
{
	UINT32			L3addr;
	UINT8			RlsCause;
}RlsResReqT;

typedef struct tagRlsResRsp
{
	UINT32			L3addr;
}RlsResRspT;

typedef struct tagReset
{
	UINT8			RestCause;
	UINT8			ResetTime[8];
}ResetT;

typedef struct tagResetAck
{

}ResetAckT;

typedef struct tagBeatHeart
{
	UINT8			sequence;
}BeatHeartT;

typedef struct tagBeatHeartAck
{
	UINT8			sequence;
}BeatHeartAckT;

typedef struct tagCongestionCtrlReq
{
	UINT8			level;
}CongestionCtrlReqT;

typedef struct tagCongestionCtrlRsp
{
	UINT8			level;
}CongestionCtrlRspT;

typedef struct tagErrNotifyReq
{
	UINT32			Uid;				//UID
	UINT32			L3Addr;				//L3addr
	UINT8			ErrCause;	
}ErrNotifyReqT;

typedef struct tagErrNotifyRsp
{
	UINT32			Uid;				//UID
	UINT32			L3Addr;				//L3addr
}ErrNotifyRspT;

typedef struct tagUTSAG_Signal_L3Addr
{
	UINT32 L3Addr;
	UINT8 msgType;
	UINT8 UTSAGPayload[100];
}UTSAG_Signal_L3AddrT;

typedef struct tagUTSAG_Signal_Uid		
{
	UINT32 Uid;
	UINT8 msgType;
	UINT8 UTSAGPayload[100];
}UTSAG_Signal_UidT;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//setup signal from UT
typedef struct tagSetupUT
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			SetupCause;
	UINT8			Digit_Num;	//UT不用，手机使用
	//UINT8			Digits[20];	//UT不用，手机使用
}SetupUTT;

//setup signal from SAG
typedef struct tagSetupSAG
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			SetupCause;
	UINT8			SelCodecinfo;
	UINT8			Digit_Num;	//主叫号码位数
	//UINT8			Digits[20];	//主叫号码
	UINT8			Digits[1];	//主叫号码
}SetupSAGT;

typedef struct tagCallProc
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			prio;
}CallProcT;

typedef struct tagAlerting
{
	UT_SAG_ContainerHeaderT	containerHeader;
}AlertingT;

typedef struct tagConnectUT
{
	UT_SAG_ContainerHeaderT	containerHeader;	
}ConnectUTT;

typedef struct tagConnectSAG
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			SelCodecInfo;
}ConnectSAGT;

typedef struct tagConnectAck
{
	UT_SAG_ContainerHeaderT	containerHeader;
}ConnectAckT;

typedef struct tagDisconnect
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			RelCause;
}DisconnectT;

typedef struct tagRelease
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			RelCause;		
}ReleaseT;

typedef struct tagReleaseComplete
{
	UT_SAG_ContainerHeaderT	containerHeader;
}ReleaseCompleteT;

typedef struct tagModiMediaReq
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			mediaType;	
}ModiMediaReqT;

typedef struct tagModiMediaRsp
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			result;	
}ModiMediaRspT;

typedef struct tagInformation
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			type;//0:dial number;1:flash hook
	UINT8			num;//effect when type==0	
}InformationT;

typedef struct tagAuthCmdReq
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			Rand[16];	
}AuthCmdReqT;

typedef struct tagAuthCmdRsp
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT32			result;	
}AuthCmdRspT;

typedef struct tagLogin
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			LAI[3];
	UINT8			CodecList[3];	//目前支持G.729和G.711
	UINT8			VersionInfo[2];
	UINT8			REG_TYPE;
}LoginT;

typedef struct tagLoginRsp
{
	UT_SAG_ContainerHeaderT	containerHeader;

	UINT16			RegPeriod;		//Periodical Update timer value
	UINT8			LAI[3];
	UINT8			LoginResult;	//login result

}LoginRspT;

typedef struct tagLogout
{
	UT_SAG_ContainerHeaderT	containerHeader;
}LogoutT;

typedef struct tagHandOverReq
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT32			UID;				//
	UINT8			LAI[3];
	UINT8			UT_CAP;
	UINT8			HO_Type;
	UINT16			ServiceOption;
	UINT8			ServiceAppType;
	UINT8			Codeclist[3];		//目前支持G.729和G.711
	UINT8			VersionInfo[2];		//Version info 
}HandOverReqT;

typedef struct tagHandOverRsp
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT32			UID;				//UID
	UINT8			LAI[3];
	UINT8			Result;				//Result
}HandOverRspT;

typedef struct tagHandOverComplete
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT32			UID;
	UINT8			Result;				//Result
}HandOverCompleteT;

typedef struct tagStatusQry
{
	UT_SAG_ContainerHeaderT	containerHeader;	
}StatusQryT;

typedef struct tagStatus
{
	UT_SAG_ContainerHeaderT	containerHeader;	
	UINT8			statusInfo;
}StatusT;

//////////////////////////////////////////////////////////////////////////

//sms msgs
typedef struct tagMOSmsDataReq
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			payload[200];
}MOSmsDataReqT;

typedef struct tagMOSmsDataRsp
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			SMSCause;
}MOSmsDataRspT;

typedef struct tagMTSmsDataReq
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			payload[200];
}MTSmsDataReqT;

typedef struct tagMTSmsDataRsp
{
	UT_SAG_ContainerHeaderT	containerHeader;
	UINT8			SMSCause;
}MTSmsDataRspT;

typedef struct tagSMSMemAvailReq
{
	UT_SAG_ContainerHeaderT	containerHeader;	
}SMSMemAvailReqT;

typedef struct tagSMSMemAvailRsp
{
	UT_SAG_ContainerHeaderT	containerHeader;	
}SMSMemAvailRspT;

union SigPayload
{
	LAPagingT				LAPaging;
	LAPagingRspT			LAPagingRsp;
	DELAPagingReqT			DELAPagingReq;
	DELAPagingRspT			DELAPagingRsp;
	AssignResReqT			AssignResReq;
	AssignResRspT			AssignResRsp;
	RlsResReqT				RlsResReq;
	RlsResRspT				RlsResRsp;
	ResetT					Reset;
	ResetAckT				ResetAck;
	BeatHeartT				BeatHeart;
	BeatHeartAckT			BeatHeartAck;
	CongestionCtrlReqT		CongestionCtrlReq;
	CongestionCtrlRspT		CongestionCtrlRsp;
	ErrNotifyReqT			ErrNotifyReq;
	ErrNotifyRspT			ErrNotifyRsp;

	UTSAG_Signal_L3AddrT	UTSAG_Payload_L3Addr;
	UTSAG_Signal_UidT		UTSAG_Payload_Uid;

	//
	SetupUTT				SetupUT;
	SetupSAGT				SetupSAG;
	CallProcT				CallProc;
	AlertingT				Alerting;
	ConnectUTT				ConnectUT;
	ConnectSAGT				ConnectSAG;
	ConnectAckT				ConnectAck;
	DisconnectT				Disconnect;
	ReleaseT				Release;
	ReleaseCompleteT		ReleaseComplete;
	ModiMediaReqT			ModiMediaReq;
	ModiMediaRspT			ModiMediaRsp;
	InformationT			Information;
	AuthCmdReqT				AuthCmdReq;
	AuthCmdRspT				AuthCmdRsp;
	LoginT					Login;
	LoginRspT				LoginRsp;
	LogoutT					Logout;
	HandOverReqT			HandOverReq;
	HandOverRspT			HandOverRsp;
	HandOverCompleteT		HandOverComplete;
	StatusQryT				StatusQry;
	StatusT					Status;
	//
	MOSmsDataReqT			MOSmsDataReq;
	MOSmsDataRspT			MOSmsDataRsp;
	MTSmsDataReqT			MTSmsDataReq;
	MTSmsDataRspT			MTSmsDataRsp;
	SMSMemAvailReqT			SMSMemAvailReq;
	SMSMemAvailRspT			SMSMemAvailRsp;

};

typedef struct tagSAbisSignal
{
	TcpPktHeaderT	tcpPktHeader;	//TCP信令包头
	SigHeaderT		sigHeader;		//信令通用头部
	SigPayload		sigPayload;		//信令payload,length=sigHeader.length
}SAbisSignalT;


#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif /* __SABIS1_1STRUCT_H */  