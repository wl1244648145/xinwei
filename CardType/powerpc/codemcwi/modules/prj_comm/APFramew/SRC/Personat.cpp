/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: Personator.cpp
 *
 * DESCRIPTION:
 *     Implementation of the APFWK's internal debug tool:Personator
 *     Personator is an isolated module based on other FWKLIB classes.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 10/30/2005  Liu Qun     Fixed the 'Stack corrupted' bug
 * 10/30/2005  Liu Qun     removed some obsoleted parts
 * 09/04/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifdef WIN32
#ifdef __WIN32_SIM__

#ifndef _INC_PERSONATOR
#include "Personator.h"
#endif

#ifndef _INC_MSGQUEUE
#include "MsgQueue.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif


#ifndef PM_QS_POSTMESSAGE
#define PM_QS_POSTMESSAGE 0x980000
#endif

#define IDR_FWKLIB                      101
#define IDI_PERSONATOR                  102
#define IDI_SMALL                       103
#define ID_FILE_REFRESH                 40001
#define ID_FILE_SETTING                 40002
#define ID_TASK_FIRST                   40101
#define ID_TASK_LAST                    40300
#define ID_CASE_FIRST                   40301
#define ID_CASE_LAST                    41300

#ifndef _INC_PTRLIST
#include "PtrList.h"
#endif

class CCase
{
private:
	static unsigned int ReFormat(char* pDest, char* pSrc, unsigned long nLen);
    typedef struct _Entry
    {
        CComMessage* pComMsg;
        int delay;
		_Entry():pComMsg(NULL),delay(0){}
    }CaseEntry, *PCaseEntry;
    CPtrList* m_plstEntry;
    char m_szName[101];

public:
    static CCase* ImportCase(CComEntity* pEntity, LPCTSTR lpszFileName);

    CCase(LPCTSTR lpszName);
    ~CCase();
    bool AddMessage(CComMessage*, int);
    int GetCount();
    char* GetName();
    int GetDelay(int index);
    CComMessage* GetMessage(int index);
};

CCase::CCase(LPCTSTR lpszName)
{
    if (lpszName!=NULL)
        ::strncpy(m_szName, lpszName,100);
	m_plstEntry = new CPtrList(10, 0);
}

CCase::~CCase()
{
    while (!m_plstEntry->empty())
    {
        PCaseEntry pEntry = (PCaseEntry)m_plstEntry->front();
        m_plstEntry->pop_front();
        if ((pEntry->pComMsg)!=NULL)
			pEntry->pComMsg->Destroy();
        delete pEntry;
    }
}

unsigned int CCase::ReFormat(char *pDest, char *pSrc, unsigned long nLen)
{
    unsigned long DestOffset=0;
    unsigned long SrcOffset = 0;
    while (SrcOffset < nLen)
    {
        if (*(pSrc+SrcOffset)==0x0d || *(pSrc+SrcOffset)==0x0a)
        {
            SrcOffset++;
            if (DestOffset==0)
                continue;
            if (*(pDest+DestOffset-1)==' ')
                continue;
            if (*(pDest+DestOffset-1)==':')
                continue;
            *(pDest+DestOffset)=' ';
            DestOffset++;
            continue;
        }
        if (*(pSrc+SrcOffset)=='\t' || *(pSrc+SrcOffset)==' ')
        {
            SrcOffset++;
            if (DestOffset==0)
                continue;
            if (*(pDest+DestOffset-1)==':')
                continue;
            if (*(pDest+DestOffset-1)==' ')
                continue;
            *(pDest+DestOffset) = ' ';
            DestOffset++;
            continue;
        }
        if (*(pSrc+SrcOffset)=='#')
        {
            do
            {
                SrcOffset++;
            }
            while (SrcOffset<nLen && *(pSrc+SrcOffset)!=0x0d && *(pSrc+SrcOffset)!=0x0a);
            continue;
        }
        if (*(pSrc+SrcOffset)==':')
        {
            SrcOffset++;
            if (*(pDest+DestOffset-1)==' ')
            {
                *(pDest+DestOffset-1)=':';
                continue;
            }
            if (*(pDest+DestOffset-1)==':')
                continue;
            *(pDest+DestOffset) = ':';
            DestOffset++;
            continue;
        }
        if (*(pSrc+SrcOffset)=='_')
        {
            SrcOffset++;
            *(pDest+DestOffset) = '_';
            DestOffset++;
            continue;
        }
        if (*(pSrc+SrcOffset)=='0' || (*(pSrc+SrcOffset)>='1' && *(pSrc+SrcOffset)<='9'))
        {
            *(pDest+DestOffset) = *(pSrc+SrcOffset);
            DestOffset++;
            SrcOffset++;
            continue;
        }
        if (*(pSrc+SrcOffset)>='A' && *(pSrc+SrcOffset)<='Z')
        {
            *(pDest+DestOffset) = *(pSrc+SrcOffset);
            DestOffset++;
            SrcOffset++;
            continue;
        }
        if (*(pSrc+SrcOffset)>='a' && *(pSrc+SrcOffset)<='z')
        {
            *(pDest+DestOffset) = (char)::toupper(*(pSrc+SrcOffset));
            DestOffset++;
            SrcOffset++;
            continue;
        }
        SrcOffset++;
    }
    *(pDest+DestOffset) = 0;
    return DestOffset;
}

