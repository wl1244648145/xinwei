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
 *   2006-09-06 fengbing  ������������͸��������callticket,callticketAck,SRQ,SRP,SRC
 *   2006-08-21 fengbing  AppType�������
 *   2006-04-19 fengbing  ��������ӿ�������������ݵļ���ͳ��
 *   2006-04-09 fengbing  �޸���VAC��ض�ʱ��ʱ��(TIMER_VAC:5sec-->2sec)
 *   2006-04-09 fengbing  ����SAbis1�ӿ��ĵ��޸ı��뷽ʽ�Ķ��壬�Ự���ȼ��Ķ��壬
 *                        DMUX�����������ȼ��Ķ���,APP_TYPE�ֶεĶ��壬Request transport
 *                        data rate�ı仯
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

//��ʱ��ID
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

//��ʱ��ʱ��,��λ��
#define VOICE_ONE_MS			(1)
#define VOICE_ONE_SECOND		(VOICE_ONE_MS*1000)
#define VOICE_ONE_MINUTE		(VOICE_ONE_SECOND*60)

#define M_TIMERLEN_ASSIGN		(7*VOICE_ONE_SECOND)
#define M_TIMERLEN_VAC			(5*VOICE_ONE_SECOND)	//���˯���޸��������пտڽ�����ʱʱ������߱��н�ͨ��
#define M_TIMERLEN_ERRRSP		(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_RELRES		(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_PROBERSP		(16*VOICE_ONE_SECOND)

#define M_TIMERLEN_BEATHEART	(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_CONGEST		(60*VOICE_ONE_SECOND)

#define M_TIMERLEN_MOVEAWAY_DISABLE	(10*VOICE_ONE_SECOND)
#define M_TIMERLEN_WAITSYNC		(8*VOICE_ONE_SECOND)//(5*VOICE_ONE_SECOND)
#define M_TIMERLEN_DELAY_RELVAC (400*VOICE_ONE_MS)		//�ӳ��ͷ�VAC

#define M_TIMERLEN_GRPPAGINGRSP		(5*VOICE_ONE_SECOND)//(15*VOICE_ONE_SECOND)	//fengbing 20090515 ����״�Ѱ����ʱ��Ϊ5��
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
#define M_MSG_EVENT_GROUP_ID_CALLCTRL	(0x00)	//���п�����Ϣ
#define M_MSG_EVENT_GROUP_ID_RESMANAGE	(0x01)	//��Դ������Ϣ
#define M_MSG_EVENT_GROUP_ID_MANAGE		(0x02)	//��������Ϣ
#define M_MSG_EVENT_GROUP_ID_UTSAG		(0x03)	//UE-SAGֱ�Ӵ�����Ϣ
#define M_MSG_EVENT_GROUP_ID_BROADCAST	(0x04)	//�㲥ҵ����Ϣ
#define M_MSG_EVENT_GROUP_ID_UTSAGOAM	(0x05)	//McBTSת����UE-SAG����Ϣ
#define M_MSG_EVENT_GROUP_ID_CPEOAM		(0x06)	//McBTS͸����OAM��Ϣ
#define M_MSG_EVENT_GROUP_ID_RELAYMSG	(0x07)	//UIDѰַ��UE-SAG OAMmessage	0x01		0x07
#define M_MSG_ENENT_GROUP_ID_BTSL2SAG	(0x08)	//��վ�����SAG����Ϣ����վ����͸��

#define M_MSG_EVENT_GROUP_NUM			M_MSG_ENENT_GROUP_ID_BTSL2SAG	

/*
 *	BTS-SAG message Event ID
 */
//���п�����Ϣ
#define M_MSG_EVENT_ID_LAPAGING				(0x01)		//LA Paging	0x01
#define M_MSG_EVENT_ID_LAPAGINGRSP			(0x02)		//LA Paging Response	0x01
#define M_MSG_EVENT_ID_DELAPAGING			(0x03)		//De-LA Paging	0x02
#define M_MSG_EVENT_ID_DELAPAGINGRSP		(0x04)		//De-LA Paging Response	0x02
#define M_MSG_EVENT_ID_LAGrpPaging			(0x05)		//LA Group paging	0x05	���п�����Ϣ	0x00
#define M_MSG_EVENT_ID_LAGrpPagingRsp			(0x06)		//LA Group paging rsp	0x06	���п�����Ϣ	0x00
#define M_MSG_EVENT_ID_LEGrpPaging			(0x07)		//LE Group paging	0x07	���п�����Ϣ	0x00
#define M_MSG_EVENT_ID_DELEPaging				(0x08)		//De-LE Group paging	0x08	���п�����Ϣ	0x00
#define M_MSG_EVENT_ID_UserStatusReport		(0x09)		//USER_STATUS_REPORT	0x09	���п�����Ϣ	0x00
#define M_MSG_EVENT_ID_GrpHandoverReq		(0x0A)		//Group Handover req	0x0A	���п�����Ϣ	0x00
#define M_MSG_EVENT_ID_GrpHandoverRsp		(0x0B)		//Group Handover rsp	0x0B	���п�����Ϣ	0x00
#define M_MSG_EVENT_ID_DVoiceConfigReq		(0x10)		//DVoice Config
#define M_MSG_EVENT_ID_DBGrpPagingReq		(0x11)		//DB Group paging	0x11		
#define M_MSG_EVENT_ID_DBGrpPagingRsp		(0x12)		//DB Group paging response	0x12	���п�����Ϣ	0x00
#define M_MSG_EVENT_ID_DBDataComplete		(0x13)		//DB data complete	0x13	���п�����Ϣ	0x00

