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

#ifndef _INC_L3OAMCPEREQ
#define _INC_L3OAMCPEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


// Reset UT Req£¨EMS£©
class CCpeReq : public CMessage
{
public: 
    CCpeReq(CMessage &rMsg);
    CCpeReq();
    bool CreateMessage(CComEntity&);
    ~CCpeReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT32 GetCPEID() const;
    void   SetCPEID(UINT32);
    
private:
#pragma pack(1)
    struct T_Req
    {
        UINT16  TransId;
        UINT32  CPEID;
    };
#pragma pack()
};
#endif
