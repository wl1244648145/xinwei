/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTDRErrCode.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   11/21/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __TDR_ERR_CODE_H__
#define __TDR_ERR_CODE_H__

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

//TDR�����붨��
const UINT16 EC_TDR_NORMAL              = 0x0000;  //��������
const UINT16 EC_TDR_UNEXPECTED_MSGID    = 0x0001;  //�Ƿ�Message ID
const UINT16 EC_TDR_SYS_ERR             = 0x0002;  //ϵͳ����
const UINT16 EC_TDR_SOCKET_ERR          = 0x0003;  //Socket����
const UINT16 EC_TDR_PARAMETER           = 0x0004;  //��������
#endif/*__TDR_ERR_CODE_H__*/
