/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    voice_msgs_struct.h
*
* DESCRIPTION: 
*
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   03/22/06   fengbing  modify LA Paging Req struct, del UT_CAP and add VersionInfo;
*   09/13/05   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__VOICE_MSGS_STRUCT_H
#define	__VOICE_MSGS_STRUCT_H

#ifdef M__SUPPORT__ENC_RYP_TION
#ifndef M_ENCRYPT_KEY_LENGTH
#define M_ENCRYPT_KEY_LENGTH (10)
#endif
#endif
//undefine PID macro in vxworks6.6
#ifdef PID
#undef PID
#endif
//==============================================================================
//struct for common use
//


typedef struct tagSigHeader
{
	UINT8	SAG_ID[4];
	UINT8	BTS_ID[2];
	UINT8	EVENT_GROUP_ID;
	UINT8	Event_ID[2];
	UINT8	Length[2];
}SigHeaderT;


//==============================================================================
//msg structs
//----------------------------------------


//----------------------------------------
//voice control msgs


typedef struct tagLAPaging
{
	UINT8 UID[4];
	UINT8 L3addr[4];
	UINT8 App_Type[2];
	UINT8 VersionInfo[2];
}LAPagingT;

typedef struct tagLAPagingRsp
{
	UINT8			UID[4];
	UINT8			L3addr[4];
	UINT8			App_Type[2];
	UINT8			Cause;	
}LAPagingRspT;

typedef struct tagDELAPagingReq
{
	UINT8			UID[4];
}DELAPagingReqT;

typedef struct tagDELAPagingRsp
{
	UINT8			UID[4];
	UINT8			Cause;
}DELAPagingRspT;

typedef struct tagAssignResReq
{
	UINT8			UID[4];
	UINT8			L3addr[4];
	UINT8			ReqRate;
	UINT8           ServerOption[2];
	UINT8           ServeAppType;
	
}AssignResReqT;

typedef struct tagAssignResRsp
{
	UINT8			UID[4];
	UINT8			L3addr[4];
	UINT8			AssignResult;
}AssignResRspT;

typedef struct tagRlsResReq
{
	UINT8			L3addr[4];
	UINT8			RlsCause;
}RlsResReqT;

typedef struct tagRlsResRsp
{
	UINT8			L3addr[4];
}RlsResRspT;

typedef struct tagReset
{
	UINT8			RestCause;
	UINT8			year[2];
	UINT8			month;
	UINT8			day;
	UINT8			hour;
	UINT8			minute;
	UINT8			second;
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
	UINT8			Uid[4];				//UID
	UINT8			L3Addr[4];			//L3addr
	UINT8			ErrCause;	
}ErrNotifyReqT;

typedef struct tagErrNotifyRsp
{
	UINT8			Uid[4];				//UID
	UINT8			L3Addr[4];			//L3addr
}ErrNotifyRspT;

typedef struct tagUTSAG_Signal_XXX
{
	UINT8 XXX[4];
	UINT8 msgType;
	UINT8 UTSAGPayload[1024];
}UTSAG_Signal_XXXT;

typedef struct tagUTSAG_Signal_L3Addr
{
	UINT8 L3Addr[4];
	UINT8 msgType;
	UINT8 UTSAGPayload[1024];
}UTSAG_Signal_L3AddrT;

typedef struct tagUTSAG_Signal_Uid		
{
	UINT8 Uid[4];
	UINT8 msgType;
	UINT8 UTSAGPayload[1024];
}UTSAG_Signal_UidT;

typedef struct tagUTSXC_Signal_GrpL3Addr
{
	UINT8 GrpL3Addr[4];
	UINT8 msgType;
	UINT8 UTSAGPayload[1024];
}UTSXC_Signal_GrpL3AddrT;

typedef struct tagUTSXC_Signal_GrpUid		
{
	UINT8 GrpUid[4];
	UINT8 msgType;
	UINT8 UTSAGPayload[1024];
}UTSXC_Signal_GrpUidT;

