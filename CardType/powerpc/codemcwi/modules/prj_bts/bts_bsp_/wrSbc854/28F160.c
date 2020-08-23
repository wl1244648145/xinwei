/* sysMtd.c - MTD driver for flash(s) on wrSbc8548 */

/*
 * Copyright (c) 2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,30mar07,b_m  written.
*/

/*
DESCRIPTION
This file provides the TFFS MTD driver for the 4 banks of 8-bit Intel TE28F160
flash on the wrSbc8548 board. The driver handles the low level operations like
erase and program of the flash. It also provides an identify routine to check if
it is compatible with the devices.

The macros DEBUG_PRINT must be defined as 'printf' to provide informative debug
messages if necessary.

The driver is only compatible and tested with wrSbc8548 board. DO NOT use this
driver on any other boards unless the flashes are absolutely the same.

INCLUDE FILES:

SEE ALSO:
*/

#include <stdio.h>
#include "tffs/flflash.h"
#include "tffs/backgrnd.h"

/* defines */

/*#undef  DEBUG_PRINT*/
#define DEBUG_PRINT  printf


#define INTEL_VENDOR_ID         0x9100

#define FLASH_2M_DEV_ID					0X4311
#define FLASH_4M_DEV_ID         0x16
#define FLASH_8M_DEV_ID         0x17
#define FLASH_16M_DEV_ID        0x18
#define FLASH_32M_DEV_ID        0x1D

#define VENDOR_ID						   	0x9100
#define DEVICE_ID   						0x4311



#define TE28F160_VENDOR_OFFSET 0
#define TE28F160_DEVICE_OFFSET 1

#define TE28F160_FLASH_ID      ((INTEL_VENDOR_ID << 8) | FLASH_16M_DEV_ID)

#define TE28F160_FLASH_SIZE    0x00200000
#define TE28F160_FLASH_NUM     1
#define TE28F160_SECTOR_SIZE   0x20000
#define TE28F160_BLOCK_NUM		 39
/*
#define TE28F160_CMD_RESET             0xFFFFFFFF
#define TE28F160_CMD_READ_STATUS       0x70707070
#define TE28F160_CMD_CLEAR_STATUS      0x50505050
#define TE28F160_CMD_ERASE_BLOCK       0x20202020
#define TE28F160_CMD_ERASE_CONFIRM     0xD0D0D0D0
#define TE28F160_CMD_PROGRAM           0x10101010
#define TE28F160_CMD_READ_ID           0x90909090

#define TE28F160_STATUS_READY          0x80808080
#define TE28F160_STATUS_READY_MASK     0x80808080
#define TE28F160_STATUS_ERA_ERR_MASK   0x20202020
#define TE28F160_STATUS_PRG_ERR_MASK   0x10101010
#define TE28F160_STATUS_ERR_MASK       (TE28F160_STATUS_ERA_ERR_MASK | TE28F160_STATUS_PRG_ERR_MASK)
*/


/*************for WBBU ver 1.0 PCB**********************/
/*
#define TE28F160_CMD_RESET             0xFFFF
#define TE28F160_CMD_READ_STATUS       0x0e00
#define TE28F160_CMD_CLEAR_STATUS      0x0a00
#define TE28F160_CMD_ERASE_BLOCK       0x0400
#define TE28F160_CMD_ERASE_CONFIRM     0x0b00
#define TE28F160_CMD_PROGRAM           0x0800
#define TE28F160_CMD_READ_ID           0x0900
#define TE28F160_CMD_LOCK1						 0X0600
#define TE28F160_CMD_LOCK2						 0X8000	
#define TE28F160_CMD_UNLOCK1					 0X0600	
#define TE28F160_CMD_UNLOCK2					 0X0B00



#define TE28F160_STATUS_READY          0x0100
#define TE28F160_STATUS_READY_MASK     0x0100
#define TE28F160_STATUS_ERA_ERR_MASK   0x0400
#define TE28F160_STATUS_PRG_ERR_MASK   0x0800
#define TE28F160_STATUS_ERR_MASK       (TE28F160_STATUS_ERA_ERR_MASK | TE28F160_STATUS_PRG_ERR_MASK)
*/


