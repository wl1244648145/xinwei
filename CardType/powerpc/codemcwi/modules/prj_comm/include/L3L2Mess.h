/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3L2MESSAGEID
#define _INC_L3L2MESSAGEID

#ifndef DATATYPE_H
#include "datatype.h"
#endif

//////////////////////////////////////////////////////////////////////////
// 定义所有<<IF-BTS-0005-L2 and L3 Interface.doc>>消息 MessageId
// 0X3000 ----0X4FFF定义BTS任务间( L3--L2)的消息：
// 0X3000 ―― 0X33FF：配置管理部分
// 0X3400 ―― 0X34FF：故障管理部分
// 0X3500 ―― 0X35FF：性能管理部分
// 0X3600 ―― 0X36FF：系统管理部分
// 0X3700 ―― 0X37FF：CPE管理部分
// 0X3800 ―― 0X38FF：诊断管理部分
// 0X3900 ―― 0x39FF：数据服务部分
// 0X3A00 ―― 0X3AFF：Voice部分
//////////////////////////////////////////////////////////////////////////
//0X3000 ―― 0X33FF：配置管理部分
const UINT16 M_L3_L2_AIRLINK_DATA_CFG_REQ      = 0X3001;
const UINT16 M_L2_L3_AIRLINK_DATA_CFG_RSP	   = 0X3002;
const UINT16 M_L3_L2_RES_MANAGE_POLICY_REQ     = 0X3004;
const UINT16 M_L2_L3_RES_MANAGE_POLICY_RSP     = 0X3005;
const UINT16 M_L3_L2_AIR_LINK_MISC_CFG_REQ     = 0X3007; 
const UINT16 M_L2_L3_AIR_LINK_MISC_CFG_RSP     = 0X3008;
const UINT16 M_L3_L2_BILLING_DATA_CFG_REQ      = 0X300A;
const UINT16 M_L2_L3_BILLING_DATA_CFG_RSP	   = 0X300B;
const UINT16 M_L3_L2_L1_GENERAL_SETTING_REQ    = 0x3011;
const UINT16 M_L2_L3_L1_GENERAL_SETTING_RSP    = 0X3012;
const UINT16 M_L3_L2_RF_CFG_REQ                = 0x3014;
const UINT16 M_L2_L3_RF_CFG_RSP	               = 0X3015;
const UINT16 M_L3_L2_CALIBRAT_CFG_GENDATA_REQ  = 0x3017;
const UINT16 M_L2_L3_CALIBRAT_CFG_GENDATA_RSP  = 0X3018;
const UINT16 M_L3_L2_CALIBRAT_CFG_DATA_REQ     = 0x301A;
const UINT16 M_L2_L3_CALIBRAT_CFG_DATA_RSP     = 0X301B;
const UINT16 M_L3_L2_INSTANT_CALIBRATION_REQ   = 0X301D;
const UINT16 M_L2_L3_INSTANT_CALIBRATION_RSP   = 0X301E;
const UINT16 M_L2_L3_CAL_GENDATA_NOTIFY        = 0x3021;
const UINT16 M_L2_L3_CAL_DATA_NOTIFY           = 0x3022;
const UINT16 M_L3_L2_BTSRSTCNT_CHANGE_NOTIFY   = 0X3023;  
const UINT16 M_L3_L2_GET_BOARDS_STATE_REQ      = 0X3024;
const UINT16 M_L2_L3_GET_BOARDS_STATE_RSP      = 0X3025;
const UINT16 M_L3_L2_GET_BTS_INFO_REQ          = 0X3026;
const UINT16 M_L2_L3_GET_BTS_INFO_RSP          = 0X3027;
const UINT16 M_L2_L3_SYNCBRD_TEMPERATUR_NOTIFY = 0X3028;
const UINT16 M_L3_L2_GET_RF_DATA_REQ           = 0X3029;
const UINT16 M_L2_L3_GET_RF_DATA_RSP           = 0X302A;
const UINT16 M_L3_L2_CFG_SYN_POWER_REQ         = 0X302B;
const UINT16 M_L2_L3_CFG_SYN_POWER_RSP         = 0X302C;
const UINT16 M_L3_L2_CFG_WANIF_CPE_REQ         = 0x302D;
const UINT16 M_L2_L3_CFG_WANIF_CPE_RSP         = 0x302E;

