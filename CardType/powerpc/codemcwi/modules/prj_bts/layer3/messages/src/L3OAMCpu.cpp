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

#ifndef _INC_L3OACPUALARMNOTIFY
#include "L3OAMCpuAlarmNofity.h"
#endif


CL3CpuAlarmNofity :: CL3CpuAlarmNofity()
{
}

CL3CpuAlarmNofity :: CL3CpuAlarmNofity(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3CpuAlarmNofity :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_OAM_CPU_RESET_NOTIFY);
    return true;
}

UINT32 CL3CpuAlarmNofity :: GetDefaultDataLen() const
{
    return (sizeof(T_Notify));
}

UINT16 CL3CpuAlarmNofity :: GetTransactionId()const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CL3CpuAlarmNofity :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

const T_CpuAlarmNofity* CL3CpuAlarmNofity ::  GetCpuAlarmNofity()const
{
    return (T_CpuAlarmNofity*)&(((T_Notify*)GetDataPtr())->CpuAlarmNofity);
}

CL3CpuAlarmNofity :: ~CL3CpuAlarmNofity()
{

}
