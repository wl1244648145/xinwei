/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3voiceMsgID.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   2006-09-14 fengbing  ֧���쳣������л�,��������ͬ����ʱ��ϢID
 *   2005-09-13 fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef	__VOICEMSGID_H
#define	__VOICEMSGID_H


#include "L3L2MessageID.h"
#include "L3OAMMessageId.h"

////////////////////////////////////////////////////////////////////////////////
//tVoice���񹫹���ϢID
/*
// tVoice��VAC֮����Ϣ 
#define	MSGID_VAC_SETUP_CMD		(0x3A01)	//Setup VAC Session setup Command
#define	MSGID_VAC_SETUP_RSP		(0x3A02)	//Setup VAC Session setup Response
#define	MSGID_VAC_SETUP_NOTIFY	(0x3A03)	//VAC Session Setup Notification
#define	MSGID_VAC_RLS_CMD		(0x3A04)	//VAC Session Release Command
#define	MSGID_VAC_RLS_NOTIFY	(0x3A05)	//VAC Session Release Notification
#define	MSGID_VAC_MODIFY_CMD	(0x3A06)	//VAC session Modify Command
#define	MSGID_VAC_MODIFY_RSP	(0x3A07)	//VAC session Modify Response
#define	MSGID_VAC_MODIFY_NOTIFY	(0x3A08)	//VAC session Modify Notification



#define	MSGID_VAC_VOICE_DATA	(0x3A10)	//Uplink Voice Data Message
#define	MSGID_VOICE_VAC_DATA	(0x3A11)	//Downlink Voice Data Message

#define	MSGID_VAC_VOICE_SIGNAL	(0x3A12)	//uplink Voice Control Message, VAC-->tVoice
#define	MSGID_VOICE_VAC_SIGNAL	(0x3A13)	//downlink Voice Control Message, tVoice-->VAC

// tVoice��L2֮�� 
#define MSGID_VOICE_MAC_PROBEREQ (0x3A20)	//probe req,paging only
#define MSGID_MAC_VOICE_PROBERSP (0x3A21)	//probe rsp,
#define MSGID_CONGESTION_REQUEST (0x3A22)	//congestion request to L2
#define MSGID_CONGESTION_REPORT	 (0x3A23)	//congestion report, answer of congestion request from L2

// tVoice��DAC֮�� 
#define MSGID_DAC_VOICE_SIGNAL	(0x6800)
#define MSGID_VOICE_DAC_SIGNAL	(0x6801)
*/

#define MSGID_VAC_SESSION_BEGIN	MSGID_VAC_SETUP_CMD
#define MSGID_VAC_SESSION_END	MSGID_VAC_MODIFY_NOTIFY

/* tVoice��tVCR֮����Ϣ */
#define	MSGID_VCR_VOICE_SIGNAL	(0x2300)	//tVCR-->tVoice signal
#define	MSGID_VOICE_VCR_SIGNAL	(0x2301)	//tVoice-->tVCR	signal
#define	MSGID_VOICE_VCR_RESETLINK (0x2302)
#define	MSGID_VCR_VOICE_FORCE_UT_REGISTER	(0x2303)
#define	MSGID_VCR_VOICE_RELEASE_VOICE_SRV	(0x2304)
#define	MSGID_VOICE_DCS_MSG (0x2305)
#define	MSGID_DCS_VOICE_MSG (0x2306)
#define	MSGID_VOICE_DCS_RESETLINK	(0x2307)
#define	MSGID_DGRPLINK_VOICE_RELEASE_DGRP_SRV (0x2308)

/* tVoice��tVDR֮����Ϣ */
#define	MSGID_VDR_VOICE_DATA	(0x2310)	//tVDR-->tVoice	voice data msg
#define	MSGID_VOICE_VDR_DATA	(0x2311)	//tVoice-->tVDR	voice data msg

/* tVoice�Ķ�ʱ����Ϣ */
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

//��Ⱥ��ض�ʱ��
#define MSGID_TIMER_GRP_PAGING_RSP 		(0x2370)//Tm_GrpPagingRsp	���Ѱ����Ӧ��ʱ��	10��	0x2370
#define MSGID_TIMER_GRP_LEPAGING_RSP 	(0x2371)//Tm_LePagingRsp	����ٺ����Ѱ����Ӧ��ʱ��	������Ĭ��0.5��	0x2371
#define MSGID_TIMER_GRP_STATUSREPORT 	(0x2372)//Tm_StatusReport	���״̬�������ڶ�ʱ��	������Ĭ��5��	0x2372
#define MSGID_TIMER_GRP_LEPAGING_LOOP 	(0x2373)//Tm_LePagingLoop	����ٺ�������ڶ�ʱ��	��SABis1�ĵ�	0x2373
#define MSGID_TIMER_GRP_RES_CLEAR 		(0x2374)//Tm_ResClear	����տڹ㲥��Դ�ӳ��ͷŶ�ʱ��	������Ĭ��10��	0x2374
#define MSGID_TIMER_GRP_LEPAGING_START 	(0x2375)//Tm_LePagingStart	�ٺ���뿪ʼ��ʱ��	������Ĭ��20��	0x2375
#define MSGID_TIMER_GRP_DATA_DETECT		(0x2376)//Tm_GrpToneDetect	�����������ⶨʱ��	������Ĭ��60��	0x2376
#define MSGID_TIMER_GRP_RLS 				(0x2377)//Tm_GrpRls	�յ�Group release���ӳ�50ms�ͷſտ���Դ��ʱ��	50ms	0x2377

/* tVoice��tCM֮����Ϣ */
#define	MSGID_VOICE_UT_REG		(M_CPEM_VOICE_VPORT_ENABLE_NOTIFY)	//device reg success msg,begin service
#define	MSGID_VOICE_UT_UNREG	(M_CPEM_VOICE_VPORT_DISABLE_NOTIFY)	//device outof service
#define MSGID_VOICE_UT_MOVEAWAY	(M_CPEM_VOICE_CPE_MOVEAWAY_NOTIFY)	//devoice move away

/* tVCR��tCM֮����Ϣ */
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

#endif /* __VOICEMSGID_H */

