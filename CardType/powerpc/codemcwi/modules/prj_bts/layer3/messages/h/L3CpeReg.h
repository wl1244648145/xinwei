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
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/


#ifndef _INC_L3CPEREGTOEMSFAIL
#define _INC_L3CPEREGTOEMSFAIL

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3CpeRegtoEmsFail : public CMessage
{
public: 
    CL3CpeRegtoEmsFail(CMessage &rMsg);
    CL3CpeRegtoEmsFail();
    bool CreateMessage(CComEntity& Entity);
    ~CL3CpeRegtoEmsFail();
    UINT32  GetDefaultDataLen() const;
    UINT16  GetTransactionId()const;
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
