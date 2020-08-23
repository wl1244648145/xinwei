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


#ifndef _INC_L3OAMSESSIONIDUPDATEREQ
#include "L3OamSessionIdUpdateReq.h"
#endif




//	SessionID Update Request(EMS)
CSessionIdUpdateReq :: CSessionIdUpdateReq(CMessage &rMsg)
    :CMessage(rMsg)
{

}

CSessionIdUpdateReq :: CSessionIdUpdateReq()
{
}

bool CSessionIdUpdateReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_SESSIONID_UPDATE_REQ);
    return true;
}

UINT32 CSessionIdUpdateReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CSessionIdUpdateReq :: GetTransactionId() const
{
    return ((T_Req*)GetDataPtr())->TransId;
}

UINT16 CSessionIdUpdateReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CSessionIdUpdateReq :: GetSessionID() const
{
    return ((T_Req *)GetDataPtr())->SessionID;
}

void CSessionIdUpdateReq :: SetSessionID(UINT32 ID)
{
    ((T_Req *)GetDataPtr())->SessionID = ID;
}

CSessionIdUpdateReq :: ~CSessionIdUpdateReq()
{
}


