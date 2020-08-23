/* sysLib.c - Wind River SBC548 board system-dependent library */

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
01g,20sep07,b_m  add ns16550, ppcIntCtlr, epic, m85xxTimer vxbus driver.
01f,10sep07,wap  Switch to VxBus PCI driver (WIND00104076)
01e,21aug07,mmi  remove vxPowerModeSet() since deprecated
01d,31jul07,agf  update vxBus cmdLineBuild
01c,25may07,b_m  modify static tlb table to enable boot device selection.
01b,03apr07,b_m  add TFFS map to static tlb table and phyMemDesc table.
01a,31jan06,kds  Modified from cds8458/sysLib.c/01c
*/

/*
 * DESCRIPTION
 * This library provides board-specific routines for SBC8548.
 *
 * INCLUDE FILES:
 *
 * SEE ALSO:
 * .pG "Configuration"
*/

/* includes */

#include <vxWorks.h>
#include <vme.h>
#include <memLib.h>
#include <cacheLib.h>
#include <sysLib.h>
#include "config.h"
#include <string.h>
#include <intLib.h>
#include <logLib.h>
#include <stdio.h>
#include <taskLib.h>
#include <vxLib.h>
#include <tyLib.h>
#include <arch/ppc/mmuE500Lib.h>
#include <arch/ppc/vxPpcLib.h>
#include <private/vmLibP.h>
#include <miiLib.h>
#include "vmLib.h"
#include "wrSbc8548.h"
#ifdef INCLUDE_PCI_BUS
#include <drv/pci/pciAutoConfigLib.h>
#include <drv/pci/pciConfigLib.h>
#include <drv/pci/pciIntLib.h>
#endif /* INCLUDE_PCI_BUS */
#define INCLUDE_ATA
#ifdef INCLUDE_ATA
    #include "ataDrv.h"
#endif /*INCLUDE_ATA*/
#include "28f160.c"
/* globals */
#ifndef _WRS_ASM
#define _WRS_ASM(x) __asm__ volatile (x)
#endif /*  _WRS_ASM */

#ifndef WRS_ASM
#define WRS_ASM(x)		_WRS_ASM(x)
#endif /* WRS_ASM */


#define EIEIO              WRS_ASM(" eieio")
/* C PPC syncronization macro */
#define PPC_EIEIO_SYNC  WRS_ASM (" eieio; sync")    
   void setBR3(unsigned int br0,unsigned int or0);
void setBR2(unsigned int br0,unsigned int or0);
void setBR1(unsigned int br0,unsigned int or0);
void setBR0(unsigned int br0,unsigned int or0);/***0xf0000801,0xfff06e65*********************/

 unsigned char  Hard_VerSion = 0 ;
STATUS bspCreateRamDisk();
TLB_ENTRY_DESC sysStaticTlbDesc [] =
{
    /*
     * effAddr,  Unused,  realAddr, ts | size | attributes | permissions
     *
     * TLB #0.  Flash
     *
     * needed be first entry here
     */

    { 0xffe00000/*FLASH_BASE_ADRS*/, 0x0, 0xffe0000/*FLASH_BASE_ADRS*/, _MMU_TLB_TS_0 | BOOT_FLASH_TLB_SIZE |
        _MMU_TLB_IPROT | _MMU_TLB_PERM_W | _MMU_TLB_PERM_X | _MMU_TLB_ATTR_I |
        _MMU_TLB_ATTR_G
    },

    /*
     * LOCAL MEMORY needed be second entry here  -
     * one TLB would be 256MB so use to get required 512MB
     */

    { 0x00000000, 0x0, 0x00000000, _MMU_TLB_TS_0 | _MMU_TLB_SZ_256M |
        _MMU_TLB_PERM_W | _MMU_TLB_PERM_X | _MMU_TLB_ATTR_M |
        CAM_DRAM_CACHE_MODE | _MMU_TLB_IPROT
    },
#if (LOCAL_MEM_SIZE > 0x10000000)
    { 0x10000000, 0x0, 0x10000000, _MMU_TLB_TS_0 | _MMU_TLB_SZ_256M |
        _MMU_TLB_PERM_W | _MMU_TLB_PERM_X | _MMU_TLB_ATTR_M |
        CAM_DRAM_CACHE_MODE | _MMU_TLB_IPROT
    },
#endif

    { CCSBAR, 0x0, CCSBAR, _MMU_TLB_TS_0 | _MMU_TLB_SZ_1M |
        _MMU_TLB_ATTR_I | _MMU_TLB_ATTR_G | _MMU_TLB_PERM_W | _MMU_TLB_IPROT
    }

#ifdef INCLUDE_LBC_SDRAM
    ,
    { LOCAL_MEM_LOCAL_ADRS2, 0x0, LOCAL_MEM_LOCAL_ADRS2, _MMU_TLB_TS_0 |
        _MMU_TLB_SZ_64M | _MMU_TLB_PERM_W | _MMU_TLB_PERM_X |
        CAM_DRAM_CACHE_MODE | _MMU_TLB_ATTR_M | _MMU_TLB_IPROT
    }
#endif /* LBC_SDRAM */

#ifdef INCLUDE_L2_SRAM
    ,
    { L2SRAM_ADDR, 0x0, L2SRAM_ADDR, _MMU_TLB_TS_0 | _MMU_TLB_SZ_256K |
        _MMU_TLB_PERM_W | _MMU_TLB_PERM_X | _MMU_TLB_ATTR_I |
        _MMU_TLB_ATTR_G
    }

#endif /* INCLUDE_L2_SRAM */
#ifdef INCLUDE_LBC_CS5

    /* 16 MB of LBC CS5 area */
    , {
        LBC_CS5_LOCAL_ADRS, 0x0, LBC_CS5_LOCAL_ADRS,
        _MMU_TLB_TS_0   | _MMU_TLB_SZ_16M | _MMU_TLB_IPROT |
        _MMU_TLB_PERM_W | _MMU_TLB_ATTR_I | _MMU_TLB_ATTR_G | _MMU_TLB_ATTR_M
    }
#endif
#ifdef INCLUDE_RAPIDIO_BUS
    ,
    {
    RAPIDIO_ADRS, 0x0, RAPIDIO_ADRS, _MMU_TLB_TS_0 | _MMU_TLB_SZ_256M |
    _MMU_TLB_ATTR_I | _MMU_TLB_PERM_W | _MMU_TLB_ATTR_G
    }
#endif

    /* Assume PCI space contiguous and within 256MB */

#ifdef INCLUDE_PCI_BUS
    ,
    { PCI_MEM_ADRS, 0x0, PCI_MEM_ADRS, _MMU_TLB_TS_0 | PCI_MMU_TLB_SZ |
        _MMU_TLB_ATTR_I | _MMU_TLB_ATTR_G | _MMU_TLB_PERM_W
    }
    ,
    { PCI_MEM_ADRS3, 0x0, PCI_MEM_ADRS3, _MMU_TLB_TS_0 |  PCI_MMU_TLB_SZ |
        _MMU_TLB_ATTR_I | _MMU_TLB_ATTR_G | _MMU_TLB_PERM_W
    }

#endif  /* INCLUDE_PCI_BUS */

};

int sysStaticTlbDescNumEnt = NELEMENTS (sysStaticTlbDesc);

#ifdef MMU_ASID_MAX     /* Base 6 MMU library in effect */

  /* macro to compose 64-bit PHYS_ADDRs */

# define PHYS_64BIT_ADDR(h, l)  (((PHYS_ADDR)(h) << 32) + (l))
#endif

/*
* sysPhysMemDesc[] is used to initialize the Page Table Entry (PTE) array
* used by the MMU to translate addresses with single page (4k) granularity.
* PTE memory space should not, in general, overlap BAT memory space but
* may be allowed if only Data or Instruction access is mapped via BAT.
*
* Address translations for local RAM, memory mapped PCI bus, the Board Control and
* Status registers, the MPC8260 Internal Memory Map, and local FLASH RAM are set here.
*
* PTEs are held, strangely enough, in a Page Table.  Page Table sizes are
* integer powers of two based on amount of memory to be mapped and a
* minimum size of 64 kbytes.  The MINIMUM recommended Page Table sizes
* for 32-bit PowerPCs are:
*
* Total mapped memory		Page Table size
* -------------------		---------------
*        8 Meg			     64 K
*       16 Meg			    128 K
*       32 Meg			    256 K
*       64 Meg			    512 K
*      128 Meg			      1 Meg
* 	.				.
* 	.				.
* 	.				.
*
* [Ref: chapter 7, PowerPC Microprocessor Family: The Programming Environments]
*
*/
PHYS_MEM_DESC sysPhysMemDesc [] =
{

    {
        /*
	 * Vector Table and Interrupt Stack
         * Must be sysPhysMemDesc [0] to allow adjustment in sysHwInit()
	 */

        (VIRT_ADDR) LOCAL_MEM_LOCAL_ADRS,
        (PHYS_ADDR) LOCAL_MEM_LOCAL_ADRS,
        LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | TLB_CACHE_MODE | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        /*
         * CCSBAR
        */
        (VIRT_ADDR) CCSBAR,
        (PHYS_ADDR) CCSBAR,
        0x00100000,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE |
        VM_STATE_MASK_MEM_COHERENCY | VM_STATE_MASK_GUARDED,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT |
        VM_STATE_MEM_COHERENCY | VM_STATE_GUARDED
    }

#ifdef INCLUDE_LBC_SDRAM
    ,
    {
        /* Must be sysPhysMemDesc [2] to allow adjustment in sysHwInit() */

        (VIRT_ADDR) LOCAL_MEM_LOCAL_ADRS2,
	(PHYS_ADDR) LOCAL_MEM_LOCAL_ADRS2,
        LOCAL_MEM_SIZE2,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | VM_STATE_MASK_MEM_COHERENCY ,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | TLB_CACHE_MODE | VM_STATE_MEM_COHERENCY
    }
#endif /* INCLUDE_LBC_SDRAM */

#ifdef INCLUDE_L2_SRAM
    ,
    {
        (VIRT_ADDR) L2SRAM_ADDR,
        (PHYS_ADDR) L2SRAM_ADDR,
        L2SRAM_WINDOW_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    }
#endif

#ifdef INCLUDE_LBC_CS5
    ,{
        (VIRT_ADDR) LBC_CS5_LOCAL_ADRS,
        (PHYS_ADDR) LBC_CS5_LOCAL_ADRS,
        16 * 0x100000,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
#endif /* INCLUDE_LBC_CS5 */

#ifdef INCLUDE_PCI_BUS
    ,
    {
        (VIRT_ADDR) PCI_MEM_ADRS,
        (PHYS_ADDR) PCI_MEM_ADRS,
        PCI_MEM_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) PCI_MEMIO_ADRS,
        (PHYS_ADDR) PCI_MEMIO_ADRS,
        PCI_MEMIO_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) PCI_IO_ADRS,
        (PHYS_ADDR) PCI_IO_ADRS,
        PCI_IO_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }



    ,
    {
        (VIRT_ADDR) PCI_MEM_ADRS3,
        (PHYS_ADDR) PCI_MEM_ADRS3,
        PCI_MEM_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) PCI_MEMIO_ADRS3,
        (PHYS_ADDR) PCI_MEMIO_ADRS3,
        PCI_MEMIO_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
    ,
    {
        (VIRT_ADDR) PCI_IO_ADRS3,
        (PHYS_ADDR) PCI_IO_ADRS3,
        PCI_IO_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
#endif /* INCLUDE_PCI_BUS */

#ifdef INCLUDE_RAPIDIO_BUS
    ,
    {
    (VIRT_ADDR) RAPIDIO_ADRS,
    (PHYS_ADDR) RAPIDIO_ADRS,
    RAPIDIO_SIZE,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE ,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    }
#endif /* INCLUDE_RAPIDIO_BUS */
#if 0
    ,
    {
        (VIRT_ADDR) FLASH_BASE_ADRS/*FLASH_BASE_ADRS*/,
        (PHYS_ADDR)FLASH_BASE_ADRS/*FLASH_BASE_ADRS*/,
       FLASH_SIZE/* FLASH_SIZE*/,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
#endif
    ,
    {
        (VIRT_ADDR) 0xffe00000/*FLASH1_BASE_ADRS*/,
        (PHYS_ADDR) 0xffe00000/*FLASH1_BASE_ADRS*/,
        FLASH1_SIZE,
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE | \
        VM_STATE_MASK_GUARDED | VM_STATE_MASK_MEM_COHERENCY,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT | \
        VM_STATE_GUARDED      | VM_STATE_MEM_COHERENCY
    }
  ,
  		
       {   
    /* CF Reg */

    (void *)  CF_BASE_ADRS,
    (void *)  CF_BASE_ADRS,
    CF_SIZE,
// VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE_ |
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE |
    VM_STATE_MASK_MEM_COHERENCY | VM_STATE_MASK_GUARDED,
    VM_STATE_VALID  | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT |
    VM_STATE_MEM_COHERENCY | VM_STATE_GUARDED
    }
    ,
	/**********************************/
    {        
    /* NV RAM */
        
    (void *) NVRAM_BASE_ADDR,
    (void *) NVRAM_BASE_ADDR,
    NVRAM_SIZE,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE |
     VM_STATE_MASK_GUARDED,
    VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT |
    VM_STATE_GUARDED
    }
    ,	
   	/**********************************/
    {        
    /* NV RAM */
        
    (void *) FPGA_BASE_ADRS,
    (void *) FPGA_BASE_ADRS,
    FPGA_SIZE,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE |
     VM_STATE_MASK_GUARDED,
    VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT |
    VM_STATE_GUARDED
    },	
		


};

int sysPhysMemDescNumEnt = NELEMENTS (sysPhysMemDesc);

/* Clock Ratio Tables */

#define MAX_VALUE_PLAT_RATIO 32
UINT32 platRatioTable[MAX_VALUE_PLAT_RATIO][2] =
{
    { 0, 0},
    { 0, 0},
    { 2, 0},
    { 3, 0},
    { 4, 0},
    { 5, 0},
    { 6, 0},
    { 7, 0},
    { 8, 0},
    { 9, 0},
    { 10, 0},
    { 0, 0},
    { 12, 0},
    { 0, 0},
    { 0, 0},
    { 0, 0},
    { 16, 0},
    { 0, 0},
    { 0, 0},
    { 0, 0},
    { 20, 0},
    { 0, 0}
};

#define MAX_VALUE_E500_RATIO 10
UINT32 e500RatioTable[MAX_VALUE_PLAT_RATIO][2] =
{
    { 0, 0},
    { 0, 0},
    { 1, 0},
    { 3, 1},
    { 2, 0},
    { 5, 1},
    { 3, 0},
    { 7, 1},
    { 4, 0},
    { 9, 1}
};


#ifdef INCLUDE_ATA

/* global data structures used by the EIDE/ATA driver */

ATA_RESOURCE    ataResources[ATA_MAX_CTRLS];  
/* ATA_RESOURCE    ataResources[1]; */

/* 
 * This structure needs to be initialized for each EIDE/ATA device present
 * in the system.  It has the following layout:

 *	number of cylinders;
 *	number of heads ;
 *	number of sectors per track ;
 *	number of bytes per sector ;
 *	precompensation cylinder;
 */

ATA_TYPE        ataTypes [ATA_MAX_CTRLS][ATA_MAX_DRIVES] =  
/* ATA_TYPE        ataTypes [1][1] =   */
{
    {
        { 761, 0, 0, 512, 0xff},  /* controller 0, drive 0 */
        {0}
    },
    {
        {0},
        {0}
    }


};
#endif /* INCLUDE_ATA */
#ifdef INCLUDE_ATA
void        sysAtaInit( int);
#endif /* INCLUDE_ATA */


UINT32   sysPciMode;
UINT32   sysPciSlotDeviceNumber;
int   sysBus      = BUS_TYPE_NONE;      /* system bus type (VME_BUS, etc) */
int   sysCpu      = CPU;                /* system CPU type (PPC8260) */
char *sysBootLine = BOOT_LINE_ADRS; /* address of boot line */
char *sysExcMsg   = EXC_MSG_ADRS;   /* catastrophic message area */
int   sysProcNum;           /* processor number of this CPU */
BOOL  sysVmeEnable = FALSE;     /* by default no VME */
UINT32  coreFreq;

IMPORT void     mmuE500TlbDynamicInvalidate();
IMPORT void     mmuE500TlbStaticInvalidate();
IMPORT void mmuE500TlbStaticInit (int numDescs,
                                  TLB_ENTRY_DESC *pTlbDesc,
                                  BOOL cacheAllow);
IMPORT BOOL     mmuPpcIEnabled;
IMPORT BOOL     mmuPpcDEnabled;
IMPORT void     sysIvprSet(UINT32);

/* forward declarations */

void   sysUsDelay (UINT32);
void   sysLedClkRoutine (int arg);

#ifdef INCLUDE_L1_IPARITY_HDLR_INBSP
    #define _EXC_OFF_L1_PARITY 0x1500
IMPORT void jumpIParity();
IMPORT void sysIvor1Set(UINT32);
UINT32 instrParityCount = 0;
#endif  /* INCLUDE_L1_IPARITY_HDLR_INBSP */


/* 8260 Reset Configuration Table (From page 9-2 in Rev0 of 8260 book) */

#define END_OF_TABLE 0

UINT32 sysClkFreqGet(void);
UINT32 ppcE500ICACHE_LINE_NUM = (128 * 12);
UINT32 ppcE500DCACHE_LINE_NUM = (128 * 12);

UINT32 ppcE500CACHE_ALIGN_SIZE = 32;

#include "sysMotI2c.c"
#include "sysMpc85xxI2c.c"

#ifdef INCLUDE_NV_RAM
    #include "eeprom.c"
    #include <mem/byteNvRam.c>      /* Generic NVRAM routines */
#else
    #include <mem/nullNvRam.c>
#endif /* INCLUDE_NV_RAM */


#ifdef INCLUDE_L1_IPARITY_HDLR
    #include "sysL1ICacheParity.c"
#endif


#ifdef  INCLUDE_ATA
    #include "ataDrv.c"
    #include "ataShow.c"
#endif /*INCLUDE_ATA*/



/*#include "sysBtsConfigData.c" */

UINT32 inFullVxWorksImage=FALSE;

#define EXT_VEC_IRQ0            0	
#define EXT_NUM_IRQ0            EXT_VEC_IRQ0
#define EXT_MAX_IRQS            200

STATUS  sysIntEnablePIC     (int intNum);   /* Enable i8259 or EPIC */
STATUS  sysCascadeIntEnable      (int intNum);
STATUS  sysCascadeIntDisable     (int intNum);
void    flashTest(VUINT16 *address,VUINT16 *buffer,VINT32 length);

UINT32   baudRateGenClk;

#include "sysL2Cache.c"

#ifdef INCLUDE_VXBUS
IMPORT void hardWareInterFaceInit();
#ifdef INCLUDE_SIO_UTILS
IMPORT void sysSerialConnectAll(void);
#endif
#endif /* INCLUDE_VXBUS */

#define WB_MAX_IRQS 256

#ifdef INCLUDE_SYSLED
#  include "sysLed.c"
#endif /* INCLUDE_SYSLED */

/* defines */

#define ZERO    0

/* needed to enable timer base */

#ifdef INCLUDE_PCI_BUS
    #define      M8260_DPPC_MASK	0x0C000000 /* bits 4 and 5 */
    #define      M8260_DPPC_VALUE	0x0C000000 /* bits (4,5) should be (1,0) */
#else
    #define      M8260_DPPC_MASK	0x0C000000 /* bits 4 and 5 */
    #define      M8260_DPPC_VALUE	0x08000000 /* bits (4,5) should be (1,0) */
#endif /*INCLUDE_PCI_BUS */

#define DELTA(a,b)                 (abs((int)a - (int)b))
#define HID0_MCP 0x80000000
#define HID1_ABE 0x00001000
#define HID1_ASTME 0x00002000
#define HID1_RXFE  0x00020000


#ifdef INCLUDE_VXBUS
#include <hwif/vxbus/vxBus.h>
#include <../src/hwif/h/busCtlr/m85xxRio.h>
#endif

#ifdef INCLUDE_MOT_TSEC_END
#include "sysNet.c"
#endif /* INCLUDE_MOT_TSEC_END */

#ifdef INCLUDE_VXBUS
#include "hwconf.c"
#endif

/********support the bigendian flash*********/
#ifdef NVRAM_BOOT
/*#include "28F160.c"*/
#endif


#ifdef INCLUDE_BRANCH_PREDICTION
IMPORT void disableBranchPrediction();
#endif

#ifdef INCLUDE_L2_SRAM
LOCAL void sysL2SramEnable(BOOL both);
#endif /* INCLUDE_L2_SRAM */

#ifdef INCLUDE_SPE
    #include <speLib.h>
IMPORT int       (* _func_speProbeRtn) () ;
#endif /* INCLUDE_SPE */

#ifdef INCLUDE_CACHE_SUPPORT
LOCAL void sysL1CacheQuery();
#endif

UINT32 sysTimerClkFreq = OSCILLATOR_FREQ;

IMPORT  void    sysL1Csr1Set(UINT32);
IMPORT  UINT    sysTimeBaseLGet(void);

LOCAL char * physTop = NULL;
LOCAL char * memTop = NULL;


#if     defined (INCLUDE_SPE)

/*******************************************************************************
*
* sysSpeProbe - Check if the CPU has SPE unit.
*
* This routine returns OK it the CPU has an SPE unit in it.
* Presently it assumes available.
*
* RETURNS: OK
*
* ERRNO: N/A
*/

int  sysSpeProbe (void)
    {
    ULONG regVal;
    int speUnitPresent = OK;

    /* The CPU type is indicated in the Processor Version Register (PVR) */

    regVal = 0;

    switch (regVal)
        {
        case 0:
        default:
            speUnitPresent = OK;
            break;
        }      /* switch  */

    return(speUnitPresent);
    }
#endif  /* INCLUDE_SPE */


/****************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.
*
* RETURNS: A pointer to the string.
*
* ERRNO: N/A
*/

char * sysModel (void)
    {
    UINT device;
    char* retChar = NULL;
    device = *M85XX_SVR(CCSBAR);

    switch(device & 0xffffff00)
	{
	case 0x80390000:
	    retChar = SYS_MODEL_8548E;
	    break;
	case 0x80310000:
	    retChar = SYS_MODEL_8548;
	    break;
	case 0x80390100:
	    retChar = SYS_MODEL_8547E;
	    break;
	case 0x80390200:
	    retChar = SYS_MODEL_8545E;
	    break;
	case 0x80310200:
	    retChar = SYS_MODEL_8545;
	    break;
	case 0x803A0000:
	    retChar = SYS_MODEL_8543E;
	    break;
	case 0x80320000:
	    retChar = SYS_MODEL_8543;
	    break;
	default:
	    retChar = SYS_MODEL_E500;
	    break;
	}


    device = *M85XX_PVR(CCSBAR);

    if ((device & 0xfff00000) != 0x80200000)
        retChar =SYS_MODEL_UNKNOWN;

    return(retChar);

    }

/******************************************************************************
*
* sysBspRev - return the BSP version with the revision eg 1.0/<x>
*
* This function returns a pointer to a BSP version with the revision.
* for eg. 1.0/<x>. BSP_REV defined in config.h is concatenated to
* BSP_VERSION and returned.
*
* RETURNS: A pointer to the BSP version/revision string.
*
* ERRNO: N/A
*/

char * sysBspRev (void)
    {
    return(BSP_VERSION BSP_REV);
    }

/******************************************************************************
*
* sysClkFreqGet - return Core Complex Bus clock freq
*
* This function returns the CCB clock freq.
*
* RETURNS: CCB clock freq
*
* ERRNO: N/A
*/

UINT32 sysClkFreqGet
(
void
)
    {
    UINT32  sysClkFreq;
    UINT32 e500Ratio,platRatio;

    platRatio = M85XX_PORPLLSR_PLAT_RATIO(CCSBAR);

#ifdef FORCE_DEFAULT_FREQ
    return(DEFAULT_SYSCLKFREQ);
#endif

    if ((platRatio>MAX_VALUE_PLAT_RATIO)||(platRatioTable[platRatio][0]==0))
        return(DEFAULT_SYSCLKFREQ); /* A default value better than zero or -1 */

    sysClkFreq = ((UINT32)(OSCILLATOR_FREQ * platRatioTable[platRatio][0]))>>((UINT32)platRatioTable[platRatio][1]);

    e500Ratio = M85XX_PORPLLSR_E500_RATIO(CCSBAR);
    coreFreq = ((UINT32)(sysClkFreq * e500RatioTable[e500Ratio][0]))>>((UINT32)e500RatioTable[e500Ratio][1]);


    return(sysClkFreq);
    }

/******************************************************************************
*
* sysCpmFreqGet - Determines the CPM Operating Frequency
*
* This routine determines the CPM Operating Frequency.
*
* From page 9-2 Rev. 0  MPC8260  PowerQUICC II User's Manual
*
* RETURNS: CPM clock frequency for the current MOD_CK and MOD_CK_H settings
*
* ERRNO: N/A
*/

UINT32 sysCpmFreqGet (void)
    {
    UINT32 sysClkFreq = sysClkFreqGet();
    return(sysClkFreq);

    }

/******************************************************************************
*
* sysBaudClkFreq - Obtains frequency of the BRG_CLK in Hz
*
* From page 9-5 in Rev. 0 MPC8260 PowerQUICC II User's Manual
*
*     baud clock = 2*cpm_freq/2^2*(DFBRG+1) where DFBRG = 01
*                = 2*cpm_freq/16
*
* RETURNS: frequency of the BRG_CLK in Hz
*
* ERRNO: N/A
*/

UINT32 sysBaudClkFreq (void)
    {
    UINT32 cpmFreq = sysCpmFreqGet();

    return cpmFreq*2/16;
    }

/******************************************************************************
*
* sysHwMemInit - initialize and configure system memory.
*
* This routine is called before sysHwInit(). It performs memory auto-sizing
* and updates the system's physical regions table, `sysPhysRgnTbl'. It may
* include the code to do runtime configuration of extra memory controllers.
*
* NOTE: This routine should not be called directly by the user application.  It
* cannot be used to initialize interrupt vectors.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysHwMemInit (void)
    {

    /* Call sysPhysMemTop() to do memory autosizing if available */

    sysPhysMemTop ();

    }

