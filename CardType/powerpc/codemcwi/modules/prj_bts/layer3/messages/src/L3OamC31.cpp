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

// Temperature Monitor Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGTEMPALARMREQ
#include "L3OamCfgTempAlarmReq.h"   
#endif
#include <string.h>

CCfgTempAlarmReq :: CCfgTempAlarmReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgTempAlarmReq :: CCfgTempAlarmReq()
{
}

bool CCfgTempAlarmReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_TEMP_MON_CFG_REQ);
	return true;
}

UINT32 CCfgTempAlarmReq :: GetDefaultDataLen() const
{
    return sizeof(T_TempAlarmCfgReq);
}

UINT16 CCfgTempAlarmReq :: GetTransactionId() const
{
    return ((T_TempAlarmCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgTempAlarmReq :: SetTransactionId(UINT16 TransId)
{
    ((T_TempAlarmCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgTempAlarmReq :: GetAlarmH() const
{
    return ((T_TempAlarmCfgReq *)GetDataPtr())->Ele.AlarmH;
}

void CCfgTempAlarmReq ::   SetAlarmH(UINT16 AlarmH)
{
    ((T_TempAlarmCfgReq *)GetDataPtr())->Ele.AlarmH = AlarmH;
}

UINT16 CCfgTempAlarmReq :: GetAlarmL() const
{
    return ((T_TempAlarmCfgReq *)GetDataPtr())->Ele.AlarmL;
}

void CCfgTempAlarmReq ::   SetAlarmL(UINT16 AlarmL)
{
    ((T_TempAlarmCfgReq *)GetDataPtr())->Ele.AlarmL = AlarmL;
}

UINT16 CCfgTempAlarmReq :: GetShutdownH() const
{
    return ((T_TempAlarmCfgReq *)GetDataPtr())->Ele.ShutdownH;
}

void  CCfgTempAlarmReq ::  SetShutdownH(UINT16 ShutdownH)
{
    ((T_TempAlarmCfgReq *)GetDataPtr())->Ele.ShutdownH = ShutdownH;
}

UINT16 CCfgTempAlarmReq :: GetShutdownL() const
{
    return ((T_TempAlarmCfgReq *)GetDataPtr())->Ele.ShutdownL;
}

void CCfgTempAlarmReq ::   SetShutdownL(UINT16 ShutdownL)
{
    ((T_TempAlarmCfgReq *)GetDataPtr())->Ele.ShutdownL = ShutdownL;
}

SINT8* CCfgTempAlarmReq :: GetEle() const
{
    return (SINT8*) (&(((T_TempAlarmCfgReq *)GetDataPtr())->Ele));
}

void CCfgTempAlarmReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_TempAlarmCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_TempAlarmCfgEle));
}

bool   CCfgTempAlarmReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_TempAlarmCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgTempAlarmReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_TempAlarmCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgTempAlarmReq :: ~CCfgTempAlarmReq()
{

}

