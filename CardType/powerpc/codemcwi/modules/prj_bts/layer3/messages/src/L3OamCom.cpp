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

#ifndef _INC_L3OAMCOMMONREQ
#include "L3OamCommonReq.h"
#endif


CL3OamCommonReq :: CL3OamCommonReq()
{
}

CL3OamCommonReq :: CL3OamCommonReq(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3OamCommonReq :: CreateMessage(CComEntity& Entity)
{
    return CMessage::CreateMessage(Entity);
}

UINT32 CL3OamCommonReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CL3OamCommonReq :: GetTransactionId() const
{
    return ((T_Req*)GetDataPtr())->TransId;
}

UINT16 CL3OamCommonReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
    return 0;
}

CL3OamCommonReq :: ~CL3OamCommonReq()
{

}


