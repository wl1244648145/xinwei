/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopTimer.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   09/02/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __SNOOP_TIMER_H__
#define __SNOOP_TIMER_H__

#include "Message.h"
#include "L3DataCommon.h"


/********************************
 *M_SNOOP_FTIMER_INTERVAL_8Sec: 
 *Snoop任务定时器Ftimer长度  (毫秒)
 ********************************/
#define M_SNOOP_FTIMER_INTERVAL_8Sec    (8 * 1000)


/********************************
 *M_SNOOP_SYNC_INTERVAL_2Sec: 
 *同步定时器长度  (毫秒)
 ********************************/
#define M_SNOOP_SYNC_INTERVAL_2Sec      (6 * 1000)


/********************************
 *stSNTimerExpire: 定时器超时消息
 *消息格式,由Snoop发往Snoop任务
 ********************************/
typedef struct _tag_stSNTimerExpire
{
    UINT8 aucMac[ M_MAC_ADDRLEN ];
} stSNTimerExpire;


/*****************************************
 *CSnoopTimerExpire类
 *****************************************/
class CSnoopTimerExpire:public CMessage
{
public:
    CSnoopTimerExpire(){}
    CSnoopTimerExpire(const CMessage &msg):CMessage( msg ){}
    ~CSnoopTimerExpire(){}

    void   SetMac(const UINT8 *);
    UINT8* GetMac() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif /*__SNOOP_TIMER_H__*/