CCase* CCase::ImportCase(CComEntity* pEntity, LPCTSTR lpszFileName)
{
    HANDLE hFile;
    DWORD dwFileSize;
    CCase* pCase = NULL;

    if (pEntity==NULL)
        return NULL;
    if (lpszFileName==NULL)
        return NULL;
    hFile = ::CreateFile(lpszFileName,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING, 
                         FILE_ATTRIBUTE_ARCHIVE|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY,
                         NULL);
    if (hFile==INVALID_HANDLE_VALUE)
        return NULL;
    dwFileSize = ::GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE || dwFileSize==0)
    {
        ::CloseHandle(hFile);
        return NULL;
    }

    char* pBuf = new char[dwFileSize];
    if (pBuf==NULL)
    {
        ::CloseHandle(hFile);
        return NULL;
    }

    char* pDest = new char[dwFileSize+1];
    if (pDest==NULL)
    {
        delete [] pBuf;
        ::CloseHandle(hFile);
        return NULL;
    }
    ::ReadFile(hFile, pBuf, dwFileSize, &dwFileSize, NULL);
    ::CloseHandle(hFile);

    dwFileSize = ReFormat(pDest, pBuf, dwFileSize);
    delete [] pBuf;


    char * p = pDest;

    char name[256];
    int total=0;

    if (::strncmp(p, "NAME:", 5)!=0)
    {
        delete [] pDest;
        return NULL;
    }
    p += 5;
    ::sscanf(p, "%s", name);
    while (*p!=' ')
        p++;
    p++;

    if (::strlen(name)!=0)
        pCase = new CCase(name);
    else
    {
        delete [] pDest;
        return NULL;
    }

    if (::strncmp(p, "TOTAL:", 6)!=0)
    {
        delete pCase;
        delete [] pDest;
        return NULL;
    }
    p+=6;
    ::sscanf(p, "%X",&total);
    while (*p != ' ')
        p++;
    p++;

    char* pPayload = new char[65535];
    if (pPayload==NULL)
    {
        delete pCase;
        delete [] pDest;
        return NULL;
    }

    for (int i=0;i<total;i++)
    {
        int payloadlen=0;

        DWORD DstTid;
        DWORD SrcTid;
        DWORD MsgId;
        DWORD EID;
        DWORD UID;
        DWORD sfid;
        DWORD Dir;
        DWORD IpType;
        DWORD FromBtsAddr;
        DWORD UdpOffset;
        DWORD DhcpOffset;
        DWORD KeyMac;
        int delay;

        if (::strncmp(p, "DSTTID:", 7) != 0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p += 7;
        ::sscanf(p, "%X",&DstTid);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "SRCTID:", 7)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=7;
        ::sscanf(p, "%X",&SrcTid);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "MSGID:", 6)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=6;
        ::sscanf(p, "%X",&MsgId);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "EID:", 4)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=4;
        ::sscanf(p, "%X", &EID);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "UID:", 4)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=4;
        ::sscanf(p, "%X", &UID);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "SFID:", 5)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=5;
        ::sscanf(p, "%X",&sfid);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "DIR:", 4)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=4;
        ::sscanf(p, "%X", &Dir);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "IPTYPE:",7)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=7;
        ::sscanf(p, "%X", &IpType);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "FROMBTS:", 8)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=8;
        ::sscanf(p, "%X", &FromBtsAddr);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "UDP:", 4)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=4;
        ::sscanf(p, "%X", &UdpOffset);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "DHCP:",5)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=5;
        ::sscanf(p, "%X", &DhcpOffset);
        while (*p!=' ')
            p++;
        p++;

        if (::strncmp(p, "KEYMAC:", 7)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=7;
        ::sscanf(p, "%X", &KeyMac);
        while (*p!=' ')
            p++;
        p++;

        //Read Data
        if (::strncmp(p, "DATA:", 5)!=0)
        {
            delete pCase;
            delete [] pDest;
            return NULL;
        }
        p+=5;
        while (*p==' ')
            p++;

        while (::strncmp(p, "DELAY:",6)!=0)
        {
            ::sscanf(p, "%X", pPayload+payloadlen++);
            while (*p!=' ')
                p++;
            while (*p==' ')
                p++;
        }

        p+=6;
        ::sscanf(p,"%X",&delay);

        while (p<(pDest+dwFileSize) && *p!=' ')
            p++;
        p++;

        CComMessage* pComMsg = new (pEntity, payloadlen) CComMessage;
        if (pComMsg!=NULL)
        {
            pComMsg->SetDstTid((TID)DstTid);
            pComMsg->SetSrcTid((TID)SrcTid);
            pComMsg->SetMessageId((UINT16)MsgId);
#if !(M_TARGET==M_TGT_CPE)
            pComMsg->SetEID((UINT32)EID);
            pComMsg->SetUID((UINT16)UID);
#endif

            //pComMsg->SetSFID((SFID)sfid);
#if !(M_TARGET==M_TGT_CPE)
            pComMsg->SetDirection((UINT8)Dir);
            pComMsg->SetIpType((UINT8)IpType);
            pComMsg->SetBtsAddr((UINT32)FromBtsAddr);
#endif
			::memcpy(pComMsg->GetDataPtr(), pPayload, payloadlen);
            if (DhcpOffset!=0)
            {
                char* pDhcp = (char*)pComMsg->GetDataPtr();
                pDhcp += DhcpOffset;
#if !(M_TARGET==M_TGT_CPE)
                pComMsg->SetDhcpPtr(pDhcp);
#endif
			}
            if (UdpOffset!=0)
            {
                char* pUdp = (char*)pComMsg->GetDataPtr();
                pUdp += UdpOffset;
#if !(M_TARGET==M_TGT_CPE)
                pComMsg->SetUdpPtr(pUdp);
#endif
			}
            if (KeyMac!=0xFF)
            {
                unsigned char* pKeyMac = (unsigned char*)pComMsg->GetDataPtr();
                pKeyMac += KeyMac;
#if !(M_TARGET==M_TGT_CPE)
                pComMsg->SetKeyMac(pKeyMac);
#endif
			}
            if (!pCase->AddMessage(pComMsg, delay))
			{
				pComMsg->Destroy();
				break;
			}
        }
    }

    delete [] pPayload;
    delete [] pDest;
    return pCase;
}

