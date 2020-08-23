#ifndef _INC_TIMEOUTNOTIFY
#define _INC_TIMEOUTNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

//�����ĳ�ʱ������Ϣ��Ӧ������Ҫ��ʱ�ش�����Ϣ����
const UINT16 M_MSG_TIMEOUT_NOTIFY              = 0XF000;

class CTimeOutNotify : public CMessage
{
public: 
    CTimeOutNotify(CMessage &rMsg);
    CTimeOutNotify();
    ~CTimeOutNotify();

    UINT16 SetTransactionId(UINT16 TransId);//These four function are virtual
    UINT16 GetTransactionId();
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;

private:
    struct T_TimerContent
    {
        UINT16 m_TransId;	
    };
};

#endif
