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

// L1 Configuration
// L1 General Setting Request£¨EMS£©
#ifndef _INC_L3OAMCFGL1GENDATAREQ
#define _INC_L3OAMCFGL1GENDATAREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgL1GenDataReq : public CMessage
{
public: 
    CCfgL1GenDataReq(CMessage &rMsg);
    CCfgL1GenDataReq();
    bool CreateMessage(CComEntity&);
    ~CCfgL1GenDataReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetSyncSrc() const;
    void   SetSyncSrc(UINT16);
    
    UINT16 GetGPSOffset() const;
    void   SetGPSOffset(UINT16);
    
    UINT16 GetAntennaMask() const;
    void   SetAntennaMask(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_L1GenCfgReq
    {   
        UINT16  TransId;
        T_L1GenCfgEle Ele;
    };
#pragma pack()
};
#endif

