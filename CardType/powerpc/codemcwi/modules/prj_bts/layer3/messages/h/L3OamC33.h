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


// Temperature Monitor Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGTEMPALARMREQ
#define _INC_L3OAMCFGTEMPALARMREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgTempAlarmReq : public CMessage
{
public: 
    CCfgTempAlarmReq(CMessage &rMsg);
    CCfgTempAlarmReq();
    bool CreateMessage(CComEntity&);
    ~CCfgTempAlarmReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT16 GetAlarmH() const;
    void   SetAlarmH(UINT16);

    UINT16 GetAlarmL() const;
    void   SetAlarmL(UINT16);

    UINT16 GetShutdownH() const;
    void   SetShutdownH(UINT16);

    UINT16 GetShutdownL() const;
    void   SetShutdownL(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_TempAlarmCfgReq
    {
        UINT16  TransId;
        T_TempAlarmCfgEle Ele;
    };
#pragma pack()
};    
#endif

