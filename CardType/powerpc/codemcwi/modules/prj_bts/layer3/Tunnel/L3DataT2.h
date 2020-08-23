/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelMeasure.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   11/18/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __TUNNEL_MEASURE_H__
#define __TUNNEL_MEASURE_H__


/****************************************************
 *TUNNEL_MEASURE_TYPE: TUNNEL任务性能统计类型
 ****************************************************/
typedef enum 
{
    //发送的请求
    MEASURE_TX_ESTABLISH_REQ = 0,
    MEASURE_TX_TERMINATE_REQ,
    MEASURE_TX_SYNC_REQ,
    MEASURE_TX_CHANGE_ANCHOR_REQ,
    //收到的回应
    MEASURE_RX_ESTABLISH_RESPONSE,
    MEASURE_RX_TERMINATE_RESPONSE,
    MEASURE_RX_SYNC_RESPONSE,
    MEASURE_RX_CHANGE_ANCHOR_RESPONSE,
    //收到的请求
    MEASURE_RX_ESTABLISH_REQ,
    MEASURE_RX_TERMINATE_REQ,
    MEASURE_RX_SYNC_REQ,
    MEASURE_RX_CHANGE_ANCHOR_REQ,
    //发送的回应
    MEASURE_TX_ESTABLISH_RESPONSE,
    MEASURE_TX_TERMINATE_RESPONSE,
    MEASURE_TX_SYNC_RESPONSE,
    MEASURE_TX_CHANGE_ANCHOR_RESPONSE,

    MEASURE_TUNNEL_MAX
}TUNNEL_MEASURE_TYPE;


/*************************************************
 *M_TYPE_STRLEN: 输出类型字符串的长度
 *************************************************/
#define M_TYPE_STRLEN    (20)


/*************************************************
 *strTunnelMeasureType: TUNNEL_MEASURE_TYPE对应的字符串
 *增加新类型时，字符串长度不要超过20字节
 *************************************************/
const UINT8 strTunnelMeasureType[ MEASURE_TUNNEL_MAX ][ M_TYPE_STRLEN ] = {
    //发送的请求
    "Tx Establish Req",     //MEASURE_TX_ESTABLISH_REQ = 0,
    "Tx Terminate Req",     //MEASURE_TX_TERMINATE_REQ,
    "Tx Sync Req",          //MEASURE_TX_SYNC_REQ,
    "Tx Chg Anchor Req",    //MEASURE_TX_CHANGE_ANCHOR_REQ,
    //收到的回应
    "Rx Establish Resp",    //MEASURE_RX_ESTABLISH_RESPONSE,
    "Rx Terminate Resp ",   //MEASURE_RX_TERMINATE_RESPONSE,
    "Rx Sync Resp",         //MEASURE_RX_SYNC_RESPONSE,
    "Rx Chg Anchor Resp",   //MEASURE_RX_CHANGE_ANCHOR_RESPONSE,
    //收到的请求
    "Rx Establish Req",     //MEASURE_RX_ESTABLISH_REQ,
    "Rx Terminate Req",     //MEASURE_RX_TERMINATE_REQ,
    "Rx Sync Req",          //MEASURE_RX_SYNC_REQ,
    "Rx Chg Anchor Req",    //MEASURE_RX_CHANGE_ANCHOR_REQ,
    //发送的回应
    "Tx Establish Resp",    //MEASURE_TX_ESTABLISH_RESPONSE,
    "Tx Terminate Resp ",   //MEASURE_TX_TERMINATE_RESPONSE,
    "Tx Sync Resp",         //MEASURE_TX_SYNC_RESPONSE,
    "Tx Chg Anchor Resp",   //MEASURE_TX_CHANGE_ANCHOR_RESPONSE,
};


#endif/*__TUNNEL_MEASURE_H__*/
