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

// Performance Logging Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGPERFLOGREQ
#define _INC_L3OAMCFGPERFLOGREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgPerfLogReq : public CMessage
{
public: 
    CCfgPerfLogReq(CMessage &rMsg);
    CCfgPerfLogReq();
    bool CreateMessage(CComEntity&);
    ~CCfgPerfLogReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    SINT32 GetFTPserverIP() const;
    void   SetFTPserverIP(SINT32);
    
    UINT16 GetFTPserverPort() const;
    void   SetFTPserverPort(UINT16);
    
    UINT8  GetUserNameLen() const;
    void   SetUserNameLen(UINT8);

    SINT8* GetUserName() const;
    void   SetUserName(SINT8*);

    UINT8  GetFTPPasswordLen() const;
    void   SetFTPPasswordLen(UINT8);

    SINT8* GetFTPPassword() const;
    void   SetFTPPassword(SINT8*);

    UINT16 GetPerfRptInter() const;
    void   SetPerfRptInter(UINT16);

    UINT16 GetPerfCollectInter() const;
    void   SetPerfCollectInter(UINT16);
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_PerfLogCfgReq
    {
        UINT16 TransId;
        T_PerfLogCfgEle Ele;
    };
#pragma pack()
};
#endif
