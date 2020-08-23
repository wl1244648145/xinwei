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

#ifndef _INC_L3OAMCOMMONRSP
#include "L3OamCommonRsp.h"
#endif


bool CL3OamCommonRsp :: CreateMessage(CComEntity& Entity)
{
    return CMessage::CreateMessage(Entity);
}

bool CL3OamCommonRsp :: CreateMessage(CComEntity &ComEntity, UINT32 Len)
{
    return CMessage :: CreateMessage(ComEntity, Len);
}

UINT32 CL3OamCommonRsp :: GetDefaultDataLen() const
{
    return sizeof(T_Rsp);
}

UINT16 CL3OamCommonRsp :: GetTransactionId() const
{
    return ((T_Rsp*)GetDataPtr())->TransId;
}

UINT16 CL3OamCommonRsp :: SetTransactionId(UINT16 TransId)
{
    ((T_Rsp *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT16 CL3OamCommonRsp :: GetResult() const
{
    return ((T_Rsp*)GetDataPtr())->Result;
}

void CL3OamCommonRsp :: SetResult(UINT16 r)
{
    ((T_Rsp *)GetDataPtr())->Result = r;
}

bool CL3OamProbeCPERsp::CreateMessage(CComEntity& Entity)
{
    return CL3OamCommonRsp::CreateMessage(Entity);
}

UINT32 CL3OamProbeCPERsp::GetDefaultDataLen() const
{
    return sizeof(T_ProbeCPE);
}

void CL3OamProbeCPERsp::setRspEID(UINT32 eid)
{
    ((T_ProbeCPE *)GetDataPtr())->eid = eid;
}
//wangwenhua add 20081119

bool CL3OamQueryCPENetWorkRsp::CreateMessage(CComEntity& Entity)
{
    return CL3OamCommonRsp::CreateMessage(Entity);
}

UINT32 CL3OamQueryCPENetWorkRsp::GetDefaultDataLen() const
{
    return sizeof(T_QueryCPENetWork);
}

void CL3OamQueryCPENetWorkRsp::setRspEID(UINT32 eid)
{
    ((T_QueryCPENetWork *)GetDataPtr())->eid = eid;
}

void CL3OamQueryCPENetWorkRsp::setRspType(UINT16 type)
{
    ((T_QueryCPENetWork *)GetDataPtr())->type = type;
}

//wangwenhua add end 20081119
