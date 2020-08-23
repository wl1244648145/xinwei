/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3OamBtsRstNotify.h
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

#ifndef _INC_L3OAMBTSRSTNOTIFY
#define _INC_L3OAMBTSRSTNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

// BTS Reset Notification£¨BTS£©
class CBtsRstNotify : public CMessage
{
public: 
    CBtsRstNotify(CMessage &rMsg);
    CBtsRstNotify();
    bool CreateMessage(CComEntity&);
    ~CBtsRstNotify();
    UINT32 GetDefaultDataLen() const;
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);  
    
    UINT32 GetActiveVersion() const;
    void   SetActiveVersion(UINT32);  

    UINT32 GetStandbyVersion() const;
    void   SetStandbyVersion(UINT32);  

    UINT16 GetBtsHWType() const;
    void   SetBtsHWType(UINT16);  

    UINT16 GetBootupSource() const;
    void   SetBootupSource(UINT16);  

    UINT16 GetResetReason() const;
    void   SetResetReason(UINT16);  
private:
#pragma pack(1)
    struct T_BtsRstNotify
    {
        UINT16  TransId;
        UINT32  ActiveVersion;
        UINT32  StandbyVersion;
        UINT16  BtsHWType;
        UINT16  BootupSource;
        UINT16  ResetReason;
    };
#pragma pack()
};


#endif