const UINT16 M_L3_L2_CFG_SYN_GPS_REQ           = 0x3030;

const UINT16 MSGID_L3_L2_VACPREFSCG_CFG = 0x303A;
const UINT16 MSGID_L2_L3_VACPREFSCG_RSP = 0x303B;

const UINT16 M_L2_L3_SAVEPOWER_CPU_REQ  = 0x303C;
const UINT16 M_L3_L2_SAVEPOWER_CPU_RSP  = 0X303D;
const UINT16 MSGID_L3_L2_SAVEPWR_CFG_REQ = 0X303E;
const UINT16 MSGID_L2_L3_SAVEPWR_CFG_RSP = 0X303F;
const UINT16 M_L3_L2_SAVEPOWER_FAN_REQ  = 0x3040;
const UINT16 M_L2_L3_SAVEPOWER_FAN_RSP  = 0X3041;
const UINT16 MSGID_L3_L2_QAM64_CFG = 0x3042;
const UINT16 MSGID_L2_L3_QAM64_RSP = 0x3043;
//#ifdef LJF_WAKEUPCONFIG
const UINT16 MSGID_L3_L2_WAKEUPCONFIG = 0x3046;

const UINT16 MSGID_L3_L2_PAYLOAD_BALANCE_CFG = 0x3B16;
const UINT16 MSGID_L2_L3_PAYLOAD_BALANCE_RSP = 0x3B17;

/////////////////////////L3
const UINT16 M_L3_L2_TEMP_MONITOR_REQ          = 0x3037;
const UINT16 M_L2_L3_TEMP_MONITOR_RSP          = 0X3038;
const UINT16 M_L3_L2_UT_DEFAULT_SER_DISCR_REQ  = 0x3031;
const UINT16 M_L2_L3_UT_DEFAULT_SER_DISCR_RSP  = 0X3032;

#ifdef LOCATION_2ND
#define M_L3_L2_LOCATION_DOA_REQ      0X3044
#define M_L2_L3_LOCATION_DOA_RSP      0X3045
#endif
const UINT16 M_L3_L2_N1_PARAMETER_REQ        = 0x3100;
const UINT16 M_L2_L3_N1_PARAMETER_RSP        = 0x3101;

const UINT16 MSGID_L3_L2_GRP_LIMIT_CFG = 0x3106;
const UINT16 MSGID_L2_L3_GRP_LIMIT_CFG_RSP = 0x3107;

#ifdef NUCLEAR_CODE
const UINT16 MSGID_LIMITAREA_L2L3_CFG_REQ          = 0x3B18;
const UINT16 MSGID_LIMITAREA_L2L3_CFG_RSP          = 0x3B19;
#endif

// 0X3400 ―― 0X34FF：故障管理部分
const UINT16 M_L3_L2_SYSPROBING_REQ            = 0X3401;
const UINT16 M_L2_L3_SYSPROBING_RSP            = 0X3402;
const UINT16 M_L2_L3_RFSTATE_NOTIFY            = 0X3404;
const UINT16 M_L2_L3_MCPSTATE_NOTIFY           = 0X3405; 
const UINT16 M_L2_L3_AUXSTATE_NOTIFY           = 0X3406;
const UINT16 M_L3_L2_CFG_ANTENNA_NOTIFY        = 0X3407;
const UINT16 M_L3_L2_RESET_CPU_NOTIFY          = 0X3408;
#ifdef WBBU_CODE
const UINT16 M_L2_CORE9_STATUS_NOTIFY         = 0x3409;
const UINT16 M_L2_AIF_STATAS_NOTIFY                   = 0x340a;
const  UINT16  M_L3_L2_SYSPROBING_REQ_core1         = 0x340b;
const  UINT16  M_L3_L2_SYSPROBING_REQ_core1_rsp         = 0x340c;
#endif
// 0X3500 ―― 0X35FF：性能管理部分 TBD
// 0X3600 ―― 0X36FF：系统管理部分
const UINT16 M_L2_L3_POWERPC_BOOTUP_NOTIFY     = 0X3601;
const UINT16 M_L2_L3_CODE_DL_NOTIFY            = 0X3604;
const UINT16 M_L2_L3_CODE_DL_COMPLETE_NOTIFY   = 0X3607;
const UINT16 M_L2_L3_TIMEDAY_NOTIFY            = 0X360A;

