 

/***********************************************************
 *                      头文件                             *
***********************************************************/
#include "../inc/bspqman.h"
#include "../inc/fman.h"
#include "../inc/compat.h"
#include "../inc/fqid.h"
#include "../inc/fsl_shmem.h"

/***********************************************************
 *                 文件内部使用的宏                        *
***********************************************************/
#define STASH_CTX_CL(p) \
({ \
	__always_unused const typeof(*(p)) *foo = (p); \
	int foolen = sizeof(*foo) / 64; \
	if (foolen > 3) \
		foolen = 3; \
	foolen; \
})

#define FQ_TAILDROP_THRESH 6400000

extern void d4 (u32 uladdr,u32 len);
/**********************************************************************
* 函数名称：BspQmFqForDeqInit
* 功能描述：软件出队的队列创建及初始化
* 访问的表：无
* 修改的表：无
* 输入参数：
* 	                      qman_fq_flag:
*                                          QMAN_FQ_FLAG_NO_ENQUEUE:    can't enqueue  
*                                          QMAN_FQ_FLAG_NO_MODIFY:    can only enqueue  
*                                          QMAN_FQ_FLAG_TO_DCPORTAL:      consumed by CAAM/PME/Fman
*                                          QMAN_FQ_FLAG_LOCKED: multi-core locking
*                                          QMAN_FQ_FLAG_RECOVER:  recovery mode
*                                          QMAN_FQ_FLAG_DYNAMIC_FQID:  (de)allocate fqid
*                          fqid: 
*                          channel:
*                          wq: 0-7, 0优先级最高
*                          pfun: 对出队数据的处理回调
* 输出参数：
* 返 回 值：
*			0:成功
*               其它:失败
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013/07/10    V1.0           
************************************************************************/
struct qman_fq  *BspQmFqForDeqInit(u32 fqid, enum qm_channel channel, 
                                                     enum qm_wq wq, unsigned long qman_fq_flag, qman_cb_dqrr pfun)
{
    struct qm_mcc_initfq opts;
	  struct qm_fqd  fqd;
	  int ret = 0;
	  int portalnum;
    struct qman_fq *fq  =  (struct qman_fq *)BspDpaShareMalloc(64, sizeof(*fq));
	  if(fq == NULL)
	  {
		    BspDpaaPrintf("file:%s, line:%d\n", __FILE__, __LINE__); 
		    return fq;
	  }
	  memset(fq, 0, sizeof(*fq));
 
    portalnum = USDPAA_QMAN_PORTAL_NUM;
 
	  fq->cb.dqrr = pfun;
	  ret = qman_create_fq(fqid, qman_fq_flag, fq, gaptQmanPortal[portalnum]);
	  if(0 != ret)
	  {
	      //printf("BspQmFqForDeqInit--error1!\n");
	      return NULL;
	  }
	  BUG_ON(ret);
	  /* FIXME: no taildrop/holdactive for "2drop" FQs */
	opts.we_mask = QM_INITFQ_WE_DESTWQ | QM_INITFQ_WE_FQCTRL |
			QM_INITFQ_WE_CONTEXTA | QM_INITFQ_WE_TDTHRESH;
	  opts.fqd.dest.channel = channel;
	  opts.fqd.dest.wq = wq;
	  opts.fqd.fq_ctrl = QM_FQCTRL_CTXASTASHING | QM_FQCTRL_TDE |QM_FQCTRL_PREFERINCACHE;
	  opts.fqd.context_a.stashing.data_cl = 1;
	  opts.fqd.context_a.stashing.context_cl = STASH_CTX_CL(fq);/* cache line count */
      ret = qm_fqd_taildrop_set(&opts.fqd.td, FQ_TAILDROP_THRESH, 0);
      BUG_ON(ret);
	  if(0 != ret)
	  {
	      printf("BspQmFqForDeqInit--error2!\n");
	      return NULL;
	  }
	  printf("gaptQmanPortal[%d]=%p\n",portalnum,gaptQmanPortal[portalnum]);
	  
	  ret = qman_init_fq(fq, QMAN_INITFQ_FLAG_SCHED, &opts, gaptQmanPortal[portalnum]);
	  // printf("BspQmFqForDeqInit---oooooooooooo\n");
	  BUG_ON(ret);
	  if(0 != ret)
	  {
	      printf("BspQmFqForDeqInit--error3!\n");
		  return NULL;
	  }
	  //printf("BspQmFqForDeqInit---3\n");
	  ret = qman_query_fq(fq, &fqd, gaptQmanPortal[portalnum]);
	  BspDpaaPrintf("fqd.td.exp = %x, fqd.td.mant = %x\n", fqd.td.exp, fqd.td.mant);
	  //fqd.td.exp = 0, fqd.td.mant = 0 
	  BUG_ON(ret);
	  return fq;
}

