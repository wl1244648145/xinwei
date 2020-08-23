/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopErrCode.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   11/25/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_ERR_CODE_H__
#define __SNOOP_ERR_CODE_H__

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//SNOOP错误码定义
const UINT16 EC_SNOOP_NORMAL               = 0x0000;  //正常流程
const UINT16 EC_SNOOP_UNEXPECTED_MSGID     = 0x0001;  //非法Message ID
const UINT16 EC_SNOOP_SYS_FAIL             = 0x0002;  //系统错误
const UINT16 EC_SNOOP_PARAMETER            = 0x0003;  //参数错误
const UINT16 EC_SNOOP_CFG_ERR              = 0x0004;  //配置数据错误
const UINT16 EC_SNOOP_NO_CCB               = 0x0005;  //用户满
const UINT16 EC_SNOOP_PACKET_ERR           = 0x0006;  //报文错误
const UINT16 EC_SNOOP_MSG_ERR              = 0x0007;  //消息错误

#endif/*__SNOOP_ERR_CODE_H__*/
