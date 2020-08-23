/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelEstablish.cpp
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

#include "L3DataTunnelEstablish.h"
#include "L3DataMsgId.h"
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif

//----------------------------------------------------
//CTunnelEstablish类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelEstablish::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelEstablish::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_ESTABLISH_REQ;
}


UINT32 CTunnelEstablish::GetDefaultDataLen() const
{
    return sizeof( stTunnelEstablish );
}

bool CTunnelEstablish::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_ESTABLISH_REQ );
    return true;
}


UINT32 CTunnelEstablish::SetFixIp(UINT32 ulIp)
{
    return ( (stTunnelEstablish*)GetDataPtr() )->ulFixIp = htonl( ulIp );
}


UINT32 CTunnelEstablish::GetFixIp()
{
    return ntohl( ( (stTunnelEstablish*)GetDataPtr() )->ulFixIp );
}


UINT16 CTunnelEstablish::SetGroupId(UINT16 usGroupId)
{
    return ( (stTunnelEstablish*)GetDataPtr() )->usGroupId = htons( usGroupId ); 
}


UINT16 CTunnelEstablish::GetGroupId()
{
    return ntohs( ( (stTunnelEstablish*)GetDataPtr() )->usGroupId );
}


//----------------------------------------------------
//CTunnelEstablishResp类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelEstablishResp::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelEstablishResp::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_ESTABLISH_RESP;
}


bool CTunnelEstablishResp::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_ESTABLISH_RESP );
    return true;
}
