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

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3OODELETEALARMTIFY
#include "L3OODeleteAlarmNotify.h"
#endif

CDeleteAlarmNotify :: CDeleteAlarmNotify(CMessage &rMsg)
    :CMessage(rMsg)
{  }

CDeleteAlarmNotify :: CDeleteAlarmNotify()
{
}
bool CDeleteAlarmNotify :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_OAM_DELETE_ALM_RECORD_NOTIFY);
	return true;
}

UINT32 CDeleteAlarmNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CDeleteAlarmNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CDeleteAlarmNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CDeleteAlarmNotify :: GetEntityType()
{
    return ((T_Notify *)GetDataPtr())->EntityType;
}

void CDeleteAlarmNotify :: SetEntityType(UINT16 Type)
{
    ((T_Notify *)GetDataPtr())->EntityType = Type;
}

UINT16 CDeleteAlarmNotify :: GetEntityIndex()
{
    return ((T_Notify *)GetDataPtr())->EntityIndex;
}

void CDeleteAlarmNotify :: SetEntityIndex(UINT16 Index)
{
    ((T_Notify *)GetDataPtr())->EntityIndex = Index;
}

UINT16 CDeleteAlarmNotify :: GetAlarmCode()
{
    return ((T_Notify *)GetDataPtr())->AlarmCode;
}

void CDeleteAlarmNotify :: SetAlarmCode(UINT16 Index)
{
    ((T_Notify *)GetDataPtr())->AlarmCode = Index;
}

CDeleteAlarmNotify :: ~CDeleteAlarmNotify()
{
}

