/* ataShow.c - ATA/IDE (LOCAL and PCMCIA) disk device driver show routine */

/* 
 * Copyright (c) 2005 - 2007 Wind River Systems, Inc. 
 * 
 * The right to copy, distribute, modify, or otherwise make use 
 * of this software may be licensed only pursuant to the terms 
 * of an applicable Wind River license agreement. 
 */
 
/*
modification history
--------------------
01p,02jul07,dee  WIND00084764 - support large disk drives, cleanup
01o,12may06,dgp  doc: fix headers, table, list, & indention fmt
01n,22feb06,dee  SPR# 118025, show device_t for displayed device
01m,13oct05,dee  SPR# 113433, better error handling for non-existent ATA
		 drives
01l,08sep05,dee  SPR#111791,111796.  Display more device parameters in
                 ataShow()
01k,31aug05,pcm  fixed documenation build errors
01j,11jan02,bsp  ataShow changed due to changes in driver structure.
01i,18mar99,jdi  doc: updated w/ info about proj facility (SPR 25727).
01h,15jan99,jkf  added endian ifdef to properly display revision
                 and model strings on big endian arches.
01g,11nov96,dgp  doc: final formatting
01f,11nov96,hdn  removed NOMANUAL from ataShowInit() and ataShow().
01e,01nov96,hdn  added support for PCMCIA.
01d,25sep96,hdn  added support for ATA-2.
01c,18mar96,hdn  added ataShowInit().
01b,01mar96,hdn  cleaned up.
01a,02mar95,hdn  written based on ideDrv.c.
*/

/*
DESCRIPTION
This library contains a driver show routine for the ATA/IDE (PCMCIA and LOCAL) 
devices supported on the IBM PC.

*/

#include <vxWorks.h>
#include <ioLib.h>
#include <stdlib.h>
#include <errnoLib.h>
#include <stdio.h>
#include <string.h>
#include <drv/pcmcia/pccardLib.h>
#include "ataDrv.h"
#define  _BYTE_ORDER  _LITTLE_ENDIAN

/* imports */

IMPORT ATA_CTRL     ataCtrl[];

LOCAL void ataAdjustAndCopyByte (char * pbuf, char * pSource);

/* globals */


/* locals */
LOCAL char ataVersionNum[8];            /* array for version number */
LOCAL char ataModelNum[40];             /* array for Model number */
LOCAL char ataSerialNum[20];            /* array for serial number */


/* function prototypes */
void ataDmaToggle(int ctrl);
XBD *devMapUnsafe(device_t);
void devUnmapUnsafe (struct device *);

/**************************************************************************
*
* ataShowInit - initialize the ATA/IDE disk driver show routine
*
* This routine links the ATA/IDE disk driver show routine into the VxWorks
* system.  It is called automatically when this show facility is configured
* into VxWorks using either of the following methods:
* .iP
* If you use the configuration header files, define
* INCLUDE_SHOW_ROUTINES in config.h.
* .iP
* If you use the Tornado project facility, select INCLUDE_ATA_SHOW.
*
* RETURNS: N/A
*/

STATUS ataShowInit (void)
    {
    return(OK);
    }

/**************************************************************************
*
* ataShow - show the ATA/IDE disk parameters
*
* This routine shows the ATA/IDE disk parameters.  Its first argument is a
* controller number, 0 or 1; the second argument is a drive number, 0 or 1.
*
* RETURNS: OK, or ERROR if the parameters are invalid.
*/

