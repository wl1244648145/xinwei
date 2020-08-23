/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    CallSignalMsg.cpp
 *
 * DESCRIPTION: 
 *	BTS SAbis Signal interface functions.
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   2006-09-06 fengbing  信令种类增加透传类信令callticket,callticketAck,SRQ,SRP,SRC,
 *                        相应改动信令统计，与其他任务接口暂时不变，但是新增信令也进行计数 
 *   2006-08-14 fengbing  add DeleteMessage() when Post CMessage failed
 *   2006-04-18 fengbing  CMsg_Signal_VCR::Post()中加入tVCR任务信令计数
 *   2006-3-27  fengbing  SAGSignalHowToFindCCB函数中加入对Error Notification Req的处理
 *
 *---------------------------------------------------------------------------*/
 
#include <string.h>


#include "voiceToolFunc.h"
#include "CallSignalMsg.h"
#include "BtsVMsgId.h"
#include "log.h"
#include "taskdef.h"

UINT16 CMsg_Signal_VCR::m_BTSID = 0xffff;
UINT32 CMsg_Signal_VCR::m_SAGID = 0xffffffff;

char CMsg_Signal_VCR::m_sigName[InvalidSignal_MSG+1][40]=
{
	"LAPaging_MSG",
	"LAPagingRsp_MSG",
	"DELAPagingReq_MSG",
	"DELAPagingRsp_MSG",
	"AssignResReq_MSG",
	"AssignResRsp_MSG",
	"RlsResReq_MSG",
	"RlsResRsp_MSG",
	"Reset_MSG",
	"ResetAck_MSG",
	"BeatHeart_MSG",
	"BeatHeartAck_MSG",
	"CongestionCtrlReq_MSG",
	"CongestionCtrlRsp_MSG",
	"ErrNotifyReq_MSG",
	"ErrNotifyRsp_MSG",
	

	"Setup_MSG",
	"CallProc_MSG",
	"Alerting_MSG",
	"Connect_MSG",
	"ConnectAck_MSG",
	"Disconnect_MSG",
	"Release_MSG",
	"ReleaseComplete_MSG",
	"ModiMediaReq_MSG",
	"ModiMediaRsp_MSG",
	"Information_MSG",
	"AuthCmdReq_MSG",
	"AuthCmdRsp_MSG",
	"Login_MSG",
	"LoginRsp_MSG",
	"Logout_MSG",
	"HandOverReq_MSG",
	"HandOverRsp_MSG",
	"HandOverComplete_MSG",
	"StatusQry_MSG",
	"Status_MSG",
	
	"MOSmsDataReq_MSG",
	"MOSmsDataRsp_MSG",
	"MTSmsDataReq_MSG",
	"MTSmsDataRsp_MSG",
	"SMSMemAvailReq_MSG",
	"SMSMemAvailRsp_MSG",
	
	"UTSAG_L3Addr_MSG",
	"UTSAG_UID_MSG",

//以下为SAbis1.5新增信令，暂时没有加入与pm任务性能统计接口，但是已经实现了计数，20060906 by fengbing

	"BROADCAST_SM_MSG", 		
	"BROADCAST_SM_ACK_MSG",				
	"REJECT_MSG", 					
	"RESET_REQ_MSG",   		    	
	"RESTART_INDICATION_MSG", 		
	"FAIL_INDICATION_MSG",	    	

	"CALLTICKET_MSG",
	"CALLTICKET_ACK_MSG",
	"SRQ_MSG",
	"SRP_MSG",
	"SRC_MSG",

//以下为SAbis1.6新增信令，暂时没有加入与pm任务性能统计接口，但是已经实现了计数，20071103 by fengbing

	"Auth_Info_Req_MSG",
	"Auth_Cmd_MSG",
	"Auth_Rsp_MSG",
	"Auth_Result_MSG",
	"BWInfo_Req_MSG",
	"BWInfo_Rsp_MSG",
	"BWInfo_Del_Req_MSG",
	"BWInfo_Modify_Req_MSG",
	"BWInfo_Modify_Rsp_MSG",
	"SWITCH_OFF_NOTIFY_MSG",
	"AccountLoginReq_MSG",	//AccountLogin req	0x0B		0x05
	"ConfigInfo_Transfer_Msg",//ConfigInfo_Transfer	0x0C		0x05

	"ChangePW_Req_MSG",
	"ChangePW_Rsp_MSG",
	"ChangePW_Ack_MSG",

	"CfgFuncInfo_Req_MSG",	//(0x53)		//ConfigFunc Info req	0x53	用户管理消息	UID寻址（OAM）
	"CfgFuncInfo_Rsp_MSG",	//(0x54)		//ConfigFunc Info rsp	0x54	用户管理消息	UID寻址（OAM）
	"VoiceInfo_Req_MSG",		//(0x55)		//VoiceInfo.req	0x55	用户管理消息	UID寻址（UTV）
	"VoiceInfo_Rsp_MSG",		//(0x56)		//VoiceInfo.rsp	0x56	用户管理消息	UID寻址（UTV）	
	"Transdata_UL_MSG",	//		Transdata uplink	0x57	用户管理消息	UID寻址
	"Transdata_DL_MSG",	//		Transdata downlink	0x58	用户管理消息	UID寻址
	"OAMTransfer_Info_Req_MSG",	//		OAMTransfer_Info req	0x59	用户管理消息	UID寻址（OAMmessage）
	"OAMTransfer_Info_Rsp_MSG",	//		OAMTransfer_Info rsp	0x5A	用户管理消息	UID寻址(OAMmessage)
	"SecuriryCardCallPara_Req_MSG",	//		SecuriryCardCallPara req	0x5B	用户管理消息	L3地址寻址
	"SecuriryCardCallPara_Rsp_MSG",	//		SecuriryCardCallPara rsp	0x5C	用户管理消息	L3地址寻址

//集群相关begin
	"LAGrpPagingReq_MSG",	//LA Group paging	0x05	呼叫控制消息	0x00
	"LAGrpPagingRsp_MSG",	//LA Group paging rsp	0x06	呼叫控制消息	0x00
	"LEGrpPaging_MSG",		//LE Group paging	0x07	呼叫控制消息	0x00
	"DeLEGrpPaging_MSG",		//De-LE Group paging	0x08	呼叫控制消息	0x00
	"StatusReport_MSG",		//USER_STATUS_REPORT	0x09	呼叫控制消息	0x00
	"GrpHandoverReq_MSG",	//Group Handover req	0x0A	呼叫控制消息	0x00
	"GrpHandoverRsp_MSG",	//Group Handover rsp	0x0B	呼叫控制消息	0x00	

	"RlsSmsLinkReq_MSG",		//Release Sms link req	0x05		0x01
	"RlsSmsLinkRsp_MSG",		//Release Sms link rsp	0x06		0x01
	"GrpResReq_MSG",			//Group Resouse req	0x07		0x01
	"GrpResRsp_MSG",			//Group Resouse rsp	0x08		0x01

	"UTSXC_GRPL3Addr_MSG",	//组L3寻址的UE-SAG message	0x03		0x03
	"UTSXC_GRPUID_MSG",		//组呼的UID寻址的UE-SAG message	0x04		0x03

	//组呼呼叫控制消息
	"PTT_SetupReq_MSG",		//PTT Setup Req	0x60	呼叫控制消息	L3地址寻址
	"PTT_SetupAck_MSG",		//PTT Setup ack	0x61	呼叫控制消息	L3地址寻址
	"PTT_Connect_MSG",		//PTT Connect	0x62	呼叫控制消息	L3地址寻址
	"PTT_ConnectAck_MSG",	//PTT Connect Ack	0x63	呼叫控制消息	L3地址寻址
	"Grp_Disconnect_MSG",		//Group Disconnect	0x64	呼叫控制消息	组L3地址寻址
	"Grp_Release_MSG",		//Group Release	0x65	呼叫控制消息	组L3地址寻址
	"Group_CallingRls_MSG",	//Group Calling Release	0x66	呼叫控制消息	L3地址寻址
	"Group_CallingRlsComplete_MSG",	//Group Calling Release Complete	0x67	呼叫控制消息	L3地址寻址
	//移动性管理消息
	"Modify_GIDs_Req_MSG",	//ATTACH/DETACHGIDs Req	0x25	移动性管理消息	UID寻址
	"Modify_GIDs_Rsp_MSG",	//ATTACH/DETACHGIDs Rsp	0x26	移动性管理消息	UID寻址
	//话权控制消息
	"PTT_PressReq_MSG",		//PTT PRESS REQ	0x70	话权控制消息	组呼的UID寻址
	"PTT_PressRsp_MSG",		//PTT PRESS RSP	0x71	话权控制消息	组呼的UID寻址
	"PTT_Granted_MSG",		//PTT GRANTED	0x72	话权控制消息	组L3地址寻址
	"PTT_Interrupt_MSG",		//PTT INTERRUPT	0x73	话权控制消息	L3寻址
	"PTT_InterruptAck_MSG",	//PTT INTERRUPT ACK	0x74	话权控制消息	L3寻址
	"PTT_Rls_MSG",			//PTT RELEASE	0x75	话权控制消息	L3地址寻址
	"PTT_RlsAck_MSG",		//PTT RELEASE ACK	0x76	话权控制消息	L3地址寻址
	"PTT_PressInfo_MSG",		//PTT PRESS INFO	0x77	话权控制消息	L3地址寻址
	"PTT_PressCancel_MSG",	//PTT PRESS CANCLE 	0x78	话权控制消息	组呼的UID寻址
	//话权申请省资源优化
	"PTT_PressApplyReq_MSG",	//属于BTS?SAG消息，EVENT GROUP ID=0x01，EVENT ID=0x09
	"PTT_PressApplyRsp_MSG",	//BTS?SAG消息，EVENT GROUP ID=0x01，EVENT ID=0x0a
	//增值业务消息
	"User_StatusQuery_MSG",	//USER STATUS QUERY	0x90	状态报告消息	L3寻址
	"User_StatusRsp_MSG",		//USER_STATUS_RESPONSE	0x91	状态报告消息	L3寻址
	"User_StatusNotify_MSG",	//USER_STATUS_NOTIFY	0x92	状态报告消息	L3寻址

//集群相关end	
//数据组播、单播begin
	"DB_GrpPaging_MSG",
	"DB_GrpPagingRsp_MSG",
	"DB_DataComplete_MSG",
	"DB_MODataReq_MSG",
	"DB_MODataRsp_MSG",
	"DB_RetransDataReq_MSG",
	"DB_RetransDataRsp_MSG",
//数据组播、单播end
	"UTSAG_UID_OAM_MSG",
	"DVoiceConfigReg_MSG",
	"UTSAG_RELAY_MSG",	//无需终端注册透传的消息
//同播begin
	"BTSL2_SAG_MSG",
//同播end

	"InvalidSignal_MSG"
};

