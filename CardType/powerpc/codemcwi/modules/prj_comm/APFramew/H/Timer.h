#ifndef _INC_TIMER
#define _INC_TIMER

#ifndef DATATYPE_H
#include "datatype.h"
#endif

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

/*******************************************
*BTS定时器很长(数天)时导致UINT32溢出，必须
*使用更大的数据类型，但是CPE上继续使用UINT32
*/
#ifdef __WIN32_SIM__
typedef unsigned __int64        TIMERINTERVALTYPE;
#elif __NUCLEUS__
typedef unsigned int            TIMERINTERVALTYPE;
#else //Vxworks
typedef unsigned long long int  TIMERINTERVALTYPE;
#endif


class CTimer 
#ifndef NDEBUG
:public CObject
#endif
{
    friend class CTimerTask;

public:
    void* operator new(size_t size);
    void operator delete(void* p);

    CTimer(bool bPeriodic, UINT32 uInterval, const CMessage& Msg);
    CTimer(bool bPeriodic, UINT32 uInterval, CComMessage* pComMsg);

    void init(UINT32);

    bool Start();
    bool Stop();

    bool SetInterval(UINT32);

    UINT32 GetExpireCount() const;
    ~CTimer();
	
#ifdef M_MONITOR_RESOURCE
	TID m_releatedTID;
#endif

private:
    UINT32 m_uRemaining;
    UINT32 m_uInterval;
    CComMessage* m_pComMsg;
    bool m_bActive;
    bool m_bPeriodic;
    UINT16 m_uListArrayIndex;
    UINT32 m_uExpireCount;

    UINT16 m_tidDst; //for audit m_pComMsg liuweidong
    UINT16 m_tidSrc;
    UINT16 m_uMsgId;
	
};

#endif
