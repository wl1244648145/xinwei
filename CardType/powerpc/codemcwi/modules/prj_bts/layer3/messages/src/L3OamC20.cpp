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

#ifndef _INC_L3OAMCFGEBDATASERVICEREQ
#include "L3OamCfgEbDataServiceReq.h" 
#endif


CCfgEbDataServiceReq :: CCfgEbDataServiceReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgEbDataServiceReq :: CCfgEbDataServiceReq()
{
}

bool CCfgEbDataServiceReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_CFG_EB_DATA_SERVICE_CFG_REQ);
	return true;
}

UINT32 CCfgEbDataServiceReq :: GetDefaultDataLen() const
{
    return sizeof(T_DataServiceCfgReq);
}

UINT16 CCfgEbDataServiceReq :: GetTransactionId() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgEbDataServiceReq :: SetTransactionId(UINT16 TransId)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8 CCfgEbDataServiceReq :: GetEgrBCFilter() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.EgrBCFilter;
}

void CCfgEbDataServiceReq ::   SetEgrBCFilter(UINT8 EgrBCFilter)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.EgrBCFilter = EgrBCFilter;
}

UINT16 CCfgEbDataServiceReq :: GetPPPSessionLen() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.PPPSessionLen;
}

void CCfgEbDataServiceReq ::   SetPPPSessionLen(UINT16 PPPSessionLen)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.PPPSessionLen = PPPSessionLen;
}

UINT16 CCfgEbDataServiceReq :: GetLBATimerLen() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.LBATimerLen;
}

void CCfgEbDataServiceReq ::   SetLBATimerLen(UINT16 LBATimerLen)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.LBATimerLen = LBATimerLen;
}

UINT8 CCfgEbDataServiceReq :: GetAccessControl() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.AccessControl;
}

void  CCfgEbDataServiceReq ::  SetAccessControl(UINT8 AccessControl)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.AccessControl = AccessControl;
}

CCfgEbDataServiceReq :: ~CCfgEbDataServiceReq()
{

}



