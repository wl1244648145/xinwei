/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataRoam.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/06/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "L3DataRoam.h"
#include "L3DataMsgId.h"

//----------------------------------------------------
//CRoam类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CRoam::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CRoam::GetDefaultDataLen() const
{
    return sizeof( stRoamReq );
}

UINT16 CRoam::GetDefaultMsgId() const
{
    return MSGID_ROAM_REQ;
}

UINT32 CRoam::SetEidInPayload(UINT32 ulEid)
{
    return ( (stRoamReq*)GetDataPtr() )->ulEid = ulEid;
}

UINT32 CRoam::GetEidInPayload() const
{
    return ( (stRoamReq*)GetDataPtr() )->ulEid;
}


void CRoam::SetEntry(const UTILEntry &Entry)
{
    memcpy( (void*)&( ( (stRoamReq*)GetDataPtr() )->Entry ), &Entry, sizeof( UTILEntry ) );
}


UTILEntry& CRoam::GetEntry() const
{
    return ( (stRoamReq*)GetDataPtr() )->Entry;
}
