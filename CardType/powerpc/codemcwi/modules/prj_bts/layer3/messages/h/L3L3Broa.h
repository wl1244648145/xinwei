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


#ifndef _INC_L3L3BROADCASTUTSWTIMER
#define _INC_L3L3BROADCASTUTSWTIMER

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3BCUTSWTimer : public CMessage
{
public: 
    CL3BCUTSWTimer(CMessage &rMsg);
    CL3BCUTSWTimer();
    bool CreateMessage(CComEntity& Entity);
    ~CL3BCUTSWTimer();
    UINT32  GetDefaultDataLen() const;
    
    UINT16  GetTransactionId()const;
    UINT16  SetTransactionId(UINT16 TransId);
    
    UINT16  GetHWType()const;
    void    SetHWType(UINT16 );
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16 TransId;
        UINT16 HWType;
    };
#pragma pack()
};

class CL3BCUTSWTimerNew : public CMessage
{
public: 
    CL3BCUTSWTimerNew(CMessage &rMsg);
    CL3BCUTSWTimerNew();
    bool CreateMessage(CComEntity& Entity);
    ~CL3BCUTSWTimerNew();
    UINT32  GetDefaultDataLen() const;
    
    UINT16  GetTransactionId()const;
    UINT16  SetTransactionId(UINT16 TransId);
    
    UINT16  GetHWType()const;
    void    SetHWType(UINT16 );
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16 TransId;
        UINT16 HWType;
    };
#pragma pack()
};

#endif
