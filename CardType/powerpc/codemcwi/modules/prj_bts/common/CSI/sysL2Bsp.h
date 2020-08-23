/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:        sysBspCsi.h
 *
 * DESCRIPTION:   define the data structures and data types used for CSI
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   02/15/2007   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef SYS_BSP_CSI_H__
#define SYS_BSP_CSI_H__

#include "vxworks.h"

#define DEVICE_RAMDISK		"/RAMDL2/"						/*  "/RAMDISK/"   */
#define RAMDISK_BLOCK_SIZE	(1024*32)    /***(1024*32)* 512 = 16M*******/

typedef enum
{
    RESET_SYSTEM_L2_SELF_RESET = 0x11,
    RESET_SYSTEM_RESET_BY_L3 = 0x22
}SW_RESET_FLAG;


typedef enum
{
    RESET_REASON_HW_WDT = 20,
    RESET_REASON_SW_WDT,
    RESET_REASON_POWER_ON,
    RESET_REASON_SW_NORMAL,
    RESET_REASON_SW_ALARM,
    RESET_REASON_SW_ABNORMAL,
    RESET_REASON_SW_INIT_FAIL,
    RESET_REASON_L3_REBOOT,
    RESET_REASON_SW_INIT_DOWNFPGA_FAIL,
    RESET_REASON_SW_INIT_EMSTIMEOUT_FAIL,
    RESET_REASON_SW_INIT_LOADCODETIMEOUT_FAIL,
    RESET_REASON_SW_INIT_NVRAMBOOT_FAIL,
    RESET_REASON_ARPNOT_GTAEWAY,
    RESET_REASON_PM_CREATE_FAIL,
    RESET_REASON_FTPC_CREATE_FAIL,
    RESET_REASON_NETMBUFFER_FULL,
    RESET_REASON_L3BOOTLINE_DIFF,
    RESET_REASON_DSPERR_IN5MIN
    
}RESET_REASON;

#define NVRAM_VALID_PATTERN 0x55aa55aa
#define L2_PCI_IF_L2_REQUEST_REBOOT_IF   0x99999999  /* should be the same as defined in pciIf.h  */
#define L2_PCI_IF_L2_REQUEST_REBOOT_CSI  0x9999999A
#define L2_PCI_IF_L2_REQUEST_REBOOT_CMD  0x9999999B



typedef struct 
{
	unsigned int    nvramSafe;   /*****0x55aa55aa*********/
    BOOL            isResetReasonSet;
    RESET_REASON    L2RstReason;
	unsigned int    resetFlag;
}T_L2BootState;


typedef struct
{
    UINT16  year;		
    UCHAR   month;
    UCHAR   day;
    UCHAR   hour;		
    UCHAR   minute;		
    UCHAR   second;		
}T_TimeDate;


#ifdef __cplusplus
extern "C"
{
#endif

int bspEnableNvRamWrite(char *startAddr, UINT32 size);
int bspDisableNvRamWrite(char *startAddr, UINT32 size);
RESET_REASON bspGetBtsResetReason();
void bspSetBtsResetReason(RESET_REASON reason);
void bspSetResetFlag(int flag);
void RequestL3ToResetL2(int);
T_TimeDate bspGetDateTime();
void bspSetDateTime(T_TimeDate * newTime);
void DiscoverLastResetReason();
void sysWdtCallbackInstall(FUNCPTR func, UINT32 arg,UINT32 period);
void PetHwWatchdog();
void RegisterRebootCallbackFunc( FUNCPTR func);

#ifdef __cplusplus
}
#endif



#endif  //SYS_BSP_CSI_H__
