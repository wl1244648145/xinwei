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
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3CPEMESSAGEID
#define _INC_L3CPEMESSAGEID

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

// 0X6000 ----0X7FFF����BTS ͬCPE��������Ϣ��
// 0X6000 �D�D 0X62FF�����ù�����
// 0X6300 �D�D 0X63FF�����ܹ�����
// 0X6400 �D�D 0X64FF�����������
// 0X6500 �D�D 0X65FF����Ϲ�����
// 0X6600 �D�D 0X66FF��ҵ����ز���
// 0X6700 �D�D 0X67FF�����ݲ���
// 0X6800 �D�D 0X68FF��Voice����
///////////////////////////////////////////////////
// 0X6000 �D�D 0X62FF��������ز���
const UINT16 M_CPE_L3_REG_NOTIFY               = 0X6001;
const UINT16 M_L3_CPE_PROFILE_UPDATE_REQ       = 0X6002;
const UINT16 M_CPE_L3_PROFILE_UPDATE_RSP       = 0X6003;
const UINT16 M_L3_CPE_SETCPETIME_NOTIFY        = 0X6005;

const UINT16 M_L3_BTS_CPE_ETHERNET_CFG_REQ  = 0X6006;//BTS��CPE�����úͲ�ѯ��Ϣ
const UINT16 M_L3_CPE_BTS_ETHERNET_CFG_RSP  = 0X6007;//CPE��ӦBTS�����ò�ѯ��Ϣ

const UINT16 M_CPE_L3_GET_NEIGHBOR_LIST_REQ    = 0X600F;
const UINT16 M_L3_CPE_GET_NEIGHBOR_LIST_RSP    = 0X6010;
const UINT16 M_L3_CPE_BROADCAST_BTS_LOADINFO   = 0X6011;
const UINT16 M_CPE_L3_SWITCH_OFF_NOTIFY        = 0X6012;
const UINT16 M_CPE_L3_HANDOVER_PARA_CFG_REQ        = 0X6013;//�л��㷨������������
//�·���ЧƵ��
const UINT16 M_BTS_CPE_VALIDFREQS_REQ  = 0x6019;
//zengjihan 20120503 for MEM
const UINT16 M_BTS_UT_MEMINFO_REPORT_REQ     = 0x601A;
const UINT16 M_UT_BTS_MEMINFO_REPORT_RSP     = 0x601B;

const UINT16 M_CPE_BTS_Z_REGISTER_REQ          = 0X6020;
const UINT16 M_BTS_CPE_Z_REGISTER_RESPONSE     = 0X6021;
const UINT16 M_L3_FROML2_L1SLEEP			     = 0X6022;//zmy_test
const UINT16 M_L3_FROM_MMI			     = 0X6023;//zmy_test
const UINT16 M_CPE_REPORT_SIGNAL = 0x3505;//cpe signal report msg   ����pmģ�飬��������־�ϱ�һ�¡�
//MEM�����¼���Ϣ
const UINT16 M_L3_CPE_MEM_SIGNAL_RPT_CFG     = 0X6024;
const UINT16 M_BTS_CPE_MEM_SIGNAL_RPT_RSP     = 0X6025;
const UINT16 M_L3_CPE_MEM_RUNTIME_REQ     = 0X6026;
const UINT16 M_BTS_CPE_MEM_RUNTIME_RSP     = 0X6027;
const UINT16 M_L3_CPE_UT_LAN_PORT_CFG     = 0X6028;
const UINT16 M_BTS_CPE_UT_LAN_PORT_RSP     = 0X6029;