STATUS ataShow
(
int ctrl,
int drive
)
    {
    ATA_CTRL      *  pCtrl  = &ataCtrl[ctrl];
    ATA_DRIVE     *  pDrive = &pCtrl->drive[drive];
    ATA_PARAM     *  pParam = &pDrive->param;
    ATAPI_TYPE    *  pType  = pDrive->driveInfo;
    char *pMode             = "UNKNOWN";
    int ix;

    /*
     * If the controller or drive number is out of bounds,
     * or the controller is not installed or the drive doesn't exist
     * return ERROR
     */
    if ((ctrl >= ATA_MAX_CTRLS) || (drive >= ATA_MAX_DRIVES) ||
              !pCtrl->installed || !pDrive->driveInfo)
	{
	printErr("Device %d on Controller %d not installed\n", drive,ctrl);
        return(ERROR);
	}
    for (ix = 14; ix >= 2; ix--)
	{
        if (pParam->majorVer & (short)(1 << ix))
            break;
        }

    if (pDrive->type == ATA_TYPE_ATA)
        printf("  device type    : ATA/ATAPI-%d ATA device \n", ix);
    else if (pDrive->type == ATA_TYPE_ATAPI)
        printf("  device type    : ATA/ATAPI-%d ATAPI device \n", ix);
    else
	{
	printf("    Unrecognized device type device installed"
               " at ctrl %d, device %d\n", ctrl, drive);
	return (ERROR);
	}

    if (pDrive->pAtaDev->ataXbd.device == 0)
	{
	printf("   Device installed but not initialized\n"
	       " at ctrl %d, device %d\n", ctrl, drive);
	return (ERROR);
	}

    printf("  XBD device_t   : 0x%x\n", pDrive->pAtaDev->ataXbd.device);

    if (pDrive->driveType == CONFIG_DEV_TYPE_CD_ROM)
        printf("  drive type     : CD-ROM device\n");
    else if (pDrive->driveType == CONFIG_DEV_TYPE_DIRECT)
        printf("  drive type     : Direct-access device\n");

    printf("  removable media: %s\n\n", pDrive->okRemovable ? "YES" : "NO");

    printf ("  intCount      =%-8d    intStatus   =0x%-8x\n",
            pCtrl->intCount, pCtrl->intStatus);
    printf ("\n");      
    printf ("ataTypes\n");
    printf ("  cylinders     =%-8d    heads       =%-8d\n",
            pType->cylinders, pType->heads);
    printf ("  sectorsTrack  =%-8d    bytesSector =%-8d\n",
            pType->sectors, pType->bytes);
    printf ("  precomp       =0x%-8x\n", pType->precomp);
    printf ("\n");
    printf ("ataParams\n");
    printf ("  cylinders     =%-8d    heads       =%-8d\n",
            (USHORT)pParam->cylinders, (USHORT)pParam->heads);  
    printf ("  config        =0x%-8x  specConfig  =0x%-8x\n",
            (USHORT)pParam->config, (USHORT)pParam->specConfig);
    printf ("  sectorsTrack  =%-8d  \n",(USHORT)pParam->sectors);
    printf ("  serial number =");
    for (ix = 0; ix < 10; ix++)
        {
#if (_BYTE_ORDER == _LITTLE_ENDIAN)
        printf ("%c", pParam->serial[ix * 2 + 1]);
        printf ("%c", pParam->serial[ix * 2]);
#else
        printf ("%c", pParam->serial[ix * 2]);
        printf ("%c", pParam->serial[ix * 2 + 1]);
#endif /* (_BYTE_ORDER == _LITTLE_ENDIAN) */

        }
    printf ("\n");

    printf ("  rev           =");

    for (ix = 0; ix < 4; ix++)
        {
#if (_BYTE_ORDER == _LITTLE_ENDIAN)
        printf ("%c", pParam->rev[ix * 2 + 1]);
        printf ("%c", pParam->rev[ix * 2]);
#else
        printf ("%c", pParam->rev[ix * 2]);
        printf ("%c", pParam->rev[ix * 2 + 1]);
#endif /* (_BYTE_ORDER == _LITTLE_ENDIAN) */
        }

    printf ("\n");
    printf ("  model         =");

    for (ix = 0; ix < 20; ix++)
        {
#if (_BYTE_ORDER == _LITTLE_ENDIAN)
        printf ("%c", pParam->model[ix * 2 + 1]);
        printf ("%c", pParam->model[ix * 2]);
#else
        printf ("%c", pParam->model[ix * 2]);
        printf ("%c", pParam->model[ix * 2 + 1]);
#endif /* (_BYTE_ORDER == _LITTLE_ENDIAN) */
        }

    printf ("\n  capacity      =0x%012x sectors\n", pDrive->capacity);
    printf ("\n");
    printf ("  multiSecs     =0x%-8x  capability   =0x%-8x\n",
            (USHORT)pParam->multiSecs, (USHORT)pParam->capabilities);
    printf ("  valid         =0x%-8x  curr-cyl    =%-8d\n",
            (USHORT)pParam->valid, (USHORT)pParam->currentCylinders);
    printf ("  curr-head     =%-8d    curr-sector =%-8d\n",
            (USHORT)pParam->currentHeads, (USHORT)pParam->currentSectors);
    printf ("  capacity0     =0x%-8x  capacity1   =0x%-8d\n",
            (USHORT)pParam->capacity0, (USHORT)pParam->capacity1);
    printf ("  multiSet      =0x%-8x  sectors0    =0x%-8x\n",
            (USHORT)pParam->multiSet, (USHORT)pParam->lba_size_1);
    printf ("  sectors1      =0x%-8x  singleDma   =0x%-8x\n",
            (USHORT)pParam->lba_size_2, (USHORT)pParam->singleDma);
    printf ("  multiDma      =0x%-8x  advancedPio =0x%-8x\n",
            (USHORT)pParam->multiDma, (USHORT)pParam->advancedPio);
    printf ("  cycleDma      =%-8d    cycleMulti  =%-8d\n",
            (USHORT)pParam->cycleTimeDma, (USHORT)pParam->cycleTimeMulti);
    printf ("  cyclePio-wo   =%-8d    cyclePio-w  =%-8d\n",
            (USHORT)pParam->cycleTimePioNoIordy, 
            (USHORT)pParam->cycleTimePioIordy);

    printf ("Capability\n");

    printf ("  MULTI: %s, IORDY: %s, DMA: %s, LBA: %s\n",
            pDrive->okMulti ? "TRUE" : "FALSE",
            pDrive->okIordy ? "TRUE" : "FALSE",
            pDrive->okDma ? "TRUE" : "FALSE",
            pDrive->okLba ? "TRUE" : "FALSE");
    printf ("  multiSectors  =0x%-8x  pioMode     =0x%-8x\n",
            (USHORT)pDrive->multiSecs, (USHORT)pDrive->pioMode);
    printf ("  singleDma     =0x%-8x  multiDma    =0x%-8x\n",
            (USHORT)pDrive->singleDmaMode, (USHORT)pDrive->multiDmaMode);
    printf ("  ultraDma      =0x%-8x  \n",(USHORT)pDrive->ultraDmaMode);


    printf ("Configuration\n");

    switch (pDrive->rwMode)
        {
        
        case ATA_PIO_DEF_W:      
            pMode = "PIO_DEF_W ";  
            break;               
        case ATA_PIO_DEF_WO :      
            pMode = "PIO_DEF_WO ";  
            break;               
        case ATA_PIO_W_0 :      
            pMode = "PIO_W_0 ";  
            break;                  
        case ATA_PIO_W_1 :      
            pMode = "PIO_W_1 ";  
            break;                          
        case ATA_PIO_W_2 :      
            pMode = "PIO_W_2 ";  
            break;                    
        case ATA_PIO_W_3 :      
            pMode = "PIO_W_3 ";  
            break;   
        case ATA_PIO_W_4 :      
            pMode = "PIO_W_4 ";  
            break;               
        case ATA_DMA_SINGLE_0  :      
            pMode = "DMA_SINGLE_0 ";  
            break;               
        case ATA_DMA_SINGLE_1  :      
            pMode = "DMA_SINGLE_1  ";  
            break;                                              
        case ATA_DMA_SINGLE_2  :      
            pMode = "DMA_SINGLE_2  ";  
            break;                                 

        case ATA_DMA_MULTI_0   :      
            pMode = "DMA_MULTI_0 ";  
            break;               
        case ATA_DMA_MULTI_1  :      
            pMode = "DMA_MULTI_1  ";  
            break;                                              
        case ATA_DMA_MULTI_2   :      
            pMode = "DMA_MULTI_2  ";  
            break;                                            
        case ATA_DMA_ULTRA_0    :      
            pMode = "DMA_ULTRA_0 ";  
            break;               
        case ATA_DMA_ULTRA_1  :      
            pMode = "DMA_ULTRA_1  ";  
            break;                                              
        case ATA_DMA_ULTRA_2    :      
            pMode = "DMA_ULTRA_2  ";  
            break;                               
        case ATA_DMA_ULTRA_3  :      
            pMode = "DMA_ULTRA_3  ";  
            break;                                              
        case ATA_DMA_ULTRA_4    :      
            pMode = "DMA_ULTRA_4  ";  
            break;                               
        case ATA_DMA_ULTRA_5    :      
            pMode = "DMA_ULTRA_5  ";  
            break;                           

        }

    printf ("  rwMode        =%-8s    rwBits      =%-8s\n",
            pMode, (pDrive->rwBits == ATA_BITS_16) ? "16BITS  " : "32BITS  ");


    return(OK);
    }

