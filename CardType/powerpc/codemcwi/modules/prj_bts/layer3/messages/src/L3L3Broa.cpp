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

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3L3BROADCASTUTSWTIMER
#include "L3L3BroadcastUTSWTimer.h"
#endif


CL3BCUTSWTimer :: CL3BCUTSWTimer()
{
}

CL3BCUTSWTimer :: CL3BCUTSWTimer(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3BCUTSWTimer :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_OAM_BC_UTSW_TIMER);
    return true;
}

UINT32 CL3BCUTSWTimer :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CL3BCUTSWTimer :: GetTransactionId()const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CL3BCUTSWTimer :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}
    
UINT16 CL3BCUTSWTimer :: GetHWType()const
{
    return ((T_Notify *)GetDataPtr())->HWType;
}

void  CL3BCUTSWTimer::  SetHWType(UINT16 type)
{
    ((T_Notify*)GetDataPtr())->HWType = type;
}

CL3BCUTSWTimer :: ~CL3BCUTSWTimer()
{

}

///////////////////////////////////////////////////////////////////////////
// a new broadcast timer
CL3BCUTSWTimerNew :: CL3BCUTSWTimerNew()
{
}

CL3BCUTSWTimerNew :: CL3BCUTSWTimerNew(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3BCUTSWTimerNew :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_OAM_BC_UTSW_TIMER_NEW);
    return true;
}

UINT32 CL3BCUTSWTimerNew :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CL3BCUTSWTimerNew :: GetTransactionId()const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CL3BCUTSWTimerNew :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}
    
UINT16 CL3BCUTSWTimerNew :: GetHWType()const
{
    return ((T_Notify *)GetDataPtr())->HWType;
}

void  CL3BCUTSWTimerNew::  SetHWType(UINT16 type)
{
    ((T_Notify*)GetDataPtr())->HWType = type;
}

CL3BCUTSWTimerNew :: ~CL3BCUTSWTimerNew()
{

}