// 0X3700 ―― 0X37FF：CPE管理部分
const UINT16 M_L2_L3_CPE_PROFILE_UPDATE_NOTIFY = 0X3701;
const UINT16 M_L3_L2_CPE_PROFILE_UPDATE_NOTIFY = 0X3704;
const UINT16 M_L3_L2_CPE_PROFILE_DELETE_NOTIFY = 0X3705;
const UINT16 M_L2_L3_BTS_LOADINFO_NOTIFY       = 0X3706;
const UINT16 M_L3_L2_BTS_UT_STATUS_DATA_REQ       = 0X3707;
const UINT16 M_L2_L3_BTS_UT_STATUS_DATA_RSP       = 0X3708;

// 0X3800 ―― 0X38FF：诊断管理部分 TBD
#define MSGID_DIAG_SHELL_CMD                0x3800
#define MSGID_DIAG_MEMORY_PEEK_CMD          0x3801
#define MSGID_DIAG_MEMORY_PEEK_RESPONSE     0x3802
#define MSGID_DIAG_MEMORY_POKE_CMD          0x3804
#define MSGID_DIAG_MEMORY_POKE_RESPONSE     0x3805
#define MSGID_DIAG_SW_VER_QUERY             0x3807
#define MSGID_DIAG_SW_VER_QUERY_RESPONSE    0x3808
#define MSGID_DIAG_STATUS_QUERY             0x3809
#define MSGID_DIAG_STATUS_QUERY_RESPONSE    0x380A
#define MSGID_DIAG_CPU_PING                 0x380B
#define MSGID_DIAG_CPU_PING_RESPONSE        0x380C
#define MSGID_DIAG_RPC_CMD                  0x380D
#define MSGID_DIAG_RPC_RESPONSE             0x380E
#define MSGID_DIAG_INFO_DISPLAY             0x380F
#ifdef WBBU_CODE

#define MSGID_DIAG_SHELL_CMD_MAC                0x38FF
#define MSGID_DIAG_COMMAND                    0x3810
#define MSGID_DIAG_PRINT                          0x3811
#endif
// 0X3900 ―― 0x39FF：数据服务部分 TBD
// 0X3A00 ―― 0X3AFF：语音相关部分 TBD
/* tVoice和VAC之间消息 */
#define	MSGID_VAC_SETUP_CMD		(0x3A01)	//Setup VAC Session setup Command
#define	MSGID_VAC_SETUP_RSP		(0x3A02)	//Setup VAC Session setup Response
#define	MSGID_VAC_SETUP_NOTIFY	(0x3A03)	//VAC Session Setup Notification
#define	MSGID_VAC_RLS_CMD		(0x3A04)	//VAC Session Release Command
#define	MSGID_VAC_RLS_NOTIFY	(0x3A05)	//VAC Session Release Notification
#define	MSGID_VAC_MODIFY_CMD	(0x3A06)	//VAC session Modify Command
#define	MSGID_VAC_MODIFY_RSP	(0x3A07)	//VAC session Modify Response
#define	MSGID_VAC_MODIFY_NOTIFY	(0x3A08)	//VAC session Modify Notification

#define MSGID_VAC_START			(0x3A09)	//VAC session start transfer, CPE only
#define MSGID_VAC_STOP			(0x3A0A)	//VAC session stop transfer, CPE only
#define MSGID_L3_VAC_SETUPRSP	(0x3A0B)	//L3-->VAC, VAC setup result

#define	MSGID_VAC_VOICE_DATA	(0x3A10)	//Uplink Voice Data Message
#define	MSGID_VOICE_VAC_DATA	(0x3A11)	//Downlink Voice Data Message

