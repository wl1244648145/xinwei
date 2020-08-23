#ifndef _INC_DEMOTASK1
#define _INC_DEMOTASK1
#endif

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#include<string.h>
#include<unistd.h>

class CDemoTask1:public CBizTask
{
private:
    TID m_tid;
public:
    CDemoTask1();
    virtual TID GetEntityId() const;
    virtual bool PostMessage(CComMessage*, SINT32);
	
    #define MAX_BLOCKED_TIME_IN_10ms_TICK1 (500)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return MAX_BLOCKED_TIME_IN_10ms_TICK1 ;}
 //   virtual void MainLoop(){}
	virtual bool DeallocateComMessage(CComMessage* pComMsg);
private:
    virtual bool Initialize();
    virtual bool ProcessComMessage(CComMessage*);

};
