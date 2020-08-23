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

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3OAMCFGSNOOPDATASERVICEREQ
#include "L3OamCfgSnoopDataServiceReq.h" 
#endif


CCfgSnoopDataServiceReq :: CCfgSnoopDataServiceReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgSnoopDataServiceReq :: CCfgSnoopDataServiceReq()
{
}

bool CCfgSnoopDataServiceReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_CFG_SNOOP_DATA_SERVICE_CFG_REQ);
	return true;
}

UINT32 CCfgSnoopDataServiceReq :: GetDefaultDataLen() const
{
    return sizeof(T_DataServiceCfgReq);
}

UINT16 CCfgSnoopDataServiceReq :: GetTransactionId() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgSnoopDataServiceReq :: SetTransactionId(UINT16 TransId)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CCfgSnoopDataServiceReq :: GetRoutingAreaID() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.RoutingAreaID;
}

void CCfgSnoopDataServiceReq :: SetRoutingAreaID(UINT32 RoutingAreaID)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.RoutingAreaID = RoutingAreaID;
}

UINT8 CCfgSnoopDataServiceReq ::  GetTargetBtsID() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetBTSID;
}

void  CCfgSnoopDataServiceReq ::  SetTargetBtsID(UINT8 id)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetBTSID = id;
}

UINT8 CCfgSnoopDataServiceReq ::  GetTargetEID() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetEID;
}

void CCfgSnoopDataServiceReq ::  SetTargetEID(UINT8 id)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetEID = id;
}

UINT8 CCfgSnoopDataServiceReq ::  GetTargetPPPoEEID() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetPPPoEEID;
}

void CCfgSnoopDataServiceReq ::  SetTargetPPPoEEID(UINT8 id)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetPPPoEEID = id;
}

CCfgSnoopDataServiceReq :: ~CCfgSnoopDataServiceReq()
{

}



