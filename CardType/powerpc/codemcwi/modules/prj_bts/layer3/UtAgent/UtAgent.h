/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   define the class for UTAgent entity
 *                proxy for all the OAM tasks on CPE, add the msg header between
 *                BTS L2 and L3, calculate the checksum
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   11/27/2005   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef __INC_UTAGENT
#define __INC_UTAGENT

#include "ComEntity.h"
#include "ComMessage.h"
#include "taskDef.h"
#include "L3L2MessageId.h"

class CUtAgent: public CComEntity
{
public:
    static CUtAgent* GetInstance();
    bool PostMessage(CComMessage*, SINT32, bool isUrgent = false);
    TID GetEntityId() const { return CurrentTid;};
    void ShowStatus();
    bool DeallocateComMessage(CComMessage* pComMsg);

private:
    
    CUtAgent();
    static CUtAgent *Instance;

    TID CurrentTid;

    static const TID ProxyTIDs[6]; 

    UINT32 ChecksumErrorCount;
    UINT32 FromUtOamMsgCount;
    UINT32 ToUtOamMsgCount;

    typedef struct
    {
        UINT8  Checksum;
        UINT8  Reserved;
        UINT16 DstTID;
        UINT16 SrcTID;
        UINT16 MsgID;
        UINT16 Length;
    }T_SecondHeader;
};


#endif //__INC_UTAGENT
