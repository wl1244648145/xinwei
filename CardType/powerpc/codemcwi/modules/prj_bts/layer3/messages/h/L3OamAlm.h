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


/*****************************************************************************
 *新增告警:
 *1)告警ID;
 *2)告警字符串
 *3)ALM_id_str_tbl[]增加他们的对应关系
 *****************************************************************************/
 

#ifndef _INC_L3OAMALMINFO
#define _INC_L3OAMALMINFO

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

//Entify Define  0x00 - 公共告警
const UINT16 ALM_ENT_L3PPC           = 0X01;
const UINT16 ALM_ENT_L2PPC           = 0X02;
const UINT16 ALM_ENT_MCP             = 0X03;
const UINT16 ALM_ENT_AUX             = 0X04;
const UINT16 ALM_ENT_ENV             = 0X05;
const UINT16 ALM_ENT_TCXO            = 0X06;
const UINT16 ALM_ENT_RF              = 0X07;
const UINT16 ALM_ENT_GPS             = 0X08;
const UINT16 ALM_ENT_SYN             = 0X09;
const UINT16 ALM_ENT_PLL             = 0X0A;
#ifndef WBBU_CODE
const UINT16 ALM_ENT_L1              = 0X0B;
#else
const UINT16 ALM_ENT_FEP              = 0X0B;
const UINT16 ALM_ENT_FPGA             = 0x0a;
const UINT16 ALM_ENT_WRRU              = 0x0d;
const  UINT16 ALM_ENT_CORE9            = 0xc;
const UINT16  ALM_ENT_AIF        = 0xe;
#endif
//Alarm ID Definition
const UINT16 ALM_ID_BTS_UPGRADE_FAIL                        = 0x0101;
const UINT16 ALM_ID_BTS_SAG_LINK                            = 0x0102;

const UINT16 ALM_ID_L2PPC_COMMFAIL                          = 0x0201;
const UINT16 ALM_ID_L2PPC_RESET                             = 0x0202;

/*****************************************************增加三个告警：
1)	数据网关1不通告警，告警描述详细信息要求携带网关1的IP地址
2)	数据网关2不通告警，告警描述详细信息要求携带网关2的IP地址
3)	业务传输断关闭射频告警，告警描述详细信息：数据业务不通，语音业务不通，或者数据和语音都不通。满足恢复射频的条件即触发此告警恢复。
注意：除了业务传输断导致关闭射频，其他原因造成的射频板关闭均使用原有的告警。
********************************************************************************/
const  UINT16  ALM_ID_GateWay1_Fail                      = 0x0103;
const  UINT16  ALM_ID_GateWay2_Fail                      = 0x0104;
const  UINT16  ALM_ID_Business_Fail_Close_RF              = 0x010e;
#ifdef WBBU_CODE
const UINT16 ALM_ID_L2PPC_CORE9_MSG_ERROR                   = 0x0208;
const UINT16 ALM_ID_L2PPC_CORE9_UPLINK_DATA_ERROR        = 0x0209;

/*******************************************************
0x0105	BBU Fiber optic module Voltuage	BBU光模块电压告警	提示/事件
0x0106	BBU Fiber optic module Current	BBU光模块电流告警	提示/事件
0x0107	BBU Fiber optic module Tx_Powert	BBU光模块发射功率告警	提示/事件
0x0108	BBU Fiber optic module Rx_Powert	BBU光模块接收功率告警	提示/事件


0x0d09	RRU Fiber optic module Voltuage	RRU光模块电压告警	提示/事件
0x0d0A	RRU Fiber optic module Current	RRU光模块电流告警	提示/事件
0x0d0B	RRU Fiber optic module Tx_Power	RRU光模块发射功率告警	提示/事件
0x0d0C	RRU Fiber optic module Rx_Power	RRU光模块接收功率告警	提示/事件

**************************************************************/
const UINT16 ALM_ID_BTS_FIBER_VOL = 0x0105;
const UINT16 ALM_ID_BTS_FIBER_Current = 0x0106;
const UINT16 ALM_ID_BTS_FIBER_TX_PWR = 0x0107;
const UINT16 ALM_ID_BTS_FIBER_RX_PWR = 0x0108;

const UINT16 ALM_ID_RRU_FIBER_VOL = 0x0d09;
const UINT16 ALM_ID_RRU_FIBER_Current = 0x0d0A;
const UINT16 ALM_ID_RRU_FIBER_TX_PWR = 0x0d0B;
const UINT16 ALM_ID_RRU_FIBER_RX_PWR = 0x0d0C;


const UINT16 ALM_ID_DSP1_RESET  = 0x0109;
const UINT16 ALM_ID_DSP2_RESET  = 0x010a;
const UINT16 ALM_ID_DSP3_RESET  = 0x010b;
const UINT16 ALM_ID_DSP4_RESET  = 0x010c;
const UINT16 ALM_ID_DSP5_RESET  = 0x010d;

#endif
//wangwenhua add 2012-4-26
const UINT16 ALM_ID_eb_no_ft_freelist   = 0x0110;
const UINT16 ALM_ID_eb_no_cdr_freelist = 0x0111;
const UINT16 ALM_ID_socket_no_ft_freelist  = 0x0112;
const UINT16 ALM_ID_socket_no_CB_freelist  = 0x0113;
const UINT16 ALM_ID_snoop_no_freelist   = 0x0114;
const UINT16 ALM_ID_arp_no_freelist  = 0x0115;
const UINT16 ALM_ID_dm_no_freelist  = 0x0116;
const UINT16 ALM_ID_cdr_err  = 0x0117;

const UINT16 ALM_ID_MCP_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY    = 0x0301;
const UINT16 ALM_ID_MCP_L2_TO_MCP_UPLINKPROF_NOT_EMPTY      = 0x0302;
const UINT16 ALM_ID_MCP_L2_TO_MCP_CONFIG_NOT_EMPTY          = 0x0303;
const UINT16 ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_NOT_FULL     = 0x0304;
const UINT16 ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_CHKSUM       = 0x0305;
const UINT16 ALM_ID_MCP_MCP_TO_L2_UPLINK_NOT_EMPTY          = 0x0306;
const UINT16 ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_NOT_FULL       = 0x0307;
const UINT16 ALM_ID_MCP_MCP_FROM_L2_UPPROF_NOT_FULL         = 0x0308;
const UINT16 ALM_ID_MCP_MCP_FROM_L2_CONFIG_NOT_FULL         = 0x0309;
const UINT16 ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_CHKSUM         = 0x030a;
const UINT16 ALM_ID_MCP_MCP_FROM_L2_UPPROF_CHKSUM           = 0x030b;
const UINT16 ALM_ID_MCP_MCP_FROM_L2_CONFIG_CHKSUM           = 0x030c;
const UINT16 ALM_ID_MCP_MCP_FEP_ALARM                       = 0x030d;

