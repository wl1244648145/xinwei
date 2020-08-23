#include "../inc/bspbman.h"
#include "../inc/TBuf.h"
#include "../inc/compat.h"
#include "../inc/bpid.h"
#include "../inc/fsl_shmem.h"
#include "../inc/fsl_bman.h"

 
#include "../inc/bsp.h"
#define BspBmanPrintf   printf
 

/***********************************************************
 *                 文件内部使用的宏                        *
***********************************************************/
#define BM_MAX_POOLGRP_COUNT         4
#define BM_MAX_POOLS_COUNT_PERGRP         10
#define BM_BUFSIZE_DIF_BITWISE     10
#define BM_BUFSIZE_MIN     256
#define BM_BUFSIZE_MAX     (BM_BUFSIZE_MIN + (BM_MAX_POOLS_COUNT_PERGRP - 1) * (1<<BM_BUFSIZE_DIF_BITWISE))


/***********************************************************
 *               文件内部使用的数据类型                    *
***********************************************************/


/***********************************************************
 *                     全局变量                            *
***********************************************************/
unsigned long g_dwbspbmanFlag = 0;
unsigned long dwBmanPhyBase = 0;
unsigned char *puBmancvirtbase = NULL;
unsigned char *gpuBmancvirtbase = NULL;
int g_bufslo = 0;

TBmPoolCfg g_tBmPoolCfg[] = {\
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 0, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 0}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 1, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 1}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 2, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 2}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 3, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 3}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 4, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 4}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 5, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 5}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 6, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 6}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 7, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 7}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 8, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 8}, \
	{.bpid=BMAN_BPID_TBUF_MEM_TYPE0_BASE+ 9, .count=0x100, .size=BM_BUFSIZE_MIN + (1<<BM_BUFSIZE_DIF_BITWISE) * 9},
};


TBmPoolGrpCtl gatBmPoolGrpCtl[] = {\
	    [tbuf_mem_type0] = {.bpid_base = BMAN_BPID_TBUF_MEM_TYPE0_BASE, .bufsizemin = BM_BUFSIZE_MIN, .bufsizemax = BM_BUFSIZE_MAX, .initflag = 0}, \
	    [tbuf_mem_type1] = {.bpid_base = BMAN_BPID_TBUF_MEM_TYPE1_BASE, .bufsizemin = BM_BUFSIZE_MIN, .bufsizemax = BM_BUFSIZE_MAX, .initflag = 0}, \
	    [tbuf_mem_type2] = {.bpid_base = BMAN_BPID_TBUF_MEM_TYPE2_BASE, .bufsizemin = BM_BUFSIZE_MIN, .bufsizemax = BM_BUFSIZE_MAX, .initflag = 0}, \
	    [tbuf_mem_type3] = {.bpid_base = BMAN_BPID_TBUF_MEM_TYPE3_BASE, .bufsizemin = BM_BUFSIZE_MIN, .bufsizemax = BM_BUFSIZE_MAX, .initflag = 0}, 
    };

TBmPoolCtlInfo  gatBmPoolCtlInfo[BM_MAX_POOLGRP_COUNT][BM_MAX_POOLS_COUNT_PERGRP]; 

unsigned int gatBmPoolGrpCount = (sizeof(gatBmPoolGrpCtl)/sizeof(TBmPoolGrpCtl));


TBmMoniCount g_atBmMoniCount[MAX_BMAN_POOL_NUM];

extern struct bman_portal *gaptBmanPortal[10];

unsigned int  BspBmGetCounter(e_BmInterModuleCounters counter, unsigned int bpid)
{
    if(CCSR_VIRTADDR_BASE == 0)
    {
        BspBmanPrintf("CCSR_VIRTADDR_BASE = 0, is not permitted\n");
    	return 0;
    }
    switch(counter)
    {
        case(e_BM_IM_COUNTERS_POOL_CONTENT):
            return (*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET + BMAN_POOL0_CONTENT_OFFSET + (bpid <<2))));
        case(e_BM_IM_COUNTERS_POOL_SW_DEPLETION):
            return (*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET + BMAN_POOL0_SDCNT_OFFSET + (bpid <<2))));
        case(e_BM_IM_COUNTERS_POOL_HW_DEPLETION):
            return (*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET + BMAN_POOL0_HDCNT_OFFSET + (bpid <<2))));
        case(e_BM_IM_COUNTERS_FBPR):
            return (*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET + FBPR_FPC)));
        default:
            break;
    }
    return 0;
}


