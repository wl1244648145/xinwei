/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3OamBtsRegRsp.h
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

#ifndef _INC_L3OAMBTSREGRSP
#define _INC_L3OAMBTSREGRSP

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3OamBtsRegRsp : public CMessage
{
public: 
    CL3OamBtsRegRsp(CMessage &rMsg);
    CL3OamBtsRegRsp();
    bool CreateMessage(CComEntity&);
    ~CL3OamBtsRegRsp();
    UINT32 GetDefaultDataLen() const;
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetResult() const;
    void   SetResult(UINT16);
    
    UINT16 GetBtsRunState() const;
    void   SetBtsRunState(UINT16);

    void   SetBootupSource(UINT16 bs)
	{
    ((T_BtsRegRsp *)GetDataPtr())->BootupSource = bs;
    }

private:
#pragma pack(1)
	struct T_BtsRegRsp 
	{
	    UINT16  TransId;
        UINT16  Result;
        UINT16  BtsRunState;   // 0 -- init   1 -- running
        UINT16  BootupSource;
    };
#pragma pack()
};
#endif

