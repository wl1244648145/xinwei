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

#ifndef _INC_L3OAMCPUWORKINGNOTIFY
#include "L3OamCpuWorkingNotify.h"
#endif

#ifndef _INC_L3OAMMESSAGEID
#include "L3OAMMessageId.h"
#endif


CL3OamCpuWorkingNotify :: CL3OamCpuWorkingNotify(CMessage &rMsg)
:CMessage(rMsg)
{  }

CL3OamCpuWorkingNotify :: CL3OamCpuWorkingNotify()
{
}

bool CL3OamCpuWorkingNotify :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_OAM_CPU_WORKING_NOTIFY);
    return true;
}

UINT32 CL3OamCpuWorkingNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CL3OamCpuWorkingNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CL3OamCpuWorkingNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8 CL3OamCpuWorkingNotify :: GetCpuType() const
{
    return (((T_Notify *)GetDataPtr())->Ele).CpuType;
}

void CL3OamCpuWorkingNotify :: SetCpuType(UINT8 E)
{
    (((T_Notify *)GetDataPtr())->Ele).CpuType = E;
}


UINT8 CL3OamCpuWorkingNotify :: GetCpuIndex() const
{
    return (((T_Notify *)GetDataPtr())->Ele).CpuIndex;
}

void CL3OamCpuWorkingNotify :: SetCpuIndex(UINT8 E)
{
    (((T_Notify *)GetDataPtr())->Ele).CpuIndex = E;
}


CL3OamCpuWorkingNotify :: ~CL3OamCpuWorkingNotify()
{

}

