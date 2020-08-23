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

//5.4.3	Calibration Action Request£¨EMS£©
#ifndef _INC_L3OAMCFGCALACTIONREQ
#include "L3OamCfgCalActionReq.h"   
#endif

CCfgCalActionReq :: CCfgCalActionReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgCalActionReq :: CCfgCalActionReq()
{
}

bool CCfgCalActionReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_INSTANT_CALIBRATION_REQ);

	return true;
}

UINT32 CCfgCalActionReq :: GetDefaultDataLen() const
{
    return sizeof(T_InstantCalReq);
}

UINT16 CCfgCalActionReq :: GetTransactionId() const
{
    return ((T_InstantCalReq *)GetDataPtr())->TransId;
}

UINT16 CCfgCalActionReq :: SetTransactionId(UINT16 TransId)
{
    ((T_InstantCalReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgCalActionReq :: GetCalType() const
{
    return ((T_InstantCalReq *)GetDataPtr())->Ele.CalType;
}

void CCfgCalActionReq :: SetCalType(UINT16 CalType)
{
    ((T_InstantCalReq *)GetDataPtr())->Ele.CalType = CalType;
}

void CCfgCalActionReq::SetCalTrigger(UINT16 CalTrigger)//  0 - manual   1 -- period
{
    ((T_InstantCalReq *)GetDataPtr())->Ele.CalTrigger = CalTrigger;
}

CCfgCalActionReq :: ~CCfgCalActionReq()
{

}