//��Դ������Ϣ
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
#define M_MSG_EVENT_ID_UTSAG_UID				(0x02)		//UIDѰַ��UE-SAG message	0x21
#define M_MSG_EVENT_ID_GRP_UTSAG_L3ADDR		(0x03)		//��L3Ѱַ��UE-SAG message	0x03		0x03
#define M_MSG_EVENT_ID_GRP_UTSAG_UID		(0x04)		//�����UIDѰַ��UE-SAG message	0x04		0x03

//�㲥ҵ����Ϣ
#define M_MSG_EVENT_ID_BROADCAST_SM 			(0x01)		
#define M_MSG_EVENT_ID_BROADCAST_SM_ACK	(0x02)		
#define M_MSG_EVENT_ID_REJECT 					(0x03)		
#define M_MSG_EVENT_ID_RESET_REQ				(0x04)		
#define M_MSG_EVENT_ID_RESTART_INDICATION 	(0x05)		
#define M_MSG_EVENT_ID_FAIL_INDICATION		(0x06)		

//UT-SAG��OAM��Ϣ
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


//bts͸����OAM��Ϣ
#define M_MSG_EVENT_ID_CPEOAM				(0x01)

//UIDѰַ��UE-SAG OAMmessage	0x01		0x07
#define M_MSG_EVENT_ID_RELAYMSG	(0x01)

//��վ�����SAG����Ϣ	0x01	ͬ��������Ϣ	0x08
#define M_MSG_ENENT_ID_BTSL2SXC_SYNC_BROADCAST (0x01)

/*
 *	UT��SAG͸����Ϣ��message type�ֶ�����
 */
//���п�����Ϣ
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
//�л���Ϣ
#define M_MSGTYPE_HANDOVERSTART		(0x10)		//Handover Start	0x10
#define M_MSGTYPE_HANDOVERSTARTACK	(0x11)		//Handover Start Ack	0x10
#define M_MSGTYPE_HANDOVERCOMPLETE	(0x12)		//Handover Complete
//�ƶ��Թ�����Ϣ
#define M_MSGTYPE_LOGINREQ			(0x20)		//Login req	0x12
#define M_MSGTYPE_LOGINRSP			(0x21)		//Login rsp	0x12
#define M_MSGTYPE_LOGOUT				(0x22)		//Logout	
#define M_MSGTYPE_AUTHCMD				(0x23)		//Authentication command	0x13
#define M_MSGTYPE_AUTHRSP				(0x24)		//Authentication rsp	0x13
#define M_MSGTYPE_MODI_GIDs_REQ		(0x25)		//ATTACH/DETACHGIDs Req	0x25	�ƶ��Թ�����Ϣ	UIDѰַ
#define M_MSGTYPE_MODI_GIDs_RSP		(0x26)		//ATTACH/DETACHGIDs Rsp	0x26	�ƶ��Թ�����Ϣ	UIDѰַ

//����Ϣ
#define M_MSGTYPE_MOSMSREQ			(0x30)		//MO Sms data req	0x14
#define M_MSGTYPE_MOSMSRSP			(0x31)		//MO Sms data rsp	0x14
#define M_MSGTYPE_MTSMSREQ			(0x32)		//MT Sms data req	0x15
#define M_MSGTYPE_MTSMSRSP			(0x33)		//MT Sms data rsp	0x15
#define M_MSGTYPE_SMSMEMAVAILREQ		(0x34)		//SMS memory available req	0x16
#define M_MSGTYPE_SMSMEMAVAILRSP		(0x35)		//SMS memory available rsp	0x16

#define M_MSGTYPE_CALLTICKET			(0x40)		//Call Ticket	0x40	��ֵҵ����Ϣ	L3��ַѰַ
#define M_MSGTYPE_CALLTICKET_ACK		(0x41)		//Call Ticket Ack	0x41	��ֵҵ����Ϣ	L3��ַѰַ
#define M_MSGTYPE_SRQ					(0x42)		//SRQ	0x42	��ֵҵ����Ϣ	UIDѰַ
#define M_MSGTYPE_SRP					(0x43)		//SRP	0x43	��ֵҵ����Ϣ	UIDѰַ
#define M_MSGTYPE_SRC					(0x44)		//SRC	0x44	��ֵҵ����Ϣ	UIDѰַ

