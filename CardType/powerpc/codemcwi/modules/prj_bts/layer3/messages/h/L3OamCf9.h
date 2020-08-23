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

//5.4.3	Calibration Action Request£¨EMS£©
#ifndef _INC_L3OAMCFGCALACTIONREQ
#define _INC_L3OAMCFGCALACTIONREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgCalActionReq : public CMessage
{
public: 
    CCfgCalActionReq(CMessage &rMsg);
    CCfgCalActionReq();
    bool CreateMessage(CComEntity&);
    ~CCfgCalActionReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetCalType() const;
    void   SetCalType(UINT16);
    void   SetCalTrigger(UINT16);
private:
#pragma pack(1)
    struct T_InstantCalReq
    {
        UINT16  TransId;
        T_InstantCalEle Ele;
    };
#pragma pack()
};
#endif
