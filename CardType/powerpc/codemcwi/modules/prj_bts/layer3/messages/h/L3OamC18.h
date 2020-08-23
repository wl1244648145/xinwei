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

#ifndef _INC_L3OAMCFGDATASERVICEREQ
#define _INC_L3OAMCFGDATASERVICEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


// Data Service Configuration Request£¨EMS£©
class CCfgDataServiceReq : public CMessage
{
public: 
    CCfgDataServiceReq(CMessage &rMsg);
    CCfgDataServiceReq();
    bool CreateMessage(CComEntity&);
    ~CCfgDataServiceReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT32 GetRoutingAreaID() const;
    void   SetRoutingAreaID(UINT32 );
    
    UINT8  GetMobility() const;
    void   SetMobility(UINT8 );
    
    UINT8  GetAccessControl() const;
    void   SetAccessControl(UINT8);
    
    UINT8  GetP2PBridging() const;
    void   SetP2PBridging(UINT8);
    
    UINT8  GetEgrARPRroxy() const;
    void   SetEgrARPRroxy(UINT8);
    
    UINT8  GetEgrBCFilter() const;
    void   SetEgrBCFilter(UINT8);
    
    UINT16 GetLBATimerLen() const;
    void   SetLBATimerLen(UINT16);
    
    UINT16 GetPPPSessionLen() const;
    void   SetPPPSessionLen(UINT16);
    UINT8  GetTargetBtsID() const;
    void   SetTargetBtsID(UINT8 );
    UINT8  GetTargetEID() const;
    void   SetTargetEID(UINT8 );
    UINT8  GetTargetPPPoEEID() const;
    void   SetTargetPPPoEEID(UINT8 );	
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_DataServiceCfgReq
    {   //data ok 2005-09-07
        UINT16  TransId;
        T_DataServiceCfgEle Ele;
    };
#pragma pack()
};
#endif
