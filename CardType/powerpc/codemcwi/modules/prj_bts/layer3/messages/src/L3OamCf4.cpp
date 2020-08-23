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

//Billing Data Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGBILLINGDATAREQ
#include "L3OamCfgBillingDataReq.h" 
#endif
#include <string.h>

CCfgBillingDataReq :: CCfgBillingDataReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgBillingDataReq :: CCfgBillingDataReq()
{
}

bool CCfgBillingDataReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_BILLING_DATA_CFG_REQ);
	return true;
}

UINT32 CCfgBillingDataReq :: GetDefaultDataLen() const
{
    return sizeof(T_BillDataCfgReq);
}

UINT16 CCfgBillingDataReq :: GetTransactionId() const
{
    return ((T_BillDataCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgBillingDataReq :: SetTransactionId(UINT16 TransId)
{
    ((T_BillDataCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgBillingDataReq :: GetUploadInter() const
{
    return ((T_BillDataCfgReq *)GetDataPtr())->Ele.UploadInter;
}

void CCfgBillingDataReq :: SetUploadInter(UINT16 Inter)
{
    ((T_BillDataCfgReq *)GetDataPtr())->Ele.UploadInter = Inter;
}
SINT8* CCfgBillingDataReq :: GetEle() const
{
    return (SINT8*) (&(((T_BillDataCfgReq *)GetDataPtr())->Ele));
}

void   CCfgBillingDataReq :: SetEle(SINT8 * E)
{
    memcpy(&(((T_BillDataCfgReq *)GetDataPtr())->Ele), 
            E, 
            sizeof(T_BillDataCfgEle));
}


bool   CCfgBillingDataReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_BillDataCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgBillingDataReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_BillDataCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}

void CCfgBillingDataReq::setUserPassword(SINT8 *pUser, SINT8 *pPwd)
{
    if ((NULL == pUser) || (NULL == pPwd) || (40 <= strlen((const char*)pUser)) || (40 <= strlen((const char*)pPwd)))
        return;
    memset(((T_BillDataCfgReq*)GetDataPtr())->arrUserName, 0, sizeof(((T_BillDataCfgReq*)GetDataPtr())->arrUserName));
    memset(((T_BillDataCfgReq*)GetDataPtr())->arrPassword, 0, sizeof(((T_BillDataCfgReq*)GetDataPtr())->arrPassword));

    memcpy(((T_BillDataCfgReq*)GetDataPtr())->arrUserName, pUser, strlen((const char*)pUser));
    memcpy(((T_BillDataCfgReq*)GetDataPtr())->arrPassword, pPwd,  strlen((const char*)pPwd));
}


CCfgBillingDataReq :: ~CCfgBillingDataReq()
{

}
