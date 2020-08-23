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


#ifndef _INC_L3OACPUALARMNOTIFY
#define _INC_L3OACPUALARMNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

class CL3CpuAlarmNofity : public CMessage
{
public: 
    CL3CpuAlarmNofity(CMessage &rMsg);
    CL3CpuAlarmNofity();
    bool CreateMessage(CComEntity& Entity);
    ~CL3CpuAlarmNofity();
    UINT32  GetDefaultDataLen() const;
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    const  T_CpuAlarmNofity* GetCpuAlarmNofity()const; 
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        T_CpuAlarmNofity CpuAlarmNofity;
    };
#pragma pack()
};
#endif
