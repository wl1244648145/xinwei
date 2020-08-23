/* ataDrv.h - ATA/ATAPI library header file */

/* 
 * Copyright (c) 2005, 2007 Wind River Systems, Inc. 
 * 
 * The right to copy, distribute, modify, or otherwise make use 
 * of this software may be licensed only pursuant to the terms 
 * of an applicable Wind River license agreement. 
 */
 
/*
modification history
--------------------
01i,20jun07,dee  WIND00097125 and general cleanup
01h,27mar07,pmr  SMP-safe
01g,19may06,dee  SPR:117483, WIND00057492; add declaration for ataRawio()
01g,25jan06,dee  add support for core dump
01f,25sep05,dee  remove ATA_DEBUG, use ADDED_CFLAGS -DATA_DEBUG
01e,15sep05,dee  SPR#111796
01d,06sep05,dee  change name from sysInWordStringReverse to sysInWordStringRev
                 to match bsp function naming
01c,29aug05,dee  use only one bio queue per controller
01b,12aug05,dee  changes for XBD conversion
01a,14dec??,bri  created

*/

#ifndef __INCataDrvh
#define __INCataDrvh

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _ASMLANGUAGE

/* includes */

#include <vxWorks.h>
#include <cdromFsLib.h>
#include <blkIo.h>
#include <wdLib.h>
#include <sys/fcntlcom.h>
#include <private/semLibP.h>
#include <drv/pcmcia/pccardLib.h>
#ifdef _WRS_VX_SMP
#include <spinLockLib.h>
#endif /* _WRS_VX_SMP */

extern    VOIDFUNCPTR       _func_sysAtaInit;
extern    void sysInWordStringRev (ULONG ioAddr, UINT16 *bufPtr, int nWords);

/* defines */

/* Special BSP INIT After ATA Reset */

#ifndef SYS_ATA_INIT_RTN
    #define SYS_ATA_INIT_RTN(pCtrl)  if (_func_sysAtaInit != NULL)    \
                                    {                                 \
                                    ((*_func_sysAtaInit)(pCtrl));     \
                                    }
#endif

#if !defined(ATA_ZERO)
#   define ATA_ZERO (0)
#endif
#define ATA_DEBUG
#ifdef  ATA_DEBUG

    #define ATA_LOCAL

    /* debug verbosity level. Highest is 20.
     *
     * Error Messages     2
     * Drive params       9
     * function entry with params                12
     * notify that an action is started          13
     * control flow like function entry and exit 14
     * Intermediate state 15
     * Interrupt messages 19
     */
    
    #define ATA_DEBUG_MSG(lvl, fmt, a1, a2, a3, a4, a5, a6)            \
      if ((lvl) <= ataDebugLevel)                                    \
            {                                                          \
            logMsg (fmt, (int)(a1), (int)(a2), (int)(a3), (int)(a4),   \
                    (int)(a5), (int)(a6));                             \
            taskDelay(15);                                                 \
            }
                    
    #define ATA_INTR_MSG(fmt, a1, a2, a3, a4, a5, a6)                  \
       if (ataIntrDebug)                                              \
            logMsg (fmt, (int)(a1), (int)(a2), (int)(a3), (int)(a4),   \
                    (int)(a5), (int)(a6));
#else
    #define ATA_DEBUG_MSG(lvl, fmt, a1, a2, a3, a4, a5, a6)
    #define ATA_INTR_MSG(fmt, a1, a2, a3, a4, a5, a6)
    #define ATA_LOCAL LOCAL
#endif


/*
 * make it compatible for little/big endian machines.
 * i.e. swap for Big Endian machines
 */
//#define  _BYTE_ORDER  _BIG_ENDIAN
#if _BYTE_ORDER == _BIG_ENDIAN
    #define ATA_SWAP(x) LONGSWAP(x)
#else
    #define ATA_SWAP(x) (x)
#endif


#define MAX_28LBA   0x0fffffff
#define MAX_48LBA   (0xffffffff | (0xffff<<32))
#define MASK_48BIT  0x0000000000ff

_WRS_PACK_ALIGN(1) struct partTable
    {
    char    bootable;
    char    startHead;
    char    startSector;
    char    startCyl;
    char    partType;
    char    endHead;
    char    endSector;
    char    endCyl;
    uint32_t  partStartSector;
    uint32_t  partNumSectors;
    };

_WRS_PACK_ALIGN(1) struct bootRecord
    {
    char    bootCode[446];
    struct  partTable partTable[4];
    int16_t signature;
    };
 

/* defines */

/* user can modify the following section
 * modification will affect the driver footprint code and data sections
 */

/*
 * Power Management feature set -
 * SMART feature set            - Must not be enabled for devices implementing
 *                                packet command feature set
 */
#undef   ATA_SMART_FEATURE         /* Enable/Disable Smart feature set    */
#undef   ATA_CFA_FEATURE           /* Enable/Disable CFA feature set      */

/* QUEUEING IS NOT SUPPORT - DO NOT DEFINE */
#undef   ATAPI_QUEUED_FEATURE      /* Enable/Disable Queued feature set   */

/* OVERLAP IS NOT YET SUPPORTED - DO NOT DEFINE */
#undef   ATAPI_OVERLAPPED_FEATURE  /* Overlapped feature set              */

#undef  ATAPI_PWR_MNGMT           /* enable/disable Pwr Manag. feature   */
#undef  ATAPI_ADV_PWR_MNGMT       /* enable/disable Pwr Manag. feature   */
#undef  ATAPI_SECURITY_FEATURE    /* enable/disable security feature     */
#undef  ATAPI_PWRUP_IN_STDBY      /* enable/disable Pwr-up in standby    */
/*
 * Removable Media feature set - Not implementing Packet Command feature set.
 *                             - Media status notification disabled.
 *                             - ATA_CMD_MEDIA_LOCK,
 *                               ATA_CMD_MEDIA_UNLOCK,
 *                               ATA_CMD_MEDIA_EJECT commands will not work if
 *                               this feature is disabled.
 */
#undef  REMOV_MEDIA_FEATURE       /* enable/disable Remov. media feature */
#undef  REMOV_MEDIA_NOTIFY        /* enable/disable Remov. status Notif. */

/* Host protected area feature set
 *                              - Use prohibited if Removable media feature set
 *                              - More than one Nonvolatile setmax address
 *                                is prohibited.
 */
#undef  HOST_PROT_AREA_FEATURE    /* Host protected area feature set     */
/* end of user configurable */

/* IoCtl definitions */

/* security feature */

#define IOCTL_DIS_MASTER_PWD               0x00
#define IOCTL_DIS_USER_PWD                 0x01
#define IOCTL_ERASE_PREPARE                0x02
#define IOCTL_NORMAL_ERASE_UNIT_USR        0x03
#define IOCTL_NORMAL_ERASE_UNIT_MSTR       0x04
#define IOCTL_ENH_ERASE_UNIT_USR           0x05
#define IOCTL_ENH_ERASE_UNIT_MSTR          0x06
#define IOCTL_FREEZE_LOCK                  0x07
#define IOCTL_SET_PASS_MSTR                0x08
#define IOCTL_SET_PASS_USR_HIGH            0x09
#define IOCTL_SET_PASS_USR_MAX             0x0A
#define IOCTL_UNLOCK_USR                   0x0B
#define IOCTL_UNLOCK_MSTR                  0x0C
/* removable media feauture */
#define IOCTL_EJECT_DISK                   0X0F
#define IOCTL_LOAD_DISK                    0X10
#define IOCTL_GET_MEDIA_STATUS             0x11
#define IOCTL_MEDIA_LOCK                   0X12
#define IOCTL_MEDIA_UNLOCK                 0X13

/* removable media notification feature */

#define IOCTL_ENA_REMOVE_NOTIFY            0x14
#define IOCTL_DISABLE_REMOVE_NOTIFY        0x15

/* Advanced Power Management feature */

#define IOCTL_ENB_SET_ADV_POW_MNGMNT       0X20
#define IOCTL_DISABLE_ADV_POW_MNGMNT       0X21

/* Power-up in standby feature */

#define IOCTL_ENB_POW_UP_STDBY             0X30

/* Host protected area feature */

#define IOCTL_READ_NATIVE_MAX_ADDRESS      0XC0
#define IOCTL_SET_MAX_ADDRESS              0XC1
#define IOCTL_SET_MAX_SET_PASS             0XC2
#define IOCTL_SET_MAX_LOCK                 0XC3
#define IOCTL_SET_MAX_UNLOCK               0XC4
#define IOCTL_SET_MAX_FREEZE_LOCK          0XC5

/* SMART feature */

#define IOCTL_SMART_READ_DATA              0XD0
#define IOCTL_SMART_ENABLE_ATTRIB_AUTO     0XD1
#define IOCTL_SMART_DISABLE_ATTRIB_AUTO    0XD2
#define IOCTL_SMART_SAVE_ATTRIB            0XD3
#define IOCTL_SMART_OFFLINE_IMMED          0XD4
#define EXEC_OFF_IMMED_OFF_MODE            0
#define EXEC_SHORT_SELF_IMMED_OFF_MODE     1
#define EXEC_EXT_SELF_IMMED_OFF_MODE       2
#define ABORT_OFF_MODE_SELF_IMMED          127
#define EXEC_SHORT_SELF_IMMED_CAP_MODE     129
#define EXEC_EXT_SELF_IMMED_CAP_MODE       130
#define IOCTL_SMART_READ_LOG_SECTOR        0XD5
#define IOCTL_SMART_WRITE_LOG_SECTOR       0XD6
#define LOG_DIR                            0
#define ERROR_LOG                          1
#define SELFTEST_LOG                       6
#define IOCTL_SMART_ENABLE_OPER            0XD8
#define IOCTL_SMART_DISABLE_OPER           0XD9
#define IOCTL_SMART_RETURN_STATUS          0XDA

/* Power management feature */

#define IOCTL_CHECK_POWER_MODE             0XE5   /* ATA_CMD_CHECK_POWER_MODE */
#define IOCTL_IDLE_IMMEDIATE               0XE1   /* ATA_CMD_IDLE_IMMEDIATE   */
#define IOCTL_SLEEP                        0XE6   /* ATA_CMD_SLEEP            */
#define IOCTL_STANDBY_IMMEDIATE            0XE0   /* ATA_CMD_STANDBY_IMMEDIATE*/

/* CFA feature */

#define IOCTL_CFA_ERASE_SECTORS                    0XF0
#define IOCTL_CFA_WRITE_MULTIPLE_WITHOUT_ERASE     0XFD
#define IOCTL_CFA_WRITE_SECTORS_WITHOUT_ERASE      0XF8
#define IOCTL_CFA_TRANSLATE_SECTOR                 0XF7
#define IOCTL_CFA_REQUEST_EXTENDED_ERROR_CODE      0XF3
#define IOCTL_ATAPI_READ_TOC_PMA_ATIP              0XF8

