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
#ifndef _INC_L3OAMCPECOMMONREQ
#include "L3OamCpeCommonReq.h"
#endif

CCpeCommonReq :: CCpeCommonReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCpeCommonReq :: CCpeCommonReq()
{
}

bool CCpeCommonReq :: CreateMessage(CComEntity& Entity)
{
    return CMessage :: CreateMessage(Entity);
}

UINT32 CCpeCommonReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CCpeCommonReq :: GetTransactionId() const
{
    return ((T_Req *)GetDataPtr())->TransId;
}

UINT16 CCpeCommonReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CCpeCommonReq :: GetCPEID() const
{
    return ((T_Req*)GetDataPtr())->CPEID;
}

void CCpeCommonReq :: SetCPEID(UINT32 E)
{
    ((T_Req*)GetDataPtr())->CPEID = E;
}

CCpeCommonReq :: ~CCpeCommonReq()
{

}





