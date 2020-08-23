#include "taskDef.h"
#include "BizTask.h"
#include "ComMessage.h"

class CsiTest1:public CBizTask
{
public:
    virtual TID GetEntityId() const ; 
    virtual bool IsNeedTransaction() const { return false;};
    ~CsiTest1(){};
    static CsiTest1 *GetInstance();
    bool ProcessComMessage(CComMessage*);
    void SendCsiTestMsg(UINT32 value);

private:
    CsiTest1();
    static CsiTest1 *instance;
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return 200;};

};




class CsiTest2:public CBizTask
{
public:
    virtual TID GetEntityId() const ; 
    virtual bool IsNeedTransaction() const { return false;};
    ~CsiTest2(){};
    static CsiTest2 *GetInstance();
    bool ProcessComMessage(CComMessage*);
    void SendCsiTestMsg(UINT32 value);

private:
    CsiTest2();
    static CsiTest2 *instance;
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return 200;};
};
