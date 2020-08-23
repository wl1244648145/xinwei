/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    voiceCommon.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   2006-09-06 fengbing  信令种类增加透传类信令callticket,callticketAck,SRQ,SRP,SRC
 *   2006-08-21 fengbing  AppType定义更改
 *   2006-04-19 fengbing  加入任务接口上信令＆语音数据的计数统计
 *   2006-04-09 fengbing  修改与VAC相关定时器时长(TIMER_VAC:5sec-->2sec)
 *   2006-04-09 fengbing  根据SAbis1接口文档修改编码方式的定义，会话优先级的定义，
 *                        DMUX语言数据优先级的定义,APP_TYPE字段的定义，Request transport
 *                        data rate的变化
 *   2006-3-26  fengbing  add congest level definition
 *   2006-3-22  fengbing  modify timeout value;
 *   2005-9-13  fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef	__VOICECOMMON_H
#define	__VOICECOMMON_H

//#define SP_COMM
#ifdef SP_COMM

#define M__SUPPORT__ENC_RYP_TION
#define M_L2L3_GRPRES_USE_LV_STRUCT

#else



#endif

//undefine PID macro in vxworks6.6
#ifdef PID
#undef PID
#endif

#ifdef DSP_BIOS
extern "C" int yyprintf(char *fmt,...);
#define VPRINT yyprintf
#else
#define VPRINT printf
#endif

#ifndef M_ENCRYPT_KEY_LENGTH
#define M_ENCRYPT_KEY_LENGTH (10)
#endif

#define M_TIMER_TYPE_PERIOD	(1)
#define M_TIMER_TYPE_ONCE		(0)

#define M_BROADCAST_MSG_SMS	(0)

//定时器ID
typedef enum _ENUM_TIMER_TVOICE
{
	TIMER_MIN_TYPE=0,

	TIMER_ASSIGN,
	TIMER_VAC,
	TIMER_ERRRSP,
	TIMER_RELRES,
	TIMER_PROBERSP,
	TIMER_DELAY_RELVAC,

	TIMER_BEATHEART,
	TIMER_CONGEST,

	TIMER_GrpPagingRsp,
	TIMER_LePagingRsp,
	TIMER_StautsReport,
	TIMER_LePagingLoop,
	TIMER_ResClear,
	TIMER_LePagingStart,	
	TIMER_GrpDataDetect,
	TIMER_GrpRls,

	TIMER_MAX
}ENUM_TIMER_TVOICE;

//定时器时长,单位秒
#define VOICE_ONE_MS			(1)
#define VOICE_ONE_SECOND		(VOICE_ONE_MS*1000)
#define VOICE_ONE_MINUTE		(VOICE_ONE_SECOND*60)

#define M_TIMERLEN_ASSIGN		(7*VOICE_ONE_SECOND)
#define M_TIMERLEN_VAC			(5*VOICE_ONE_SECOND)	//针对睡眠修改语音被叫空口建链超时时长，提高被叫接通率
#define M_TIMERLEN_ERRRSP		(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_RELRES		(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_PROBERSP		(16*VOICE_ONE_SECOND)

#define M_TIMERLEN_BEATHEART	(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_CONGEST		(60*VOICE_ONE_SECOND)

#define M_TIMERLEN_MOVEAWAY_DISABLE	(10*VOICE_ONE_SECOND)
#define M_TIMERLEN_WAITSYNC		(8*VOICE_ONE_SECOND)//(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_DELAY_RELVAC (400*VOICE_ONE_MS)		//延迟释放VAC

#define M_TIMERLEN_GRPPAGINGRSP		(5*VOICE_ONE_SECOND)//(15*VOICE_ONE_SECOND)	//fengbing 20090515 组呼首次寻呼超时改为5秒
#define M_TIMERLEN_LEPAGINGRSP		(1000)
#define M_TIMERLEN_STATUSREPORT		(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_LEPAGINGLOOP		(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_RESCLEAR			(10*VOICE_ONE_SECOND)
#define M_TIMERLEN_LEPAGINGSTART		(20*VOICE_ONE_SECOND)
#define M_TIMERLEN_GRPDATADETECT		(60*VOICE_ONE_SECOND)
#define M_TIMERLEN_GRPRLS				(150)//(50)

//////////////////////////////////////////////////////////////////////////

/*
 *	BTS-SAG message Event Group ID
 */
#define M_MSG_EVENT_GROUP_ID_CALLCTRL	(0x00)	//呼叫控制消息
#define M_MSG_EVENT_GROUP_ID_RESMANAGE	(0x01)	//资源管理消息
#define M_MSG_EVENT_GROUP_ID_MANAGE		(0x02)	//管理类消息
#define M_MSG_EVENT_GROUP_ID_UTSAG		(0x03)	//UE-SAG直接传输消息
#define M_MSG_EVENT_GROUP_ID_BROADCAST	(0x04)	//广播业务消息
#define M_MSG_EVENT_GROUP_ID_UTSAGOAM	(0x05)	//McBTS转发的UE-SAG类消息
#define M_MSG_EVENT_GROUP_ID_CPEOAM		(0x06)	//McBTS透传的OAM消息
#define M_MSG_EVENT_GROUP_ID_RELAYMSG	(0x07)	//UID寻址的UE-SAG OAMmessage	0x01		0x07
#define M_MSG_ENENT_GROUP_ID_BTSL2SAG	(0x08)	//基站层二与SAG的消息，基站层三透传

#define M_MSG_EVENT_GROUP_NUM			M_MSG_ENENT_GROUP_ID_BTSL2SAG	

/*
 *	BTS-SAG message Event ID
 */
