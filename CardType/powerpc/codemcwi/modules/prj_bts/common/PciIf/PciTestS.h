#include "taskDef.h"
#include "BizTask.h"
#include "ComMessage.h"

class PciTestStub:public CBizTask
{
public:
    virtual TID GetEntityId() const ; 
    virtual bool IsNeedTransaction() const { return false;};
    ~PciTestStub(){};
    static PciTestStub *GetInstance();
    bool ProcessMessage(CMessage&);
    void SendPciTestMsg(UINT32 len);

private:
    PciTestStub();
    static PciTestStub *instance;
};
