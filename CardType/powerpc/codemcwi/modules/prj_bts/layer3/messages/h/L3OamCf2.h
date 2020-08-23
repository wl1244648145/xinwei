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
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/

// L2 Configuration
// 5.3.1	Airlink Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGAIRLINKREQ
#define _INC_L3OAMCFGAIRLINKREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgAirLinkReq : public CMessage
{
public: 
    CCfgAirLinkReq(CMessage &rMsg);
    CCfgAirLinkReq();
    bool CreateMessage(CComEntity&);
    ~CCfgAirLinkReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_AirLinkCfgReq
    {
        UINT16 TransId;
        T_AirLinkCfgEle Ele;
    };
#pragma pack()
};
#endif