bool CCase::AddMessage(CComMessage* pComMsg, int delay)
{
    if (pComMsg==NULL)
        return false;

    PCaseEntry pEntry = new CaseEntry;
    if (pEntry==NULL)
		return false;
    pEntry->pComMsg = pComMsg;
    pEntry->delay = delay;
    m_plstEntry->push_back(pEntry);
	return true;
}

int CCase::GetCount()
{
    return m_plstEntry->size();
}

char* CCase::GetName()
{
    return m_szName;
}

CComMessage* CCase::GetMessage(int index)
{
    if (index >= GetCount())
        return NULL;

	CPtrList::iterator first = m_plstEntry->begin();
	CPtrList::iterator last = m_plstEntry->end();
    for (int i=0; i<index && first!=last;++i)
        ++first;
    PCaseEntry pEntry = (PCaseEntry)*first;
    return pEntry->pComMsg;
}

int CCase::GetDelay(int index)
{
    if (index >= GetCount())
        return 0;

	CPtrList::iterator first = m_plstEntry->begin();
	CPtrList::iterator last = m_plstEntry->end();
    for (int i=0; i<index && first!=last;++i)
        ++first;
    PCaseEntry pEntry = (PCaseEntry)*first;
    return pEntry->delay;
}

#define WM_READY    WM_USER+1
#define WM_EXIT     WM_USER+2
#define WM_SETTING  WM_USER+3
#define WM_SENDMSG  WM_USER+4