#define M_MSGTYPE_CHANGEPWREQ		(0x50)		//Change PW req	0x50	�û�������Ϣ	UIDѰַ
#define M_MSGTYPE_CHANGEPWRSP		(0x51)		//Change PW rsp	0x51	�û�������Ϣ	UIDѰַ
#define M_MSGTYPE_CHANGEPWACK		(0x52)		//Change PW Ack	0x52	�û�������Ϣ	UIDѰַ
#define M_MSGTYPE_CfgFuncInfo_Req		(0x53)		//ConfigFunc Info req	0x53	�û�������Ϣ	UIDѰַ��OAM��
#define M_MSGTYPE_CfgFuncInfo_Rsp		(0x54)		//ConfigFunc Info rsp	0x54	�û�������Ϣ	UIDѰַ��OAM��
#define M_MSGTYPE_VoiceInfo_Req		(0x55)		//VoiceInfo.req	0x55	�û�������Ϣ	UIDѰַ��UTV��
#define M_MSGTYPE_VoiceInfo_Rsp		(0x56)		//VoiceInfo.rsp	0x56	�û�������Ϣ	UIDѰַ��UTV��
#define M_MSGTYPE_TRANSDATA_REQ		(0x57)		//Transdata req	0x57	�û�������Ϣ	UIDѰַ
#define M_MSGTYPE_TRANSDATA_RSP		(0x58)		//Transdata rsp	0x58	�û�������Ϣ	UIDѰַ
#define M_MSGTYPE_OAMTRANSFER_INFO_REQ	(0x59)		//OAMTransfer_Info req	0x59	�û�������Ϣ	UIDѰַ��OAMmessage��
#define M_MSGTYPE_OAMTRANSFER_INFO_RSP	(0x5A)		//OAMTransfer_Info rsp	0x5A	�û�������Ϣ	UIDѰַ(OAMmessage)
#define M_MSGTYPE_SecurityCardCallPara_Req	(0x5B)		//SecurityCardCallPara req
#define M_MSGTYPE_SecurityCardCallPara_Rsp	(0x5C)		//SecurityCardCallPara rsp


//������п�����Ϣ
#define M_MSGTYPE_PTT_SETUP_REQ		(0x60)		//PTT Setup Req	0x60	���п�����Ϣ	L3��ַѰַ
#define M_MSGTYPE_PTT_SETUP_ACK		(0x61)		//PTT Setup ack	0x61	���п�����Ϣ	L3��ַѰַ
#define M_MSGTYPE_PTT_CONNECT		(0x62)		//PTT Connect	0x62	���п�����Ϣ	L3��ַѰַ
#define M_MSGTYPE_PTT_CONNECT_ACK	(0x63)		//PTT Connect Ack	0x63	���п�����Ϣ	L3��ַѰַ
#define M_MSGTYPE_GRP_DISCONNECT		(0x64)		//Group Disconnect	0x64	���п�����Ϣ	��L3��ַѰַ
#define M_MSGTYPE_GRP_RLS				(0x65)		//Group Release	0x65	���п�����Ϣ	��L3��ַѰַ
#define M_MSGTYPE_GRP_CALLING_RLS	(0x66)		//Group Calling Release	0x66	���п�����Ϣ	L3��ַѰַ
#define M_MSGTYPE_GRP_CALLING_RLSACK	(0x67)		//Group Calling Release Complete	0x67	���п�����Ϣ	L3��ַѰַ

//��Ȩ������Ϣ
#define M_MSGTYPE_PTT_PRESS_REQ		(0x70)		//PTT PRESS REQ	0x70	��Ȩ������Ϣ	�����UIDѰַ
#define M_MSGTYPE_PTT_PRESS_RSP		(0x71)		//PTT PRESS RSP	0x71	��Ȩ������Ϣ	�����UIDѰַ
#define M_MSGTYPE_PTT_GRANTED		(0x72)		//PTT GRANTED	0x72	��Ȩ������Ϣ	��L3��ַѰַ
#define M_MSGTYPE_PTT_INTERRUPT		(0x73)		//PTT INTERRUPT	0x73	��Ȩ������Ϣ	L3Ѱַ
#define M_MSGTYPE_PTT_INTERRUPT_ACK	(0x74)		//PTT INTERRUPT ACK	0x74	��Ȩ������Ϣ	L3Ѱַ
#define M_MSGTYPE_PTT_RLS			(0x75)		//PTT RELEASE	0x75	��Ȩ������Ϣ	L3��ַѰַ
#define M_MSGTYPE_PTT_RLS_ACK			(0x76)		//PTT RELEASE ACK	0x76	��Ȩ������Ϣ	L3��ַѰַ
#define M_MSGTYPE_PTT_PRESS_INFO		(0x77)		//PTT PRESS INFO	0x77	��Ȩ������Ϣ	L3��ַѰַ
#define M_MSGTYPE_PTT_PRESS_CANCEL	(0x78)		//PTT PRESS CANCLE 	0x78	��Ȩ������Ϣ	�����UIDѰַ

