/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    SAGCommon.h
*
* DESCRIPTION: 
*		ģ��SAG�ĳ�������
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-12-06   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__SAGCOMMON_H
#define	__SAGCOMMON_H

//VCR��SAG��TCP�ӿڶ���
#define M_TCP_PKT_BEGIN_FLAG		(0x7ea5)		//HEAD FALGȡֵΪ0x7ea5
#define M_TCP_PKT_END_FLAG			(0x7e0d)		//END FLAGȡֵΪ0x7e0d
#define M_TCP_PKT_SABIS1_USERTYPE	(0x1106)		//�û����ͣ�SAbis1�ӿڹ̶�Ϊ0x1106

#define  M_G729_DATALEN	10			//G729 10ms DATA length
#define  M_G711_DATALEN	80			//G711 10ms DATA length

//////////////////////////////////////////////////////////////////////////

/*
 *	BTS-SAG message Event Group ID
 */
#define M_MSG_EVENT_GROUP_ID_CALLCTRL	(0x00)	//���п�����Ϣ
#define M_MSG_EVENT_GROUP_ID_RESMANAGE	(0x01)	//��Դ������Ϣ
#define M_MSG_EVENT_GROUP_ID_MANAGE		(0x02)	//��������Ϣ
#define M_MSG_EVENT_GROUP_ID_UTSAG		(0x03)	//UE-SAGֱ�Ӵ�����Ϣ

#define M_MSG_EVENT_GROUP_NUM			M_MSG_EVENT_GROUP_ID_UTSAG	

/*
 *	BTS-SAG message Event ID
 */
//���п�����Ϣ
#define M_MSG_EVENT_ID_LAPAGING				(0x01)		//LA Paging	0x01
#define M_MSG_EVENT_ID_LAPAGINGRSP			(0x02)		//LA Paging Response	0x01
#define M_MSG_EVENT_ID_DELAPAGING			(0x03)		//De-LA Paging	0x02
#define M_MSG_EVENT_ID_DELAPAGINGRSP		(0x04)		//De-LA Paging Response	0x02
//��Դ������Ϣ
#define M_MSG_EVENT_ID_ASSGNRESREQ			(0x01)		//Assignmengt transport resource req	0x03
#define M_MSG_EVENT_ID_ASSGNRESRSP			(0x02)		//Assignmengt transport resource rsp	0x04
#define M_MSG_EVENT_ID_RLSRESREQ			(0x03)		//Release transport resource req	0x05
#define M_MSG_EVENT_ID_RLSRESRSP			(0x04)		//Release transport resource rsp	0x05
//��������Ϣ
#define M_MSG_EVENT_ID_RESET				(0x01)		//Reset	0x07
#define M_MSG_EVENT_ID_RESETACK				(0x02)		//Reset ack	0x08
#define M_MSG_EVENT_ID_ERRNOTIFYREQ			(0x03)		//Error notification req	0x09
#define M_MSG_EVENT_ID_ERRNOTIFYRSP			(0x04)		//Error notification rsp	0x0a
#define M_MSG_EVENT_ID_BEATHEART			(0x05)		//beatheart	0x0b
#define M_MSG_EVENT_ID_BEATHEARTACK			(0x06)		//Beatheart ack	0x0c
#define M_MSG_EVENT_ID_CONGESTREQ			(0x07)		//Congestion control req	0x0d
#define M_MSG_EVENT_ID_CONGESTRSP			(0x08)		//Congestion control req	0x0e
//UE-SAGֱ�Ӵ�����Ϣ
#define M_MSG_EVENT_ID_UTSAG_L3ADDR			(0x01)		//L3��ַѰַ��UE-SAG message	0x20
#define M_MSG_EVENT_ID_UTSAG_UID			(0x02)		//UIDѰַ��UE-SAG message	0x21

#define M_VOICE_SIGNAL_MAX_EVENT_ID			(0x20)

/*
 *	UT��SAG͸����Ϣ��message type�ֶ�����
 */
