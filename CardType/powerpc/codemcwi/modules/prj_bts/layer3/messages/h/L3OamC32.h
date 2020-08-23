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

#ifndef _INC_L3OAMCFGSNOOPDATASERVICEREQ
#define _INC_L3OAMCFGSNOOPDATASERVICEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


// Data Service Configuration Request£¨EMS£©
class CCfgSnoopDataServiceReq : public CMessage
{
public: 
    CCfgSnoopDataServiceReq(CMessage &rMsg);
    CCfgSnoopDataServiceReq();
    bool CreateMessage(CComEntity&);
    ~CCfgSnoopDataServiceReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT32 GetRoutingAreaID() const;
    void   SetRoutingAreaID(UINT32 );

    UINT8  GetTargetBtsID() const;
    void   SetTargetBtsID(UINT8 );
    UINT8  GetTargetEID() const;
    void   SetTargetEID(UINT8 );
    UINT8  GetTargetPPPoEEID() const;
    void   SetTargetPPPoEEID(UINT8 );	
		
private:
#pragma pack(1)
    struct T_DataServiceCfgReq
    {   //data ok 2005-09-07
        UINT16  TransId;
        T_SnoopDataServiceCfgEle Ele;
    };
#pragma pack()
};
#endif
