/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelChangeAnchor.cpp
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

#include "L3DataTunnelChangeAnchor.h"
#include "L3DataMsgId.h"
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif

//----------------------------------------------------
//CTunnelChangeAnchor类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelChangeAnchor::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelChangeAnchor::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_CHANGE_ANCHOR_REQ;
}


bool CTunnelChangeAnchor::CreateMessage(CComEntity &Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_CHANGE_ANCHOR_REQ );
    return true;
}


//----------------------------------------------------
//CTunnelChangeAnchorResp类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelChangeAnchorResp::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelChangeAnchorResp::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_CHANGE_ANCHOR_RESP;
}


bool CTunnelChangeAnchorResp::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_CHANGE_ANCHOR_RESP );
    return true;
}


//----------------------------------------------------
//CTunnelHeartBeat类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelHeartBeat::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelHeartBeat::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_HEARTBEAT;
}


bool CTunnelHeartBeat::CreateMessage(CComEntity &Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_HEARTBEAT);
    return true;
}


//----------------------------------------------------
//CTunnelHeartBeatResp类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelHeartBeatResp::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelHeartBeatResp::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_HEARTBEAT_RESP;
}


bool CTunnelHeartBeatResp::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_HEARTBEAT_RESP);
    return true;
}

