/* config.h - Wind River SBC8548 BSP configuration file */

/*
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01l,21sep07,b_m  add ns16550, ppcIntCtlr, epic, m85xxTimer vxbus drivers.
01k,17sep07,h_k  removed INCLUDE_PCI_BUS_SHOW. (CQ:105028)
01j,10sep07,wap  Switch to VxBus PCI driver (WIND00104076)
01i,21aug07,mmi  remove legacy power management define
01h,07aug07,dtr  Update for latest Rio Driver.
01g,31jul07,agf  inc BSP_REV for vxWorks 6.6; change tsec drivers from hEND 
                 to END
01f,25may07,b_m  add BOOT_FLASH macro to support boot device selection.
01e,23may07,b_m  add MAX_MAC_DEVS macro.
01d,06apr07,b_m  modify to use m85xxCCSR driver.
01c,27mar07,b_m  add MPC8548 rev.2 and TFFS define.
01b,27feb06,kds  removing INCLUDE_SHOW_ROUTINES
01a,30jan06,kds  Modify from cds8548/config.h/01d
*/

#ifndef	__INCconfigh
#define	__INCconfigh

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#define BSP_VER_1_1     1
#define BSP_VER_1_2     1
#define BSP_VERSION     "2.0"
#define BSP_REV         "/4"

/* for MPC8548 rev.2 silicon */
#define REV2_SILICON

/* boot flash selection */
#define ON_BOARD_FLASH      1
#define SODIMM_FLASH        2
#define BOOT_FLASH          ON_BOARD_FLASH

#define SYS_MODEL           "Wind River SBC"

#define SYS_MODEL_8548E     "Wind River SBC8548E - Security Engine"
#define SYS_MODEL_8548      "Wind River SBC8548"
#define SYS_MODEL_8547E     "Wind River SBC8547E - Security Engine"
#define SYS_MODEL_8545E     "Wind River SBC8545E - Security Engine"
#define SYS_MODEL_8545      "Wind River SBC8545"
#define SYS_MODEL_8543E     "Wind River SBC8543E - Security Engine"
#define SYS_MODEL_8543      "Wind River SBCC8543"

#define SYS_MODEL_E500      "Freescale E500 : Unknown system version"
#define SYS_MODEL_UNKNOWN   "Freescale Unknown processor"

#define INCLUDE_AUX_CLK

/* Define Clock Speed and source  */

#define	FREQ_33_MHZ	 33000000
#define	FREQ_66_MHZ	 66000000
#define	FREQ_100_MHZ	 99999999
#define	FREQ_133_MHZ	133333333
#define	FREQ_266_MHZ	266666666
#define FREQ_400_MHZ    400000000
#define	FREQ_333_MHZ	333333333
#define	FREQ_533_MHZ	533333333

#define	OSCILLATOR_FREQ	FREQ_66_MHZ

/*
 * DDR will work only for 400/533Mhz at the moment
 * These ratios are tested in bootrom for correct init
 * boot sector too small for 333 ddr init as well so ifdef'd out for now.
 */

#if (OSCILLATOR_FREQ == FREQ_33_MHZ)
#define PLAT_RATIO_533_MHZ 16
#define PLAT_RATIO_400_MHZ 12
#define PLAT_RATIO_333_MHZ 8 /* Unsupported */
#endif
#if (OSCILLATOR_FREQ == FREQ_66_MHZ)
#define PLAT_RATIO_533_MHZ 8
#define PLAT_RATIO_400_MHZ 6
#define PLAT_RATIO_333_MHZ 4 /* Unsupported */
#endif

/* ECC not supported on wrSbc8548 */

#undef INCLUDE_DDR_ECC
#define DEFAULT_SYSCLKFREQ FREQ_533_MHZ
/*#define DEFAULT_SYSCLKFREQ FREQ_400_MHZ*/

#undef FORCE_DEFAULT_FREQ /* Use to force freq used with DUART/Timers etc */

/* This value is the 60x bus-assigned SDRAM Refresh Timer PSRT setting */

#define	LSRT_VALUE	0x20

/*
 * This value is the setting for the MPTPR[PTP] Refresh timer prescaler.
 * The value is dependent on the OSCILLATOR_FREQ value.  For other values
 * a conditionally compiled term must be created here for that OSCILLATOR_FREQ
 * value.
 *
 * BRGCLK_DIV_FACTOR
 * Baud Rate Generator division factor - 0 for division by 1
 *					 1 for division by 16
 */

#define	DIV_FACT_1	0
#define	DIV_FACT_16	1

/*
 * Assume 533MHz CCB with local bus clk ratio of 8
 */

#define	TPR			0x0000

#include <configAll.h>

#include "wrSbc8548.h"

/**when boot from NVram,pls add NVRAM_BOOT definition**/
/*#define NVRAM_BOOT*/
#undef NVRAM_BOOT 

/*#define INCLUDE_DEMO*/
/*DEMO add by xcl */
/* Common Includes for VXBUS RIO and ETSEC */

#define INCLUDE_VXBUS
#define INCLUDE_VXB_CMDLINE

/**************wangwenhua migrate from V5**********************/
#define INCLUDE_ATA
#define INCLUDE_ATA_SHOW
#define  INCLUDE_RTC_NVRAM_SUPPORT           /* RTC_NVRAM support */
#define  INCLUDE_I2C_SUPPORT           /* I2C support */

#undef INCLUDE_RAPIDIO_BUS

/*
 * RAPIDIO supports only point to point shared memory support
 * This includes TIPC and SM objects
 */

