/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    btsCommon.h
 *
 * DESCRIPTION: This module defines Message ID.
 *
 * HISTORY:
 *
 *   Date       Author    Description
 *   ---------  ------    ----------------------------------------------------
 *
 *
 *---------------------------------------------------------------------------*/

#ifndef __BTSMsgCode_H__
#define __BTSMsgCode_H__

#include "btsTypes.h"

/*****************************************************
 *Message ID �Ķ��尴ģ�黮��Ϊ���漸���֣�
 *0X0000 ----0X0FFF Relay Message ID��
 *0x0f00: Relay broadcast OAM to UT
 *0x0f01: Relay broadcast Traffic to UT
 *0x0000: Relay high priority OAM to UT
 *0x0001: Relay low priority OAM to UT
 *0x0010: Relay high priority traffic to UT
 *0x0011: Relay low priority traffic to UT
 *
 *0X1000 ----0X4FFF����BTS��������Ϣ��
 *0X1000 �D�D 0X13FF�����ù�����
 *0X1400 �D�D 0X14FF�����Ϲ�����
 *0X1500 �D�D 0X15FF�����ܹ�����
 *0X1600 �D�D 0X16FF��ϵͳ������
 *0X1700 �D�D 0X17FF��CPE������
 *0X1800 �D�D 0X18FF����Ϲ�����
 *0X1A00 �D�D 0X1AFF��Data Service����
 *
 *0X5000 ----0X7FFF����BTS ͬCPE��������Ϣ��
 *0X5000 �D�D 0X52FF�����ù�����
 *0X5300 �D�D 0X53FF�����ܹ�����
 *0X5400 �D�D 0X54FF�����������
 *0X5500 �D�D 0X55FF����Ϲ�����
 *0X5600 �D�D 0X56FF��ҵ����ز���
 *0X5700 �D�D 0X57FF��Data Service����
 *
 *0XF000 ----0XFFF����CPE��������Ϣ��
 *0XF000 �D�D 0XF100��Data Service����
 *
 *0X8000 ----0XF000��������ʱ�����䣻	
*****************************************************/

#endif /*__BTSMsgCode_H__*/