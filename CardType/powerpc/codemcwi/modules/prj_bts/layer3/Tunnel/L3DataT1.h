/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelErrCode.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   11/17/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __TUNNEL_ERR_CODE_H__
#define __TUNNEL_ERR_CODE_H__

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//TUNNEL�����붨��
const UINT16 EC_TUNNEL_NORMAL              = 0x0000;  //��������
const UINT16 EC_TUNNEL_UNEXPECTED_MSGID    = 0x0001;  //�Ƿ�Message ID
const UINT16 EC_TUNNEL_SYS_ERR             = 0x0002;  //ϵͳ����
const UINT16 EC_TUNNEL_SOCKET_ERR          = 0x0003;  //Socket����
const UINT16 EC_TUNNEL_PARAMETER           = 0x0004;  //��������
const UINT16 EC_TUNNEL_CB_USEDUP           = 0x0005;  //CB�ù�
const UINT16 EC_TUNNEL_CB_INDEX_ERR        = 0x0006;  //CB�±����
const UINT16 EC_TUNNEL_CB_BUSY             = 0x0007;  //���ڽ���Tunnel�������Ե�
const UINT16 EC_TUNNEL_NO_CB               = 0x0008;  //û�ҵ���Ӧ�Ŀ��ƿ�
#endif/*__TUNNEL_ERR_CODE_H__*/