#ifdef WBBU_CODE
const UINT16 ALM_ID_MCP_CORE9_DOWN_DATA_LOST = 0x030e;		//MCP从Core9接收下行链路数据告警	提示/事件
const UINT16 ALM_ID_MCP_CORE9_UPLINK_Prof_LOST = 0x030f	;	//MCP从Core9接收上行链路数据告警	提示/事件
const UINT16 ALM_ID_MCP_CORE9_CFG_MSG_LOST = 0x0310;		//MCP从Core9接收配置消息告警	提示/事件
const UINT16 ALM_ID_MCP_FEP_UPLINK_DATA_ERROR = 0x0311	;	//MCP从FEP接收上行数据告警	提示/事件
#endif
const UINT16 ALM_ID_AUX_TO_L2_CONTROL_FAIL                  = 0x0401;
const UINT16 ALM_ID_AUX_TO_L2_RANGING_BUF_NOT_EMPTY         = 0x0402;
const UINT16 ALM_ID_AUX_L2_TO_AUX_CONTROL_FAIL              = 0x0403;
const UINT16 ALM_ID_AUX_L2_TO_AUX_WEIGHT_NOT_EMPTY          = 0x0404;
const UINT16 ALM_ID_AUX_L2_TO_AUX_CONFIG_NOT_EMPTY          = 0x0405;
const UINT16 ALM_ID_AUX_L2_FROM_AUX_RANGING_NOT_FULL        = 0x0406;
const UINT16 ALM_ID_AUX_L2_FROM_AUX_RANGING_CHKSUM          = 0x0407;
const UINT16 ALM_ID_AUX_AUX_FROM_L2_BUF_NOT_FULL            = 0x0408;
const UINT16 ALM_ID_AUX_AUX_FROM_L2_CHKSUM                  = 0x0409;
const UINT16 ALM_ID_AUX_AUX_TO_FEP_BUF_NOT_EMPTY            = 0x040a;
const UINT16 ALM_ID_AUX_FEP_FROM_AUX_CHKSUM                 = 0x040b;
#ifndef WBBU_CODE
const UINT16 ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM         = 0x040c;
#endif
#ifdef WBBU_CODE
const UINT16 ALM_ID_AUX_AUX_FROM_CORE9_DOWN_DATA_ALM  = 0x040c		;//AUX从Core9接收下行链路数据告警
const  UINT16 ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM = 0x040d	;//AUX向FEP通信告警
const UINT16 ALM_ID_AUX_AUX_FROM_FPGA_RecMSG_ALM  =0x040e		;//AUX从FPGA接收消息丢告警	
const UINT16 ALM_ID_AUX_AUX_FROM_FPGA_CHECKSUM_ALM  =0x040f		;//AUX从FPGA接收消息校验和错告警	
const UINT16 ALM_ID_AUX_AUX_Calibration_Err = 0x0410;//aux校准失败的告警
#endif

const UINT16 ALM_ID_ENV_SYNC_TEMPERATURE                    = 0x0501;
const UINT16 ALM_ID_ENV_DIGITAL_BOARD_TEMPERATURE           = 0x0502;
const UINT16 ALM_ID_ENV_FAN_STOP                            = 0x0503;
const UINT16 ALM_ID_ENV_SYNC_SHUTDOWN                       = 0x0504;

const UINT16 ALM_ID_PLL_TCXO_FREQOFF                        = 0x0601;
const UINT16 ALM_ID_PLL_PLLLOSELOCK_MINOR                   = 0x0602;
const UINT16 ALM_ID_PLL_PLLLOSELOCK_SERIOUS                 = 0x0603;
const UINT16 ALM_ID_PLL_FACTORY_DATA                        = 0x0604;
const UINT16 ALM_ID_PLL_SSP_CHECKSUM_ERROR                  = 0x0605;
const UINT16 ALM_ID_PLL_AUX2SYNC_CHECKSUM_ERROR             = 0x0606;
const UINT16 ALM_ID_PLL_SYNC_NORESP_ERROR                   = 0x0607;


const UINT16 ALM_ID_RF_BOARD_VOLTAGE_MINOR                  = 0x0701;
const UINT16 ALM_ID_RF_BOARD_VOLTAGE_SERIOUS                = 0x0702;
const UINT16 ALM_ID_RF_BOARD_CURRENT_MINOR                  = 0x0703;
const UINT16 ALM_ID_RF_BOARD_CURRENT_SERIOUS                = 0x0704;
const UINT16 ALM_ID_RF_TTA_VOLTAGE_MINOR                    = 0x0705;
const UINT16 ALM_ID_RF_TTA_VOLTAGE_SERIOUS                  = 0x0706;
const UINT16 ALM_ID_RF_TTA_CURRENT_MINOR                    = 0x0707;
const UINT16 ALM_ID_RF_TTA_CURRENT_SERIOUS                  = 0x0708;
const UINT16 ALM_ID_RF_TX_POWER_MINOR                       = 0x0709;
const UINT16 ALM_ID_RF_TX_POWER_SERIOUS                     = 0x070a;
const UINT16 ALM_ID_RF_RF_DISABLED                          = 0x070b;
const UINT16 ALM_ID_RF_BOARD_SSP_CHKSUM_ERROR               = 0x070c;
const UINT16 ALM_ID_RF_BOARD_RF_CHKSUM_ERROR                = 0x070d;
const UINT16 ALM_ID_RF_BOARD_RF_NORESPONSE                  = 0x070e;
#ifdef WBBU_CODE
const UINT16 ALM_ID_RF_BOARD_ZBB_MINOR                =0x070f		;//驻波比轻度告警	提示/事件
const UINT16 ALM_ID_RF_BOARD_ZBB_SERIOUS                 =0x0710		;//驻波比重度告警	紧急
#endif
const UINT16 ALM_ID_GPS_SIGNAL                              = 0x0801;
const UINT16 ALM_ID_GPS_LOC_CLOCK                           = 0x0802;
const UINT16 ALM_ID_GPS_LOST                                = 0x0803;
const UINT16 ALM_ID_GPS_CFG_ERROR                           = 0x0804;
const UINT16 ALM_ID_GPS_COMMNICATION_FAIL                   = 0x0805;
const UINT16 ALM_ID_GPS_ANTENNA_NOT_CONNECTED               = 0x0806;
#ifdef WBBU_CODE
const UINT16  ALM_ID_gps_warning                            = 0x0a01;
const UINT16  ALM_ID_tdd_fep_rx_warning                  = 0x0a04;
const UINT16   ALM_ID_frame_sync_warning                = 0x0a05;
const UINT16   ALM_ID_sfp_los_warning                      = 0x0a06;
const UINT16   ALM_ID_fan_warning                      = 0x0a07;
const UINT16   ALM_ID_10ms_warning                      = 0x0a08;//这个告警EMS上暂时不支持
const UINT16   ALM_ID_AD4001_warning                      = 0x0a09;//这个告警EMS上暂时不支持
const UINT16 ALM_ID_FEP_MCP_DOWN_DATA_LOST = 0x0b01	;	//FEP从MCP接收下行数据告警	提示/事件
const UINT16 ALM_ID_FEP_MCP_DOWN_WEIGHT_ERROR =0x0b02	;	//FEP从MCP接收下行weight告警	提示/事件
const UINT16 ALM_ID_FEP_AUX_CFG_MSG_ERROR =0x0b03	;	//FEP从AUX接收配置消息告警	提示/事件
const UINT16 ALM_ID_CORE9_L2_RSP_TIMEOUT=0x0c01	;	//Core9接收L2消息响应超时	提示/事件
const UINT16 ALM_ID_CORE9_L2_CFG_MSG_WARN =0x0c02		;//Core9从L2接收组合消息告警	提示/事件
const UINT16 ALM_ID_CORE9_L2_DOWN_DATA_WARN =0x0c03	;	//Core9从L2接收下行数据告警	提示/事件
const UINT16 ALM_ID_CORE9_L2_UP_PROFILE_WARN =0x0c04	;	//Core9从L2接收上行profile数据告警	提示/事件
const UINT16 ALM_ID_CORE9_MCP_UPLINK_DATA_LOST =0x0c05	;	//Core9从MCP接收上行链路数据告警	提示/事件
const UINT16 ALM_ID_CORE9_FROM_AUX_RECVMSG_LOST =0x0c06		;//Core9从AUX接收消息丢告警
const UINT16 ALM_ID_CORE9_FROM_AUX_CHECKSUM_ERROR = 0x0c07;		//Core9从AUX接收消息校验和错告警


