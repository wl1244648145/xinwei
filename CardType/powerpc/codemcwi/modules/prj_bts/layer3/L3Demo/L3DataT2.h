/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTDRMeasure.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   11/22/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __TDR_MEASURE_H__
#define __TDR_MEASURE_H__


/****************************************************
 *TDR_MEASURE_TYPE: TDR任务性能统计类型
 ****************************************************/
typedef enum 
{
    //发送的请求
    MEASURE_TDR_SOCK_ERR,
    MEASURE_TDR_ETHERIP_PACKETS,
    MEASURE_TDR_NO_BUFFER,

    MEASURE_TDR_MAX
}TDR_MEASURE_TYPE;


/*************************************************
 *M_TYPE_STRLEN: 输出类型字符串的长度
 *************************************************/
#define M_TYPE_STRLEN    (20)


/*************************************************
 *strTDRMeasureType: TDR_MEASURE_TYPE对应的字符串
 *增加新类型时，字符串长度不要超过20字节
 *************************************************/
const UINT8 strTDRMeasureType[ MEASURE_TDR_MAX ][ M_TYPE_STRLEN ] = {
    "Socket Err",    //MEASURE_TDR_SOCK_ERR,
    "Rx EtherIp Packets",    //MEASURE_TDR_ETHERIP_PACKETS,
    "Buffer Exhausted",    //MEASURE_TDR_NO_BUFFER,
};


#endif/*__TDR_MEASURE_H__*/
