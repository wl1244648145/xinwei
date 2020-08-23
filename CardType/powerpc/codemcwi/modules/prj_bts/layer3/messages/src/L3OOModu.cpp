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

#ifndef _INC_L3OOModuleInitNOTIFY
#include "L3OOModuleInitNotify.h"
#endif

CCfgModuleInitNotify :: CCfgModuleInitNotify(CMessage &rMsg)
:CMessage(rMsg)
{}

CCfgModuleInitNotify :: CCfgModuleInitNotify()
{}

bool CCfgModuleInitNotify :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_OAM_DATA_CFG_INIT_NOTIFY);
    return true;
}

UINT32 CCfgModuleInitNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CCfgModuleInitNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CCfgModuleInitNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8 CCfgModuleInitNotify :: GetType() const
{
    return ((T_Notify *)GetDataPtr())->Type;
}

void CCfgModuleInitNotify :: SetType(UINT8 T)
{
    ((T_Notify *)GetDataPtr())->Type = T;
}

CCfgModuleInitNotify :: ~CCfgModuleInitNotify()
{}
