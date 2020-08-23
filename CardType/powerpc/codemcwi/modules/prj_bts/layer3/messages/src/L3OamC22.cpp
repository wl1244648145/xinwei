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
#ifndef _INC_L3OAMCFGGPSLOCNOTIFY
#include "L3OamCfgGpsLocNotify.h"     
#endif


CCfgGpsLocNotify :: CCfgGpsLocNotify(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgGpsLocNotify :: CCfgGpsLocNotify()
{
}

bool CCfgGpsLocNotify :: CreateMessage(CComEntity &Entity)
{
    if (true == CMessage :: CreateMessage(Entity))
        {
        SetMessageId(M_BTS_EMS_GPS_DATA_NOTIFY);
        return true;
        }
    else
        {
        return false;
        }
}

UINT32 CCfgGpsLocNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CCfgGpsLocNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CCfgGpsLocNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT32 CCfgGpsLocNotify :: GetLatitude() const
{
    return ((T_Notify *)GetDataPtr())->Ele.Latitude;
}

void CCfgGpsLocNotify ::   SetLatitude(SINT32 Latitude)
{
    ((T_Notify *)GetDataPtr())->Ele.Latitude = Latitude;
}

SINT32 CCfgGpsLocNotify :: GetLongitude() const
{
    return ((T_Notify *)GetDataPtr())->Ele.Longitude;
}

void CCfgGpsLocNotify ::   SetLongitude(SINT32 Longitude)
{
    ((T_Notify *)GetDataPtr())->Ele.Longitude = Longitude;
}

SINT32 CCfgGpsLocNotify :: GetHeight() const
{
    return ((T_Notify *)GetDataPtr())->Ele.Height;
}

void CCfgGpsLocNotify ::   SetHeight(SINT32 Height)
{
    ((T_Notify *)GetDataPtr())->Ele.Height = Height;
}

CCfgGpsLocNotify :: ~CCfgGpsLocNotify()
{

}



