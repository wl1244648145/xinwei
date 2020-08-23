/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    btsCommon.h
 *
 * DESCRIPTION: This module defines types common for any processor and
 *              specifies the specifics for that processor's compiler.
 *
 * HISTORY:
 *
 *   Date       Author    Description
 *   ---------  ------    ----------------------------------------------------
 *   06/14/05   Hui Zhou  Initial file creation. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __BTS_COMMON_H__
#define __BTS_COMMON_H__

#include "os.h"
#include "btsTypes.h"
#include "BtsConfig.h"

#define M_Max_BCH_Num   10
#define M_Max_RRCH_Num  10
#define M_Max_RARCH_Num 20

#define M_Max_Preferred_BTS 7

#define M_Info_Broadcast_Period     10      // number of frames

#define M_BW_Reconfig_Threshold     80      // number of bits per frame

#define M_DEFAULT_BTS_PPC           0       // default power per channel
#define PPC_INVALID                 100

// BTS Tasks
#define FRK_TID_MAIN 0 //input task 
#define FRK_TID_L2_SEND 1
#define FRK_TID_L2_PROC 2
#define FRK_TID_L2_RANGING 3
#define FRK_TID_L2_CAL 4
#define FRK_TID_L2_ALARM  5
//L2 DIAG TASK ID
#define FRK_TID_L2_DIAG_MONITOR 6
#define FRK_TID_L2_DIAG_AGENT	7

//outside task id
#define FRK_TID_L1 10
#define FRK_TID_L3 11

/***************** L2 API ****************/
#define L2_SEND_TIMING 1 //processed by l2 sending task 
#define L2_L1_UL_RAW_DATA_BLOCK 2 
#define L2_L3_OAM_MSG 4 //processed by l2 oam task
#define L2_L3_DATA_MSG 5 /* processed by l2 processing task, on receiving this msg, frk input task call AppendMsgToL2ProcessingQueue()
to append it to l2 processing task's int mq, and send a L2_MSG_INCOMING to notify l2 processing task */
#define L2_L3_VOICE_MSG 6 //processed by l2 processing task, the same as L2_L3_DATA_MSG 
#define L2_L1_MSG 7 //processed by l2 processing task, MCP Ul Buffer Image
#define MODEM_L3_OAM_MSG 8 // ???


#define L2_RECV_TIMING 9 //processed by l2 sending task 

#define L2_FRAME_START_TIMING 10 //processed by BTS and CPE L2 main task
#define L2_CHANGE_ARQBLK_STATE 11 //processed by BTS and CPE L2 main task

//simulator msgs
#define MAC_PAC_RECV_TIMING 20 

/************************ frk core if ************************/
#ifdef __cplusplus
extern "C" {
#endif

void *L2Processing_create();
void L2Processing_init(void *arg);
int L2Processing_mproc(void *pmsg,int len,void *pEnv);
void L2Processing_tproc(void *td, void *pEnv);

void *L2Send_create();
void L2Send_init(void *arg);
int L2Send_mproc(void *pmsg,int len,void *pEnv);
void L2Send_tproc(void *td, void *pEnv);


void *L2AlarmSend_create();
void L2AlarmSend_init(void *arg);
int L2AlarmSend_mproc(void *pmsg,int len,void *pEnv);
void L2AlarmSend_tproc(void *td, void *pEnv);


void *L2UdpServerMonitor_create();
void L2UdpServerMonitor_init(void *arg);
int L2UdpServerMonitor_mproc(void *pmsg,int len,void *pEnv);
void L2UdpServerMonitor_tproc(void *td, void *pEnv);

void *L2UdpServerMain_create();
void L2UdpServerMain_init(void *arg);
int L2UdpServerMain_mproc(void *pmsg,int len,void *pEnv);
void L2UdpServerMain_tproc(void *td, void *pEnv);

// Bts Cpe Initialization
void btsCpeInitilize();

#ifdef __cplusplus
}
#endif

#endif



