/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bbu_config.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_SHMEM_H
#define BSP_SHMEM_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <net/if.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "fcntl.h"
#include "unistd.h"
#include "stdio.h"

#define MAS0 624 
#define MAS1 625 
#define MAS2 626 
#define MAS3 627 
#define MAS4 628 
#define MAS5 339 
#define MAS6 630 
#define MAS7 944 
#define MAS8 341 

#define MAS0_TLBSEL(x)	((x << 28) & 0x30000000)
#define MAS0_ESEL(x)	((x << 16) & 0x0FFF0000)
#define MAS0_NV(x)	((x) & 0x00000FFF)
#define MAS0_TLBSEL_MSK	0x30000000
#define MAS0_ESEL_MSK	0x0FFF0000

#define MAS1_VALID 	0x80000000
#define MAS1_IPROT	0x40000000
#define MAS1_TID(x)	((x << 16) & 0x3FFF0000)
#define MAS1_TS		0x00001000
#define MAS1_TSIZE(x)	((x << 8) & 0x00000F00)

#define MAS2_EPN	0xFFFFF000
#define MAS2_X0		0x00000040
#define MAS2_X1		0x00000020
#define MAS2_W		0x00000010
#define MAS2_I		0x00000008
#define MAS2_M		0x00000004
#define MAS2_G		0x00000002
#define MAS2_E		0x00000001

#define MAS3_RPN	0xFFFFF000
#define MAS3_U0		0x00000200
#define MAS3_U1		0x00000100
#define MAS3_U2		0x00000080
#define MAS3_U3		0x00000040
#define MAS3_UX		0x00000020
#define MAS3_SX		0x00000010
#define MAS3_UW		0x00000008
#define MAS3_SW		0x00000004
#define MAS3_UR		0x00000002
#define MAS3_SR		0x00000001

#define MAS4_TLBSELD(x) MAS0_TLBSEL(x)
#define MAS4_TIDDSEL	0x000F0000
#define MAS4_TSIZED(x)	MAS1_TSIZE(x)
#define MAS4_X0D	0x00000040
#define MAS4_X1D	0x00000020
#define MAS4_WD		0x00000010
#define MAS4_ID		0x00000008
#define MAS4_MD		0x00000004
#define MAS4_GD		0x00000002
#define MAS4_ED		0x00000001

#define MAS6_SPID0	0x3FFF0000
#define MAS6_SPID1	0x00007FFE
#define MAS6_SAS	0x00000001
#define MAS6_SPID	MAS6_SPID0

#define MAS7_RPN	0xFFFFFFFF

#define FSL_BOOKE_MAS0(tlbsel,esel,nv) \
		(MAS0_TLBSEL(tlbsel) | MAS0_ESEL(esel) | MAS0_NV(nv))
#define FSL_BOOKE_MAS1(v,iprot,tid,ts,tsize) \
		((((v) << 31) & MAS1_VALID)             |\
		(((iprot) << 30) & MAS1_IPROT)          |\
		(MAS1_TID(tid))				|\
		(((ts) << 12) & MAS1_TS)                |\
		(MAS1_TSIZE(tsize)))
#define FSL_BOOKE_MAS2(epn, wimge) \
		(((epn) & MAS3_RPN) | (wimge))
#define FSL_BOOKE_MAS3(rpn, user, perms) \
		(((rpn) & MAS3_RPN) | (user) | (perms))

#define BOOKE_PAGESZ_1K         0
#define BOOKE_PAGESZ_4K         1
#define BOOKE_PAGESZ_16K        2
#define BOOKE_PAGESZ_64K        3
#define BOOKE_PAGESZ_256K       4
#define BOOKE_PAGESZ_1M         5
#define BOOKE_PAGESZ_4M         6
#define BOOKE_PAGESZ_16M        7
#define BOOKE_PAGESZ_64M        8
#define BOOKE_PAGESZ_256M       9
#define BOOKE_PAGESZ_1G		    10
#define BOOKE_PAGESZ_4G		    11
#define BOOKE_PAGESZ_16GB	    12
#define BOOKE_PAGESZ_64GB	    13
#define BOOKE_PAGESZ_256GB	    14
#define BOOKE_PAGESZ_1TB	    15

ULONG  g_dwBootSize  = 0;
ULONG  g_dwRamSize  = 0;
ULONG  g_dwBbxPhyBase  = 0;
ULONG  g_dwBbxSize  = 0;
ULONG  g_dwBootParamPhyBase  = 0;
ULONG  g_dwBootParamSize  = 0;
ULONG  g_dwUsDpaaPhyBase  = 0;
ULONG  g_dwUsDpaaSize  = 0;
#define SHM_PAGE_OFFSET   0x10000000
#define SHM_BLOCK_NAME_MAX_LEN  20
static unsigned long g_dwShmHeapInitDone = 0;
typedef struct tagTGshmMallocParam
{
	unsigned long dwsize;
	unsigned long dwalign;
	unsigned long dwphyaddr;
	unsigned char aucName[SHM_BLOCK_NAME_MAX_LEN];
	unsigned long dwindex;
}TGshmMallocParam;


