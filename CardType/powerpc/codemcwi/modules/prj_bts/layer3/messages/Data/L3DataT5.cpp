/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelTerminate.cpp
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

#include "L3DataTunnelTerminate.h"
#include "L3DataMsgId.h"
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif


//----------------------------------------------------
//CTunnelTerminate类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelTerminate::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelTerminate::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_TERMINATE_REQ;
}


bool CTunnelTerminate::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_TERMINATE_REQ );
    return true;
}


//----------------------------------------------------
//CTunnelTerminateResp类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelTerminateResp::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelTerminateResp::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_TERMINATE_RESP;
}


bool CTunnelTerminateResp::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_TERMINATE_RESP );
    return true;
}


//----------------------------------------------------
//CTunnelTerminate类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelTerminate::GetDefaultMsgId

DESCRIPTION:
    设置MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
    CTUNNELDelete
==============================================================*/
UINT16 CTUNNELDelete::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_DELETE_TIMER;
}


bool CTUNNELDelete::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_DELETE_TIMER );
    return true;
}
