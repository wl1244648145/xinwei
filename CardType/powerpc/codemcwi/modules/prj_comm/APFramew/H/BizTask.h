#ifndef _INC_BIZTASK
#define _INC_BIZTASK

#ifndef _INC_TASK
#include "Task.h"
#endif

class CMsgQueue;

#ifndef __NUCLEUS__	// nucleus not use transaction
class CTransaction;
class CTransactionManager;
#endif

class CBizTask : public CTask
{
private:
#ifndef __NUCLEUS__
    CTransactionManager* m_pTransactManager;
#endif
protected:
	SINT32 m_iMsgQMax;
    SINT32 m_iMsgQOption;
    CMsgQueue* m_pMsgQueue;

public:
    CBizTask();
    //virtual ~CBizTask();
	virtual bool PostMessage(CComMessage*,SINT32, bool isUrgent=false);

#ifndef __NUCLEUS__    
    CTransaction* CreateTransact(CMessage& MsgRequest,
                                 CMessage& MsgTimeoutResp,
                                 SINT32 uCount,
                                 SINT32 uTimeout);
    CTransaction* FindTransact(UINT16 uId);
#endif

#ifndef NDEBUG
    virtual bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
#endif

protected:
    bool CreateMsgQueue();

#ifndef __NUCLEUS__
    virtual bool IsNeedTransaction() const;
#endif

    virtual bool Initialize();
    virtual CComMessage* GetMessage();
    virtual SINT32 GetMsgCount();
    virtual void MainLoop();
#ifndef __NUCLEUS__
    virtual bool ProcessComMessage(CComMessage* pComMsg);
    virtual bool IsMonitoredForDeadlock() { return true;};
#endif
private:
    CBizTask(CBizTask&);
};

#endif
