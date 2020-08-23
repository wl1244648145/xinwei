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

#ifndef _INC_L3L3L2RFSTATENOTIFY
#define _INC_L3L3L2RFSTATENOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

class CL3L2RFStateNoitfy : public CMessage
{
public: 
    CL3L2RFStateNoitfy(CMessage &rMsg);
    CL3L2RFStateNoitfy();
    bool CreateMessage(CComEntity&);
    ~CL3L2RFStateNoitfy();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    UINT8  GetErrorAntenna();    
    const  T_RFStateInfo* GetRFStateInfo()const; 
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        T_RFStateInfo RFStateInfo;
    };
#pragma pack()
};
#endif