SigIDST	CMsg_Signal_VCR::m_SigIDS[InvalidSignal_MSG+1]=
{
	//EventGroupID					//EventID
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_LAPAGING},			//LAPaging_MSG=0,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_LAPAGINGRSP},		//LAPagingRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DELAPAGING},			//DELAPagingReq_MSG,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DELAPAGINGRSP},		//DELAPagingRsp_MSG,

	{M_MSG_EVENT_GROUP_ID_RESMANAGE,M_MSG_EVENT_ID_ASSGNRESREQ},		//AssignResReq_MSG,
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,M_MSG_EVENT_ID_ASSGNRESRSP},		//AssignResRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,M_MSG_EVENT_ID_RLSRESREQ},			//RlsResReq_MSG,
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,M_MSG_EVENT_ID_RLSRESRSP},			//RlsResRsp_MSG,

	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_RESET},				//Reset_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_RESETACK},			//ResetAck_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_BEATHEART},			//BeatHeart_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_BEATHEARTACK},		//BeatHeartAck_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_CONGESTREQ},			//CongestionCtrlReq_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_CONGESTRSP},			//CongestionCtrlRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_ERRNOTIFYREQ},		//ErrNotifyReq_MSG,
	{M_MSG_EVENT_GROUP_ID_MANAGE,	M_MSG_EVENT_ID_ERRNOTIFYRSP},		//ErrNotifyRsp_MSG,

	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Setup_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	CallProc_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Alerting_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Connect_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	ConnectAck_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Disconnect_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Release_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	ReleaseComplete_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	ModiMediaReq_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	ModiMediaRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Information_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	AuthCmdReq_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	AuthCmdRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	Login_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	LoginRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	Logout_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	HandOverReq_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	HandOverRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	HandOverComplete_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	StatusQry_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Status_MSG,
	
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	MOSmsDataReq_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	MOSmsDataRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	MTSmsDataReq_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	MTSmsDataRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	SMSMemAvailReq_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	SMSMemAvailRsp_MSG,
	
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	UTSAG_L3Addr_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	UTSAG_UID_MSG,

	{M_MSG_EVENT_GROUP_ID_BROADCAST,	M_MSG_EVENT_ID_BROADCAST_SM},				//BROADCAST_SM_MSG,
	{M_MSG_EVENT_GROUP_ID_BROADCAST,	M_MSG_EVENT_ID_BROADCAST_SM_ACK},			//BROADCAST_SM_ACK_MSG,
	{M_MSG_EVENT_GROUP_ID_BROADCAST,	M_MSG_EVENT_ID_REJECT},						//REJECT_MSG,
	{M_MSG_EVENT_GROUP_ID_BROADCAST,	M_MSG_EVENT_ID_RESET_REQ},					//RESET_REQ_MSG,
	{M_MSG_EVENT_GROUP_ID_BROADCAST,	M_MSG_EVENT_ID_RESTART_INDICATION},			//RESTART_INDICATION_MSG,
	{M_MSG_EVENT_GROUP_ID_BROADCAST,	M_MSG_EVENT_ID_FAIL_INDICATION},			//FAIL_INDICATION_MSG,

	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	CALLTICKET_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	CALLTICKET_ACK_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	SRQ_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	SRP_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	SRC_MSG,

	//以下为SAbis1.6新增信令，暂时没有加入与pm任务性能统计接口，但是已经实现了计数，20071103 by fengbing
	
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_AUTH_INFO_REQ},		//Auth_Info_Req_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_AUTH_CMD},			//Auth_Cmd_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_AUTH_RSP},			//Auth_Rsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_AUTH_RESULT},		//Auth_Result_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_BWINFO_REQ},			//BWInfo_Req_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_BWINFO_RSP},			//BWInfo_Rsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_BWINFO_DEL_REQ},		//BWInfo_Del_Req_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_BWINFO_MODIFY_REQ},	//BWInfo_Modify_Req_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_BWINFO_MODIFY_RSP},	//BWInfo_Modify_Rsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_SWITCH_OFF_NOTIFY},		//SWITCH_OFF_NOTIFY_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_ACCOUNT_LOGIN_REQ},		//AccountLoginReq_MSG,	//AccountLogin req	0x0B		0x05
	{M_MSG_EVENT_GROUP_ID_UTSAGOAM,	M_MSG_EVENT_ID_ConfigInfo_Transfer},	//ConfigInfo_Transfer_Msg,//ConfigInfo_Transfer	0x0C		0x05
	
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//		ChangePW_Req_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//		ChangePW_Rsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//		ChangePW_Ack_MSG,

	{M_MSG_EVENT_GROUP_ID_CPEOAM,	M_MSG_EVENT_ID_CPEOAM},	//CfgFuncInfo_Req_MSG,	//(0x53)		//ConfigFunc Info req	0x53	用户管理消息	UID寻址（OAM）
	{M_MSG_EVENT_GROUP_ID_CPEOAM,	M_MSG_EVENT_ID_CPEOAM},	//CfgFuncInfo_Rsp_MSG,	//(0x54)		//ConfigFunc Info rsp	0x54	用户管理消息	UID寻址（OAM）
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//VoiceInfo_Req_MSG,		//(0x55)		//VoiceInfo.req	0x55	用户管理消息	UID寻址（UTV）
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//VoiceInfo_Rsp_MSG,		//(0x56)		//VoiceInfo.rsp	0x56	用户管理消息	UID寻址（UTV）	
	{M_MSG_EVENT_GROUP_ID_RELAYMSG,	M_MSG_EVENT_ID_RELAYMSG},//Transdata_UL_MSG,	//		Transdata uplink	0x57	用户管理消息	UID寻址
	{M_MSG_EVENT_GROUP_ID_RELAYMSG,	M_MSG_EVENT_ID_RELAYMSG},//Transdata_DL_MSG,	//		Transdata downlink	0x58	用户管理消息	UID寻址
	{M_MSG_EVENT_GROUP_ID_RELAYMSG,	M_MSG_EVENT_ID_RELAYMSG},//OAMTransfer_Info_Req_MSG,	//		OAMTransfer_Info req	0x59	用户管理消息	UID寻址（OAMmessage）
	{M_MSG_EVENT_GROUP_ID_RELAYMSG,	M_MSG_EVENT_ID_RELAYMSG},//OAMTransfer_Info_Rsp_MSG,	//		OAMTransfer_Info rsp	0x5A	用户管理消息	UID寻址(OAMmessage)
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//SecuriryCardCallPara_Req_MSG,	//		SecuriryCardCallPara req	0x5B	用户管理消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//SecuriryCardCallPara_Rsp_MSG,	//		SecuriryCardCallPara rsp	0x5C	用户管理消息	L3地址寻址

