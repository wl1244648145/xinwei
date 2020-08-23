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
#ifndef _INC_L3OODELETEALARMTIFY
#define _INC_L3OODELETEALARMTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CDeleteAlarmNotify : public CMessage
{
public: 
    CDeleteAlarmNotify(CMessage &rMsg);
    CDeleteAlarmNotify();
    bool CreateMessage(CComEntity&);
    ~CDeleteAlarmNotify();
    UINT32 GetDefaultDataLen() const;
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT16 GetEntityType();
    void   SetEntityType(UINT16);  

    UINT16 GetEntityIndex();
    void   SetEntityIndex(UINT16);  

    UINT16 GetAlarmCode();
    void   SetAlarmCode(UINT16);  
    
private:
#pragma pack(1)
	struct T_Notify 
	{
	    UINT16  TransId;
        UINT16  AlarmCode;		
        UINT16  EntityType;	   
        UINT16  EntityIndex;		
    };
#pragma pack()
};
#endif
