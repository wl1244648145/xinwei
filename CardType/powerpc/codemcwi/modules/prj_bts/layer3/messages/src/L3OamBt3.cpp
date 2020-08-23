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

#ifndef _INC_L3OAMBTSREGREQ
#include "L3OamBtsRegReq.h"
#endif

#include <string.h>


CBtsRegReq :: CBtsRegReq(CMessage &rMsg)    
    :CMessage(rMsg)
{
}

CBtsRegReq :: CBtsRegReq()
{
}

bool CBtsRegReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_REG_REQ);
    return true;
}


UINT32 CBtsRegReq :: GetDefaultDataLen() const
{
    return sizeof(T_BtsRegReq);
}


UINT16 CBtsRegReq :: GetTransactionId() const
{
    return ((T_BtsRegReq *)GetDataPtr())->TransId;
}

UINT16 CBtsRegReq :: SetTransactionId(UINT16 TransId)
{
    ((T_BtsRegReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CBtsRegReq :: GetResult() const
{
    return ((T_BtsRegReq *)GetDataPtr())->Result;
}

void CBtsRegReq :: SetResult(UINT16 Result)
{
   ((T_BtsRegReq*)GetDataPtr())->Result = Result;
}

SINT8* CBtsRegReq :: GetSessionId() const
{
    return ((T_BtsRegReq*)GetDataPtr())->SessionID;
}

void CBtsRegReq :: SetSessionId(SINT8* ID)
{
    strcpy(((T_BtsRegReq*)GetDataPtr())->SessionID, ID);
}

UINT32 CBtsRegReq :: GetBtsPublicIp()
{
    return ((T_BtsRegReq *)GetDataPtr())-> BtsPublicIp;
}

UINT16 CBtsRegReq :: GetBtsPublicPort()
{
    return ((T_BtsRegReq *)GetDataPtr())-> BtsPublicPort;
}

CBtsRegReq :: ~CBtsRegReq()
{
}


