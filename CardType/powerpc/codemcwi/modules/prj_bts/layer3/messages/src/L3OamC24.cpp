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
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

// L1 Configuration
// L1 General Setting Request£¨EMS£©
#ifndef _INC_L3OAMCFGL1GENDATAREQ
#include "L3OamCfgL1GenDataReq.h"   
#endif
#include <string.h>
CCfgL1GenDataReq :: CCfgL1GenDataReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgL1GenDataReq :: CCfgL1GenDataReq()
{
}

bool CCfgL1GenDataReq :: CreateMessage(CComEntity &Entity)
{
    if (false == CMessage::CreateMessage(Entity))
        {
        return false;
        }
    SetMessageId(M_EMS_BTS_L1_GENERAL_SETTING_REQ);
	return true;
}

UINT32 CCfgL1GenDataReq :: GetDefaultDataLen() const
{
    return sizeof(T_L1GenCfgReq);
}

UINT16 CCfgL1GenDataReq :: GetTransactionId() const
{
    return ((T_L1GenCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgL1GenDataReq :: SetTransactionId(UINT16 TransId)
{
    ((T_L1GenCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgL1GenDataReq :: GetGPSOffset() const
{
    return ((T_L1GenCfgReq *)GetDataPtr())->Ele.GpsOffset;
}

void CCfgL1GenDataReq ::   SetGPSOffset(UINT16 G)
{
    ((T_L1GenCfgReq *)GetDataPtr())->Ele.GpsOffset = G;
}

UINT16 CCfgL1GenDataReq :: GetSyncSrc() const
{
    return ((T_L1GenCfgReq *)GetDataPtr())->Ele.SyncSrc;
}

void CCfgL1GenDataReq ::   SetSyncSrc(UINT16 SyncSrc)
{
    ((T_L1GenCfgReq *)GetDataPtr())->Ele.SyncSrc = SyncSrc;
}

UINT16 CCfgL1GenDataReq :: GetAntennaMask() const
{
    return ((T_L1GenCfgReq *)GetDataPtr())->Ele.AntennaMask;
}

void CCfgL1GenDataReq ::   SetAntennaMask(UINT16 A)
{
    ((T_L1GenCfgReq *)GetDataPtr())->Ele.AntennaMask = A;
}

SINT8* CCfgL1GenDataReq :: GetEle() const
{
    return (SINT8*) (&(((T_L1GenCfgReq *)GetDataPtr())->Ele));
}

bool   CCfgL1GenDataReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_L1GenCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgL1GenDataReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_L1GenCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



void   CCfgL1GenDataReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) ((T_L1GenCfgEle *)&(((T_L1GenCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_L1GenCfgEle));
}
CCfgL1GenDataReq :: ~CCfgL1GenDataReq()
{

}