/* IoCtl definitions END*/

#define IDE_LOCAL               0       /* ctrl type: LOCAL(IDE)   */
#define ATA_PCMCIA              1       /* ctrl type: PCMCIA       */
#define PARIDE                  2       /* ctrl type: Paralle Port */
#define USB                     3       /* media : USB             */

#define ATA_MAX_DRIVES          2       /* Always 2 drives per controller */

#define ATA_MAX_CTRLS           2

#define ATA_LIKE_WAIT_FOREVER   (sysClkRateGet() * 5) /* 1 minute */

/* config */

/* Page 63, 7.1.7.1, Ref-2 */

#define CONFIG_PROT_TYPE         0xc000 /* Protocol Type */
#define CONFIG_PROT_TYPE_ATAPI   0x8000 /* ATAPI         */

#define CONFIG_REMOVABLE         0x0080 /* Removable */

#define CONFIG_PKT_TYPE          0x0060 /* CMD DRQ Type        */
#define CONFIG_PKT_TYPE_MICRO    0x0000 /* Microprocessor DRQ  */
#define CONFIG_PKT_TYPE_INTER    0x0020 /* Interrupt DRQ       */
#define CONFIG_PKT_TYPE_ACCEL    0x0040 /* Accelerated DRQ     */

#define CONFIG_PKT_SIZE          0x0003 /* Command Packet Size */
#define CONFIG_PKT_SIZE_12       0x0000 /* 12 bytes            */

/* config END*/


/* word 2 of device parametes specific configuration values */

#define ATA_SPEC_CONFIG_VALUE_0 0x37c8  /* SET FEATURE subcomm req */
                                        /* IDENTIFY DEVICE resp incomplete*/
#define ATA_SPEC_CONFIG_VALUE_1 0x738c  /* SET FEATURE subcomm req */
                                        /* IDENTIFY DEVICE resp complete*/
#define ATA_SPEC_CONFIG_VALUE_2 0x8c73  /* SET FEATURE subcomm NOT req */
                                        /* IDENTIFY DEVICE resp incomplete*/
#define ATA_SPEC_CONFIG_VALUE_3 0xc837  /* SET FEATURE subcomm NOT req */
                                        /* IDENTIFY DEVICE resp complete*/

/*
 * Drive Types.
 * This is 12 to 8 bits of 1st word of "ATAPI Identify device" command
 * Table , Page 113, Ref-1.
 */
 
#define CONFIG_DEV_TYPE_MASK                0x1f00
#define CONFIG_DEV_TYPE_DIRECT              0x00
#define CONFIG_DEV_TYPE_SEQUENTIAL          0x01
#define CONFIG_DEV_TYPE_PRINTER             0x02
#define CONFIG_DEV_TYPE_PROCESSOR           0x03
#define CONFIG_DEV_TYPE_WRITE_ONCE          0x04
#define CONFIG_DEV_TYPE_CD_ROM              0x05
#define CONFIG_DEV_TYPE_SCANNER             0x06
#define CONFIG_DEV_TYPE_OPTICAL             0x07
#define CONFIG_DEV_TYPE_MEDIUM_CHANGER      0x08
#define CONFIG_DEV_TYPE_COMMUNICATION       0x09
#define CONFIG_DEV_TYPE_ARRAY_CONTROLLER    0x0C
#define CONFIG_DEV_TYPE_ENCLOSER_SERVICE    0x0D
#define CONFIG_DEV_TYPE_RED_BLK_CMD_DEV     0x0E
#define CONFIG_DEV_TYPE_OPT_CARD_RW         0x00
#define CONFIG_DEV_TYPE_UNKNOWN             0x1F

/* Host controller functions macro's */


/* Read a BYTE from IO port, `ioAdrs' */

#ifndef ATA_IO_BYTE_READ
    #define ATA_IO_BYTE_READ(ioAdrs)         sysInByte (ioAdrs)
#endif  /* ATA_IO_BYTE_READ */


/* Write a BYTE `byte' to IO port, `ioAdrs' */

#ifndef ATA_IO_BYTE_WRITE
    #define ATA_IO_BYTE_WRITE(ioAdrs, byte)  sysOutByte (ioAdrs, byte)
#endif  /* ATA_IO_BYTE_WRITE */

/* Read 16-bit little-endian `nWords' into `pData' from IO port, `ioAdrs' */

#ifndef ATA_IO_NWORD_READ
    #define ATA_IO_NWORD_READ(ioAdrs, pData, nWords)                   \
                               sysInWordStringRev (ioAdrs, pData, nWords)
#endif  /* ATA_IO_NWORD_READ */


/* Write 16-bit little-endian `nWords' from `pData' into IO port, `ioAdrs' */

#ifndef ATA_IO_NWORD_WRITE
    #define ATA_IO_NWORD_WRITE(ioAdrs, pData, nWords)    \
        sysOutWordStringRev (ioAdrs, pData, nWords)
#endif /* ATA_IO_NWORD_WRITE */



/* Read 32-bit little-endian `nLongs' into `pData' from IO port, `ioAdrs' */

#ifndef ATA_IO_NLONG_READ
    #define ATA_IO_NLONG_READ(ioAdrs, pData, nLongs)     \
        sysInLongString (ioAdrs, pData, nLongs)
#endif  /* ATA_IO_NLONG_READ */


/* Write 32-bit little-endian `nLongs' from `pData' into IO port, `ioAdrs' */

#ifndef ATA_IO_NLONG_WRITE
    #define ATA_IO_NLONG_WRITE(ioAdrs, pData, nLongs)    \
        sysOutLongString (ioAdrs, pData, nLongs)
#endif  /* ATA_IO_NLONG_WRITE */


/* Read 32-bit CPU-endian `nWords' into `pData' from IO port, `ioAdrs' */
#define _BYTE_ORDER   _LITTLE_ENDIAN
/*#define _BYTE_ORDER   _BIG_ENDIAN*/
#ifndef ATA_IO_NWORD_READ_SWAP
    #if (_BYTE_ORDER == _BIG_ENDIAN)
        #define ATA_IO_NWORD_READ_SWAP(ioAdrs, pData, nWords)            \
       ATA_IO_NWORD_READ (ioAdrs, pData, nWords)
       
    #else   /* (_BYTE_ORDER == _BIG_ENDIAN) */
        #define ATA_IO_NWORD_READ_SWAP(ioAdrs, pData, nWords)            \
           sysInWordString (ioAdrs, pData, nWords)
      
    #endif /* (_BYTE_ORDER == _BIG_ENDIAN) */
#endif  /* ATA_IO_NLONG_READ_SWAP */

/* 
 * This macro provides a small delay, which is expected to be more than 400nSec
 * that is used in several places in the ATA command protocols:
 * 1) It is recommended that the host delay 400ns after
 *    writing the command register.
 * 2) ATA-4 has added a new requirement that the host delay
 *    400ns if the DEV bit in the Device/Head register is
 *    changed.  This was not recommended or required in ATA-1,
 *    ATA-2 or ATA-3.  
 * 3) ATA-4 has added another new requirement that the host delay
 *    after the last word of a data transfer before checking the
 *    status register.  This was not recommended or required in
 *    ATA-1, ATA-2 or ATA-3.  
 *  
 */

#define ATA_DELAY_400NSEC   ATA_IO_BYTE_READ(ATAPI_ASTATUS);    \
                            ATA_IO_BYTE_READ(ATAPI_ASTATUS);    \
                            ATA_IO_BYTE_READ(ATAPI_ASTATUS);    \
                            ATA_IO_BYTE_READ(ATAPI_ASTATUS);    \
                            ATA_IO_BYTE_READ(ATAPI_ASTATUS) 

#define ATA_SIGNATURE       0x01010000
#define ATAPI_SIGNATURE     0x010114EB

#define ATAPI_MAX_CMD_LENGTH 12 /* maximum length in bytes of an ATAPI command */

#ifndef ATA_SEM_TIMEOUT_DEF
    #define ATA_SEM_TIMEOUT_DEF 5       /* default timeout for ATA sync sem  */
#endif 

#ifndef ATA_WDG_TIMEOUT_DEF
    #define ATA_WDG_TIMEOUT_DEF 5       /* default timeout for ATA watch dog */
#endif  /* These two are also defined in pc.h */

/* Device types */

#define ATA_TYPE_NONE       0x00    /* device is faulty or not present */
#define ATA_TYPE_ATA        0x01    /* ATA device */
#define ATA_TYPE_ATAPI      0x02    /* ATAPI device */
#define ATA_TYPE_INIT       0xFF    /* device must be identified */

/* Device  states */

#define ATA_DEV_OK      0   /* device is OK                      */
#define ATA_DEV_NONE    1   /* device absent or does not respond */
#define ATA_DEV_DIAG_F  2   /* device diagnostic failed          */
#define ATA_DEV_PREAD_F 3   /* read device parameters failed     */
#define ATA_DEV_MED_CH  4   /* medium have been changed          */
#define ATA_DEV_INIT    255 /* uninitialized device              */

/* Errors */

/* Register mode and other definitions */

/* size/drive/head register +6 : addressing mode CHS or LBA */
/* These are only in ATA1, ATA2. not defined in ATAPI 5*/

#define ATA_SDH_IBM             0xa0    /* chs, 512 bytes sector, ecc */
#define ATA_SDH_LBA             0xe0    /* lba, 512 bytes sector, ecc */

/** Register Bit Definitions **/

/* Device Control register +6 WR Control Block */

#define ATA_CTL_4BIT            0x8     /* use 4 head bits (wd1003) */
#define ATA_CTL_SRST            0x4     /* reset controller  */
#define ATA_CTL_NIEN            0x2     /* disable interrupts */

/* Feature Register */

#define FEAT_OVERLAP            0x02    /* command may be overlapped */
#define FEAT_DMA                0x01    /* data will be transferred via DMA */

/* Error register +1 RD */

#define ERR_ABRT                 0x04    /* command aborted ATA_ERR_ABRT */

/* other bits of error register are command dependent */

/* Error Register */

#define ERR_SENSE_KEY            0xf0 /* Sense Key mask            */

