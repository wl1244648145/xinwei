 

#ifndef _BSP_SHMEM_H
#define _BSP_SHMEM_H


/***********************************************************
 *                    其它条件编译选项                     *
***********************************************************/
/***********************************************************
 *                   标准、非标准头文件                    *
***********************************************************/
/***********************************************************
 *                        常量定义                         *
***********************************************************/

/***********************************************************
 *                       全局宏                            *
***********************************************************/
#define MAX_CORE_NUM   4

#define SHM_PAGE_OFFSET   0x10000000

/*全局index宏定义，BSP使用0~0x2000000的范围，OSS使用0x2000000以上
的范围，OSS的宏定义在OSS_ShareMem.h头文件中*/
#define P_BSP_BEGIN   0
#define P_BSP_END   0x7FFF
#define P_OSS_BEGIN   0x8000
#define P_OSS_END   0xFFFF

unsigned long BspAlignSize(unsigned long  align, unsigned long  size);

inline static void *BspShmP2V(unsigned long p)
{
	return ((void *)((void *)(p) + (unsigned int)SHM_PAGE_OFFSET));
}

inline static unsigned long BspShmV2P(void *v)
{
	return (((unsigned int)(v) - (unsigned int)SHM_PAGE_OFFSET));
}

inline static void *BspP2V(unsigned long p)
{
	return BspShmP2V(p);
}

inline static unsigned long BspV2P(void *v)
{
	return BspShmV2P(v);
}

#define BspShmPrintf   printf
#define SHM_BLOCK_NAME_MAX_LEN  20

typedef struct tagTGshmMallocParam
{
	unsigned long dwsize;
	unsigned long dwalign;
	unsigned long dwphyaddr;
	unsigned char aucName[SHM_BLOCK_NAME_MAX_LEN];
	unsigned long dwindex;
}TGshmMallocParam;


unsigned long BspGetBootSize();
unsigned long BspGetRamSize();
unsigned long BspGetBbxPhyBase();
unsigned long BspGetBbxSize();

unsigned long BspGetBootParamPhyBase();
unsigned long BspGetBootParamSize();
unsigned long BspGetUsDpaPhyBase();
unsigned long BspGetUsDpaSize();

int BspSetSlaveCoreCfgBbx(unsigned long dwcoreid, unsigned long phybase, unsigned long virtbase, unsigned long size);
int BspSetSlaveCoreCfgFdt(unsigned long dwcoreid, unsigned long phybase, unsigned long virtbase, unsigned long size);
int BspSetSlaveCoreCfgGdb(unsigned long dwcoreid, unsigned long phybase, unsigned long virtbase, unsigned long size);
int BspSetSlaveCoreCfgIp(unsigned long dwcoreid, unsigned char *pauc);
int BspSetSlaveCoreCfgMac(unsigned long dwcoreid, unsigned char *pauc);
int BspSetSlaveCoreCfgIpc(unsigned long dwcoreid, unsigned long phybase, unsigned long virtbase, unsigned long size);
int BspSetSlaveCoreCfgShmMan(unsigned long dwcoreid, unsigned long phybase, unsigned long virtbase, unsigned long size);

unsigned long BspShmemPhyMalloc(unsigned long  align, unsigned long  size, const char *name, unsigned long index);
void  *BspShmemVirtMalloc(unsigned long  align, unsigned long  size, const char *name, unsigned long index,int *pdwphyaddr);

int bsp_sharemem_init();
/* 高16bit */

#define  UX         0x00000020
#define  SX         0x00000010
#define  UW         0x00000008
#define  SW         0x00000004
#define  UR         0x00000002
#define  SR         0x00000001

/* 低16bit */
#define WRITE_THROUGH                   0x00000010
#define CACHE_INHIBITED                 0x00000008
#define MEMORY_COHERENCE_REGUIRED       0x00000004
#define GUARDED                         0x00000002
#define ENDIAN                          0x00000001

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
#define BOOKE_PAGESZ_1G		10
#define BOOKE_PAGESZ_4G		11
#define BOOKE_PAGESZ_16GB	12
#define BOOKE_PAGESZ_64GB	13
#define BOOKE_PAGESZ_256GB	14
#define BOOKE_PAGESZ_1TB	15

 

#endif /* _BSP_SHMEM_H */

