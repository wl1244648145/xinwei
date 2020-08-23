/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelSync.cpp
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

#include "L3DataTunnelSync.h"
#include "L3DataMsgId.h"
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif
#include <string.h>
//----------------------------------------------------
//CTunnelSync�ඨ��
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelSync::GetDefaultMsgId

DESCRIPTION:
    ����MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelSync::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_SYNC_REQ;
}


bool CTunnelSync::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_SYNC_REQ );
    return true;
}


void CTunnelSync::SetDATA(DATA &Data)
{
    memcpy( (void*)&( ( (stTunnelSync*)GetDataPtr() )->Data ), (const void*)&Data, sizeof( DATA ) );
}


DATA& CTunnelSync::GetDATA() const
{
    return ( (stTunnelSync*)GetDataPtr() )->Data;
}


/*============================================================
MEMBER FUNCTION:
    CTunnelSync::GetDefaultDataLen

DESCRIPTION:
    ��ȡCMessage Payload�ĳ���

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: ����

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CTunnelSync::GetDefaultDataLen() const
{
    return sizeof( stTunnelSync );
}


//----------------------------------------------------
//CTunnelSyncResp�ඨ��
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelSyncResp::GetDefaultMsgId

DESCRIPTION:
    ����MsgId;

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgId

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnelSyncResp::GetDefaultMsgId() const
{
    return MSGID_TUNNEL_SYNC_RESP;
}


bool CTunnelSyncResp::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_TUNNEL_SYNC_RESP );
    return true;
}

