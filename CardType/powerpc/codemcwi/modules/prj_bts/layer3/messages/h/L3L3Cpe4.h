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


#ifndef _INC_L3L3CPESWDLRESULTNOTIFY
#define _INC_L3L3CPESWDLRESULTNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3CpeSWDLResultNotify : public CMessage
{
public: 
    CL3CpeSWDLResultNotify(CMessage &rMsg);
    CL3CpeSWDLResultNotify();
    bool CreateMessage(CComEntity& Entity);
    ~CL3CpeSWDLResultNotify();
    UINT32  GetDefaultDataLen() const;
    
    UINT16  GetTransactionId() const;
    UINT16  SetTransactionId(UINT16 TransId);
    
    UINT16  GetResult() const;
    void  SetResult(UINT16 );
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 Result;
    };
#pragma pack()
};

#endif
