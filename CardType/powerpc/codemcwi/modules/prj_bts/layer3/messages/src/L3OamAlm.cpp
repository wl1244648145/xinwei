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

#ifndef _INC_L3OAMALMNOTIFYEMS
#include "L3OamAlmNotifyEms.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

#include <string.h>

CAlarmNotifyEms :: CAlarmNotifyEms()
{
}

CAlarmNotifyEms :: CAlarmNotifyEms(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CAlarmNotifyEms :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_BTS_EMS_ALM_NOTIFY);
    return true;
}

UINT32 CAlarmNotifyEms :: GetDefaultDataLen() const
{
    return sizeof(T_AlarmNotify);
}


UINT16 CAlarmNotifyEms :: GetTransactionId() const
{
    return ((T_AlarmNotify *)GetDataPtr())->TransId;
}

UINT16 CAlarmNotifyEms :: SetTransactionId(UINT16 TransId)
{
    ((T_AlarmNotify *)GetDataPtr())->TransId = TransId;
	return 0;
}


UINT32 CAlarmNotifyEms :: GetSequenceNum()
{
    return ((T_AlarmNotify *)GetDataPtr())->SequenceNum;
}

void CAlarmNotifyEms :: SetSequenceNum(UINT32 Num)
{
    ((T_AlarmNotify *)GetDataPtr())->SequenceNum = Num;
}


UINT8 CAlarmNotifyEms :: GetFlag()
{
    return ((T_AlarmNotify *)GetDataPtr())->Flag;
}

void CAlarmNotifyEms :: SetFlag(UINT8 F)
{
    ((T_AlarmNotify *)GetDataPtr())->Flag = F;
}


UINT16 CAlarmNotifyEms :: GetYear()
{
    return ((T_AlarmNotify *)GetDataPtr())->Year;
}

void CAlarmNotifyEms :: SetYear(UINT16 TransId)
{
    ((T_AlarmNotify *)GetDataPtr())->Year = TransId;
}

UINT8 CAlarmNotifyEms :: GetMonth()
{
    return ((T_AlarmNotify *)GetDataPtr())->Month;
}

void CAlarmNotifyEms :: SetMonth(UINT8 Mon)
{
    ((T_AlarmNotify *)GetDataPtr())->Month = Mon;
}

UINT8 CAlarmNotifyEms :: GetDay()
{
    return ((T_AlarmNotify *)GetDataPtr())->Day;
}

void CAlarmNotifyEms :: SetDay(UINT8 D)
{
    ((T_AlarmNotify *)GetDataPtr())->Day = D;
}

UINT8 CAlarmNotifyEms :: GetHour()
{
    return ((T_AlarmNotify *)GetDataPtr())->Hour;
}

void CAlarmNotifyEms :: SetHour(UINT8 H)
{
    ((T_AlarmNotify *)GetDataPtr())->Hour = H;
}

UINT8 CAlarmNotifyEms :: GetMinute()
{
    return ((T_AlarmNotify *)GetDataPtr())->Minute;
}

void CAlarmNotifyEms :: SetMinute(UINT8 Min)
{
    ((T_AlarmNotify *)GetDataPtr())->Minute = Min;
}

UINT8 CAlarmNotifyEms :: GetSecond()
{
    return ((T_AlarmNotify *)GetDataPtr())->Second;
}

void CAlarmNotifyEms :: SetSecond(UINT8 Sec)
{
    ((T_AlarmNotify *)GetDataPtr())->Second = Sec;
}

UINT16 CAlarmNotifyEms :: GetEntityType()
{
    return ((T_AlarmNotify *)GetDataPtr())->EntityType;
}

void CAlarmNotifyEms :: SetEntityType(UINT16 Type)
{
    ((T_AlarmNotify *)GetDataPtr())->EntityType = Type;
}
UINT16 CAlarmNotifyEms :: GetEntityIndex()
{
    return ((T_AlarmNotify *)GetDataPtr())->EntityIndex;
}

void CAlarmNotifyEms :: SetEntityIndex(UINT16 Index)
{
    ((T_AlarmNotify *)GetDataPtr())->EntityIndex = Index;
}

UINT16 CAlarmNotifyEms :: GetAlarmCode()
{
    return ((T_AlarmNotify *)GetDataPtr())->AlarmCode;
}

void CAlarmNotifyEms :: SetAlarmCode(UINT16 Index)
{
    ((T_AlarmNotify *)GetDataPtr())->AlarmCode = Index;
}

UINT16 CAlarmNotifyEms :: GetSeverity()
{
    return ((T_AlarmNotify *)GetDataPtr())->Severity;
}

void CAlarmNotifyEms :: SetSeverity(UINT16 Sec)
{
    ((T_AlarmNotify *)GetDataPtr())->Severity = Sec;
}


UINT8 CAlarmNotifyEms :: GetInfoLen()
{
    return ((T_AlarmNotify *)GetDataPtr())->InfoLen;
}

void CAlarmNotifyEms :: SetInfoLen(UINT8 Len)
{
    ((T_AlarmNotify *)GetDataPtr())->InfoLen = Len;
}

SINT8* CAlarmNotifyEms :: GetAlarmInfo()
{
    return ((T_AlarmNotify *)GetDataPtr())->AlarmInfo;
}

void CAlarmNotifyEms :: SetAlarmInfo(const SINT8* Info)
{
    strcpy(((T_AlarmNotify *)GetDataPtr())->AlarmInfo, Info);
}

CAlarmNotifyEms :: ~CAlarmNotifyEms()
{

}