#ifdef INCLUDE_RAPIDIO_BUS
#define INCLUDE_SM_COMMON
#define INCLUDE_M85XX_RAPIDIO
#define INCLUDE_M85XX_CPU
#define DRV_RESOURCE_M85XXCCSR
#define INCLUDE_M85XX_RIO_SM_CFG
#define INCLUDE_VXBUS_SM_SUPPORT
#define RAPIDIO_BUS_STATIC_TABLE
#define VXBUS_TABLE_CONFIG
#endif


#ifdef INCLUDE_VXBUS
#define INCLUDE_VXBUS_INIT
#define INCLUDE_HWMEM_ALLOC
#define INCLUDE_VXBUS_ACTIVATE
#define INCLUDE_PARAM_SYS
#define INCLUDE_PLB_BUS
#define INCLUDE_VXBUS_SHOW
#define INCLUDE_ETSEC_VXB_MEND/**wangwenhua add 20090319*/
#undef  INCLUDE_TSEC_VXB_END 

/*******************by xcl*************************/
/*#define INCLUDE_OPTIONAL_TSECS*/
/**************************************************/

#define INCLUDE_MII_BUS
#define INCLUDE_GENERICPHY
#define INCLUDE_END
#define HWMEM_POOL_SIZE 50000
#define DRV_SIO_NS16550
#define INCLUDE_SIO_UTILS
#define INCLUDE_INTCTLR_LIB
#define DRV_INTCTLR_PPC
#define DRV_INTCTLR_EPIC
#define DRV_TIMER_M85XX
#endif

#ifdef DRV_TIMER_M85XX
#define INCLUDE_TIMER_SYS
#define INCLUDE_TIMER_STUB
#endif

/*
 * PCI bus support, off by default
 *

#define INCLUDE_PCI_BUS
 */

#ifdef INCLUDE_PCI_BUS

#define DRV_PCIBUS_M85XX
#define DRV_RESOURCE_M85XXCCSR
#define INCLUDE_PCI_BUS_AUTOCONF

/* Intel PRO/1000 ethernet support */

#define INCLUDE_GEI825XX_VXB_END
#define INCLUDE_GEITBIPHY

#endif /* INCLUDE_PCI_BUS */

#if 0

  #define INCLUDE_IFMEDIA
  #define INCLUDE_VXBUS_SHOW
  #define INCLUDE_IFCONFIG
  #define INCLUDE_NET_IF_SHOW
  #define INCLUDE_SHOW_ROUTINES
  #define INCLUDE_ISR_OBJECTS

#endif

/*
 * Other useful includes:
 * #define INCLUDE_IFMEDIA
 * #define INCLUDE_VXBUS_SHOW
 * #define INCLUDE_IFCONFIG
 * #define INCLUDE_NET_IF_SHOW
 * #define INCLUDE_SHOW_ROUTINES
 * #define INCLUDE_PCI_CFGSHOW
 * #define INCLUDE_ISR_OBJECTS
 */

/*
 * Need LINKHDR extension to support FCB in ETSEC to
 * reduce FCB insertion requiring it's own BD
 */

#undef MAX_LINKHDR_CFG
#define MAX_LINKHDR_CFG       32

#define WDT_RATE_MIN         (sysTimerClkFreq / (1 << 29))
#define WDT_RATE_MAX         (sysTimerClkFreq / (1 << 21))

#define DEFAULT_BOOT_LINE \
"motetsec(0,0)host:vxWorks h=192.168.0.1 e=192.168.0.2 \
u=vxworks pw=vxworks f=0x0"
#define INCLUDE_END

/* MMU and CACHE */

#define INCLUDE_MMU_BASIC
#define USER_I_MMU_ENABLE
#define USER_D_MMU_ENABLE

#undef E500_L1_PARITY_RECOVERY

#ifdef E500_L1_PARITY_RECOVERY

  /*
   * *** NOTE FOR PROJECT FACILITY USERS ***
   * Needs to use WRITETHROUGH, building with Project Facility must also
   * change USER_D_CACHE_MODE and USER_I_CACHE_MODE in Project Facility.
   */

# define CACHE_LIBRARY_MODE     CACHE_WRITETHROUGH
# define CAM_DRAM_CACHE_MODE    _MMU_TLB_ATTR_W
# define TLB_CACHE_MODE         VM_STATE_CACHEABLE_WRITETHROUGH

#else  /* E500_L1_PARITY_RECOVERY */

# define CACHE_LIBRARY_MODE     (CACHE_COPYBACK | CACHE_SNOOP_ENABLE)
# define CAM_DRAM_CACHE_MODE    _MMU_TLB_ATTR_M
# define TLB_CACHE_MODE         VM_STATE_CACHEABLE | VM_STATE_MEM_COHERENCY

#endif

#define INCLUDE_CACHE_SUPPORT
#define USER_D_CACHE_ENABLE
#undef  USER_D_CACHE_MODE
#define USER_D_CACHE_MODE  (CACHE_LIBRARY_MODE)
#define USER_I_CACHE_ENABLE
#undef  USER_I_CACHE_MODE
#define USER_I_CACHE_MODE (CACHE_LIBRARY_MODE)

#define  INCLUDE_L2_CACHE
#undef   INCLUDE_L2_SRAM

  /*
   * If E500_L1_PARITY_RECOVERY is not efined, use local BSP handler.
   * Works for L1 instr cache but not data cache.  Writethrough not needed.
   */

#ifdef E500_L1_PARITY_RECOVERY

