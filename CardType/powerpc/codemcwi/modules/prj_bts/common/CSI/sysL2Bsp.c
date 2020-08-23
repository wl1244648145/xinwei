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
#include "mcWill_bts.h"
#include "mv64360.h"
#include <dosFsLib.h>
#include <stat.h>
#include <ramDrv.h>
#include <time.h>



FUNCPTR WdtCallbackRoutine = NULL;
UINT32  WdtCallbackRoutineArg = 0;
UINT32  SysWdtTaskId = 0;
UINT32  WdtCallbackRoutinePeriodInTick = 10;
#define M_TP_CSI_WDT               0
#define M_TP_CSI_WDT_INIT          50

#define M_BOOT_STATE_SAFE_PATTERN 0x12345678

void sysWdtCallbackInstall(FUNCPTR func, UINT32 arg,UINT32 period)
{
    if ( NULL == WdtCallbackRoutine )
    {
        WdtCallbackRoutine = func;
        WdtCallbackRoutineArg = arg;
        WdtCallbackRoutinePeriodInTick = period;
        if (SysWdtTaskId)
        {
            taskPrioritySet(SysWdtTaskId, M_TP_CSI_WDT);
        }
    }
}

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

	    taskDelay(WdtCallbackRoutinePeriodInTick);   /* delay for 100 ms */
	}
}

void StartWdtTask()
{
    SysWdtTaskId = taskSpawn ("tWatchDog", M_TP_CSI_WDT_INIT, 0, 10240, (FUNCPTR)WatchdogTask, 0,0,0,0,0,0,0,0,0,0);
    printf("Start Software Watchdog task .........................Done\n");
}


int bspEnableNvRamWrite(char *startAddr, UINT32 size)
{
    return TRUE;
}
int bspDisableNvRamWrite(char *startAddr, UINT32 size)
{
    return TRUE;
}


T_L2BootState *L2BootState = (T_L2BootState*)MV_SRAM_RESET_INFO_ADRS;
T_TimeDate *L2SystemTime = (T_TimeDate*)MV_SRAM_BOOT_TIME_ADDR;
T_TimeDate bspGetDateTime()
{
    T_TimeDate pDateTime;

    memcpy(&pDateTime, L2SystemTime, sizeof(T_TimeDate));
    return pDateTime;	
}

void bspSetDateTime(T_TimeDate * newTime)
{
    memcpy(L2SystemTime, newTime, sizeof(T_TimeDate));
}


void DiscoverLastResetReason()
{
    struct tm time_s;
    T_TimeDate timeDate;
    struct timespec timeSpec;
    unsigned int secCount;

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


    /* set system time using time passed by L3 */
    timeDate = bspGetDateTime();
    time_s.tm_sec = timeDate.second;
    time_s.tm_min = timeDate.minute;
    time_s.tm_hour = timeDate.hour;

    time_s.tm_mday = timeDate.day;
    time_s.tm_mon = timeDate.month;
    time_s.tm_year = timeDate.year;
    time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

    secCount = mktime(&time_s);
    timeSpec.tv_sec = secCount;    
    timeSpec.tv_nsec = 0;
    clock_settime(CLOCK_REALTIME, &timeSpec);

}


RESET_REASON bspGetBtsResetReason()
{
    return L2BootState->L2RstReason;
}


void bspSetBtsResetReason(RESET_REASON reason)
{
    if (! L2BootState->isResetReasonSet)
    {
        L2BootState->isResetReasonSet = TRUE;
        L2BootState->L2RstReason = reason;
    }
}

void bspSetResetFlag(int flag)
{
    L2BootState->resetFlag = flag;
}

FUNCPTR RebootCallbackFunc = NULL;
void RequestL3ToResetL2(int reason)
{
    *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_MSG0_CPU0) = 
                        LONGSWAP (reason);   /**clear interruptr**/			
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

void RegisterRebootCallbackFunc( FUNCPTR func)
{
    if ( NULL == RebootCallbackFunc )
    {
        RebootCallbackFunc = func;
    }
}

STATUS bspCreateRamDisk()
{
    BLK_DEV               *pBlkDev; 
    DOS_VOL_DESC          *pVolDesc; 

    pBlkDev = ramDevCreate ((char *)0, 512, 1024, RAMDISK_BLOCK_SIZE/*1024*32*/, 0); /**512* (1024*32) = 16M*********/      
    if ( (pVolDesc = dosFsDevInit(DEVICE_RAMDISK, pBlkDev, NULL)) == NULL ) 
        return ERROR;
    if ( dosFsVolFormat((char *)DEVICE_RAMDISK, 0x0004, NULL) != OK )  
        return ERROR;

    cd(DEVICE_RAMDISK);

    return OK;
}

