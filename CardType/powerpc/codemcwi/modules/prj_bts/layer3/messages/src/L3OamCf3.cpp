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


#ifndef _INC_L3OAMCFGARPDATASERVICEREQ
#include "L3OamCfgArpDataServiceReq.h" 
#endif


CCfgArpDataServiceReq :: CCfgArpDataServiceReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgArpDataServiceReq :: CCfgArpDataServiceReq()
{
}

bool CCfgArpDataServiceReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_CFG_ARP_DATA_SERVICE_CFG_REQ);
	return true;
}

UINT32 CCfgArpDataServiceReq :: GetDefaultDataLen() const
{
    return sizeof(T_DataServiceCfgReq);
}

UINT16 CCfgArpDataServiceReq :: GetTransactionId() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgArpDataServiceReq :: SetTransactionId(UINT16 TransId)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8 CCfgArpDataServiceReq :: GetP2PBridging() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.P2PBridging;
}

void CCfgArpDataServiceReq ::   SetP2PBridging(UINT8 P2PBridging)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.P2PBridging = P2PBridging;
}


UINT8 CCfgArpDataServiceReq :: GetEgrARPRroxy() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.EgrARPRroxy;
}

void CCfgArpDataServiceReq ::   SetEgrARPRroxy(UINT8 EgrARPRroxy)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.EgrARPRroxy = EgrARPRroxy;
}

CCfgArpDataServiceReq :: ~CCfgArpDataServiceReq()
{

}