# if defined(INCLUDE_CACHE_SUPPORT) && defined(USER_I_CACHE_ENABLE)
#   if (defined(_WRS_VXWORKS_MAJOR) && (_WRS_VXWORKS_MAJOR >= 6))
#     define INCLUDE_L1_IPARITY_HDLR              /* VxWorks 6.x */
#   else  /* _WRS_VXWORKS_MAJOR */
#     define INCLUDE_L1_IPARITY_HDLR_INBSP        /* VxWorks 5.5.x */
#   endif /* _WRS_VXWORKS_MAJOR */
# endif  /* INCLUDE_CACHE_SUPPORT && USER_I_CACHE_ENABLE */

#endif  /* E500_L1_PARITY_RECOVERY */

#define INCLUDE_BRANCH_PREDICTION

#if ((defined(INCLUDE_L2_CACHE)) && (defined(INCLUDE_L2_SRAM)))
#define L2_CACHE_SIZE      L2SIZ_256KB
#define L2_SRAM_SIZE       L2SIZ_256KB
#elif ((defined(INCLUDE_L2_CACHE)) && (!defined(INCLUDE_L2_SRAM)))
#define L2_CACHE_SIZE      L2SIZ_512KB
#define L2_SRAM_SIZE       0         /* Not Used */
#else
#define L2_SRAM_SIZE       L2SIZ_512KB
#define L2_CACHE_SIZE      0         /* Not Used */
#endif

#define L2SRAM_ADDR        0x7FF80000
#define L2SRAM_WINDOW_SIZE 0x80000

/* Disable Support for SPE 64bit registers */

#define INCLUDE_SPE

/* TSEC is included */

#define INCLUDE_MOT_TSEC_END

#ifdef INCLUDE_MOT_TSEC_END
#define INCLUDE_PRIMARY_TSEC_END
#define INCLUDE_SECONDARY_TSEC_END
#undef INCLUDE_TERTIARY_TSEC_END
#undef INCLUDE_QUARTINARY_TSEC_END
#define INCLUDE_END
#endif /* INCLUDE_MOT_TSEC_END */

/* Serial channel and TTY */

#undef  NUM_TTY
#define NUM_TTY 2

/* Clock rates */

#define	SYS_CLK_RATE_MIN	1	/* minimum system clock rate */
#define	SYS_CLK_RATE_MAX	8000	/* maximum system clock rate */
#define	AUX_CLK_RATE_MIN	((CCB_FREQ / 16) / (1 << FIT_DIVIDER_TAP_21))	/* minimum auxiliary clock rate */
#define	AUX_CLK_RATE_MAX	((CCB_FREQ / 16) / (1 << FIT_DIVIDER_TAP_10))	/* maximum auxiliary clock rate */
#define AUX_CLK_RATE        ((CCB_FREQ / 16) / (1 << FIT_DIVIDER_TAP_18))

/*
 * Although the support for point to point has been added it is untested
 * with Serial RIO. SImilar was tested on parallel RIO
 */

#ifdef INCLUDE_RAPIDIO_BUS

/* RapidIO information */

#define RAPIDIO_REG_BASE 0xc0000
#define RAPIDIO_BASE   (CCSBAR + RAPIDIO_REG_BASE)
#define RAPIDIO_ADRS   0xc0000000
#define RAPIDIO_SIZE   0x10000000
#define RAPIDIO_BUS_ADRS 0x80000000
#define RAPIDIO_BUS_SIZE 0x10000000
#endif

/* PCI support is available just not defined by default */

#ifdef  INCLUDE_PCI_BUS

#define EPIC_EX_DFT_POLAR	EPIC_INT_ACT_LOW

/*  cds85xx support dual PCI controllers */

/*

CPU Addr                                  PCI Addr ( PCI1 or PCI2)
PCI_LOCAL_MEM_BUS        -------------------------- PCI_MSTR_MEM_BUS
                         -                        -
                         -                        -
PCI_LOCAL_MEM_BUS +      -------------------------- PCI_MSTR_MEM_BUS +
PCI_LOCAL_MEM_SIZE       -                        - PCI_MSTR_MEM_SIZE
                         -                        -
                         -                        ----- PCI Bridge (for PCI1 only)
                         -                        -     configuration regs
                         -                        -
CPU_PCI_MEM_ADRS  (PCI1) -------------------------- PCI_MEM_ADRS
CPU_PCI_MEM_ADRS2 (PCI2) -                        - PCI_MEM_ADRS2
CPU_PCI_MEM_ADRS3(PCIEX) -------------------------- PCI_MEM_ADRS3
                         -                        -
CPU_PCI_MEMIO_ADRS       -------------------------- PCI_MEMIO_ADRS
CPU_PCI_MEMIO_ADRS2      -                        - PCI_MEMIO_ADRS2
CPU_PCI_MEMIO_ADRS3      -                        - PCI_MEMIO_ADRS3
                         -                        -
CPU_PCI_IO_ADRS  (PCI1)  -------------------------- PCI_IO_ADRS
CPU_PCI_IO_ADRS2 (PCI2)  -                        - PCI_IO_ADRS2
CPU_PCI_IO_ADRS3 (PCIEX) -                        - PCI_IO_ADRS3
                         -                        -
CPU_PCI_IO_ADRS  (PCI1)+ -------------------------- PCI_IO_ADRS +
CPU_PCI_IO_ADRS2 (PCI2)+                            PCI_IO_ADRS2 +
CPU_PCI_IO_ADRS2 (PCI3)                             PCI_IO_ADRS3
CPU_PCI_IO_SIZE          -                        - PCI_IO_SIZE
                         -                        -
*/

/* PCI based addresses */

