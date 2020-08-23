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


#ifndef _INC_L3OAMCOMMONREQ
#define _INC_L3OAMCOMMONREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3OamCommonReq : public CMessage
{
public: 
    CL3OamCommonReq(CMessage &);
    CL3OamCommonReq();
    bool CreateMessage(CComEntity& Entity);
    virtual ~CL3OamCommonReq();
    UINT32  GetDefaultDataLen() const;
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
private:
#pragma pack(1)
    struct T_Req
    {
        UINT16 TransId;
    };
#pragma pack()
};
#endif