#define SENSE_NO_SENSE           0x00 /* no sense sense key        */
#define SENSE_RECOVERED_ERROR    0x10 /* recovered error sense key */
#define SENSE_NOT_READY          0x20 /* not ready sense key       */
#define SENSE_MEDIUM_ERROR       0x30 /* medium error sense key    */
#define SENSE_HARDWARE_ERROR     0x40 /* hardware error sense key  */
#define SENSE_ILLEGAL_REQUEST    0x50 /* illegal request sense key */
#define SENSE_UNIT_ATTENTION     0x60 /* unit attention sense key  */
#define SENSE_DATA_PROTECT       0x70 /* data protect sense key    */
#define SENSE_BLANK_CHECK        0x80 /* blank check */
#define SENSE_VENDOR_SPECIFIC    0x90 /* vendor specific skey */
#define SENSE_COPY_ABORTED       0xa0 /* copy aborted */
#define SENSE_ABORTED_COMMAND    0xb0 /* aborted command sense key */
#define SENSE_EQUAL              0xc0 /* equal */
#define SENSE_VOLUME_OVERFLOW    0xd0 /* volume overflow */
#define SENSE_MISCOMPARE         0xe0 /* miscompare sense key      */
#define SENSE_RESERVED           0xf0


#define ERR_MCR                  0x08 /* Media Change Requested    */
#define ERR_ABRT                 0x04 /* Aborted command           */
#define ERR_EOM                  0x02 /* End Of Media              */
#define ERR_ILI                  0x01 /* Illegal Length Indication */

/* Interrupt Reason Register */

#define INTR_RELEASE             0x04 /*Bus released before command completion*/
#define INTR_IO                  0x02 /*1 - In to the Host; 0 - Out to device */
#define INTR_COD                 0x01 /* 1 - Command; 0 - user Data           */

#define USE_LBA                  0x40    /* used to 'OR' into Dev/Head register */

/* The drive number bit */

#define ATA_DRIVE_BIT            4       /* usage :-      1<<ATA_DRIVE_BIT */

/* status register +7 RD */

#define ATA_STAT_BUSY            0x80    /* controller busy */
#define ATA_STAT_READY           0x40    /*selected drive ready-ATA_STAT_DRDY */
#define ATA_STAT_DMAR            0x20    /*DMA Ready */
#define ATA_STAT_SERV            0x10    /*Service */
#define ATA_STAT_DRQ             0x08    /* Data Request    */
#define ATA_STAT_ERR             0x01    /* Error Detect    */
#define ATA_STAT_CHK             0x01    /* check    */

/* following are not in ATAPI5 */

#define ATA_STAT_WRTFLT          0x20    /* write fault. ATA_STAT_BIT5
                                          * sff8020i says that it is for
                                          * DMA Ready also.
                                          */
#define ATA_STAT_SEEKCMPLT       0x10    /* seek complete. ATA_STAT_BIT4
                                          * sff8020i- this is for service and
                                          * DSC.
                                          */
#define ATA_STAT_ECCCOR          0x04    /* sff8020-i correctable error occured
                                          * ATA_STAT_CORR
                                          */

/* ATAPI registers */                                /* (ATA/ATAPI) */
#define ATA_DATA(base0)      (base0 + 0)  /* (RW) data register (16 bits) */
#define ATA_ERROR(base0)     (base0 + 1*2 +1)  /* (R)  error register          */
#define ATA_FEATURE(base0)   (base0 + 1*2+1)  /* (W)  feature/precompensation */
#define ATA_SECCNT(base0)    (base0 + 2*2+1)  /* (RW) sector count for ATA.
                                           * R-Interrupt reason W-unused  */
#define ATA_SECTOR(base0)    (base0 + 3*2+1)  /* (RW) first sector number.
                                           * ATAPI- Reserved for SAMTAG   */
#define ATA_CYL_LO(base0)    (base0 + 4*2+1)  /* (RW) cylinder low byte
                                           * ATAPI - Byte count Low       */
#define ATA_CYL_HI(base0)    (base0 + 5*2+1)  /* (RW) cylinder high byte
                                           * ATAPI - Byte count High      */
#define ATA_SDH(base0)       (base0 + 6*2+1)  /* (RW) sector size/drive/head
                                           * ATAPI - drive select         */
#define ATA_COMMAND(base0)   (base0 + 7*2+1)  /* (W)  command register        */
#define ATA_STATUS(base0)    (base0 + 7*2+1) /* (R)  immediate status        */
#define ATA_A_STATUS(base1)  (base1 + 6*2+1)  /* (R)  alternate status        */
#define ATA_D_CONTROL(base1) (base1 + 6*2+1)  /* (W)  disk controller control */
#define ATA_D_ADDRESS(base1) (base1 + 7*2+1)  /* (R)  disk controller address */
#if 0
#define	ATA_DATA(base0)		(base0 + 0) /* (RW) data register (16 bits) */
#define ATA_ERROR(base0)	(base0 +  1*2 )+1;//+1) /* (R)  error register	    */
#define	ATA_FEATURE(base0)	(base0 + 1*2);// +1) /* (W)  feature/precompensation */
#define	ATA_SECCNT(base0)	(base0 + 2*2 )+1;//+1) /* (RW) sector count	    */
#define	ATA_SECTOR(base0)	(base0 + 3*2)+1;// +1) /* (RW) first sector number	    */
#define	ATA_CYL_LO(base0)	(base0 + 4*2);// +1) /* (RW) cylinder low byte	    */
#define	ATA_CYL_HI(base0)	(base0 + 5*2 );//+1) /* (RW) cylinder high byte	    */
#define	ATA_SDH(base0)		(base0 + 6*2 );//+1) /* (RW) sector size/drive/head  */
#define	ATA_COMMAND(base0)	(base0 + 7*2)+1;//+1) /* (W)  command register	    */
#define	ATA_STATUS(base0) 	(base0 + 7 *2)+1;//+1) /* (R)  immediate status	    */
#define	ATA_A_STATUS(base1)	(base1 + 6*2 )+1;//+1) /* (R)  alternate status	    */
#define	ATA_D_CONTROL(base1)	(base1 + 6*2)+1;// +1) /* (W)  disk controller control */
#define	ATA_D_ADDRESS(base1)	(base1 + 7*2)+1;// +1) /* (R)  disk controller address *//*?????????????by xcl*/
#endif



#define ATAPI_DATA             (long)pCtrl->ataReg.data    /* (RW/RW)data reg. (16 bits)   */
#define ATAPI_ERROR            (long)pCtrl->ataReg.error   /* (R /R )error reg.            */
#define ATAPI_FEATURE          (long)pCtrl->ataReg.feature /* (W /W )feature reg.          */
#define ATAPI_SECCNT_INTREASON (long)pCtrl->ataReg.seccnt  /* (RW/R )seccount/intr reason  */
#define ATAPI_SECTOR           (long)pCtrl->ataReg.sector  /* (RW/  )Sector Number reg     */
#define ATAPI_CYLLOW_BCOUNT_LO (long)pCtrl->ataReg.cylLo   /* (RW/RW)cylLow/byte count low */
#define ATAPI_CYLHI_BCOUNT_HI  (long)pCtrl->ataReg.cylHi   /* (RW/RW)cylHi/byte count high */
#define ATAPI_CYLLOW           (long)pCtrl->ataReg.cylLo   /* (RW/RW)cylLow/byte count low */
#define ATAPI_CYLHI            (long)pCtrl->ataReg.cylHi   /* (RW/RW)cylHi/byte count high */
#define ATAPI_SDH_D_SELECT     (long)pCtrl->ataReg.sdh     /* (RW/RW)sdh/drive select reg. */
#define ATAPI_STATUS           (long)pCtrl->ataReg.status  /* (R /R )status reg.           */
#define ATAPI_COMMAND          (long)pCtrl->ataReg.command /* (W /W )command reg.          */
#define ATAPI_D_CONTROL        (long)pCtrl->ataReg.dControl/* (W /W )device control        */
#define ATAPI_ASTATUS          (long)pCtrl->ataReg.aStatus /* (R /R )alternate status      */

#define ATA_WORD54_58_VALID 0x01
#define ATA_WORD64_70_VALID 0x02
#define ATA_WORD88_VALID    0x04

                /* Commands */

#define ATA_CMD_DIAGNOSE    0x90    /* execute controller diagnostic */
#define ATAPI_CMD_SRST      0x08    /* Device reset, N-NONATAPI,M-ATAPI 
                                           ATA_CMD_DEVICE_RESET */
#define ATA_CMD_RECALIB                                0x10

        /*   Recalibrate   obsolete   */
        
#define ATA_CMD_FORMAT                                 0x50

        /*   Format track  obsolete   */

/* Commands mandatory for nonATAPI devices. */ /* 6.4.1, Page 21, Ref-1 */

#define ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC     0X90

                 /*   Device diagnostic, Y   */
                 /*   PIO data-in, Y  ATA_CMD_IDENTIFY_DEVICE */
#define ATA_CMD_IDENT_DEV           0xEC     /* identify */

#define ATA_CMD_INITP                                  0X91

                 /*
                  * Non-data, M-(NONATAPI if CHS supported),?,
                  * N-ATAPI  ATA_CMD_INITIALIZE_DEVICE_PARAMETERS 
                  */
                  
#define ATA_CMD_READ_DMA                               0XC8

                 /*   DMA, M-NONATAPI, N-ATAPI   */
                 
#define ATA_CMD_READ_DMA_EXT                           0X25

                 /*   DMA, M-NONATAPI, N-ATAPI   */

#define ATA_CMD_READ_MULTI                             0XC4

                 /*   PIO data-in, Y-NONATAPI, N-ATAPI ATA_CMD_READ_MULTIPLE  */
                 
#define ATA_CMD_READ_MULTI_EXT                         0X29

                 /*   PIO data-in, Y-NONATAPI, N-ATAPI ATA_CMD_READ_MULTIPLE  */

#define ATA_CMD_READ_VERIFY_SECTOR_S                   0X40

                 /*   Non-data, 8.28,Page 149, Ref-1 */
                 
#define ATA_CMD_READ                                   0X20
                 
                 /*   PIO data-in, Y  ATA_CMD_READ_SECTORS */
                 
#define ATA_CMD_READ_EXT                               0X24

                 /*   PIO data-in, Y  ATA_CMD_READ_SECTORS */

#define ATA_CMD_SEEK                                   0X70
                 
                 /*   Non-data, Y-NONATAPI, N-ATAPI   */
                 
#define ATA_CMD_SET_FEATURE                            0XEF

                 /*   Non-data, Y   ATA_CMD_SET_FEATURES*/
                 
#define ATA_CMD_SET_MULTI                              0XC6
                 
   /*   PIO data out, Y-NONATAPI, N-ATAPI ATA_CMD_SET_MULTIPLE_MODE */
   
#define ATA_CMD_WRITE_DMA                              0XCA

                 /*   DMA, Y-NONATAPI, N-ATAPI   */
                 
#define ATA_CMD_WRITE_DMA_EXT                          0X35

                 /*   DMA, Y-NONATAPI, N-ATAPI   */

#define ATA_CMD_WRITE_MULTI                            0XC5

     /*   PIO data out, Y-NONATAPI, N-ATAPI ATA_CMD_WRITE_MULTIPLE*/
     
