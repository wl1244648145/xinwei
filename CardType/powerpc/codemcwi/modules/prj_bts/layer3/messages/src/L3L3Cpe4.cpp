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

#ifndef _INC_L3L3CPESWDLRESULTNOTIFY
#include "L3L3CpeSWDLResultNotify.h"
#endif


CL3CpeSWDLResultNotify :: CL3CpeSWDLResultNotify()
{
}

CL3CpeSWDLResultNotify :: CL3CpeSWDLResultNotify(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3CpeSWDLResultNotify :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_CPE_L3_UPGRADE_SW_FINISH_NOTIFY);
    return true;
}

UINT32 CL3CpeSWDLResultNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CL3CpeSWDLResultNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CL3CpeSWDLResultNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CL3CpeSWDLResultNotify :: GetResult() const
{
    return ((T_Notify *)GetDataPtr())->Result;
}

void CL3CpeSWDLResultNotify :: SetResult(UINT16 R)
{
    ((T_Notify *)GetDataPtr())->Result = R;
}


CL3CpeSWDLResultNotify :: ~CL3CpeSWDLResultNotify()
{

}


