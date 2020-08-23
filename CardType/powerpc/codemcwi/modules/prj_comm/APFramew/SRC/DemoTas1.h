#ifndef _INC_DEMOTASK2
#define _INC_DEMOTASK2
#endif

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#include<string.h>
#include <unistd.h>

class CDemoTask2:public CBizTask
{
public:
    CDemoTask2();
    virtual TID GetEntityId() const;
    virtual bool PostMessage(CComMessage*, SINT32);
    #define MAX_BLOCKED_TIME_IN_10ms_TICK (500)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return MAX_BLOCKED_TIME_IN_10ms_TICK ;}
    virtual bool DeallocateComMessage(CComMessage* pComMsg);
	
private:
    virtual bool Initialize();
    virtual void MainLoop();
    virtual bool ProcessComMessage(CComMessage*);
};
