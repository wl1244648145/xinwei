#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_TRANSACTIONMANAGER
#include "TransactionManager.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_TIMEOUTNOTIFY
#include "TimeOutNotify.h"
#endif

#ifdef __WIN32_SIM__

#ifndef _INC_TYPEINFO
#include <typeinfo.h>
#endif

#else

#ifndef __TYPEINFO__
#include <typeinfo>
using namespace std;
#endif

#endif

CMutex* CTransaction::s_pmutexTransId = NULL;
UINT16 CTransaction::s_uNextTransId = 0x0000;
bool CTransaction::s_bClassInited = false;

bool CTransaction::InitTransactionClass()
{
    if (s_bClassInited)
        return true;
    s_pmutexTransId = new CMutex;
    if (s_pmutexTransId==NULL)
        return false;
    s_uNextTransId = 0x0000;
    s_bClassInited = true;
    return true;
}

CTransaction::CTransaction(CMessage& MsgRequest,
                           CMessage& MsgTimeout,
                           CMessage& MsgTimeoutResp,
                           SINT32 iCount,
                           SINT32 iTimeout)
:m_pMsgRequest(MsgRequest.m_pMsg),
m_uRequestTransId(MsgRequest.GetTransactionId()),
m_pMsgTimeout(MsgTimeout.m_pMsg),
m_pMsgTimeoutResp(MsgTimeoutResp.m_pMsg),
m_iTimeout(iTimeout),
m_uId(0),
m_iCount(iCount),
m_pManager(NULL),
m_pTimer(NULL)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransaction::CTransaction");
#endif

    if (!s_pmutexTransId->Wait())
        LOG(LOG_SEVERE,0,"mutex Wait failed.");
    m_uId = s_uNextTransId++;
    if (!s_pmutexTransId->Release())
        LOG(LOG_SEVERE,0,"mutex Release failed.");
    
    m_uId |= 0x8000;

    if (m_pMsgRequest!=NULL)
        m_pMsgRequest->AddRef();
    if (m_pMsgTimeout!=NULL)
        m_pMsgTimeout->AddRef();
    if (m_pMsgTimeoutResp!=NULL)
        m_pMsgTimeoutResp->AddRef();

    MsgRequest.SetTransactionId(m_uId);
    MsgTimeoutResp.SetTransactionId(m_uRequestTransId);
#ifndef NDEBUG
    if (!Construct(CObject::M_OID_TRANSACTION))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif
}

UINT16 CTransaction::GetId() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransaction::GetId");
#endif

    if (!ASSERT_VALID(this))
        return 0;

    return m_uId;
}

CMessage CTransaction::GetRequestMessage() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransaction::GetRequestMessage");
#endif

    if (!ASSERT_VALID(this))
        return CMessage((CComMessage*)NULL);

    return CMessage(m_pMsgRequest);
}

UINT16 CTransaction::GetRequestTransId() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransaction::GetRequestTransId");
#endif

    if (!ASSERT_VALID(this))
        return 0;

    return m_uRequestTransId;
}

bool CTransaction::BeginTransact()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransaction::BeginTransact");
#endif

    if (!ASSERT_VALID(this))
        return false;

    if (!ReTransmit())
    {
        LOG(LOG_SEVERE,0,"ReTransmit failed.");
        return false;
    }

    if ((m_pTimer = new CTimer(1,m_iTimeout,CMessage(m_pMsgTimeout)))!=NULL)
    {
        m_pTimer->Start();
        return true;
    }
    return false;
}

bool CTransaction::ReTransmit()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransaction::ReTransmit");
#endif

    if (!ASSERT_VALID(this))
        return false;

    if (m_iCount>0)
    {
        m_iCount--;
        return CComEntity::PostEntityMessage(m_pMsgRequest);
    }
    else
    {
        //判断是否重复进入这个判断,如果重复了,就删除消息
        if(m_iCount < 0)
        {            
	     return false;//框架会删除的
        }
        if (m_pMsgTimeoutResp!=NULL)
        {
           m_iCount -= 1;//加重入标志jiaying20100930
           return CComEntity::PostEntityMessage(m_pMsgTimeoutResp);//超时后由任务自己释放内存jiaying20100817
        }
        return false;
    }
}

bool CTransaction::EndTransact()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransaction::EndTransact");
#endif

    if (!ASSERT_VALID(this))
        return false;

    if ((m_pTimer!=NULL)||(m_pTimer!=(void*)0xffffffff))
    {
        m_pTimer->Stop();
        delete m_pTimer;
        m_pTimer = NULL;
    }

    if (m_pManager==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pManager==NULL.");
        return false;
    }
    
    if (!m_pManager->Remove(this))
    {
        LOG(LOG_SEVERE,0,"Remove from manager failed.");
        return false;
    }
    return true;
}

#ifndef NDEBUG
bool CTransaction::AssertValid(const char* lpszFileName, UINT32 nLine) const
{
#ifdef __WIN32_SIM__
    if (::IsBadReadPtr(this,sizeof(CTransaction)) || ::IsBadWritePtr((void*)this,sizeof(CTransaction)) )
    {
        CLog::LogAdd(lpszFileName,nLine,M_LL_CRITICAL,0,"Invalid CTransaction pointer.");
        return false;
    }
#endif
    if (typeid(*this) != typeid(CTransaction))
    {
        LOG(LOG_CRITICAL,0,"Invalid typeid.");
        return false;
    }
    return true;
}
#endif

CTransaction::~CTransaction()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransaction::~CTransaction");
#endif

    if (m_pMsgRequest!=NULL)
    	{
        m_pMsgRequest->Destroy();
    	}
    if (m_pMsgTimeout!=NULL)
    	{
        m_pMsgTimeout->Destroy();
    	}
    if (m_pMsgTimeoutResp!=NULL)
    	{
        m_pMsgTimeoutResp->Destroy();
    	}
#ifndef NDEBUG
    if (!Destruct(CObject::M_OID_TRANSACTION))
        LOG(LOG_SEVERE,0,"Destruct failed.");
#endif
}