/******************************************************************************
*
* sysHwInit - initialize the system hardware
*
* This routine initializes various feature of the MPC8260 ADS board. It sets up
* the control registers, initializes various devices if they are present.
*
* NOTE: This routine should not be called directly by the user.
*
* RETURNS: NA
*
* ERRNO: N/A
*/

void sysHwInit (void)
    {


#ifdef INCLUDE_RAPIDIO_BUS

    /* Errata not yet described - required for rapidIO TAS */

    *(UINT32*)(CCSBAR + 0x1010) = 0x01040004;
#endif
  setBR0(0xffe01001,0xffe06e65);
    sysIvprSet(0x0);

    /* Disable L1 Icache */

    sysL1Csr1Set(vxL1CSR1Get() & ~0x1);

    /* Check for architecture support for 36 bit physical addressing */

#if defined(PPC_e500v2)
    vxHid0Set(_PPC_HID0_MAS7EN|vxHid0Get());
#endif

    /* Enable machine check pin */

    vxHid0Set(HID0_MCP|vxHid0Get());

#ifdef E500_L1_PARITY_RECOVERY

    /* Enable Parity in L1 caches */

    vxL1CSR0Set(vxL1CSR0Get() | _PPC_L1CSR_CPE);
    vxL1CSR1Set(vxL1CSR1Get() | _PPC_L1CSR_CPE);
#endif  /* E500_L1_PARITY_RECOVERY */

    /* enable time base for delay use before DEC interrupt is setup */

    vxHid0Set(vxHid0Get() | _PPC_HID0_TBEN);

    sysTimerClkFreq = sysClkFreqGet()>>3 /* Clock div is 8 */;

#ifdef INCLUDE_AUX_CLK
    sysAuxClkRateSet(127);
#endif

#ifdef INCLUDE_CACHE_SUPPORT
    sysL1CacheQuery();
#endif /* INCLUDE_CACHE_SUPPORT */

    /* Initialise L2CTL register */

    vxL2CTLSet(0x28000000,M85XX_L2CTL(CCSBAR));

    /*
     * Need to setup static TLB entries for bootrom or any non-MMU
     * enabled images
     */
    mmuE500TlbDynamicInvalidate();
    mmuE500TlbStaticInvalidate();
    mmuE500TlbStaticInit(sysStaticTlbDescNumEnt, &sysStaticTlbDesc[0], TRUE);

#if (!defined(INCLUDE_MMU_BASIC) && !defined(INCLUDE_MMU_FULL))
    mmuPpcIEnabled=TRUE;
    mmuPpcDEnabled=TRUE;
#else /* !defined(INCLUDE_MMU_BASIC) && !defined(INCLUDE_MMU_FULL) */
    if (inFullVxWorksImage==FALSE)
        {
        mmuPpcIEnabled=TRUE;
        mmuPpcDEnabled=TRUE;
        }
    /* Enable I Cache if instruction mmu disabled */

#if (defined(USER_I_CACHE_ENABLE) && !defined(USER_I_MMU_ENABLE))
    mmuPpcIEnabled=TRUE;
#endif /* (defined(USER_I_CACHE_ENABLE) && !defined(USER_I_MMU_ENABLE)) */

#endif /* !defined(INCLUDE_MMU_BASIC) && !defined(INCLUDE_MMU_FULL) */


#if (defined(INCLUDE_L2_CACHE) && defined(INCLUDE_CACHE_SUPPORT))
    vxHid1Set(HID1_ABE); /* Address Broadcast enable */
    sysL2CacheInit();
#endif /* INCLUDE_L2_CACHE  && INCLUDE_CACHE_SUPPORT*/

    /* Machine check via RXFE for RIO */

    vxHid1Set(vxHid1Get()| HID1_ASTME | HID1_RXFE); /* Address Stream Enable */

    /* enable the flash/rtc&nvram window */

    *M85XX_LAWBAR3(CCSBAR) = FLASH1_BASE_ADRS >> LAWBAR_ADRS_SHIFT;
    *M85XX_LAWAR3(CCSBAR)  = LAWAR_ENABLE | LAWAR_TGTIF_LBC | LAWAR_SIZE_64MB;
    WRS_ASM("isync");

/* local bus cs1 is allocated for nvram in normal operation, for flash when in debug mode **/ 


/*
#if (BOOT_FLASH == ON_BOARD_FLASH)
    *M85XX_BR6(CCSBAR) = 0xD0001801;
    *M85XX_OR6(CCSBAR) = 0xFC006E65;
#else
    *M85XX_BR6(CCSBAR) = 0xFB800801;
    *M85XX_OR6(CCSBAR) = 0xFF806E65;
#endif
*/
    WRS_ASM("isync");

#ifdef INCLUDE_VXBUS
    hardWareInterFaceInit();
#endif /* INCLUDE_VXBUS */

#ifdef E500_L1_PARITY_RECOVERY
    vxIvor1Set(_EXC_OFF_L1_PARITY);
#endif  /* E500_L1_PARITY_RECOVERY */
#ifdef INCLUDE_L1_IPARITY_HDLR
    installL1ICacheParityErrorRecovery();
#endif /* INCLUDE_L1_IPARITY_HDLR */

#if defined(INCLUDE_L2_SRAM)
#if (defined(INCLUDE_L2_CACHE) && defined(INCLUDE_CACHE_SUPPORT))
    sysL2SramEnable(TRUE);
#elif (defined(INCLUDE_L2_SRAM))
    sysL2SramEnable(FALSE);
#endif
#endif

    CACHE_PIPE_FLUSH();
   i2cDrvInit(0,0);

    }

#ifdef INCLUDE_L2_SRAM
/*************************************************************************
*
* sysL2SramEnable - enables L2SRAM if L2SRAM only
*
* This routine enables L2SRAM if L2SRAM only or initializes blk
* size etc and leaves the rest to L2 cache code.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
LOCAL void sysL2SramEnable
(
BOOL both
)
    {
    volatile int l2CtlVal;

    /*
     * if INCLUDE_L2_CACHE and CACHE_SUPPORT
     * if ((L2_SRAM_SIZE + L2_CACHE_SIZE) > l2Siz)
     */

    /* Setup Windows for L2SRAM */

    *(M85XX_L2SRBAR0(CCSBAR)) = (UINT32)(L2SRAM_ADDR & M85XX_L2SRBAR_ADDR_MSK);

    /* Get present value */

    l2CtlVal = vxL2CTLGet(M85XX_L2CTL(CCSBAR));

    /* Disable L2CTL initially to allow changing of block size */

    l2CtlVal &=(~M85XX_L2CTL_L2E_MSK);
    vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));
    l2CtlVal &= ~M85XX_L2CTL_L2BLKSIZ_MSK;
    l2CtlVal &= ~M85XX_L2CTL_L2SRAM_MSK;

    if (both == TRUE)
        {
        /* Setup size of SRAM */

        l2CtlVal |= (L2SIZ_256KB << M85XX_L2CTL_L2BLKSIZ_BIT) |
                    (0x2 << M85XX_L2CTL_L2SRAM_BIT);
        }
    else
        {
        l2CtlVal |= (L2SIZ_512KB << M85XX_L2CTL_L2BLKSIZ_BIT) |
                    (0x1 << M85XX_L2CTL_L2SRAM_BIT);
        }

    /* Setup L2CTL for SRAM */

    vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));

    if (both == FALSE)
        {
        /* This is done here so L2SRAM is set before enable */

        l2CtlVal |= M85XX_L2CTL_L2E_MSK; /* No cache so go ahead and enable */

        /* Enable L2CTL for SRAM */

        vxL2CTLSet(l2CtlVal,M85XX_L2CTL(CCSBAR));
        }

    }
#endif /* INCLUDE_L2_SRAM */

/**************************************************************************
*
* sysPhysMemTop - get the address of the top of physical memory
*
* This routine returns the address of the first missing byte of memory,
* which indicates the top of memory.
*
* RETURNS: The address of the top of physical memory.
*
* ERRNO: N/A
*
* SEE ALSO: sysMemTop()
*/

char * sysPhysMemTop (void)
    {

    if (physTop == NULL)
        {
        physTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);
        }

    return(physTop) ;
    }

