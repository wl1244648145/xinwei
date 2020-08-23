/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: MsgQueue.cpp
 *
 * DESCRIPTION:
 *     Implementation of the FWKLIB's internal message queue object.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 11/01/2005  Liu Qun     Change implementation to CPtrList under __WIN32_SIM__
 * 07/12/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_AP_MSGQUEUE
#include "ApMsgQueue.h"
#endif

#include "ApAssert.h"
#include "FrameworkErrorCode.h"
#include <string.h>

T_MsgQueueNode CMsgQueue::MsgQueNodePool[MSG_QUEUE_POOL_NODE_NUM];
UINT16  CMsgQueue::FirstFreeNodeIndex;
UINT16 CMsgQueue::FreeMsgQueueNodeNum;
UINT16 CMsgQueue::MsgQueueCreatedNum;
UINT32  FreeQueueNodeRunOutCount=0;

#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section("p_init")
#else
#pragma CODE_SECTION("p_init")
#endif
#endif
bool CMsgQueue::InitMsgQueueClass()
{
    for (int i=0; i<MSG_QUEUE_POOL_NODE_NUM; i++)
    {
        MsgQueNodePool[i]._NextNodeIndex = i+1;
        MsgQueNodePool[i]._Value = 0xFFFFFFFF;
    }
    MsgQueNodePool[MSG_QUEUE_POOL_NODE_NUM-1]._NextNodeIndex = INVALID_NODE_INDEX;
    FirstFreeNodeIndex = 0;
    FreeMsgQueueNodeNum = MSG_QUEUE_POOL_NODE_NUM;

    MsgQueueCreatedNum = 0;
    return true;
}


// this function can only be called with interrupt disabled
UINT16 CMsgQueue::GetFreeMsgNode()
{
    UINT16 nodeIndex = INVALID_NODE_INDEX;

    if (MSG_QUEUE_POOL_NODE_NUM > FirstFreeNodeIndex)
    {
        nodeIndex = FirstFreeNodeIndex;
        FirstFreeNodeIndex = MsgQueNodePool[nodeIndex]._NextNodeIndex;
        MsgQueNodePool[nodeIndex]._NextNodeIndex = INVALID_NODE_INDEX;

        FreeMsgQueueNodeNum --;
    }
    else
    {
        FreeQueueNodeRunOutCount ++;
    }    
    
    return nodeIndex;
}

void CMsgQueue::ReturnFreeMsgNode(UINT16 nodeIndex)
{
    MsgQueNodePool[nodeIndex]._NextNodeIndex = FirstFreeNodeIndex;
    FirstFreeNodeIndex = nodeIndex;
    FreeMsgQueueNodeNum ++;
}

CMsgQueue::CMsgQueue(SINT32 iMaxMsgs, SINT32 option)
{
    m_sQueueHeadNodeIndex = INVALID_NODE_INDEX;
    m_sQueueTailNodeIndex = INVALID_NODE_INDEX;
    m_sMaxMsgs = iMaxMsgs;
    m_sCurrentMsgs = 0;
    m_uMaxQueuedMsgs = 0;

    MsgQueueCreatedNum ++;

    memset((void*)&m_QueuePendSem, 0, sizeof(NU_SEMAPHORE));
    if ( NU_SUCCESS != NU_Create_Semaphore(&m_QueuePendSem, "QueueActSem", 0, NU_FIFO))
    {
        LOG(LOG_CRITICAL, FMK_ERROR_CREATE_MSG_QUEUE_FAIL, "Queue Action semaphore Create Failed");
    }
}


bool CMsgQueue::PostMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent)
{
/*delete by huangjl
    ApAssertRtnV( (NULL!=this), LOG_CRITICAL, FMK_ERROR_INVALID_OBJECT, "Post Message to NULL QUEUE", ;, false);
//<==

    ApAssertRtnV( (NULL!=pMsg), LOG_CRITICAL, FMK_ERROR_INVALID_OBJECT, "Post NULL Message ", ;, false);
//<==
    ApAssertRtnV( (pMsg->AddRef()), LOG_CRITICAL, FMK_ERROR_POST_MSG_FAIL, "AddRef in Post Message Failed", ;, false);
//<==
*/
    if ( true == PostMessage((UINT32)pMsg, timeout, isUrgent))
    {
        return true;
    }
    else
    {
        if (!pMsg->Release())
            LOG(LOG_SEVERE, FMK_ERROR_POST_MSG_FAIL, "Release ComMessage Reference failed.");
        return false;
    }
}


