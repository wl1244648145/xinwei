/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    L3DataSocketMsgIds.h
*
* DESCRIPTION: 
*
* HISTORY:
*
*   Date       Author        Description
*   ---------  ------        ------------------------------------------------
*   11/01/06   xinwang		 initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef _INC_TSOCKET_MSGIDS
#define _INC_TSOCKET_MSGIDS

//timer
#define M_TSOCKET_TIMER_FT_EXPIRE		 0xFF01
#define M_TSOCKET_TIMER_CB_EXPIRE		 0xFF02
//lijinan 090105 for jamming rpt flow ctl
#define M_TSOCKET_TIMER_JAMMINGL2_EXPIRE		 0xFF03
#define M_TSOCKET_TIMER_JAMMINGBTS_EXPIRE		 0xFF04


//message area
#define MSGAREA_EMS				         0x0001
#define MSGAREA_TUNNEL_MANAGEMENT        0x0002
#define MSGAREA_TUNNEL_DATA              0x0003
#define MSGAREA_LOADING_INFO             0x0004
#define MSGAREA_DIAG                     0x0005
#define MSGAREA_JAMMING_REPORT           0x0006
#define MSGAREA_JAMMING_REPORT_RSP       0x0007
#define MSGAREA_PAIREDCPE_PROF           0x0008
#define MSGAREA_NEIGHBOR_BTS             0x0009
#define MSGAREA_GRPSRV_HORESREQ		0x000A
#define MSGAREA_GRPSRV_HORESRSP		0x000B
#if 1//def M_CLUSTER_SAME_F
#define MSGAREA_GROUP_RESOURCE_RPT_RSP		0x000C
#endif

//zengjihan 20120801 for GPSSYNC
#ifdef WBBU_CODE
#define MSGAREA_GPS     0x000D
#endif

#define M_TSOCKET_BTSIP_NOTIFY           0x00C1
#define M_TSOCKET_BTSIP_REQ              0x00C2
#define M_TSOCKET_BTSIP_RSP              0x00C3


#endif/*_INC_TSOCKET_MSGIDS*/