#define ATA_CMD_WRITE_MULTI_EXT                        0X39

     /*   PIO data out, Y-NONATAPI, N-ATAPI ATA_CMD_WRITE_MULTIPLE*/

#define ATA_CMD_WRITE                                  0X30

     /*   PIO data out, Y-NONATAPI, N-ATAPI ATA_CMD_WRITE_SECTORS */
     
#define ATA_CMD_WRITE_EXT                              0X34

     /*   PIO data out, Y-NONATAPI, N-ATAPI ATA_CMD_WRITE_SECTORS */

/* Commands for only ATA. but not for ATAPI */

#define ATA_CMD_DOWNLOAD_MICROCODE                     0X92

                 /* PIO data-out, O-NONATAPI,N-ATAPI */
/* ATA_CMD_INITP*/

#define ATA_CMD_READ_BUFFER                            0XE4

                 /* PIO data-in, O-NONATAPI, N-ATAPI   */
                 
/*ATA_CMD_READ_DMA*/

/*ATA_CMD_READ_MULTIPLE*/

/*ATA_CMD_READ_VERIFY_SECTORS*/

/*ATA_CMD_SEEK*/

/*ATA_CMD_SET_MULTI*/

#define ATA_CMD_WRITE_BUFFER                           0XE8

                 /* PIO data out, 0-NONATAPI,N-ATAPI   */
                 
/*ATA_CMD_WRITE_DMA*/

/*ATA_CMD_WRITE_MULTIPLE*/

/*ATA_CMD_WRITE_SECTORS*/

/*
 * ATA commands mandatory for ATAPI (CD-ROM) device
 * 7.0, Table 18, Page 59, Ref 2
 */
 
#define ATA_CMD_CHECK_POWER_MODE                       0XE5
                                /*   Non-data, ?    ?-(M-ATAPI Driver tells)  */
                                
#define ATA_CMD_IDLE_IMMEDIATE                         0XE1
                                /*   Non-data, ?   ?-(M-ATAPI Driver tells)   */
                                
#define ATA_CMD_SLEEP                                  0XE6
                                /*   Non-data, ?   ?-(M-ATAPI Driver tells)   */
                                
#define ATA_CMD_STANDBY_IMMEDIATE                      0XE0
                                /*   Non-data, Y-POWER_MANAGEMENT,
                                     ?-(M-ATAPI Driver tells)   */
                                     
#define ATA_CMD_READ_VERIFY_SECTORS                    0X40
                                /*   Non-data,Y-NONATAPI, N-ATAPI   */
                                

/* Packet Device commands */

#define ATA_PI_CMD_SRST                                0X08
                   /*   Device reset, N-NONATAPI,M-ATAPI ATA_CMD_DEVICE_RESET */
                   
#define ATA_PI_CMD_PKTCMD                              0XA0
                   /*   Packet, N-NONATAPI, M-ATAPI  ATA_CMD_PACKET */
                   
#define ATA_PI_CMD_IDENTD                              0XA1
                   /*   PIO data-in, N-NONATAPI,M-ATAPI
                        ATA_CMD_IDENTIFY_PACKET_DEVICE */
                        
#define ATA_PI_CMD_SERVICE                             0XA2
                   /*   PACKET or READ/WRITE DMA QUEUED, Y-(PACKET & OVERLAPPED)
                        ATA_CMD_SERVICE */
                        
#define ATA_CMD_NOP                                    0X00
                   /*   Non-data, O-NONATAPI,M-ATAPI,M-OVERLAP   */
                   
/* CompactFlash Association CFA memory Commands */

#define ATA_CMD_CFA_ERASE_SECTORS                  0XC0
                                            /* Non-data,     Y-(if CFA)   */
                                            
#define ATA_CMD_CFA_REQUEST_EXTENDED_ERROR_CODE    0X03
                                            /* Non-data,     Y-(if CFA)   */
                                            
#define ATA_CMD_CFA_TRANSLATE_SECTOR               0X87
                                            /* PIO data-in,  Y-(if CFA)   */
                                            
#define ATA_CMD_CFA_WRITE_MULTIPLE_WITHOUT_ERASE   0XCD
                                            /* PIO data-out, Y-(if CFA)   */
                                            
#define ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE    0X38
                                            /* PIO data-out, Y-(if CFA)   */

/* others */

#define ATA_CMD_FLUSH_CACHE               0XE7   /* Non-data, Y   */
#define ATA_CMD_GET_MEDIA_STATUS          0XDA   /* Non-data, ?   */
#define ATA_CMD_IDLE                      0XE3   /* Non-data, ?   */
#define ATA_CMD_MEDIA_EJECT               0XED   /* Non-data, ?   */
#define ATA_CMD_MEDIA_LOCK                0XDE   /* Non-data, ?   */
#define ATA_CMD_MEDIA_UNLOCK              0XDF   /* Non-data, ?   */
#define ATA_CMD_READ_DMA_QUEUED           0XC7   /* DMA QUEUED,
                                                    M-(OVERLAP-Y & ATAPI-N),
                                                    N-ATAPI   */
#define ATA_CMD_READ_NATIVE_MAX_ADDRESS   0XF8   /* Non-data,
                                                  * M-HOST_PROTECTED_AREA,
                                                  * N-Removable feature   */
#define ATA_CMD_SECURITY_DISABLE_PASSWORD 0XF6   /* PIO data-out, M-SECURITY */
#define ATA_CMD_SECURITY_ERASE_PREPARE    0XF3   /* Non-data, M-SECURITY     */
#define ATA_CMD_SECURITY_ERASE_UNIT       0XF4   /* PIO data-out, M-SECURITY */
#define ATA_CMD_SECURITY_FREEZE_LOCK      0XF5   /* Non-data, M-SECURITY     */
#define ATA_CMD_SECURITY_SET_PASSWORD     0XF1   /* PIO data-out, M-SECURITY */
#define ATA_CMD_SECURITY_UNLOCK           0XF2   /* PIO data-out, M-SECURITY */

#define ATA_CMD_SET_MAX                   0XF9   /* Non-data, ?               */
#define ATA_CMD_SMART                     0XB0   /* Non-data, Y-SMART,N-ATAPI */
#define ATA_CMD_STANDBY                   0XE2   /* Non-data, ?   */
#define ATA_CMD_WRITE_DMA_QUEUED          0XCC   /* DMA QUEUED,
                                                    Y-(OVERLAP-Y & ATAPI-N)
                                                    N-ATAPI   */

/* ATAPI  MMC Commands */
/* ANSI INCITS 360-2002 Annex I, page 397 */

#define ATAPI_CMD_INQUIRY                              0X12
#define ATAPI_CMD_LOAD_UNLOAD_MEDIUM                   0XA6
#define ATAPI_CMD_MECHANISM_STATUS                     0XBD
#define ATAPI_CMD_MODE_SELECT                          0X55
#define ATAPI_CMD_MODE_SENSE                           0X5A
#define ATAPI_CMD_PAUSE_RESUME                         0X4B
#define ATAPI_CMD_PLAY_AUDIO10                         0X45
#define ATAPI_CMD_PLAY_AUDIO12                         0XA5
#define ATAPI_CMD_PLAY_AUDIO_MSF                       0X47
#define ATAPI_CMD_PLAY_CD_LBA                          0XBC
#define ATAPI_CMD_PLAY_CD_MSF                          0XB4
#define ATAPI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL         0X1E
#define ATAPI_CMD_READ10                               0X28
#define ATAPI_CMD_READ12                               0XA8
#define ATAPI_CMD_READ_CD_CAPACITY                     0X25  /* 0x23 */
#define ATAPI_CMD_READ_CD                              0XBE
#define ATAPI_CMD_READ_CD_MSF                          0XB9
#define ATAPI_CMD_READ_HEADER                          0X44
#define ATAPI_CMD_READ_SUB_CHANNEL                     0X42
#define ATAPI_CMD_READ_TOC_PMA_ATIP                    0X43  /* PMA/ATIP are
                                                                from sff8090i */
#define ATAPI_CMD_REQUEST_SENSE                        0X03
#define ATAPI_CMD_SCAN                                 0XBA
#define ATAPI_CMD_SEEK                                 0X2B
#define ATAPI_CMD_SET_CD_SPEED                         0XBB
#define ATAPI_CMD_STOP_PLAY_SCAN                       0X4E
#define ATAPI_CMD_START_STOP_UNIT                      0X1B
#define ATAPI_CMD_TEST_UNIT_READY                      0X00


#define ATAPI_CMD_BLANK                                0XA1
#define ATAPI_CMD_CLOSE_TRACK_RZONE_SESSION_BORDER     0X5B
#define ATAPI_CMD_COMPARE                              0X39
#define ATAPI_CMD_ERASE_10                             0X2C
#define ATAPI_CMD_FORMAT_UNIT                          0X04
#define ATAPI_CMD_GET_CONFIGURATION                    0X46
#define ATAPI_CMD_GET_EVENT_STATUS_NOTIFICATION        0X4A
#define ATAPI_CMD_GET_PERFORMANCE                      0XAC
#define ATAPI_CMD_LOCK_UNLOCK_CACHE                    0X36
#define ATAPI_CMD_LOG_SELECT                           0X4C
#define ATAPI_CMD_LOG_SENSE                            0X4D
#define ATAPI_CMD_PRE_FETCH                            0X34
#define ATAPI_CMD_READ_BUFFER                          0X3C
#define ATAPI_CMD_READ_BUFFER_CAPACITY                 0X5C
#define ATAPI_CMD_READ_DISC_INFORMATION                0X51
#define ATAPI_CMD_READ_DVD_STRUCTURE                   0XAD
#define ATAPI_CMD_READ_FORMAT_CAPACITIES               0X23
#define ATAPI_CMD_READ_TRACK_RZONE_INFORMATION         0X52
#define ATAPI_CMD_RECEIVE_DIAGNOSTIC_RESULTS           0X1C
#define ATAPI_CMD_RELEASE_6                            0X17
#define ATAPI_CMD_RELEASE_10                           0X57
#define ATAPI_CMD_REPAIR_RZONE                         0X58
#define ATAPI_CMD_REPORT_KEY                           0XA4
#define ATAPI_CMD_RESERVE_6                            0X16
#define ATAPI_CMD_RESERVE_10                           0X56
#define ATAPI_CMD_RESERVE_TRACK_RZONE                  0X53
#define ATAPI_CMD_SEND_CUE_SHEET                       0X5D
#define ATAPI_CMD_SEND_DIAGNOSTIC                      0X1D
#define ATAPI_CMD_SEND_EVENT                           0XA2
#define ATAPI_CMD_SEND_KEY                             0XA3
#define ATAPI_CMD_SEND_OPC_INFORMATION                 0X54
#define ATAPI_CMD_SET_READ_AHEAD                       0XA7
#define ATAPI_CMD_SET_STREAMING                        0XB6
#define ATAPI_CMD_SYNCHRONIZE_CACHE                    0X35
#define ATAPI_CMD_VERIFY_10                            0X2F
#define ATAPI_CMD_WRITE_10                             0X2A
#define ATAPI_CMD_WRITE_12                             0XAA
#define ATAPI_CMD_WRITE_VERIFY_10                      0X2E
#define ATAPI_CMD_WRITE_BUFFER                         0X3B


