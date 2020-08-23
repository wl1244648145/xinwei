/* Copyright (c) 2008-2010 Freescale Semiconductor, Inc.
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

#include "../inc/qman_private.h"

/* Global variable containing revision id (even on non-control plane systems
 * where CCSR isn't available). FIXME: hard-coded. */
u16 qman_ip_rev = QMAN_REV1;

/*****************/
/* Portal driver */
/*****************/
struct qm_portal gatQmPortal[10];
struct qman_portal *gaptQmanPortal[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static __thread struct qm_portal portal;
static __thread int fd;
DEFINE_PER_CPU(struct qman_portal *, qman_affine_portal);

u8 qm_portal_num(void)
{
    return 1;
}
EXPORT_SYMBOL(qm_portal_num);

struct qm_portal *qm_portal_get(u8 idx)
{
	if (unlikely(idx >= 1))
		return NULL;

	return &portal;
}
EXPORT_SYMBOL(qm_portal_get);

const struct qm_portal_config *qm_portal_config(const struct qm_portal *portal)
{
    return &portal->config;
}

EXPORT_SYMBOL(qm_portal_config);

static struct qm_portal *__qm_portal_add(const struct qm_addr *addr,
				const struct qm_portal_config *config, int portalnum)
{
    struct qm_portal *ret = &(gatQmPortal[portalnum]);
	  ret->addr = *addr;
	  ret->config = *config;
	  ret->config.bound = 0;
#ifdef CONFIG_FSL_QMAN_BUG_AND_FEATURE_REV1

    ret->bugs = (void *)get_zeroed_page(GFP_KERNEL);
	if (!ret->bugs) {
		pr_err("Can't get zeroed page for 'bugs'\n");
		return NULL;
	}
#endif
    return ret;
}

int __qm_portal_bind(struct qm_portal *portal, u8 iface)
{
    int ret = -EBUSY;
	if (!(portal->config.bound & iface)) {
		portal->config.bound |= iface;
		ret = 0;
	}
	return ret;
}

void __qm_portal_unbind(struct qm_portal *portal, u8 iface)
{
    QM_ASSERT(portal->config.bound & iface);
	  portal->config.bound &= ~iface;
}

/* Handlers for NULL portal callbacks (ie. where the contextB field, normally
 * pointing to the corresponding FQ object, is NULL). */
static enum qman_cb_dqrr_result null_cb_dqrr(struct qman_portal *qm,
					struct qman_fq *fq,
					const struct qm_dqrr_entry *dqrr)
{
    pr_warning("Ignoring unowned DQRR frame on portal %p.\n", qm);
	  return qman_cb_dqrr_consume;
}

static void null_cb_mr(struct qman_portal *qm, struct qman_fq *fq,
			const struct qm_mr_entry *msg)
{
	pr_warning("Ignoring unowned MR msg on portal %p, verb 0x%02x. there is %d buf in bufpool %d\n",
			qm, msg->verb, BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, msg->ern.fd.bpid), msg->ern.fd.bpid);
}
static const struct qman_fq_cb null_cb = {
	.dqrr = null_cb_dqrr,
	.ern = null_cb_mr,
	.dc_ern = null_cb_mr,
	.fqs = null_cb_mr
};

/***************/
/* Driver load */
/***************/

int qman_thread_init(int cpu)
{
    return 0;
}

#include "../inc/bspqman.h"



struct qman_portal *BspGetQmanPortal(int portalnum)
{
    if(portalnum < 10)
    {
		    return gaptQmanPortal[portalnum];
    }
	//printf("error on line:%d, in file:%s\n", __LINE__, __FILE__);

	 return NULL;
}


int g_dwBspQmanPortInitSeq = 0;
void *ptmp_new = NULL;