#define PCI_LAW_BASE             0x50000000       /* PCI LAW window */
#define PCI_LAW_SIZE             0x10000000       /* PCI LAW size   */

#define PCI_MEM_SIZE             0x04000000        /* 64 MB */
#define PCI_MEMIO_SIZE           0x04000000        /* 64 MB */
#define PCI_IO_SIZE              0x04000000        /* 64 MB */

#define PCI_MEM_ADRS             (PCI_LAW_BASE)
#define PCI_MEMIO_ADRS           (PCI_MEM_ADRS   + PCI_MEM_SIZE)
#define PCI_IO_ADRS              (PCI_MEMIO_ADRS + PCI_MEMIO_SIZE)

#define PCIEX_LAW_BASE            0x60000000       /* PCI LAW window */
#define PCIEX_LAW_SIZE            0x10000000       /* PCI LAW size   */

#define PCI_MEM_ADRS3            (PCIEX_LAW_BASE)
#define PCI_MEMIO_ADRS3          (PCI_MEM_ADRS3   + PCI_MEM_SIZE)
#define PCI_IO_ADRS3             (PCI_MEMIO_ADRS3 + PCI_MEMIO_SIZE)

#if (PCI_LAW_SIZE == 0x10000000)
#define  PCI_LAW_WIN_SZ          LAWAR_SIZE_256MB
#endif

#if (PCI2_LAW_SIZE == 0x10000000)
#define  PCI2_LAW_WIN_SZ         LAWAR_SIZE_256MB
#endif

#if (PCIEX_LAW_SIZE == 0x10000000)
#define  PCIEX_LAW_WIN_SZ         LAWAR_SIZE_256MB
#endif

#define PCI_MMU_TLB_SZ           _MMU_TLB_SZ_256M

#if (PCI_MEM_SIZE == 0x04000000)
#define PCI_MEM_SIZE_MASK        PCI_ATTR_WS_64M
#endif

#if (PCI_MEMIO_SIZE == 0x04000000)
#define PCI_MEMIO_SIZE_MASK      PCI_ATTR_WS_64M
#endif

#if (PCI_IO_SIZE == 0x04000000)
#define PCI_IO_SIZE_MASK         PCI_ATTR_WS_64M
#endif

#define PCI_BRIDGE_PIMMR_BASE_ADRS  0x40000000

/* just 1:1 mapping */

/* PCI 1 */

#define CPU_PCI_MEM_ADRS          PCI_MEM_ADRS
#define CPU_PCI_MEMIO_ADRS        PCI_MEMIO_ADRS
#define CPU_PCI_IO_ADRS           PCI_IO_ADRS

/* PCI 2 */

#define CPU_PCI_MEM_ADRS2         PCI_MEM_ADRS2
#define CPU_PCI_MEMIO_ADRS2       PCI_MEMIO_ADRS2
#define CPU_PCI_IO_ADRS2          PCI_IO_ADRS2

/* PCI Express */

#define CPU_PCI_MEM_ADRS3         PCI_MEM_ADRS3
#define CPU_PCI_MEMIO_ADRS3       PCI_MEMIO_ADRS3
#define CPU_PCI_IO_ADRS3          PCI_IO_ADRS3

#define CPU_PCI_MEM_SIZE          PCI_MEM_SIZE
#define CPU_PCI_MEMIO_SIZE        PCI_MEMIO_SIZE
#define CPU_PCI_IO_SIZE           PCI_IO_SIZE

/* CPU from PCI bus */

#define PCI_MSTR_MEM_BUS           0x00000000
#define PCI_MSTR_MEM_SIZE          0x40000000 /* 1G */

/* CPU Address that is visible from PCI */

#define PCI_LOCAL_MEM_BUS         0x00000000
#define PCI_LOCAL_MEM_SIZE        PCI_MSTR_MEM_SIZE
#define PCI_LOCAL_MEM_SIZE_MASK   PCI_ATTR_WS_1G

#define INCLUDE_PCI_AUTOCONF

#ifndef PCI_CFG_TYPE
#   ifdef INCLUDE_PCI_AUTOCONF
#      define PCI_CFG_TYPE PCI_CFG_AUTO
#   else
#      define PCI_CFG_TYPE PCI_CFG_FORCE
#   endif /* INCLUDE_PCI_AUTOCONF */
#endif /* PCI_CFG_TYPE */

#endif /* INCLUDE_PCI */

#ifdef INCLUDE_PCI

/* Intel 8254x support */

#ifdef INCLUDE_GEI8254X_END
 #define INCLUDE_PRIMARY_GEI_END
 #undef INCLUDE_SECONDARY_GEI_END
  #ifndef INCLUDE_END
     #define INCLUDE_END
  #endif  /* INCLUDE_END */
#endif /* INCLUDE_GEI8254X_END */

#endif /* INCLUDE_PCI */

/* Shared mmeory is set up for Serial RIO but is untested */

#if defined(INCLUDE_SM_COMMON)
#   define INCLUDE_SMEND
#   define INCLUDE_SM_NET
#   define INCLUDE_BSD
#   define INCLUDE_SM_NET_SHOW
#   define INCLUDE_NET_DRV
#   define INCLUDE_NET_LIB
#   ifdef  STANDALONE
#          define STANDALONE_NET
#   endif  /* STANDALONE */


/*
 * Changing SM_OFF_BOARD to TRUE also requires changing
 * SM_ANCHOR_ADRS and SM_MEM_ADRS appropriately.
 */

#define SM_OFF_BOARD    FALSE

#define INCLUDE_SM_SEQ_ADDR
#undef SM_TAS_TYPE
#define SM_TAS_TYPE SM_TAS_HARD

