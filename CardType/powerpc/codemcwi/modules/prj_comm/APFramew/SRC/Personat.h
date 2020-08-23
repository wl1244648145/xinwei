#ifndef _INC_PERSONATOR
#define _INC_PERSONATOR

#ifdef WIN32
#ifdef __WIN32_SIM__

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

class CPersonator : public CBizTask
{
public:
    static CPersonator* GetInstance();
    ~CPersonator();
private:
    CPersonator();
    typedef struct _PERSONATOR_TASK_LIST_ENTRY
    {
        TID tid;
        char* pszText;
    }PersonatorTaskListEntry;
    static PersonatorTaskListEntry s_TaskList[];
    TID m_tid;

    static CPersonator* s_pPersonator;
    static CComEntity::EntityEntry s_EntityTableBackup[M_TID_MAX+1];
    static bool s_bEndAllMsg;
    static HANDLE s_hWinThread;
    static DWORD s_dwWinThreadId;
    static HANDLE s_hSendThread;
    static DWORD s_dwSendThreadId;

    static DWORD WINAPI SendThreadProc(LPVOID lpParam);
    static DWORD WINAPI WinThreadProc(LPVOID lpParam);
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    virtual TID GetEntityId() const;
    virtual bool Initialize();
    virtual void MainLoop();
    virtual bool IsNeedTransaction() const;

    CPersonator(CPersonator&);
};

#else
#error "This module can only be used on Windows platform."
#endif //__WIN32_SIM__

#endif //WIN32

#endif //_INC_PERSONATOR
