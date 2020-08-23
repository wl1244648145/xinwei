#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif
/*
#ifndef _INC_LOG
#include "Log.h"
#endif

extern bool InitL3Oam();
extern bool InitL3DataSvc();
extern bool InitL3VoiceSvc();

#include "AfwInit.h"
*/
#include "shellLib.h"
#include "stdio.h"
#include "taskLib.h"

extern void pre_loader();

extern "C"
int L3main()
{
//    AfwInit();
    shellPromptSet("L3xx->");

    printf("main running!!\n");

    pre_loader();
 
#if 0
#ifdef PRJ_BUILD
    if (!InitL3DataSvc())
    {
        LOG(LOG_CRITICAL,0,"L3 Data Service Start failed.");
        //Restart System
        return false;
    }
    if (!InitL3VoiceSvc())
    {
        LOG(LOG_CRITICAL,0,"L3 Voice Service Start failed.");
        //Restart System
        return false;
    }
    if (!InitL3Oam())
    {
        LOG(LOG_CRITICAL,0,"L3 OAM Start failed.");
        //Restart System
        return false;
    }
#endif  // PRJ_BUILD
#endif
    return true;
}
