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
#ifndef _INC_L3OAMCFGCALDATAREQ
#include "L3OamCfgCalDataReq.h"   
#endif
#include <string.h>
CCfgCalDataReq :: CCfgCalDataReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgCalDataReq :: CCfgCalDataReq()
{
}

bool CCfgCalDataReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_CALIBRAT_CFG_REQ);
	return true;
}

UINT32 CCfgCalDataReq :: GetDefaultDataLen() const
{
    return sizeof(T_CalCfgReq);
}

UINT16 CCfgCalDataReq :: GetTransactionId() const
{
    return ((T_CalCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgCalDataReq :: SetTransactionId(UINT16 TransId)
{
    ((T_CalCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT8* CCfgCalDataReq :: GetEle() const
{
    return (SINT8*) (&(((T_CalCfgReq *)GetDataPtr())->Ele));
}

void   CCfgCalDataReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_CalCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_CaliDataEle));
}

bool   CCfgCalDataReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_CalCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgCalDataReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_CalCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgCalDataReq :: ~CCfgCalDataReq()
{

}

