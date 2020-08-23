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
 *Message ID 的定义按模块划分为下面几部分：
 *0X0000 ----0X0FFF Relay Message ID；
 *0x0f00: Relay broadcast OAM to UT
 *0x0f01: Relay broadcast Traffic to UT
 *0x0000: Relay high priority OAM to UT
 *0x0001: Relay low priority OAM to UT
 *0x0010: Relay high priority traffic to UT
 *0x0011: Relay low priority traffic to UT
 *
 *0X1000 ----0X4FFF定义BTS任务间的消息：
 *0X1000 ―― 0X13FF：配置管理部分
 *0X1400 ―― 0X14FF：故障管理部分
 *0X1500 ―― 0X15FF：性能管理部分
 *0X1600 ―― 0X16FF：系统管理部分
 *0X1700 ―― 0X17FF：CPE管理部分
 *0X1800 ―― 0X18FF：诊断管理部分
 *0X1A00 ―― 0X1AFF：Data Service部分
 *
 *0X5000 ----0X7FFF定义BTS 同CPE任务间的消息：
 *0X5000 ―― 0X52FF：配置管理部分
 *0X5300 ―― 0X53FF：性能管理部分
 *0X5400 ―― 0X54FF：软件管理部分
 *0X5500 ―― 0X55FF：诊断管理部分
 *0X5600 ―― 0X56FF：业务相关部分
 *0X5700 ―― 0X57FF：Data Service部分
 *
 *0XF000 ----0XFFF定义CPE任务间的消息：
 *0XF000 ―― 0XF100：Data Service部分
 *
 *0X8000 ----0XF000留部分暂时不分配；	
*****************************************************/

#endif /*__BTSMsgCode_H__*/