//呼叫控制消息
#define M_MSG_EVENT_ID_LAPAGING				(0x01)		//LA Paging	0x01
#define M_MSG_EVENT_ID_LAPAGINGRSP			(0x02)		//LA Paging Response	0x01
#define M_MSG_EVENT_ID_DELAPAGING			(0x03)		//De-LA Paging	0x02
#define M_MSG_EVENT_ID_DELAPAGINGRSP		(0x04)		//De-LA Paging Response	0x02
#define M_MSG_EVENT_ID_LAGrpPaging			(0x05)		//LA Group paging	0x05	呼叫控制消息	0x00
#define M_MSG_EVENT_ID_LAGrpPagingRsp			(0x06)		//LA Group paging rsp	0x06	呼叫控制消息	0x00
#define M_MSG_EVENT_ID_LEGrpPaging			(0x07)		//LE Group paging	0x07	呼叫控制消息	0x00
#define M_MSG_EVENT_ID_DELEPaging				(0x08)		//De-LE Group paging	0x08	呼叫控制消息	0x00
#define M_MSG_EVENT_ID_UserStatusReport		(0x09)		//USER_STATUS_REPORT	0x09	呼叫控制消息	0x00
#define M_MSG_EVENT_ID_GrpHandoverReq		(0x0A)		//Group Handover req	0x0A	呼叫控制消息	0x00
#define M_MSG_EVENT_ID_GrpHandoverRsp		(0x0B)		//Group Handover rsp	0x0B	呼叫控制消息	0x00
#define M_MSG_EVENT_ID_DVoiceConfigReq		(0x10)		//DVoice Config
#define M_MSG_EVENT_ID_DBGrpPagingReq		(0x11)		//DB Group paging	0x11		
#define M_MSG_EVENT_ID_DBGrpPagingRsp		(0x12)		//DB Group paging response	0x12	呼叫控制消息	0x00
#define M_MSG_EVENT_ID_DBDataComplete		(0x13)		//DB data complete	0x13	呼叫控制消息	0x00

//资源管理消息
#define M_MSG_EVENT_ID_ASSGNRESREQ			(0x01)		//Assignmengt transport resource req	0x03
#define M_MSG_EVENT_ID_ASSGNRESRSP			(0x02)		//Assignmengt transport resource rsp	0x04
#define M_MSG_EVENT_ID_RLSRESREQ				(0x03)		//Release transport resource req	0x05
#define M_MSG_EVENT_ID_RLSRESRSP				(0x04)		//Release transport resource rsp	0x05
#define M_MSG_EVENT_ID_RlsSMSLinkReq			(0x05)		//Release Sms link req	0x05		0x01
#define M_MSG_EVENT_ID_RlsSMSLinkRsp			(0x06)		//Release Sms link rsp	0x06		0x01
#define M_MSG_EVENT_ID_GrpResReq				(0x07)		//Group Resouse req	0x07		0x01
#define M_MSG_EVENT_ID_GrpResRsp				(0x08)		//Group Resouse rsp	0x08		0x01
#define M_MSG_EVENT_ID_PttPressApplyReq		(0x09)		//Ptt Press Apply Req 0x09	0x01
#define M_MSG_EVENT_ID_PttPressApplyRsp		(0x0A)		//Ptt Press Apply Rsp 0x0A	0x01

//管理类消息
#define M_MSG_EVENT_ID_RESET				(0x01)		//Reset	0x07
#define M_MSG_EVENT_ID_RESETACK				(0x02)		//Reset ack	0x08
#define M_MSG_EVENT_ID_ERRNOTIFYREQ			(0x03)		//Error notification req	0x09
#define M_MSG_EVENT_ID_ERRNOTIFYRSP			(0x04)		//Error notification rsp	0x0a
#define M_MSG_EVENT_ID_BEATHEART			(0x05)		//beatheart	0x0b
#define M_MSG_EVENT_ID_BEATHEARTACK			(0x06)		//Beatheart ack	0x0c
#define M_MSG_EVENT_ID_CONGESTREQ			(0x07)		//Congestion control req	0x0d
#define M_MSG_EVENT_ID_CONGESTRSP			(0x08)		//Congestion control req	0x0e

//UE-SAG直接传输消息
#define M_MSG_EVENT_ID_UTSAG_L3ADDR			(0x01)		//L3地址寻址的UE-SAG message	0x20
#define M_MSG_EVENT_ID_UTSAG_UID				(0x02)		//UID寻址的UE-SAG message	0x21
#define M_MSG_EVENT_ID_GRP_UTSAG_L3ADDR		(0x03)		//组L3寻址的UE-SAG message	0x03		0x03
#define M_MSG_EVENT_ID_GRP_UTSAG_UID		(0x04)		//组呼的UID寻址的UE-SAG message	0x04		0x03

//广播业务消息
#define M_MSG_EVENT_ID_BROADCAST_SM 			(0x01)		
#define M_MSG_EVENT_ID_BROADCAST_SM_ACK	(0x02)		
#define M_MSG_EVENT_ID_REJECT 					(0x03)		
#define M_MSG_EVENT_ID_RESET_REQ				(0x04)		
#define M_MSG_EVENT_ID_RESTART_INDICATION 	(0x05)		
#define M_MSG_EVENT_ID_FAIL_INDICATION		(0x06)		

//UT-SAG的OAM消息
#define M_MSG_EVENT_ID_AUTH_INFO_REQ		(0x01)//Auth_Info_Req_MSG,
#define M_MSG_EVENT_ID_AUTH_CMD				(0x02)//Auth_Cmd_MSG,
#define M_MSG_EVENT_ID_AUTH_RSP				(0x03)//Auth_Rsp_MSG,
#define M_MSG_EVENT_ID_AUTH_RESULT			(0x04)//Auth_Result_MSG,
#define M_MSG_EVENT_ID_BWINFO_REQ 			(0x05)//BWInfo_Req_MSG,
#define M_MSG_EVENT_ID_BWINFO_RSP 			(0x06)//BWInfo_Rsp_MSG,
#define M_MSG_EVENT_ID_BWINFO_DEL_REQ 		(0x07)//BWInfo_Del_Req_MSG,
#define M_MSG_EVENT_ID_BWINFO_MODIFY_REQ	(0x08)//BWInfo_Modify_Req_MSG,
#define M_MSG_EVENT_ID_BWINFO_MODIFY_RSP	(0x09)//BWInfo_Modify_Rsp_MSG,
#define M_MSG_EVENT_ID_SWITCH_OFF_NOTIFY	(0x0A)//SWITCH_OFF_Notify
#define M_MSG_EVENT_ID_ACCOUNT_LOGIN_REQ	(0x0B)//AccountLoginReq_MSG,	//AccountLogin req	0x0B		0x05
#define M_MSG_EVENT_ID_ConfigInfo_Transfer	(0x0C)//ConfigInfo_Transfer_Msg,//ConfigInfo_Transfer	0x0C		0x05


//bts透传的OAM消息
#define M_MSG_EVENT_ID_CPEOAM				(0x01)

//UID寻址的UE-SAG OAMmessage	0x01		0x07
#define M_MSG_EVENT_ID_RELAYMSG	(0x01)

//基站层二与SAG的消息	0x01	同播管理消息	0x08
#define M_MSG_ENENT_ID_BTSL2SXC_SYNC_BROADCAST (0x01)

/*
 *	UT－SAG透传消息的message type字段类型
 */
