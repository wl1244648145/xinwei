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

#ifndef _INC_L3OAMCFGINITFAILNOTIFY
#include "L3OamCfgInitFailNotify.h"     
#endif


CCfgInitFailNotify :: CCfgInitFailNotify(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgInitFailNotify :: CCfgInitFailNotify()
{

}

bool CCfgInitFailNotify :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_OAM_CFGDATAINIT_FAIL_NOTIFY);

	return true;
}

UINT32 CCfgInitFailNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);//sizeof(T_GpsDataCfgReq); transid len
}

UINT16 CCfgInitFailNotify :: GetTransactionId() const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CCfgInitFailNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgInitFailNotify :: GetResult() const
{
    return ((T_Notify *)GetDataPtr())->Result;
}

void CCfgInitFailNotify :: SetResult(UINT16 Result)
{
    ((T_Notify*)GetDataPtr())->Result = Result;
}

CCfgInitFailNotify :: ~CCfgInitFailNotify()
{

}