const UINT16	Alarm_ID_WRRU_Temp =0x0d01	;//	RRU温度告警
const UINT16	Alarm_ID_WRRU_Current = 0x0d02	;	//RRU电流告警
const UINT16	Alarm_ID_WRRU_recv_nopoll = 0x0d05	;//	RRU与BBU心跳告警
const UINT16	Alarm_ID_WRRU_Download_MCU = 0x0d06	;//	RRU代码加载失败告警（MCU）
const UINT16	Alarm_ID_WRRU_Download_FPGA =0x0d07;//		RRU代码加载失败告警（FPGA）
const UINT16	Alarm_ID_WRRU_Reset = 0x0d08	;//	RRU复位告警
  const UINT16 Alarm_ID_RRU_CheckSum_ERR =0x0d09;
  const UINT16 Alarm_ID_RRU_AFC4001_ERR =0x0d0a;

const UINT16	Alarm_ID_FEP_AUX_FPGA_LINK_ERROR = 0x0e01	;	//FEPAUX收FPGA LINK ERROR告警
const UINT16	Alarm_ID_FEP_AUX_DSP2_LINK_ERROR= 0x0e02	;	//FEPAUX收DSP2 LINK ERROR 告警
const UINT16	Alarm_ID_FEP_AUX_DSP3_LINK_ERROR = 0x0e03	;	//FEPAUX收DSP3 LINK ERROR 告警
const UINT16	Alarm_ID_FEP_AUX_DSP4_LINK_ERROR = 0x0e04	;	//FEPAUX收DSP4 LINK ERROR 告警
const UINT16	Alarm_ID_FEP_AUX_DSP2_AIF_HEAD_ERROR = 0x0e05	;	//FEPAUX收DSP2 AIF HEADER LOST 告警
const UINT16	Alarm_ID_FEP_AUX_DSP3_AIF_HEAD_ERROR = 0x0e06	;	//FEPAUX收DSP3 AIF HEADER LOST 告警
const UINT16	Alarm_ID_FEP_AUX_DSP4_AIF_HEAD_ERROR = 0x0e07	;	//FEPAUX收DSP4 AIF HEADER LOST 告警
const UINT16	Alarm_ID_FEP_AUX_AIF_RECONFIG_ERROR = 0x0e08	;	//FEPAUX AIF RECONFIG 告警
const UINT16	Alarm_ID_DSP2_DSP3_FEP_LINK_ERROR = 0x0e09	;	//DSP2&3收FEP LINK ERROR 告警
const UINT16	Alarm_ID_DSP2_DSP3_CORE9_LINK_ERROR = 0x0e0a	;	//DSP2&3收CORE9 LINK ERROR 告警
const UINT16	Alarm_ID_DSP2_DSP3_FEP_AIF_HEAD_LOST = 0x0e0b	;	//DSP2&3收FEP AIF HEADER LOST 告警
const UINT16	Alarm_ID_DSP2_DSP3_CORE9_AIF_HEAD_LOST = 0x0e0c	;	//DSP2&3收CORE9 AIF HEADER LOST 告警
const UINT16	Alarm_ID_DSP2_DSP3_AIF_RECONFIG = 0x0e0d	;	//DSP2&3 AIF RECONFIG 告警
//const UINT16	Alarm_ID_FEP_AUX_DSP2_AIF_HEAD_ERROR= 0x0e0e	;	//MCP6收FEP LINK ERROR 告警
//const UINT16	Alarm_ID_ = 0x0e0f	;	//MCP6收FEP LINK ERROR 告警
//const UINT16	Alarm_ID_ = 0x0e10	;	//MCP6收FEP AIF HEADER LOST 告警
const UINT16	Alarm_ID_CORE9_AUX_LINK_ERROR = 0x0e11	;	//CORE9收AUX LINK ERROR 告警
const UINT16	Alarm_ID_CORE9_DSP2_LINK_ERROR = 0x0e12	;	//CORE9收DSP2 LINK ERROR 告警
const UINT16	Alarm_ID_CORE9_DSP3_LINK_ERROR = 0x0e13	;	//CORE9收DSP3 LINK ERROR 告警
const UINT16	Alarm_ID_CORE9_DSP5_LINK_ERROR = 0x0e14	;	//CORE9收DSP5 LINK ERROR 告警
//const UINT16	Alarm_ID_ = 0x0e15	;	
const UINT16	Alarm_ID_CORE9_AUX_AIF_HEADER_LOST = 0x0e16	;	//CORE9收AUX AIF HEADER LOST 告警
const UINT16	Alarm_ID_CORE9_DSP2_AIF_HEADER_LOST = 0x0e17	;	//CORE9收DSP2 AIF HEADER LOST 告警
const UINT16	Alarm_ID_CORE9_DSP3_AIF_HEADER_LOST  = 0x0e18	;	//CORE9收DSP3 AIF HEADER LOST 告警
const UINT16	Alarm_ID_CORE9_DSP5_AIF_HEADER_LOST  = 0x0e19	;	//CORE9收DSP5 AIF HEADER LOST 告警
const UINT16	Alarm_ID_CORE9_AIF_RECONFIG  = 0x0e1a	;	//CORE9 AIF RECONFIG 告警

const UINT16  Alarm_ID_AIF_Dsp2_To_AUX_Linkerror  = 0x0e01;
const UINT16 Alarm_ID_AIF_Dsp2_To_AUX_HeaderLost = 0x0e02;
const UINT16 Alarm_ID_AIF_Dsp3_To_AUX_Linkerror = 0x0e03;
const UINT16 Alarm_ID_AIF_Dsp3_To_AUX_HeaderLost = 0x0e04;
const UINT16 Alarm_ID_AIF_Dsp4_To_AUX_Linkerror = 0x0e05;
const UINT16 Alarm_ID_AIF_Dsp4_To_AUX_HeaderLost = 0x0e06;
  const UINT16 Alarm_ID_AIF_FPGA_To_AUX_Linkerror = 0x0e07;
  const UINT16 Alarm_ID_AIF_FPGA_To_AUX_Timeout = 0x0e08;
  const UINT16 Alarm_ID_AIF_AUX_AIF_Reconfig = 0x0e09;
 const UINT16 Alarm_ID_AIF_Dsp4_To_Dsp2_Linkerror = 0x0e0a;
const UINT16 Alarm_ID_AIF_Dsp4_To_Dsp2_HeaderLost = 0x0e0b;
  const UINT16 Alarm_ID_AIF_FEP_To_Dsp2_Linkerror = 0x0e0c;
  const UINT16 Alarm_ID_AIF_FEP_To_Dsp2_HeaderLost	= 0x0e0d;
  const UINT16 Alarm_ID_AIF_Dsp2_AIF_Reconfig = 0x0e0e;
  const UINT16 Alarm_ID_AIF_Dsp4_To_Dsp3_Linkerror = 0x0e0f;
 const UINT16 Alarm_ID_AIF_Dsp4_To_Dsp3_HeaderLost = 0x0e10;
const UINT16 Alarm_ID_AIF_FEP_To_Dsp3_Linkerror = 0x0e11;
 const UINT16 Alarm_ID_AIF_FEP_To_Dsp3_HeaderLost = 0x0e12;
const UINT16 Alarm_ID_AIF_Dsp3_AIF_Reconfig =0x0e13;
  const UINT16 Alarm_ID_AIF_FEP_To_Dsp4_Linkerror = 0x0e14;
const UINT16 Alarm_ID_AIF_FEP_To_Dsp4_HeaderLost = 0x0e15;
  const UINT16 Alarm_ID_AIF_AUX_To_Core9_Linkerror = 0x0e16;
  const UINT16 Alarm_ID_AIF_AUX_To_Core9_HeaderLost = 0x0e17;
  const UINT16 Alarm_ID_AIF_Dsp2_To_Core9_Linkerror = 0x0e18;
 const UINT16 Alarm_ID_AIF_Dsp2_To_Core9_HeaderLost = 0x0e19;
  const UINT16 Alarm_ID_AIF_Dsp3_To_Core9_Linkerror =0x0e1a;
 const UINT16 Alarm_ID_AIF_Dsp3_To_Core9_HeaderLost = 0x0e1b;
  const UINT16 Alarm_ID_AIF_Dsp5_To_Core9_Linkerror = 0x0e1c;
  const UINT16 Alarm_ID_AIF_Dsp5_To_Core9_HeaderLost = 0x0e1d;
