/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    SetACLConfigReq.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   10/12/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include "L3DataACLConfig.h"



//----------------------------------------------
//L3DataACLConfigÀà¶¨Òå
//----------------------------------------------
UINT32  SetACLConfigReq::GetDefaultDataLen() const
{
    return sizeof(ACLConfigReq);  
}
UINT16  SetACLConfigReq::GetDefaultMsgId() const
{
    return M_EMS_BTS_CFG_ACL_REQ;
}
UINT16  SetACLConfigReq::GetXid() const 
{
    return( (ACLConfigReq*)GetDataPtr() )->Xid;
}
UINT8  SetACLConfigReq::GetIndex() const 
{
    return( (ACLConfigReq*)GetDataPtr() )->Index;
}
UINT8  SetACLConfigReq::GetACLCount() const 
{
    return( (ACLConfigReq*)GetDataPtr() )->ACLCount;
}
const ACLConfig* SetACLConfigReq::GetACLConfigEntry()
{
    return( (ACLConfigReq*)GetDataPtr() )->ACLConfigEntry;
}


