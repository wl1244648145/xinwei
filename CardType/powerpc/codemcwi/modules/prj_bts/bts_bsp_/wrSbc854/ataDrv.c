/* ataDrv.c - ATA/IDE and ATAPI CDROM (LOCAL and PCMCIA) disk device driver */

/*
 * Copyright (c) 1989, 2001-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
02n,15jun07,dee  allow operation with 6300esb I/O controller
02m,27mar07,pmr  SMP-safe
02l,13jul06,dee  SPR:117483, WIND00057492, WIND00057622
		 add ataRawio support for mkbootxxx functionality 
02k,19jan06,dee  SPR# 118025, add core dump support
02j,13oct05,dee  SPR# 113433, better error handling for non-existent ATA
                 drives
02i,12oct05,dee  Fix SPR#113365, detect removable device on first insertion;
		 SPR#113108, cleanup gnu compiler warnings
02h,19sep05,dee  Fix SPR#112529, change all ERF events to ASYNC
02g,07sep05,dee  SPR#111796 fix ioctl handling when checking for media present
02f,01sep05,pcm  fixed documentation
02e,08aug05,dee  Change to XBD (extended block device) interface.
02d,29jul05,dee  checkin new dma version ATAPI-5
02c,24may05,dee  fix SPR#102006, pccard ata disk, cleanup copyright
02b,07sep04,jyo  Fixing SPR 100119: Removing references to RT11.
02a,18feb04,rfr  Added ioctl FIOSYNC
01i,22feb02,rip  api change for ModeNegotiate (to take into account chan,drv)
                 Also merged in changes from smz wrt cdrom multimedia
01h,01feb02,ero  changed the name of the function ataDrv to ataPiDrv
01g,01feb02,taw  Bug Fix
01f,31jan02,smz  removed hardware depencies.
01e,24jan02,bsp  bug fixed in determining best multi DMA mode if supported.
01d,15jan02,bsp  memory initialization for packet command functions done  
01c,11jan02,bsp  some of function names changed as WRCC rule
01b,20dec01,bsp  standardized as per WRCC and some functions moved from/to 
                 atapiLib.c to differentiate from low level function to 
                 high level ( with respective to functionality).
01a,21Nov01,bsp  written.
*/

/*
DESCRIPTION:
\sh BLOCK DEVICE DRIVER:
This is a Block Device Driver for ATA/ATAPI devices on IDE host controller. 
It also provides neccessary functions to user for device and its features 
control which are not used or utilized by file system. 

This driver provides standard Block Device Driver functions,(blkRd, blkWrt, 
ioctl, statusChk, and reset) for ATA and ATAPI devices separately as the 
scheme of implementation differs. These functions are implemented as 
ataBlkRd(), ataBlkWrt(), ataBlkIoctl(), ataStatus() and ataReset() for ATA
devices and atapiBlkRd(), atapiBlkWrt(), atapiBlkIoctl(), atapiStatusChk() 
and atapiReset() for ATAPI devices. The Block Device Structure BLK_DEV is
updated with these function pointers ata initialization of the driver 
depending on the type of the device in function ataDevCreate().

ataDrv(), a user callable function, initializes ATA/ATAPI devices present on
the specified IDE controller(either primary or secondary), which must be called
once for each controller, before usage of this driver, usally called from
usrRoot()in usrConfig.c. 

The routine ataDevCreate(), which is user callable function, is used to mount
a logical drive on an ATAPI drive.This routine returns a pointer to BLK_DEV 
structure, which is used to mount the file system on the logical drive.

\sh OTHER NECESSARY FUNCTIONS FOR USER:

There are various functions provided to user, which can be classified to 
different catagories as device contol function, device information functions 
and functions meant for packet devices.

Device Control Function:

atapiIoctl() function is used to control a device. Block Device Driver functions 
ataBlkIoctl() and atapiBlkIcotl()functions are also routed to this function. 
This function implements various control command functions which are not used 
by the I/O system (like power managment feature set commands, host protected 
feature set commands, security feature set commands, media control functions 
etc).

Device Information Function:

In this catagory various functions are implmented depending on the information 
required. These functions return information required ( like cylinder count, 
Head count, device serial number, device Type, etc)from the internal device 
structures.

Packet Command Functions:

Although Block Device Driver functions deliver packet commands using functions 
provided by atapiLib.c for required functionality. There are group of functions 
provided in this driver to user for ATAPI device, which implements packet 
commands for 'CD_ROM' that comply to 'ATAPI-SFF8020i' specification which are 
essentially required for CD ROM operation for file system.  These 
functions are named after their command name (like for REQUEST SENSE packet 
command atapiReqSense() function).  To issue other packet commands 
atapiPktCmdSend() can be used.

This driver also provides a generic function atapiPktCmdSend() to issue a 
packet command to ATAPI devices, which can be utilized by user to issue packet
command directly instead using the implmented functions also may be used to 
send new commands ( may come in later specs) to device. User can issue any 
packet command using atapiPktCmdSend() function to the required device by 
passing its BLK_DEV structure pointer and pointer for ATAPI_CMD command packet.

typedef of ATAPI_CMD

.CS
    typedef struct atapi_cmd
        {
        UINT8          cmdPkt [ATAPI_MAX_CMD_LENGTH];
        char           **ppBuf;
        UINT32         bufLength;
        ATA_DATA_DIR   direction;
        UINT32         desiredTransferSize;
        BOOL           dma;
        BOOL           overlap;
        } ATAPI_CMD;
.CE

and ATA_DATA_DIR typedef is 

.CS
typedef enum  /@ with respect to host/memory @/
    {
    NON_DATA, /@ non data command     @/
    OUT_DATA, /@ to drive from memory @/
    IN_DATA   /@ from drive to memory @/
    } ATA_DATA_DIR;
.CE

User is expected supposed to fill the ATAPI_CMD structure with required 
parameters of the packet and pass the ATAPI_CMD structure pointer to  
atapiPktCmdSend() fuuction for command execution.  

All the packet command functions require ATA_DEV structure to be passed, which 
alternatively a BLK_DEV Device Structure of the device. One should type convert 
the structure and the same BLK_DEV structrue pointer to these functions.

The routine ataPiRawio() supports physical I/O access. The first
argument is the controller number, 0 or 1; the second argument is drive
number, 0 or 1; the third argument is a pointer to an ATA_RAW structure.

\sh PARAMETERS:
The ataPiDrv() function requires a configuration flag as a parameter.
The configuration flag is one of the following or Bitwise OR of any of the
following combination:

configuration flag =
    Transfer mode | Transfer bits | Transfer unit | Geometry parameters

.TS
tab(|);
l l l.
Transfer mode       | Description                   |  Transfer Rate

ATA_PIO_DEF_0       | PIO default mode              |
ATA_PIO_DEF_1       | PIO default mode, no IORDY    |
ATA_PIO_0           | PIO mode 0                    |     3.3 MBps
ATA_PIO_1           | PIO mode 1                    |     5.2 MBps
ATA_PIO_2           | PIO mode 2                    |     8.3 MBps
ATA_PIO_3           | PIO mode 3                    |    11.1 MBps
ATA_PIO_4           | PIO mode 4                    |    16.6 MBps
ATA_PIO_AUTO        | PIO max supported mode        |
ATA_DMA_SINGLE_0    | Single DMA mode 0             |     2.1 MBps
ATA_DMA_SINGLE_1    | Single DMA mode 1             |     4.2 MBps
ATA_DMA_SINGLE_2    | Single DMA mode 2             |     8.3 MBps
ATA_DMA_MULTI_0     | Multi word DMA mode 0         |     4.2 MBps
ATA_DMA_MULTI_1     | Multi word DMA mode 1         |    13.3 MBps
ATA_DMA_MULTI_2     | Multi word DMA mode 2         |    16.6 MBps
ATA_DMA_ULTRA_0     | Ultra DMA mode 0              |    16.6 MBps
ATA_DMA_ULTRA_1     | Ultra DMA mode 1              |    25.0 MBps
ATA_DMA_ULTRA_2     | Ultra DMA mode 2              |    33.3 MBps
ATA_DMA_ULTRA_3     | Ultra DMA mode 3              |    44.4 MBps
ATA_DMA_ULTRA_4     | Ultra DMA mode 4              |    66.6 MBps
ATA_DMA_ULTRA_5     | Ultra DMA mode 5              |   100.0 MBps
ATA_DMA_AUTO        | DMA max supported mode        |

Transfer bits

ATA_BITS_16      | RW bits size, 16 bits
ATA_BITS_32      | RW bits size, 32 bits

Transfer unit

ATA_PIO_SINGLE   | RW PIO single sector
ATA_PIO_MULTI    | RW PIO multi sector

Geometry parameters

ATA_GEO_FORCE    | set geometry in the table
ATA_GEO_PHYSICAL | set physical geometry
ATA_GEO_CURRENT  | set current geometry
.TE

ISA SingleWord DMA mode is obsolete in ata-3.

The Transfer rates shown above are the Burst transfer rates.
If ATA_PIO_AUTO is specified, the driver automatically chooses the maximum
PIO mode supported by the device. If ATA_DMA_AUTO is specified, the driver
automatically chooses the maximum Ultra DMA mode supported by the device and if
the device doesn't support the Ultra DMA mode of data transfer, the driver
chooses the best Multi Word DMA mode. If the device doesn't support the
multiword DMA mode, driver chooses the best single word DMA mode. If the device
doesn't support DMA mode, driver automatically chooses the best PIO mode.
So it is recommended to specify the ATA_DMA_AUTO.

If ATA_PIO_MULTI is specified, and the device does not support it, the driver
automatically chooses single sector or word mode. If ATA_BITS_32 is specified,
the driver uses 32-bit transfer mode regardless of the capability of the drive.
The Single word DMA mode will not be supported by the devices compliant to
ATA/ATAPI-5 or higher.

This driver supports UDMA mode data transfer from device to host, provided 80 
conductor cable is used for required controller device. This check done at the 
initilisation of the device from the device parameters and if 80 conductor
cable is connected then UDMA mode transfer is selected for operation subject to 
condition that required UDMA mode is supported by device as well as host. 
This driver follows ref-3 Chapter 4 "Determining a Drive's Transfer Rate 
Capability" to determine drives best transfer rate for all modes ( ie UDMA, 
MDMA, SDMA and PIO modes).

The host IDE Bus master functions are to be mapped to follwing macro defined 
for various functionality in header file which are used in this driver. 

'ATA_HOST_CTRL_INIT'          - initialize the controller

'ATA_HOST_DMA_ENGINE_INIT'    - initialize bus master DMA engine  

'ATA_HOST_DMA_ENGINE_START'   - Start bus master operation

'ATA_HOST_DMA_ENGINE_STOP'    - Stop bus master operation

'ATA_HOST_DMA_TRANSFER_CHK'   - check bus master data transfer complete

'ATA_HOST_DMA_MODE_NEGOTIATE' - get mode supported by controller

'ATA_HOST_SET_DMA_RWMODE'     - set controller to required mode

'ATA_HOST_CTRL_RESET'         - reset the controller  

If ATA_GEO_PHYSICAL is specified, the driver uses the physical geometry
parameters stored in the drive.  If ATA_GEO_CURRENT is specified,
the driver uses current geometry parameters initialized by BIOS.
If ATA_GEO_FORCE is specified, the driver uses geometry parameters stored
in sysLib.c.

The geometry parameters are stored in the structure table
`ataTypes[]' in sysLib.c. That table has two entries, the first for
drive 0, the second for drive 1. The members of the structure are:
.CS
    int cylinders;              /@ number of cylinders @/
    int heads;                  /@ number of heads @/
    int sectors;                /@ number of sectors per track @/
    int bytes;                  /@ number of bytes per sector @/
    int precomp;                /@ precompensation cylinder @/
.CE


The driver supports two controllers and two drives on each. This is dependent 
on the configuration parameters supplied to ataPiDrv().

\sh SMP CONSIDERATIONS
Most of the processing in this driver occurs in the context of a dedicated
task, and therefore is inherently SMP-safe.  One area of possible concurrence
occurs in the interrupt service routine, ataIntr().  An ISR-callable spin lock
take/give pair has been placed around the code which acknowledges/clears the
ATA controller's interrupt status register.  If the BSP or application provides
functions for ataIntPreProcessing or ataIntPostProcessing, consideration will
have to be given to making these functions SMP-safe.  Most likely, some
portion(s) of these functions will need to be protected by a spin lock.  The 
spin lock allocated for the controller can be used.  Consult the SMP Migration
Guide for hints.

References:
    1) ATAPI-5 specification "T13-1321D Revision 1b, 7 July 1999"
    2) ATAPI for CD-ROMs "SFF-8020i Revision 2.6, Jan 22,1996"
    3) Intel 82801BA (ICH2), 82801AA (ICH), and 82801AB (ICH0) IDE Controller
       Programmer's Reference Manual, Revision 1.0 July 2000

Source of Reference Documents:
    1) ftp://ftp.t13.org/project/d1321r1b.pdf
    2) http://www.bswd.com/sff8020i.pdf

SEE ALSO:
.pG "I/O System"

*/

/* includes */

#include <vxWorks.h>
#include <taskLib.h>
#include <ioLib.h>
#include <memLib.h>
#include <stdlib.h>
#include <errnoLib.h>
#include <stdio.h>
#include <string.h>
#include <private/semLibP.h>
#include <intLib.h>
#include <iv.h>
#include <wdLib.h>
#include <sysLib.h>
#include <sys/fcntlcom.h>
#include <logLib.h>
#include <drv/xbd/xbd.h>           /* XBD library header */
#include <drv/erf/erfLib.h>        /* event frame work library header */
#include <drv/pcmcia/pccardLib.h>
#include "ataDrv.h"

/* SMP-safe */
#ifdef _WRS_VX_SMP
#include <spinLockLib.h>
#define ATA_SPIN_ISR_INIT(p) \
    SPIN_LOCK_ISR_INIT(&p->spinlock, 0);
#define ATA_SPIN_ISR_TAKE(p) \
    SPIN_LOCK_ISR_TAKE(&p->spinlock)
#define ATA_SPIN_ISR_GIVE(p) \
    SPIN_LOCK_ISR_GIVE(&p->spinlock)
#else /* _WRS_VX_SMP */
#define ATA_SPIN_ISR_INIT(p)
#define ATA_SPIN_ISR_TAKE(p)
#define ATA_SPIN_ISR_GIVE(p)
#endif /* _WRS_VX_SMP */

/* Import the Definitions, which are defined in BSP's sysLib.c */

IMPORT ATA_TYPE     ataTypes [ATA_MAX_CTRLS][ATA_MAX_DRIVES];
IMPORT ATA_RESOURCE ataResources [ATA_MAX_CTRLS];
//IMPORT void sysAtaInit(ATA_CTRL *pCtrl);

/* forward declarations */

LOCAL STATUS ataDeviceSelect (ATA_CTRL *pCtrl, int device);
LOCAL void   ataWdog    (int ctrl);
LOCAL struct bio * ataGetNextBio (ATA_CTRL *ataXbdCtrl);
LOCAL void ataExecBio (ATA_CTRL *ataXbdCtrl, struct bio *bio, int drive); 
LOCAL void ataXbdService (ATA_CTRL *ataXbdCtrl); 
LOCAL void ataXbdDevCreateSyncHandler (UINT16, UINT16, void *, void *);
LOCAL int ataXbdMediaTest ( ATA_XBD * );
LOCAL int ataXbdTest ( ATA_XBD * );
LOCAL int ataXbdEject ( ATA_XBD *);
LOCAL STATUS ataCoreDump ( ATA_DEV *, sector_t, UINT32, char *);
LOCAL STATUS ataCoreWrite ( int, int, UINT32, UINT32, UINT32, void *,
			    UINT32, sector_t);

#ifdef ATA_DEV_BOOT_SEC_FIND
STATUS ataDevBootSecFind (int ctrl, int dev);
#endif /* ATA_DEV_BOOT_SEC_FIND */
//#define ATA_DEBUG
#if defined (ATA_DEBUG)

#ifndef ATA_DEBUG_LEVEL
#define ATA_DEBUG_LEVEL 0  /* if not defined on command line, default to 1 */
#endif

int  ataIntrDebug   = 0 ;/* interrupt notification On */
int  ataDebugLevel  = ATA_DEBUG_LEVEL; /* debug messages output level */

LOCAL char * ataErrStrs [] =      /* error reason strings */
{
    "No Sense",                                         /*  0 */
    "Recovered Error",                                  /*  1 */
    "Not Ready",                                        /*  2 */
    "Medium Error",                                     /*  3 */
    "Hardware Error",                                   /*  4 */
    "Illegal Request",                                  /*  5 */
    "Unit Attention",                                   /*  6 */
    "Data Protected",                                   /*  7 */
    "Blank check",                                      /*  8 */
    "Vendor specific",                                  /*  9 */
    "Copy aborted",                                     /* 10 */
    "Command aborted",                                  /* 11 */
    "Equal",                                            /* 12 */
    "Volume overflow",                                  /* 13 */
    "Miscompare",                                       /* 14 */
    "Reserved",                                         /* 15 */
    "Wait for Command Packet request time expire",      /* 16 */
    "Error in Command Packet Request",                  /* 17 */
    "Wait for Data Request time expire",                /* 18 */
    "Data Request for NON Data command",                /* 19 */
    "Error in Data Request",                            /* 20 */
    "Error in End of data transfer condition",          /* 21 */
    "Extra transfer request",                           /* 22 */
    "Transfer size requested exceeds desired size",     /* 23 */
    "Transfer direction miscompare",                    /* 24 */
    "Overlapped commands are not implemented",          /* 25 */
    "DMA transfer is not implemented",                  /* 26 */
    "Unknown Error",                                    /* 27 */
};

#endif


/* forward declarations */

LOCAL int ataXbdIoctl    (struct xbd *, int, void *);
LOCAL int ataXbdStrategy (struct xbd *, struct bio *);
LOCAL int ataXbdDump     (struct xbd *, sector_t, void *, size_t);

ATA_LOCAL STATUS atapiNonOverlapTransferLoop (ATA_DEV    * pAtapiDev,
                                              ATAPI_CMD  * pComPack,
                                              FUNCPTR    pTransferProc);
LOCAL STATUS atapiPIOTransfer (ATA_DEV * pAtapiDev);
LOCAL STATUS atapiPktCmdExec  (ATA_DEV * pAtapiDev, ATAPI_CMD * pComPack);


LOCAL STATUS atapiDmaTransfer (ATA_DEV * pAtapiDev);
LOCAL STATUS atapiReqSense (ATA_DEV *pAtapiDev);

#ifdef ATAPI_OVERLAPPED_FEATURE
LOCAL STATUS atapiOverlapTransferLoop (ATA_DEV    * pAtapiDev,
                                       ATAPI_CMD  * pComPack
                                       FUNCPTR    pTransferProc);
#endif  /* ATAPI_OVERLAPPED_FEATURE */


ATA_CTRL  ataCtrl [ATA_MAX_CTRLS];   /* Number of controllers is hardware dependant */
                                     /* see sysAta.c */

/* BSP specific ATA Init/Reset routine */

VOIDFUNCPTR       _func_sysAtaInit = NULL;

/* local variables  */

LOCAL int  ataRetry          = 2;       /* max retry count */
int        flushCacheCount   = 0;     /* counter for flush ioctl */   
LOCAL int  coreIsDumping     = 0;
/*
 * functions to be exported to XBD interface
 */
LOCAL struct xbd_funcs ataXbdFuncs =
    {
    ataXbdIoctl,
    ataXbdStrategy,
    ataXbdDump,
    };

#ifdef ATA_DEBUG
void atapiParamsPrint  (int ctrl, int drive);
#endif

LOCAL void   ataIntr    (ATA_CTRL *pCtrl);

#ifdef HOST_PROT_AREA_FEATURE
LOCAL UINT32 ataCHSPack (UINT8 sectorNo, UINT8 cylinderLow,
                         UINT8 cylinderHigh, UINT8 Head);
#endif

LOCAL STATUS ataStub (void);

LOCAL STATUS ataBlkRd (ATA_DEV * pDev, UINT32 startBlk, UINT32 nBlks, char * p);
LOCAL STATUS ataBlkWrt (ATA_DEV * pDev, UINT32 startBlk, UINT32 nBlks, char * p);
LOCAL STATUS ataBlkIoctl (ATA_DEV * pDev, int function, int arg);
LOCAL STATUS atapiBlkIoctl (ATA_DEV * pDev, int function, int arg);
LOCAL STATUS ataReset (ATA_DEV * pDev);
LOCAL STATUS ataStatus (ATA_DEV * pDev);
LOCAL void   ataBlkDevFill (ATA_DEV *   pDev);
ATA_LOCAL STATUS atapiStatusChk (ATA_DEV * pDev);
LOCAL STATUS atapiReset (ATA_DEV * pAtapiDev);
LOCAL STATUS atapiBlkWrt (ATA_DEV * pAtapiDev, UINT32 startBlk,
                          UINT32 nBlks, char * pBuf);
LOCAL STATUS atapiBlkRd (ATA_DEV * pAtapiDev, UINT32 startBlk,
                         UINT32 nBlks, char * pBuf);
LOCAL STATUS ataDriveInit (int ctrl,int drive);
LOCAL void ataBestTransferModeFind  (int ctrl, int drive);
LOCAL void ataUDmaCableChk (int ctrl,int drive); 

/*******************************************************************************
*
* ataIntr - ATA/IDE controller interrupt handler.
*
* RETURNS: N/A
*/

LOCAL void ataIntr
    (
    ATA_CTRL *pCtrl
    )
    {
#if 0
    /* perform some pre processing if necessary */
    if (pCtrl->ataIntPreProcessing != NULL)
        (*pCtrl->ataIntPreProcessing)(pCtrl);

    pCtrl->intCount++;

    /* the following clears the interrupt */
    ATA_SPIN_ISR_TAKE(pCtrl);
    pCtrl->intStatus = ATA_IO_BYTE_READ (ATAPI_STATUS);
    ATA_SPIN_ISR_GIVE(pCtrl);

    /* perform some post processing if necessary */
    if (pCtrl->ataIntPostProcessing != NULL)
        (*pCtrl->ataIntPostProcessing)(pCtrl);

    semGive (&pCtrl->syncSem);
    ATA_DEBUG_MSG(19,"ataIntr:  ctrl=%d ISR count %d Status= %#x \n",
                  pCtrl->ctrl, pCtrl->intCount, pCtrl->intStatus, 0, 0, 0);
#endif
 #if 0
		unsigned int* cfIntrAdrs = (unsigned int*)0x1400f11c;
		unsigned int*	cpuIntCauseAdrs = (unsigned int*)0x1400000c;
		unsigned int* cpu_mpp24_31IntEnableAdrs = (unsigned int*)0x1400001c;
#endif
//     *M85XX_EIVPR11(CCSBAR)&=0xffffffff;
	*M85XX_EIVPR11(CCSBAR)|=0x80000000;

    	ATA_DEBUG_MSG(3,"AtaIntr getting the int status\n",0,0,0,0,0,0);/*add by xcl*/

	pCtrl->intCount++;
    	ATA_DEBUG_MSG(3,"ataIntr:  ISR count %d Status= %#x \n",
                   pCtrl->intCount, pCtrl->intStatus, 0, 0, 0,0);

//    ATA_CTRL *pCtrl	= &ataCtrl[ctrl];
  //  	ATA_DEBUG_MSG(3,"intLevel = %x,intStatus = %x\n",pCtrl->intLevel,pCtrl->status,0,0,0,0);/*add by xcl*/
#if 0
    ATA_DEBUG_MSG(3,"AtaIntr getting the int status\n",0,0,0,0,0,0);/*add by xcl*/
		*cpu_mpp24_31IntEnableAdrs = *cpu_mpp24_31IntEnableAdrs & 0xfffffff7;  /* mask the interrup of cpu Int[high]*/
#endif
//    pCtrl->intCount++;
    pCtrl->intStatus = ATA_IO_BYTE_READ (ATAPI_STATUS  /*pCtrl->intStatus*/);
#if 0
    *cfIntrAdrs = *cfIntrAdrs & 0x00000004;
    *cpuIntCauseAdrs = *cpuIntCauseAdrs & 0xfffffff7;
		*cpu_mpp24_31IntEnableAdrs = *cpu_mpp24_31IntEnableAdrs | 0x00000008; 
    #endif
  *M85XX_EIVPR11(CCSBAR)&=0x7fffffff;
    semGive (&pCtrl->syncSem);
    }

/**************************************************************************
*
* ataDrv - Initialize the ATA driver
*
* This routine initializes the ATA/ATAPI device driver, initializes IDE
* host controller and sets up interrupt vectors for requested controller. This 
* function must be called once for each controller, before any access to drive 
* on the controller, usually which is called by usrRoot() in usrConfig.c. 
* 
* If it is called more than once for the same controller, it returns OK with a 
* message display 'Host controller already initialized ', and does nothing as 
* already required initialization is done.  
*
* Additionally it identifies devices available on the controller and 
* initializes depending on the type of the device (ATA or ATAPI). Initialization
* of device includes reading parameters of the device and configuring to the 
* defaults.
* 
* RETURNS: OK, or ERROR if initialization fails.
*
* SEE ALSO: ataDevCreate()
*/

STATUS ataDrv
    (
    int ctrl,       /* controller no. 0,1   */
    int drives,     /* number of drives 1,2 */
    int vector,     /* interrupt vector     */
    int level,      /* interrupt level      */
    int configType, /* configuration type   */
    int semTimeout, /* timeout seconds for sync semaphore */
    int wdgTimeout  /* timeout seconds for watch dog      */
    )
    {
    ATA_CTRL        * pCtrl     = &ataCtrl[ctrl];
    ATA_RESOURCE    *pAta       = &ataResources[ctrl];
    PCCARD_RESOURCE *pResource  = &pAta->resource;
    ATA_DRIVE       *pDrive;
    ATA_TYPE        *pType;
    int             drive;
    STATUS          dok = OK;

    ATA_DEBUG_MSG (14,"ataDrv: entered\n\n", 0, 0, 0, 0, 0, 0);
    ATA_DEBUG_MSG( 13,"     controller   = %d \n     No of drives = %d \n"
                      "     vector       = %#x\n     level        = %#x\n"
                      "     configType   = %#x\n     semTimeout   = %d \n",
                      ctrl, drives, vector, level, configType, semTimeout);
      
    if ((ctrl >= ATA_MAX_CTRLS) || (drives > ATA_MAX_DRIVES))
        {
        printErr("ataDrv: Invalid Controller number or Number of drives");
        return(ERROR);
        }

    if (!(pCtrl->installed))
        {
        /* zero entire structure first */
        bzero((char *)pCtrl, sizeof (ATA_CTRL));

        pCtrl->ctrl = ctrl;   /* save the controller number for quick access */

        /* setup controller I/O address first
         * This is done early so we can use device access functions
         */
        pCtrl->ataReg.data     = (UINT32 *)ATA_DATA      (pResource->ioStart[0]);
        pCtrl->ataReg.error    = (UINT32 *)ATA_ERROR     (pResource->ioStart[0]);
        pCtrl->ataReg.feature  = (UINT32 *)ATA_FEATURE   (pResource->ioStart[0]);
        pCtrl->ataReg.seccnt   = (UINT32 *)ATA_SECCNT    (pResource->ioStart[0]);
        pCtrl->ataReg.sector   = (UINT32 *)ATA_SECTOR    (pResource->ioStart[0]);
        pCtrl->ataReg.cylLo    = (UINT32 *)ATA_CYL_LO    (pResource->ioStart[0]);
        pCtrl->ataReg.cylHi    = (UINT32 *)ATA_CYL_HI    (pResource->ioStart[0]);
        pCtrl->ataReg.sdh      = (UINT32 *)ATA_SDH       (pResource->ioStart[0]);
        pCtrl->ataReg.command  = (UINT32 *)ATA_COMMAND   (pResource->ioStart[0]);
        pCtrl->ataReg.status   = (UINT32 *)ATA_STATUS    (pResource->ioStart[0]);
        pCtrl->ataReg.aStatus  = (UINT32 *)ATA_A_STATUS  (pResource->ioStart[1]);
        pCtrl->ataReg.dControl = (UINT32 *)ATA_D_CONTROL (pResource->ioStart[1]);
   //     printf("hehe\n");
 // return(ERROR);


          ATA_DEBUG_MSG (18,"ataDrv1: %x,%x\n",
                       pResource->ioStart[0], pResource->ioStart[1], 0, 0, 0, 0);
   
             ATA_DEBUG_MSG(20, "ata Drv:%x,%x,%x,%x,%x,%x\n",
                          pCtrl->ataReg.data ,pCtrl->ataReg.error ,pCtrl->ataReg.feature,pCtrl->ataReg.seccnt,pCtrl->ataReg.sector,pCtrl->ataReg.cylLo);

                ATA_DEBUG_MSG(20, "ata Drv1:%x,%x,%x,%x,%x,%x\n",
                         pCtrl->ataReg.cylHi , pCtrl->ataReg.sdh ,pCtrl->ataReg.command,pCtrl->ataReg.status,pCtrl->ataReg.aStatus,pCtrl->ataReg.dControl);
        if (ataCtrlReset(ctrl) == ERROR)
            {
            ATA_DEBUG_MSG (18,"ataDrv: Controller %d reset failed\n",
                           ctrl, 0, 0, 0, 0, 0);
            return(ERROR);
            }

        ATA_DEBUG_MSG (18,"ataDrv: Controller %d to be installed, intCount %x\n",
                       ctrl, pCtrl->intCount, 0, 0, 0, 0);

        if (semCInit (&pCtrl->ataBioReadySem, SEM_Q_PRIORITY, SEM_EMPTY) != OK)
            {
            ATA_DEBUG_MSG(20, "ataDrv: cannot initialize Sync semaphore, returning\n",
                          0,0,0,0,0,0);
            return(ERROR);
            }
        if (semBInit (&pCtrl->syncSem, SEM_Q_FIFO, SEM_EMPTY) != OK)
            {
            ATA_DEBUG_MSG(20, "ataDrv: cannot initialize Sync semaphore, returning\n",
                          0,0,0,0,0,0);
            return(ERROR);
            }
        if (semMInit (&pCtrl->muteSem, 
                      SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE) != OK)
            {
            ATA_DEBUG_MSG(20, "ataDrv: cannot initialize Mutex semaphore, returning\n", 
                          0,0,0,0,0,0);
            return(ERROR);
            }

        /* SMP-safe */
	ATA_SPIN_ISR_INIT (pCtrl);

        /* Call system INIT routine to allow bsp specific parameters; */
        /* like DMA and interrupts */
        ATA_DEBUG_MSG (1, "ataDrv: Calling sysAtaInit (if present):\n",
                       0, 0, 0, 0, 0, 0);
        SYS_ATA_INIT_RTN (pCtrl);

        if (pCtrl->ataDmaReset != NULL)
            /* execute DMA reset function for this controller */
            (*pCtrl->ataDmaReset)(ctrl);

        if (pCtrl->ataDmaInit != NULL)
	    {
            /* execute DMA init function for this controller */
            if ((*pCtrl->ataDmaInit)(ctrl) != OK)
		pCtrl->ataHostDmaSupportOkay = FALSE; /* ctrl not found, no DMA */
            }
        
        pCtrl->wdgId = wdCreate ();

        if (semTimeout == 0)
            pCtrl->semTimeout = ATA_SEM_TIMEOUT_DEF;
        else
            pCtrl->semTimeout = semTimeout;

        if (wdgTimeout == 0)
            pCtrl->wdgTimeout = ATA_WDG_TIMEOUT_DEF;
        else
            pCtrl->wdgTimeout = wdgTimeout;

        if (pCtrl->ataIntConnect == NULL) /* bsp did not setup pointer, use default */
            pCtrl->ataIntConnect = (FUNCPTR)intConnect;

        (*pCtrl->ataIntConnect)((VOIDFUNCPTR *)INUM_TO_IVEC (vector),
                                         (VOIDFUNCPTR)ataIntr, pCtrl);

        if (pCtrl->ataIntEnable == NULL)  /* bsp did not setup pointer, use default */
            pCtrl->ataIntEnable = (FUNCPTR)sysIntEnablePIC;
        (*pCtrl->ataIntEnable)(level);    /* unmask the interrupt level */

        ATA_DEBUG_MSG(20, "ataDrv: Attached interrupts ctrl=%x\n", ctrl, 0,0,0,0,0);

        pCtrl->intLevel   = level;
        pCtrl->wdgOkay    = TRUE;
        pCtrl->configType = configType;

        pCtrl->installed = TRUE;
              
        for (drive = 0; drive < drives; drive++)
            {
            pDrive = &pCtrl->drive[drive];
            pType  = &ataTypes[ctrl][drive];

            ATA_DEBUG_MSG(20,"ataDrv: pType = 0x%x, Addr of pDrive->driveInfo = 0x%x\n", 
                          (int)pType, (int)&pDrive->driveInfo, 0,0,0,0);

            pDrive->driveInfo   = pType;  /* save pointer to drive info */
            pDrive->state       = ATA_DEV_INIT;
            pDrive->type        = ATA_TYPE_INIT;
            pDrive->diagCode    = 0;
            pDrive->Reset       = ataInit;

            ATA_DEBUG_MSG(20, "ataDrv: Calling ataDriveInit: %x/%x\n",
                               ctrl, drive, 0,0,0,0);

            if ((ataDriveInit (ctrl, drive)) == ERROR)
                {
                dok = ERROR;

                /* zero entire structure to indicate drive is not present. */
                bzero((char *)pDrive, sizeof (ATA_DRIVE));

                printErr("ataDrv: ataDriveInit failed on Channel %d Device %d\n",
                              ctrl, drive);
                }
            }

        ATA_DEBUG_MSG (1, "ataDrv sysAtaInit returned:\n",
                       0, 0, 0, 0, 0, 0);

        }
    else
        printErr ("Host controller already initialized \n");        

    return(dok);
    } /* ataPiDrv */


/*******************************************************************************
*
* ataXbdDevCreate - create an XBD device for a ATA/IDE disk
*
* Use the existing code to create a standard block dev device, then create an
* XBD device associated with the BLKDEV.
* RETURNS: a device identifier upon success, or NULLDEV otherwise
*
* ERRNO:
*/

