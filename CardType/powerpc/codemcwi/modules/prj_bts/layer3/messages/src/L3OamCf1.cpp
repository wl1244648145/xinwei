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

// 5.3.3	Airlink Miscellaneous Setting Request£¨EMS£©
#ifndef _INC_L3OAMCFGAIRLINKMISCREQ
#include "L3OamCfgAirLinkMisc.h"    
#endif
#include <string.h>

CCfgAirLinkMiscReq :: CCfgAirLinkMiscReq(CMessage &rMsg)
:CMessage(rMsg)
{  }
CCfgAirLinkMiscReq :: CCfgAirLinkMiscReq()
{
}
bool CCfgAirLinkMiscReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_AIR_LINK_MISC_CFG_REQ);
	return true;
}

UINT32 CCfgAirLinkMiscReq :: GetDefaultDataLen() const
{
    return sizeof(T_AirLinkMisCfgReq);
}

UINT16 CCfgAirLinkMiscReq :: GetTransactionId() const
{
    return ((T_AirLinkMisCfgReq*)GetDataPtr())->TransId;
}

UINT16 CCfgAirLinkMiscReq :: SetTransactionId(UINT16 TransId)
{
    ((T_AirLinkMisCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT8 CCfgAirLinkMiscReq :: GetSSTSync() const
{
    return ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.SSTSync;
}

void CCfgAirLinkMiscReq :: SetSSTSync(SINT8 Sync)
{
    ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.SSTSync = Sync;
}

UINT8 CCfgAirLinkMiscReq :: GetSNRTLeave() const
{
    return ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.SNRTLeave;
}

void CCfgAirLinkMiscReq :: SetSNRTLeave(UINT8 Leave)
{
    ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.SNRTLeave = Leave;
}

UINT8 CCfgAirLinkMiscReq :: GetSNRTEnter() const
{
    return ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.SNRTEnter;
}

void CCfgAirLinkMiscReq :: SetSNRTEnter(UINT8 Enter)
{
    ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.SNRTEnter = Enter;
}

UINT8 CCfgAirLinkMiscReq :: GetBTSLTLeave() const
{
    return ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.BTSLTLeave;
}

void CCfgAirLinkMiscReq :: SetBTSLTLeave(UINT8 Leaver)
{
    ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.BTSLTLeave = Leaver;
}

UINT8 CCfgAirLinkMiscReq :: GetBTSLTLEnter() const
{
    return ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.BTSLTLEnter;
}

void CCfgAirLinkMiscReq :: SetBTSLTLEnter(UINT8 Enter)
{
    ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.BTSLTLEnter = Enter;
}

UINT8 CCfgAirLinkMiscReq :: GetWakeupInterval() const
{
    return ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.WakeupInterval;
}

void CCfgAirLinkMiscReq :: SetWakeupInterval(UINT8 Interval)
{
    ((T_AirLinkMisCfgReq *)GetDataPtr())->Ele.WakeupInterval = Interval;
}

SINT8* CCfgAirLinkMiscReq :: GetEle() const
{
    return (SINT8*) (&(((T_AirLinkMisCfgReq *)GetDataPtr())->Ele));
}

void  CCfgAirLinkMiscReq :: SetEle(SINT8 * E)
{
    memcpy((&(((T_AirLinkMisCfgReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_AirLinkMisCfgEle));
}

bool   CCfgAirLinkMiscReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_AirLinkMisCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgAirLinkMiscReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_AirLinkMisCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}

CCfgAirLinkMiscReq :: ~CCfgAirLinkMiscReq()
{

}
