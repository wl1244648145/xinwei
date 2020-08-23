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

// L2 Configuration
// 5.3.1	Airlink Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGAIRLINKREQ
#include "L3OamCfgAirLinkReq.h"     
#endif

#include <string.h>
CCfgAirLinkReq :: CCfgAirLinkReq(CMessage &rMsg)
:CMessage(rMsg)
{  }
CCfgAirLinkReq :: CCfgAirLinkReq()
{
}
bool CCfgAirLinkReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_CALIBRAT_CFG_DATA_REQ);
	return true;
}

UINT32 CCfgAirLinkReq :: GetDefaultDataLen() const
{
    return sizeof(T_AirLinkCfgReq);
}

UINT16 CCfgAirLinkReq :: GetTransactionId() const
{
    return ((T_AirLinkCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgAirLinkReq :: SetTransactionId(UINT16 TransId)
{
    ((T_AirLinkCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT8* CCfgAirLinkReq :: GetEle() const
{
    return (SINT8*) (&(((T_AirLinkCfgReq *)GetDataPtr())->Ele));
}

void   CCfgAirLinkReq :: SetEle(SINT8 * E)
{
    memcpy((&(((T_AirLinkCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_AirLinkCfgEle));
}

bool   CCfgAirLinkReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_AirLinkCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgAirLinkReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_AirLinkCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}


CCfgAirLinkReq :: ~CCfgAirLinkReq()
{

}
