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

#ifndef _INC_L3OAMBTSRSTNOTIFY
#include "L3OamBtsRstNotify.h"
#endif

CBtsRstNotify :: CBtsRstNotify(CMessage &rMsg)
:CMessage(rMsg)
{
}

CBtsRstNotify :: CBtsRstNotify()
{
}

bool CBtsRstNotify :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_BTS_EMS_RST_NOTIFY);
    return true;
}

UINT32 CBtsRstNotify :: GetDefaultDataLen() const
{
    return sizeof(T_BtsRstNotify);
}


UINT16 CBtsRstNotify :: GetTransactionId() const
{
    return ((T_BtsRstNotify *)GetDataPtr())->TransId;
}

UINT16 CBtsRstNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_BtsRstNotify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CBtsRstNotify :: GetActiveVersion() const
{
    return ((T_BtsRstNotify *)GetDataPtr())->ActiveVersion;
}

void CBtsRstNotify :: SetActiveVersion(UINT32 Version)
{
    ((T_BtsRstNotify *)GetDataPtr())->ActiveVersion = Version;
}
UINT32 CBtsRstNotify :: GetStandbyVersion() const
{
    return ((T_BtsRstNotify *)GetDataPtr())->StandbyVersion;
}

void CBtsRstNotify :: SetStandbyVersion(UINT32 Version)
{
    ((T_BtsRstNotify *)GetDataPtr())->StandbyVersion = Version;
}

UINT16 CBtsRstNotify :: GetBtsHWType() const
{
    return ((T_BtsRstNotify *)GetDataPtr())->BtsHWType;
}

void CBtsRstNotify :: SetBtsHWType(UINT16 HWType)
{
    ((T_BtsRstNotify *)GetDataPtr())->BtsHWType = HWType;
}

UINT16 CBtsRstNotify :: GetBootupSource() const
{
    return ((T_BtsRstNotify *)GetDataPtr())->BootupSource;
}

void CBtsRstNotify :: SetBootupSource(UINT16 Source)
{
    ((T_BtsRstNotify *)GetDataPtr())->BootupSource = Source;
}


UINT16 CBtsRstNotify :: GetResetReason() const
{
    return ((T_BtsRstNotify *)GetDataPtr())->ResetReason;
}

void CBtsRstNotify :: SetResetReason(UINT16 Reason)
{
    ((T_BtsRstNotify *)GetDataPtr())->ResetReason = Reason;
}


CBtsRstNotify :: ~CBtsRstNotify()
{
}



