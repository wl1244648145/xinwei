/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelChangeAnchor.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/06/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __TUNNEL_CHANGE_ANCHOR_H__
#define __TUNNEL_CHANGE_ANCHOR_H__

#include "Message.h"
#include "L3DataMessages.h"
#include "L3DataTunnelRequestBase.h"
#include "L3DataTunnelResponseBase.h"

/*****************************************
 *CTunnelChangeAnchor类
 *****************************************/
class CTunnelChangeAnchor:public CTunnelRequestBase
{
public:
    CTunnelChangeAnchor(){}
    CTunnelChangeAnchor(const CMessage &msg):CTunnelRequestBase( msg ){}
    ~CTunnelChangeAnchor(){}

    bool   CreateMessage(CComEntity&);
//    UINT16 SetMsgCode();

protected:
    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CTunnelChangeAnchorResp类
 *****************************************/
class CTunnelChangeAnchorResp:public CTunnelResponseBase
{
public:
    CTunnelChangeAnchorResp(){}
    CTunnelChangeAnchorResp(const CMessage &msg):CTunnelResponseBase( msg ){}
    ~CTunnelChangeAnchorResp(){}

    bool   CreateMessage(CComEntity&);
//    UINT16 SetMsgCode();

protected:
    UINT16 GetDefaultMsgId() const;
};

/*****************************************
 *CTunnelHeartBeat类
 *****************************************/
class CTunnelHeartBeat:public CTunnelRequestBase
{
public:
    CTunnelHeartBeat(){}
    CTunnelHeartBeat(const CMessage &msg):CTunnelRequestBase( msg ){}
    ~CTunnelHeartBeat(){}

    bool   CreateMessage(CComEntity&);

protected:
    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CTunnelHeartBeatResp类
 *****************************************/
class CTunnelHeartBeatResp:public CTunnelResponseBase
{
public:
    CTunnelHeartBeatResp(){}
    CTunnelHeartBeatResp(const CMessage &msg):CTunnelResponseBase( msg ){}
    ~CTunnelHeartBeatResp(){}

    bool   CreateMessage(CComEntity&);

protected:
    UINT16 GetDefaultMsgId() const;
};

#endif /*__TUNNEL_CHANGE_ANCHOR_H__*/
