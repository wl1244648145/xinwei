/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_shmem.c
* 功能:
* 版本:
* 编制日期:
* 作者:
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/

/**************************** 私用头文件* **********************************/
#include "../../../com_inc/bsp_types.h"
#include "../inc/bsp_shmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include "../../../com_inc/bsp_shmem_ext.h"

/******************************************************************************
* 函数名: BspShmP2V
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void *BspShmP2V(unsigned long p,unsigned long size)
{
    u32 fd;
    u8 *g_u8tmp1 = 0;
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (BSP_ERROR == fd)
    {
        bsp_dbg("can not open mem!\n");
        return NULL;
    }
//	g_u8tmp1 = mmap((void *)0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,(void *)p);
    g_u8tmp1 = mmap((void *)0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,(off_t)(void *)p);
    close(fd);
    return (void *)g_u8tmp1;//((void *)((void *)(p) + (unsigned int)SHM_PAGE_OFFSET));
}
/******************************************************************************
* 函数名: BspShmV2P
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspShmV2P(void *v)
{
    return (((unsigned int)(v) - (unsigned int)SHM_PAGE_OFFSET));
    //return virt_to_phys(v);
}

/******************************************************************************
* 函数名: BspGetUsDpaSize
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspGetUsDpaSize()
{
    return g_dwUsDpaaSize;
}

/******************************************************************************
* 函数名: BspGetBootSize
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspAlignSize(unsigned long  align, unsigned long  size)
{
    if(size & (align - 1))
    {
        return ((size + align)  & (~(align - 1)));
    }
    else
        return size;
}

/******************************************************************************
* 函数名: BspGetBootSize
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspGetBootSize()
{
    return g_dwBootSize;
}

/******************************************************************************
* 函数名: BspGetRamSize
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspGetRamSize()
{
    return g_dwRamSize;
}

/******************************************************************************
* 函数名: BspGetBbxPhyBase
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspGetBbxPhyBase()
{
    return g_dwBbxPhyBase;
}

/******************************************************************************
* 函数名: BspGetBbxSize
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspGetBbxSize()
{
    return g_dwBbxSize;
}
/******************************************************************************
* 函数名: BspGetBootParamPhyBase
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspGetBootParamPhyBase()
{
    return g_dwBootParamPhyBase;
}
/******************************************************************************
* 函数名: BspGetBootParamSize
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspGetBootParamSize()
{
    return g_dwBootParamSize;
}

/******************************************************************************
* 函数名: BspGetUsDpaPhyBase
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspGetUsDpaPhyBase()
{
    return g_dwUsDpaaPhyBase;
}

/******************************************************************************
* 函数名: BspShmemPhyMalloc
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long BspShmemPhyMalloc(unsigned long  align, unsigned long  size, const char *name, unsigned long index)
{
    //unsigned long phyaddr;
    TGshmMallocParam tGshmMallocParam;
    int fd_usdpaa;
    int ret = 0;

    if(g_dwShmHeapInitDone == 0)
    {
        printf("%s failed, because shmem is not inited\n", __FUNCTION__);
        return 0;
    }

    if((SHM_BLOCK_NAME_MAX_LEN <= strlen(name)) || (strlen(name) <=0))
    {
        printf("%s failed, %s is too long\n", __FUNCTION__, name);
        return 0;
    }

    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return 0;
    }

    tGshmMallocParam.dwalign = align;
    tGshmMallocParam.dwsize = size;
    strncpy((char *)tGshmMallocParam.aucName, name, SHM_BLOCK_NAME_MAX_LEN);
    tGshmMallocParam.dwindex = index;
    ret = ioctl(fd_usdpaa, USDPAA_IOC_GSHM_MALLOC, &tGshmMallocParam);
    //printf("===================USDPAA_IOC_GSHM_MALLOC->0x%lx\n================\n",USDPAA_IOC_GSHM_MALLOC);
    if (ret < 0)
    {
        fprintf(stderr, "@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return 0;
    }
    close(fd_usdpaa);
    return tGshmMallocParam.dwphyaddr;
}
extern  unsigned int dwtmpphyaddr;

/******************************************************************************
* 函数名: BspShmemVirtMalloc
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void  *BspShmemVirtMalloc(unsigned long  align, unsigned long  size, const char *name, unsigned long index,int *pdwphyaddr)
{
    unsigned long phyaddr;
    int ret;
    unsigned char *pucvirtbase;

    phyaddr =  BspShmemPhyMalloc(align, BspAlignSize(0x100000, size), name, index);
    //printf("phyaddr -->0x%08x\n",phyaddr);
    *pdwphyaddr = phyaddr;
    //printf("dwtmpphyaddr -->0x%08x\n",dwtmpphyaddr);
    if(0 == phyaddr)
    {
        printf("%s failed, size = 0x%x, name = %s\n", __FUNCTION__,(unsigned int)size, name);
        return NULL;
    }

    pucvirtbase = BspShmP2V(phyaddr,size);
    //printf("phyaddr->0x%lx,pucvirtbase -->0x%lx\n",phyaddr,(unsigned long)pucvirtbase);

    ret = mprotect((void *)(pucvirtbase), BspAlignSize(0x100000, size), PROT_READ | PROT_WRITE);
    if (0 != ret)
    {
        printf("can't mprotect() shmem device");
        return NULL;
    }

    return pucvirtbase;
}

/******************************************************************************
* 函数名: BspShmInitStateSetOk
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int  BspShmInitStateSetOk()
{
    int fd_usdpaa;
    int ret = 0;

    if(g_dwShmHeapInitDone == 0)
    {
        printf("%s failed, because shmem is not inited\n", __FUNCTION__);
        return -1;
    }

    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return -ENODEV;
    }

    ret = ioctl(fd_usdpaa, USDPAA_IOC_GSHM_INITSETOK, NULL);
    if (ret < 0)
    {
        /* fprintf(stderr, "@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno)); */
        close(fd_usdpaa);
        return -1;
    }
    close(fd_usdpaa);

    return 0;

}