CPersonator* CPersonator::s_pPersonator=NULL;
CComEntity::EntityEntry CPersonator::s_EntityTableBackup[M_TID_MAX+1];
HANDLE CPersonator::s_hWinThread = NULL;
HANDLE CPersonator::s_hSendThread = NULL;
DWORD CPersonator::s_dwWinThreadId = 0;
DWORD CPersonator::s_dwSendThreadId = 0;

CPersonator* CPersonator::GetInstance()
{
    if (s_pPersonator==NULL)
        s_pPersonator = new CPersonator;
    return s_pPersonator;
}

CPersonator::CPersonator()
{
    ::strcpy(m_szName,"Personator");
    m_iMsgQMax = 100;
    m_tid = M_TID_MAX;
}

CPersonator::~CPersonator()
{
}

DWORD WINAPI CPersonator::WinThreadProc(LPVOID lpParam)
{
    HINSTANCE hInst;
    MSG msg;
	WNDCLASSEX wcex;
    HWND hWnd;
    HMENU hMenu;
    HMENU hMenuFile;
    HMENU hMenuTask;

    hInst = ::GetModuleHandle(NULL);

    wcex.cbSize = sizeof(WNDCLASSEX); 
    wcex.style			= CS_DBLCLKS|CS_NOCLOSE;
    wcex.lpfnWndProc	= (WNDPROC)WindowProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= ::LoadIcon(hInst, (LPCTSTR)IDI_PERSONATOR);
    wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= "PersonatorWindowClass";
    wcex.hIconSm		= ::LoadIcon(hInst, (LPCTSTR)IDI_SMALL);
    ::RegisterClassEx(&wcex);


    hMenu = ::CreateMenu();

    hMenuFile = ::CreatePopupMenu();
    ::AppendMenu(hMenuFile, MF_STRING, ID_FILE_SETTING, "设置...");
    ::AppendMenu(hMenuFile, MF_STRING, ID_FILE_REFRESH, "更新用例");

    hMenuTask = ::CreatePopupMenu();
    for (int i=0;;i++)
    {
        if (s_TaskList[i].tid==M_TID_MAX)
            break;
        ::AppendMenu(hMenuTask, MF_STRING, ID_TASK_FIRST + s_TaskList[i].tid, s_TaskList[i].pszText);
    }
    ::AppendMenu(hMenuTask, MF_STRING, ID_TASK_FIRST + M_TID_MAX, "M_TID_MAX");

    ::AppendMenu(hMenu, MF_STRING|MF_POPUP, (UINT)hMenuFile, "文件(&F)");
    ::AppendMenu(hMenu, MF_STRING|MF_POPUP, (UINT)hMenuTask, "任务(&T)");

    hWnd = ::CreateWindow("PersonatorWindowClass", "Personator", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,200 ,50, NULL, hMenu, hInst, lpParam);

    ::SetClassLong(hWnd, GCL_HICON, (LONG)::LoadIcon(hInst, MAKEINTRESOURCE(IDI_PERSONATOR)));
    ::SetClassLong(hWnd, GCL_HICONSM, (LONG)::LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL)));

    ::ShowWindow(hWnd, SW_SHOW);
    ::UpdateWindow(hWnd);
    
    while (::GetMessage(&msg,NULL, 0,0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
    return (DWORD)msg.wParam;
}

LRESULT CALLBACK CPersonator::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static OPENFILENAME ofn;
    static TCHAR szPath[MAX_PATH+1];
    static DWORD dwMainThreadId = 0;
    static CPtrList* plstCase=NULL;
	int wmId, wmEvent;
    static CCase* pCaseCur=NULL;
    static int nIndexCur=0;

	switch (message) 
	{
    case WM_CREATE:
        {
            MSG msg;
            bool bReady=false;

            plstCase = new CPtrList(20,0);
            if (plstCase==NULL)
                return -1;

            CREATESTRUCT* lpcs = (CREATESTRUCT*)lParam;
            dwMainThreadId = (DWORD)lpcs->lpCreateParams;
            ::PostThreadMessage(dwMainThreadId, WM_READY, 0, (LPARAM)::GetCurrentThreadId());
            s_hSendThread = ::CreateThread(NULL, 0, SendThreadProc, (LPVOID)::GetCurrentThreadId(), 0, &s_dwSendThreadId);
            if (s_hSendThread!=NULL)
            {
                while (!bReady)
                {
                    if (::PeekMessage(&msg, NULL, WM_READY, WM_READY, PM_REMOVE|PM_QS_POSTMESSAGE) && 
                        msg.message == WM_READY && (DWORD)msg.lParam == s_dwSendThreadId)
                        bReady = true;
                }
            }
            //Initialize the ofn structure
            ::ZeroMemory(szPath, MAX_PATH+1);
            ::ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hWnd;
            ofn.hInstance = ::GetModuleHandle(NULL);
            ofn.lpstrFilter = "CaseFile (*.cas)\0*.CAS\0\0";
            ofn.lpstrCustomFilter = NULL;
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = szPath;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.lpstrTitle = "指定文件路径";
            ofn.Flags = OFN_EXPLORER|OFN_LONGNAMES|OFN_PATHMUSTEXIST|OFN_NOTESTFILECREATE|OFN_READONLY;
            ofn.lpstrDefExt = "cas";

            ::PostMessage(hWnd, WM_COMMAND, ID_FILE_SETTING, 0);
            return 0;
        }
        break;
    case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
        if (wmId>=ID_TASK_FIRST && wmId<=ID_TASK_LAST)
        {
            int index = wmId - ID_TASK_FIRST;
            if (CComEntity::s_EntityTable[index].pEntity!=s_pPersonator)
            {//Personate it
                s_EntityTableBackup[index].pEntity = CComEntity::s_EntityTable[index].pEntity;
                s_EntityTableBackup[index].idsys = CComEntity::s_EntityTable[index].idsys;
                CComEntity::s_EntityTable[index].pEntity = s_pPersonator;
                CComEntity::s_EntityTable[index].idsys = dwMainThreadId;
                HMENU hMenu = ::GetMenu(hWnd);
                hMenu = ::GetSubMenu(hMenu, 1);
                ::CheckMenuItem(hMenu, wmId, MF_BYCOMMAND|MF_CHECKED);
            }
            else
            {//De-personate it
                CComEntity::s_EntityTable[index].pEntity = s_EntityTableBackup[index].pEntity;
                CComEntity::s_EntityTable[index].idsys = s_EntityTableBackup[index].idsys;
                s_EntityTableBackup[index].pEntity = NULL;
                s_EntityTableBackup[index].idsys = 0xFFFFFFFF;
                HMENU hMenu = ::GetMenu(hWnd);
                hMenu = ::GetSubMenu(hMenu, 1);
                ::CheckMenuItem(hMenu, wmId, MF_BYCOMMAND|MF_UNCHECKED);
            }
            break;
        }
        if (wmId>=ID_CASE_FIRST && wmId <= ID_CASE_LAST)
        {
            if (pCaseCur!=NULL)
            {
                ::MessageBox(hWnd,"请等待前一个Case发送完毕","请稍等",MB_OK);
                break;
            }
            int index = wmId - ID_CASE_FIRST;
			CPtrList::iterator first = plstCase->begin();
			CPtrList::iterator last = plstCase->end();
            for (int i=0; i<index && first!=last; ++i)
            {
                ++first;
            }
            if (first!=last)
            {
                pCaseCur = (CCase*)*first;
                nIndexCur = 0;
                CComMessage* pComMsg = pCaseCur->GetMessage(nIndexCur);
                ::PostThreadMessage(s_dwSendThreadId, WM_SENDMSG, 0, (LPARAM)pComMsg);
                SetTimer(hWnd, 1, pCaseCur->GetDelay(nIndexCur)*1000,NULL);
            }
            break;
        }
		switch (wmId)
		{
        case ID_FILE_SETTING:
            szPath[0]=0;
            if (::GetOpenFileName(&ofn))
                ::PostMessage(hWnd,WM_COMMAND, ID_FILE_REFRESH,0);
            break;
        case ID_FILE_REFRESH:
            {
                //delete all cases
                while (!plstCase->empty())
                {
                    CCase* pCase = (CCase*)plstCase->front();
                    plstCase->pop_front();
                    delete pCase;
                }
                HMENU hMenuWnd = ::GetMenu(hWnd);
                HMENU hMenuCase = ::GetSubMenu(hMenuWnd, 2);
                if (hMenuCase!=NULL)
				{
					::DeleteMenu(hMenuWnd, 2, MF_BYPOSITION);
                    ::DestroyMenu(hMenuCase);
                }
                hMenuCase = ::CreatePopupMenu();
                //find all case files
                char szFile[MAX_PATH];
                char* p = ::strrchr(szPath, '\\');
                if (p==NULL)
                {
                    ::DestroyMenu(hMenuCase);
                    ::DrawMenuBar(hWnd);
                    break;
                }
                *(p+1) = '\0';
                ::sprintf(szFile, "%s%s",szPath,"*.cas");
                WIN32_FIND_DATA finddata;
                HANDLE hFind = ::FindFirstFile(szFile, &finddata);
                if (hFind!=INVALID_HANDLE_VALUE)
                {
                    int index=0;
                    do
                    {
                        //parse each file and add to menu
                        ::sprintf(szFile, "%s%s",szPath, finddata.cFileName);
                        CCase* pCase = CCase::ImportCase(s_pPersonator,szFile);
                        if (pCase!=NULL)
                        {
                            plstCase->push_back(pCase);
                            ::AppendMenu(hMenuCase, MF_STRING, (UINT)(ID_CASE_FIRST+index), pCase->GetName());
                            index++;
                        }
                    }
                    while (::FindNextFile(hFind,&finddata));
                    ::FindClose(hFind);
                }
                ::InsertMenu(hMenuWnd, 2,MF_BYPOSITION|MF_POPUP, (UINT)hMenuCase, "用例(&C)");
                ::DrawMenuBar(hWnd);
            }
            break;
       default:
            return ::DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
    case WM_TIMER:
        {
            unsigned int nId = (unsigned int)wParam;
            if (nId==1)
            {
                ::KillTimer(hWnd, 1);
                nIndexCur++;
                if (nIndexCur<pCaseCur->GetCount())
                {
                    CComMessage* pComMsg = pCaseCur->GetMessage(nIndexCur);
                    ::PostThreadMessage(s_dwSendThreadId, WM_SENDMSG, 0, (LPARAM)pComMsg);
                    ::SetTimer(hWnd, 1, pCaseCur->GetDelay(nIndexCur)*1000, NULL);
                }
                else
                {
                    pCaseCur = NULL;
                    nIndexCur = 0;
                }

            }
        }
        break;
	case WM_DESTROY:
        ::PostThreadMessage(s_dwSendThreadId, WM_EXIT, 0, 0);
        ::PostQuitMessage(0);
		break;
	default:
        return ::DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

TID CPersonator::GetEntityId() const
{
    return m_tid;
}

bool CPersonator::Initialize()
{
    MSG msg;
    bool bReady = false;

    if (!CBizTask::Initialize())
        return false;

    ::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    s_hWinThread = ::CreateThread(NULL,0, WinThreadProc,(LPVOID)::GetCurrentThreadId(),0,&s_dwWinThreadId);
    if (s_hWinThread==NULL)
        return false;
    
    while (!bReady)
    {
        if (::PeekMessage(&msg, NULL, WM_READY, WM_READY, PM_REMOVE|PM_QS_POSTMESSAGE) && 
           msg.message==WM_READY && (DWORD)msg.lParam==s_dwWinThreadId)
                bReady = true;
    }

    return true;
}

void CPersonator::MainLoop()
{
    for (;;)
    {
        CComMessage* pComMsg = m_pMsgQueue->GetMessage(WAIT_FOREVER);
        if (pComMsg!=NULL)
        {
            LOGMSG(LOG_DEBUG, 0,pComMsg, "A Message Received:");
            pComMsg->Destroy();
        }
    }
}

bool CPersonator::IsNeedTransaction() const
{
    return false;
}

DWORD WINAPI CPersonator::SendThreadProc(LPVOID lpParam)
{
    MSG msg;
    DWORD dwMainThreadId = (DWORD)lpParam;

    ::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    ::PostThreadMessage(dwMainThreadId, WM_READY, 0, (LPARAM)::GetCurrentThreadId());

    while (::GetMessage(&msg, NULL, 0, 0))
    {
        switch (msg.message)
        {
        case WM_SENDMSG:
            {
                CComMessage* pComMsg = (CComMessage*)msg.lParam;
                if (pComMsg!=NULL)
                {
                    DWORD dwDataLen = pComMsg->GetDataLength();
                    CComMessage* pComMsgNew = new (s_pPersonator, dwDataLen) CComMessage;
                    if (pComMsgNew!=NULL)
                    {
                        pComMsgNew->SetDstTid(pComMsg->GetDstTid());
                        pComMsgNew->SetSrcTid(pComMsg->GetSrcTid());
                        pComMsgNew->SetMessageId(pComMsg->GetMessageId());
#if !(M_TARGET==M_TGT_CPE)
                        pComMsgNew->SetEID(pComMsg->GetEID());
                        pComMsgNew->SetUID(pComMsg->GetUID());
#endif

                        //pComMsgNew->SetSFID(pComMsg->GetSFID());
                        ::memcpy(pComMsgNew->GetDataPtr(), pComMsg->GetDataPtr(), dwDataLen);
                        //pComMsgNew->SetUrgent(pComMsg->IsUrgent());
#if !(M_TARGET==M_TGT_CPE)
                        pComMsgNew->SetDirection(pComMsg->GetDirection());
                        pComMsgNew->SetIpType(pComMsg->GetIpType());
                        pComMsgNew->SetBtsAddr(pComMsg->GetBtsAddr());
						if (pComMsg->GetDhcpPtr()!=NULL)
                        {
                            DWORD offset = (char*)(pComMsg->GetDhcpPtr()) - (char*)(pComMsg->GetDataPtr());
                            pComMsgNew->SetDhcpPtr((char*)(pComMsgNew->GetDataPtr()) + offset);
						}
                        if (pComMsg->GetUdpPtr()!=NULL)
                        {
                            DWORD offset = (char*)(pComMsg->GetUdpPtr()) - (char*)(pComMsg->GetDataPtr());
                            pComMsgNew->SetUdpPtr((char*)(pComMsgNew->GetDataPtr()) + offset);
						}
                        if (pComMsg->GetKeyMac()!=NULL)
                        {
                            DWORD offset = (unsigned char*)(pComMsg->GetKeyMac()) - (unsigned char*)(pComMsg->GetDataPtr());
                            pComMsgNew->SetKeyMac((unsigned char*)(pComMsgNew->GetDataPtr()) + offset);
						}
#endif
                        ::CComEntity::PostEntityMessage(pComMsgNew,NO_WAIT,true);
                    }
                }
            }
            break;
        case WM_EXIT:
            ::PostQuitMessage(0);
            break;
        }
    }
    return msg.wParam;
}

#else
#error "This module can only be used on Windows platform."
#endif //__WIN32_SIM__

#endif //WIN32