/**************************************************************************
*
* ataDmaToggle - turn on or off an individual controllers dma support
*
* This routine lets you toggle the DMA setting for an individual
* controller.  The controller number is passed in as a parameter, and
* the current value is toggled.
*
* RETURNS: OK, or ERROR if the parameters are invalid.
*/

void ataDmaToggle
    (
    int ctrl
    )
    {
    ATA_CTRL      *  pCtrl  = &ataCtrl[ctrl];

    if (pCtrl->ataHostDmaSupportOkay == TRUE)
        pCtrl->ataHostDmaSupportOkay = FALSE;
    else
        pCtrl->ataHostDmaSupportOkay = TRUE;

    printf("Controller %d DMA is %d\n", ctrl, pCtrl->ataHostDmaSupportOkay);
    }

/**************************************************************************
*
* atapiCylinderCountGet - get the number of cylinders in the drive.
*
* This function is used to get cyclinder count of the ATA/ATAPI drive specified 
* by <ctrl> and <drive> from drive structure.
*
* RETURNS: Cylinder count.
*/

UINT16 atapiCylinderCountGet
(
int ctrl,
int drive
)
    {
    return(UINT16)(ataCtrl[ ctrl].drive[ drive].param.cylinders);
    } /* atapiCylinderCountGet */