//集群相关begin
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_LAGrpPaging},	//	LAGrpPagingReq_MSG,	//LA Group paging	0x05	呼叫控制消息	0x00
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_LAGrpPagingRsp},//	LAGrpPagingRsp_MSG,	//LA Group paging rsp	0x06	呼叫控制消息	0x00
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_LEGrpPaging},	//	LEGrpPaging_MSG,		//LE Group paging	0x07	呼叫控制消息	0x00
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DELEPaging},		//	DeLEGrpPaging_MSG,		//De-LE Group paging	0x08	呼叫控制消息	0x00
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_UserStatusReport},//	StatusReport_MSG,		//USER_STATUS_REPORT	0x09	呼叫控制消息	0x00
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_GrpHandoverReq},//	GrpHandoverReq_MSG,	//Group Handover req	0x0A	呼叫控制消息	0x00
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_GrpHandoverRsp},//	GrpHandoverRsp_MSG,	//Group Handover rsp	0x0B	呼叫控制消息	0x00	

	{M_MSG_EVENT_GROUP_ID_RESMANAGE,	M_MSG_EVENT_ID_RlsSMSLinkReq},//	RlsSmsLinkReq_MSG,		//Release Sms link req	0x05		0x01
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,	M_MSG_EVENT_ID_RlsSMSLinkRsp},//	RlsSmsLinkRsp_MSG,		//Release Sms link rsp	0x06		0x01
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,	M_MSG_EVENT_ID_GrpResReq},//	GrpResReq_MSG,			//Group Resouse req	0x07		0x01
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,	M_MSG_EVENT_ID_GrpResRsp},//	GrpResRsp_MSG,			//Group Resouse rsp	0x08		0x01

	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_GRP_UTSAG_L3ADDR},//	UTSXC_GRPL3Addr_MSG,	//组L3寻址的UE-SAG message	0x03		0x03
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_GRP_UTSAG_UID},//	UTSXC_GRPUID_MSG,		//组呼的UID寻址的UE-SAG message	0x04		0x03

	//组呼呼叫控制消息
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_SetupReq_MSG,		//PTT Setup Req	0x60	呼叫控制消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_SetupAck_MSG,		//PTT Setup ack	0x61	呼叫控制消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_Connect_MSG,		//PTT Connect	0x62	呼叫控制消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_ConnectAck_MSG,	//PTT Connect Ack	0x63	呼叫控制消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_GRP_UTSAG_L3ADDR},//	Grp_Disconnect_MSG,	//Group Disconnect	0x64	呼叫控制消息	组L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_GRP_UTSAG_L3ADDR},//	Grp_Release_MSG,		//Group Release	0x65	呼叫控制消息	组L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Group_CallingRls_MSG,	//Group Calling Release	0x66	呼叫控制消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	Group_CallingRlsComplete_MSG,	//Group Calling Release Complete	0x67	呼叫控制消息	L3地址寻址
	//移动性管理消息
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	Modify_GIDs_Req_MSG,	//ATTACH/DETACHGIDs Req	0x25	移动性管理消息	UID寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	Modify_GIDs_Rsp_MSG,	//ATTACH/DETACHGIDs Rsp	0x26	移动性管理消息	UID寻址
	//话权控制消息
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_GRP_UTSAG_UID},//	PTT_PressReq_MSG,		//PTT PRESS REQ	0x70	话权控制消息	组呼的UID寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_GRP_UTSAG_UID},//	PTT_PressRsp_MSG,		//PTT PRESS RSP	0x71	话权控制消息	组呼的UID寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_GRP_UTSAG_L3ADDR},//	PTT_Granted_MSG,		//PTT GRANTED	0x72	话权控制消息	组L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_Interrupt_MSG,		//PTT INTERRUPT	0x73	话权控制消息	L3寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_InterruptAck_MSG,	//PTT INTERRUPT ACK	0x74	话权控制消息	L3寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_Rls_MSG,			//PTT RELEASE	0x75	话权控制消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_RlsAck_MSG,		//PTT RELEASE ACK	0x76	话权控制消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	PTT_PressInfo_MSG,		//PTT PRESS INFO	0x77	话权控制消息	L3地址寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_GRP_UTSAG_UID},//	PTT_PressCancel_MSG,	//PTT PRESS CANCLE 	0x78	话权控制消息	组呼的UID寻址
	//话权申请省资源优化
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,	M_MSG_EVENT_ID_PttPressApplyReq},//	PTT_PressApplyReq_MSG,
	{M_MSG_EVENT_GROUP_ID_RESMANAGE,	M_MSG_EVENT_ID_PttPressApplyRsp},//	PTT_PressApplyRsp_MSG,	
	//增值业务消息
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	User_StatusQuery_MSG,	//USER STATUS QUERY	0x90	状态报告消息	L3寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	User_StatusRsp_MSG,	//USER_STATUS_RESPONSE	0x91	状态报告消息	L3寻址
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_L3ADDR},//	User_StatusNotify_MSG,	//USER_STATUS_NOTIFY	0x92	状态报告消息	L3寻址
	