//��ֵҵ����Ϣ
#define M_MSGTYPE_USR_STATUS_QRY		(0x90)		//USER STATUS QUERY	0x90	״̬������Ϣ	L3Ѱַ
#define M_MSGTYPE_USR_STATUS_RSP		(0x91)		//USER_STATUS_RESPONSE	0x91	״̬������Ϣ	L3Ѱַ
#define M_MSGTYPE_USR_STATUS_NOTI	(0x92)		//USER_STATUS_NOTIFY	0x92	״̬������Ϣ	L3Ѱַ

#define M_MSGTYPE_DB_MO_DATA_REQ	(0xA0)		//MO DB data req	0xA0		UIDѰַ
#define M_MSGTYPE_DB_MO_DATA_RSP	(0xA1)		//MO DB data rsp	0xA1		UIDѰַ
#define M_MSGTYPE_DB_RETRANSDATA_REQ	(0xA2)		//RetranData req	0xA2		UIDѰַ
#define M_MSGTYPE_DB_RETRANSDATA_RSP	(0xA3)		//RetranData rsp	0xA3		UIDѰַ

#define M_MSGTYPE_MAX_VALUE			(M_MSGTYPE_DB_RETRANSDATA_RSP)
//////////////////////////////////////////////////////////////////////////
typedef enum
{
	//FORMID���ڱ�ʶ���ݰ������ݷ�װ��ʽ���������£�
	SRTP,//0b000 	�û�ƽ�����������ݣ�
	SDATA,//0b001 	�û�ƽ�����������ݣ�
	SyncBroadcastVData,//0b010 	  ��Ⱥ���ͬ���û�ƽ������������
	SyncBroadcastDData,//0b011 	  ��Ⱥ���ͬ���û�ƽ������������
	NATAP_PROTOCOL=7//#define M_PROTOCOL_ID_NATAP	(7)
}ENUM_FormIDT;

typedef enum
{
	CALLGRANT_FREE,////0 ��Ȩ����
	CALLGRANT_INUSE////1 ��Ȩռ�� 	
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
	PRIO_0,				// 0δ�������ȼ�
	PRIO_1,				// 1���ȼ�1����ͣ�
	PRIO_2,				// 2���ȼ�2
	PRIO_3,				// 3���ȼ�3
	PRIO_HIGH1,		// 4Ԥռ����Ȩ1
	PRIO_HIGH2,		// 5Ԥռ����Ȩ2
	PRIO_HIGH3,		// 6Ԥռ����Ȩ3
	PRIO_EMERGENT		// 7Ԥռ����Ȩ4���������У�
}ENUM_CallPrioty;
#define M_DEFAULT_VOICE_PRIORITY			(PRIO_3)		//Ŀǰ�汾�̶������������ȼ� 3
#define M_DMUX_VOICEDATA_PRIORITY		(0x04)		//DMUX�ӿ��й涨 4

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
{// ͨ������
	COMM_TYPE_BROADCAST,				// 0���㲥
	COMM_TYPE_VOICE_GRPCALL,			// 1���������
	COMM_TYPE_MULTIMEDIA_GRPCALL,	// 2��������ý�����
	COMM_TYPE_DATA_GRPCALL,			// 3���������
	COMM_TYPE_SYNC_BROADCAST_VOICE,// 4������ͬ��
	COMM_TYPE_SYNC_BROADCAST_DATA// 5������ͬ��
}ENUM_CommTypeT;

typedef enum
{
	TRANSGRANT_PERMIT_TALK=1,// 1������
	TRANSGRANT_FORBID_TALK,// 2��������
	TRANSGRANT_WAITING_TALK,// 3��Ȩ�Ŷ�
	TRANSGRANT_GRPSRV_NOT_EXIST,// 4�����������
	TRANSGRANT_SXC_CANCEL_WAITING_TALK// 5����������ȡ���Ŷ�	
}ENUM_TransmissionGrantT;

typedef enum
{
	USE_PAGING_CHANNEL=0,
	USE_DAC_CHANNEL
}ENUM_sessionTypeT;

