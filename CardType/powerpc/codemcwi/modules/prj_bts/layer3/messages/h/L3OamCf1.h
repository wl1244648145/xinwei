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

// 5.3.3	Airlink Miscellaneous Setting Request£¨EMS£©
#ifndef _INC_L3OAMCFGAIRLINKMISCREQ
#define _INC_L3OAMCFGAIRLINKMISCREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgAirLinkMiscReq : public CMessage
{
public: 
    CCfgAirLinkMiscReq(CMessage &rMsg);
    CCfgAirLinkMiscReq();
    bool CreateMessage(CComEntity&);
    ~CCfgAirLinkMiscReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    SINT8  GetSSTSync() const;
    void   SetSSTSync(SINT8);
    
    UINT8  GetSNRTLeave() const;
    void   SetSNRTLeave(UINT8);
    
    UINT8  GetSNRTEnter() const;
    void   SetSNRTEnter(UINT8);
    
    UINT8  GetBTSLTLeave() const;
    void   SetBTSLTLeave(UINT8);
    
    UINT8  GetBTSLTLEnter() const;
    void   SetBTSLTLEnter(UINT8);
    
    UINT8  GetWakeupInterval() const;
    void   SetWakeupInterval(UINT8);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_AirLinkMisCfgReq
    {
        UINT16 TransId;
        T_AirLinkMisCfgEle Ele;
    };
#pragma pack()
};
#endif
