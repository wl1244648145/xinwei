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

#ifndef _INC_L3OAMBTSREGRSP
#include "L3OamBtsRegRsp.h"
#endif


CL3OamBtsRegRsp :: CL3OamBtsRegRsp()
{
}

CL3OamBtsRegRsp :: CL3OamBtsRegRsp(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3OamBtsRegRsp :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_BTS_EMS_REG_RSP);
    return true;
}
UINT32 CL3OamBtsRegRsp :: GetDefaultDataLen() const
{
    return sizeof(T_BtsRegRsp);
}


UINT16 CL3OamBtsRegRsp :: GetTransactionId() const
{
    return ((T_BtsRegRsp *)GetDataPtr())->TransId;
}

UINT16 CL3OamBtsRegRsp :: SetTransactionId(UINT16 TransId)
{
    ((T_BtsRegRsp *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CL3OamBtsRegRsp :: GetResult() const
{
    return ((T_BtsRegRsp *)GetDataPtr())->Result;
}

void CL3OamBtsRegRsp :: SetResult(UINT16 R)
{
    ((T_BtsRegRsp *)GetDataPtr())->Result = R;
}

UINT16 CL3OamBtsRegRsp :: GetBtsRunState() const
{
    return ((T_BtsRegRsp *)GetDataPtr())->BtsRunState;
}

void CL3OamBtsRegRsp :: SetBtsRunState(UINT16 R)
{
    ((T_BtsRegRsp *)GetDataPtr())->BtsRunState = R;
}


CL3OamBtsRegRsp :: ~CL3OamBtsRegRsp()
{

}

