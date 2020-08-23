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

#ifndef _INC_L3OAMCPESWITCHOFFNOTIFY
#include "L3OamCpeSwitchOffNotify.h"
#endif

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

CCpeSwitchOffNotify :: CCpeSwitchOffNotify(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCpeSwitchOffNotify :: CCpeSwitchOffNotify()
{
}

bool CCpeSwitchOffNotify :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_BTS_EMS_UT_SWITCH_OFF_NOTIFY);
    return true;
}

UINT32 CCpeSwitchOffNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CCpeSwitchOffNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CCpeSwitchOffNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CCpeSwitchOffNotify :: GetCPEID() const
{
    return ((T_Notify*)GetDataPtr())->CPEID;
}

void CCpeSwitchOffNotify :: SetCPEID(UINT32 E)
{
    ((T_Notify*)GetDataPtr())->CPEID = E;
}

CCpeSwitchOffNotify :: ~CCpeSwitchOffNotify()
{
}
