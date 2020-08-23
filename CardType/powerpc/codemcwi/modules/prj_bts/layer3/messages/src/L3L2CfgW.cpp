/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMCOMMONREQ
#include "L3L2CfgWanIfCpeEidReq.h"
#endif


CL3OamCfgWanIfCpeEidReq :: CL3OamCfgWanIfCpeEidReq()
{
}

CL3OamCfgWanIfCpeEidReq :: CL3OamCfgWanIfCpeEidReq(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3OamCfgWanIfCpeEidReq :: CreateMessage(CComEntity& Entity)
{
    return CMessage::CreateMessage(Entity);
}

UINT32 CL3OamCfgWanIfCpeEidReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CL3OamCfgWanIfCpeEidReq :: GetTransactionId() const
{
    return ((T_Req*)GetDataPtr())->TransId;
}

UINT16 CL3OamCfgWanIfCpeEidReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT32 CL3OamCfgWanIfCpeEidReq::GetWanCpeEid() const
{
    return ((T_Req*)GetDataPtr())->WanIfCpeEID;
}
UINT32 CL3OamCfgWanIfCpeEidReq::SetWanCpeEid(UINT32 eid)
{
    ((T_Req *)GetDataPtr())->WanIfCpeEID = eid;
}


CL3OamCfgWanIfCpeEidReq :: ~CL3OamCfgWanIfCpeEidReq()
{

}


