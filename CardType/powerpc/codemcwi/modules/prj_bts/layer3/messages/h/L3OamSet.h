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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMSETCPETIMENOTIFY
#define _INC_L3OAMSETCPETIMENOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


//广播同步cpe时钟
class CSetCpeTimeNotify : public CMessage
{
public: 
    CSetCpeTimeNotify(CMessage &rMsg);
    CSetCpeTimeNotify();
    bool CreateMessage(CComEntity&);
    ~CSetCpeTimeNotify();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetYear() const;
    void   SetYear(UINT16);  
    
    UINT8 GetMonth() const;
    void  SetMonth(UINT8);  
    
    UINT8 GetDay() const;
    void  SetDay(UINT8);  
    
    UINT8 GetHour() const;
    void  SetHour(UINT8);  
    
    UINT8 GetMinute() const;
    void  SetMinute(UINT8);  
    
    UINT8 GetSecond() const;
    void  SetSecond(UINT8);  
    
private:
#pragma pack(1)
    struct T_NotifyTime
    {
        UINT16  TransId;
////////UINT16  Version;
        UINT16  Year;		
        UINT8   Month;
        UINT8   Day;
        UINT8   Hour;		
        UINT8   Minute;		
        UINT8   Second;		
    };
#pragma pack()
};


#endif