//集群相关end		
//数据组播、单播begin
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DBGrpPagingReq},//	DB_GrpPaging_MSG,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DBGrpPagingRsp},//	DB_GrpPagingRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DBDataComplete},//	DB_DataComplete_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	DB_MODataReq_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	DB_MODataRsp_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	DB_RetransDataReq_MSG,
	{M_MSG_EVENT_GROUP_ID_UTSAG,	M_MSG_EVENT_ID_UTSAG_UID},//	DB_RetransDataRsp_MSG,
//数据组播、单播end
	{M_MSG_EVENT_GROUP_ID_CPEOAM,	M_MSG_EVENT_ID_CPEOAM},//	UTSAG_UID_OAM_MSG,
	{M_MSG_EVENT_GROUP_ID_CALLCTRL,	M_MSG_EVENT_ID_DVoiceConfigReq},//DVoiceConfigReg_MSG,	//DVoice Config
	{M_MSG_EVENT_GROUP_ID_RELAYMSG,	M_MSG_EVENT_ID_RELAYMSG},//UTSAG_RELAY_MSG,	//无需终端注册透传的消息
//同播begin
	{M_MSG_ENENT_GROUP_ID_BTSL2SAG,	M_MSG_ENENT_ID_BTSL2SXC_SYNC_BROADCAST},//BTSL2_SAG_MSG,
//同播end
	
	//InvalidSignal_MSG,
};

sigTableT	CMsg_Signal_VCR::sigTabEventID[M_MSG_EVENT_GROUP_NUM+1]=
{
//呼叫控制消息
	{0x13,
		{
			InvalidSignal_MSG,
			LAPaging_MSG,		//LA Paging				0x01	呼叫控制消息	0x00
			LAPagingRsp_MSG,		//LA Paging Response	0x02	呼叫控制消息	0x00
			DELAPagingReq_MSG,	//De-LA Paging			0x03	呼叫控制消息	0x00
			DELAPagingRsp_MSG,	//De-LA Paging Response	0x04		
			
			LAGrpPagingReq_MSG,	//LA Group paging	0x05	呼叫控制消息	0x00
			LAGrpPagingRsp_MSG,	//LA Group paging rsp	0x06	呼叫控制消息	0x00
			LEGrpPaging_MSG,		//LE Group paging	0x07	呼叫控制消息	0x00
			DeLEGrpPaging_MSG,		//De-LE Group paging	0x08	呼叫控制消息	0x00
			StatusReport_MSG,		//USER_STATUS_REPORT	0x09	呼叫控制消息	0x00
			GrpHandoverReq_MSG,	//Group Handover req	0x0A	呼叫控制消息	0x00
			GrpHandoverRsp_MSG,	//Group Handover rsp	0x0B	呼叫控制消息	0x00		
			
			InvalidSignal_MSG,	//reserved for Grout use
			InvalidSignal_MSG,	//reserved for Grout use
			InvalidSignal_MSG,	//reserved for Grout use
			InvalidSignal_MSG,	//reserved for Grout use
			DVoiceConfigReg_MSG,	//DVoice Config
			DB_GrpPaging_MSG,	//DB Group paging	0x11		
			DB_GrpPagingRsp_MSG,	//DB Group paging response	0x12	呼叫控制消息	0x00
			DB_DataComplete_MSG,	//DB data complete	0x13	呼叫控制消息	0x00
		}
	},
//资源管理消息
	{0x0A,
		{
			InvalidSignal_MSG,
			AssignResReq_MSG,	//Assignmengt transport resource req	0x01	0x01
			AssignResRsp_MSG,	//Assignmengt transport resource rsp	0x02	0x01
			RlsResReq_MSG,		//Release transport resource req		0x03	0x01
			RlsResRsp_MSG,		//Release transport resource rsp		0x04	0x01

			RlsSmsLinkReq_MSG,		//Release Sms link req	0x05		0x01
			RlsSmsLinkRsp_MSG,		//Release Sms link rsp	0x06		0x01
			GrpResReq_MSG,			//Group Resouse req	0x07		0x01
			GrpResRsp_MSG,			//Group Resouse rsp	0x08		0x01
			PTT_PressApplyReq_MSG,	//PTT PRESS APPLY REQ	EVENT GROUP ID=0x01，EVENT ID=0x09
			PTT_PressApplyRsp_MSG,	//PTT PRESS APPLY RSP	EVENT GROUP ID=0x01，EVENT ID=0x0a
		}
	},
//管理类消息
	{0x08,
		{
			InvalidSignal_MSG,
			Reset_MSG,				//Reset						0x01		0x02
			ResetAck_MSG,			//Reset ack					0x02		0x02
			ErrNotifyReq_MSG,		//Error notification req	0x03		0x02
			ErrNotifyRsp_MSG,		//Error notification rsp	0x04		0x02
			BeatHeart_MSG,			//beatheart					0x05		0x02
			BeatHeartAck_MSG,		//Beatheart ack				0x06		0x02
			CongestionCtrlReq_MSG,	//Congestion control req	0x07		0x02
			CongestionCtrlRsp_MSG,	//Congestion control req	0x08		0x02
		}
	},
//UE-SAG直接传输消息
	{0x04,	
		{
			InvalidSignal_MSG,
			UTSAG_L3Addr_MSG,	//L3地址寻址的UE-SAG message	0x01		0x03
			UTSAG_UID_MSG,		//UID寻址的UE-SAG message		0x02		0x03

			UTSXC_GRPL3Addr_MSG,	//组L3寻址的UE-SAG message	0x03		0x03
			UTSXC_GRPUID_MSG,		//组呼的UID寻址的UE-SAG message	0x04		0x03
		}
	},
//广播业务消息
	{0x06,	
		{
			InvalidSignal_MSG,
			BROADCAST_SM_MSG, 		
			BROADCAST_SM_ACK_MSG,				
			REJECT_MSG, 					
			RESET_REQ_MSG,   		    	
			RESTART_INDICATION_MSG, 		
			FAIL_INDICATION_MSG,	
		}
	},
//UT-SAG的OAM消息
	{0x0C,	
		{
			InvalidSignal_MSG,
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
			AccountLoginReq_MSG,
			ConfigInfo_Transfer_Msg,
		}
	},
//透传的CPE的OAM消息
	{0x01,	
		{
			InvalidSignal_MSG,
			UTSAG_UID_OAM_MSG,
		}
	},
//UID寻址的UE-SAG OAMmessage	0x01		0x07
	{0x01,	
		{
			InvalidSignal_MSG,
			UTSAG_RELAY_MSG,
		}
	},

//基站层二与SAG的消息	0x01	同播管理消息	0x08
	{0x01,	
		{
			InvalidSignal_MSG,
			BTSL2_SAG_MSG,
		}
	}
	
};