/**************************************************************************
*
* atapiHeadCountGet - get the number heads in the drive.
*
* This function is used to get head count of the ATA/ATAPI drive specified 
* by <ctrl> and <drive> from drive structure.
*
* RETURNS: Number of heads in the drive.
*/

UINT8 atapiHeadCountGet
(
int ctrl,
int drive
)
    {
    return(UINT8)(ataCtrl[ ctrl].drive[ drive].param.heads);
    } /* atapiHeadCountGet */


/**************************************************************************
*
* atapiDriveSerialNumberGet - get the drive serial number.
*
* This function is used to get drive serial number  of the ATA/ATAPI drive 
* specified by <ctrl> and <drive> from drive structure. It returns a pointer to
* character array of 20 bytes length which contains serial number in ascii.
*
* RETURNS: Drive serial number.
*/

char * atapiDriveSerialNumberGet
(
int ctrl,
int drive
)
    {
    ataAdjustAndCopyByte(ataSerialNum,
                         (ataCtrl[ ctrl].drive[ drive].param.serial));           
    return(ataSerialNum);
    } /* atapiDriveSerialNumberGet */


/**************************************************************************
*
* atapiFirmwareRevisionGet - get the firm ware revision of the drive.
*
* This function is used to get drive Firmware revision of the ATA/ATAPI drive 
* specified by <ctrl> and <drive> from drive structure. It returns a pointer to
* character array of 8 bytes length which contains serial number in ascii.
*
* RETURNS: firmware revision.
*/

char * atapiFirmwareRevisionGet
(
int ctrl,
int drive
)
    {

    ataAdjustAndCopyByte(ataVersionNum,
                         (ataCtrl[ ctrl].drive[ drive].param.rev));              
    return(ataVersionNum);
    } /* atapiFirmwareRevisionGet */


/**************************************************************************
*
* atapiModelNumberGet - get the model number of the drive.
*
* This function is used to get drive Model Number of the ATA/ATAPI drive 
* specified by <ctrl> and <drive> from drive structure. It returns a pointer to
* character array of 40 bytes length which contains serial number in ascii.
*
* RETURNS: pointer to the model number.
*/

char * atapiModelNumberGet
(
int ctrl,
int drive
)
    {
    ataAdjustAndCopyByte(ataModelNum,
                         (ataCtrl[ ctrl].drive[ drive].param.model));            
    return(ataModelNum);
    } /* atapiModelNumberGet */


/**************************************************************************
*
* atapiFeatureSupportedGet - get the features supported by the drive.
*
* This function is used to get drive Feature supported by the ATA/ATAPI drive 
* specified by <ctrl> and <drive> from drive structure. It returns a 32 bit 
* value whose bits represents the features supported. The following table gives 
* the cross reference for the bits.
*
* \ts
* Bit 21 | Power-up in Standby Feature
* Bit 20 | Removable Media Status Notification Feature
* Bit 19 | Adavanced Power Management Feature
* Bit 18 | CFA Feature
* Bit 10 | Host protected Area Feature
* Bit 4  | Packet Command Feature
* Bit 3  | Power Management Feature
* Bit 2  | Removable Media Feature
* Bit 1  | Security Mode Feature
* Bit 0  | SMART Feature
* \te
*
* RETURNS: Supported features.
*/

