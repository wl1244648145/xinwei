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

#ifndef _INC_L3OAMCFGGPSLOCNOTIFY
#define _INC_L3OAMCFGGPSLOCNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgGpsLocNotify : public CMessage
{
public: 
    CCfgGpsLocNotify(CMessage &rMsg);
    CCfgGpsLocNotify();
    bool CreateMessage(CComEntity&);
    ~CCfgGpsLocNotify();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    SINT32 GetLatitude() const;
    void   SetLatitude(SINT32);

    SINT32 GetLongitude() const;
    void   SetLongitude(SINT32);

    SINT32 GetHeight() const;
    void   SetHeight(SINT32);

private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        T_GpsDataNotifyEle Ele;
    };
#pragma pack()
};
#endif