device_t ataXbdDevCreate
    (
    int ctrl,         /* ATA controller number, 0 is the primary controller */
    int drive,        /* ATA drive number, 0 is the master drive */
    UINT32 nBlocks,   /* number of blocks on device, 0 = use entire disc */
    UINT32 blkOffset, /* offset BLK_DEV nBlocks from the start of the drive */
    const char * name /* name of xbd device to create */
    )
    {
    ATA_DEV   * pDev;
    BLK_DEV   * blkDev;
    ATA_CTRL  * pCtrl   = &ataCtrl[ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[drive];
    device_t    retVal;
    int         error;
    char        s[20];      /* used for building task name */

    if ((ctrl >= ATA_MAX_CTRLS) || (drive >= ATA_MAX_DRIVES) || 
              !pCtrl->installed || !pDrive->driveInfo)
        {
        printErr("ataXbdDevCreate: ERROR- Device %d on Controller %d not installed\n",
                       drive, ctrl);
        return(NULLDEV);
        }

    if ((pCtrl->svcTaskId == ERROR) || (pCtrl->svcTaskId == 0))
        {
        sprintf(s, "tAtaSvc%d", ctrl);    /* build taskname with controller number */
        pCtrl->svcTaskId = taskSpawn (s, 50, 0,
                                      4096, /* Stack size. */
                                      (FUNCPTR)ataXbdService, (int) pCtrl,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0);

        if (pCtrl->svcTaskId == ERROR)
            return (NULLDEV);
        }

    if ((blkDev = ataDevCreate (ctrl, drive, nBlocks, blkOffset)) == (BLK_DEV *) NULL)
        {
        printErr ("ataXbdDevCreate ERROR: %x  Device %d on Controller %d\n",
                   errno, drive, ctrl);
        return (NULLDEV);
        }

    /* block dev device is now created.  Next instantiate the XBD for the
     * the higher level app and generate an insertion event.
     */
    pDev = pDrive->pAtaDev;
    if ( semBInit (&pDev->ataXbd.xbdSemId, SEM_Q_PRIORITY, SEM_EMPTY) != OK)
        return (NULLDEV);

    /* register new event handler for base device synchronization */
    if (erfHandlerRegister (xbdEventCategory, xbdEventInstantiated,
                            ataXbdDevCreateSyncHandler, pDev, 0) != OK)
        {
        return (NULLDEV);
        }

    strncpy((char *)&pDev->ataXbd.name, name, sizeof(devname_t));  /* store name */
    ATA_DEBUG_MSG(1,"xbd = 0x%x, funcs = 0x%x, %s, blksize=0x%x, blks=0x%x\n",
                     &pDev->ataXbd.xbd, &ataXbdFuncs, name,
                     pDev->blkDev.bd_bytesPerBlk, pDev->blkDev.bd_nBlocks, 0);
    error = xbdAttach(&pDev->ataXbd.xbd, &ataXbdFuncs, name, 
                       pDev->blkDev.bd_bytesPerBlk,
                       pDev->blkDev.bd_nBlocks, &retVal);
    ATA_DEBUG_MSG(1,"after xbdAttach\n",0,0,0,0,0,0);

    if (error == 0)
        {
        pDev->ataXbd.ataDev = pDev;
        if (pDev->ataXbd.xbd.xbd_nblocks == 0)
            pDev->ataXbd.xbdInserted = 0;  /* no media inserted */
        else
            pDev->ataXbd.xbdInserted = 1;  /* media inserted */
        blkDev->bd_readyChanged = 0;
        pDev->ataXbd.device = retVal;   /* save device instance in control block */
        erfEventRaise (xbdEventCategory, xbdEventPrimaryInsert,
                       ERF_ASYNC_PROC, (void *)retVal, NULL);
        ATA_DEBUG_MSG(1,"xbdAttached and event raised. device_t = 0x%x xbdSize=0x%x\n",
			 retVal, pDev->ataXbd.xbd.xbd_nblocks, 0, 0, 0, 0);
        semTake (&pDev->ataXbd.xbdSemId, WAIT_FOREVER);  /* wait for synchronization */
        return (retVal);
        }
    else
        return (NULLDEV);
    }

/*******************************************************************************
*
* ataDevCreate - create a device for a ATA/IDE disk
*
* This routine creates a device for a specified ATA/IDE or ATAPI CDROM disk.
*
* <ctrl> is a controller number for the ATA controller; the primary controller
* is 0.  The maximum is specified via ATA_MAX_CTRLS.
*
* <drive> is the drive number for the ATA hard drive; the master drive
* is 0.  The maximum is specified via ATA_MAX_DRIVES.
*
* The <nBlocks> parameter specifies the size of the device in blocks.
* If <nBlocks> is zero, the whole disk is used.
*
* The <blkOffset> parameter specifies an offset, in blocks, from the start
* of the device to be used when writing or reading the hard disk.  This
* offset is added to the block numbers passed by the file system during
* disk accesses.  (VxWorks file systems always use block numbers beginning
* at zero for the start of a device.)
*
*
* RETURNS:
* A pointer to a block device structure (BLK_DEV) or NULL if memory cannot
* be allocated for the device structure.
*
* SEE ALSO: dosFsMkfs(), dosFsDevInit(), rawFsDevInit()
*/

BLK_DEV * ataDevCreate
    (
    int ctrl,     /* ATA controller number, 0 is the primary controller */
    int drive,    /* ATA drive number, 0 is the master drive */
    UINT32 nBlocks,  /* number of blocks on device, 0 = use entire disc */
    UINT32 blkOffset /* offset BLK_DEV nBlocks from the start of the drive */
    )
    {
    ATA_CTRL  * pCtrl   = &ataCtrl[ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[drive];
    ATA_DEV   * pDev;

    if ((ctrl >= ATA_MAX_CTRLS) || (drive >= ATA_MAX_DRIVES) || 
              !pCtrl->installed || !pDrive->driveInfo)
        {
        printErr("ataDevCreate: ERROR- Device %d on Controller %d not installed\n",
                       drive, ctrl);
        return(NULL);
        }

    if ((pDev = (ATA_DEV *)malloc(sizeof (ATA_DEV))) == NULL)
        {
        printErr("ataDevCreate: malloc() failed\n");
        return(NULL);
        }

    /* zero entire structure first */
    bzero((char *)pDev, sizeof (ATA_DEV));

    pDrive->pAtaDev = pDev;

    pDev->ctrl          = ctrl;
    pDev->drive         = drive;
    pDev->blkOffset     = blkOffset;
    pDev->nBlocks       = nBlocks;

    ataBlkDevFill (pDev);
    ATA_DEBUG_MSG (1,"ataDevCreate%d/%d: returning to caller\n",
                      ctrl, drive, 0, 0, 0, 0);

    return(&pDev->blkDev);
    }   /* ataDevCreate */


/**************************************************************************
*
* ataDriveInit - initialize ATA drive
*
* This routine checks the drive presence, identifies its type, initializes 
* the drive and driver control structures with the parameters of the drive.
*
* RETURNS: OK if drive was initialized successfuly, or ERROR.
*/

LOCAL STATUS ataDriveInit
    (
    int ctrl,
    int drive
    )
    {
    ATA_CTRL  * pCtrl        = &ataCtrl[ ctrl];
    ATA_DRIVE * pDrive       = &pCtrl->drive[ drive];
    ATA_PARAM * pParam       = &pDrive->param;
    ATAPI_TYPE  * pType      = pDrive->driveInfo;
    int         configType   = pCtrl->configType;   /* configuration type */
    int		status = OK;

    ATA_DEBUG_MSG(1,"ataDriveInit: ctrl=%d, drive=%d\n",ctrl,drive,
                  0,0,0,0);

    if (pType->cylinders == 0)
        return(ERROR);     /* user specified the device as not present */

    if (!coreIsDumping) 
	status = semTake (&pCtrl->muteSem, sysClkRateGet() * pCtrl->semTimeout * 2);
    if (status != OK)
        {
        ATA_DEBUG_MSG (18, "ataDriveInit: Error getting mutex - ERROR\n", 
                       0, 0, 0, 0, 0, 0);
        return(ERROR);
        }
    else
        {
        ATA_DEBUG_MSG (18, "ataDriveInit %d/%d: mutex successful\n", 
                       ctrl, drive, 0, 0, 0, 0);
        }

    /* Identify device presence and its type */

    ATA_DEBUG_MSG (19, "ataDriveInit: Call ataDevIdentify %d/%d\n",
                   ctrl, drive, 0, 0, 0, 0);

    if (ataDevIdentify ( ctrl, drive) != OK)
	{
        pDrive->state = ATA_DEV_NONE;
        goto driveInitExit;
	}

    /* Decide to use the DMA mode or not, configured later */

    pDrive->usingDma = FALSE;

    /* Set Reset function according to device type */


    if (pDrive->type == ATA_TYPE_ATA)
        {
        pDrive->Reset = NULL; /* can't reset individual ATA device */

        if (ataParamRead (ctrl, drive, (char *)pParam, ATA_CMD_IDENT_DEV) == OK)
            {
            /* find out geometry */

            if ((configType & ATA_GEO_MASK) == ATA_GEO_FORCE)
                {
                ATA_DEBUG_MSG (13,"ataDriveInit: configType = ATA_GEO_FORCE\n",
                               0,0,0,0,0,0);
                (void) ataCmd (ctrl, drive, ATA_CMD_INITP, 0, 0, 0, 0, 0, 0);
                (void) ataParamRead (ctrl, drive, (char *)pParam, ATA_CMD_IDENT_DEV);
                }
            else if ((configType & ATA_GEO_MASK) == ATA_GEO_PHYSICAL)
                {
                ATA_DEBUG_MSG (13,"ataDriveInit: configType = ATA_GEO_PHYSICAL\n",
                               0,0,0,0,0,0);
                pType->cylinders = pParam->cylinders;
                pType->heads     = pParam->heads;
                pType->sectors   = pParam->sectors;
                }
            else if ((configType & ATA_GEO_MASK) == ATA_GEO_CURRENT)
                {

                if ((pParam->currentCylinders != 0)   &&
                    (pParam->currentHeads != 0) &&
                    (pParam->currentSectors != 0))
                    {
                    ATA_DEBUG_MSG (13,"ataDriveInit: configType = ATA_GEO_CURRENT1\n",
                                   0,0,0,0,0,0);
                    pType->cylinders = pParam->currentCylinders;
                    pType->heads     = pParam->currentHeads;
                    pType->sectors   = pParam->currentSectors;
                    }
                else
                    {
                    ATA_DEBUG_MSG (13,"ataDriveInit: configType = ATA_GEO_CURRENT2\n",
                                   0,0,0,0,0,0);
                    pType->cylinders = pParam->cylinders;
                    pType->heads     = pParam->heads;
                    pType->sectors   = pParam->sectors;
                    }
                }

            /* 
             * Not all modern hard drives report a true capacity value 
             * in their IDENTIFY DEVICE CHS fields.
             * For example, a Western Digital 20 Gb drive reports 
             * its CHS as 16383 cylinders, 16 heads, and 63 spt.
             * This is about 8.4GB, but the LBA sectors is reported
             * as 0x02607780, which is closer to 20Gb, the true capacity
             * of the drive.  The reason for this is PC BIOS can have a 
             * 8.4GB limitation, and drive manufacturers have broken the 
             * ATA specification to be compatable.  Negative competition. 
             * Note that the ATA specifications original limit is 
             * about 136.9 Gb, however when combinined with a PC BIOS 
             * interface, a 8.4 Gb limit is produced.    
             * VxWorks does not have such limitations being a true 32bit OS,
             * but since the drive manufactures are not honoring the CHS
             * values, we have to allow for devices that demand "pure" LBA
             * and present incorrect CHS.
             * If the drive supports Logical Block Addresses (LBA)
             * then we need to check the field located at 16bit words 60 & 61,
             * "Total number of user addressable sectors (LBA mode only)". 
             * If this value is greater than the CHS fields report, 
             * then 60-61 holds the true size of the disk and that 
             * will be reported to the block device interface.
             * Note that the CHS values are still left as the disk reported.
             * This is tracked at WRS as SPR#22830
             */

	    if (pParam->capabilities & ATA_IOLBA_MASK)
	        {   /* if (drive supports LBA) */
                pDrive->capacity = ((UINT32)pParam->lba_size_1) |
                                  (((UINT32)pParam->lba_size_2) << 16);

                ATA_DEBUG_MSG (1, "ataDriveInit %d/%d: ID_DRIVE reports LBA (60-61) as 0x%08lx\n",
                               ctrl, drive, (UINT32)pDrive->capacity, 0, 0, 0);
                }

            if (pDrive->capacity == (sector_t)MAX_28LBA)
                {
                pDrive->capacity = pParam->maxLBA[0] | (pParam->maxLBA[1] << 16) |
                          (pParam->maxLBA[2] << 32) | (pParam->maxLBA[3] << 48);
                pDrive->use48LBA = TRUE;
                }
            if (pDrive->capacity > MAX_48LBA)
                {
                return ERROR;
                }

            /*
             * reinitialize the controller with parameters read from the
             * controller.
             */

            (void) ataCmd (ctrl, drive, ATA_CMD_INITP, 0, 0, 0, 0, 0, 0);

            /*
             * recalibrate if the drive does not report the version number or
             * if the drive is compliant to ATA-3 or lower specification.
             */

            if ((pParam->majorVer == 0)    ||        /* does not report */
                ((~pParam->majorVer) == 0) ||        /* does not report */
                ((pParam->majorVer & 0xf) != 0)        /* ATA-3 or lower  */
               )
                {
                (void) ataCmd (ctrl, drive, ATA_CMD_RECALIB, ATA_ZERO, 
                               ATA_ZERO,ATA_ZERO, ATA_ZERO, ATA_ZERO,
                               ATA_ZERO);
                }

            }
        else
            {
            pDrive->state = ATA_DEV_PREAD_F;
            goto driveInitExit;
            }
        }
    else if (pDrive->type == ATA_TYPE_ATAPI)
        {
        /* Although ATAPI device parameters have been read by ataDevIdentify(), 
         * execute ATAPI Identify Device command to allow ATA commands 
         * acceptance by an ATAPI device after Diagnostic or Reset commands. 
         */

        pDrive->Reset = ataPiInit;
        if (ataParamRead (ctrl, drive, pParam, ATA_PI_CMD_IDENTD) != OK)
            {
            pDrive->state = ATA_DEV_PREAD_F;

            goto driveInitExit;
            }
        /* 
         * Set command length here. bit 0 and 1 of Word 0 of device parameters 
         * indicate command length of packet. cmdLength is 12 for CD-ROM 
         * devices. 16 for SAM compliant devices. 
         */

        pDrive->cmdLength   = (pParam->config & CONFIG_PKT_SIZE)  ? 16   : 12;
        pDrive->driveType = (UINT8)((pParam->config & CONFIG_DEV_TYPE_MASK)>>8);

        ATA_DEBUG_MSG (15,"ataDriveInit %d/%d: Packet Device\n", ctrl, drive, 0, 0, 0, 0);
        ATA_DEBUG_MSG (15,"         pDrive->cmdLength   = %#d\n"
                          "         pDrive->driveType   = %#x\n",
                       pDrive->cmdLength,
                       pDrive->driveType, 0, 0, 0, 0);

        }

    pDrive->okRemovable  =
                    (pParam->config & CONFIG_REMOVABLE) ? TRUE : FALSE;
    pDrive->okInterleavedDMA  =
                    (pParam->capabilities & ATA_INTER_DMA_MASK) ? TRUE :FALSE;
    pDrive->okCommandQue =
                    (pParam->capabilities & ATA_CMD_QUE_MASK  ) ? TRUE :FALSE;
    pDrive->okOverlap =
                    (pParam->capabilities & ATA_OVERLAP_MASK  ) ? TRUE :FALSE;

    /* find out supported capabilities of the drive */

    pDrive->multiSecs =  pParam->multiSecs & ATA_MULTISEC_MASK;
    pDrive->okMulti   = (pDrive->multiSecs != 0) ? TRUE : FALSE;
    pDrive->okIordy   = (pParam->capabilities & ATA_IORDY_MASK)  ? TRUE : FALSE;
    pDrive->okLba     = (pParam->capabilities & ATA_IOLBA_MASK)  ? USE_LBA : 0;
    pDrive->okDma     = (pParam->capabilities & ATA_DMA_CAP_MASK)? TRUE : FALSE;
    

    ATA_DEBUG_MSG (11,"ataDriveInit: \n"
                   "         pDrive->multiSecs   = %#d\n"
                   "         pDrive->okMulti     = %#d\n"
                   "         pDrive->okIordy     = %#d\n"
                   "         pDrive->okLba       = %#d\n"
                   "         pDrive->okRemovable = %#d\n"
                   "         pDrive->okDma       = %#d\n",
                   pDrive->multiSecs,
                   pDrive->okMulti,
                   pDrive->okIordy,
                   pDrive->okLba,
                   pDrive->okRemovable,
                   pDrive->okDma);

    /* Initialize the mode registers for "not supported" status */

    pDrive->pioMode       = 0xff;
    pDrive->singleDmaMode = 0xff;
    pDrive->multiDmaMode  = 0xff;
    pDrive->ultraDmaMode  = 0xff;

    /*
     * find out supported max PIO mode.
     * PIO mode less than 2 is Obsolete in ATA/ATAPI-5, Following code is for
     * downward compatibility of the driver with the drives compliant to
     * ATA/ATAPI-4 or lower.
     */

    pDrive->pioMode = (pParam->pioMode >> 8) & ATA_PIO_MASK_012;

    if (pDrive->pioMode > ATA_SET_PIO_MODE_2) /* PIO 0,1,2 */
        pDrive->pioMode = ATA_SET_PIO_MODE_0;

    /* 
     * bits 0 and 1 of word 64 gives (advancedPio) the advanced PIO 
     * mode capability 
     */

    if ((pDrive->okIordy) && (pParam->valid & ATA_WORD64_70_VALID))  /* PIO 3,4 */
        {
        /* bit 0 indicates PIO mode 3 */
        if (pParam->advancedPio & ATA_BIT_MASK0)
            pDrive->pioMode = ATA_SET_PIO_MODE_3;
        /* bit 1 indicates PIO mode 4 */
        if (pParam->advancedPio & ATA_BIT_MASK1)
            pDrive->pioMode = ATA_SET_PIO_MODE_4;
        }

    /* find out supported max DMA mode, check bit 1 of 'valid' */

    if ((pDrive->okDma) && (pParam->valid & ATA_WORD64_70_VALID))
    /* 
     * In the above check (pParam->valid & 0x02) 
     * is not required for ATAPI5 drives.
     */
        {

        /*
         * single DMA Mode is Retired in ATA3 and higher.
         * The following code is for downward compatibility of the driver with
         * drives compliant to ATA-2
         */

        pDrive->singleDmaMode = (pParam->dmaMode >> 8) & 0x03;
        if (pDrive->singleDmaMode >= ATA_SET_SDMA_MODE_2)
            pDrive->singleDmaMode = ATA_SET_SDMA_MODE_0;
        pDrive->multiDmaMode  = ATA_SET_MDMA_MODE_0;

        if (pParam->singleDma & ATA_BIT_MASK2)
            pDrive->singleDmaMode = ATA_SET_SDMA_MODE_2;
        else if (pParam->singleDma & ATA_BIT_MASK1)
            pDrive->singleDmaMode = ATA_SET_SDMA_MODE_1;
        else if (pParam->singleDma & ATA_BIT_MASK0)
            pDrive->singleDmaMode = ATA_SET_SDMA_MODE_0;

        if (pParam->multiDma & ATA_BIT_MASK2)
            pDrive->multiDmaMode = ATA_SET_MDMA_MODE_2;
        else if (pParam->multiDma & ATA_BIT_MASK1)
            pDrive->multiDmaMode = ATA_SET_MDMA_MODE_1;
        else if (pParam->multiDma & ATA_BIT_MASK0)
            pDrive->multiDmaMode = ATA_SET_MDMA_MODE_0;
        }

    /* find out supported max Ultra DMA mode */

    if ((pDrive->okDma) && (pParam->valid & ATA_WORD88_VALID))
        {
        if (pParam->ultraDmaMode & ATA_BIT_MASK5)
            pDrive->ultraDmaMode = ATA_SET_UDMA_MODE_5;
        else if (pParam->ultraDmaMode & ATA_BIT_MASK4)
            pDrive->ultraDmaMode = ATA_SET_UDMA_MODE_4;
        else if (pParam->ultraDmaMode & ATA_BIT_MASK3)
            pDrive->ultraDmaMode = ATA_SET_UDMA_MODE_3;
        else if (pParam->ultraDmaMode & ATA_BIT_MASK2)
            pDrive->ultraDmaMode = ATA_SET_UDMA_MODE_2;
        else if (pParam->ultraDmaMode & ATA_BIT_MASK1)
            pDrive->ultraDmaMode = ATA_SET_UDMA_MODE_1;
        else if (pParam->ultraDmaMode & ATA_BIT_MASK0)
            pDrive->ultraDmaMode = ATA_SET_UDMA_MODE_0;
        }

    /* find out transfer mode to use */

    pDrive->rwBits = configType & ATA_BITS_MASK;
    pDrive->rwPio  = configType & ATA_PIO_MASK;
/*    pDrive->rwDma  = configType & ATA_DMA_MASK;    */
    pDrive->rwMode = ATA_PIO_DEF_W;

    ATA_DEBUG_MSG (11,"ataDriveInit:\n"
                   "         pDrive->pioMode       = %d\n"
                   "         pDrive->singleDmaMode = %d\n"
                   "         pDrive->multiDmaMode  = %d\n"
                   "         pDrive->rwBits        = %d\n"
                   "         pDrive->rwPio         = %d\n",
                   pDrive->pioMode,
                   pDrive->singleDmaMode,
                   pDrive->multiDmaMode,
                   pDrive->rwBits,
                   pDrive->rwPio,
                   NULL);
    ATA_DEBUG_MSG (11,"ataDriveInit:\n"
                   "         Serial No        : %20s\n"
                   "         Model Number     : %40s\n"
                   "         Firmware revision: %8s \n",
                   pParam->serial, pParam->model, pParam->rev, 0, 0, 0);
    switch (configType & ATA_MODE_MASK)
        {
        case ATA_PIO_AUTO:
/*            pDrive->rwMode = ATA_PIO_W_0 + pDrive->pioMode;  */
/*            break;  */

        case ATA_DMA_AUTO:
            /* If it is not in "not supported" status, use the best UDMA */

            if (pDrive->ultraDmaMode != 0xff)
                pDrive->rwMode = ATA_DMA_ULTRA_0 + pDrive->ultraDmaMode;

            /*
             * If UDMA is not supported then if MDMA is not in
             * "not supported" status, use the best MDMA
             */

            else if (pDrive->multiDmaMode != 0xff)
                {
                if (pDrive->multiDmaMode)
                    pDrive->rwMode = ATA_DMA_MULTI_0 + 
                                     pDrive->multiDmaMode;
                else
                    pDrive->rwMode = ATA_PIO_W_0 + pDrive->pioMode;
                }

            /* 
             * If MDMA is not supported then if SDMA is not in
             * "not supported" status, use the best SDMA
             * retired in ATAPI-5. for downward compatibility
             */

            else if (pDrive->singleDmaMode != 0xff)
                {
                pDrive->rwMode = ((pDrive->pioMode<=ATA_SET_PIO_MODE_1)
                                  &&(pDrive->singleDmaMode==2)) ? 
                                 ATA_DMA_SINGLE_2 :
                                 ATA_PIO_W_0 + pDrive->pioMode;
                }

            /* If SDMA is not supported then use the best PIO mode */

            else
                pDrive->rwMode = ATA_PIO_W_0 + pDrive->pioMode;
            break;

        default:
            pDrive->rwMode = configType & ATA_MODE_MASK;
        }

    ataBestTransferModeFind (ctrl, drive);

    ATA_DEBUG_MSG (11, "Drive's best rwMode = %#x\n",
                   pDrive->rwMode, 0, 0, 0, 0, 0);

    /* Decide to use the DMA mode or not */
    ATA_DEBUG_MSG(5,"ataDriveInit: Using DMA mode for xfers\n", 0, 0, 0, 0, 0, 0);

    pDrive->usingDma = FALSE;


    /* Negotiate with the host for best transfer mode */
    ATA_DEBUG_MSG (14, "rwMode before negotiation with host = %#x\n",
                   pDrive->rwMode, 0, 0, 0, 0, 0);
    if (pCtrl->ataDmaModeNegotiate != NULL)
        {
        pDrive->rwMode = 
        (*pCtrl->ataDmaModeNegotiate)( pDrive->rwMode );
        }
    ATA_DEBUG_MSG (11, "rwMode after negotiation with host = %#x\n",
                   pDrive->rwMode, 0, 0, 0, 0, 0);

    /* 
     * update 'usingDma' flag depending upon the capabilities of the 
     * drive and host 
     */

    pDrive->usingDma = (pDrive->okDma &&
                        (pDrive->rwMode >= ATA_DMA_SINGLE_0)) ? TRUE : FALSE;

    ATA_DEBUG_MSG(3,"ataDriveInit %d/%d: usingDma=%d okDma=%d DMAOK=%d rwMode=%d\n",
                     ctrl, drive, pDrive->usingDma, pDrive->okDma,
                     pCtrl->ataHostDmaSupportOkay, pDrive->rwMode);

    if (!pDrive->usingDma)

        pDrive->rwMode = ATA_PIO_W_0 + pDrive->pioMode;
    /* Set the host for the rw mode */
    if (pCtrl->ataDmaModeSet != NULL)
        {
        (*pCtrl->ataDmaModeSet) ( ctrl, drive, pDrive->rwMode );
        }

    ATA_DEBUG_MSG(14,"ataDriveInit %d/%d: calling ataBestTransferModeFind\n",
                  ctrl, drive, 0, 0, 0, 0);
    ataBestTransferModeFind (ctrl, drive);

    /* Set the drive transfer mode */

    ATA_DEBUG_MSG (9,"ATA_CMD_SET_FEATURE ATA_SUB_SET_RWMODE %d\n", 
                   pDrive->rwMode, 0, 0, 0, 0, 0);
    (void) ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE, ATA_SUB_SET_RWMODE,
                   pDrive->rwMode, 0, 0, 0, 0);

    /* Disable reverting to power on defaults */

    ATA_DEBUG_MSG (9,"ATA_CMD_SET_FEATURE ATA_SUB_DISABLE_REVE\n", 
                   0, 0, 0, 0, 0, 0);
    (void) ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE, ATA_SUB_DISABLE_REVE,
                   ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO);

    /*
     * Issue spinup command if the drive is in Power-up in standby mode
     * 8.12.11, Ref-1. word 2 of Identify device.
     */

    if ((pParam->specConfig == ATA_SPEC_CONFIG_VALUE_0) | 
        (pParam->specConfig == ATA_SPEC_CONFIG_VALUE_1))
        {

        /* spin up*/

        ATA_DEBUG_MSG (9,"ATA_CMD_SET_FEATURE ATA_SUB_POW_UP_STDBY_SPIN %#x\n",
                       pParam->specConfig, 0, 0, 0, 0, 0);
        (void) ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                       ATA_SUB_POW_UP_STDBY_SPIN,ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                       ATA_ZERO, ATA_ZERO);
        }

#ifdef ATA_SMART_FEATURE

    /*
     * Enable the smart operations
     * if (Is the drive supporting the smart and not supporting the packet?)
     *     if (Is smart not enabled?)
     *         Enable smart operations.
     * Bit 0 indicates SMART feature set
     */

    if ((pParam->suppCommand1 & 0x0011) == 0x0001)
        {
        pDrive->supportSmart = TRUE;
        if ((pParam->enableCommandFeature1 & 0x0011) == 0x0000)
            {
            (void) ataCmd( ctrl, drive, ATA_CMD_SMART, ATA_SMART_ENABLE_OPER,
                           0, 0, 0, 0, 0);
            }
        }
#else /* ATA_SMART_FEATURE */

    pDrive->supportSmart = FALSE;
    if ((pParam->enableCommandFeature1) & 0x0001)
        {
        (void) ataCmd( ctrl, drive, ATA_CMD_SMART, ATA_SMART_DISABLE_OPER,
                       0, 0, 0, 0, 0);
        }

#endif /* ATA_SMART_FEATURE */

#ifdef REMOV_MEDIA_NOTIFY

    /* 
     * bit 4 of word 83 indicates support of removable media notification 
     * feature support
     */

    if (pParam->suppCommand2 & 0x0010)
        if ((pParam->enableCommandFeature2 & 0x0010) == 0)
            (void) ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                           ATA_SUB_ENABLE_NOTIFY, 0, 0, 0, 0, 0);
#else /* REMOV_MEDIA_NOTIFY */
    if (pParam->suppCommand2 & 0x0010)
        if ((pParam->enableCommandFeature2 & 0x0010) != 0)
            (void) ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                           ATA_SUB_DISABLE_NOTIFY, 0, 0, 0, 0, 0);
#endif /* REMOV_MEDIA_NOTIFY */

#ifdef ATAPI_ADV_PWR_MNGMT


        /* bit 3 of word 83 indicates the advanced managment feature set support*/

    if (pParam->suppCommand2 & 0x0008)
        if ((pParam->enableCommandFeature2 & 0x0008) == 0)
            (void) ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                           ATA_SUB_ENB_ADV_POW_MNGMNT, 0x90, 0, 0, 0, 0);
#else /* ATAPI_ADV_PWR_MNGMT */
    if (pParam->suppCommand2 & 0x0008)
        if ((pParam->enableCommandFeature2 & 0x0008) != 0)
            (void) ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                           ATA_SUB_DIS_ADV_POW_MNGMT, 0, 0, 0, 0, 0);
#endif /* ATAPI_ADV_PWR_MNGMT */


        /* Set multiple mode (multisector read/write) */

    if ((pDrive->rwPio == ATA_PIO_MULTI) &&
        (pDrive->type == ATA_TYPE_ATA))
        {
        if (pDrive->okMulti)
            {
            ATA_DEBUG_MSG (9,"ATA_CMD_SET_MULTI %d\n",
                           pDrive->multiSecs, 0, 0, 0, 0, 0);
            (void) ataCmd (ctrl, drive, ATA_CMD_SET_MULTI,
                           pDrive->multiSecs, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                           ATA_ZERO, ATA_ZERO);
            }
        else
            pDrive->rwPio = ATA_PIO_SINGLE;
        }

    pDrive->state = ATA_DEV_OK;

    driveInitExit:

    if (!coreIsDumping)
	semGive (&pCtrl->muteSem);

    if (pDrive->state != ATA_DEV_OK)
        return(ERROR);
         
    ATA_DEBUG_MSG (14,"ataDriveInit %d/%d exited\n", ctrl, drive, 0, 0, 0, 0);
    return(OK);
    } /* ataDriveInit */


/**************************************************************************
*
* ataBlkDevFill() - fills block device structure 
*
* This routine fills a block device structure with real parameters
* corresponding to the given device, or with stubs if a device is not
* detected by previous ataDriveInit () call.  The routine considers
* offset and size requested for the device in ataDevCreate () call.
*
* RETURNS: N/A
*
* SEE ALSO: ataDevCreate (), ataDriveInit ()
*/

LOCAL void ataBlkDevFill
    (
    ATA_DEV *   pDev
    )
    {
    ATA_CTRL   * pCtrl     = &ataCtrl[pDev->ctrl];
    ATA_DRIVE  * pDrive    = &pCtrl->drive[pDev->drive];
    ATA_PARAM  * pParam    = &pDrive->param;
    ATAPI_TYPE   * pType     = pDrive->driveInfo;
    BLK_DEV    * pBlkDev   = &pDev->blkDev;
    UINT32       nBlocks   = pDev->nBlocks;
    UINT32       maxBlks;

    ATA_DEBUG_MSG (1,"ataBlkDevFill %d/%d: entered, pDev=0x%x\n",
                   pDev->ctrl, pDev->drive, (int)pDev, 0, 0, 0);
    if ((pDrive->state == ATA_DEV_OK) || (pDrive->state == ATA_DEV_MED_CH))
        {
        if (pDrive->type == ATA_TYPE_ATA)
            {
            ATA_DEBUG_MSG(1,"ataBlkDevFill %d/%d: TYPE_ATA\n",
                             pDev->ctrl, pDev->drive, 0, 0, 0, 0);

            /* 
             * if LBA is supported and drive capacity is not zero
             * and drive capacity is greater than the product of
             * CHS, then we should use the LBA value.
             */
            if ((pDrive->okLba != 0)      &&
                (pDrive->capacity != 0)   &&
                (pDrive->capacity > (sector_t)(pType->cylinders * pType->heads * pType->sectors)))
                {
                maxBlks = (UINT32)(pDrive->capacity - pDev->blkOffset);
                ATA_DEBUG_MSG(1,"ataBlkDevFill: ATA %d/%d using LBA cyl=%d, heads=%d, secs=%d\n",
                              pDev->ctrl, pDev->drive,
                              pType->cylinders, pType->heads, pType->sectors, 0);
                }

            else /* just use CHS */
                {
                maxBlks = ((pType->cylinders * pType->heads * pType->sectors) 
                           - pDev->blkOffset);
                ATA_DEBUG_MSG(1,"ATA %d/%d using CHS cyl=%d, heads=%d, secs=%d\n",
                              pDev->ctrl, pDev->drive, 
                              pType->cylinders, pType->heads, pType->sectors, 0);     
                }

            if (nBlocks == 0)
                nBlocks = maxBlks;

            if (nBlocks > maxBlks)
                nBlocks = maxBlks;

            pBlkDev->bd_nBlocks         = nBlocks;
            pBlkDev->bd_bytesPerBlk     = pType->bytes;
            pBlkDev->bd_blksPerTrack    = pType->sectors;
            pBlkDev->bd_nHeads          = pType->heads;
            pBlkDev->bd_removable       = TRUE;
            pBlkDev->bd_retry           = 1;
            pBlkDev->bd_mode            = O_RDWR;
            pBlkDev->bd_readyChanged    = TRUE;
            pBlkDev->bd_blkRd           = ataBlkRd;
            pBlkDev->bd_blkWrt          = ataBlkWrt;
            pBlkDev->bd_ioctl           = ataBlkIoctl;
            pBlkDev->bd_reset           = ataReset;
            pBlkDev->bd_statusChk       = ataStatus;
            }
        else if (pDrive->type == ATA_TYPE_ATAPI)
            {
            ATA_DEBUG_MSG(1,"ataBlkDevFill %d/%d: TYPE_ATAPI\n", 
                          pDev->ctrl, pDev->drive, 0, 0, 0, 0);            
            /* 
             * number of blocks is set to zero, and bytes per block
             * is set to a defaults for a CDROM.
             * However these elements are updated in 
             * atapiStatusChk(), which is called before any operation
             * when media is changed.
             */

            pBlkDev->bd_nBlocks       = 0 ;  
            pBlkDev->bd_bytesPerBlk   = ATAPI_CDROM_BYTE_PER_BLK;  
            pBlkDev->bd_blksPerTrack  = ATAPI_BLOCKS;
            pBlkDev->bd_nHeads        = 1;

            if (pParam->config & CONFIG_REMOVABLE)
                pBlkDev->bd_removable = TRUE;
            else
                pBlkDev->bd_removable = FALSE;

            pBlkDev->bd_retry         = 1;
            pBlkDev->bd_mode          = O_RDONLY;
            pBlkDev->bd_readyChanged  = TRUE;
            pBlkDev->bd_blkRd         = atapiBlkRd;
            pBlkDev->bd_blkWrt        = atapiBlkWrt;
            pBlkDev->bd_ioctl         = atapiBlkIoctl;
            pBlkDev->bd_reset         = atapiReset;
            pBlkDev->bd_statusChk     = atapiStatusChk;
            atapiStatusChk(pDev);       /* test device for media installed */
                                        /* modify geometry */
            }
        else
            {
            /* Set default values */

            ATA_DEBUG_MSG(1,"ataBlkDevFill %d/%d: default values\n", 
                         pDev->ctrl, pDev->drive, 0, 0, 0, 0);

            pBlkDev->bd_nBlocks      = 0;
            pBlkDev->bd_bytesPerBlk  = ATA_BYTES_PER_BLOC;
            pBlkDev->bd_blksPerTrack = 0;
            pBlkDev->bd_nHeads       = 0;
            pBlkDev->bd_removable    = TRUE;
            pBlkDev->bd_retry        = 1;
            pBlkDev->bd_mode         = O_RDWR;
            pBlkDev->bd_readyChanged = TRUE;
            pBlkDev->bd_blkRd        = ataStub;
            pBlkDev->bd_blkWrt       = ataStub;
            pBlkDev->bd_ioctl        = ataStub;
            pBlkDev->bd_reset        = ataReset;
            pBlkDev->bd_statusChk    = ataStub;
            }

        ATA_DEBUG_MSG (9,"\n     pBlkDev->bd_nBlocks      = %#d\n"
                       "     pBlkDev->bd_bytesPerBlk  = %#d\n"
                       "     pBlkDev->bd_blksPerTrack = %#d\n"
                       "     pBlkDev->bd_nHeads       = %#d\n",
                       pBlkDev->bd_nBlocks,
                       pBlkDev->bd_bytesPerBlk,
                       pBlkDev->bd_blksPerTrack,
                       pBlkDev->bd_nHeads, 0, 0);
        }
    else
        ATA_DEBUG_MSG(2,"ataBlkDevFill %d/%d: bad device state\n",
                         pDev->ctrl, pDev->drive, 0, 0, 0, 0);

    return;
    } /* ataBlkDevFill */

