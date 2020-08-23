/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopMeasure.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/20/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_MEASURE_H__
#define __SNOOP_MEASURE_H__


/****************************************************
 *IN_TYPE: Snoop任务接收到的消息的性能统计类型
 ****************************************************/
typedef enum 
{
    //Traffic from EB.
    IN_TRAFFIC = 0,
    //TUNNEL REQUEST.
    IN_TUNNEL_ESTABLISH_REQ,
    IN_TUNNEL_TERMINATE_REQ,
    IN_TUNNEL_CHANGE_ANCHOR_REQ,
    IN_TUNNEL_SYNC_REQ,
    //TUNNEL RESPONSE.
    IN_TUNNEL_ESTABLISH_RESP,
    IN_TUNNEL_TERMINATE_RESP,
    IN_TUNNEL_CHANGE_ANCHOR_RESP,
    IN_TUNNEL_SYNC_RESP,
    //ROAM REQUEST
    IN_ROAM_REQ,
    //DAIB Synchronize Response.
    IN_DAIB_SYNC_RESP,

    IN_TYPE_MAX
}IN_TYPE;


/*************************************************
 *M_TYPE_STRLEN: 输出类型字符串的长度
 *************************************************/
#define M_TYPE_STRLEN    (20)


/*************************************************
 *strInType: IN_TYPE对应的字符串
 *增加新类型时，字符串长度不要超过20字节
 *************************************************/
const UINT8 strInType[ IN_TYPE_MAX ][ M_TYPE_STRLEN ] = {
    "In Traffic",
    //TUNNEL REQUEST
    "In TN EST REQ",
    "In TN TER REQ",
    "In TN CHG REQ",
    "In TN SYN REQ",
    //TUNNEL RESPONSE
    "In TN EST RESP",
    "In TN TER RESP",
    "In TN CHG RESP",
    "In TN SYN RESP",
    //ROAM REQUEST
    "In ROAM REQ",
    //DAIB Synchronize Response.
    "In DAIB SYN RESP"
};


/****************************************************
 *OUT_TYPE: Snoop任务发送消息的性能统计类型
 ****************************************************/
typedef enum 
{
    //Traffic to EB.
    OUT_TRAFFIC_EB = 0,
    //TUNNEL REQUEST.
    OUT_TUNNEL_ESTABLISH_REQ,
    OUT_TUNNEL_TERMINATE_REQ,
    OUT_TUNNEL_CHANGE_ANCHOR_REQ,
    OUT_TUNNEL_SYNC_REQ,
    //TUNNEL RESPONSE.
    OUT_TUNNEL_ESTABLISH_RESP,
    OUT_TUNNEL_TERMINATE_RESP,
    OUT_TUNNEL_CHANGE_ANCHOR_RESP,
    OUT_TUNNEL_SYNC_RESP,

    //DAIB Synchronize Request.
    OUT_DAIB_SYNC_REQ,

    OUT_TYPE_MAX
}OUT_TYPE;


/*************************************************
 *strOutType: OUT_TYPE对应的字符串
 *增加新类型时，字符串长度不要超过20字节
 *************************************************/
const UINT8 strOutType[ OUT_TYPE_MAX ][ M_TYPE_STRLEN ] = {
    "Out Traffic EB",
    //TUNNEL REQUEST
    "Out TN EST REQ",
    "Out TN TER REQ",
    "Out TN CHG REQ",
    "Out TN SYN REQ",
    //TUNNEL RESPONSE
    "Out TN EST RESP",
    "Out TN TER RESP",
    "Out TN CHG RESP",
    "Out TN SYN RESP",

    //DAIB Synchronize Response.
    "Out DAIB SYN REQ"
};
#endif/*__SNOOP_MEASURE_H__*/
