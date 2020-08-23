/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataFTCheckVLAN.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   06/01/06   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include "L3DataFTCheckVLAN.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"


//------------------------------------------
//------------------------------------------
void CFTCheckVLAN::SetVlanID(UINT16 usVlanID)
{
    ( (stFTCheckVLAN*)GetDataPtr() )->usVlanID = usVlanID;
}


//------------------------------------------
//------------------------------------------
UINT16 CFTCheckVLAN::GetVlanID() const
{
    return ( (stFTCheckVLAN*)GetDataPtr() )->usVlanID;
}


/*============================================================
MEMBER FUNCTION:
    CFTCheckVLAN::GetDefaultDataLen

DESCRIPTION:
    ��ȡCMessage Payload�ĳ���

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: ����

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CFTCheckVLAN::GetDefaultDataLen() const
{
    return sizeof( stFTCheckVLAN );
}


//------------------------------------------
//------------------------------------------
UINT16 CFTCheckVLAN::GetDefaultMsgId() const
{
    return MSGID_FT_CHECK_VLAN;
}

