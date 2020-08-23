/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMsgId.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   08/01/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __L3_DATAMSGID_H__
#define __L3_DATAMSGID_H__

#include "L3L2MessageId.h"
#include "L3EmsMessageId.h"

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//////////////////BTS 数据任务间////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
/**************************************
 *BTS Data Service任务间(IpList)管理消息
 **************************************/
#define MSGID_IPLIST_SYNC_REQ               (0x2300)
#define MSGID_IPLIST_SYNC_RESP              (0x2301)
#define MSGID_IPLIST_ARP_ADD_NOTIFICATION   (0x2302)
#define MSGID_IPLIST_ARP_DEL_NOTIFICATION   (0x2303)
#define MSGID_IPLIST_ADD_FIXIP              (0x2304)
#define MSGID_IPLIST_DELETE                 (0x2305)
#define MSGID_IPLIST_PUBIP_MODIFY           (0x2306)  
#define MSGID_IPLIST_DELETE_BY_CPE          (0x2307)

/****************************************
 *BTS Data Service任务间(隧道)管理消息
 ****************************************/
#define MSGID_TUNNEL_TERMINATE_REQ          (0x2310)
#define MSGID_TUNNEL_TERMINATE_RESP         (0x2311)
#define MSGID_TUNNEL_ESTABLISH_REQ          (0x2312)
#define MSGID_TUNNEL_ESTABLISH_RESP         (0x2313)
#define MSGID_TUNNEL_SYNC_REQ               (0x2314)
#define MSGID_TUNNEL_SYNC_RESP              (0x2315)
#define MSGID_TUNNEL_CHANGE_ANCHOR_REQ      (0x2316)
#define MSGID_TUNNEL_CHANGE_ANCHOR_RESP     (0x2317)
#define MSGID_TUNNEL_MSGFROM_TCR            (0x2318)
#define MSGID_TUNNEL_HEARTBEAT              (0x2319)
#define MSGID_TUNNEL_HEARTBEAT_RESP         (0x231A)
#define MSGID_TUNNEL_DELETE_TIMER           (0x231B) /***wangwenhua add 20080606***/


/************************************************
 *BTS Data Service任务间(FORWARDING TABLE)管理消息
 ************************************************/
#define MSGID_FT_ENTRY_EXPIRE               (0x2320)
#define MSGID_FT_ADD_ENTRY                  (0x2321)
#define MSGID_FT_DEL_ENTRY                  (0x2322)
#define MSGID_FT_UPDATE_ENTRY               (0x2323)
#define MSGID_FT_CHECK_VLAN                 (0x2324)
#define MSGID_FT_MODIFY_BTSPUBIP            (0x2325)



/**************************************
 *BTS Data Service任务间(ROAM)处理消息
 **************************************/
#define MSGID_ROAM_REQ                      (0x2330)
#define MSGID_ROAM_RESP                     (0x2331)


/**************************************
 *BTS Data Service任务间(ARP)处理消息
 **************************************/
#define MSGID_ARP_PROXY_REQ                 (0x2340)
#define MSGID_ARP_ADD_ENTRY                 (0x2341)



/*****************************************
 *BTS Data Service任务间(TRAFFIC)处理消息
 *****************************************/
#define MSGID_TRAFFIC_SNOOP_REQ             (0x2350)
#define MSGID_TRAFFIC_ETHERIP               (0x2351)
//#define MSGID_TRAFFIC_INGRESS               (0x2352)
#define MSGID_TRAFFIC_EGRESS                (0x2353)
#define MSGID_TRAFFIC_FORWARD               (0x2354)
#define MSGID_TRAFFIC_IPSTACK                 (0x2355)

/**************************************
 *BTS Data Service任务间(TIMER)消息
 *定义各任务定时器超时时往
 *本任务发送的消息ID
 **************************************/
#define MSGID_TIMER_SNOOP                   (0x2360)
#define MSGID_TIMER_DM                      (0x2361)
#define MSGID_TIMER_TUNNEL                  (0x2362)
#define MSGID_TIMER_UTDM                    (0x2363)
#define MSGID_TIMER_HEARTBEAT               (0x2364)


/**************************************
 *BTS Data Service任务间(EID表)管理消息
 **************************************/
#define MSGID_EID_DEL_TABLE                 (0x2370)

/************************************************
*BTS Data Service任务(MAC FILTER TABLE)管理消息
************************************************/
#define MSGID_MFT_ENTRY_EXPIRE              (0x2380)
#define MSGID_MFT_ADD_ENTRY                 (0x2381)
#define MSGID_MFT_DEL_ENTRY                 (0x2382)
#define MSGID_MFT_UPDATE_ENTRY              (0x2383)
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
///////////////BTS 数据任务<----> CPE数据任务///////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#define MSGID_UT_BTS_DATASERVICE_REQ                (0x6700)
#define MSGID_UT_BTS_DATASERVICE_RESP               (0x6701)
#define MSGID_UT_BTS_DAIBUPDATE_REQ                 (0x6702)
#define MSGID_UT_BTS_DAIBUPDATE_RESP                (0x6703)
#define MSGID_UT_BTS_ADDRFLTR_TABLEUPDATE_REQ       (0x6704)
#define MSGID_UT_BTS_ADDRFLTR_TABLEUPDATE_RESP      (0x6705)
#define MSGID_UT_BTS_PROTOCOLFLTR_TABLEUPDATE_REQ   (0x6706)
#define MSGID_UT_BTS_PROTOCOLFLTR_TABLEUPDATE_RESP  (0x6707)
#define MSGID_UT_BTS_DATACONFIG_REQ                 (0x6708)
#define MSGID_UT_BTS_DATACONFIG_RESP                (0x6709)

//lijinan 20101020 for video 
#define  MSGID_VEDIO_IPADDRESS_REPORT     0x670a
#define  MSGID_VEDIO_IPADDRESS_RESULT      0x670b

#define MSG_BTS_TO_CB3000  0x3000
#define MSG_BTS_TO_CB3000_FAIL  0x3001

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
///////////////////////CPE数据任务间////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/**************************************
 *BTS Data Service任务间(IpList)管理消息
 **************************************/
#define MSGID_UTDM_ENABLE                           (0x0020)
#define MSGID_UTDM_DISABLE                          (0x0021)

/**************************************
 *MSGID_TRAFFIC_INGRESS: 上行数据消息ID
 **************************************/
#define MSGID_TRAFFIC_INGRESS                       (0x0100)

#endif /*__L3_DATAMSGID_H__*/
