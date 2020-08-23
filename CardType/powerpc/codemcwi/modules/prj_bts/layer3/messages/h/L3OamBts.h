/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3OamBtsDataDLoadNotify.h
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

#ifndef _INC_L3OAMBTSDATADLOADNOTIFY
#define _INC_L3OAMBTSDATADLOADNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

// BTS Data Download Notification£¨BTS£©
class CBtsDataDLoadNotify : public CMessage
{
public: 
    CBtsDataDLoadNotify(CMessage &rMsg);
    CBtsDataDLoadNotify();
    bool CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;
    ~CBtsDataDLoadNotify();
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);  
    
    UINT32 GetActiveVerion() const;
    void   SetActiveVerion(UINT32);  

    UINT32 GetStandbyVersion() const;
    void   SetStandbyVersion(UINT32);  

    UINT16 GetBtsHWType() const;
    void   SetBtsHWType(UINT16);  
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        UINT32  ActiveVersion;
        UINT32  StandbyVersion;
        UINT16  BtsHWType;
    };
#pragma pack()
};
#endif
