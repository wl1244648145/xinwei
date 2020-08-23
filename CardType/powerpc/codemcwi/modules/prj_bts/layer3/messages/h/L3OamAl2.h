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
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMALMNOTIFYOAM
#define _INC_L3OAMALMNOTIFYOAM

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CAlarmNotifyOam : public CMessage
{
public: 
    CAlarmNotifyOam(CMessage &rMsg);
    CAlarmNotifyOam();
    bool CreateMessage(CComEntity&);
    ~CAlarmNotifyOam();
    UINT32 GetDefaultDataLen() const;

public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT32 GetSequenceNum();
    void   SetSequenceNum(UINT32);  

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

    UINT16 GetAlarmCode();
    void   SetAlarmCode(UINT16);  

    UINT8  GetSeverity();
    void   SetSeverity(UINT8);  

    UINT16 GetInfoLen();
    void   SetInfoLen(UINT16);  

    SINT8* GetAlarmInfo();
    void   SetAlarmInfo(const SINT8*);

    void   SetAlarmLength();

    void   show();
};
#endif
