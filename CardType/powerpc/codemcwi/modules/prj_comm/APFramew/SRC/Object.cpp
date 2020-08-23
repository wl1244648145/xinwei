/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: Object.cpp
 *
 * DESCRIPTION:
 *     Implementation of the FWKLIB's Generic Object class.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 09/21/2005  Liu Qun     Change static count table to dyna created
 * 07/12/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_OBJECT
#include "Object.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

UINT32 CObject::s_MutexCount=0;
CObject::PCountEntry CObject::s_pObjectCount=NULL;
bool CObject::s_bCountInited=false;

CObject::CObject()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CObject::CObject");
    if (!Construct(M_OID_OBJECT))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif

}

CObject::~CObject()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CObject::~CObject");
    if (!Destruct(M_OID_OBJECT))
        LOG(LOG_SEVERE,0,"Destruct failed.");
#endif
}


bool CObject::Construct(OID oid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CObject::Construct");
#endif

    if (oid >= M_OID_MAX)
    {
        LOG(LOG_SEVERE,0,"oid>=M_OID_MAX.");
        return false;
    }
#ifndef NDEBUG
    if (!s_pObjectCount[oid].pMutex->Wait())
    {
        LOG(LOG_SEVERE,0,"mutex Wait failed.");
        return false;
    }
#endif

    s_pObjectCount[oid].ulCount++;

#ifndef NDEBUG
    if (!s_pObjectCount[oid].pMutex->Release())
    {
        LOG(LOG_SEVERE,0,"mutex Release failed.");
        return false;
    }
#endif
    return true;
}

bool CObject::Destruct(OID oid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CObject::Destruct");
#endif

    if (oid >= M_OID_MAX)
    {
        LOG(LOG_SEVERE,0,"oid>=M_OID_MAX.");
        return false;
    }
#ifndef NDEBUG
    if (!s_pObjectCount[oid].pMutex->Wait())
    {
        LOG(LOG_SEVERE,0,"mutex Wait failed.");
        return false;
    }
#endif

    s_pObjectCount[oid].ulCount--;

#ifndef NDEBUG
    if (!s_pObjectCount[oid].pMutex->Release())
    {
        LOG(LOG_SEVERE,0,"mutex Release failed.");
        return false;
    }
#endif
    return true;
}

#ifndef NDEBUG
bool AssertValidObject(const CObject* pOb, char* lpszFileName, int nLine)
{
    if (pOb==NULL)
    {
        LOG(LOG_CRITICAL,0,"NULL CObject pointer.");
        return false;
    }
#ifdef __WIN32_SIM__
    //check pOb
    if (::IsBadReadPtr(pOb,sizeof(CObject)) || ::IsBadWritePtr((void*)pOb,sizeof(CObject)) )
    {
        LOG(LOG_CRITICAL,0,"Invalid CObject pointer.");
        return false;
    }
    //check vptr
    if (*(void**)pOb!=NULL &&  ::IsBadReadPtr(*(void**)pOb, sizeof(void*)) )
    {
        LOG(LOG_CRITICAL,0,"Invalid vptr.");
        return false;
    }
#else
    if (*(void**)pOb==NULL)
    {
        LOG(LOG_CRITICAL,0,"Invalid vptr.");
        return false;
    }
#endif
    //derived class size mem check should be in each AssertValid() funcs.
    return pOb->AssertValid(lpszFileName, nLine);
}

bool CObject::AssertValid(const char* , UINT32 ) const
{
    return true;
}

void CObject::Dump() const
{
}
#endif

void CObject::ShowCount()
{
	LOG(LOG_DEBUG,0,"s_ObjectCount[]=");
	LOG(LOG_DEBUG,0,"OID\tCount");
	for(int i=M_OID_OBJECT;i<M_OID_MAX;i++)
    {
        LOG2(LOG_DEBUG,0,"%d\t%u\n",i,s_pObjectCount[i].ulCount);
    }
	LOG1(LOG_DEBUG,0,"Mutex Count = %u", s_MutexCount);
}

bool CObject::InitObjectClass()
{
    if (s_bCountInited==true)
        return true;

    s_pObjectCount = new CObject::CountEntry[M_OID_MAX];
    if (s_pObjectCount==NULL)
        return false;

    #ifndef NDEBUG
    for (int oid = M_OID_OBJECT;oid<M_OID_MAX;oid++)
    {
        CMutex *pMutex = new CMutex;
        if (pMutex==NULL)
            return false;
        s_pObjectCount[oid].pMutex = pMutex;
    }
    #endif
    
    s_bCountInited = true;
    return true;
}