/***************************************************************************
*
* sysMemTop - get the address of the top of VxWorks memory
*
* This routine returns a pointer to the first byte of memory not
* controlled or used by VxWorks.
*
* The user can reserve memory space by defining the macro USER_RESERVED_MEM
* in config.h.  This routine returns the address of the reserved memory
* area.  The value of USER_RESERVED_MEM is in bytes.
*
* RETURNS: The address of the top of VxWorks memory.
*
* ERRNO: N/A
*/

char * sysMemTop (void)
    {

    if (memTop == NULL)
        {
        memTop = sysPhysMemTop () - USER_RESERVED_MEM;

#ifdef INCLUDE_EDR_PM

        /* account for ED&R persistent memory */

        memTop = memTop - PM_RESERVED_MEM;
#endif

        }

    return memTop;
    }

/**************************************************************************
*
* sysToMonitor - transfer control to the ROM monitor
*
* This routine transfers control to the ROM monitor.  Normally, it is called
* only by reboot()--which services ^X--and bus errors at interrupt level.
* However, in some circumstances, the user may wish to introduce a
* <startType> to enable special boot ROM facilities.
*
* RETURNS: Does not return.
*
* ERRNO: N/A
*/

STATUS sysToMonitor
    (
    int startType   /* parameter passed to ROM to tell it how to boot */
    )
    {
    FUNCPTR pRom = (FUNCPTR) (0xfff00104/*ROM_TEXT_ADRS + 4*/);   /* Warm reboot */

    intLock();


#ifdef INCLUDE_BRANCH_PREDICTION
    disableBranchPrediction();
#endif /* INCLUDE_BRANCH_PREDICTION */

#ifdef INCLUDE_CACHE_SUPPORT
    cacheDisable(INSTRUCTION_CACHE);
    cacheDisable(DATA_CACHE);
#endif
    sysClkDisable();


#ifdef INCLUDE_AUX_CLK
    sysAuxClkDisable();
#endif

    vxMsrSet(0);

    /* Clear unnecessary TLBs */

    mmuE500TlbDynamicInvalidate();
    mmuE500TlbStaticInvalidate();

    (*pRom) (startType);    /* jump to bootrom entry point */

    return(OK);    /* in case we ever continue from ROM monitor */
    }

/******************************************************************************
*
* sysHwInit2 - additional system configuration and initialization
*
* This routine connects system interrupts and does any additional
* configuration necessary.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysHwInit2 (void)
    {

#ifdef  INCLUDE_VXBUS
    vxbDevInit();
#endif /* INCLUDE_VXBUS */

#ifdef INCLUDE_SYSLED

     /* clear the LED display */

     sysLedInit();

     /* cascade the demo LEDs */

     sysDemoLedsCascade();

#ifdef INCLUDE_AUX_CLK
    sysAuxClkRateSet(AUX_CLK_RATE);
    sysAuxClkConnect((FUNCPTR)sysLedClkRoutine, 0);
    sysAuxClkEnable();
#endif

#endif /* INCLUDE_SYSLED */

    /*
     * This was previously reqd for errata workaround #29, the workaround
     * has been replaced with patch for spr99776, so it now serves as an
     * example of implementing an l1 instruction parity handler
     */

#ifdef INCLUDE_L1_IPARITY_HDLR_INBSP
    memcpy((void*)_EXC_OFF_L1_PARITY, (void *)jumpIParity, sizeof(INSTR));
    cacheTextUpdate((void *)_EXC_OFF_L1_PARITY, sizeof(INSTR));
    sysIvor1Set(_EXC_OFF_L1_PARITY);
    cacheDisable(INSTRUCTION_CACHE);
    vxL1CSR1Set(vxL1CSR1Get() | _PPC_L1CSR_CPE);
    cacheEnable(INSTRUCTION_CACHE);
#endif  /* INCLUDE_L1_IPARITY_HDLR_INBSP */

#if defined(INCLUDE_VXBUS) && defined(INCLUDE_SIO_UTILS)
    sysSerialConnectAll();
#endif

#if     defined (INCLUDE_SPE)
    _func_speProbeRtn = sysSpeProbe;
#endif  /* INCLUDE_SPE */


#ifdef  INCLUDE_VXBUS
    taskSpawn("devConnect",0,0,10000,vxbDevConnect,0,0,0,0,0,0,0,0,0,0);
#endif /* INCLUDE_VXBUS */

	 setBR1(0xf0000801,0xfff06e65);/***0xf0000801,0xfff06e65*********************/
  	 setBR2( 0xd1001001,0xfff06e65/*0xfff06e25 0xfff06621*/);/****0xd1001001,0xfff06ff7*******************/
  	 setBR3( 0xd0001001,0xfff06f65);/****0xd0001001,0xfff06ff7*******************/

//	  bspCreateRamDisk();
	  *M85XX_GPIOCR(CCSBAR) |= 0x00030200;/**pin 14,15,22***/
         *M85XX_GPOUTDR(CCSBAR) |= 0x00580000;/*10*\11\12 high**//***9 high fan control***/
        //    i2cDrvInit(0,0);
  #ifdef INCLUDE_ATA
        sysAtaInit(0);  /* device zero used first time through */
  //	  setBR2( 0xd0001001,0xfff06ff7);/****0xd0001001,0xfff06ff7*******************/
     
	// setBR1(0xf0000801,0xfff06e65);/***0xf0000801,0xfff06e65*********************/
  //    *M85XX_GPIOCR(CCSBAR) |= 0x00030200;/**pin 14,15,22***/
    //    sysAtaInit(0);  /* device zero used first time through */
    #endif /* INCLUDE_ATA */

    }

/******************************************************************************
*
* sysProcNumGet - get the processor number
*
* This routine returns the processor number for the CPU board, which is
* set with sysProcNumSet().
*
* RETURNS: The processor number for the CPU board.
*
* ERRNO: N/A
*
* SEE ALSO: sysProcNumSet()
*/

int sysProcNumGet (void)
    {
    return(sysProcNum);
    }

/******************************************************************************
*
* sysProcNumSet - set the processor number
*
* This routine sets the processor number for the CPU board.  Processor numbers
* should be unique on a single backplane.
*
* Not applicable for the bus-less 8260Ads.
*
* RETURNS: N/A
*
* ERRNO: N/A
*
* SEE ALSO: sysProcNumGet()
*/

void sysProcNumSet
    (
    int     procNum         /* processor number */
    )
    {
    sysProcNum = procNum;
    }

/******************************************************************************
*
* sysLocalToBusAdrs - convert a local address to a bus address
*
* This routine gets the VMEbus address that accesses a specified local
* memory address.
*
* Not applicable for the 8260Ads
*
* RETURNS: ERROR, always.
*
* ERRNO: N/A
*
* SEE ALSO: sysBusToLocalAdrs()
*/

STATUS sysLocalToBusAdrs
    (
    int     adrsSpace,  /* bus address space where busAdrs resides */
    char *  localAdrs,  /* local address to convert */
    char ** pBusAdrs    /* where to return bus address */
    )
    {

    *pBusAdrs = localAdrs;

    return(OK);
    }

/******************************************************************************
*
* sysBusToLocalAdrs - convert a bus address to a local address
*
* This routine gets the local address that accesses a specified VMEbus
* physical memory address.
*
* Not applicable for the 8260Ads
*
* RETURNS: ERROR, always.
*
* ERRNO: N/A
*
* SEE ALSO: sysLocalToBusAdrs()
*/

STATUS sysBusToLocalAdrs
    (
    int     adrsSpace,  /* bus address space where busAdrs resides */
    char *  busAdrs,    /* bus address to convert */
    char ** pLocalAdrs  /* where to return local address */
    )
    {

    *pLocalAdrs = busAdrs;

    return(OK);
    }


/******************************************************************************
*
* sysUsDelay - delay at least the specified amount of time (in microseconds)
*
* This routine will delay for at least the specified amount of time using the
* lower 32 bit "word" of the Time Base register as the timer.
*
* NOTE:  This routine will not relinquish the CPU; it is meant to perform a
* busy loop delay.  The minimum delay that this routine will provide is
* approximately 10 microseconds.  The maximum delay is approximately the
* size of UINT16; however, there is no roll-over compensation for the total
* delay time, so it is necessary to back off two times the system tick rate
* from the maximum.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysUsDelay
    (
    UINT32    delay        /* length of time in microsec to delay */
    )
    {
    register UINT baselineTickCount;
    register UINT curTickCount;
    register UINT terminalTickCount;
    register int actualRollover = 0;
    register int calcRollover = 0;
    UINT ticksToWait;
    UINT requestedDelay;
    UINT oneUsDelay;

    /* Exit if no delay count */

    if ((requestedDelay = delay) == 0)
        return;

    /*
     * Get the Time Base Lower register tick count, this will be used
     * as the baseline.
     */

    baselineTickCount = sysTimeBaseLGet();

    /*
     * Calculate number of ticks equal to 1 microsecond
     *
     * The Time Base register and the Decrementer count at the same rate:
     * once per 8 System Bus cycles.
     *
     * e.g., 199999999 cycles     1 tick      1 second            25 ticks
     *       ----------------  *  ------   *  --------         ~  --------
     *       second               8 cycles    1000000 microsec    microsec
     */

    /* add to round up before div to provide "at least" */

    oneUsDelay = ((sysTimerClkFreq + 1000000) / 1000000);

    /* Convert delay time into ticks */

    ticksToWait = requestedDelay * oneUsDelay;

    /* Compute when to stop */

    terminalTickCount = baselineTickCount + ticksToWait;

    /* Check for expected rollover */

    if (terminalTickCount < baselineTickCount)
        {
        calcRollover = 1;
        }

    do
        {

        /*
         * Get current Time Base Lower register count.
         * The Time Base counts UP from 0 to
         * all F's.
         */

        curTickCount = sysTimeBaseLGet();

        /* Check for actual rollover */

        if (curTickCount < baselineTickCount)
            {
            actualRollover = 1;
            }

        if (((curTickCount >= terminalTickCount)
             && (actualRollover == calcRollover)) ||
            ((curTickCount < terminalTickCount)
             && (actualRollover > calcRollover)))
            {

            /* Delay time met */

            break;
            }
        }
    while (TRUE); /* breaks above when delay time is met */
    }
#include "time.h"
#ifdef  INCLUDE_RTC_NVRAM_SUPPORT
#include "rtc_m48.c"
#endif 
void bspGetTime( char *pbuf);
void bspSetTime(char*pbuf);
#include "bootLib.c"
#include "sysBtsConfigData.c" 
T_TimeDate BSP_GetDateTime();
void bspSetSystemTime()
{
    struct tm time_s;
    T_TimeDate timeDate;
    struct timespec timeSpec;
    unsigned int secCount;
    char   pbuffer[7];
 if((Hard_VerSion<5)||(Hard_VerSion>15))
 {
    rtcDrv();
    rtcDevCreate(NVRAM_BASE_ADDR);
 }
    timeDate = BSP_GetDateTime();
    if(timeDate.second>59)
    	{
    	   	time_s.tm_sec = 0;
    	}
    else
    	{
   		 time_s.tm_sec = timeDate.second;
    	}
    if(timeDate.minute>59)
    	{
    	  	 time_s.tm_min= 25;
    	}
    else
    	{
   		 time_s.tm_min = timeDate.minute;
    	}
    if( timeDate.hour>23)
    	{

    	    time_s.tm_hour  = 10;
    	}
    else
    	{
   		 time_s.tm_hour = timeDate.hour;
    	}
      if(timeDate.day>31)
      	{
      	   time_s.tm_mday = 24;
      	}
      else
      	{
    		time_s.tm_mday = timeDate.day;
      	}
      if(timeDate.month>12)
      	{
      		time_s.tm_mon = 4;
      	}
      else
      	{
    		time_s.tm_mon = timeDate.month - 1;
      	}

      if ( timeDate.year < 2010 ||timeDate.year>2030)
      	{
      	    time_s.tm_year = 2011 - 1900;
      	}
      else
     	{
   		 time_s.tm_year = timeDate.year - 1900;
     	}
    time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

    secCount = mktime(&time_s);
    printf("Time:%d,%d,%d,%d,%d,%d,%d\n",time_s.tm_sec ,time_s.tm_min, time_s.tm_hour ,timeDate.day,timeDate.month,timeDate.year ,time_s.tm_year);
    timeSpec.tv_sec = secCount;    
    timeSpec.tv_nsec = 0;
    clock_settime(CLOCK_REALTIME, &timeSpec);

     if ( timeDate.year < 2010 ||timeDate.year>2030|| timeDate.month > 12 || timeDate.day > 31 || timeDate.hour >23 || timeDate.minute > 59 || timeDate.second >59 )
    {
        timeDate.year = 2011;
        timeDate.month = 5;
        timeDate.day = 24;
        timeDate.hour = 10;
        timeDate.minute = 25;
        timeDate.second = 0;
        if((Hard_VerSion<5)||(Hard_VerSion>15))
        {
	       rtcDateSet(timeDate.year, timeDate.month, timeDate.day, 0/*dayOfWeek*/);
	       rtcTimeSet(timeDate.hour, timeDate.minute, timeDate.second);
	     //  rebootBTS(0);
       }
        else
        {
                    pbuffer[0] = 0x59;/********59秒*************/
		      pbuffer[1] = 0x37;/********37分*********/
		      pbuffer[2] = 0x13;/********13点*********/
		      pbuffer[3] = 0x22;/*******22号*****/
		      pbuffer[4] = 0x05;/*******星期五**************/
		      pbuffer[5] = 0x04;/*******4月********/
		      pbuffer[6] = 0x11;/***2011年**/
        	  bspSetTime(pbuffer);
        }
    }



    
}
/* NVRAM code */


/******************************************************************************* 
* sysNvRamRead - read from eeprom.
*
* This routine reads a single byte from a specified offset in NVRAM.
*
* RETURNS: N/A.
*
* NOMANUAL
*/

UINT8 sysNvRead
    (
    ULONG offset  /* NVRAM offset to read the byte from */
    )
    {
    UCHAR data;
    data = *(unsigned char*)(NVRAM_BASE_ADDR + offset);
    return data;
    }

/******************************************************************************* 
* sysNvRamWrite - write to eeprom.
*
* This routine writes a single byte to a specified offset in NVRAM.
*
* RETURNS: N/A.
*
* NOMANUAL
*/

void sysNvWrite
    (
    ULONG	offset,	/* NVRAM offset to write the byte to */
    UCHAR	data	/* datum byte */
    )
    {
    volatile unsigned char * 	ptr;
    int		 		count = 500;

    ptr = (unsigned char*)(NVRAM_BASE_ADDR + offset);

    *ptr = data;
    
    while (*ptr != data)
        {
        
        sysMsDelay (5); /* 5 milliseconds */

        if (count-- == 0) /* abort */
            break;
        }
    }


/*****oam if******
    UINT16  year;		
    UCHAR   month;
    UCHAR   day;
    UCHAR   hour;		
    UCHAR   minute;		
    UCHAR   second;	

s----
minutes---
hours------
days-------
week------
months-----
years-----


************/

#include <time.h>
T_TimeDate bspGetDateTime()
{
    T_TimeDate pDateTime;
  //  char   pbuffer[7];
    #if 0
  if((Hard_VerSion<5)||(Hard_VerSion>15))
  	{
  		 rtcDateTimeGet(&pDateTime);
  	}
  else
  	{
  	     bspGetTime(pbuffer);
  	     pDateTime.year = pbuffer[6] +2000;
  	     pDateTime.month = pbuffer[5];
  	     pDateTime.day = pbuffer[3];
  	     pDateTime.hour = pbuffer[2];
  	     pDateTime.minute = pbuffer[1];
  	     pDateTime.second = pbuffer[0];
  	}
  #endif
  	time_t timer = time(NULL);

	struct tm TimeStru;
	localtime_r(&timer, &TimeStru);
      	pDateTime.year =  (TimeStru.tm_year+1900);
	pDateTime.month = TimeStru.tm_mon+1;
	 pDateTime.day= TimeStru.tm_mday;
	 pDateTime.hour = TimeStru.tm_hour;
	pDateTime.minute = TimeStru.tm_min;
	pDateTime.second = TimeStru.tm_sec;
	
    return pDateTime;	
}



T_TimeDate BSP_GetDateTime()
{
    T_TimeDate pDateTime;
    char   pbuffer[7];
  if((Hard_VerSion<5)||(Hard_VerSion>15))
  	{
  		 rtcDateTimeGet(&pDateTime);
  	}
  else
  	{
  	     bspGetTime(pbuffer);
  	     pDateTime.year = pbuffer[6] +2000;
  	     pDateTime.month = pbuffer[5];
  	     pDateTime.day = pbuffer[3];
  	     pDateTime.hour = pbuffer[2];
  	     pDateTime.minute = pbuffer[1];
  	     pDateTime.second = pbuffer[0];
  	}
    return pDateTime;	
}

void bspDateSet(int year,int month,int dayOfMonth,int dayOfWeek)
{
	rtcDateSet(year, month, dayOfMonth, dayOfWeek);
}
    
void bspTimeSet(int hour,int minute,int second)
{
	rtcTimeSet( hour, minute, second);
}


/***nvram***********************/
int nvram_write(int addr, char *data, int len){
	memcpy((unsigned char*)(NVRAM_BASE_ADDR + addr), data, len);
	return OK;
}

