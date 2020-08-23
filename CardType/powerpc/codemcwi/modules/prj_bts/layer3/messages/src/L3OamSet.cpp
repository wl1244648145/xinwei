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

#ifndef _INC_L3CPEMESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3OAMSETCPETIMENOTIFY
#include "L3OamSetCpeTimeNotify.h"
#endif


CSetCpeTimeNotify :: CSetCpeTimeNotify()
{
}

CSetCpeTimeNotify :: CSetCpeTimeNotify(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CSetCpeTimeNotify :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage::CreateMessage(Entity))
        return false;
    SetMessageId(M_L2_L3_TIMEDAY_NOTIFY);
    return true;
}

UINT32 CSetCpeTimeNotify :: GetDefaultDataLen() const
{
    return sizeof(T_NotifyTime);
}


UINT16 CSetCpeTimeNotify :: GetTransactionId() const
{
    return ((T_NotifyTime *)GetDataPtr())->TransId;
}

UINT16 CSetCpeTimeNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_NotifyTime *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CSetCpeTimeNotify :: GetYear() const
{
    return ((T_NotifyTime *)GetDataPtr())->Year;
}

void CSetCpeTimeNotify :: SetYear(UINT16 year)
{
    ((T_NotifyTime *)GetDataPtr())->Year = year;
}

UINT8 CSetCpeTimeNotify :: GetMonth() const
{
    return ((T_NotifyTime *)GetDataPtr())->Month;
}

void CSetCpeTimeNotify :: SetMonth(UINT8 Mon)
{
    ((T_NotifyTime *)GetDataPtr())->Month = Mon;
}

UINT8 CSetCpeTimeNotify :: GetDay() const
{
    return ((T_NotifyTime *)GetDataPtr())->Day;
}

void CSetCpeTimeNotify :: SetDay(UINT8 D)
{
    ((T_NotifyTime *)GetDataPtr())->Day = D;
}

UINT8 CSetCpeTimeNotify :: GetHour() const
{
    return ((T_NotifyTime *)GetDataPtr())->Hour;
}

void CSetCpeTimeNotify :: SetHour(UINT8 H)
{
    ((T_NotifyTime *)GetDataPtr())->Hour = H;
}

UINT8 CSetCpeTimeNotify :: GetMinute() const
{
    return ((T_NotifyTime *)GetDataPtr())->Minute;
}

void CSetCpeTimeNotify :: SetMinute(UINT8 Min)
{
    ((T_NotifyTime *)GetDataPtr())->Minute = Min;
}

UINT8 CSetCpeTimeNotify :: GetSecond() const
{
    return ((T_NotifyTime *)GetDataPtr())->Second;
}

void CSetCpeTimeNotify :: SetSecond(UINT8 Sec)
{
    ((T_NotifyTime *)GetDataPtr())->Second = Sec;
}

CSetCpeTimeNotify :: ~CSetCpeTimeNotify()
{

}