#define	MSGID_VAC_VOICE_SIGNAL	(0x3A12)	//uplink Voice Control Message, VAC-->tVoice
#define	MSGID_VOICE_VAC_SIGNAL	(0x3A13)	//downlink Voice Control Message, tVoice-->VAC

/* tVoice和L2之间 */
#define MSGID_VOICE_MAC_PROBEREQ (0x3A20)	//probe req,paging only
#define MSGID_MAC_VOICE_PROBERSP (0x3A21)	//probe rsp,
#define MSGID_CONGESTION_REQUEST (0x3A22)	//congestion request to L2
#define MSGID_CONGESTION_REPORT	 (0x3A23)	//congestion report, answer of congestion request from L2
#define MSGID_SAGSTATUS_REPORT	(0x3A24)	//SAG status,0:disconnected;1:connected

#define MSGID_FAX_DATA_UL (0x2311)
#define MSGID_FAX_DATA_DL (0x2312)

//集群相关L2<-->L3
#define MSGID_GRP_L2L3_SETUP_IND 					(0x3C00)//Group_Setup.indication	0X3C00
#define MSGID_GRP_L2L3_SETUP_RSP 					(0x3C01)//Group_Setup.response	0X3C01
#define MSGID_GRP_L2L3_RES_REQ 					(0x3C02)//Group_Resource.request	0X3C02
#define MSGID_GRP_L2L3_RES_RSP 					(0x3C03)//Group_Resource.confirmation	0X3C03
#define MSGID_GRP_L2L3_RES_IND 					(0x3C04)//Group_Resource.indication	0X3C04
#define MSGID_GRP_L2L3_RES_ACK 					(0x3C05)//Group_Resource.response	0X3C05
#define MSGID_GRP_L2L3_SIGNAL_REQ 				(0x3C06)//Group_Signal.request	0X3C06
#define MSGID_GRP_L2L3_SIGNAL_IND 				(0x3C07)//Group_Signal. indication	0X3C07
#define MSGID_GRP_L2L3_PAGING_REQ 				(0x3C08)//Group_Paging.request	0X3C08
#define MSGID_GRP_L2L3_PAGING_RSP 				(0x3C09)//Group_Paging.response	0X3C09
#define MSGID_GRP_L2L3_L2_STATUS_REPORT 			(0x3C0A)//Group_Status_report.request	0X3C0A
#define MSGID_GRP_L2L3_CPE_STATUS_REPORT_IND 	(0x3C0B)//Group_Status_report.indication	0X3C0B
#define MSGID_GRP_L2L3_LINK_QUALITY_IND 			(0x3C0C)//Group_Link_Quality.indication	0X3C0C
#define MSGID_GRP_L2L3_MAC_RLS_REQ 				(0x3C0D)//Group_Mac_Session_Rls.request	0X3C0D
#define MSGID_GRP_L2L3_MAC_RLS_RSP 				(0x3C0E)//Group_Mac_Session_Rls.confirmation	0X3C0E
#define MSGID_GRP_L2L3_REPORT_ID_IND 				(0x3C0F)//Group_Report_Index.indication	0X3C0F
#define MSGID_GRP_L2L3_VAC_RLS_RSP 				(0x3C10)//Group_Vac_Session_Rls.confirmation	0X3C10

#define MSGID_GRP_L2L3_CPE_PTT_PRESS_REQ			(0x3C12)//cpe_ptt_press_req	0X3C12	cpe L3->L2
#define MSGID_GRP_L2L3_CPE_PTT_PRESS_RSP			(0x3C13)//cpe_ptt_press_rsp	0X3C13	cpe L2->L3
#define MSGID_GRP_L2L3_CPE_PTT_PRESS_CANCEL		(0x3C14)//cpe_ptt_press_cancel.	0X3C14	cpe L3->L2
#define MSGID_GRP_L2L3_CPE_PTT_RELEASE			(0x3C15)//cpe_ptt_release	0X3C15	cpe L3->L2
#define MSGID_GRP_L2L3_CPE_PTT_CONFIRM			(0x3C16)//cpe_ptt_confirm	0X3C16	cpe L3->L2
#define MSGID_GRP_L2L3_BTS_PTT_PRESS_REQ			(0x3C17)//bts_ptt_press_req	0X3C17	bts L2->L3
#define MSGID_GRP_L2L3_BTS_PTT_PRESS_RSP			(0x3C18)//bts_ptt_press_rsp	0X3C18	bts L3->L2
#define MSGID_GRP_L2L3_BTS_PTT_PRESS_CANCEL		(0x3C19)//bts_ptt_press_cancel.	0X3C19	bts L2->L3
#define MSGID_GRP_L2L3_BTS_PTT_PRESS_RELEASE	(0x3C1A)//bts_ptt_press_release	0X3C1a	bts L2->L3