//CCSR_VIRTADDR_BASE
/* 大小为2的exp次方 */
void bm_set_memory(u16 eba, u32 ba, int prio, u32 exp)
{
	*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET +FBPR_BARE )) = eba;
	*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET +FBPR_BAR )) = ba;
	*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET +FBPR_AR )) = (prio ? 0x40000000 : 0) | (exp - 1);
}

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
#define BOOKE_PAGESZ_1G		     10
#define BOOKE_PAGESZ_4G		11
#define BOOKE_PAGESZ_16GB	12
#define BOOKE_PAGESZ_64GB	13
#define BOOKE_PAGESZ_256GB	14
#define BOOKE_PAGESZ_1TB	15

/******************************************************************************
* 函数名: BspSetTlb1
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
*作者:刘刚
******************************************************************************/
void BspSetTlb1(int TlbSel,     unsigned long ulVirAddrStartL,    unsigned long ulPhyAddrStartL,    unsigned short usPageSize)
{
	volatile unsigned long ulMas0, ulMas1, ulMas2, ulMas3;
	int i =0;
	ulMas0=FSL_BOOKE_MAS0(1, TlbSel, 0);

	mtspr(MAS0, ulMas0);
	asm volatile("isync; tlbre; isync; msync" : : : "memory");
	if((mfspr(MAS1)) & 0x80000000)
	{
		for(i = 0; i < 64; i++)
		{
			ulMas0=FSL_BOOKE_MAS0(1, i, 0);
			
			mtspr(MAS0, ulMas0);
			asm volatile("isync; tlbre; isync; msync" : : : "memory");
			if((mfspr(MAS1)) & 0x80000000)
				continue;
			else
				break;
		}
		
		if(i >= 64)
		{
			BspDpaaPrintf("error, in file:%s, on line:%d\n", __FILE__, __LINE__);
			return;
		}
	}

	ulMas1=FSL_BOOKE_MAS1(1, 1, 0, 0, usPageSize);
	ulMas2=FSL_BOOKE_MAS2(ulVirAddrStartL, 0);
	ulMas3=FSL_BOOKE_MAS3(ulPhyAddrStartL, 0, MAS3_SX|MAS3_SW|MAS3_SR);
	mtspr(MAS0, ulMas0);
	mtspr(MAS1, ulMas1);
	mtspr(MAS2, ulMas2);
	mtspr(MAS3, ulMas3);
	mtspr(MAS7, 0);
	mtspr(MAS8, 0);
	asm volatile("isync; tlbwe; isync; msync" : : : "memory");
	
}

/******************************************************************************
* 函数名: BspUnCinvSetTlb1
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
*作者:刘刚
******************************************************************************/
void BspUnCinvSetTlb1(int TlbSel,     unsigned long ulVirAddrStartL,    unsigned long ulPhyAddrStartL,    unsigned short usPageSize)
{
	unsigned long ulMas0, ulMas1, ulMas2, ulMas3;
	int i;
	ulMas0=FSL_BOOKE_MAS0(1, TlbSel, 0);
	
	mtspr(MAS0, ulMas0);
	
	asm volatile("isync; tlbre; isync; msync" : : : "memory");
	if((mfspr(MAS1)) & 0x80000000)
	{
		for(i = 0; i < 64; i++)
		{
			ulMas0=FSL_BOOKE_MAS0(1, TlbSel, 0);
			mtspr(MAS0, ulMas0);
			asm volatile("isync; tlbre; isync; msync" : : : "memory");
			if((mfspr(MAS1)) & 0x80000000)
				continue;
			else
				break;
		}
		if(i >= 64)
		{
			BspDpaaPrintf("error, in file:%s, on line:%d\n", __FILE__, __LINE__);
		}
	}

	ulMas1=FSL_BOOKE_MAS1(1, 1, 0, 0, usPageSize);
	ulMas2=FSL_BOOKE_MAS2(ulVirAddrStartL, 0xa);
	ulMas3=FSL_BOOKE_MAS3(ulPhyAddrStartL, 0, MAS3_SX|MAS3_SW|MAS3_SR);
	mtspr(MAS0, ulMas0);
	mtspr(MAS1, ulMas1);
	mtspr(MAS2, ulMas2);
	mtspr(MAS3, ulMas3);
	mtspr(MAS7, 0);
	mtspr(MAS8, 0);
	asm volatile("isync; tlbwe; isync; msync" : : : "memory");
	
}

