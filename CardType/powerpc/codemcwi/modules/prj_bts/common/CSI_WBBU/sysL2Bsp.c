/*******************************************************************
*
*    DESCRIPTION: sysL2BspCsi.cpp - This file implements the BSP routines for CSI.
*
*    AUTHOR: Yushu Shi
*
*    HISTORY:
*
*    DATE: 2/15/2007
*
*******************************************************************/
#include "sysL2BspCsi.h"
#include <time.h>
#include <bios.h>
#include <bcache.h>

Fxn WdtCallbackRoutine = NULL;
UINT32  WdtCallbackRoutineArg = 0;
UINT32  SysWdtTaskId = 0;
UINT32  WdtCallbackRoutinePeriodInTick = 10;
#define M_TP_CSI_WDT               0
#define M_TP_CSI_WDT_INIT          50

#define M_BOOT_STATE_SAFE_PATTERN 0x12345678

extern void getTime(void * time);

void PetHwWatchdog()
{
    
}

void WatchdogTask()
{
	while(1)
    {
        /*PetHwWatchdog();  */

        if (WdtCallbackRoutine)
        {
            WdtCallbackRoutine(WdtCallbackRoutineArg);
        }

	    TSK_sleep(WdtCallbackRoutinePeriodInTick);   /* delay for 100 ms */
	}
}

void StartWdtTask()
{
    //SysWdtTaskId = taskSpawn ("tWatchDog", M_TP_CSI_WDT_INIT, 0, 10240, (FUNCPTR)WatchdogTask, 0,0,0,0,0,0,0,0,0,0);
    printf("Start Software Watchdog task .........................Done\n");
}


int bspEnableNvRamWrite(char *startAddr, UINT32 size)
{
    BCACHE_inv(startAddr, size, TRUE);
    return TRUE;
}
int bspDisableNvRamWrite(char *startAddr, UINT32 size)
{
    BCACHE_wbInv(startAddr, size, TRUE);
    return TRUE;
}

T_L2BootState *L2BootState = (T_L2BootState*)MV_SRAM_RESET_INFO_ADRS;
T_TimeDate bspGetDateTime()
{
    T_TimeDate pDateTime;
    
    getTime(&pDateTime);

    return pDateTime;	
}

void bspSetDateTime(T_TimeDate * newTime)
{
    //memcpy(L2SystemTime, newTime, sizeof(T_TimeDate));
}


void DiscoverLastResetReason()
{
    BCACHE_inv(L2BootState, sizeof(T_L2BootState), TRUE);
    
    if (L2BootState->nvramSafe != M_BOOT_STATE_SAFE_PATTERN )
    {
        L2BootState->nvramSafe = M_BOOT_STATE_SAFE_PATTERN;
        L2BootState->L2RstReason = RESET_REASON_POWER_ON;
    }
    else if (L2BootState->resetFlag == RESET_SYSTEM_RESET_BY_L3)
    {
        L2BootState->L2RstReason = RESET_REASON_L3_REBOOT;
    }

    L2BootState->isResetReasonSet = FALSE;
    L2BootState->resetFlag = RESET_SYSTEM_RESET_BY_L3;

    BCACHE_wb(L2BootState, sizeof(T_L2BootState), TRUE);
}


RESET_REASON bspGetBtsResetReason()
{
    return L2BootState->L2RstReason;
}


void bspSetBtsResetReason(RESET_REASON reason)
{
    if (! L2BootState->isResetReasonSet)
    {
        BCACHE_inv(L2BootState, sizeof(T_L2BootState), TRUE);
        L2BootState->isResetReasonSet = TRUE;
        L2BootState->L2RstReason = reason;
        BCACHE_wb(L2BootState, sizeof(T_L2BootState), TRUE);
    }
}

void bspSetResetFlag(int flag)
{
    BCACHE_inv(L2BootState, sizeof(T_L2BootState), TRUE);
    L2BootState->resetFlag = flag;
    BCACHE_wb(L2BootState, sizeof(T_L2BootState), TRUE);
}

Fxn RebootCallbackFunc = NULL;
void RequestL3ToResetL2(int reason)
{
}

void L2RebootHook()
{
    if (RebootCallbackFunc )
    {
        RebootCallbackFunc();   /* should allow CSI to take over and reset the BTS */
    }
    else
    {
        RequestL3ToResetL2(L2_PCI_IF_L2_REQUEST_REBOOT_CMD);
    }
}

void RegisterRebootCallbackFunc( Fxn func)
{
    if ( NULL == RebootCallbackFunc )
    {
        RebootCallbackFunc = func;
    }
}

void getCpuRegister(DSP_CPU_REGISTER *reg)
{
    //extern cregister volatile unsigned int PC;
    extern cregister volatile unsigned int IER;
    
    //reg->pc = PC;
    reg->ier = IER;
}

void getExcRegister(DSP_EXC_REGISTER *reg)
{
    extern cregister volatile unsigned int NRP;
    extern cregister volatile unsigned int EFR;
    reg->nrp = NRP;
    reg->efr = EFR;
    reg->mpFar[0] = L1PMPFAR_VALUE;
    reg->mpFsr[0] = L1PMPFSR_VALUE;
    reg->mpFar[1] = L1DMPFAR_VALUE;
    reg->mpFsr[1] = L1DMPFSR_VALUE;
    reg->mpFar[2] = L2MPFAR_VALUE;
    reg->mpFsr[2] = L2MPFSR_VALUE;
}
