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
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

// UT Service Descriptor Configuration Request
#ifndef _INC_L3OAMCFGCPESERVICEREQ
#define _INC_L3OAMCFGCPESERVICEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgCpeServiceReq : public CMessage
{
public: 
    CCfgCpeServiceReq(CMessage &rMsg);
    CCfgCpeServiceReq();
    bool CreateMessage(CComEntity&);
    ~CCfgCpeServiceReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_UTSDCfgReq
    {
        UINT16 TransId;
        T_UTSDCfgEle Ele;
    };
#pragma pack()
};
#endif
