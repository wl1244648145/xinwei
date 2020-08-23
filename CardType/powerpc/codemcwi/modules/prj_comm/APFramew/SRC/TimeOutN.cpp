#ifndef _INC_TIMEOUTNOTIFY
#include "TimeOutNotify.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

CTimeOutNotify :: CTimeOutNotify(CMessage &rMsg)    
:CMessage(rMsg)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTimeOutNotify::CTimeOutNotify(CMessage&)");
#endif
}

CTimeOutNotify :: CTimeOutNotify()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTimeOutNotify::CTimeOutNotify()");
#endif
}

UINT16 CTimeOutNotify::SetTransactionId(UINT16 TransId)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTimeOutNotify::SetTransId");
#endif
    if (!ASSERT_VALID(this))
        return (~TransId);

    return ((T_TimerContent *)GetDataPtr())->m_TransId = TransId;
}

UINT16 CTimeOutNotify :: GetTransactionId()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTimeOutNotify::GetTransId");
#endif
    if (!ASSERT_VALID(this))
        return 0;

    return ((T_TimerContent *)GetDataPtr())->m_TransId;
}

CTimeOutNotify :: ~CTimeOutNotify()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTimeOutNotify::~CTimeOutNotify");
#endif

}

UINT32 CTimeOutNotify::GetDefaultDataLen() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTimeOutNotify::GetDefaultDataLen");
#endif
    return sizeof(struct T_TimerContent);
}

UINT16 CTimeOutNotify::GetDefaultMsgId() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTimeOutNotify::GetDefaultMsgId");
#endif
    return M_MSG_TIMEOUT_NOTIFY;
}