//呼叫控制消息
#define M_MSGTYPE_SETUP				(0x01)		//Setup	0x01
#define M_MSGTYPE_CALLPROC			(0x02)		//Call Proceeding	0x02
#define M_MSGTYPE_ALERTING			(0x03)		//Alerting	0x03
#define M_MSGTYPE_CONNECT				(0x04)		//Connect	0x04
#define M_MSGTYPE_CONNECTACK			(0x05)		//Connect Ack	0x05
#define M_MSGTYPE_DISCONNECT			(0x06)		//Disconnect	0x06
#define M_MSGTYPE_INFORMATION		(0x07)		//Information	0x07
#define M_MSGTYPE_RELEASE				(0x08)		//Release	0x08
#define M_MSGTYPE_RELEASECOMPLETE	(0x09)		//Release complete	0x09
#define M_MSGTYPE_MODIMEDIATYPEREQ	(0x0a)		//Modify media type req	0x0a
#define M_MSGTYPE_MODIMEDIATYPERSP	(0x0b)		//Modify media type rsp	0x0b
//切换消息
#define M_MSGTYPE_HANDOVERSTART		(0x10)		//Handover Start	0x10
#define M_MSGTYPE_HANDOVERSTARTACK	(0x11)		//Handover Start Ack	0x10
#define M_MSGTYPE_HANDOVERCOMPLETE	(0x12)		//Handover Complete
//移动性管理消息
#define M_MSGTYPE_LOGINREQ			(0x20)		//Login req	0x12
#define M_MSGTYPE_LOGINRSP			(0x21)		//Login rsp	0x12
#define M_MSGTYPE_LOGOUT				(0x22)		//Logout	
#define M_MSGTYPE_AUTHCMD				(0x23)		//Authentication command	0x13
#define M_MSGTYPE_AUTHRSP				(0x24)		//Authentication rsp	0x13
#define M_MSGTYPE_MODI_GIDs_REQ		(0x25)		//ATTACH/DETACHGIDs Req	0x25	移动性管理消息	UID寻址
#define M_MSGTYPE_MODI_GIDs_RSP		(0x26)		//ATTACH/DETACHGIDs Rsp	0x26	移动性管理消息	UID寻址

//短消息
#define M_MSGTYPE_MOSMSREQ			(0x30)		//MO Sms data req	0x14
#define M_MSGTYPE_MOSMSRSP			(0x31)		//MO Sms data rsp	0x14
#define M_MSGTYPE_MTSMSREQ			(0x32)		//MT Sms data req	0x15
#define M_MSGTYPE_MTSMSRSP			(0x33)		//MT Sms data rsp	0x15
#define M_MSGTYPE_SMSMEMAVAILREQ		(0x34)		//SMS memory available req	0x16
#define M_MSGTYPE_SMSMEMAVAILRSP		(0x35)		//SMS memory available rsp	0x16

#define M_MSGTYPE_CALLTICKET			(0x40)		//Call Ticket	0x40	增值业务消息	L3地址寻址
#define M_MSGTYPE_CALLTICKET_ACK		(0x41)		//Call Ticket Ack	0x41	增值业务消息	L3地址寻址
#define M_MSGTYPE_SRQ					(0x42)		//SRQ	0x42	增值业务消息	UID寻址
#define M_MSGTYPE_SRP					(0x43)		//SRP	0x43	增值业务消息	UID寻址
#define M_MSGTYPE_SRC					(0x44)		//SRC	0x44	增值业务消息	UID寻址

#define M_MSGTYPE_CHANGEPWREQ		(0x50)		//Change PW req	0x50	用户管理消息	UID寻址
#define M_MSGTYPE_CHANGEPWRSP		(0x51)		//Change PW rsp	0x51	用户管理消息	UID寻址
#define M_MSGTYPE_CHANGEPWACK		(0x52)		//Change PW Ack	0x52	用户管理消息	UID寻址
#define M_MSGTYPE_CfgFuncInfo_Req		(0x53)		//ConfigFunc Info req	0x53	用户管理消息	UID寻址（OAM）
#define M_MSGTYPE_CfgFuncInfo_Rsp		(0x54)		//ConfigFunc Info rsp	0x54	用户管理消息	UID寻址（OAM）
#define M_MSGTYPE_VoiceInfo_Req		(0x55)		//VoiceInfo.req	0x55	用户管理消息	UID寻址（UTV）
#define M_MSGTYPE_VoiceInfo_Rsp		(0x56)		//VoiceInfo.rsp	0x56	用户管理消息	UID寻址（UTV）
#define M_MSGTYPE_TRANSDATA_REQ		(0x57)		//Transdata req	0x57	用户管理消息	UID寻址
#define M_MSGTYPE_TRANSDATA_RSP		(0x58)		//Transdata rsp	0x58	用户管理消息	UID寻址
#define M_MSGTYPE_OAMTRANSFER_INFO_REQ	(0x59)		//OAMTransfer_Info req	0x59	用户管理消息	UID寻址（OAMmessage）
#define M_MSGTYPE_OAMTRANSFER_INFO_RSP	(0x5A)		//OAMTransfer_Info rsp	0x5A	用户管理消息	UID寻址(OAMmessage)
#define M_MSGTYPE_SecurityCardCallPara_Req	(0x5B)		//SecurityCardCallPara req
#define M_MSGTYPE_SecurityCardCallPara_Rsp	(0x5C)		//SecurityCardCallPara rsp


//组呼呼叫控制消息
#define M_MSGTYPE_PTT_SETUP_REQ		(0x60)		//PTT Setup Req	0x60	呼叫控制消息	L3地址寻址
#define M_MSGTYPE_PTT_SETUP_ACK		(0x61)		//PTT Setup ack	0x61	呼叫控制消息	L3地址寻址
#define M_MSGTYPE_PTT_CONNECT		(0x62)		//PTT Connect	0x62	呼叫控制消息	L3地址寻址
#define M_MSGTYPE_PTT_CONNECT_ACK	(0x63)		//PTT Connect Ack	0x63	呼叫控制消息	L3地址寻址
#define M_MSGTYPE_GRP_DISCONNECT		(0x64)		//Group Disconnect	0x64	呼叫控制消息	组L3地址寻址
#define M_MSGTYPE_GRP_RLS				(0x65)		//Group Release	0x65	呼叫控制消息	组L3地址寻址
#define M_MSGTYPE_GRP_CALLING_RLS	(0x66)		//Group Calling Release	0x66	呼叫控制消息	L3地址寻址
#define M_MSGTYPE_GRP_CALLING_RLSACK	(0x67)		//Group Calling Release Complete	0x67	呼叫控制消息	L3地址寻址

