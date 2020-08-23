/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataDelEidTable.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/14/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include "L3DataDelEidTable.h"
#include "L3DataMsgId.h"
#include "L3DataAssert.h"

//----------------------------------------------------
//CDelEidTable类定义
//----------------------------------------------------



/*============================================================
MEMBER FUNCTION:
    CDelEidTable::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CDelEidTable::GetDefaultDataLen() const
{
    return sizeof( stDelEidTable );
}


UINT16 CDelEidTable::GetDefaultMsgId() const
{
    return MSGID_EID_DEL_TABLE;
}


UINT32 CDelEidTable::SetEidInPayload(UINT32 ulEid)
{
    return ( (stDelEidTable*)GetDataPtr() )->ulEid = ulEid;
}

UINT32 CDelEidTable::GetEidInPayload() const
{
    return ( (stDelEidTable*)GetDataPtr() )->ulEid;
}

