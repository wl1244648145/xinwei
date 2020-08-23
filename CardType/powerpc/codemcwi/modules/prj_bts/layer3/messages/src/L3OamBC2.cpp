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

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMBCCPESWRSP
#include "L3OamBCCPESWRsp.h"
#endif

#if 0
CBCUTSWRsp :: CBCUTSWRsp(CMessage &rMsg)
:CMessage(rMsg)
{
}

CBCUTSWRsp :: CBCUTSWRsp()
{
}

bool CBCUTSWRsp :: CreateMessage(CComEntity &ComEntity)
{
    CMessage :: CreateMessage(ComEntity);
    SetMessageId(M_BTS_EMS_BC_UPGRADE_UT_SW_RSP);
    
    return true;
}

UINT32 CBCUTSWRsp :: GetDefaultDataLen() const
{
    return sizeof(T_Rsp);
}

UINT16 CBCUTSWRsp :: GetTransactionId() const
{
    return ((T_Rsp *)GetDataPtr())->TransId;
}

UINT16 CBCUTSWRsp :: SetTransactionId(UINT16 Id)
{
    ((T_Rsp *)GetDataPtr())->TransId = Id;
	return 0;
}

UINT16 CBCUTSWRsp :: GetResult()
{
    return ((T_Rsp *)GetDataPtr())->Result;
}

void CBCUTSWRsp :: SetResult(UINT16 Res)
{
    ((T_Rsp *)GetDataPtr())->Result = Res;
}

CBCUTSWRsp :: ~CBCUTSWRsp()
{
}

#endif