/*远距离Ranging配置*/
#define MSGID_L3_L2_REMOTE_RANGE_CFG 0x3b14
#define MSGID_L2_L3_REMOTE_RANGE_RSP 0x3b15
#ifdef WBBU_CODE
#define MSGID_L2_L3_LOAD_CFG                0x4003
#endif
/* tVoice和DAC之间 */
#define MSGID_DAC_VOICE_SIGNAL	(0x6800)
#define MSGID_VOICE_DAC_SIGNAL	(0x6801)
//集群相关走DAC的消息
#define MSGID_GRP_DAC_UL_L3_SIGNAL 	(0x6802)//上行组L3寻址的UE-SAG message	0x6802	UTV	TVOICE
#define MSGID_GRP_DAC_DL_L3_SIGNAL 	(0x6803)//下行组L3寻址的UE-SAG message	0x6803	TVOICE	UTV
#define MSGID_GRP_DAC_UL_UID_SIGNAL 	(0x6804)//上行组呼的UID寻址的UE-SAG message	0x6804	UTV	TVOICE
#define MSGID_GRP_DAC_DL_UID_SIGNAL 	(0x6805)//下行组呼的UID寻址的UE-SAG message	0x6805	TVOICE	UTV
#define MSGID_GRP_DAC_HO_RES_REQ 	(0x6806)//Ho_Resource.request	0x6806	CPECM	TVOICE
#define MSGID_GRP_DAC_HO_RES_RSP 		(0x6807)//Ho_Resource.response	0x6807	TVOICE	CPECM
#define MSGID_GRP_DAC_RES_REQ 		(0x6808)//GroupShare_Resource.Request	0x6808	UTV	TVOICE
#define MSGID_GRP_DAC_RES_RSP 		(0x6809)//GroupShare_Resource.Response	0x6809	TVOICE	UTV
#define MSGID_GRP_DAC_PTT_RES_REQ	(0x680C)//Ptt Press Req(DAC)
#define MSGID_GRP_DAC_PTT_RES_RSP	(0x680D)//Ptt Press Rsp(DAC)
#define MSGID_GRP_DAC_CALL_IND 		(0x3A30)//Group_Call_Indication	0x3A30	TVOICE	UTV

/* CPE */
#define	MSGID_VAC_UPLINK_SIGNAL		(0x3A12)	//uplink Voice Control Message, tUTV-->VAC
#define	MSGID_VAC_DOWNLINK_SIGNAL	(0x3A13)	//downlink Voice Control Message, VAC-->tUTV
			
#define MSGID_DAC_UPLINK_SINGAL		(0x6800)	//Uplink Voice Control Message to L3Voice		0x6800	TID_VOICE	TID_UTV
#define MSGID_DAC_DOWNLINK_SIGNAL	(0x6801)	//Downlink voice Control Message to L3Voice		0x6801	TID_UTV	TID_VOICE		

#define MSGID_DAC_UL_CPEOAM_MAG		(0x680A)	//上行的透传oam消息
#define MSGID_DAC_DL_CPEOAM_MSG		(0x680B)	//下行的透传oam消息
#define MSGID_DAC_UL_CPE_TELNO_MSG	(0x680E)	//上行cpe向bts报告电话号码的消息


#define MSGID_DB_DCS_UPLINK_SIGNAL		(0x680F)
#define MSGID_DB_DCS_DOWNLINK_SIGNAL	(0x6810)
#define MSGID_DB_DATA_UPLINK				(0x6811)
#define MSGID_DB_DATA_DOWNLINK			(0x6812)

