#ifndef _INC_AFWMAIN
#include "AfwMain.h"
#endif

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifdef M_TGT_L3
#ifndef _INC_DEMOTASK1
#include "DemoTask1.h"
#endif

#ifndef _INC_DEMOTASK2
#include "DemoTask2.h"
#endif

#include "Timer.h"

bool InitL3DataSvc()
{
    return true;
}

bool InitL3VoiceSvc()
{
    return true;
}

bool InitL3Oam()
{
    LOG(LOG_CRITICAL,0,"InitL3Oam\n");
    CTask* pTask = new CDemoTask1;
    if (pTask==NULL || !(pTask->Begin()))
    {
        LOG(LOG_CRITICAL,0,"Task1 failed start up\n");
        return false;
    }
    else
        LOG(LOG_CRITICAL,0,"Task1 start up\n");
    
    pTask = new CDemoTask2;
    if (pTask==NULL || !pTask->Begin())
        return false;
    else
        LOG(LOG_CRITICAL,0,"Task2 start up\n");
    
 //   CTimer* pTimer = new CTimer(true, 1000, NULL);
  //  pTimer->Start();
    return true;
}

#endif

#ifdef WBBU_CODE
#define PRJ_BUILD
extern "C"  void initdbglvl();
#endif

bool AfwMain()
{
#ifdef PRJ_BUILD

#ifdef WBBU_CODE
	initdbglvl();
#endif
LOG(LOG_CRITICAL,0,"L3 initdbglvl after\n");

#if (M_TARGET==M_TGT_L3)

    if (!InitL3Oam())
    {
        LOG(LOG_CRITICAL,0,"L3 OAM Start failed.");
        //Restart System
        return false;
    }
    LOG(LOG_CRITICAL,0,"OAM tasks start SUCCESS.");

    if (!InitL3DataSvc())
    {
        LOG(LOG_CRITICAL,0,"Data Service tasks start failed.");
        //Restart System
        return false;
    }
    LOG(LOG_CRITICAL,0,"Data Service tasks start SUCCESS.");
    
  //EnableVlanEndInterface();
  
    LOG(LOG_CRITICAL,0,"VLanEnd Interface start SUCCESS.");

    if (!InitL3VoiceSvc())
    {
        LOG(LOG_CRITICAL,0,"Voice Service tasks start failed.");
        //Restart System
        return false;
    }
    LOG(LOG_CRITICAL,0,"Voice Service tasks start SUCCESS.");

#else
    #error "You must define a valid M_TARGET"
#endif

#endif  // PRJ_BUILD
    return true;
}

