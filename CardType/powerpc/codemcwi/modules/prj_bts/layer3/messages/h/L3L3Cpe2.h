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


#ifndef _INC_L3L3CPESWDLPACKRSP
#define _INC_L3L3CPESWDLPACKRSP

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3CpeSWDLPackRsp : public CMessage
{
public: 
    CL3CpeSWDLPackRsp(CMessage &rMsg);
    CL3CpeSWDLPackRsp();
    bool CreateMessage(CComEntity& Entity);
    ~CL3CpeSWDLPackRsp();
    UINT32  GetDefaultDataLen() const;

    UINT16  GetTransactionId() const;
    UINT16  SetTransactionId(UINT16 TransId);

    UINT16  GetSWPackSeqNum() const;
    void  SetSWPackSeqNum(UINT16 );
private:
#pragma pack(1)
    struct T_Rsp
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 SWPackSeqNum;
    };
#pragma pack()
};

#endif
