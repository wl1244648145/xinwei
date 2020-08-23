#ifndef _INC_OBJECT
#define _INC_OBJECT

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

#ifndef _INC_MUTEX
#include "Mutex.h"
#endif
#include <stdio.h>
class CObject
{
protected:
    typedef enum {
        M_OID_OBJECT,
        M_OID_COMMESSAGE,
        M_OID_MSGQUEUE,
        M_OID_COMENTITY,
        M_OID_CCBBASE,
        M_OID_CCBTABLE,
        M_OID_FSMSTATE,
        M_OID_FSMTRANSITION,
        M_OID_FSM,
        M_OID_TASK,
        //all derived pure tasks
        M_OID_EMSAGENTRX,
        M_OID_L2AGENTRX,
        M_OID_BIZTASK,
        //all derived business tasks
        M_OID_EMSAGENTTX,
        M_OID_L2AGENTTX,
        M_OID_CM,
        M_OID_FM,
        M_OID_PM,
        M_OID_UM,
        M_OID_SM,
        M_OID_BM,
        M_OID_GM,
        M_OID_SYS,
        M_OID_FTPCLIENT,
        M_OID_LOG,
        M_OID_EB,
        M_OID_SNOOP,
        M_OID_TUNNEL,
        M_OID_ARP,
        M_OID_DM,
        M_OID_TCR,
        M_OID_TDR,
        M_OID_VOICE,
        M_OID_VCR,
        M_OID_VDR,
        M_OID_VSCAN,
        M_OID_UTV,
        M_OID_UTDM,
        M_OID_TIMER,
        M_OID_TRANSACTION,
        M_OID_TRANSACTIONMANAGER,
        M_OID_MESSAGE,
#ifdef WBBU_CODE
        M_OID_L2IF,
#endif
        M_OID_WANIF,
        //all derived Message classes
        M_OID_MAX
    }OID;
private:
    typedef struct _CountEntry {
        UINT32 ulCount;
        #ifndef NDEBUG
        CMutex *pMutex;
        _CountEntry():ulCount(0),pMutex(NULL){}
        #else
        _CountEntry():ulCount(0){}
        #endif
    }CountEntry,*PCountEntry;

    static UINT32 s_MutexCount;
    static PCountEntry s_pObjectCount;
    static bool s_bCountInited;

protected:
    CObject();
    virtual ~CObject();

    bool Construct(OID oid);
    bool Destruct(OID oid);

public:

#ifndef NDEBUG
    virtual bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
    virtual void Dump() const;
#endif

    static void ShowCount();
    static bool InitObjectClass();

    friend class CMutex;
};

#ifndef NDEBUG
bool AssertValidObject(const CObject* pOb, char fn[], int nLine);
#define ASSERT_VALID(pOb) (::AssertValidObject(pOb,__FILE__,__LINE__))
#else
#define ASSERT_VALID(pOb) ((bool)((pOb)!=NULL))

#endif

#endif
