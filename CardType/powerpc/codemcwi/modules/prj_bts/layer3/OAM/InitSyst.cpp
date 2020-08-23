#ifndef _INC_AFWINIT
#include "AfwInit.h"
#endif

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif


#ifndef __WIN32_SIM__
#include "L3TaskDiag.h"
#include "l3BootTask.h"
#endif


#ifndef _INC_L3OAMSYSTEM
#include "L3OamSystem.h"
#endif



bool InitL3Oam()
{
    CTaskSystem *pSys = CTaskSystem ::GetInstance();
    pSys->Begin();
	
#ifndef __WIN32_SIM__
    L3BootTask  *pBoot = L3BootTask::GetInstance();
    pBoot->Begin();

    CL3TaskDiag *diagTask = CL3TaskDiag::GetInstance();
    diagTask->Begin();
#endif
    pSys->SYS_getLocalCfgFromCF();
    return true;
}



#ifdef __WIN32_SIM__
#include <windows.h>
#endif

#ifdef M_TGT_L2
extern "C" void PreAfwInit();
#endif

int main1(void)
{
#ifdef __WIN32_SIM__
    HANDLE hbufNew = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hbufNew!=INVALID_HANDLE_VALUE)
    {
        COORD coord;
        coord.X = 120;
        coord.Y = 3000;
        ::SetConsoleScreenBufferSize(hbufNew, coord);
    }
#endif

#ifdef M_TGT_L2
    PreAfwInit();
#endif

    int iRet = !AfwInit();

#ifdef __WIN32_SIM__
    ::Sleep(WAIT_FOREVER);
#endif

    return iRet;
}