/******************************************************************************
* 函数名: BspShmInitStateGet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
unsigned long  long BspShmInitStateGet()
{
    int fd_usdpaa;
    int ret = 0;
    unsigned long  long dwstate;

    if(g_dwShmHeapInitDone == 0)
    {
        printf("%s failed, because shmem is not inited\n", __FUNCTION__);
        return 0;
    }

    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return -ENODEV;
    }

    ret = ioctl(fd_usdpaa, USDPAA_IOC_GSHM_INIT_STATEGET, &dwstate);
    if (ret < 0)
    {
        /* fprintf(stderr, "@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno)); */
        close(fd_usdpaa);
        return 0;
    }
    close(fd_usdpaa);

    return dwstate;
}

/******************************************************************************
* 函数名: BspGetShmLeftSize
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspGetShmLeftSize()
{
    int ret;
    int fd_usdpaa;
    unsigned long dwGshmLeftSize;

    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return -ENODEV;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_GSHM_LEFT_SIZE, &dwGshmLeftSize);
    if (ret < 0)
    {
        /* fprintf(stderr, "@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno)); */
        close(fd_usdpaa);
        return 0;
    }
    close(fd_usdpaa);
    return dwGshmLeftSize;
}

/******************************************************************************
* 函数名: bsp_sharemem_init
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
UINT32 bsp_sharemem_init()
{
    int ret;
    int fd_usdpaa;
    int fd_mem;
    system("mknod /dev/usdpaa c 248 0");
    system("mknod /dev/usdpaa_cinh c 249 0");
    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return -ENODEV;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_GSHMRAMSIZE, &g_dwRamSize);

    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }

    ret = ioctl(fd_usdpaa, USDPAA_IOC_GSHMBOOTSIZE, &g_dwBootSize);

    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_BBX_BASE, &g_dwBbxPhyBase);

    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_BBX_SIZE, &g_dwBbxSize);

    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }

    ret = ioctl(fd_usdpaa, USDPAA_IOC_BOOTPARAM_BASE, &g_dwBootParamPhyBase);

    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_BOOTPARAM_SIZE, &g_dwBootParamSize);

    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_USDPAA_BASE, &g_dwUsDpaaPhyBase);

    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_USDPAA_SIZE, &g_dwUsDpaaSize);
    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }
    close(fd_usdpaa);
    g_dwShmHeapInitDone = 1;
    fd_mem = open("/dev/mem", O_RDWR);
    if (fd_mem < 0)
    {
        printf("can't open /dev/mem device");
        return -1;
    }
    close(fd_mem);
#if 1
    printf("g_dwBootSize = 0x%lx\n", BspGetBootSize());
    printf("g_dwRamSize = 0x%lx\n", BspGetRamSize());
    printf("g_dwBbxPhyBase = 0x%lx\n", BspGetBbxPhyBase());
    printf("g_dwBbxSize = 0x%lx\n", BspGetBbxSize());
    printf("g_dwBootParamPhyBase = 0x%lx\n", BspGetBootParamPhyBase());
    printf("g_dwBootParamSize = 0x%lx\n", BspGetBootParamSize());
    printf("g_dwGshmLeftSize = 0x%lx\n", BspGetShmLeftSize());
    printf("usdpaabase = 0x%lx\n", BspGetUsDpaPhyBase());
    printf("usdpaasize = 0x%lx\n", BspGetUsDpaSize());
    printf("init start: 0x%lx\n", (unsigned long)BspShmInitStateGet());
#endif

    BspShmInitStateSetOk();
    printf("init start: %d\n", (int)BspShmInitStateGet());
    return 0;
}

void  BspShmemInfoShow(void)
{
    unsigned long phyaddr;
    TGshmMallocParam tGshmMallocParam;
    int fd_usdpaa;
    int ret = 0;
    if(g_dwShmHeapInitDone == 0)
    {
        printf("%s failed, because shmem is not inited\n", __FUNCTION__);
        return;
    }
    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return 0;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_GSHM_SHOW, NULL);
    if (ret < 0)
    {
        close(fd_usdpaa);
        return;
    }
    close(fd_usdpaa);
}

void BspShowEmacDataCnt(void)
{
    volatile int ret;
    int fd_usdpaa;
    int fd_mem;
    int ibytecnt=0;
    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return -ENODEV;
    }
    ret = ioctl(fd_usdpaa, USDPAA_IOC_GET_ADDR, &ibytecnt);
    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        close(fd_usdpaa);
        return ret;
    }
    printf("get kernel pack count->0x%lx\n",ibytecnt);
}

void *bsp_shm_p2v(unsigned long dwphyaddr)
{
    WORD16  dwThreadTypeIndex = 0;
    WORD16  dwTableItemCounts = (WORD16)sizeof(gaUserShmemConfigTable)/sizeof(T_ShmemRegInfo);
    WORD32  dwviraddr = 0;
    for (dwThreadTypeIndex = 0; dwThreadTypeIndex < dwTableItemCounts; dwThreadTypeIndex++)
    {
        if (TRUE == (gaUserShmemConfigTable + dwThreadTypeIndex)->isFlag && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr >0 && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr >0 && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwLen >0 )
        {
            if ( (unsigned long)(dwphyaddr) >= (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr && \
                    (unsigned long)(dwphyaddr) < (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr+(gaUserShmemConfigTable + dwThreadTypeIndex)->dwLen)
            {
                dwviraddr = (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr + \
                            (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr - (unsigned int)dwphyaddr;
                break;
            }
        }
    }
    return (void *)dwviraddr;
}

unsigned long bsp_shm_v2p(void *pvaddr)
{
    WORD16  dwThreadTypeIndex = 0;
    WORD16  dwTableItemCounts = (WORD16)sizeof(gaUserShmemConfigTable)/sizeof(T_ShmemRegInfo);
    WORD32  dwphyaddr = 0;
    for (dwThreadTypeIndex = 0; dwThreadTypeIndex < dwTableItemCounts; dwThreadTypeIndex++)
    {
        if (TRUE == (gaUserShmemConfigTable + dwThreadTypeIndex)->isFlag && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr >0 && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr >0 && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwLen >0 )
        {

            if ( (unsigned int)(pvaddr) >= (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr && \
                    (unsigned int)(pvaddr) < (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr + (gaUserShmemConfigTable + dwThreadTypeIndex)->dwLen)
            {
                dwphyaddr = (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr + \
                            (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr - (unsigned int)pvaddr;
                break;
            }
        }
    }
    return dwphyaddr;
}

ULONG bsp_config_sys_shm_table(void)
{
    WORD16  dwThreadTypeIndex = 0;
    WORD16  dwTableItemCounts = (WORD16)sizeof(gaUserShmemConfigTable)/sizeof(T_ShmemRegInfo);
    WORD32  dwphyaddr = 0;
    WORD32 fd;
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    unsigned char *dwviraddr = 0;
    if (BSP_ERROR == fd)
    {
        bsp_dbg("can not open mem!\n");
        close(fd);
        return NULL;
    }
    for (dwThreadTypeIndex = 0; dwThreadTypeIndex < dwTableItemCounts; dwThreadTypeIndex++)
    {
        if (TRUE == (gaUserShmemConfigTable + dwThreadTypeIndex)->isFlag)
        {
            dwphyaddr =  BspShmemPhyMalloc(0x100000, \
                                           BspAlignSize(0x100000, (gaUserShmemConfigTable + dwThreadTypeIndex)->dwLen), \
                                           (gaUserShmemConfigTable + dwThreadTypeIndex)->pName, \
                                           (gaUserShmemConfigTable + dwThreadTypeIndex)->index);
            if (dwphyaddr>0)
            {
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr = dwphyaddr;
                dwviraddr = mmap((void *)0,(gaUserShmemConfigTable + dwThreadTypeIndex)->dwLen,\
                                 PROT_READ|PROT_WRITE,MAP_SHARED,fd,(off_t)(void *)dwphyaddr);
                if (dwviraddr>0)
                {
                    (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr = (unsigned long *)dwviraddr;
                }
            }
            else
            {
                return BSP_ERROR;
            }
        }
    }
    close(fd);
    return BSP_OK;
}

ULONG bsp_find_phyaddr_from_tbl(unsigned char *pname,unsigned int dwindex)
{
    WORD16	dwThreadTypeIndex = 0;
    WORD16  dwTableItemCounts = (WORD16)sizeof(gaUserShmemConfigTable)/sizeof(T_ShmemRegInfo);
    WORD32  dwphyaddr = 0;
    for (dwThreadTypeIndex = 0; dwThreadTypeIndex < dwTableItemCounts; dwThreadTypeIndex++)
    {
        if (TRUE == (gaUserShmemConfigTable + dwThreadTypeIndex)->isFlag && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr >0 && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr >0 && \
                (gaUserShmemConfigTable + dwThreadTypeIndex)->dwLen >0 )
        {

            if((!(strcmp(pname, (gaUserShmemConfigTable + dwThreadTypeIndex)->pName))) && \
                    (dwindex == (gaUserShmemConfigTable + dwThreadTypeIndex)->index))
            {
                dwphyaddr = (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr;
                break;
            }
        }
    }
    return dwphyaddr;
}

#if 0
ULONG bsp_get_ub_addr(unsigned int *paddr,unsigned int *plen)
{
    unsigned int fd;
    unsigned char *pubaddr=0;
    ULONG dwaddr = 0;
    ULONG dwlen  = 0;

    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (BSP_ERROR == fd)
    {
        bsp_dbg("can not open mem!\n");
        return BSP_ERROR;
    }
    if (paddr == NULL || plen == NULL)
    {
        return BSP_ERROR;
    }
    pubaddr = mmap((void *)0,UBPOOLSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,bsp_find_phyaddr_from_tbl("ubpool",0));
    *paddr = pubaddr;
    *plen  = UBPOOLSIZE;
    return BSP_OK;
}
#endif
static u8 *ub_virt_addr;
static u32 ub_phy_addr;

u8 *ub_pa_to_va(u32 pa)
{
    if (pa < ub_phy_addr)
    {
        printf("%s Invaild argument\n", __func__);
        return NULL;
    }

    return (u8 *)((pa - ub_phy_addr) + (u32)ub_virt_addr);
}

u32 ub_va_to_pa(u8 *va)
{
    if (va == NULL || (u32)va < (u32)ub_virt_addr)
    {
        printf("%s Invaild argument\n", __func__);
        return 0;
    }

    return (u32)(va - ub_virt_addr) + ub_phy_addr;
}

void *bsp_get_ub_addr(int iLen)
{
    unsigned int fd;
    unsigned char *pubaddr=0;
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (BSP_ERROR == fd)
    {
        bsp_dbg("can not open mem!\n");
        return BSP_ERROR;
    }
    if (iLen<=0)
    {
        return BSP_ERROR;
    }
    pubaddr = mmap((void *)0,UBPOOLSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,bsp_find_phyaddr_from_tbl("ubpool",0
                                                                                                    ));

    /* get ub virtual and physical address */
    ub_virt_addr = pubaddr;
    ub_phy_addr = bsp_find_phyaddr_from_tbl("ubpool", 0);

    return pubaddr;
}
