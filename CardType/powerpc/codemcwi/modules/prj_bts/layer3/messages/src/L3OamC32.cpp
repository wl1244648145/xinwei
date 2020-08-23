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

// ToS Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGTOSREQ
#include "L3OamCfgTosReq.h"         
#endif
#include <string.h>

CCfgTosReq :: CCfgTosReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgTosReq :: CCfgTosReq()
{
}

bool CCfgTosReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_QOS_CFG_REQ);
	return true;
}

UINT32 CCfgTosReq :: GetDefaultDataLen() const
{
    return sizeof(T_ToSCfgReq);
}

UINT16 CCfgTosReq :: GetTransactionId() const
{
    return ((T_ToSCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgTosReq :: SetTransactionId(UINT16 TransId)
{
    ((T_ToSCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

bool   CCfgTosReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy((((T_ToSCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}


CCfgTosReq :: ~CCfgTosReq()
{

}