UINT32 atapiFeatureSupportedGet
(
int ctrl,
int drive
)
    {
    UINT32 featureSupport;

    featureSupport = ((ataCtrl[ ctrl].drive[ drive].param.suppCommand2)<<16) |
                     (ataCtrl[ ctrl].drive[ drive].param.suppCommand1);
    return(featureSupport);
    } /* atapiFeatureSupportedGet */


/**************************************************************************
*
* atapiFeatureEnabledGet - get the enabled features.
*
* This function is used to get drive Features Enabled by the ATA/ATAPI drive 
* specified by <ctrl> and <drive> from drive structure. It returns a 32 bit 
* value whose bits represents the features Enabled. The following table gives 
* the cross reference for the bits.
*
* \ts
* Bit 21 | Power-up in Standby Feature
* Bit 20 | Removable Media Status Notification Feature
* Bit 19 | Adavanced Power Management Feature
* Bit 18 | CFA Feature
* Bit 10 | Host protected Area Feature
* Bit 4  | Packet Command Feature
* Bit 3  | Power Management Feature
* Bit 2  | Removable Media Feature
* Bit 1  | Security Mode Feature
* Bit 0  | SMART Feature
* \te
*
* RETURNS: enabled features.
*/

UINT32 atapiFeatureEnabledGet
(
int ctrl,
int drive
)
    {
    UINT32 featureEnabled;

    featureEnabled=((ataCtrl[ctrl].drive[drive].param.enableCommandFeature2)<<16
                   )|(ataCtrl[ctrl].drive[drive].param.enableCommandFeature1);
    return(featureEnabled);
    } /* atapiFeatureEnabledGet */


/**************************************************************************
*
* atapiMaxUDmaModeGet - get the Maximum Ultra DMA mode the drive can support.
*
* This function is used to get drive maximum UDMA mode supported  by the 
* ATA/ATAPI drive specified by <ctrl> and <drive> from drive structure.
* The following bits are set for corresponding modes supported.
*
* \is
* \i Bit4 Ultra DMA mode 4 and below are supported
* \i Bit3 Ultra DMA mode 3 and below are supported
* \i Bit2 Ultra DMA mode 2 and below are supported
* \i Bit1 Ultra DMA mode 1 and below are supported
* \i Bit0 Ultra DMA mode 0 is supported
* \ie
* RETURNS: Maximum Ultra DMA mode.
*/

UINT8 atapiMaxUDmaModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)(ataCtrl[ ctrl].drive[ drive].param.ultraDmaMode);
    } /* atapiMaxUDmaModeGet */


/**************************************************************************
*
* atapiCurrentUDmaModeGet - get the enabled Ultra DMA mode.
*
* This function is used to get drive  UDMA mode enable  in the 
* ATA/ATAPI drive specified by <ctrl> and <drive> from drive structure
* The following bit is set for corresponding mode selected.
*
* \ms
* \m -
* Bit4 Ultra DMA mode 4 is Selected
* \m -
* Bit3 Ultra DMA mode 3 is Selected
* \m -
* Bit2 Ultra DMA mode 2 is Selected
* \m -
* Bit1 Ultra DMA mode 1 is Selected
* \m -
* Bit0 Ultra DMA mode 0 is Selected
* \me
*
* RETURNS: Enabled Ultra DMA mode.
*/

UINT8 atapiCurrentUDmaModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)((ataCtrl[ ctrl].drive[ drive].param.ultraDmaMode)>>8);
    } /* atapiCurrentUDmaModeGet */


/**************************************************************************
*
* atapiMaxMDmaModeGet - get the Maximum Multi word DMA mode the drive supports.
*
* This function is used to get drive maximum MDMA mode supported  by the 
* ATA/ATAPI drive specified by <ctrl> and <drive> from drive structure
* The following bits are set for corresponding modes supported.
*
* \ts
* Bit2 | Multi DMA mode 2 and below are supported
* Bit1 | Multi DMA mode 1 and below are supported
* Bit0 | Multi DMA mode 0 is supported
* \te
*
* RETURNS: Maximum Multi word DMA mode.
*/

