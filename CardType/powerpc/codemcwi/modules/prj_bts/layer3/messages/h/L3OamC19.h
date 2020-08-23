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

#ifndef _INC_L3OAMCFGDELACLREQ
#define _INC_L3OAMCFGDELACLREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CDeleteACLReq : public CMessage
{
public: 
    CDeleteACLReq(CMessage &rMsg);
    CDeleteACLReq();
    bool CreateMessage(CComEntity&);
    ~CDeleteACLReq();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);  

    UINT8  GetIndex() const;
    void   SetIndex(UINT8);  
private:
#pragma pack(1)
    struct T_Notify
	{
	    UINT16 TransId;
        UINT8  Index;
	};
#pragma pack()
};
#endif
