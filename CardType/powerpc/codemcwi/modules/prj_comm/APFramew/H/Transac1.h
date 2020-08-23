#ifndef _INC_TRANSACTIONMANAGER
#define _INC_TRANSACTIONMANAGER

#ifndef _INC_OBJECT
#include "Object.h"
#endif

#include <map>
using namespace std;

class CTransaction;

class CTransactionManager
#ifndef NDEBUG
:public CObject
#endif
{
private:
    map<UINT16, CTransaction*> m_mapTransaction;

public:
    CTransactionManager();
    ~CTransactionManager();
    bool Insert(CTransaction* pTransact);
    bool Remove(CTransaction* pTransact);
    CTransaction* Find(UINT16 uId);

#ifndef NDEBUG
    virtual bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
#endif

};

#endif
