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

#ifndef _INC_L3OAMCFGARPDATASERVICEREQ
#define _INC_L3OAMCFGARPDATASERVICEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


// Data Service Configuration Request£¨EMS£©
class CCfgArpDataServiceReq : public CMessage
{
public: 
    CCfgArpDataServiceReq(CMessage &rMsg);
    CCfgArpDataServiceReq();
    bool CreateMessage(CComEntity&);
    ~CCfgArpDataServiceReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT8 GetP2PBridging() const;
    void   SetP2PBridging(UINT8);
    
    UINT8 GetEgrARPRroxy() const;
    void   SetEgrARPRroxy(UINT8);
private:
#pragma pack(1)
    struct T_DataServiceCfgReq
    {   //data ok 2005-09-07
        UINT16  TransId;
        T_ArpDataServiceCfgEle Ele;
    };
#pragma pack()
};
#endif
