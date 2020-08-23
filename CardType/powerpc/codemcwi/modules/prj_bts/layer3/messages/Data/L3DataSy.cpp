/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSyncIL.cpp
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

#include <string.h>

#include "L3DataSyncIL.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"


//----------------------------------------------------
//CSyncIL类定义
//----------------------------------------------------



/*============================================================
MEMBER FUNCTION:
    CSyncIL::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CSyncIL::GetDefaultDataLen() const
{
    return sizeof( stSyncIL );
}


UINT16 CSyncIL::GetDefaultMsgId() const
{
    return MSGID_IPLIST_SYNC_REQ;
}


UINT32 CSyncIL::SetEidInPayload(UINT32 ulEid)
{
    return ( (stSyncIL*)GetDataPtr() )->ulEid = ulEid;
}

UINT32 CSyncIL::GetEidInPayload() const
{
    return ( (stSyncIL*)GetDataPtr() )->ulEid;
}


UINT8 CSyncIL::SetOp(UINT8 ucOp)
{
    return ( (stSyncIL*)GetDataPtr() )->ucOp = ucOp;
}


UINT8 CSyncIL::GetOp() const
{
    return ( (stSyncIL*)GetDataPtr() )->ucOp;
}

UINT8 CSyncIL::SetIpType(UINT8 ucIpType)
{
    return ( (stSyncIL*)GetDataPtr() )->ucIpType = ucIpType;
}


UINT8 CSyncIL::GetIpType() const
{
    return ( (stSyncIL*)GetDataPtr() )->ucIpType;
}

bool CSyncIL::SetNeedResp(bool bNeedResp)
{
    return ( (stSyncIL*)GetDataPtr() )->bNeedResp = bNeedResp;
}


bool CSyncIL::GetNeedResp() const
{
    return ( (stSyncIL*)GetDataPtr() )->bNeedResp;
}


void CSyncIL::SetEntry(const UTILEntry &Entry)
{
    memcpy( (void*)&( ( (stSyncIL*)GetDataPtr() )->Entry ), &Entry, sizeof( UTILEntry ) );
}


UTILEntry& CSyncIL::GetEntry() const
{
    return ( (stSyncIL*)GetDataPtr() )->Entry;
}


//----------------------------------------------------
//CSyncILResp类定义
//----------------------------------------------------


UINT32 CSyncILResp::SetEidInPayload(UINT32 ulEid)
{
    return ( (stSyncILResp*)GetDataPtr() )->ulEid = ulEid;
}

UINT32 CSyncILResp::GetEidInPayload() const
{
    return ( (stSyncILResp*)GetDataPtr() )->ulEid;
}

void CSyncILResp::SetMac(const UINT8* pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stSyncILResp*)GetDataPtr() )->aucMac, pMac, M_MAC_ADDRLEN );
}


UINT8* CSyncILResp::GetMac() const
{
    return ( (stSyncILResp*)GetDataPtr() )->aucMac;
}


UINT8 CSyncILResp::SetIpType(UINT8 ucIpType)
{
    return ( (stSyncILResp*)GetDataPtr() )->ucIpType = ucIpType;
}


UINT8 CSyncILResp::GetIpType() const
{
    return ( (stSyncILResp*)GetDataPtr() )->ucIpType;
}

bool CSyncILResp::SetResult(bool bSuccess)
{
    return ( (stSyncILResp*)GetDataPtr() )->bSuccess = bSuccess;
}

bool CSyncILResp::GetResult() const
{
    return ( (stSyncILResp*)GetDataPtr() )->bSuccess;
}

/*============================================================
MEMBER FUNCTION:
    CSyncILResp::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CSyncILResp::GetDefaultDataLen() const
{
    return sizeof( stSyncILResp );
}


UINT16 CSyncILResp::GetDefaultMsgId() const
{
    return MSGID_IPLIST_SYNC_RESP;
}