/**************************************************************************
*
*
* This routine reads one or more blocks from the specified device,
* starting with the specified block number.
*
* If any block offset was specified during ataDevCreate(), it is added
* to <startBlk> before the transfer takes place.
*
* RETURNS: OK, ERROR if the read command didn't succeed.
*/

LOCAL STATUS ataBlkRd
    (
    ATA_DEV * pDev,
    UINT32       startBlk,
    UINT32       nBlks,
    char    * pBuf
    )
    {
    ATA_DEBUG_MSG (16,"ataBlkRd %d/%d: entered\n", pDev->ctrl, pDev->drive, 0, 0, 0, 0);
    return(ataBlkRW (pDev, startBlk, nBlks, pBuf, O_RDONLY));
    } /* ataBlkRd */

/**************************************************************************
*
* ataBlkWrt - write one or more blocks to a ATA/IDE disk
*
* This routine writes one or more blocks to the specified device,
* starting with the specified block number.
*
* If any block offset was specified during ataDevCreate(), it is added
* to <startBlk> before the transfer takes place.
*
* RETURNS: OK, ERROR if the write command didn't succeed.
*/

LOCAL STATUS ataBlkWrt
    (
    ATA_DEV *pDev,
    UINT32   startBlk,
    UINT32   nBlks,
    char    *pBuf
    )
    {
    ATA_DEBUG_MSG (16,"ataBlkWrt %d/%d: entered\n", pDev->ctrl, pDev->drive, 0, 0, 0, 0);
    return(ataBlkRW (pDev, startBlk, nBlks, pBuf, O_WRONLY));
    } /* ataBlkWrt */


/**************************************************************************
*
* ataBlkIoctl - This routine perform the driver specific request. 
*
* RESULT: OK, ERROR and errno must be set
*/

LOCAL STATUS ataBlkIoctl
    (
    ATA_DEV *pDev,
    int      function,
    int      arg
    )
    {
    ATA_CTRL  * pCtrl = &ataCtrl[pDev->ctrl];
    ATA_DRIVE * pDrive   = &pCtrl->drive[pDev->drive];
    int status = ERROR;

    if ((function & 0xffff0000) == 0xcb100000)
        {
        /* ignore CBIO ioctl's */
        return(OK);
        }
    
    ATA_DEBUG_MSG (14,"ataBlkIoctl: %d/%d ioctl = 0x%x\n", pDev->ctrl, 
                       pDev->drive, function, 0, 0, 0);

    if ((!pCtrl->installed) || (pDrive->state != ATA_DEV_OK))
        {
        printErr("ataBlkIoctl: Device state NOT OK, ioctl ignored\n");
        return(ERROR);
        }

    semTake (&pCtrl->muteSem, WAIT_FOREVER);

    switch (function)
        {
        case FIODISKFORMAT:
            (void) errnoSet (S_ioLib_UNKNOWN_REQUEST);
            break;

        case FIOSYNC:
            (void)ataCmd (pDev->ctrl, pDev->drive, ATA_CMD_FLUSH_CACHE, 0, 0, 0, 0, 0, 0);
            flushCacheCount++;     /* was part of statements immed below */
            status = OK;
            break;

        default:
            status = atapiIoctl (function, pDev->ctrl, pDev->drive,
                                 ATA_ZERO, arg, ATA_ZERO, ATA_ZERO);
        }

    semGive (&pCtrl->muteSem);

    return(status);
    } /* ataBlkIoctl */

/**************************************************************************
*
* ataStub - block device stub
*
* This routine services as stub for a block device structure's routine
* pointers.
*
* RETURNS: ERROR always.
*/

LOCAL STATUS ataStub (void)
    {
    return(ERROR);
    } /* ataStub */


/***************************************************************************
*
* ataBlkRW - read or write sectors to a ATA/IDE disk.
*
* Read or write sectors to a ATA/IDE disk. <startBlk> is the start Block, 
* <nBlks> is the number of blocks, <pBuf> is data buffer pointer and 
* <direction>  is the direction either to read or write. It should be O_WRONLY
* for data write to drive or O_RDONLY for read data from drive.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/
STATUS ataBlkRW
    (
    ATA_DEV *pDev,
    sector_t startBlk,
    UINT32 nBlks,
    char *pBuf,
    int  direction
    )
    {
    ATA_CTRL  * pCtrl    = &ataCtrl[pDev->ctrl];
    ATA_DRIVE * pDrive   = &pCtrl->drive[pDev->drive];
    BLK_DEV   * pBlkDev  = &pDev->blkDev;
    ATAPI_TYPE  * pType  = pDrive->driveInfo;
    STATUS      status   = ERROR;

    UINT32      retryRW0  = 0;
    UINT32      retryRW1  = 0;
    UINT32      retrySeek = 0;
    UINT32      cylinder;
    UINT32      head;
    UINT32      sector;
    UINT32      nSecs;
    int           ix;

    /* sanity check */

    if ((!pCtrl->installed) || (pDrive->state != ATA_DEV_OK) || (pBuf == NULL))
        {
        ATA_DEBUG_MSG (1, "ataBlkRW: ERROR - installed=%d state=%d buf=0x%x\n",
             pCtrl->installed, pDrive->state, pBuf, 0, 0, 0);
        return(ERROR);
        }

    nSecs = pBlkDev->bd_nBlocks;

    if ((startBlk + nBlks) > nSecs)
        {
        ATA_DEBUG_MSG (5, "ataBlkRW: startBlk=%d nBlks=%d: 0 - %d\n", startBlk, nBlks,
                       nSecs, 0, 0, 0);

        return(ERROR);
        }

    startBlk += pDev->blkOffset;

    semTake (&pCtrl->muteSem, WAIT_FOREVER);

    for (ix = 0; ix < nBlks; ix += nSecs)
        {
        if (pDrive->okLba != 0)
            {
            head     = (startBlk & ATA_LBA_HEAD_MASK) >> 24;
            cylinder = (startBlk & ATA_LBA_CYL_MASK) >> 8;
            sector   = (startBlk & ATA_LBA_SECTOR_MASK);
            }
        else
            {
            cylinder = startBlk / (pType->sectors * pType->heads);
            sector   = startBlk % (pType->sectors * pType->heads);
            head     = sector / pType->sectors;
            sector   = sector % pType->sectors + 1;
            }
        nSecs    = min (nBlks - ix, ATA_MAX_RW_SECTORS);

        ATA_DEBUG_MSG (14,"cyl = %#x\n"
                          "                    head = %#x\n"
                          "                    sec = %#x\n",
                          cylinder, head, sector, 0, 0, 0);

        retryRW1 = 0;
        retryRW0 = 0;

        if ((pDrive->usingDma == TRUE) && (pCtrl->ataHostDmaSupportOkay == TRUE))
            {
            ATA_DEBUG_MSG(5,"ataBlkRW: calling ataDmaRW\n", 0, 0, 0, 0, 0, 0);
            if (ataDmaRW (pDev->ctrl, pDev->drive, cylinder, head,
                          sector,pBuf, nSecs, direction, startBlk) != OK)
                {
		pDrive->usingDma = FALSE;
		goto dmafail;  /* dma isn't working, use PIO  */
                }
            }
        else
            {
dmafail:
            while (ataRW (pDev->ctrl, pDev->drive, cylinder, head,
                          sector,pBuf, nSecs, direction, startBlk) != OK)
                {
                if (++retryRW0 > ataRetry)
                    {
		    ATA_DEBUG_MSG(4,"ataBlkRW retrying %d/%d\n", pDev->ctrl, pDev->drive,
                                    0, 0, 0, 0);
#if 0
                    (void)ataCmd (pDev->ctrl, pDev->drive, ATA_CMD_RECALIB,
                                  0, 0, 0, 0, 0, 0);
#endif

                    if (++retryRW1 > ataRetry)
                        goto done;

                    retrySeek = 0;

                    while (ataCmd (pDev->ctrl, pDev->drive, ATA_CMD_SEEK,
                                   cylinder, head, 0, 0, 0, 0) != OK)

                        if (++retrySeek > ataRetry)
                            goto done;

                    retryRW0 = 0;
                    }
                }
            }

        startBlk += nSecs;
        pBuf += pBlkDev->bd_bytesPerBlk * nSecs;
        }

    status = OK;

done:

    if (status == ERROR)
        (void)errnoSet (S_ioLib_DEVICE_ERROR);

    semGive (&pCtrl->muteSem);

    return(status);
    } /* ataBlkRW */

/**************************************************************************
*
* ataReset - reset a ATA/IDE disk controller
*
* This routine resets a ATA/IDE disk controller.
*
* RETURNS: OK, always.
*/

LOCAL STATUS ataReset
    (
    ATA_DEV * pDev
    )
    {
    ATA_CTRL  *  pCtrl;
    ATA_DRIVE *  pDrive;

    ATA_DEBUG_MSG (9,"ataReset: entered\n", 0, 0, 0, 0, 0, 0);

    if (pDev == NULL)
	return(ERROR);

    pCtrl = &ataCtrl[pDev->ctrl];
    pDrive = &pCtrl->drive[pDev->drive];

    if ((pCtrl == NULL) || (pDrive == NULL))
	return(ERROR);

    if ((!pCtrl->installed) || (pDrive->Reset == NULL))
        return(ERROR);

    if (pDrive->state != ATA_DEV_MED_CH)
        {

        /* power on, entire drive changed, or failure recovering */

	if (!coreIsDumping)
            semTake (&pCtrl->muteSem, WAIT_FOREVER);

        if (pDrive->state != ATA_DEV_INIT)
            {

            /* higher level tried to recover after failure */

            if (pDrive->Reset (pDev->ctrl, pDev->drive) != OK)
                {
		if (!coreIsDumping)
                    semGive (&pCtrl->muteSem);
                return(ERROR);
                }
            }

        if (ataDriveInit (pDev->ctrl, pDev->drive) != OK)
            {
	    if (!coreIsDumping)
		semGive (&pCtrl->muteSem);
            return(ERROR);
            }

        ataBlkDevFill (pDev);

	if (!coreIsDumping)
	    semGive	(&pCtrl->muteSem);
        }

    /* Identify medium */

    pDrive->state = ATA_DEV_OK;

    return(OK);
    } /* ataReset */

/**************************************************************************
*
* ataStatus - check status of a ATA/IDE disk controller
*
* This routine check status of a ATA/IDE disk controller.
*
* RETURNS: OK, ERROR .
*/

LOCAL STATUS ataStatus
    (
    ATA_DEV * pDev
    )
    {
    ATA_CTRL  * pCtrl    = &ataCtrl[pDev->ctrl];
    ATA_DRIVE * pDrive   = &pCtrl->drive[pDev->drive];

    if ((!pCtrl->installed) || (pDrive->state != ATA_DEV_OK))
        {
        ATA_DEBUG_MSG (2,"ataStatus %d/%d NOT OKAY state=%d installed=%d\n", 
                  pDev->ctrl, pDev->drive, pDrive->state, pCtrl->installed, 0, 0);
        return(ERROR);
        }

    ATA_DEBUG_MSG (2,"ataStatus %d/%d okay\n", pDev->ctrl, pDev->drive, 0, 0, 0, 0);
    return(OK);
    } /* ataStatus */


/**************************************************************************
*
* atapiBlkWrt - write one or more blocks to an ATAPI Device.
*
* This routine writes one or more blocks to the specified device,
* starting with the specified block number.
*
* RETURNS: OK, ERROR if the write command didn't succeed.
*/