typedef struct tagBroadcast_SM	
{
	UINT8 GlobalMsgSeq[4];
	UINT8 ServiceCategoryIndicator[2];
	UINT8 ServiceCategory[2];
	UINT8 SequenceNumber[2];
	UINT8 BroadcastData;
}Broadcast_SMT;

typedef struct tagBroadcast_SM_ACK	
{
	UINT8 GlobalMsgSeq[4];
	UINT8 BroadcastStatus;
}Broadcast_SM_ACKT;

typedef struct tagSendRejectToSAG
{
	UINT8			RejectReason;
}SendRejectToSAGT;

typedef struct tagRestartIndiToSAG
{
	UINT8			Indication;
}RestartIndiToSAGT;

typedef struct tagAuthInfoReq
{
	UINT8	Uid[4];
	UINT8	Pid[4];
}AuthInfoReqT;

typedef struct tagAuthCmdReq
{
	UINT8	Uid[4];
	UINT8	Pid[4];
	UINT8	Rand[16];
}AuthCmdReqT;

typedef struct tagAuthRsp
{
	UINT8	Uid[4];
	UINT8	Pid[4];
	UINT8	Res[4];
}AuthRspT;

typedef struct tagAuthResult
{
	UINT8	Uid[4];
	UINT8	Pid[4];
	UINT8	Auth_res;
}AuthResultT;

typedef struct tagBwInfoReq
{
	UINT8	Uid[4];
	UINT8	Pid[4];
}BwInfoReqT;

typedef struct tagBwInfoRsp
{
	UINT8	Uid[4];
	UINT8	Pid[4];
	UINT8	Admin_Status;
	UINT8	Perf_Log_status;
	UINT8	Perf_Data_Collect_Interval[2];
	UINT8	BwInfo[200];
}BwInfoRspT;

typedef struct tagBwInfoDelReq
{
	UINT8	Uid[4];
}BwInfoDelReqT;

typedef struct tagBwInfoModifyReq
{
	UINT8	Uid[4];							//	UID 4	M
	UINT8	Admin_Status;					//	Admin_Status	1	M
	UINT8	Perf_Log_Status;				//	Perf_Log_Status 1	M
	UINT8	Perf_Data_Collect_Interval[2];	//	Perf_Data_Collect_Interval	2	M
	UINT8	BwInfo[200];						//	BWInfo	N	M
}BwInfoModifyReqT;

typedef struct tagBwInfoModifyRsp
{
	UINT8	Uid[4];//	UID 4	M
	UINT8	Flag;//	Flag	1	M
}BwInfoModifyRspT;

typedef struct _SXCGrpResT
{
	UINT8 SCG_INDEX;//SCG_INDEX 1	C		���ز������
	UINT8 DL_DSCH_SLOT_INDEX;//DL_DSCH _SLOT_INDEX 1	C		DSCH��������ʱ϶���
	UINT8 DL_DSCH_SCH_INDEX;//DL_DSCH_SCH_INDEX 1	C		DSCH�����������ŵ���
	UINT8 UL_SLOT_INDEX;//UL_SLOT_INDEX 1	C		USCCH��������ʱ϶���
	UINT8 UL_SCH_INDEX;//UL_SCH_INDEX	1	C		USCCH�����������ŵ���
}SXCGrpResT;

typedef struct tagGrpLAPaging
{
	UINT8 GID[2];			//GID	2	M		
	UINT8 GrpSize[2];			//Group size	2	M		���Ա�ڸû�վ����Ŀ
	UINT8 GrpL3Addr[4];		//L3Addr	4	M		��L3
	UINT8 CommType;		//Communication Type	1	M		ͨ������
	UINT8 CallPrioty;			//Call priority	1	M		��������Ȩ
	UINT8 EncryptFlag;		//Encryption Flag	1	M		�˵��˼���
	UINT8 transID;
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8 EncryptKey[M_ENCRYPT_KEY_LENGTH];//fengbing 20100204
#endif
}GrpLAPagingT;

