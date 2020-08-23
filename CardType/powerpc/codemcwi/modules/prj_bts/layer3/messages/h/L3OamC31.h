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

#ifndef _INC_L3OAMCFGSFIDREQ
#define _INC_L3OAMCFGSFIDREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgSFIDReq : public CMessage
{
public: 
    CCfgSFIDReq(CMessage &rMsg);
    CCfgSFIDReq();
    bool CreateMessage(CComEntity&);
    ~CCfgSFIDReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_SFIDReq
    {
        UINT16  TransId;
        T_SFIDCfgEle Ele;
    };
#pragma pack()
};
#endif
