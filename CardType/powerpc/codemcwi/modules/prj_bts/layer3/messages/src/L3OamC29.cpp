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
#include "L3EMSMessageId.h"
#endif

#ifndef _INC_L3OAMCFGSFIDREQ
#include "L3OamCfgSFIDReq.h" 
#endif
#include <string.h>

CCfgSFIDReq :: CCfgSFIDReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgSFIDReq :: CCfgSFIDReq()
{
}

bool CCfgSFIDReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_CFG_SFID_REQ);
	return true;
}

UINT32 CCfgSFIDReq :: GetDefaultDataLen() const
{
    return sizeof(T_SFIDReq);
}

UINT16 CCfgSFIDReq :: GetTransactionId() const
{
    return ((T_SFIDReq *)GetDataPtr())->TransId;
}

UINT16 CCfgSFIDReq :: SetTransactionId(UINT16 TransId)
{
    ((T_SFIDReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

bool   CCfgSFIDReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (UINT8*)&(((T_SFIDReq*)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgSFIDReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy((UINT8*)&(((T_SFIDReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}

CCfgSFIDReq :: ~CCfgSFIDReq()
{

}