const UINT16 Alarm_ID_AIF_Core9_To_Dsp5_Handshake_Timeout = 0x0e1e;
  const UINT16  Alarm_ID_AIF_AIF_Reconfig = 0x0e1f;

#endif
const UINT16 ALM_ENT_INDEX0          = 0;
const UINT16 ALM_ENT_INDEX1          = 1;
const UINT16 ALM_ENT_INDEX2          = 2;
const UINT16 ALM_ENT_INDEX3          = 3;
const UINT16 ALM_ENT_INDEX4          = 4;
const UINT16 ALM_ENT_INDEX5          = 5;
const UINT16 ALM_ENT_INDEX6          = 6;
const UINT16 ALM_ENT_INDEX7          = 7;

const UINT16 ALM_CLASS_CRITICAL      = 1;
const UINT16 ALM_CLASS_MAJOR         = 2;
const UINT16 ALM_CLASS_MINOR         = 3;
const UINT16 ALM_CLASS_INFO          = 4;

const UINT32 ALM_INFO_LEN            = 200*2;
const UINT32 MAX_ALM_TPYE            = 500;

const UINT8  ALM_FLAG_CLEAR          = 0;
const UINT8  ALM_FLAG_SET            = 1;

const UINT8  ALM_NOT_HANDLED         = 0;
const UINT8  ALM_HANDLED             = 1;

const UINT16  MAX_SET_ALM_NUM        = 5;    //最多连续产生的告警次数，当前全是5次         
const UINT16  MIN_SET_ALM_NUM        = 1;  
const UINT16  MAX_CLEAR_ALM_NUM      = 5;    //已有几次没有告警产生
const UINT16  MIN_CLEAR_ALM_NUM      = 1;    

const UINT8   ALM_DEF_RATIO          = 80;   // 产生告警处理动作要达到的告警产生比例
const UINT8   ALM_CRITICAL_RATIO     = 50;   // 产生告警处理动作要达到的告警产生比例
const UINT8   ALM_COMMON_RATIO       = 100;  // 产生告警处理动作要达到的告警产生比例

#define M_ALM_MAX_ENDURE_SEC_AUX    (10*1000)
#define M_ALM_MAX_ENDURE_SEC_L2PPC  (30*1000)
#define M_ALM_MAX_ENDURE_SEC_MCP    (10*1000)
#define M_ALM_MAX_ENDURE_SEC_ENV    (10*1000)
#define M_ALM_MAX_ENDURE_SEC_TCXO   (1*60*1000)
#define M_ALM_MAX_ENDURE_SEC_RF     (5*1000)
#define M_ALM_MAX_ENDURE_SEC_GPS    (1*1000)
#define M_ALM_MAX_ENDURE_SEC_PLL    (1*1000)
#define M_ALM_MAX_ENDURE_SEC_DEF    (0xFFFFFFFF)


//恢复告警串
#define STR_CLEAR_ALARM                                "clear ALARM..."

//设置告警串
#define STR_BTS_BKVERFAIL                              "\r\nBTS software upgrade fail."
#define STR_BTS_SAG_LINK                               "\r\nLink between BTS and SAG is down."

#define STR_L2PPCRESET                                 "\r\nReset CPU:%s, Index[%d]"
//L3L2通信
#define STR_L3L2LINKFAIL                               "\r\nL2-L3 communicate fail."


#define STR_GateWay1_Fail                              "\r\nGateWay1:%x fail\n"
#define STR_GateWay2_Fail                              "\r\nGateWay2:%x fail\n"

#define STR_CloseRF_Warn1                               "\r\n Data business fail"
#define STR_CloseRF_Warn2                               "\r\n Voice business fail"
#define STR_CloseRF_Warn3                              "\r\n Data and voice business fail"
#define STR_CloseRF_Warn4                             "\r\n Data or  voice business fail"
#ifdef WBBU_CODE
#define STR_L3L2_Core1_LINKFAIL                               "\r\nL2-L3 core1 communicate fail."
#endif
//RF
#define STR_BOARD_VOLTAGE_OUTOFRANGE_MINOR             "\r\nAntenna[%d]: Minor   board voltage out of range."
#define STR_BOARD_VOLTAGE_OUTOFRANGE_SERIOUS           "\r\nAntenna[%d]: Serious board voltage out of range."
#define STR_BOARD_CURRENT_OUTOFRANGE_MINOR             "\r\nAntenna[%d]: Minor   board current out of range."
#define STR_BOARD_CURRENT_OUTOFRANGE_SERIOUS           "\r\nAntenna[%d]: Serious board current out of range."
#define STR_TTA_VOLTAGE_OUTOFRANGE_MINOR               "\r\nAntenna[%d]: Minor   TTA voltage out of range."
#define STR_TTA_VOLTAGE_OUTOFRANGE_SERIOUS             "\r\nAntenna[%d]: Serious TTA voltage out of range."
#define STR_TTA_CURRENT_OUTOFRANGE_MINOR               "\r\nAntenna[%d]: Minor   TTA current out of range."
#define STR_TTA_CURRENT_OUTOFRANGE_SERIOUS             "\r\nAntenna[%d]: Serious TTA current out of range."
#define STR_TX_POWER_OUTOFRANGE_MINOR                  "\r\nAntenna[%d]: Minor   TX power out of range."
#define STR_TX_POWER_OUTOFRANGE_SERIOUS                "\r\nAntenna[%d]: Seriou  TX power out of range."
#define STR_RF_DISABLED                                "\r\nAntenna[%d]: Antenna is disabled."
#define STR_BOARD_SSP_CHKSUM_ERROR                     "\r\nAntenna[%d]: SSP checksum error."
#define STR_BOARD_RF_CHKSUM_ERROR                      "\r\nAntenna[%d]: Antenna checksum error."
#ifndef WBBU_CODE
#define STR_BOARD_RF_NORESPONSE                        "\r\nAntenna[%d]: Board is not present."
#else
#define STR_BOARD_RF_NORESPONSE                        "\r\nAntenna[%d]: Board is ERR."
#endif
//MCP
#define STR_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY           "\r\nL2 to   MCP[%d] downlinkdata not empty."
#define STR_L2_TO_MCP_UPLINKPROF_NOT_EMPTY             "\r\nL2 to   MCP[%d] uplinkprof not empty."
#define STR_L2_TO_MCP_CONFIG_NOT_EMPTY                 "\r\nL2 to   MCP[%d] config not empty."
#define STR_L2_FROM_MCP_UPLINKDATA_NOT_FULL            "\r\nL2 from MCP[%d] uplinkdata not full."
#define STR_L2_FROM_MCP_UPLINKDATA_CHKSUM              "\r\nL2 from MCP[%d] uplinkdata checksum error."
#define STR_MCP_TO_L2_UPLINK_NOT_EMPTY                 "\r\nMCP[%d] to   L2 uplink not empty."
#define STR_MCP_FROM_L2_DOWNLINK_NOT_FULL              "\r\nMCP[%d] from L2 downlink not full."
#define STR_MCP_FROM_L2_UPPROF_NOT_FULL                "\r\nMCP[%d] from L2 upprof not full."
#define STR_MCP_FROM_L2_CONFIG_NOT_FULL                "\r\nMCP[%d] from L2 config not full."
#define STR_MCP_FROM_L2_DOWNLINK_CHKSUM                "\r\nMCP[%d] from L2 downlink checksum error."
#define STR_MCP_FROM_L2_UPPROF_CHKSUM                  "\r\nMCP[%d] from L2 upprof checksum error."
#define STR_MCP_FROM_L2_CONFIG_CHKSUM                  "\r\nMCP[%d] from L2 config checksum error."
#define STR_MCP_TO_FEP_NOT_EMPTY                       "\r\nMCP[%d] to   FEP[%d] not empty."
#define STR_MCP_FROM_FEP_NOT_FULL                      "\r\nMCP[%d] from FEP[%d] not full."
#define STR_MCP_FROM_FEP_CHKSUM                        "\r\nMCP[%d] from FEP[%d] checksum error."
#ifdef WBBU_CODE
#define STR_Core9_From_MCP_UplinkData_CHKSUM    "\r\nCore9 from MCP[%d] checksum error."