/* sub command of ATA_CMD_SET_MAX */ /* Table 30, 8.38, Ref-1 */

#define ATA_SUB_SET_MAX_ADDRESS          0x00
#define ATA_SUB_SET_MAX_SET_PASS         0x01
#define ATA_SUB_SET_MAX_LOCK             0x02
#define ATA_SUB_SET_MAX_UNLOCK           0x03
#define ATA_SUB_SET_MAX_FREEZE_LOCK      0x04

/* ATA_SUB_SET_MAX_ADDRESS sector count options */
    
#define SET_MAX_VOLATILE     0x00
#define SET_MAX_NON_VOLATILE 0x01

/* sub command of ATA_CMD_SET_FEATURE */ /* Table 27, Page 167, Ref-1 */

/* #define ATA_SUB_ENABLE_8BIT     0x01    Retired.
                                           enable 8bit data transfer */
#define ATA_SUB_ENABLE_WCACHE      0x02    /* enable write cache */
#define ATA_SUB_SET_RWMODE         0x03    /* set transfer mode */
#define ATA_SUB_ENB_ADV_POW_MNGMNT 0x05    /* enable advanced power management*/
#define ATA_SUB_ENB_POW_UP_STDBY   0x06    /* Enable Power-Up In Standby.     */
#define ATA_SUB_POW_UP_STDBY_SPIN  0x07    /* device spin-up.*/
#define ATA_SUB_BOOTMETHOD         0x09    /* Reserved for Address offset
                                              reserved area boot method
                                              technical report */
#define ATA_SUB_ENA_CFA_POW_MOD1   0x0A    /* Enable CFA power mode 1 */
#define ATA_SUB_DISABLE_NOTIFY     0x31    /*Disable Media Status Notification*/

/* #define ATA_SUB_DISABLE_RETRY   0x33    obsolete. disable retry */
/* #define ATA_SUB_SET_LENGTH      0x44    obsolete. length of
                                           vendor specific bytes */
/* #define ATA_SUB_SET_CACHE       0x54    obsolete. set cache segments */

#define ATA_SUB_DISABLE_LOOK       0x55    /* disable read look-ahead feature */
#define ATA_SUB_ENA_INTR_RELEASE   0x5D    /* Enable release interrupt */
#define ATA_SUB_ENA_SERV_INTR      0x5E    /* Enable service interrupt */
#define ATA_SUB_DISABLE_REVE       0x66    /*disable reverting to power on def*/

/* #define ATA_SUB_DISABLE_ECC     0x77    obsolete. disable ECC */
/* #define ATA_SUB_DISABLE_8BIT    0x81    Retired. disable 8bit data transfer*/

#define ATA_SUB_DISABLE_WCACHE     0x82    /* disable write cache */
#define ATA_SUB_DIS_ADV_POW_MNGMT  0x85    /* Disable advanced power mgmt */
#define ATA_SUB_DISB_POW_UP_STDBY  0x86    /* Disable PowerUp In Stdby feature*/

/* #define ATA_SUB_ENABLE_ECC      0x88    obsolete. enable ECC */

#define ATA_SUB_BOOTMETHOD_REPORT  0x89    /* Reserved for Address offset
                                              reserved area boot method
                                              technical report */
#define ATA_SUB_DIS_CFA_POW_MOD1   0x8A    /* Enable CFA power mode 1 */
#define ATA_SUB_ENABLE_NOTIFY      0x95    /* Enable Media Status Notification*/

/*#define ATA_SUB_ENABLE_RETRY     0x99    obsolete. enable retries */

#define ATA_SUB_ENABLE_LOOK        0xaa    /* enable read look-ahead feature */

/*#define ATA_SUB_SET_PREFETCH     0xab    obsolete. set maximum prefetch */
/*#define ATA_SUB_SET_4BYTES       0xbb    obsolete. 4 bytes of vendor
                                           specific bytes */
#define ATA_SUB_ENABLE_REVE        0xcc    /* enable reverting to power on def*/
#define ATA_SUB_DIS_INTR_RELEASE   0xDD    /* Disable release interrupt */
#define ATA_SUB_DIS_SERV_INTR      0xDE    /* Disable service interrupt */

/* sub command of ATA_CMD_SMART */ /* Table 32, Page 184, Ref-1 */

#define ATA_SMART_READ_DATA         0XD0
#define ATA_SMART_ATTRIB_AUTO       0XD2
#define ATA_SMART_SAVE_ATTRIB       0XD3
#define ATA_SMART_OFFLINE_IMMED     0XD4
#define ATA_SMART_READ_LOG_SECTOR   0XD5
#define ATA_SMART_WRITE_LOG_SECTOR  0XD6
#define ATA_SMART_ENABLE_OPER       0XD8
#define ATA_SMART_DISABLE_OPER      0XD9
#define ATA_SMART_RETURN_STATUS     0XDA

/* arg1 values of Sub command ATA_SMART_ATTRIB_AUTO */

#define ATA_SMART_SUB_ENABLE_ATTRIB_AUTO    0xf1
#define ATA_SMART_SUB_DISABLE_ATTRIB_AUTO   0x00

/* arg1 values of Sub command ATA_SMART_OFFLINE_IMMED */

#define ATA_SMART_SUB_EXEC_OFF_IMMED_OFF_MODE           0
#define ATA_SMART_SUB_EXEC_SHORT_SELF_IMMED_OFF_MODE    1
#define ATA_SMART_SUB_EXEC_EXT_SELF_IMMED_OFF_MODE      2
#define ATA_SMART_SUB_ABORT_OFF_MODE_SELF_IMMED         127
#define ATA_SMART_SUB_EXEC_SHORT_SELF_IMMED_CAP_MODE    129
#define ATA_SMART_SUB_EXEC_EXT_SELF_IMMED_CAP_MODE      130

/*
 * arg1 values of Sub command ATA_SMART_READ_LOG_SECTOR /
 *                            ATA_SMART_WRITE_LOG_SECTOR
 */
#define ATA_SMART_SUB_LOG_DIRECTORY 0x00
#define ATA_SMART_SUB_ERROR_LOG     0x01
#define ATA_SMART_SUB_SELF_TEST_LOG 0x06

/* transfer modes of ATA_SUB_SET_RWM ODE */ /* Table 28, Page 168, Ref-1 */

#define ATA_PIO_DEF_W       0x00    /* PIO default trans. mode */
#define ATA_PIO_DEF_WO      0x01    /* PIO default trans. mode, no IORDY */
#define ATA_PIO_W_0         0x08    /* PIO flow control trans. mode 0 */
#define ATA_PIO_W_1         0x09    /* PIO flow control trans. mode 1 */
#define ATA_PIO_W_2         0x0a    /* PIO flow control trans. mode 2 */
#define ATA_PIO_W_3         0x0b    /* PIO flow control trans. mode 3 */
#define ATA_PIO_W_4         0x0c    /* PIO flow control trans. mode 4 */

#define ATA_DMA_SINGLE_0    0x10    /* singleword DMA mode 0 */
#define ATA_DMA_SINGLE_1    0x11    /* singleword DMA mode 1 */
#define ATA_DMA_SINGLE_2    0x12    /* singleword DMA mode 2 */

#define ATA_DMA_MULTI_0     0x20    /* multiword DMA mode 0 */
#define ATA_DMA_MULTI_1     0x21    /* multiword DMA mode 1 */
#define ATA_DMA_MULTI_2     0x22    /* multiword DMA mode 2 */

#define ATA_DMA_ULTRA_0     0x40    /* Ultra DMA mode 0     */
#define ATA_DMA_ULTRA_1     0x41    /* Ultra DMA mode 1     */
#define ATA_DMA_ULTRA_2     0x42    /* Ultra DMA mode 2     */
#define ATA_DMA_ULTRA_3     0x43    /* Ultra DMA mode 3     */
#define ATA_DMA_ULTRA_4     0x44    /* Ultra DMA mode 4     */
#define ATA_DMA_ULTRA_5     0x45    /* Ultra DMA mode 5     */
#define ATA_DMA_ULTRA_6     0x46    /* Ultra DMA mode 6 (not supported) */


/* configuration flags: transfer mode, bits, unit, geometry
 *
 *    15  14  13  12  | 11  10   9   8  |  7   6   5   4  |  3   2   1   0
 *  ------------------|-----------------|-----------------|----------------
 *     0   0   0   0  |  0   0   0   0  |  0   0   0   0  |  0   0   0   0
 *  ---------|--------|--------|--------|----|---|--------|----|-----------|
 *  -BIT MASK|-PIOMASK|--------|GEO MASK|----|---|DMA MASK|PIO-|----Mode---|
 *    32  16   P   P                           U   M   S     P
 *             I   I                           L   U   I     I
 *     B   B   O   O                           T   L   N     O
 *     I   I                                   R   T   G
 *     T   T   M   S                           A   I   L     F
 *     S   S   U   I                                   E     L
 *             L   N                                         O
 *             T   G                           D   D   D     W
 *             I   L                           M   M   M
 *                 E                           A   A   A     C
 *                                                           O
 *                                                           N
 *                                                           T
 *                                                           R
 *                                                           O
 *                                                           L
 *
 */
#define ATA_MODE_MASK    0x00FF               /* transfer mode mask         */
#define ATA_GEO_MASK     0x0300               /* geometry mask              */
#define ATA_PIO_MASK     0x3000               /* RW PIO mask                */
#define ATA_BITS_MASK    0xc000         //wangwenhua   /* RW bits size mask          */

#define ATA_PIO_DEF_0    ATA_PIO_DEF_W        /* PIO default mode           */
#define ATA_PIO_DEF_1    ATA_PIO_DEF_WO       /* PIO default mode, no IORDY */
#define ATA_PIO_0        ATA_PIO_W_0          /* PIO mode 0                 */
#define ATA_PIO_1        ATA_PIO_W_1          /* PIO mode 1                 */
#define ATA_PIO_2        ATA_PIO_W_2          /* PIO mode 2                 */
#define ATA_PIO_3        ATA_PIO_W_3          /* PIO mode 3                 */
#define ATA_PIO_4        ATA_PIO_W_4          /* PIO mode 4                 */
#define ATA_PIO_AUTO     0x000D               /* PIO max supported mode     */

