/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelRequestBase.cpp
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

#include "L3DataTunnelRequestBase.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h"
#include <string.h> 
#endif

//----------------------------------------------------
//CTunnelRequestBase类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CTunnelRequestBase::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CTunnelRequestBase::GetDefaultDataLen() const
{
    return sizeof( stTunnelReq );
}


UINT16 CTunnelRequestBase::GetMsgCode() const
{
    return ntohs( ( (stTunnelReq*)GetDataPtr() )->usMsgCode );
}


UINT32 CTunnelRequestBase::SetEidInPayload(UINT32 ulEid)
{
    return ( (stTunnelReq*)GetDataPtr() )->ulEid = htonl( ulEid );
}

UINT32 CTunnelRequestBase::GetEidInPayload() const
{
    return ntohl( ( (stTunnelReq*)GetDataPtr() )->ulEid );
}


void CTunnelRequestBase::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stTunnelReq*)GetDataPtr() )->aucMac, pMac, M_MAC_ADDRLEN );
}


UINT8* CTunnelRequestBase::GetMac() const
{
    return ( (stTunnelReq*)GetDataPtr() )->aucMac;
}


UINT32 CTunnelRequestBase::SetDstBtsID(UINT32 ulDstBtsID)
{
    return ( (stTunnelReq*)GetDataPtr() )->ulDstBtsID = htonl( ulDstBtsID );
}


UINT32 CTunnelRequestBase::GetDstBtsID() const
{
    return ntohl( ( (stTunnelReq*)GetDataPtr() )->ulDstBtsID );
}

/*
UINT32 CTunnelRequestBase::SetDstBtsIP(UINT32 ulDstBtsIP)
{
    return ( (stTunnelReq*)GetDataPtr() )->ulDstBtsIP = htonl( ulDstBtsIP );
}


UINT32 CTunnelRequestBase::GetDstBtsIP() const
{
    return ntohl( ( (stTunnelReq*)GetDataPtr() )->ulDstBtsIP );
}

UINT16 CTunnelRequestBase::SetDstPort(UINT16 usDstPort)
{
    return ( (stTunnelReq*)GetDataPtr() )->usDstPort = htonl( usDstPort );
}


UINT16 CTunnelRequestBase::GetDstPort() const
{
    return ntohl( ( (stTunnelReq*)GetDataPtr() )->usDstPort );
}
*/


UINT32 CTunnelRequestBase::SetSenderBtsID(UINT32 ulSenderBtsID)
{
    return ( (stTunnelReq*)GetDataPtr() )->ulSenderBtsID = htonl( ulSenderBtsID );
}


UINT32 CTunnelRequestBase::GetSenderBtsID() const
{
    return ntohl( ( (stTunnelReq*)GetDataPtr() )->ulSenderBtsID );
}
UINT32 CTunnelRequestBase::SetSenderBtsIP(UINT32 ulSenderBtsIP)
{
    return ( (stTunnelReq*)GetDataPtr() )->ulSenderBtsIP = htonl( ulSenderBtsIP );
}


UINT32 CTunnelRequestBase::GetSenderBtsIP() const
{
    return ntohl( ( (stTunnelReq*)GetDataPtr() )->ulSenderBtsIP );
}

UINT16 CTunnelRequestBase::SetSenderPort(UINT16 usSenderPort)
{
    return ( (stTunnelReq*)GetDataPtr() )->usSenderPort = htonl( usSenderPort );
}


UINT16 CTunnelRequestBase::GetSenderPort() const
{
    return ntohl( ( (stTunnelReq*)GetDataPtr() )->usSenderPort );
}



UINT8 CTunnelRequestBase::SetIpType(UINT8 ucIpType)
{
    return ( (stTunnelReq*)GetDataPtr() )->ucIpType = ucIpType;
}


UINT8 CTunnelRequestBase::GetIpType() const
{
    return ( (stTunnelReq*)GetDataPtr() )->ucIpType;
}


UINT16 CTunnelRequestBase::SetMsgCode(UINT16 usMsgId)
{
    return ( (stTunnelReq*)GetDataPtr() )->usMsgCode = htons( usMsgId );
}
