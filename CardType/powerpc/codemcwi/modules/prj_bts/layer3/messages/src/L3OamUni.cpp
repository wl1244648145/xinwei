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

#ifndef _INC_L3OAMUNICASTUTSWREQFAIL
#include "L3OamUnicastUTSWReqFail.h"
#endif

CL3OamUnicastUTSWReqFail :: CL3OamUnicastUTSWReqFail(CMessage &rMsg)
    :CMessage(rMsg)
{  }
CL3OamUnicastUTSWReqFail :: CL3OamUnicastUTSWReqFail()
{
}
bool CL3OamUnicastUTSWReqFail :: CreateMessage(CComEntity& Entity, UINT16 usMsgID)
{
	CMessage :: CreateMessage(Entity);
	SetMessageId(usMsgID);
	return true;
}

UINT32 CL3OamUnicastUTSWReqFail :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CL3OamUnicastUTSWReqFail :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CL3OamUnicastUTSWReqFail :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}


UINT32 CL3OamUnicastUTSWReqFail :: GetCPEID() const
{
    return ((T_Notify *)GetDataPtr())->CPEID;
}

void CL3OamUnicastUTSWReqFail :: SetCPEID(UINT32 Id)
{
    ((T_Notify*)GetDataPtr())->CPEID = Id;
}

CL3OamUnicastUTSWReqFail :: ~CL3OamUnicastUTSWReqFail()
{
}
