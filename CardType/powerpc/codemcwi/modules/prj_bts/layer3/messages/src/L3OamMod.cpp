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

#ifndef _INC_L3OAMMODULEBOOTUPNOTIFY
#include "L3OamModuleBootupNotify.h"
#endif

CModuleBootupNotify :: CModuleBootupNotify(CMessage &rMsg)    
    :CMessage(rMsg)
{
}

CModuleBootupNotify :: CModuleBootupNotify()
{
}

bool CModuleBootupNotify :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_OAM_SYSTEM_RUNNING_NOTIFY);
    return true;
}

UINT32 CModuleBootupNotify :: GetDefaultDataLen() const
{
    return sizeof(T_BootupInfo);
}

UINT16 CModuleBootupNotify :: GetTransactionId() const
{
    return ((T_BootupInfo*)GetDataPtr())->TransId;
}

UINT16 CModuleBootupNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_BootupInfo *)GetDataPtr())->TransId = TransId;
	return 0;
}

CModuleBootupNotify :: ~CModuleBootupNotify()
{
}