//ҵ������AppType
typedef enum
{
	APPTYPE_VOICE_QCELP=0,		//	00	����Ӧ��/ QCELP�������������Ӧ��
	APPTYPE_VOICE_G729,		//	01	����/ G729�������������Ӧ��
	APPTYPE_VOICE_G729B,		//	02	����/ G729B��������
	APPTYPE_RESERVED1=3,		//	03	������������
	APPTYPE_SMS,				//	04	����ϢӦ��
	APPTYPE_LOWSPEED_DATA,		//	05	��������
	APPTYPE_RESERVED2,			//	06	G729D�������������Ӧ��
	APPTYPE_BER_TEST,			//	07	��ʾΪ������Խ�������
	APPTYPE_G729A_BCH,          //  08  G729 plus�������������Ӧ��(G729A+BCH����)
	APPTYPE_G729D_B,            //  09  G729D+B�������������Ӧ��
	APPTYPE_G729A_FAX,            //  0a  G729+����
	APPTYPE_G729B_FAX,            //  0b  G729B+����
	APPTYPE_CMD_REG=0x10,		//	0x10	ָ��Ǽ�
	APPTYPE_P2P_LOAD,            //  11  �˵��˳���ҵ��
	APPTYPE_WEB_LOAD,            //  12  ����ģʽ����ҵ��
	APPTYPE_OAM_FORCE_REGISTER,		// 13 OAMǿ��ע��
	APPTYPE_ENCRYPT_VOICE,		// 14 ��������Ӧ��//fengbing 20100204
	APPTYPE_VOICE_SWITCH=0x20,       //  20 �����л�Ӧ��
	APPTYPE_DUPCPE_PAGING,		//	0x21 �I��Ѱ��
	APPTYPE_GRP_MANAGE,			//22	�鸽��/ȥ����Ѱ��
	APPTYPE_ENC_SIGNAL_SEND,	//	0x23���ⲿ���������·�
	APPTYPE_RESERVED		   //	����
}ENUM_AppTypeT;

//11.36  ServiceAppType
typedef enum
{
	SERV_APPTYPE_DTE=0,		//	00	��׼DTE
	SERV_APPTYPE_RATE_DWNLOAD,		    //	01	���߷�������
	SERV_APPTYPE_REMOTE_CPE,		//	02	Զ���ն�ά��
	SERV_APPTYPE_RESERVED		    //	����
}ENUM_ServiceAppTypeT;

//Setup Cause
typedef enum
{
	SETUPCAUSE_VOICE=0,			//	00H �� ������
	SETUPCAUSE_DATA,				//	01H �� ��������
	SETUPCAUSE_BROADCAST,		//	02H �� �㲥
	SETUPCAUSE_CARRYSRV,			//	03H �� ����ҵ��
	SETUPCAUSE_ENCRYPT_VOICE		//04H �� ��������
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
	REL_CAUSE_UNKNOWN,			//0x09	δ֪ԭ��
	REL_CAUSE_USR_REFUSE,			//0x0A	�û��ܽ�
	REL_CAUSE_TALK_TOOLONG,		//0x0B	������ʱ�䳬ʱ�ͷ�
	REL_CAUSE_IDLE_TIMEOUT,		//0x0C	���г�ʱ�ͷ�
	REL_CAUSE_PTT_INTERRUPT,		//0x0D	ǿ�ƻ�Ȩ�ͷ�
	REL_CAUSE_INTERRUPT,			//0x0E	��ǿ��
	REL_CAUSE_PTT_PREEMPT,		//0x0F	�������û���ռ��Ȩ
	REL_CAUSE_PEER_BUSY,			//0x10	�Զ�æ
	REL_CAUSE_CALL_BOOKCED		//0x11	���б�Ԥռ
	
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
	CODEC_G729B_UNT,//01H �� G.729b UNT
	CODEC_G729D,	//02H �� G.729d
	CODEC_G711A,	//03H �� G.711(PCM A��)
	CODEC_G729B_SID,	//01H �� G.729b SID
	CODEC_G729AB,	//5H �� G.729AB��
	CODEC_ENCRYPT_G729A,//6H �� ��������֡/������Ϣ֡��//6H �� �����Ѵ�������֡/������Ϣ֡��

	CODEC_GRPTONE=0x0A,	//��Ⱥר�ñ��뷽ʽ����ͬG.729A��������־��ͬG.729�Ĵ���//AH �� PTT���������֡����
	CODEC_ENCRYPT_GRPTONE=0x0B,	//��Ⱥר�ñ��뷽ʽ����ͬG.729A��������־��ͬG.729�Ĵ���//BH �� �Ѵ���PTT/������Ϣ֡���������֡����
	CODEC_ERR_FRAME=0x0D,	//DH �� ��֡��
	CODEC_NULL_FRAME,	//EH �� ��֡��
	CODEC_DTMF_FRAME	//FH �� DTMF����֡
}ENUM_VoiceCodecT;
#define isEncSrtpVData(codec)	 (CODEC_ENCRYPT_G729A==codec || CODEC_ENCRYPT_GRPTONE==codec)
#define isGrpSrtpVData(codec) (CODEC_GRPTONE==codec || CODEC_ENCRYPT_GRPTONE==codec)

//L2L3֮��ӿڣ���Ϊ��Ⱥ������ʱcid�ֶι̶���д0xFE
#define GRP_TONE_FLAG_CID		(0xFE)	

