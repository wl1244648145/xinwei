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
#ifndef _INC_L3OAMCOMMON
#define _INC_L3OAMCOMMON

#ifndef DATATYPE_H
#include "datatype.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#define BTS_SYNC_SRC_BTS    0x00
#define BTS_SYNC_SRC_GPS    0x01
#define BTS_SYNC_SRC_485    0x05


//0 - L2,  1 - AUX, 2-MCP
#define BTS_CPU_TYPE_L2PPC  0x00
#define BTS_CPU_TYPE_AUX    0x01
#define BTS_CPU_TYPE_MCP    0x02
#define BTS_CPU_TYPE_FEP    0x03
#define BTS_CPU_TYPE_L1     0x04
#define BTS_CPU_TYPE_BTS    0x05

#define BTS_CPU_ELEMENT_ALL 0x7F

#define BTS_CPU_INDEX_MCP0  0x0
#define BTS_CPU_INDEX_MCP1  0x1
#define BTS_CPU_INDEX_MCP2  0x2
#define BTS_CPU_INDEX_MCP3  0x3
#define BTS_CPU_INDEX_MCP4  0x4
#define BTS_CPU_INDEX_MCP5  0x5
#define BTS_CPU_INDEX_MCP6  0x6
#define BTS_CPU_INDEX_MCP7  0x7

#define BTS_CPU_INDEX_FEP0  0x0
#define BTS_CPU_INDEX_FEP1  0x1

///////////////////////////////////////////////////////////
const UINT8  ENCRYPED_BTSID_LEN    = 32;
const UINT16 SIZEOF_TRANSID        = 2;
const UINT16 SIZE_KBYTE            = 1024;
const UINT16 OAM_SUCCESS           = 0;
const UINT16 OAM_FAILURE           = 1;
const UINT16 OAM_DEFAUIT_TRANSID   = 0XFFFF;
const UINT16 OAM_TIMEOUT_ERR       = 0X0001;      //定时器超时

const UINT8  M_TIMER_PERIOD_NOT    = 0;
const UINT8  M_TIMER_PERIOD_YES    = 1;
const UINT32 IP_HEAD_SIZE          = 20;
const UINT32 UDP_HEAD_SIZE         = 8;
const UINT32 EMS_BTS_MSGHEAD_SIZE  = 10;

#ifdef OAM_DEBUG
const UINT32 OAM_REQ_RSP_PERIOD    = (50 * 60* 1000);       
#else
const UINT32 OAM_REQ_RSP_PERIOD    = (15 * 1000);   //( LJF 5->10 )    
#endif
const UINT32 OAM_REQ_RESEND_CNT1   = 1; 
const UINT32 OAM_REQ_RESEND_CNT3   = 3; 
#ifndef WBBU_CODE
const UINT32 OAM_REQ_RESEND_L2_L3_CNT40 = 38;
#else
const UINT32 OAM_REQ_RESEND_L2_L3_CNT40 = 10;
#endif
const UINT8  L1_CAL_DATA_CNTS      = 32;
const UINT8  NVRAM_WRITE_ENABLE    = 1;
const UINT8  NVRAM_WRITE_DISABLE   = 0;
const UINT8  TASK_INIT_FROM_NVRAM        = 1;
const UINT8  TASK_INIT_FROM_EMS     = 0;

const UINT8  FILE_CPE_CODE_IMG     = 0;
const UINT8  FILE_BTS_CODE_IMG     = 1;
const UINT8  FILE_Z_CODE_IMG       = 2;

const UINT16 BTS_RESET_CODE_UPDATE = 0;

const UINT16 gDefaultPrdRegTime    = 24;    //1 Day
const UINT16 gMAXPrdRegTime        = 7*24;	//7 Days

const UINT32 SCG_NUM               = 5;
const UINT32 NEIGHBOR_BTS_NUM      = 20;
const UINT32 RACH_RARCH_CFG_NUM    = 20;
const UINT32 BCH_INFO_NUM          = 10;
const UINT32 RRCH_INFO_NUM         = 10;
const UINT32 FEP_NUM               = 2;
const UINT32 MCP_NUM             = 8;
const UINT32 DL_TIME_SLOT_NUM      = 8; 
const UINT32 ANTENNA_NUM           = 8; 
const UINT32 SUB_CARRIER_NUM       = 640;
const UINT8  FILE_NAME_LEN         = 60;//jy081202,now file name may be longer than 30
const UINT8  RESEND_CNT1           = 1;
const UINT16 MAX_TOS_ELE_NUM       = 256;
const UINT16 CALIBRAT_DATA_ELE_NUM = 32;
const UINT16 MAX_ACL_CFG_NUM       = 50;
const UINT16 MAX_SFID_CFG_CNT      = 256;
const UINT8  MAC_ADDR_LEN          = 6;
const UINT8  MAX_FIX_IP_NUM        = 20;
const UINT8  MAX_ACL_CFG_SIZE      = 150;
const UINT16 CALI_ERRFLAG_WORD_CNT = 9;
const UINT16 PRED_H_WORD_CNT       = 12;

