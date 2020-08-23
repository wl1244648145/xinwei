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

#ifndef _INC_L3OAMUPDATECPESWRARENOTIFY
#include "L3OamUpdateCpeSWRateNotify.h"

#endif

CUpdateCpeSWRateNotify :: CUpdateCpeSWRateNotify(CMessage &rMsg)
    :CMessage(rMsg)
{  }
CUpdateCpeSWRateNotify :: CUpdateCpeSWRateNotify()
{
}
bool CUpdateCpeSWRateNotify :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_BTS_EMS_UPGRADE_UT_SW_PROGRESS);
	return true;
}

UINT32 CUpdateCpeSWRateNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CUpdateCpeSWRateNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CUpdateCpeSWRateNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}


UINT32 CUpdateCpeSWRateNotify :: GetCPEID()
{
    return ((T_Notify *)GetDataPtr())->CPEID;
}

void CUpdateCpeSWRateNotify :: SetCPEID(UINT32 Id)
{
    ((T_Notify*)GetDataPtr())->CPEID = Id;
}

UINT8 CUpdateCpeSWRateNotify :: GetProgress()
{
    return ((T_Notify *)GetDataPtr())->Progress;
}

void CUpdateCpeSWRateNotify :: SetProgress(UINT8 Rate)
{
    ((T_Notify*)GetDataPtr())->Progress = Rate;
}


CUpdateCpeSWRateNotify :: ~CUpdateCpeSWRateNotify()
{
}
