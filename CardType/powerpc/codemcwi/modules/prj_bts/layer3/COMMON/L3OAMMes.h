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
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/


#ifndef _INC_L3OAMMESSAGEID
#define _INC_L3OAMMESSAGEID

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

//////////////////////////////////////////////////////////////////////////
//定义L3oam内部消息id
//0X2001 ---- 0X22FF OAM部分内部使用Message ID部分
//////////////////////////////////////////////////////////////////////////
const UINT16 M_OAM_CALIBRATION_TIMER           = 0X2001;
const UINT16 M_OAM_CFGDATAINIT_FAIL_NOTIFY     = 0x2002;
const UINT16 M_OAM_CPE_REGISTER_TIMER          = 0x2005;
const UINT16 M_OAM_CPE_RECORD_KEEP_TIMER       = 0x2006;
const UINT16 M_OAM_CPE_PROF_UPDATE_FAIL        = 0x2007;
const UINT16 M_OAM_CPE_REG_TO_EMS_FAIL         = 0x2008;
const UINT16 M_OAM_CPE_PROBE_FAIL              = 0x2009;
const UINT16 M_OAM_CPE_RESET_FAIL              = 0x200A;
const UINT16 M_OAM_CPE_CHECK_TO_DELETE_TIMER   = 0x200B;
//const UINT16 M_OAM_DELETE_CPE_DATA_NOTIFY      = 0x200C;
//const UINT16 M_OAM_BROADCAST_BTSLOADINFO_TIMER = 0x200D;
const UINT16 M_OAM_ALARM_SEND_TO_EMS_TIMER     = 0x200E; //告警检测定时器消息:
const UINT16 M_OAM_BTS_BOOT_UP_TIMER           = 0X200F;
const UINT16 M_OAM_DATA_CFG_FINISH_NOTIFY      = 0X2011; // 数据配置完成指示消息
const UINT16 M_OAM_DATA_CFG_INIT_NOTIFY        = 0X2012; 
const UINT16 M_OAM_SETCPETIME_TIMER            = 0X2013; // 设置cpe时钟消息
const UINT16 M_OAM_BTSREG_NOTIFY_TIMER         = 0X2014; // BTS注册超时消息
const UINT16 M_EMS_BTS_LINK_TIMER              = 0X2015;
const UINT16 M_OAM_PROBING_L2SYS_TIMER         = 0X2016;
const UINT16 M_OAM_PROBING_L2SYS_FAIL          = 0X2017;
const UINT16 M_OAM_BTS_RESET_NOTIFY            = 0X2018; //个模块向系统管理模块发送的bts复位通知消息
const UINT16 M_OAM_BTS_PERIODREG_NOTIFY_TIMER  = 0X2019; // BTS周期注册
const UINT16 M_OAM_BTS_INIT_FROM_NVRAM_TIMER   = 0X201A; // BTS从nvram启动初始化数据超时
const UINT16 M_OAM_DELETE_ALM_RECORD_NOTIFY    = 0X201B; // 通知告警管理模块删除某告警
const UINT16 M_OAM_EMS_DL_NOTIFY_DATA_TIMER    = 0X201C; 
const UINT16 M_OAM_SYS_CFG_INIT_FROM_EMS_OK    = 0X201D; 
const UINT16 M_OAM_CFG_SYS_REV_FIRST_CFGMSG    = 0X201E; 
const UINT16 M_OAM_BTS_CLOCK_TIMER             = 0X201F; 
const UINT16 M_OAM_BTS_TEMPERATURE_MON_TIMER   = 0X2020;


const UINT16 M_CFG_SNOOP_DATA_SERVICE_CFG_REQ  = 0X2021;
const UINT16 M_SNOOP_CFG_DATA_SERVICE_CFG_RSP  = 0X2022;
const UINT16 M_CFG_DM_DATA_SERVICE_CFG_REQ     = 0X2024;
const UINT16 M_DM_CFG_DATA_SERVICE_CFG_RSP     = 0X2025;
const UINT16 M_CFG_ARP_DATA_SERVICE_CFG_REQ    = 0X2027;
const UINT16 M_ARP_CFG_DATA_SERVICE_CFG_RSP    = 0X2028;
const UINT16 M_CFG_EB_DATA_SERVICE_CFG_REQ     = 0X202A;
const UINT16 M_EB_CFG_DATA_SERVICE_CFG_RSP     = 0X202B;
#ifdef WBBU_CODE
const UINT16 M_OAM_PROBING_L2SYS_FAIL_core1          = 0X202c;
#endif
const UINT16 MSGID_OAM_NOTIFYBTSPUBIP          = 0x2326;//notify 'EB BTSPubIP changed

const UINT16 MSGID_OAM_NOTIFY_GET_BTSPUBIP          = 0x2328;//notify socket get BTSPubIP
const UINT16 M_CPEM_DM_CPE_DATA_CFG_NOTIFY     = 0X202D;
const UINT16 M_CPEM_DM_CPE_PROBE_REQ     = 0X202E;//向dm请求cpe信息jy080725
const UINT16 M_OAM_DATA_SERVICE_CFG_FAIL       = 0X2031;
const UINT16 M_CPEM_VOICE_VPORT_ENABLE_NOTIFY  = 0X2032;
const UINT16 M_CPEM_VOICE_VPORT_DISABLE_NOTIFY = 0X2033;
const UINT16 M_CM_DM_DELETE_UT_NOTIFY          = 0x2034;
const UINT16 M_OAM_BTS_INITFROMEMS_TIMER       = 0x2035;
const UINT16 M_OAMSYS_CFG_INIT_L2DATA_NOTIFY   = 0x2036;
const UINT16 M_OAMSYS_CFG_INIT_AUXDATA_NOTIFY  = 0x2037;
const UINT16 M_OAMSYS_CFG_INIT_FEPDATA_NOTIFY  = 0x2038;
const UINT16 M_CPEM_VOICE_CPE_MOVEAWAY_NOTIFY  = 0X2039;
const UINT16 MSGID_OAM_WRITE_NVRAM = 0x203A;
#ifdef WBBU_CODE
const UINT16 MSGID_L2L3_BOOT_REQ_LOADPARA = 0x4003;
#endif
const UINT16 M_OAM_UNICAST_UTSW_REQ_FAIL       = 0X2040;
const UINT16 M_OAM_SEND_DATA_NO_RSP_FAIL       = 0X2041;
const UINT16 M_OAM_BC_UTSW_TIMER               = 0X2042;
//const UINT16 M_OAM_UNICAST_ZSW_REQ_FAIL        = 0X2043;
const UINT16 M_OAM_BC_UTSW_TIMER_NEW           = 0X2044;
#ifdef M_TGT_WANIF
const UINT16 M_OAM_CPE_Probe_TO_WanIF_TIMER   = 0x2045;
#endif
const UINT16 M_OAM_CPE_CHECK_SAG_DEFAULT_TIMER   = 0x2046;
const UINT16 M_OAM_PERIOD_PERFDATAQUERY_TIMER  = 0X2050; // 周期性能数据查询
const UINT16 M_OAM_PERIOD_PERFDATA_LOGFILE_UPLOAD_TIMER  = 0X2051; // 周期性能日志上传
const UINT16 M_OAM_PERIOD_PERFDATA_WAIT_TIMER  = 0X2052;//网管配置到真正启动统计的等待时间jy080923 
const UINT16 M_OAM_PERIOD_PERFDATA_WAIT_UPLOAD_TIMER  = 0X2053;//延时上报定时器jy080923
const UINT16 M_OAM_PERIOD_RTMONITOR_ACTIVE_TIMER  = 0X2054;//延时上报定时器jy080923
const UINT16 M_OAM_PERIOD_RTMONITOR_START_TIMER  = 0X2055;//延时上报定时器jy080923
const UINT16 M_OAM_PERIOD_RTMONITOR_DELAY_TIMER  = 0X2056;//延时上报定时器jy080923