#define ATA_DMA_AUTO     0x0046               /* DMA max supported mode     */

#define ATA_GEO_FORCE    0x0100               /* set geometry in the table  */
#define ATA_GEO_PHYSICAL 0x0200               /* set physical geometry      */
#define ATA_GEO_CURRENT  0x0300               /* set current geometry       */

#define ATA_PIO_SINGLE   0x1000               /* RW PIO single sector       */
#define ATA_PIO_MULTI    0x2000               /* RW PIO multi sector        */

/* PIO Mode codes, these are also offset of set values */

#define ATA_SET_PIO_MODE_0      0x0
#define ATA_SET_PIO_MODE_1      0x1
#define ATA_SET_PIO_MODE_2      0x2
#define ATA_SET_PIO_MODE_3      0x3
#define ATA_SET_PIO_MODE_4      0x4

/* Single, multi and Udma codes, these are also offset of set values */

#define ATA_SET_SDMA_MODE_0     0x0
#define ATA_SET_SDMA_MODE_1     0x1
#define ATA_SET_SDMA_MODE_2     0x2

#define ATA_SET_MDMA_MODE_0     0x0
#define ATA_SET_MDMA_MODE_1     0x1
#define ATA_SET_MDMA_MODE_2     0x2

#define ATA_SET_UDMA_MODE_0     0x0
#define ATA_SET_UDMA_MODE_1     0x1
#define ATA_SET_UDMA_MODE_2     0x2
#define ATA_SET_UDMA_MODE_3     0x3
#define ATA_SET_UDMA_MODE_4     0x4
#define ATA_SET_UDMA_MODE_5     0x5

/* Bit masks */

#define ATA_BIT_MASK0     0x0001
#define ATA_BIT_MASK1     0x0002
#define ATA_BIT_MASK2     0x0004
#define ATA_BIT_MASK3     0x0008
#define ATA_BIT_MASK4     0x0010
#define ATA_BIT_MASK5     0x0020
#define ATA_BIT_MASK6     0x0040
#define ATA_BIT_MASK7     0x0080
#define ATA_BIT_MASK8     0x0100
#define ATA_BIT_MASK9     0x0200
#define ATA_BIT_MASK10    0x0400
#define ATA_BIT_MASK11    0x0800
#define ATA_BIT_MASK12    0x1000
#define ATA_BIT_MASK13    0x2000
#define ATA_BIT_MASK14    0x4000
#define ATA_BIT_MASK15    0x8000


#define ATA_BITS_16      0x4000               /* RW bits size, 16 bits      */
#define ATA_BITS_32      0x8000               /* RW bits size, 32 bits      */

#define ATA_BYTES_PER_BLOC  512

#define ATA_MAX_RW_SECTORS         0x100  /* max sectors per transfer   */
#define ATAPI_CDROM_BYTE_PER_BLK   2048   /* user data in CDROM Model   */
#define ATAPI_BLOCKS                100   /* number of blocks */

#define ATA_MULTISEC_MASK   0x00ff

/* Capabilities fields and masks */

#define ATA_INTER_DMA_MASK  0x8000
#define ATA_CMD_QUE_MASK    0x4000
#define ATA_OVERLAP_MASK    0x2000
#define ATA_IORDY_MASK      0x0800
#define ATA_IOLBA_MASK      0x0200
#define ATA_DMA_CAP_MASK    0x0100

/* hardware reset results - bit masks*/
#define ATA_HWRR_CBLID      0x2000

/* PIO Mode bits and masks */

#define ATA_PIO_MASK_012    0x03  /* PIO Mode 0,1,2 */
#define ATA_PIO_MASK_34     0x02  /* PIO Mode 3,4 */

/* LBA Mask and bits */

#define ATA_LBA_HEAD_MASK   0x0f000000
#define ATA_LBA_CYL_MASK    0x00ffff00
#define ATA_LBA_SECTOR_MASK 0x000000ff

/* capabilities */

#define CAPABIL_DMA         0x0100   /* DMA Supported               */
#define CAPABIL_LBA         0x0200   /* LBA Supported               */
#define CAPABIL_IORDY_CTRL  0x0400   /* IORDY can be disabled       */
#define CAPABIL_IORDY       0x0800   /* IORDY Supported             */
#define CAPABIL_OVERLAP     0x2000   /* Overlap Operation Supported */

/* Command Related Definitions */

/* ATAPI_CMD_START_STOP_UNIT. Page-197,10.8.25,Ref-2 */

#define STOP_DISK           0X00
#define START_DISK          0X01
#define EJECT_DISK          0X02
#define LOAD_DISK           0X03

/* ATAPI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL */

#define MEDIA_UNLOCK        0x00
#define MEDIA_LOCK          0x01

  /* Delay to ensure Status Register content is valid */

#define ATA_WAIT_STATUS     sysDelay ()  /* >= 400 ns */


/* enum */

typedef enum  /* with respect to host/memory */
    {
    IN_DATA  = O_RDONLY, /* from drive to memory */
    OUT_DATA = O_WRONLY, /* to drive from memory */
    NON_DATA             /* non data command     */
    } ATA_DATA_DIR;

/* type defs  */

typedef ATA_RESOURCE ATAPI_RESOURCE;

/* members of this structure shall be in this order only. */

typedef struct ataParams     /* Table 21, Page 108, Ref-1 */
    {
    short config;            /* 0     General configuration - Bit significant */
    UINT16 cylinders;        /* 1     number of Logical cylinders             */
    short specConfig;        /* 2     Specific Configuaraion ( removcyl )     */
    UINT16 heads;            /* 3     number of Logical heads                 */
    short retired4;          /* 4-5   Retired ( bytesTrack )                  */
    short retired5;          /* 4-5   Retired ( bytesSec )                    */
    UINT16 sectors;          /* 6     no. of (Logical sectors)/(logical track)*/
    short retired7;          /* 7-9   Retired ( bytesGap )                    */
    short retired8;          /* 7-9   Retired ( bytesSync )                   */
    short retired9;          /* 7-9   Retired ( vendstat )                    */
    char  serial[20];        /* 10-19 Drive serial number                     */
    short retired20;         /* 20-21 Retired ( type )                        */
    short retired21;         /* 20-21 Retired ( size )                        */
    short obsolete22;        /* 22    obsolete ( bytesEcc )                   */
    char  rev[8];            /* 23-26 firmware revision                       */
    char  model[40];         /* 27-46 model number                            */
    short multiSecs;         /* 47    RW multiple support. bits 15-8 = 80h.
                                      bits 7-0 = 00h reserved, =01h to ffh
                                      max no of secs to transfer /interrupt   */
    short reserved48;        /* 48    reserved                                */
    short capabilities;      /* 49    capabilities - Bit significant          */
    short capabilities2;     /* 50    capabilities                            */
    short pioMode ;          /* 51    Retired (X) ( new- retired51 )          */
    short dmaMode ;          /* 52    Retired (R) ( new- retired52 )          */
    short valid;             /* 53    field validity R-15-3 reserved;
                                      F-2= word 88 validity;
                                      F-1= word 64-70 validity;
                                      V-0= word 54-58 validity                */
    short currentCylinders;  /* 54    number of current logical cylinders (V) */
    short currentHeads;      /* 55    number of current logical heads (V)     */
    short currentSectors;    /* 56    no. of current logical sectors/track (V)*/
    short capacity0;         /* 57    current capacity in sectors             */
    short capacity1;         /* 58    current capacity in sectors             */
    short multiSet;          /* 59    multiple sector setting                 */
    UINT16 lba_size_1;       /* 60    total number of user addressable sectors*/
    UINT16 lba_size_2;       /* 61    total number of user addressable sectors*/
    short singleDma;         /* 62    Retired (R) ( New- retired62)           */
    short multiDma;          /* 63    multi word DMA transfer                 */
    short advancedPio;       /* 64    flow control PIO modes supported        */
    short cycleTimeDma;      /* 65    minimum MDMA transfer cycle time/word   */
    short cycleTimeMulti;    /* 66    recommended multiword DMA cycle time    */
    short cycleTimePioNoIordy; /* 67 min PIO transfer cycle time wo flow ctl  */
    short cycleTimePioIordy;   /* 68    min PIO transfer cycle time w IORDY   */
    short reserved69;          /* 69    reserved */
    short reserved70;          /* 70    reserved */

    /* ATAPI */
    
    short pktCmdRelTime;       /* [71]  Typical Time for Release after Packet */
    short servCmdRelTime;      /* [72]  Typical Time for Release after SERVICE*/
    short majorRevNum;         /* [73]  Major Revision Number (0|FFFF if no)  */
    short minorVersNum;        /* [74]  Minor  Version Number (0|FFFF if no)  */
    short queueDepth;          /* 75    queuedepth bits15-5 reserved;
                                        4-0 (max queue depth-1) */
    short reserved76[4];       /* 76-79 reserved                              */
    short majorVer;            /* 80    Major version Number                  */
    short minorVer;            /* 81    Minor version Number                  */
    short suppCommand1;        /* 82    Command set supported                 */
    short suppCommand2;        /* 83    Command set supported                 */
    short suppCommandFeatureExt;  /* 84    Command set/feature support extn   */
    short enableCommandFeature1;  /* 85    Command set/feature enabled        */
    short enableCommandFeature2;  /* 86    Command set/feature enabled        */
    short defaultCommandFeature;  /* 87    Default Command set/feature        */
    short ultraDmaMode;           /* 88    Ultra DMA mode. bits 15-13 reserved;
                                           12 UDMA mode4;11 UDMA mode3;
                                           10 UDMA mode2;9 UDMA mode1;
                                           8 UDMA mode0;7-5 reserved;
                                           4 upto UDMA4;3 upto UDMA3;
                                           2 upto UDMA2;1 upto UDMA1;
                                           0 upto UDMA0                       */
    short securityEraseTime;      /* 89    Time Required for security erase unit
                                           completion (F) */
    short enSecurityEraseTime;    /* 90    Time Required for enhansed security
                                           erase completion (F) */
    short advPowerManVal;         /* 91    Current adv. power mgmnt value (V) */
    short masterPassRev;          /* 92    Master password Revision code (V)  */
    short hardResetResult;        /* 93      Hardware Reset result            */
    short acousticMgmt;           /* 94    acoustic management */
    short reserved95[5];          /* 95-99  reserved                         */
    UINT16 maxLBA[4];             /* 100-103  reserved */
    short reserved104[23];        /* 104-126  reserved                         */
    short removeNotification;     /* 127     Removable Media Status Notification
                                             feature set supported */
    short securityStatus;         /* 128     Security status                  */
    short vendorSpecific129[31];  /* 129-159 Vendor Specifc                   */
    short cfaPowerMode;           /* 160     CFA power mode 1                 */
    short reserved161[15];        /* 161-175 Reserved                         */
    char  mediaSerialNum[60];     /* 176-205 current media serial number */
    short reserved206[49];        /* 206-254 Reserved                         */
    short integrityWord;          /* 255     Integrity word                   */
    } ATA_PARAM;