/******************************************************************************
* 函数名: size2bpid
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
*作者:刘刚
******************************************************************************/
void BspUnCinvSetTlb0(int TlbSel,     unsigned long ulVirAddrStartL,    unsigned long ulPhyAddrStartL,    unsigned short usPageSize)
{
	unsigned long ulMas0, ulMas1, ulMas2, ulMas3;
	int i;
	ulMas0=FSL_BOOKE_MAS0(0, TlbSel, 0);

	mtspr(MAS0, ulMas0);
	
	asm volatile("isync; tlbre; isync; msync" : : : "memory");
	if((mfspr(MAS1)) & 0x80000000)
	{
		for(i = 0; i < 512; i++)
		{
			ulMas0=FSL_BOOKE_MAS0(0, TlbSel, 0);
			mtspr(MAS0, ulMas0);
			asm volatile("isync; tlbre; isync; msync" : : : "memory");
			if((mfspr(MAS1)) & 0x80000000)
				continue;
			else
				break;
		}
		if(i >= 512)
		{
			BspDpaaPrintf("error, in file:%s, on line:%d\n", __FILE__, __LINE__);
		}
	}

	ulMas1=FSL_BOOKE_MAS1(1, 1, 0, 0, usPageSize);
	ulMas2=FSL_BOOKE_MAS2(ulVirAddrStartL, 0xa);
	ulMas3=FSL_BOOKE_MAS3(ulPhyAddrStartL, 0, MAS3_SX|MAS3_SW|MAS3_SR);
	mtspr(MAS0, ulMas0);
	mtspr(MAS1, ulMas1);
	mtspr(MAS2, ulMas2);
	mtspr(MAS3, ulMas3);
	mtspr(MAS7, 0);
	mtspr(MAS8, 0);
	asm volatile("isync; tlbwe; isync; msync" : : : "memory");
	
}

/******************************************************************************
* 函数名: size2bpid
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
*作者:刘刚
******************************************************************************/
inline int size2bpid(int memtype, unsigned long size )
{
    int i;
	printf("gatBmPoolGrpCtl[memtype].initflag->0x%x\n",gatBmPoolGrpCtl[memtype].initflag);
	printf("gatBmPoolGrpCtl[memtype].poolcount->0x%x\n",gatBmPoolGrpCtl[memtype].poolcount);
	//if((memtype >= BM_MAX_POOLGRP_COUNT) || (!gatBmPoolGrpCtl[memtype].initflag))
	//{
	//	BspBmanPrintf("%s failed, in file:%s, on line:%d, memtype = %d, size = %ld\n", __FUNCTION__, __FILE__, __LINE__, memtype, size);
	//	return -1;
	//}
	for(i = 0; i < (gatBmPoolGrpCtl[memtype].poolcount); i++)
	{
		if(size <= (gatBmPoolCtlInfo[memtype][i].size))
		return (BMAN_BPID_TBUF_MEM_TYPE0_BASE + BM_MAX_POOLS_COUNT_PERGRP * memtype + i);
	}
	BspBmanPrintf("%s failed, in file:%s, on line:%d, memtype = %d, size = %ld\n", __FUNCTION__, __FILE__, __LINE__, memtype, size);
	return -1;
	
}

/******************************************************************************
* 函数名: BspRetTbuf
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
*作者:刘刚
******************************************************************************/