//���п�����Ϣ
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
//�л���Ϣ
#define M_MSGTYPE_HANDOVERSTART			(0x10)		//Handover Start	0x10
#define M_MSGTYPE_HANDOVERSTARTACK		(0x11)		//Handover Start Ack	0x10
#define M_MSGTYPE_HANDOVERCOMPLETE		(0x12)		//Handover Complete
//�ƶ��Թ�����Ϣ
#define M_MSGTYPE_LOGINREQ				(0x20)		//Login req	0x12
#define M_MSGTYPE_LOGINRSP				(0x21)		//Login rsp	0x12
#define M_MSGTYPE_LOGOUT				(0x22)		//Logout	
#define M_MSGTYPE_AUTHCMD				(0x23)		//Authentication command	0x13
#define M_MSGTYPE_AUTHRSP				(0x24)		//Authentication rsp	0x13
//����Ϣ
#define M_MSGTYPE_MOSMSREQ				(0x30)		//MO Sms data req	0x14
#define M_MSGTYPE_MOSMSRSP				(0x31)		//MO Sms data rsp	0x14
#define M_MSGTYPE_MTSMSREQ				(0x32)		//MT Sms data req	0x15
#define M_MSGTYPE_MTSMSRSP				(0x33)		//MT Sms data rsp	0x15
#define M_MSGTYPE_SMSMEMAVAILREQ		(0x34)		//SMS memory available req	0x16
#define M_MSGTYPE_SMSMEMAVAILRSP		(0x35)		//SMS memory available rsp	0x16

#define M_MSGTYPE_MAX_VALUE				(0x35)
//////////////////////////////////////////////////////////////////////////

#define M_DEFAULT_VOICE_PRIORITY		(0x03)		//Ŀǰ�汾�̶������������ȼ� 3
#define M_DMUX_VOICEDATA_PRIORITY		(0x04)		//DMUX�ӿ��й涨 4

#define M_REQ_TRANS_RATE_DEFAULT		(0x00)		//Ĭ�ϵ�Request transport resource rate,0x00��ʾ������

//ҵ������AppType
typedef enum
{
	APPTYPE_VOICE_QCELP=0,		//	00	����Ӧ��/ QCELP�������������Ӧ��
	APPTYPE_VOICE_G729=1,		//	01	����/ G729�������������Ӧ��
	APPTYPE_VOICE_G729B=1,		//	01	����/ G729B��������
	APPTYPE_RESERVED1=3,		//	03	������������
	APPTYPE_SMS,				//	04	����ϢӦ��
	APPTYPE_LOWSPEED_DATA,		//	05	��������
	APPTYPE_RESERVED2,			//	06	������������
	APPTYPE_BER_TEST,			//	07	��ʾΪ������Խ�������
	APPTYPE_CMD_REG=0x10,		//	0x10	ָ��Ǽ�
	APPTYPE_DUPCPE_PAGING,		//	0x11 �I��Ѱ��
	APPTYPE_BEARER_SERVICE		//	0x12 ����ҵ��
}ENUM_AppTypeT;

//Setup Cause
typedef enum
{
	SETUPCAUSE_VOICE=0,			//	00H �� ������
	SETUPCAUSE_DATA,			//	01H �� ��������
	SETUPCAUSE_BROADCAST,		//	02H �� �㲥
	SETUPCAUSE_CARRYSRV			//	03H �� ����ҵ��
}ENUM_SetupCauseT;

//release cause
typedef enum
{
	REL_CAUSE_NORMAL=0,				//0x00	�����ͷ�����
	REL_CAUSE_UT,					//0x01	BS�����ͷ�����
	REL_CAUSE_SAG,					//0x02	�����쳣������ͷ�
	REL_CAUSE_USRSRVNOTALLOW,		//0x03	�û�ҵ������
	REL_CAUSE_SRVNOTSUPPORT,		//0x04	����ҵ�������ݲ�֧��
	REL_CAUSE_AUTHFAIL,				//0x05	��Ȩʧ��������ͷ�
	REL_CAUSE_AIRLINKFAIL,			//0x06	���д�����·����
	REL_CAUSE_TIMERTIMEOUT,			//0x07	��ʱ����ʱ������ͷ�
	REL_CAUSE_TRANSPORTRESFAIL,		//0x08	Sabis1������Դ����ʧ��
	REL_CAUSE_UNKNOWN				//0x09	δ֪ԭ��
}ENUM_ReleaseCauseT;

//reset Cause
typedef enum
{
	RESETCAUSE_HARDWARE_ERROR,		//00H �� Ӳ�����ϣ�
	RESETCAUSE_SOFTWARE_ERROR,		//01H �� �������
	RESETCAUSE_LINK_ERROR,			//02H �� ������·����
	RESETCAUSE_POWEROFF,			//03H �� �ϵ�
	RESETCAUSE_ONREQUEST,			//04H �� OMCҪ��λ
	RESETCAUSE_UPGRADE,				//05H - ϵͳ����
	RESETCAUSE_UNKONWREASON			//06H - ��֪��ԭ��
									//���� �� Ŀǰ������
}ENUM_ResetCauseT;

