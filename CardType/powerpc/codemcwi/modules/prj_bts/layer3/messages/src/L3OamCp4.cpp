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
#ifndef _INC_L3OAMCPEREQ
#include "L3OamCpeReq.h"
#endif

CCpeReq :: CCpeReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCpeReq :: CCpeReq()
{
}

bool CCpeReq :: CreateMessage(CComEntity& Entity)
{
    return CMessage :: CreateMessage(Entity);
}

UINT32 CCpeReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CCpeReq :: GetTransactionId() const
{
    return ((T_Req *)GetDataPtr())->TransId;
}

UINT16 CCpeReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CCpeReq :: GetCPEID() const
{
    return ((T_Req*)GetDataPtr())->CPEID;
}

void CCpeReq :: SetCPEID(UINT32 E)
{
    ((T_Req*)GetDataPtr())->CPEID = E;
}

CCpeReq :: ~CCpeReq()
{

}