#define TE28F160_CMD_RESET                    	0xFFFF
#define TE28F160_CMD_READ_STATUS       	0x0070	     
#define TE28F160_CMD_CLEAR_STATUS      	0x0050    	 
#define TE28F160_CMD_ERASE_BLOCK        	0x0020      	
#define TE28F160_CMD_ERASE_CONFIRM   	0x00d0    
#define TE28F160_CMD_PROGRAM               	0x0010    
#define TE28F160_CMD_READ_ID                	0x0090    
#define TE28F160_CMD_LOCK1				0x0060		 
#define TE28F160_CMD_LOCK2				0x0001		 	
#define TE28F160_CMD_UNLOCK1			0x0060		 	
#define TE28F160_CMD_UNLOCK2			0x00d0		 



#define TE28F160_STATUS_READY               	0x0080	
#define TE28F160_STATUS_READY_MASK     	0x0080	
#define TE28F160_STATUS_ERA_ERR_MASK   	0x0020	
#define TE28F160_STATUS_PRG_ERR_MASK   	0x0010	
#define TE28F160_STATUS_ERR_MASK       		(TE28F160_STATUS_ERA_ERR_MASK | TE28F160_STATUS_PRG_ERR_MASK)



#define TE28F160_OP_TIMEOUT            100000
#define FLASH_OP_BASE 									0xffe00000/*FLASH1_BASE_ADRS*/

#define TE28F160_RESET(addr)   \
    do  \
    {   \
        *addr = TE28F160_CMD_RESET;    \
    } while(0);

#define TE28F160_READ_STATUS(addr) \
    do  \
    {   \
        *addr = TE28F160_CMD_READ_STATUS;  \
    } while(0);

#define TE28F160_CLEAR_STATUS(addr)    \
    do  \
    {   \
        *addr = TE28F160_CMD_CLEAR_STATUS;   \
    } while(0);

#define TE28F160_ERASE_BLOCK(addr) \
    do  \
    {   \
        *addr = TE28F160_CMD_ERASE_BLOCK;  \
    } while(0);

#define TE28F160_ERASE_CONFIRM(addr)   \
    do  \
    {   \
        *addr = TE28F160_CMD_ERASE_CONFIRM;    \
    } while(0);

#define TE28F160_PROGRAM(addr, value)  \
    do  \
    {   \
        *addr = TE28F160_CMD_PROGRAM;    \
        *addr = value;  \
    } while(0);

#define TE28F160_READ_ID(addr) \
    do  \
    {   \
        *addr = TE28F160_CMD_READ_ID;  \
    } while(0);
/*
#define TE28F160_UNLOCK(addr) \
    do  \
    {   \
        *addr = TE28F160_CMD_READ_ID;  \
    } while(0);
*/

/************************************************
Get the offset address of dedicated Block
*************************************************/
UINT32 blockOffsetAddrs(UINT16 blk)
{	
			if(blk<31)  /****top 32k size blocks ****/
				return (UINT32)(blk*32*1024*2); 
			else       /*****other 4k size blocks****/
				return (UINT32)(31*32*1024*2 + (blk-31)*4*1024*2);
}

/******unlock a flash block, the flash block is set as locked after power-cycle or Reset********/

void unLockBlock(UINT blk)
{
			if(blk<31)  /**** 32k size blocks ****/
			{
						*(UINT16*)((blk*32*1024*2)+ FLASH_OP_BASE) = TE28F160_CMD_UNLOCK1;
						*(UINT16*)((blk*32*1024*2)+ FLASH_OP_BASE) = TE28F160_CMD_UNLOCK2;
			}
			else       /*****top 4k size blocks****/
			{
						*(UINT16*)(31*32*1024*2 + (blk-31)*4*1024*2 + FLASH_OP_BASE) = TE28F160_CMD_UNLOCK1;
						*(UINT16*)(31*32*1024*2 + (blk-31)*4*1024*2 + FLASH_OP_BASE) = TE28F160_CMD_UNLOCK2;
			}
			
}

