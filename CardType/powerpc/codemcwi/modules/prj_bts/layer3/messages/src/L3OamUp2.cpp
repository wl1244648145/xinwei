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

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMUPDATECPESWRESULTNOTIFY
#include "L3OamUpdateCpeSWResultNotify.h"
#endif

CUpdateUTSWResultNotify :: CUpdateUTSWResultNotify(CMessage &rMsg)
    :CMessage(rMsg)
{  }

CUpdateUTSWResultNotify :: CUpdateUTSWResultNotify()
{
}

bool CUpdateUTSWResultNotify :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_BTS_EMS_UPGRADE_UT_SW_NOTIFY);
	return true;
}

UINT32 CUpdateUTSWResultNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CUpdateUTSWResultNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CUpdateUTSWResultNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}


UINT32 CUpdateUTSWResultNotify :: GetCPEID()
{
    return ((T_Notify *)GetDataPtr())->CPEID;
}

void CUpdateUTSWResultNotify :: SetCPEID(UINT32 Id)
{
    ((T_Notify*)GetDataPtr())->CPEID = Id;
}

UINT16 CUpdateUTSWResultNotify :: GetResult()
{
    return ((T_Notify *)GetDataPtr())->Result;
}

void CUpdateUTSWResultNotify :: SetResult(UINT16 R)
{
    ((T_Notify*)GetDataPtr())->Result = R;
}

CUpdateUTSWResultNotify :: ~CUpdateUTSWResultNotify()
{
}
