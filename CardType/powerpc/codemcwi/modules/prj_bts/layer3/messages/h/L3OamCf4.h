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

//Billing Data Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGBILLINGDATAREQ
#define _INC_L3OAMCFGBILLINGDATAREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgBillingDataReq : public CMessage
{
public: 
    CCfgBillingDataReq(CMessage &rMsg);
    CCfgBillingDataReq();
    bool CreateMessage(CComEntity&);
    ~CCfgBillingDataReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetUploadInter() const;
    void   SetUploadInter(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
	void   setUserPassword(SINT8 *pUser, SINT8 *pPwd);
private:
#pragma pack(1)
    struct T_BillDataCfgReq
    {
        UINT16  TransId;
		UINT8   arrUserName[40];
		UINT8   arrPassword[40];
        T_BillDataCfgEle Ele;
    };
#pragma pack()
};
#endif

