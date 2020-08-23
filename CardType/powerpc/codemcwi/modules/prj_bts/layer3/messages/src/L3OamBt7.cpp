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
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMBTSSWUPDATEREQ
#include "L3OamBtsSWUpdateReq.h"
#endif
CBtsSWUpdateReq :: CBtsSWUpdateReq(CMessage &rMsg)
    :CMessage(rMsg)
{  }

CBtsSWUpdateReq :: CBtsSWUpdateReq()
{
}
bool CBtsSWUpdateReq :: CreateMessage(CComEntity&)
{
	return true;
}

UINT32 CBtsSWUpdateReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CBtsSWUpdateReq :: GetTransactionId() const
{
    return ((T_Req *)GetDataPtr())->TransId;
}

UINT16 CBtsSWUpdateReq :: SetTransactionId(UINT16 Id)
{
    ((T_Req *)GetDataPtr())->TransId = Id;
	return 0;
}

CBtsSWUpdateReq :: ~CBtsSWUpdateReq()
{
}

