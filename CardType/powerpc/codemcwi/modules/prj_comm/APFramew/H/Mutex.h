#ifndef _INC_MUTEX
#define _INC_MUTEX

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

#ifdef __WIN32_SIM__
#include <windows.h>
#elif __NUCLEUS__
#include "nucleus.h"
#else
#ifndef __INCsemLibh
//#include <semLib.h>//modify by huangjl
#endif
#endif

#ifdef COMIP
//#define NU_SEMAPHORE_SIZE 	11
#endif

class CMutex
{
private:
#ifdef __WIN32_SIM__
    static HANDLE s_mutexCount;
    HANDLE m_hMutex;
#elif __NUCLEUS__
	NU_SEMAPHORE m_semaphore;
#else
    static SEM_ID s_mutexCount;
    SEM_ID m_Id;
#endif

public:
    CMutex();
    ~CMutex();
    #ifndef __NUCLEUS__
    bool Wait() const;
    bool Release() const;
    #else
    bool Wait();
    bool Release();
    #endif
    
};

#endif //_INC_MUTEX

