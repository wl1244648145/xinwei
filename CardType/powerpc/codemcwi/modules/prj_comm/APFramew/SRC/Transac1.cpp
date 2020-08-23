#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#ifndef _INC_TRANSACTIONMANAGER
#include "TransactionManager.h"
#endif

#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifdef __WIN32_SIM__
#ifndef _INC_TYPEINFO
#include <typeinfo.h>
#endif
#else
#ifndef __TYPEINFO__
#include <typeinfo>
using namespace std;
#endif
#endif

typedef map<UINT16, CTransaction*>::value_type ValType;

bool CTransactionManager::Insert(CTransaction* pTransact)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransactionManager::Insert");
#endif

    if (!ASSERT_VALID(this))
        return false;

    if (!ASSERT_VALID(pTransact))
        return false;

    if (pTransact->m_pManager!=NULL)
    {
        LOG(LOG_SEVERE,0,"pTransact had been inserted to a manager already.");
        return false;
    }
    m_mapTransaction.insert(ValType(pTransact->GetId(), pTransact));
    pTransact->m_pManager = this;
    return true;
}

bool CTransactionManager::Remove(CTransaction* pTransact)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransactionManager::Remove");
#endif

    if (!ASSERT_VALID(this))
        return false;

    if (!ASSERT_VALID(pTransact))
        return false;

    if (pTransact->m_pManager != this)
    {
        LOG(LOG_SEVERE,0,"pTransact not belong to this manager.");
#ifdef WBBU_CODE
      pTransact->m_pManager = NULL;
#endif
        return false;
    }
    
    if (1 != m_mapTransaction.erase(pTransact->GetId()) )
    {
        LOG(LOG_SEVERE,0,"Removing Transaction error.");
#ifdef WBBU_CODE
         pTransact->m_pManager = NULL;
#endif
        return false;
    }
    pTransact->m_pManager = NULL;
    return true;
}
/*
bool CTransactionManager::Remove(UINT16 uId)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransactionManager::Remove");
#endif

    if (!ASSERT_VALID(this))
        return false;

    CTransaction* pTransact = Find(uId);
    if (pTransact == NULL)
    {
        LOG(LOG_WARN,0,"Transaction not find.");
        return false;
    }

    if (pTransact->m_pManager != this)
    {
        LOG(LOG_SEVERE,0,"Transaction belong to other manager too.");
        return false;
    }

    m_mapTransaction.erase(uId);
    pTransact->m_pManager = NULL;
    return true;
}
*/

CTransaction* CTransactionManager::Find(UINT16 uId)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransactionManager::Find");
#endif

    if (!ASSERT_VALID(this))
        return NULL;

    map<UINT16, CTransaction*>::iterator it = m_mapTransaction.find(uId);
    if (it != m_mapTransaction.end())
        return it->second;
    return NULL;
}

CTransactionManager::CTransactionManager()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransactionManager::CTransactionManager");

    if (!Construct(CObject::M_OID_TRANSACTIONMANAGER))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif
}

CTransactionManager::~CTransactionManager()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CTransactionManager::~CTransactionManager");

    if (!Destruct(CObject::M_OID_TRANSACTIONMANAGER))
        LOG(LOG_SEVERE,0,"Destruct failed.");
#endif
}

#ifndef NDEBUG
bool CTransactionManager::AssertValid(const char* lpszFileName, UINT32 nLine) const
{
#ifdef __WIN32_SIM__
    if (::IsBadReadPtr(this,sizeof(CTransactionManager)) || ::IsBadWritePtr((void*)this,sizeof(CTransactionManager)) )
    {
        CLog::LogAdd(lpszFileName,nLine,M_LL_CRITICAL,0,"Invalid CTransactionManager pointer.");
        return false;
    }
#endif
    if (typeid(*this) != typeid(CTransactionManager))
    {
        LOG(LOG_CRITICAL,0,"Invalid typeid.");
        return false;
    }
    return true;
}
#endif