//话权控制消息
#define M_MSGTYPE_PTT_PRESS_REQ		(0x70)		//PTT PRESS REQ	0x70	话权控制消息	组呼的UID寻址
#define M_MSGTYPE_PTT_PRESS_RSP		(0x71)		//PTT PRESS RSP	0x71	话权控制消息	组呼的UID寻址
#define M_MSGTYPE_PTT_GRANTED		(0x72)		//PTT GRANTED	0x72	话权控制消息	组L3地址寻址
#define M_MSGTYPE_PTT_INTERRUPT		(0x73)		//PTT INTERRUPT	0x73	话权控制消息	L3寻址
#define M_MSGTYPE_PTT_INTERRUPT_ACK	(0x74)		//PTT INTERRUPT ACK	0x74	话权控制消息	L3寻址
#define M_MSGTYPE_PTT_RLS			(0x75)		//PTT RELEASE	0x75	话权控制消息	L3地址寻址
#define M_MSGTYPE_PTT_RLS_ACK			(0x76)		//PTT RELEASE ACK	0x76	话权控制消息	L3地址寻址
#define M_MSGTYPE_PTT_PRESS_INFO		(0x77)		//PTT PRESS INFO	0x77	话权控制消息	L3地址寻址
#define M_MSGTYPE_PTT_PRESS_CANCEL	(0x78)		//PTT PRESS CANCLE 	0x78	话权控制消息	组呼的UID寻址

//增值业务消息
#define M_MSGTYPE_USR_STATUS_QRY		(0x90)		//USER STATUS QUERY	0x90	状态报告消息	L3寻址
#define M_MSGTYPE_USR_STATUS_RSP		(0x91)		//USER_STATUS_RESPONSE	0x91	状态报告消息	L3寻址
#define M_MSGTYPE_USR_STATUS_NOTI	(0x92)		//USER_STATUS_NOTIFY	0x92	状态报告消息	L3寻址

#define M_MSGTYPE_DB_MO_DATA_REQ	(0xA0)		//MO DB data req	0xA0		UID寻址
#define M_MSGTYPE_DB_MO_DATA_RSP	(0xA1)		//MO DB data rsp	0xA1		UID寻址
#define M_MSGTYPE_DB_RETRANSDATA_REQ	(0xA2)		//RetranData req	0xA2		UID寻址
#define M_MSGTYPE_DB_RETRANSDATA_RSP	(0xA3)		//RetranData rsp	0xA3		UID寻址

#define M_MSGTYPE_MAX_VALUE			(M_MSGTYPE_DB_RETRANSDATA_RSP)
//////////////////////////////////////////////////////////////////////////
typedef enum
{
	//FORMID用于标识数据包的数据封装格式，定义如下：
	SRTP,//0b000 	用户平面语音类数据；
	SDATA,//0b001 	用户平面数据类数据；
	SyncBroadcastVData,//0b010 	  集群组呼同播用户平面语音类数据
	SyncBroadcastDData,//0b011 	  集群组呼同播用户平面数据类数据
	NATAP_PROTOCOL=7//#define M_PROTOCOL_ID_NATAP	(7)
}ENUM_FormIDT;

typedef enum
{
	CALLGRANT_FREE,////0 话权空闲
	CALLGRANT_INUSE////1 话权占用 	
}ENUM_CallGrantT;

typedef enum
{
	ENCRYPT_CTRL_NOTUSE=0,
	ENCRYPT_VOICE
}ENUM_EncryptCtrlT;

typedef enum
{
	IS_CALL_OWNER=0,
	NOT_CALL_OWNER
}ENUM_CallOwnerShip;

typedef enum
{
	PRIO_0,				// 0未定义优先级
	PRIO_1,				// 1优先级1（最低）
	PRIO_2,				// 2优先级2
	PRIO_3,				// 3优先级3
	PRIO_HIGH1,		// 4预占优先权1
	PRIO_HIGH2,		// 5预占优先权2
	PRIO_HIGH3,		// 6预占优先权3
	PRIO_EMERGENT		// 7预占优先权4（紧急呼叫）
}ENUM_CallPrioty;
#define M_DEFAULT_VOICE_PRIORITY			(PRIO_3)		//目前版本固定语音服务优先级 3
#define M_DMUX_VOICEDATA_PRIORITY		(0x04)		//DMUX接口中规定 4

typedef enum
{
	CONGEST_LEVEL0,	//0x00
	CONGEST_LEVEL1,	//0x01
	CONGEST_LEVEL2	//0x02
}ENUM_CongestLevel;

#define M_LAPAGING_CAUSE_SUCCESS	(0)		//Paging success
#define M_LAPAGING_CAUSE_HOOKOFF	(0x30)	//user hookoff, cannot ring

//Communication Type
typedef enum
{// 通信类型
	COMM_TYPE_BROADCAST,				// 0：广播
	COMM_TYPE_VOICE_GRPCALL,			// 1：语音组呼
	COMM_TYPE_MULTIMEDIA_GRPCALL,	// 2：语音多媒体组呼
	COMM_TYPE_DATA_GRPCALL,			// 3：数据组呼
	COMM_TYPE_SYNC_BROADCAST_VOICE,// 4：语音同播
	COMM_TYPE_SYNC_BROADCAST_DATA// 5：数据同播
}ENUM_CommTypeT;

typedef enum
{
	TRANSGRANT_PERMIT_TALK=1,// 1允许讲话
	TRANSGRANT_FORBID_TALK,// 2不允许讲话
	TRANSGRANT_WAITING_TALK,// 3话权排队
	TRANSGRANT_GRPSRV_NOT_EXIST,// 4该组呼不存在
	TRANSGRANT_SXC_CANCEL_WAITING_TALK// 5交换机主动取消排队	
}ENUM_TransmissionGrantT;

typedef enum
{
	USE_PAGING_CHANNEL=0,
	USE_DAC_CHANNEL
}ENUM_sessionTypeT;

