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
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMCFGDATASERVICEREQ
#include "L3OamCfgDataServiceReq.h" 
#endif
#include <string.h>

CCfgDataServiceReq :: CCfgDataServiceReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgDataServiceReq :: CCfgDataServiceReq()
{
}

bool CCfgDataServiceReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_DATA_SERVICE_CFG_REQ);
	return true;
}

UINT32 CCfgDataServiceReq :: GetDefaultDataLen() const
{
    return sizeof(T_DataServiceCfgReq);
}

UINT16 CCfgDataServiceReq :: GetTransactionId() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgDataServiceReq :: SetTransactionId(UINT16 TransId)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CCfgDataServiceReq :: GetRoutingAreaID() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.RoutingAreaID;
}

void CCfgDataServiceReq :: SetRoutingAreaID(UINT32 RoutingAreaID)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.RoutingAreaID = RoutingAreaID;
}

UINT8 CCfgDataServiceReq :: GetMobility() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.Mobility;
}

void   CCfgDataServiceReq :: SetMobility(UINT8 Mobility)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.Mobility = Mobility;
}


UINT8 CCfgDataServiceReq :: GetAccessControl() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.AccessControl;
}

void  CCfgDataServiceReq ::  SetAccessControl(UINT8 AccessControl)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.AccessControl = AccessControl;
}


UINT8 CCfgDataServiceReq :: GetP2PBridging() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.P2PBridging;
}

void CCfgDataServiceReq ::   SetP2PBridging(UINT8 P2PBridging)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.P2PBridging = P2PBridging;
}


UINT8 CCfgDataServiceReq :: GetEgrARPRroxy() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.EgrARPRroxy;
}

void CCfgDataServiceReq ::   SetEgrARPRroxy(UINT8 EgrARPRroxy)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.EgrARPRroxy = EgrARPRroxy;
}


UINT8 CCfgDataServiceReq :: GetEgrBCFilter() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.EgrBCFilter;
}

void CCfgDataServiceReq ::   SetEgrBCFilter(UINT8 EgrBCFilter)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.EgrBCFilter = EgrBCFilter;
}


UINT16 CCfgDataServiceReq :: GetLBATimerLen() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.LBATimerLen;
}

void CCfgDataServiceReq ::   SetLBATimerLen(UINT16 LBATimerLen)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.LBATimerLen = LBATimerLen;
}


UINT16 CCfgDataServiceReq :: GetPPPSessionLen() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.PPPSessionLen;
}

void CCfgDataServiceReq ::   SetPPPSessionLen(UINT16 PPPSessionLen)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.PPPSessionLen = PPPSessionLen;
}


SINT8* CCfgDataServiceReq :: GetEle() const
{
    return (SINT8*) (&(((T_DataServiceCfgReq *)GetDataPtr())->Ele));
}

void   CCfgDataServiceReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_DataServiceCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_DataServiceCfgEle));
}

bool   CCfgDataServiceReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_DataServiceCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgDataServiceReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_DataServiceCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}

UINT8 CCfgDataServiceReq ::  GetTargetBtsID() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetBTSID;
}

void  CCfgDataServiceReq ::  SetTargetBtsID(UINT8 id)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetBTSID = id;
}

UINT8 CCfgDataServiceReq ::  GetTargetEID() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetEID;
}

void CCfgDataServiceReq ::  SetTargetEID(UINT8 id)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetEID = id;
}

UINT8 CCfgDataServiceReq ::  GetTargetPPPoEEID() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetPPPoEEID;
}

void CCfgDataServiceReq ::  SetTargetPPPoEEID(UINT8 id)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.TargetPPPoEEID = id;
}

CCfgDataServiceReq :: ~CCfgDataServiceReq()
{

}



