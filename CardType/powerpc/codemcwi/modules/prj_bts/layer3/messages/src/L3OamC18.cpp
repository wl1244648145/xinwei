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
#include "L3EMSMessageId.h"
#endif

#ifndef _INC_L3OAMCFGDELACLREQ
#include "L3OamCfgDelACLReq.h"
#endif

CDeleteACLReq :: CDeleteACLReq(CMessage &rMsg)
    :CMessage(rMsg)
{  }

CDeleteACLReq :: CDeleteACLReq()
{
}
bool CDeleteACLReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_DEL_ACL_REQ);
	return true;
}

UINT32 CDeleteACLReq :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CDeleteACLReq :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CDeleteACLReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8 CDeleteACLReq :: GetIndex()const
{
    return ((T_Notify *)GetDataPtr())->Index;
}

void CDeleteACLReq :: SetIndex(UINT8 Index)
{
    ((T_Notify *)GetDataPtr())->Index = Index;
}

CDeleteACLReq :: ~CDeleteACLReq()
{
}