bool CMsgQueue::PostMessage(UINT32 uMsg, SINT32 timeout, bool isUrgent)
{

    if ( m_sCurrentMsgs >= m_sMaxMsgs)
    {  // will not put more messages than maximum
        return false;
    }

    UINT16 oldLevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    // get a message node from the pool
    UINT16 nodeIndex = GetFreeMsgNode();
    if ( INVALID_NODE_INDEX == nodeIndex )
    {
        ::NU_Control_Interrupts(oldLevel);	//restore CPSR(restore global interrput)
        return false;
    }

    if ( INVALID_NODE_INDEX == m_sQueueHeadNodeIndex )
    {  // queue is empty
        m_sQueueHeadNodeIndex = nodeIndex;
        m_sQueueTailNodeIndex = nodeIndex;
    }
    else 
    {
        if (isUrgent)
        {  // insert the urgent message to the front of current head node
            MsgQueNodePool[nodeIndex]._NextNodeIndex = m_sQueueHeadNodeIndex;
            m_sQueueHeadNodeIndex = nodeIndex;
        }
        else
        {  // append the new node to the tail of the queue
            MsgQueNodePool[m_sQueueTailNodeIndex]._NextNodeIndex = nodeIndex;
            m_sQueueTailNodeIndex = nodeIndex;
        }
    }

    MsgQueNodePool[nodeIndex]._Value = uMsg;
    m_sCurrentMsgs ++;
    if (m_sCurrentMsgs > m_uMaxQueuedMsgs)
    {
        m_uMaxQueuedMsgs = m_sCurrentMsgs;
    }

    ::NU_Control_Interrupts(oldLevel);	//restore CPSR(restore global interrput)

    if ( NU_SUCCESS != NU_Release_Semaphore(&m_QueuePendSem))
    {   // post semaphore for queued messages
        LOG(LOG_SEVERE, FMK_ERROR_POST_MSG_FAIL, "m_uMaxQueuedMsgs failed");
        return false;
    }

    return true;
}

CComMessage* CMsgQueue::GetMessage(SINT32 timeout)
{
    UINT32 value;

    if ( false == GetMessage(&value, timeout))
    {
        return NULL;
    }

    return (CComMessage*)value;
}


bool CMsgQueue::GetMessage(UINT32 *pMsg, SINT32 timeout)
{
   // delete by huangjl

 //   ApAssertRtnV( (NULL!=this), LOG_CRITICAL, FMK_ERROR_INVALID_OBJECT, "Post Message to NULL QUEUE", ;, false);
//<==

    if ( NU_SUCCESS != NU_Obtain_Semaphore(&m_QueuePendSem, timeout))
    {
        LOG(LOG_SEVERE, FMK_ERROR_GET_MSG_FAIL, "Wait for m_uMaxQueuedMsgs timeout.");
        return false;
    }

    UINT16 oldLevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    UINT16 nodeIndex = m_sQueueHeadNodeIndex;

    if (MSG_QUEUE_POOL_NODE_NUM <= nodeIndex)
    {
        LOG(LOG_SEVERE, FMK_ERROR_POST_MSG_FAIL, "Queue GetMsg return invalid Node Index");
        ::NU_Control_Interrupts(oldLevel);	//restore CPSR(restore global interrput)
        return false;
    }

    *pMsg = MsgQueNodePool[m_sQueueHeadNodeIndex]._Value;
    m_sQueueHeadNodeIndex = MsgQueNodePool[m_sQueueHeadNodeIndex]._NextNodeIndex;

    ReturnFreeMsgNode(nodeIndex);
    m_sCurrentMsgs --;
    
    ::NU_Control_Interrupts(oldLevel);	//restore CPSR(restore global interrput)

    return true;
}

SINT32 CMsgQueue::GetCount(void) const
{
    // delete by huangjl

 //   ApAssertRtnV( (NULL!=this), LOG_CRITICAL, FMK_ERROR_INVALID_OBJECT, "Post Message to NULL QUEUE", ;, 0);
//<==
    return m_sCurrentMsgs;
}

bool CMsgQueue::PeekFirstMessage(UINT32 *msg)
{
    if (m_sCurrentMsgs)
    {
        if ( MSG_QUEUE_POOL_NODE_NUM > m_sQueueHeadNodeIndex )
        {
            *msg =  MsgQueNodePool[m_sQueueHeadNodeIndex]._Value;
            return true;
        }
    }

    return false;
}