char * nvram_read(int addr, char *data, int len){
	memcpy(data, (unsigned char*)(NVRAM_BASE_ADDR + addr), len);
	return data;
}


int bspSetNvRamWrite(char *startAddr, UINT32 size, UINT8 flag)
{

    char * start;
    int len;
    UINT32 temp;
 //    UINT32 taskid;
//     int i;
     UINT8 flag1 ;
    UINT32 pageSize ;
    pageSize= vmBasePageSizeGet();

     flag1 = 0;

    if ( (UINT32)startAddr < NVRAM_BASE_ADDR || (UINT32)(startAddr+size)>= (NVRAM_BASE_ADDR + NVRAM_SIZE))
    {
        logMsg("bspSetNvRamWrite invalid address 0x%x, len=%d, flag=%d\n", (UINT32)startAddr, size,flag,0,0,0);
      taskSuspend(0);
#if 0
     taskid = taskIdSelf();
    for(i =0; i < 40; i++)
    {
    	    if(task_suspend_id[i] == taskid)
    	    	{
    	    	    task_suspend_time[i]++;
		   flag1 =1;
		    break;
    	    	}
    	}
	if(flag1==0)
	{
		  if(task_id_index<40)
		  {
		   task_suspend_id[task_id_index]=taskid;
		   task_suspend_time[task_id_index]++;
		  }
		   task_id_index++;
	}
	
        return FALSE;
#endif
        return FALSE;
    }

    start = (char *)((UINT32)startAddr & (~ (pageSize- 1)));
    temp = (UINT32)startAddr & (pageSize-1);
    len = (temp + size-1) - ((temp + size -1) & (pageSize -1)) + pageSize;
    if(((unsigned int)start+len) >= (NVRAM_BASE_ADDR + NVRAM_SIZE))
    {
        len = len - pageSize;   /**use to rtc*****/
    }

    if(flag==1)
    {
        vmBaseStateSet(NULL, start , len , VM_STATE_MASK_WRITABLE, VM_STATE_WRITABLE);
	}
    else
    {
        vmBaseStateSet(NULL, start , len , VM_STATE_MASK_WRITABLE, VM_STATE_WRITABLE_NOT);
	}

	return TRUE;
}


int bspEnableNvRamWrite(char *startAddr, UINT32 size)
{
    return bspSetNvRamWrite(startAddr, size, 1);
}

int bspDisableNvRamWrite(char *startAddr, UINT32 size)
{
    return bspSetNvRamWrite(startAddr, size, 0);
}


#include <taskLib.h>
/*TargAddr is offset*/
int bspNvRamWrite(char * TargAddr, char *ScrBuff, int size)
{
	char * start;
	int len;
	UINT32 temp;
    UINT32 pageSize = vmBasePageSizeGet();
 //  int i;
 //  unsigned int taskid;
//   unsigned char flag;
	start = (char *)((UINT32)TargAddr & (~ (pageSize- 1)));
    temp = (UINT32)TargAddr & (pageSize-1);
    len = (temp + size-1) - ((temp + size -1) & (pageSize -1)) + pageSize;
	if(((unsigned int)start+len) >= (NVRAM_BASE_ADDR + NVRAM_SIZE))
    {
		len = len - pageSize;   /**use to rtc*****/
	}

    if ((UINT32)TargAddr < NVRAM_BASE_ADDR || (UINT32)(TargAddr+size)>= (NVRAM_BASE_ADDR + NVRAM_SIZE))
    {
        logMsg("bspNvRamWrite invalid address 0x%x, len=%d\n", (UINT32)TargAddr, size,0,0,0,0);
       taskSuspend(0);
	  #if 0
           taskid = taskIdSelf();
	    for(i =0; i < 40; i++)
	    {
	    	    if(task_suspend_id[i] == taskid)
	    	    	{
	    	    	    task_suspend_time[i]++;
			   flag =1;
			    break;
	    	    	}
	    	}
		if(flag==0)
		{
			  if(task_id_index<40)
			  {
			   task_suspend_id[task_id_index]=taskid;
			   task_suspend_time[task_id_index]++;
			  }
			   task_id_index++;
		}
	
          return FALSE;
		#endif
		 return FALSE;
    }

	vmBaseStateSet(NULL, start , len , VM_STATE_MASK_WRITABLE, VM_STATE_WRITABLE);
	memcpy(TargAddr, ScrBuff, size);
	vmBaseStateSet(NULL, start , len , VM_STATE_MASK_WRITABLE, VM_STATE_WRITABLE_NOT);

	//return OK;
	return TRUE;
}

STATUS bspNvRamRead(char * TargAddr, char *SrcBuff, int size)
{
    UINT32 srcAddr = (UINT32)SrcBuff;

    if ( srcAddr >= NVRAM_BASE_ADDR && (srcAddr + size < (NVRAM_BASE_ADDR + NVRAM_SIZE)))
    {
        /*return nvram_read((int)ScrBuff + OAM_ALL_OFFSET, TargAddr, size);*/
        memcpy(TargAddr, SrcBuff, size);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void sysMsDelay
    (
    UINT      delay                   /* length of time in MS to delay */
    )
    {
    sysUsDelay ( (UINT32) delay * 100 );
    }


/*********************************************************************
*
* sysDelay - Fixed 1ms delay.
*
* This routine consumes 1ms of time. It just calls sysMsDelay.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysDelay (void)
    {
    sysMsDelay (1);
    }

/***************************************************************************
*
* sysIntConnect - connect the BSP interrupt to the proper epic/i8259 handler.
*
* This routine checks the INT level and connects the proper routine.
* pciIntConnect() or intConnect().
*
* RETURNS:
* OK, or ERROR if the interrupt handler cannot be built.
*
* ERRNO: N/A
*/

STATUS sysIntConnect
    (
    VOIDFUNCPTR *vector,        /* interrupt vector to attach to     */
    VOIDFUNCPTR routine,        /* routine to be called              */
    int parameter               /* parameter to be passed to routine */
    )
    {
    int tmpStat = ERROR;
    UINT32 read;

    if (((int) vector < 0) || ((int) vector >= EXT_VEC_IRQ0 + EXT_MAX_IRQS))
        {
        logMsg ("Error in sysIntConnect: out of range vector = %d.\n",
                (int)vector,2,3,4,5,6);

        return(ERROR);
        }

    if (vxMemProbe ((char *) routine, VX_READ, 4, (char *) &read) != OK)
        {
        logMsg ("Error in sysIntConnect: Cannot access routine.\n",
                1,2,3,4,5,6);
        return(ERROR);
        }

    if ((int) vector < EXT_VEC_IRQ0)
        {
        tmpStat = intConnect (vector, routine, parameter);
        }
    else
        {

        /*
	 * call external int controller connect
         * tmpStat = cascadeIntConnect (vector, routine, parameter);
	 */
        }

    if (tmpStat == ERROR)
        {
        logMsg ("Error in sysIntConnect: Connecting vector = %d.\n",
                (int)vector,2,3,4,5,6);
        }

    return(tmpStat);
    }

/*******************************************************************************
*
* sysIntEnable - enable an interrupt
*
* This function call is used to enable an interrupt.
*
* RETURNS: OK or ERROR if unable to enable interrupt.
*
* ERRNO: N/A
*/

STATUS sysIntEnable
    (
    int intNum
    )
    {
    int tmpStat = ERROR;
    if (((int) intNum < 0) || ((int) intNum >= EXT_NUM_IRQ0 + EXT_MAX_IRQS))
        {
        logMsg ("Error in sysIntEnable: Out of range intNum = %d.\n",
                (int)intNum,2,3,4,5,6);

        return(ERROR);
        }

    if ((int) intNum < EXT_NUM_IRQ0)
        {
        tmpStat = intEnable (intNum);
        }
    else
        {
        /* call external int controller connect */

        tmpStat = sysCascadeIntEnable (intNum - EXT_NUM_IRQ0);
        }

    if (tmpStat == ERROR)
        {
        logMsg ("Error in sysIntEnable: intNum = %d.\n",
                (int)intNum,2,3,4,5,6);
        }

    return(tmpStat);
    }

/****************************************************************************
*
* sysCascadeIntEnable - enable an external controller interrupt
*
* This function call is used to enable an interrupt outside of the MPC8540 PIC.
*
* RETURNS: OK or ERROR if unable to enable interrupt.
*
* ERRNO: N/A
*/

STATUS sysCascadeIntEnable
    (
    int intNum
    )
    {
    return(ERROR);
    }

/****************************************************************************
*
* sysIntDisable - disable an interrupt
*
* This function call is used to disable an interrupt.
*
* RETURNS: OK or ERROR if unable to disable interrupt.
*
* ERRNO: N/A
*/

STATUS sysIntDisable
    (
    int intNum
    )
    {
    int tmpStat = ERROR;
    if (((int) intNum < 0) || ((int) intNum >= EXT_NUM_IRQ0 + EXT_MAX_IRQS))
        {
        logMsg ("Error in sysIntDisable: Out of range intNum = %d.\n",
                (int)intNum,2,3,4,5,6);

        return(ERROR);
        }

    if ((int) intNum < EXT_NUM_IRQ0)
        {
        tmpStat = intDisable (intNum);
        }
    else
        {
        /* call external int controller connect */

        tmpStat = sysCascadeIntDisable (intNum - EXT_NUM_IRQ0);
        }


    if (tmpStat == ERROR)
        {
        logMsg ("Error in sysIntDisable: intNum = %d.\n",
                (int)intNum,2,3,4,5,6);
        }

    return(tmpStat);
    }

/**************************************************************************
*
* sysCascadeIntDisable - disable an external controller interrupt
*
* This function call is used to disable an interrupt outside of the MPC8540 PIC.
*
* RETURNS: OK or ERROR if unable to disable interrupt.
*
* ERRNO: N/A
*/

STATUS sysCascadeIntDisable
    (
    int intNum
    )
    {
    return(ERROR);
    }


#ifdef INCLUDE_ATA

/******************************************************************************
*
* sysAtaInit - initialize the EIDE/ATA interface
*
* Perform the necessary initialization required before starting up the
* ATA/EIDE driver.
*/

void sysAtaInit
(
int ctrl
)
{
/** device cs 0

io address
primary:
    1F0h-1F7h,
    3F6h-3F7h
secondary:
    170h-177h,
    376h-377h
**/
    ataResources[0].resource.ioStart[0] = 0xd0000000 ;   /**???????????????***/
    ataResources[0].resource.ioStart[1] = 0xd0000000 + 0x20; /**???????????????***/


    /*
     * initialize the Controller 0 data structure
     */

    ataResources[0].ctrlType   = IDE_LOCAL;   /* controller type: IDE_LOCAL or ATA_PCMCIA */
    ataResources[0].drives     = 1;           /* 1,2: number of drives */
    ataResources[0].intVector  = (EXT_VEC_IRQ0 +11);//INT_VEC_P0_GPP24_31;  /* MPP 26  ????????? Interrupt number */
    ataResources[0].intLevel   = 11;	//10;  /* MPP 26  ????????? Interrupt level */
    ataResources[0].configType = (ATA_PIO_AUTO | ATA_GEO_PHYSICAL |ATA_BITS_16);    /* 0,1: configuration type */
    ataResources[0].semTimeout = 0;   /* timeout seconds for sync semaphore */
    ataResources[0].wdgTimeout = 0;    /* timeout seconds for watch dog */

    ataResources[0].sockTwin = 0;               /* socket number for twin card */
    ataResources[0].pwrdown = 0;                /* power down mode */
}


/*******************************************************************************
*
* sysInByteString - reads a string of bytes from an io address.
*
* This function reads a byte string from a specified o address.
*
* RETURNS: N/A
*/

void sysInByteString
(
ULONG   ioAddr,
char *  bufPtr,
int     nBytes
)
{
    int loopCtr;

    for ( loopCtr = 0; loopCtr < nBytes; loopCtr++ )
        *bufPtr++ = *(char *)ioAddr;
   PPC_EIEIO_SYNC;
}

/*******************************************************************************
*
* sysOutByteString - writes a string of bytes to an io address.
*
* This function writes a byte string to a specified io address.
*
* RETURNS: N/A
*/

void sysOutByteString
(
ULONG   ioAddr,
char *  bufPtr,
int     nBytes
)
{
    int loopCtr;

    for ( loopCtr = 0; loopCtr < nBytes; loopCtr++ )
        *(char *)ioAddr = *bufPtr++;
    PPC_EIEIO_SYNC;
}

/*******************************************************************************
*
* sysInWordString - reads a string of words from an io address.
*
* This function reads a word string from a specified io address.
*
* RETURNS: N/A
*/

void sysInWordString(ULONG   ioAddr,
                     UINT16 *    bufPtr,
                     int     nWords
                     )
{
    int loopCtr;

    for ( loopCtr = 0; loopCtr < nWords; loopCtr++ )
        *bufPtr++ = *(short *)ioAddr;
    PPC_EIEIO_SYNC;
}

/*******************************************************************************
*
* sysInWordStringRev - reads a string of words that are byte reversed
*
* This function reads a string of words that are byte reversed from a
* specified io address.
*
* RETURNS: N/A
*/

void sysInWordStringRev(ULONG      ioAddr,
                        UINT16 *   bufPtr,
                        int        nWords
                        )
{
    int loopCtr;

    for ( loopCtr = 0; loopCtr < nWords; loopCtr++ )
        *bufPtr++ = sysInWord (ioAddr);
}

/*******************************************************************************
*
* sysOutWordString - writes a string of words to an io address.
*
* This function writes a word string from a specified io address.
*
* RETURNS: N/A
*/

void sysOutWordString(ULONG   ioAddr,
                      UINT16 *    bufPtr,
                      int     nWords
                      )
{
  int loopCtr;

    for ( loopCtr = 0; loopCtr < nWords; loopCtr++ )
        *(short *)ioAddr = *bufPtr++;
   PPC_EIEIO_SYNC;
 
//   sysOutWordStringRev(ioAddr,bufPtr,nWords);
}
/*******************************************************************************
*
* sysOutWordStringRev - writes a string of words that bytes are reversed to an io address.
*
* This function writes a word string from a specified io address.
*
* RETURNS: N/A
*/

void sysOutWordStringRev(ULONG   ioAddr,
                         UINT16 *    bufPtr,
                         int     nWords
                         )
{
    int loopCtr;
    char temp[2];

    char* p = (char*) bufPtr;

    for ( loopCtr = 0; loopCtr < nWords; loopCtr++ )
    {
        temp[0] = p[2*loopCtr + 1];
        temp[1] = p[2*loopCtr];

        *(short *)ioAddr = *(short*)temp;
    }
   PPC_EIEIO_SYNC;


}

/*******************************************************************************
*
* sysInLongString - reads a string of longwords from an io address.
*
* This function reads a longword string from a specified io address.
*
* RETURNS: N/A
*/
void sysInLongString(ULONG    ioAddr,
                     ULONG *  bufPtr,
                     int      nLongs
                     )
{
    int loopCtr;

    for ( loopCtr = 0; loopCtr < nLongs; loopCtr++ )
        *bufPtr++ = *(int *)ioAddr;
   PPC_EIEIO_SYNC;
}

/*******************************************************************************
*
* sysOutLongString - writes a string of longwords to an io address.
*
* This function writes a longword string from a specified io address.
*
* RETURNS: N/A
*/

void sysOutLongString(ULONG   ioAddr,
                      ULONG * bufPtr,
                      int     nLongs 
                      )
{
    int loopCtr;

    for ( loopCtr = 0; loopCtr < nLongs; loopCtr++ )
        *(int *)ioAddr = *bufPtr++;
 PPC_EIEIO_SYNC;
}



/*******************************************************************************
*
* sysDelay - delay for approximately one millisecond
*
* Delay for approximately one milli-second.
*
* RETURNS: N/A
*/
/*
void sysDelay (void)
{
    sysMsDelay (1);
}
*/

#if 0
/*******************************************************************************
*
* sysIntEnablePIC - enable an ISA/PCI interrupt
*
* This function call is used to enable an ISA/PCI interrupt.
*
* RETURNS: OK or ERROR if unable to enable interrupt.
*/

STATUS sysIntEnablePIC(int intNum)
{

    if ( (intNum < INT_LVL_MIN) || (intNum > INT_LVL_MAX) )
    {
        logMsg ("sysIntEnablePIC: Invalid interrupt number %d.\n", 
        intNum,2,3,4,5,6);
        return(ERROR);
    }

    return(intEnable (intNum));
}

#else

STATUS sysIntEnablePIC(int intNum)
{
 //   return(sysGpioIntEnable(intNum));
  *M85XX_EIVPR11(CCSBAR)&=0x7f3fffff;
//    *M85XX_EIVPR11(CCSBAR)|=0x008a0043;
    *M85XX_EIVPR11(CCSBAR)|=0x00ca0000;    //ghbit
 //   return(sysGpioIntEnable(intNum));
 return 0;

}

#endif

#endif /* INCLUDE_ATA */


#ifdef INCLUDE_CACHE_SUPPORT
/***********************************************************************
*
* sysL1CacheQuery - configure L1 cache size and alignment
*
* Populates L1 cache size and alignment from configuration registers.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void sysL1CacheQuery(void)
    {
    UINT32 temp;
    UINT32 align;
    UINT32 cachesize;

    temp = vxL1CFG0Get();

    cachesize = (temp & 0xFF) << 10;

    align = (temp >> 23) & 0x3;


    switch (align)
        {
        case 0:
            ppcE500CACHE_ALIGN_SIZE=32;
            break;
        case 1:
            ppcE500CACHE_ALIGN_SIZE=64;
            break;
        default:
            ppcE500CACHE_ALIGN_SIZE=32;
            break;
        }

    ppcE500DCACHE_LINE_NUM = (cachesize / ppcE500CACHE_ALIGN_SIZE);
    ppcE500ICACHE_LINE_NUM = (cachesize / ppcE500CACHE_ALIGN_SIZE);

    /*
     * The core manual suggests for a 32 byte cache line and 8 lines per set
     * we actually need 12 unique address loads to flush the set.
     * The number of lines to flush should be ( 3/2 * cache lines )
     */

    ppcE500DCACHE_LINE_NUM = (3*ppcE500DCACHE_LINE_NUM)>>1;
    ppcE500ICACHE_LINE_NUM = (3*ppcE500ICACHE_LINE_NUM)>>1;

    }

#endif /* INCLUDE_CACHE_SUPPORT */

/***************************************************************************
*
* saveExcMsg - write exception message to save area for catastrophic error
*
* The message will be displayed upon rebooting with a bootrom.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void saveExcMsg
    (
    char *errorMsg
    )
    {
    strcpy ((char *)EXC_MSG_OFFSET, errorMsg);
    }


void chipErrataCpu29Print(void)
    {
    saveExcMsg("Silicon fault detected, possible machine state corruption.\nSystem rebooted to limit damage.");
    }



/***************************************************************************
*
* vxImmrGet - get the CPM DPRAM base address
*
* This routine returns the CPM DP Ram base address for CPM device drivers.
*
* RETURNS:
*
* ERRNO: N/A
*/

UINT32 vxImmrGet(void)
    {
    return(CCSBAR + 0x80000);
    }

/***************************************************************************
*
* sysGetPeripheralBase   - Provides CCSRBAR address fro security engine
*                          drivers.
*
* RETURNS:
*
* ERRNO: N/A
*/
UINT32 sysGetPeripheralBase()
    {
    return(CCSBAR);
    }

#if defined (_GNU_TOOL)
void sysIntHandler (void)
    {
    }

void vxDecInt (void)
    {
    }

int excRtnTbl = 0;
#endif  /* _GNU_TOOL */

#ifdef INCLUDE_SYSLED

/***********************************************************************
*
* sysLedClkRoutine - shows clock activity on LED
*
* This routine blinks the dot on the Hex LED
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void sysLedClkRoutine
    (
    int arg
    )
    {
    static int clkIntCount = 0;
    static INT8 clkCount = 0;

    if((clkIntCount++ % 60) == 0)
        {
        (clkCount++ & 0x1) ?
            sysLedSet(LED_POINT, LED_POINT) : sysLedSet(LED_POINT, 0);
        }
    }

#endif /* INCLUDE_SYSLED */

#define INCLUDE_SHOW_ROUTINES
#ifdef INCLUDE_SHOW_ROUTINES

/***************************************************************************
*
* coreLbcShow - Show routine for local bus controller
*
* This routine shows the local bus controller registers.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void coreLbcShow(void)
    {
    VINT32 tmp, tmp2;

    tmp = * (VINT32 *) M85XX_BR0(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR0(CCSBAR);
    printf("Local bus BR0 = 0x%x\tOR0 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR1(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR1(CCSBAR);
    printf("Local bus BR1 = 0x%x\tOR1 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR2(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR2(CCSBAR);
    printf("Local bus BR2 = 0x%x\tOR2 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR3(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR3(CCSBAR);
    printf("Local bus BR3 = 0x%x\tOR3 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR4(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR4(CCSBAR);
    printf("Local bus BR4 = 0x%x\tOR4 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR5(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR5(CCSBAR);
    printf("Local bus BR5 = 0x%x\tOR5 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR6(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR6(CCSBAR);
    printf("Local bus BR6 = 0x%x\tOR6 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_BR7(CCSBAR);
    tmp2 = * (VINT32 *) M85XX_OR7(CCSBAR);
    printf("Local bus BR7 = 0x%x\tOR7 = 0x%x\n", tmp, tmp2);

    tmp = * (VINT32 *) M85XX_LBCR(CCSBAR);
    printf("Local bus LBCR = 0x%x\n", tmp);

    tmp = * (VINT32 *) M85XX_LCRR(CCSBAR);
    printf("Local bus LCRR = 0x%x\n", tmp);
    }

    #define xbit0(x, n)    ((x & (1 << (31 - n))) >> (31 - n))  /* 0..31 */
    #define xbit32(x, n)   ((x & (1 << (63 - n))) >> (63 - n))  /* 32..63 */
    #define onoff0(x, n)   xbit0(x, n) ? "ON", "OFF"
    #define onoff32(x, n)  xbit32(x, n) ? "ON", "OFF"

/***************************************************************************
*
* coreShow - Show routine for core registers
*
* This routine shows the core registers.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void coreShow(void)
    {
    VUINT32 tmp, tmp2;

    tmp = vxMsrGet();
    printf("MSR - 0x%x\n", tmp);
    printf("      UCLE-%x SPE-%x WE-%x CE-%x EE-%x PR-%x ME-%x\n",
           xbit32(tmp,37), xbit32(tmp,38), xbit32(tmp,45), xbit32(tmp,46),
           xbit32(tmp,48), xbit32(tmp,49), xbit32(tmp,51));
    printf("      UBLE-%x DE-%x IS-%x DS-%x PMM-%x\n",
           xbit32(tmp,53), xbit32(tmp,54), xbit32(tmp,58), xbit32(tmp,59),
           xbit32(tmp,61));
    tmp = vxHid0Get();
    tmp2 = vxHid1Get();
    printf("HID0 = 0x%x, HID1 = 0x%x\n", tmp, tmp2);
    tmp = vxL1CSR0Get();
    printf("L1CSR0: cache is %s - 0x%x\n", tmp&1?"ON":"OFF", tmp);
    tmp = vxL1CSR1Get();
    printf("L1CSR1: Icache is %s - 0x%x\n", tmp&1?"ON":"OFF", tmp);
    tmp = vxL1CFG0Get();
    tmp2 = vxL1CFG1Get();
    printf("L1CFG0 = 0x%x, L1CFG1 = 0x%x\n", tmp, tmp2);
    tmp = *(VUINT32 *) (CCSBAR + 0x20000);
    printf("L2CTL - 0x%x\n", tmp);
    printf("        l2 is %s\n", tmp&0x80000000?"ON":"OFF");
    printf("        l2siz-%x l2blksz-%x l2do-%x l2io-%x\n",
           (xbit0(tmp,2)<<1)|xbit0(tmp,3), (xbit0(tmp,4)<<1)|xbit0(tmp,5),
           xbit0(tmp,9), xbit0(tmp,10));
    printf("        l2pmextdis-%x l2intdis-%x l2sram-%x\n",
           xbit0(tmp,11), xbit0(tmp,12),
           (xbit0(tmp,13)<<2)|(xbit0(tmp,14)<<1)|xbit0(tmp,15));
    tmp = *(VUINT32 *) (CCSBAR + 0x20100);
    tmp2 = *(VUINT32 *) (CCSBAR + 0x20108);
    printf("L2SRBAR0 - 0x%x\n", tmp);
    printf("L2SRBAR1 - 0x%x\n", tmp2);

    printf("Core Freq = %3d Hz\n",coreFreq);
    printf("CCB Freq = %3d Hz\n",sysClkFreqGet());
    printf("PCI Freq = %3d Hz\n",OSCILLATOR_FREQ);
    printf("CPM Freq = %3d Hz\n",sysClkFreqGet());

    }

#endif
    void setBR3(unsigned int br0,unsigned int or0)/**0xd1001001,0xfff06ff7***/
     	{
     	   
     	    *M85XX_BR3(CCSBAR) = br0;
          *M85XX_OR3(CCSBAR) = or0;
     	}
	 void setBR2(unsigned int br0,unsigned int or0)/****0xd0001001,0xfff06ff7*******************/
     	{
     	    *M85XX_BR2(CCSBAR) = br0;
          *M85XX_OR2(CCSBAR) = or0;
     	}
	void setBR1(unsigned int br0,unsigned int or0)/***0xf0000801,0xfff06e65*********************/
     	{
     	    *M85XX_BR1(CCSBAR) = br0;
          *M85XX_OR1(CCSBAR) = or0;
     	}

	void setBR0(unsigned int br0,unsigned int or0)/***0xf0000801,0xfff06e65*********************/
     	{
     	    *M85XX_BR0(CCSBAR) = br0;
          *M85XX_OR0(CCSBAR) = or0;
     	}
#ifdef INCLUDE_PCI_BUS

LOCAL UCHAR sysPci1IntRoute [NUM_PCI1_SLOTS][4] =
    {PCI_XINT1_LVL, PCI_XINT2_LVL, PCI_XINT3_LVL, PCI_XINT4_LVL};

LOCAL UCHAR sysPci3IntRoute [NUM_PCIEX_SLOTS][4] =
    {PCIEX_XINT1_LVL, PCIEX_XINT2_LVL, PCIEX_XINT3_LVL, PCIEX_XINT4_LVL};

/*******************************************************************************
*
* sysPci1AutoconfigIntrAssign - PCI 1 autoconfig support routine
*
* This routine peforms the PCI 1 auto configuration interrupt assignment
* support function.
*
* RETURNS: PCI interrupt line number given pin mask
*/

UCHAR sysPci1AutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,                  /* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,                     /* pointer to function in question */
    UCHAR pin                           /* contents of PCI int pin register */
    )
    {
    UCHAR tmpChar = 0xff;

    /*
     * Ensure this is a reasonable value for bus zero.
     * If OK, return INT level, else we return 0xff.
     */

    if ((pin > 0) && (pin < 5))
        tmpChar = sysPci1IntRoute [0][(pin-1)];

    /* return the value to be assigned to the pin */

    return (tmpChar);
    }