/* define ATA_XBD structure/typedef so the xbd functions can find the
 * pointer to the device structure ataDev.  This is a locally defined
 * structure that is passed to the xbd routines.  The first entity is
 * XBD structure - and this must always be the first entity.  Other external
 * routines will modify this part.  The rest of the structure can
 * be ata driver specific.
 */
typedef struct ata_xbd
    {
    XBD        xbd;           /* must be first, add new items below here */
    SEMAPHORE  xbdSemId;      /* used for xbdBlkDevCreateSync() */
    devname_t  name;          /* name of device            */
    device_t   device;        /* device number of this instance */
    UINT32     xbdInserted;   /* device is inserted */
    BOOL       xbdInstantiated;  /* TRUE if stack init is complete */
    struct ataDev *ataDev;       /* necessary to access the device structure */
    } ATA_XBD;

typedef struct ataDev
    {
    BLK_DEV      blkDev;        /* must be here so ATA_DEV* == BLK_DEV*  */
    ATA_XBD      ataXbd;        /* actual xbd for this device */
    int          ctrl;          /* ctrl no.  0 - 1   */
    int          drive;         /* drive no. 0 - 1   */
    UINT32       blkOffset;     /* sector offset     */
    UINT32       nBlocks;       /* number of sectors */

    char         *pBuf;         /* Current position in an user buffer */
    char         *pBufEnd;      /* End of user buffer                 */
    ATA_DATA_DIR direction;     /* Transfer direction                 */
    int          transCount;    /* Number of transfer cycles          */
    int          errNum;        /* Error description message number   */

    /* ATAPI Registers contents */
    
    UINT8        intReason;     /* Interrupt Reason Register */
    UINT8        status;        /* Status Register           */
    UINT16       transSize;     /* Byte Count Register       */

    } ATA_DEV;

typedef struct ataInfo
    {
    UINT32 cylinders;       /* number of cylinders         */
    UINT32 heads;           /* number of heads             */
    UINT32 sectors;         /* number of sectors per track */
    UINT32 bytes;           /* number of bytes per sector  */
    UINT32 precomp;         /* precompensation cylinder    */
    } ATAPI_TYPE;

typedef ATAPI_TYPE ATA_TYPE;

typedef struct ataDrive
    {
    ATA_PARAM    param;               /* geometry parameter        */
    ATAPI_TYPE   *driveInfo;          /* drive info */
    BOOL         okMulti;             /* MULTI: TRUE if supported  */
    BOOL         okIordy;             /* IORDY: TRUE if supported  */
    BOOL         okDma;               /* DMA:   TRUE if supported  */
    BOOL         usingDma;            /* drive configured for dma  */
    BOOL         okInterleavedDMA;    /* Interleaved DMA operation */
    BOOL         okCommandQue;        /* Queue Command operation   */
    BOOL         okOverlap;           /* OverLap operation         */
    BOOL         okRemovable;         /* Removable Media:          */
    BOOL         supportSmart;        /* Supports SMART feature set*/
    BOOL         use48LBA;            /* use 48 bit logical address  */
    sector_t     capacity;            /* max capacity of drive     */
    short        multiSecs;           /* supported max sectors RW  */
    short        pioMode;             /* supported max PIO mode    */
    short        singleDmaMode;       /* supported max
                                         single word DMA mode. obs */
    short        multiDmaMode;        /* supported max
                                         multi word DMA mode       */
    short        ultraDmaMode;        /* supported max
                                         Ultra DMA mode            */
    short        rwMode;              /* RW mode:
                                         PIO[0,1,2,3,4],
                                         Single DMA[0,1,2],
                                         Multi word DMA[0,1,2],
                                         Ultra DMA[0,1,2,3,4,5]    */
    short        rwBits;              /* RW bits: 16 or 32         */
    short        rwPio;               /* RW PIO unit: single or
                                         multi sector              */

    UINT8        state;               /* device state              */
    UINT8        checkPower;          /* status from ATA_CMD_CHECK_PWR, 
                                         what is power state */
    UINT8        diagCode;            /* diagnostic code           */
    UINT8        type;                /* device type ATA/ATAPI/NONE*/
    UINT8        driveType;           /* drive type HDD/CD-ROM/CD-R/
                                         CD-RW/DVD/TAPE/ZIP/ZAJ/
                                         LS-120/ etc. */
    UINT8        okLba;               /* LBA:   0x40 if supported  */
    UINT8        rsrv[2];             /* byte fill */
                                                                 
    STATUS       (*Reset)(int Ctrl,int dev);  /* pointer to reset function */
    ATA_DEV      *pAtaDev;                    /* pointer to ATA block device
                                                 structure                 */
    UINT8        cmdLength;                   /* 12 or 16 byte command     */
    
    /* setfeature : sub 0x95     */
    
    BOOL         okPEJ;                       /* Power Eject capable.      */
    BOOL         okLock;                      /* Supports Lock             */
    short        mediaStatusNotifyVer;        /* status notify version     */
    UINT16       nativeMaxAdd[4];             /* 3 = LBA 27:24 / Head
                                               * 2 = LBA 23:16 / cylHi
                                               * 1 = LBA 15:8  / cylLow
                                               * 0 = LBA 7:0   / sector no
                                               */
    short        CFAerrorCode;                /* result of "CFA request
                                               * extended error code" cmd  */
    UINT32       signature;    
} ATA_DRIVE;

typedef struct ataReg
    {
    UINT32       *data;         /* (RW) data register (16 bits)       */
    UINT32       *error;        /* (R)  error register                */
    UINT32       *feature;      /* (W) feature or write-precomp       */
    UINT32       *seccnt;       /* (RW) sector count                  */
    UINT32       *sector;       /* (RW) first sector number           */
    UINT32       *cylLo;        /* (RW) cylinder low byte             */
    UINT32       *cylHi;        /* (RW) cylinder high byte            */
    UINT32       *sdh;          /* (RW) sector size/drive/head        */
    UINT32       *command;      /* (W)  command register              */
    UINT32       *status;       /* (R)  immediate status              */
    UINT32       *aStatus;      /* (R)  alternate status              */
    UINT32       *dControl;     /* (W)  disk controller control       */
    } ATA_REG;

typedef struct ataCtrl
    {
    ATA_DRIVE    drive[ATA_MAX_DRIVES]; /* drives per controller              */
    ATA_REG      ataReg;                /* Ata registers located in drives    */
    SEMAPHORE    ataBioReadySem;   /* bio queue counting semaphore */ 
    SEMAPHORE    syncSem;          /* binary sem for syncronization      */
    SEMAPHORE    muteSem;          /* mutex sem for mutual-exclusion     */
    struct bio   *bioQueueh;       /* bio queue head for master device   */
    struct bio   *bioQueuet;       /* bio queue tail for master device   */
    int          svcTaskId;        /* id of service task for this ctrl   */
    WDOG_ID      wdgId;            /* watch dog                          */
    int          ctrl;             /* controller number */
    int          pwrdown;          /* power down mode                    */
    int          ctrlType;         /* type of controller                 */
    int          intVector;        /* interrupt vector                   */
    int          intLevel;         /* interrupt level                    */
    int          intCount;         /* interrupt count                    */
    int          intStatus;        /* interrupt status                   */
    int          configType;       /* user recommended configuaration.
                                    * This is the value passed to ataPiDrv
                                    * during driver intialization.
                                    */
    int          semTimeout;       /* timeout seconds for sync semaphore */
    int          wdgTimeout;       /* timeout seconds for watch dog      */
    BOOL         wdgOkay;          /* watch dog status                   */
    BOOL         installed;        /* TRUE if a driver is installed      */
    BOOL         changed;          /* TRUE if a card is installed        */
    BOOL         uDmaCableOkay;    /* Set to 1 if the devices are connected
                                    * to the controller with a 80 conductor
                                    * cable using 40 pin connector
                                    * (UDMA cable).
                                    */
    /* The following function pointers  and Boolean must be set by the 
     * bsp (sysAta.c) sysAtaInit() function
     */
    FUNCPTR ataIntConnect;         /* interrupt connect routine */
    FUNCPTR ataIntDisconnect;      /* interrupt disconnect routine */
    FUNCPTR ataIntEnable;          /* interrupt enable routine */
    FUNCPTR ataIntDisable;         /* interrupt disable routine */
    FUNCPTR ataIntPreProcessing;   /* this can be used for executing
                                    * code in the beginning of the
                                    * interrupt service routine */
    FUNCPTR ataIntPostProcessing;  /* this can be used for executing
                                    * code in the beginning of the
                                    * interrupt service routine */
    FUNCPTR ataDmaInit;            /* initialize the DMA hardware */
    FUNCPTR ataDmaSet;             /* setup the DMA hardware for xfer */
    FUNCPTR ataDmaStart;           /* start the DMA operation */
    FUNCPTR ataDmaStop;            /* stop DMA functions */
    FUNCPTR ataDmaCheck;           /* test status of DMA */
    FUNCPTR ataDmaModeSet;         /* set mode of DMA operations */
    FUNCPTR ataDmaModeNegotiate;   /* determine DMA xfer mode */
    FUNCPTR ataDmaReset;           /* reset the DMA controller */
    BOOL    ataHostDmaSupportOkay; /* DMA supported */
#ifdef _WRS_VX_SMP
    spinlockIsr_t spinlock;		   /* SMP */
#endif /* _WRS_VX_SMP */
    } ATA_CTRL;

typedef struct ataRaw
    {
                         /* this is for ATA RAW ACCESS ioctl        */
    UINT32 cylinder;     /* cylinder (0 -> (cylindres-1))           */
    UINT32 head;         /* head (0 -> (heads-1))                   */
    UINT32 sector;       /* sector (1 -> sectorsTrack)              */
    char   *pBuf;        /* pointer to buffer (bytesSector * nSecs) */
    UINT32 nSecs;        /* number of sectors (1 -> sectorsTrack)   */
    int    direction;    /* read=0, write=1                         */
    } ATA_RAW;


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


