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

//TUNNEL错误码定义
const UINT16 EC_TUNNEL_NORMAL              = 0x0000;  //正常流程
const UINT16 EC_TUNNEL_UNEXPECTED_MSGID    = 0x0001;  //非法Message ID
const UINT16 EC_TUNNEL_SYS_ERR             = 0x0002;  //系统错误
const UINT16 EC_TUNNEL_SOCKET_ERR          = 0x0003;  //Socket错误
const UINT16 EC_TUNNEL_PARAMETER           = 0x0004;  //参数错误
const UINT16 EC_TUNNEL_CB_USEDUP           = 0x0005;  //CB用光
const UINT16 EC_TUNNEL_CB_INDEX_ERR        = 0x0006;  //CB下标错误
const UINT16 EC_TUNNEL_CB_BUSY             = 0x0007;  //正在进行Tunnel操作，稍等
const UINT16 EC_TUNNEL_NO_CB               = 0x0008;  //没找到对应的控制块
#endif/*__TUNNEL_ERR_CODE_H__*/
