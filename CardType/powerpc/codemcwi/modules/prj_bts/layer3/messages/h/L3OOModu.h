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

#ifndef _INC_L3OOModuleInitNOTIFY
#define _INC_L3OOModuleInitNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CCfgModuleInitNotify : public CMessage
{
public: 
    CCfgModuleInitNotify(CMessage &rMsg);
    CCfgModuleInitNotify();
    bool CreateMessage(CComEntity&);
    ~CCfgModuleInitNotify();
    UINT32 GetDefaultDataLen() const;

public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT8  GetType() const;
    void   SetType(UINT8);  

private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16 TransId;
        UINT8  Type;
    };
#pragma pack()
};
#endif
