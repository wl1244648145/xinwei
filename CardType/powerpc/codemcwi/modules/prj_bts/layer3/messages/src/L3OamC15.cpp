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

//	Calibration Configuration Data Request£¨EMS£©
#ifndef _INC_L3OAMCFGCFGCALREQ
#include "L3OamCfgCfgCalReq.h"      
#endif
#include <string.h>

CCfgCalReq :: CCfgCalReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgCalReq :: CCfgCalReq()
{
}

bool CCfgCalReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_CALIBRAT_CFG_REQ);
	return true;
}

UINT32 CCfgCalReq :: GetDefaultDataLen() const
{
    return sizeof( T_CalCfgReq);
}

UINT16 CCfgCalReq :: GetTransactionId() const
{
    return (( T_CalCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgCalReq :: SetTransactionId(UINT16 TransId)
{
    (( T_CalCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgCalReq :: GetCalIner() const
{
    return (( T_CalCfgReq *)GetDataPtr())->Ele.CalIner;
}

void CCfgCalReq :: SetCalIner(UINT16 CalIner)
{
    (( T_CalCfgReq *)GetDataPtr())->Ele.CalIner = CalIner;
}

UINT16 CCfgCalReq :: GetCalType() const
{
    return (( T_CalCfgReq *)GetDataPtr())->Ele.CalType;
}

void CCfgCalReq :: SetCalType(UINT16 CalType)
{
    (( T_CalCfgReq *)GetDataPtr())->Ele.CalType = CalType;
}

SINT8* CCfgCalReq :: GetEle() const
{
    return (SINT8*) (&(((T_CalCfgReq *)GetDataPtr())->Ele));
}

void   CCfgCalReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_CalCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_CalCfgEle));
}

bool   CCfgCalReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
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

bool   CCfgCalReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
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


CCfgCalReq :: ~CCfgCalReq()
{

}


