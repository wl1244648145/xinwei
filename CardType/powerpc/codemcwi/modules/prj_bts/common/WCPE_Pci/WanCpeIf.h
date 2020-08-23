#include "taskDef.h"
#include "BizTask.h"
#include "ComMessage.h"

#define M_EGRESS_Q_SIZE     (1000)
#define M_BUFFERED_INGRESS_MSG_NUM (2000)

#define M_CREATOR_DRV               (0xFFFFFFFF)

class WanCpeIF:public CBizTask
{
public:
    virtual TID GetEntityId() const ; 
    virtual bool IsNeedTransaction() const { return false;};
    static WanCpeIF *GetInstance();
    bool ProcessComMessage(CComMessage* );
    bool DeallocateComMessage(CComMessage*);

    static void RxDriverPacketCallBack(char *, UINT16, char *);
    static void WanIfFreeMsgCallBack (UINT32 param);

    CComMessage* GetComMessage();

    #define WANCPEIF_MAX_BLOCKED_TIME_IN_10ms_TICK 100
	bool IsMonitoredForDeadlock()  { return true; };
	int  GetMaxBlockedTime() { return WANCPEIF_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    void ShowStatus();

    ~WanCpeIF(){};

private:
    WanCpeIF();
    bool Initialize();

    bool   SendToWAN(CComMessage*);
    bool   InitComMessageList();

    CComMessage *m_plistComMessage;
    UINT32 MsgCountFromWANToWCPE;
    UINT32 MsgCountFromWCPEToWAN;

    static WanCpeIF *instance;
};
