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

//	Calibration Configuration Data Request£¨EMS£©
#ifndef _INC_L3OAMCFGCFGCALREQ
#define _INC_L3OAMCFGCFGCALREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgCalReq : public CMessage
{
public: 
    CCfgCalReq(CMessage &rMsg);
    CCfgCalReq();
    bool CreateMessage(CComEntity&);
    ~CCfgCalReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetCalIner() const;
    void   SetCalIner(UINT16);
    
    UINT16 GetCalType() const;
    void   SetCalType(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_CalCfgReq
    {
        UINT16  TransId;
        T_CalCfgEle Ele;	
    };
#pragma pack()
};
#endif
