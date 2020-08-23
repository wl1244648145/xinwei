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
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

// BTS Reset Request£¨EMS£©
#ifndef _INC_L3OAMCFGBTSRSTREQ
#include "L3OamCfgBtsRstReq.h"      
#endif


CCfgBtsRstReq :: CCfgBtsRstReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgBtsRstReq :: CCfgBtsRstReq()
{
}

bool CCfgBtsRstReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_BTS_RESET_REQ);
	return true;
}

UINT32 CCfgBtsRstReq :: GetDefaultDataLen() const
{
    return sizeof(T_BtsRstReq);
}

UINT16 CCfgBtsRstReq :: GetTransactionId() const
{
    return ((T_BtsRstReq *)GetDataPtr())->TransId;
}

UINT16 CCfgBtsRstReq :: SetTransactionId(UINT16 TransId)
{
    ((T_BtsRstReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgBtsRstReq :: GetDataSource() const
{
    return ((T_BtsRstReq *)GetDataPtr())->Ele.DataSource;
}

void CCfgBtsRstReq :: SetDataSource(UINT16 DataSource)
{
    ((T_BtsRstReq *)GetDataPtr())->Ele.DataSource = DataSource;
}
CCfgBtsRstReq :: ~CCfgBtsRstReq()
{

}

