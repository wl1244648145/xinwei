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
 *   08/03/2005   ÃÔæ≤Œ∞       Initial file creation.
 *---------------------------------------------------------------------------*/

//≈‰÷√”Ô“Ù∂Àø⁄–≈œ¢ 
#ifndef _INC_L3L3CFGVOICEPORTNOTIFY
#define _INC_L3L3CFGVOICEPORTNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CCfgVoicePortNotify : public CMessage
{
public: 
    CCfgVoicePortNotify(CMessage &rMsg);
    CCfgVoicePortNotify();
    bool CreateMessage(CComEntity&);
    ~CCfgVoicePortNotify();
    UINT32 GetDefaultDataLen() const;  
public:
   //UINT16 GetTransactionId() const;
   // UINT16 SetTransactionId(UINT16);
    
    UINT8  GetVoicePort() const;
    void   SetVoicePort(UINT8);

    UINT32 GetVoicePortID() const;
    void   SetVoicePortID(UINT32);

    UINT8  getIsCpeZ() const;
    void   setIsCpeZ(bool);

private:
#pragma pack(1)
    struct T_CfgNotify
    {
        //UINT16 TransId;
        UINT8  VoicePort;
        UINT32 VoicePortID;
		UINT8  isCpeZ;
    };
#pragma pack()
};
#endif