/*******************************************************************************
*
* sysPci3AutoconfigIntrAssign - PCI Express autoconfig support routine
*
* This routine peforms the PCI Express auto configuration interrupt assignment
* support function.
*
* RETURNS: PCI interrupt line number given pin mask
*/

UCHAR sysPci3AutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,                  /* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,                     /* pointer to function in question */
    UCHAR pin                           /* contents of PCI int pin register */
    )
    {
    UCHAR tmpChar = 0xff;

    /*
     * Ensure this is a reasonable value for bus zero.
     * If OK, return INT level, else we return 0xff.
     */
    if (((pin > 0) && (pin < 5))                                &&
        (((pLoc->device) - sysPciSlotDeviceNumber) < NUM_PCIEX_SLOTS)   &&
        (((pLoc->device) - sysPciSlotDeviceNumber) >= 0))
        {
        tmpChar =
            sysPci3IntRoute [0][(pin-1)];
        }

    /* return the value to be assigned to the pin */

    return (tmpChar);
    }

/*******************************************************************************
*
* sysPci1AutoconfigInclude - PCI 1 autoconfig support routine
*
* This routine performs the PCI 1 auto configuration support function.
*
* RETURNS: OK or ERROR
*/

STATUS sysPci1AutoconfigInclude
    (
    PCI_SYSTEM * pSys,                  /* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,                     /* pointer to function in question */
    UINT devVend                        /* deviceID/vendorID of device */
    )
    {

    /*
     * Only support BUS 0;
     * Host controller itself (device number is 0) won't be configured;
     * Bridge on the Arcadia board (device number 17) won't be configured;
     */

    if ((pLoc->bus > 0)                                               ||
        (pLoc->bus == 0 && pLoc->device == 0 && pLoc->function == 0)  ||
        (pLoc->bus == 0 && pLoc->device == 18 && pLoc->function == 0) ||
        (devVend == PCI_ARCADIA_BRIDGE_DEV_ID))

        return(ERROR);


    return (OK); /* Autoconfigure all devices */
    }


/*******************************************************************************
*
* sysPci3AutoconfigInclude - PCI Express autoconfig support routine
*
* This routine performs the PCI Express auto configuration support function.
*
* RETURNS: OK or ERROR
*/

STATUS sysPci3AutoconfigInclude
    (
    PCI_SYSTEM * pSys,                  /* PCI_SYSTEM structure pointer */
    PCI_LOC * pLoc,                     /* pointer to function in question */
    UINT devVend                        /* deviceID/vendorID of device */
    )
    {

    /*
     * Only support BUS 0;
     * Host controller itself (device number is 0) won't be configured;
     */

    if ((pLoc->bus > 2) ||
        (pLoc->bus == 0 && pLoc->device == 0 && pLoc->function == 0) ||
        (pLoc->device > 0))
        return(ERROR);

    return OK; /* Autoconfigure all devices */
    }
#endif /* INCLUDE_PCI_BUS */

#ifdef INCLUDE_SIO_UTILS
#   define BSP_SERIAL_CHAN_GET  bspSerialChanGet
#else /* INCLUDE_SIO_UTILS */
#   define BSP_SERIAL_CHAN_GET  sysSerialChanGet
#endif /* INCLUDE_SIO_UTILS */

/*******************************************************************************
*
* BSP_SERIAL_CHAN_GET - get the serial channel
*
* This routine is called by the vxbus sio driver.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

SIO_CHAN *BSP_SERIAL_CHAN_GET
    (
    int channel
    )
    {
    return ((SIO_CHAN *)ERROR);
    }
 #include <ioLib.h>
   
 #include <netDrv.h>
#include <dosFsLib.h>
#include <stat.h>
#include <ramDrv.h>

#define DEVICE_RAMDISK		"/RAMD"   /*  "/RAMDISK/"   */
#define RAMDISK_BLOCK_SIZE	(1024*72*2)    /***(1024*72)* 512 = 32M*******/
#define RAMDISK_DOWNLOAD_PATH	"load/"
#define RAMDISK_DOWNLOAD_PATH_1 "cal/"
/*******************************

   pBlkDev = ramDevCreate (NULL,  512,  32,  416,  0);
    xbdBlkDevCreate (pBlkDev, "/ramDrv");
    dosFsVolFormat ("/ramDrv:0", DOS_OPT_BLANK, NULL);

*******************************/

STATUS bspCreateRamDisk()
{
    BLK_DEV               *pBlkDev; 
    DOS_VOL_DESC          *pVolDesc; 
    STATUS                      result;
    /********************/
 //  unsigned char *p;
//  p = (unsigned char *)malloc(64*)
   #if 1
    pBlkDev = ramDevCreate ((char *)0, 512, 1024, RAMDISK_BLOCK_SIZE/*1024*32*/, 0); /**512* (1024*32) = 16M*********/  
    logMsg("pBlkDev:%x\n",pBlkDev,1,2,3,4,5);
    if ( (pVolDesc = xbdBlkDevCreate(pBlkDev,DEVICE_RAMDISK )) == NULL ) 
    	{
    //	printf("he\n");
       logMsg("xbdBlkDevCreate error\n",0,1,2,3,4,5);
        return ERROR;
    	}
    
	result = dosFsVolFormat("/RAMD:0", 4, 0);
	//result = dosFsVolFormat("/RAMD:0", 2, 0);
	logMsg("result:%x\n",result,1,2,3,4,5);
    if (result /*dosFsVolFormat("/RAMD:0", 4, 0) */!= OK )  
   	{
    	//printf("hehe\n");
		if(dosFsVolFormat("/RAMD:0", 4, 0)!=OK)
		{
			if(dosFsVolFormat("/RAMD:0", 4, 0)!=OK)
			{
		        logMsg("dosFsVolFormat error\n",0,1,2,3,4,5);
	       		return ERROR;
			}
		}
    	}
   //   cd("/RAMD:0");
	//mkdir("load");
	#endif
  //xbdRamDiskDevCreate(512,1024*1024,FALSE,DEVICE_RAMDISK);

    return OK;
}


