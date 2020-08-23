#ifndef _INC_TRANSACTION
#define _INC_TRANSACTION

#ifndef _INC_MUTEX
#include "Mutex.h"
#endif

#ifndef _INC_OBJECT
#include "Object.h"
#endif

#ifndef _INC_COMENTITY
#include "ComEntity.h"
#endif

class CComMessage;
class CMessage;
class CTimer;
class CTransactionManager;

class CTransaction
#ifndef NDEBUG
:public CObject
#endif
{
private:
    static CMutex* s_pmutexTransId;
    static UINT16 s_uNextTransId;
    static bool s_bClassInited;

    CComMessage* m_pMsgRequest;
    UINT16 m_uRequestTransId;
    CComMessage* m_pMsgTimeoutResp;
    CComMessage* m_pMsgTimeout;
    CTimer* m_pTimer;
    SINT32 m_iTimeout;

    UINT16 m_uId;
    SINT32 m_iCount;

    CTransactionManager* m_pManager;

public:
    static bool InitTransactionClass();

    CTransaction(CMessage& MsgRequest,
                 CMessage& MsgTimeout,
                 CMessage& MsgTimeoutResp,
                 SINT32 iCount,
                 SINT32 iTimeout);
    UINT16 GetId() const;
    CMessage GetRequestMessage() const;
    UINT16 GetRequestTransId() const;
    bool BeginTransact();
    bool ReTransmit();
    bool EndTransact();

#ifndef NDEBUG
    virtual bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
#endif

    ~CTransaction();

    friend class CTransactionManager;
};
#endif
