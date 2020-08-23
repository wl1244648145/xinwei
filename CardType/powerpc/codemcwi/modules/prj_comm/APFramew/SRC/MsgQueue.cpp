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
#ifndef _INC_MSGQUEUE
#include "MsgQueue.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifdef __WIN32_SIM__

#ifndef _INC_TYPEINFO
#include <typeinfo.h>
#endif

#elif __NUCLEUS__

#include <string.h>
extern NU_MEMORY_POOL System_Memory;

#else

#ifndef __TYPEINFO__
#include <typeinfo>
using namespace std;
#endif
//#include "taskLib.h"//delete by huangjl
#include "Vxw_hdrs.h" //add by huangjl

#endif

CMsgQueue::CMsgQueue(SINT32 iMaxMsgs, SINT32 option)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMsgQueue::CMsgQueue");
#endif

#ifdef __WIN32_SIM__
	m_plstMsg = new CPtrList(iMaxMsgs, 0);
	if (m_plstMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Create PtrList failed.");
		return;
	}

    if ( (m_hMutex=::CreateMutex(NULL,true,NULL))==NULL)
    {
        LOG(LOG_CRITICAL,0,"Protect construction failed.");
        return;
    }

    if ( (m_hEvent = ::CreateEvent(NULL, true, false, NULL)) == NULL)
    {
        LOG(LOG_CRITICAL,0,"CreateEvent failed.");
        return;
    }

    if (::ReleaseMutex(m_hMutex)==0)
    {
        LOG(LOG_CRITICAL,0,"Release m_hMutexGet failed.");
        if (!::CloseHandle(m_hEvent))
            LOG(LOG_CRITICAL,0,"Close m_hEvnet failed.");
        if (!::CloseHandle(m_hMutex))
            LOG(LOG_CRITICAL,0,"Close m_hMutexGet failed.");
        return;
    }
#elif __NUCLEUS__
    m_mem_queue = NULL;
	if ( (m_mem_queue = (void*)new UINT8[iMaxMsgs*sizeof(CComMessage*)]) == NULL )
    {
        LOG(LOG_SEVERE,0,"NU_Allocate_Memory for CMsgQueue failed.");
        return;
    }
    ::memset(&m_queue, 0, sizeof(UNSIGNED)*NU_QUEUE_SIZE);
    if(NU_SUCCESS!=NU_Create_Queue(&m_queue,
                                   "",
                                   m_mem_queue, 
                                   iMaxMsgs*sizeof(CComMessage*)/sizeof(UNSIGNED),
                                   NU_FIXED_SIZE,
                                   sizeof(CComMessage*)/sizeof(UNSIGNED),
                                   option))
    {
        LOG(LOG_SEVERE,0,"ERROR!!!CMsgQueue::CMsgQueue% NU_Create_Queue failed.");
        delete [] m_mem_queue;
		// NU_Deallocate_Memory(m_mem_queue);
        m_mem_queue = NULL;
        return;
    }
#else //VxWorks
    if ( (m_Id = ::msgQCreate(iMaxMsgs, sizeof(CComMessage*), option))==NULL)
    {
        LOG(LOG_SEVERE,0,"ERROR!!!CMsgQueue::CMsgQueue% msgQCreate failed.");
        return;
    }
#endif

#ifndef NDEBUG
    if (!Construct(CObject::M_OID_MSGQUEUE))
        LOG(LOG_SEVERE,0,"ERROR!!!CMsgQueue::CMsgQueue% Construct failed.");
#endif
}

CMsgQueue::~CMsgQueue()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMsgQueue::~CMsgQueue");

    if (!ASSERT_VALID(this))
        return;
#endif

#ifdef __WIN32_SIM__
    if (::WaitForSingleObject(m_hMutex,WAIT_FOREVER)!=WAIT_OBJECT_0)
    {
        LOG(LOG_CRITICAL,0,"Protect Destruction failed.");
        return;
    }

    while (!m_plstMsg->empty())
    {
        CComMessage* pComMsg = (CComMessage*)m_plstMsg->front();
        pComMsg->Destroy();
    }
    delete m_plstMsg;

    if (!::CloseHandle(m_hEvent))
    {
        LOG(LOG_SEVERE,0,"Destroy m_hEvent failed.");
    }
    m_hEvent = NULL;

    if (!::CloseHandle(m_hMutex))
    {
        LOG(LOG_CRITICAL,0,"Close m_hMutex failed.");
    }
    m_hMutex = NULL;
	