/*
 * INCLUDE_SM_NET and INCLUDE_SM_SEQ_ADDR are the shared memory backplane
 * driver and the auto address setup which are excluded by default.
 * To exclude them, uncomment the following lines:
 *
 * #define INCLUDE_SM_NET
 * #define INCLUDE_SM_SEQ_ADDR
 */

#ifndef _ASMLANGUAGE
IMPORT char* rioHostAdrs;

#undef  SM_ANCHOR_ADRS
#define SM_ANCHOR_ADRS  ((sysProcNumGet() == 0) ? \
        ((char*) (LOCAL_MEM_LOCAL_ADRS + SM_ANCHOR_OFFSET)) : \
        ((char*) ((UINT32)rioHostAdrs + SM_ANCHOR_OFFSET)))

#endif

#define SM_INT_TYPE		SM_INT_BUS  /* or SM_INT_NONE */

#define SM_MEM_ADRS		0x00010000	/* Start of actual SM region */
#define SM_MEM_SIZE		0x00070000
#define SM_OBJ_MEM_ADRS		(SM_MEM_ADRS+SM_MEM_SIZE) /* SM Objects pool */
#define SM_OBJ_MEM_SIZE		0x00040000

#define SM_INT_ARG1   46 /*EPIC_SR_IN_DB1_INT_VEC*/
#define SM_INT_ARG2   46 /*EPIC_SR_IN_DB1_INT_VEC*/
#define SM_INT_ARG3   1


#define SM_TIPC_ADRS_DEFAULT    ((int) SM_OBJ_MEM_ADRS + SM_OBJ_MEM_SIZE)
#define SM_TIPC_ADRS            SM_TIPC_ADRS_DEFAULT
#define SM_TIPC_SIZE_DEFAULT    0x00020000
#define SM_TIPC_SIZE            SM_TIPC_SIZE_DEFAULT /* SM TIPC pool size  */

#undef IPFORWARDING_CFG
#define IPFORWARDING_CFG        TRUE

#endif /* defined(INCLUDE_SM_NET) */


/*
 * Set the Carrier Card's SW2[12]=00 for the following configuration,
 * the bootrom would be in the last 1 MB of flash0, and UBOOT would still
 * be in the flash 1.
 */

#define FLASH_WINDOW_SIZE               0x00800000
#define CDS85XX_FLASH_RESERVED_SIZE     0x00100000

#if (BOOT_FLASH == ON_BOARD_FLASH)

/* LBC CS0 - flash 0 - 8MB, 8-bit flash - default for bootrom */

#define FLASH_BASE_ADRS              0xfff00000   /*0xfff80000*/
#define FLASH_ADRS_MASK                0xfff00000/* 0xfff80000*/
#define FLASH_SIZE                      0x00100000/*0x00080000*/

#define BOOT_FLASH_TLB_SIZE             _MMU_TLB_SZ_16M

/* LBC CS6 - flash 1 - 64MB, 32-bit flash SODIMM - for TFFS */

#define FLASH1_BASE_ADRS                0xd0000000
#define FLASH1_ADRS_MASK                0xf0000000
#define FLASH1_SIZE                     0x00200000
/*#define FLASH1_BASE_ADRS                0xfff00000
#define FLASH1_ADRS_MASK                0xfff00000
#define FLASH1_SIZE                     0x00100000*/

#else

/* LBC CS0 - flash 0 - 64MB, 32-bit flash SODIMM - default for bootrom */

#define FLASH_BASE_ADRS                 0xfc000000
#define FLASH_ADRS_MASK                 0xfc000000
#define FLASH_SIZE                      0x04000000

#define BOOT_FLASH_TLB_SIZE             _MMU_TLB_SZ_64M

/* LBC CS6 - flash 1 - 8MB, 8-bit flash */

#define FLASH1_BASE_ADRS                0xfff00000
#define FLASH1_ADRS_MASK                0xfff00000
#define FLASH1_SIZE                     0x00100000

#endif
#define CF_BASE_ADRS           (0xD0000000) /*cs2 CF base Addr */
#define CF_SIZE                (0x00100000) /* 512KB*2  */    
#define INT_LVL_CF   11//26   /* gpio level*/

#define FPGA_BASE_ADRS           (0xD1000000) /*cs3 FPGA base Addr */
#define FPGA_SIZE                (0x00100000) /* 512KB*2 */    
#define FPGA_REV_REG				0x60   /**L2*****/


#define NVRAM_BASE_ADDR         (0xF0000000)
#define NV_RAM_ADRS             NVRAM_BASE_ADDR            /* this is an offset, base addr*/
#define NVRAM_SIZE              (0x00100000) /*cs1 512*2KB */   

#define NV_RAM_INTRVL           0x00000001

#define NV_RAM_ADRS             NVRAM_BASE_ADDR            /* this is an offset, base addr*/

#define RTC_WATCHDOG            0x7FFF7
#define NV_RAM_READ(x)          sysNvRead(x)    /* sysLib.c */
#define NV_RAM_WRITE(x,y)       sysNvWrite(x,y) /* sysLib.c */

