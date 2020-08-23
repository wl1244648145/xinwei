/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataARPMeasure.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   11/28/05   yang huawei   initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __ARP_MEASURE_H__
#define __ARP_MEASURE_H__

/*************************************************
 *ARPMEASURE: Performance Measure Direction (from)
 *************************************************/
typedef enum {
    ARP_FROM_AI = 0,
    ARP_FROM_WAN,
    ARP_FROM_TDR,
    ARP_FROM_MAX
}ARPFROMMEASURE;

typedef enum {
    ARP_TO_AI = 0 ,
    ARP_TO_WAN,
    ARP_TO_TDR,
    ARP_UNKNOW,  //
    ARP_REPLY,
    ARP_TO_MAX
}ARPTOMEASURE;
/*************************************************
 *strARPFromDir: FROMDIR对应的字符串
 *增加新方向时，不要超过10字节
 *************************************************/
#define M_FROMDIR_STRLEN    (15)
const UINT8 strARPFromDir[ARP_FROM_MAX][M_FROMDIR_STRLEN] = {
    "AI",      //ARP_FROM_AI = 0,
    "WAN",     //ARP_FROM_WAN,
    "TDR"     //ARP_FROM_TDR,
};
const UINT8 strARPToDir[ARP_TO_MAX][M_FROMDIR_STRLEN] = {
    "TO AI",      //ARP_FROM_AI = 0,
    "TO WAN",     //ARP_FROM_WAN,
    "TO TDR",     //ARP_FROM_TDR,
    "TO UNKNOW",
    "Reply"
    //ARP_FROM_AI = 0,
};

#endif/*__ARP_MEASURE_H__*/