void lockBlock(UINT blk)
{
			if(blk<31)  /**** 32k size blocks ****/
			{
						*(UINT16*)((blk*32*1024*2)+ FLASH_OP_BASE) = TE28F160_CMD_LOCK1;
						*(UINT16*)((blk*32*1024*2)+ FLASH_OP_BASE) = TE28F160_CMD_LOCK2;
			}
			else       /*****top 4k size blocks****/
			{
						*(UINT16*)(31*32*1024*2 + (blk-31)*4*1024*2 + FLASH_OP_BASE) = TE28F160_CMD_LOCK1;
						*(UINT16*)(31*32*1024*2 + (blk-31)*4*1024*2 + FLASH_OP_BASE) = TE28F160_CMD_LOCK2;
			}
			
}
#if 1
/*******************************************************************************/

/*******************************************************************************
*
* TE28F160Program - low level byte programming routine
*
*/

FLStatus TE28F160Program
    (
    UINT16 *    addr,
    UINT16      value
    )
    {
    UINT16  status = TE28F160_STATUS_READY;
    UINT32  timeout = 0;

    /* set timeout = 5s */
    TE28F160_READ_STATUS(addr);
    do
    {
    		timeout++;
        status = *addr;
        if (timeout >= TE28F160_OP_TIMEOUT)
            break;
    } while ((status & TE28F160_STATUS_READY_MASK) != TE28F160_STATUS_READY);

    if ((status & TE28F160_STATUS_ERR_MASK) != 0)
        TE28F160_CLEAR_STATUS(addr);

    TE28F160_PROGRAM(addr, value);

    /* set timeout = 5s */
    timeout = 0;
    do
    {
    		timeout++;
        status = *addr;
        if (timeout >= TE28F160_OP_TIMEOUT)
        {
						printf("\nTime out. Status is %x /*Time is %x,*/ Time out is %x",status,/*flMsecCounter,*/timeout);
            TE28F160_RESET(addr);
            return flTimedOut;
        }
    } while ((status & TE28F160_STATUS_READY_MASK) != TE28F160_STATUS_READY);

    /* check program error bit */
    if (status & TE28F160_STATUS_PRG_ERR_MASK)
    {
				printf("\n Error is %x",status);
        TE28F160_RESET(addr);
        return flWriteFault;
    }

    TE28F160_RESET(addr);
    return flOK;
    }

/*******************************************************************************
*
* TE28F160Write - write routine for TE28F160 flash
*
*/

FLStatus TE28F160Write
    (
    int     				offsetAddress,
    const void  		*buffer,
    int             length
    )
    {
    UINT16*  	chipAddress;
    UINT16 		data;
    int i;
    
    if(((UINT32)buffer)&0x1 || length&0x1)
    {
#ifdef DEBUG_PRINT
        DEBUG_PRINT("\nBuffer address or length should be Word aligned.\n");
#endif
    return flWriteFault;
    }
    
    chipAddress = offsetAddress + FLASH_OP_BASE;
    for (i=0;i<length/2;i++)
    {
    		data =*(UINT16*)(buffer + 2*i);
        if (TE28F160Program(chipAddress, data) != flOK)
        {
#ifdef DEBUG_PRINT
        DEBUG_PRINT("[TE28F160Write]: program flash error @ 0x%08x ...\n", chipAddress);
#endif

        		TE28F160_RESET(chipAddress);
            return flWriteFault;
    		}
    		chipAddress ++;
    	
    }

    for (i=0;i<length/2;i++)
    {
				if(*(UINT16*)(FLASH_OP_BASE + offsetAddress + 2*i) != *(UINT16*)(buffer + 2*i))
				{
#ifdef DEBUG_PRINT
        DEBUG_PRINT("[TE28F160Write]: data double check error @ 0x%08x ...\n", FLASH_OP_BASE + i/2);
#endif
        		TE28F160_RESET(chipAddress);
            return flWriteFault;
    		}
    	
    }

    return flOK;
    }