UINT8 atapiMaxMDmaModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)(ataCtrl[ ctrl].drive[ drive].param.multiDma);
    } /* atapiMaxMDmaModeGet */


/**************************************************************************
*
* atapiCurrentMDmaModeGet - get the enabled Multi word DMA mode.
*
* This function is used to get drive  MDMA mode enable  in the 
* ATA/ATAPI drive specified by <ctrl> and <drive> from drive structure.
* The following bit is set for corresponding mode selected.
*
* \ms
* \m -
* Bit2 Multi DMA mode 2 is Selected
* \m -
* Bit1 Multi DMA mode 1 is Selected
* \m -
* Bit0 Multi DMA mode 0 is Selected
* \me
*
* RETURNS: Enabled Multi word DMA mode.
*/

UINT8 atapiCurrentMDmaModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)((ataCtrl[ ctrl].drive[ drive].param.dmaMode)>>8);
    } /* atapiCurrentMDmaModeGet */


/**************************************************************************
*
* atapiMaxSDmaModeGet - get the Maximum Single word DMA mode the drive supports
*
* This function is used to get drive maximum SDMA mode supported  by the 
* ATA/ATAPI drive specified by <ctrl> and <drive> from drive structure
*
* RETURNS: Maximum Single word DMA mode.
*/

UINT8 atapiMaxSDmaModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)(ataCtrl[ ctrl].drive[ drive].param.singleDma);
    } /* atapiMaxSDmaModeGet */


/**************************************************************************
*
* atapiCurrentSDmaModeGet - get the enabled Single word DMA mode.
*
* This function is used to get drive  SDMA mode enable  in the 
* ATA/ATAPI drive specified by <ctrl> and <drive> from drive structure
*
* RETURNS: Enabled Single word DMA mode.
*/

UINT8 atapiCurrentSDmaModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)((ataCtrl[ ctrl].drive[ drive].param.singleDma)>>8);
    } /* atapiCurrentSDmaModeGet */


/**************************************************************************
*
* atapiMaxPioModeGet - get the Maximum PIO mode that drive can support.
*
* This function is used to get drive maximum PIO mode supported  by the 
* ATA/ATAPI drive specified by <ctrl> and <drive> from drive structure
*
* RETURNS: maximum PIO mode.
*/

UINT8 atapiMaxPioModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)(ataCtrl[ ctrl].drive[ drive].pioMode);
    } /* atapiMaxPioModeGet */


/**************************************************************************
*
* atapiCurrentPioModeGet - get the enabled PIO mode.
*
* This function is used to get drive  current PIO mode enabled  in the 
* ATA/ATAPI drive specified by <ctrl> and <drive> from drive structure.
*
* RETURNS: Enabled PIO mode.
*/

UINT8 atapiCurrentPioModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)(ataCtrl[ ctrl].drive[ drive].pioMode);
    } /* atapiCurrentPioModeGet */


/**************************************************************************
*
* atapiCurrentRwModeGet - get the current Data transfer mode.
*
* This function will return the current Data transfer mode if it is
* PIO 0,1,2,3,4 mode, SDMA 0,1,2 mode, MDMA 0,1,2 mode or UDMA 0,1,2,3,4,5 
* mode.
*
* RETURNS: current PIO mode.
*/

UINT8 atapiCurrentRwModeGet
(
int ctrl,
int drive
)
    {
    return(UINT8)(ataCtrl[ ctrl].drive[ drive].rwMode);
    } /* atapiCurrentRwModeGet */


/**************************************************************************
*
* atapiDriveTypeGet - get the drive type.
*
* This function routine will return the type of the drive if
* it is CD-ROM or Printer etc. The following table indicates the type depending
* on the return value.
* \ts
* 0x00h | Direct-access device  
* 0x01h | Sequential-access device
* 0x02h | Printer Device
* 0x03h | Processor device
* 0x04h | Write-once device
* 0x05h | CD-ROM device                
* 0x06h | Scanner device 
* 0x07h | Optical memory device
* 0x08h | Medium Change Device
* 0x09h | Communications device
* 0x0Ch | Array Controller Device
* 0x0Dh | Encloser Services Device
* 0x0Eh | Reduced Block Command Devices
* 0x0Fh | Optical Card Reader/Writer Device           
* 0x1Fh | Unknown or no device type
* \te 
*
* RETURNS: drive type.
*/

