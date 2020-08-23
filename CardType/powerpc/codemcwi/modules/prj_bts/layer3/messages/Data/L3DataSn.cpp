/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopTimer.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/02/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "L3DataSnoopTimer.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"

//----------------------------------------------
//CTimerExpireMsg类定义
//----------------------------------------------


//------------------------------------------
//------------------------------------------
void CSnoopTimerExpire::SetMac(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( ( (stSNTimerExpire*)GetDataPtr() )->aucMac, pMac, M_MAC_ADDRLEN );
}


//------------------------------------------
//------------------------------------------
UINT8* CSnoopTimerExpire::GetMac() const
{
    return ( (stSNTimerExpire*)GetDataPtr() )->aucMac;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTimerExpire::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CSnoopTimerExpire::GetDefaultDataLen() const
{
    return sizeof( stSNTimerExpire );
}


UINT16 CSnoopTimerExpire::GetDefaultMsgId() const
{
    return MSGID_TIMER_SNOOP;
}