typedef struct tagGrpLAPagingRsp
{
	UINT8 UID[4];		//UID	4	M		
	UINT8 GrpL3Addr[4];	//L3Addr	4	M		��L3��ַ
	UINT8 commType;	//Communication Type	1	M		ͨ������
	UINT8 Cause;			//Cause	1	M		
}GrpLAPagingRspT;

typedef struct tagGrpResReq
{
	UINT8 GID[2];		//GID	2	M		
	UINT8 UID[4];		//UID	4	M		
	UINT8 commType;	//Communication Type	1	M		ͨ������
//GrpVersion003 begin
	UINT8 PID[4];
//GrpVersion003 end	
}GrpResReqT;

typedef struct tagGrpResRsp
{
	UINT8 GID[2];		//GID	2	M		
//GrpVersion003 begin
	UINT8 UID[4];
//GrpVersion003 end	
	UINT8 GrpL3Addr[4];	//L3 Addr	4	M		��L3
	UINT8 Cause;			//Cause	1	M		�ɹ�/ʧ��
//GrpVersion003 begin
	UINT8 PID[4];
//GrpVersion003 end	
	UINT8 transID;
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8 EncryptFlag;	//fengbing 20100204//Encryption Flag	1	M		�˵��˼���
	//UINT8 EncryptKey[M_ENCRYPT_KEY_LENGTH];//fengbing 20100204//�����Կ����	10	O		��Encryption FlagָʾΪ�ܺ�ʱ��Я��
#endif
}GrpResRspT;

typedef struct tagPttPressApplyReq
{
	UINT8 UID[4];		//UID	4	M		
	UINT8 GID[2];		//GID	2	M		
	UINT8 prio;			//prio	1	M		Ԥ��
	UINT8 EncryptCtrl;	//Encrypt Control	1	M		���ܿ���
	UINT8 sessionType[2];//Session Type	2	M		0�������ŵ�;1��ҵ���ŵ�
}PttPressApplyReqT;

typedef struct tagPttPressApplyRsp
{
	UINT8 UID[4];		//UID	4	M		
	UINT8 GID[2];		//GID	2	M		
	UINT8 TransmissionGrant;	//Transmission grant	1	M		����/������/�Ŷ�/���������
	UINT8 EncryptCtrl;	//Encryption Flag	1	M		�˵��˼��ܱ�־λ
	UINT8 sessionType[2];//Session Type	2	M		0�������ŵ�;1��ҵ���ŵ�	
}PttPressApplyRspT;

typedef struct tagLEGrpPaging
{
	UINT8 GID[2];		//GID	2	M		
	UINT8 GrpL3Addr[4];	//L3Addr	4	M		��L3
//GrpVersion003 begin
	UINT8 GrpSize[2];
//GrpVersion003 end	
	UINT8 commType;	//Communication Type	1	M		
	UINT8 CallPrioty;		//Call priority	1	M		��������Ȩ
	UINT8 EncryptFlag;	//Encryption Flag	1	M		�˵��˼���
	UINT8 LEPeriod;		//LE period	1	M		�ٺ�����
//GrpVersion003 begin
	UINT8 transID;
//GrpVersion003 end	
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8 EncryptKey[M_ENCRYPT_KEY_LENGTH];//fengbing 20100204
#endif
}LEGrpPagingT;

typedef struct tagDeLEGrpPaging
{
	UINT8 GID[2];		//GID	2	M		
}DeLEGrpPagingT;

typedef struct _GrpStatus
{
	UINT8	UID[4];
	UINT8	status;
}GrpCpeStatusT;

#define M_MAX_CPENUM_IN_ONE_STATUSREPORT	(100)
typedef struct tagStatusReport
{
	UINT8 GrpL3Addr[4];	//��L3	4	M		
	UINT8 num;			//NUM	1	M		
	GrpCpeStatusT UserStatusArr[M_MAX_CPENUM_IN_ONE_STATUSREPORT];
}StatusReportT;

typedef struct tagGrpHandoverReq
{
	UINT8 GID[2];		//GID	2	M		
	UINT8 UID[4];		//UID	4	M		
	UINT8 HO_Type;		//HO_Type	1	M		ͨ��̬/��ͨ��̬�л�
	UINT8 VersionInfo[2];	//Version info 	2	M		
//GrpVersion003 begin
	UINT8 PID[4];
	UINT8 curBTSID[4];
//GrpVersion003 end	
}GrpHandoverReqT;

typedef struct tagGrpHandoverRsp
{
	UINT8 GID[2];		//GID	2	M		
	UINT8 UID[4];		//UID	4	M		ע
	UINT8 Result;		//Handover Result	1	M		
	UINT8 GrpL3Addr[4];	//L3 addr	4	M		ͬ������Я����L3�������ȫF
//GrpVersion003 begin
	UINT8 PID[4];
	UINT8 curBTSID[4];
//GrpVersion003 end	
	UINT8 transID;
}GrpHandoverRspT;

typedef struct tagDVoiceCfgReq
{
	UINT8 DVoice;		//DVoice	1	M
	UINT8 L3Addr1[4];	//L3 Addr1	4	M
	UINT8 L3Addr2[4];	//L3 Addr2	4	M
}DVoiceCfgReqT;

typedef struct tagBtsL2SxcMsg
{
	UINT8 msgType1;
	UINT8 payload[10];
}BtsL2SxcMsgT;

union SigPayload
{
	LAPagingT				LAPaging;
	LAPagingRspT				LAPagingRsp;
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
	ErrNotifyReqT				ErrNotifyReq;
	ErrNotifyRspT				ErrNotifyRsp;

	UTSAG_Signal_XXXT		UTSAG_Signal_XXX;
	UTSAG_Signal_L3AddrT		UTSAG_Payload_L3Addr;
	UTSAG_Signal_UidT		UTSAG_Payload_Uid;
	UTSXC_Signal_GrpL3AddrT 	UTSXC_Payload_GrpL3Addr;
	UTSXC_Signal_GrpUidT 		UTSXC_Payload_GrpUid;
	
	Broadcast_SMT           		Broadcast_SM;
	Broadcast_SM_ACKT       	Broadcast_SM_ACK;
	SendRejectToSAGT        	SendRejectToSAG;
	RestartIndiToSAGT       		RestartIndiToSAG;

	GrpLAPagingT				GrpLAPaging;
	GrpLAPagingRspT			GrpLAPagingRsp;
	GrpResReqT				GrpResReq;
	GrpResRspT				GrpResRsp;
	PttPressApplyReqT			PttPressApplyReq;
	PttPressApplyRspT			PttPressApplyRsp;
	LEGrpPagingT				LEGrpPaging;
	DeLEGrpPagingT			DeLEGrpPaging;
	StatusReportT				StatusReport;
	GrpHandoverReqT			GrpHandoverReq;
	GrpHandoverRspT			GrpHandoverRsp;

	DVoiceCfgReqT			DVoiceCfgReq;
	BtsL2SxcMsgT			BtsL2SxcMsg;
	
};

typedef struct tagVoiceVCRCtrlMsg
{
	SigHeaderT		sigHeader;	//����ͨ��ͷ��
	SigPayload		sigPayload;	//����payload,length=sigHeader.length
}VoiceVCRCtrlMsgT;

/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////��L2��Ⱥ��ؽӿ���Ϣ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
#define M_GRP_L2L3_SUCCESS		(0)
#define M_NOTUSE_TRANSID		(0xff)

//Group_Resource.request	0X3C02
typedef enum
{
	FREE_GRPRES=0,	// 0���ͷŸ������Դ
	GET_GRPRES,	// 1�����������Դ
	GET_GRPRES_VIDEOUT
}ENUM_GrpResOptType;
#define ALLOC_GRPRES (GET_GRPRES)
#define QUERY_GRPRES (GET_GRPRES)

typedef enum
{
	NO_REPORTID,	// 0������Ҫ����״̬�������
	NEED_REPORTID,	// 1����Ҫ����״̬�������
	REPORTID_IN_EFFECT=1
}ENUM_ReportIDFlag;

typedef enum
{
	NOT_NEED_RSP,
	NEED_RSP
}ENUM_RspFlag;
typedef struct tagL2L3GrpResReq
{
	UINT8 cid;
	UINT8 GID[2];
	UINT8 Operation;
	UINT8 ReportIndexFlag;
	UINT8 Reason;
	UINT8 transID;
	UINT8 needRspFlag;
	UINT8 btsID[4];
	UINT8 EID[4];
	UINT8 grpSize[2];
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8 EncryptFlag;
#endif
}L2L3_GrpResReqT;

//Group_Resource.confirmation	0X3C03
typedef struct _AirRes
{
	UINT8 SCG_INDEX;			//���ز����
	UINT8 DL_DSCH_SLOT_INDEX;	// DSCH��ռ����ʱ϶���
	UINT8 DL_DSCH_SCH_INDEX;	// DSCH��ռ�������ŵ���
	UINT8 UL_SLOT_INDEX;		//����ʱ϶���
	UINT8 UL_SCH_INDEX;		//�������ŵ���
	UINT8 ReportPeriod[2];
	UINT8 BackoffWin[2];
	UINT8 EncryptFlag;
}AirResT;

typedef struct _AirResPart2
{
	UINT8 len;
	UINT8 content;
	UINT8 GetStructLen(){return(len+1);};
}AirResPart2T;

typedef struct _VideoCpeGrpRes
{
	UINT8 realResFlag;//Real_Resource_Flag	8	0�������� //1������
	UINT8 Use_Video_Dsch_Flag;//20100107fengbing//0��δʹ����ƵDSCH;1��ʹ����ƵDSCH
	UINT8 SCG_INDEX;//SCG_INDEX	8	���ز����
	UINT8 DL_DSCH_SLOT_INDEX;//DL_DSCH_SLOT_INDEX	8	DSCH��������ʱ϶���
	UINT8 DL_DSCH_SCH_INDEX;//DL_DSCH_SCH_INDEX	8	DSCH�����������ŵ���
	UINT8 UL_SLOT_INDEX;//UL_SLOT_INDEX	8	USCCH��������ʱ϶���
	UINT8 UL_SCH_INDEX;//UL_SCH_INDEX	8	USCCH�����������ŵ����
}VideoCpeGrpResT;//20100105fengbing

typedef struct tagGrpResCfm
{
	UINT8 cid;
	UINT8 GID[2];
	UINT8 Operation;
	UINT8 ReportIndexFlag;
	UINT8 Reason;
	UINT8 Result;
	UINT8 needRspFlag;
	UINT8 transID;
	AirResT airRes;
	UINT8 btsID[4];
	UINT8 EID[4];
	UINT8 ReportID[2];
}L2L3_GrpResCfmT;

//Group_Signal.request	0X3C06
typedef struct tagGrpSignalReq
{
	UINT8 cid;
	UINT8 GID[2];			//GID	16	���ʶ
	UINT8 payload[2048];		//Payload	Variable	ע��Payload����ΪL3͸����Ϣ
}L2L3_GrpSignalReqT;

typedef enum
{
//Paging_Type	8	0���㲥1���������2����ý�����3���������������Ԥ��
	PAGINGTYPE_BROADCAST,	
	PAGINGTYPE_GRP_VOICE,
	PAGINGTYPE_GRP_MULTIMEDIA,
	PAGINGTYPE_GRP_DATASRV,
	PAGINGTYPE_GRP_SYNCBROADCAST_VOICE,
	PAGINGTYPE_GRP_SYNCBROADCAST_DATA
}ENUM_GrpPagingType;

#ifdef M__SUPPORT__ENC_RYP_TION
////�˵��˼���ָʾ
typedef enum
{
	ENCRYPT_CTRL_NO_ENCRYPTION,////00��������
	ENCRYPT_CTRL_ENCRYPT_WITH_KEY,////01�����ܣ���Ҫ������Կ����
	ENCRYPT_CTRL_ENCRYPT_WITHOUT_KEY,////10�����ܣ�����Ҫ������Կ����
	ENCRYPT_CTRL_RESERVED////11��Ԥ��
}ENUM_EncryptCtrlFlag;
#endif

typedef enum
{
	ISNOT_LEPAGING,
	IS_LEPAGING
}ENUM_LEFlag;

//Group_Paging.request	0X3C08
typedef struct tagGrpPagingReq
{
	UINT8 cid;
	UINT8 GID[2];		//GID	16	���ʶ����
	UINT8 PagingType;	//Paging_Type	8	0���㲥1���������2����ý�����3���������������Ԥ��
	UINT8 needRspFlag;	//Response_Ind	8	��Ӧָʾ��0������Ҫ��Ӧ1����Ҫ��Ӧ������Ԥ��
	UINT8 isLEPaging;	//Late_Enty_Ind	8	�ٺ����ָʾ0���ǳٺ����1���ٺ����
	UINT8 grpSize[2];		//Size	16	���С
	UINT8 pagingTimes;	//Paging_Times	8	Ѱ������
	UINT8 transID;		//Transaction_Id	8	�ỰID
	UINT8 callPrioty;		//Call_Priority	8	000��δ�������ȼ�001-111���û��������ȼ�
	UINT8 EncryptFlag;	//EncryptionFlag	8	�˵��˼���ָʾ�˵��˼���ָʾ00��������01�����ܣ���Ҫ������Կ����10�����ܣ�����Ҫ������Կ����11��Ԥ��
	UINT8 DataPrio;//Service_Priority	8	�������������ҵ�����ȼ�0~3���������ȼ�	������Ԥ��
	UINT8 CodeType;//CodeType	8	��ɾ��������, �������Я��
	UINT8 DB_sn;//DB_sn	8	�鲥���ݿ����к�, �������Я��
	UINT8 SrvType;//Service_type	8	0000���ı����߶������·�	0001���ļ��·�	0010����ý���·�	������Ԥ�� 	�������Я��
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8 EncryptKey[M_ENCRYPT_KEY_LENGTH];//fengbing 20100204//�����Կ����	10	O		��Encryption FlagָʾΪ�ܺ�ʱ��Я��
#endif
}L2L3_GrpPagingReqT;

//Group_Paging.response	0X3C09
typedef struct tagGrpPagingRsp
{
	UINT8 cid;
	UINT8 GID[2];		//GID	16	���ʶ����
	UINT8 Result;		//RESULT	8	Ѱ�����0���ɹ�1��ʧ��������Ԥ��
	UINT8 EID[4];		//EID	32	�ն˱�ʶ
}L2L3_GrpPagingRspT;

//Group_Status_report.indication	0X3C0B
typedef struct tagStatusReportIndi
{
	UINT8 cid;
	UINT8 GID[2];		//GID	16	���ʶ
	UINT8 EID[4];		//EID	32	�ձ�ʶ
}L2L3_StatusReportIndiT;

//bts_ptt_press_req	0X3C17	bts L2->L3
typedef struct tagBtsPttPressReq
{
	UINT8 cid;			//CID	8	
	UINT8 GID[2];		//GID	16	���ʶ
	UINT8 prio;			//PTT Priority	8	
	UINT8 EncryptCtrl;	//Encrypt Control	8	
}L2L3_BtsPttPressReqT;
//bts_ptt_press_rsp	0X3C18	bts L3->L2
typedef struct tagBtsPttPressRsp
{
	UINT8 cid;			//CID	8	
	UINT8 GID[2];		//GID	16	���ʶ
	UINT8 Grant;
	UINT8 EncryptCtrl;
	//UINT8 result;			//Result	8	���1����Ȩ����2��ȡ���Ŷӣ��������ȼ���ռ��3���Ŷӳɹ�4���鲻����������Ԥ��
}L2L3_BtsPttPressRspT;
//bts_ptt_press_cancel.	0X3C19	bts L2->L3
typedef struct tagBtsPttPressCancel
{
	UINT8 cid;			//CID	8	
	UINT8 GID[2];		//GID	16	���ʶ
}L2L3_BtsPttPressCancelT;
//bts_ptt_press_release	0X3C1a	bts L2->L3
typedef struct tagBtsPttPressRelease
{
	UINT8 cid;			//CID	8	
	UINT8 GID[2];		//GID	16	���ʶ
}L2L3_BtsPttPressReleaseT;

//MBMSGroupResource.Indication����վL3��֪ͨ��վL2������������Դ����Ϣ
typedef struct _L2L3GrpResIndicationT
{
	UINT8 CID;//CID 8	
	UINT8 GID[2];//GID 16	���ʶ
	UINT8 Report_Index_Ind;//Report_Index_Ind	8	0����״̬�������//1����״̬�������//������Ԥ��
	UINT8 Transaction_Id;//Transaction_Id	8	�ỰID
	SXCGrpResT sxcGrpRes;
	UINT8 Report_Period[2];//Report_Period	16	״̬�������ڣ���λ��frame��
	UINT8 Backoff_Win[2];//Backoff_Win 16	�˱ܴ���С
	UINT8 Report_Index[2];//Report_Index	16	״̬�������	
}L2L3_GrpResIndicationT;

/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////��CPE��Ⱥ��ؽӿ���Ϣ//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	NORMAL_UT=0,
	VIDEO_UT
}ENUM_ReqGrpResCpeType;//fengbing 20091229 ������Ƶ�ն�������Դ
//��UID͸����Ϣ�ӿ�
union payloadUnion
{
	UINT8 buf[2000];
	UINT8 GID[2];
};
typedef struct tagCPEGrpUIDSignal
{
	UINT8 cid;
	UINT8 msgType;
	payloadUnion payload;
}CPEGrpUIDSignalT;

//��L3͸����Ϣ�ӿ�
typedef struct tagCPEGrpL3AddrSignal
{
	UINT8 cid;
	UINT8 msgType;
	UINT8 GID[2];
	UINT8 payload[2000];
}CPEGrpL3AddrSignalT;

//Ho_Resource.request
typedef struct tagCPEHoResReq
{
	UINT8 cid;		//CID	1	M
	UINT8 rsv1;		//Reserved 1 byte
	UINT8 btsID[4];	//TargetBTS ID	4	M
	UINT8 GID[2];	//GID	2	M
	UINT8 EID[4];	//EID	4	M
	UINT8 UID[4];	//UID	4	M
//GrpVersion003 begin
	UINT8 CommType;
//GrpVersion003 end	
	UINT8 ReqType;//fengbing 20091229 ������Ƶ�ն�������Դ
}CPE_HoResReqT;

//Ho_Resource.response
typedef struct tagCPEHoResRsp
{
	UINT8 cid;		//CID	8	M
	UINT8 rsv1;		//Reserved 1 byte
	UINT8 btsID[4];	//TargetBTS ID	32	M
	UINT8 GID[2];	//GID	16	M
	UINT8 Result;	//Result	8	M
	UINT8 reportID_Ind;
	UINT8 transID;	//Transaction_Id	8	M

					//SCG_INDEX	8	M
					//DL_DSCH _SLOT_INDEX	8	M
					//DL_DSCH_SCH_INDEX	8	M
					//UL_SLOT_INDEX	8	M
					//UL_SCH_INDEX	8	M
					//Report_Period	16	״̬�������ڣ���λ��frame��
					//Backoff_Win	16	�˱ܴ���С
					//EncryptionFlag	8	�˵��˼���

	AirResT airRes;
	UINT8 reportID[2];	//Report_Index	16	M
}CPEHoResRspT;

//GroupShare_Resource.Request
typedef struct tagCPEGrpResReq
{
	UINT8 cid;		//CID	1	M
	UINT8 GID[2];	//GID	2	M
	UINT8 EID[4];	//EID	4	M
	UINT8 UID[4];	//UID	4	M
//GrpVersion003 begin
	UINT8 CommType;
//GrpVersion003 end	
	UINT8 ReqType;//fengbing 20091229 ������Ƶ�ն�������Դ
}CPEGrpResReqT;