void BspShowFq(void)
{
    struct qm_fqd  fqd;
	int ret = 0;
	int portalnum;
    struct qman_fq *fq  =  (struct qman_fq *)BspDpaShareMalloc(64, sizeof(*fq));
	if(fq == NULL)
	{
	    BspDpaaPrintf("file:%s, line:%d\n", __FILE__, __LINE__); 
		return ;
	}
	memset(fq, 0, sizeof(*fq));
    portalnum = USDPAA_QMAN_PORTAL_NUM;
    ret = qman_query_fq(fq, &fqd, gaptQmanPortal[portalnum]);
    ret = ret;
	
	BspDpaaPrintf("fqd.td.exp = %x, fqd.td.mant = %x\n", fqd.td.exp, fqd.td.mant);
	BspDpaaPrintf("fqd.orprws=0x%lx\n",(unsigned long)fqd.orprws);
	BspDpaaPrintf("fqd.oa=0x%lx\n",(unsigned long)fqd.oa);
	BspDpaaPrintf("fqd.olws=0x%lx\n",(unsigned long)fqd.olws);


	BspDpaaPrintf("fqd.cgid=0x%lx\n",(unsigned long)fqd.cgid);

	BspDpaaPrintf("fqd.fq_ctrl=0x%lx\n",(unsigned long)fqd.fq_ctrl);

	BspDpaaPrintf("fqd.dest_wq=0x%lx\n",(unsigned long)fqd.dest_wq);


	BspDpaaPrintf("fqd.dest.channel=0x%lx\n",(unsigned long)fqd.dest.channel);
    BspDpaaPrintf("fqd.dest.wq=0x%lx\n",(unsigned long)fqd.dest.wq);
	 
		
    BspDpaaPrintf("fqd.context_b=0x%lx\n",(unsigned long)fqd.context_b);

	BspDpaaPrintf("fqd.context_a.hi=0x%lx\n",(unsigned long)fqd.context_a.hi);
	BspDpaaPrintf("fqd.context_a.lo=0x%lx\n",(unsigned long)fqd.context_a.lo);

	BspDpaaPrintf("fqd.context_a.context_hi=0x%lx\n",(unsigned long)fqd.context_a.context_hi);
	BspDpaaPrintf("fqd.context_a.context_lo=0x%lx\n",(unsigned long)fqd.context_a.context_lo);
	//fqd.context_b


}

EXPORT_SYMBOL(BspQmFqForDeqInit);



/**********************************************************************
* 函数名称：BspQmFqForEnqInit
* 功能描述：软件入队的队列创建及初始化
* 访问的表：无
* 修改的表：无
* 输入参数：
* 	                      qman_fq_flag:
*                                          QMAN_FQ_FLAG_NO_ENQUEUE:    can't enqueue  
*                                          QMAN_FQ_FLAG_NO_MODIFY:    can only enqueue  
*                                          QMAN_FQ_FLAG_TO_DCPORTAL:      consumed by CAAM/PME/Fman
*                                          QMAN_FQ_FLAG_LOCKED: multi-core locking
*                                          QMAN_FQ_FLAG_RECOVER:  recovery mode
*                                          QMAN_FQ_FLAG_DYNAMIC_FQID:  (de)allocate fqid
*                          fqid: 
*                          channel:
*                          wq: 0-7, 0优先级最高
*                          pfun:  入队非法mr 回调
* 输出参数：
* 返 回 值：
*			0:成功
*               其它:失败
* 其它说明：
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2013/07/18    V1.0           
************************************************************************/
struct qman_fq *BspQmFqForEnqInit(u32 fqid, enum qm_channel channel, 
                                                     enum qm_wq wq, unsigned long qman_fq_flag, qman_cb_mr pfun)
{
	struct qm_mcc_initfq opts;
	int ret;
	int portalnum;
	struct qman_fq *fq = (struct qman_fq *)BspDpaShareMalloc(64, sizeof(*fq));
	if(fq == NULL)
	{
		BspDpaaPrintf("file:%s, line:%d\n", __FILE__, __LINE__); 
		return fq;
	}
	memset(fq, 0, sizeof(*fq));

 
    portalnum = USDPAA_QMAN_PORTAL_NUM;
 

