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
 *M_TUNNEL_TRANS_RETRY_COUNT: Transaction���
 *�ط��Ĵ���
 *M_TUNNEL_TRANS_INTERVAL   : Transaction���
 *�ط���� (����)
 ******************************************/
#define M_TUNNEL_TIMER_RETRY_COUNT  (3)
#define M_TUNNEL_TIMER_INTERVAL     (20000)


/********************************
 *stTunnelTimerExpire: ��ʱ����ʱ��Ϣ
 *��Ϣ��ʽ,��TUNNEL����TUNNEL����
 ********************************/
typedef struct _tag_stTunnelTimerExpire
{
    CMac Mac;
} stTunnelTimerExpire;


/*****************************************
 *CTunnelTimerExpire ��
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