//REG_TYPE	����
typedef enum
{
	REGTYPE_POWERON=0,			//00H	�����Ǽ�
	REGTYPE_NEWLOCATION,		//01H	λ�ø���
	REGTYPE_PERIOD,				//02H	����ע��
	REGTYPE_CMDREG				//03H	ָ��Ǽ�
								//04H - FFH	����
}ENUM_RegTypeT;

//HO_TYPE	����
typedef enum
{
	HO_TYPE_CONVERSATION,		//00H	ͨ��״̬�µ��л�
	HO_TYPE_NOTCONNECTED		//01H	��ͨ��״̬�µ��л�
								//02H - FFH	����
}ENUM_HoTypeT;

typedef enum
{
	LOGINRSP_SUCCESS,				//00H	ע��ɹ�

	LOGINRSP_USRNOTEXIST,			//01H	����ʹ�á�HLR���޴��û�
	LOGINRSP_ARGUMENTERROR,			//02H	�������󣨺���������ı�ǩ��
	LOGINRSP_LACKOFARGUMENT,		//03H	ȱ�ٲ���������������ı�ǩ��
	LOGINRSP_BUSYORFAULT,			//04H	����æ�����
	LOGINRSP_OLDCPENOTREGONOTHERSAG,//05H	���ֻ�δ������SAG��ע��
	LOGINRSP_NOROAMRIGHT,			//06H	������Ȩ��
	LOGINRSP_AUTHFAIL,				//07H	��Ȩʧ��
	LOGINRSP_STEAL_USER,//08H	�û�������
	LOGINRSP_USER_DUPLICATE,//09H	�û�������
	LOGINRSP_UNKNOW_REASON,//0aH	��ȷ��ԭ��
	LOGINRSP_BTS_BLACKLIST,//0x0d	��վ���ν�ֹ
	LOGINRSP_SOFTPHONE,//0x10	�ն����ʹ���ΪSofthphone�ն�
	LOGINRSP_CPE_NUM_SPLIT//0x11	�ն����ʹ���Ϊ���ŷ����ն�
									//����	����

}ENUM_LoginResultT;

