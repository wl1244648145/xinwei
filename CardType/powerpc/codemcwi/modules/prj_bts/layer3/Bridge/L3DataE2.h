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

//EB�����붨��
const UINT16 EC_EB_NORMAL               = 0x0000;  //��������
const UINT16 EC_EB_UNEXPECTED_MSGID     = 0x0001;  //�Ƿ�Message ID
const UINT16 EC_EB_SYS_FAIL             = 0x0002;  //ϵͳ����
const UINT16 EC_EB_SOCKET_ERR           = 0x0003;  //Socket����
const UINT16 EC_EB_PARAMETER            = 0x0004;  //��������
const UINT16 EC_EB_MSG_EXCEPTION        = 0x0005;  //�쳣��Ϣ
const UINT16 EC_EB_NO_ENTRY             = 0x0006;  //ת������

#endif/*__EB_ERR_CODE_H__*/
