/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMFTEntryExpire.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   04/27/07   xin wang      initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "L3DataMFTEntryExpire.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"


//------------------------------------------
//------------------------------------------
void CMFTEntryExpire::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stMFTEntryExpire*)GetDataPtr() )->MAC, pMac, M_MAC_ADDRLEN );
    return;
}


//------------------------------------------
//------------------------------------------
UINT8* CMFTEntryExpire::GetMac() const
{
    return ( (stMFTEntryExpire*)GetDataPtr() )->MAC;
}


/*============================================================
MEMBER FUNCTION:
    CMFTEntryExpire::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CMFTEntryExpire::GetDefaultDataLen() const
{
    return sizeof( stMFTEntryExpire );
}

UINT16 CMFTEntryExpire::GetDefaultMsgId() const
{
    return MSGID_MFT_ENTRY_EXPIRE;
}
#if 0
void CMFTEntryExpire::SetType(UINT8 type)
{
    ( (stMFTEntryExpire*)GetDataPtr() )->TYPE = type;
	return ;
}

UINT8 CMFTEntryExpire::GetType() const
{
	return ( (stMFTEntryExpire*)GetDataPtr() )->TYPE;
}
#endif
