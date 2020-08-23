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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

// RF Enable Reqeust（EMS）L2
#ifndef _INC_L3OAMCFGRFREQ
#include "L3OamCfgRfReq.h"    
#endif
#include <string.h>
CCfgRfReq :: CCfgRfReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgRfReq :: CCfgRfReq()
{
}

bool CCfgRfReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_RF_CFG_REQ);
	return true;
}

UINT32 CCfgRfReq :: GetDefaultDataLen() const
{
    return sizeof(T_RfCfgReq);
}

UINT16 CCfgRfReq :: GetTransactionId() const
{
    return ((T_RfCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgRfReq :: SetTransactionId(UINT16 TransId)
{
    ((T_RfCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT8* CCfgRfReq :: GetEle() const
{
    return (SINT8*) (&(((T_RfCfgReq *)GetDataPtr())->Ele));
}

void CCfgRfReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_RfCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_RfCfgEle));
}

bool   CCfgRfReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_RfCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgRfReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_RfCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgRfReq :: ~CCfgRfReq()
{

}

CCfgRRangingReq :: CCfgRRangingReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgRRangingReq :: CCfgRRangingReq()
{
}

bool CCfgRRangingReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    //SetMessageId(MSGID_L3_L2_REMOTE_RANGE_CFG);
	return true;
}

UINT32 CCfgRRangingReq :: GetDefaultDataLen() const
{
    return sizeof(T_RRangingReq)-2;//去掉2个字节flag
}

UINT16 CCfgRRangingReq :: GetTransactionId() const
{
    return ((T_RRangingReq *)GetDataPtr())->TransId;
}

UINT16 CCfgRRangingReq :: SetTransactionId(UINT16 TransId)
{
    ((T_RRangingReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT8* CCfgRRangingReq :: GetEle() const
{
    return (SINT8*) (&(((T_RRangingReq *)GetDataPtr())->Ele));
}

void CCfgRRangingReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_RRangingReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_RangingPara)-2);
}

bool   CCfgRRangingReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_RRangingReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgRRangingReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_RRangingReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgRRangingReq :: ~CCfgRRangingReq()
{

}

CCfgClusterNumLmtReq :: CCfgClusterNumLmtReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgClusterNumLmtReq :: CCfgClusterNumLmtReq()
{
}

bool CCfgClusterNumLmtReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    //SetMessageId(MSGID_L3_L2_REMOTE_RANGE_CFG);
	return true;
}

UINT32 CCfgClusterNumLmtReq :: GetDefaultDataLen() const
{
    return sizeof(T_CLUSTERNUMLMTMSG)-2;//去掉2个字节flag
}

UINT16 CCfgClusterNumLmtReq :: GetTransactionId() const
{
    return ((T_CLUSTERNUMLMTMSG*)GetDataPtr())->TransId;
}

UINT16 CCfgClusterNumLmtReq :: SetTransactionId(UINT16 TransId)
{
    ((T_CLUSTERNUMLMTMSG*)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT8* CCfgClusterNumLmtReq :: GetEle() const
{
    return (SINT8*) (&(((T_CLUSTERNUMLMTMSG*)GetDataPtr())->Ele));
}

void CCfgClusterNumLmtReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_CLUSTERNUMLMTMSG *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_CLUSTERNUMLMTMSG)-2);
}

bool   CCfgClusterNumLmtReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_CLUSTERNUMLMTMSG *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgClusterNumLmtReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_CLUSTERNUMLMTMSG*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgClusterNumLmtReq :: ~CCfgClusterNumLmtReq()
{

}

