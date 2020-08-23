/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataFTAddEntry.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   03/28/06   xiao weifang  FixIp用户做成非永久性用户
 *   08/09/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "L3DataFTAddEntry.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"

//----------------------------------------------
//CFTAddEntry类定义
//----------------------------------------------


//------------------------------------------
//------------------------------------------
UINT32 CFTAddEntry::SetEidInPayload(UINT32 ulEid)
{
    return ( (stFTAddEntry*)GetDataPtr() )->ulEid = ulEid;
}

//------------------------------------------
//------------------------------------------
UINT32 CFTAddEntry::GetEidInPayload() const
{
    return ( (stFTAddEntry*)GetDataPtr() )->ulEid;
}

//------------------------------------------
//------------------------------------------
void CFTAddEntry::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stFTAddEntry*)GetDataPtr() )->aucMac, pMac, M_MAC_ADDRLEN );
    return;
}

//------------------------------------------
//------------------------------------------
UINT8* CFTAddEntry::GetMac() const
{
    return ( (stFTAddEntry*)GetDataPtr() )->aucMac;
}

//------------------------------------------
//------------------------------------------
bool CFTAddEntry::SetServing(bool bIsServing)
{
    return ( (stFTAddEntry*)GetDataPtr() )->bIsServing = bIsServing;
}

//------------------------------------------
//------------------------------------------
bool CFTAddEntry::GetServing() const
{
    return ( (stFTAddEntry*)GetDataPtr() )->bIsServing;
}

//------------------------------------------
//------------------------------------------
bool CFTAddEntry::SetTunnel(bool bIsTunnel)
{
    return ( (stFTAddEntry*)GetDataPtr() )->bIsTunnel = bIsTunnel;
}

//------------------------------------------
//------------------------------------------
bool CFTAddEntry::GetTunnel() const
{
    return ( (stFTAddEntry*)GetDataPtr() )->bIsTunnel;
}

//------------------------------------------
//------------------------------------------
UINT32 CFTAddEntry::SetPeerBtsID(UINT32 ulPeerBtsID)
{
    return ( (stFTAddEntry*)GetDataPtr() )->ulPeerBtsID = ulPeerBtsID;
}

//------------------------------------------
//------------------------------------------
UINT32 CFTAddEntry::GetPeerBtsID() const
{
    return ( (stFTAddEntry*)GetDataPtr() )->ulPeerBtsID;
}



//------------------------------------------
//------------------------------------------
UINT8 CFTAddEntry::SetIpType(UINT8 ucIpType)
{
    return ( (stFTAddEntry*)GetDataPtr() )->ucIpType = ucIpType;
}

//------------------------------------------
//------------------------------------------
UINT8 CFTAddEntry::GetIpType() const
{
    return ( (stFTAddEntry*)GetDataPtr() )->ucIpType;
}

//------------------------------------------
//------------------------------------------
UINT16 CFTAddEntry::setGroupId(UINT16 usGroupID)
{
    return ( (stFTAddEntry*)GetDataPtr() )->usGroupID = usGroupID;
}

//------------------------------------------
//------------------------------------------
UINT16 CFTAddEntry::getGroupId() const
{
    return ( (stFTAddEntry*)GetDataPtr() )->usGroupID;
}

//------------------------------------------
//------------------------------------------
bool CFTAddEntry::SetAuth(bool bIsAuthed)
{
    return ( (stFTAddEntry*)GetDataPtr() )->bIsAuthed = bIsAuthed;
}

//------------------------------------------
//------------------------------------------
bool CFTAddEntry::GetAuth() const
{
    return ( (stFTAddEntry*)GetDataPtr() )->bIsAuthed;
}

/*============================================================
MEMBER FUNCTION:
    CFTAddEntry::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CFTAddEntry::GetDefaultDataLen() const
{
    return sizeof( stFTAddEntry );
}


//------------------------------------------
//------------------------------------------
UINT16 CFTAddEntry::GetDefaultMsgId() const
{
    return MSGID_FT_ADD_ENTRY;
}


//----------------------------------------------
//CFTUpdateEntry类定义
//----------------------------------------------


//------------------------------------------
//------------------------------------------
UINT16 CFTUpdateEntry::GetDefaultMsgId() const
{
    return MSGID_FT_UPDATE_ENTRY;
}

UINT32 CFTUpdateEntry::GetDefaultDataLen() const
{
    return sizeof( stFTAddEntry ) + sizeof(bool);
}
