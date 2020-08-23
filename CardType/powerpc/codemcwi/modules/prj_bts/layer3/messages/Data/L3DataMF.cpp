/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMFTAddEntry.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   05/08/07   xin wang      initialization.
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "L3DataMFTAddEntry.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"

//----------------------------------------------
//CMFTAddEntry类定义
//----------------------------------------------


//------------------------------------------
//------------------------------------------
//UINT32 CFTAddEntry::SetEidInPayload(UINT32 ulEid)
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->ulEid = ulEid;
//}

//------------------------------------------
//------------------------------------------
//UINT32 CFTAddEntry::GetEidInPayload() const
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->ulEid;
//}

//------------------------------------------
//------------------------------------------
void CMFTAddEntry::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stMFTAddEntry*)GetDataPtr() )->MAC, pMac, M_MAC_ADDRLEN );
    return;
}

//------------------------------------------
//------------------------------------------
UINT8* CMFTAddEntry::GetMac() const
{
    return ( (stMFTAddEntry*)GetDataPtr() )->MAC;
}
#if 0
void CMFTAddEntry::SetType(UINT8 type)
{
    ( (stMFTAddEntry*)GetDataPtr() )->TYPE = type;
    return ;
}

UINT8 CMFTAddEntry::GetType() const
{
    return ( (stMFTAddEntry*)GetDataPtr() )->TYPE;
}
#endif    
//------------------------------------------
//------------------------------------------
//bool CFTAddEntry::SetServing(bool bIsServing)
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->bIsServing = bIsServing;
//}
//
////------------------------------------------
////------------------------------------------
//bool CFTAddEntry::GetServing() const
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->bIsServing;
//}
//
////------------------------------------------
////------------------------------------------
//bool CFTAddEntry::SetTunnel(bool bIsTunnel)
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->bIsTunnel = bIsTunnel;
//}
//
////------------------------------------------
////------------------------------------------
//bool CFTAddEntry::GetTunnel() const
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->bIsTunnel;
//}
//
////------------------------------------------
////------------------------------------------
//UINT32 CFTAddEntry::SetPeerBtsID(UINT32 ulPeerBtsID)
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->ulPeerBtsID = ulPeerBtsID;
//}
//
////------------------------------------------
////------------------------------------------
//UINT32 CFTAddEntry::GetPeerBtsID() const
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->ulPeerBtsID;
//}
//
//
//
////------------------------------------------
////------------------------------------------
//UINT8 CFTAddEntry::SetIpType(UINT8 ucIpType)
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->ucIpType = ucIpType;
//}
//
////------------------------------------------
////------------------------------------------
//UINT8 CFTAddEntry::GetIpType() const
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->ucIpType;
//}
//
////------------------------------------------
////------------------------------------------
//bool CFTAddEntry::SetAuth(bool bIsAuthed)
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->bIsAuthed = bIsAuthed;
//}
//
////------------------------------------------
////------------------------------------------
//bool CFTAddEntry::GetAuth() const
//{
//    return ( (stFTAddEntry*)GetDataPtr() )->bIsAuthed;
//}

/*============================================================
MEMBER FUNCTION:
    CMFTAddEntry::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CMFTAddEntry::GetDefaultDataLen() const
{
    return sizeof( stMFTAddEntry );
}


//------------------------------------------
//------------------------------------------
UINT16 CMFTAddEntry::GetDefaultMsgId() const
{
    return MSGID_MFT_ADD_ENTRY;
}


//----------------------------------------------
//CMFTUpdateEntry类定义
//----------------------------------------------


//------------------------------------------
//------------------------------------------
UINT16 CMFTUpdateEntry::GetDefaultMsgId() const
{
    return MSGID_MFT_UPDATE_ENTRY;
}