//业务类型AppType
typedef enum
{
	APPTYPE_VOICE_QCELP=0,		//	00	语音应用/ QCELP语音编码的语音应用
	APPTYPE_VOICE_G729,		//	01	保留/ G729语音编码的语音应用
	APPTYPE_VOICE_G729B,		//	02	保留/ G729B语音编码
	APPTYPE_RESERVED1=3,		//	03	保留，待定义
	APPTYPE_SMS,				//	04	短消息应用
	APPTYPE_LOWSPEED_DATA,		//	05	低速数据
	APPTYPE_RESERVED2,			//	06	G729D语音编码的语音应用
	APPTYPE_BER_TEST,			//	07	表示为误码测试建链请求
	APPTYPE_G729A_BCH,          //  08  G729 plus语音编码的语音应用(G729A+BCH编码)
	APPTYPE_G729D_B,            //  09  G729D+B语音编码的语音应用
	APPTYPE_G729A_FAX,            //  0a  G729+传真
	APPTYPE_G729B_FAX,            //  0b  G729B+传真
	APPTYPE_CMD_REG=0x10,		//	0x10	指令登记
	APPTYPE_P2P_LOAD,            //  11  端到端承载业务
	APPTYPE_WEB_LOAD,            //  12  网络模式承载业务
	APPTYPE_OAM_FORCE_REGISTER,		// 13 OAM强制注册
	APPTYPE_ENCRYPT_VOICE,		// 14 加密语音应用//fengbing 20100204
	APPTYPE_VOICE_SWITCH=0x20,       //  20 语音切换应用
	APPTYPE_DUPCPE_PAGING,		//	0x21 孖机寻呼
	APPTYPE_GRP_MANAGE,			//22	组附属/去附属寻呼
	APPTYPE_ENC_SIGNAL_SEND,	//	0x23：外部处理信令下发
	APPTYPE_RESERVED		   //	保留
}ENUM_AppTypeT;

//11.36  ServiceAppType
typedef enum
{
	SERV_APPTYPE_DTE=0,		//	00	标准DTE
	SERV_APPTYPE_RATE_DWNLOAD,		    //	01	无线费率下载
	SERV_APPTYPE_REMOTE_CPE,		//	02	远程终端维护
	SERV_APPTYPE_RESERVED		    //	保留
}ENUM_ServiceAppTypeT;

//Setup Cause
typedef enum
{
	SETUPCAUSE_VOICE=0,			//	00H － 语音；
	SETUPCAUSE_DATA,				//	01H － 低速数据
	SETUPCAUSE_BROADCAST,		//	02H － 广播
	SETUPCAUSE_CARRYSRV,			//	03H － 承载业务
	SETUPCAUSE_ENCRYPT_VOICE		//04H － 加密语音
}ENUM_SetupCauseT;

//release cause
typedef enum
{
	REL_CAUSE_NORMAL=0,				//0x00	正常释放连接
	REL_CAUSE_UT,					//0x01	BS发起释放连接
	REL_CAUSE_SAG,					//0x02	网络异常发起的释放
	REL_CAUSE_USRSRVNOTALLOW,		//0x03	用户业务不允许
	REL_CAUSE_SRVNOTSUPPORT,		//0x04	申请业务网络暂不支持
	REL_CAUSE_AUTHFAIL,				//0x05	鉴权失败引起的释放
	REL_CAUSE_AIRLINKFAIL,			//0x06	空中传输链路故障
	REL_CAUSE_TIMERTIMEOUT,			//0x07	定时器超时引起的释放
	REL_CAUSE_TRANSPORTRESFAIL,		//0x08	Sabis1传输资源分配失败
	REL_CAUSE_UNKNOWN,			//0x09	未知原因
	REL_CAUSE_USR_REFUSE,			//0x0A	用户拒接
	REL_CAUSE_TALK_TOOLONG,		//0x0B	讲话总时间超时释放
	REL_CAUSE_IDLE_TIMEOUT,		//0x0C	空闲超时释放
	REL_CAUSE_PTT_INTERRUPT,		//0x0D	强制话权释放
	REL_CAUSE_INTERRUPT,			//0x0E	被强拆
	REL_CAUSE_PTT_PREEMPT,		//0x0F	被其它用户抢占话权
	REL_CAUSE_PEER_BUSY,			//0x10	对端忙
	REL_CAUSE_CALL_BOOKCED		//0x11	呼叫被预占
	
}ENUM_ReleaseCauseT;

//reset Cause
typedef enum
{
	RESETCAUSE_HARDWARE_ERROR,		//00H － 硬件故障；
	RESETCAUSE_SOFTWARE_ERROR,		//01H － 软件故障
	RESETCAUSE_LINK_ERROR,			//02H － 传输链路故障
	RESETCAUSE_POWEROFF,			//03H － 断电
	RESETCAUSE_ONREQUEST,			//04H － OMC要求复位
	RESETCAUSE_UPGRADE,				//05H - 系统升级
	RESETCAUSE_UNKONWREASON			//06H - 不知道原因
									//其他 － 目前保留。
}ENUM_ResetCauseT;

//编码方式
typedef enum 
{
	CODEC_G729A=0,	//00H － G.729a
	CODEC_G729B_UNT,//01H － G.729b UNT
	CODEC_G729D,	//02H － G.729d
	CODEC_G711A,	//03H － G.711(PCM A律)
	CODEC_G729B_SID,	//01H － G.729b SID
	CODEC_G729AB,	//5H － G.729AB；
	CODEC_ENCRYPT_G729A,//6H － 加密语音帧/加密信息帧；//6H － 个呼已处理语音帧/冗余信息帧；

	CODEC_GRPTONE=0x0A,	//集群专用编码方式，等同G.729A，静音标志等同G.729的处理//AH － PTT（组呼语音帧）。
	CODEC_ENCRYPT_GRPTONE=0x0B,	//集群专用编码方式，等同G.729A，静音标志等同G.729的处理//BH － 已处理PTT/冗余信息帧（组呼语音帧）。
	CODEC_ERR_FRAME=0x0D,	//DH － 错帧。
	CODEC_NULL_FRAME,	//EH － 空帧；
	CODEC_DTMF_FRAME	//FH － DTMF号码帧
}ENUM_VoiceCodecT;
#define isEncSrtpVData(codec)	 (CODEC_ENCRYPT_G729A==codec || CODEC_ENCRYPT_GRPTONE==codec)
#define isGrpSrtpVData(codec) (CODEC_GRPTONE==codec || CODEC_ENCRYPT_GRPTONE==codec)

//L2L3之间接口，当为集群语音包时cid字段固定填写0xFE
#define GRP_TONE_FLAG_CID		(0xFE)	

//REG_TYPE	含义
typedef enum
{
	REGTYPE_POWERON=0,			//00H	开机登记
	REGTYPE_NEWLOCATION,		//01H	位置更新
	REGTYPE_PERIOD,				//02H	周期注册
	REGTYPE_CMDREG				//03H	指令登记
								//04H - FFH	保留
}ENUM_RegTypeT;