LOCAL STATUS atapiBlkWrt
    (
    ATA_DEV   * pAtapiDev,
    UINT32      startBlk,
    UINT32      nBlks,
    char      * pBuf
    )
    {
    ATA_CTRL  * pCtrl  = &ataCtrl[pAtapiDev->ctrl];
    ATA_DRIVE * pDrive = &pCtrl->drive[pAtapiDev->drive];
    ATAPI_CMD   atapiCmd;
    UINT32      i;
    char      * pUpdatedBuf = pBuf;

    ATA_DEBUG_MSG (5,"atapiBlkWrt%d/%d: state=%#x startBlk=%#x"
                   " nBlks=%#x pBuf=%#x\n",
                   pAtapiDev->ctrl, pAtapiDev->drive, pDrive->state,
                   startBlk, nBlks, pBuf);

    if ((!pCtrl->installed) || (pDrive->state != ATA_DEV_OK) || (pBuf == NULL))
        return(ERROR);

    /* Sanity check */

    if ((startBlk >= pAtapiDev->blkDev.bd_nBlocks) ||
        ((startBlk + nBlks) >= pAtapiDev->blkDev.bd_nBlocks))
        {
        ATA_DEBUG_MSG (2, "atapiBlkWrt%d/%d ERROR: block numbers must be in "
                       "range 0 - %d\n", pAtapiDev->ctrl, pAtapiDev->drive,
                       (pAtapiDev->blkDev.bd_nBlocks - 1), 0, 0, 0);
        return(ERROR);
        }

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    /* Fill Command Packet */

    atapiCmd.cmdPkt[0] = ATAPI_CMD_WRITE_12;


    /* write the start block number to 2-6 bytes of command packet */

    for (i = 2; i < 6; i++)
        atapiCmd.cmdPkt[i] += (UINT8)( startBlk >> (8 * (5 - i)));

    /* write number of blocks to 6 to 10 bytes of command packet */

    for (i = 6; i < 10; i++)
        atapiCmd.cmdPkt[i] += (UINT8)( nBlks >> (8 * (9 - i))); 

    /* number of bytes needs to be transferred */

    i = nBlks * pAtapiDev->blkDev.bd_bytesPerBlk;

    atapiCmd.ppBuf               = &pUpdatedBuf;
    atapiCmd.bufLength           = i;
    atapiCmd.direction           = OUT_DATA;
    atapiCmd.overlap             = FALSE;
    atapiCmd.desiredTransferSize = i;
    if ((pDrive->usingDma == TRUE) && (pCtrl->ataHostDmaSupportOkay == TRUE))
        {
        ATA_DEBUG_MSG (4,"dma true blkWrt \n", 0, 0, 0, 0, 0, 0);
        atapiCmd.dma             = TRUE;
        }
    else
        atapiCmd.dma             = FALSE;

    /* Execute Packet Command */

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    ATA_DEBUG_MSG (5, "atapiBlkWrt%d/%d: Ok\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    } /* atapiBlkWrt */

/**************************************************************************
*
* atapiBlkRd - read one or more blocks from an ATAPI Device.
*
* This routine reads one or more blocks from the specified device,
* starting with the specified block number.
*
* RETURNS: OK, ERROR if the read command didn't succeed.
*/

LOCAL STATUS atapiBlkRd
    (
    ATA_DEV   * pAtapiDev,
    UINT32      startBlk,
    UINT32      nBlks,
    char      * pBuf
    )
    {
    ATA_CTRL  * pCtrl   = &ataCtrl[pAtapiDev->ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[pAtapiDev->drive];
    ATAPI_CMD   atapiCmd;
    UINT32      i;
    char      * pUpdatedBuf = pBuf;

    ATA_DEBUG_MSG (5, "atapiBlkRd%d/%d: state=%#x startBlk=%#x"
                   " nBlks=%#x pBuf=%#x\n",
                   pAtapiDev->ctrl, pAtapiDev->drive, pDrive->state,
                   startBlk, nBlks, pBuf);

    if ((!pCtrl->installed) || (pDrive->state != ATA_DEV_OK) || (pBuf == NULL))
        return(ERROR);

    /* Sanity check */

    if ( (startBlk >= pAtapiDev->blkDev.bd_nBlocks) ||
         ((startBlk + nBlks) >= pAtapiDev->blkDev.bd_nBlocks))
        {
        ATA_DEBUG_MSG (2, "atapiBlkRd%d/%d ERROR: block numbers must be in "
                       "range 0 - %#d\n", pAtapiDev->ctrl, pAtapiDev->drive,
                       (pAtapiDev->blkDev.bd_nBlocks - 1), 0, 0, 0);
        return(ERROR);
        }

    /* Fill Command Packet */
    /* use READ12 packet command to read data */

    atapiCmd.cmdPkt[0] = ATAPI_CMD_READ12;
    atapiCmd.cmdPkt[1] = 0;
    atapiCmd.cmdPkt[2] = (UINT8)( startBlk >> 24);
    atapiCmd.cmdPkt[3] = (UINT8)( startBlk >> 16);
    atapiCmd.cmdPkt[4] = (UINT8)( startBlk >> 8);
    atapiCmd.cmdPkt[5] = (UINT8)( startBlk );
    atapiCmd.cmdPkt[6] = (UINT8)( nBlks >> 24);
    atapiCmd.cmdPkt[7] = (UINT8)( nBlks >> 16);
    atapiCmd.cmdPkt[8] = (UINT8)( nBlks >> 8);
    atapiCmd.cmdPkt[9] = (UINT8)( nBlks );
    atapiCmd.cmdPkt[10] = 0;
    atapiCmd.cmdPkt[11] = 0;

    i = nBlks * pAtapiDev->blkDev.bd_bytesPerBlk;

    ATA_DEBUG_MSG (4,"atapiBlkRd: i =%#x Bytes/block %#x \n", i,
                   pAtapiDev->blkDev.bd_bytesPerBlk , 0, 0, 0, 0);

    atapiCmd.ppBuf           = &pUpdatedBuf;
    atapiCmd.bufLength       = i;
    atapiCmd.direction       = IN_DATA;
    atapiCmd.desiredTransferSize = i;
    atapiCmd.overlap         = FALSE;
    if ((pDrive->usingDma == TRUE) && (pCtrl->ataHostDmaSupportOkay == TRUE))
        {
        ATA_DEBUG_MSG (4,"atapiBlkRd: dma true\n", 0, 0, 0, 0, 0, 0);
        atapiCmd.dma             = TRUE;
        }
    else
        {
        ATA_DEBUG_MSG (4,"atapiBlkRd: dma false\n", 0, 0, 0, 0, 0, 0);
        atapiCmd.dma             = FALSE;
        }

    /* Execute Packet Command */

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    ATA_DEBUG_MSG (5, "atapiBlkRd%d/%d: Ok\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    } /* atapiBlkRd */

/*************************************************************************
*
* atapiReset - reset a ATAPI drive
*
* This routine resets a ATAPI drive
*
* RETURNS: OK, or ERROR.
*/

LOCAL STATUS atapiReset
    (
    ATA_DEV * pAtapiDev
    )
    {
    ATA_CTRL  * pCtrl   = &ataCtrl[pAtapiDev->ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[pAtapiDev->drive];

    ATA_DEBUG_MSG (5, "atapiReset: %d/%d: state=%#x\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, pDrive->state, 0, 0, 0);

    if (!pCtrl->installed)
        return(ERROR);

    /* check if media changed */

    if (pDrive->state != ATA_DEV_MED_CH)
        {
        /* power on, entire drive changed, or failure recovering */
        semTake (&pCtrl->muteSem, WAIT_FOREVER);

        if (pDrive->state != ATA_DEV_INIT)
            {    /* higher level tried to recover after failure */

            if (pDrive->Reset (pAtapiDev->ctrl, pAtapiDev->drive) != OK)
                {
                semGive (&pCtrl->muteSem);
                return(ERROR);
                }
            }

        if (ataDriveInit (pAtapiDev->ctrl, pAtapiDev->drive) != OK)
            {
            semGive (&pCtrl->muteSem);
            return(ERROR);
            }

        ataBlkDevFill (pAtapiDev);

        semGive (&pCtrl->muteSem);
        }

    /* Identify medium */

    if (atapiTestUnitRdy (pAtapiDev) != OK)
        return(ERROR);

    if (atapiReadCapacity (pAtapiDev) != OK)
        {
        pAtapiDev->blkDev.bd_readyChanged = TRUE;
        return(ERROR);
        }
    else
        pDrive->state = ATA_DEV_OK;

    ATA_DEBUG_MSG (5, "atapiReset: %d/%d: Ok\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    } /* atapiReset */

/**************************************************************************
*
* atapiStatusChk - check device status
*
* This routine issues a TEST UNIT READY command to a ATAPI device to detect
* a medium change. It is called by filesystems before doing open()'s or
* creat()'s.
*
* RETURNS: OK or ERROR.
*/

ATA_LOCAL STATUS atapiStatusChk
    (
    ATA_DEV   * pAtapiDev  /* pointer to device descriptor */
    )
    {
    ATA_CTRL  * pCtrl   = &ataCtrl[pAtapiDev->ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[pAtapiDev->drive];

    ATA_DEBUG_MSG (2, "atapiStatusChk: %d/%d: state=%#d readyChanged=%#d\n",
                   pAtapiDev->ctrl, pAtapiDev->drive, pDrive->state,
                   pAtapiDev->blkDev.bd_readyChanged, 0, 0);

    if (!pCtrl->installed)
        return(ERROR);

    if ( (pDrive->state != ATA_DEV_MED_CH) && (pDrive->state != ATA_DEV_OK)  )
        return(ERROR);

    if (atapiTestUnitRdy (pAtapiDev) != OK)
        return(ERROR);

    /* Device OK, medium may be changed, Check Medium Change */

    if (pDrive->state == ATA_DEV_MED_CH || pAtapiDev->blkDev.bd_readyChanged)
        {
        if (atapiReadCapacity (pAtapiDev) != OK)
            {
            pAtapiDev->blkDev.bd_readyChanged = TRUE;
            return(ERROR);
            }
        else
            pDrive->state = ATA_DEV_OK;
        }

    ATA_DEBUG_MSG (2,"atapiStatusChk%d/%d: Ok: state=%#d readyChanged=%#d\n\n",
                   pAtapiDev->ctrl, pAtapiDev->drive, pDrive->state,
                   pAtapiDev->blkDev.bd_readyChanged, 0, 0);

    return(OK); /* open or create operations can continue */

    } /* atapiStatusChk */

/**************************************************************************
*
* atapiPktCmdSend - Issue a Packet command.
*
* This function issues a packet command to specified drive. 
*
* See library file description for more details.
*
* RETURN: SENSE_NO_SENSE if success, or ERROR if not successful for any reason
*
* ERRNO:  S_ioLib_DEVICE_ERROR
*/

UINT8  atapiPktCmdSend
    (
    ATA_DEV   * pAtapiDev,
    ATAPI_CMD * pComPack
    )
    {
    ATA_CTRL  * pCtrl   = &ataCtrl[pAtapiDev->ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[pAtapiDev->drive]; 
    UINT8 temp;

    if ((pDrive->usingDma == TRUE) && (pCtrl->ataHostDmaSupportOkay == TRUE))
        pComPack->dma = TRUE;
    else
        pComPack->dma = FALSE;

#ifdef ATAPI_OVERLAPPED_FEATURE
    if (pDrive->okOverlap  == TRUE)
        pComPack->overlap  = TRUE;
#endif /* ATAPI_OVERLAPPED_FEATURE */   

    /* Call the packet command */

    temp = atapiPktCmd (pAtapiDev, pComPack);

    return(temp);
    }

/**************************************************************************
*
* atapiIoctl - Control the drive.
*
* This routine is used to control the drive like setting the password, putting
* in power save mode, locking/unlocking the drive, ejecting the medium etc. The 
* argument <function> defines the ioctl command, <password>, and integer array
* is the password required or set password value for some commands. Arguments 
* <arg0>, pointer <arg1>, pointer to pointer buffer <ppBuf> are commad 
* specific.
*
* The following commands are supported for various functionality.
*
* \is
*
* \i IOCTL_DIS_MASTER_PWD         
* Disable the master password. where 4th parameter is the master password.
* \i IOCTL_DIS_USER_PWD           
* Disable the user password.
* \i IOCTL_ERASE_PREPARE   
* Prepare the drive for erase incase the user  password lost, and it is in
* max security mode.
* \i IOCTL_ENH_ERASE_UNIT_USR  
* Erase in enhanced mode supplying the user password.
* \i IOCTL_ENH_ERASE_UNIT_MSTR   
* Erase in enhanced mode supplying the master password.
* \i IOCTL_NORMAL_ERASE_UNIT_MSTR 
* Erase the drive in normal mode supplying the master password.
* \i IOCTL_NORMAL_ERASE_UNIT_USR
* Erase the drive in normal mode supplying the user password.
* \i IOCTL_FREEZE_LOCK         
* Freeze lock the drive.
* \i IOCTL_SET_PASS_MSTR 
* Set the master password.
* \i IOCTL_SET_PASS_USR_MAX       
* Set the user password in Maximum security mode.
* \i IOCTL_SET_PASS_USR_HIGH      
* Set the user password in High security mode.
* \i IOCTL_UNLOCK_MSTR            
* Unlock the master password.
* \i IOCTL_UNLOCK_USR             
* Unlock the user password.
* \i IOCTL_CHECK_POWER_MODE       
* Find the drive power saving mode.
* \i IOCTL_IDLE_IMMEDIATE         
* Idle the drive immediatly. this will get the drive from the standby or
* active mode to idle mode immediatly.
* \i IOCTL_SLEEP     
* Set the drive in sleep mode. this is the highest power saving mode. to
* return to the normal active or IDLE mode, drive need an hardware reset or
* power on reset or device reset command.
* \i IOCTL_STANDBY_IMMEDIATE    
* Standby the drive immediatly.
* \i IOCTL_EJECT_DISK          
* Eject the media of an ATA drive. Use IOsystem ioctl function for ATAPI drive.
* \i IOCTL_GET_MEDIA_STATUS 
* Find the media status.
* \i IOCTL_ENA_REMOVE_NOTIFY  
* Enable the drive's removable media notification feature set.
* \ie
*
* The following table describes these arguments validity. These are tabulated in
* the following form
*
* \bs
* ----------------------------------------------------------------------
* FUNCTION                       
* password [16]         arg0            *arg1               **ppBuf
* ----------------------------------------------------------------------
*
* IOCTL_DIS_MASTER_PWD
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                             
* IOCTL_DIS_USER_PWD                        
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                                                
* IOCTL_ERASE_PREPARE                   
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_ENH_ERASE_UNIT_USR         
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                      
* IOCTL_ENH_ERASE_UNIT_MSTR        
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                      
* IOCTL_NORMAL_ERASE_UNIT_MSTR     
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                       
* IOCTL_NORMAL_ERASE_UNIT_USR      
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                       
* IOCTL_FREEZE_LOCK      
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_SET_PASS_MSTR              
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                      
* IOCTL_SET_PASS_USR_MAX         
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                       
* IOCTL_SET_PASS_USR_HIGH          
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_UNLOCK_MSTR                
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_UNLOCK_USR                 
* password            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_READ_NATIVE_MAX_ADDRESS    - it returns address in <arg1>
* ATA_ZERO         (ATA_SDH_IBM or    LBA/CHS add           ATA_ZERO
*                   ATA_SDH_LBA )   ( LBA 27:24 / Head       
*                                     LBA 23:16 / cylHi      
*                                     LBA 15:8  / cylLow     
*                                     LBA 7:0   / sector no )
*                               
* IOCTL_SET_MAX_ADDRESS           - <arg1> is pointer to LBA address
* ATA_ZERO    SET_MAX_VOLATILE or     LBA address           ATA_ZERO
*             SET_MAX_NON_VOLATILE                       
*                                            
* IOCTL_SET_MAX_SET_PASS           
* password            ATA_ZERO         ATA_ZERO             ATA_ZERO
*
* IOCTL_SET_MAX_LOCK               
* ATA_ZERO            ATA_ZERO         ATA_ZERO             ATA_ZERO
*
* IOCTL_SET_MAX_UNLOCK             
* ATA_ZERO            ATA_ZERO         ATA_ZERO             ATA_ZERO
* 
* IOCTL_SET_MAX_FREEZE_LOCK        
* ATA_ZERO            ATA_ZERO         ATA_ZERO             ATA_ZERO
*
* IOCTL_CHECK_POWER_MODE           - returns power mode in <arg1>
* ATA_ZERO            ATA_ZERO         returns power        ATA_ZERO
*                                      mode
*          power modes  :-1)    0x00  Device in standby mode
*                         2)    0x80  Device in Idle mode
*                         3)    0xff  Device in Active or Idle mode                                    
*
* IOCTL_IDLE_IMMEDIATE          
* ATA_ZERO            ATA_ZERO         ATA_ZERO             ATA_ZERO
*
* IOCTL_SLEEP                     
* ATA_ZERO            ATA_ZERO         ATA_ZERO             ATA_ZERO
* 
* IOCTL_STANDBY_IMMEDIATE         
* ATA_ZERO            ATA_ZERO         ATA_ZERO             ATA_ZERO
* 
* IOCTL_ENB_POW_UP_STDBY          
* ATA_ZERO            ATA_ZERO         ATA_ZERO             ATA_ZERO
*
* IOCTL_ENB_SET_ADV_POW_MNGMNT    
* ATA_ZERO                arg0         ATA_ZERO             ATA_ZERO
*                          
*  NOTE:- arg0 value - 1). for minimum power consumption with standby 0x01h
*                      2). for minimum power consumption without standby 0x01h
*                      3). for maximum performance  0xFEh
*
* IOCTL_DISABLE_ADV_POW_MNGMNT    
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                                           
* IOCTL_EJECT_DISK                                          
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                                           
* IOCTL_LOAD_DISK                                           
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                                           
* IOCTL_MEDIA_LOCK                                          
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*                                                           
* IOCTL_MEDIA_UNLOCK                                        
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_GET_MEDIA_STATUS      - returns status in <arg1>    
* ATA_ZERO            ATA_ZERO            status            ATA_ZERO
* 
*   NOTE: value in <arg1> is 
*           0x04    -Command aborted
*           0x02    -No media in drive
*           0x08    -Media change is requested
*           0x20    -Media changed                      
*           0x40    -Write Protected
*                
* IOCTL_ENA_REMOVE_NOTIFY       
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_DISABLE_REMOVE_NOTIFY   
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_SMART_DISABLE_OPER  
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
* 
* IOCTL_SMART_ENABLE_ATTRIB_AUTO    
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
* 
* IOCTL_SMART_DISABLE_ATTRIB_AUTO   
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
*
* IOCTL_SMART_ENABLE_OPER         
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
* 
* IOCTL_SMART_OFFLINE_IMMED       
* ATA_ZERO          SubCommand          ATA_ZERO            ATA_ZERO
*       (refer to ref1 page no 190)
*
* IOCTL_SMART_READ_DATA     - returns pointer to pointer <ppBuf> of read data                   
* ATA_ZERO            ATA_ZERO          ATA_ZERO            read data
*                                                       
* IOCTL_SMART_READ_LOG_SECTOR  - returns pointer to pointer <ppBuf>of read data
* ATA_ZERO          no of sector to        log Address      read data                                       
*                   be read                     
* 
* IOCTL_SMART_RETURN_STATUS         
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO    
*
* IOCTL_SMART_SAVE_ATTRIB       
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO
* 
* IOCTL_SMART_WRITE_LOG_SECTOR 
* ATA_ZERO          no of to be written     Log Sector address  write data
* 
*       NOTE: - <ppBuf> contains pointer to pointer data buffer to be written
*
* IOCTL_CFA_ERASE_SECTORS        
* ATA_ZERO          sector count        PackedCHS/LBA       ATA_ZERO
*
* IOCTL_CFA_REQUEST_EXTENDED_ERROR_CODE     
* ATA_ZERO            ATA_ZERO          ATA_ZERO            ATA_ZERO 
*                
* IOCTL_CFA_TRANSLATE_SECTOR - <ppbuf> returns pointer to data pointer.
* ATA_ZERO            ATA_ZERO           PackedLBA/CHS        read data                    
*                                                     
* IOCTL_CFA_WRITE_MULTIPLE_WITHOUT_ERASE 
* ATA_ZERO          sector count        PackedCHS/LBA       write data
*
*       NOTE: -<pbuf> contains pointer to data pointer.     
*                                                       
* IOCTL_CFA_WRITE_SECTORS_WITHOUT_ERASE       
* ATA_ZERO          sector count        PackedCHS/LBA       write data
*
* \be
*
* RETURNS: OK or ERROR
*/

STATUS atapiIoctl
    (
    int         function,       /* The IO operation to do */
    int         ctrl,           /* Controller number of the drive */
    int         drive,          /* Drive number */
    int         password [16],  /* Password to set. NULL if not applicable */
    int         arg0,           /* 1st arg to pass. NULL if not applicable */
    UINT32   *  arg1,           /* Ptr to 2nd arg.  NULL if not applicable */
    UINT8    ** ppBuf           /* The data buffer */
    )
    {
    ATA_CTRL  * pCtrl  = &ataCtrl[ctrl];
    ATA_DRIVE * pDrive = &pCtrl->drive[drive];
    ATA_DEV   * pAtapiDev = pDrive->pAtaDev;
    UINT8       dataBuffer[512];

    dataBuffer[0]    = 0x00;

    if ((ctrl >= ATA_MAX_CTRLS) || (drive >= ATA_MAX_DRIVES))
        {
        printErr("Invalid Controller number or Number of drives");
        return(ERROR);
        }

    if (pDrive->type == ATA_TYPE_NONE)
        {
        printErr("Controller %d ,drive %d is not present.",ctrl,drive);
        return(ERROR);
        }

    switch (function)
        {
#ifdef ATAPI_SECURITY_FEATURE
        {
        int   i;
        case IOCTL_DIS_MASTER_PWD:         /* 0X00 */
            dataBuffer[0] = 0x01;
            /* no break */
        case IOCTL_DIS_USER_PWD:           /* 0x01 */
            for (i=0; i<16; i++)
                dataBuffer[i+1]=password[i];

            if (ataCmd (ctrl, drive, ATA_CMD_SECURITY_DISABLE_PASSWORD,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)dataBuffer, 0xff);
            break;

        case IOCTL_ERASE_PREPARE:          /* 0x02 */
            if (ataCmd (ctrl, drive, ATA_CMD_SECURITY_ERASE_PREPARE,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_ENH_ERASE_UNIT_USR:     /* 0x05 */
        case IOCTL_ENH_ERASE_UNIT_MSTR:    /* 0x06 */
            dataBuffer[0] = 0x03;

        case IOCTL_NORMAL_ERASE_UNIT_MSTR: /* 0x04 */
            dataBuffer[0] |= 0x01;
            /* no break */
        case IOCTL_NORMAL_ERASE_UNIT_USR:  /* 0x03 */
            if (function == IOCTL_ENH_ERASE_UNIT_USR)
                dataBuffer[0] = 0x02;
            for (i=0; i<16; i++)
                dataBuffer[i+1] = password[i];

            if (ataCmd (ctrl, drive, ATA_CMD_SECURITY_ERASE_UNIT,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)dataBuffer, 0xff);
            break;

        case IOCTL_FREEZE_LOCK:            /* 0x07 */
            if (ataCmd (ctrl, drive, ATA_CMD_SECURITY_FREEZE_LOCK,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_SET_PASS_MSTR:
            dataBuffer[0] = 0x01;
            for (i=0; i<16; i++)
                dataBuffer[i+1] = password[i];

            if (ataCmd (ctrl, drive, ATA_CMD_SECURITY_SET_PASSWORD,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)dataBuffer, 0xff);
            break;

        case IOCTL_SET_PASS_USR_MAX :
            dataBuffer[0] = 0x80;

            /* no break */

        case IOCTL_SET_PASS_USR_HIGH:
            for (i=0; i<16; i++)
                dataBuffer[i+1] = password[i];

            if (ataCmd( ctrl, drive, ATA_CMD_SECURITY_SET_PASSWORD,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)dataBuffer, 0xff);
            break;

        case IOCTL_UNLOCK_MSTR :
            dataBuffer[0] = 0x01;

            /* no break */

        case IOCTL_UNLOCK_USR :
            for (i=0; i<16; i++)
                dataBuffer[i+1] = password[i];

            if (ataCmd (ctrl, drive, ATA_CMD_SECURITY_UNLOCK,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)dataBuffer, 0xff);
            break;
        }
#endif /* ATAPI_SECURITY_FEATURE */

#ifdef HOST_PROT_AREA_FEATURE
        case IOCTL_READ_NATIVE_MAX_ADDRESS:

            /* arg0 can be ATA_SDH_IBM or ATA_SDH_LBA */

            if (ataCmd (ctrl, drive, ATA_CMD_READ_NATIVE_MAX_ADDRESS,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        arg0) != OK)
                goto RETERR;
            *arg1 = ataCHSPack (pDrive->nativeMaxAdd [3],
                                pDrive->nativeMaxAdd [2],
                                pDrive->nativeMaxAdd [1],
                                pDrive->nativeMaxAdd [0]);

            /* 
             * clear LBA ( ie 4 bit of device/Head register value) 
             * bit if it has read in LBA address 
             */

            *arg1 ^= 0x40000000;  
            break;

        case IOCTL_SET_MAX_ADDRESS:
            if (ataCmd (ctrl, drive, ATA_CMD_SET_MAX,
                        ATA_SUB_SET_MAX_ADDRESS, arg0,
                        (UINT8)((*arg1) & 0xff),
                        (UINT8)(((*arg1) >>8)& 0xff),
                        (UINT8)(((*arg1) >>16)& 0xff),
                        (UINT8)(((*arg1) >>24)^ 0x40)) != OK)
                goto RETERR;
            break;

        case IOCTL_SET_MAX_SET_PASS:
            dataBuffer [0] = 0x00;
            for (i=0; i<16; i++)
                dataBuffer [i+1] = password[i];
            if (ataCmd (ctrl, drive, ATA_CMD_SET_MAX,
                        ATA_SUB_SET_MAX_SET_PASS,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO)
                != OK)
                goto RETERR;
            ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)dataBuffer, 0xff);
            break;

        case IOCTL_SET_MAX_LOCK:
            if (ataCmd (ctrl, drive, ATA_CMD_SET_MAX,
                        ATA_SUB_SET_MAX_LOCK, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_SET_MAX_UNLOCK:
            if (ataCmd (ctrl, drive, ATA_CMD_SET_MAX,
                        ATA_SUB_SET_MAX_UNLOCK, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_SET_MAX_FREEZE_LOCK:
            if (ataCmd (ctrl, drive, ATA_CMD_SET_MAX,
                        ATA_SUB_SET_MAX_FREEZE_LOCK, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;

#endif /* HOST_PROT_AREA_FEATURE */

#ifdef ATAPI_PWR_MNGMT
        case IOCTL_CHECK_POWER_MODE:
            if (ataCmd (ctrl, drive, ATA_CMD_CHECK_POWER_MODE,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            *arg1 = ATA_IO_BYTE_READ (ATAPI_SECCNT_INTREASON);
            break;

        case IOCTL_IDLE_IMMEDIATE:
            if (ataCmd (ctrl, drive, ATA_CMD_IDLE_IMMEDIATE,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_SLEEP:
            if (ataCmd (ctrl, drive, ATA_CMD_SLEEP,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_STANDBY_IMMEDIATE:
            if (ataCmd (ctrl, drive, ATA_CMD_STANDBY_IMMEDIATE,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_ENB_POW_UP_STDBY:
            if (ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                        ATA_SUB_ENB_POW_UP_STDBY, ATA_ZERO, ATA_ZERO,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;

#endif /* ATAPI_PWR_MNGMT */

#ifdef ATAPI_ADV_PWR_MNGMT
        case IOCTL_ENB_SET_ADV_POW_MNGMNT:
            {
            ATA_PARAM * pParam = &pDrive->param;
            if (pParam->suppCommand2 & 0x0008)
                {
                if (ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                            ATA_SUB_ENB_ADV_POW_MNGMNT, arg0, ATA_ZERO,
                            ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                    goto RETERR;
                }
            else
                {
                ATA_DEBUG_MSG (2, "Advanced Power Management feature is not supported"
                               " by the drive.\n",
                               0, 0, 0, 0, 0, 0);
                goto RETERR;
                }
            break;
            }

        case IOCTL_DISABLE_ADV_POW_MNGMNT:
            if (ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                        ATA_SUB_DIS_ADV_POW_MNGMT, ATA_ZERO, ATA_ZERO,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;
#endif /* ATAPI_ADV_PWR_MNGMT */

#ifdef REMOV_MEDIA_FEATURE
        case IOCTL_EJECT_DISK:
            if (pDrive->type == ATA_TYPE_ATAPI)
                atapiStartStopUnit(pAtapiDev, EJECT_DISK);
            else
                {
                if ((pParam->enableCommandFeature1 & 0x04) ||
                    (pParam->enableCommandFeature2 & 0x08))

                    if (ataCmd( ctrl, drive, ATA_CMD_MEDIA_EJECT,
                                ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                                ATA_ZERO, ATA_ZERO) != OK)
                        goto RETERR;
                }
            break;
        case IOCTL_LOAD_DISK:
            if (pDrive->type == ATA_TYPE_ATAPI)
                atapiStartStopUnit(pAtapiDev, LOAD_DISK);
            else
                {
                ATA_DEBUG_MSG (2,"Not supported by ATA devices.\n",
                               0, 0, 0, 0, 0, 0);
                }
            break;
        case IOCTL_MEDIA_LOCK:
            if (pDrive->type == ATA_TYPE_ATAPI)
                atapiCtrlMediumRemoval(pAtapiDev, MEDIA_LOCK);
            else
                {
                if ((pParam->enableCommandFeature1 & 0x04) ||
                    (pParam->enableCommandFeature2 & 0x08))

                    if (ataCmd (ctrl, drive, ATA_CMD_MEDIA_LOCK,
                                ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                                ATA_ZERO, ATA_ZERO) != OK)
                        goto RETERR;
                }
            break;
        case IOCTL_MEDIA_UNLOCK:
            if (pDrive->type == ATA_TYPE_ATAPI)
                atapiCtrlMediumRemoval (pAtapiDev, MEDIA_UNLOCK);
            else
                {
                if ((pParam->enableCommandFeature1 & 0x04) ||
                    (pParam->enableCommandFeature2 & 0x08))

                    if (ataCmd (ctrl, drive, ATA_CMD_MEDIA_UNLOCK,
                                ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                                ATA_ZERO, ATA_ZERO) != OK)
                        goto RETERR;
                }
            break;

#endif /* REMOV_MEDIA_FEATURE */

#ifdef REMOV_MEDIA_NOTIFY
        {
        short       temp;
        case IOCTL_GET_MEDIA_STATUS:
            if (ataCmd (ctrl, drive, ATA_CMD_GET_MEDIA_STATUS,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            temp = ATA_IO_BYTE_READ (ATAPI_ERROR);
            *arg1 = temp;
            break;

        case IOCTL_ENA_REMOVE_NOTIFY:
            if (ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                        ATA_SUB_ENABLE_NOTIFY, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO,ATA_ZERO) != OK)
                goto RETERR;
            pDrive->mediaStatusNotifyVer = 
            ATA_IO_BYTE_READ (ATAPI_CYLLOW_BCOUNT_LO);
            temp = ATA_IO_BYTE_READ (ATAPI_CYLHI_BCOUNT_HI);
            pDrive->okPEJ  = (temp & 0x04) ? TRUE : FALSE;
            pDrive->okLock = (temp & 0x02) ? TRUE : FALSE;
            break;

        case IOCTL_DISABLE_REMOVE_NOTIFY:
            if (ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE,
                        ATA_SUB_DISABLE_NOTIFY, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO,ATA_ZERO,ATA_ZERO) != OK)
                goto RETERR;
            pDrive->mediaStatusNotifyVer = 
                    ATA_IO_BYTE_READ (ATAPI_CYLLOW_BCOUNT_LO);
            temp = ATA_IO_BYTE_READ (ATAPI_CYLHI_BCOUNT_HI);
            pDrive -> okPEJ  = (temp & 0x04) ? TRUE : FALSE;
            pDrive -> okLock = (temp & 0x02) ? TRUE : FALSE;
            break;
        }
#endif /* REMOV_MEDIA_NOTIFY */

#ifdef ATA_SMART_FEATURE
        case IOCTL_SMART_DISABLE_OPER:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART, ATA_SMART_DISABLE_OPER,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO) 
                != OK)
                goto RETERR;
            break;

        case IOCTL_SMART_ENABLE_ATTRIB_AUTO:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART, ATA_SMART_ATTRIB_AUTO,
                        0xf1, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_SMART_DISABLE_ATTRIB_AUTO:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART, ATA_SMART_ATTRIB_AUTO,
                        0x00, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_SMART_ENABLE_OPER:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART, ATA_SMART_ENABLE_OPER,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO) 
                != OK)
                goto RETERR;
            break;

        case IOCTL_SMART_OFFLINE_IMMED:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART, ATA_SMART_OFFLINE_IMMED,
                        arg0, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_SMART_READ_DATA:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART,
                        ATA_SMART_READ_DATA, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO, ATA_ZERO)!= OK)
                goto RETERR;
            ATA_IO_NWORD_READ (ATAPI_DATA, (void *)dataBuffer,256);
            *ppBuf = dataBuffer;
            break;

        case IOCTL_SMART_READ_LOG_SECTOR:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART,
                        ATA_SMART_READ_LOG_SECTOR, arg0,
                        *arg1, ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            ATA_IO_NWORD_READ (ATAPI_DATA, (void *)dataBuffer,256);
            *ppBuf = dataBuffer;
            break;

        case IOCTL_SMART_RETURN_STATUS:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART, ATA_SMART_RETURN_STATUS,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO) 
                != OK)
                goto RETERR;
            break;

        case IOCTL_SMART_SAVE_ATTRIB:
            if (ataCmd (ctrl, drive, ATA_CMD_SMART, ATA_SMART_SAVE_ATTRIB,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO) 
                != OK)
                goto RETERR;
            break;

        case IOCTL_SMART_WRITE_LOG_SECTOR:

            /* arg0 is LogAddress. *arg1 is No of sectors to write */

            if (ataCmd (ctrl, drive, ATA_CMD_SMART, ATA_SMART_WRITE_LOG_SECTOR,
                        arg0, *arg1, ATA_ZERO, ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)(*ppBuf),256);
            break;

#endif /* ATA_SMART_FEATURE */

#ifdef ATA_CFA_FEATURE
        case IOCTL_CFA_ERASE_SECTORS:

            /* 
             * arg0 = sector count
             * *arg1 = PackedCHS/LBA
             */

            if (ataCmd (ctrl, drive, ATA_CMD_CFA_ERASE_SECTORS, ATA_ZERO, arg0,
                        (UINT8)((*arg1) & 0xff),
                        (UINT8)(((*arg1) >>8)& 0xff),
                        (UINT8)(((*arg1) >>16)& 0xff),
                        (UINT8)(((*arg1) >>24)^ 0x40)) != OK)
                goto RETERR;
            break;

        case IOCTL_CFA_REQUEST_EXTENDED_ERROR_CODE:
            if (ataCmd (ctrl,drive, ATA_CMD_CFA_REQUEST_EXTENDED_ERROR_CODE,
                        ATA_ZERO, ATA_ZERO, ATA_ZERO, ATA_ZERO, 
                        ATA_ZERO, ATA_ZERO) != OK)
                goto RETERR;
            break;

        case IOCTL_CFA_TRANSLATE_SECTOR:
            if (ataCmd (ctrl,drive, ATA_CMD_CFA_TRANSLATE_SECTOR,ATA_ZERO, 
                        ATA_ZERO, (UINT8)((*arg1) & 0xff),
                        (UINT8)(((*arg1) >>8)& 0xff),
                        (UINT8)(((*arg1) >>16)& 0xff),
                        (UINT8)(((*arg1) >>24)^ 0x40)) != OK)
                goto RETERR;
            ATA_IO_NWORD_READ (ATAPI_DATA, (short *) dataBuffer, 256);
            * ppBuf = dataBuffer;
            break;

        case IOCTL_CFA_WRITE_MULTIPLE_WITHOUT_ERASE:
            if (ataCmd (ctrl, drive, ATA_CMD_CFA_WRITE_MULTIPLE_WITHOUT_ERASE,
                        ATA_ZERO, arg0,
                        (UINT8)((*arg1) & 0xff),
                        (UINT8)(((*arg1) >> 8)& 0xff),
                        (UINT8)(((*arg1) >>16)& 0xff),
                        (UINT8)(((*arg1) >>24)^ 0x40)) != OK)
                goto RETERR;

            /* Modify this when implementing as part of transfer loop */

            ATA_IO_NWORD_WRITE (ATAPI_DATA, dataBuffer,256);
            break;

        case IOCTL_CFA_WRITE_SECTORS_WITHOUT_ERASE:
            if (ataCmd (ctrl,drive,ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE,
                        ATA_ZERO, arg0,
                        (UINT8)((*arg1) & 0xff),
                        (UINT8)(((*arg1) >> 8)& 0xff),
                        (UINT8)(((*arg1) >>16)& 0xff),
                        (UINT8)(((*arg1) >>24)^ 0x40)) != OK)
                goto RETERR;

            /* Modify this when implementing as part of transfer loop */

            ATA_IO_NWORD_WRITE (ATAPI_DATA, dataBuffer,256);
            break;

#endif /* ATA_CFA_FEATURE */

        case IOCTL_ATAPI_READ_TOC_PMA_ATIP :
            {
                READ_TOC_STRUCT * pData = (READ_TOC_STRUCT *)arg0;

                if (atapiReadTocPmaAtip(
                                       pAtapiDev,
                                       pData->transferLength,
                                       (char*)pData->pResultBuf) != OK)
                    goto RETERR;
                break;
            }

        default:
            printErr ("Unsupported command 0x%x or feature.\nPlease refer to"
                      " VxWorks documentation to know how to enable the feature"
                      " in the driver, if the device supports it\n", function);
        }    
    return(OK);  

    RETERR:
    return(ERROR);

    } /* atapiIoctl */


#ifdef HOST_PROT_AREA_FEATURE
/**************************************************************************
*
* ataCHSPack - Pack Cylinder,Head,sector values to address a sector.
*
* This routine packes all the 3 parameters (cylinder, head, sector number)
* to a single 32 bit word to adress a sector on the drive.
*
* RETURNS: OK.
*/

LOCAL UINT32 ataCHSPack
    (
    UINT8 Head,
    UINT8 cylinderHigh,
    UINT8 cylinderLow,
    UINT8 sectorNo
    )
    {
    return((((Head & 0x0f)|0x40) << 24) +
           (cylinderHigh       << 16) +
           (cylinderLow        <<  8) +
           (sectorNo)
          );
    } /* ataCHSPack */
#endif

/**************************************************************************
*
* ataBestTransferModeFind - Justify the best transfer mode using the cycle time
*
* 
* RETURNS: N/A.
*/

LOCAL void ataBestTransferModeFind
    (
    int ctrl,
    int drive
    )
    {
    ATA_CTRL  * pCtrl        = &ataCtrl[ctrl];
    ATA_DRIVE * pDrive       = &pCtrl->drive[drive];
    ATA_PARAM * pParam       = &pDrive->param;
    short       rwMode       = pDrive->rwMode;

    /* Justify the best transfer mode using the cycle time */

    switch (rwMode)
        {

        /* 
         * Cycle time threshold values are not implemented for verification
         * of best UDMA mode. But the Table 50, Section 10.2.3, Ref-1 says
         * that 60, 90, 120, 160, 240ns for UDMA mode 4,3,2,1,0 respectively
         */

        /* 
         * Don't operate the device in UDMA modes 3,4,5
         * if 80 conductor cable is not present
         */
        
        case ATA_DMA_ULTRA_5:
        case ATA_DMA_ULTRA_4:
        case ATA_DMA_ULTRA_3:
            ataUDmaCableChk (ctrl,drive);
            if (pCtrl->uDmaCableOkay == FALSE)
                rwMode = ATA_DMA_ULTRA_2;
            break;

            /* 
             * Find Best Multi/Single DMA mode
             * Table 15, Section 4.3, Ref-3 is implemented.
             * But the Table 49, Section 10.2.3, Ref-1 says the Cycle time
             * threshold values are 120, 150, 480ns for MDMA 2,1,0 respectively.
             */

        case ATA_DMA_MULTI_2:
            if (pParam->cycleTimeDma > 120)
                rwMode = ATA_DMA_MULTI_1;
            break; 
        case ATA_DMA_MULTI_1:
            if (pParam->cycleTimeDma > 180)
                rwMode = ATA_DMA_SINGLE_2;
            break;
        case ATA_DMA_SINGLE_2:
            if (pParam->cycleTimeDma < 240)
                break;
        case ATA_DMA_MULTI_0:
        case ATA_DMA_SINGLE_1:
        case ATA_DMA_SINGLE_0:

            /* Dma shall be disabled. So set Best PIO mode */

            rwMode = ATA_PIO_W_0 + pDrive->pioMode;
            break;
        default:
            rwMode = pDrive->rwMode;
        }


    pDrive->rwMode = rwMode;

    /*
     * The following switch shall not be included in the above switch.
     * Because this (pDrive->rwMode) test is done on the "output of the
     * above switch statement" also.
     */

    switch (pDrive->rwMode)
        {

        /*
         * Find Best PIO mode
         * Table 17, Section 4.4, Ref-3 is implemented.
         * But Table 48, section 10.2.2, Ref-1 says the Cycle time
         * threshold values are 120, 180, 240, 383, 600ns for
         * PIOmode 4,3,2,1,0 respectively.
         */
        
        case ATA_PIO_W_4:
            if (pParam->cycleTimePioIordy > 120)
                rwMode = ATA_PIO_W_3;

            /*no break;*/

        case ATA_PIO_W_3:
            if (pParam->cycleTimePioIordy > 180)
                rwMode = ATA_PIO_W_2;
            if (pParam->cycleTimePioIordy > 240)
                rwMode = ATA_PIO_W_0;

            /*no break;*/

        case ATA_PIO_W_2: /* Drive must support <= 240ns */
            break;
        case ATA_PIO_W_1:
        case ATA_PIO_W_0:
            rwMode = ATA_PIO_W_0;
            break;
            /* 
             * There shall be some method to compare with
             * pParam->cycleTimePioNoIordy for the PIO modes 0,1,2 as there
             * will not be any IOReady siganl. But Ref-3 is not suggesting
             * any method.
             */

        default:
            rwMode = pDrive->rwMode;
        }
    pDrive->rwMode = rwMode;

    }


/**************************************************************************
*
* ataUDmaCableChk - Find the cable type.
*
* This routine will find the cable type used for specified controller and 
* device. If a 40 conductor cable is connected uDmaCableOkay flag is made 
* FALSE and if 80 conductor cable using 40 pin connector is set to TRUE of 
* ataCtrl.
*
* RETURNS: OK
*/
LOCAL void ataUDmaCableChk
    (
    int ctrl, 
    int drive
    )
    {
    ATA_CTRL  * pCtrl        = &ataCtrl[ctrl];

    /* 
     * if 13 bit of 93 word of device parameters set to 1 
     * it indicates the 80 conductor cable is connected.
     */

    /* cut out this check, as it is not always a valid determination */
#if FALSE
    if ((pCtrl->drive[drive].param.hardResetResult & ATA_HWRR_CBLID) != 0)
        pCtrl->uDmaCableOkay = FALSE;
    else
#endif
        pCtrl->uDmaCableOkay = TRUE;

    } /* ataUDmaCableChk */



#if defined (ATA_DEBUG)
/**************************************************************************
*
* atapiParamsPrint - Print the drive parameters.
*
* This user callable routine will read the current parameters from the
* corresponding drive and will print the specified range of parameters on
* the console.
*
* RETURNS: N/A.
*/

void atapiParamsPrint
    (
    int ctrl,
    int drive
    )
    {
    ATA_CTRL  * pCtrl        = &ataCtrl[ctrl];
    ATA_DRIVE * pDrive       = &pCtrl->drive[drive];
    ATA_PARAM * pParam       = &pDrive->param;
    char      * mychar       = (char *)pParam;
    int         i            = 0;

    for (i = 0; i <= sizeof(ATA_PARAM)/2; i++)
        {
        printf("\n            pParam[%d] = %#x %#x         %c %c          ",
               i, *(mychar + 2 * i + 1 ), *(mychar + 2 * i ),
               *(mychar + 2 * i + 1),*(mychar + 2 * i ));
        }

    }

#endif

/**************************************************************************
*
* atapiBlkIoctl - do device specific control function
*
* This routine is called when the file system cannot handle an ioctl()
* function.
*
*  Functions :
*  Play CD      - ATAPI_CMD_PLAY_CD_LBA
*  Set CD Speed - ATAPI_CMD_SET_CD_SPEED
*
* RETURNS:  OK or ERROR.
*/

LOCAL STATUS atapiBlkIoctl
    (
    ATA_DEV * pAtapiDev,
    int       function,
    int       arg
    )
    {
    ATA_CTRL  * pCtrl   = &ataCtrl[pAtapiDev->ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[pAtapiDev->drive];
    int         status  = ERROR;

    ATA_DEBUG_MSG (5, "atapiBlkIoctl%d/%d: state=%#x func=%#d arg=%#d\n",
                   pAtapiDev->ctrl, pAtapiDev->drive, pDrive->state, function,
                   arg,0);

    if ((function & 0xffff0000) == 0xcb100000)
        {
        /* ignore CBIO ioctl's */
        return(OK);
        }
    
    if ((!pCtrl->installed) || (pDrive->state != ATA_DEV_OK))
        {
        ATA_DEBUG_MSG (2,"Device not okay\n", 0, 0, 0, 0, 0, 0);
        return(ERROR);
        }


    semTake (&pCtrl->muteSem, WAIT_FOREVER);

    switch (function)
        {
        case FIODISKFORMAT:
            (void) errnoSet (S_ioLib_UNKNOWN_REQUEST);
            break;

        case FIOSYNC:
            (void)ataCmd (pAtapiDev->ctrl, pAtapiDev->drive, ATA_CMD_FLUSH_CACHE, 0, 0, 0, 0, 0, 0);
            flushCacheCount++;     /* was part of statements immed below */
            status = OK;
            break;

        default:
    status = atapiIoctl (function, pAtapiDev->ctrl, pAtapiDev->drive,
                         ATA_ZERO, arg, ATA_ZERO, ATA_ZERO);
        }

    semGive (&pCtrl->muteSem);
    
    return(status);
    } /* atapiBlkIoctl */

/**************************************************************************
*
* atapiCtrlMediumRemoval - Issues PREVENT/ALLOW MEDIUM REMOVAL packet command
*
* This function issues a command to drive to PREVENT or ALLOW MEDIA removal. 
* Argument <arg0> selects to LOCK_EJECT or UNLOCK_EJECT. 
* 
* To lock media eject <arg0> should be 'LOCK_EJECT'
* To unload media eject <arg0> should be 'UNLOCK_EJECT'
*
* RETURN: OK or ERROR 
*/

STATUS atapiCtrlMediumRemoval
    (
    ATA_DEV   * pAtapiDev,
    int         arg0
    )
    {
    ATAPI_CMD   atapiCmd;

    ATA_DEBUG_MSG (15,"atapiCtrlMediumRemoval entered arg0 %#x\n",
                   arg0, 0, 0, 0, 0, 0);    

    if (pAtapiDev == NULL)
        return(ERROR);

    if ((arg0 != MEDIA_UNLOCK)&&( arg0 != MEDIA_LOCK))
        return(ERROR);

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    atapiCmd.cmdPkt[0] = ATAPI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL;
    atapiCmd.cmdPkt[4] = arg0;  /* 0 = allow, 1 = prevent */

    atapiCmd.ppBuf           = NULL;
    atapiCmd.bufLength       = 0;
    atapiCmd.direction       = NON_DATA;
    atapiCmd.desiredTransferSize = 0;
    atapiCmd.dma             = FALSE;
    atapiCmd.overlap         = FALSE;

    ATA_DEBUG_MSG (9, "atapiCommand%d/%d: %#x\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, ATAPI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL, 
                   0, 0, 0);

    if (atapiPktCmdSend (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)

        return(ERROR);
    else
        return(OK);

    }  /* atapiCtrlMediumRemoval */

/**************************************************************************
*
* atapiRead10 - read one or more blocks from an ATAPI Device.
*
* This routine reads one or more blocks from the specified device,
* starting with the specified block number.
*
* The name of this routine relates to the SFF-8090i (Mt. Fuji), used for
* DVD-ROM, and indicates that the entire packet command uses 10 bytes,
* rather than the normal 12.
*
* RETURNS: OK, ERROR if the read command didn't succeed.
*/

STATUS atapiRead10
    (
    ATA_DEV   * pAtapiDev,
    UINT32      startBlk,
    UINT32      nBlks,
    UINT32      transferLength,
    char      * pBuf
    )
    {
    ATA_CTRL  * pCtrl   = &ataCtrl[pAtapiDev->ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[pAtapiDev->drive];
    ATAPI_CMD   atapiCmd;
    int         i;
    char      * pUpdatedBuf = pBuf;

    ATA_DEBUG_MSG (5, "atapiRead10%d/%d: state=%#x startBlk=%#x"
                   " nBlks=%#x pBuf=%#x\n",
                   pAtapiDev->ctrl, pAtapiDev->drive, pDrive->state,
                   startBlk, nBlks, pBuf);

    if (!pCtrl->installed)
        return(ERROR);

    if (pDrive->state != ATA_DEV_OK)
        return(ERROR);

    /* Sanity check */

    if ( (startBlk >= pAtapiDev->blkDev.bd_nBlocks) ||
         ((startBlk + nBlks) >= pAtapiDev->blkDev.bd_nBlocks))
        {
        ATA_DEBUG_MSG (2, "atapiRead10 %d/%d ERROR: block numbers must be in "
                       "range 0 - %#d\n", pAtapiDev->ctrl, pAtapiDev->drive,
                       (pAtapiDev->blkDev.bd_nBlocks - 1), 0, 0, 0);
        return(ERROR);
        }

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    /* Fill Command Packet */
    atapiCmd.cmdPkt[0] = ATAPI_CMD_READ10;
    for (i = 2; i < 6; i++)
        atapiCmd.cmdPkt[i] += (UINT8)( startBlk >> (8 * (5 - i)));
    atapiCmd.cmdPkt[7] = (transferLength >> 8) & 0xff;
    atapiCmd.cmdPkt[8] = transferLength & 0xff;

    i = nBlks * pAtapiDev->blkDev.bd_bytesPerBlk;

    atapiCmd.ppBuf           = &pUpdatedBuf;
    atapiCmd.bufLength       = i;
    atapiCmd.direction       = IN_DATA;
    atapiCmd.desiredTransferSize = i;
    atapiCmd.overlap         = FALSE;
    if (pDrive->usingDma == TRUE)
        {
        ATA_DEBUG_MSG (4,"dma true ReadIO \n", 0, 0, 0, 0, 0, 0);
        atapiCmd.dma         = TRUE;
        }
    else
        atapiCmd.dma         = FALSE;

    /* Execute Packet Command */

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    ATA_DEBUG_MSG (5, "atapiRead10%d/%d: Ok\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    } /* atapiRead10 */

/**************************************************************************
*
* atapiReadCapacity - issue a READ CD-ROM CAPACITY command to a ATAPI device
*
* This routine issues a READ CD-ROM CAPACITY command to a specified ATAPI 
* device.
*
* RETURN: OK, or ERROR if the command fails.
*/

STATUS atapiReadCapacity
    (
    ATA_DEV *   pAtapiDev
    )
    {
    BLK_DEV *   pBlkDev = &pAtapiDev->blkDev;

    ATAPI_CMD   atapiCmd;
    UINT8       resultBuf[8];
    char *      pBuf = (char *)resultBuf;
    int         i;

    ATA_DEBUG_MSG (14, "atapiReadCapacity%d/%d: \n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    /* clear command packet before use */
    for (i = 0; i < sizeof(ATAPI_CMD); i++)
        atapiCmd.cmdPkt[i] = 0;

    atapiCmd.cmdPkt[0] = ATAPI_CMD_READ_CD_CAPACITY;

    atapiCmd.ppBuf                  = &pBuf;
    atapiCmd.bufLength              = 8;
    atapiCmd.direction              = IN_DATA;
    atapiCmd.desiredTransferSize    = 8;
    atapiCmd.dma                    = FALSE;
    atapiCmd.overlap                = FALSE;

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    pBlkDev->bd_nBlocks = 0;
    pBlkDev->bd_bytesPerBlk = 0;

    for (i = 0; i < 4; i++)
        pBlkDev->bd_nBlocks += ((ULONG)resultBuf[i]) << (8 * (3 - i));

    pBlkDev->bd_nBlocks++;                 /* ULONG blocks on device */
    pBlkDev->bd_blksPerTrack = pBlkDev->bd_nBlocks;
    /* ULONG blocks per track */
    /*
     * This will work only for data CDs.
     * need modification for audio or audio mixed data CDs to work.
     */

    for (i = 4; i < 8; i++)
        pBlkDev->bd_bytesPerBlk += ((ULONG)resultBuf[i]) << (8 * (7 - i));

    ATA_DEBUG_MSG (14,"atapiReadCapacity%d/%d: Ok: nBlocks=%d bytesPerBlk=%d\n",
                   pAtapiDev->ctrl, 
                   pAtapiDev->drive, 
                   pBlkDev->bd_nBlocks, 
                   pBlkDev->bd_bytesPerBlk, 0, 0);

    return(OK);
    } /* atapiReadCapacity */

/**************************************************************************
*
* atapiReadTocPmaAtip - issue a READ TOC command to a ATAPI device
*
* This routine issues a READ TOC command to a specified ATAPI device.
*
* RETURN: OK, or ERROR if the command fails.
*/

STATUS atapiReadTocPmaAtip
    (
    ATA_DEV * pAtapiDev,
    UINT32    transferLength,
    char    * resultBuf
    )
    {
    ATAPI_CMD   atapiCmd;
    char *      pBuf = (char *)resultBuf;

    ATA_DEBUG_MSG (14, "atapiReadToc%d/%d: \n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    atapiCmd.cmdPkt[0] = ATAPI_CMD_READ_TOC_PMA_ATIP;
    atapiCmd.cmdPkt[7] = (UINT8)(transferLength >> 8); 
    atapiCmd.cmdPkt[8] = (UINT8)(transferLength     ); 

    atapiCmd.ppBuf               = &pBuf;
    atapiCmd.bufLength           = transferLength;
    atapiCmd.direction           = IN_DATA;
    atapiCmd.desiredTransferSize = transferLength;
    atapiCmd.dma                 = FALSE;
    atapiCmd.overlap             = FALSE;

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    ATA_DEBUG_MSG (14,"atapiReadToc%d/%d: Ok: First track = %d Last track = %d\n",
                   pBuf[2], pBuf[3], 0, 0, 0, 0);

    return(OK);
    } /* atapiReadTocPmaAtip */


/**************************************************************************
*
* atapiScan - issue SCAN packet command to ATAPI drive.
*
* This function issues SCAN packet command to ATAPI drive. The <function> 
* argument should be 0x00 for fast forward and 0x10 for fast reversed operation.
*
* RETURN: OK or ERROR
*/

STATUS atapiScan
    (
    ATA_DEV   * pAtapiDev,
    UINT32      startAddressField,
    int         function
    )
    {
    ATAPI_CMD   atapiCmd;

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    atapiCmd.cmdPkt[0]  = ATAPI_CMD_SCAN;
    atapiCmd.cmdPkt[1]  = function;                                         /* 0x00 for fast-forward, 0x10 fast-reverse 
                                                                             * bit 4: direction                        
                                                                             */
    atapiCmd.cmdPkt[2]  = (startAddressField >> 24) & 0xff; /* MSB */
    atapiCmd.cmdPkt[3]  = (startAddressField >> 16) & 0xff;
    atapiCmd.cmdPkt[4]  = (startAddressField >> 8) & 0xff;
    atapiCmd.cmdPkt[5]  = startAddressField & 0xff; /* LSB */

    atapiCmd.ppBuf           = NULL;
    atapiCmd.bufLength       = 0;        
    atapiCmd.direction       = NON_DATA; 
    atapiCmd.desiredTransferSize = 0;    
    atapiCmd.overlap         = FALSE;
    atapiCmd.dma             = FALSE;

    /* Execute Packet Command */

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    ATA_DEBUG_MSG (5, "atapiScan%d/%d: Ok\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    }  /* atapiScan */




/**************************************************************************
*
* atapiSeek - issues a SEEK packet command to drive.
*
* This function issues a SEEK packet command (not ATA SEEK command) to 
* the specified drive.
*
* RETURN: OK or ERROR
*/

STATUS atapiSeek
    (
    ATA_DEV * pAtapiDev,
    UINT32    addressLBA
    )
    {
    ATAPI_CMD   atapiCmd;

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    atapiCmd.cmdPkt[0] = ATAPI_CMD_SEEK;
    atapiCmd.cmdPkt[2] = (addressLBA >> 24) & 0xff; /* MSB */
    atapiCmd.cmdPkt[3] = (addressLBA >> 16) & 0xff;
    atapiCmd.cmdPkt[4] = (addressLBA >> 8) & 0xff;
    atapiCmd.cmdPkt[5] = addressLBA & 0xff; /* LSB */

    atapiCmd.ppBuf           = NULL;
    atapiCmd.bufLength       = 0;
    atapiCmd.direction       = NON_DATA;
    atapiCmd.desiredTransferSize = 0;
    atapiCmd.dma             = FALSE;
    atapiCmd.overlap         = FALSE;

    /* Execute Packet Command */

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    ATA_DEBUG_MSG (5, "atapiSeek%d/%d: Ok\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    }  /* atapiSeek */

/**************************************************************************
*
* atapiSetCDSpeed - issue SET CD SPEED packet command to ATAPI drive.
*
* This function issues SET CD SPEED packet command to ATAPI drive while reading
* and writing of ATAPI drive(CD-ROM) data. The arguments <readDriveSpeed> and 
* <writeDriveSpeed> are in Kbytes/Second.
* 
* RETURN: OK or ERROR
*/

STATUS atapiSetCDSpeed
    (
    ATA_DEV   * pAtapiDev,
    int         readDriveSpeed,
    int         writeDriveSpeed
    )
    {
    ATAPI_CMD   atapiCmd;

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    atapiCmd.cmdPkt[0] = ATAPI_CMD_SET_CD_SPEED;
    atapiCmd.cmdPkt[2] = (readDriveSpeed >> 8) & 0xff; /* MSB */
    atapiCmd.cmdPkt[3] = readDriveSpeed & 0xff; /* LSB */
    atapiCmd.cmdPkt[4] = (writeDriveSpeed >> 8) & 0xff; /* MSB */
    atapiCmd.cmdPkt[5] = writeDriveSpeed & 0xff; /* LSB */

    atapiCmd.ppBuf           = NULL;
    atapiCmd.bufLength       = 0;        
    atapiCmd.direction       = NON_DATA; 
    atapiCmd.desiredTransferSize = 0;    
    atapiCmd.dma             = FALSE;    
    atapiCmd.overlap         = FALSE;

    /* Execute Packet Command */

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    ATA_DEBUG_MSG (5, "atapiSetCDSpeed%d/%d: Ok\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    }  /* atapiSetCDSpeed */

/**************************************************************************
*
* atapiStopPlayScan - issue STOP PLAY/SCAN packet command to ATAPI drive.
*
* RETURN: OK or ERROR
*/

STATUS atapiStopPlayScan
    (
    ATA_DEV   * pAtapiDev
    )
    {
    ATAPI_CMD   atapiCmd;

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    atapiCmd.cmdPkt[0] = ATAPI_CMD_STOP_PLAY_SCAN;

    atapiCmd.ppBuf           = NULL;
    atapiCmd.bufLength       = 0;        
    atapiCmd.direction       = NON_DATA; 
    atapiCmd.desiredTransferSize = 0;    
    atapiCmd.dma             = FALSE;    
    atapiCmd.overlap         = FALSE;

    /* Execute Packet Command */

    if (atapiPktCmd (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        return(ERROR);

    ATA_DEBUG_MSG (5, "atapiStopPlayScan%d/%d: Ok\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    }  /* atapiStopPlayScan */

/**************************************************************************
*
* atapiStartStopUnit - Issues START STOP UNIT packet command
*
* This function issues a command to drive to MEDIA EJECT and MEDIA LOAD. 
* Argument <arg0> selects to EJECT or LOAD. 
* 
* To eject media <arg0> should be 'EJECT_DISK'
* To load media <arg0> should be 'LOAD_DISK'
*
* RETURN: OK or ERROR 
*/

STATUS atapiStartStopUnit
    (
    ATA_DEV   * pAtapiDev,
    int         arg0
    )
    {
    ATAPI_CMD   atapiCmd;

    ATA_DEBUG_MSG (5,"atapiStartStopUnit entered%x \n", arg0,0, 0, 0, 0, 0);

    /* initialize array with zeros */

    if (pAtapiDev == NULL)
        return(ERROR);

    if ((arg0 != EJECT_DISK)&&( arg0 != LOAD_DISK))
        {
        ATA_DEBUG_MSG (2,"atapiStartStopUnit ERROR%x \n", 0 ,0, 0, 0, 0, 0);
        return(ERROR);
        }

    /* clear command packet before use */
    memset(&atapiCmd.cmdPkt[0], 0, 12);

    atapiCmd.cmdPkt[0] = ATAPI_CMD_START_STOP_UNIT;
    atapiCmd.cmdPkt[1] = 0x00; /* bit0 = Immed */
    atapiCmd.cmdPkt[4] = arg0; /* 0-stopDisk,1-StartDisk&ReadTOC,
                                          2-Eject,3-Load */
    atapiCmd.ppBuf           = NULL;
    atapiCmd.bufLength       = 0;
    atapiCmd.direction       = NON_DATA;
    atapiCmd.desiredTransferSize = 0;
    atapiCmd.dma             = FALSE;
    atapiCmd.overlap         = FALSE;

    ATA_DEBUG_MSG (9, "atapiStartStopUnit%d/%d: %#x\n", pAtapiDev->ctrl,
                   pAtapiDev->drive, ATAPI_CMD_START_STOP_UNIT, 0, 0, 0);

    if (atapiPktCmdSend (pAtapiDev, &atapiCmd) != SENSE_NO_SENSE)
        {
        ATA_DEBUG_MSG (2, "atapiStartStopUnit%d/%d: atapiPktCmdSend failed\n", 
                       pAtapiDev->ctrl, pAtapiDev->drive, 0, 0, 0, 0);      
        return(ERROR);
        }
    return(OK);

    }  /* atapiStartStopUnit */

/**************************************************************************
*
* atapiTestUnitRdy - issue a TEST UNIT READY command to a ATAPI drive
*
* This routine issues a TEST UNIT READY command to a specified ATAPI drive.
*
* RETURNS: OK, or ERROR if the command fails.
*/

STATUS atapiTestUnitRdy
    (
    ATA_DEV    * pAtapiDev
    )
    {
    ATAPI_CMD    atapiCmd;
    int          i;
    int          error;

    ATA_DEBUG_MSG (9, "atapiTestUnitRdy: %d/%d: \n",
                   pAtapiDev->ctrl, pAtapiDev->drive, 0, 0, 0, 0);


    /* clear command packet before use */
    for (i = 0; i < sizeof(atapiCmd); i++)
        atapiCmd.cmdPkt[i] = 0;

    /* build a TEST UNIT READY command and execute it */
    atapiCmd.cmdPkt[0] = ATAPI_CMD_TEST_UNIT_READY;
    atapiCmd.ppBuf               = NULL;
    atapiCmd.bufLength           = 0;
    atapiCmd.direction           = NON_DATA;
    atapiCmd.desiredTransferSize = 0;
    atapiCmd.dma                 = FALSE;
    atapiCmd.overlap             = FALSE;

    i = 0;
    do
        {
        error = atapiPktCmd (pAtapiDev, &atapiCmd) & ERR_SENSE_KEY;

        /* drive may queue unit attention conditions */

        if ( ((error != SENSE_NO_SENSE) && (error != SENSE_UNIT_ATTENTION))
             || (++i > 5))
            {
            /* problem with the device is detected  */
            return(ERROR);
            }

        } while (error != SENSE_NO_SENSE);

    ATA_DEBUG_MSG (14, "atapiTestUnitRdy: %d/%d: Ok\n", 
                   pAtapiDev->ctrl, pAtapiDev->drive, 0, 0, 0, 0);

    return(OK);
    } /* atapiTestUnitRdy */


/**************************************************************************
*
* ataCmd - issue a RegisterFile command to ATA/ATAPI device.
* 
* This function executes ATA command to ATA/ATAPI devices specified by 
* arguments <ctrl> and <drive>. <cmd> is command to be executed and other 
* arguments <arg0> to <arg5> are interpreted for differently in each case 
* depending on the <cmd> command. 
* Some commands (like ATA_CMD_SET_FEATURE) have sub commands the case in 
* which <arg0> is interpreted as subcommand and <arg1> is subcommand specific.
*
* In general these arguments <arg0> to <arg5> are interpreted as command 
* registers of the device as mentioned below.
*
* \is
* \i arg0      - Feature Register
* \i arg1      - Sector count    
* \i arg2      - Sector number   
* \i arg3      - CylLo           
* \i arg4      - CylHi           
* \i arg5      - sdh Register    
* \ie
*
* As these registers are interpreted for different purpose for each command, 
* arguments are not named after registers.
*
* The following commands are valid in this function and the validity of each 
* argument for different commands. Each command is tabulated in the form
*\bs
* ---------------------------------------------------------------------  
* COMMAND 
*       ARG0     |   ARG1    |   ARG2   |   ARG3   |   ARG4   |   ARG5
* ---------------------------------------------------------------------
* 
* ATA_CMD_INITP                
*        0             0           0          0          0         0
*
* ATA_CMD_RECALIB
*        0             0           0          0          0         0
*
* ATA_PI_CMD_SRST                                  
*        0             0           0          0          0         0
*
* ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC              
*        0             0           0          0          0         0
*
* ATA_CMD_SEEK
*       cylinder    head           0          0          0         0
* or   LBA high     LBA low 
*
* ATA_CMD_SET_FEATURE       
*       FR          SC             0          0          0         0
*   (SUBCOMMAND)  (SubCommand   
*               Specific Value)
*
* ATA_CMD_SET_MULTI
*   sectors per block  0          0           0          0         0
*
* ATA_CMD_IDLE
*       SC             0          0           0          0         0
*    (Timer Period) 
*
* ATA_CMD_STANDBY
*       SC             0          0           0          0         0
*    (Timer Period)              
*
* ATA_CMD_STANDBY_IMMEDIATE
*        0             0           0          0          0         0
*
* ATA_CMD_SLEEP                
*        0             0           0          0          0         0
*
* ATA_CMD_CHECK_POWER_MODE    
*        0             0           0          0          0         0
*
* ATA_CMD_IDLE_IMMEDIATE      
*        0             0           0          0          0         0
*
* ATA_CMD_SECURITY_DISABLE_PASSWORD 
*    ATA_ZERO     ATA_ZERO    ATA_ZERO    ATA_ZERO   ATA_ZERO     ATA_ZERO  
*
* ATA_CMD_SECURITY_ERASE_PREPARE    
*        0             0           0          0          0         0
*
* ATA_CMD_SECURITY_ERASE_UNIT       
*    ATA_ZERO     ATA_ZERO    ATA_ZERO    ATA_ZERO   ATA_ZERO     ATA_ZERO
*
* ATA_CMD_SECURITY_FREEZE_LOCK      
*        0             0           0          0          0         0
*
* ATA_CMD_SECURITY_SET_PASSWORD     
*        0             0           0          0          0         0
*
* ATA_CMD_SECURITY_UNLOCK           
*        0             0           0          0          0         0
*
* ATA_CMD_SMART  (not implemented)      
*       FR          SC          SN        ATA_ZERO   ATA_ZERO     ATA_ZERO
*  (SUBCOMMAND) (SubCommand   (SubCommand   
*             Specific Value)  Specific Value)   
*
* ATA_CMD_GET_MEDIA_STATUS   
*        0             0           0          0          0         0
*
* ATA_CMD_MEDIA_EJECT        
*        0             0           0          0          0         0
* 
* ATA_CMD_MEDIA_LOCK         
*        0             0           0          0          0         0
*
* ATA_CMD_MEDIA_UNLOCK       
*        0             0           0          0          0         0
*
* ATA_CMD_CFA_ERASE_SECTORS  
*        0             0           0          0          0         0
*
* ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE 
*     ATA_ZERO        SC      ATA_ZERO    ATA_ZERO   ATA_ZERO     ATA_ZERO
*
* ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE
*     ATA_ZERO        SC      ATA_ZERO    ATA_ZERO   ATA_ZERO     ATA_ZERO
*   
* ATA_CMD_CFA_TRANSLATE_SECTOR
*     ATA_ZERO    ATA_ZERO        SN         cylLo     cylHi       DH
*
* ATA_CMD_CFA_REQUEST_EXTENDED_ERROR_CODE
*     ATA_ZERO    ATA_ZERO    ATA_ZERO    ATA_ZERO   ATA_ZERO     ATA_ZERO
*  
* ATA_CMD_SET_MAX      
*       FR        ATA_ZERO    ATA_ZERO    ATA_ZERO   ATA_ZERO     ATA_ZERO
*     (SUBCOMMAND) 
* \be
* 
* The following are the subcommands valid for ATA_CMD_SET_MAX and are 
* tabulated as  below                                                                     
*
*\bs
* ---------------------------------------------------------------------  
* SUBCOMMAND(in ARG0)
*       ARG1    |       ARG2    |       ARG3    |       ARG4    |      ARG5
* ---------------------------------------------------------------------
*
* ATA_SUB_SET_MAX_ADDRESS        
*       SC      sector no   cylLo       cylHi       head + modebit
* (SET_MAX_VOLATILE
*       or           
* SET_MAX_NON_VOLATILE)
*                      
* ATA_SUB_SET_MAX_SET_PASS     
*       ATA_ZERO        ATA_ZERO        ATA_ZERO        ATA_ZERO       ATA_ZERO 
*
* ATA_SUB_SET_MAX_LOCK         
*       ATA_ZERO        ATA_ZERO        ATA_ZERO        ATA_ZERO       ATA_ZERO
*
* ATA_SUB_SET_MAX_UNLOCK       
*       ATA_ZERO        ATA_ZERO        ATA_ZERO        ATA_ZERO       ATA_ZERO
*
* ATA_SUB_SET_MAX_FREEZE_LOCK  
*       ATA_ZERO        ATA_ZERO        ATA_ZERO        ATA_ZERO       ATA_ZERO
* 
* \be
*
* In ATA_CMD_SET_FEATURE  subcommand only arg0 and arg1 are valid, all 
* other are  ATA_ZERO. 
* \bs
* ------------------------------------------------------
* SUBCOMMAND(ARG0)                          ARG1
* ------------------------------------------------------
*
* ATA_SUB_ENABLE_8BIT                       ATA_ZERO
*
* ATA_SUB_ENABLE_WCACHE                     ATA_ZERO 
*
* ATA_SUB_SET_RWMODE                        mode                
*                                    (see page no 168 table 28 in atapi Spec5 )
* ATA_SUB_ENB_ADV_POW_MNGMNT                0x90
*
* ATA_SUB_ENB_POW_UP_STDBY                  ATA_ZERO
*
* ATA_SUB_POW_UP_STDBY_SPIN                 ATA_ZERO
*
* ATA_SUB_BOOTMETHOD                        ATA_ZERO
*
* ATA_SUB_ENA_CFA_POW_MOD1                  ATA_ZERO
*
* ATA_SUB_DISABLE_NOTIFY                    ATA_ZERO
*
* ATA_SUB_DISABLE_RETRY                     ATA_ZERO
*
* ATA_SUB_SET_LENGTH                        ATA_ZERO    
*
* ATA_SUB_SET_CACHE                         ATA_ZERO
*
* ATA_SUB_DISABLE_LOOK                      ATA_ZERO
*
* ATA_SUB_ENA_INTR_RELEASE                  ATA_ZERO
* 
* ATA_SUB_ENA_SERV_INTR                     ATA_ZERO
*
* ATA_SUB_DISABLE_REVE                      ATA_ZERO
*
* ATA_SUB_DISABLE_ECC                       ATA_ZERO
*                                              
* ATA_SUB_DISABLE_8BIT                      ATA_ZERO
*                                              
* ATA_SUB_DISABLE_WCACHE                    ATA_ZERO
*                                              
* ATA_SUB_DIS_ADV_POW_MNGMT                 ATA_ZERO         
*                                              
* ATA_SUB_DISB_POW_UP_STDBY                 ATA_ZERO
*                                              
* ATA_SUB_ENABLE_ECC                        ATA_ZERO
*                                              
* ATA_SUB_BOOTMETHOD_REPORT                 ATA_ZERO                                     
*                                              
* ATA_SUB_DIS_CFA_POW_MOD1                  ATA_ZERO
*                                              
* ATA_SUB_ENABLE_NOTIFY                     ATA_ZERO         
*                                              
* ATA_SUB_ENABLE_RETRY                      ATA_ZERO
*                                              
* ATA_SUB_ENABLE_LOOK                       ATA_ZERO
*                                              
* ATA_SUB_SET_PREFETCH                      ATA_ZERO
*
* ATA_SUB_SET_4BYTES                        ATA_ZERO    
*                                              
* ATA_SUB_ENABLE_REVE                       ATA_ZERO
*                                              
* ATA_SUB_DIS_INTR_RELEASE                  ATA_ZERO
*
* ATA_SUB_DIS_SERV_INTR                     ATA_ZERO                                                  
*\be
*
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

STATUS ataCmd
    (
    int ctrl,     /* Controller number. 0 or 1 */
    int drive,    /* Drive number.      0 or 1 */
    int cmd,      /* Command Register          */
    int arg0,     /* argument0 */
    int arg1,     /* argument1 */
    int arg2,     /* argument2 */
    int arg3,     /* argument3 */
    int arg4,     /* argument4 */
    int arg5      /* argument5 */
    )
    {
    ATA_CTRL   *pCtrl    = &ataCtrl[ctrl];
    ATA_DRIVE * pDrive   = &pCtrl->drive[drive];
    ATAPI_TYPE  * pType  = pDrive->driveInfo;
    BOOL        retry    = TRUE;
    int       retryCount = 0;
    int       semStatus;
    UINT8     error = 0;

    ATA_DEBUG_MSG(6, "ataCmd %d/%d: cmd=%#x \n",
                  ctrl, drive, cmd, 0, 0, 0);
    ATA_DEBUG_MSG(12, "      args= %#x %#x %#x %#x %#x %#x\n",
                  arg0, arg1, arg2, arg3, arg4, arg5);

    while (retry)
        {
        ataDeviceSelect(pCtrl, drive);
	ATA_DEBUG_MSG(6, "ataCmd: %d/%d beginning of while\n", ctrl,drive,
                          0,0,0,0);
        switch (cmd)
            {
            case ATA_CMD_INITP:
                if (pDrive->type == ATA_TYPE_ATAPI)
                    {
                    /* command not allowed for packet device */
                    ATA_DEBUG_MSG(1,"ataCmd: %d/%d INIT PARAM command"
                                  " not allowed\n",
                                  ctrl, drive, 0,0,0,0);
                    return(OK);
                    }
                ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, pType->sectors);
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT) | 
                                   ((pType->heads - 1) & 0x0f));
                ATA_DEBUG_MSG (4,"\n"
                               "\n ATAPI_SECCNT_INTREASON = %#x"
                               "\n ATAPI_SDH_D_SELECT     = %#x\n",
                               pType->sectors, 
                               (drive << ATA_DRIVE_BIT) | ((pType->heads - 1) & 0x0f), 0,0,0,0);

                break;

            case ATA_CMD_RECALIB:
                /* 
                 * ATA_CMD_RECALIB is not in ATAPI5.
                 * But for downward compatibility
                 */
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                ATA_DEBUG_MSG (4," ATAPI_SDH_D_SELECT = %#x\n",
                               (drive << ATA_DRIVE_BIT), 0, 0, 0, 0, 0);
                break;

            case ATAPI_CMD_SRST:
                if (pDrive->type == ATA_TYPE_ATA)
                    {
                    /* command not allowed for non-packet device */
                    ATA_DEBUG_MSG(1,"ataCmd: %d/%d DEVICE RESET command"
                                  " not allowed\n",
                                  ctrl, drive, 0,0,0,0);
                    return(OK);
                    }
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                break;

            case ATA_CMD_DIAGNOSE:
                break;

            case ATA_CMD_SEEK:
                if (pDrive->type == ATA_TYPE_ATA)
                    {
                    ATA_IO_BYTE_WRITE (ATAPI_CYLLOW, arg0);
                    ATA_IO_BYTE_WRITE (ATAPI_CYLHI, arg0>>8);
                    ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, 
                                       pDrive->okLba | (drive << ATA_DRIVE_BIT) | (arg1 & 0xf));
                    }
                else
                    {
                    ATA_DEBUG_MSG (2,"ataCmd: SEEK command not supported for ATAPI"
                                   " devices.\n", 0, 0, 0, 0, 0, 0);
                    return(ERROR);
                    }
                break;

            case ATA_CMD_SET_FEATURE:
                ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, arg1);
                ATA_IO_BYTE_WRITE (ATAPI_FEATURE, arg0);
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                retry = FALSE;  /* if it fails, it is not supported, don't retry */
                break;

            case ATA_CMD_SET_MULTI:
                if (pDrive->type == ATA_TYPE_ATA)
                    {
                    ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, arg0);
                    ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                    }
                else
                    {
                    ATA_DEBUG_MSG (2,"ataCmd: SEEK command not supported for ATAPI"
                                   " devices.\n", 0, 0, 0, 0, 0, 0);
                    return(ERROR);
                    }
                break;

                /* ATAPI_PWR_MNGMT */
            case ATA_CMD_IDLE:
            case ATA_CMD_STANDBY:
                /* arg0-Timer Period */
                ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, (UINT8)arg0);
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                retry = 0;   /* device doesn't support if error, no retry */
                break;

            case ATA_CMD_STANDBY_IMMEDIATE:
            case ATA_CMD_IDLE_IMMEDIATE:
            case ATA_CMD_SLEEP:
            case ATA_CMD_CHECK_POWER_MODE:
            case ATA_CMD_FLUSH_CACHE:
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                break;


                /* ATAPI_SECURITY_FEATURE */

            case ATA_CMD_SECURITY_ERASE_PREPARE:
            case ATA_CMD_SECURITY_FREEZE_LOCK:
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                break;

            case ATA_CMD_SECURITY_ERASE_UNIT:
            case ATA_CMD_SECURITY_DISABLE_PASSWORD:
            case ATA_CMD_SECURITY_SET_PASSWORD:
            case ATA_CMD_SECURITY_UNLOCK:
                break;


                /* ATAPI_SMART_FEATURE */

            case ATA_CMD_SMART:

                /*
                 * feature register = arg0 = smart commands.
                 *
                 * ATA_SMART_READ_DATA         0XD0
                 * ATA_SMART_ATTRIB_AUTO       0XD2
                 * ATA_SMART_SAVE_ATTRIB       0XD3
                 * ATA_SMART_OFFLINE_IMMED     0XD4
                 * ATA_SMART_READ_LOG_SECTOR   0XD5
                 * ATA_SMART_WRITE_LOG_SECTOR  0XD6
                 * ATA_SMART_ENABLE_OPER       0XD8
                 * ATA_SMART_DISABLE_OPER      0XD9
                 * ATA_SMART_RETURN_STATUS     0XDA
                 */
                switch (arg0)
                    {
                    case ATA_SMART_ATTRIB_AUTO:
                        ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, arg1);

                        /*
                         * arg1=
                         * ATA_SMART_SUB_ENABLE_ATTRIB_AUTO    0xf1
                         * ATA_SMART_SUB_DISABLE_ATTRIB_AUTO   0x00
                         */

                        break;

                    case ATA_SMART_READ_LOG_SECTOR:
                    case ATA_SMART_WRITE_LOG_SECTOR:

                        /*
                         * arg1 = No of sectors to read : Sector count.
                         * arg2 = LogAddress : Sector No.
                         *
                         * Log address |     content            |  R/W
                         *-------------|------------------------|------
                         *     00      | Log Directory          |   RO
                         *     01      | SMART error Log        |   RO
                         *     06      | SMART self test log    |   RO
                         *    80-9f    | Host Vendor specific   |   R/W
                         *    A0-BF    | Device Vendor specific |   VS
                         */

                        ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, arg1);
                        ATA_IO_BYTE_WRITE (ATAPI_SECTOR, arg2);
                        break;

                    case ATA_SMART_OFFLINE_IMMED:

                        /*
                         * arg1 = subcmd
                         *
                         * SubCommands:
                         *  ATA_SMART_SUB_EXEC_OFF_IMMED_OFF_MODE           0
                         *  ATA_SMART_SUB_EXEC_SHORT_SELF_IMMED_OFF_MODE    1
                         *  ATA_SMART_SUB_EXEC_EXT_SELF_IMMED_OFF_MODE      2
                         *  ATA_SMART_SUB_ABORT_OFF_MODE_SELF_IMMED         127
                         *  ATA_SMART_SUB_EXEC_SHORT_SELF_IMMED_CAP_MODE    129
                         *  ATA_SMART_SUB_EXEC_EXT_SELF_IMMED_CAP_MODE      130
                         */

                        ATA_IO_BYTE_WRITE (ATAPI_SECTOR, arg1);
                    case ATA_SMART_RETURN_STATUS:
                    case ATA_SMART_ENABLE_OPER:
                    case ATA_SMART_SAVE_ATTRIB:
                    case ATA_SMART_READ_DATA:
                    case ATA_SMART_DISABLE_OPER:
                        break;
                    default:
                        logMsg ("Incorrect SMART command.",0,0,0,0,0,0);
                        return(ERROR);
                    }

                /* 
                 * For all smart commands cylinder low and cylinder high are to 
                 * written with 0x4fh and 0xc2h respectively
                 */

                ATA_IO_BYTE_WRITE (ATAPI_CYLLOW, 0X4F);
                ATA_IO_BYTE_WRITE (ATAPI_CYLHI, 0XC2);
                ATA_IO_BYTE_WRITE (ATAPI_FEATURE, arg0);
                break;

            case ATA_CMD_GET_MEDIA_STATUS:
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                break;

            case ATA_CMD_MEDIA_EJECT:
            case ATA_CMD_MEDIA_LOCK:
            case ATA_CMD_MEDIA_UNLOCK:
                if (pDrive->type == ATA_TYPE_ATA)
                    ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
                else
                    {
                    ATA_DEBUG_MSG (2,"ataCmd: Media commands not supported for ATAPI"
                                   " devices.\n", 0, 0, 0, 0, 0, 0);
                    return(ERROR);
                    }
                break;

            case ATA_CMD_CFA_ERASE_SECTORS:
            case ATA_CMD_CFA_WRITE_MULTIPLE_WITHOUT_ERASE:
            case ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE:
                ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, arg1);
            case ATA_CMD_CFA_TRANSLATE_SECTOR:
                ATA_IO_BYTE_WRITE (ATAPI_SECTOR, arg2);
                ATA_IO_BYTE_WRITE (ATAPI_CYLLOW, arg3);
                ATA_IO_BYTE_WRITE (ATAPI_CYLHI, arg4);
                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, 
                    pDrive->okLba | (arg5 & 0x0f) | (drive << ATA_DRIVE_BIT));
            case ATA_CMD_CFA_REQUEST_EXTENDED_ERROR_CODE:
                break;


                /* HOST_PROT_AREA_FEATURE */
            case ATA_CMD_READ_NATIVE_MAX_ADDRESS:

                /*
                 * arg5 is expected mode of native max address.
                 * ATA_SDH_IBM = CHS mode
                 * ATA_SDH_LBA = LBA mode
                 */

                ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, 
                    pDrive->okLba | (arg5 & 0x0f) | (drive << ATA_DRIVE_BIT));
                /* CHECK use of LBA here **DOUG  */
                break;

            case ATA_CMD_SET_MAX:
                switch (arg0)
                    {
                    case ATA_SUB_SET_MAX_ADDRESS:

                        /*
                         * arg1 indicates volatility of this setting
                         *      SET_MAX_VOLATILE      0x00
                         *      SET_MAX_NON_VOLATILE  0x01
                         * arg2 is sector no  / LBA 7:0
                         * arg3 cylLo         / LBA 15:8
                         * arg4 cylHi         / LBA 23:16
                         * arg5 (head         / LBA 27:24   )  + mode bit
                         */

                        ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, arg1);
                        ATA_IO_BYTE_WRITE (ATAPI_SECTOR, arg2);
                        ATA_IO_BYTE_WRITE (ATAPI_CYLLOW,  arg3);
                        ATA_IO_BYTE_WRITE (ATAPI_CYLHI,  arg4);
                        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, 
                            pDrive->okLba | (arg5 & 0x0f) | (drive >> ATA_DRIVE_BIT));
                        break;
                    case ATA_SUB_SET_MAX_SET_PASS:
                    case ATA_SUB_SET_MAX_LOCK:
                    case ATA_SUB_SET_MAX_UNLOCK:
                    case ATA_SUB_SET_MAX_FREEZE_LOCK:
                        break;
                    default:
                        logMsg ("Incorrect SET MAX command.",0,0,0,0,0,0);
                        return(ERROR);
                    }
                ATA_IO_BYTE_WRITE (ATAPI_FEATURE, arg0);
                break;

            case ATA_CMD_READ_VERIFY_SECTORS:
                break;

            case ATA_CMD_NOP:

                /*
                 * arg0 is subcommand.
                 * 00- abort outstanding command,
                 * 01-don't abort outstanding command
                 */

                ATA_IO_BYTE_WRITE (ATAPI_FEATURE, arg0);
                break;

            default:
                ATA_DEBUG_MSG(1,"ataCmd: Bad ATA command 0x%x\n", cmd, 0,0,0,0,0);
                return(OK);
            }

        ATA_IO_BYTE_WRITE (ATAPI_COMMAND, cmd);
        semStatus = semTake (&pCtrl->syncSem, 
                             sysClkRateGet() * pCtrl->semTimeout);

        if ((pCtrl->intStatus & ATA_STAT_ERR) || (semStatus == ERROR))
            {
            if (pCtrl->intStatus & ATA_STAT_ERR)
                {
                error = ATA_IO_BYTE_READ (ATAPI_ERROR); 
                if (error & ERR_ABRT)
                    {
                    /* command couldn't complete, return without retry */

                    ATA_DEBUG_MSG (1, "ataCmd: cmd aborted by device, returning\n"
                                       "  status=0x%x semStatus=%d err=0x%x\n",
                                       pCtrl->intStatus, semStatus, error, 0, 0, 0);
                    return (ERROR);
                    }
                }
            else
                ATA_DEBUG_MSG (1, "ataCmd: error - retrying  status=0x%x semStatus=%d err=0x%x\n",
                           pCtrl->intStatus, semStatus,
                           error, 0, 0, 0);

            if (++retryCount > ataRetry)
                return(ERROR);
            }
        else
            retry = FALSE;
        }

    /* Post command operations or failure messages */
    switch (cmd)
        {
        case ATA_CMD_SEEK:
            if (ataStatusChk(pCtrl, ATA_STAT_SEEKCMPLT, ATA_STAT_SEEKCMPLT) != OK)
                {
                return (ERROR);
                }
            break;

        case ATA_CMD_DIAGNOSE:
            if (ataStatusChk(pCtrl, ATA_STAT_BUSY | ATA_STAT_READY | ATA_STAT_DRQ,
                         !ATA_STAT_BUSY | ATA_STAT_READY | !ATA_STAT_DRQ) != OK)
                {
                return (ERROR);
                }
            pDrive->signature = ((ATA_IO_BYTE_READ(ATAPI_SECCNT_INTREASON) << 24)  | 
                                 (ATA_IO_BYTE_READ(ATAPI_SECTOR) << 16)  |
                                 (ATA_IO_BYTE_READ(ATAPI_CYLLOW)  << 8)   | 
                                 ATA_IO_BYTE_READ(ATAPI_CYLHI));
            ATA_DEBUG_MSG(1,"ataCmd: DIAGNOSTIC signature = 0x%x\n",
                          pDrive->signature, 0,0,0,0,0);
            break;
        case ATA_CMD_CHECK_POWER_MODE:
            pDrive->checkPower = ATA_IO_BYTE_READ(ATAPI_SECCNT_INTREASON);
            ATA_DEBUG_MSG(1,"ataCmd: Power Mode = 0x%x\n",
                          pDrive->checkPower, 0,0,0,0,0);
            break;

#ifdef REMOV_MEDIA_FEATURE
        {
        UINT8   tempError;
        case ATA_CMD_MEDIA_EJECT:
        case ATA_CMD_MEDIA_LOCK:
        case ATA_CMD_MEDIA_UNLOCK:
            tempError = ATA_IO_BYTE_READ(ATAPI_ERROR);
            if (tempError)
                {
                if (cmd == ATA_CMD_MEDIA_LOCK)
                    if (tempError & 8)
                        logMsg("Media change Request",0,0,0,0,0,0);
                if (tempError & 4)
                    logMsg("Aborted",0,0,0,0,0,0);
                if (tempError&2)
                    logMsg("No media present",0,0,0,0,0,0);
                }
            break;
        }
#endif /* REMOV_MEDIA_FEATURE */

#ifdef ATA_SMART_FEATURE
        case ATA_CMD_SMART:
            switch (arg0)       /* 
                                 * this can be optimised using an
                                 * "if" instead of "switch"
                                 */
                {
                case ATA_SMART_RETURN_STATUS:
                    if (((ATA_IO_BYTE_READ(ATAPI_CYLLOW)) == 
                         0xF4)|| ((ATA_IO_BYTE_READ(ATAPI_CYLHI))
                                  == 0x2C))
                        logMsg("SMART RETURN STATUS: Threshold Exceeded",
                               0,0,0,0,0,0);
                    break;
                case ATA_SMART_ATTRIB_AUTO:
                case ATA_SMART_READ_LOG_SECTOR:
                case ATA_SMART_WRITE_LOG_SECTOR:
                case ATA_SMART_OFFLINE_IMMED:
                case ATA_SMART_ENABLE_OPER:
                case ATA_SMART_SAVE_ATTRIB:
                case ATA_SMART_READ_DATA:
                case ATA_SMART_DISABLE_OPER:
                default:
                }
            break;
#endif /* ATA_SMART_FEATURE */

#ifdef HOST_PROT_AREA_FEATURE
        case ATA_CMD_READ_NATIVE_MAX_ADDRESS:
            pDrive->nativeMaxAdd [3] = 
            ATA_IO_BYTE_READ(ATAPI_SDH_D_SELECT) ;
            pDrive->nativeMaxAdd [2] =
            ATA_IO_BYTE_READ(ATAPI_CYLHI);
            pDrive->nativeMaxAdd [1] = 
            ATA_IO_BYTE_READ(ATAPI_CYLLOW);
            pDrive->nativeMaxAdd [0] = ATA_IO_BYTE_READ(ATAPI_SECTOR);
            logMsg ("Native Max Address(LBA 0xe0/ CHS 0xa0) for sdh of %#x is\n"
                    "LBA 27:24 / Head      = %#x \n"
                    "LBA 23:16 / cylHi     = %#x \n"
                    "LBA 15:8  / cylLow    = %#x \n"
                    "LBA 7:0   / sector no = %#x \n", 
                    arg5,
                    pDrive->nativeMaxAdd[3],pDrive->nativeMaxAdd [2],
                    pDrive->nativeMaxAdd[1],pDrive->nativeMaxAdd [0],
                    ATA_ZERO);
            break;
        case ATA_CMD_SET_MAX:
            switch (arg0)
                {
                case ATA_SUB_SET_MAX_ADDRESS:
                    pDrive->nativeMaxAdd [3] =
                    ATA_IO_BYTE_READ (ATAPI_SDH_D_SELECT);
                    pDrive->nativeMaxAdd [2] =
                    ATA_IO_BYTE_READ (ATAPI_CYLHI);
                    pDrive->nativeMaxAdd [1] =
                    ATA_IO_BYTE_READ (ATAPI_CYLLOW);
                    pDrive->nativeMaxAdd [0] =
                    ATA_IO_BYTE_READ (ATAPI_SECTOR);
                    logMsg ("Native Max Address (LBA 0xe0/ CHS 0xa0) "
                            "for sdh of %#x is \n"
                            "LBA 27:24 / Head      = %#x \n"
                            "LBA 23:16 / cylHi     = %#x \n"
                            "LBA 15:8  / cylLow    = %#x \n"
                            "LBA 7:0   / sector no = %#x \n",arg5,
                            pDrive->nativeMaxAdd [3],
                            pDrive->nativeMaxAdd [2],
                            pDrive->nativeMaxAdd [1],
                            pDrive->nativeMaxAdd [0], ATA_ZERO);
                    break;
                case ATA_SUB_SET_MAX_SET_PASS:
                case ATA_SUB_SET_MAX_LOCK:
                case ATA_SUB_SET_MAX_UNLOCK:
                case ATA_SUB_SET_MAX_FREEZE_LOCK:
                default:
                }
            break;
#endif /* HOST_PROT_AREA_FEATURE */

#ifdef ATA_CFA_FEATURE
        case ATA_CMD_CFA_REQUEST_EXTENDED_ERROR_CODE:
            /*
             * The Error Register value gives its error code.
             * Table 16, 8.2.5, Ref-1
             */
            if ((ATA_IO_BYTE_READ(C)) & 0x01)/* 0x20 */
                pDrive->CFAerrorCode = ATA_IO_BYTE_READ(ATAPI_ERROR);
            break;
        case ATA_CMD_CFA_ERASE_SECTORS:
        case ATA_CMD_CFA_WRITE_MULTIPLE_WITHOUT_ERASE:
        case ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE:

            /* check if any error is occured */

            temp = ATA_IO_BYTE_READ(pCtrl->status);
            if (temp & 0x01)
                {
                ATA_DEBUG_MSG (2,"%#x Command  failed. status = %#x \n",
                               cmd,temp,0,0,0,0);/*
                                              * this is not necessary if
                                              * status can be read more
                                              * than one time
                                              */
                /* check if media bit in error register is set */

                if ((ATA_IO_BYTE_READ(ATAPI_ERROR)) & 0x01)
                    {
                    logMsg ("Media error occured at the sector \n"
                            "sector Number / LBA (7:0)   %#x \n"
                            "Cylinder Low  / LBA (15:8)  %#x \n"
                            "Cylinder High / LBA (23:16) %#x \n"
                            "Head Number   / LBA (27:24) %#x \n",
                            ATA_IO_BYTE_READ(ATAPI_SECTOR),
                            ATA_IO_BYTE_READ(ATAPI_CYLLOW),
                            ATA_IO_BYTE_READ(ATAPI_CYLHI),
                            ATA_IO_BYTE_READ(ATAPI_SDH_D_SELECT),
                            0,0);
                    }
                }
            break;  
        case ATA_CMD_CFA_TRANSLATE_SECTOR:
            break;
#endif /* ATA_CFA_FEATURE */

        case ATA_CMD_NOP:
            logMsg("NOP Command aborted",0,0,0,0,0,0);
            break;

        case ATA_CMD_READ_VERIFY_SECTORS:
        case ATA_CMD_FLUSH_CACHE:
            break;

        default:
            break;
        }

    ATA_DEBUG_MSG (2, "ataCmd end - ctrl %d, drive %d: Ok\n", ctrl, drive, 0, 0, 0, 0);

    return(OK);
    }


/**************************************************************************
*
* ataInit - initialize ATA device.
*
* This routine issues a soft reset command to ATA device for initialization.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

STATUS ataInit
    (
    int ctrl,
    int drive
    )
    {
    ATA_CTRL * pCtrl = &ataCtrl[ctrl];
    int        ix;
    int        device_signature;

    ATA_DEBUG_MSG (9, "ataInit: ctrl=%d\n", ctrl, 0, 0, 0, 0, 0);

    /* Set reset bit */

    ATA_IO_BYTE_WRITE (ATAPI_D_CONTROL, ATA_CTL_SRST);

    for (ix = 0; ix < 20; ix++)     /* >= 5 mks */
        ATA_DELAY_400NSEC;  /* sysDelay() */

    /* Clear reset bit */

    ATA_IO_BYTE_WRITE (ATAPI_D_CONTROL, 0);

    for (ix = 0; ix < 5000; ix++)   /* 2 ms */
        ATA_DELAY_400NSEC;   /* sysDelay() */

    pCtrl->wdgOkay = TRUE;

    /* Start the ATA  watchdog */

    if (!coreIsDumping)
        wdStart (pCtrl->wdgId, (sysClkRateGet() * pCtrl->wdgTimeout),
             (FUNCPTR)ataWdog, ctrl);

    /* Wait for BUSY bit to be cleared */

    while ((ATA_IO_BYTE_READ (ATAPI_ASTATUS) & ATA_STAT_BUSY)
           && (pCtrl->wdgOkay))
        ;

    /* Stop the ATA watchdog */

    if (!coreIsDumping)
	{
	wdCancel (pCtrl->wdgId);

        if (!pCtrl->wdgOkay)
	    {
	    ATA_DEBUG_MSG (2, "ataInit error:\n", 0, 0, 0, 0, 0, 0);
	    pCtrl->wdgOkay = TRUE;
	    return(ERROR);
	    }
        }

    ATA_DEBUG_MSG (3, "ataInit: Calling sysAtaInit (if present):\n",
                   0, 0, 0, 0, 0, 0);

    /* Call out to bsp specific setup routine */

    SYS_ATA_INIT_RTN (pCtrl);

    /*
     * The following allows recovery after an interrupt
     * caused by drive software reset
     */

    semBInit(&pCtrl->syncSem, SEM_Q_FIFO, SEM_EMPTY);

    ATA_DEBUG_MSG (2, "ataInit end\n", 0, 0, 0, 0, 0, 0);
    while ((ATA_IO_BYTE_READ (ATAPI_ASTATUS) & ATA_STAT_BUSY))
        ;
    device_signature = ((ATA_IO_BYTE_READ(ATAPI_SECCNT_INTREASON) << 24)  | 
                        (ATA_IO_BYTE_READ(ATAPI_SECTOR) << 16)  |
                        (ATA_IO_BYTE_READ(ATAPI_CYLLOW) << 8)    | 
                        ATA_IO_BYTE_READ(ATAPI_CYLHI));
    if (device_signature == ATA_SIGNATURE)
        {
        ATA_DEBUG_MSG(2, "ataInit: Reset: ATA device=0x%x\n",
                      device_signature,0,0,0,0,0);
        }
    else if (device_signature == ATAPI_SIGNATURE)
        {
        ATA_DEBUG_MSG(2, "ataInit: Reset: ATAPI device=0x%x\n",
                      device_signature,0,0,0,0,0);
        }
    else
        ATA_DEBUG_MSG(2, "ataInit: Reset: unknown device=0x%x\n",
                      device_signature,0,0,0,0,0);

    return(OK);
    }
/**************************************************************************
*
* ataRW - read/write a data from/to required sector.
*
* Read/write a number of sectors on the current track
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

STATUS ataRW
    (
    int       ctrl,
    int       drive,
    UINT32    cylinder,
    UINT32    head,
    UINT32    sector,
    void    * buffer,
    UINT32    nSecs,
    int       direction,
    sector_t  startBlk
    )
    {
    ATA_CTRL   * pCtrl      = &ataCtrl[ctrl];
    ATA_DRIVE  * pDrive     = &pCtrl->drive[drive];
    ATAPI_TYPE * pType    = pDrive->driveInfo;
    int          retryCount = 0;
    UINT32       block      = 1;
    UINT32       nSectors   = nSecs;
    UINT32       nWords;
    int          semStatus;
    short      * pBuf;
    UINT8	 cmd;

    ATA_DEBUG_MSG (15, "ataRW %d/%d c=%d h=%d s=%d buf=%#x \n",
                   ctrl, drive, cylinder, head, sector, (int)buffer);
    ATA_DEBUG_MSG (15, "        n=%d dir=%d\n", nSecs, direction, 0, 0, 0, 0);

    if (buffer == NULL)
        return(ERROR);

    while (++retryCount <= ataRetry)
        {
        /* wait for device idle: DRQ=0,BSY=0 */
        if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 0) != OK)
            {
            return (ERROR);
            }
        pBuf = buffer;

        /*
         * The following code determines if the disk drive supports 48bit logical
         * block addressing and uses that during the read or write operation.
         * Internally the controller has FIFO's at the traditional task register
         * locations.  If the drive supports 48bit LBA, 2 register writes are
         * made to the appropriate registers and the appropriate command issued.
         * The sector count is always written as 2 bytes.  If 48 bit LBA is
         * not supported, the double write has no effect since the final register
         * value is the correct one.
         */
        ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, nSecs >> 8    );
        ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, nSecs         );
        if (pDrive->use48LBA == TRUE)
           {
            ATA_IO_BYTE_WRITE (ATAPI_SECTOR,       (UINT8)((startBlk >> 24) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_SECTOR,       (UINT8)(startBlk & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, (UINT8)((startBlk >> 32) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, (UINT8)((startBlk >> 8) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,  (UINT8)((startBlk >> 40) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,  (UINT8)((startBlk >> 16) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT,
                               pDrive->okLba  | (drive << ATA_DRIVE_BIT));
           }
	else
	    {
	    ATA_IO_BYTE_WRITE (ATAPI_SECTOR,           sector        );
	    ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, cylinder      );
	    ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,  cylinder >> 8 );
	    ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT,
		    pDrive->okLba  | (drive << ATA_DRIVE_BIT) | (head & 0xf));
	    }

        ATA_DEBUG_MSG (15, "\n            Sector Count  = %#x"
                           "\n            Sector        = %#x"
                           "\n            Cyl high      = %#x"
                           "\n            Cyl low       = %#x"
                           "\n            Device/head   = %#x\n",
                           nSecs, sector, cylinder >> 8, cylinder,
                           pDrive->okLba | (drive << ATA_DRIVE_BIT) | (head & 0xf), 0);

        if (pDrive->rwPio == ATA_PIO_MULTI)
            block = pDrive->multiSecs;

        nWords = (pType->bytes * block) >> 1;

        if (direction == O_WRONLY)
            {
            if (pDrive->rwPio == ATA_PIO_MULTI)
                {
                if (pDrive->use48LBA == TRUE)
                    cmd = ATA_CMD_WRITE_MULTI_EXT;
                else
                    cmd = ATA_CMD_WRITE_MULTI;
                }
            else
                {
                if (pDrive->use48LBA == TRUE)
                    cmd = ATA_CMD_WRITE_EXT;
                else
                    cmd = ATA_CMD_WRITE;
                }

            ATA_IO_BYTE_WRITE (ATAPI_COMMAND, cmd);

            while (nSectors > 0)
                {
                if ((pDrive->rwPio == ATA_PIO_MULTI) && (nSectors < block))
                    {
                    block = nSectors;
                    nWords = (pType->bytes * block) >> 1;
                    }

                /* Wait for DRQ and !BSY */
                if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 
                          ATA_STAT_DRQ | !ATA_STAT_BUSY) != OK)
                    {
                    return (ERROR);
                    }

                if (pDrive->rwBits == ATA_BITS_16)
                    ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)pBuf, nWords);
                else
                    ATA_IO_NLONG_WRITE (ATAPI_DATA, (void *)pBuf, nWords >> 1);

                semStatus = semTake (&pCtrl->syncSem,
                                     sysClkRateGet() * pCtrl->semTimeout);
                if ((pCtrl->intStatus & ATA_STAT_ERR) || (semStatus == ERROR))
                    goto errorRW;

                pBuf     += nWords;
                nSectors -= block;
                }
            }
        else
            {
            if (pDrive->rwPio == ATA_PIO_MULTI)
                {
                if (pDrive->use48LBA == TRUE)
                    cmd = ATA_CMD_READ_MULTI_EXT;
                else
                    cmd = ATA_CMD_READ_MULTI;
                }
            else
                {
                if (pDrive->use48LBA == TRUE)
                    cmd = ATA_CMD_READ_EXT;
                else
                    cmd = ATA_CMD_READ;
                }

            ATA_IO_BYTE_WRITE (ATAPI_COMMAND, cmd);

            while (nSectors > 0)
                {
                if ((pDrive->rwPio == ATA_PIO_MULTI) && (nSectors < block))
                    {
                    block = nSectors;
                    nWords = (pType->bytes * block) >> 1;
                    }

                semStatus = semTake (&pCtrl->syncSem,
                                     sysClkRateGet() * pCtrl->semTimeout);

                if ((pCtrl->intStatus & ATA_STAT_ERR) || (semStatus == ERROR))
                    goto errorRW;

                /* Wait for DRQ and !BSY */
                if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 
                                        ATA_STAT_DRQ | !ATA_STAT_BUSY) != OK)
                    {
                    return (ERROR);
                    }

                if (pDrive->rwBits == ATA_BITS_16)
                    ATA_IO_NWORD_READ (ATAPI_DATA, (void *)pBuf, nWords);
                else
                    ATA_IO_NLONG_READ (ATAPI_DATA, (void *)pBuf, nWords >> 1);

                pBuf     += nWords;
                nSectors -= block;
                }
            }

        ATA_DEBUG_MSG (15, "ataRW: end\n", 0, 0, 0, 0, 0, 0);

        return(OK);

        errorRW:
        ATA_DEBUG_MSG (2, "ataRW err: intStatus=%#x semStatus=%d\n",
                       pCtrl->intStatus, semStatus, 0, 0, 0, 0); 

        }
    return(ERROR);
    } /* ataRW */


/**************************************************************************
*
* ataDmaRW - read/write a number of sectors on the current track in DMA mode
*
* Read/write a number of sectors on the current track in DMA mode
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

STATUS ataDmaRW
    (
    int    ctrl,
    int    drive,
    UINT32 cylinder,
    UINT32 head,
    UINT32 sector,
    void * buffer,
    UINT32 nSecs,
    int    direction,
    sector_t startBlk
    )
    {
    ATA_CTRL  * pCtrl      = &ataCtrl[ctrl];
    ATA_DRIVE * pDrive     = &pCtrl->drive[drive];
    ATAPI_TYPE  * pType      = pDrive->driveInfo;
    int         semStatus;
    char      * pBuf;
    UINT8	cmd;

    if (buffer == NULL)
        return(ERROR);

    ATA_DEBUG_MSG (15, "ataDmaRW %d/%d c=%d h=%d s=%d buf=%#x \n",
                   ctrl, drive, cylinder, head, sector, (int)buffer);
    ATA_DEBUG_MSG (15, "n=%d dir=%d\n", nSecs, direction, 0, 0, 0, 0);

    pBuf = (char *)buffer;

    ATA_DEBUG_MSG (4,"calling ATA_HOST_DMA_ENGINE_SET..\n", 
                       0, 0, 0, 0, 0, 0);

    /* wait for device idle: DRQ=0,BSY=0, DRDY=1 */
    if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_READY, 
                      !ATA_STAT_DRQ | !ATA_STAT_BUSY | ATA_STAT_READY) != OK)
        {
        return (ERROR);
        }

    if (pCtrl->ataDmaSet != NULL)
        {
        if ((*pCtrl->ataDmaSet)(ctrl, drive, pBuf, 
                                         (UINT32)(pType->bytes*nSecs),
                                         direction) != OK)
        return (ERROR);
        }

    /*
     * The following code determines if the disk drive supports 48bit logical
     * block addressing and uses that during the read or write operation.
     * Internally the controller has FIFO's at the traditional task register
     * locations.  If the drive supports 48bit LBA, 2 register writes are
     * made to the appropriate registers and the appropriate command issued.
     * The sector count is always written as 2 bytes.  If 48 bit LBA is
     * not supported, the double write has no effect since the final register
     * value is the correct one.
     */
    ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON,  nSecs >> 8    );
    ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON,  nSecs          );
    if (pDrive->use48LBA == TRUE)
        {
        ATA_IO_BYTE_WRITE (ATAPI_SECTOR,       (UINT8)((startBlk >> 24) & MASK_48BIT));
        ATA_IO_BYTE_WRITE (ATAPI_SECTOR,       (UINT8)(startBlk & MASK_48BIT));
        ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, (UINT8)((startBlk >> 32) & MASK_48BIT));
        ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, (UINT8)((startBlk >> 8) & MASK_48BIT));
        ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,  (UINT8)((startBlk >> 40) & MASK_48BIT));
        ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,  (UINT8)((startBlk >> 16) & MASK_48BIT));
        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT,
                           pDrive->okLba  | (drive << ATA_DRIVE_BIT));
        }
    else
        {
        ATA_IO_BYTE_WRITE (ATAPI_SECTOR,            sector         );
        ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO,  cylinder       );
        ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,   cylinder >> 8  );
        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT,
                          pDrive->okLba | (drive << ATA_DRIVE_BIT) | (head & 0xf));
        }
    if (direction == O_WRONLY)
        {
        if (pDrive->use48LBA == TRUE)
            cmd = ATA_CMD_WRITE_DMA_EXT;
        else
            cmd = ATA_CMD_WRITE_DMA;
        ATA_DEBUG_MSG (4,"O_WRONLY \n", 0, 0, 0, 0, 0, 0);
        }
    else
        {
        if (pDrive->use48LBA == TRUE)
            cmd = ATA_CMD_READ_DMA_EXT;
        else
            cmd = ATA_CMD_READ_DMA;
        ATA_DEBUG_MSG( 4,"O_RDONLY \n", 0, 0, 0, 0, 0, 0);
        }

    ATA_IO_BYTE_WRITE (ATAPI_COMMAND, cmd);

    /* HI4:HDMA0 */
    /* DMA Transfer will be done here */

    /* Do we need to wait here ???
    while ((ATA_IO_BYTE_READ (ATAPI_ASTATUS) &
           (ATA_STAT_BUSY | ATA_STAT_DRQ)) == 00);
    while ((ATA_IO_BYTE_READ (ATAPI_ASTATUS) &
           (ATA_STAT_BUSY | ATA_STAT_DRQ)) == (ATA_STAT_BUSY | ATA_STAT_DRQ));
    */

    if (pCtrl->ataDmaStart != NULL)
        {
        (*pCtrl->ataDmaStart)(ctrl);
        }

    /* HDMA0 : HDMA1 */
    /* All the data transfer happens here */
    /* HDMA1 : HDMA2 */

    ATA_DEBUG_MSG (4,"ataDmaRW: Waiting for semaphore \n", 0, 0, 0, 0, 0, 0);
    semStatus = semTake (&pCtrl->syncSem, ATA_LIKE_WAIT_FOREVER );

    /* Be sure to stop all DMAC's before exiting this call. */
    if (pCtrl->ataDmaStop != NULL)
        {
        (*pCtrl->ataDmaStop) (ctrl);
        }

    if (semStatus == ERROR)
        {
        printErr("Device not responding (No Interrupt)\n");
        ATA_DEBUG_MSG (4,"ataDmaRW: bad semaphore return\n", 0, 0, 0, 0, 0, 0);
        return (ERROR);
        }

    /* HDMA2 : HDMA0 */

    ATA_DEBUG_MSG (4,"ataDmaRW: Got semaphore \n", 0, 0, 0, 0, 0, 0);

    if ((ATA_IO_BYTE_READ (ATAPI_ASTATUS) & (ATA_STAT_BUSY | ATA_STAT_DRQ)))
        {
        ATA_DEBUG_MSG (4,"ataDmaRW: Bad status\n", 0, 0, 0, 0, 0, 0);
        return (ERROR);
        }

    /* HDMA0 : HI0 */

    return (OK);
    }

/*******************************************************************************
*
* ataPiInit - init a ATAPI CD-ROM disk controller
*
* This routine resets a ATAPI CD-ROM disk controller.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

STATUS ataPiInit 
    (
    int ctrl,
    int drive
    )
    {
    ATA_CTRL    *pCtrl = &ataCtrl[ctrl];
    int     retryCount = 0;
    int     i;

    ATA_DEBUG_MSG (2, "ataPiInit%d/%d: \n", ctrl, drive, 0, 0, 0, 0);

    while (TRUE) /* Forever */
        {
        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));

        ATA_WAIT_STATUS;

        ATA_IO_BYTE_WRITE (ATAPI_COMMAND, ATA_PI_CMD_SRST);

        for (i = 0; i < 5000; i++)  /* 2 ms */
            {
            ATA_WAIT_STATUS;
            }

        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));

        ATA_WAIT_STATUS;

        pCtrl->wdgOkay = TRUE;

        wdStart (pCtrl->wdgId, (sysClkRateGet() * pCtrl->wdgTimeout), 
                 (FUNCPTR)ataWdog, ctrl);

        while ( (ATA_IO_BYTE_READ (ATAPI_STATUS) & (ATA_STAT_BUSY | 
                                                    ATA_STAT_READY | ATA_STAT_WRTFLT | ATA_STAT_DRQ | 
                                                    ATA_STAT_ECCCOR | ATA_STAT_ERR)) && (pCtrl->wdgOkay) )
            ;

        wdCancel (pCtrl->wdgId);

        if (pCtrl->wdgOkay)
            break;

        pCtrl->wdgOkay = TRUE;

        ATA_DEBUG_MSG (1, "ataPiInit%d/%d: ERROR: status=0x%x error=0x%x\n", 
                       ctrl, drive, ATA_IO_BYTE_READ(ATAPI_STATUS), 
                       ATA_IO_BYTE_READ(ATAPI_ERROR), 0, 0);

        if (++retryCount == ataRetry)
            return(ERROR);
        }

    /* The following allow to recover after accidental interrupt */

    if (semTake (&pCtrl->syncSem, NO_WAIT) == OK)
        {
        ATA_DEBUG_MSG (2, "ataPiInit%d/%d: WARNING: interrupt cleared: "
                       "status=0x%x dev=0x%x error=0x%x\n", ctrl, drive, 
                       ATA_IO_BYTE_READ(ATAPI_STATUS), 
                       ATA_IO_BYTE_READ(ATAPI_SDH_D_SELECT), 
                       ATA_IO_BYTE_READ(ATAPI_ERROR), 0);
        }
    else
        {
        ATA_DEBUG_MSG (2, "ataPiInit%d/%d: Ok: status=0x%x dev=0x%x error=0x%x"
                       "\n", ctrl, drive, ATA_IO_BYTE_READ(ATAPI_STATUS), 
                       ATA_IO_BYTE_READ(ATAPI_SDH_D_SELECT), 
                       ATA_IO_BYTE_READ(ATAPI_ERROR), 0);
        }

    return(OK);
    } /* ataPiInit */