SignalType	CMsg_Signal_VCR::sigTabMsgType[M_MSGTYPE_MAX_VALUE+1]=
{
						//消息名称				消息类型	消息类别	注释
	InvalidSignal_MSG,		//0x00	reserved

//呼叫控制消息
	Setup_MSG,				//Setup					0x01	呼叫控制消息	L3地址寻址
	CallProc_MSG,			//Setup ack				0x02	呼叫控制消息	L3地址寻址
	Alerting_MSG,				//Alerting				0x03	呼叫控制消息	L3地址寻址
	Connect_MSG,			//Connect				0x04	呼叫控制消息	L3地址寻址
	ConnectAck_MSG,			//Connect Ack			0x05	呼叫控制消息	L3地址寻址
	Disconnect_MSG,			//Disconnect			0x06	呼叫控制消息	L3地址寻址
	Information_MSG,			//Information			0x07	呼叫控制消息	L3地址寻址
	Release_MSG,				//Release				0x08	呼叫控制消息	L3地址寻址
	ReleaseComplete_MSG,		//Release complete		0x09	呼叫控制消息	L3地址寻址
	ModiMediaReq_MSG,		//Modify media type req	0x0a	呼叫控制消息	L3地址寻址
	ModiMediaRsp_MSG,		//Modify media type rsp	0x0b	呼叫控制消息	L3地址寻址
	
	//0x0c--0x0f	reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	
//切换消息
	HandOverReq_MSG,		//Handover req		0x10	切换消息	L3地址寻址
	HandOverRsp_MSG,		//Handover rsp		0x11	切换消息	L3地址寻址
	HandOverComplete_MSG,	//Handover complete	0x12	切换消息	L3地址寻址

	//0x13--0x1f	reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG,

//移动性管理消息
	Login_MSG,				//Login req					0x20	移动性管理消息	UID寻址
	LoginRsp_MSG,			//Login rsp					0x21	移动性管理消息	UID寻址
	Logout_MSG,				//Logout					0x22	移动性管理消息	UID寻址
	AuthCmdReq_MSG,		//Authentication command	0x23	移动性管理消息	UID寻址
	AuthCmdRsp_MSG,			//Authentication rsp		0x24	移动性管理消息	UID寻址
	Modify_GIDs_Req_MSG,		//ATTACH/DETACHGIDs Req	0x25	移动性管理消息	UID寻址
	Modify_GIDs_Rsp_MSG,		//ATTACH/DETACHGIDs Rsp	0x26	移动性管理消息	UID寻址

	//0x27--0x2f	reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, 

//短消息
	MOSmsDataReq_MSG,		//MO Sms data req			0x30	短消息	UID寻址
	MOSmsDataRsp_MSG,		//MO Sms data rsp			0x31	短消息	UID寻址
	MTSmsDataReq_MSG,		//MT Sms data req			0x32	短消息	UID寻址
	MTSmsDataRsp_MSG,		//MT Sms data rsp			0x33	短消息	UID寻址
	SMSMemAvailReq_MSG,		//SMS memory available req	0x34	短消息	UID寻址
	SMSMemAvailRsp_MSG,		//SMS memory available rsp	0x35	短消息	UID寻址

	//0x36--0x3f	reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG,

//增值业务消息
	CALLTICKET_MSG,			//Call Ticket	0x40	增值业务消息	L3地址寻址
	CALLTICKET_ACK_MSG,	//Call Ticket Ack	0x41	增值业务消息	L3地址寻址
	SRQ_MSG,				//SRQ	0x42	增值业务消息	UID寻址
	SRP_MSG,				//SRP	0x43	增值业务消息	UID寻址
	SRC_MSG,				//SRC	0x44	增值业务消息	UID寻址

	//0x45--0x4f reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
						
//用户管理消息
	ChangePW_Req_MSG,		//Change PW req	0x50	用户管理消息	UID寻址
	ChangePW_Rsp_MSG,		//Change PW rsp	0x51	用户管理消息	UID寻址
	ChangePW_Ack_MSG,		//Change PW Ack	0x52	用户管理消息	UID寻址
	CfgFuncInfo_Req_MSG,		//(0x53)		//ConfigFunc Info req	0x53	用户管理消息	UID寻址（OAM）
	CfgFuncInfo_Rsp_MSG,		//(0x54)		//ConfigFunc Info rsp	0x54	用户管理消息	UID寻址（OAM）
	VoiceInfo_Req_MSG,		//(0x55)		//VoiceInfo.req	0x55	用户管理消息	UID寻址（UTV）
	VoiceInfo_Rsp_MSG,		//(0x56)		//VoiceInfo.rsp	0x56	用户管理消息	UID寻址（UTV）
	Transdata_UL_MSG,	//		Transdata uplink	0x57	用户管理消息	UID寻址
	Transdata_DL_MSG,	//		Transdata downlink	0x58	用户管理消息	UID寻址
	OAMTransfer_Info_Req_MSG,	//		OAMTransfer_Info req	0x59	用户管理消息	UID寻址（OAMmessage）
	OAMTransfer_Info_Rsp_MSG,	//		OAMTransfer_Info rsp	0x5A	用户管理消息	UID寻址(OAMmessage)
	SecuriryCardCallPara_Req_MSG,	//		SecuriryCardCallPara req	0x5B	用户管理消息	L3地址寻址
	SecuriryCardCallPara_Rsp_MSG,	//		SecuriryCardCallPara rsp	0x5C	用户管理消息	L3地址寻址

	//0x5d--0x5f reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,

//组呼呼叫控制消息
	PTT_SetupReq_MSG,		//PTT Setup Req	0x60	呼叫控制消息	L3地址寻址
	PTT_SetupAck_MSG,		//PTT Setup ack	0x61	呼叫控制消息	L3地址寻址
	PTT_Connect_MSG,		//PTT Connect	0x62	呼叫控制消息	L3地址寻址
	PTT_ConnectAck_MSG,		//PTT Connect Ack	0x63	呼叫控制消息	L3地址寻址
	Grp_Disconnect_MSG,		//Group Disconnect	0x64	呼叫控制消息	组L3地址寻址
	Grp_Release_MSG,			//Group Release	0x65	呼叫控制消息	组L3地址寻址
	Group_CallingRls_MSG,		//Group Calling Release	0x66	呼叫控制消息	L3地址寻址
	Group_CallingRlsComplete_MSG,	//Group Calling Release Complete	0x67	呼叫控制消息	L3地址寻址

	//0x68--0x6f reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	
//话权控制消息
	PTT_PressReq_MSG,		//PTT PRESS REQ	0x70	话权控制消息	组呼的UID寻址
	PTT_PressRsp_MSG,		//PTT PRESS RSP	0x71	话权控制消息	组呼的UID寻址
	PTT_Granted_MSG,			//PTT GRANTED	0x72	话权控制消息	组L3地址寻址
	PTT_Interrupt_MSG,		//PTT INTERRUPT	0x73	话权控制消息	L3寻址
	PTT_InterruptAck_MSG,		//PTT INTERRUPT ACK	0x74	话权控制消息	L3寻址
	PTT_Rls_MSG,				//PTT RELEASE	0x75	话权控制消息	L3地址寻址
	PTT_RlsAck_MSG,			//PTT RELEASE ACK	0x76	话权控制消息	L3地址寻址
	PTT_PressInfo_MSG,		//PTT PRESS INFO	0x77	话权控制消息	L3地址寻址
	PTT_PressCancel_MSG,		//PTT PRESS CANCLE 	0x78	话权控制消息	组呼的UID寻址

	//0x79--0x7f reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, 

	//0x80--0x8f reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,

//增值业务消息
	User_StatusQuery_MSG,	//USER STATUS QUERY	0x90	状态报告消息	L3寻址
	User_StatusRsp_MSG,		//USER_STATUS_RESPONSE	0x91	状态报告消息	L3寻址
	User_StatusNotify_MSG,		//USER_STATUS_NOTIFY	0x92	状态报告消息	L3寻址
	
	//0x93--0x9f reserved
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG, InvalidSignal_MSG,
	InvalidSignal_MSG,

//数据组播、广播消息
	DB_MODataReq_MSG,		//MO DB data req	0xA0		UID寻址
	DB_MODataRsp_MSG,		//MO DB data rsp	0xA1		UID寻址
	DB_RetransDataReq_MSG,		//RetranData req	0xA2		UID寻址
	DB_RetransDataRsp_MSG,		//RetranData rsp	0xA3		UID寻址

};

