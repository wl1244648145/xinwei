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

#ifndef FSL_SHMEM_H
#define FSL_SHMEM_H

/* For an efficient conversion between user-space virtual address map(s) and bus
 * addresses required by hardware for DMA, we use a single contiguous mmap() on
 * the /dev/fsl-shmem device, a pre-arranged physical base address (and
 * similarly reserved from regular linux use by a "mem=<...>" kernel boot
 * parameter). See conf.h for the hard-coded constants that are used. */

/* drain buffer pools of any stale entries (assumes Fman is quiesced),
 * mmap() the device,
 * carve out bman buffers and seed them into buffer pools,
 * initialise ad-hoc DMA allocation memory.
 *    -> returns non-zero on failure.
 */

/* Ad-hoc DMA allocation (not optimised for speed...). NB, the size must be
 * provided to 'free'. */
void *fsl_shmem_memalign(size_t boundary, size_t size);
void fsl_shmem_free(void *ptr, size_t size);
extern int BspDpaShmemSetup(void);


 
#define BspDpaShareMalloc fsl_shmem_memalign
 

 
#define USDPAA_IOC_MAGIC 'S'
#define USDPAA_CHRDEV_NAME "/dev/usdpaa"
#define	IOCPARM_MASK	0x1fff		/* parameter length, at most 13 bits */
#define	SHMEM_IOC_OUT		(unsigned long)0x40000000
#define	SHMEM_IOC_IN		(unsigned long)0x80000000
#define	_IOC_SHMEM(inout,group,num,len) \
	(inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define	_IOR_SHMEM(g,n,t)	_IOC_SHMEM(SHMEM_IOC_OUT,	(g), (n), sizeof(t))
#define	_IOW_SHMEM(g,n,t)	_IOC_SHMEM(SHMEM_IOC_IN,	(g), (n), sizeof(t))

#define    USDPAA_IOC_GSHMBOOTSIZE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 1, unsigned int)
#define    USDPAA_IOC_GSHMRAMSIZE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 2, unsigned int)
#define    USDPAA_IOC_WAITQMINT            _IOR_SHMEM(USDPAA_IOC_MAGIC, 3, unsigned int)   /*  等待qman的中断 */
#define    USDPAA_IOC_BBX_BASE           _IOR_SHMEM(USDPAA_IOC_MAGIC, 4, unsigned int)   /*  获取黑匣子物理基地 */
#define    USDPAA_IOC_BBX_SIZE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 5, unsigned int)   /*  获取黑匣子大小 */
#define    USDPAA_IOC_BOOTPARAM_BASE           _IOR_SHMEM(USDPAA_IOC_MAGIC, 6, unsigned int)   /*  获取黑匣子物理基地 */
#define    USDPAA_IOC_BOOTPARAM_SIZE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 7, unsigned int)   /*  获取黑匣子大小 */
#define    USDPAA_IOC_GSHM_LEFT_SIZE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 8, unsigned int)   /*  获取剩余共享内存大小 */
#define    USDPAA_IOC_USDPAA_BASE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 9, unsigned int)   /*  获取usdpaa网口接收内存的起始地 */
#define    USDPAA_IOC_USDPAA_SIZE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 10, unsigned int)   /*  获取usdpaa网口接收内存的大小 */
#define    USDPAA_IOC_GSHM_MALLOC            _IOR_SHMEM(USDPAA_IOC_MAGIC, 11, unsigned int)   /*  共享内存申请接口 */
#define    USDPAA_IOC_GSHM_INITSETOK            _IOR_SHMEM(USDPAA_IOC_MAGIC, 12, unsigned int)   /*  设置用户态初始化完毕标志 */
#define    USDPAA_IOC_GSHM_INIT_STATEGET            _IOR_SHMEM(USDPAA_IOC_MAGIC, 13, unsigned int)   /*  获取用户态初始化状态标志 */
#define    USDPAA_IOC_GSHM_SHOW            _IOR_SHMEM(USDPAA_IOC_MAGIC, 14,unsigned int)   /*  显示共享内存的分配情况 */
#define    USDPAA_IOC_GSHM_FREE            _IOR_SHMEM(USDPAA_IOC_MAGIC, 15, unsigned int)   /*  共享内存释放 */
#define    USDPAA_IOC_WAITQM9INT            _IOR_SHMEM(USDPAA_IOC_MAGIC, 16, unsigned int)   /*  等待qman9的中断 */
#define    USDPAA_IOC_GET_ADDR               _IOR_SHMEM(USDPAA_IOC_MAGIC, 17,unsigned int) 
#define    USDPAA_IOC_OPEN_MIRROR             _IOR(USDPAA_IOC_MAGIC, 18,unsigned int) 
#define    USDPAA_IOC_CLOSE_MIRROR            _IOR(USDPAA_IOC_MAGIC, 19,unsigned int) 
#define    USDPAA_IOC_LOOPBACK                _IOR(USDPAA_IOC_MAGIC, 20, unsigned int) 
#define    USDPAA_IOC_NOLOOPBACK              _IOR(USDPAA_IOC_MAGIC, 21, unsigned int)
#define    USDPAA_IOC_DETECTUSB               _IOR(USDPAA_IOC_MAGIC, 22, unsigned int)
#define    USDPAA_IOC_GETIICSTATUS            _IOR(USDPAA_IOC_MAGIC, 23, unsigned int) 
#define    USDPAA_IOC_SETIICSTATUS            _IOR(USDPAA_IOC_MAGIC, 24, unsigned int)
#define    USDPAA_IOC_HMIINIT                   _IOR(USDPAA_IOC_MAGIC, 25, unsigned int) 
#define    USDPAA_IOC_HMISETID                 _IOR(USDPAA_IOC_MAGIC, 26, unsigned int)
#define    USDPAA_IOC_HMIGETDATA               _IOR(USDPAA_IOC_MAGIC, 27, unsigned int)
#define    USDPAA_IOC_HMISETDATA                _IOR(USDPAA_IOC_MAGIC, 28, unsigned int)
#endif /* !FSL_SHMEM_H */