#define BOOT_LINE_ADRS_L3         (NVRAM_BASE_ADDR + /*0*/0x4)/***wangwenhua modify 20091126***/
#define BOOT_LINE_ADRS_L2         (NVRAM_BASE_ADDR + 0x100)  /** point to nvram (BOOT_LINE_ADRS + BOOT_LINE_SIZE) **/
#if 0
#define L2_SRAM_BOOT_LINE_SIZE     (0x100) 
#define L2_SRAM_BOOT_TIME_SIZE     (0x10)
#define L2_SRAM_BOOT_LINE_OFFSET   (MV_SRAM_SIZE - L2_SRAM_BOOT_LINE_SIZE)
#define L2_SRAM_BOOT_TIME_OFFSET   (L2_SRAM_BOOT_LINE_OFFSET - L2_SRAM_BOOT_TIME_SIZE)
#endif

#define NVRAM_BASE_ADDR_BOOT_STATE     (NVRAM_BASE_ADDR + 0x200)
#define NVRAM_BASE_ADDR_APP_PARAMS     (NVRAM_BASE_ADDR + 0x300)
#define NVRAM_BASE_ADDR_PARA_PARAMS (NVRAM_BASE_ADDR+0x400)/*用来保存新加的参数判断*/
#define NVRAM_BASE_ADDR_NETWORK_PARAS (NVRAM_BASE_ADDR+0x500)/***用来保存网口配置参数20090224**********/
#define NVRAM_BASE_ADDR_OTHER_PARAS (NVRAM_BASE_ADDR+0x600)/*保存参数在nvram中*/
#define NVRAM_BASE_ADDR_OAM_DATA_CRC (NVRAM_BASE_ADDR+0x700)/*保存OAM配置数据校验和*/
#define NVRAM_BASE_ADDR_OAM            (NVRAM_BASE_ADDR + 0x1000)
#define NVRAM_OAM_MIB_SIZE             (100*1024)
#define NVRAM_BASE_ADDR_DATA           (NVRAM_BASE_ADDR_OAM + NVRAM_OAM_MIB_SIZE + 4096)	/*+4096:是为了保证和OAM不在同一NVRAM物理页上*/
#define NVRAM_DATA_SERVICE_SIZE        (204*1024)
#define NVRAM_TASK_SOCKET_DATA_BASE    (NVRAM_BASE_ADDR_DATA + NVRAM_DATA_SERVICE_SIZE)
#define NVRAM_TASK_SOCKET_DATA_SIZE    (12*1024)
#define NVRAM_CSI_BASE                 (NVRAM_TASK_SOCKET_DATA_BASE + NVRAM_TASK_SOCKET_DATA_SIZE)
#define NVRAM_CSI_SIZE                 (120*1024)
/*lijinan 20081217 计费系统增加*/
#define NVRAM_CDR_BASE    (NVRAM_CSI_BASE + NVRAM_CSI_SIZE + 4096)
#define NVRAM_CDR_SIZE    (4*1024)
#define SDRAM_TSK_INFORMATION (NVRAM_CDR_BASE + NVRAM_CDR_SIZE+ 4096)
/*CSI扩充部分*/
#define NVRAM_CSI_BASE_NEW                 (NVRAM_CDR_BASE + NVRAM_CDR_SIZE+10240)
#define NVRAM_CSI_SIZE_NEW                 (40*1024)

#define NVRAM_BASE_ADDR_BOOT_STATE     (NVRAM_BASE_ADDR + 0x200)

/* LBC CS3 - SDRAM */

/*#define INCLUDE_LBC_SDRAM */
#undef INCLUDE_LBC_SDRAM

/* NOTE this should match the LAWAR SIZE in romInit for the chosen SDRAM */

#define LOCAL_MEM_SIZE2            0x4000000   /* 64 Mbyte memory available */
#define LOCAL_MEM_LOCAL_ADRS2      0xf0000000  /* Base of RAM */
#define LBC_SDRAM_LOCAL_SIZE_MASK  0xfc000000
#define LBC_SDRAM_LOCAL_ADRS       LOCAL_MEM_LOCAL_ADRS2
#define LBC_SDRAM_LOCAL_SIZE       LOCAL_MEM_SIZE2

/* LBC CS5 - EEPROM, user LEDs, user switches (decoded in EPLD) */

#define INCLUDE_LBC_CS5

#ifdef INCLUDE_LBC_CS5
#define INCLUDE_NV_RAM
#define LBC_CS5_LOCAL_ADRS       0xf8000000
#define LBC_CS5_LOCAL_SIZE_MASK  0xff000000
#endif


/* NVRAM configuration */

#ifdef  INCLUDE_NV_RAM


#define NV_RAM_ADRS 						0x00800000
/*#ifdef NVRAM_BOOT
	#define NV_RAM_ADRS          (0xfff80000 + 0x00070000)
#else	
	#define NV_RAM_ADRS       FLASH1_BASE_ADRS
#endif NVRAM_BOOT*/

/*#   define NV_RAM_ADRS          (LBC_CS5_LOCAL_ADRS + 0x00b00000)*/
#   undef  NV_RAM_SIZE
#   define NV_RAM_SIZE          (0x2000 - 0x10)  /* 8KB - 16 */
#   define NV_RAM_INTRVL        1
#   undef  NV_BOOT_OFFSET
#   define NV_BOOT_OFFSET       0
#   define NV_MAC_ADRS_OFFSET   0x200
#else
#   define NV_RAM_SIZE 0
#   define NV_RAM_ADRS 0
#endif  /* INCLUDE_NV_RAM */

/* User switches */

#undef INCLUDE_SWITCHES
#ifdef INCLUDE_SWITCHES
#define SWITCH_ADRS         (LBC_CS5_LOCAL_ADRS + 0x00100000)
#endif /* INCLUDE_SWITCHES */

/* Memory addresses */

#define INCLUDE_DDR_SDRAM

/* NOTE this should match the LAWAR SIZE in romInit for the chosen SDRAM */

