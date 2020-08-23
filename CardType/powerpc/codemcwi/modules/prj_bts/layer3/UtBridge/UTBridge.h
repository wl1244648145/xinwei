/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    UTBridge.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   02/07/06   Yushu Shi    initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __UT_BRIDGE_H__
#define __UT_BRIDGE_H__

#include "vxWorks.h" 
#include "inetLib.h" 
#include "stdioLib.h" 
#include "strLib.h" 
#include "hostLib.h" 
#include "ioLib.h" 
#include <msgQLib.h>

#include <list>

#include "BizTask.h"
#include "ComMessage.h"
#include "LogArea.h"

//UT Bridge任务参数定义
#define M_TASK_UTBRIDGE_TASKNAME      "tUTBridge"
#define M_TASK_UTBRIDGE_OPTION        ( VX_FP_TASK )
#define M_TASK_UTBRIDGE_MSGOPTION     ( MSG_Q_FIFO )
#define M_TASK_UTBRIDGE_STACKSIZE     ( 20480 )
#define M_TASK_UTBRIDGE_MAXMSG        (1000)

//ComMessage链表的长度
#define M_MAX_LIST_SIZE             (1000)


//UT Bridge task
class CUTBridge : public CBizTask
{
public:
    static CUTBridge* GetInstance();
    static void RxDriverPacketCallBack(char *, UINT16, char *);
    static void EBFreeMsgCallBack (UINT32 param);

    inline TID GetEntityId() const 
    {
        return     CurrentTid;
    }

    CComMessage* GetComMessage();

private:
    CUTBridge();
    ~CUTBridge();

    bool Initialize();
    bool ProcessComMessage(CComMessage*);
    inline bool IsNeedTransaction() { return false;}
    virtual bool DeallocateComMessage(CComMessage*);


private:

    //static members:
    static CUTBridge* Instance;

    typedef struct _tag_stComMessageNode
    {
        NODE  lstHdr;
        CComMessage *pstComMsg;
    }stComMessageNode;

    LIST    m_listComMessage;
    LIST    m_listFreeNode;

    TID     CurrentTid;
    TID     ProxyTIDs[4];

    typedef struct
    {
        UINT16 CPEMacAddr[3];
        UINT16 BtsMacAddr[3];
        UINT16 Protocol;
        UINT16 DstTID;
        UINT16 SrcTID;
        UINT16 MsgID;
        UINT16 Length;
    }T_SecondHeader;
};

#endif /* __UT_BRIDGE_H__ */