//���뷽ʽ
typedef enum 
{
	CODEC_G729A=0,	//00H �� G.729a
	CODEC_G729B,	//01H �� G.729b
	CODEC_G729D,	//02H �� G.729d
	CODEC_G711A		//03H �� G.711(PCM A��)

}ENUM_VoiceCodecT;

//REG_TYPE	����
typedef enum
{
	REGTYPE_POWERON=0,			//00H	�����Ǽ�
	REGTYPE_NEWLOCATION,		//01H	λ�ø���
	REGTYPE_PERIOD,				//02H	����ע��
	REGTYPE_CMDREG,				//03H	ָ��Ǽ�
								//04H - FFH	����
}ENUM_RegTypeT;

//HO_TYPE	����
typedef enum
{
	HO_TYPE_CONVERSATION,		//00H	ͨ��״̬�µ��л�
	HO_TYPE_NOTCONNECTED,		//01H	��ͨ��״̬�µ��л�
								//02H - FFH	����
}ENUM_HoTypeT;

typedef enum
{
	LOGINRSP_SUCCESS,				//success
	
	LOGINRSP_USRNOTEXIST,			//01H	����ʹ�á�HLR���޴��û�
	LOGINRSP_ARGUMENTERROR,			//02H	�������󣨺���������ı�ǩ��
	LOGINRSP_LACKOFARGUMENT,		//03H	ȱ�ٲ���������������ı�ǩ��
	LOGINRSP_BUSYORFAULT,			//04H	����æ�����
	LOGINRSP_OLDCPENOTREGONOTHERSAG,//05H	���ֻ�δ������SAG��ע��
	LOGINRSP_NOROAMRIGHT,			//06H	������Ȩ��
	LOGINRSP_NONUMBER,				//0x0007   δָ��绰���롣
	LOGINRSP_STEALNUMBER,			//0x0008   �û������ԡ�
	LOGINRSP_DUPLICATEUSER,			//0x0009   �û������ơ�
	LOGINRSP_UNKNOWREASON			//0x000a   ��ȷ����ԭ��
}ENUM_LoginResultT;

//SMS_Cause
typedef enum
{
	SMSCAUSE_SUCCESS,				//0		�ɹ�
	SMSCAUSE_SETUPLINKFAIL,			//21	�������ɹ�
	SMSCAUSE_NOANSWER,				//22	�û���Ӧ��
	SMSCAUSE_UTONLINE,				//23	�û�������(���û��ں���,������sms_link��Ϣ��ʱ)
	SMSCAUSE_SEQUENCEERROR,			//24	��Ϣ��Ŵ�SAG��Ϊ�Ƿ���ʧ�ܣ�
	SMSCAUSE_UTTIMEOUT,				//25	�����ֻ���Ϣ��ʱ��������10���ղ�ȫ��Ϣ��ʱ��
	SMSCAUSE_SAGTIMEOUT,			//26	��վ����SAG��Ϣ��ʱ�����������10���ղ���SAG����������������ʱ��
	SMSCAUSE_FORMATERROR,			//27	��Ϣ��ʽ������Ϣ���Ȳ��Եȣ�
	SMSCAUSE_LINKDOWN,				//28	�û��ػ����ѳ������������·�ͷ�
	SMSCAUSE_INVALIDUSR,			//29	UID��/�Ƿ��û�
	SMSCAUSE_RELLINKFAIL,			//30	�������ɹ�
	SMSCAUSE_UTMEMOVERFLOW,			//31	�ֻ�������
	SMSCAUSE_OTHER					//32	����ԭ��
}ENUM_SMSCauseT;

//error notification ��error cause
#define M_ERRNOTIFY_ERR_CAUSE_AIRFAIL		(1)	//1	BS��⵽���нӿ���·ʧ��
#define M_ERRNOTIFY_ERR_CAUSE_CPERELEASE	(2)	//2	�ն��쳣�ͷ�

#define INVALID_UID	(0xffffffff)		//  ��Ч��UID
#define NO_L3ADDR	(0xffffffff)		//	Ĭ��L3��ַ0xffffffff
#define SMS_L3ADDR	(0xfffffffe)		//	SMS��Ϣ��L3Addr
#define M_DEFAULT_ERR_NOTI_L3ADDR		(0xfffffffe)	//error notify��Ϣ�����û��L3��ַʱ��д
//////////////////////////////////////////////////////////////////////////

//ȫ������������Ϣ���ͣ�tVoice�ڲ�ʹ��
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