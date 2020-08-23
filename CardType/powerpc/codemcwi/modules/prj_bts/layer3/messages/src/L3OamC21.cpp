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

// GPS Data Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGGPSDATAREQ
#include "L3OamCfgGpsDataReq.h"     
#endif

#include "L3L2MessageID.h"
CCfgGpsDataReq :: CCfgGpsDataReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgGpsDataReq :: CCfgGpsDataReq()
{
}

bool CCfgGpsDataReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_GPS_DATA_CFG_REQ);
	return true;
}

UINT32 CCfgGpsDataReq :: GetDefaultDataLen() const
{
    return sizeof(T_GpsDataCfgReq);
}

UINT16 CCfgGpsDataReq :: GetTransactionId() const
{
    return ((T_GpsDataCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgGpsDataReq :: SetTransactionId(UINT16 TransId)
{
    ((T_GpsDataCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT32 CCfgGpsDataReq :: GetLatitude() const
{
    return ((T_GpsDataCfgReq *)GetDataPtr())->Ele.Latitude;
}

void CCfgGpsDataReq ::   SetLatitude(SINT32 Latitude)
{
    ((T_GpsDataCfgReq *)GetDataPtr())->Ele.Latitude = Latitude;
}

SINT32 CCfgGpsDataReq :: GetLongitude() const
{
    return ((T_GpsDataCfgReq *)GetDataPtr())->Ele.Longitude;
}

void CCfgGpsDataReq ::   SetLongitude(SINT32 Longitude)
{
    ((T_GpsDataCfgReq *)GetDataPtr())->Ele.Longitude = Longitude;
}

SINT32 CCfgGpsDataReq :: GetHeight() const
{
    return ((T_GpsDataCfgReq *)GetDataPtr())->Ele.Height;
}

void CCfgGpsDataReq ::   SetHeight(SINT32 Height)
{
    ((T_GpsDataCfgReq *)GetDataPtr())->Ele.Height = Height;
}

SINT32 CCfgGpsDataReq :: GetGMTOffset() const
{
    return ((T_GpsDataCfgReq *)GetDataPtr())->Ele.GMTOffset;
}

void CCfgGpsDataReq ::   SetGMTOffset(SINT32 GMTOffset)
{
    ((T_GpsDataCfgReq *)GetDataPtr())->Ele.GMTOffset = GMTOffset;
}

UINT8 CCfgGpsDataReq :: GetSatelliteCnt() const
{
    return ((T_GpsDataCfgReq *)GetDataPtr())->Ele.SatelliteCnt;
}

void CCfgGpsDataReq :: SetSatelliteCnt(UINT8 SatelliteCnt)
{
    ((T_GpsDataCfgReq *)GetDataPtr())->Ele.SatelliteCnt = SatelliteCnt;
}


CCfgGpsDataReq :: ~CCfgGpsDataReq()
{

}



//class CGPSSynchReq
UINT32 CGPSSynchReq::GetDefaultDataLen() const
{
     return sizeof(T_GPSSynchReq);
}
UINT16 CGPSSynchReq::GetDefaultMsgId() const
{
     return M_L3_L2_CFG_SYN_GPS_REQ;
}
UINT16 CGPSSynchReq::SetTransactionId(UINT16 transid)
{
     return  ((T_GPSSynchReq*)GetDataPtr())->TransId = transid;

}
    
void   CGPSSynchReq::SetHour(UINT8 hr) 
{
      ((T_GPSSynchReq*)GetDataPtr())->Hour = hr;

}
void   CGPSSynchReq::SetMinute(UINT8 mn) 
{
      ((T_GPSSynchReq*)GetDataPtr())->Minute = mn;

}
void   CGPSSynchReq::SetSecond(UINT8 sd) 
{
      ((T_GPSSynchReq*)GetDataPtr())->Second = sd;
}
void   CGPSSynchReq::SetMinSecond(int msd) 
{
      ((T_GPSSynchReq*)GetDataPtr())->MinSec = msd;

}


