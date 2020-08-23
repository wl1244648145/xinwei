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

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3L3L2CFGAIRLINKREQ
#include "L3L3L2CfgAirLinkReq.h"     
#endif

#include <string.h>
CL3L2CfgAirLinkReq :: CL3L2CfgAirLinkReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CL3L2CfgAirLinkReq :: CL3L2CfgAirLinkReq()
{
}

bool CL3L2CfgAirLinkReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L3_L2_AIRLINK_DATA_CFG_REQ);
	return true;
}

UINT32 CL3L2CfgAirLinkReq :: GetDefaultDataLen() const
{
    return sizeof(T_AirLinkCfgReq);
}

UINT16 CL3L2CfgAirLinkReq :: GetTransactionId()const
{
    return ((T_AirLinkCfgReq *)GetDataPtr())->TransId;
}

UINT16 CL3L2CfgAirLinkReq :: SetTransactionId(UINT16 TransId)
{
    ((T_AirLinkCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CL3L2CfgAirLinkReq :: GeBtsID()const
{
    return ((T_AirLinkCfgReq *)GetDataPtr())->BtsID;
}

void   CL3L2CfgAirLinkReq :: SetBtsID(UINT32 id)
{
    ((T_AirLinkCfgReq *)GetDataPtr())->BtsID = id;
}

UINT16 CL3L2CfgAirLinkReq :: GetNetworkID()const
{
    return ((T_AirLinkCfgReq *)GetDataPtr())->NetworkID;
}

void   CL3L2CfgAirLinkReq :: SetNetworkID(UINT16 id)
{
    ((T_AirLinkCfgReq *)GetDataPtr())->NetworkID = 0;
}

UINT16 CL3L2CfgAirLinkReq :: GetResetCnt()const
{
    return ((T_AirLinkCfgReq *)GetDataPtr())->ResetCnt;
}

void   CL3L2CfgAirLinkReq :: SetResetCnt(UINT16 cnt)
{
    ((T_AirLinkCfgReq *)GetDataPtr())->ResetCnt = cnt;
}

SINT8* CL3L2CfgAirLinkReq :: GetEle()const
{
    return (SINT8*) (&(((T_AirLinkCfgReq *)GetDataPtr())->Ele));
}

void   CL3L2CfgAirLinkReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_AirLinkCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_AirLinkCfgEle));
}

CL3L2CfgAirLinkReq :: ~CL3L2CfgAirLinkReq()
{

}
