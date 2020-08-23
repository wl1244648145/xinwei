/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataEBMeasure.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   11/24/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __EB_MEASURE_H__
#define __EB_MEASURE_H__

/*************************************************
 *FROMDIR: Performance Measure Direction (from)
 *************************************************/
typedef enum 
{
    EB_FROM_AI = 0,
    EB_FROM_WAN,
    EB_FROM_TDR,
    //Internal只用于性能统计,是内部Snoop后进行数据转发的方向，
    //注意修改strFromDir
    EB_FROM_INTERNAL,

    EB_FROM_MAX
}FROMDIR;


/*************************************************
 *strFromDir: FROMDIR对应的字符串
 *增加新方向时，不要超过10字节
 *************************************************/
#define M_FROM_STRLEN    (10)
const UINT8 strFromDir[EB_FROM_MAX][M_FROM_STRLEN] = {
    "From AI",       //EB_FROM_AI = 0,
    "From WAN",      //EB_FROM_WAN,
    "From TDR",      //EB_FROM_TDR,
                     //Internal只用于性能统计,是内部Snoop后进行数据转发的方向，
    "From Intr"      //EB_FROM_INTERNAL,
};


/*************************************************
 *TODIR: Performance Measure Direction (to)
 *************************************************/
typedef enum 
{
    EB_TO_AI = 0,
    EB_TO_WAN,
    EB_TO_TDR,

    EB_TO_MAX
}TODIR;


/*************************************************
 *strToDir: TODIR对应的字符串
 *增加新方向时，不要超过10字节
 *************************************************/
#define M_TODIR_STRLEN    (10)
const UINT8 strToDir[EB_TO_MAX][M_TODIR_STRLEN] = {
    "To AI",       //EB_TO_AI = 0,
    "To WAN",      //EB_TO_WAN,
    "To TDR",      //EB_TO_TDR,
};


/*************************************************
 *TRAFFICTYPE: traffic类型
 *************************************************/
typedef enum
{
    TYPE_TRAFFIC_DHCP = 0,
    TYPE_TRAFFIC_ARP,
    TYPE_TRAFFIC_PPPoEDISCOVERY,
    TYPE_TRAFFIC_TO_WAN,
    TYPE_TRAFFIC_TO_TDR,
    TYPE_TRAFFIC_TO_AI,
    TYPE_TRAFFIC_BROADCAST_WAN,
    TYPE_TRAFFIC_BROADCAST_TDR,
    TYPE_TRAFFIC_BROADCAST_AI,
    TYPE_TRAFFIC_UNKNOWN,
    TYPE_TRAFFIC_NOT_AUTHED,    //bIsAuthed = false;
    TYPE_TRAFFIC_BROADCAST_FORBIDDEN,   //m_bEgressBCFltrEn = false;
    TYPE_EGRESS_DHCPC,  //DHCP Client Packets to Egress.
    TYPE_LICENSE_USEDUP,// no free FTEntries.
    TYPE_ILLEGAL_USER,  // find no FTEntry.
    //other possible packet types.请注意修改strTrafficType
    TYPE_TRAFFIC_LOST,
    TYPE_TRAFFIC_MAX
}TRAFFICTYPE;


/*************************************************
 *strTrafficType: traffic类型对应的字符串
 *增加新类型时，不要超过20字节
 *************************************************/
#define M_TRAFFICTYPE_STRLEN    (20)
const UINT8 strTrafficType[TYPE_TRAFFIC_MAX][M_TRAFFICTYPE_STRLEN] = 
{
    "DHCP",             //TYPE_TRAFFIC_DHCP = 0,
    "ARP",              //TYPE_TRAFFIC_ARP,
    "PPPoEDisc",        //TYPE_TRAFFIC_PPPoEDISCOVERY,
    "To WAN",           //TYPE_TRAFFIC_TO_WAN,
    "To TDR",           //TYPE_TRAFFIC_TO_TDR,
    "To AI",            //TYPE_TRAFFIC_TO_AI,
    "BC Wan",           //TYPE_TRAFFIC_BROADCAST_WAN,
    "BC TDR",           //TYPE_TRAFFIC_BROADCAST_TDR,
    "BC AI",            //TYPE_TRAFFIC_BROADCAST_AI,
    "Unknown Packet",   //TYPE_TRAFFIC_UNKNOWN,
    "Not Authed",       //TYPE_TRAFFIC_NOT_AUTHED,
    "BC Forbidden",     //YPE_TRAFFIC_BROADCAST_FORBIDDEN,
    "Egrs DHCPC",       //TYPE_EGRESS_DHCPC,
    "License Usedup",   //TYPE_LICENSE_USEDUP,
    "Illegal User",     //TYPE_ILLEGAL_USER
    "Egrs Pkts lost"    //TYPE_TRAFFIC_LOST
                        //other possible packet types.
};


#endif/*__EB_MEASURE_H__*/
