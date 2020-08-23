/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    DmTimer.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   10/12/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __DM_TIMER_H__
#define __DM_TIMER_H__

#include "L3dataCommon.h"
#include "Message.h"
#include "L3dataMsgId.h"

#pragma pack (1)

/********************************
 *M_DM_FTIMER_INTERVAL_2Sec: 
 ********************************/
#define M_DM_FTIMER_INTERVAL_2Sec    (2000)



const UINT32      RESEND_CNT3        = 3;

/*************************************************
 *Timer: Timer 类型；
 *************************************************/
typedef enum 
    {
    RECEND_TIMER= 0,
    SYNC_TIMER
    }TIMERTYPE;



/********************************
 *stTimerExpire: 定时器超时消息
 *消息格式,由DM发往DM任务
 ********************************/
typedef struct _tag_DmTimerExpire
{
    UINT16 tranid;//add tranid to find tansaction to del it jiaying20100813
    UINT32 Eid;
    UINT8  TimerType;
} DmTimerExpire;


/*****************************************
 *CTimerExpire类
 *****************************************/
class CDmTimerExpire:public CMessage
    {
public:
    CDmTimerExpire(){}
    CDmTimerExpire(const CMessage& Msg):CMessage(Msg){}
    ~CDmTimerExpire(){}


    void SetInEid(UINT32) ;
    UINT32 GetInEid() const;
    UINT8 GetTimerType() const;
    void SetTimerType(UINT8) ;
    UINT16 SetTransactionId(UINT16 tranid);
    UINT16 GetTransactionId() const;
protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;

    };


#pragma pack ()
#endif /*__DM_TIMER_H__*/

