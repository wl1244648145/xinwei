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

#ifndef _INC_L3OAMBCCPESWRATENOTIFY
#define _INC_L3OAMBCCPESWRATENOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#if 0
// Broadcast UT Software Progress Notification£¨BTS£©
class CBCUTSWRateNotify : public CMessage
{
public: 
    CBCUTSWRateNotify(CMessage &rMsg);
    CBCUTSWRateNotify();
    bool CreateMessage(CComEntity&);
    ~CBCUTSWRateNotify();
    UINT32 GetDefaultDataLen() const;  
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetCpeHWType();
    void   SetCpeHWType(UINT16 Type);
    
    UINT8  GetProgress();
    void   SetProgress(UINT8 pro);
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16 TransId;
        UINT8  Progress;
		UINT16 CpeHWType;
    };
#pragma pack()
};
#endif

#endif
