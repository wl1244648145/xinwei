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

#ifndef _INC_L3OAMBTSDATADLOADNOTIFY
#include "L3OamBtsDataDLoadNotify.h"
#endif

CBtsDataDLoadNotify :: CBtsDataDLoadNotify(CMessage &rMsg)    
    :CMessage(rMsg)
{

}

CBtsDataDLoadNotify :: CBtsDataDLoadNotify()
{
}

bool CBtsDataDLoadNotify :: CreateMessage(CComEntity &Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_BTS_EMS_DATA_DL_NOTIFY);
    return true;
}

UINT32 CBtsDataDLoadNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}


UINT16 CBtsDataDLoadNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CBtsDataDLoadNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CBtsDataDLoadNotify :: GetActiveVerion() const
{
    return ((T_Notify *)GetDataPtr())->ActiveVersion;
}

void CBtsDataDLoadNotify :: SetActiveVerion(UINT32 Version)
{
    ((T_Notify *)GetDataPtr())->ActiveVersion = Version;
}


UINT32 CBtsDataDLoadNotify :: GetStandbyVersion() const
{
    return ((T_Notify *)GetDataPtr())->StandbyVersion;
}

void CBtsDataDLoadNotify :: SetStandbyVersion(UINT32 Version) 
{
    ((T_Notify *)GetDataPtr())->StandbyVersion = Version;
}

UINT16 CBtsDataDLoadNotify :: GetBtsHWType() const
{
    return ((T_Notify *)GetDataPtr())->BtsHWType;
}

void CBtsDataDLoadNotify :: SetBtsHWType(UINT16 HWType)
{
    ((T_Notify *)GetDataPtr())->BtsHWType = HWType;
}

CBtsDataDLoadNotify :: ~CBtsDataDLoadNotify()
{
}