//extern unsigned char *puBmancvirtbase;
TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long dwline, const char *pucFuncName)
{
	unsigned long bpid;
	struct bm_buffer bufs;
	TBuf  *ptBuf = NULL;
	int ret;
	int portalnum;
    printf("gpuBmancvirtbase->0x%08x\n",(unsigned int)gpuBmancvirtbase);
	if((dwMemType >= gatBmPoolGrpCount))
	{
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		return NULL;
	}
	bpid = size2bpid(dwMemType, dwsize);
    printf("bpid->0x%lx\n",bpid);
	portalnum = USDPAA_BMAN_PORTAL_NUM;

	ret = BspBmanAcquire(bpid, &bufs,1, gaptBmanPortal[portalnum]);
	g_bufslo = bufs.lo;
	printf("g_bufslo->0x%x\n",(unsigned int)g_bufslo);
//	printf("ret->0x%lx\n",ret);
	if(ret != 1)
	{
		g_atBmMoniCount[bpid].dwGetfailed++;
		BspBmanPrintf("%s failed, in file:%s, on line:%d, ret = %d, bpid = %d\n", __FUNCTION__, __FILE__, __LINE__, ret, (int)bpid);
		return NULL;
	}
	g_atBmMoniCount[bpid].dwGetSuccess++;
    ptBuf = (TBuf *)(gpuBmancvirtbase +(bufs.lo -dwBmanPhyBase));
	ptBuf->dwDataSize = 0;
	ptBuf->pucData = NULL;
	ptBuf->wBpid = bpid;
	ptBuf->wMemType = tbuf_mem_type0;
	   
    ptBuf->pucData  = gpuBmancvirtbase + (bufs.lo -dwBmanPhyBase)+ UNIHEAD_SIZE_128BYTES;//mmap((void *)0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,(bufs.lo + BMAN_BUF_PRERSV_SIZE));
 
	//ptBuf->pucData = __shmem_ptov((bufs.lo)) + BMAN_BUF_PRERSV_SIZE;
	ptBuf->pucEnd = ptBuf->pucData + dwsize;
	ptBuf->wBpid = bpid;
	//ptBuf->pucBufStart = ptBuf - UNIHEAD_SIZE_128BYTES;
    printf("ptBuf->pucData->0x%p\n",ptBuf->pucData);
	printf("ptBuf->pucEnd->0x%p\n",ptBuf->pucEnd);
	*(ptBuf->pucEnd)  = BMAN_TBUF_CHECKBYTE;


#ifdef BMAN_POOL_TRACE_ON
	//strncpy(ptBufCtl->aucFuncName, pucFuncName, (MAX_BMAN_FUNCTION_NAME <= strlen(pucFuncName) ? : MAX_BMAN_FUNCTION_NAME, strlen(pucFuncName)));
	//ptBufCtl->aucFuncName[MAX_BMAN_FUNCTION_NAME] = 0;
#endif
  // printf("33333333333333333333\n");         
	return ptBuf;
}

/******************************************************************************
* 函数名: BspRetTbuf
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspRetTbuf(TBuf  *ptBuf)
{
	TBufCtl *ptBufCtl = NULL;
	struct bm_buffer bufs;
	int ret;
	int portalnum;
	if((ptBuf == NULL)  || ((ptBuf->pucEnd) == NULL)  || (BMAN_TBUF_CHECKBYTE != (*(ptBuf->pucEnd))))
	{
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}
	//ptBufCtl = (TBufCtl *)((unsigned long)ptBuf - sizeof(TBufCtl));
	
		
	/*  buf 填充  */
	//printf("BspRetTbuf->ptBuf->wBpid->0x%lx\n",ptBuf->wBpid);
	bufs.bpid = ptBuf->wBpid;
	bufs.hi = 0;
	bufs.lo = (u32)g_bufslo;
 
	portalnum = USDPAA_BMAN_PORTAL_NUM;
 
	//ptBufCtl->wUse = 0;
	ret = BspBmanRelease(ptBuf->wBpid, &bufs, 1, 0, gaptBmanPortal[portalnum]);
	if(ret != 0)
	{
		g_atBmMoniCount[ptBufCtl->bpid].dwRetfailed++;
		BspBmanPrintf("%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, ret);
		return -1;
	}
	//g_atBmMoniCount[ptBufCtl->bpid].dwRetSuccess++;
	
	return 0;
}

