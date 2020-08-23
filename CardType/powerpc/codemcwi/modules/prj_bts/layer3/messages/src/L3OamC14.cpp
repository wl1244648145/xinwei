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

//5.4.4.1	Calibration Result Notification£¨BTS£©
#ifndef _INC_L3OAMCFGCALRSTNOTIFY
#include "L3OamCfgCalRstNotify.h"   
#endif
#include <string.h>

CCfgCalRstNotify :: CCfgCalRstNotify(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgCalRstNotify :: CCfgCalRstNotify()
{
}

bool CCfgCalRstNotify :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_BTS_EMS_CALIBRAT_CFG_DATA_RSP);
	return true;
}

UINT32 CCfgCalRstNotify :: GetDefaultDataLen() const
{
    return sizeof(T_CalRstNotify);
}

UINT16 CCfgCalRstNotify :: GetTransactionId() const
{
    return ((T_CalRstNotify *)GetDataPtr())->TransId;
}

UINT16 CCfgCalRstNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_CalRstNotify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgCalRstNotify :: GetType() const  // 1:TXCAL_I  2:TXCAL_Q  3:RXCAL_I  4:RXCAL_Q
{
    return ((T_CalRstNotify *)GetDataPtr())->Ele.Type;
}

UINT16 CCfgCalRstNotify :: GetAntennalIndex() const  //	0~7
{
    return ((T_CalRstNotify *)GetDataPtr())->Ele.AntennalIndex;
}

SINT8* CCfgCalRstNotify :: GetEle() const
{
    return (SINT8*) (&(((T_CalRstNotify *)GetDataPtr())->Ele));
}

void   CCfgCalRstNotify :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_CalRstNotify *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_CaliResultNotifyEle));
}

bool   CCfgCalRstNotify :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_CalRstNotify *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgCalRstNotify :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_CalRstNotify*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgCalRstNotify :: ~CCfgCalRstNotify()
{

}

