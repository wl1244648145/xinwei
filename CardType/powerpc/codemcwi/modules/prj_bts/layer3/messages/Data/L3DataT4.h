/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelSync.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/05/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __TUNNEL_SYNC_H__
#define __TUNNEL_SYNC_H__

#include "Message.h"
#include "L3DataMessages.h"
#include "L3DataTunnelRequestBase.h"
#include "L3DataTunnelResponseBase.h"

/*****************************************
 *CTunnelSync类
 *****************************************/
class CTunnelSync:public CTunnelRequestBase
{
public:
    CTunnelSync(){}
    CTunnelSync(const CMessage &msg):CTunnelRequestBase( msg ){}
    ~CTunnelSync(){}

    bool   CreateMessage(CComEntity&);
//    UINT16 SetMsgCode();
    void   SetDATA(DATA&);
    DATA&  GetDATA() const;

protected:
    //消息结构有变，长度有变
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CTunnelSyncResp类
 *****************************************/
class CTunnelSyncResp:public CTunnelResponseBase
{
public:
    CTunnelSyncResp(){}
    CTunnelSyncResp(const CMessage &msg):CTunnelResponseBase( msg ){}
    ~CTunnelSyncResp(){}

    bool   CreateMessage(CComEntity&);
//    UINT16 SetMsgCode();

protected:
    UINT16 GetDefaultMsgId() const;
};

#endif /*__TUNNEL_SYNC_H__*/