static struct bman_pool *gpatbmpool[64] = {NULL};
//extern unsigned long dwBmanPhyBase;
/******************************************************************************
* 函数名: BspBmanBufPoolInit
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspBmanBufPoolInit(unsigned long  bpid, unsigned long bufcfgsize, unsigned long bufcfgcount,  enum tbuf_mem_type memtype, unsigned char *pucvirtbase)
{
	int i = 0;
	int ret;
	unsigned long phybase;
//	unsigned long totalsize;
	unsigned long bufsize;
	int portalnum = USDPAA_BMAN_PORTAL_NUM;

	TBuf  *ptTBuf = NULL;
	struct bm_buffer bufs;
	
	struct bman_pool_params params = {
		.bpid	= bpid,
		.flags	= BMAN_POOL_FLAG_DEPLETION,
		.cb	= NULL,
		.cb_ctx	= gpatbmpool + bpid
	};

 
	portalnum = USDPAA_BMAN_PORTAL_NUM;
 
      
	if((bpid > 63) || (bpid == 0) || (NULL == gaptBmanPortal[portalnum]) || (pucvirtbase == NULL))
	{
		g_dwbspbmanFlag = __LINE__;
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	
	}

	if(NULL != gpatbmpool[bpid])
	{
		g_dwbspbmanFlag = __LINE__;
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}
    
	gpatbmpool[bpid] = bman_new_pool(&params, gaptBmanPortal[portalnum]);
	g_dwbspbmanFlag = __LINE__;

	if(NULL== gpatbmpool[bpid])
	{
		g_dwbspbmanFlag = __LINE__;
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

      /* 每个buf占用的总长度 */
	bufsize  = BMAN_BUF_TOTAL_LEN(bufcfgsize);
	g_dwbspbmanFlag = __LINE__;
	if(NULL == pucvirtbase)
	{
		g_dwbspbmanFlag = __LINE__;
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	
	}
      
	phybase = (u32)dwBmanPhyBase;//__shmem_vtop(pucvirtbase);
	for(i =  0; i < bufcfgcount; i++)
	{
		ptTBuf = (TBuf*)(pucvirtbase + i * bufsize);
		
		/*  TBuf 头的初始化*/
		ptTBuf->dwDataSize = 0;
		ptTBuf->pucData = NULL;
		ptTBuf->pucEnd = pucvirtbase + i * bufsize + bufsize - 1;
		ptTBuf->wBpid = bpid;
		ptTBuf->wMemType = memtype;
		
		
		/*  buf 填充  */
		bufs.bpid = bpid;
		bufs.hi = 0;
		bufs.lo = phybase + i * (bufsize)  + UNIHEAD_SIZE_128BYTES;
		//printf("hello,,,bufs.bpid->0x%lx,bufs.lo->0x%lx\n",bufs.bpid,bufs.lo);
		g_dwbspbmanFlag = __LINE__;
		ret = BspBmanRelease(bpid, &bufs, 1, 0, gaptBmanPortal[portalnum]);
		g_dwbspbmanFlag = __LINE__;
		if(ret != 0)
		{
			g_dwbspbmanFlag = __LINE__;
			BspBmanPrintf("BspBmanPoolInit failed, in file:%s, on line:%d, ret = %d, bpid = %d\n", __FILE__, __LINE__, ret, (int)bpid);
			return -1;
		}
	}
	return 0;

}