#define MSGID_CPE_DCS_UPLINK_SIGNAL		(0x680F) //cpe-dcs上行透传信令
#define MSGID_DCS_CPE_DOWNLINK_SIGNAL	(0x6810) //dcs-cpe下行透传信令
#define MSGID_CPE_DCS_DATA_UPLINK		(0x6811) //cpe-dcs上行透传数据
#define MSGID_DCS_CPE_DATA_DOWNLINK		(0x6812) //dcs-cpe下行透传数据

// 0X8000 ----0X8FFF Relay Message ID；
#define MSGID_HI_PRIORITY_UNICAST_OAM               (0x8000)
#define MSGID_LOW_PRIORITY_UNICAST_OAM               (0x8001)
#define MSGID_BROADCAST_OAM                         (0x8F00)
#define MSGID_BROADCAST_TRAFFIC_TO_UT               (0x8f01)
#define MSGID_HIGH_PRIORITY_TRAFFIC                 (0x8010)
#define MSGID_LOW_PRIORITY_TRAFFIC                  (0x8011)
#define MSGID_REALTIME_TRAFFIC                      (0x8012)
#define MSGID_VIDEO_DATA_TRAFFIC   (0x8020)  //lijinan 20101020
#define MSGID_VIDEO_DATA								(0x8020)

#ifdef M_CLUSTER_EN
#define MSGID_GROUP_L3_UPLINK_SIGNAL	(0x6802)//上行组L3寻址消息
#define MSGID_GROUP_L3_DOWNLINK_SIGNAL	(0x6803)//下行组L3寻址消息
#define MSGID_GROUP_UID_UPLINK_SIGNAL	(0x6804)//上行组UID寻址消息
#define MSGID_GROUP_UID_DOWNLINK_SIGNAL	(0x6805)//下行组UID寻址消息
#define MSGID_HO_RESOURCE_REQ				(0x6806)
#define MSGID_HO_RESOURCE_RSP				(0x6807)
#define MSGID_GROUP_RESOURCE_REQ			(0x6808)
#define MSGID_GROUP_RESOURCE_RSP			(0x6809)

#define MSGID_GROUP_RESRC_IND		(0x3C04)
#define MSGID_GROUP_SIGNAL_IND		(0x3C07)
#define MSGID_LINK_QUALITY_IND		(0x3C0C)
#define MSGID_MAC_SESSION_RLS_REQ		(0x3C0D)
#define MSGID_MAC_SESSION_RLS_CONFIRM	(0x3C0E)
#define MSGID_GROUP_STAUS_REPORT_REQ	(0x3C0A)
#define MSGID_GROUP_REPORT_INDAX_IND		(0x3C0F)
#define MSGID_VAC_SESSION_RLS_CONFIRM	(0x3C10)
#define MSGID_GROUP_INFO_IND				(0x3C11)

#define MSGID_GROUP_SETUP_IND		(0x3C00)	
#define MSGID_GROUP_SETUP_RSP		(0x3C01)

#define MSGID_PTT_PRESS_REQ_IND	(0x3C12)
#define MSGID_PTT_PRESS_RSP_IND	(0x3C13)
#define MSGID_PTT_PRESS_CANCEL_IND	(0x3C14)
#define MSGID_PTT_PRESS_RLS_IND	(0x3C15)
#define MSGID_PTT_CONFIRM	(0x3C16)
#define PTT_AGREE_CONFIRM	(0)
#define PTT_QUEUE_CONFIRM	(1)
#define PTT_TIMEOUT_CONFIRM (2)
			//组呼寻呼指示
#define MSGID_GROUP_PAGING_REQ_ACTIVE	(0x3A30)	//数据业务期间激活组呼的寻呼消息
#endif
#define MSGID_GROUP_SCGINFO_IND		(0x3C1B)
#define MSGID_GROUP_SCGINFO_RSP		(0x3C1C)
#ifdef DATA_BROADCAST_EN
#define MSGID_GROUP_DB_SETUP_IND		(0x3C1D)	
#define MSGID_GROUP_DB_SETUP_RSP		(0x3C1E)
#endif
#endif 

