/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    SAGCommon.h
*
* DESCRIPTION: 
*		模拟SAG的常量定义
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-12-06   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__SAGCOMMON_H
#define	__SAGCOMMON_H

//VCR与SAG的TCP接口定义
#define M_TCP_PKT_BEGIN_FLAG		(0x7ea5)		//HEAD FALG取值为0x7ea5
#define M_TCP_PKT_END_FLAG			(0x7e0d)		//END FLAG取值为0x7e0d
#define M_TCP_PKT_SABIS1_USERTYPE	(0x1106)		//用户类型，SAbis1接口固定为0x1106

#define  M_G729_DATALEN	10			//G729 10ms DATA length
#define  M_G711_DATALEN	80			//G711 10ms DATA length

//////////////////////////////////////////////////////////////////////////

/*
 *	BTS-SAG message Event Group ID
 */
#define M_MSG_EVENT_GROUP_ID_CALLCTRL	(0x00)	//呼叫控制消息
#define M_MSG_EVENT_GROUP_ID_RESMANAGE	(0x01)	//资源管理消息
#define M_MSG_EVENT_GROUP_ID_MANAGE		(0x02)	//管理类消息
#define M_MSG_EVENT_GROUP_ID_UTSAG		(0x03)	//UE-SAG直接传输消息

#define M_MSG_EVENT_GROUP_NUM			M_MSG_EVENT_GROUP_ID_UTSAG	

/*
 *	BTS-SAG message Event ID
 */
//呼叫控制消息
#define M_MSG_EVENT_ID_LAPAGING				(0x01)		//LA Paging	0x01
#define M_MSG_EVENT_ID_LAPAGINGRSP			(0x02)		//LA Paging Response	0x01
#define M_MSG_EVENT_ID_DELAPAGING			(0x03)		//De-LA Paging	0x02
#define M_MSG_EVENT_ID_DELAPAGINGRSP		(0x04)		//De-LA Paging Response	0x02
//资源管理消息
#define M_MSG_EVENT_ID_ASSGNRESREQ			(0x01)		//Assignmengt transport resource req	0x03
#define M_MSG_EVENT_ID_ASSGNRESRSP			(0x02)		//Assignmengt transport resource rsp	0x04
#define M_MSG_EVENT_ID_RLSRESREQ			(0x03)		//Release transport resource req	0x05
#define M_MSG_EVENT_ID_RLSRESRSP			(0x04)		//Release transport resource rsp	0x05
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
#define M_MSG_EVENT_ID_UTSAG_UID			(0x02)		//UID寻址的UE-SAG message	0x21

#define M_VOICE_SIGNAL_MAX_EVENT_ID			(0x20)

/*
 *	UT－SAG透传消息的message type字段类型
 */
//呼叫控制消息
#define M_MSGTYPE_SETUP					(0x01)		//Setup	0x01
#define M_MSGTYPE_CALLPROC				(0x02)		//Call Proceeding	0x02
#define M_MSGTYPE_ALERTING				(0x03)		//Alerting	0x03
#define M_MSGTYPE_CONNECT				(0x04)		//Connect	0x04
#define M_MSGTYPE_CONNECTACK			(0x05)		//Connect Ack	0x05
#define M_MSGTYPE_DISCONNECT			(0x06)		//Disconnect	0x06
#define M_MSGTYPE_INFORMATION			(0x07)		//Information	0x07
#define M_MSGTYPE_RELEASE				(0x08)		//Release	0x08
#define M_MSGTYPE_RELEASECOMPLETE		(0x09)		//Release complete	0x09
#define M_MSGTYPE_MODIMEDIATYPEREQ		(0x0a)		//Modify media type req	0x0a
#define M_MSGTYPE_MODIMEDIATYPERSP		(0x0b)		//Modify media type rsp	0x0b
//切换消息
#define M_MSGTYPE_HANDOVERSTART			(0x10)		//Handover Start	0x10
#define M_MSGTYPE_HANDOVERSTARTACK		(0x11)		//Handover Start Ack	0x10
#define M_MSGTYPE_HANDOVERCOMPLETE		(0x12)		//Handover Complete
//移动性管理消息
#define M_MSGTYPE_LOGINREQ				(0x20)		//Login req	0x12
#define M_MSGTYPE_LOGINRSP				(0x21)		//Login rsp	0x12
#define M_MSGTYPE_LOGOUT				(0x22)		//Logout	
#define M_MSGTYPE_AUTHCMD				(0x23)		//Authentication command	0x13
#define M_MSGTYPE_AUTHRSP				(0x24)		//Authentication rsp	0x13
//短消息
#define M_MSGTYPE_MOSMSREQ				(0x30)		//MO Sms data req	0x14
#define M_MSGTYPE_MOSMSRSP				(0x31)		//MO Sms data rsp	0x14
#define M_MSGTYPE_MTSMSREQ				(0x32)		//MT Sms data req	0x15
#define M_MSGTYPE_MTSMSRSP				(0x33)		//MT Sms data rsp	0x15
#define M_MSGTYPE_SMSMEMAVAILREQ		(0x34)		//SMS memory available req	0x16
#define M_MSGTYPE_SMSMEMAVAILRSP		(0x35)		//SMS memory available rsp	0x16

