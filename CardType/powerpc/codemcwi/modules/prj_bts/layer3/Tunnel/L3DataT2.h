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
 *TUNNEL_MEASURE_TYPE: TUNNEL��������ͳ������
 ****************************************************/
typedef enum 
{
    //���͵�����
    MEASURE_TX_ESTABLISH_REQ = 0,
    MEASURE_TX_TERMINATE_REQ,
    MEASURE_TX_SYNC_REQ,
    MEASURE_TX_CHANGE_ANCHOR_REQ,
    //�յ��Ļ�Ӧ
    MEASURE_RX_ESTABLISH_RESPONSE,
    MEASURE_RX_TERMINATE_RESPONSE,
    MEASURE_RX_SYNC_RESPONSE,
    MEASURE_RX_CHANGE_ANCHOR_RESPONSE,
    //�յ�������
    MEASURE_RX_ESTABLISH_REQ,
    MEASURE_RX_TERMINATE_REQ,
    MEASURE_RX_SYNC_REQ,
    MEASURE_RX_CHANGE_ANCHOR_REQ,
    //���͵Ļ�Ӧ
    MEASURE_TX_ESTABLISH_RESPONSE,
    MEASURE_TX_TERMINATE_RESPONSE,
    MEASURE_TX_SYNC_RESPONSE,
    MEASURE_TX_CHANGE_ANCHOR_RESPONSE,

    MEASURE_TUNNEL_MAX
}TUNNEL_MEASURE_TYPE;


/*************************************************
 *M_TYPE_STRLEN: ��������ַ����ĳ���
 *************************************************/
#define M_TYPE_STRLEN    (20)


/*************************************************
 *strTunnelMeasureType: TUNNEL_MEASURE_TYPE��Ӧ���ַ���
 *����������ʱ���ַ������Ȳ�Ҫ����20�ֽ�
 *************************************************/
const UINT8 strTunnelMeasureType[ MEASURE_TUNNEL_MAX ][ M_TYPE_STRLEN ] = {
    //���͵�����
    "Tx Establish Req",     //MEASURE_TX_ESTABLISH_REQ = 0,
    "Tx Terminate Req",     //MEASURE_TX_TERMINATE_REQ,
    "Tx Sync Req",          //MEASURE_TX_SYNC_REQ,
    "Tx Chg Anchor Req",    //MEASURE_TX_CHANGE_ANCHOR_REQ,
    //�յ��Ļ�Ӧ
    "Rx Establish Resp",    //MEASURE_RX_ESTABLISH_RESPONSE,
    "Rx Terminate Resp ",   //MEASURE_RX_TERMINATE_RESPONSE,
    "Rx Sync Resp",         //MEASURE_RX_SYNC_RESPONSE,
    "Rx Chg Anchor Resp",   //MEASURE_RX_CHANGE_ANCHOR_RESPONSE,
    //�յ�������
    "Rx Establish Req",     //MEASURE_RX_ESTABLISH_REQ,
    "Rx Terminate Req",     //MEASURE_RX_TERMINATE_REQ,
    "Rx Sync Req",          //MEASURE_RX_SYNC_REQ,
    "Rx Chg Anchor Req",    //MEASURE_RX_CHANGE_ANCHOR_REQ,
    //���͵Ļ�Ӧ
    "Tx Establish Resp",    //MEASURE_TX_ESTABLISH_RESPONSE,
    "Tx Terminate Resp ",   //MEASURE_TX_TERMINATE_RESPONSE,
    "Tx Sync Resp",         //MEASURE_TX_SYNC_RESPONSE,
    "Tx Chg Anchor Resp",   //MEASURE_TX_CHANGE_ANCHOR_RESPONSE,
};


#endif/*__TUNNEL_MEASURE_H__*/