int  BspQmanPortalInit(unsigned long  portalnum, int cpu)
{
    struct qm_portal_config cfg = 
    {
		    .cpu = cpu,
		    .irq = -1,
		    /* FIXME: hard-coded */
		    .channel = qm_channel_swportal0 + portalnum,
		    /* FIXME: hard-coded */
		    .pools = QM_SDQCR_CHANNELS_POOL_MASK,
		    /* FIXME: hard-coded */
		    .has_hv_dma = 1
    };
	unsigned long tmp;
	struct qm_addr addr;
	struct qm_portal *portal; 
	int  fd_usdpaa = -1;
	int  fd_usdpaa_cinh = -1;
	//void *ptmp = NULL;
    
	fd_usdpaa = open("/dev/usdpaa", O_RDWR);
	if (fd_usdpaa< 0) {
		perror("can't open /dev/usdpaa device");
		return -ENODEV;
	}


     /*  为了使虚拟地址16K对齐，映射两次 */
	ptmp_new = mmap64(0 , 16*1024 *2, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd_usdpaa, CONFIG_SYS_QMAN_MEM_BASE + (unsigned long)(portalnum*16*1024));
	if (ptmp_new == MAP_FAILED)
	{
		perror("mmap of CENA failed\n");
		close(fd_usdpaa);
		return -1;
	}
	printf("ptmp_new = %p\n", ptmp_new);
	munmap(ptmp_new, 16*1024 + 16*1024);
   	addr.addr_ce = mmap64(/*QMAN_CENA(portalnum)*/(void *)(((unsigned long)ptmp_new +16*1024) & (~(16*1024 - 1) )), 16*1024, PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_FIXED, fd_usdpaa, CONFIG_SYS_QMAN_MEM_BASE + (unsigned long)(portalnum*16*1024));
    	if (addr.addr_ce == MAP_FAILED)
      {
		perror("mmap of CENA failed\n");
        	close(fd_usdpaa);
             return -1;
	}
		
	printf("will close fd_usdpaa\n");
	close(fd_usdpaa);


    fd_usdpaa_cinh = open("/dev/usdpaa_cinh", O_RDWR);
	if (fd_usdpaa_cinh< 0) {
		perror("can't open /dev/usdpaa_cinh device");
		return -ENODEV;
	}
	addr.addr_ci = mmap64(/* QMAN_CINH(portalnum) */0, 4*1024, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd_usdpaa_cinh, CONFIG_SYS_QMAN_MEM_BASE + (unsigned long)0x100000 + (unsigned long)(portalnum*4*1024));
	if (addr.addr_ci == MAP_FAILED)
	{
		perror("mmap of CINH failed");
		close(fd_usdpaa_cinh);
		return -1;
	}
    printf("will close fd_usdpaa_cinh\n");
	close(fd_usdpaa_cinh);
	//d4(addr.addr_ci,0x100);
	//while(1);

    portal = __qm_portal_add(&addr, &cfg, portalnum);
    g_dwBspQmanPortInitSeq = __LINE__;
    if (!portal)
        return -ENOMEM;
    g_dwBspQmanPortInitSeq = __LINE__;
	pr_info("Qman portal at %p:%p (%d:%d,v%04x)\n", addr.addr_ce,
		addr.addr_ci, cfg.cpu, cfg.channel, qman_ip_rev);

	 

	#ifndef CONFIG_FSL_QMAN_PORTAL_DISABLEAUTO
	  if (cfg.cpu == -1)
		return 0;
    g_dwBspQmanPortInitSeq = __LINE__;
   // printf("gaptQmanPortal[%d]=%d\n",portalnum,gaptQmanPortal[portalnum]);
	  if (!gaptQmanPortal[portalnum]) 
	  {
		    u32 flags = 0;
		    if (cfg.has_hv_dma)
			flags = QMAN_PORTAL_FLAG_RSTASH |
				QMAN_PORTAL_FLAG_DSTASH;
		    g_dwBspQmanPortInitSeq = __LINE__;
       // printf("sssssssssssssssssssssss\n");
		
		gaptQmanPortal[portalnum] = qman_create_portal(portal, flags, NULL,&null_cb);
		g_dwBspQmanPortInitSeq = __LINE__;
        if (!gaptQmanPortal[portalnum]) 
        {
	        pr_err("Qman portal auto-initialisation failed\n");
			g_dwBspQmanPortInitSeq = __LINE__;
			return 0;
		}
        #if 1
		    /* default: enable all (available) pool channels */
		    //qman_static_dequeue_add_ex(portal, ~0);
        #endif
		    printf("Qman portal %d auto-initialised\n", (int)portalnum);
    }
#endif

	

    return 0;
}