#define stringify(s)	tostring(s)
#define tostring(s)	#s

#define mfdcr(rn)	({unsigned int rval; \
			asm volatile("mfdcr %0," stringify(rn) \
				     : "=r" (rval)); rval;})
#define mtdcr(rn, v)	asm volatile("mtdcr " stringify(rn) ",%0" : : "r" (v))

#define mfmsr()		({unsigned int rval; \
			asm volatile("mfmsr %0" : "=r" (rval)); rval;})
#define mtmsr(v)	asm volatile("mtmsr %0" : : "r" (v))

#define mfspr(rn)	({unsigned int rval; \
			asm volatile("mfspr %0," stringify(rn) \
				     : "=r" (rval)); rval;})
#define mtspr(rn, v)	asm volatile("mtspr " stringify(rn) ",%0" : : "r" (v))

#define tlbie(v)	asm volatile("tlbie %0 \n sync" : : "r" (v))


#define USDPAA_IOC_MAGIC   'S'
#define USDPAA_CHRDEV_NAME "/dev/usdpaa"

#define	IOCPARM_MASK	    (ULONG)0x1fff
#define	SHMEM_IOC_OUT		(ULONG)0x40000000
#define	SHMEM_IOC_IN		(ULONG)0x80000000
#define	_IOC_SHMEM(inout,group,num,len) \
	(inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define	_IOR_SHMEM(g,n,t)	_IOC_SHMEM(SHMEM_IOC_OUT,	(g), (n), sizeof(t))
#define	_IOW_SHMEM(g,n,t)	_IOC_SHMEM(SHMEM_IOC_IN,	(g), (n), sizeof(t))

#define    USDPAA_IOC_GSHMBOOTSIZE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 1,  unsigned int)
#define    USDPAA_IOC_GSHMRAMSIZE             _IOR_SHMEM(USDPAA_IOC_MAGIC, 2,  unsigned int)
#define    USDPAA_IOC_WAITQMINT               _IOR_SHMEM(USDPAA_IOC_MAGIC, 3,  unsigned int)  
#define    USDPAA_IOC_BBX_BASE                _IOR_SHMEM(USDPAA_IOC_MAGIC, 4,  unsigned int)   
#define    USDPAA_IOC_BBX_SIZE                _IOR_SHMEM(USDPAA_IOC_MAGIC, 5,  unsigned int)  
#define    USDPAA_IOC_BOOTPARAM_BASE          _IOR_SHMEM(USDPAA_IOC_MAGIC, 6,  unsigned int)   
#define    USDPAA_IOC_BOOTPARAM_SIZE          _IOR_SHMEM(USDPAA_IOC_MAGIC, 7,  unsigned int)  
#define    USDPAA_IOC_GSHM_LEFT_SIZE          _IOR_SHMEM(USDPAA_IOC_MAGIC, 8,  unsigned int) 
#define    USDPAA_IOC_USDPAA_BASE             _IOR_SHMEM(USDPAA_IOC_MAGIC, 9,  unsigned int)   
#define    USDPAA_IOC_USDPAA_SIZE             _IOR_SHMEM(USDPAA_IOC_MAGIC, 10, unsigned int)  
#define    USDPAA_IOC_GSHM_MALLOC             _IOR_SHMEM(USDPAA_IOC_MAGIC, 11, unsigned int) 
#define    USDPAA_IOC_GSHM_INITSETOK          _IOR_SHMEM(USDPAA_IOC_MAGIC, 12, unsigned int)   
#define    USDPAA_IOC_GSHM_INIT_STATEGET      _IOR_SHMEM(USDPAA_IOC_MAGIC, 13, unsigned int)
#define    USDPAA_IOC_GSHM_SHOW               _IOR_SHMEM(USDPAA_IOC_MAGIC, 14, unsigned int)  
#define    USDPAA_IOC_GSHM_FREE               _IOR_SHMEM(USDPAA_IOC_MAGIC, 15, unsigned int)  
#define    USDPAA_IOC_WAITQM9INT              _IOR_SHMEM(USDPAA_IOC_MAGIC, 16, unsigned int)

#define    USDPAA_IOC_GET_ADDR                _IOR_SHMEM(USDPAA_IOC_MAGIC, 17,unsigned int) 

#endif

