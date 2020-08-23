/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3L3L2BtsRstCntChangeNotify.h
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

//向后太返回开工指示消息,对应于接口文档中的3.7	BTS Boot Up Notification（BTS）
#ifndef _INC_L3L2BTSRSTCNTCHANGENOTIFY
#define _INC_L3L2BTSRSTCNTCHANGENOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3L2BtsRstCntChangeNotify : public CMessage
{
public: 
    CL3L2BtsRstCntChangeNotify(CMessage &rMsg);
    CL3L2BtsRstCntChangeNotify();
    bool CreateMessage(CComEntity& Entity);
    ~CL3L2BtsRstCntChangeNotify();
    UINT32 GetDefaultDataLen() const;  
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetRstCnt() const;
    void   SetRstCnt(UINT16);
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        UINT16  RstCnt;
    };
#pragma pack()
};

#endif
