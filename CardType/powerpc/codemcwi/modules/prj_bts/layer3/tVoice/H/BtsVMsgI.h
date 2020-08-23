/*******************************************************************************
* Copyright (c) 2009 by Beijing Arrowping Communication Co.Ltd.All Rights Reserved   
* File Name      : BtsVMsgId.h
* Create Date    : 27-Apr-2011
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/

#ifndef	__BtsVMsgId_H
#define	__BtsVMsgId_H

#include "L3OAMMessageId.h"



#ifdef __cplusplus
extern "C" {
#endif


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

#define MSGID_CPE_UL_RELAY_MSG	(0x6813)//OAMTransfer_Info req	0x6813
#define MSGID_CPE_DL_RELAY_MSG	(0x6814)//OAMTransfer_Info rsp	0x6814


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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#define MSGID_VAC_SESSION_BEGIN	MSGID_VAC_SETUP_CMD
#define MSGID_VAC_SESSION_END	MSGID_VAC_MODIFY_NOTIFY

/* tVoice和tVCR之间消息 */
#define	MSGID_VCR_VOICE_SIGNAL	(0x2300)	//tVCR-->tVoice signal
#define	MSGID_VOICE_VCR_SIGNAL	(0x2301)	//tVoice-->tVCR	signal
#define	MSGID_VOICE_VCR_RESETLINK (0x2302)
#define	MSGID_VCR_VOICE_FORCE_UT_REGISTER	(0x2303)
#define	MSGID_VCR_VOICE_RELEASE_VOICE_SRV	(0x2304)
#define	MSGID_VOICE_DCS_MSG (0x2305)
#define	MSGID_DCS_VOICE_MSG (0x2306)
#define	MSGID_VOICE_DCS_RESETLINK	(0x2307)
#define	MSGID_DGRPLINK_VOICE_RELEASE_DGRP_SRV (0x2308)
#define MSGID_VCR_VOICE_SABIS1_RESET (0x2309)

/* tVoice和tVDR之间消息 */
#define	MSGID_VDR_VOICE_DATA	(0x2310)	//tVDR-->tVoice	voice data msg
#define	MSGID_VOICE_VDR_DATA	(0x2311)	//tVoice-->tVDR	voice data msg

/* tVoice的定时器消息 */
//ccb timer
enum
{
	MSGID_CCBTIMER_BEGIN=0x2320,

	MSGID_TIMER_ASSIGN=0x2320,		//(0x2320)	//Tassign timeout
	MSGID_TIMER_VAC,				//(0x2321)	//Tvac timeout
	MSGID_TIMER_ERRRSP,				//(0x2322)	//Terrrsp timeout
	MSGID_TIMER_RELRES,				//(0x2323)	//Trelres timeout
	MSGID_TIMER_PROBERSP,			//(0x2324)	//Tprobersp timeout
	MSGID_TIMER_WAITSYNC,			//(0x2325)	//Twaitsync timeout
	MSGID_TIMER_DELAY_RELVAC,		//(0x2326)	//delay rel vac

	MSGID_CCBTIMER_END
};

//global timer
#define	MSGID_TIMER_BEATHEART	(0x2350)	//Tbeatheart timeout
#define MSGID_TIMER_CONGEST		(0x2351)	//Tcongest timeout

//集群相关定时器
#define MSGID_TIMER_GRP_PAGING_RSP 		(0x2370)//Tm_GrpPagingRsp	组呼寻呼响应定时器	10秒	0x2370
#define MSGID_TIMER_GRP_LEPAGING_RSP 	(0x2371)//Tm_LePagingRsp	组呼迟后进入寻呼响应定时器	可配置默认0.5秒	0x2371
#define MSGID_TIMER_GRP_STATUSREPORT 	(0x2372)//Tm_StatusReport	组呼状态报告周期定时器	可配置默认5秒	0x2372
#define MSGID_TIMER_GRP_LEPAGING_LOOP 	(0x2373)//Tm_LePagingLoop	组呼迟后进入周期定时器	见SABis1文档	0x2373
#define MSGID_TIMER_GRP_RES_CLEAR 		(0x2374)//Tm_ResClear	组呼空口广播资源延迟释放定时器	可配置默认10秒	0x2374
#define MSGID_TIMER_GRP_LEPAGING_START 	(0x2375)//Tm_LePagingStart	迟后进入开始定时器	可配置默认20秒	0x2375
#define MSGID_TIMER_GRP_DATA_DETECT		(0x2376)//Tm_GrpToneDetect	组呼语音包监测定时器	可配置默认60秒	0x2376
#define MSGID_TIMER_GRP_RLS 				(0x2377)//Tm_GrpRls	收到Group release后延迟50ms释放空口资源定时器	50ms	0x2377

/* tVoice和tCM之间消息 */
#define	MSGID_VOICE_UT_REG		(M_CPEM_VOICE_VPORT_ENABLE_NOTIFY)	//device reg success msg,begin service
#define	MSGID_VOICE_UT_UNREG	(M_CPEM_VOICE_VPORT_DISABLE_NOTIFY)	//device outof service
#define MSGID_VOICE_UT_MOVEAWAY	(M_CPEM_VOICE_CPE_MOVEAWAY_NOTIFY)	//devoice move away

/* tVCR和tCM之间消息 */
#define MSGID_CM_VCR_PERFORMANCE_REQ	(0x6103)	//voice performance request
#define MSGID_VCR_CM_PERFORMANCE_RSP	(0x6104)	//voice performance response

#define MSGID_VOICE_SET_CFG		(0x6105)	//config BTS L3 voice task

#define MSGID_AUTHDEV_REQ			(0x6200)
#define MSGID_AUTH_CMD				(0x6201)
#define MSGID_AUTH_RSP				(0x6202)
#define MSGID_AUTHDEV_RESULT		(0x6203)
#define MSGID_BWINFO_REQ			(0x6204)
#define MSGID_BWINFO_RSP			(0x6205)
#define MSGID_BWINFO_DEL_REQ		(0x6206)
#define MSGID_BWINFO_DEL_RSP		(0x6207)
#define MSGID_BWINFO_UPDATE_REQ		(0x6208)
#define MSGID_BWINFO_UPDATE_RSP		(0x6209)

#ifdef M_SYNC_BROADCAST
#define MSGID_GRP_L2L3_MBMSGrpRes_Indication (0x3C1F)//MBMSGroupResource.Indication	0x3C1F	BTSL3-->L2VAC
#define MSGID_SYNCBROADCAST_DATA (0x3C20)//同播数据包消息ID为0x3C20

#define MSGID_L2L3_BTSL2SXC_UL_MSG (0x3047)//MBMS_RS.Request	0x3047	BTSL2-->BTSL3
#define MSGID_L2L3_BTSL2SXC_DL_MSG (0x3048)//MBMS_RS.Indication	0x3048	BTSL3-->L2VAC

#define MSGID_GRP_L2L3_MBMS_MEDIA_DELAY_RSP (0x3049)//L2L3_MBMS_MEDIA_DELAY_RSP	0x304C	BTSL2-->BTSL3，不带语音数据的GrpL3为0xFFFF的SRTP

#endif//M_SYNC_BROADCAST


#ifdef __cplusplus
}
#endif

#endif /* __BtsVMsgId_H */