#elif __NUCLEUS__
    if(NU_SUCCESS!=NU_Delete_Queue(&m_queue))
    {
        LOG(LOG_SEVERE,0,"NU_Delete_Queue failed.");
    }
    if(m_mem_queue!=NULL)
    {
		delete [] m_mem_queue;
        //NU_Deallocate_Memory(m_mem_queue);
    }
#else //VxWorks
    if (::msgQDelete(m_Id)==ERROR)
    {
        LOG(LOG_SEVERE,0,"msgQDelete failed.");
        return;
    }
#endif

#ifndef NDEBUG
    if (!Destruct(CObject::M_OID_MSGQUEUE))
        LOG(LOG_SEVERE,0,"Destruct failed.");
#endif
}

#ifndef __NUCLEUS__
bool CMsgQueue::PostMessage(const CComMessage* pMsg, SINT32 timeout, bool isUrgent)
#else
bool CMsgQueue::PostMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent)
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMsgQueue::PostMessage");

    if (!ASSERT_VALID(this))
        return false;

    if (!ASSERT_VALID(pMsg))
        return false;
#endif

    if (!pMsg->AddRef())
    {
        LOG(LOG_CRITICAL,0,"Add Reference failed.");
        return false;
    }

#ifdef __WIN32_SIM__
    if (::WaitForSingleObject(m_hMutex,timeout)!=WAIT_OBJECT_0)
    {
        LOG(LOG_SEVERE,0,"Wait for m_hMutex timeout.");
        if (!pMsg->Release())
            LOG(LOG_CRITICAL,0,"Release ComMessage Reference failed.");
        return false;
    }
    if (isUrgent)
        m_plstMsg->push_front((void*)pMsg);
    else
        m_plstMsg->push_back((void*)pMsg);
    if (m_plstMsg->size()==1)
    {
        if (!::SetEvent(m_hEvent))
        {
            LOG(LOG_SEVERE,0,"SetEvent failed.");
            return false;
        }
    }
    if (::ReleaseMutex(m_hMutex)==0)
    {
        LOG(LOG_SEVERE,0,"ReleaseMutex failed.");
        return false;
    }
    return true;
#elif __NUCLEUS__
    if( !isUrgent)
    {
        if(NU_SUCCESS==NU_Send_To_Queue(&m_queue, 
                                        (VOID*)&pMsg, 
                                        sizeof(CComMessage*)/sizeof(UNSIGNED),
                                        timeout))
            return true;
        else
        {
            if (!pMsg->Release())
                LOG(LOG_CRITICAL,0,"Release ComMessage Reference failed.");
            return false;
        }
    }
    else
    {
        if(NU_SUCCESS==NU_Send_To_Front_Of_Queue(&m_queue,
                                                 (VOID*)&pMsg,
                                                 sizeof(CComMessage*)/sizeof(UNSIGNED),
                                                 timeout))
            return true;
        else
        {
            if (!pMsg->Release())
                LOG(LOG_CRITICAL,0,"Release ComMessage Reference failed.");
            return false;
        }
    }
#else //VxWorks
    if (::msgQSend(m_Id,(char*)&pMsg,sizeof(CComMessage*),timeout, isUrgent)==OK)
        return true;
    else
    {
        if (!pMsg->Release())
            LOG(LOG_CRITICAL,0,"Release ComMessage Reference failed.");
        return false;
    }
#endif
}

CComMessage* CMsgQueue::GetMessage(SINT32 timeout)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMsgQueue::GetMessage");
#endif

    const CComMessage* pMsg = NULL;

#ifndef NDEBUG
    if (!ASSERT_VALID(this))
        return NULL;
#endif