	fq->cb.ern = pfun;
	ret = qman_create_fq(fqid, qman_fq_flag, fq, gaptQmanPortal[portalnum]);
	BUG_ON(ret);
	opts.we_mask = QM_INITFQ_WE_DESTWQ | QM_INITFQ_WE_FQCTRL |
		       QM_INITFQ_WE_CONTEXTB | QM_INITFQ_WE_CONTEXTA;
	opts.fqd.dest.channel = channel;
	opts.fqd.dest.wq = wq;
	opts.fqd.fq_ctrl =
#ifdef POC_2FWD_TX_PREFERINCACHE
		QM_FQCTRL_PREFERINCACHE |
#endif
		0;
	opts.fqd.context_b = 0;
	opts.fqd.context_a.hi = 0x80000000;
	opts.fqd.context_a.lo = 0;
	ret = qman_init_fq(fq, QMAN_INITFQ_FLAG_SCHED, &opts, gaptQmanPortal[portalnum]);
	if (ret) {
		/* revert to NO_MODIFY */
		BspDpaaPrintf("error in file:%s, on line:%d\n", __FILE__, __LINE__);
		BUG_ON(ret);
		return NULL;
	}
	return fq;
}
  
EXPORT_SYMBOL(BspQmFqForEnqInit);


void qm_set_memory(enum qm_memory memory, u16 eba,
			u32 ba, int enable, int prio, int stash, u32 exp)
{
	u32 offset = (memory == qm_memory_fqd) ? FQD_BARE : PFDR_BARE;

	*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + offset + 0x00)) = eba;
	*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + offset + 0x04)) = ba;
	*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + offset + 0x10 )) = 		(enable ? 0x80000000 : 0) |
		                                                                                                                                     (prio ? 0x40000000 : 0) |
		                                                                                                                                      (stash ? 0x20000000 : 0) |
		                                                                                                                                       (exp - 1);
}

int qm_init_pfdr(u32 pfdr_start, u32 num)
{
	u8 rslt = MCR_get_rslt(*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + QMAN_MCR)) );

	QM_ASSERT(pfdr_start && !(pfdr_start & 7) && !(num & 7) && num);
	/* Make sure the command interface is 'idle' */
	
	if(!MCR_rslt_idle(rslt))
		panic("QMAN_MCR isn't idle");

	/* Write the MCR command params then the verb */
	*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + QMAN_MCP0)) = pfdr_start;
	/* TODO: remove this - it's a workaround for a model bug that is
	 * corrected in more recent versions. We use the workaround until
	 * everyone has upgraded. */
	 *((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + QMAN_MCP1)) =  (pfdr_start + num - 16);
	lwsync();
	(*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + QMAN_MCR)) ) = MCR_INIT_PFDR;

	/* Poll for the result */
	do {
		rslt = MCR_get_rslt(*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + QMAN_MCR)) );
	} while(!MCR_rslt_idle(rslt));
	if (MCR_rslt_ok(rslt))
		return 0;
	if (MCR_rslt_eaccess(rslt))
		return -EACCES;
	if (MCR_rslt_inval(rslt))
		return -EINVAL;
	pr_crit("Unexpected result from MCR_INIT_PFDR: %02x\n", rslt);
	return -ENOSYS;
}


void qm_set_QCSP_BAR( u16 eba,			u32 ba)
{
	*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + QCSP_BARE)) = eba;
	*((unsigned long*)(CCSR_VIRTADDR_BASE + QMAN_CCSR_OFFSET + QCSP_BAR)) = ba;
}


void BspShowQmanRegInfo(void)
{
	printf("\r\n=====QMan Software Portal Configuration Registers====\r\n");
    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET),0x100);
	printf("\r\n=====Dynamic Debug(DD) Configuration Registers=======\r\n");
	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x200),0x20);
	printf("\r\n=====Direct Connect Portal(DCP) Configuration Registsers=\r\n");
	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x300),0x40);
	printf("\r\n======Packet Frame Descriptor Record(PFDR)Manager Query Registers=\r\n");
	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x400),0x10);
	printf("\r\n======Single Frame Descriptor Record(SFDR)Manager Register===\r\n");
	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x500),0x10);
	printf("\r\n======Work Queue Semaphore and Context Manager Register=====\r\n");
	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x600),0x100);
	printf("\r\n======Qman Error Capture Registers=========================\r\n");
    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xa00),0x100);
    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xa70),0x40);
	printf("\r\n======Qman Initialization and Debug Control Registers===\r\n");
    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xb00),0x40);
    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xbe0),0x40);
	printf("\r\n======Qman Initiator Interface Memory Window Configuration Registers\r\n");
    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xc00),0x40);  
    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xd00),0x40);	
	printf("\r\n======Qman Interrupt and Error Registers=======\r\n");
    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xe00),0x40);
}
