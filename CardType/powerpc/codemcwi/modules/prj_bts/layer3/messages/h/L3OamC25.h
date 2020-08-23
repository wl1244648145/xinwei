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

#ifndef _INC_L3OAMCFGINITFAILNOTIFY
#define _INC_L3OAMCFGINITFAILNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif
class CCfgInitFailNotify:public CMessage
{
public:
    CCfgInitFailNotify(CMessage &rMsg);
    CCfgInitFailNotify();
    bool CreateMessage(CComEntity &Entity);
    UINT32 GetDefaultDataLen() const;
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16  GetResult() const;
    void  SetResult(UINT16 Result);
    void SetFailMsgID(UINT16 usMsgID){ ((T_Notify*)GetDataPtr())->MsgID = usMsgID; };
    ~CCfgInitFailNotify();
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16 TransId;
        UINT16 Result;
		UINT16 MsgID;
    };
#pragma pack()
};
#endif

