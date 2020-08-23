/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelResponseBase.cpp
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

#include "L3DataTunnelResponseBase.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif
#include <string.h>
//----------------------------------------------------
//CTunnelResponseBase类定义
//----------------------------------------------------


UINT16 CTunnelResponseBase::GetMsgCode() const
{
    return ntohs( ( (stTunnelResp*)GetDataPtr() )->usMsgCode );
}


UINT32 CTunnelResponseBase::SetEidInPayload(UINT32 ulEid)
{
    return ( (stTunnelResp*)GetDataPtr() )->ulEid = htonl( ulEid );
}

UINT32 CTunnelResponseBase::GetEidInPayload() const
{
    return ntohl( ( (stTunnelResp*)GetDataPtr() )->ulEid );
}

void CTunnelResponseBase::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stTunnelResp*)GetDataPtr() )->aucMac, pMac, M_MAC_ADDRLEN );
}


UINT8* CTunnelResponseBase::GetMac() const
{
    return ( (stTunnelResp*)GetDataPtr() )->aucMac;
}


UINT32 CTunnelResponseBase::SetDstBtsID(UINT32 ulDstBtsID)
{
    return ( (stTunnelResp*)GetDataPtr() )->ulDstBtsID = htonl( ulDstBtsID );
}
UINT32 CTunnelResponseBase::GetDstBtsID() const
{
    return ntohl( ( (stTunnelResp*)GetDataPtr() )->ulDstBtsID );
}

UINT32 CTunnelResponseBase::SetDstBtsIP(UINT32 ulDstBtsIP)
{
    return ( (stTunnelResp*)GetDataPtr() )->ulDstBtsIP = htonl( ulDstBtsIP );
}
UINT32 CTunnelResponseBase::GetDstBtsIP() const
{
    return ntohl( ( (stTunnelResp*)GetDataPtr() )->ulDstBtsIP );
}

UINT16 CTunnelResponseBase::SetDstPort(UINT32 ulDstPort)
{
    return ( (stTunnelResp*)GetDataPtr() )->ulDstPort = htonl( ulDstPort );
}


UINT16 CTunnelResponseBase::GetDstPort() const
{
    return ntohl( ( (stTunnelResp*)GetDataPtr() )->ulDstPort );
}


bool CTunnelResponseBase::SetResult(bool bSuccess)
{
#ifndef WBBU_CODE
    return ( (stTunnelResp*)GetDataPtr() )->bSuccess = bSuccess;
#else
    return ( (stTunnelResp*)GetDataPtr() )->bSuccess = htonl(bSuccess);
#endif
}

bool CTunnelResponseBase::GetResult() const
{
#ifndef WBBU_CODE
    return ( (stTunnelResp*)GetDataPtr() )->bSuccess;
#else
    return (bool)(ntohl( (stTunnelResp*)GetDataPtr() )->bSuccess);
#endif
}

/*============================================================
MEMBER FUNCTION:
    CTunnelResponseBase::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CTunnelResponseBase::GetDefaultDataLen() const
{
    return sizeof( stTunnelResp );
}


UINT16 CTunnelResponseBase::SetMsgCode(UINT16 usMsgId)
{
    return ( (stTunnelResp*)GetDataPtr() )->usMsgCode = htons( usMsgId );
}
