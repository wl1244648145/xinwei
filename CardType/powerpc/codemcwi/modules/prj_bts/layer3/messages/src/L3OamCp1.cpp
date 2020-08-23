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

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMCPELOCATIONNOTIFY
#include "L3OamCpeLocationNotify.h"         
#endif


CCpeLocationNotify :: CCpeLocationNotify(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCpeLocationNotify :: CCpeLocationNotify()
{
}

bool CCpeLocationNotify :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_UT_MOVEAWAY_NOTIFY);
	return true;
}

UINT32 CCpeLocationNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CCpeLocationNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CCpeLocationNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CCpeLocationNotify :: GetCPEID() const
{
    return ((T_Notify *)GetDataPtr())->CPEID;
}

void CCpeLocationNotify :: SetCPEID(UINT32 E)
{
    ((T_Notify *)GetDataPtr())->CPEID = E;
}

CCpeLocationNotify :: ~CCpeLocationNotify()
{

}