const UINT16 M_BTS_RPT_RPTONOFF_REQ   = 0x6030;
const UINT16 M_RPT_BTS_RPTONOFF_RSP   = 0x6031;
const UINT16 M_BTS_RPT_RPTCFG_REQ     = 0x6032;
const UINT16 M_RPT_BTS_RPTCFG_RSP     = 0x6033;
const UINT16 M_BTS_RPT_RPTGET_REQ     = 0x6034;
const UINT16 M_RPT_BTS_RPTGET_RSP     = 0x6035;
const UINT16 M_CPE_BB_RPTGET         = 0x6036;
const UINT16 M_RPT_CPE_2BBUART         = 0x6037;
const UINT16 M_RPT_BBUART_2CPE         = 0x6038;
const UINT16 M_CPE_BB_SW_UPDATE_REQ  = 0x6039;
const UINT16 M_CPE_BB_SW_UPDATE_RSP  = 0x603A;
const UINT16 M_BTS_RPT_RESET_REQ  = 0x603B;
const UINT16 M_BTS_RPT_RESET_RSP  = 0x603C;

const UINT16 M_CPE_BTS_LOADINFO_REQ   = 0x6014;
const UINT16 M_BTS_CPE_LOADINFO_RSP   = 0x6015;
#ifdef LOCATION_2ND
const UINT16 M_BTS_CPE_LOCATION_REQ  = 0x603C;
const UINT16 M_CPE_BTS_LOCATION_RSP  = 0x603D;
#endif
// ����Ӫά��������,ut��֤������Ϣ
const UINT16 M_CPE_L3_ACCESS_REQ               = 0X6100;
const UINT16 M_L3_CPE_AUTH_CMD                 = 0X6101;
const UINT16 M_CPE_L3_AUTH_RSP                 = 0X6102;
const UINT16 M_L3_CPE_AUTH_RESULT              = 0X6103;
const UINT16 M_CPE_L3_REGISTER_REQ             = 0X6104;
//const UINT16 M_L3_CPE_REGISTER_RSP             = 0X6105;
const UINT16 M_L3_CPE_UPDATE_BWINFO            = 0X6106;
const UINT16 M_CPE_L3_AccountLogin_req            = 0X6107;
const UINT16 MSGID_UM_VCR                      = 0x2301;

// 0X6300 �D�D 0X63FF��������ز��� TBD

// 0X6400 �D�D 0X64FF�������ز���
const UINT16 M_L3_CPE_UPGRADE_SW_REQ           = 0X6401;
const UINT16 M_CPE_L3_UPGRADE_SW_RSP           = 0X6402;
const UINT16 M_L3_CPE_UPGRADE_SW_PACK_REQ      = 0X6404;
const UINT16 M_CPE_L3_UPGRADE_SW_PACK_RSP      = 0X6405;
const UINT16 M_CPE_L3_UPGRADE_SW_FINISH_NOTIFY = 0X6407;

const UINT16 M_CPE_L3_UPGRADE_BOOTLOADER_REQ = 0X640A;
const UINT16 M_CPE_L3_UPGRADE_BOOTLOADER_RSP = 0X640B;
const UINT16 M_CPE_L3_UPGRADE_BOOTLOADER_PACK_REQ = 0X640C;
const UINT16 M_CPE_L3_UPGRADE_BOOTLOADER_PACK_RSP = 0X640D;
const UINT16 M_CPE_L3_UPGRADE_BOOTLOADER_FINISH_NOTIFY = 0X640E;
//for Z SW DL
const UINT16 M_L3_CPE_UPGRADE_Z_SW_REQ          = 0X0424;
const UINT16 M_CPE_L3_UPGRADE_Z_SW_RSP       = 0X0425;
//const UINT16 M_CPE_L3_UPGRADE_Z_SW_PROGRESS  = 0X0426;
const UINT16 M_CPE_L3_UPGRADE_Z_SW_FINISH_NOTIFY    = 0X0427;
const UINT16 M_L3_CPE_DL_Z_SW_PACK_REQ              = 0X0428;
const UINT16 M_CPE_L3_DL_Z_SW_PACK_RSP              = 0X0429;
const UINT16 M_CPE_L3_MZ_DELETE_UID_REQ              = 0X6022;
	//liu jian feng add
