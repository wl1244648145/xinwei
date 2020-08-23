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
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef _INC_L3OAMBTSREGREQ
#define _INC_L3OAMBTSREGREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

// BTS Register Request £¨EMS£©
class CBtsRegReq : public CMessage
{
public: 
    CBtsRegReq(CMessage &rMsg);
    CBtsRegReq();
    bool CreateMessage(CComEntity&);
    ~CBtsRegReq();
    UINT32 GetDefaultDataLen() const;
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT16 GetResult() const;
    void   SetResult(UINT16);  

    SINT8* GetSessionId() const;
    void   SetSessionId(SINT8*);  

	UINT32 GetBtsPublicIp();
	UINT16 GetBtsPublicPort();
	
private:
    #define SESSION_ID_LEN  32 
#pragma pack(1)
	struct T_BtsRegReq 
	{
	    UINT16  TransId;
        UINT16  Result;
        SINT8   SessionID[SESSION_ID_LEN];
		UINT32  BtsPublicIp;
		UINT16  BtsPublicPort;
		UINT16  HeartBeatInterval;
    };
#pragma pack()
};
#endif