//信令统计表
UINT32	CMsg_Signal_VCR::RxSignalCounter[InvalidSignal_MSG+1]={0,};
UINT32	CMsg_Signal_VCR::TxSignalCounter[InvalidSignal_MSG+1]={0,};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*
 *	信令消息的创建，需要在信令数据区末尾多申请2个字节供tVCR任务填写PKT的结束标志
 *	PKT的开始标志和包头部分使用commessage的数据区前面的保留缓冲区
 */
bool CMsg_Signal_VCR::CreateMessage(CComEntity& Entity, UINT32 uDataSize)
{
	bool ret = CMessage::CreateMessage(Entity, uDataSize+sizeof(UINT16));
	if(ret)
		SetDataLength(uDataSize);	//设置信令数据长度
	return ret;
}

//信令消息的发送，先统计再发送
bool CMsg_Signal_VCR::Post()
{
	bool ret;
	ret = CMessage::Post();
#if 1 
	if(!ret)
	{
		DeleteMessage();
	}
#endif 
	return ret;
}

/*
 *	fill BTSID&SAGID for signal message, 
 *  called when constructing messages sending to tVCR
 */
void CMsg_Signal_VCR::SetBTSSAGID()
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)GetDataPtr();
	VSetU16BitVal(pData->sigHeader.BTS_ID, m_BTSID);
	VSetU32BitVal(pData->sigHeader.SAG_ID, m_SAGID);
}

/*
 *	分析得到从SAG接收消息的消息类型
 *	透传消息返回UTSAG_L3Addr_MSG or UTSAG_UID_MSG or UTSAG_UID_OAM_MSG
 *	非法消息返回InvalidSignal_MSG
 */
SignalType  CMsg_Signal_VCR::ParseMessageFromSAG()
{
	UINT16 msgID = GetMessageId();
	UINT16 msgLen = GetDataLength();
	VoiceVCRCtrlMsgT *pSigVCR;

	//消息ID检查
	if(MSGID_VCR_VOICE_SIGNAL!=msgID)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseMessageFromSAG(), msg is not signal from SAG!!!");
		return InvalidSignal_MSG;
	}
	//消息长度检查,检查VCR接口信令长度
	if(msgLen<sizeof(SigHeaderT)) 
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseMessageFromSAG(), VCR ctrl msgLen error!!!");
		return InvalidSignal_MSG;
	}
	
	pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();
	UINT8 eventGroutID = pSigVCR->sigHeader.EVENT_GROUP_ID;
	if(eventGroutID>M_MSG_EVENT_GROUP_NUM)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseMessageFromSAG(), invalid EventGroupID!!!");
		return InvalidSignal_MSG;
	}
	//net order to host oder
	UINT16 eventID = VGetU16BitVal(pSigVCR->sigHeader.Event_ID);
	if(eventID>sigTabEventID[eventGroutID].nMaxSigIndex)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseMessageFromSAG(), invalid EVENT_ID!!!");
		return InvalidSignal_MSG;
	}
	else
		return sigTabEventID[eventGroutID].sigTypeArr[eventID];
}

/*
 *	分析得到向SAG发送消息的消息类型
 *	透传消息返回UTSAG_L3Addr_MSG or UTSAG_UID_MSG or UTSAG_UID_OAM_MSG
 *	非法消息返回InvalidSignal_MSG
 */
SignalType CMsg_Signal_VCR::ParseMessageToSAG()
{
	UINT16 msgID = GetMessageId();
	UINT16 msgLen = GetDataLength();
	VoiceVCRCtrlMsgT *pSigVCR;
	
	//消息ID检查
	if(MSGID_VOICE_VCR_SIGNAL!=msgID)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseMessageToSAG(), msg is not signal to SAG!!!");
		return InvalidSignal_MSG;		
	}
	//消息长度检查,检查VCR接口信令长度
	if(msgLen<sizeof(SigHeaderT)) 
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseMessageToSAG(), VCR ctrl msgLen error!!!");
		return InvalidSignal_MSG;
	}
	
	pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();
	UINT8 eventGroupID = pSigVCR->sigHeader.EVENT_GROUP_ID;
	if(eventGroupID>M_MSG_EVENT_GROUP_NUM)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseMessageToSAG(), invalid EventGroupID!!!");
		return InvalidSignal_MSG;
	}
	//net order to host oder
	UINT16 eventID = VGetU16BitVal(pSigVCR->sigHeader.Event_ID);
	if(eventID>sigTabEventID[eventGroupID].nMaxSigIndex)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseMessageToSAG(), invalid EVENT_ID!!!");
		return InvalidSignal_MSG;
	}
	else
		return sigTabEventID[eventGroupID].sigTypeArr[eventID];
}