int BspBmanPoolInit(unsigned long  bpid, TBmPoolCfg *ptBmpoolCfg, enum tbuf_mem_type memtype, unsigned char *pucvirtbase)
{
	int ret;

	if((NULL == ptBmpoolCfg) || (bpid > 63) || (bpid == 0) /* || (bpid != (ptBmpoolCfg->bpid))*/)
	{
		g_dwbspbmanFlag = __LINE__;
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	
	}
	g_dwbspbmanFlag = __LINE__;
	//printf("bpid:0x%lx, ptBmpoolCfg->size:0x%lx, ptBmpoolCfg->count:0x%lx, memtype:0x%lx, pucvirtbase:0x%lx\n",bpid, ptBmpoolCfg->size, ptBmpoolCfg->count, memtype, pucvirtbase);
	ret = BspBmanBufPoolInit(bpid, ptBmpoolCfg->size, ptBmpoolCfg->count, memtype, pucvirtbase);

	if(ret != 0)
	{
		g_dwbspbmanFlag = __LINE__;
		BspBmanPrintf("BspBmanBufPoolInit failed, in file:%s, on line:%d, ret = %d\n", __FILE__, __LINE__, ret);
		return -1;
	}

	return 0;

}
unsigned long  BspBmanPoolGrpCalcTotalSize(TBmPoolCfg *ptBmpoolCfg, unsigned long poolcount, enum tbuf_mem_type memtype, unsigned long dwCoreMask)
{
	int i;
	unsigned long dwtotalsize = 0;
	if((NULL == ptBmpoolCfg) || (poolcount > BM_MAX_POOLS_COUNT_PERGRP))
	{
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		g_dwbspbmanFlag = __LINE__;
		return -1;
	
	}
	for(i = 0; i < poolcount; i ++)
	{
		dwtotalsize = dwtotalsize + BMAN_POOL_CALC_TOTAL_SIZE(ptBmpoolCfg[i].size, ptBmpoolCfg[i].count);
	}
	return dwtotalsize;
}