//HO_TYPE	含义
typedef enum
{
	HO_TYPE_CONVERSATION,		//00H	通话状态下的切换
	HO_TYPE_NOTCONNECTED		//01H	非通话状态下的切换
								//02H - FFH	保留
}ENUM_HoTypeT;

typedef enum
{
	LOGINRSP_SUCCESS,				//00H	注册成功

	LOGINRSP_USRNOTEXIST,			//01H	不可使用。HLR中无此用户
	LOGINRSP_ARGUMENTERROR,			//02H	参数错误（含错误参数的标签）
	LOGINRSP_LACKOFARGUMENT,		//03H	缺少参数（含错误参数的标签）
	LOGINRSP_BUSYORFAULT,			//04H	网络忙或故障
	LOGINRSP_OLDCPENOTREGONOTHERSAG,//05H	老手机未在其它SAG上注册
	LOGINRSP_NOROAMRIGHT,			//06H	无漫游权限
	LOGINRSP_AUTHFAIL,				//07H	鉴权失败
	LOGINRSP_STEAL_USER,//08H	用户被盗窃
	LOGINRSP_USER_DUPLICATE,//09H	用户被复制
	LOGINRSP_UNKNOW_REASON,//0aH	不确定原因
	LOGINRSP_BTS_BLACKLIST,//0x0d	基站漫游禁止
	LOGINRSP_SOFTPHONE,//0x10	终端类型错误，为Softhphone终端
	LOGINRSP_CPE_NUM_SPLIT//0x11	终端类型错误，为机号分离终端
									//其它	保留

}ENUM_LoginResultT;

//SMS_Cause
typedef enum
{
	SMSCAUSE_SUCCESS=0,				//0		成功
	SMSCAUSE_SETUPLINKFAIL=21,		//21	建链不成功
	SMSCAUSE_NOANSWER,				//22	用户不应答
	SMSCAUSE_UTONLINE,				//23	用户正在线(当用户在呼叫,而又有sms_link消息来时)
	SMSCAUSE_SEQUENCEERROR,			//24	消息序号错（SAG认为是发送失败）
	SMSCAUSE_UTTIMEOUT,				//25	接收手机消息超时（建链后10秒收不全消息则超时）
	SMSCAUSE_SAGTIMEOUT,			//26	基站接收SAG消息超时（如果建链后10秒收不到SAG发来的下行数据则超时）
	SMSCAUSE_FORMATERROR,			//27	消息格式错（如消息长度不对等）
	SMSCAUSE_LINKDOWN,				//28	用户关机或已出服务区造成链路释放
	SMSCAUSE_INVALIDUSR,			//29	UID错/非法用户
	SMSCAUSE_RELLINKFAIL,			//30	拆链不成功
	SMSCAUSE_UTMEMOVERFLOW,			//31	手机缓存满
	SMSCAUSE_OTHER					//32	其它原因
}ENUM_SMSCauseT;

//BroadcastStatus
typedef enum
{
	BRDCSTSTAS_SUCCESS=0,			//0		成功
	BRDCSTSTAS_FAIL,		        //1	    失败，未给出原因；
	BRDCSTSTAS_BS_FULL,       		//0x02：BS存储空间已满；
	BRDCSTSTAS_BS_OVERLOAD, 		//0x03：BS过载（无法在指定的时间发送广播）；
	BRDCSTSTAS_OTHER 
}ENUM_BrdCstStus;

//RejectReason 拒绝原因。有效范围为0x80 ~ 0xFF。
typedef enum
{
	REJECT_RECERVED1=0x80,			//0x80	保留
	REJECT_LENTHERR,		        //0x81：消息长度错误。
	REJECT_EVENTIDERR,       		//0x82：EventID字段未定义。
	REJECT_SUB_EVENTIDERR, 		    //0x83：SubEventID字段未定义。
    REJECT_DSTADDRERR,              //0x84：目的地址错误。
    REJECT_SRCADDRERR,              //0x85：源地址错误。
	REJECT_RECERVED2                //0x86 ~ 0xFF：保留。
}ENUM_REJECTRESN;

typedef enum
{
	DISABLE_BTS_VDATA_INNER_SWITCH=0,
	ENABLE_BTS_VDATA_INNER_SWITCH=1
}ENUM_DVoice;

typedef enum
{
	FUNCLIST_0=0,
	FUNCLIST_1,
	FUNCLIST_2,
	FUNCLIST_3,
	FUNCLIST_4,
	FUNCLIST_5//脱网参数
}ENUM_FUNCLIST;

typedef enum
{
	CONFIGRESULT_0=0,
	CONFIGRESULT_1,
	CONFIGRESULT_2,
	CONFIGRESULT_3,
	CONFIGRESULT_4,
	CONFIGRESULT_5//脱网参数
}ENUM_CONFIGRESULT;

typedef enum
{
	MSG_TYPE1__NOTUSE,
	
	MSG_TYPE1__MBMS_DELAY_REQ=0x1,//MBMS_DELAY_REQ	0x01	同播管理消息	
	MSG_TYPE1__MBMS_DELAY_RSP,//MBMS_DELAY_RSP	0x02	同播管理消息	
	MSG_TYPE1__MBMS_RS_IND,//MBMS_RS_IND 0x03	同播管理消息	
	MSG_TYPE1__MBMS_RS_RSP,//MBMS_RS_RSP 0x04	同播管理消息	
	MSG_TYPE1__MBMS_RS_REQ,//MBMS_RS_REQ 0x05	同播管理消息	

	MSG_TYPE1__COUNT_PLUS_ONE
}ENUM_MSGTYPE1T;

#define M_TERMINAL_TYPE_GRPCPE	(0x30)	//30H	集群终端
#define M_SWVERSION_V52PLUS	(2)			// 2	V5.2+版本

#define M_STATUSREPORT_HEARTBEAT		(0x01)

//error notification 的error cause
#define M_ERRNOTIFY_ERR_CAUSE_AIRFAIL		(0x00)	//0x00	BS检测到空中接口链路失败
#define M_ERRNOTIFY_ERR_CAUSE_CPERELEASE	(0x01)	//0x01	终端异常释放
#define M_ERRNOTIFY_ERR_CAUSE_SAG_FOUND_ERROR	(0x80)	//0x80	SAG检测到异常

#define M_SABIS_SUCCESS	(0)
#define M_SABIS_FAIL		(1)
#define M_INVALID_GRPL3ADDR 	(0xffffffff)
#define M_INVALID_GID			(0xffff)
#define INVALID_UID (0xffffffff)
#define NO_L3ADDR	(0xffffffff)		//	默认L3地址0xffffffff
#define SMS_L3ADDR	(0xfffffffe)		//	SMS消息中L3Addr
#define M_DEFAULT_ERR_NOTI_L3ADDR		(0xfffffffe)	//error notify消息中如果没有L3地址时填写
#define NO_EID		(0xffffffff)
#define NO_CID		(0xff)
#define DEFAULT_CID	(0)
#define M_INVALID_BTSID			(0xffffffff)

#define VOICE_CCB_NUM (6000)
#define GRP_CCB_NUM	(200)					//支持200组定义
#define MAX_INSRV_GRPNUM	(100)				//支持100个组并发业务
#define  STATUSREPORT_POOLSIZE		(6000)	//状态报告项Pool

//////////////////////////////////////////////////////////////////////////

//全部呼叫信令消息类型，tVoice内部使用
typedef enum
{
	LAPaging_MSG=0,
	LAPagingRsp_MSG,
	DELAPagingReq_MSG,
	DELAPagingRsp_MSG,
	
	AssignResReq_MSG,
	AssignResRsp_MSG,
	RlsResReq_MSG,
	RlsResRsp_MSG,

	Reset_MSG,
	ResetAck_MSG,
	BeatHeart_MSG,
	BeatHeartAck_MSG,
	CongestionCtrlReq_MSG,
	CongestionCtrlRsp_MSG,
	ErrNotifyReq_MSG,
	ErrNotifyRsp_MSG,
	
	Setup_MSG,
	CallProc_MSG,
	Alerting_MSG,
	Connect_MSG,
	ConnectAck_MSG,
	Disconnect_MSG,
	Release_MSG,
	ReleaseComplete_MSG,
	ModiMediaReq_MSG,
	ModiMediaRsp_MSG,
	Information_MSG,
	AuthCmdReq_MSG,
	AuthCmdRsp_MSG,
	Login_MSG,
	LoginRsp_MSG,
	Logout_MSG,
	HandOverReq_MSG,
	HandOverRsp_MSG,
	HandOverComplete_MSG,
	StatusQry_MSG,
	Status_MSG,
	
	MOSmsDataReq_MSG,
	MOSmsDataRsp_MSG,
	MTSmsDataReq_MSG,
	MTSmsDataRsp_MSG,
	SMSMemAvailReq_MSG,
	SMSMemAvailRsp_MSG,
	
	UTSAG_L3Addr_MSG,
	UTSAG_UID_MSG,

//以下为SAbis1.5新增信令，暂时没有加入与pm任务性能统计接口，但是已经实现了计数，20060906 by fengbing

	BROADCAST_SM_MSG, 		
	BROADCAST_SM_ACK_MSG,				
	REJECT_MSG, 					
	RESET_REQ_MSG,   		    	
	RESTART_INDICATION_MSG, 		
	FAIL_INDICATION_MSG,	    	

	CALLTICKET_MSG,
	CALLTICKET_ACK_MSG,
	SRQ_MSG,
	SRP_MSG,
	SRC_MSG,

//以下为SAbis1.6新增信令，暂时没有加入与pm任务性能统计接口，但是已经实现了计数，20071103 by fengbing

	Auth_Info_Req_MSG,
	Auth_Cmd_MSG,
	Auth_Rsp_MSG,
	Auth_Result_MSG,
	BWInfo_Req_MSG,
	BWInfo_Rsp_MSG,
	BWInfo_Del_Req_MSG,
	BWInfo_Modify_Req_MSG,
	BWInfo_Modify_Rsp_MSG,
	SWICH_OFF_NOTIFY_MSG,
	AccountLoginReq_MSG,	//AccountLogin req	0x0B		0x05
	ConfigInfo_Transfer_Msg,//ConfigInfo_Transfer	0x0C		0x05

	ChangePW_Req_MSG,
	ChangePW_Rsp_MSG,
	ChangePW_Ack_MSG,

	CfgFuncInfo_Req_MSG,	//(0x53)		//ConfigFunc Info req	0x53	用户管理消息	UID寻址（OAM）
	CfgFuncInfo_Rsp_MSG,	//(0x54)		//ConfigFunc Info rsp	0x54	用户管理消息	UID寻址（OAM）
	VoiceInfo_Req_MSG,		//(0x55)		//VoiceInfo.req	0x55	用户管理消息	UID寻址（UTV）
	VoiceInfo_Rsp_MSG,		//(0x56)		//VoiceInfo.rsp	0x56	用户管理消息	UID寻址（UTV）
	Transdata_UL_MSG,	//		Transdata uplink	0x57	用户管理消息	UID寻址
	Transdata_DL_MSG,	//		Transdata downlink	0x58	用户管理消息	UID寻址
	OAMTransfer_Info_Req_MSG,	//		OAMTransfer_Info req	0x59	用户管理消息	UID寻址（OAMmessage）
	OAMTransfer_Info_Rsp_MSG,	//		OAMTransfer_Info rsp	0x5A	用户管理消息	UID寻址(OAMmessage)
	SecuriryCardCallPara_Req_MSG,	//		SecuriryCardCallPara req	0x5B	用户管理消息	L3地址寻址
	SecuriryCardCallPara_Rsp_MSG,	//		SecuriryCardCallPara rsp	0x5C	用户管理消息	L3地址寻址
	
//集群相关begin
	LAGrpPagingReq_MSG,	//LA Group paging	0x05	呼叫控制消息	0x00
	LAGrpPagingRsp_MSG,	//LA Group paging rsp	0x06	呼叫控制消息	0x00
	LEGrpPaging_MSG,		//LE Group paging	0x07	呼叫控制消息	0x00
	DeLEGrpPaging_MSG,		//De-LE Group paging	0x08	呼叫控制消息	0x00
	StatusReport_MSG,		//USER_STATUS_REPORT	0x09	呼叫控制消息	0x00
	GrpHandoverReq_MSG,	//Group Handover req	0x0A	呼叫控制消息	0x00
	GrpHandoverRsp_MSG,	//Group Handover rsp	0x0B	呼叫控制消息	0x00	

	RlsSmsLinkReq_MSG,		//Release Sms link req	0x05		0x01
	RlsSmsLinkRsp_MSG,		//Release Sms link rsp	0x06		0x01
	GrpResReq_MSG,			//Group Resouse req	0x07		0x01
	GrpResRsp_MSG,			//Group Resouse rsp	0x08		0x01

	UTSXC_GRPL3Addr_MSG,	//组L3寻址的UE-SAG message	0x03		0x03
	UTSXC_GRPUID_MSG,		//组呼的UID寻址的UE-SAG message	0x04		0x03

	//组呼呼叫控制消息
	PTT_SetupReq_MSG,		//PTT Setup Req	0x60	呼叫控制消息	L3地址寻址
	PTT_SetupAck_MSG,		//PTT Setup ack	0x61	呼叫控制消息	L3地址寻址
	PTT_Connect_MSG,		//PTT Connect	0x62	呼叫控制消息	L3地址寻址
	PTT_ConnectAck_MSG,	//PTT Connect Ack	0x63	呼叫控制消息	L3地址寻址
	Grp_Disconnect_MSG,	//Group Disconnect	0x64	呼叫控制消息	组L3地址寻址
	Grp_Release_MSG,		//Group Release	0x65	呼叫控制消息	组L3地址寻址
	Group_CallingRls_MSG,	//Group Calling Release	0x66	呼叫控制消息	L3地址寻址
	Group_CallingRlsComplete_MSG,	//Group Calling Release Complete	0x67	呼叫控制消息	L3地址寻址
	//移动性管理消息
	Modify_GIDs_Req_MSG,	//ATTACH/DETACHGIDs Req	0x25	移动性管理消息	UID寻址
	Modify_GIDs_Rsp_MSG,	//ATTACH/DETACHGIDs Rsp	0x26	移动性管理消息	UID寻址
	//话权控制消息
	PTT_PressReq_MSG,		//PTT PRESS REQ	0x70	话权控制消息	组呼的UID寻址
	PTT_PressRsp_MSG,		//PTT PRESS RSP	0x71	话权控制消息	组呼的UID寻址
	PTT_Granted_MSG,		//PTT GRANTED	0x72	话权控制消息	组L3地址寻址
	PTT_Interrupt_MSG,		//PTT INTERRUPT	0x73	话权控制消息	L3寻址
	PTT_InterruptAck_MSG,	//PTT INTERRUPT ACK	0x74	话权控制消息	L3寻址
	PTT_Rls_MSG,			//PTT RELEASE	0x75	话权控制消息	L3地址寻址
	PTT_RlsAck_MSG,		//PTT RELEASE ACK	0x76	话权控制消息	L3地址寻址
	PTT_PressInfo_MSG,		//PTT PRESS INFO	0x77	话权控制消息	L3地址寻址
	PTT_PressCancel_MSG,	//PTT PRESS CANCLE 	0x78	话权控制消息	组呼的UID寻址
	//话权申请省资源优化
	PTT_PressApplyReq_MSG,
	PTT_PressApplyRsp_MSG,
	//增值业务消息
	User_StatusQuery_MSG,	//USER STATUS QUERY	0x90	状态报告消息	L3寻址
	User_StatusRsp_MSG,	//USER_STATUS_RESPONSE	0x91	状态报告消息	L3寻址
	User_StatusNotify_MSG,	//USER_STATUS_NOTIFY	0x92	状态报告消息	L3寻址
//集群相关end
//数据组播、单播begin
	DB_GrpPaging_MSG,
	DB_GrpPagingRsp_MSG,
	DB_DataComplete_MSG,
	DB_MODataReq_MSG,
	DB_MODataRsp_MSG,
	DB_RetransDataReq_MSG,
	DB_RetransDataRsp_MSG,
//数据组播、单播end
	UTSAG_UID_OAM_MSG,
	DVoiceConfigReg_MSG,	//DVoice Config
	UTSAG_RELAY_MSG,	//无需终端注册透传的消息
//同播begin
	BTSL2_SAG_MSG,
//同播end
	InvalidSignal_MSG

	//NotParsed_MSG=0xffff
}SignalType;

//Error Codes for L3Voice Tasks
#include "logarea.h"
const UINT16 EC_L3VOICE_NORMAL               = 0x0000;  //正常流程
const UINT16 EC_L3VOICE_UNEXPECTED_MSGID     = 0x0001;  //非法Message ID
const UINT16 EC_L3VOICE_SYS_FAIL             = 0x0002;  //系统错误
const UINT16 EC_L3VOICE_SOCKET_ERR           = 0x0003;  //Socket错误
const UINT16 EC_L3VOICE_INVALID_SIGNAL       = 0x0004;  //参数错误
const UINT16 EC_L3VOICE_MSG_EXCEPTION        = 0x0005;  //异常消息
const UINT16 EC_L3VOICE_MSG_SND_FAIL         = 0x0006;	//任务间发送消息失败
const UINT16 EC_L3VOICE_INVALID_VOICEDATA    = 0x0007;	//非法语音包


//Counters for BTS L3Voice tasks
typedef struct _VoicePerfCounter
{
//tVoice-voiceData
	UINT32 nVoiDataToVAC;
	UINT32 nVoiDataFromVAC;
	UINT32 nVoiDataToVDR;
	UINT32 nVoiDataFromVDR;

	UINT32 n10msPktToVAC;
	UINT32 n10msPktFromVAC;
	UINT32 n10msPktToVDR;
	UINT32 n10msPktFromVDR;

	UINT32 nNullPkt;			//uplink
	UINT32 nG729BPkt;		//uplink
	UINT32 nUpLinkVdataLost;
	UINT32 nDownLinkVdataBadSent;

	UINT32 nG729BPktFromVDR;	//donwlink 
	UINT32 nG729BPktToVAC;		//downlink counter when not using JitterBuffer
//tVoice-Signals
	UINT32 nSigFromVAC;
	UINT32 nSigFromDAC;
	UINT32 nSigToVAC;
	UINT32 nSigToDAC;

	UINT32 nSigToVCR;
	UINT32 nSigFromVCR;

	UINT32 nSigToOtherBTS;
	UINT32 nSigFromOtherBTS;
//tVDR
	UINT32 nVoiDataToMG;
	UINT32 nVoiDataFromMG;
	UINT32 nVoiDataToTvoice;
	UINT32 nVoiDataFromTvoice;
//tVCR
	UINT32 nSigToSAG;
	UINT32 nSigFromSAG;
	UINT32 nSigToTvoice;
	UINT32 nSigFromTvoice;
	UINT32 nSigToUm;
	UINT32 nSigFromUm;
	UINT32  nVDRCCBNULL;
}VoicePerfCounterT;
extern VoicePerfCounterT Counters;
#endif /* __VOICECOMMON_H */   