#define  STR_MCP_From_Core9_Downlink_Lost      "\r\nMCP[%d] from core9 Downlink_Lost."
#define  STR_MCP_From_Core9_UpProf_Lost    "\r\nMCP[%d] from core9 UpProf_Lost."
#define STR_MCP_From_Core9_Config_Lostl   "\r\nMCP[%d] from core9 Config_Lostl "
#define STR_MCP_From_Core9_Downlink_CHKSUM "\r\nMCP[%d] from core9 Downlink_CHKSUM."
#define STR_MCP_From_Core9_UpProf_CHKSUM  "\r\nMCP[%d] from core9 UpProf_CHKSUM."
#define STR_MCP_From_Core9_Config_CHKSUM  "\r\nMCP[%d] from core9 Config_CHKSUM."



#define  STR_FEP_From_MCP_DownlinkData_Lost   "\r\n fep[%d] from mcp[%d] checksum error."
#define  STR_FEP_From_MCP_DownlinkData_CHKSUM "\r\n fep[%d] from mcp[%d] checksum error."

#define    STR_MCP_From_FEP_Lost     "\r\nMCP[%d] from FEP[%d] checksum error."
#define    STR_MCP_From_FEP_CHKSUM   "\r\nMCP[%d] from FEP[%d] checksum error."
#endif
//AUX
#define STR_AUX_TO_L2_CONTROL_FAIL                     "\r\nAUX to L2 control fail"
#define STR_AUX_TO_L2_RANGING_BUF_NOT_EMPTY            "\r\nAUX to L2 ranging buffer not empty."
#define STR_L2_TO_AUX_CONTROL_FAIL                     "\r\nL2  to AUX control fail."

#define STR_L2_TO_AUX_WEIGHT_NOT_EMPTY                 "\r\nL2 to   AUX weight not empty.   time slot[%d]"
#define STR_L2_TO_AUX_CONFIG_NOT_EMPTY                 "\r\nL2 to   AUX config not empty.   time slot[%d]"
#define STR_L2_FROM_AUX_RANGING_NOT_FULL               "\r\nL2 from AUX ranging not full.   time slot[%d]"
#define STR_L2_FROM_AUX_RANGING_CHKSUM                 "\r\nL2 from AUX ranging checksum error. time slot[%d]"

#define STR_AUX_FROM_L2_BUF_NOT_FULL                   "\r\nAUX from L2 buffer not full.    time slot[%d]"
#define STR_AUX_FROM_L2_CHKSUM                         "\r\nAUX from L2 checksum error.     time slot[%d]"

#define STR_AUX_TO_FEP_BUF_NOT_EMPTY                   "\r\nAUX to  FEP[%d] buffer not empty.   time slot[%d]"

#define STR_FEP_FROM_AUX_BUF_NOT_FULL                  "\r\nFEP[%d] from AUX buffer not full.   time slot[%d]"
#define STR_FEP_FROM_AUX_CHKSUM                        "\r\nFEP[%d] from AUX checksum error.    time slot[%d]"

#define STR_AUX_FROM_FEP_BUF_NOT_FULL                  "\r\nAUX from FEP[%d] buffer not full"
#define STR_AUX_FROM_FEP_CHKSUM                        "\r\nAUX from FEP[%d] checksum error"
#define STR_FEP_TO_AUX_BUF_NOT_EMPTY                   "\r\nFEP[%d] to AUX buffer not empty"

//GPS
#define STR_GPS_MNT_FAILURE_CNT                        "\r\nFPGA GPS maintanence failure."
#define STR_GPS_TRACKIN_FAILURE_CNT                    "\r\nFPGA GPS tracking failure."
#define STR_GPS_LOST                                   "\r\nGPS tracking satellite number below configured threshold."
#define STR_GPS_CFG_ERROR                              "\r\nSync Src is not GPS."
#define STR_GPS_COMMUNICATE_FAIL                       "\r\nGPS communication failure."
#define STR_GPS_ANTENNA_NOT_CONNECTED                  "\r\nGPS antenna not connected."
#define STR_GPS_CFG_ERROR_485                              "\r\nSync Src is not GPS, is 485."

//FSC, by xiaoweifang
#define STR_PLLLOSELOCK_MINOR                          "\r\nPLL lose lock (minor)."
#define STR_PLLLOSELOCK_SERIOUS                        "\r\nPLL lose lock (serious)."
#define STR_TCXOFRENOFFSET                             "\r\nTCXO offset > 2.5ppm."
#define STR_FSC_FACTORY_DATA                           "\r\nSynthersize card bad factory data."
#define STR_FSC_SSP_CHECKSUM_ERROR                     "\r\nSynthersize card serial port communication checksum error."
#define STR_FSC_AUX2SYNC_CHECKSUM_ERROR                "\r\nSynthersize card AUX2SYNC checksum error."
#define STR_FSC_SYNC_NORESP_ERROR                      "\r\nSynthersize card nonexistence."

//ENV
#define STR_SYNC_TEMPERATURE                           "\r\nSYNC card temperature alarm.    too low alarms:%d, minor low alarms:%d, minor high alarms:%d, too high alarms:%d,"
#define STR_DIGITAL_BORD_TEMPERATURE                   "\r\nDigital board Temperature[%d degree] alarm."
#define STR_ENV_FAN_STOP                               "\r\nBTS fan stop running. Binary bit[0x%X],--bit0~2, 1:alarm; 0:normal."
#define STR_SYNC_SHUTDOWN                              "\r\nSYNC card SHUTDOWN alarm. temperature too low alarms:%d, minor low alarms:%d, minor high alarms:%d, too high alarms:%d,"

#ifdef WBBU_CODE
#define STR_Core9_From_AUX_Message_Lost   			 "\r\n Core9_From_AUX_Message_Lost"
#define STR_Core9_From_AUX_Message_CHKSUM   			"\r\nCore9_From_AUX_Message_CHKSUM."
#define STR_AUX_From_Core9_Config_Lost 	  		"\r\nAUX_From_Core9_Config_Lost."
#define STR_AUX_From_Core9_Config_CHKSUM  		 "\r\nAUX_From_Core9_Config_CHKSUM."
#define STR_AUX_From_FPGA_Message_Lost  		 "\r\nAUX_From_FPGA_Message_Lost "
#define STR_AUX_From_FPGA_Message_CHKSUM  		 "\r\nAUX_From_FPGA_Message_CHKSUM"
#define STR_FEP_From_AUX_Message_CHKSUM  		 "\r\nFEP[%d]_From_AUX_Message_CHKSUM."
 #define STR_AUX_From_FEP_Message_CHKSUM  		 "\r\nAUX_From_FEP[%d]_Message_CHKSUM."