bool CMsg_Signal_VCR::isUTSAGMsg()
{
	//EVENT_ID & EVENT_GROUP_ID
	VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();
	UINT8 eventGrpID = pSigVCR->sigHeader.EVENT_GROUP_ID;
	UINT16 eventID = VGetU16BitVal(pSigVCR->sigHeader.Event_ID);	//net order to host oder
	if(M_MSG_EVENT_GROUP_ID_UTSAG==eventGrpID)
	{
		if(eventID<M_MSG_EVENT_ID_UTSAG_L3ADDR || M_MSG_EVENT_ID_GRP_UTSAG_UID<eventID)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		if(M_MSG_EVENT_GROUP_ID_CPEOAM==eventGrpID)
		{
			if(M_MSG_EVENT_ID_CPEOAM==eventID)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if(M_MSG_EVENT_GROUP_ID_RELAYMSG==eventGrpID)
			{
				if(M_MSG_EVENT_ID_RELAYMSG==eventID)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}	 
}

/*
 *	分析得到从SAG接收透传消息的消息类型
 */
SignalType	CMsg_Signal_VCR::ParseUTSAGMsgFromSAG()
{
	UINT16 msgID = GetMessageId();
	UINT16 msgLen = GetDataLength();
	VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();

	//消息ID
	if(MSGID_VCR_VOICE_SIGNAL!=msgID)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseUTSAGMsgFromSAG(), msg is not signal from SAG!!!");
		return InvalidSignal_MSG;		
	}	

	//消息长度应该不小于 信令通用头部长度＋UID/L3Addr长度＋MessageType长度
	if(msgLen<(sizeof(SigHeaderT)+sizeof(UINT32)+sizeof(UINT8)))
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseUTSAGMsgFromSAG(), msg len error!!!");
		return InvalidSignal_MSG;
	}

	//EVENT_ID & EVENT_GROUP_ID
	if(!isUTSAGMsg())
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseUTSAGMsgFromSAG(), invalid EVENT_ID or EVENT_GROUP_ID!!!");
		return InvalidSignal_MSG;
	}

	//MessageType
	UINT8 msgType = pSigVCR->sigPayload.UTSAG_Signal_XXX.msgType;
	if(msgType>M_MSGTYPE_MAX_VALUE)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseUTSAGMsgFromSAG(), invalid MessageType!!!");
		return InvalidSignal_MSG;		
	}
	return sigTabMsgType[msgType];
}

/*
 *	分析得到向SAG发送透传消息的消息类型
 */
SignalType	CMsg_Signal_VCR::ParseUTSAGMsgToSAG()
{
	UINT16 msgID = GetMessageId();
	UINT16 msgLen = GetDataLength();
	VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();

	//消息ID
	if(MSGID_VOICE_VCR_SIGNAL!=msgID)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseUTSAGMsgToSAG(), msg is not signal to SAG!!!");
		return InvalidSignal_MSG;		
	}	
	
	//消息长度应该不小于 信令通用头部长度＋UID/L3Addr长度＋MessageType长度
	if(msgLen<(sizeof(SigHeaderT)+sizeof(UINT32)+sizeof(UINT8)))
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseUTSAGMsgToSAG(), msg len error!!!");
		return InvalidSignal_MSG;
	}
	
	//EVENT_ID & EVENT_GROUP_ID
	if(!isUTSAGMsg())
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseUTSAGMsgToSAG(), invalid EVENT_ID or EVENT_GROUP_ID!!!");
		return InvalidSignal_MSG;
	}
	
	//MessageType
	UINT8 msgType = pSigVCR->sigPayload.UTSAG_Signal_XXX.msgType;
	if(msgType>M_MSGTYPE_MAX_VALUE)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "CMsg_Signal_VCR::ParseUTSAGMsgToSAG(), invalid MessageType!!!");
		return InvalidSignal_MSG;		
	}
	return sigTabMsgType[msgType];	
}

/*
 *	根据消息类型设置消息的EVENT GROUP ID(net order)和EVENT ID
 */
void CMsg_Signal_VCR::SetSigIDS(SignalType sigType)
{
	VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();
	pSigVCR->sigHeader.EVENT_GROUP_ID = m_SigIDS[sigType].EventGroupID;
	VSetU16BitVal(pSigVCR->sigHeader.Event_ID, m_SigIDS[sigType].EventID);
}

/*
 *	设置信令消息头的长度字段
 */
void CMsg_Signal_VCR::SetSigHeaderLengthField(UINT16 len)
{
	VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();
	VSetU16BitVal(pSigVCR->sigHeader.Length, len);
}

/*
 *	分析从SAG收到的信令消息是根据L3Addr还是UID来找CCB,只处理需要注入状态机的消息,for BTS
 *	return value: true when success, false when fail
 *	Parameters:
 *	how--output parameter,0:use UID,1:use L3Addr,2:need not find CCB
 *	Uid_L3addr--output paramter, when how==0, Uid_L3addr = UID, when how==1, Uid_L3addr = L3Addr
 */
bool CMsg_Signal_VCR::SAGSignalHowToFindCCB(UINT8& how, UINT32& Uid_L3addr)
{
	enum{ USE_UID, USE_L3ADDR, NEEDNOTFINDCCB };
	if(GetSrcTid()!=M_TID_VCR)
		return false;
	
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)GetDataPtr();

	SignalType sigType = ParseMessageFromSAG();
	switch(sigType) 
	{

	//================================================================
	case LAPaging_MSG:
		how = USE_UID;
		Uid_L3addr = VGetU32BitVal(pData->sigPayload.LAPaging.UID);
		break;
	case DELAPagingReq_MSG:
		how = USE_UID;
		Uid_L3addr = VGetU32BitVal(pData->sigPayload.DELAPagingReq.UID);
		break;
	//case LAPagingRsp_MSG:不可能收到
	//case DELAPagingRsp_MSG:不可能收到
	case AssignResReq_MSG:
		how = USE_UID;
		Uid_L3addr = VGetU32BitVal(pData->sigPayload.AssignResReq.UID);
		break;
	case AssignResRsp_MSG:
		how = USE_UID;
		Uid_L3addr = VGetU32BitVal(pData->sigPayload.AssignResRsp.UID);
		break;
	case RlsResReq_MSG:
		how = USE_L3ADDR;
		Uid_L3addr = VGetU32BitVal(pData->sigPayload.RlsResReq.L3addr);
		break;
	case RlsResRsp_MSG:
		how = USE_L3ADDR;
		Uid_L3addr = VGetU32BitVal(pData->sigPayload.RlsResRsp.L3addr);
		break;
	//case Reset_MSG:不注入状态机，不透传，所以不用找CCB
	//case ResetAck_MSG:不注入状态机，不透传，所以不用找CCB
	//case BeatHeart_MSG:不注入状态机，不透传，所以不用找CCB
	//case BeatHeartAck_MSG:不注入状态机，不透传，所以不用找CCB
	//case CongestionCtrlReq_MSG:不注入状态机，不透传，所以不用找CCB
	//case CongestionCtrlRsp_MSG:不注入状态机，不透传，所以不用找CCB
	case ErrNotifyReq_MSG:
		how = USE_UID;
		Uid_L3addr = VGetU32BitVal(pData->sigPayload.ErrNotifyReq.Uid);
		break;
	case ErrNotifyRsp_MSG:
		how = USE_UID;
		Uid_L3addr = VGetU32BitVal(pData->sigPayload.ErrNotifyRsp.Uid);
		break;

	//===========================================================
/*
	透传的消息不会进入本函数
	case UTSAG_L3Addr_MSG:
		how = USE_L3ADDR;
		Uid_L3addr = pData->sigPayload.UTSAG_Payload_L3Addr.L3Addr;
		break;
	case UTSAG_UID_MSG:
		how = USE_UID;
		Uid_L3addr = pData->sigPayload.UTSAG_Payload_Uid.Uid;
		break;
*/
	//===========================================================

	case InvalidSignal_MSG:
	default:
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), "invalid signal");
		how = NEEDNOTFINDCCB;
		Uid_L3addr = NO_L3ADDR;
		return false;
	}
	//Uid_L3addr = ntohl(Uid_L3addr);
	return true;
}