/**************************************************************************
*
* ataDevIdentify - identify device
*
* This routine checks whether the device is connected to the controller,
* if it is, this routine determines drive type. The routine set `type'
* field in the corresponding ATA_DRIVE structure. If device identification
* failed, the routine set `state' field in the corresponding ATA_DRIVE 
* structure to ATA_DEV_NONE.
*
* RETURNS: TRUE if a device present, FALSE otherwise
*/

STATUS ataDevIdentify
    (
    int ctrl,
    int dev
    )
    {
    ATA_CTRL  * pCtrl   = &ataCtrl[ ctrl];
    ATA_DRIVE * pDrive  = &pCtrl->drive[ dev];

    /* Select device */
    if (ataDeviceSelect(pCtrl, dev) != OK)
        {
        ATA_DEBUG_MSG(1,"ataDevIdentify: device %d not selected.  ERROR\n",
                      dev, 0,0,0,0,0);
        return(ERROR);
        }

    /* 
     * Find if the device is present or not before trying to find if it is
     * ATA or ATAPI. For this one logic is to write some value to a R/W
     * register in the drive and read it back. If we get correct value, 
     * device is present other wise not. But due to the capacitance of the
     * bus (IDE bus), some times we get the correct value even the device is 
     * not present. To solve this problem, write a value to one R/W register
     * in the drive, and write other value to another R/W register in the 
     * drive. Now read the first register.
     */
    ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, 0xaa);
    ATA_IO_BYTE_WRITE (ATAPI_SECTOR, 0x55);

    if (ATA_IO_BYTE_READ (ATAPI_SECCNT_INTREASON) == 0xaa)
        { 
        /* device is present, now find out the type.
         * The signature field has already been filled in by the
         * ataCtrlReset() and is used to get device type.
         */

        if (pDrive->signature == ATAPI_SIGNATURE)
            {
            pDrive->type = ATA_TYPE_ATAPI;
            ATA_DEBUG_MSG(1,"ataDeviceIdentify: Found ATAPI device on %d/%d  "
                            "Params=0x%x\n",
                          ctrl, dev, &pDrive->param ,0,0,0);
            return(OK);
            }
        else if (pDrive->signature == ATA_SIGNATURE)
            {
            pDrive->type = ATA_TYPE_ATA;
            ATA_DEBUG_MSG(1,"ataDeviceIdentify: Found ATA device on %d/%d  "
                            "Params=0x%x\n",
                          ctrl, dev, &pDrive->param ,0,0,0);
            return(OK);
            }
        else
            {
            pDrive->type = ATA_TYPE_NONE;
            pDrive->state = ATA_TYPE_NONE;
            pDrive->signature = 0;
            ATA_DEBUG_MSG(1,"ataDeviceIdentify: Unknown device found on %d/%d\n",
                          ctrl, dev, 0,0,0,0);
            return(ERROR);
            }
        }
    else
        {
        pDrive->type = ATA_TYPE_NONE;
        pDrive->state = ATA_TYPE_NONE;
        pDrive->signature = 0xffffffff;
        ATA_DEBUG_MSG(1,"ataDeviceIdentify: Unknown device found on %d/%d\n",
                      ctrl, dev, 0,0,0,0);
        return(ERROR);
        }
    } /* ataDevIdentify */