const UINT16 M_OAM_GPS_HEARTBEAT_TIMER         = 0X2060;
const UINT16 M_OAM_GET_GPS_VALIDEDATA_TIMER    = 0X2061;
const UINT16 M_OAM_CFG_GPS_DATA_ERR_ALM_TIMER  = 0X2062;

const UINT16 M_OAM_CFG_FTP_TRANSFER_FILE_REQ   = 0x2063;

const UINT16 M_OAM_CFG_SAVE_POWER_FAN_REQ   = 0x2064;
const UINT16 M_OAM_PERIOD_BAK_NVRAMDATA_TIMER   = 0x2065;//定时备份nvram备份数据

//同boot任务的消息
const UINT16 M_OAM_CPU_ALARM_NOTIFY            = 0X2070;
const UINT16 M_OAM_CPU_WORKING_NOTIFY          = 0X2071;
const UINT16 M_OAM_SYSTEM_RUNNING_NOTIFY       = 0X2072;
const UINT16 M_OAM_CPU_RESET_NOTIFY            = 0X2073; 

const UINT16 M_OAM_ALM_CLEAR_NOTIFY            = 0X2080; 

//同VOICE任务的消息
const UINT16 M_OAM_VOICE_SAG_LINK_ALM          = 0X2090; 
const UINT16 M_VOICE_EB_BILLING_NOTIFY         = 0X2091;

const UINT16 M_CFG_EB_VLAN_GROUP_CFG_REQ       = 0X20A0;
const UINT16 M_CFG_EB_VLAN_GROUP_CFG_RSP       = 0X20A1;

//通知socket发往备用ems的消息
const UINT16 M_SYS_SOCKET_BAK_EMS              = 0X20B1;

//gps通知alm修改rf mask
const UINT16 M_GPS_ALM_CHG_RF_MASK_REQ     = 0X20B2;

//UM通知SM终端已经切走,上报下载情况
const UINT16 M_UM_SM_UT_MOVE_AWAY_NOTIFY     = 0X20B3;
#ifdef WBBU_CODE
const UINT16 M_WRRU_ALM_CHG_RF_MASK_REQ     = 0X20B4;
const UINT16 M_FPGA_ALM_CHG_RF_MASK_REQ     = 0X20B5;
#endif
const UINT16 M_OAM_CFG_HLR_TIME_REQ = 0X20B6; //全网组呼定时上报
const UINT16 M_OAM_CFG_ALM_CLR_RF_ALM_REQ = 0x20b7; //cm在配置本地同步源时通知alm清除gps失步告警位
//loadinginfo
#define M_SOCKET_LOADINGINFO_TX          0xABCD

//for Jamming forwarding 
#define M_SOCKET_JNT_REFRESH             0xDCBA

#define BTS_Jamming_Rpt					 0X3B01
#define BTS_Jamming_Rpt_Rsp		     	 0X3B02
#define BTS_PairedCpe_Prof_Msg			 0X3B03
#define M_SOCKET_JAMMING_NEIGHBOR_NOTIFY 0x3B04
#define M_SOCKET_JAMMING_NEIGHBOR_RSP    0x3B05
#if 1//def M_CLUSTER_SAME_F
#define BTS_GROUP_RESOURCE_Rpt_Rsp          0x3B11
#endif
#define MSGID_CLUSTER_L2L3_PARA_CFG 				0x3B12//集群功能及参数配置请求
#define MSGID_CLUSTER_L2L3_PARA_CFG_RSP 			0x3B13


//集群通信
#define MSGID_GRP_HO_RES_REQ (0x3B20)//Grp_Ho_Resource_Req	0x3B20	M_TID_VOICE	M_TID_EMSAGENTTX
#define MSGID_GRP_HO_RES_RSP (0x3B21)//Grp_Ho_Resource_Rsp	0x3B21	M_TID_EMSAGENTTX	M_TID_VOICE


const UINT16 M_OAM_BTS_GET_BTS_TIME_FROM_EMS_TIMER =0x202f;
#endif 

