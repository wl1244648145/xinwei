#ifndef _INC_TASK
#define _INC_TASK

#ifndef _INC_COMENTITY
#include "ComEntity.h"
#endif

#ifdef __WIN32_SIM__

#ifndef _WINDOWS_
#include <windows.h>
#endif

#elif __NUCLEUS__

#ifndef NUCLEUS
#include "Nucleus.h"
#endif

#else //VxWorks

#endif

#ifdef COMIP
#define NU_TASK_SIZE	42
#endif

class CComMessage;
class CMessage;

class CTask : public CComEntity
{
protected:
#define M_TF_STOPPED  0x00000000
#define M_TF_RUNNING  0x00000001
#define M_TF_SUSPEND  0x00000002
#define M_TF_STATEALL 0x00000003
#define M_TF_INITED   0x00000100

    UINT32 m_uFlags;

private:

#ifdef __WIN32_SIM__
    HANDLE m_hThread;
#elif __NUCLEUS__
	NU_TASK m_TCB;
#else //VxWorks
#endif

protected:
    char m_szName[13];
    UINT8 m_uPriority;
#ifdef __NUCLEUS__
	void *m_ptrStack;
#else
    UINT32 m_uOptions;  // only used in Vxworks
#endif
    UINT16 m_uStackSize;

#ifndef COMIP
    #ifndef M_TGT_CPE
    int    m_uMaxBlockedTime; 
    bool   m_bNeedDeadlockMonitor;
    #endif
#endif

public:
#ifndef __NUCLEUS__
    static bool TaskLock();
    static bool TaskUnlock();
#endif
    
#ifdef __WIN32_SIM__
    static TID   GetCurrentTaskId();
#else
    static void GetCurrentTaskName(char *);
#endif

    CTask();

    bool Begin();

#ifndef __NUCLEUS__
    bool Resume();
    bool Suspend();
    bool Restart();
    bool End();

    bool SetPriority(UINT8 uPriority) const;
    bool GetPriority(SINT32*) const;
#endif

#ifndef NDEBUG
    virtual bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
#endif

    virtual bool PostMessage(CComMessage*, SINT32, bool isUrgent = false);

protected:
    //virtual ~CTask();
    void Run();

    virtual bool Initialize()=0;
    virtual void MainLoop()=0;
#ifndef COMIP
    #ifndef M_TGT_CPE
    virtual bool IsMonitoredForDeadlock()=0;
    virtual int  GetMaxBlockedTime() = 0;
    #endif
#endif
    //virtual CComMessage* GetMessage();
    virtual bool ProcessComMessage(CComMessage*);
    virtual bool ProcessMessage(CMessage&);
    virtual void PostProcess();

private:
#ifdef __WIN32_SIM__
    static DWORD WINAPI TaskProc(void* pTask);
#elif __NUCLEUS__
	static VOID TaskProc(UNSIGNED argc, VOID *argv);
#else //VxWorks
    static STATUS TaskProc(CTask* pTask);
#endif

    CTask(CTask& task); //Forbid the default copy constructor
};

#endif
