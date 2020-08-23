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

#ifndef _INC_L3OAMBCCPESWRSP
#define _INC_L3OAMBCCPESWRSP

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#if 0
class CBCUTSWRsp : public CMessage
{
public: 
    CBCUTSWRsp(CMessage &rMsg);
    CBCUTSWRsp();
    bool CreateMessage(CComEntity&);
    ~CBCUTSWRsp();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT16 GetResult();
    void   SetResult(UINT16);


    
private:
#pragma pack(1)
    struct T_Rsp
    {
        UINT16 TransId;
        UINT16 Result;
    };
#pragma pack()
};
#endif

#endif