#define STR_L2_From_Core9_UplinkData_Lost     "\r\n L2_From_Core9_UplinkData_Lost"
#define STR_L2_From_Core9_UplinkData_CHKSUM   "\r\n L2_From_Core9_UplinkData_CHKSUM "
#define STR_L2_From_Core9_Message_Lost   "\r\n L2_From_Core9_Message_Lost"
#define STR_L2_From_Core9_Message_CHKSUM   "\r\n L2_From_Core9_Message_CHKSUM"
#define STR_Core9_From_L2_UpProf_Lost   "\r\n Core9_From_L2_UpProf_Lost"
#define STR_Core9_From_L2_UpProf_CHKSUM   "\r\n Core9_From_L2_UpProf_CHKSUM"
#define STR_Core9_From_L2_DownlinkData_Lost   "\r\n Core9_From_L2_DownlinkData_Lost"
#define STR_Core9_From_L2_DownlinkData_CHKSUM   "\r\n Core9_From_L2_DownlinkData_CHKSUM"
#define STR_Core9_From_L2_Config_Lost   "\r\n Core9_From_L2_Config_Lost "
#define STR_Core9_From_L2_Config_CHKSUM   "\r\n Core9_From_L2_Config_CHKSUM"
#define STR_Core9_From_L2_Timeout   "\r\n Core9_From_L2_Timeout "

#define STR_FEP_From_L2_Weight_Lost  "\r\n FEP[%d]_From_L2_Weight_Lost"
#define STR_FEP_From_L2_Weight_CHKSUM   "\r\n  FEP[%d]_From_L2_Weight_CHKSUM "
#define   STR_gps_warning   "\r\nWBBU FPGA_gps_warning."

#define 	STR_tdd_fep_rx_warning  "\r\nWBBU FPGA tdd_fep_rx_warning."
#define	STR_frame_sync_warning  "\r\nWBBU FPGA frame_sync_warning."
#define	STR_sfp_los_warning   "\r\nWBBU FPGA sfp_los_warning."
#define	STR_sfp_fan_warning   "\r\nWBBU FPGA fan status warning( 0-stop,1-running)[fan1:%x,fan2:%x,fan3:%x,fan4:%x,fan5:%x]"
#define	STR_sfp_fan_warning_Recv   "\r\nWBBU FPGA fan_warning recovering"
#define  STR_WRRU_L3_COMM_FAIL  "\r\nWRRU_WBBU_COMM_FAIL."
#define STR_WRRU_RESET                           "\r\nWRRU Reset.."
#define STR_WRRU_TEMP                          "\r\nWRRU Temp Warn:[%d]."
#define STR_WRRU_CURRENT               "\r\nWRRU Current Overrun Warn:[%d A]."
#define 	STR_tdd_10ms_warning  "\r\nWBBU FPGA tdd_10ms_warning"
#define 	STR_AD4001_warning  "\r\nWBBU FPGA AD4001 LOCK FAIL."
#define STR_WRRU_ZBB             "\r\nWRRU ZhuBoBi channel[%d]."

#define STR_MCU_DOWNLOAD_WARN   "\r\nWRRU MCU_DOWNLOADWAR."

#define STR_FPGA_DOWNLOAD_WARN   "\r\nWRRU FPGA_DOWNLOADWAR."


#define STR_RRU_AFC4001_WARN    "\r\nWRRU AFC4001 Alarm."
#define  STR_WRRU_SYN_ERROR    "\r\nWRRU SYN ERROR[%d]."
//AIF告警
 #define STR_AIF_Dsp2_To_AUX_Linkerror    "\r\nAIF_Dsp2_To_AUX_Linkerror"
  #define STR_AIF_Dsp2_To_AUX_HeaderLost    "\r\nAIF_Dsp2_To_AUX_HeaderLostr"
#define STR_AIF_Dsp3_To_AUX_Linkerror    "\r\nAIF_Dsp2_To_AUX_Linkerror"
 #define STR_AIF_Dsp3_To_AUX_HeaderLost    "\r\nAIF_Dsp3_To_AUX_HeaderLost"
  #define STR_AIF_Dsp4_To_AUX_Linkerror    "\r\nAIF_Dsp4_To_AUX_Linkerror"
  #define STR_AIF_Dsp4_To_AUX_HeaderLost    "\r\nAIF_Dsp4_To_AUX_HeaderLost"
 #define STR_AIF_FPGA_To_AUX_Linkerror    "\r\nAIF_FPGA_To_AUX_Linkerror"
  #define STR_AIF_FPGA_To_AUX_Timeout    "\r\nAIF_FPGA_To_AUX_Timeout"
  #define STR_AIF_AUX_AIF_Reconfig    "\r\nAIF_AUX_AIF_Reconfig r"
  #define STR_AIF_Dsp4_To_Dsp2_Linkerror    "\r\nAIF_Dsp4_To_Dsp2_Linkerror"
  #define STR_AIF_Dsp4_To_Dsp2_HeaderLost    "\r\nAIF_Dsp2_To_AUX_Linkerror"
  #define STR_AIF_FEP_To_Dsp2_Linkerror    "\r\nAIF_FEP_To_Dsp2_Linkerror"
  #define STR_AIF_FEP_To_Dsp2_HeaderLost	    "\r\nAIF_FEP_To_Dsp2_HeaderLost"
 #define STR_AIF_Dsp2_AIF_Reconfig    "\r\nAIF_Dsp2_AIF_Reconfig"
 #define STR_AIF_Dsp4_To_Dsp3_Linkerror    "\r\nAIF_Dsp4_To_Dsp3_Linkerror"
 #define STR_AIF_Dsp4_To_Dsp3_HeaderLost    "\r\nAIF_Dsp4_To_Dsp3_HeaderLost "
 #define STR_AIF_FEP_To_Dsp3_Linkerror    "\r\nAIF_FEP_To_Dsp3_Linkerror "
 #define STR_AIF_FEP_To_Dsp3_HeaderLost    "\r\nAIF_FEP_To_Dsp3_HeaderLost"
 #define STR_AIF_Dsp3_AIF_Reconfig    "\r\nAIF_Dsp3_AIF_Reconfig "
 #define STR_AIF_FEP_To_Dsp4_Linkerror    "\r\nAIF_FEP_To_Dsp4_Linkerror"
 #define STR_AIF_FEP_To_Dsp4_HeaderLost    "\r\nAIF_FEP_To_Dsp4_HeaderLost "
  #define STR_AIF_AUX_To_Core9_Linkerror    "\r\nAIF_AUX_To_Core9_Linkerror"
 #define STR_AIF_AUX_To_Core9_HeaderLost    "\r\nAIF_AUX_To_Core9_HeaderLost"
#define STR_AIF_Dsp2_To_Core9_Linkerror    "\r\nAIF_Dsp2_To_Core9_Linkerror"
#define STR_AIF_Dsp2_To_Core9_HeaderLost    "\r\nAIF_Dsp2_To_Core9_HeaderLost"
#define STR_AIF_Dsp3_To_Core9_Linkerror    "\r\nAIF_Dsp3_To_Core9_Linkerror "
 #define STR_AIF_Dsp3_To_Core9_HeaderLost    "\r\nAIF_Dsp3_To_Core9_HeaderLost"
 #define STR_AIF_Dsp5_To_Core9_Linkerror    "\r\nAIF_Dsp5_To_Core9_Linkerror"
 #define STR_AIF_Dsp5_To_Core9_HeaderLost    "\r\nAIF_Dsp5_To_Core9_HeaderLost "
 #define STR_AIF_Core9_To_Dsp5_Handshake_Timeout    "\r\nAIF_Core9_To_Dsp5_Handshake_Timeout"
#define STR_AIF_AIF_Reconfig    "\r\nAIF_Reconfig"

