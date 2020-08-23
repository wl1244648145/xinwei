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

#ifndef _INC_L3OAMCFGDMDATASERVICEREQ
#include "L3OamCfgDmDataServiceReq.h" 
#endif


CCfgDmDataServiceReq :: CCfgDmDataServiceReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgDmDataServiceReq :: CCfgDmDataServiceReq()
{
}

bool CCfgDmDataServiceReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_CFG_DM_DATA_SERVICE_CFG_REQ);
	return true;
}

UINT32 CCfgDmDataServiceReq :: GetDefaultDataLen() const
{
    return sizeof(T_DataServiceCfgReq);
}

UINT16 CCfgDmDataServiceReq :: GetTransactionId() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgDmDataServiceReq :: SetTransactionId(UINT16 TransId)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8 CCfgDmDataServiceReq :: GetMobility() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.Mobility;
}

void   CCfgDmDataServiceReq :: SetMobility(UINT8 Mobility)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.Mobility = Mobility;
}


UINT8 CCfgDmDataServiceReq :: GetAccessControl() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.AccessControl;
}

void  CCfgDmDataServiceReq ::  SetAccessControl(UINT8 AccessControl)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.AccessControl = AccessControl;
}

UINT16 CCfgDmDataServiceReq :: GetLBATimerLen() const
{
    return ((T_DataServiceCfgReq *)GetDataPtr())->Ele.LBATimerLen;
}

void CCfgDmDataServiceReq ::   SetLBATimerLen(UINT16 LBATimerLen)
{
    ((T_DataServiceCfgReq *)GetDataPtr())->Ele.LBATimerLen = LBATimerLen;
}

CCfgDmDataServiceReq :: ~CCfgDmDataServiceReq()
{

}



