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

#ifndef _INC_L3OAMUPDATECPESWRESULTNOTIFY
#define _INC_L3OAMUPDATECPESWRESULTNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

// Upgrade UT Software Result Notification£¨BTS£©
class CUpdateUTSWResultNotify : public CMessage
{
public: 
    CUpdateUTSWResultNotify(CMessage &rMsg);
    CUpdateUTSWResultNotify();
    bool CreateMessage(CComEntity&);
    ~CUpdateUTSWResultNotify();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT32 GetCPEID();
    void   SetCPEID(UINT32);

    UINT16 GetResult();
    void   SetResult(UINT16);
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        UINT32  CPEID;
        UINT16  Result;
    };
#pragma pack()
};
#endif
