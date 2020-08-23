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
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMBCCPESWRATENOTIFY
#include "L3OamBCCPESWRateNotify.h"
#endif


// Broadcast UT Software Progress Notification£¨BTS£©
#if 0
CBCUTSWRateNotify :: CBCUTSWRateNotify(CMessage &rMsg)
:CMessage(rMsg)
{  }
CBCUTSWRateNotify :: CBCUTSWRateNotify()
{
}
bool CBCUTSWRateNotify :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_BTS_EMS_BC_UPGRADE_UT_SW_PROGRESS);
	return true;
}

UINT32 CBCUTSWRateNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CBCUTSWRateNotify :: GetTransactionId()const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CBCUTSWRateNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}


UINT16 CBCUTSWRateNotify :: GetCpeHWType()
{
    return ((T_Notify *)GetDataPtr())->CpeHWType;
}

void CBCUTSWRateNotify :: SetCpeHWType(UINT16 Type)
{
    ((T_Notify *)GetDataPtr())->CpeHWType = Type;
}


UINT8 CBCUTSWRateNotify :: GetProgress()
{
    return ((T_Notify *)GetDataPtr())->Progress;
}

void CBCUTSWRateNotify :: SetProgress(UINT8 Pro)
{
    ((T_Notify *)GetDataPtr())->Progress = Pro;
}

CBCUTSWRateNotify :: ~CBCUTSWRateNotify()
{
}

#endif