int BspBmanPoolGrpInit(TBmPoolCfg *ptBmpoolCfg, unsigned long poolcount, enum tbuf_mem_type memtype, unsigned long dwCoreMask)
{
	int i;
	unsigned long bpid;
	int ret;

	unsigned long dwtotalsize = 0;

	u32 fd;
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (-1 == fd)
    {
		    printf("can not open mem!\n");
		    return -1;
    }
	
	if((NULL == ptBmpoolCfg) || (poolcount > BM_MAX_POOLS_COUNT_PERGRP) || (memtype >= BM_MAX_POOLGRP_COUNT))
	{
		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
		g_dwbspbmanFlag = __LINE__;
		return -1;
	}
	

	dwtotalsize = BspBmanPoolGrpCalcTotalSize(ptBmpoolCfg, poolcount, memtype, dwCoreMask);
	dwtotalsize = BspAlignSize(0x100000, dwtotalsize);
	dwBmanPhyBase  =  BspShmemPhyMalloc(0x100000, dwtotalsize, "bmpoolgrp", memtype);
	if (0 == dwBmanPhyBase)
	{
		BspBmanPrintf("%s failed, in file:%s, on line:%d, maybe mem is not enough\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	} 
    	
    puBmancvirtbase = mmap((void *)0,dwtotalsize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,dwBmanPhyBase);
    gpuBmancvirtbase = puBmancvirtbase;

	
	ret = mprotect((void *)(puBmancvirtbase), dwtotalsize, PROT_READ | PROT_WRITE);
	if (0 != ret) {
		BspBmanPrintf("can't mprotect() shmem device");
		return -1;
	}


	for(i = 0; i < poolcount; i++)
	{
		bpid = BMAN_BPID_TBUF_MEM_TYPE0_BASE + BM_MAX_POOLS_COUNT_PERGRP * memtype + i;   /*(ptBmpoolCfg + i)->bpid;  */
		
		g_dwbspbmanFlag = __LINE__;
		ret = BspBmanPoolInit(bpid, ptBmpoolCfg + i, memtype, puBmancvirtbase);
		puBmancvirtbase = puBmancvirtbase + BMAN_POOL_CALC_TOTAL_SIZE(ptBmpoolCfg[i].size, ptBmpoolCfg[i].count);
		g_dwbspbmanFlag = __LINE__;
		if(0 != ret)
		{
			g_dwbspbmanFlag = __LINE__;
			BspBmanPrintf("%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, ret);
			return ret;
		}
		memcpy(&(gatBmPoolCtlInfo[memtype][i]), (ptBmpoolCfg + i), sizeof(TBmPoolCfg));
		gatBmPoolCtlInfo[memtype][i].bpid = bpid;
		gatBmPoolCtlInfo[memtype][i].initflag = 1;
		g_dwbspbmanFlag = __LINE__;
		//BspBmanPrintf("there is %d bufs in buf pool %d\n", BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, bpid), bpid);
	}
	gatBmPoolGrpCtl[memtype].poolcount = poolcount;
	gatBmPoolGrpCtl[memtype].initflag = 1;
	return 0;
}
#define TBUF_MEM_TYPE4  4  /* IP报文组包用的buf pool */
#define MAX_USER_PLANE_IPSIZE  2048
int BspIpReasembleBufPoolInit(unsigned int bufcount)
{
	int ret;
	unsigned long dwtotalsize;
	unsigned long dwPhyBase;
	unsigned char *pucvirtbase;


	dwtotalsize = BMAN_POOL_CALC_TOTAL_SIZE(MAX_USER_PLANE_IPSIZE, bufcount);
	dwtotalsize = BspAlignSize(0x100000, dwtotalsize);
	

	pucvirtbase  =  BspShmemVirtMalloc(0x100000, dwtotalsize, "IpReasemble", 0,(int *)&pucvirtbase);

	dwPhyBase = __shmem_vtop(pucvirtbase);
	dwPhyBase = dwPhyBase;
	ret = mprotect((void *)(pucvirtbase), BspAlignSize(0x100000, dwtotalsize), PROT_READ | PROT_WRITE);
	if (0 != ret) {
		BspBmanPrintf("can't mprotect() shmem device");
		return -1;
	}


	ret = BspBmanBufPoolInit(BMAN_BPID_IP_REASSEMBLE, MAX_USER_PLANE_IPSIZE, bufcount, TBUF_MEM_TYPE4, pucvirtbase);
	if(ret != 0)
	{
		BspBmanPrintf("BspBmanBufPoolInit failed, in file:%s, on line:%d, ret = %d\n", __FILE__, __LINE__, ret);
		return -1;
	}
        	
	return 0;
}
/******************************************************************************
* 函数名: BspDbgShowGtpuPktStat
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspBmanInitExample()
{
    int ret;

    g_dwbspbmanFlag = __LINE__;
    ret = BspBmanPoolGrpInit(g_tBmPoolCfg, sizeof(g_tBmPoolCfg)/sizeof(TBmPoolCfg), tbuf_mem_type0, 0xff);
    if(0 != ret)
    {
    	g_dwbspbmanFlag = __LINE__;
    	BspBmanPrintf("%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, ret);
    	return ret;
    }    

    return ret;
}
void BspTBuffInit(void *pBuff, int bpid, unsigned int buffsize)
{
    TBufCtl *ptBufCtl;
    TBuf *ptTBuf;
    

    ptTBuf = ((TBuf*)((unsigned int)pBuff - UNIHEAD_SIZE_128BYTES));
    ptBufCtl = ((TBufCtl*)ptTBuf - 1); 

    /*  TBufCtl 头的初始化*/
    ptBufCtl ->bpid = bpid;
    ptBufCtl->bufSize = buffsize + BMAN_BUF_PRERSV_SIZE + BMAN_BUF_POSTRSV_SIZE;
    ptBufCtl->checkword = BMAN_TBUFCTL_CHECKWORD;
    ptBufCtl->dwUsedSize = 0;
    ptBufCtl->wUse = 0;
    ptBufCtl->bufPa = (unsigned long)pBuff;
    
    /*  TBuf 头的初始化*/
    ptTBuf->dwDataSize = 0;
    //ptTBuf->pucBufStart = pBuff;
    ptTBuf->pucData = NULL;
    ptTBuf->pucEnd = (unsigned char*)((unsigned)pBuff + buffsize - 1);
    ptTBuf->wBpid = bpid;
    ptTBuf->wMemType = tbuf_mem_type0;
}

