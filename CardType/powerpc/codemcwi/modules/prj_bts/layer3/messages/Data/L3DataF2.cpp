/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataFTDelEntry.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   08/10/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "L3DataFTDelEntry.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"


//------------------------------------------
//------------------------------------------
void CFTDelEntry::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stFTDelEntry*)GetDataPtr() )->aucMac, pMac, M_MAC_ADDRLEN );
    return;
}


//------------------------------------------
//------------------------------------------
UINT8* CFTDelEntry::GetMac() const
{
    return ( (stFTDelEntry*)GetDataPtr() )->aucMac;
}

//------------------------------------------
//------------------------------------------
void CFTDelEntry::SetTunnel(bool bIsCreateTempTunnel)
{
    ((stFTDelEntry*)GetDataPtr())->bCreateTempTunnel = bIsCreateTempTunnel;
}


//------------------------------------------
//------------------------------------------
bool CFTDelEntry::isCreateTempTunnel() const
{
    return ((stFTDelEntry*)GetDataPtr())->bCreateTempTunnel;
}


//------------------------------------------
//------------------------------------------
void CFTDelEntry::setTunnelPeerBtsIP(UINT32 btsIP)
{
    ((stFTDelEntry*)GetDataPtr())->ulPeerIP = btsIP;
}


//------------------------------------------
//------------------------------------------
UINT32 CFTDelEntry::getTunnelPeerBtsIP() const
{
    return ((stFTDelEntry*)GetDataPtr())->ulPeerIP;
}


//------------------------------------------
//------------------------------------------
void CFTDelEntry::setTunnelPeerBtsPort(UINT16 port)
{
    ((stFTDelEntry*)GetDataPtr())->usPeerPort = port;
}


//------------------------------------------
//------------------------------------------
UINT16 CFTDelEntry::getTunnelPeerBtsPort() const
{
    return ((stFTDelEntry*)GetDataPtr())->usPeerPort;
}



/*============================================================
MEMBER FUNCTION:
    CFTDelEntry::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CFTDelEntry::GetDefaultDataLen() const
{
    return sizeof( stFTDelEntry );
}


//------------------------------------------
//------------------------------------------
UINT16 CFTDelEntry::GetDefaultMsgId() const
{
    return MSGID_FT_DEL_ENTRY;
}
