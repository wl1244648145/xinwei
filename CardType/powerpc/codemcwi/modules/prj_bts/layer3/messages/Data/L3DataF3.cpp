/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataFTEntryExpire.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   08/09/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "L3DataFTEntryExpire.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"


//------------------------------------------
//------------------------------------------
void CFTEntryExpire::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stFTEntryExpire*)GetDataPtr() )->aucMac, pMac, M_MAC_ADDRLEN );
    ( (stFTEntryExpire*)GetDataPtr() )->Dm_Sync_Flag = 0;
    return;
}


//------------------------------------------
//------------------------------------------
UINT8* CFTEntryExpire::GetMac() const
{
    return ( (stFTEntryExpire*)GetDataPtr() )->aucMac;
}

//------------------------------------------
//------------------------------------------
void CFTEntryExpire::SetFlag(const UINT8 flag)
{
   
    ( (stFTEntryExpire*)GetDataPtr() )->Dm_Sync_Flag = flag;
    return;
}


//------------------------------------------
//------------------------------------------
UINT8 CFTEntryExpire::GetFlag() const
{
    return ( (stFTEntryExpire*)GetDataPtr() )->Dm_Sync_Flag;
}
/*============================================================
MEMBER FUNCTION:
    CFTEntryExpire::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CFTEntryExpire::GetDefaultDataLen() const
{
    return sizeof( stFTEntryExpire );
}


//------------------------------------------
//------------------------------------------
UINT16 CFTEntryExpire::GetDefaultMsgId() const
{
    return MSGID_FT_ENTRY_EXPIRE;
}
