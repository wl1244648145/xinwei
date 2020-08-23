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
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L3CPESWDLPACKRSP
#include "L3L3CpeSWDLPackRsp.h"
#endif


CL3CpeSWDLPackRsp :: CL3CpeSWDLPackRsp()
{
}

CL3CpeSWDLPackRsp :: CL3CpeSWDLPackRsp(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3CpeSWDLPackRsp :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_CPE_L3_UPGRADE_SW_PACK_RSP);
    return true;
}

UINT32 CL3CpeSWDLPackRsp :: GetDefaultDataLen() const
{
    return sizeof(T_Rsp);
}

UINT16 CL3CpeSWDLPackRsp :: GetTransactionId() const
{
    return ((T_Rsp *)GetDataPtr())->TransId;
}

UINT16 CL3CpeSWDLPackRsp :: SetTransactionId(UINT16 TransId)
{
    ((T_Rsp *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16  CL3CpeSWDLPackRsp :: GetSWPackSeqNum() const
{
    return ((T_Rsp *)GetDataPtr())->SWPackSeqNum;
}

void  CL3CpeSWDLPackRsp :: SetSWPackSeqNum(UINT16 E)
{
    ((T_Rsp *)GetDataPtr())->SWPackSeqNum = E;
}

CL3CpeSWDLPackRsp :: ~CL3CpeSWDLPackRsp()
{

}


