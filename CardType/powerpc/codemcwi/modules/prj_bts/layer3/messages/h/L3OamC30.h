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


// 5.3.2	Resource Management Policy Request£¨EMS£©
#ifndef _INC_L3OAMCFGRMPOLICYREQ
#define _INC_L3OAMCFGRMPOLICYREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgRMPolicyReq : public CMessage
{
public: 
    CCfgRMPolicyReq(CMessage &rMsg);
    CCfgRMPolicyReq();
    bool CreateMessage(CComEntity&);
    ~CCfgRMPolicyReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetBWReqInterval() const;
    void   SetBWReqInterval(UINT16);
    
    UINT16 GetSRelThreshold() const;
    void   SetSRelThreshold(UINT16);
    
    SINT16 GetMinULSS() const;
    void   SetMinULSS(SINT16);
    
    UINT16  GetMaxDLPPUser() const;
    void   SetMaxDLPPUser(UINT16);
    
    UINT16 GetServDesIndex() const;
    void   SetServDesIndex(UINT16);
    
    UINT16 GetDLBWPerUser() const;
    void   SetDLBWPerUser(UINT16);
    
    UINT16 GetULBWPerUser() const;
    void   SetULBWPerUser(UINT16);
    
    UINT16 GetRsvTCH() const;
    void   SetRsvTCH(UINT16);
    
    UINT16  GetRsvPower() const;
    void   SetRsvPower(UINT16);
    
    UINT16  GetPCRange() const;
    void   SetPCRange(UINT16);
    
    UINT16 GetStepSize() const;
    void   SetStepSize(UINT16);
    
    UINT16 GetMaxUser() const;
    void   SetMaxUser(UINT16);
    
    UINT8* GetBWDistClass() const;
    void   SetBWDistClass(UINT8*);
    
    UINT8  GetULModMask() const;
    void   SetULModMask(UINT8);
    
    UINT8  GetDLModMask() const;
    void   SetDLModMask(UINT8);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_RMPoliceReq
    {
        UINT16  TransId;
        T_RMPoliceEle Ele;
    };
#pragma pack()
};
#endif