const UINT16 M_CPE_L3_ROLLBACK_TIMEOUT		 = 0X6431;	//CCPETaskSM��T_ROLLBACK��ʱ
const UINT16 M_CPE_L3_UNICAST_TIMEOUT		   = 0X6432;	//CCPETaskSM��Unicast��ʱ
const UINT16 M_CPE_L3_BOOTLOADER_TIMEOUT	 = 0X6433;	//CCPETaskSM��Unicast��ʱ
const UINT16 M_CPE_L3_SYSRESET_TIMEOUT	 = 0X6434;	//CCPETaskSM��Unicast��ʱ

const UINT16 M_CPE_L3_GETUSERINFO_TIMEOUT	 			= 0X6434;	//CCPETaskSM�����û�����
const UINT16 M_CPE_L3_CHGPWDRESULT_TIMEOUT	 			= 0X6435;	//CCPETaskSM�ȴ�HLR���û������޸Ľ��
const UINT16 M_CPE_L3_NICKNAMERESULT_TIMEOUT 			=0x6436;
// 0X6500 �D�D 0X65FF����Ϲ�����
const UINT16 M_L3_CPE_PROBE_UT_REQ             = 0x6501;
const UINT16 M_CPE_L3_PROBE_UT_RSP             = 0x6502;
const UINT16 M_L3_CPE_RESET_UT_REQ             = 0x6504;
const UINT16 M_CPE_L3_RESET_UT_RSP     	       = 0X6505;
const UINT16 M_L3_CPE_CLEAR_HIST_REQ             = 0x6506;
const UINT16 M_CPE_L3_CLEAR_HIST_RSP     	       = 0X6507;
const UINT16  M_L3_CPE_REBOOT_DSP_REQ       = 0X6508;
const UINT16 M_BTS_CPE_HWDESC_REQ          = 0x650b;
const UINT16 M_CPE_BTS_HWDESC_RSP     	    = 0X650c;
/*MEM133���������ѯ*/
const UINT16 M_BTS_CPE_UT_LAYER3_DATA_REQ        = 0X6509;//jy080923
const UINT16 M_CPE_BTS_UT_LAYER3_DATA_RSP        = 0X650a;//jy080923

//wangwenhua add 20081210 for debug
const UINT16 M_L3_CPE_BTS_COMM_MSG_CFG_REQ = 0x650d;
const UINT16 M_L3_CPE_BTS_DEBUG_COMM_MSG = 0x650e;

const UINT16  M_L3_CPE_BTS_BEAT_REQ   = 0x6510;
const UINT16 M_L3_BTS_CPE_BEAT_RSP   = 0x6511;
const UINT16 M_L3_BTS_CPE_FORCE_REGISTER = 0x6512;
// 0X6600 �D�D 0X66FF��ҵ����ز��� TBD
// 0X6700 �D�D 0X67FF��������ز��� TBD
// 0X6800 �D�D 0X68FF��������ز��� TBD

// 0X8000 ----0X8FFF Relay Message ID��
// 0x8f00: Relay broadcast OAM to UT
// 0x8f01: Relay broadcast Traffic to UT
// 0x8000: Relay high priority OAM to UT
// 0x8001: Relay low priority OAM to UT
// 0x8010: Relay high priority traffic to UT
// 0x8011: Relay low priority traffic to UT
// 0X9000 ----0XFFFF��Ϊ����������ʱ�����䣻	
// 

/**************************************
 *MSGID_TRAFFIC_INGRESS: ����������ϢID
 **************************************/
#define MSGID_TRAFFIC_INGRESS              (0x0100)

#ifdef RCPE_SWITCH
const UINT16 M_CPE_L3_TRUNK_MAC_MOVEAWAY_NOTIFY   = 0x6824;
const UINT16 M_CPE_L3_TRUNK_MAC_MOVEAWAY_GET   = 0x6825;
const UINT16 M_CPE_L3_TRUNK_MAC_MOVEAWAY_RSP   = 0x6826;
#endif
#endif 