#define LOCAL_MEM_SIZE          0x10000000  /* 256/512 Mbyte memory available */
#define LOCAL_MEM_LOCAL_ADRS    0x00000000  /* Base of RAM */

#define DDR_SDRAM_LOCAL_ADRS       LOCAL_MEM_LOCAL_ADRS
#define DDR_SDRAM_LOCAL_SIZE       LOCAL_MEM_SIZE
#define DDR_SDRAM_LOCAL_ADRS_END   (DDR_SDRAM_LOCAL_ADRS + DDR_SDRAM_LOCAL_SIZE)

#ifdef INCLUDE_MMU_BASIC

#define INCLUDE_AIM_MMU_CONFIG
#define INCLUDE_MMU_OPTIMIZE

#define INCLUDE_AIM_MMU_SHOW

#define INCLUDE_AIM_MMU_MEM_POOL_CONFIG

/*
 * The following parameters are to configure initial memory usage for
 * page table and region tables and in event of running out the increment
 * of memory allocation and is specified as a number of MMU pages (4KByte
 * blocks).
 */

#define AIM_MMU_INIT_PT_NUM 0x40
#define AIM_MMU_INIT_PT_INCR 0x20
#define AIM_MMU_INIT_RT_NUM 0x10
#define AIM_MMU_INIT_RT_INCR 0x10

/* This write protects the page tables */

#define INCLUDE_AIM_MMU_PT_PROTECTION

/*
 * INCLUDE_LOCK_TEXT_SECTION Requires use of CAM entries which can
 * possibly be already used in sysStaticTlbDesc. Also the larger the
 * page size that can be used the less CAM entries required so aligning
 * RAM_LOW_ADRS on a large page size boundary is usually necessary.
 * Locking the text section should increase performance because no
 * Instruction TLB misses are taken within the text section.
 */

#define INCLUDE_LOCK_TEXT_SECTION
#undef INCLUDE_PAGE_SIZE_OPTIMIZATION /* Not available for 85XX */

#endif /* INCLUDE_MMU_BASIC */

#define INCLUDE_CTORS_DTORS

/*
 * Using software FP support. Athough task based 32 bit HW_FP is allowed
 * no optimised libraries are available. The kernel doesn't execute any hw
 * fp instructions
 */

#undef  INCLUDE_HW_FP
#undef  INCLUDE_PPC_FPU
#define INCLUDE_SW_FP

/*
 * The constants ROM_XXX_ADRS, ROM_SIZE, and RAM_XXX_ADRS are defined
 * in both config.h and Makefile.  All definitions for these constants
 * must be identical.
 */

#ifndef RAM_LOW_ADRS
#define	RAM_LOW_ADRS		0x00100000
#endif  /* RAM_LOW_ADRS */

#ifndef RAM_HIGH_ADRS
#define	RAM_HIGH_ADRS		0x01000000
#endif  /* RAM_HIGH_ADRS */

#define ROM_TEXT_ADRS /*0xFFF80100*/0xfff00100
#define ROM_BASE_ADRS /*0xFFF80000*/0xfff0000
#define ROM_SIZE     /* 0x00080000*/0x00100000

#define USER_RESERVED_MEM 0x100000

#define INCLUDE_SYSLED
#undef INCLUDE_SYSLED     /*BY XCL*/

#ifndef MAX_MAC_DEVS
#   define MAX_MAC_DEVS     1
#endif

#ifdef INCLUDE_END

#   define ETHERNET_MAC_HANDLER

#   define WR_ENET0         0x00    /* WR specific portion of MAC (MSB->LSB) */
#   define WR_ENET1         0xA0
#   define WR_ENET2         0x1E

#   define CUST_ENET3_0     0xA0    /* Customer portion of MAC address */
#   define CUST_ENET3_1     0xA1
#   define CUST_ENET3_2     0xA2
#   define CUST_ENET3_3     0xA3
#   define CUST_ENET4       0xAA
#   define CUST_ENET5       0xA0

#   define MAX_MAC_ADRS     4
#   define MAC_ADRS_LEN     6

#   define ENET_DEFAULT     0x1EA00000

#endif

/* add TFFS as boot device */
#undef INCLUDE_TFFS
#define INCLUDE_TFFS
#ifdef INCLUDE_TFFS
#   define INCLUDE_TFFS_MOUNT
#   define INCLUDE_TFFS_SHOW
#endif

#ifdef INCLUDE_TFFS
#   ifndef INCLUDE_DOSFS
#       define INCLUDE_DOSFS        /* file system to be used */
#   endif   /* INCLUDE_DOSFS */
#endif  /* INCLUDE_TFFS */

/* Use DOS File System */
#ifdef INCLUDE_DOSFS
#   define  INCLUDE_DOSFS_MAIN
#   define  INCLUDE_DOSFS_CACHE
#   define  INCLUDE_DOSFS_FAT
#	define	INCLUDE_DOSFS_FMT
#	define	INCLUDE_DOSFS_DIR_FIXED
#   define  INCLUDE_DOSFS_DIR_VFAT
#   define  INCLUDE_DISK_UTIL
#endif  /* INCLUDE_DOSFS */

#if !defined(PRJ_BUILD)
/* configuration for bootrom */
#   define INCLUDE_ERF
#   define INCLUDE_XBD
#   define INCLUDE_FS_MONITOR
#   define INCLUDE_FS_EVENT_UTIL
#   define INCLUDE_DEVICE_MANAGER
#endif

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#endif  /* INCconfigh */

#if defined(PRJ_BUILD)
    #include "prjParams.h"
