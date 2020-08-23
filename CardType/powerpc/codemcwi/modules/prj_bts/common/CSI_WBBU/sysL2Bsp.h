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

#include "btsTypes.h"
#include "os.h"

#define DEVICE_RAMDISK		"/RAMDL2/"						/*  "/RAMDISK/"   */
#define RAMDISK_BLOCK_SIZE	(1024*32)    /***(1024*32)* 512 = 16M*******/

#if defined CORE0
#define CSI_IMAGE_ADDRESS       (0x83F00000)
#elif defined CORE1 
#define CSI_IMAGE_ADDRESS       (0x84F00000)
#endif
#define CSI_IMAGE_SIZE          (0x000F0000) // 960K
#define MV_SRAM_BOOT_TIME_ADDR (0x86FF0000)
#define MV_SRAM_RESET_INFO_ADRS (CSI_IMAGE_ADDRESS+CSI_IMAGE_SIZE)

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

#define L1PMPFAR                            (0x0184A400)
#define L1PMPFSR                            (0x0184A404)
#define L1DMPFAR                            (0x0184AC00)
#define L1DMPFSR                            (0x0184AC04)
#define L2MPFAR                             (0x0184A000) 
#define L2MPFSR                             (0x0184A004)
#define L1PMPFAR_VALUE                     (*((volatile UINT32*)L1PMPFAR))
#define L1PMPFSR_VALUE                     (*((volatile UINT32*)L1PMPFSR))
#define L1DMPFAR_VALUE                     (*((volatile UINT32*)L1DMPFAR))
#define L1DMPFSR_VALUE                     (*((volatile UINT32*)L1DMPFSR))
#define L2MPFAR_VALUE                      (*((volatile UINT32*)L2MPFAR)) 
#define L2MPFSR_VALUE                      (*((volatile UINT32*)L2MPFSR))  
typedef struct 
{
	unsigned int    nvramSafe;   /*****0x55aa55aa*********/
    Bool            isResetReasonSet;
    RESET_REASON    L2RstReason;
	unsigned int    resetFlag;
}T_L2BootState;


typedef struct
{
    UINT16  year;		
    UINT8   month;
    UINT8   day;
    UINT8   hour;		
    UINT8   minute;		
    UINT8   second;		
}T_TimeDate;
    
typedef struct
{
    UINT32 sp;
    UINT32 pc;
    UINT32 ier;
}DSP_CPU_REGISTER;

typedef struct
{
    UINT32 nrp;
    UINT32 efr;
    UINT32 mpFsr[3];
    UINT32 mpFar[3];
}DSP_EXC_REGISTER;

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
//void sysWdtCallbackInstall(FUNCPTR func, UINT32 arg,UINT32 period);
void PetHwWatchdog();
//void RegisterRebootCallbackFunc( FUNCPTR func);
void getCpuRegister(DSP_CPU_REGISTER *reg);
void getExcRegister(DSP_EXC_REGISTER *reg);
#ifdef __cplusplus
}
#endif



#endif  //SYS_BSP_CSI_H__
