/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelTimer.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   11/18/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __TUNNEL_TIMER_H__
#define __TUNNEL_TIMER_H__

#include "Message.h"
#include "L3DataMsgId.h"


/******************************************
 *M_TUNNEL_TRANS_RETRY_COUNT: Transaction最大
 *重发的次数
 *M_TUNNEL_TRANS_INTERVAL   : Transaction最大
 *重发间隔 (毫秒)
 ******************************************/
#define M_TUNNEL_TIMER_RETRY_COUNT  (3)
#define M_TUNNEL_TIMER_INTERVAL     (20000)


/********************************
 *stTunnelTimerExpire: 定时器超时消息
 *消息格式,由TUNNEL发往TUNNEL任务
 ********************************/
typedef struct _tag_stTunnelTimerExpire
{
    CMac Mac;
} stTunnelTimerExpire;


/*****************************************
 *CTunnelTimerExpire 类
 *****************************************/
class CTunnelTimerExpire:public CMessage
{
public:
    CTunnelTimerExpire(){}
    CTunnelTimerExpire(const CMessage &msg):CMessage( msg ){}
    ~CTunnelTimerExpire(){}

    void SetMac(CMac &Mac)
        {
        ( (stTunnelTimerExpire*)GetDataPtr() )->Mac = Mac;
        }

    CMac& GetMac() const
        {
        return ( (stTunnelTimerExpire*)GetDataPtr() )->Mac;
        }

protected:

    UINT32 GetDefaultDataLen() const
        {
        return sizeof( stTunnelTimerExpire );
        }
    UINT16 GetDefaultMsgId() const
        {
        return MSGID_TIMER_TUNNEL;
        }
};

#endif /*__TUNNEL_TIMER_H__*/