UINT8 atapiDriveTypeGet
(
int ctrl,
int drive
)
    {
    return(ataCtrl[ ctrl].drive[ drive].driveType);
    } /* atapiDriveTypeGet */


/**************************************************************************
*
* atapiVersionNumberGet - get the ATA/ATAPI version number of the drive.
*
* This function will return the ATA/ATAPI version number of
* the drive. Most significant 16 bits represent the Major Version Number and 
* the Lease significant 16 bits represents the minor Version Number.
* 
* Major Version Number
* \ts
* Bit 22 | ATA/ATAPI-6
* Bit 21 | ATA/ATAPI-5
* Bit 20 | ATA/ATAPI-4
* Bit 19 | ATA-3
* Bit 18 | ATA-2
* \te
* Minor version Number (bit 15 through bit 0)
* \ts
* 0001h | Obsolete
* 0002h | Obsolete
* 0003h | Obsolete
* 0004h | ATA-2 published, ANSI X3.279-1996
* 0005h | ATA-2 X3T10 948D prior to revision 2k
* 0006h | ATA-3 X3T10 2008D revision 1
* 0007h | ATA-2 X3T10 948D revision 2k
* 0008h | ATA-3 X3T10 2008D revision 0
* 0009h | ATA-2 X3T10 948D revision 3
* 000Ah | ATA-3 published, ANSI X3.298-199x
* 000Bh | ATA-3 X3T10 2008D revision 6
* 000Ch | ATA-3 X3T13 2008D revision 7 and 7a
* 000Dh | ATA/ATAPI-4 X3T13 1153D revision 6
* 000Eh | ATA/ATAPI-4 T13 1153D revision 13
* 000Fh | ATA/ATAPI-4 X3T13 1153D revision 7
* 0010h | ATA/ATAPI-4 T13 1153D revision 18
* 0011h | ATA/ATAPI-4 T13 1153D revision 15
* 0012h | ATA/ATAPI-4 published, ANSI NCITS 317-1998
* 0013h | Reserved
* 0014h | ATA/ATAPI-4 T13 1153D revision 14
* 0015h | ATA/ATAPI-5 T13 1321D revision 1
* 0016h | Reserved
* 0017h | ATA/ATAPI-4 T13 1153D revision 17
* 0018h-FFFFh | Reserved
* \te
*
* RETURNS: ATA/ATAPI version number.
*/

UINT32 atapiVersionNumberGet
(
int ctrl,
int drive
)
    {
    return(UINT32)(((ataCtrl[ ctrl].drive[ drive].param.majorVer)<<16)|
                   (ataCtrl[ ctrl].drive[ drive].param.minorVer));
    } /* atapiVersionNumberGet */


/**************************************************************************
*
* atapiRemovMediaStatusNotifyVerGet - get the Media Stat Notification Version.
*
* This function will return the removable media status notification
* version of the  drive.
*
* RETURNS: Version Number.
*/

UINT16 atapiRemovMediaStatusNotifyVerGet
(
int ctrl,
int drive
)
    {
    return(UINT16)(ataCtrl[ ctrl].drive[ drive].mediaStatusNotifyVer);
    } /* atapiRemovMediaStatusNotifyVerGet */


/**************************************************************************
*
* atapiCurrentCylinderCountGet - get logical number of cylinders in the drive.
*
* This function will return the number of logical cylinders
* in the drive. This value represents the no of cylinders that can be addressed.
*
* RETURNS: Cylinder count.
*/

UINT16 atapiCurrentCylinderCountGet
(
int ctrl,
int drive
)
    {
    return(UINT16)(ataCtrl[ ctrl].drive[ drive].param.currentCylinders);
    } /* atapiCurrentCylinderCountGet */


/**************************************************************************
*
* atapiCurrentHeadCountGet - get the number of read/write heads in the drive.
*
* This function will return the number of heads in the drive from device 
* structure.
*
* RETURNS: Number of heads.
*/

UINT8 atapiCurrentHeadCountGet
(
int ctrl,
int drive
)
    {
    return(UINT8)(ataCtrl[ ctrl].drive[ drive].param.currentHeads);
    } /* atapiCurrentHeadCountGet */


/**************************************************************************
*
* atapiBytesPerTrackGet - get the number of Bytes per track.
*
* This function will return the number of Bytes per track.
* This function will return correct values for drives of ATA/ATAPI-4 or less
* as this feild is retired for the drives compliant to ATA/ATAPI-5 or higher.
*
* RETURNS: Bytes per track.
*/