#endif /* PRJ_BUILD */






#undef  NUM_SYS_MBLKS
#define NUM_SYS_MBLKS 400
#undef  NUM_SYS_CLBLKS
#define NUM_SYS_CLBLKS 400
#undef  PMA_SYSPOOL
#define PMA_SYSPOOL 0
#undef  PMS_SYSPOOL
#define PMS_SYSPOOL 0
#undef  SIZ_SYS_16
#define SIZ_SYS_16 100
#undef  NUM_SYS_16
#define NUM_SYS_16 200
#undef  PMA_SYS_16
#define PMA_SYS_16 0
#undef  PMS_SYS_16
#define PMS_SYS_16 0
#undef  SIZ_SYS_32
#define SIZ_SYS_32 88
#undef  NUM_SYS_32
#define NUM_SYS_32 100
#undef  PMA_SYS_32
#define PMA_SYS_32 0
#undef  PMS_SYS_32
#define PMS_SYS_32 0
#undef  SIZ_SYS_64
#define SIZ_SYS_64 400
#undef  NUM_SYS_64
#define NUM_SYS_64 40
#undef  PMA_SYS_64
#define PMA_SYS_64 0
#undef  PMS_SYS_64
#define PMS_SYS_64 0
#undef  SIZ_SYS_128
#define SIZ_SYS_128 400
#undef  NUM_SYS_128
#define NUM_SYS_128 0
#undef  PMA_SYS_128
#define PMA_SYS_128 0
#undef  PMS_SYS_128
#define PMS_SYS_128 0
#undef  SIZ_SYS_256
#define SIZ_SYS_256 584
#undef  NUM_SYS_256
#define NUM_SYS_256 0
#undef  PMA_SYS_256
#define PMA_SYS_256 0
#undef  PMS_SYS_256
#define PMS_SYS_256 0
#undef  SIZ_SYS_512
#define SIZ_SYS_512 2000
#undef  NUM_SYS_512
#define NUM_SYS_512 0
#undef  PMA_SYS_512
#define PMA_SYS_512 0
#undef  PMS_SYS_512
#define PMS_SYS_512 0
#undef  SIZ_SYS_1024
#define SIZ_SYS_1024 2000
#undef  NUM_SYS_1024
#define NUM_SYS_1024 80
#undef  PMA_SYS_1024
#define PMA_SYS_1024 0
#undef  PMS_SYS_1024
#define PMS_SYS_1024 0
#undef  SIZ_SYS_2048
#define SIZ_SYS_2048 2048
#undef  NUM_SYS_2048
#define NUM_SYS_2048 0
#undef  PMA_SYS_2048
#define PMA_SYS_2048 0
#undef  PMS_SYS_2048
#define PMS_SYS_2048 0


#undef  NUM_DAT_128
#define NUM_DAT_128 1024
#undef  PMA_DAT_128
#define PMA_DAT_128 0
#undef  PMS_DAT_128
#define PMS_DAT_128 0
#undef  NUM_DAT_256
#define NUM_DAT_256 160
#undef  PMA_DAT_256
#define PMA_DAT_256 0
#undef  PMS_DAT_256
#define PMS_DAT_256 0
#undef  NUM_DAT_512
#define NUM_DAT_512 160
#undef  PMA_DAT_512
#define PMA_DAT_512 0
#undef  PMS_DAT_512
#define PMS_DAT_512 0
#undef  NUM_DAT_1024
#define NUM_DAT_1024 16*10
#undef  PMA_DAT_1024
#define PMA_DAT_1024 0
#undef  PMS_DAT_1024
#define PMS_DAT_1024 0
#undef  NUM_DAT_2048
#define NUM_DAT_2048 40
#undef  PMA_DAT_2048
#define PMA_DAT_2048 0
#undef  PMS_DAT_2048
#define PMS_DAT_2048 0
#undef  NUM_DAT_4096
#define NUM_DAT_4096 0

#undef  NUM_POOL_1
#define NUM_POOL_1 300
#undef  SIZE_POOL_1
#define SIZE_POOL_1 64
#undef  MIN_PRIO_POOL_1
#define MIN_PRIO_POOL_1 0
#undef  NUM_POOL_2
#define NUM_POOL_2 500
#undef  SIZE_POOL_2
#define SIZE_POOL_2 128
#undef  MIN_PRIO_POOL_2
#define MIN_PRIO_POOL_2 0
#undef  NUM_POOL_3
#define NUM_POOL_3 400
#undef  SIZE_POOL_3
#define SIZE_POOL_3 256
#undef  MIN_PRIO_POOL_3
#define MIN_PRIO_POOL_3 0
#undef  NUM_POOL_4
#define NUM_POOL_4 600
#undef  SIZE_POOL_4
#define SIZE_POOL_4 512
#undef  MIN_PRIO_POOL_4
#define MIN_PRIO_POOL_4 0
#undef  NUM_POOL_5
#define NUM_POOL_5 800
#undef  SIZE_POOL_5
#define SIZE_POOL_5 1500
#undef  MIN_PRIO_POOL_5
#define MIN_PRIO_POOL_5 0
#undef  NUM_POOL_6
#define NUM_POOL_6 40
#undef  SIZE_POOL_6
#define SIZE_POOL_6 10000
#undef  MIN_PRIO_POOL_6
#define MIN_PRIO_POOL_6 0
#undef  NUM_POOL_11
#define NUM_POOL_11 10
#undef  SIZE_POOL_11
#define SIZE_POOL_11 65535
#undef  MIN_PRIO_POOL_11
#define MIN_PRIO_POOL_11 0
