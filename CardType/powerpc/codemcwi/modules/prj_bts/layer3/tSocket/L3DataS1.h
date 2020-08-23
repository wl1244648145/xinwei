/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    L3DataSocketErrCode.h
*
* DESCRIPTION: 
*
* HISTORY:
*
*   Date       Author        Description
*   ---------  ------        ------------------------------------------------
*   11/01/06   xinwang       initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef _INC_TSOCKET_ERR_CODE
#define _INC_TSOCKET_ERR_CODE

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//SOCKET错误码定义
const UINT16 EC_SOCKET_NORMAL               =0x0000;  //正常流程
const UINT16 EC_SOCKET_UNEXPECTED_MSGID     =0x0001;  //非法Message ID
const UINT16 EC_SOCKET_SYS_ERR              =0x0002;  //系统错误
const UINT16 EC_SOCKET_SOCKET_ERR           =0x0003;  //Socket错误
const UINT16 EC_SOCKET_PARAMETER            =0x0004;  //参数错误
const UINT16 EC_SOCKET_MSG_EXCEPTION        =0x0005;  //ComMessage错误
const UINT16 EC_SOCKET_CB_INDEX_ERR         =0x0006;  //CB下标错误
const UINT16 EC_SOCKET_CB_USEDUP            =0x0007;  //CB用完
const UINT16 EC_SOCKET_FCB_INDEX_ERR        =0x0008;  //FCB(NVRAM)下标错误
const UINT16 EC_SOCKET_FCB_USEDUP           =0x0009;  //FCB(NVRAM)用完

#endif/*_INC_TSOCKET_ERR_CODE*/