/*******************************************************************************
*
* ataParamRead - Read drive parameters
*
* Read drive parameters.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

STATUS ataParamRead
    (
    int      ctrl,
    int      drive,
    void     *buffer,
    int      command
    )
    {
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];
    BOOL     retry  = TRUE;
    int      retryCount = 0;
    int      semStatus;

    ATA_DEBUG_MSG (2, "ataParamRead: ctrl=%d drive=%d\n", ctrl, drive, 0, 0, 0, 0);

    while (retry)
        {
        /* wait for device idle: DRQ=0,BSY=0, DRDY=1 */
        if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY,
                                  !ATA_STAT_DRQ | !ATA_STAT_BUSY) != OK)
            {
            ATA_DEBUG_MSG(1,"ataParamRead status=0x%x\n", ATA_IO_BYTE_READ (ATAPI_ASTATUS),
                          0, 0, 0, 0, 0);
            continue;
            }

        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));
        ATA_IO_BYTE_WRITE (ATAPI_COMMAND, command);
        ATA_DEBUG_MSG(1,"ataParamRead: %d/%d get semaphore, wait for interrupt\n",
                      ctrl, drive, 0,0,0,0);
        semStatus = semTake (&pCtrl->syncSem, 
                             sysClkRateGet() * 2);

        if ((pCtrl->intStatus & ATA_STAT_ERR) || (semStatus == ERROR))
            {

            ATA_DEBUG_MSG (1, "ataParamRead%d/%d - err: status=0x%x intStatus=0x%x "
                           "error=0x%x semStatus=%d\n", ctrl, drive, 
                           ATA_IO_BYTE_READ (ATAPI_ASTATUS), pCtrl->intStatus, 
                           ATA_IO_BYTE_READ (ATAPI_ERROR), semStatus);

            if (++retryCount > ataRetry)
                return(ERROR);
            else
                {
                ATA_DEBUG_MSG(1,"ataParamRead: try to read params again\n",
                              0, 0, 0, 0, 0, 0);
                }
            }
        else
            retry = FALSE;
        }

    /* wait for device idle: DRQ=0,BSY=0, DRDY=1 */
    if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 
                      ATA_STAT_DRQ | !ATA_STAT_BUSY) != OK)
        {
        return (ERROR);
        }

 //  ATA_IO_NWORD_READ_SWAP (ATAPI_DATA, buffer, 256);
//    ATA_IO_NWORD_READ(ATAPI_DATA, buffer, 256);/***2010311*****/
sysInWordString(ATAPI_DATA, buffer, 256);
    ATA_DEBUG_MSG (2, "ataParamRead end:\n", 0, 0, 0, 0, 0, 0);


    return(OK);
    }   /* ataParamRead */


/*******************************************************************************
*
* ataCtrlReset - reset the specified ATA/IDE disk controller
*
* This routine resets the ATA controller specified by ctrl.
* The device control register is written with SRST=1
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

STATUS ataCtrlReset
    (
    int ctrl
    )
    {
    ATA_CTRL    *pCtrl  = &ataCtrl[ctrl]; 
    ATA_DRIVE   *pDrive;
    int         drive;
    int         ix;

    ATA_DEBUG_MSG (2, "ataCtrlReset: ctrl=%d\n", ctrl, 0, 0, 0, 0, 0);

    /* Set reset bit */

    ATA_IO_BYTE_WRITE (ATAPI_D_CONTROL, ATA_CTL_SRST | ATA_CTL_NIEN);

    for (ix = 0; ix < 100; ix++)
        sysDelay ();

    ATA_IO_BYTE_WRITE (ATAPI_D_CONTROL, 0);  /* clear control reg */

    for (ix = 0; ix < 10000; ix++)
        sysDelay ();

    /*
     * timeout max 30 seconds for controller to reset 
     * if it is busy after this timeout, this is an error
     *
     */
    ix = 0;
    while (ATA_IO_BYTE_READ (ATAPI_ASTATUS) & ATA_STAT_BUSY)
        {
	printErr("ataCtrlReset taskDelay      \r", 0,0,0,0,0,0);
	printErr("ataCtrlReset taskDelay   %d,%x \r", ix,ATAPI_ASTATUS,0,0,0,0);
	taskDelay(sysClkRateGet());
	if (++ix == 30)
	    {
	    printErr("\nController %d reset timeout.  ERROR\n", ctrl);
	    return (ERROR);
	    }
        }

    for (drive = 0; drive < ATA_MAX_DRIVES; drive++)
        {
        pDrive = &pCtrl->drive[drive];
        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));

        pDrive->signature = ((ATA_IO_BYTE_READ(ATAPI_SECCNT_INTREASON) << 24)  | 
                             (ATA_IO_BYTE_READ(ATAPI_SECTOR) << 16)            |
                             (ATA_IO_BYTE_READ(ATAPI_CYLLOW) << 8)             | 
                              ATA_IO_BYTE_READ(ATAPI_CYLHI));
        if (pDrive->signature == ATA_SIGNATURE)
            {
            ATA_DEBUG_MSG(2, "ataCtrlReset: Reset: ATA device=0x%x\n",
                          pDrive->signature,0,0,0,0,0);
            }
        else if (pDrive->signature == ATAPI_SIGNATURE)
            {
            ATA_DEBUG_MSG(2, "ataCtrlReset: Reset: ATAPI device=0x%x\n",
                          pDrive->signature,0,0,0,0,0);
            }
        else
            ATA_DEBUG_MSG(2, "ataCtrlReset: Reset: unknown device=0x%x\n",
                          pDrive->signature,0,0,0,0,0);

        }

    ATA_DEBUG_MSG (3, "ataCtrlReset: returning to caller\n", 0, 0, 0, 0, 0, 0);
    return(OK);
    }

/* Select device */
LOCAL STATUS ataDeviceSelect
    (
    ATA_CTRL    *pCtrl,
    int         device
    )
    {
    int     i;
    STATUS  status;

    i = 0;
    while (i++ < 10000000)
        {
        if (((status = ATA_IO_BYTE_READ (ATAPI_STATUS) & (ATA_STAT_BUSY | ATA_STAT_DRQ))) == 0)
            {
            status = OK;
            break;
            }
        }
    if (status != OK)
        {
        ATA_DEBUG_MSG(1,"ataDeviceSelect %d/%d: status = 0x%x read timed out\n",
                      pCtrl->ctrl, device, status, 0,0,0);
        return(ERROR);
        }

    /* Write device id to dev/head register to select */

    ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (device << ATA_DRIVE_BIT));

    ATA_WAIT_STATUS;   /* delay at least 400 ns before reading status again */

    i = 0;
    while (i++ < 10000000)
        {
        if (((status = ATA_IO_BYTE_READ (ATAPI_STATUS)) & (ATA_STAT_BUSY | ATA_STAT_DRQ)) == 0)
            {
            ATA_DEBUG_MSG(1,"ataDeviceSelect: %d/%d OK - select count = %d\n",
			    pCtrl->ctrl, device, i,0,0,0);
            return (OK);   /* device is selected */
            }
        }
    ATA_DEBUG_MSG(1,"ataDeviceSelect failed %d/%d status = 0x%x\n",pCtrl->ctrl,
                     device, status, 0, 0, 0);
    return(ERROR);
    }


/**************************************************************************
*
* ataStatusChk - Check status of drive and compare to requested status.
*
* Wait until the drive is ready.
*
* RETURNS: OK, ERROR if the drive status check times out.
*/
STATUS ataStatusChk
    (
    ATA_CTRL  * pCtrl,
    UINT8      mask,
    UINT8      status
    )
    {
    int i;

    ATA_DEBUG_MSG (15,"ataStatusChk %d: addr=0x%x mask=0x%x status=0x%x\n",
                  pCtrl->ctrl, ATAPI_ASTATUS, mask, status, 0, 0);

    i = 0;
    while ((ATA_IO_BYTE_READ(ATAPI_ASTATUS) & mask) != status)
        {
            if (++i > 1000000)
            {
            ATA_DEBUG_MSG (15,"ataStatusChk: timed out\n",
                          0, 0, 0, 0, 0, 0);
            return (ERROR);

            }
            else
                ATA_DELAY_400NSEC; /* wait ~400 nanoseconds */
        }
    return (OK);

    }
/**************************************************************************
*
* ataWdog - ATA/IDE controller watchdog handler.
*
* RETURNS: N/A
*/

LOCAL void ataWdog
    (
    int ctrl
    )
    {
    ATA_CTRL * pCtrl = &ataCtrl[ ctrl];

    pCtrl->wdgOkay = FALSE;
    } /* ataWdog */

#if defined(ATA_DEV_BOOT_SEC_FIND)
STATUS ataDevBootSecFind 
    (
    int ctrl, 
    int drive
    )
    {
    int    cylinder = 0;
    int    head = 0;
    int    sector = 0;
    int    count = 0;

    char *buffer = malloc(512);

    if (buffer == NULL)
        return(ERROR);

    for (sector = 0; sector < 64; sector++)
        {
        ataRW(ctrl, drive, cylinder, head, sector, (void *)buffer, 1, 3);

        if ( (*buffer == (char)0xe9) || (*buffer == (char)0xeb) )
            {
            printf("possibly at sector offset %d (0x%x)\n", sector, sector);
            count++;
            }
        }

    free(buffer);
    return(OK);
    }

#endif  /* ATA_DEV_BOOT_SEC_FIND */


/**************************************************************************
*
* atapiPktCmd - execute an ATAPI command with error processing
*
* This routine executes a single ATAPI command, checks the command
* completion status and tries to recover if an error encountered during
* command execution at any stage.
*
* RETURN: SENSE_NO_SENSE if success, or ERROR if not successful for any reason.
*
* ERRNO:  S_ioLib_DEVICE_ERROR
*/