void testramdisk(const char *filename)
{
        char  filename1[200];
       int fd0, fd1;
	 char buffer[100];
	 char buffer1[100];
	 int i;
	 int count;
	 for(i = 0; i< 100; i++)
	 {
	    buffer[i] = i;
	    buffer1[i] =0;
	 }
      strcpy(filename1, "/RAMD:0/mcp.out");
    //strcat(filename1, RAMDISK_DOWNLOAD_PATH);
   // strcat(filename1, filename);
    fd1 = open(filename1, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if ( fd1 == ERROR )
    {
        printf( "Open file1 %s on RamDisk error! \n",(int)filename);
        return ERROR;
    }

    write(fd1, buffer, 100);  

   close(fd1);

    fd0 = open(filename1, O_RDONLY, 0664);
    if ( fd0 == ERROR )
    {
        printf( "Open file %s on RamDisk error! \n",(int)filename1);
        return ERROR;
    }

     count = read(fd0, buffer1, 100);   
     close(fd0);
   logMsg("count:%d\n",count,1,2,3,4,5);
   for(i =0; i< count; i++)
   {
        printf("%d,",buffer1[i]);
   }
   
}

void ataload()
{
//	int ix;
	
#ifndef  INCLUDE_ATA
    {    
    /* initialize hard disk driver */
    IMPORT ATA_RESOURCE ataResources[];
    ATA_RESOURCE *pAtaResource;
 //   printf("wangwenhua\n");
//	return;
    for (ix = 0; ix < 1/*ATA_MAX_CTRLS*/; ix++)
        {
        pAtaResource = &ataResources[ix];
        if (pAtaResource->ctrlType == IDE_LOCAL)
            if ((ataDrv_new (ix, pAtaResource->drives, pAtaResource->intVector,
		   pAtaResource->intLevel, pAtaResource->configType,
                   pAtaResource->semTimeout, pAtaResource->wdgTimeout))
		== ERROR)
		{
#ifdef INCLUDE_STDIO
		printf ("ataDrv returned ERROR from usrRoot.\n");
#endif /* INCLUDE_STDIO */
		}
        }
    }

//#ifdef  INCLUDE_SHOW_ROUTINES
    ataShowInit ();                     /* install ATA/IDE show routine */
//#endif  /* INCLUDE_SHOW_ROUTINES */
#endif  /* INCLUDE_ATA */
}
#define FTP_DEVICE_NAME		"/FTPSERVER/"
#define FTP_SERVER_PATH		   "" /* "download/"  */
LOCAL int  copy_file_to_ramdisk(const char *filename)
{
	  UINT32   RegVal =  *M85XX_GPOUTDR(CCSBAR);
	

    char buffer[100]; 
    int fd0, fd1;
    char filename0[200], filename1[200];
    struct stat statData;   /* used to get size */
    int state;
    int i;//,j;
    strcpy(filename0, FTP_DEVICE_NAME);
    strcat(filename0, FTP_SERVER_PATH); 
  


    strcat(filename0, filename);

    int openCount=0;
    do
    {
        fd0 = open(filename0, O_RDONLY, 0664);
        if ( fd0 == ERROR )
        {
       
          printf("Open file %s on boot device error! \n",(int)filename0);
        }
        else
        {
            break;
        }
        openCount ++;
    } while ( openCount < 3 );
    if (ERROR == fd0)
    {
        printf("Give up on opening file %s on boot device! BOOT UP FAILED \n",(int)filename);
        return ERROR;
    }

    /**DEBUG**/
    //printf("filename0 is :%s!!!\n",filename0);

    strcpy(filename1, "/RAMD:0/");
    strcat(filename1, RAMDISK_DOWNLOAD_PATH);
    strcat(filename1, filename);
    fd1 = open(filename1, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if ( fd1 == ERROR )
    {
       printf("Open file %s on RamDisk error! \n",(int)filename1);
        return ERROR;
    }
    /**DEBUG**/
    //printf("filename1 is :%s!!!\n",filename1);

    state = ioctl (fd0, FIOFSTATGET, (int)&statData);
    if ( state == ERROR )
    {
        printf( "L3bootTask ioctl error!!!!\n");
        close(fd0);
        close(fd1);
        return ERROR;
    }
    for ( i=0; i<statData.st_size;)
    {
        int count = read(fd0, buffer, 100);   
        write(fd1, buffer, count);  
        i = i + count;
        if(RegVal&0x00000080)////24为高，则为0
     	   	{
     	   	     *M85XX_GPOUTDR(CCSBAR) &=0xffffff7f; 
     	   	}
     	   else
     	   	{
     	   	   *M85XX_GPOUTDR(CCSBAR) |= 0x00000080;
     	   	}

	//if(count<100)
	/*{
	       for(j =0; j< count; j++)
            {
                printf("%02x ",buffer[j]);
             }
	}*/
    }

  printf(" Download file :%s finished",(int)filename);

    close(fd0);
    close(fd1);


    return 1;
}

  void getFileFromCF(const char *filename)
  {
	char buffer[100]; 
    int fd0, fd1;
    char filename0[100], filename1[100];
    struct stat statData;   /* used to get size */
    int state;
    int i;//,j;
     char path[40];
     strcpy(path, "/RAMD:0/"/*DEVICE_RAMDISK*/);
      strcat(path, RAMDISK_DOWNLOAD_PATH);
    mkdir(path);
    //strcpy(filename0, "/ata0a/);
     if ( bspGetBootPlane() != BOOT_PLANE_B )
     {
            strcpy(filename0, "/ata0a/BTSA/");  
     }
     else
     {
            strcpy(filename0, "/ata0a/BTSB/");
     }
    strcat(filename0, filename);

    int openCount=0;
    do
    {
        fd0 = open(filename0, O_RDONLY, 0664);
        if ( fd0 == ERROR )
        {
       
          printf("Open file %s on boot device error! \n",(int)filename0);
        }
        else
        {
            break;
        }
        openCount ++;
    } while ( openCount < 3 );
    if (ERROR == fd0)
    {
        printf("Give up on opening file %s on boot device! BOOT UP FAILED \n",(int)filename);
        return ERROR;
    }

    /**DEBUG**/
    //printf("filename0 is :%s!!!\n",filename0);

    strcpy(filename1, "/RAMD:0/");
    strcat(filename1, RAMDISK_DOWNLOAD_PATH);
    strcat(filename1, filename);
    fd1 = open(filename1, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if ( fd1 == ERROR )
    {
       printf("Open file %s on RamDisk error! \n",(int)filename1);
        return ERROR;
    }
    /**DEBUG**/
    //printf("filename1 is :%s!!!\n",filename1);

    state = ioctl (fd0, FIOFSTATGET, (int)&statData);
    if ( state == ERROR )
    {
        printf( "L3bootTask ioctl error!!!!\n");
        close(fd0);
        close(fd1);
        return ERROR;
    }
    for ( i=0; i<statData.st_size;)
    {
        int count = read(fd0, buffer, 100);   
        write(fd1, buffer, count);  
        i = i + count;
	//if(count<100)
	/*{
	       for(j =0; j< count; j++)
            {
                printf("%02x ",buffer[j]);
             }
	}*/
    }

  printf(" Download file :%s finished\n",(int)filename);

    close(fd0);
    close(fd1);



  }
void getFileFromftp(const char *filename)
{

     char path[40];
     BOOT_PARAMS       bootParams;
     (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
   strcpy(path, "/RAMD:0/"/*DEVICE_RAMDISK*/);
   strcat(path, RAMDISK_DOWNLOAD_PATH);
    mkdir(path);
#if 0
     if(netDevCreate(FTP_DEVICE_NAME, "172.16.24.73", 1)!=OK)  /**0 is rsh, 1 is ftp*********/
     {
         printf("netDevCreate error\n");
     }
      iam("vxworks", "vxworks");
#endif
      if(netDevCreate(FTP_DEVICE_NAME, bootParams.had, 1)!=OK)  /**0 is rsh, 1 is ftp*********/
      {
              printf("netDevCreate error\n");
        }
       iam(bootParams.usr, bootParams.passwd);


       printf("usr:%c%c%c%c%c%c%c\n",bootParams.usr[0],bootParams.usr[1],bootParams.usr[2],bootParams.usr[3],bootParams.usr[4],bootParams.usr[5],bootParams.usr[6]);
        printf("pwd:%c%c%c%c%c%c%c\n",bootParams.passwd[0],bootParams.passwd[1],bootParams.passwd[2],bootParams.passwd[3],bootParams.passwd[4],bootParams.passwd[5],bootParams.passwd[6]);
	 copy_file_to_ramdisk(filename);
}
void getFileFromftp1(const char *filename)
{

     char path[40];
   strcpy(path, "/RAMD:0/"/*DEVICE_RAMDISK*/);
   strcat(path, RAMDISK_DOWNLOAD_PATH);
    mkdir(path);
     if(netDevCreate(FTP_DEVICE_NAME, "172.16.24.73", 1)!=OK)  /**0 is rsh, 1 is ftp*********/
     {
         printf("netDevCreate error\n");
     }
      iam("vxworks", "vxworks");
	 copy_file_to_ramdisk(filename);
}
void fpga_reset(void)
 	{
 	#if 0
 	  	*M8260_IOP_PCPAR(vxImmrGet()) &= 0x7fffffff;
	*M8260_IOP_PCDIR(vxImmrGet()) |= 0x80000000;
	*M8260_IOP_PCDAT(vxImmrGet()) |= 0x80000000;
	taskDelay(1);
	*M8260_IOP_PCDAT(vxImmrGet()) &= 0x7fffffff;
	taskDelay(1);
	*M8260_IOP_PCDAT(vxImmrGet()) |= 0x80000000;
	#endif
 	}
/*********************************************************************************
CONFIG_DONE              GPIN14          PCI2-AD1    AC23   input
CONFIG_CLK                GPOUT11       PCI2-AD12   AB21   output
CONFIG_INIT_B            GPIN15       PCI2-AD0  AB19   input
CONFIG_PROGRAM_B    GPOUT12      PCI2-AD11    AA20   output
CONFIG_D_IN               GPOUT13      PCI2-AD10    AC20   output



*************************************************************************************/


void	init_FPGA_GPIO(void)
{
#if 0
    	*M8260_IOP_PDPAR(vxImmrGet()) &= 0xffff07ff;	/* PDPAR 16, 17, 18, 19, 20 = 0 (GPIO) */
	*M8260_IOP_PDDIR(vxImmrGet()) &= 0xffff5fff;	/* PDDIR 16, 18 = 0 (input) */
	*M8260_IOP_PDDIR(vxImmrGet()) |= 0x00005800;	/* PDDIR 17, 19, 20 = 1 (output) */
	#endif
}



/******************************************************************************
*
*	FUNCTION NAME: read_WRRU_Protect
*	___________________________________________________________________________
*
*	DESCRIPTION:	read_WRRU_Protect            GPIN10     
*

*
*	RETURNS:		N/A
*
*******************************************************************************/
 unsigned short	read_WRRU_Protect()
{
 	/* if ((*M85XX_GPINDR(CCSBAR)) & 0x00200000)
	{
		return (1);
	}
	else
	{
		return (0);
	}
	*/
	 unsigned int base_addr = 0xd100001e;
         return *(unsigned short*)base_addr;
}
/******************************************************************************
*
*	FUNCTION NAME: set_nCONFIG
*	___________________________________________________________________________
*
*	DESCRIPTION:	CONFIG_INIT_B            GPIN15       PCI2-AD0   AB19   input
*

*
*	RETURNS:		N/A
*
*******************************************************************************/
 UCHAR	read_nINT()
{
 	 if ((*M85XX_GPINDR(CCSBAR)) & 0x00010000)
	{
		return (1);
	}
	else
	{
		return (0);
	}
	
}

 /******************************************************************************
*
*	FUNCTION NAME: set_nCONFIG
*	___________________________________________________________________________
*
*	DESCRIPTION:	CONFIG_PROGRAM_B    GPOUT12      PCI2-AD11    AA20   output
*
*	INPUTS:			setting	-	0 : set nCONFIG low 
*								1 : set nCONFIG high
*
*	RETURNS:		N/A
*
*******************************************************************************/
void	set_nPROGRAM(UINT8 setting)
{
   	if (setting)
	{
		*M85XX_GPOUTDR(CCSBAR)  |= 0x00080000;	/*  GPOUT12      PCI2-AD11    AA20   */
	}
	else
	{
		*M85XX_GPOUTDR(CCSBAR) &= 0xfff7ffff;	/* clear  GPOUT12      PCI2-AD11    AA20   */
	}
	
}

 /******************************************************************************
*
*	FUNCTION NAME: set_DCLK
*	___________________________________________________________________________
*
*	DESCRIPTION:	CONFIG_CLK                GPOUT11       PCI2-AD12   AB21   output
*
*	INPUTS:			setting	-	0 : set DCLK low 
*								1 : set DCLK high
*
*	RETURNS:		N/A
*
*******************************************************************************/
void	set_DCLK(UINT8 setting)
{

   	if (setting)
	{
		*M85XX_GPOUTDR(CCSBAR) |= 0x00100000;	/* set PCI2-AD12 */
	}
	else
	{
		*M85XX_GPOUTDR(CCSBAR) &= 0xffefffff;	/* clear PCI2-AD12 */
	}
	
}

 /******************************************************************************
*
*	FUNCTION NAME: set_DATA0
*	___________________________________________________________________________
*
*	DESCRIPTION:	GPOUT13      PCI2-AD10    AC20   output
*
*	INPUTS:			setting	-	0 : set DATA0 low 
*								1 : set DATA0 high
*
*	RETURNS:		N/A
*
*******************************************************************************/
 void	set_DATA0(UINT8 setting)
 	{
 	
 	    	if (setting)
	{
		*M85XX_GPOUTDR(CCSBAR)|= 0x00040000;	/* GPOUT13      PCI2-AD10    AC20   output */
	}
	else
	{
		*M85XX_GPOUTDR(CCSBAR) &= 0xfffbffff;	/* clear GPOUT13      PCI2-AD10    AC20   output */
	}
	
 	}
 /******************************************************************************
*
*	FUNCTION NAME: read_nSTATUS
*	___________________________________________________________________________
*
*	DESCRIPTION:	read nSTATUS pin (PD16)
*
*	INPUTS:			N/A
*
*	RETURNS:		status of nSTATUS pin
*
*******************************************************************************/
UCHAR    read_nSTATUS(void)
{
#if 0
   	if (*M8260_IOP_PDDAT(vxImmrGet()) & 0x00008000)
	{
		return (1);
	}
	else
	{
		return (0);
	}
	#endif
	return 0;
}

 /******************************************************************************
*
*	FUNCTION NAME: read_CONF_DONE
*	___________________________________________________________________________
*
*	DESCRIPTION:	read CONFIG_DONE              GPIN14          PCI2-AD1    AC23   input
*
*	INPUTS:			N/A
*
*	RETURNS:		status of CONF_DONE pin
*
*******************************************************************************/

 UCHAR	 read_CONF_DONE(void)
 {

 	 if ((*M85XX_GPINDR(CCSBAR) )& 0x00020000)
	{
		return (1);
	}
	else
	{
		return (0);
	}
	
	
 }

 void testfpgafile()
{
        char  filename1[200];
       FILE  *fd1;

	 char buffer1[100];
	 int i;
	 int count;

      strcpy(filename1, "/RAMD:0/load/dsp_image");

    fd1 = fopen(filename1, "rb"/*O_RDONLY, 0664*/);
    if ( fd1 == ERROR )
    {
        printf( "Open file1 %s on RamDisk error! \n",(int)filename1);
        return ERROR;
    }

   do{
             // (&data_L2, 1/*size*/, 1, file_L2)
	      count = fread(buffer1,1,100, fd1);   
	      for(i =0; i< count; i++)
            {
                printf("%02x ",buffer1[i]);
             }	 
   	}while(!feof(fd1));
     fclose(fd1);

   
}

 void readaddr(unsigned int addr,unsigned int len)
 {
     unsigned short value;
     int i;
     unsigned short *p;
     p = (unsigned short *)addr;
    for(i =0; i < len; i++)
    {
        value =*p;//*(unsigned short*)addr;
	  printf("%x,",value);
	 p++;
	 // if((i%10)==0)
	 // printf("\n");
    }
      
 }

 void reset_dsp(unsigned short index)    /*  index = 0 ~ 4 */
 {
   int i;
  *(unsigned short*)0xd1000012 = 0x0000;
	for(i=0;i<5000;i++)
		;
	*(unsigned short*)0xd1000012 = (0x1<<(index+8)) | (0x1<<index);

	printf("reset_dsp:%x\n",index);

 }


 void resetDsp(unsigned char index)
 {
           int i;
     unsigned short p= *(unsigned short*)0xd1000012 ;
	//*(unsigned short*)0xd1000012 = 0x0000;
	for(i=0;i<5000;i++)
		;
	*(unsigned short*)0xd1000012 = p|((0x1<<(index+8)) | (0x1<<index));
 }
 void test_fpga_wrru_code()
 {
       int i,j, k,l;
      unsigned char  data[4][112];
	unsigned short tt;
	unsigned int base_addr = 0xd100001c;
      for(i = 0; i< 4;i++)
      	{
      	    for(j = 0; j< 112;j++)
      	    	{
      	    	     data[i][j] = i +j;
      	    	}
      	}
	  k = 0;

	for(;;)
	{
	     for(i = 0; i< 4; i++)
	     {
	    	      while(read_WRRU_Protect()==1)
	    	      	{
	    	      	}
	    	 
	            for(j =0; j <56; j++)
	            	{
	            		tt= data[i][2*j]*0x100+data[i][2*j+1];
	            		*(unsigned short*)(base_addr) =tt;
	            	}
	            
			
	     }
	     for(l = 0; l < 10000; l++)
	     {
	     }
		
	}
	
	  
}
/*****************************
read fpga hardware version


*************************************/
unsigned short  Read_Fpga_Version_Hard()
{
   unsigned int base_addr = 0xd1000000;
   return *(unsigned short*)base_addr;
}

/**********************

read fpga software version


***********************/
unsigned int Read_Fpga_Version_Soft()
{
      unsigned int    version;
      unsigned int base_addr_H = 0xd1000002;
      unsigned int base_addr_L  = 0xd1000004;
      version =  (*(unsigned short*)base_addr_H)*0x10000;
	version+= (*(unsigned short*)base_addr_L);
	return version;
	  
}

/**********************
read 光口状态

**************************/
unsigned short Read_Fpga_Light_Status()
{
       unsigned int base_addr_L  = 0xd1000006;
       return (*(unsigned short*)base_addr_L);
}
/*************************
读3路信息

******************************/

unsigned short  Read_Fpga_Lindex_Status(unsigned char index)
{
     unsigned int base_addr_L  = 0xd1000008;
	base_addr_L+=index*2; 
	 
      return (*(unsigned short*)base_addr_L);
}
/**********************************jiajunming 2012-7-28***********************

如果做主从同步，需要对主bbu和从bbu都做操作，具体如下：

主bbu操作：
	m 0xd100000e 5555   -- 本板输入为GPS
	m 0xd1000020 5555    -- 本板输出为GPS

从bbu操作：
	m 0xd100000e cccc	-- 本板输入为从同步；
	m 0xd1000020 cccc	-- 本板输出为从同步；

*********************************************************************************/
void Set_Fpga_Clk(unsigned char type)
{
       unsigned int base_addr = 0xd100000e;
	unsigned int base_addr_Sec = 0xd1000020;
	if(type == 0) /*local 10ms**/
	{
	     *(unsigned short*)base_addr = 0xaaaa;
		 printf("Set_Fpga_Clk:local 10ms\n");
	}
	else if(type ==1)/***1588**/
	{
	    *(unsigned short*)base_addr = 0x9999;
		 printf("Set_Fpga_Clk:1588\n");
	}
	else if(type==2)/***gps**/
	{
	     *(unsigned short*)base_addr = 0x5555;
	      *(unsigned short*)base_addr_Sec = 0x5555;
		  printf("Set_Fpga_Clk:gps\n");
	}
	else if(type ==3)
	{
	     	     *(unsigned short*)base_addr = 0xcccc;
		      *(unsigned short*)base_addr_Sec = 0xcccc;
		  printf("Set_Fpga_Clk:slave\n");
	}
	else
	{
	       *(unsigned short*)base_addr = 0x5555;
		  printf("Set_Fpga_Clk:gps\n");
	}
}

unsigned int Read_Aif_Status(unsigned char index)/***0 -aif0 , 1-aif1***/
{
    unsigned int status;
  unsigned int base_addr = 0xd1000014;
  unsigned int base_addr_H = base_addr + index*2;
  unsigned int base_addr_L = base_addr_H + 2;
   status =  (*(unsigned short*)base_addr_H)*0x10000;
   status+= (*(unsigned short*)base_addr_L);
   return status;
}
/**************************************/
unsigned short Read_Fiber_Delay(unsigned char index)
{
      unsigned int status;
        unsigned int base_addr = 0xd1000008;
     unsigned int base_addr_H = base_addr + index*2;
     status =  (*(unsigned short*)base_addr_H);
     return status;
}
/******************************************
用于rru代码加载时使用，1-表示扇区0，2-扇区1，3-扇区2
fpga根据此寄存器的值决定将代码发送到哪个光口。



********************************************/
void Set_RRU_Code(unsigned char rru_no)
{
       unsigned int base_addr_L1  = 0xd100003c;
       *(unsigned short*)base_addr_L1 = (rru_no +1);
}
/********************************************

设置rru的扇区信息给FPGA




*************************************************/
void Set_RRU_Info(unsigned char rru0,unsigned char rru1,unsigned char rru2)
{
        unsigned int base_addr_L1  = 0xd1000046;
         unsigned int base_addr_L2  = 0xd1000048;
          unsigned int base_addr_L3  = 0xd100004a;
      if((rru0==4)&&(rru1==4)&&(rru2==4))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0ACE;
      	     	*(unsigned short*)base_addr_L2 = 0x0123;
      	     	*(unsigned short*)base_addr_L3= 0x0321;
      	}
        if((rru0==4)&&(rru1==8)&&(rru2==0))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0ACD;
      	     	*(unsigned short*)base_addr_L2 = 0x0122;
      	     	*(unsigned short*)base_addr_L3= 0x00E1;
      	}
          if((rru0==4)&&(rru1==0)&&(rru2==8))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0AEF;
      	     	*(unsigned short*)base_addr_L2 = 0x0133;
      	     	*(unsigned short*)base_addr_L3= 0x0E01;
      	}
       if((rru0==0)&&(rru1==4)&&(rru2==8))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0CEF;
      	     	*(unsigned short*)base_addr_L2 = 0x0233;
      	     	*(unsigned short*)base_addr_L3= 0x0E10;
      	}
            if((rru0==0)&&(rru1==8)&&(rru2==4))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0ECD;
      	     	*(unsigned short*)base_addr_L2 = 0x0322;
      	     	*(unsigned short*)base_addr_L3= 0x01E0;
      	}
              if((rru0==8)&&(rru1==0)&&(rru2==4))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0EAB;
      	     	*(unsigned short*)base_addr_L2 = 0x0311;
      	     	*(unsigned short*)base_addr_L3= 0x010E;
      	}
                if((rru0==8)&&(rru1==4)&&(rru2==0))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0CAB;
      	     	*(unsigned short*)base_addr_L2 = 0x0211;
      	     	*(unsigned short*)base_addr_L3= 0x001E;
      	}
          if((rru0==4)&&(rru1==0)&&(rru2==0))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0ACE;
      	     	*(unsigned short*)base_addr_L2 = 0x0123;
      	     	*(unsigned short*)base_addr_L3= 0x0321;
      	}
          if((rru0==0)&&(rru1==4)&&(rru2==0))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0ACE;
      	     	*(unsigned short*)base_addr_L2 = 0x0123;
      	     	*(unsigned short*)base_addr_L3= 0x0321;
      	}
          if((rru0==0)&&(rru1==0)&&(rru2==4))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x0ACE;
      	     	*(unsigned short*)base_addr_L2 = 0x0123;
      	     	*(unsigned short*)base_addr_L3= 0x0321;
      	}
          if((rru0==8)&&(rru1==0)&&(rru2==0))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x00AB;
      	     	*(unsigned short*)base_addr_L2 = 0x0011;
      	     	*(unsigned short*)base_addr_L3= 0x000E;
      	}

          if((rru0==0)&&(rru1==8)&&(rru2==0))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x00CD;
      	     	*(unsigned short*)base_addr_L2 = 0x0022;
      	     	*(unsigned short*)base_addr_L3= 0x00E0;
      	}

          if((rru0==0)&&(rru1==0)&&(rru2==8))
      	{
      	     *(unsigned short*)base_addr_L1 = 0x00EF;
      	     	*(unsigned short*)base_addr_L2 = 0x0033;
      	     	*(unsigned short*)base_addr_L3= 0x0E00;
      	}
 
      	
}
 void test222()
 {
      char temp =1;;
      int i = 0;
       for(;;)
       {
         temp = read_WRRU_Protect();
	  if(temp==1)
	  	{
	  	    i++;
		   if(i%10==0)
		   	{
	  	           printf("read_WRRU_Protect ==1\n");
		   	}
		   
	  	}
       }
 }
 /****************************

假设pcode 的长度为112*4

 ********************************/
