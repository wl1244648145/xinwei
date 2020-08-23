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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/
#include <stdio.h>

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMALMNOTIFYOAM
#include "L3OamAlmNotifyOam.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

#include <string.h>


CAlarmNotifyOam:: CAlarmNotifyOam()
{
}

CAlarmNotifyOam :: CAlarmNotifyOam(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CAlarmNotifyOam :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;

    SetMessageId(M_BTS_EMS_ALM_NOTIFY);
    return true;
}

UINT32 CAlarmNotifyOam :: GetDefaultDataLen() const
{
    return sizeof(T_AlarmNotify);
}

UINT16 CAlarmNotifyOam :: GetTransactionId() const
{
    return ((T_AlarmNotify *)GetDataPtr())->TransId;
}

UINT16 CAlarmNotifyOam :: SetTransactionId(UINT16 TransId)
{
    ((T_AlarmNotify *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT32 CAlarmNotifyOam :: GetSequenceNum()
{
    return ((T_AlarmNotify *)GetDataPtr())->SequenceNum;
}

void CAlarmNotifyOam :: SetSequenceNum(UINT32 Num)
{
    ((T_AlarmNotify *)GetDataPtr())->SequenceNum = Num;
}


UINT8 CAlarmNotifyOam :: GetFlag()
{
    return ((T_AlarmNotify *)GetDataPtr())->Flag;
}

void CAlarmNotifyOam :: SetFlag(UINT8 F)
{
    ((T_AlarmNotify *)GetDataPtr())->Flag = F;
}


UINT16 CAlarmNotifyOam :: GetYear()
{
    return ((T_AlarmNotify *)GetDataPtr())->Year;
}

void CAlarmNotifyOam :: SetYear(UINT16 TransId)
{
    ((T_AlarmNotify *)GetDataPtr())->Year = TransId;
}

UINT8 CAlarmNotifyOam :: GetMonth()
{
    return ((T_AlarmNotify *)GetDataPtr())->Month;
}

void CAlarmNotifyOam :: SetMonth(UINT8 Mon)
{
    ((T_AlarmNotify *)GetDataPtr())->Month = Mon;
}

UINT8 CAlarmNotifyOam :: GetDay()
{
    return ((T_AlarmNotify *)GetDataPtr())->Day;
}

void CAlarmNotifyOam :: SetDay(UINT8 D)
{
    ((T_AlarmNotify *)GetDataPtr())->Day = D;
}

UINT8 CAlarmNotifyOam :: GetHour()
{
    return ((T_AlarmNotify *)GetDataPtr())->Hour;
}

void CAlarmNotifyOam :: SetHour(UINT8 H)
{
    ((T_AlarmNotify *)GetDataPtr())->Hour = H;
}

UINT8 CAlarmNotifyOam :: GetMinute()
{
    return ((T_AlarmNotify *)GetDataPtr())->Minute;
}

void CAlarmNotifyOam :: SetMinute(UINT8 Min)
{
    ((T_AlarmNotify *)GetDataPtr())->Minute = Min;
}

UINT8 CAlarmNotifyOam :: GetSecond()
{
    return ((T_AlarmNotify *)GetDataPtr())->Second;
}

void CAlarmNotifyOam :: SetSecond(UINT8 Sec)
{
    ((T_AlarmNotify *)GetDataPtr())->Second = Sec;
}

UINT16 CAlarmNotifyOam :: GetEntityType()
{
    return ((T_AlarmNotify *)GetDataPtr())->EntityType;
}

void CAlarmNotifyOam :: SetEntityType(UINT16 Type)
{
    ((T_AlarmNotify *)GetDataPtr())->EntityType = Type;
}

UINT16 CAlarmNotifyOam :: GetEntityIndex()
{
    return ((T_AlarmNotify *)GetDataPtr())->EntityIndex;
}

void CAlarmNotifyOam :: SetEntityIndex(UINT16 Index)
{
    ((T_AlarmNotify *)GetDataPtr())->EntityIndex = Index;
}

UINT16 CAlarmNotifyOam :: GetAlarmCode()
{
    return ((T_AlarmNotify *)GetDataPtr())->AlarmCode;
}

void CAlarmNotifyOam :: SetAlarmCode(UINT16 Index)
{
    ((T_AlarmNotify *)GetDataPtr())->AlarmCode = Index;
}

UINT8 CAlarmNotifyOam :: GetSeverity()
{
    return ((T_AlarmNotify *)GetDataPtr())->Severity;
}

void CAlarmNotifyOam :: SetSeverity(UINT8 Sec)
{
    ((T_AlarmNotify *)GetDataPtr())->Severity = Sec;
}

UINT16 CAlarmNotifyOam :: GetInfoLen()
{
    return ((T_AlarmNotify *)GetDataPtr())->InfoLen;
}

void CAlarmNotifyOam :: SetInfoLen(UINT16 Len)
{
    ((T_AlarmNotify *)GetDataPtr())->InfoLen = Len;
}

SINT8* CAlarmNotifyOam :: GetAlarmInfo()
{
    return ((T_AlarmNotify *)GetDataPtr())->AlarmInfo;
}

void CAlarmNotifyOam :: SetAlarmInfo(const SINT8* Info)
{
    strcpy(((T_AlarmNotify *)GetDataPtr())->AlarmInfo, Info);
}

void CAlarmNotifyOam ::SetAlarmLength()
{
    //这个函数必须在SetInfoLen()之后调用
    UINT16 len = sizeof(T_AlarmNotify) - ALM_INFO_LEN + GetInfoLen();
    SetDataLength(len);
}


CAlarmNotifyOam :: ~CAlarmNotifyOam()
{
}

void CAlarmNotifyOam :: show()
{
    printf("\r\n-------------------------------");
    printf("\r\nseq no: %d",GetSequenceNum());
    printf("\r\nflag: %s",(1 == GetFlag())?"set":"clear");
    UINT8 strTime[24]={0};
    sprintf((char*)strTime, "%4d/%2d/%2d %2d:%2d:%2d", 
        GetYear(),
        GetMonth(),
        GetDay(),
        GetHour(),
        GetMinute(),
        GetSecond());
    printf("\r\ntime: %s", strTime);
    UINT16 usCode = GetAlarmCode();
    printf("\r\nAlarm Id: %d(0x%X)", usCode, usCode);
    printf("\r\nseverity: %d", GetSeverity());
    printf("\r\nalarm::\r\n");
    printf(GetAlarmInfo());
}