#ifdef __WIN32_SIM__
    if (::WaitForSingleObject(m_hMutex,timeout)!=WAIT_OBJECT_0)
    {
        LOG(LOG_SEVERE,0,"Wait for m_hMutex timeout.");
        return NULL;
    }

    if (m_plstMsg->empty())
    {
        if (::ReleaseMutex(m_hMutex)==0)
        {
            LOG(LOG_CRITICAL,0,"Release m_hMutex failed.");
            return NULL;
        }

        if (::WaitForSingleObject(m_hEvent, timeout)!=WAIT_OBJECT_0)
        {
            LOG(LOG_SEVERE,0,"Wait for event failed.");
            return NULL;
        }

        if (::WaitForSingleObject(m_hMutex,timeout)!=WAIT_OBJECT_0)
        {
            LOG(LOG_CRITICAL,0,"Wait for m_hMutex failed.");
            return NULL;
        }
    }

    if (!::ResetEvent(m_hEvent))
    {
        LOG(LOG_CRITICAL,0,"Reset m_hEvent failed.");
        return NULL;
    }

    pMsg = (const CComMessage*)m_plstMsg->front();
    m_plstMsg->pop_front();

    if (::ReleaseMutex(m_hMutex)==0)
    {
        LOG(LOG_SEVERE,0,"ReleaseMutex failed.");
        return (CComMessage*)pMsg;
    }
#elif __NUCLEUS__
    UNSIGNED rcv_size;
    if(NU_SUCCESS==NU_Receive_From_Queue(&m_queue,
                                         (VOID*)&pMsg, 
                                         sizeof(CComMessage*)/sizeof(UNSIGNED),
                                         &rcv_size,
                                         timeout))
    {
        return (rcv_size==sizeof(CComMessage*)/sizeof(UNSIGNED)) ? (CComMessage*)pMsg : NULL;
    }
    else
        return NULL;
#else //VxWorks
    int rc = ::msgQReceive(m_Id,(char*)&pMsg,sizeof(CComMessage*),timeout);

    if (sizeof(CComMessage*)== rc)
        return (CComMessage*)pMsg;
    else
        return NULL;
#endif
}

SINT32 CMsgQueue::GetCount(void) const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMsgQueue::GetCount");

    if (!ASSERT_VALID(this))
        return -1;
#endif

#ifdef __WIN32_SIM__
    return m_plstMsg->size();
#elif __NUCLEUS__
    CHAR qname[10];
    VOID *start_addr;
    UNSIGNED qsize;
    UNSIGNED qavail;
    UNSIGNED msgnum;
    OPTION msgtype;
    UNSIGNED msgsize;
    OPTION suspendtype;
    UNSIGNED taskwaiting;
    NU_TASK *firsttask;
    if(NU_SUCCESS==NU_Queue_Information((NU_QUEUE *)&m_queue,
                                        qname,
                                        &start_addr,
                                        &qsize,
                                        &qavail,
                                        &msgnum,
                                        &msgtype,
                                        &msgsize,
                                        &suspendtype,
                                        &taskwaiting,
                                        &firsttask))
    {
        return msgnum;
    }
    else
        return -1;
#else //VxWorks
    SINT32 count;
    if ( (count=::msgQNumMsgs(m_Id)) != ERROR )
        return count;
#endif

}

#ifndef NDEBUG
bool CMsgQueue::AssertValid(const char* lpszFileName, UINT32 nLine) const
{
#ifdef __WIN32_SIM__
    if (::IsBadReadPtr(this,sizeof(CMsgQueue)) || ::IsBadWritePtr((void*)this,sizeof(CMsgQueue)) )
    {
        CLog::LogAdd(lpszFileName,nLine, M_LL_CRITICAL,0,"Invalid CMsgQueue pointer.");
        return false;
    }
    if (m_plstMsg==NULL)
    {
        CLog::LogAdd(lpszFileName, nLine, M_LL_CRITICAL, 0, "m_plstMsg==NULL.");
        return false;
    }
    if (m_hMutex==NULL)
    {
        CLog::LogAdd(lpszFileName, nLine, M_LL_CRITICAL,0,"m_hMutex==NULL.");
        return false;
    }
    if (m_hEvent==NULL)
    {
        CLog::LogAdd(lpszFileName, nLine, M_LL_CRITICAL,0,"m_hEvent==NULL.");
    }
#elif __NUCLEUS__
    if(m_mem_queue==NULL)
    {
        CLog::LogAdd(lpszFileName,nLine, M_LL_CRITICAL,0,"m_mem_queue==NULL.");
        return false;
    }
#else //VxWorks
    if (m_Id==NULL)
    {
        LOG(LOG_CRITICAL,0,"m_Id==NULL.");
        return false;
    }
#endif

#ifndef __NUCLEUS__
    if (typeid(*this) != typeid(CMsgQueue))
    {
        LOG(LOG_CRITICAL,0,"Invalid typeid.");
        return false;
    }
#endif

    return true;
}
#endif
