/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3OamBtsRegReq.h
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
#ifndef _INC_L3OAMALMHANDLENOTIFY
#define _INC_L3OAMALMHANDLENOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3OamAlarmHandleNotify : public CMessage
{
public: 
    CL3OamAlarmHandleNotify(CMessage &rMsg);
    CL3OamAlarmHandleNotify();
    bool CreateMessage(CComEntity&);
    ~CL3OamAlarmHandleNotify();
    UINT32 GetDefaultDataLen() const;
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    bool   SetMsgData(SINT8* , UINT16 );
private:
#pragma pack(1)
	struct T_Notify 
	{
	    UINT16  TransId;
        SINT8   Rsv[2];
    };
#pragma pack()
};
#endif