UINT8 atapiPktCmd
    (
    ATA_DEV   * pAtapiDev,
    ATAPI_CMD * pComPack
    )
    {
    int         device;
    ATA_CTRL  * pCtrl;
    ATA_DRIVE * pDrive;
    UCHAR       error      = 0; /* Error Register */
    int         retryCount = 0;

    if ((pAtapiDev == NULL) || (pComPack == NULL))
        return(ERROR);

    device = pAtapiDev->drive;
    pCtrl = &ataCtrl[pAtapiDev->ctrl];
    pDrive = &pCtrl->drive[device];

    ATA_DEBUG_MSG (9,"atapiPktCmd: entered\n", 0, 0, 0, 0, 0, 0);
    if (pComPack->bufLength != 0)
        {
        ATA_DEBUG_MSG (4," pComPack->bufLength = %#x\n", pComPack->bufLength, 
                       0, 0, 0, 0, 0);
        if (pComPack->desiredTransferSize == 0)
            return(ERROR);

        /*  
         * Do some adjustment here of byte count transfer as interpretation by 
         * device will change depending on the byte count loaded in taskfile 
         * registers.
         * 1) data transfer ( byte count ) should be even if data transfer 
         * size is less the buffer( total data requested): 
         * 2) data transfer ( byte count ) is interpreted as 0xfffeH if it is 
         * 0xffffH
         */

        if (pComPack->desiredTransferSize > 0xfffe)
            {
            pComPack->desiredTransferSize = 0xfffe;
            ATA_DEBUG_MSG(4,"atapiPktCmd: truncating transfer size\n",0,0,0,0,0,0);
            }
        else
            if (pComPack->desiredTransferSize < pComPack->bufLength)
            pComPack->desiredTransferSize &= 0xfffffffe;
        }
    else
        {
        ATA_DEBUG_MSG (4," bufLength = 0\n", 0, 0, 0, 0, 0, 0);
        pComPack->dma = FALSE;
        pComPack->desiredTransferSize = 0;
        }
    ATA_DEBUG_MSG (11,"       desiredTransferSize = %#x\n",
                   pComPack->desiredTransferSize, 0, 0, 0, 0, 0);

    semTake (&pCtrl->muteSem, WAIT_FOREVER);

    retryPktCmd:
    error = 0;

    if (atapiPktCmdExec (pAtapiDev, pComPack) == OK)
        {

        /* Check the Completion Status */

        ATA_DEBUG_MSG (4," pAtapiDev->status %#x  pComPack->dma %#x\n",
                       pAtapiDev->status, pComPack->dma, 0, 0, 0, 0);

        if ((pAtapiDev->status & ATA_STAT_ERR) == 0 )
            goto okPktCmd;

        /* If Error, Read Error Register */

        error = ATA_IO_BYTE_READ (ATAPI_ERROR);
        pAtapiDev->errNum = (error >> 4) & 0x0f;

        switch (error & ERR_SENSE_KEY)
            {
            /* only handle special cases, all others just default */
            /* error code is set just above switch in errNum */
            
            case SENSE_RECOVERED_ERROR:
                if (pAtapiDev->pBuf < pAtapiDev->pBufEnd)
                    {   /* not medium access commands */
                    break;
                    }
                else
                    goto okPktCmd;
                break;

            case SENSE_UNIT_ATTENTION:
                /* Medium may have changed, or Power on or Reset occured, or
                 * Mode parameter changed
                 */
		ATA_DEBUG_MSG(1,"MEDIA CHANGE MEDIA CHANGE MEDIA CHANGE\n",0,0,0,0,0,0);
                pDrive->state = ATA_DEV_MED_CH;
                pAtapiDev->blkDev.bd_readyChanged = TRUE;
                break;

            default:
                break;
            }

        /* 
         * (error & ~ERR_SENSE_KEY):
         * ERR_MCR      0x08    Media Change Requested
         * ERR_ABRT     0x04    Aborted command
         * ERR_EOM      0x02    End Of Media
         * ERR_ILI      0x01    Illegal Length Indication
         *
         * pAtapiDev->intReason:
         * INTR_IO      0x02: 1 - In to the Host/0 - Out to the device
         * INTR_COD     0x01: 1 - Command/0 - user Data
         *
         */

        ATA_DEBUG_MSG (2, "atapiPktCmd%d/%d: ERROR: %s: status=%#x  "
                       "error=%#x\n", pAtapiDev->ctrl, device,
                       ataErrStrs[ pAtapiDev->errNum], pAtapiDev->status,
                       error, 0);
        }
    /* Execute a request sense command to try to find out more about what
     * went wrong (and clear a unit attention)?
     */

    if (error != 0)
        {
        ATA_DEBUG_MSG(2,"atapiPktCmd: CmdExec error 0x%x - request sense\n",
                      error, 0, 0, 0, 0, 0);
        atapiReqSense (pAtapiDev);
        }
    else
        error = -1;

    if ( (error & ERR_SENSE_KEY) != SENSE_UNIT_ATTENTION )
        {
        atapiInit (pAtapiDev->ctrl, device);
        ATA_DEBUG_MSG(4,"atapiPktCmd: error - retrying\n",0,0,0,0,0,0);
        if (++retryCount < ataRetry)
            goto retryPktCmd;
        }

    semGive (&pCtrl->muteSem);

    (void)errnoSet (S_ioLib_DEVICE_ERROR);

    return (error);

    okPktCmd:

    semGive (&pCtrl->muteSem);

    return(SENSE_NO_SENSE);

    } /* atapiPktCmd */

/**************************************************************************
*
* atapiPktCmdExec - execute an ATAPI command without error processing
*
* This routine executes a single ATAPI command without checking of
* completion status, and attempts to recover. An invoking routine is 
* responsible for error processing.
*
* RETURN: OK, or ERROR if an error is encountered during data transfer.
*
* ERRNO:  S_ioLib_DEVICE_ERROR
*/

LOCAL STATUS  atapiPktCmdExec
    (
    ATA_DEV   * pAtapiDev,
    ATAPI_CMD * pComPack
    )
    {
    int          device = pAtapiDev->drive;
    ATA_CTRL   * pCtrl  = &ataCtrl[pAtapiDev->ctrl];
    FUNCPTR      pTransferProc;
    FUNCPTR      pTransferLoopProc = (FUNCPTR)atapiNonOverlapTransferLoop;
    UCHAR        features = 0; /* Features Register */
    UINT32       desiredTransferSize = pComPack->desiredTransferSize;

    ATA_DEBUG_MSG (5, "atapiPktCmdExec%d/%d: cmd=%#x\n", pAtapiDev->ctrl,
                   device, pComPack->cmdPkt[0], 0, 0, 0);

    /* if a DATA command packet comes with NULL buffer pointer */

    if ((pComPack->ppBuf == NULL) && (pComPack->direction != NON_DATA))
        {
        ATA_DEBUG_MSG(5,"atapiPktCmdExec: DATA packet with NULL buf pointer\n",
                         0, 0, 0, 0, 0, 0);
        return (ERROR);
        }

    /* Initialize transfer control structure */

    if (pComPack->direction == NON_DATA)
        {   
        /* this is a non-data packet, no data transfer */
        pAtapiDev->pBuf       = NULL; 
        pAtapiDev->pBufEnd    = NULL;  
        pComPack->dma       = FALSE;
        ATA_DEBUG_MSG(4,"non-data xfer size = 0x%x\n",desiredTransferSize,
                          0, 0, 0, 0, 0);
        }
    else            
        {
        /* this is a read or write data packet */
        pAtapiDev->pBuf     = *(pComPack->ppBuf);
        pAtapiDev->pBufEnd  = *(pComPack->ppBuf) + pComPack->bufLength;
        pComPack->dma       = TRUE;
        }    

    pAtapiDev->direction  = pComPack->direction;
    pAtapiDev->transCount = 0;
    pAtapiDev->errNum     = 27;     /* unknown error */
    pAtapiDev->intReason  = 0;
    pAtapiDev->status     = 0;
    pAtapiDev->transSize  = 0;

    /* 
     * For NON_DATA commands DMA is disabled here, so that All 
     * NON_DATA command execution from here proceed in PIO mode. 
     */

    if (pComPack->direction == NON_DATA)
        pComPack->dma = FALSE;
    else
        pComPack->dma = TRUE;

    if ((pComPack->dma) && (pCtrl->ataHostDmaSupportOkay == TRUE))
        {
        features |= FEAT_DMA;
        pTransferProc = (FUNCPTR)atapiDmaTransfer;
        ATA_DEBUG_MSG (4,"pTransferProc = atapiDmaTransfer\n", 
                        0, 0, 0, 0, 0, 0);
        }
    else
        {
        features      = 0;
        pTransferProc = (FUNCPTR)atapiPIOTransfer;
        ATA_DEBUG_MSG (4,"pTransferProc = atapiPIOTransfer\n", 
                        0, 0, 0, 0, 0, 0);
        }

#ifdef ATAPI_OVERLAPPED_FEATURE
    if ((pComPack->overlap) && 
                  (pCtrl->drive[device].param.capabilities & CAPABIL_OVERLAP))
        {
            features |= FEAT_OVERLAP;
            pTransferLoopProc = (FUNCPTR)atapiOverlapTransferLoop;
            }
#endif /* ATAPI_OVERLAPPED_FEATURE */


    /* The following state transitions are for packet
     * transfers of data to device.
     *
     * Make sure device idle before writing packet
     * Idle -> write packet cmd -> check BSY=0,DRQ=1  HP0:HP1
     * write packet -> wait for interrupt             HP1:HP3
     * INTRQ asserted -> check status                 HP3:HP2
     * BSY=1                                          HP2:HP2
     * BSY=0,DRQ=1 -> transfer data (PIO)             HP2:HP4
     * BSY=0,DRQ=1 -> transfer data DMA               HP2:HP4
     * PIO transfer complete or DMA setup -> Wait INT HP4:HP3
     * INTRQ asserted -> check status                 HP3:HP2
     * BSY=0,DRQ=0 command complete -> idle           HP2:HI0
     */

    /* check for device idle: DRQ=0,BSY=0 */
    if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 
                           !ATA_STAT_DRQ | !ATA_STAT_BUSY) != OK)
        {
        ATA_DEBUG_MSG(4,"device not idle - ERROR\n",0,0,0,0,0,0)
        return (ERROR);
        }
    
    /* Select device */
    if (((ATA_IO_BYTE_READ (ATAPI_SDH_D_SELECT) >> ATA_DRIVE_BIT) & 1) != device)
        {
        /* must select proper device */
        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (device << ATA_DRIVE_BIT));
        /* wait for device idle: DRQ=0,BSY=0 */
        if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY,
                               !ATA_STAT_DRQ | !ATA_STAT_BUSY) != OK)
            {
            ATA_DEBUG_MSG(4,"device not idle after select - ERROR\n",0,0,0,0,0,0)
            return (ERROR);
            }
        }

    /* bus is now idle, can setup Packet Command */
    /* Initialize Task File */

    ATA_IO_BYTE_WRITE (ATAPI_FEATURE, features);
    ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, 0);
    ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, (UINT8)(desiredTransferSize & 0xff));
    ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI, 
                       (UINT8)((desiredTransferSize >> 8) & 0xff));

    /***** Issue `ATAPI Packet Command' *****/

    ATA_IO_BYTE_WRITE (ATAPI_COMMAND, ATA_PI_CMD_PKTCMD);
    ATA_DELAY_400NSEC;

    /* Wait for DRQ and !BSY */
    if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 
                            ATA_STAT_DRQ | !ATA_STAT_BUSY) != OK)
        {
        return (ERROR);
        }

    /* Write Command Packet Bytes (6 words of Command) via PIO */
    ATA_DEBUG_MSG (13,"atapiPktCmdExec: Writing command packet. \n", 0, 0, 0, 0, 0, 0);    
    ATA_DEBUG_MSG(4,"READ12 packet is: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", 
           pComPack->cmdPkt[0],pComPack->cmdPkt[1],pComPack->cmdPkt[2],
           pComPack->cmdPkt[3],pComPack->cmdPkt[4],pComPack->cmdPkt[5]);
    ATA_DEBUG_MSG(4,"                  0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", 
           pComPack->cmdPkt[6],pComPack->cmdPkt[7],pComPack->cmdPkt[8],
           pComPack->cmdPkt[9],pComPack->cmdPkt[10],pComPack->cmdPkt[11]);

    ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)pComPack->cmdPkt, 6);

    if ((pComPack->dma) && (pCtrl->ataHostDmaSupportOkay == TRUE))
        {
        if (pCtrl->ataDmaSet != NULL)
            {
            ATA_DEBUG_MSG (4,"ATA_HOST_DMA_ENGINE_SET...\n", 0, 0, 0, 0, 0, 0);
            (*pCtrl->ataDmaSet) (pAtapiDev->ctrl, pAtapiDev->drive,
                                 pAtapiDev->pBuf, pComPack->bufLength, pAtapiDev->direction);
            }

        if (pCtrl->ataDmaStart != NULL)
            {
            ATA_DEBUG_MSG(4,"ATA_HOST_DMA_ENGINE_START...\n",0,0,0,0,0,0);
            (*pCtrl->ataDmaStart)(pAtapiDev->ctrl);
            }
        }

    /***** Transfer the Data *****/

    if (pTransferLoopProc(pAtapiDev, pComPack, pTransferProc) == OK)
        {
        ATA_DEBUG_MSG (9, "atapiPktCmdExec%d/%d: Ok: bufStart=%p buf=%p n=%d "
                       "com=%#x\n", pAtapiDev->ctrl, device,
                       (pComPack->ppBuf ? *(pComPack->ppBuf) : 0), 
                       pAtapiDev->pBuf,
                       pAtapiDev->transCount, pComPack->cmdPkt[0]);

        if (pComPack->direction != NON_DATA)
            {
            if (pAtapiDev->pBufEnd == NULL)
                *(pComPack->ppBuf) += pComPack->bufLength;
            else
                *(pComPack->ppBuf) = pAtapiDev->pBuf;
            }

        ATA_DEBUG_MSG (14,"atapiPktCmdExec - Return OK  \n", 0, 0, 0, 0, 0, 0);
        return(OK);
        }
    else
        ATA_DEBUG_MSG (14,"pTransferLoopProc - Returned ERROR \n", 0, 0, 0, 0, 
                       0, 0);

    if (pComPack->ppBuf != NULL)
        {
        ATA_DEBUG_MSG (2, "    bufStart=%p buf=%p n=%d com=%#x intReason=%#x "
                       "byteCount=%#x\n", *(pComPack->ppBuf), pAtapiDev->pBuf,
                       pAtapiDev->transCount, pComPack->cmdPkt[0],
                       pAtapiDev->intReason, pAtapiDev->transSize);
        }
    else
        {
        ATA_DEBUG_MSG (2, "    bufStart=%p buf=%p n=%d com=%#x intReason=%#x "
                       "byteCount=%#x\n", *(pComPack->ppBuf), pAtapiDev->pBuf,
                       pAtapiDev->transCount, pComPack->cmdPkt[0],
                       pAtapiDev->intReason, pAtapiDev->transSize);
        ATA_DEBUG_MSG (14,"pTransferLoopProc - Returned ERROR \n", 0, 0, 0, 0, 
                   0, 0);
        }
    return(ERROR);
    } /* atapiPktCmdExec */


/**************************************************************************
*
* atapiNonOverlapTransferLoop - loop for DRQ Interrupts without Overlapping
*
* This routine loop for Data Request Interrupts until all data packets are
* transferred. It invoked when atapiPktCmd() executes a NON Overlapped
* command.
*
* RETURN: OK, or ERROR
*
* SEE ALSO: atapiOverlapTransferLoop()
*/

ATA_LOCAL STATUS atapiNonOverlapTransferLoop
    (
    ATA_DEV    * pAtapiDev,
    ATAPI_CMD  * pComPack,
    FUNCPTR    pTransferProc
    )
    {
    ATA_CTRL * pCtrl   = &ataCtrl[pAtapiDev->ctrl];
    int        semStatus;

    ATA_DEBUG_MSG (14,"atapiNonOverlapTransferLoop: entered\n",0,0,0,0,0,0);

    if (pTransferProc == (FUNCPTR)atapiDmaTransfer)
        {
        ATA_DEBUG_MSG (4," waiting for Interrupt .... \n", 0, 0, 0, 0, 0, 0);

        /* HPD3 */

        /* Here data transfer will be done via DMARQ/DMACK.*/

        /* Wait for the Interrupt Request for command completion */
        /* HPD4 : HPD3 */
        semStatus = semTake (&pCtrl->syncSem,  sysClkRateGet() *
                             (pCtrl->semTimeout+10));

        /* HPD3:HPD2 */
        /* IRQ is over. Stop IDE controller BusMaster */
        if (pCtrl->ataDmaStop != NULL)
            {
            (*pCtrl->ataDmaStop)(pAtapiDev->ctrl);
            }

        if (semStatus == ERROR)
            {
            printErr("ATAPI device not responding (No DMA Interrupt)\n");
            return(ERROR);
            }

        /* HPD3:HPD2 */

        pAtapiDev->status = pCtrl->intStatus;
        pAtapiDev->intReason = ATA_IO_BYTE_READ (ATAPI_SECCNT_INTREASON);
        if (((pCtrl->intStatus) & (ATA_STAT_BUSY | ATA_STAT_DRQ | ATA_STAT_ERR)) ||
            ((pAtapiDev->intReason) & INTR_RELEASE))
            {
            ATA_DEBUG_MSG (10,"Error in DMA transfer status "
                           "intReason = %#x   "
                           "intStatus = %#x \n", pAtapiDev->intReason,
                           pCtrl->intStatus, 0, 0, 0, 0);
            return(ERROR);
            }
        return(OK);
        }
    else
        {
        while ( TRUE )
            {

            /* Wait for INTRQ */
            ATA_DEBUG_MSG (5, "inside while\n",0,0,0,0,0,0);

            semStatus = semTake (&pCtrl->syncSem, sysClkRateGet() *
                                 (pCtrl->semTimeout+10));

            if (semStatus == ERROR)
                {
                pAtapiDev->errNum = 18;
                ATA_DEBUG_MSG(5,"xferloop: sem error = 0x%x\n", semStatus,
                              0,0,0,0,0);
                return(ERROR);
                }

            pAtapiDev->status = pCtrl->intStatus;
            pAtapiDev->intReason = ATA_IO_BYTE_READ (ATAPI_SECCNT_INTREASON);

            if ( (pAtapiDev->status & ATA_STAT_DRQ) == 0)
                {
                ATA_DEBUG_MSG (16,"transferLoop: Command terminated\n", 0, 0, 0, 0, 0, 0);
                break; /* Command terminated */
                }

            if (pAtapiDev->direction == NON_DATA)       /* non data command */
                {
                ATA_DEBUG_MSG (10,"Error  NON_DATA Command execution \n",
                               0, 0, 0, 0, 0, 0);                       
                pAtapiDev->errNum = 19;
                return(ERROR);
                }

            if (((pAtapiDev->intReason & INTR_COD) != 0) || /* CoD cleared  */
                ((pAtapiDev->status & ATA_STAT_BUSY) != 0)) /* BUSY cleared */
                {
                pAtapiDev->errNum = 20;
                return(ERROR);
                }

            /* Transfer single data packet */

            ATA_DEBUG_MSG (16, "Calling pTransferProc()\n",0,0,0,0,0,0);
            if (pTransferProc(pAtapiDev) != OK)
                return(ERROR);

            pAtapiDev->transCount++;
            } /* while ( TRUE ) and end of "else" */
        }

    ATA_DEBUG_MSG (5, "  transCount = %#x\n",pAtapiDev->transCount,0,0,0,0,0);

    if ( ((pAtapiDev->intReason & INTR_COD) == 0)    || /* CoD set */
         ((pAtapiDev->intReason & INTR_IO) == 0)     || /* IO set */
         ((pAtapiDev->status & ATA_STAT_READY) == 0) || /* DRDY set */
         ((pAtapiDev->status & ATA_STAT_BUSY) != 0) /* BUSY cleared */
       )
        {
        pAtapiDev->errNum = 21;
        ATA_DEBUG_MSG (5, "xferLoop: ERROR\n",0,0,0,0,0,0);
        return(ERROR);
        }
    ATA_DEBUG_MSG (5, "xferLoop: OK\n",0,0,0,0,0,0);
    return(OK);
    } /* atapiNonOverlapTransferLoop */

#ifdef ATAPI_OVERLAPPED_FEATURE
/**************************************************************************
*
* atapiOverlapTransferLoop - loop for DRQ Interrupts with Overlapping
*
* This routine loop for  Data Request  Interrupts until all data packets
* are transferred.  It invoked when atapiPktCmd() executes an Overlapped 
* command. When Device executes an Overlapped command, it releases the ATA 
* bus until the device is ready to  transfer a data  or  to  present the 
* completion status.
*
* RETURN: OK, or ERROR
*
* SEE ALSO: atapiNonOverlapTransferLoop()
*/

LOCAL STATUS atapiOverlapTransferLoop
    (
    ATA_DEV    * pAtapiDev,
    ATAPI_CMD  * pComPack,
    FUNCPTR    pTransferProc
    )
    {
    ATA_DEBUG_MSG (14,"atapiOverlapTransferLoop() entered\n", 
                   0, 0, 0, 0, 0, 0);
    pAtapiDev->errNum = 25;
    return(ERROR);
    } /* atapiOverlapTransferLoop */

#endif /* ATAPI_OVERLAPPED_FEATURE */


/**************************************************************************
*
* atapiDmaTransfer -  a stub 
*
* RETURN: OK
*
* SEE ALSO: atapiPioTransfer()
*/

LOCAL STATUS atapiDmaTransfer
    (
    ATA_DEV   * pAtapiDev
    )
    {
    /*
     * program control never comes here.
     * This is only for FUNCPTR comparision only.
     */
    return(0);
    } /* atapiDmaTransfer */


/**************************************************************************
*
* atapiInit - init ATAPI CD-ROM disk controller
*
* This routine resets the ATAPI CD-ROM disk controller.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/
STATUS atapiInit
    (
    int ctrl,
    int drive
    )
    {
    ATA_CTRL *  pCtrl   = &ataCtrl[ctrl];
    int         retryCount = 0;
    int         i;

    ATA_DEBUG_MSG (9, "atapiInit%d/%d: \n", ctrl, drive, 0, 0, 0, 0);

    while (TRUE) /* Forever */
        {
        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));

        ATA_DELAY_400NSEC;

        ATA_IO_BYTE_WRITE (ATAPI_COMMAND, ATA_PI_CMD_SRST);

        for (i = 0; i < 5000; i++)  /* 2 ms */
            ATA_DELAY_400NSEC;

        ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT, (drive << ATA_DRIVE_BIT));

        ATA_DELAY_400NSEC;

        pCtrl->wdgOkay = TRUE;

        wdStart (pCtrl->wdgId, (sysClkRateGet() * pCtrl->wdgTimeout),
                 (FUNCPTR)ataWdog, ctrl);

        while ( (ATA_IO_BYTE_READ (ATAPI_STATUS) & 
                 (ATA_STAT_BUSY | ATA_STAT_READY | ATA_STAT_WRTFLT | ATA_STAT_DRQ |
                                                    ATA_STAT_ECCCOR | ATA_STAT_ERR)) && (pCtrl->wdgOkay) )
            ;

        wdCancel (pCtrl->wdgId);

        if (pCtrl->wdgOkay)
            break;

        pCtrl->wdgOkay = TRUE;

        ATA_DEBUG_MSG (1, "ataPiInit%d/%d: ERROR: status=0x%x error=0x%x\n", 
                       ctrl, drive, ATA_IO_BYTE_READ(ATAPI_STATUS), 
                       ATA_IO_BYTE_READ(ATAPI_ERROR), 0, 0);

        if (++retryCount == ataRetry)
            return(ERROR);
        }

    /* The following allow to recover after accidental interrupt */

    semTake (&pCtrl->syncSem, NO_WAIT);

    return(OK);
    } /* atapiInit */


/**************************************************************************
*
* atapiPIOTransfer - transfer a single data packet via PIO
*
* This routine transfers a single data packet via PIO.
*
* RETURN: OK, or ERROR
*
* SEE ALSO: atapiDMATransfer()
*/

LOCAL STATUS atapiPIOTransfer
    (
    ATA_DEV   * pAtapiDev
    )
    {
    ATA_CTRL  * pCtrl=&ataCtrl[pAtapiDev->ctrl];
    UINT32      desiredSize;
    UINT32      remainderSize;
    short       bucket;

    ATA_DEBUG_MSG (14,"atapiPIOTransfer: entered\n", 0, 0, 0, 0, 0, 0);

    /* Read Byte Count */

    pAtapiDev->transSize = 
    ((UINT16)ATA_IO_BYTE_READ (ATAPI_CYLHI_BCOUNT_HI)) << 8;
    pAtapiDev->transSize += ATA_IO_BYTE_READ (ATAPI_CYLLOW_BCOUNT_LO);

    /*
     * Page 23, Ref-2.
     * If the Device requests more data to be transfered than required by the
     * command protocol, the Host shall pad when sending data to the Device,
     * and dump extra data into a bit bucket when reading data from the Device.
     */

    if ( (pAtapiDev->pBufEnd == NULL) && (pAtapiDev->transSize != 0) )
        {
        pAtapiDev->errNum = 22;
        ATA_DEBUG_MSG(4,"atapiPIOTransfer: BufEnd=NULL, transSize=0x%x\n",
                         pAtapiDev->transSize, 0, 0, 0, 0, 0);
        return(ERROR);
        }
    desiredSize = pAtapiDev->pBufEnd - pAtapiDev->pBuf;

    ATA_DEBUG_MSG (5,"atapiPIOTransfer: desiredSize = %#x"
                   "   pAtapiDev->transSize = 0x%x\n",
                   desiredSize, pAtapiDev->transSize, 0, 0, 0, 0);

    if (desiredSize <= pAtapiDev->transSize)
        {
        remainderSize = pAtapiDev->transSize - desiredSize;
        pAtapiDev->pBufEnd = NULL;
        }
    else
        {
        desiredSize = pAtapiDev->transSize;
        remainderSize = 0;
        }

    /* Transfer Data Bytes */

    switch (pAtapiDev->direction)
        {
        case OUT_DATA:
            if ((pAtapiDev->intReason & INTR_IO) != 0)   /* IO cleared */
                goto errorPIOTransfer;

            ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)pAtapiDev->pBuf,
                                desiredSize / 2);

            while (remainderSize)
                {
                bucket = 0;
                ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)&bucket, 1);
                remainderSize -= 2;
                }
            break;

        case IN_DATA:

            if ((pAtapiDev->intReason & INTR_IO) == 0)   /* IO set */
                goto errorPIOTransfer;

            ATA_IO_NWORD_READ (ATAPI_DATA, (void *)pAtapiDev->pBuf,
                               desiredSize / 2);
            while (remainderSize)
                {
                ATA_IO_NWORD_READ (ATAPI_DATA, (void *)&bucket, 1);
                remainderSize -= 2;
                }
            break;

        default:
            goto errorPIOTransfer;
        }
    pAtapiDev->pBuf += pAtapiDev->transSize;
    ATA_DEBUG_MSG(4,"atapiPIOTransfer: OK\n",0, 0, 0, 0, 0, 0);
    return(OK);

    errorPIOTransfer:
    ATA_DEBUG_MSG(4,"atapiPIOTransfer: ERROR\n",0, 0, 0, 0, 0, 0);
    pAtapiDev->errNum = 24;
    return(ERROR);
    } /* atapiPIOTransfer */


/**************************************************************************
*
* atapiReqSense - issue a REQUEST_SENSE command to a device 
*
* This routine issues a REQUEST_SENSE command to a specified ATAPI device and
* read the results.
*
* RETURNS: OK, or ERROR if the command fails.
*/

LOCAL STATUS atapiReqSense
    (
    ATA_DEV * pAtapiDev
    )
    {
    ATA_CTRL  *  pCtrl       = &ataCtrl[pAtapiDev->ctrl];
    ATAPI_CMD    atapiCmd;
    char         reqSenseData [18];
    char      *  pBuf = (char *)reqSenseData;
    int          error = 0;
    int          i;

    ATA_DEBUG_MSG (10, "atapiReqSense%d/%d: \n", pAtapiDev->ctrl,
                   pAtapiDev->drive, 0, 0, 0, 0);

    /* build a REQUEST SENSE command and execute it */
    /* Clear packet command before use. */
    for (i = 0; i < sizeof(ATAPI_CMD); i++)
        atapiCmd.cmdPkt[i] = 0;

    atapiCmd.cmdPkt[0] = ATAPI_CMD_REQUEST_SENSE;
    atapiCmd.cmdPkt[4] = 18;

    atapiCmd.ppBuf               = &pBuf;
    atapiCmd.bufLength           = 18;
    atapiCmd.direction           = IN_DATA;
    atapiCmd.desiredTransferSize = 18;
    atapiCmd.dma                 = FALSE;
    atapiCmd.overlap             = FALSE;

    if (atapiPktCmdExec (pAtapiDev, &atapiCmd) != OK)
        return(ERROR);

    /* REQUEST SENSE command status != GOOD indicates fatal error */

    if ( (pAtapiDev->status & ATA_STAT_ERR) != 0 )
        {
        error = ATA_IO_BYTE_READ (ATAPI_ERROR);
        ATA_DEBUG_MSG (2, "atapiReqSense%d/%d: ERROR: status=%#x error=%#x\n",
                       pAtapiDev->ctrl, pAtapiDev->drive, pAtapiDev->status, 
                       error, 0, 0);

        return(ERROR);
        }

    ATA_DEBUG_MSG (10, "atapiReqSense%d/%d: Ok: senseKey=%#x addSenseCode=%#x"
                   "\n", pAtapiDev->ctrl, pAtapiDev->drive,
                   (reqSenseData[2] & 0x0f), reqSenseData[12], 0, 0);

    return(OK);
    } /* atapiReqSense */


/*--------------------------------------------------------------------------
 *
 * This section of the file contains routines for handling the XBD nature of
 * the ATA driver.  A service task will be created for each controller.
 *
 * The service task will process bio's from the strategy call, and initiate
 * the appropriate disk operation.
 */

 /***************************************************************************
 *
 * ataXbdService - task level routine to handle read and write operation
 *
 * This routine waits on a semaphore from strategy routine. It calls
 * ataGetNextBio and ataExecBio to exercise all the bios currently
 * chained for this device.
 *
 * Note that this task is spawned when the device is attached to the system and
 * deleted after the device is detached and all the resources for this device
 * are freed.
 *
 * RETURN: N/A
 *
 * ERRNO: none
 *
 *\NOMANUAL
 */

LOCAL void ataXbdService
    (
    ATA_CTRL *pCtrl /* pointer to the ata XBD control structure */
    )
    {
    struct bio *  bio;

    while (TRUE)
        {
        /* Wait semaphore from strategy routine */

        if (semTake(&pCtrl->ataBioReadySem, WAIT_FOREVER) == ERROR)
            return;
        /* get all the bios queued for this device and process them */

        ATA_DEBUG_MSG(1,"ctrl%d bio queue ready\n", pCtrl->ctrl, 0,0,0,0,0);
        while ((bio = ataGetNextBio(pCtrl)) != NULL)
            {
            /* bio->bio_error has drive number, from strategy call */
            ATA_DEBUG_MSG(1,"processing bio for drive %d  bio=0x%x\n",
                             bio->bio_error, bio,0,0,0,0);
            ataExecBio (pCtrl, bio, bio->bio_error);
            }
        }
    }

/*******************************************************************************
*
* ataGetNextBio - get the next bio in the bio chain
*
* This routine is called to get the next bio in the bio chain for an XBD device.
* 
* The drive number for the operation is stored in the bio->bio_error field
*
* RETURNS: pointer to the next bio
*
* ERRNO: none
*
*\NOMANUAL
*/

LOCAL struct bio * ataGetNextBio
    (
    ATA_CTRL *ataXbdCtrl  /* pointer to XBD block device wrapper */
    )
    {
    struct bio *  retVal;

    /* take the mutex semaphore for this XBD interface */
    if (semTake (&ataXbdCtrl->muteSem, WAIT_FOREVER) == ERROR)
        return NULL;

    retVal = ataXbdCtrl->bioQueueh;

    if (retVal != NULL)
        {
        ataXbdCtrl->bioQueueh = retVal->bio_chain;
        retVal->bio_chain = NULL;

        if (ataXbdCtrl->bioQueueh == NULL)
            ataXbdCtrl->bioQueuet = NULL;
        }

    semGive (&ataXbdCtrl->muteSem);
    return (retVal);
    }

/***************************************************************************
*
* ataExecBio - execute a bio
*
* This routine processes the bio for read from or write to the XBD block device.
*
* RETURNS: N/A
*
* ERRNO: none
*
*\NOMANUAL
*/

LOCAL void ataExecBio
    (
    ATA_CTRL   *pCtrl,  /* pointer to XBD block device wrapper */
    struct bio *bio,         /* pointer to bio */
    int        drive         /* drive number */
    )
    {
    ATA_DEV *  pDev = pCtrl->drive[drive].pAtaDev;
    BLK_DEV *  bd = &pDev->blkDev;
    int        status = ERROR;
    unsigned   bd_bsize = pDev->ataXbd.xbd.xbd_blocksize;
    sector_t   bd_blocks = pDev->ataXbd.xbd.xbd_nblocks;
    sector_t   nblocks;
    unsigned   size;

    ATA_DEBUG_MSG(1,"Entering ataExecBio %d/%d biodrive=%d\n", pCtrl->ctrl, drive, bio->bio_error, 0,0,0); 

    /* Check that all of this transaction fits in the disk */
    size = bio->bio_bcount;
    nblocks = size / bd_bsize;

    /* If our starting block number is bad, done with error */
    if (bd_blocks <= bio->bio_blkno)
        {
        bio->bio_bcount = 0;
        bio_done (bio, ENOSPC);
        return;
        }

    /* If we overrun the end of the disk, truncate the block number */
    if (bd_blocks < bio->bio_blkno + nblocks)
        {
        nblocks = bd_blocks - bio->bio_blkno;
        }

    /* calculate the real size and residual */
    size = nblocks * bd_bsize;
    bio->bio_bcount = size;

    /* If we have less than 1 block, set the resid and done it */
    if (nblocks == 0)
        {
        bio->bio_bcount = 0;
        bio_done (bio, 0);
        return;
        }

    if (bio->bio_flags & BIO_READ)
        {
        ATA_DEBUG_MSG(1,"Calling ata block read\n", 0,0,0,0,0,0);
        status = bd->bd_blkRd (bd, (int) bio->bio_blkno,
                   (int) nblocks, bio->bio_data);
        }
    else if (bio->bio_flags & BIO_WRITE)
        {
        ATA_DEBUG_MSG(1,"Calling ata block write\n", 0,0,0,0,0,0);
        status = bd->bd_blkWrt (bd, (int)bio->bio_blkno,
                   (int) nblocks, bio->bio_data);
        }
    if (status == OK)
        bio_done (bio, 0);
    else
        {
        bio->bio_bcount = 0;
        /* S_ioLib_DISK_NOT_PRESENT is *really* ENXIO */
        if (errno == S_ioLib_DISK_NOT_PRESENT)
            {
            bio_done (bio, ENXIO);
            /* The underlying media has gone away - remove this device */
            }
        else
        {
        bio_done (bio, errno);
        }
    }
}

/***************************************************************************
*
* ataXbdDump - XBD block device dump routine
*
* /NOMANUAL
*/

LOCAL int ataXbdDump
    (
    struct xbd *  self,
    sector_t      blkno,
    void *        data,
    size_t        size
    )
    {
    ATA_DEV *pDev;
    ATA_XBD *xbd;
    UINT32   nBlks;

    coreIsDumping = 1;	    /* set the flag for use in dump routines */
    xbd = (struct ata_xbd *)self;
    pDev = xbd->ataDev;
    nBlks = size/(xbd->ataDev->blkDev.bd_bytesPerBlk);

#if defined(ATA_DUMP_DEBUG)
    printf("xbd = 0x%x, blkno=0x%x%x, data=0x%x nBlks=0x%x\n",
	    xbd, (int)(blkno>>32), (int)(blkno&0xffffffff), data, nBlks);
#endif
    ataCoreDump(pDev, blkno, nBlks, (char *)data);
    coreIsDumping = 0;
    return (EINVAL);
    }


/***************************************************************************
*
* ataXbdStrategy - XBD block device strategy routine
*
* /NOMANUAL
*/

LOCAL int ataXbdStrategy
    (
    struct xbd *  self,       /* pointer to XBD */
    struct bio *  bio         /* pointer to bio */
    )
    {
    ATA_XBD    * ataXbd = (ATA_XBD *) self;
    int          status;
    struct bio * next_bio;
    ATA_CTRL   * pCtrl = &ataCtrl[ataXbd->ataDev->ctrl];
    ATA_DRIVE  * pDrive = &pCtrl->drive[ataXbd->ataDev->drive];

    ATA_DEBUG_MSG(1,"ataXbdStrategy called: xbd = 0x%x, bio = 0x%x\n",
             self, bio, 0, 0, 0, 0);

    if (pDrive->okRemovable)
        {
        /* Test to see if we have been removed.
         * If status is not OK, then the XBD has been ejected and outstanding
         * bio's have been purged from the bio queue.
         * return status should be either S_ioLib_MEDIA_CHANGED or  ENXIO 
         */
        status = ataXbdTest(ataXbd);
        if (status != OK)
            {
            bio_done(bio, status);  /* bio queue has been purged, kill this one */
            return (status);
            }
        }

    /* we have a non-zero XBD and there is media inserted */
    if (semTake (&pCtrl->muteSem, WAIT_FOREVER) != OK)
        return (ERROR);

    /* Iterate through the chain, running each bio as we get it */
    for ( ; bio != NULL; bio = next_bio)
        {
        /* store drive number in bio error field so we know which
         * drive to read or write from when pulling bio's off queue.
        */
        bio->bio_error = ataXbd->ataDev->drive;

        /* Make sure that we can get the next one */
        next_bio = bio->bio_chain;

        /* Add this bio as the tail of the queue */
        bio->bio_chain = NULL;

        if (pCtrl->bioQueuet)
            {
            pCtrl->bioQueuet->bio_chain = bio;
            }
        else
            {
            pCtrl->bioQueueh = bio;
            }
        pCtrl->bioQueuet = bio;

        /* Tap our service task */
        semGive (&pCtrl->ataBioReadySem);
        }

    semGive (&pCtrl->muteSem);
    return (OK);
    }

