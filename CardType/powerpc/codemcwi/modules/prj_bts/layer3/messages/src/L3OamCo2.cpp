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

#ifndef _INC_L3OAMCOMMONRSPFROMCPE
#include "L3OamCommonRspFromCpe.h"
#endif


CL3OamCommonRspFromCpe :: CL3OamCommonRspFromCpe()
{
}

CL3OamCommonRspFromCpe :: CL3OamCommonRspFromCpe(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3OamCommonRspFromCpe :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    return true;
}

UINT32 CL3OamCommonRspFromCpe :: GetDefaultDataLen() const
{
    return sizeof(T_Rsp);
}

UINT16 CL3OamCommonRspFromCpe :: GetTransactionId() const
{
    return ((T_Rsp*)GetDataPtr())->TransId;
}

UINT16 CL3OamCommonRspFromCpe :: SetTransactionId(UINT16 TransId)
{
    ((T_Rsp *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT16 CL3OamCommonRspFromCpe :: GetResult() const
{
    return ((T_Rsp*)GetDataPtr())->Result;
}

void CL3OamCommonRspFromCpe :: SetResult(UINT16 r)
{
    ((T_Rsp *)GetDataPtr())->Result = r;
}

CL3OamCommonRspFromCpe :: ~CL3OamCommonRspFromCpe()
{

}
#if 0
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

CL3OamCommonRspFromCpeZ:: CL3OamCommonRspFromCpeZ()
{
}

CL3OamCommonRspFromCpeZ :: CL3OamCommonRspFromCpeZ(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3OamCommonRspFromCpeZ :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    return true;
}

UINT32 CL3OamCommonRspFromCpeZ :: GetDefaultDataLen() const
{
    return sizeof(T_ZRsp);
}

UINT16 CL3OamCommonRspFromCpeZ :: GetTransactionId() const
{
    return ((T_ZRsp*)GetDataPtr())->TransId;
}

UINT16 CL3OamCommonRspFromCpeZ :: SetTransactionId(UINT16 TransId)
{
    ((T_ZRsp *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT16 CL3OamCommonRspFromCpeZ :: GetResult() const
{
    return ((T_ZRsp*)GetDataPtr())->Result;
}
UINT16 CL3OamCommonRspFromCpeZ :: GetZPid() const
{
    return ((T_ZRsp*)GetDataPtr())->ZPid;
}

void CL3OamCommonRspFromCpeZ :: SetResult(UINT16 r)
{
    ((T_ZRsp *)GetDataPtr())->Result = r;
}

CL3OamCommonRspFromCpeZ :: ~CL3OamCommonRspFromCpeZ()
{

}
#endif