#define STR_AUX_CAL_ERR_0    "\r\n Last Calibration Result Is Err"
#define STR_AUX_CAL_ERR_4   "\r\n Calibration Uplink  Msg CheckSum Err"
#define STR_AUX_CAL_ERR_5  "\r\n Calibration Dnlink  Msg CheckSum Err"
#define STR_AUX_CAL_ERR_6   "\r\n Calibration Wrru Not Enter Calibarion Processing"
#define STR_AUX_CAL_Right    "\r\n Calibration Result Is Ok"




#define STR_RRU_Fiber_Voltage_Warn    "\r\n RRU Fiber optic module Voltuage Low:%10.2f(uV) "
#define STR_RRU_Fiber_Current_Warn      "\r\n RRU Fiber optic module Current Low:%10.2f(uA)"
#define STR_RRU_Fiber_TX_PWR_Warn      "\r\n RRU Fiber optic module TX_Power Low:%10.2f(uW)"
#define STR_RRU_Fiber_RX_PWR_Warn      "\r\n RRU Fiber optic module Rx_Power Low :%10.2f(uW)"

#define STR_BBU_Fiber_Voltage_Warn          "\r\n BBU Fiber optic module Voltuage Low:%10.2f(uV) "
#define STR_BBU_Fiber_Current_Warn            "\r\n BBU Fiber optic module Current Low:%10.2f(uA)"
#define STR_BBU_Fiber_TX_PWR_Warn            "\r\n BBU Fiber optic module TX_Power Low:%10.2f(uW)"
#define STR_BBU_Fiber_RX_PWR_Warn             "\r\n BBU Fiber optic module Rx_Power Low :%10.2f(uW)"


#define STR_L3_Reset_DSP1_Warm       "\r\n BBU L3 Reset DSP1 Due to Cannot Get Version"
#define STR_L3_Reset_DSP2_Warm       "\r\n BBU L3 Reset DSP2 Due to Cannot Get Version"
#define STR_L3_Reset_DSP3_Warm       "\r\n BBU L3 Reset DSP3 Due to Cannot Get Version"
#define STR_L3_Reset_DSP4_Warm       "\r\n BBU L3 Reset DSP4 Due to Cannot Get Version"
#define STR_L3_Reset_DSP5_Warm       "\r\n BBU L3 Reset DSP5 Due to Cannot Get Version"


#define STR_BBU_RRU_MSG_WARN       "\r\n RRU to BBU MSg CheckSum ERR"

#endif
#pragma pack(1)

typedef struct 
{
    UINT16  TransId;
    UINT32  SequenceNum;    //  ID for the alarm
    UINT8   Flag;           //  0-  clear   1-  set 
    
    UINT16  Year;       
    UINT8   Month;
    UINT8   Day;
    UINT8   Hour;       
    UINT8   Minute;     
    UINT8   Second;     

    UINT16  EntityType;     //0x00 - 公共告警   0x01 - L3 PPC   0x02 - L2 PPC  0x03 - MCP
                            //0x04 - AUX        0x05 - ENV      0x06 - TCXO    0x07 - RF
                            //0x08  - GPS       0x09  - SYN     0x0A  - PLL
    UINT16  EntityIndex;        
    UINT16  AlarmCode;      
    UINT8   Severity;       //  1 - critical   2 - major   3 - minor   4 - informational .. 
    UINT16  InfoLen;        //  Alarm Info Length       
    SINT8   AlarmInfo[ALM_INFO_LEN];    //  Text to explain the alarm
}T_AlarmNotify;             


//RF State struct
struct T_RFChStateInfo
{
    UINT32 BoardVoltminorAlarm;
    UINT32 BoardVoltseriousAlarm;
    UINT32 BoardCurrminorAlarm;
    UINT32 BoardCurrseriousAlarm;
    UINT32 TTAVoltminorAlarm;
    UINT32 TTAVoltserirousAlarm;
    UINT32 TTACurrminorAlarm;
    UINT32 TTACurrSeriousAlarm;
    UINT32 TxPowerminorAlarm;
    UINT32 TxPowerSeriousAlarm;
    UINT32 RFDisable;
    UINT32 SSPChkErr;
    UINT32 RFChkErr;
    UINT32 RFNoRsp;

    UINT32 RSV1;                //RF channel is disabled by MCU/ARM
    UINT32 RSV2;                //RF channel is enabled by MCU/ARM
        
};

struct T_RFStateInfo
{ 
    T_RFChStateInfo RFChStateInfo[ANTENNA_NUM];
    UINT32  TCXOFrenOffset;
    UINT32  PLLLoseLock;
    UINT32  FlashErr;          //by xiaoweifang.
    UINT32  SSPChkErr;
    UINT32  AUX2SYNchecksumErr;
    UINT32  SynNoRsp;
    SINT16  temperature;    //  2   当前温度 
};

//MCP通信状态struct
struct T_McpFepStateInfo
{
    UINT32  MCP_To_FEP_Not_Empty;
    UINT32  MCP_From_FEP_Not_Full;
    UINT32  MCP_From_FEP_CHKSUM;
};

struct T_L2McpStateInfo
{
    UINT32  L2_To_MCP_DownlinkData_Not_Empty;
    UINT32  L2_To_MCP_UplinkProf_Not_Empty;
    UINT32  L2_To_MCP_Config_Not_Empty;
    UINT32  L2_From_MCP_UplinkData_Not_Full;
    UINT32  L2_From_MCP_UplinkData_CHKSUM;
};

struct T_McpL2StateInfo{
    UINT32  MCP_To_L2_Uplink_Not_Empty;
    UINT32  MCP_From_L2_Downlink_Not_Full;
    UINT32  MCP_From_L2_UpProf_Not_Full;
    UINT32  MCP_From_L2_Config_Not_Full;
    UINT32  MCP_From_L2_Downlink_CHKSUM;
    UINT32  MCP_From_L2_UpProf_CHKSUM;
    UINT32  MCP_From_L2_Config_CHKSUM;
    T_McpFepStateInfo McpFepStateInfo[FEP_NUM]; 
};

#ifdef WBBU_CODE
 struct T_Core9MCPInfo
{
        UINT32  Core9_From_MCP_UplinkData_Lost;
        UINT32 Core9_From_MCP_UplinkData_CHKSUM;
};

 struct T_MCPCore9Info
{
 UINT32  MCP_From_Core9_Downlink_Lost;
  UINT32 MCP_From_Core9_UpProf_Lost;
  UINT32 MCP_From_Core9_Config_Lostl;
  UINT32 MCP_From_Core9_Downlink_CHKSUM;
  UINT32 MCP_From_Core9_UpProf_CHKSUM;
  UINT32 MCP_From_Core9_Config_CHKSUM;
};

struct T_FEPFromMCP
{
        UINT32   FEP_From_MCP_DownlinkData_Lost;
 	 UINT32   FEP_From_MCP_DownlinkData_CHKSUM;
};

struct T_MCPFromFEP
{
        UINT32   MCP_From_FEP_Lost;
 	 UINT32   MCP_From_FEP_CHKSUM;
};
#endif
struct T_MCPStateInfo
{    
#ifndef WBBU_CODE
    UINT32  reserve;
    T_L2McpStateInfo  L2McpStateInfo[MCP_NUM];
    T_McpL2StateInfo  McpL2StateInfo[MCP_NUM];  
 #else
UINT32 RSV;


T_Core9MCPInfo  Core9MCPInfo[MCP_NUM];


  T_MCPCore9Info    MCPCore9Info[MCP_NUM];


T_FEPFromMCP   FEPFromMCPInfo[MCP_NUM][2];

T_MCPFromFEP   MCPFromFEPInfo[MCP_NUM][2];