UINT16 atapiBytesPerTrackGet
(
int ctrl,
int drive
)
    {
    return(UINT16)(ataCtrl[ ctrl].drive[ drive].param.retired4);
    } /* atapiBytesPerTrackGet */


/**************************************************************************
*
* atapiBytesPerSectorGet - get the number of Bytes per sector.
*
* This function will return the number of Bytes per sector.
* This function will return correct values for drives of ATA/ATAPI-4 or less
* as this field is retired for the drives compliant to ATA/ATAPI-5 or higher.
*
* RETURNS: Bytes per sector.
*/

UINT16 atapiBytesPerSectorGet
(
int ctrl,
int drive
)
    {
    return(UINT16)(ataCtrl[ ctrl].drive[ drive].param.retired5);
    } /* atapiBytesPerSectorGet */


/**************************************************************************
*
* ataAdjustAndCopyByte - swaps every seccesive bytes and copies string
*
* RETURNS: N/A
*/

LOCAL void ataAdjustAndCopyByte
(
char * pbuf,
char * pSource
)
    {
    int i,j;

    for (i=1,j=0;i<=strlen(pSource);i+=2)
        {
#if (_BYTE_ORDER == _LITTLE_ENDIAN)
        pbuf[j]=pSource[i];
        j++;
        pbuf[j]=pSource[i-1];
        j++;
#else
        pbuf[j]=pSource[i-1];
        j++;
        pbuf[j]=pSource[i];
        j++;
#endif /* (_BYTE_ORDER == _LITTLE_ENDIAN) */
        }
    pbuf[j]='\0';            

    }

/************************************************************
*
* ataDumpPartTable - dump the partition table from sector 0
*                    of the specified hard disk
*
* This routine will display on stdout, the partition table
* of the specified hard drive.
*
* INPUTS:
*       ctrl - controller number of the ata driver 0 or 1
*       port - disk number of the drive 0 or 1
*
* RETURNS:
*        OK or ERROR
*
*********************************************************************/
STATUS ataDumpPartTable
    (
    int ctrl,
    int port
    )
    {
    ATA_CTRL  *pCtrl    = &ataCtrl[ctrl];
    ATA_DRIVE *pDrive   = &pCtrl->drive[port];
    struct bootRecord *bootRec;
    int i;

    if (pDrive->pAtaDev == NULL)
        return ERROR;

    bootRec = malloc (sizeof(struct bootRecord));
    if (bootRec == NULL)
        return (ERROR);
    if (ataBlkRW (pDrive->pAtaDev, (sector_t)0, 1, (char *)bootRec, O_RDONLY) == ERROR)
        return ERROR;
    printf("Partition    Start            End          Size       Bootable\n");
    for (i = 0; i < 4; i++)
        {
        struct partTable *pt;
        pt = &bootRec->partTable[i];
        printf("    %d      0x%.8x     0x%.8x     0x%.8x     %s\n", i,
                pt->partStartSector,
                pt->partStartSector + pt->partNumSectors -1,
                pt->partNumSectors,
                (pt->bootable==(char)0x80)?"YES":"NO");
        }
    return OK;
    }

/**********************************************************************
* ataDumptest - a quick test of the dump functionality for ATA driver
*
* <device_t> device id of the device to dump to.  This can be any XBD
*            device.  Could be the XBD of the disk device itself, or
*            could be the xbd of a partition overlayed on the drive.
*
* <sector>   sector offset to begin dump relative to start of xbd.
* <blocks>   number of blocks to dump to device
* <*data>    buffer that contains data to dump
*
* RETURNS:   N/A
*/

void ataDumptest
    (
    device_t d,
    sector_t sector,
    UINT32   blocks,
    char    *data
    )
    {
    XBD *xbd;

    xbd = (XBD *)devMapUnsafe(d);
    if ((xbd == NULL) || (data == NULL))
	{
	printf("bad parameters: xbd=0x%x, data=0x%x\n", xbd, data);
	printf("Usage: ataDumptest(device_t, (long long)sector, blks, *buf\n");
	return;
	}

    (*xbd->xbd_funcs->xf_dump)(xbd, sector, data, blocks*512);

    devUnmapUnsafe ((struct device *) xbd);
    }