unsigned char wrru_flag = 0;

 void setPrintWrru(unsigned char flag)
 {
     wrru_flag = flag;
 }
 void fpga_wrru_code(unsigned char *pcode)
 {
       int i,j, k,l;
      unsigned char  data[4][112];
	unsigned short tt;
	unsigned int base_addr = 0xd100001c;
      for(i = 0; i< 4;i++)
      	{
      	  if(wrru_flag==1)
      	  {
	      	   printf("local bus data:%\n");
	          for(j = 0; j < 112; j++)
	          	{
	          	    printf("%02x,",pcode[i*112+j]);
	          	}
      	  }
	   memcpy(data[i],pcode+(i*112),112);
      	   // 
	   // pcode+=112;
      	}
	  k = 0;

	//for(;;)
	//{
	     for(i = 0; i< 4; i++)
	     {
	    	      while(read_WRRU_Protect()==1)
	    	      	{
	    	      	}
	    	 
	            for(j =0; j <56; j++)
	            	{
	            		tt= data[i][2*j]*0x100+data[i][2*j+1];
	            		*(unsigned short*)(base_addr) =tt;
	            	}
	            
	            	     for(l = 0; l < 20000; l++)
				{
				}
	           
			
	     }
		
	//}
	
	  
}



 void ddrtest1(unsigned int base_addr,unsigned int len)
{
	unsigned int i;
	
	for(i = 0;i<len;i++)
	{
		*(unsigned int*)(base_addr) = 0x123456;
		  base_addr+=4;
	}
	/*for(i = 0x2000000;i<0x10000000;i+=0x100000)
	{
		if(*(int*)i != 0x5a5a0f0f)
			printf("\nddr test error at 0x%x",i);
	}*/
	printf("\nddr test finished...base_addr:%08x\n",base_addr);
}
  void readddr(unsigned int base_addr,unsigned int len)
{
	unsigned int i;
	
	for(i = 0;i<len;i++)
	{
		printf("%04x ",*(unsigned short*)(base_addr) );
		  base_addr+=2;
	}
	/*for(i = 0x2000000;i<0x10000000;i+=0x100000)
	{
		if(*(int*)i != 0x5a5a0f0f)
			printf("\nddr test error at 0x%x",i);
	}*/
	printf("\nddr test finished...base_addr:%08x\n",base_addr);
}

 void malloctest()
 	{
 	  char * p;
	  int count = 0;
 	//    for(;;)
 	    	{
 	    	       p = (char *)malloc(1024*1024*32);
			if(p!=NULL)
			{
			   printf("malloc addr %08x\n",p);
			   count++;
			   memset(p,0x55,32*1024*1024);
			}
			else
				{
				 printf("no addr to alloc:%d\n",count);
				 //break;
				}
 	    	}
 	}
 /***bootFlag*********/
#define REBOOT_DISABLE		0x55
#define REBOOT_ENABLE		0x88
int bootFlag = REBOOT_ENABLE;    /***0x55 is disable, other enable************/

FUNCPTR RebootCallbackFunc = NULL;
//int bootFlag = REBOOT_ENABLE;    /***0x55 is disable, other enable************/

 UINT32 GetBtsIpAddr()
{
    BOOT_PARAMS              params;
    char*                    addrStr;

    static UINT32 BtsIpAddress = 0;

    if ( BtsIpAddress != 0 )
    {
        return BtsIpAddress;
    }

    if ( OK == usrBootLineCrack(BOOT_LINE_ADRS, &params) )
    {
        addrStr = params.ead;

        (void)strtok(addrStr, ":");

        BtsIpAddress = inet_addr(addrStr);
    }

    return BtsIpAddress;
}
 int getMacFromSTD1
    (
    char *pMac
    )
    {
    char buf [BOOT_FIELD_LEN];
    char *pString;
    char *pStr;
    int i;
    char ch,n;
/*
    if ((i = promptRead (buf, sizeof (buf))) != 0)
       return (i);
*/    
	if(1!=scanf("%s",buf))
        return (-1);

    /* scan for number */

    pString = buf;

    while (isspace(*pString))
        ++pString;
    /* pick base */

    if (*pString == '$')
    {
    ++pString;
    }
    else if (strncmp (pString, "0x", 2) == 0)
    {
    pString += 2;
    }


    /* scan string */

    pStr = pString;

    i = 0;

    FOREVER
    {
    ch = *pStr;

    if ( (':' == ch)
		||('-' == ch) )
        {
        ++pStr;
        continue;
        }

    if ('\0' == ch)
        break;

    if (!isascii (ch))
        break;

    if (isdigit (ch))
        n = ch - '0';
    else
        {
        if (isupper (ch))
            ch = tolower (ch);

        if ((ch < 'a') || (ch > 'f'))
            break;

        n = ch - 'a' + 10;
        }

/*    value = (value * (hex ? 16 : 10)) + n;*/
    if (0 != i % 2)
        {
        *(pMac + (i/2)) |= n;
        }
    else
        {
        *(pMac + (i/2)) |= (n<<4);
        }

    ++i;
    ++pStr;
    }

    return (1);
    }



void ResetDspNew(unsigned char index)
{
     int i;
     unsigned short  p = *(unsigned short*)0xd1000012 ;
     	for(i=0;i<5000;i++)
		;
	//*(unsigned short*)0xd1000012 = p|((0x1<<(index+8)) | (0x1<<index));

	if(index==4)
	{
	     *(unsigned short*)0xd1000012 = p&0x000f;
	      p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0010;
	     return;
	}
     if(index==0)
     	{
     	     *(unsigned short*)0xd1000012 = p&0x001e;
	      p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0001;
	     return;
     	}
     
     	if(index ==2)
     	{
     	     	*(unsigned short*)0xd1000012 = p&0x001b;
	      p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0004;
	     return;
     	}
        if(index == 1) 
     	{
               *(unsigned short*)0xd1000012 = p&0x001e;
	      p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p&0x001d;
	       p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0002;
	     	/*       p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x1;*/

        return;
     	
     	}
       if(index==3)
     	{
     	        *(unsigned short*)0xd1000012 = p&0x001e;
	      p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p&0x0017;
	       p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0008;
	   /*  	       p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x1;*/
	     return;
     	}
       else
       {
       	   printf("dsp index error :%d\n",index);
       }
     	
}



void ResetDspNew1(unsigned char index)
{
     int i;
     unsigned short  p = *(unsigned short*)0xd1000012 ;
     	for(i=0;i<5000;i++)
		;
	//*(unsigned short*)0xd1000012 = p|((0x1<<(index+8)) | (0x1<<index));

	if(index==4)
	{
	     *(unsigned short*)0xd1000012 = p&0x000f;
	      p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0010;
	     return;
	}
     if(index==0)
     	{
     	     *(unsigned short*)0xd1000012 = p&0x001e;
	      p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0001;
	     return;
     	}
     
     	if(index ==2)
     	{
     	     	*(unsigned short*)0xd1000012 = p&0x001b;
	      p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0004;
	     return;
     	}
        if(index == 1) 
     	{
#if 0
        	*(unsigned short*)0xd1000012 = p&0x001e;
	      p = *(unsigned short*)0xd1000012 ;
#endif
	     *(unsigned short*)0xd1000012 = p&0x001d;
	       p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0002;
	     	/*       p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x1;*/

        return;
     	
     	}
       if(index==3)
     	{
#if 0
    	   *(unsigned short*)0xd1000012 = p&0x001e;
	      p = *(unsigned short*)0xd1000012 ;
#endif
	     *(unsigned short*)0xd1000012 = p&0x0017;
	       p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x0008;
	   /*  	       p = *(unsigned short*)0xd1000012 ;
	     *(unsigned short*)0xd1000012 = p|0x1;*/
	     return;
     	}
       else
       {
       	   printf("dsp index error :%d\n",index);
       }
     	
}


#if 0
stSagAddr
PetHwWatchdog
RegisterRebootCallbackFunc
execute
startcsitest()
exitTelnetSession
dwSagIp
ToggleRunningLed
bspGetDeviceID
sysWdtCallbackInstall

#endif
#define CPU_WATCH_DOG
#ifdef CPU_WATCH_DOG

FUNCPTR WdtCallbackRoutine = NULL;
UINT32  WdtCallbackRoutineArg = 0;

UINT32  WdtCallbackRoutinePeriodInTick = 10;
#define M_TP_CSI_WDT               0
#define M_TP_CSI_WDT_INIT 50
UINT32  SysWdtTaskId = 0;

void sysWdtCallbackInstall(FUNCPTR func, UINT32 arg,UINT32 period)
{
    if ( NULL == WdtCallbackRoutine )
    {
        WdtCallbackRoutine = func;
        WdtCallbackRoutineArg = arg;
        WdtCallbackRoutinePeriodInTick = period;
        if (SysWdtTaskId)
        {
           taskPrioritySet(SysWdtTaskId,/* M_TP_CSI_WDT_INIT*/M_TP_CSI_WDT);
        }
    }
}
/*******************************
24喂狗，让这个管角高低变化
25是选择够，如果为0，则软件喂狗，否则CPLD自己模拟喂狗

**********************************/
void PetHwWatchdog()
{
   // sysOutByte(NVRAM_BASE_ADDR + RTC_WATCHDOG, 0x8E /*0xff*/);  /***0x86 1s, 0x8E 3s*******/
   UINT32   RegVal =  *M85XX_GPOUTDR(CCSBAR);
   if(RegVal&0x00000080)////24为高，则为0
   	{
   	     *M85XX_GPOUTDR(CCSBAR) &=0xffffff7f; 
   	}
   else
   	{
   	   *M85XX_GPOUTDR(CCSBAR) |= 0x00000080;
   	}
}


void L3_APP_Run()
{
   // sysOutByte(NVRAM_BASE_ADDR + RTC_WATCHDOG, 0x8E /*0xff*/);  /***0x86 1s, 0x8E 3s*******/
   UINT32   RegVal =  *M85XX_GPOUTDR(CCSBAR);
   if(RegVal&0x00000001)////31为高，则为0
   	{
   	     *M85XX_GPOUTDR(CCSBAR) &=0xfffffffe; 
   	}
   else
   	{
   	   *M85XX_GPOUTDR(CCSBAR) |= 0x00000001;
   	}
}
unsigned char stop_watchdog= 0;
void WatchdogTask()
{
	while(1)
    {
       if(stop_watchdog==0)
       	{
        		PetHwWatchdog();
       	}
#if 1
        if (WdtCallbackRoutine)
        {
       //     printf("WatchdogTask\n");
            WdtCallbackRoutine(WdtCallbackRoutineArg);
        }
#endif
	    taskDelay(WdtCallbackRoutinePeriodInTick);   /* delay for 100 ms */
	}
}

