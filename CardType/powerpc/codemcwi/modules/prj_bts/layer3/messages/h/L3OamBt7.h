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

#ifndef _INC_L3OAMBTSSWUPDATEREQ
#define _INC_L3OAMBTSSWUPDATEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CBtsSWUpdateReq : public CMessage
{
public: 
    CBtsSWUpdateReq(CMessage &rMsg);
    CBtsSWUpdateReq();
    bool CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;
    ~CBtsSWUpdateReq();

public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    
private:
#pragma pack(1)
    struct T_Req
    { UINT16 TransId; };
#pragma pack()
};
#endif
