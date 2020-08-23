#ifndef _INC_AP_MSGQUEUE
#define _INC_AP_MSGQUEUE

#ifndef NUCLEUS
//#include "NUCLEUS.h"//delete by huangjl
#endif

#ifndef DATATYPE_H
#include "dataType.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef BF_NU_L2
#define MSG_QUEUE_POOL_NODE_NUM   200
#else
#define MSG_QUEUE_POOL_NODE_NUM   800//250//200
#endif


#define INVALID_NODE_INDEX  0xFFFF

typedef struct 
{
    UINT16 _NextNodeIndex;
    UINT32 _Value;
}T_MsgQueueNode;


class CMsgQueue 
{
//Attributes
private:
//Operations
public:
    static bool InitMsgQueueClass();

    CMsgQueue(SINT32 iMaxMsgs, SINT32 option);
    bool PostMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent=false);

    bool PostMessage(UINT32 uMsg, SINT32 timeout, bool isUrgent=false);

    bool PeekFirstMessage(UINT32 *);

    CComMessage* GetMessage(SINT32 timeout);
    bool GetMessage(UINT32 *msg,SINT32 timeout);
    SINT32 GetCount(void) const;
    CMsgQueue();

private:

    static T_MsgQueueNode MsgQueNodePool[MSG_QUEUE_POOL_NODE_NUM];
    static UINT16  FirstFreeNodeIndex;
    static UINT16 GetFreeMsgNode();
    static void   ReturnFreeMsgNode(UINT16);
    
    static UINT16 FreeMsgQueueNodeNum;
    static UINT16 MsgQueueCreatedNum;

    NU_SEMAPHORE   m_QueuePendSem;

    UINT16 m_sQueueHeadNodeIndex;
    UINT16 m_sQueueTailNodeIndex;
    
    UINT16 m_sMaxMsgs;
    UINT16 m_sCurrentMsgs;
    UINT16 m_uMaxQueuedMsgs;

};

#endif //_INC_AP_MSGQUEUE
