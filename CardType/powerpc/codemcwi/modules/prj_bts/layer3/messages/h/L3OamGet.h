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

#ifndef _INC_L3OAMGETBOARDSSTATERSP
#define _INC_L3OAMGETBOARDSSTATERSP

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3L2CFGCOMMON
#include "L3L2CfgCommon.h"
#endif


class CL3GetBoardsStatesRsp : public CMessage
{
public: 
    CL3GetBoardsStatesRsp(CMessage &rMsg);
    CL3GetBoardsStatesRsp();
    bool CreateMessage(CComEntity&);
    ~CL3GetBoardsStatesRsp();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
	UINT16 GetResult() const;
    UINT16 SetResult(UINT16);

	void   SetEle(SINT8*, UINT32 Len);
private:
#pragma pack(1)
    struct T_Req
    {
        UINT16 TransId;
        UINT16 Result;
        T_BTSBoardsState Ele;
    };
#pragma pack()
};
#endif
