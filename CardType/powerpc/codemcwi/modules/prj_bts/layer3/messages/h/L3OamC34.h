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
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/

// ToS Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGTOSREQ
#define _INC_L3OAMCFGTOSREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgTosReq : public CMessage
{
public: 
    CCfgTosReq(CMessage &rMsg);
    CCfgTosReq();
    bool CreateMessage(CComEntity&);
    ~CCfgTosReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_ToSCfgReq
    {
        UINT16 TransId;
        T_ToSCfgEle  Ele[MAX_TOS_ELE_NUM];	
    };
#pragma pack()
};
#endif