/***************************************************************************
*
* ataXbdIoctl - XBD block device ioctl routine
*
* /NOMANUAL
*/

LOCAL int ataXbdIoctl
    (
    struct xbd *  xbd,
    int           command,
    void    *     arg
    )
    {
    ATA_XBD *  ataXbd = (ATA_XBD *)xbd;
    device_t            dev;
    INT16               reType;
    BLK_DEV *           bd = &ataXbd->ataDev->blkDev;
    XBD_GEOMETRY        *geo;
    int                 status;
    ATA_CTRL   * pCtrl = &ataCtrl[ataXbd->ataDev->ctrl];
    ATA_DRIVE  * pDrive = &pCtrl->drive[ataXbd->ataDev->drive];
    struct bio * bio;

    ATA_DEBUG_MSG(1,"ataXbdIoctl called:  xbd = 0x%x, ioctl = 0x%x\n",
            ataXbd, command, 0, 0, 0, 0);

    if (pDrive->okRemovable)
        {
        /* Test to see if we have been removed.
         * If status is not OK, then the XBD has been ejected and outstanding
         * bio's have been purged from the bio queue.
         * return status should be either S_ioLib_MEDIA_CHANGED or  ENXIO 
         */
        status = ataXbdTest(ataXbd);
        if (status == S_ioLib_MEDIA_CHANGED) 
    	    return (status);
        }    

    switch (command)
        {
        case XBD_SOFT_EJECT:
        case XBD_HARD_EJECT:
            /* we are the base XBD so if we get these it is for us */
	    /*
	     * first raise a removal event
	     * then purge the bio queue
	     * last raise an insertion event
	     *
	     */
	    if (semTake (&pCtrl->muteSem, WAIT_FOREVER) != OK)
		return (ERROR);
            dev = ataXbd->xbd.xbd_dev.dv_dev;
            erfEventRaise(xbdEventCategory, xbdEventRemove, ERF_ASYNC_PROC,
                         (void *)dev, NULL);
            while ((bio = ataGetNextBio(pCtrl)))
		bio_done(bio, ENXIO);
            reType = (command == XBD_HARD_EJECT) ?
                      xbdEventPrimaryInsert : xbdEventSoftInsert;
            erfEventRaise(xbdEventCategory, reType, ERF_ASYNC_PROC,
                                (void *)dev, NULL);
	    semGive (&pCtrl->muteSem);
            status = OK;
	    break;

        case XBD_GETGEOMETRY:
            geo = (XBD_GEOMETRY *) arg;
            geo->heads = bd->bd_nHeads;
            geo->secs_per_track = bd->bd_blksPerTrack;
            geo->total_blocks = bd->bd_nBlocks;
            geo->blocksize = bd->bd_bytesPerBlk;
            geo->cylinders = geo->blocksize / geo->secs_per_track / geo->heads;
            status = OK;
	    break;

        case XBD_GETBASENAME:
            dev = ataXbd->xbd.xbd_dev.dv_dev;
            if (devName(dev, arg) == ERROR)
                status = errno;
            else
		status = OK;
	    break;

        case XBD_SYNC:
	    if (semTake (&pCtrl->muteSem, WAIT_FOREVER) != OK)
		return (ERROR);
            bd->bd_ioctl(bd, FIOSYNC, 0);
	    semGive (&pCtrl->muteSem);
            status = OK;
	    break;

        case XBD_TEST:
            ATA_DEBUG_MSG(1,"XBD_TEST return OK\n",0,0,0,0,0,0); 
            status = OK;
	    break;

        case XBD_STACK_COMPLETE:
            if (!ataXbd->xbdInstantiated)
                {
		if (semTake (&pCtrl->muteSem, WAIT_FOREVER) != OK)
		    {
		    status = ERROR;
		    break;
		    }
                erfEventRaise (xbdEventCategory, xbdEventInstantiated,
                               ERF_ASYNC_PROC,
                               ataXbd->xbd.xbd_dev.dv_xname, NULL);
                ataXbd->xbdInstantiated = TRUE;
                ATA_DEBUG_MSG(1,"xbd %d/%d instantiated\n", 
                                 ataXbd->ataDev->ctrl, ataXbd->ataDev->drive,0,0,0,0);
		semGive (&pCtrl->muteSem);
                }
            status = OK;
	    break;

        default:
            status = ENOTSUP;
	    break;
        }  /* end switch */

    return (status);
    }

/***************************************************************************
*
* ataXbdDevCreateSyncHandler -
*/

LOCAL void ataXbdDevCreateSyncHandler
    (
    UINT16 Category,
    UINT16 Type,
    void * eventData,
    void * userData
    )
    {
    ATA_DEV   *  pDev;
    devname_t *  devname;

    if ((Category == xbdEventCategory) && (Type == xbdEventInstantiated))
        {
        devname = (devname_t *) eventData;
        pDev = (ATA_DEV *) userData;

        if (strncmp ((char *) devname,
                     (char *) pDev->ataXbd.name,
                     sizeof (devname_t)) != 0)
            {
            return;
            }

        erfHandlerUnregister (xbdEventCategory, xbdEventInstantiated,
                              ataXbdDevCreateSyncHandler, userData);

        ATA_DEBUG_MSG(1,"Giving xbd %d/%d sync semaphore\n",
                         pDev->ctrl, pDev->drive,0,0,0,0);
        semGive (&pDev->ataXbd.xbdSemId);
        }

    return;
    }


/***************************************************************************
* ataXbdMediaTest - Test a block wrapper for media existence
*
* /NOMANUAL
*/

LOCAL int ataXbdMediaTest
    (
    ATA_XBD *  ataXbd
    )
    {
    BLK_DEV *  bd = &ataXbd->ataDev->blkDev;
    int error;
    BOOL rc_bit;

    /* call the driver's status routine, if one exists */

    if (bd->bd_statusChk)
        error = bd->bd_statusChk(bd);
    else
        error = OK;

    ATA_DEBUG_MSG(1,"ataXbdMediaTest: %d/%d  error=0x%x  readyChanged = %d\n",
                     ataXbd->ataDev->ctrl, ataXbd->ataDev->drive, error, bd->bd_readyChanged,0,0);
    rc_bit = bd->bd_readyChanged;
    bd->bd_readyChanged = 0;

    if (error != OK)
        return (ERROR);

    if (rc_bit)
	return (ERROR);

    return (OK);
    }


/***************************************************************************
*
* ataXbdTest - Test to see if media is present or not.  If a change
*              of media has occurred, test XBD's for 0 size and eject
*              the device and instantiate the next device accordingly.
*
* /NOMANUAL
*/
LOCAL int ataXbdTest
    (
    ATA_XBD    *  ataXbd     /* pointer to XBD */
    )
    {
    int status;

    status = ataXbdMediaTest(ataXbd);
    if (ataXbd->xbdInserted)
        {
        /* We are a full-size XBD - if there is no media, then eject */
        if (status == ERROR)
            {
            /* Removal happened */
	    ATA_DEBUG_MSG(1,"Calling Eject %d/%d, full size and no media\n",
	                     ataXbd->ataDev->ctrl, ataXbd->ataDev->drive, 0, 0,0,0);
            ataXbdEject(ataXbd);
            return (S_ioLib_MEDIA_CHANGED);
            }
        }
    else
        {
        /* We are a 0 size XBD */
        if (status == OK)
            {
	    /* if there IS media then eject */
	    ATA_DEBUG_MSG(1,"Calling Eject %d/%d, zero size and media\n",
	                     ataXbd->ataDev->ctrl, ataXbd->ataDev->drive, 0, 0,0,0);
            ataXbdEject(ataXbd);
            return (S_ioLib_MEDIA_CHANGED);
            }
        else
            {
            /* 0 size and no media, check for media */
            status = ataXbdMediaTest(ataXbd); 
            if (status == OK)
                {
                ataXbdEject(ataXbd);
                return (S_ioLib_MEDIA_CHANGED);
                }
            else
                {
	        ATA_DEBUG_MSG(1,"Returning Error %d/%d, zero size and no media\n",
	                         ataXbd->ataDev->ctrl, ataXbd->ataDev->drive, 0, 0,0,0);
	        return (ENOSPC);
                }
	    }
        }
    return (OK);
    }


/***************************************************************************
*
* ataXbdEject - Eject the device and instantiate the next step
*
* /NOMANUAL
*/

LOCAL int ataXbdEject
    (
    ATA_XBD   *ataXbd
    )
    {
    int error;
    device_t device;
    BLK_DEV *  bd = &ataXbd->ataDev->blkDev;
    struct bio *bio;
    ATA_CTRL * pCtrl = &ataCtrl[ataXbd->ataDev->ctrl];

    ATA_DEBUG_MSG(1,"entered ataXbdEject %d/%d\n",
                     pCtrl->ctrl, ataXbd->ataDev->drive, 0,0,0,0);

    semTake(&pCtrl->muteSem, WAIT_FOREVER);

    /* Announce the ejection */
    erfEventRaise(xbdEventCategory, xbdEventMediaChanged, ERF_ASYNC_PROC,
                  (void *)ataXbd->xbd.xbd_dev.dv_dev, NULL);

    /* Detach the XBD handle */
    xbdDetach(&ataXbd->xbd);

    /* Done any pending bio's */
    while ((bio = ataGetNextBio(pCtrl)))
        bio_done(bio, ENXIO);

    /*
    * Now we instantiate the next XBD - if we have media, then its a regular XBD
    * but if we don't, its an xbd of size 0 which fills in for the XBD until media
    * is actually inserted
    */

    if (ataXbdMediaTest(ataXbd) == OK)
        {
        /* We have new media to mount */
        error = xbdAttach ((struct xbd *) ataXbd, &ataXbdFuncs, ataXbd->name,
                             bd->bd_bytesPerBlk, bd->bd_nBlocks, &device);

        ataXbd->xbdInserted = 1;
        }
    else
        {
        /* We are making a 0-block length XBD */
        error = xbdAttach ((struct xbd *) ataXbd, &ataXbdFuncs, ataXbd->name,
                    512, 0, &device);
        ataXbd->xbdInserted = 0;
        }


    /* Announce our new XBD */
    if (error == OK)
        {
        erfEventRaise(xbdEventCategory, xbdEventPrimaryInsert,
                        ERF_ASYNC_PROC, (void *)device, NULL);
        ATA_DEBUG_MSG(1,"xbdAttached and event raised. device_t = 0x%x xbdSize=0x%x\n",
			 device, bd->bd_nBlocks, 0, 0, 0, 0);
        }

    semGive(&pCtrl->muteSem);
    return (error);
    }

/***************************************************************************
*
* ataCoreDump - write sectors to a ATA/IDE disk with ints off, no OS
*
* Write sectors to a ATA/IDE disk without using any OS services and with
* interrupts off. 
*      <pDev> pointer to device.
*      <startBlk> is the start Block,
*      <nBlks> is the number of blocks,
*      <pBuf> is data buffer pointer 
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/
LOCAL STATUS ataCoreDump
    (
    ATA_DEV *pDev,
    sector_t startBlk,
    UINT32 nBlks,
    char *pBuf
    )
    {
    ATA_CTRL  * pCtrl    = &ataCtrl[pDev->ctrl];
    ATA_DRIVE * pDrive   = &pCtrl->drive[pDev->drive];
    BLK_DEV   * pBlkDev  = &pDev->blkDev;
    ATAPI_TYPE  * pType  = pDrive->driveInfo;
    STATUS       status = ERROR;

    UINT32      retryRW0  = 0;
    UINT32      retryRW1  = 0;
    UINT32      retrySeek = 0;
    UINT32      cylinder;
    UINT32      head;
    UINT32      sector;
    UINT32      nSecs;
    int         ix, level;

#if defined(ATA_DUMP_DEBUG)
    printf("cast of sector_t startBlk=0x%x%x to UINT32 startBlk=0x%x\n",
	   (int)(startBlk>>32), (int)startBlk, (int)startBlk);
#endif

    ataReset(pDrive->pAtaDev);

#ifdef _WRS_VX_SMP
    ATA_SPIN_ISR_TAKE (pCtrl);
#else
    level = intLock();	/* interrupts off for entire operation */
#endif

    /* sanity check */
    if ((!pCtrl->installed) || (pDrive->state != ATA_DEV_OK) || (pBuf == NULL))
        goto done;

    nSecs = pBlkDev->bd_nBlocks;

    if (((UINT32)startBlk + nBlks) > nSecs)
        goto done;

    startBlk += pDev->blkOffset;

    for (ix = 0; ix < nBlks; ix += nSecs)
        {
        if (pDrive->okLba != 0)
            {
            head     = ((UINT32)startBlk & ATA_LBA_HEAD_MASK) >> 24;
            cylinder = ((UINT32)startBlk & ATA_LBA_CYL_MASK) >> 8;
            sector   = ((UINT32)startBlk & ATA_LBA_SECTOR_MASK);
            }
        else
            {
            cylinder = (UINT32)startBlk / (pType->sectors * pType->heads);
            sector   = (UINT32)startBlk % (pType->sectors * pType->heads);
            head     = sector / pType->sectors;
            sector   = sector % pType->sectors + 1;
            }
        nSecs    = min (nBlks - ix, ATA_MAX_RW_SECTORS);

        retryRW1 = 0;
        retryRW0 = 0;

        while (ataCoreWrite (pDev->ctrl, pDev->drive, cylinder,
                      head, sector,pBuf, nSecs, startBlk) != OK)
            {
            if (++retryRW0 > ataRetry)
                {
                (void)ataCmd (pDev->ctrl, pDev->drive, ATA_CMD_RECALIB,
                              0, 0, 0, 0, 0, 0);

                if (++retryRW1 > ataRetry)
                    goto done;

                retrySeek = 0;

                while (ataCmd (pDev->ctrl, pDev->drive, ATA_CMD_SEEK,
                               cylinder, head, 0, 0, 0, 0) != OK)

                    if (++retrySeek > ataRetry)
                        goto done;

                retryRW0 = 0;
                }
            }

        startBlk += nSecs;
        pBuf += pBlkDev->bd_bytesPerBlk * nSecs;
        }

    status = OK;

done:
#ifdef _WRS_VX_SMP
    ATA_SPIN_ISR_GIVE (pCtrl);
#else
    intUnlock(level);	/* interrupts back to previous value */
#endif

    if (status == ERROR)
        (void)errnoSet (S_ioLib_DEVICE_ERROR);

    return(status);
    } /* ataCoreDump */

/**************************************************************************
*
* ataCoreWrite - write a data to disk sectors.
*
* write a number of sectors to the disk with interrupts off
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

LOCAL STATUS ataCoreWrite
    (
    int       ctrl,
    int       drive,
    UINT32    cylinder,
    UINT32    head,
    UINT32    sector,
    void    * buffer,
    UINT32    nSecs,
    sector_t  startBlk
    )
    {
    ATA_CTRL   * pCtrl      = &ataCtrl[ctrl];
    ATA_DRIVE  * pDrive     = &pCtrl->drive[drive];
    ATAPI_TYPE * pType      = pDrive->driveInfo;
    int          retryCount = 0;
    UINT32       block      = 1;
    UINT32       nSectors   = nSecs;
    UINT32       nWords;
    short      * pBuf;
    STATUS	 status = ERROR;
    UINT8	 cmd;

    if (buffer == NULL)
        return(ERROR);

    while (++retryCount <= ataRetry)
        {
        /* wait for device idle: DRQ=0,BSY=0 */
        if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 0) != OK)
            {
            continue;
            }
        pBuf = buffer;

        /*
         * The following code determines if the disk drive supports 48bit logical
         * block addressing and uses that during the read or write operation.
         * Internally the controller has FIFO's at the traditional task register
         * locations.  If the drive supports 48bit LBA, 2 register writes are
         * made to the appropriate registers and the appropriate command issued.
         * The sector count is always written as 2 bytes.  If 48 bit LBA is
         * not supported, the double write has no effect since the final register
         * value is the correct one.
         */
        ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, nSecs >> 8    );
        ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, nSecs         );
        if (pDrive->use48LBA == TRUE)
            {
            ATA_IO_BYTE_WRITE (ATAPI_SECTOR,       (UINT8)((startBlk >> 24) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_SECTOR,       (UINT8)(startBlk & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, (UINT8)((startBlk >> 32) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, (UINT8)((startBlk >> 8) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,  (UINT8)((startBlk >> 40) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,  (UINT8)((startBlk >> 16) & MASK_48BIT));
            ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT,
                               pDrive->okLba  | (drive << ATA_DRIVE_BIT));
            }
        else
            {
            ATA_IO_BYTE_WRITE (ATAPI_SECCNT_INTREASON, nSecs         );
            ATA_IO_BYTE_WRITE (ATAPI_SECTOR,           sector        );
            ATA_IO_BYTE_WRITE (ATAPI_CYLLOW_BCOUNT_LO, cylinder      );
            ATA_IO_BYTE_WRITE (ATAPI_CYLHI_BCOUNT_HI,  cylinder >> 8 );
            ATA_IO_BYTE_WRITE (ATAPI_SDH_D_SELECT,
                pDrive->okLba  | (drive << ATA_DRIVE_BIT) | (head & 0xf));
            }

        nWords = (pType->bytes * block) >> 1;

        if (pDrive->rwPio == ATA_PIO_MULTI)
            {
            block = pDrive->multiSecs;
            if (pDrive->use48LBA == TRUE)
                cmd = ATA_CMD_WRITE_MULTI_EXT;
            else
                cmd = ATA_CMD_WRITE_MULTI;
            }
        else
            {
            if (pDrive->use48LBA == TRUE)
                cmd = ATA_CMD_WRITE_EXT;
            else
                cmd = ATA_CMD_WRITE;
            }

        ATA_IO_BYTE_WRITE (ATAPI_COMMAND, cmd);

        while (nSectors > 0)
            {
            if ((pDrive->rwPio == ATA_PIO_MULTI) && (nSectors < block))
                {
                block = nSectors;
                nWords = (pType->bytes * block) >> 1;
                }

            /* Wait for DRQ and !BSY */
            if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 
                      ATA_STAT_DRQ | !ATA_STAT_BUSY) != OK)
                {
                break;
                }

            if (pDrive->rwBits == ATA_BITS_16)
                ATA_IO_NWORD_WRITE (ATAPI_DATA, (void *)pBuf, nWords);
            else
                ATA_IO_NLONG_WRITE (ATAPI_DATA, (void *)pBuf, nWords >> 1);

            /* wait for device idle: DRQ=0,BSY=0 */
	    if (ataStatusChk(pCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 0) != OK)
		{
		continue;
		}

            pBuf     += nWords;
            nSectors -= block;
	    if (nSectors > 0)
		continue;
	    else
		retryCount = ataRetry;  /* force end of outer while */
            } /* end while (nSectors > 0) */
        }
    return(status);
    } /* ataCoreWrite */


/*******************************************************************************
*
* ataXbdRawio - do raw I/O access
*
* This routine is called to perform raw I/O access.
*
* <device>   is the XBD device identifier for the drive
* <sector>   starting sector for I/O operation
* <numSecs>  number of sectors to read/write
* <data>     pointer to data buffer
* <dir>	     read or write
*
* The <pAtaRaw> is a pointer to the structure ATA_RAW which is defined in
* ataDrv.h.
*
* RETURNS:
* OK, or ERROR if the parameters are not valid.
*
*/
STATUS ataXbdRawio
    (
    device_t	device,
    sector_t	sector,
    UINT32	numSecs,
    char	*data,
    int		direction
    )
    {
    STATUS	status;
    XBD		*xbd;
    ATA_XBD	*ataXbd;

    xbd = (XBD *)devMap(device);
    if (xbd == NULL)
	return (ERROR);

    ataXbd = (ATA_XBD *)xbd;

    status = ataBlkRW (ataXbd->ataDev, (UINT32)sector, numSecs, data,
			direction); 

    devUnmap ((struct device *) xbd);
    return (status);
    }

/*******************************************************************************
*
* ataRawio - do raw I/O access
*
* This routine is called to perform raw I/O access.
*
* <drive> is a drive number for the hard drive: it must be 0 or 1.
*
* The <pAtaRaw> is a pointer to the structure ATA_RAW which is defined in
* ataDrv.h.
*
* RETURNS:
* OK, or ERROR if the parameters are not valid.
*
*/

STATUS ataRawio
    (
    int      ctrl,
    int      drive,
    ATA_RAW  *pAtaRaw
    )
    {
    ATA_CTRL  *pCtrl    = &ataCtrl[ctrl];
    ATA_DRIVE *pDrive   = &pCtrl->drive[drive];
    ATA_TYPE  *pType    = &ataTypes[ctrl][drive];
    ATA_DEV   ataDev;
    BLK_DEV   *pBlkdev  = &ataDev.blkDev;
    UINT      startBlk;

    if ((ctrl >= ATA_MAX_CTRLS) || (drive >= ATA_MAX_DRIVES) ||
        !pCtrl->installed)
        return (ERROR);

    if ((pAtaRaw->cylinder      >= pType->cylinders)    ||
        (pAtaRaw->head          >= pType->heads)        ||
        (pAtaRaw->sector        >  pType->sectors)      ||
        (pAtaRaw->sector        == 0))
        return (ERROR);

    /*
     * if LBA is supported and drive capacity is not zero
     * and drive capacity is greater than the product of
     * CHS, then we should use the LBA value.
     */

    if ((pDrive->okLba != 0)          &&
        (pDrive->capacity != 0)       &&
        (pDrive->capacity > (pType->cylinders * pType->heads * pType->sectors)))
        {
        pBlkdev->bd_nBlocks = (pDrive->capacity && 0xffffffff); /* fix when disks over 2TB */
        }
    else /* just use CHS value */
        {
        pBlkdev->bd_nBlocks     = pType->cylinders * pType->heads *
                                  pType->sectors;
        }

    pBlkdev->bd_bytesPerBlk     = pType->bytes;
    pBlkdev->bd_blksPerTrack    = pType->sectors;
    pBlkdev->bd_nHeads          = pType->heads;
    pBlkdev->bd_removable       = FALSE;
    pBlkdev->bd_retry           = 1;
    pBlkdev->bd_mode            = O_RDWR;
    pBlkdev->bd_readyChanged    = TRUE;
    pBlkdev->bd_blkRd           = ataBlkRd;
    pBlkdev->bd_blkWrt          = ataBlkWrt;
    pBlkdev->bd_ioctl           = ataBlkIoctl;
    pBlkdev->bd_reset           = ataReset;
    pBlkdev->bd_statusChk       = ataStatus;

    ataDev.ctrl                 = ctrl;
    ataDev.drive                = drive;
    ataDev.blkOffset            = 0;

    startBlk = pAtaRaw->cylinder * (pType->sectors * pType->heads) +
               pAtaRaw->head * pType->sectors + pAtaRaw->sector - 1;

    return (ataBlkRW (&ataDev, (sector_t)startBlk, pAtaRaw->nSecs, pAtaRaw->pBuf,
                     pAtaRaw->direction));
    }



/*
 * ataDecRawIoTest
 *
 * This function is used to test the whole ata device's setctors read write operation.
 */

extern STATUS ataDecRawIoTest( int ctrl, int drive )
{
	ATA_PARAM pParam;
	ATA_RAW rawAta;
	int head, cylinder, sector, cap, sec;
	char buffer[ 1024 ];
	STATUS rv;
	int i;

	printf( "This operation will destroy all the data in the CF card.\n" );
	/*STATUS ataParamRead
    (
    int      ctrl,
    int      drive,
    void     *buffer,
    int      command
    )*/
	rv = ataParamRead( ctrl, drive, &pParam, ATA_CMD_IDENT_DEV);
	if( rv == ERROR )
	{
		printf( "Reading the parameter of the ata device error.\n" );
		return rv;
	}

	/*ataDevParamDump( &pParam );*/
	if( (UINT16) pParam.config != 0x848A )
	{
		printf( "Not a valid CF device: %x.\n", (UINT16) pParam.config );
		return rv;
	}
	
	printf( "The default number of cylinders is %x.\n", pParam.cylinders );
	printf( "The default number of heads is %x.\n", pParam.heads );
	printf( "The default number of sectors per track is %x.\n", pParam.sectors );

	if( pParam.valid != 0x0001 )
	{
		printf( "Could not get the user addressable parameters for this device.\n" );
		return ERROR;
	}
	
	printf( "The user addressable number of cylinders is %x.\n", pParam.currentCylinders );
	printf( "The user addressable number of heads is %x.\n", pParam.currentHeads );
	printf( "The user addressable number of sectors per track is %x.\n", pParam.currentSectors );
	cap = ( (UINT16) pParam.capacity1 << 16 ) + (UINT16) pParam.capacity0;
	printf( "The user addressable sectors for this device is %x\n", cap );
	if( cap != pParam.currentCylinders * pParam.currentHeads * pParam.currentSectors )
	{
		printf( "The user addressable number of sectors is not right.\n" );
		return ERROR;
	}

	printf( "Writing and verifying all the user addressable sectors...\n" );
	getchar( );

	/* Writing every sectors with 0x55aa */
	for( i = 0; i < 512; i += 2 )
	{
		buffer[ i ] = 0x55;
		buffer[ i + 1 ] = 0xAA;
	};
	
	for( head = 0; head < pParam.currentHeads; head++ )
	for( cylinder = 0; cylinder < pParam.currentCylinders - 1; cylinder++ )
	for( sector = 1; sector <= pParam.currentSectors; sector++ )
	{
		rawAta.cylinder = cylinder;
		rawAta.head = head;
		rawAta.sector = sector;
		rawAta.pBuf = buffer;
		rawAta.nSecs = 1;
		rawAta.direction = 1;
		
		rv = ataRawio( ctrl, drive, &rawAta );
		if( rv == ERROR )
		{
			printf( "Writing to the sector at head: cylinder: sector - %x:%x:%x error.\n", head, cylinder, sector );
			return ERROR;
		}
	}

	for( head = 0; head < pParam.currentHeads; head++ )
	for( cylinder = 0; cylinder < pParam.currentCylinders - 1; cylinder++ )
	for( sector = 1; sector <= pParam.currentSectors; sector++ )
	{
		rawAta.cylinder = cylinder;
		rawAta.head = head;
		rawAta.sector = sector;
		rawAta.pBuf = buffer;
		rawAta.nSecs = 1;
		rawAta.direction = 0;

		rv = ataRawio( ctrl, drive, &rawAta );
		if( rv == ERROR )
		{
			printf( "Reading sector at head: cylinder: sector - %x:%x:%x error.\n", head, cylinder, sector );
			return ERROR;
		}

		/* Verifying every sectors data written */
		for( i = 0; i < 512; i += 2 )
		{
			if( ( buffer[ i ] != 0x55 ) || ( buffer[ i + 1 ] != 0xAA ) )
			{
				printf( "Data written verifying error.\n" );
				return ERROR;
			};
		};		
	}

	sec = 0;
	for( head = 0; head < pParam.currentHeads; head++ )
	for( cylinder = 0; cylinder < pParam.currentCylinders - 1; cylinder++ )
	for( sector = 1; sector <= pParam.currentSectors; sector++ )
	{
	
		rawAta.cylinder = cylinder;
		rawAta.head = head;
		rawAta.sector = sector;
		rawAta.pBuf = buffer;
		rawAta.nSecs = 1;
		rawAta.direction = 1;

	/* Writing every sectors with 0xaa55 */
	for( i = 0; i < 512; i += 2 )
	{
		buffer[ i ] = 0xaa;
		buffer[ i + 1 ] = 0x55;
	};
		
		rv = ataRawio( ctrl, drive, &rawAta );
		if( rv == ERROR )
		{
			printf( "Writing to the sector at head: cylinder: sector - %x:%x:%x error.\n", head, cylinder, sector );
			return ERROR;
		}
	}

	for( head = 0; head < pParam.currentHeads; head++ )
	for( cylinder = 0; cylinder < pParam.currentCylinders - 1; cylinder++ )
	for( sector = 1; sector <= pParam.currentSectors; sector++ )
	{
			
		rawAta.cylinder = cylinder;
		rawAta.head = head;
		rawAta.sector = sector;
		rawAta.pBuf = buffer;
		rawAta.nSecs = 1;
		rawAta.direction = 0;
		
		rv = ataRawio( ctrl, drive, &rawAta );
		if( rv == ERROR )
		{
			printf( "Reading sector at head: cylinder: sector - %x:%x:%x error.\n", head, cylinder, sector );
			return ERROR;
		}

		/* Verifying every sectors data written */
		for( i = 0; i < 512; i += 2 )
		{
			if( ( buffer[ i ] != 0xaa ) || ( buffer[ i + 1 ] != 0x55 ) )
			{
				printf( "Data written verifying error.\n" );
				return ERROR;
			};
		};		
	}

	sec = 0;

	for( head = 0; head < pParam.currentHeads; head++ )
	for( cylinder = 0; cylinder < pParam.currentCylinders - 1; cylinder++ )
	for( sector = 1; sector <= pParam.currentSectors; sector++ )
	{

		/* Writing every sectors with linear values */
		for( i = 0; i < 512; i += 2 )
		{
			buffer[ i ] = sec & 0xFF;
			buffer[ i + 1 ] = ( sec & 0xFF00 ) >> 8;
		}
		sec++;

		rawAta.cylinder = cylinder;
		rawAta.head = head;
		rawAta.sector = sector;
		rawAta.pBuf = buffer;
		rawAta.nSecs = 1;
		rawAta.direction = 1;
		
		rv = ataRawio( ctrl, drive, &rawAta );
		if( rv == ERROR )
		{
			printf( "Writing to the sector at head: cylinder: sector - %x:%x:%x error.\n", head, cylinder, sector );
			return ERROR;
		}
	}

	sec = 0;
	for( head = 0; head < pParam.currentHeads; head++ )
	for( cylinder = 0; cylinder < pParam.currentCylinders - 1; cylinder++ )
	for( sector = 1; sector <= pParam.currentSectors; sector++ )
	{

		rawAta.cylinder = cylinder;
		rawAta.head = head;
		rawAta.sector = sector;
		rawAta.pBuf = buffer;
		rawAta.nSecs = 1;
		rawAta.direction = 0;
		
		rv = ataRawio( ctrl, drive, &rawAta );
		if( rv == ERROR )
		{
			printf( "Reading sector at head: cylinder: sector - %x:%x:%x error.\n", head, cylinder, sector );
			return ERROR;
		}

		/* Writing every sectors with linear values */
		for( i = 0; i < 512; i += 2 )
		{
			if( buffer[ i ] != ( sec & 0xFF ) )
			{
				printf( "Data written verifying error.i: %x, sec: %x\n", i, sec );
				printf( "buffer[ i ]: %x = sec & 0xFF: %x\n", buffer[ i ], sec & 0xFF );
				return ERROR;
			};
			if( buffer[ i + 1 ] != ( ( sec & 0xFF00 ) >> 8 ) )
			{
				printf( "Data written verifying error.\n" );
				return ERROR;
			};
		}
		sec++;
		
	}
	
	return OK;
}

/*
 * ataBandwidthTest
 *
 * This function is used to test the speed of the ATA device.
 */
extern int ataBandwidthTest( int ctrl, int drive )
{
	ATA_PARAM pParam;
	ATA_RAW rawAta;
	char buffer[ 1024 ];
	int i, rv;
	int startTime, endTime, time;
	int bytesRead = 0;
	int bandwidth;
	int head, cylinder, sector, cap;
	int consoleFd;

	printf( "This operation will destroy all the data in the CF card.\n" );
	
	//rv = ataPread( ctrl, drive, &pParam );
		rv = ataParamRead( ctrl, drive, &pParam, ATA_CMD_IDENT_DEV);
	if( rv == ERROR )
	{
		printf( "Reading the parameter of the ata device error.\n" );
		return rv;
	}

	/*ataDevParamDump( &pParam );*/
	if( (UINT16) pParam.config != 0x848A )
	{
		printf( "Not a valid CF device: %x.\n", (UINT16) pParam.config );
		return rv;
	}
	
	printf( "The default number of cylinders is %x.\n", pParam.cylinders );
	printf( "The default number of heads is %x.\n", pParam.heads );
	printf( "The default number of sectors per track is %x.\n", pParam.sectors );

	if( pParam.valid != 0x0001 )
	{
		printf( "Could not get the user addressable parameters for this device.\n" );
		return ERROR;
	}
	
	printf( "The user addressable number of cylinders is %x.\n", pParam.currentCylinders );
	printf( "The user addressable number of heads is %x.\n", pParam.currentHeads );
	printf( "The user addressable number of sectors per track is %x.\n", pParam.currentSectors );
	
	cap = ( (UINT16) pParam.capacity1 << 16 ) + (UINT16) pParam.capacity0;
	printf( "The user addressable sectors for this device is %x\n", cap );
	if( cap != pParam.currentCylinders * pParam.currentHeads * pParam.currentSectors )
	{
		printf( "The user addressable number of sectors is not right.\n" );
		return ERROR;
	}

	/* Writing every sectors with 0x55aa */
	for( i = 0; i < 512; i++ )
	{
		buffer[ i ] = i;
	};

    consoleFd = open ("/tyCo/0", O_RDWR, 0);

	i = 0;

	printf( "Press any key to start this test...\n" );
	getchar( );
	printf( "Press any key to stop this operation..." );

	startTime = tickGet ();
	
	while( bytesRead == 0 )
	{	
		for( head = 0; head < pParam.currentHeads; head++ )
		for( cylinder = 0; cylinder < pParam.currentCylinders - 1; cylinder++ )
		for( sector = 1; sector <= pParam.currentSectors; sector++ )
		{

			rawAta.cylinder = cylinder;
			rawAta.head = head;
			rawAta.sector = sector;
			rawAta.pBuf = buffer;
			rawAta.nSecs = 1;
			rawAta.direction = 0;
			
			rawAta.direction = 1;
			rv = ataRawio( ctrl, drive, &rawAta );
			if( rv == ERROR )
			{
				printf( "Writing to the sector at CHS: 0-0-1 error.\n" );
				return ERROR;
			}
			i++;
			
			rawAta.direction = 0;
		
			rv = ataRawio( ctrl, drive, &rawAta );
			if( rv == ERROR )
			{
				printf( "Reading sector at CHS: 0-0-1 error.\n" );
				return ERROR;
			}
			i++;
			(void) ioctl (consoleFd, FIONREAD, (int) &bytesRead);
			if( bytesRead != 0 )
				goto end;
		}
	}
	
end:
	endTime = tickGet ();

	time = ( endTime - startTime );

	bandwidth = ( i * 512 * sysClkRateGet () ) / time;
	printf( "time: %x, startTime: %x, endTime: %x, tick: %x\n", i, startTime, endTime, sysClkRateGet () );
	printf( "\nThe bandwidth is 0x%x bytes/second.\n", bandwidth );
	
	return bandwidth;
}
