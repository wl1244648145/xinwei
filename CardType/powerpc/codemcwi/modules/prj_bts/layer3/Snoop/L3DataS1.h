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

//SNOOP�����붨��
const UINT16 EC_SNOOP_NORMAL               = 0x0000;  //��������
const UINT16 EC_SNOOP_UNEXPECTED_MSGID     = 0x0001;  //�Ƿ�Message ID
const UINT16 EC_SNOOP_SYS_FAIL             = 0x0002;  //ϵͳ����
const UINT16 EC_SNOOP_PARAMETER            = 0x0003;  //��������
const UINT16 EC_SNOOP_CFG_ERR              = 0x0004;  //�������ݴ���
const UINT16 EC_SNOOP_NO_CCB               = 0x0005;  //�û���
const UINT16 EC_SNOOP_PACKET_ERR           = 0x0006;  //���Ĵ���
const UINT16 EC_SNOOP_MSG_ERR              = 0x0007;  //��Ϣ����

#endif/*__SNOOP_ERR_CODE_H__*/
