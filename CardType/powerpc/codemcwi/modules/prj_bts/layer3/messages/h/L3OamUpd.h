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

#ifndef _INC_L3OAMUPDATECPESWRARENOTIFY
#define _INC_L3OAMUPDATECPESWRARENOTIFY


#ifndef _INC_MESSAGE
#include "Message.h"
#endif

// UpGrade UT Software Progress Notification£¨BTS£©
class CUpdateCpeSWRateNotify : public CMessage
{
public: 
    CUpdateCpeSWRateNotify(CMessage &rMsg);
    CUpdateCpeSWRateNotify();
    bool CreateMessage(CComEntity&);
    ~CUpdateCpeSWRateNotify();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT32 GetCPEID();
    void   SetCPEID(UINT32);

    UINT8 GetProgress();
    void   SetProgress(UINT8);
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        UINT32  CPEID;
        UINT8   Progress;
    };
#pragma pack()
};

#endif