//GroupShare_Resource.Response
typedef struct tagCPEGrpResRsp
{
	UINT8 cid;		//CID	8	M
	UINT8 GID[2];	//GID	16	M
	UINT8 Result;	//Result	8	M
	UINT8 reportID_Ind;
	UINT8 transID;	//Transaction_Id	8	M
	
					//SCG_INDEX	8	M
					//DL_DSCH _SLOT_INDEX	8	M
					//DL_DSCH_SCH_INDEX	8	M
					//UL_SLOT_INDEX	8	M
					//UL_SCH_INDEX	8	M
					//Report_Period	16	״̬�������ڣ���λ��frame��
					//Backoff_Win	16	�˱ܴ���С
					//EncryptionFlag	8	�˵��˼���

	AirResT airRes;
	UINT8 reportID[2];//Report_Index	16	M
}CPEGrpResRspT;

//Group_Call_Indication
typedef struct tagGrpCallInd
{
	UINT8 cid;			//CID	8	
	UINT8 GID[2];		//GID	16	���ʶ
	UINT8 callPrioty;		//�������ȼ�	8	000��δ�������ȼ�001-111���û��������ȼ�
	UINT8 pagingType;	//Type	8	000���㲥001���������010�����ý�����011���������������Ԥ��
	UINT8 LEFlag;		//Late_Entry_Ind	8	�ٺ����ָʾ0���ǳٺ����1���ٺ����
	UINT8 needRsp;		//Response_Ind	8	�����Ӧָʾ0������Ҫ��Ӧ1����Ҫ��Ӧ
	UINT8 transID;		//Transaction_Id	8	�ỰID
#ifdef M__SUPPORT__ENC_RYP_TION
	UINT8 EncryptFlag;	//fengbing 20100204//Encryption Flag	1	M		�˵��˼���
	UINT8 EncryptKey[M_ENCRYPT_KEY_LENGTH];//fengbing 20100204//�����Կ����	10	O		��Encryption FlagָʾΪ�ܺ�ʱ��Я��
#endif
}GrpCallIndT;

typedef struct tagPttPressReq
{
	UINT8	Cid;
	UINT8	msgType;
	UINT8	Gid[2];
	UINT8	CallPrioity;
	UINT8	EncryptControl;
}PTTPressReqT;

typedef struct tagPttPressRsp
{
	UINT8	Cid;
	UINT8	msgType;	
	UINT8	GID[2];
	UINT8	Grant;
	UINT8	EncryptCtrl;
}PTTPressRspT;

/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////BTS�伯Ⱥ��ؽӿ���Ϣ//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//Ho_Resource.Request
typedef CPE_HoResReqT BTS_HoResReqT;//fengbing 20091229
	
//Ho_Resource.Response
typedef struct tagBTSHoResRsp
{
	UINT8 cid;			//CID	8	M
	UINT8 btsID[4];		//TargetBTS ID	32	M
	UINT8 GID[2];		//GID	16	M
	UINT8 EID[4];		//EID	32
	UINT8 Result;		//Result	8	M
	UINT8 reportID_Ind;	//0����״̬�������1����״̬�������������Ԥ��
	UINT8 transID;		//Transaction_Id	8	M
					//SCG_INDEX	8	M
					//DL_DSCH _SLOT_INDEX	8	M
					//DL_DSCH_SCH_INDEX	8	M
					//UL_SLOT_INDEX	8	M
					//UL_SCH_INDEX	8	M
					//Report_Period	16	״̬�������ڣ���λ��frame��
					//Backoff_Win	16	�˱ܴ���С
					//EncryptionFlag	8	�˵��˼���

	AirResT airRes;
	UINT8 reportID[2];	//Report_Index	16	M					
}BTSHoResRspT;



#endif /* __VOICE_MSGS_STRUCT_H */  


