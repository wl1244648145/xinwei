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

#ifndef _INC_L3OAMCFGEBDATASERVICEREQ
#define _INC_L3OAMCFGEBDATASERVICEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

// Data Service Configuration Request£¨EMS£©
class CCfgEbDataServiceReq : public CMessage
{
public: 
    CCfgEbDataServiceReq(CMessage &rMsg);
    CCfgEbDataServiceReq();
    bool CreateMessage(CComEntity&);
    ~CCfgEbDataServiceReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT8  GetEgrBCFilter() const;
    void   SetEgrBCFilter(UINT8);
    
    UINT16 GetPPPSessionLen() const;
    void   SetPPPSessionLen(UINT16);

    UINT16 GetLBATimerLen() const;
    void   SetLBATimerLen(UINT16);
    
    UINT8  GetAccessControl() const;
    void   SetAccessControl(UINT8);
private:
#pragma pack(1)
    struct T_DataServiceCfgReq
    {   
        UINT16  TransId;
        T_EbDataServiceCfgEle Ele;
    };
#pragma pack()
};
#endif