  #endif
};
#ifdef WBBU_CODE
struct T_Fep_FromL2_Weight_Info
{
    UINT32  FEP_From_L2_Weight_Lost;
    UINT32  FEP_From_L2_Weight_CHKSUM;

};
struct T_Core9StateInfo
{
  UINT32  L2_From_Core9_UplinkData_Lost;
UINT32 L2_From_Core9_UplinkData_CHKSUM;
UINT32 L2_From_Core9_Message_Lost;
UINT32 L2_From_Core9_Message_CHKSUM;
UINT32 Core9_From_L2_UpProf_Lost;//没有实现
UINT32 Core9_From_L2_UpProf_CHKSUM;
UINT32 Core9_From_L2_DownlinkData_Lost;//没有实现
UINT32 Core9_From_L2_DownlinkData_CHKSUM;
UINT32 Core9_From_L2_Config_Lost;//没有实现
UINT32 Core9_From_L2_Config_CHKSUM;
UINT32 Core9_From_L2_Timeout;

  T_Fep_FromL2_Weight_Info  fepfromL2Weight[2];

};

struct T_AifStateInfo
{
   UINT32  Dsp2_To_AUX_Linkerror;
  UINT32 Dsp2_To_AUX_HeaderLost;
  UINT32 Dsp3_To_AUX_Linkerror;
  UINT32 Dsp3_To_AUX_HeaderLost;
  UINT32 Dsp4_To_AUX_Linkerror;
  UINT32 Dsp4_To_AUX_HeaderLost;
  UINT32 FPGA_To_AUX_Linkerror;
  UINT32 FPGA_To_AUX_Timeout;
  UINT32 AUX_AIF_Reconfig;
  UINT32 Dsp4_To_Dsp2_Linkerror;
  UINT32 Dsp4_To_Dsp2_HeaderLost;
  UINT32 FEP_To_Dsp2_Linkerror;
  UINT32 FEP_To_Dsp2_HeaderLost	;
  UINT32 Dsp2_AIF_Reconfig;
  UINT32 Dsp4_To_Dsp3_Linkerror;
  UINT32 Dsp4_To_Dsp3_HeaderLost;
  UINT32 FEP_To_Dsp3_Linkerror;
  UINT32 FEP_To_Dsp3_HeaderLost;
  UINT32 Dsp3_AIF_Reconfig;
  UINT32 FEP_To_Dsp4_Linkerror;
  UINT32 FEP_To_Dsp4_HeaderLost;
  UINT32 AUX_To_Core9_Linkerror;
  UINT32 AUX_To_Core9_HeaderLost;
  UINT32 Dsp2_To_Core9_Linkerror;
  UINT32 Dsp2_To_Core9_HeaderLost;
  UINT32 Dsp3_To_Core9_Linkerror;
  UINT32 Dsp3_To_Core9_HeaderLost;
  UINT32 Dsp5_To_Core9_Linkerror;
  UINT32 Dsp5_To_Core9_HeaderLost;
  UINT32 Core9_To_Dsp5_Handshake_Timeout;
  UINT32 AIF_Reconfig;

};
#endif
struct T_McpCommunicateTest
{
    T_L2McpStateInfo  L2McpStateInfo[DL_TIME_SLOT_NUM];
    T_McpL2StateInfo  McpL2StateInfo[DL_TIME_SLOT_NUM];  
};


//AUX通信状态struct
struct T_AuxFepStateInfo
{
    UINT32  AUX_To_FEP_Buf_Not_Empty;  //(i)    4   Downlink weight information
};

struct T_FepAuxStateInfo
{
    UINT32  FEP_From_AUX_Buf_Not_Full; //   4   
    UINT32  FEP_From_AUX_CHKSUM;       //   4   0-ok 1-fail
};

struct T_AuxFromFepStateInfo
{
    UINT32  AUX_From_FEP_Buf_Not_Full;//    4   
    UINT32  AUX_From_FEP_CHKSUM;//  4   
    UINT32  FEP_To_AUX_Buf_Not_Empty;// 4   Error message passing from FEP
};

struct T_L2AUXStateInfo
{
    UINT32  L2_To_AUX_Weight_Not_Empty;      // 4   
    UINT32  L2_To_AUX_Config_Not_Empty;      // 4   
    UINT32  L2_From_AUX_Ranging_Not_Full;    // 4   
    UINT32  L2_From_AUX_Ranging_CHKSUM;      // 4   
};

struct T_AUXL2StateInfo
{
    UINT32  AUX_From_L2_Buf_Not_Full;        // 4   0-false   1-true
    UINT32  AUX_From_L2_CHKSUM;              // 4   0-ok 1-fail
};

struct T_AuxCommunicateTest
{
    UINT32  AUX_To_L2_Control_Fail;          // #frame that control cannot be sent to L2.
                                             // Once control message go through the interface, AUX shall reset this to 0,
    UINT32  AUX_To_L2_Ranging_Buf_Not_Empty; // 4   0-false  1-true
    UINT32  L2_To_Aux_Control_Fail;          // 4   

    T_L2AUXStateInfo  L2AUXStateInfo[DL_TIME_SLOT_NUM];     
    T_AUXL2StateInfo  AUXL2StateInfo[DL_TIME_SLOT_NUM];
    
    T_AuxFepStateInfo AuxFepStateInfo[DL_TIME_SLOT_NUM][FEP_NUM]; 
    T_FepAuxStateInfo FepAuxStateInfo[DL_TIME_SLOT_NUM][FEP_NUM];
    
    T_AuxFromFepStateInfo AuxFromFepStateInfo[FEP_NUM];

};

struct T_AUXStateInfo
{
   #ifndef WBBU_CODE
    UINT32  GPS_MNT_FAILURE_CNT;             // 4   Record GPS failure times
    UINT32  GPS_TRACKIN_FAILURE_CNT;         // 4   Record FPGA lose lock times
    UINT32  TemperatureL;                    // < 5 4   
    UINT32  TemperatureH;                    // > 65    4   
    UINT32  TemperatureML;                   // < 0 4   
    UINT32  TemperatureMH;                   // > 70    4   

    UINT32  AUX_To_L2_Control_Fail;          // #frame that control cannot be sent to L2.
                                             // Once control message go through the interface, AUX shall reset this to 0,
    UINT32  AUX_To_L2_Ranging_Buf_Not_Empty; // 4   0-false  1-true
    UINT32  L2_To_Aux_Control_Fail;          // 4   

    T_L2AUXStateInfo  L2AUXStateInfo[DL_TIME_SLOT_NUM];     
    T_AUXL2StateInfo  AUXL2StateInfo[DL_TIME_SLOT_NUM];
    
    T_AuxFepStateInfo AuxFepStateInfo[DL_TIME_SLOT_NUM][FEP_NUM]; 
    T_FepAuxStateInfo FepAuxStateInfo[DL_TIME_SLOT_NUM][FEP_NUM];
    
    T_AuxFromFepStateInfo AuxFromFepStateInfo[FEP_NUM];
    UINT32  FAN_status;                     //0:normal; >0: abnormal;
    #else
	 UINT32	Core9_From_AUX_Message_Lost;
	 UINT32 Core9_From_AUX_Message_CHKSUM;
	 UINT32 AUX_From_Core9_Config_Lost;
	UINT32 AUX_From_Core9_Config_CHKSUM;
	UINT32 AUX_From_FPGA_Message_Lost;
	UINT32 AUX_From_FPGA_Message_CHKSUM;

	 UINT32  FEP_From_AUX_Message_CHKSUM[2];


	  UINT32 AUX_From_FEP_Message_CHKSUM[2];

  #endif
};

struct T_CpuWorkNofity
{
    UINT8 CpuType;
    UINT8 CpuIndex;
};

struct T_CpuAlarmNofity
{
    UINT8  CpuType;
    UINT8  CpuIndex;
    UINT16 AlmCode;
};

struct T_CpuResetNofity
{
    UINT8  CpuType;
    UINT8  CpuIndex;
};

#pragma pack()
#endif