/*******************************************************************************
*
* TE28F160Erase - erase routine for TE28F160 flash
*
*/

FLStatus TE28F160Erase
    (
/*    FLFlash vol,*/
    int     firstErasableBlock,
    int     numOfErasableBlocks
    )
    {
    UINT16 * block = NULL;
    UINT16 status = TE28F160_STATUS_READY;
    UINT16 timeout = 0;
    int i;
/*
    if (flWriteProtected(vol.socket))
	    return flWriteProtect;
*/
    for (i = firstErasableBlock; i < firstErasableBlock + numOfErasableBlocks; i++)
	{
				block = blockOffsetAddrs(i)+ FLASH_OP_BASE;
/*	    block = (UINT16 *)vol.map(&vol, i * vol.erasableBlockSize, 0);
*/

#ifdef DEBUG_PRINT
        DEBUG_PRINT("\nErasing block#%03d @ 0x%08x ...\r", i, block);
#endif


    	/* set timeout = 5s */
    	timeout = 0;
        TE28F160_READ_STATUS(block);
        do
        {
        		timeout++;
            status = *block;
            if (timeout >= TE28F160_OP_TIMEOUT)
                break;
        } while ((status & TE28F160_STATUS_READY_MASK) != TE28F160_STATUS_READY);

        if ((status & TE28F160_STATUS_ERR_MASK) != 0)
            TE28F160_CLEAR_STATUS(block);

				unLockBlock(i);
				
        TE28F160_ERASE_BLOCK(block);
        TE28F160_ERASE_CONFIRM(block);
			
    	/* set timeout = 5s */
    	timeout = 0;
        do
        {
            status = *block;
            if (timeout >= TE28F160_OP_TIMEOUT)
            {
            		timeout++;
                TE28F160_RESET(block);
                return flTimedOut;
            }
        } while ((status & TE28F160_STATUS_READY_MASK) != TE28F160_STATUS_READY);

        /* check erase error bit */
    	if (status & TE28F160_STATUS_ERA_ERR_MASK)
        {
            TE28F160_RESET(block);
 			   		return flWriteFault;
        }
	}

    TE28F160_RESET(block);
    return flOK;
    }


/*******************************************************************************
*
* TE28F160Identify - identify routine for TE28F160 flash
*
*/

FLStatus TE28F160Identify
    (
/*    FLFlash vol  */
    )
    {
    UINT16 * base = (UINT16 *)FLASH_OP_BASE;
    UINT16   vendor, device;

#ifdef DEBUG_PRINT
    DEBUG_PRINT("Entering TE28F160Identify routine @ base address 0x%08x ...\n", (UINT32)base);
#endif

    /* check the flash id */
    TE28F160_READ_ID(base);
    vendor = *(base + TE28F160_VENDOR_OFFSET);
    device = *(base + TE28F160_DEVICE_OFFSET);
#ifdef DEBUG_PRINT
    DEBUG_PRINT("Vendor is %x, Device is %x",vendor,device);
#endif

    if ((vendor == VENDOR_ID) && (device == DEVICE_ID))
       ;/* vol.type = TE28F160_FLASH_ID;*/
    else
    {
        TE28F160_RESET(base);
        return flUnknownMedia;
    }
/*
    vol.chipSize = TE28F160_FLASH_SIZE;
    vol.noOfChips = TE28F160_FLASH_NUM;
    vol.interleaving = TE28F160_FLASH_NUM;
    vol.erasableBlockSize = TE28F160_SECTOR_SIZE * vol.interleaving;
    vol.write = TE28F160Write;
    vol.erase = TE28F160Erase;
*/
    TE28F160_RESET(base);
    return flOK;
    }
#endif
