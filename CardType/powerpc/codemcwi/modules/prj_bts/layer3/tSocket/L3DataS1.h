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

//SOCKET�����붨��
const UINT16 EC_SOCKET_NORMAL               =0x0000;  //��������
const UINT16 EC_SOCKET_UNEXPECTED_MSGID     =0x0001;  //�Ƿ�Message ID
const UINT16 EC_SOCKET_SYS_ERR              =0x0002;  //ϵͳ����
const UINT16 EC_SOCKET_SOCKET_ERR           =0x0003;  //Socket����
const UINT16 EC_SOCKET_PARAMETER            =0x0004;  //��������
const UINT16 EC_SOCKET_MSG_EXCEPTION        =0x0005;  //ComMessage����
const UINT16 EC_SOCKET_CB_INDEX_ERR         =0x0006;  //CB�±����
const UINT16 EC_SOCKET_CB_USEDUP            =0x0007;  //CB����
const UINT16 EC_SOCKET_FCB_INDEX_ERR        =0x0008;  //FCB(NVRAM)�±����
const UINT16 EC_SOCKET_FCB_USEDUP           =0x0009;  //FCB(NVRAM)����

#endif/*_INC_TSOCKET_ERR_CODE*/