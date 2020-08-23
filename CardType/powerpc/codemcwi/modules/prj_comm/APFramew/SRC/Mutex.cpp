/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: Mutex.cpp
 *
 * DESCRIPTION:
 *     Implementation of the FWKLIB's Mutex Object.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 07/12/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/

#ifndef _INC_MUTEX
#include "Mutex.h"
#endif

#ifndef _INC_OBJECT
#include "Object.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#include "vxwk2pthread.h"

#ifdef __WIN32_SIM__
HANDLE CMutex::s_mutexCount = NULL;
#elif __NUCLEUS__
#include <string.h>
#else //VxWorks
SEM_ID CMutex::s_mutexCount = NULL;
#endif

CMutex::CMutex()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMutex::CMutex");
#endif

#ifdef __WIN32_SIM__
    if ((m_hMutex = ::CreateMutex(NULL,false,NULL))==NULL)
    {
        LOG(LOG_CRITICAL,0,"Create m_hMutex failed.");
#elif __NUCLEUS__   
    ::memset(&m_semaphore, 0, sizeof(m_semaphore));

    if(NU_SUCCESS!=::NU_Create_Semaphore(&m_semaphore, "", 1, NU_PRIORITY))
	{
		LOG(LOG_CRITICAL,0,"Create m_semaphore failed.");
#else //VxWorks
    if ((m_Id = ::semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE))==NULL)
    {
        LOG(LOG_CRITICAL,0,"Create m_Id failed.");
#endif
        return;
    }

#ifndef __NUCLEUS__
#ifndef NDEBUG
#ifdef __WIN32_SIM__
    if (s_mutexCount==NULL)
        s_mutexCount = ::CreateMutex(NULL,false,NULL);
#else //VxWorks
    if (s_mutexCount==NULL)
        s_mutexCount = ::semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
#endif

    if (s_mutexCount==NULL)
    {
        LOG(LOG_CRITICAL,0,"Create s_mutexCount failed.");
        return;
    }

#ifdef __WIN32_SIM__
    if (::WaitForSingleObject(s_mutexCount,INFINITE)!=WAIT_OBJECT_0)
    {
        LOG(LOG_SEVERE,0,"WaitForSingleObject failed.");
#else //VxWorks
    if (::semTake(s_mutexCount,WAIT_FOREVER)==ERROR)
    {
        LOG(LOG_SEVERE,0,"semTake failed.");
#endif
        return;
    }

    ++CObject::s_MutexCount;

#ifdef __WIN32_SIM__
    if (::ReleaseMutex(s_mutexCount)==0)
        LOG(LOG_SEVERE,0,"ReleaseMutex failed.");
#else //VxWorks
    if (::semGive(s_mutexCount)==ERROR)
        LOG(LOG_SEVERE,0,"semGive failed.");
#endif

#endif // NDEBUG
#endif // __NUCLEUS__ 
}

CMutex::~CMutex()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMutex::~CMutex");

    if (this==NULL)
    {
        LOG(LOG_CRITICAL,0,"this==NULL.");
        return;
    }
#endif

#ifdef __WIN32_SIM__
    if (m_hMutex==NULL)
    {
        LOG(LOG_SEVERE,0,"m_hMutex==NULL");
        return;
    }
#elif __NUCLEUS__
#else //VxWorks
    if (m_Id==NULL)
    {
        LOG(LOG_SEVERE,0,"m_Id==NULL");
        return;
    }
#endif

#ifdef __WIN32_SIM__
    if (::CloseHandle(m_hMutex)==0)
    {
        m_hMutex=NULL; 
        LOG(LOG_SEVERE,0,"Delete m_hMutex failed.");

#elif __NUCLEUS__
        if(NU_SUCCESS!=::NU_Delete_Semaphore(&m_semaphore))
	{
        ::memset(&m_semaphore, 0, sizeof(m_semaphore));
		LOG(LOG_SEVERE,0,"Delete m_semaphore failed.");

#else //VxWorks
    if (::semDelete(m_Id)==ERROR)
    {
        m_Id = NULL;
        LOG(LOG_SEVERE,0,"Delete m_Id failed.");
#endif
        return;
    }

#ifndef __NUCLEUS__
#ifndef NDEBUG
    if (s_mutexCount==NULL)
    {
        LOG(LOG_CRITICAL,0,"s_mutexCount==NULL.");
        return;
    }

#ifdef __WIN32_SIM__
    if (::WaitForSingleObject(s_mutexCount,INFINITE)!=WAIT_OBJECT_0)
    {
        LOG(LOG_SEVERE,0,"WaitForSingleObject failed.");
#else //VxWorks
    if (::semTake(s_mutexCount,WAIT_FOREVER)==ERROR)
    {
        LOG(LOG_SEVERE,0,"semTake failed.");
#endif
        return;
    }

    --CObject::s_MutexCount;

#ifdef __WIN32_SIM__
    if (::ReleaseMutex(s_mutexCount)==0)
        LOG(LOG_SEVERE,0,"ReleaseMutex failed.");
#else //VxWorks
    if (::semGive(s_mutexCount)==ERROR)
        LOG(LOG_SEVERE,0,"semGive failed.");
#endif

#endif // NDEBUG
#endif  // __NUCLEUS__
}

#ifndef __NUCLEUS__
bool CMutex::Wait() const
#else
bool CMutex::Wait()
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMutex::Wait");

    if (this==NULL)
    {
        LOG(LOG_CRITICAL,0,"this==NULL.");
        return false;
    }
#endif

#ifdef __WIN32_SIM__
    if (m_hMutex==NULL)
    {
        LOG(LOG_SEVERE,0,"m_hMutex==NULL.");
        return false;
    }
#elif __NUCLEUS__
#else //VxWorks
    if (m_Id==NULL)
    {
        LOG(LOG_SEVERE,0,"m_Id==NULL.");
        return false;
    }
#endif

#ifdef __WIN32_SIM__
    return ::WaitForSingleObject(m_hMutex,INFINITE)==WAIT_OBJECT_0;
#elif __NUCLEUS__
    return (::NU_Obtain_Semaphore(&m_semaphore, NU_SUSPEND)==NU_SUCCESS);
#else //VxWorks
    return ::semTake(m_Id, WAIT_FOREVER)==OK;
#endif
}

#ifndef __NUCLEUS__
bool CMutex::Release() const
#else
bool CMutex::Release()
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMutex::Release");

    if (this==NULL)
    {
        LOG(LOG_CRITICAL,0,"this==NULL.");
        return false;
    }
#endif

#ifdef __WIN32_SIM__
    if (m_hMutex==NULL)
    {
        LOG(LOG_SEVERE,0,"m_hMutex==NULL.");
        return false;
    }
#elif __NUCLEUS__
#else //VxWorks
    if (m_Id==NULL)
    {
        LOG(LOG_SEVERE,0,"m_Id==NULL.");
        return false;
    }
#endif

#ifdef __WIN32_SIM__
    return ::ReleaseMutex(m_hMutex)!=0;
#elif __NUCLEUS__
    return ::NU_Release_Semaphore(&m_semaphore)==NU_SUCCESS;
#else //VxWorks
    return ::semGive(m_Id)==OK;
#endif
}


