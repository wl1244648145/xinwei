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

#ifndef _INC_L3OAMSESSIONIDUPDATEREQ
#define _INC_L3OAMSESSIONIDUPDATEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif




//	SessionID Update Request(EMS)
class CSessionIdUpdateReq : public CMessage
{
public: 
    CSessionIdUpdateReq(CMessage &rMsg);
    CSessionIdUpdateReq();
    bool CreateMessage(CComEntity&);
    ~CSessionIdUpdateReq();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT32  GetSessionID() const;
    void   SetSessionID(UINT32);  
private:
#pragma pack(1)
    struct T_Req
    {
        UINT16  TransId;
        UINT32  SessionID;
    };
#pragma pack()
};
#endif
