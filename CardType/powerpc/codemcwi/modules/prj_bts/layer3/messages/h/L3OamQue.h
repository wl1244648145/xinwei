#ifndef _INC_L3OAMQUEUEALMREQ
#define _INC_L3OAMQUEUEALMREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CGetAlmReq : public CMessage
{
public: 
    CGetAlmReq(CMessage &rMsg);
    CGetAlmReq();
    bool CreateMessage(CComEntity&);
    ~CGetAlmReq();
    UINT32 GetDefaultDataLen() const;

public:
    UINT16 GetTransId();
    void   SetTransId(UINT16);  

    UINT16 GetSequenceNum();
    void   SetSequenceNum(UINT16);  

    UINT8  GetFlag();
    void   SetFlag(UINT8);  

    UINT16 GetYear();
    void   SetYear(UINT16);  
    
    UINT8 GetMonth();
    void  SetMonth(UINT8);  
    
    UINT8 GetDay();
    void  SetDay(UINT8);  
    
    UINT8 GetHour();
    void  SetHour(UINT8);  
    
    UINT8 GetMinute();
    void  SetMinute(UINT8);  
    
    UINT8 GetSecond();
    void  SetSecond(UINT8);  
    
    UINT16 GetEntityType();
    void   SetEntityType(UINT16);  

    UINT16 GetEntityIndex();
    void   SetEntityIndex(UINT16);  

    UINT16 GetCodeIndex();
    void   SetCodeIndex(UINT16);  

    UINT16 GetSeverity();
    void   SetSeverity(UINT16);  

    UINT8 GetInfoLen();
    void   SetInfoLen(UINT8);  

    SINT8* GetAlarmInfo();
    void   SetAlarmInfo(const SINT8*);  
};
#endif