const UINT16 WITHOUT_PROFILEDATA   = 0;
const UINT16 WITH_PROFILEDATA      = 1;

const UINT8   CPE_ADM_STATUS_ADM        = 0;
const UINT8   CPE_ADM_STATUS_SUS        = 1;
const UINT8   CPE_ADM_STATUS_FLOW_LIMITED       = 0x11;
const UINT8   CPE_ADM_STATUS_NOPAY        = 0x10;

const UINT8   CPE_ADM_STATUS_NOROAM_CID = 0x06;
const UINT8   CPE_ADM_STATUS_PID_UID_ERR= 0x0b;
const UINT8   CPE_ADM_STATUS_NOROAM_LAI = 0x0c;
const UINT8   CPE_ADM_STATUS_NOROAM_BID = 0x0d;
const UINT8   CPE_ADM_STATUS_INV        = 0xFF;
const UINT8 CPE_ADM_STATUS_SAG_DOWN = 0x04;//添加原因值4,jiaying20100720

const UINT16 IPADDR_LEN             = 16;
const UINT16 FILEVER_STR_LEN        = 16;   // bts."111.222.333.444".bin
const UINT16 USER_NAME_LEN          = 10;
const UINT16 USER_PASSWORD_LEN      = 10;
const UINT16 FILE_DIRECTORY_LEN     = 50;
const UINT16 MAX_SWPACK_BC_SIZE        = 200; //BC 1024过于长
const UINT16 MAX_SWPACK_UC_SIZE        = 1024;
const UINT16 MAX_HWVER_LIST_NUM     = 10;  //暂时定为10个。
const UINT16 DEVICE_HW_VER_SIZE     = 8;

//const UINT8  UTHW_TYPE_CPE          = 0;   // 0:CPE    1:HS    2:PCCARD
//const UINT8  UTHW_TYPE_HS           = 1;
//const UINT8  UTHW_TYPE_PCCARD       = 2;
/************************************************/
/* HardType elucidation:                        */
/*                                              */
/* X   X   X   X   X   X   X   X                */
/* |   |   |   |   |   |   |   |                */
/* z  rsv rsv rda  |-----------|                */
/* nz rsv rsv max  |handset/pcmcia/cpe1/cpe5m   */
/* z:1 nz:0 rda:1 max:0                         */
/*handset:0x01 pcmcia:0x02 cpe1m:0x03 cpe5m:0x04*/
/************************************************/
const UINT8  UTHW_TYPE_MAX            = 0x00;
const UINT8  UTHW_TYPE_RDA            = 0x01;

const UINT8  UTHW_TYPE_NZBOX          = 0x00;
const UINT8  UTHW_TYPE_ZBOX           = 0x01;

const UINT8  UTHW_TYPE_HANDSET        = 0x01;
const UINT8  UTHW_TYPE_PCMCIA         = 0x02;
const UINT8  UTHW_TYPE_CPE_1M         = 0x03;
const UINT8  UTHW_TYPE_CPE_5M         = 0x04;
const UINT8  RPTHW_TYPE_3B              = 0x3B;
const UINT8  RPTHW_TYPE_3C              = 0x3C;
#ifdef MZ_2ND
const UINT8  HW_TYPE_ZBOX_7C          = 0x7C;
const UINT8  HW_TYPE_CPEZ_77          = 0x77;
#endif

const SINT8  APPCODE_POSTFIX[5]       = ".BIN"; 
const SINT8  APPCODE_BTS_PREFIX[5]    = "BTS."; 
const SINT8  APPCODE_CPE_PREFIX[5]    = "CPE."; 
const SINT8  APPCODE_HS_PREFIX[5]     = "HS."; 
const SINT8  APPCODE_PCCARD_PREFIX[8] = "PCCARD."; 

#ifndef __WIN32_SIM__
void OAM_LOGSTR (LOGLEVEL level, UINT32 errcode, const char* text);
void OAM_LOGSTR1(LOGLEVEL level, UINT32 errcode, const char* text, int arg1);
void OAM_LOGSTR2(LOGLEVEL level, UINT32 errcode, const char* text, int arg1, int arg2);
void OAM_LOGSTR3(LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2, int arg3);
void OAM_LOGSTR4(LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2, int arg3, int arg4);
#else
void OAM_LOGSTR (char *filename, int lineno, LOGLEVEL level, UINT32 errcode, const char* text);
void OAM_LOGSTR1(char *filename, int lineno, LOGLEVEL level, UINT32 errcode, const char* text, int arg1);
void OAM_LOGSTR2(char *filename, int lineno, LOGLEVEL level, UINT32 errcode, const char* text, int arg1, int arg2);
void OAM_LOGSTR3(char *filename, int lineno, LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2, int arg3);
void OAM_LOGSTR4(char *filename, int lineno, LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2, int arg3, int arg4);
#endif

#endif 
