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

#ifndef _INC_L3OAMCPUWORKINGNOTIFY
#define _INC_L3OAMCPUWORKINGNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif


class CL3OamCpuWorkingNotify : public CMessage
{
public: 
    CL3OamCpuWorkingNotify(CMessage &rMsg);
    CL3OamCpuWorkingNotify();
    bool CreateMessage(CComEntity&);
    ~CL3OamCpuWorkingNotify();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT8  GetCpuType() const;
    void   SetCpuType(UINT8);

    UINT8  GetCpuIndex() const;
    void   SetCpuIndex(UINT8);

private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        T_CpuWorkNofity Ele;
    };
#pragma pack()
};
#endif
