/* Copyright (c) 2010 Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Freescale Semiconductor nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../inc/private.h"

/* For an efficient conversion between user-space virtual address map(s) and bus
 * addresses required by hardware for DMA, we use a single contiguous mmap() on
 * the /dev/fsl-shmem device, a pre-arranged physical base address (and
 * similarly reserved from regular linux use by a "mem=<...>" kernel boot
 * parameter). See conf.h for the hard-coded constants that are used. */
unsigned int dwtmpphyaddr=0;
//static int fd;

/* Present "carve up" is to use the first 0x5b80000 bytes of shmem for buffer
 * pools and the rest of shmem (which is 256MB total) for ad-hoc allocations. */
#define SHMEM_ALLOC_BAR	((void *)FSL_SHMEM_VIRT + 0x5b80000)
#define SHMEM_ALLOC_SZ	(FSL_SHMEM_SIZE - 0x05b80000)




unsigned long g_shmemstart = 0;
unsigned long g_shmemsize = 0;
void * fsl_shmem_memalign(size_t align, size_t size)
{
    unsigned long addr;
    unsigned long oldstart;

    if(g_shmemsize < (align + size))
    {
        printf("[core%d]dpaa share mem is not enough, function:%s, line:%d\n", BspGetSelfCoreId(), __FUNCTION__, __LINE__);
        return 0;
    }
    oldstart = g_shmemstart;

    if(g_shmemstart && (align - 1))
        addr =  g_shmemstart;
    else
        addr = (g_shmemstart + align) & (~(align - 1));

    g_shmemstart = addr + size;

    g_shmemsize = g_shmemsize - (addr - oldstart) - size;
    return (void*)addr;

}






/**********************************************************************
* 函数名称：BspShmemSetup
* 功能描述：共享内存初始化
* 访问的表：无
* 修改的表：无
* 输入参数：
* 	                      无
* 输出参数：
* 返 回 值：
*			0:成功
*               其它:失败
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013/07/10    V1.0
************************************************************************/
int g_dwShMemSetup = 0;
#define FSL_SHMEM_PHYS	(u32)0x70000000 /* 1.75G */
#define FSL_SHMEM_SIZE	(u32)0x10000000 /* 256M */
#define MEM_SIZE_64M	(u32)0x4000000 /* 64M */

/******************************************************************************
* 函数名: BspDpaShmemSetup
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspDpaShmemSetup(void)
{
    //void *p;
    //unsigned int dwtmpphyaddr1=0;
    u32 fd;
    u8 *g_u8tmp1 = 0;
    int ret = -ENODEV;
    unsigned long  dwphybase = 0;
    unsigned char *pucvirtbase = 0;

    pucvirtbase = BspShmemVirtMalloc(0x100000, BspAlignSize(0x100000, DPAA_SHMEM_HEAP_SIZE), "dpaheap", 0,(int *)&dwtmpphyaddr);


    dwphybase = dwtmpphyaddr;
    ret = mprotect((void *)(pucvirtbase), BspAlignSize(0x100000, DPAA_SHMEM_HEAP_SIZE), PROT_READ | PROT_WRITE);
    if (0 != ret)
    {
        printf("can't mmap() shmem device\r\n");
        return -1;
    }
    g_shmemstart = (unsigned long)pucvirtbase;
    g_shmemsize  = DPAA_SHMEM_HEAP_SIZE;
    printf("bsp dpaa shmem device mapped (phys=0x%x,virt=%p,sz=0x%x)\n",(unsigned int)dwphybase, pucvirtbase,(unsigned int)BspAlignSize(0x100000, DPAA_SHMEM_HEAP_SIZE));
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (-1 == fd)
    {
        printf("can not open mem!\n");
        return -1;
    }
    g_u8tmp1 = mmap((void *)0,BspGetUsDpaSize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,(void *)BspGetUsDpaPhyBase());
    close(fd);
#if 0
    ret = mprotect((void *)g_u8tmp1, BspGetUsDpaSize(), PROT_READ | PROT_WRITE);
    if (ret != 0)
    {
        printf("\r\nmprotect() failed, in function:%s, on line:%d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    return ret;
#else
    return 0;
#endif
}