void StartWdtTask()
{
  //  UINT32 gppRegVal;
    *M85XX_GPOUTDR(CCSBAR) &= 0xffffffbf ;/**25low **/
  
    SysWdtTaskId = taskSpawn ("tWatchDog", M_TP_CSI_WDT/*M_TP_CSI_WDT_INIT*/, 0, 10240, (FUNCPTR)WatchdogTask, 0,0,0,0,0,0,0,0,0,0);
    printf("Watchdog Task started\n");		

    //MV64360_REG_RD(MV64360_GPP_IO_CTRL, &gppRegVal);   
   // gppRegVal |= (1 << 3);
  //  MV64360_REG_WR(MV64360_GPP_IO_CTRL, gppRegVal);    

}


void ToggleRunningLed()
{
   // UINT32 gppRegVal;


  //  taskLock();
  //  MV64360_REG_RD(MV64360_GPP_VAL, &gppRegVal);   
  //  gppRegVal ^= (1 << 3);    /* PIN 2 is SW running indicator */               
   // MV64360_REG_WR(MV64360_GPP_VAL, gppRegVal);    
    //taskUnlock();
}
#endif

#ifdef  INT_OPEN
void startFPGA_Int()
{
       intConnect(INUM_TO_IVEC(INUM_IRQ1), (VOIDFUNCPTR)Fpga_IntPrc, 0);	
}
#endif
#define FPGA_ALARM
#ifdef FPGA_ALARM
#if 0
Warning_Type(0)gps_warning；
Warning_Type(1)    dd_fep_rx_warning；
Warning_Type(2)：frame_sync_warning；
Warning_Type(3)：sfp_los_warning；
各比特独立（可能同时发生），为'1'时正常，为'0'时告警；

#endif
unsigned short  Read_Fpga_Alarm()
{
     unsigned short  p = *(unsigned short*)0xd100002a ;
     return p;
     
}

unsigned int Judge_Aif_Status(unsigned char index)/***0 -aif0 , 1-aif1***/
{
	  unsigned short  status1 = 0,status2 = 0;
	 // unsigned int base_addr;// = 0xd1000014;
	  unsigned int base_addr_H;// = base_addr + index*4;
	  unsigned int base_addr_L ;
	  int result = 0;
	  if(index==0)
	  {
		  base_addr_H = 0xd1000014;
	  }
	  else
	  {
		  base_addr_H = 0xd1000018;
	  }
	  base_addr_L = base_addr_H + 2;
  
   status1 =  (*(unsigned short*)base_addr_H);
   status2= (*(unsigned short*)base_addr_L);
   if(status1!=0x0022)
   	{
   	  result =1;
   	}
     else
     	{
     	     if((status2< 0x8200)||(status2>0x8203))
     	     	{
     	     	    result = 1;
     	     	}
     	}
	 if(result==1)
	 	{
	 	    logMsg("Aif_Status:%x,%x\n",status1,status2,1,2,3,4);
			
	 	}
	return result;
   
}



#endif
//#define REBOOT_DISABLE		0x55
//#define REBOOT_ENABLE		0x88
//int bootFlag = REBOOT_ENABLE;    /***0x55 is disable, other enable************/

//FUNCPTR RebootCallbackFunc = NULL;

void rebootDisable()
{
	bootFlag = REBOOT_DISABLE;
}

void rebootEnable()
{
	bootFlag = REBOOT_ENABLE;
}

void RegisterRebootCallbackFunc( FUNCPTR func)
{
    if ( NULL == RebootCallbackFunc )
    {
        RebootCallbackFunc = func;
    }
}

 int reset_BTS(int type)
{
   #if 1
	if(bootFlag == REBOOT_DISABLE)
    {
        logMsg("reset disable, please execute rebootEnable() !!!!\n",0,0,0,0,0,0);		
	}
    else
    {
        bspSetResetFlag(RESET_SYSTEM_SW_RESET);
        if (RebootCallbackFunc )
        {
            RebootCallbackFunc();   /* should allow CSI to take over and reset the BTS */
        }  /* call back fund should never return, if returned, something weird happened, should
		      go ahead reset using CPLD */
		if(SysWdtTaskId)
              taskDelete(SysWdtTaskId);
		ResetBTSViaCPLD();  
	}
	#endif
	return 0;
	
}

 int rebootBTS(int type)
{
	return reset_BTS(type);
}
//设置不同ts模式下的值
#if 0
REG_SLOT_MODE	0x002c	读/写	时隙比模式：
REG_SLOT_MODE(15..8):0x2A=2:6;
0x2B=3:5; 0x2C=4:4; 0x2D=5:3;0x2E=6:2;

RRU编号：
REG_SLOT_MODE(7..0):0x01=rru_1; 0x02=rru_2; 0x03=rru_3; 
REG_TDD_OFFSET(15..0)	0x002e	读/写	时隙比模式不同时设置不同的值：
2:6 = 0x 0002 17AE
3:5 = 0x 0001 0BD7
4:4 = 0x 0000 0000(默认)
5:3 = 0x 0008 5428
6:2 = 0x 0007 4851

offset = EMS config*4
#endif
 void SetTsMode(unsigned char flag,unsigned char rru_no,unsigned short  offset)
 {
     	  unsigned int base_addr_ts;// = 0xd1000014;
	  unsigned int base_addr_offset_H,base_addr_offset_L;// = base_addr + index*4;
	//  unsigned int base_addr_L ;
	unsigned short  fep_rx_int_mode = 0;
	unsigned short   ts_mode =0;
	 ts_mode =  (*(unsigned short*)0xd100002c);//先将以前的值保留


        if((ts_mode == 0x2a01)&&(flag==2))
        {
                   printf("ts flag :%d\n",flag);
        	      return;
        }
          if((ts_mode == 0x2b01)&&(flag==3))
         {
                   printf("ts flag :%d\n",flag);
          	     return;
          }
           if((ts_mode == 0x2c01)&&(flag==4))
          {
                 printf("ts flag :%d\n",flag);
          	    return;
          }

            if((ts_mode == 0x2d01)&&(flag==5))
          {
                printf("ts flag :%d\n",flag);
          	   return;
          }

           if((ts_mode == 0x2e01)&&(flag==6))
          {
                printf("ts flag :%d\n",flag);
          	   return;
          }
	  fep_rx_int_mode =  (*(unsigned short*)0xd1000010);//先将以前的值保留
	base_addr_ts = 0xd100002c;
	base_addr_offset_H = 0xd100002e;
	base_addr_offset_L = 0xd1000030;
	 *(unsigned short*)0xd1000010 = 0x5555;
      if(flag==2)
      	{
      	     *(unsigned short*)0xd100002c = 0x2a00+rru_no;
      	     *(unsigned short*)0xd1000030 = 0x0002;
      	     *(unsigned short*)0xd100002e = 0x17ae;
      	     *(unsigned short*)0xd1000034 = 0x0002;
      	     	*(unsigned short*)0xd1000032 = 0x9851;
      	}
      else if(flag==3)
      	{
      	     *(unsigned short*)0xd100002c = 0x2b00+rru_no;
      	     *(unsigned short*)0xd1000030 = 0x0001;
      	     *(unsigned short*)0xd100002e = 0x0bd7;
      	           	     *(unsigned short*)0xd1000034 = 0x0003;
      	     	*(unsigned short*)0xd1000032 = 0xa428;
      	}
      else if(flag==4)
      	{
      	 *(unsigned short*)0xd100002c = 0x2c00+rru_no;
      	 *(unsigned short*)0xd1000030 = 0x0000;
      	 *(unsigned short*)0xd100002e = 0x0000;
      	       	     *(unsigned short*)0xd1000034 = 0x0004;
      	     	*(unsigned short*)0xd1000032 = 0xb000;
      	}
      else if(flag==5)
      	{
      	    *(unsigned short*)0xd100002c = 0x2d00+rru_no;
      	    *(unsigned short*)0xd1000030 = 0x0008;
      	    *(unsigned short*)0xd100002e = 0x5428;
      	          	     *(unsigned short*)0xd1000034 = 0x0005;
      	     	*(unsigned short*)0xd1000032 = 0xbbd7;
      	}
      else if(flag==6)
      	{
      	 	*(unsigned short*)0xd100002c = 0x2e00+rru_no;
      	      *(unsigned short*)0xd1000030 = 0x0007;
      	     	*(unsigned short*)0xd100002e = 0x4851;
      	      *(unsigned short*)0xd1000034 = 0x0006;
      	     	*(unsigned short*)0xd1000032 = 0xc7ae;
      	}
      else
      	{
      	      printf("ts flag err:%d\n",flag);
      	}
      sysMsDelay(3000);
       *(unsigned short*)0xd1000010 = fep_rx_int_mode;//再将以前的值进行恢复20111026 modify
 }


void SetOffset(unsigned short offset)
{
  unsigned int temp = offset*6144;
  unsigned int temp1 = temp/100;
  	unsigned short  fep_rx_int_mode = 0;

  	unsigned short temp2 =0;
  	unsigned short temp3 =0;
  	unsigned int temp4 =0;
 temp2 =  (*(unsigned short*)0xd1000036);//先将以前的值保留
 temp3 =  (*(unsigned short*)0xd1000038);//先将以前的值保留 	
temp4 = temp2+temp3*0x100;
if(temp1== temp4)
{
       printf("SetOffset:%d\n",offset);
      return;
}
	  fep_rx_int_mode =  (*(unsigned short*)0xd1000010);//先将以前的值保留
 *(unsigned short*)0xd1000010 = 0x5555;

 
    *(unsigned short*)0xd1000036 = (unsigned short)(temp1);//low
    *(unsigned short*)0xd1000038 = (unsigned short )(temp1>>16);//high

          sysMsDelay(3000);
       *(unsigned short*)0xd1000010 = fep_rx_int_mode;
}
 short  ReadTemp(unsigned char port,unsigned char offset)
{
    short Temp = 0;
   // unsigned short  a,b;
  //  char array[2];
    int deviceAddress;
    if(port==0)
    	{
    	   deviceAddress = 0x48;
    	}
    else if(port ==1)
    	{
    	 deviceAddress = 0x49;
    	}
    else if(port ==2)
    	{
    	    deviceAddress = 0x4a;
    	}
    
     i2cTempRegRead (0,deviceAddress,offset,2,(unsigned short *)&Temp);
  //   offset=1;
  //   i2cTempRegRead (0,deviceAddress,offset,1,&b);
   // printf("a:%x,b:%x\n",array[0],array[1]);
     if(offset==0)
     	{
     //printf("Temp:%x\n",Temp);
     return Temp/8;
     	}
     else
     	return Temp;
}
unsigned short  WriteTemp(unsigned char port,unsigned offset,unsigned short value)
{
   unsigned short Temp = 0;
   Temp = value;
    int deviceAddress;
    if(port==0)
    	{
    	   deviceAddress = 0x48;
    	}
    else if(port ==1)
    	{
    	 deviceAddress = 0x49;
    	}
    else if(port ==2)
    	{
    	    deviceAddress = 0x4a;
    	}
     i2cTempRegWrite (0,deviceAddress,offset,2,&Temp);
    // if(offset==0)
     //	{
     //printf("Temp:%x\n",Temp);
   //  return Temp>>3;
     	//}
     //else
     	return Temp;
}

  int myAuthenticateCallback (/*Ipftps_session * session,
                                   char * password*/)
  	{
  		  return 0;
  	}



/************************************

检测是否有CF卡，0为没有，1为有


************************************/


unsigned char CF_Detect(void)
 {

 	 if ((*M85XX_GPINDR(CCSBAR) )& 0x00040000)/*****13 为低有CF卡，高为没有*********/
	{
		return (0);
	}
	else
	{
		return (1);
	}
	
	
 }


void Change_Boot_Plane()
{

      BOOT_PLANE plane = bspGetBootPlane();
  //  OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "l3oam file task bts current master boot plane[%d]!", plane);
 //   int Mplane; 
    if(BOOT_PLANE_B == plane)
    {    
        bspSetBootPlane(BOOT_PLANE_A);   
     //   Mplane = BOOT_PLANE_A;
     //   bspSetSMFlag(Mplane);
     printf("change to A Plane\n");
    }
    else
    {
        bspSetBootPlane(BOOT_PLANE_B);   
     //   Mplane = BOOT_PLANE_B;
      //  bspSetSMFlag(Mplane);
      printf("change to B Plane\n");
    }
}

void bspGetTime( char *pbuf)
{
         int i;
         char a,b;
 	  i2crtcDateTimeGet(  0, 0x51, 2,7,pbuf);
 	  pbuf[0]= pbuf[0]&0x7f;
 	    pbuf[1]=  pbuf[1]&0x7f;
 	    pbuf[2]=  pbuf[2]&0x3f;
 	     pbuf[3]= pbuf[3]&0x3f;
 	     pbuf[4]= pbuf[4]&0x7;
 	     pbuf[5]= pbuf[5]&0x1f;
 	    pbuf[6]=  pbuf[6]&0xff;
 	//   printf("time1:%2x,%2x,%2x,%2x,%2x,%2x\n",pbuf[0]&0x7f,pbuf[1]&0x7f,pbuf[2]&0x3f,pbuf[3]&0x3f,pbuf[5]&0x1f,pbuf[6]&0xff);
 	  for(i= 0 ; i< 7; i++)
 	  	{
 	  	     a = pbuf[i]>>4;
 	  	     b = pbuf[i]&0xf;
 	  	     pbuf[i] = a*10+b;
 	  	}
 	    
 	  
   //  printf("time:%2d,%2d,%2d,%2d,%2d,%2d\n",pbuf[0],pbuf[1],pbuf[2],pbuf[3],pbuf[5],pbuf[6]);
 	 
}
void bspSetTime(char*pbuf)
{
          int i;
         char a,b;
        #if 1
         	  for(i= 0 ; i< 7; i++)
 	  	{
 	  	     a = pbuf[i]/10;
 	  	     b = pbuf[i]%10;
 	  	     pbuf[i] = a*0x10+b;
 	  	}
           pbuf[0]= pbuf[0]&0x7f;/*****s*****/
 	    pbuf[1]=  pbuf[1]&0x7f;/*****min*****/
 	    pbuf[2]=  pbuf[2]&0x3f;/*****hour*****/
 	     pbuf[3]= pbuf[3]&0x3f;/*****day*****/
 	     pbuf[4]= pbuf[4]&0x7;/*****week*****/
 	     pbuf[5]= pbuf[5]&0x1f;/*****month*****/
 	    pbuf[6]=  pbuf[6]&0xff;/*****year*****/
         #endif

        // printf("timeSet:%2x,%2x,%2x,%2x,%2x,%2x\n",pbuf[0],pbuf[1],pbuf[2],pbuf[3],pbuf[5],pbuf[6]);
 	 i2crtcTimeSet(0, 0x51,2,    7,pbuf);
}
/*************
s----
minutes---
hours------
days-------
week------
months-----
years-----


****/

void setTime()
{
       char pbuf[7];
      pbuf[0] = 0x59;
      pbuf[1] = 0x28;
      pbuf[2] = 0x10;
      pbuf[3] = 0x22;
      pbuf[4] = 0x05;
      pbuf[5] = 0x04;
      pbuf[6] = 0x11;
      bspSetTime(pbuf);
      
}
void getTime()
{
     char pbuf[7];
     bspGetTime(pbuf);
     printf("time:%2d,%2d,%2d,%2d,%2d,%2d\n",pbuf[0],pbuf[1],pbuf[2],pbuf[3],pbuf[5],pbuf[6]);
}





/****************************************************************************
FAN_CONTROL PCI_AD14(AA19)(GPOUT9),POWER_CONTROL PCI_AD13(AB19)(GPOUT10)
********************************************************************************/
/*********************************************************

flag = 0 set the corresponding bit 0        
flag =1 set the corresponding bit 1

***********************************************************/
void FanControl(unsigned char flag)
{

   	if (flag)
	{
		*M85XX_GPOUTDR(CCSBAR)  |= 0x00400000;	/*  FAN_CONTROL PCI_AD14(AA19)(GPOUT9)   */
	}
	else
	{
		*M85XX_GPOUTDR(CCSBAR) &= 0xffbfffff;	/* clear  FAN_CONTROL PCI_AD14(AA19)(GPOUT9)   */
	}
	
}
void PowerControl(unsigned char flag)
{
	if (flag)
	{
		*M85XX_GPOUTDR(CCSBAR)  |= 0x00200000;	/*  POWER_CONTROL PCI_AD13(AB19)(GPOUT10)  */
	}
	else
	{
		*M85XX_GPOUTDR(CCSBAR) &= 0xffdfffff;	/* clear  POWER_CONTROL PCI_AD13(AB19)(GPOUT10)   */
	}
}

 /************************************
0xd100005c的最低比特,写0打开，写1关闭RRU电源

 ******************************************/


 void  OpenRRUPower()
 {
        *(unsigned short*)0xd100005c  =0; 
 }

 void CloseRRUPower()
 {
       *(unsigned short*)0xd100005c = 1; 
 }

