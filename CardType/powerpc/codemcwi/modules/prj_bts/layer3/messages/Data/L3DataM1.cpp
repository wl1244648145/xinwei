/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMFTDelEntry.cpp
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

#include "L3DataMFTDelEntry.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"


//------------------------------------------
//------------------------------------------
void CMFTDelEntry::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stMFTDelEntry*)GetDataPtr() )->MAC, pMac, M_MAC_ADDRLEN );
    return;
}


//------------------------------------------
//------------------------------------------
UINT8* CMFTDelEntry::GetMac() const
{
    return ( (stMFTDelEntry*)GetDataPtr() )->MAC;
}
#if 0
void CMFTDelEntry::SetType(UINT8 type)
{
    ( (stMFTDelEntry*)GetDataPtr() )->TYPE = type;
	return ;
}

UINT8 CMFTDelEntry::GetType() const
{
	return ( (stMFTDelEntry*)GetDataPtr() )->TYPE;
}
#endif
/*============================================================
MEMBER FUNCTION:
    CMFTDelEntry::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CMFTDelEntry::GetDefaultDataLen() const
{
    return sizeof( stMFTDelEntry );
}


//------------------------------------------
//------------------------------------------
UINT16 CMFTDelEntry::GetDefaultMsgId() const
{
    return MSGID_MFT_DEL_ENTRY;
}