#define M_MSGTYPE_MAX_VALUE				(0x35)
//////////////////////////////////////////////////////////////////////////

#define M_DEFAULT_VOICE_PRIORITY		(0x03)		//目前版本固定语音服务优先级 3
#define M_DMUX_VOICEDATA_PRIORITY		(0x04)		//DMUX接口中规定 4

#define M_REQ_TRANS_RATE_DEFAULT		(0x00)		//默认的Request transport resource rate,0x00表示不限制

//业务类型AppType
typedef enum
{
	APPTYPE_VOICE_QCELP=0,		//	00	语音应用/ QCELP语音编码的语音应用
	APPTYPE_VOICE_G729=1,		//	01	保留/ G729语音编码的语音应用
	APPTYPE_VOICE_G729B=1,		//	01	保留/ G729B语音编码
	APPTYPE_RESERVED1=3,		//	03	保留，待定义
	APPTYPE_SMS,				//	04	短消息应用
	APPTYPE_LOWSPEED_DATA,		//	05	低速数据
	APPTYPE_RESERVED2,			//	06	保留，待定义
	APPTYPE_BER_TEST,			//	07	表示为误码测试建链请求
	APPTYPE_CMD_REG=0x10,		//	0x10	指令登记
	APPTYPE_DUPCPE_PAGING,		//	0x11 孖机寻呼
	APPTYPE_BEARER_SERVICE		//	0x12 承载业务
}ENUM_AppTypeT;

//Setup Cause
typedef enum
{
	SETUPCAUSE_VOICE=0,			//	00H － 语音；
	SETUPCAUSE_DATA,			//	01H － 低速数据
	SETUPCAUSE_BROADCAST,		//	02H － 广播
	SETUPCAUSE_CARRYSRV			//	03H － 承载业务
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
	REL_CAUSE_UNKNOWN				//0x09	未知原因
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
	CODEC_G729B,	//01H － G.729b
	CODEC_G729D,	//02H － G.729d
	CODEC_G711A		//03H － G.711(PCM A律)

}ENUM_VoiceCodecT;

//REG_TYPE	含义
typedef enum
{
	REGTYPE_POWERON=0,			//00H	开机登记
	REGTYPE_NEWLOCATION,		//01H	位置更新
	REGTYPE_PERIOD,				//02H	周期注册
	REGTYPE_CMDREG,				//03H	指令登记
								//04H - FFH	保留
}ENUM_RegTypeT;

//HO_TYPE	含义
typedef enum
{
	HO_TYPE_CONVERSATION,		//00H	通话状态下的切换
	HO_TYPE_NOTCONNECTED,		//01H	非通话状态下的切换
								//02H - FFH	保留
}ENUM_HoTypeT;

typedef enum
{
	LOGINRSP_SUCCESS,				//success
	
	LOGINRSP_USRNOTEXIST,			//01H	不可使用。HLR中无此用户
	LOGINRSP_ARGUMENTERROR,			//02H	参数错误（含错误参数的标签）
	LOGINRSP_LACKOFARGUMENT,		//03H	缺少参数（含错误参数的标签）
	LOGINRSP_BUSYORFAULT,			//04H	网络忙或故障
	LOGINRSP_OLDCPENOTREGONOTHERSAG,//05H	老手机未在其它SAG上注册
	LOGINRSP_NOROAMRIGHT,			//06H	无漫游权限
	LOGINRSP_NONUMBER,				//0x0007   未指配电话号码。
	LOGINRSP_STEALNUMBER,			//0x0008   用户被盗窃。
	LOGINRSP_DUPLICATEUSER,			//0x0009   用户被复制。
	LOGINRSP_UNKNOWREASON			//0x000a   不确定的原因
}ENUM_LoginResultT;

//SMS_Cause
typedef enum
{
	SMSCAUSE_SUCCESS,				//0		成功
	SMSCAUSE_SETUPLINKFAIL,			//21	建链不成功
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

//error notification 的error cause
#define M_ERRNOTIFY_ERR_CAUSE_AIRFAIL		(1)	//1	BS检测到空中接口链路失败
#define M_ERRNOTIFY_ERR_CAUSE_CPERELEASE	(2)	//2	终端异常释放

#define INVALID_UID	(0xffffffff)		//  无效的UID
#define NO_L3ADDR	(0xffffffff)		//	默认L3地址0xffffffff
#define SMS_L3ADDR	(0xfffffffe)		//	SMS消息中L3Addr
#define M_DEFAULT_ERR_NOTI_L3ADDR		(0xfffffffe)	//error notify消息中如果没有L3地址时填写
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
	
//	SetupUT_MSG,
//	SetupSAG_MSG,
	Setup_MSG,
	CallProc_MSG,
	Alerting_MSG,
//	ConnectUT_MSG,
//	ConnectSAG_MSG,
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
	InvalidSignal_MSG,

	//NotParsed_MSG=0xffff
}SignalType;


#endif /* __SAGCOMMON_H */