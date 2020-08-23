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

// UT Service Descriptor Configuration Request
#ifndef _INC_L3OAMCFGCPESERVICEREQ
#include "L3OamCfgCpeServiceReq.h"  
#endif
#include <string.h>
CCfgCpeServiceReq :: CCfgCpeServiceReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgCpeServiceReq :: CCfgCpeServiceReq()
{
}

bool CCfgCpeServiceReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_UT_SERVICE_CFG_REQ);
	return true;
}

UINT32 CCfgCpeServiceReq :: GetDefaultDataLen() const
{
    return sizeof(T_UTSDCfgReq);
}

UINT16 CCfgCpeServiceReq :: GetTransactionId() const
{
    return ((T_UTSDCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgCpeServiceReq :: SetTransactionId(UINT16 TransId)
{
    ((T_UTSDCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}


SINT8* CCfgCpeServiceReq :: GetEle() const
{
    return (SINT8*) (&(((T_UTSDCfgReq *)GetDataPtr())->Ele));
}

void   CCfgCpeServiceReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_UTSDCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_UTSDCfgEle));
}

bool   CCfgCpeServiceReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_UTSDCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgCpeServiceReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_UTSDCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}


CCfgCpeServiceReq :: ~CCfgCpeServiceReq()
{

}

