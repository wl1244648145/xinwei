#ifndef _INC_MSGQUEUE
#define _INC_MSGQUEUE

#if (defined __NUCLEUS__) && (defined AP_MSG_QUEUE)
#include "ApMsgQueue.h"
#else

#ifndef _INC_OBJECT
#include "Object.h"
#endif

#ifdef __WIN32_SIM__
#include <windows.h>

#ifndef _INC_PTRLIST
#include "PtrList.h"
#endif

#elif __NUCLEUS__

#ifndef NUCLEUS
#include "NUCLEUS.h"
#endif

#else

#ifndef __INCmsgQLibh
#include "vxwk2pthread.h" //add by huangjl
//#include <msgQLib.h>//delete by huangjl
#endif

#endif

class CComMessage;

class CMsgQueue 
#ifndef NDEBUG
: public CObject
#endif
{
//Attributes
private:
#ifdef __WIN32_SIM__
    HANDLE m_hMutex;
    CPtrList* m_plstMsg;
    HANDLE m_hEvent;
#elif __NUCLEUS__
    NU_QUEUE m_queue;
    VOID *m_mem_queue;
#else
    MSG_Q_ID m_Id;
#endif

//Operations
public:
    CMsgQueue(SINT32 iMaxMsgs, SINT32 option);
    ~CMsgQueue();
#ifndef __NUCLEUS__
    bool PostMessage(const CComMessage* pMsg, SINT32 timeout, bool isUrgent=false);
#else
    bool PostMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent=false);
#endif
    CComMessage* GetMessage(SINT32 timeout);
    SINT32 GetCount(void) const;

#ifndef NDEBUG
    virtual bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
#endif

private:
    CMsgQueue(CMsgQueue&);
};

#endif  // defined __NUCLEUS__ && defined AP_MSG_QUEUE

#endif  //_INC_MSGQUEUE
