

#include "../inc/bman_private.h"

/*****************/
/* Portal driver */
/*****************/

struct bm_portal gatBmPortal[10];
struct bman_portal *gaptBmanPortal[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static __thread struct bm_portal portal;
static __thread int fd;
DEFINE_PER_CPU(struct bman_portal *, bman_affine_portal);

u8 bm_portal_num(void)
{
	return 1;
}
EXPORT_SYMBOL(bm_portal_num);

struct bm_portal *bm_portal_get(u8 idx)
{
	if (unlikely(idx >= 1))
		return NULL;

	return &portal;
}
EXPORT_SYMBOL(bm_portal_get);

const struct bm_portal_config *bm_portal_config(const struct bm_portal *portal)
{
	return &portal->config;
}
EXPORT_SYMBOL(bm_portal_config);

static struct bm_portal *__bm_portal_add(const struct bm_addr *addr,
				const struct bm_portal_config *config, int portalnum)
{
	struct bm_portal *ret = &(gatBmPortal[portalnum]);
	ret->addr = *addr;
	ret->config = *config;
	ret->config.bound = 0;
	return ret;
}

int __bm_portal_bind(struct bm_portal *portal, u8 iface)
{
	int ret = -EBUSY;
	if (!(portal->config.bound & iface)) {
		portal->config.bound |= iface;
		ret = 0;
	}
	return ret;
}

void __bm_portal_unbind(struct bm_portal *portal, u8 iface)
{
	BM_ASSERT(portal->config.bound & iface);
	portal->config.bound &= ~iface;
}


/***************/
/* Driver load */
/***************/

int bman_thread_init(int cpu)
{
	return 0;
}



#include "../inc/bspbman.h"

struct bman_portal g_atbman_portal[10];
unsigned long g_dwbmandebug = 0;
/* userdpaa:portalnum = USDPAA_BMAN_PORTAL_NUM; cpu =0 */
/**********************************************************************
* 函数名称：BspBmanPortalInit
* 功能描述：bman portal初始化
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
*作者:刘刚
************************************************************************/
int  BspBmanPortalInit(unsigned long  portalnum, int cpu)
{
	struct bm_portal_config cfg = {
		.cpu = cpu,
		.irq = -1,
		/* FIXME: hard-coded */
		.mask = BMAN_DEPLETION_FULL
	};
	unsigned long tmp;
	struct bm_addr addr;
	struct bm_portal *portal;

 
	int  fd_usdpaa = -1;
	int  fd_usdpaa_cinh = -1;
	void *ptmp = NULL;

    
    
	fd_usdpaa = open("/dev/usdpaa", O_RDWR);
	if (fd_usdpaa< 0) {
		perror("can't open /dev/usdpaa device");
		return -ENODEV;
	}
       /*  为了使虚拟地址16K对齐，映射两次 */
	ptmp = mmap64(0, 16*1024 *2, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd_usdpaa, CONFIG_SYS_BMAN_MEM_BASE + (unsigned long)(portalnum*16*1024));
	if (ptmp == MAP_FAILED)
	{
		perror("mmap of CENA failed\n");
		close(fd_usdpaa);
		return -1;
	}

	printf("ptmp = %p\n", ptmp);
	//d4(ptmp,0x100);
	//while(1);
	
	//	addr.addr_ce = mmap64(/* BMAN_CENA(portalnum) */(void *)(((unsigned long)ptmp +16*1024) & (~(16*1024 - 1) )), 16*1024, PROT_READ | PROT_WRITE,
	//		MAP_SHARED  | MAP_FIXED, fd_usdpaa, CONFIG_SYS_BMAN_MEM_BASE + (unsigned long)(portalnum*16*1024));

	
	munmap(ptmp, 16*1024 * 2);
	
	addr.addr_ce = mmap64(/* BMAN_CENA(portalnum) */(void *)(((unsigned long)ptmp +16*1024) & (~(16*1024 - 1) )), 16*1024, PROT_READ | PROT_WRITE,
			MAP_SHARED  | MAP_FIXED, fd_usdpaa, CONFIG_SYS_BMAN_MEM_BASE + (unsigned long)(portalnum*16*1024));
	if (addr.addr_ce == MAP_FAILED)
	{
		perror("mmap of CENA failed\n");
		close(fd_usdpaa);
		return -1;
	}

	//addr.addr_ce = (unsigned long)ptmp & (~(16*1024 - 1) );
		
	BspDpaaPrintf("will close fd_usdpaa\n");
	close(fd_usdpaa);


	fd_usdpaa_cinh = open("/dev/usdpaa_cinh", O_RDWR);
	if (fd_usdpaa_cinh< 0) {
		perror("can't open /dev/usdpaa_cinh device");
		return -ENODEV;
	}
	addr.addr_ci = mmap64(/* BMAN_CINH(portalnum) */0, 4*1024, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd_usdpaa_cinh, CONFIG_SYS_BMAN_MEM_BASE + (unsigned long)0x100000 + (unsigned long)(portalnum*4*1024));
	if (addr.addr_ci == MAP_FAILED)
	{
		perror("mmap of CINH failed");
		close(fd_usdpaa_cinh);
		return -1;
	}
	BspDpaaPrintf("will close fd_usdpaa_cinh\n");
	close(fd_usdpaa_cinh);
 

    
	portal = __bm_portal_add(&addr, &cfg, portalnum);
	if (!portal)
		return -ENOMEM;
	pr_info("Bman portal at %p:%p (%d)\n", addr.addr_ce, addr.addr_ci,
		cfg.cpu);
#ifndef CONFIG_FSL_BMAN_PORTAL_DISABLEAUTO
    if (cfg.cpu == -1)/* jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj */
		    return 0;
    if (!gaptBmanPortal[portalnum]) 
	  {
	  	  //printf("loading bman_create_portal \n");
		    gaptBmanPortal[portalnum] = bman_create_portal(portal, &cfg.mask);
		    if (!gaptBmanPortal[portalnum]) 
		    {
			      pr_err("Bman portal auto-initialisation failed\n");
			      return 0;
		    }
		    //pr_info("Bman portal %d auto-initialised\n", portalnum);
	}
#endif
	return 0;
}

