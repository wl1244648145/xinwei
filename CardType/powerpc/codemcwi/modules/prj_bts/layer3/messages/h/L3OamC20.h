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

#ifndef _INC_L3OAMCFGDMDATASERVICEREQ
#define _INC_L3OAMCFGDMDATASERVICEREQ


#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


// Data Service Configuration Request£¨EMS£©
class CCfgDmDataServiceReq : public CMessage
{
public: 
    CCfgDmDataServiceReq(CMessage &rMsg);
    CCfgDmDataServiceReq();
    bool CreateMessage(CComEntity&);
    ~CCfgDmDataServiceReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT8 GetMobility() const;
    void   SetMobility(UINT8 );
    
    UINT8 GetAccessControl() const;
    void   SetAccessControl(UINT8);

    UINT16 GetLBATimerLen() const;
    void   SetLBATimerLen(UINT16);
private:
#pragma pack(1)
    struct T_DataServiceCfgReq
    {   //data ok 2005-09-07
        UINT16  TransId;
        T_DmDataServiceCfgEle Ele;
    };
#pragma pack()
};
#endif
