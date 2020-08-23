#include <msgQLib.h>
#include <string.h>
#include <stdio.h>
#include "UtBridgeTest.h"
#include "log.h"
#include "UTBridge.h"


#include <taskLib.h>

#include "sysDma.h"
#include "AfwInit.h"
#include "PciIf.h"
#include "BtsVacSim.h"

void brmain()
{
    AfwInit();
    CTaskPciIf *pciIf = CTaskPciIf::GetInstance();
    CUTBridge *bridgeIf = CUTBridge::GetInstance(); 
    CBTSVACSim *vacSim = CBTSVACSim::GetInstance();

    pciIf->Begin();
    bridgeIf->Begin();
}

extern "C"
void brtest()
{
    taskSpawn ("tBrInit", 50, 0, 5120, (FUNCPTR)brmain, 0,0,0,0,0,0,0,0,0,0);
}


bool InitL3DataSvc()
{    
    return true;
}

bool InitL3VoiceSvc()
{   
    return true;
}


bool InitL3Oam(void)
{
    return true;
}


