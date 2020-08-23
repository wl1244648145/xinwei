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

// BTS Reset Request£¨EMS£©
#ifndef _INC_L3OAMCFGBTSRSTREQ
#define _INC_L3OAMCFGBTSRSTREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgBtsRstReq : public CMessage
{
public: 
    CCfgBtsRstReq(CMessage &rMsg);
    CCfgBtsRstReq();
    bool CreateMessage(CComEntity&);
    ~CCfgBtsRstReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetDataSource() const;
    void   SetDataSource(UINT16);
    
private:
#pragma pack(1)
    struct T_BtsRstReq
    {
        UINT16 TransId;
        T_BtsRstEle Ele;	
    };
#pragma pack()
};
#endif
