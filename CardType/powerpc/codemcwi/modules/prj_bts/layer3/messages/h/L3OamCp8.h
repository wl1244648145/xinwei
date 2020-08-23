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

#ifndef _INC_L3OAMCPESWITCHOFFNOTIFY
#define _INC_L3OAMCPESWITCHOFFNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif


//BTS sends this message to the EMS when the Modem is switched off
class CCpeSwitchOffNotify : public CMessage
{
public: 
    CCpeSwitchOffNotify(CMessage &rMsg);
    CCpeSwitchOffNotify();
    bool CreateMessage(CComEntity&);
    ~CCpeSwitchOffNotify();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT32 GetCPEID() const;
    void   SetCPEID(UINT32);
    
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        UINT32  CPEID;
    };
#pragma pack()
};
#endif
