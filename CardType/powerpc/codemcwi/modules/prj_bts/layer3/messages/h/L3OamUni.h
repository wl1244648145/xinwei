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


#ifndef _INC_L3OAMUNICASTUTSWREQFAIL
#define _INC_L3OAMUNICASTUTSWREQFAIL

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3OamUnicastUTSWReqFail : public CMessage
{
public: 
    CL3OamUnicastUTSWReqFail(CMessage &rMsg);
    CL3OamUnicastUTSWReqFail();
    bool CreateMessage(CComEntity& Entity, UINT16 usMsgID=0X2040/*M_OAM_UNICAST_UTSW_REQ_FAIL*/);
    ~CL3OamUnicastUTSWReqFail();
    UINT32  GetDefaultDataLen() const;
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT32 GetCPEID() const;
    void   SetCPEID (UINT32 );
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16 TransId;
        UINT32 CPEID;
    };
#pragma pack()
};
#endif