//清除信令统计计数器
void CMsg_Signal_VCR::ClearSignalCounters()
{
	for(int i=0;i<=InvalidSignal_MSG;i++)
		RxSignalCounter[i] = TxSignalCounter[i] = 0;
}

//接收信令统计
void CMsg_Signal_VCR::DoCountRxSignal()
{
	SignalType sigType = ParseMessageFromSAG();
	++RxSignalCounter[sigType];
	if(InvalidSignal_MSG!=sigType)
	{
		VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();
		if(M_MSG_EVENT_GROUP_ID_UTSAG==pSigVCR->sigHeader.EVENT_GROUP_ID || 
			M_MSG_EVENT_GROUP_ID_CPEOAM==pSigVCR->sigHeader.EVENT_GROUP_ID ||
			M_MSG_EVENT_GROUP_ID_RELAYMSG==pSigVCR->sigHeader.EVENT_GROUP_ID)
		{
			sigType = ParseUTSAGMsgFromSAG();
			++RxSignalCounter[sigType];
		}
	}
}

//发送信令统计
void CMsg_Signal_VCR::DoCountTxSignal()
{
	SignalType sigType = ParseMessageToSAG();
	++TxSignalCounter[sigType];
	if(InvalidSignal_MSG!=sigType)
	{
		VoiceVCRCtrlMsgT *pSigVCR = (VoiceVCRCtrlMsgT*)GetDataPtr();
		if(M_MSG_EVENT_GROUP_ID_UTSAG==pSigVCR->sigHeader.EVENT_GROUP_ID ||
			M_MSG_EVENT_GROUP_ID_CPEOAM==pSigVCR->sigHeader.EVENT_GROUP_ID ||
			M_MSG_EVENT_GROUP_ID_RELAYMSG==pSigVCR->sigHeader.EVENT_GROUP_ID)
		{
			sigType = ParseUTSAGMsgToSAG();
			++TxSignalCounter[sigType];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
UINT8 CMsg_UTSAGSignal_VAC::GetCID()
{
	VoiceVACCtrlMsgT* pData = (VoiceVACCtrlMsgT*)GetDataPtr();
	return pData->Cid;
}

void CMsg_UTSAGSignal_VAC::SetCID(UINT8 cid)
{
	VoiceVACCtrlMsgT* pData = (VoiceVACCtrlMsgT*)GetDataPtr();
	pData->Cid = cid;
}

//设置从MessageType开始的信令静荷，len为静荷长度
void CMsg_UTSAGSignal_VAC::SetSignalPayload(UINT8* pPayload, UINT16 len)
{
	VoiceVACCtrlMsgT* pData = (VoiceVACCtrlMsgT*)GetDataPtr();
	memcpy((void*)&pData->sigPayload, (void*)pPayload, len);
	SetPayloadLength(len+1);	//sigPayload长度+cid长度
}

//得到从MessageType开始的信令静荷，len为静荷长度
void CMsg_UTSAGSignal_VAC::GetSignalPayload(UINT8*& pPayload, UINT16& len)
{
	VoiceVACCtrlMsgT* pData = (VoiceVACCtrlMsgT*)GetDataPtr();
	pPayload = (UINT8*)&pData->sigPayload;
	len = GetDataLength()-1;	//消息长度-cid长度
}

bool CMsg_UTSAGSignal_VAC::Post()
{
	bool ret =  CMessage::Post();
	if(!ret)
		DeleteMessage();
	return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
UINT8 CMsg_UTSAGSignal_DAC::GetCID()
{
	VoiceDACCtrlMsgT* pData = (VoiceDACCtrlMsgT*)GetDataPtr();
	return pData->Cid;
}

void CMsg_UTSAGSignal_DAC::SetCID(UINT8 cid)
{
	VoiceDACCtrlMsgT* pData = (VoiceDACCtrlMsgT*)GetDataPtr();
	pData->Cid = cid;
}

//设置从MessageType开始的信令静荷，len为静荷长度
void CMsg_UTSAGSignal_DAC::SetSignalPayload(UINT8* pPayload, UINT16 len)
{
	VoiceDACCtrlMsgT* pData = (VoiceDACCtrlMsgT*)GetDataPtr();
	memcpy((void*)&pData->sigPayload, (void*)pPayload, len);
	SetPayloadLength(len+1);	//sigPayload长度+cid长度
}

//得到从MessageType开始的信令静荷，len为静荷长度
void CMsg_UTSAGSignal_DAC::GetSignalPayload(UINT8*& pPayload, UINT16& len)
{
	VoiceDACCtrlMsgT* pData = (VoiceDACCtrlMsgT*)GetDataPtr();
	pPayload = (UINT8*)&pData->sigPayload;
	len = GetDataLength()-1;	//消息长度-cid长度
}

bool CMsg_UTSAGSignal_DAC::Post()
{
	bool ret =  CMessage::Post();
	if(!ret)
		DeleteMessage();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void GetVoicePerfData(UINT8 *pData)
{
//	struct T_L3VOICEPerfData
//	{
//		T_L3VOICEPerfDataEle  RxPerformanceDATA;
//		T_L3VOICEPerfDataEle  TxPerformanceDATA;
//	};

	if(pData!=NULL)
	{
#if 0
		//Rx
		memcpy( (void*)pData,
				(void*)CMsg_Signal_VCR::RxSignalCounter,
				sizeof(CMsg_Signal_VCR::RxSignalCounter) );
		//Tx
		memcpy( (void*)(pData + sizeof(CMsg_Signal_VCR::RxSignalCounter)),
			(void*)CMsg_Signal_VCR::TxSignalCounter,
			sizeof(CMsg_Signal_VCR::TxSignalCounter) );
#endif
		//Rx
		memcpy( (void*)pData,
				(void*)CMsg_Signal_VCR::RxSignalCounter,
				(MAX_PERF_COUNTER_SIGNAL+1)*sizeof(UINT32) );
		memcpy( (void*)(pData+(MAX_PERF_COUNTER_SIGNAL+1)*sizeof(UINT32)), 
		        (void*)&CMsg_Signal_VCR::RxSignalCounter[InvalidSignal_MSG], 
		        sizeof(UINT32) );
		//Tx
		UINT8* pDataTx = pData + (MAX_PERF_COUNTER_SIGNAL+2)*sizeof(UINT32);
		memcpy( (void*)pDataTx,
				(void*)CMsg_Signal_VCR::TxSignalCounter,
				(MAX_PERF_COUNTER_SIGNAL+1)*sizeof(UINT32) );
		memcpy( (void*)(pDataTx+(MAX_PERF_COUNTER_SIGNAL+1)*sizeof(UINT32)), 
		        (void*)&CMsg_Signal_VCR::TxSignalCounter[InvalidSignal_MSG], 
		        sizeof(UINT32) );	
		
			
	}
}




