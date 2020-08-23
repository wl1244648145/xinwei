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
#ifndef _INC_L3OAMCFGCALGENDATAREQ
#include "L3OamCfgCalGenDataReq.h"   
#endif
#include <string.h>

CCfgCalGenDataReq :: CCfgCalGenDataReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgCalGenDataReq :: CCfgCalGenDataReq()
{
}

bool CCfgCalGenDataReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_CALIBRAT_CFG_GENDATA_REQ);
	return true;
}

UINT32 CCfgCalGenDataReq :: GetDefaultDataLen() const
{
    return sizeof(T_CalCfgGenReq);
}

UINT16 CCfgCalGenDataReq :: GetTransactionId() const
{
    return ((T_CalCfgGenReq *)GetDataPtr())->TransId;
}

UINT16 CCfgCalGenDataReq :: SetTransactionId(UINT16 TransId)
{
    ((T_CalCfgGenReq *)GetDataPtr())->TransId = TransId;
	return 0;
}



SINT8* CCfgCalGenDataReq :: GetEle() const
{
    return (SINT8*) (&(((T_CalCfgGenReq *)GetDataPtr())->Ele));
}

void   CCfgCalGenDataReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_CalCfgGenReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_CaliGenCfgEle));
}


bool   CCfgCalGenDataReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_CalCfgGenReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgCalGenDataReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_CalCfgGenReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgCalGenDataReq :: ~CCfgCalGenDataReq()
{

}