typedef struct smart_data /* Table 34, section 8.41.5.8, Ref-1 */
    {
    short    vendorSpecific0[362];         /* 0 - 361     */
    short    offLineCollectionStatus;      /* 362
                                            * Table 35, 8.41.5.8.1 Ref-1
                                            */
    short    selfTestExecutionStatus;      /* 363
                                            * Table 36, 8.41.5.8.2 Ref-1
                                            */
    short    timeOffLine1;                 /* 364 in secs */
    short    timeOffLine2;                 /* 365 in secs */
    short    vendorSpecific366;            /* 366         */
    short    offlineCollectionCapability;  /* 367         */
    short    smartCapability1;             /* 368         */
    short    smartCapability2;             /* 369         */
    short    errorLogCapability;           /* 370
                                            * bit-0:  1-supported
                                            *         0-notsupported
                                            */
    short    vendorSpecific371;            /* 371         */
    short    shortSelfTestPollTime;        /* 372 in mins */
    short    extendedSelfTestPollTime;     /* 373 in mins */
    short    reserved374[12];              /* 374 - 385   */
    short    vendorSpecific386[125];       /* 386-510     */
    short    checksum;                     /* 511         */
    } SMART_DATA;


typedef struct smart_log_dir_entry
    {
    short          noOfSectors;
    short          reserved;
    } SMART_LOG_DIR_ENTRY;


typedef struct smart_log_directory    /* Table 38, section 8.41.6.8.1, Ref-1 */
    {
    short           smartLogVersion1;         /* 0                 */
    short           smartLogVersion2;         /* 1                 */
    SMART_LOG_DIR_ENTRY smartLogEntry[255];       /* 2 - 511 (255 * 2) */
    } SMART_LOG_DIRECTORY;


typedef struct readTocStruct
    {
    UINT32   transferLength;
    UINT8 *  pResultBuf;
    } READ_TOC_STRUCT;


typedef struct command_data_struct /* Table 41 , 8.41.6.8.2.3.1, Ref-1 */
    {
    short dControl;   /* Device Control / Alternate status */
    short feature;    /* Error / Feature */
    short seccnt;
    short sector;
    short cylLo;
    short cylHi;
    short sdh;
    short command;
    short timeStampLSB;
    short timeStampNextLSB;
    short timeStampNextMSB;
    short timeStampMSB;
    } COMMAND_DATA_STRUCT;


typedef struct error_data_struct /* Table 42 , 8.41.6.8.2.3.2, Ref-1 */
    {
    short reserved0;
    short error;                 /* Error / Feature */
    short seccnt;
    short sector;
    short cylLo;
    short cylHi;
    short sdh;
    short status;                /* Status / command */
    short extendedErrorInfo[19];
    short state;                 /* 27 */
    short LifeTimeStampLSB;      /* 28 */
    short LifeTimeStampMSB;      /* 29 */
    } ERROR_DATA_STRUCT;


typedef struct error_log_data_struct /* Table 40, 8.41.6.8.2.3, Ref-1 */
    {
    COMMAND_DATA_STRUCT commandDataStruct[5]; /* fifth is where error reported*/
    ERROR_DATA_STRUCT   errorDataStruct;
    } ERROR_LOG_DATA_STRUCT;


typedef struct smart_error_log_sector /* Table 39, 8.41.6.8.2 , Ref-1 */
    {
    short                    errorLogVersion;        /* 0       */
    short                    errorLogIndex;          /* 1       */
    ERROR_LOG_DATA_STRUCT    ErrLogDataStruct[5];    /* 2 - 91
                                                      * 92-181
                                                      * 182-271
                                                      * 272-361
                                                      * 362-451
                                                      */
    short                    DeviceErrCount1;        /* 452     */
    short                    DeviceErrCount2;        /* 453     */
    short                    reserved454[57];        /* 454-510 */
    short                    checksum;               /* 511     */
    } SMART_ERROR_LOG_SECTOR;


typedef struct self_test_descriptor  /* Table 45, 8.41.6.8.3.2, Ref-1 */
    {
    short sector;               /* 0 */
    short selfTestExecStatus;   /* 1 */
    short lifeTimeStampLSB;     /* 2 */
    short lifeTimeStampMSB;     /* 3 */
    short failChechPoint;       /* 4 */
    short failingLBALSB;        /* 5 */
    short failingLBAnextLSB;    /* 6 */
    short failingLBAnextMSB;    /* 7 */
    short failingLBAMSB;        /* 8 */
    short vendorSpecific[15];   /* 9 - 23 */
    } SELF_TEST_DESCRIPTOR;


typedef struct self_test_log /* Table 44, 8.41.6.8.3, Ref-1 */
    {
    short                   revision;
    SELF_TEST_DESCRIPTOR    descriptorEntry[21]; /* 2 - 505 (24 * 21) */
    short                   vendorSpecific1;     /* 506 */
    short                   vendorSpecific2;     /* 507 */
    short                   selfTestIndex;       /* 508 */
    short                   reserved509;         /* 509 */
    short                   reserved510;         /* 510 */
    short                   checksum;            /* 511 */
    } SELF_TEST_LOG;

typedef UINT8 CMD_PKT [ATAPI_MAX_CMD_LENGTH];

extern ATA_RESOURCE ataResources[];

/* function declarations  */

extern STATUS  ataDrv  (int ctrl, int drives, int vector, int level,
                        BOOL configType, int semTimeout, int wdgTimeout);
extern BLK_DEV *ataDevCreate   (int ctrl, int drive, UINT32 nBlks, UINT32 offset);
extern device_t ataXbdDevCreate (int ctrl, int drive, UINT32 nBlks, UINT32 offset, const char *);
extern UINT8   atapiPktCmdSend   (ATA_DEV * pAtapiDev, ATAPI_CMD * pComPack);
extern UINT16  atapiGetCylinderCount        (int ctrl, int drive);
extern UINT8   atapiGetHeadCount            (int ctrl, int drive);
extern char *  atapiGetDriveSerialNumber    (int ctrl, int drive);
extern char *  atapiGetFirmwareRevision     (int ctrl, int drive);
extern char *  atapiGetModelNumber          (int ctrl, int drive);
extern UINT32  atapiGetFeatureSupported     (int ctrl, int drive);
extern UINT32  atapiGetFeatureEnabled       (int ctrl, int drive);
extern UINT8   atapiGetMaxDmaMode           (int ctrl, int drive);
extern UINT8   atapiGetCurrentDmaMode       (int ctrl, int drive);
extern UINT8   atapiGetMaxPioMode           (int ctrl, int drive);
extern UINT8   atapiGetCurrentPioMode       (int ctrl, int drive);
extern UINT8   atapiGetCurrenRwMode         (int ctrl, int drive);
extern UINT8   atapiGetDriveType            (int ctrl, int drive);
extern UINT32  atapiGetAtapiVersionNumber   (int ctrl, int drive);
extern UINT16  atapiGetRemovMediaStatusNotifyVer (int ctrl, int drive);
extern UINT16  atapiGetCurrentCylinderCount (int ctrl, int drive);
extern UINT8   atapiGetCurrentHeadCount     (int ctrl, int drive);
extern UINT16  atapiGetBytesPerTrack        (int ctrl, int drive);
extern UINT16  atapiGetBytesPerSector       (int ctrl, int drive);
extern STATUS  atapiIoctl (int function, int ctrl, int drive, int password [16],
                           int arg0, UINT32 * arg1, UINT8 ** ppBuf);
extern STATUS  atapiInit (int ctrl, int dev);
extern STATUS  atapiTestUnitRdy (ATA_DEV *pAtapiDev);
extern STATUS  atapiReadCapacity (ATA_DEV *pAtapiDev);
extern STATUS  atapiCtrlMediumRemoval (ATA_DEV * pAtapiDev, int arg0);
extern STATUS  atapiRequestSense (ATA_DEV * pAtapiDev,char * pBuf);
extern STATUS  atapiReadTocPmaAtip (ATA_DEV * pAtapiDev, UINT32 transferLength,
                                    char    * resultBuf);
extern STATUS  atapiRead10 (ATA_DEV * pAtapiDev,UINT32 startBlk, 
                            UINT32 nBlks, UINT32 transferLength, char * pBuf);
extern UINT8   atapiGetMaxSDmaMode (int ctrl,int drive );
extern UINT8   atapiGetCurrentSDmaMode (int ctrl,int drive );
extern UINT8   atapiGetCurrentRwMode (int ctrl,int drive );
extern STATUS  atapiScan         (ATA_DEV * pAtapiDev, UINT32 startAddressField,
                                  int function);
extern STATUS  atapiSeek         (ATA_DEV * pAtapiDev, UINT32 addressLBA);   
extern STATUS  atapiSetCDSpeed   (ATA_DEV * pAtapiDev, int readDriveSpeed, 
                                  int writeDriveSpeed);    
extern STATUS  atapiStopPlayScan (ATA_DEV * pAtapiDev);
extern STATUS  atapiStartStopUnit (ATA_DEV * pAtapiDev,int arg0);
extern STATUS  ataCtrlReset (int ctrl);
extern STATUS  ataParamRead (int ctrl, int drive,void *buffer, int command);
extern STATUS  ataPiInit (int ctrl, int drive );
extern STATUS ataBlkRW (ATA_DEV * pDev, sector_t startBlk, UINT32 nBlks,
                        char * p,int direction);
extern STATUS  ataRW    (int ctrl, int drive, UINT32 cylinder, UINT32 head, 
                     UINT32 sector, void *p, UINT32 nSecs, int direction, sector_t);
extern STATUS  ataDmaRW (int ctrl, int drive, UINT32 cylinder, UINT32 head, 
                     UINT32 sector, void *p, UINT32 nSecs, int direction, sector_t);
extern STATUS  ataWait (int ctrl, int request, BOOL reset);
extern STATUS  ataStatusChk (ATA_CTRL * pCtrl, UINT8 mask, UINT8 status);
extern STATUS  ataPRead (int ctrl, int drive, void *p);
extern STATUS  ataInit (int ctrl, int drive);
extern STATUS  ataCmd (int ctrl, int drive, int cmd, int arg0, int arg1,
                      int arg2, int arg3, int arg4, int arg5);
extern STATUS  ataDevIdentify (int ctrl, int dev);
extern STATUS  atapiInit (int ctrl, int dev);
extern STATUS  atapiPRead (int ctrl, int drive, void * buffer);
extern UINT8   atapiPktCmd (ATA_DEV   * pAtapiDev,ATAPI_CMD * pComPack);
extern STATUS  atapiNonDataCmd (ATA_DEV  * pAtapiDev, ATAPI_CMD * pComPack);
extern STATUS  ataShowInit (void);
extern STATUS  ataDrv  (int ctrl, int drives, int vector, int level,
                        BOOL configType, int semTimeout, int wdgTimeout);

extern STATUS  ataRawio (int ctrl, int drive, ATA_RAW *pAtaRaw);
extern STATUS  ataXbdRawIo (device_t device, sector_t sector, UINT32 numSecs,
			 char *data, int direction);
#endif  /* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCataDrvh */

