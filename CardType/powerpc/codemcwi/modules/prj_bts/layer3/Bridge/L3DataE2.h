/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataEBErrCode.h
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

#ifndef __EB_ERR_CODE_H__
#define __EB_ERR_CODE_H__

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//EB错误码定义
const UINT16 EC_EB_NORMAL               = 0x0000;  //正常流程
const UINT16 EC_EB_UNEXPECTED_MSGID     = 0x0001;  //非法Message ID
const UINT16 EC_EB_SYS_FAIL             = 0x0002;  //系统错误
const UINT16 EC_EB_SOCKET_ERR           = 0x0003;  //Socket错误
const UINT16 EC_EB_PARAMETER            = 0x0004;  //参数错误
const UINT16 EC_EB_MSG_EXCEPTION        = 0x0005;  //异常消息
const UINT16 EC_EB_NO_ENTRY             = 0x0006;  //转发表满

#endif/*__EB_ERR_CODE_H__*/