//SMS_Cause
typedef enum
{
	SMSCAUSE_SUCCESS=0,				//0		�ɹ�
	SMSCAUSE_SETUPLINKFAIL=21,		//21	�������ɹ�
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

//BroadcastStatus
typedef enum
{
	BRDCSTSTAS_SUCCESS=0,			//0		�ɹ�
	BRDCSTSTAS_FAIL,		        //1	    ʧ�ܣ�δ����ԭ��
	BRDCSTSTAS_BS_FULL,       		//0x02��BS�洢�ռ�������
	BRDCSTSTAS_BS_OVERLOAD, 		//0x03��BS���أ��޷���ָ����ʱ�䷢�͹㲥����
	BRDCSTSTAS_OTHER 
}ENUM_BrdCstStus;

//RejectReason �ܾ�ԭ����Ч��ΧΪ0x80 ~ 0xFF��
typedef enum
{
	REJECT_RECERVED1=0x80,			//0x80	����
	REJECT_LENTHERR,		        //0x81����Ϣ���ȴ���
	REJECT_EVENTIDERR,       		//0x82��EventID�ֶ�δ���塣
	REJECT_SUB_EVENTIDERR, 		    //0x83��SubEventID�ֶ�δ���塣
    REJECT_DSTADDRERR,              //0x84��Ŀ�ĵ�ַ����
    REJECT_SRCADDRERR,              //0x85��Դ��ַ����
	REJECT_RECERVED2                //0x86 ~ 0xFF��������
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
	FUNCLIST_5//��������
}ENUM_FUNCLIST;

typedef enum
{
	CONFIGRESULT_0=0,
	CONFIGRESULT_1,
	CONFIGRESULT_2,
	CONFIGRESULT_3,
	CONFIGRESULT_4,
	CONFIGRESULT_5//��������
}ENUM_CONFIGRESULT;

typedef enum
{
	MSG_TYPE1__NOTUSE,
	
	MSG_TYPE1__MBMS_DELAY_REQ=0x1,//MBMS_DELAY_REQ	0x01	ͬ��������Ϣ	
	MSG_TYPE1__MBMS_DELAY_RSP,//MBMS_DELAY_RSP	0x02	ͬ��������Ϣ	
	MSG_TYPE1__MBMS_RS_IND,//MBMS_RS_IND 0x03	ͬ��������Ϣ	
	MSG_TYPE1__MBMS_RS_RSP,//MBMS_RS_RSP 0x04	ͬ��������Ϣ	
	MSG_TYPE1__MBMS_RS_REQ,//MBMS_RS_REQ 0x05	ͬ��������Ϣ	

	MSG_TYPE1__COUNT_PLUS_ONE
}ENUM_MSGTYPE1T;

#define M_TERMINAL_TYPE_GRPCPE	(0x30)	//30H	��Ⱥ�ն�
#define M_SWVERSION_V52PLUS	(2)			// 2	V5.2+�汾

#define M_STATUSREPORT_HEARTBEAT		(0x01)

//error notification ��error cause
#define M_ERRNOTIFY_ERR_CAUSE_AIRFAIL		(0x00)	//0x00	BS��⵽���нӿ���·ʧ��
#define M_ERRNOTIFY_ERR_CAUSE_CPERELEASE	(0x01)	//0x01	�ն��쳣�ͷ�
#define M_ERRNOTIFY_ERR_CAUSE_SAG_FOUND_ERROR	(0x80)	//0x80	SAG��⵽�쳣

#define M_SABIS_SUCCESS	(0)
#define M_SABIS_FAIL		(1)
#define M_INVALID_GRPL3ADDR 	(0xffffffff)
#define M_INVALID_GID			(0xffff)
#define INVALID_UID (0xffffffff)
#define NO_L3ADDR	(0xffffffff)		//	Ĭ��L3��ַ0xffffffff
#define SMS_L3ADDR	(0xfffffffe)		//	SMS��Ϣ��L3Addr
#define M_DEFAULT_ERR_NOTI_L3ADDR		(0xfffffffe)	//error notify��Ϣ�����û��L3��ַʱ��д
#define NO_EID		(0xffffffff)
#define NO_CID		(0xff)
#define DEFAULT_CID	(0)
#define M_INVALID_BTSID			(0xffffffff)

#define VOICE_CCB_NUM (6000)
#define GRP_CCB_NUM	(200)					//֧��200�鶨��
#define MAX_INSRV_GRPNUM	(100)				//֧��100���鲢��ҵ��
#define  STATUSREPORT_POOLSIZE		(6000)	//״̬������Pool

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

//����ΪSAbis1.5���������ʱû�м�����pm��������ͳ�ƽӿڣ������Ѿ�ʵ���˼�����20060906 by fengbing

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

//����ΪSAbis1.6���������ʱû�м�����pm��������ͳ�ƽӿڣ������Ѿ�ʵ���˼�����20071103 by fengbing

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

	CfgFuncInfo_Req_MSG,	//(0x53)		//ConfigFunc Info req	0x53	�û�������Ϣ	UIDѰַ��OAM��
	CfgFuncInfo_Rsp_MSG,	//(0x54)		//ConfigFunc Info rsp	0x54	�û�������Ϣ	UIDѰַ��OAM��
	VoiceInfo_Req_MSG,		//(0x55)		//VoiceInfo.req	0x55	�û�������Ϣ	UIDѰַ��UTV��
	VoiceInfo_Rsp_MSG,		//(0x56)		//VoiceInfo.rsp	0x56	�û�������Ϣ	UIDѰַ��UTV��
	Transdata_UL_MSG,	//		Transdata uplink	0x57	�û�������Ϣ	UIDѰַ
	Transdata_DL_MSG,	//		Transdata downlink	0x58	�û�������Ϣ	UIDѰַ
	OAMTransfer_Info_Req_MSG,	//		OAMTransfer_Info req	0x59	�û�������Ϣ	UIDѰַ��OAMmessage��
	OAMTransfer_Info_Rsp_MSG,	//		OAMTransfer_Info rsp	0x5A	�û�������Ϣ	UIDѰַ(OAMmessage)
	SecuriryCardCallPara_Req_MSG,	//		SecuriryCardCallPara req	0x5B	�û�������Ϣ	L3��ַѰַ
	SecuriryCardCallPara_Rsp_MSG,	//		SecuriryCardCallPara rsp	0x5C	�û�������Ϣ	L3��ַѰַ
	
//��Ⱥ���begin
	LAGrpPagingReq_MSG,	//LA Group paging	0x05	���п�����Ϣ	0x00
	LAGrpPagingRsp_MSG,	//LA Group paging rsp	0x06	���п�����Ϣ	0x00
	LEGrpPaging_MSG,		//LE Group paging	0x07	���п�����Ϣ	0x00
	DeLEGrpPaging_MSG,		//De-LE Group paging	0x08	���п�����Ϣ	0x00
	StatusReport_MSG,		//USER_STATUS_REPORT	0x09	���п�����Ϣ	0x00
	GrpHandoverReq_MSG,	//Group Handover req	0x0A	���п�����Ϣ	0x00
	GrpHandoverRsp_MSG,	//Group Handover rsp	0x0B	���п�����Ϣ	0x00	

	RlsSmsLinkReq_MSG,		//Release Sms link req	0x05		0x01
	RlsSmsLinkRsp_MSG,		//Release Sms link rsp	0x06		0x01
	GrpResReq_MSG,			//Group Resouse req	0x07		0x01
	GrpResRsp_MSG,			//Group Resouse rsp	0x08		0x01

	UTSXC_GRPL3Addr_MSG,	//��L3Ѱַ��UE-SAG message	0x03		0x03
	UTSXC_GRPUID_MSG,		//�����UIDѰַ��UE-SAG message	0x04		0x03

	//������п�����Ϣ
	PTT_SetupReq_MSG,		//PTT Setup Req	0x60	���п�����Ϣ	L3��ַѰַ
	PTT_SetupAck_MSG,		//PTT Setup ack	0x61	���п�����Ϣ	L3��ַѰַ
	PTT_Connect_MSG,		//PTT Connect	0x62	���п�����Ϣ	L3��ַѰַ
	PTT_ConnectAck_MSG,	//PTT Connect Ack	0x63	���п�����Ϣ	L3��ַѰַ
	Grp_Disconnect_MSG,	//Group Disconnect	0x64	���п�����Ϣ	��L3��ַѰַ
	Grp_Release_MSG,		//Group Release	0x65	���п�����Ϣ	��L3��ַѰַ
	Group_CallingRls_MSG,	//Group Calling Release	0x66	���п�����Ϣ	L3��ַѰַ
	Group_CallingRlsComplete_MSG,	//Group Calling Release Complete	0x67	���п�����Ϣ	L3��ַѰַ
	//�ƶ��Թ�����Ϣ
	Modify_GIDs_Req_MSG,	//ATTACH/DETACHGIDs Req	0x25	�ƶ��Թ�����Ϣ	UIDѰַ
	Modify_GIDs_Rsp_MSG,	//ATTACH/DETACHGIDs Rsp	0x26	�ƶ��Թ�����Ϣ	UIDѰַ
	//��Ȩ������Ϣ
	PTT_PressReq_MSG,		//PTT PRESS REQ	0x70	��Ȩ������Ϣ	�����UIDѰַ
	PTT_PressRsp_MSG,		//PTT PRESS RSP	0x71	��Ȩ������Ϣ	�����UIDѰַ
	PTT_Granted_MSG,		//PTT GRANTED	0x72	��Ȩ������Ϣ	��L3��ַѰַ
	PTT_Interrupt_MSG,		//PTT INTERRUPT	0x73	��Ȩ������Ϣ	L3Ѱַ
	PTT_InterruptAck_MSG,	//PTT INTERRUPT ACK	0x74	��Ȩ������Ϣ	L3Ѱַ
	PTT_Rls_MSG,			//PTT RELEASE	0x75	��Ȩ������Ϣ	L3��ַѰַ
	PTT_RlsAck_MSG,		//PTT RELEASE ACK	0x76	��Ȩ������Ϣ	L3��ַѰַ
	PTT_PressInfo_MSG,		//PTT PRESS INFO	0x77	��Ȩ������Ϣ	L3��ַѰַ
	PTT_PressCancel_MSG,	//PTT PRESS CANCLE 	0x78	��Ȩ������Ϣ	�����UIDѰַ
	//��Ȩ����ʡ��Դ�Ż�
	PTT_PressApplyReq_MSG,
	PTT_PressApplyRsp_MSG,
	//��ֵҵ����Ϣ
	User_StatusQuery_MSG,	//USER STATUS QUERY	0x90	״̬������Ϣ	L3Ѱַ
	User_StatusRsp_MSG,	//USER_STATUS_RESPONSE	0x91	״̬������Ϣ	L3Ѱַ
	User_StatusNotify_MSG,	//USER_STATUS_NOTIFY	0x92	״̬������Ϣ	L3Ѱַ
//��Ⱥ���end
//�����鲥������begin
	DB_GrpPaging_MSG,
	DB_GrpPagingRsp_MSG,
	DB_DataComplete_MSG,
	DB_MODataReq_MSG,
	DB_MODataRsp_MSG,
	DB_RetransDataReq_MSG,
	DB_RetransDataRsp_MSG,
//�����鲥������end
	UTSAG_UID_OAM_MSG,
	DVoiceConfigReg_MSG,	//DVoice Config
	UTSAG_RELAY_MSG,	//�����ն�ע��͸������Ϣ
//ͬ��begin
	BTSL2_SAG_MSG,
//ͬ��end
	InvalidSignal_MSG

	//NotParsed_MSG=0xffff
}SignalType;

//Error Codes for L3Voice Tasks
#include "logarea.h"
const UINT16 EC_L3VOICE_NORMAL               = 0x0000;  //��������
const UINT16 EC_L3VOICE_UNEXPECTED_MSGID     = 0x0001;  //�Ƿ�Message ID
const UINT16 EC_L3VOICE_SYS_FAIL             = 0x0002;  //ϵͳ����
const UINT16 EC_L3VOICE_SOCKET_ERR           = 0x0003;  //Socket����
const UINT16 EC_L3VOICE_INVALID_SIGNAL       = 0x0004;  //��������
const UINT16 EC_L3VOICE_MSG_EXCEPTION        = 0x0005;  //�쳣��Ϣ
const UINT16 EC_L3VOICE_MSG_SND_FAIL         = 0x0006;	//����䷢����Ϣʧ��
const UINT16 EC_L3VOICE_INVALID_VOICEDATA    = 0x0007;	//�Ƿ�������


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


