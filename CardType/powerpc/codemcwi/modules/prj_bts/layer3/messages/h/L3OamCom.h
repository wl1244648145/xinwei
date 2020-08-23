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


#ifndef _INC_L3OAMCOMMONFAIL
#define _INC_L3OAMCOMMONFAIL

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3OamCommonFail : public CMessage
{
public: 
    CL3OamCommonFail(CMessage &rMsg);
    CL3OamCommonFail();
    bool CreateMessage(CComEntity& Entity);
    ~CL3OamCommonFail();
    UINT32  GetDefaultDataLen() const;
    UINT16  GetTransactionId() const;
    UINT16  SetTransactionId(UINT16 TransId);
private:
#pragma pack(1)
    struct T_Fail
    {
        UINT16 TransId;
    };
#pragma pack()
};

#endif
