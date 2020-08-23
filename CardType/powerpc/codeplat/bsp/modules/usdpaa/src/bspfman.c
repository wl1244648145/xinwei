#include "../inc/fm_regs.h"
#include "../inc/fm_pcd.h"
#include "../inc/fqid.h"
#include "../inc/fsl_qman.h"
#include "../inc/fsl_bman.h"
#include "../inc/TBuf.h"
#include "../inc/bspbman.h"
#include "../inc/compat.h"
#include "../inc/fsl_shmem.h"
#include "../inc/pr.h"
#include "../inc/bspshmem.h"
#include <inttypes.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include "../inc/bspdpaa.h"

#define ETH_HEADER_LEN 14
#define IP_MIN_SIZE         40
#define  UDP_DEST_GTPU  2152
#define IP_PROTOCOL_UDP (uint8_t)0x11 /* UDP */

typedef struct tagGtpuPktStat
{
	unsigned long dwfdformaterr;
	unsigned long dwbmdepletion;
	unsigned long dwethertypeerr;
	unsigned long dwl3protocolerr;
	unsigned long dwl4desterr;
	unsigned long dwiphdrerr;
	unsigned long dwcallbackcount;
	unsigned long dwinputerr;
	unsigned long dwchecksumerr;
	unsigned long dwenqueueerr;
	unsigned long dweqsuccess;
	unsigned long dwtbufheadcheckerr;
	unsigned long dwloopcnt;
}GtpuPktStat;

extern irqreturn_t QmPortalIsr(void *ptr);
/***********************************************************
 *                     全局变量                            *
***********************************************************/
struct qman_fq  *gptfq_gtpu_udp = NULL;
struct qman_fq  *gptfq_MacRx = NULL;

/* 当前网口发送队列 */
struct qman_fq  *gptTx = NULL;

/*  每个网口的发送队列 */
struct qman_fq  *gaptTxFq[10] = {NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL};

/* 用户面发的mac头 */
unsigned char g_aucUpEmacHeader[] = {0x40,0x00,0xc0,0xfe,0x01,0x11,    0x40,0x00,0xc0,0xfe,0x07,0x10,   0x08,0x00};//{0x40,0x00,0xc0,0xfe,0x01,0x5a,    0x40,0x00,0xc0,0xfe,0x07,0x10,   0x08,0x00};
//typedef  unsigned int (*RECVGMAC_FUNCPTR)( TBuf* ptBuf);
extern unsigned int  BspEmacRegRecvCallBack (RECVGMAC_FUNCPTR pCallBack);
extern int BspEnQmInt(int portalnum);
extern int  BspQmanPortalInit(unsigned long  portalnum, int cpu);
extern int  BspBmanPortalInit(unsigned long  portalnum, int cpu);

RECVGMAC_FUNCPTR  g_pfRecvGmacPacket = NULL;


unsigned long g_dwDpaaInitDone = 0;
unsigned long g_dwusdpaaflag = 0;

GtpuPktStat g_tGtpuPktStat;

/******************************************************************************
* 函数名: BspDropFrame
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/
static inline void BspDropFrame(const struct qm_fd *fd)
{
	struct bm_buffer buf;
	int ret;
	int portalnum;
 
	portalnum = USDPAA_BMAN_PORTAL_NUM;
 
	BUG_ON(fd->format != qm_fd_contig);
	buf.hi = fd->addr_hi;
	buf.lo = fd->addr_lo;
retry:
	ret = BspBmanRelease(fd->bpid, &buf, 1, 0, gaptBmanPortal[portalnum]);
	if (ret) {
		BspDpaaPrintf("file:%s, on line:%d,ret = %d\n", __FILE__, __LINE__,ret);
#ifdef POC_BACKOFF
		//cpu_spin(POC_BACKOFF_CYCLES);
#else
		barrier();
#endif
		goto retry;
	}
	BspDpaaPrintf("file:%s, on line:%d, ret = %d, count = %d\n", __FILE__, __LINE__, ret,  BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, 12));
}

unsigned long gdwUdpCount = 0;
unsigned long long int gallRcvUdpTime[100];
unsigned long long int gllStartTime = 0;


/******************************************************************************
* 函数名: BspTBufPull
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/
static inline unsigned char *BspTBufPull(TBuf* ptBuf, unsigned int len)
{
	ptBuf->dwDataSize = ptBuf->dwDataSize - len;
	ptBuf->pucData = ptBuf->pucData + len;
	return (ptBuf->pucData);
}


/******************************************************************************
* 函数名: cb_dqrr_gtpu_udp_rx
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/
static enum qman_cb_dqrr_result cb_dqrr_gtpu_udp_rx(struct qman_portal *qm,
					struct qman_fq *fq,
					const struct qm_dqrr_entry *dqrr)
{
    TBuf* ptBuf;
    TBufCtl *ptBufCtl = NULL;
      
    if(BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, dqrr->fd.bpid) < 100)
    {
        g_tGtpuPktStat.dwbmdepletion++;
	BspDpaaPrintf("recv gtpu udp packet, buf count %d, in pool %d\n", BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, dqrr->fd.bpid), dqrr->fd.bpid);
    }
	
    if((dqrr->fd.format) != qm_fd_contig)
    {
        //g_tGtpuPktStat.dwfdformaterr;
		g_tGtpuPktStat.dwfdformaterr;
	BspDropFrame(&(dqrr->fd));
	BspDpaaPrintf("error, in file:%s, on line:%d\n", __FILE__, __LINE__);
	return qman_cb_dqrr_consume;
    }
	
#if 1	
	ptBuf = (TBuf*)__shmem_ptov((dqrr->fd.addr_lo  - UNIHEAD_SIZE_128BYTES));
	ptBufCtl = (TBufCtl *)((unsigned long)ptBuf - sizeof(TBufCtl));
	
	
	ptBuf->dwDataSize = dqrr->fd.length20;
	ptBuf->pucData = __shmem_ptov(dqrr->fd.addr_lo  + dqrr->fd.offset);
	//ptBuf->pucBufStart = (unsigned char *)((void*)ptBuf + UNIHEAD_SIZE_128BYTES);
	ptBuf->pucEnd = ptBuf->pucData + ptBuf->dwDataSize;
	*(ptBuf->pucEnd) = BMAN_TBUF_CHECKBYTE;
	
	ptBufCtl->dwLine = __LINE__;
	ptBufCtl->wUse = 1;	
	if((BMAN_TBUFCTL_CHECKWORD != (ptBufCtl->checkword)) || ((ptBufCtl->bpid) != (ptBuf->wBpid)))
	{
		g_tGtpuPktStat.dwtbufheadcheckerr++;
	   	BspDpaaPrintf("error, in file:%s, on line:%d, ptBufCtl->checkword = 0x%x, ptBuf->wBpid = %d\n", __FILE__, __LINE__, (unsigned int)ptBufCtl->checkword, (unsigned int)ptBuf->wBpid);
	}
#endif

	g_atBmMoniCount[dqrr->fd.bpid].dwGetSuccess++;
	if(g_pfRecvGmacPacket != NULL)
	{
 
        struct ether_header *prot_eth;
        prot_eth = (struct ether_header *)(ptBuf->pucData);
        ptBuf->dwDataSize = ptBuf->dwDataSize - ETH_HDR_LEN;
        ptBuf->pucData = ptBuf->pucData  + ETH_HDR_LEN;

        switch (prot_eth->ether_type)
        {
           case ETHERTYPE_IP:
           {
               	struct iphdr  *iphdr = (typeof(iphdr))(prot_eth + 1);
            	struct udphdr * udphdr;
            	ptBuf->dwDataSize = ptBuf->dwDataSize - iphdr->ihl;
            	ptBuf->pucData = ptBuf->pucData  + iphdr->ihl;
            	
            	if(IP_PROTOCOL_UDP != iphdr->protocol)
            	{
            		g_tGtpuPktStat.dwl3protocolerr++;
            		BspDpaaPrintf("protocol = 0x%x, not udp packet, in function:%s, on line:%d\n", iphdr->protocol, __FUNCTION__, __LINE__);
            		BspDropFrame(&(dqrr->fd));
            		return qman_cb_dqrr_consume;
            	}
            
            	if(iphdr->frag_off)
            	{
            		g_tGtpuPktStat.dwiphdrerr++;
            		BspDpaaPrintf("frag_off = 0x%x, not udp packet, in function:%s, on line:%d\n", iphdr->frag_off, __FUNCTION__, __LINE__);
            		BspDropFrame(&(dqrr->fd));
            		return qman_cb_dqrr_consume;
            	}
            		
            
            	udphdr = (typeof(udphdr))(ptBuf->pucData);
            	ptBuf->dwDataSize = ptBuf->dwDataSize - sizeof(*udphdr);
            	ptBuf->pucData = ptBuf->pucData  + sizeof(*udphdr);
            
            	if(udphdr->dest != UDP_DEST_GTPU)
            	{
            		g_tGtpuPktStat.dwl4desterr++;
            		BspDpaaPrintf("udp dest port = 0x%x, not gtpu packet, in function:%s, on line:%d\n", udphdr->dest, __FUNCTION__, __LINE__);
            		BspDropFrame(&(dqrr->fd));
            		return qman_cb_dqrr_consume;
            	}
            		
               	break;
           }
        default:
        {
        	g_tGtpuPktStat.dwethertypeerr++;
        	BspDpaaPrintf("ether_type = 0x%x, not ip packet, in function:%s, on line:%d\n", prot_eth->ether_type, __FUNCTION__, __LINE__);
        	BspDropFrame(&(dqrr->fd));
        	return qman_cb_dqrr_consume;
        }
        }
 

		g_tGtpuPktStat.dwcallbackcount++;
		g_pfRecvGmacPacket(ptBuf);
	}
	else
	{
#if 1
		/* 环回 */
		char tmp;
		int i;//,j;
		for(i = 0; i < 6; i++)/* 交换mac地址 */
		{
			tmp = *(ptBuf->pucData + i);
			*(ptBuf->pucData + i) = *(ptBuf->pucData + i + 6) ;
			*(ptBuf->pucData + i + 6)  = tmp;
			
		}
		g_atBmMoniCount[dqrr->fd.bpid].dwRetSuccess++;
		BspQmanEnqueue(gptTx, &(dqrr->fd), 0, gaptQmanPortal[USDPAA_QMAN_PORTAL_NUM]);
		g_tGtpuPktStat.dwloopcnt++;
#else
		//printf("will drop packet, in file:%s, on line:%d\n", __FILE__, __LINE__);
		BspDropFrame(&(dqrr->fd));
#endif
	}
	return qman_cb_dqrr_consume;
}


int g_dwDqrrEmacRx = 0;
static enum qman_cb_dqrr_result CbDqrrEmacRx(struct qman_portal *qm,
					struct qman_fq *fq,
					const struct qm_dqrr_entry *dqrr)
{
	TBuf* ptBuf = NULL;
	//TBufCtl *ptBufCtl = NULL;
	
    printf("%s:recv packet, count= %d, in pool %d\n", __func__,  BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, dqrr->fd.bpid), dqrr->fd.bpid);
	if(BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, dqrr->fd.bpid) < 100)
	{
		BspDpaaPrintf("recv gtpu udp packet, buf count %d, in pool %d\n", BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, dqrr->fd.bpid), dqrr->fd.bpid);
	}
	
	if((dqrr->fd.format) != qm_fd_contig)
	{
		BspDpaaPrintf("error, in file:%s, on line:%d\n", __FILE__, __LINE__);
	}
	
	g_atBmMoniCount[dqrr->fd.bpid].dwGetSuccess++;
	if(g_pfRecvGmacPacket != NULL)
	{
	    g_pfRecvGmacPacket(ptBuf);
	}
	else
	{
        printf("will drop packet, in file:%s, on line:%d\n", __FILE__, __LINE__);
        BspDropFrame(&(dqrr->fd));
	}
	return qman_cb_dqrr_consume;
}

 /* struct qman_fq  *BspQmFqForDeqInit(u32 fqid, enum qm_channel channel, enum qm_wq wq, unsigned long qman_fq_flag, qman_cb_dqrr pfun) */
//extern struct qman_fq  *BspQmFqForDeqInit(u32 fqid, enum qm_channel channel, 

 /******************************************************************************
* 函数名: QmPollTask
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/
int  BspFqGtpuUdpInit(void)
{
	gptfq_gtpu_udp = BspQmFqForDeqInit(FQID_GTPU_UDP, qm_channel_swportal0 + USDPAA_QMAN_PORTAL_NUM, qm_wq_0, QMAN_FQ_FLAG_NO_ENQUEUE, cb_dqrr_gtpu_udp_rx);
	if(gptfq_gtpu_udp == NULL)
	{
		BspDpaaPrintf("BspQmFqForDeqInit failed, in file:%s, on line:%d\n", __FILE__, __LINE__);
		return -1;
	}
	//qman_static_dequeue_add((0x8000 >> 4), gaptQmanPortal[USDPAA_QMAN_PORTAL_NUM]);
	return 0;
}

/******************************************************************************
* 函数名: BspFqMacRxInit
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/                                                 
int  BspFqMacRxInit(void)
{
	gptfq_MacRx = BspQmFqForDeqInit(0x128 , 0x0, qm_wq_3, QMAN_FQ_FLAG_TO_DCPORTAL/*QMAN_FQ_FLAG_NO_ENQUEUE*/, CbDqrrEmacRx);

	if(gptfq_MacRx == NULL)
	{
		BspDpaaPrintf("BspQmFqForDeqInit failed, in file:%s, on line:%d\n", __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

/******************************************************************************
* 函数名: cb_ern_fortx
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/
static void cb_ern_fortx(struct qman_portal *qm, struct qman_fq *fq,
				const struct qm_mr_entry *msg)
{
	BspDpaaPrintf("enqueue error, fq->fqid = %d \n", fq->fqid);
	BspDropFrame(&msg->ern.fd);
	BspDpaaPrintf("there is %d bufs, in buf pool%d\n", BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, msg->ern.fd.bpid), msg->ern.fd.bpid);
}

#define CHANNEL_TX(n)	((qm_channel_fman0_sp0 + 0x20 * ((n) / 5)) + ((n) + 1) % 5)


extern struct qman_fq *BspQmFqForEnqInit(u32 fqid, enum qm_channel channel, 
                                                     enum qm_wq wq, unsigned long qman_fq_flag, qman_cb_mr pfun);
/******************************************************************************
* 函数名: dpadelay
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/
int  BspTxFqInit(int portnum)
{
    switch(portnum)
    {
        case 5:
		{
	        gptTx = BspQmFqForEnqInit(0, 0x41,qm_wq_3, QMAN_FQ_FLAG_TO_DCPORTAL, cb_ern_fortx);    
            break;
        }
	    case 6:
		{
	        gptTx = BspQmFqForEnqInit(0, 0x42,qm_wq_3, QMAN_FQ_FLAG_TO_DCPORTAL, cb_ern_fortx);    
            break;
        }
		default:
		    break;
    }
	if(gptTx == NULL)
	{
        BspDpaaPrintf("BspQmFqForEnqInit failed, in file:%s, on line:%d\n", __FILE__, __LINE__);
	
		return -1;
	}
	gaptTxFq[portnum] = gptTx;
    return 0;
}


#if 0
char gaucPacket[1500] = {
    0x44, 0x37, 0xe6, 0xc7, 0xde, 0xcd, /* DST MAC */
    0x00, 0xe0, 0x0c, 0x00, 0xea, 0x03, /* SRC MAC */
    0x08, 0x00,
    0x45, 0x00, 0x00, 0x3c, 0xbf, 0xdc, 0x00, 0x00, 0x80, 0x06,
    0x0, 0x0,  /* checksum */
    0xa9, 0xfe, 0xcc, 0xac, /* SRC IP */
    0xa9, 0xfe, 0xcc, 0xaa, /* DST IP */
    0x08, 0x00, 0x47, 0x5c, 0x05, 0x00, 0x00, 0x00, /* Date */
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, /* Date */
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, /* Date */
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61, /* Date */
    0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, /* Date */
};
#else

char gaucPacket[1500] = {
    0x1c, 0x6f, 0x65, 0x0b, 0x19, 0x2f, /* DST MAC */
    0x00, 0xe0, 0x0c, 0x03, 0xea, 0x03, /* SRC MAC */
    0x08, 0x00,
    0x45, 0x00, 0x00, 0x3c, 0xbf, 0xdc, 0x00, 0x00, 0x80, 0x06,
    0x0, 0x0,  /* checksum */
    0xac, 0x10, 0x18, 0xd4, /* SRC IP */
    0xac, 0x10, 0x18, 0x39, /* DST IP */
    0x08, 0x00, 0x47, 0x5c, 0x05, 0x00, 0x00, 0x00, /* Date */
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, /* Date */
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, /* Date */
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61, /* Date */
    0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, /* Date */
};

#endif

void dpadelay(unsigned int count)
{
	volatile unsigned  int i;
      for(i = 0; i < count; i++);
}

/******************************************************************************
* 函数名: BspUpIpSend
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/
extern int g_bufslo;
int BspUpIpSend(TBuf* ptBuf)
{
    struct qm_fd qmfd;
    int reval;
    //TBufCtl *ptBufCtl = NULL;
	#if 1
    if((ptBuf == NULL)  || ((ptBuf->pucEnd) == NULL)  || (BMAN_TBUF_CHECKBYTE != (*(ptBuf->pucEnd))) || (IP_MIN_SIZE >  (ptBuf->dwDataSize)) || (NULL == gptTx))
    {
        g_tGtpuPktStat.dwinputerr++;
        /*  内存谁来释放 */
	BspDpaaPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
	return -1;
    }
	#endif

#ifdef EMAC_GLOBAL_PORT_TX_BAKE
	extern unsigned char BspGetMchMaster(void);
	BspMCHSWitch((unsigned int) BspGetMchMaster());
#endif

    memcpy( (void *)(ptBuf->pucData), (const void *)gaucPacket, 1400);
    qmfd.cmd = 0x70000000;
	qmfd.bpid = ptBuf->wBpid;
	qmfd.addr_lo = (u32)(g_bufslo);//__shmem_vtop(ptBuf->pucBufStart);
	qmfd.addr_hi = 0;
	qmfd.format = qm_fd_contig;
	qmfd.offset = UNIHEAD_SIZE_128BYTES;//(ptBuf->pucData - ptBuf->pucBufStart);
	qmfd.length20 = ptBuf->dwDataSize;

	
	reval = BspQmanEnqueue(gptTx, &qmfd, 0, gaptQmanPortal[USDPAA_QMAN_PORTAL_NUM]);
	if(0 != reval)
	{
		g_tGtpuPktStat.dwenqueueerr++;
	}
	else
	{
		/* 业务多线程的话需要互斥 */
		//ptBufCtl->wUse = 0;	
		g_tGtpuPktStat.dweqsuccess++;
	}
	return reval;
}

int g_dwFqTxSendCount = 0;
int g_dwEmacSendBpid = 0;
extern u8 *g_u8ShareMem;

TBuf  *ptBuf;	

extern unsigned long g_sharephyaddr;

int BspEmacDataSendTest()
{

    struct bm_buffer buf;
    struct qm_fd qmfd;
    int tmp;
    int ret = 0;
    unsigned char *pucPacket = NULL;

	TBuf  *ptBuf = NULL;
    int i;
    int count;
    struct qm_mcr_queryfq_np np;
    int bmportalnum = USDPAA_BMAN_PORTAL_NUM;
    int qmportalnum = USDPAA_QMAN_PORTAL_NUM;
    g_dwusdpaaflag = __LINE__;
   
	while(1)//for(i = 0; i < 2; )   /*  网口报文发送测试 */
    {
		ptBuf  = BspGetTBuf(1400, tbuf_mem_type0, __LINE__, __FUNCTION__);
		ptBuf->dwDataSize = 1400;
		//printf("hahaha--0!\n");
		//memcpy(ptBuf->pucData, gaucPacket + 14, 100);
		
		g_dwusdpaaflag = __LINE__;
		//printf("hahaha--1!\n");		
		if(NULL == ptBuf)
		{
			count = BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, 12);
			BspDpaaPrintf("file:%s, on line:%d, ptBuf = %p count = %d\n", __FILE__, __LINE__, ptBuf, count);
			continue;		
		}
		g_dwusdpaaflag = __LINE__;
		//printf("hahaha--2!\n");

		ret =  BspUpIpSend(ptBuf);
		if(0 != ret)
		{
            BspRetTbuf(ptBuf);
		}
		if(0 != ret)
		{
			count = BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, 12);
			BspDpaaPrintf("file:%s, on line:%d, ret = %d, ptBuf = %p, count = %d, i =%d\n", __FILE__, __LINE__, ret, ptBuf, count, i);
		}

		i++;
    }

}
unsigned char *gpucCCSR = NULL;
#ifdef __QM_ISR_CYCLE__
unsigned long  g_adwQmIsrCount = 0;
unsigned long  g_adwQmIsrCycles[100];
extern  unsigned int BSP_CLK_GetCycle();

#define QM_IRS_CYCLE_START do \
{ \
    if (g_adwQmIsrCount < 100) \
        g_adwQmIsrCycles[g_adwQmIsrCount] = BSP_CLK_GetCycle(); \
}while(0);

#define QM_IRS_CYCLE_END do \
{ \
    if (g_adwQmIsrCount < 100) \
    { \
        g_adwQmIsrCycles[g_adwQmIsrCount] = (BSP_CLK_GetCycle()) - g_adwQmIsrCycles[g_adwQmIsrCount]; \
        g_adwQmIsrCount++; \
    } \
}while(0);
#else
#define QM_IRS_CYCLE_START
#define QM_IRS_CYCLE_END
#endif

unsigned long g_dwCurrentBuffID = 0;
unsigned long g_dwQmPollTaskCnt = 0;

/******************************************************************************
* 函数名: QmPollTask
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/
int  QmPollTask()
{
	//int ret;
	int portalnum;
	portalnum = USDPAA_QMAN_PORTAL_NUM;
	int fd_usdpaa = -1;
	BspDpaaPrintf("coming in QmPollTask\n");
	fd_usdpaa = open("/dev/usdpaa", O_RDWR);
	if (fd_usdpaa< 0) 
	{
		BspDpaaPrintf("can't open /dev/usdpaa device");
		return -ENODEV;
	}
	while(1)
	{
		g_dwQmPollTaskCnt++;
		ioctl(fd_usdpaa, USDPAA_IOC_WAITQMINT, NULL);
		//ioctl(fd_usdpaa, USDPAA_IOC_WAITQM9INT, NULL);
		QmPortalIsr(gaptQmanPortal[portalnum]);
        BspEnQmInt(portalnum);
	}
}

/******************************************************************************
* 函数名: BspDpaaAppInit
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
******************************************************************************/ 
int BspDpaaAppInit(void)
{
    int ret = 0;
    int bmportalnum;
    int qmportalnum;
    int iport=0;
	bmportalnum = USDPAA_BMAN_PORTAL_NUM;
    qmportalnum = USDPAA_QMAN_PORTAL_NUM;
    pthread_t  qmpoll_thread;
    pthread_attr_t attr;
    struct sched_param param;
    system("mknod /dev/usdpaa c 248 0");
    system("mknod /dev/usdpaa_cinh  c 249 0");
	system("mknod /dev/fm0 c 253 0");
    system("mknod /dev/fm0-pcd c 253 1");
    ret = BspDpaShmemSetup();
    if(0 != ret)
    {
        g_dwusdpaaflag = __LINE__;
	    BspDpaaPrintf("BspDpaShmemSetup failed, in file:%s, on line:%d, ret = %d\n", __FILE__, __LINE__, ret);
	    return ret;
    }
    for (iport=0;iport<4;iport++)
    {
        ret = BspQmanPortalInit(qmportalnum+iport, 0);
    }
    g_dwusdpaaflag = __LINE__;
    if(0 != ret)
    {
        g_dwusdpaaflag = __LINE__;
        BspDpaaPrintf("BspQmanPortalInit failed, in file:%s, on line:%d, ret = %d\n", __FILE__, __LINE__, ret);
	    return ret;
    }    
   // while(1);
    g_dwusdpaaflag = __LINE__;
    //bm_set_memory(0, 0x79000000,0, 21);
    for (iport=0;iport<4;iport++)
    {
        ret = BspBmanPortalInit(bmportalnum+iport,0);
    }
    g_dwusdpaaflag = __LINE__;
    if(0 != ret)
    {
        g_dwusdpaaflag = __LINE__;
	    BspDpaaPrintf("BspBmanPortalInit failed, in file:%s, on line:%d, ret = %d\n", __FILE__, __LINE__, ret);
	    return ret;
    }

	#if 1
	ret = BspTxFqInit(EMAC_GLOBAL_PORT_TX);
	//ret = BspTxFqInit(EMAC_GLOBAL_PORT_TX+1);
    //printf("need to add fq init--0\n");
    #endif

    #if 0
	printf("now loading BspFqMacRxInit!\n");
    /*******************************************  网口接收fq 初始化 *****************************/
    ret = BspFqMacRxInit();
    if(0 != ret)
    {
        g_dwusdpaaflag = __LINE__;
	    printf("BspBmanPortalInit failed, in file:%s, on line:%d, ret = %d\n", __FILE__, __LINE__, ret);
	return ret;
    }
    //printf("need to add fq init--1\n");
    #endif

    /*******************************************  网口接收fq 初始化 *****************************/

	//BspQmFqForDeqInit
    #if 0
    /*******************************************  网口发送fq 初始化 *****************************/
    gptTx = BspQmFqForEnqInit(FQID_MACTX_CTRL_TEST, CHANNEL_TX( EMAC_GLOBAL_PORT_TX), qm_wq_0, QMAN_FQ_FLAG_TO_DCPORTAL, cb_ern_fortx);    
    if(gptTx == NULL)
    {
        BspDpaaPrintf("BspQmFqForEnqInit failed, in file:%s, on line:%d\n", __FILE__, __LINE__);
	 return -1;
    }
	#endif
	
	 //printf("need to add fq init--2\n");
    /*******************************************  网口发送fq 初始化 *****************************/
    /* 设置QmPollTask线程的调度策略和优先级: SCHED_FIFO, 99 */
#if 0
    pthread_attr_init(&attr); 
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 99; 
    pthread_attr_setschedparam(&attr, &param);
    
    ret = pthread_create(&qmpoll_thread, &attr, (void*)QmPollTask, NULL);
    if (ret != 0)
    {
        BspDpaaPrintf("pthread_create qmpoll_thread failed\n");
        pthread_attr_destroy(&attr);
        return (-1);
    }
    pthread_attr_destroy(&attr);
    BspDpaaPrintf("wait to be add request irq,in file:%s, on line:%d\n", __FILE__, __LINE__);
    ret = BspEnQmInt(qmportalnum);
    if(0 != ret)
    {
        BspDpaaPrintf("BspEnQmInt failed, in file:%s, on line:%d, ret = %d\n", __FILE__, __LINE__, ret);
	 return ret;
    }
#endif
    BspDpaaPrintf("BspUsDpaaInit done\n");
    g_dwDpaaInitDone = 1;
    return ret;
}


/******************************************************************************
* 函数名: BspDpaaInit
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
* 作者:刘刚
cpu---ethsw1	          fm0 dTSEC1  eth0
cpu---ethse2	          fm0 dTSEC2  eth1
cpu---debug	port    fm0 dTSEC3  eth2
cpu---core net         fm0 dTSEC5  eth3

*******************************************************************************/
int BspDpaaInit(void)
{
    extern unsigned long  BspGetDataNetMac(unsigned char * pucAddr);
    BspGetDataNetMac(g_aucUpEmacHeader + 6);
    return BspDpaaAppInit();
}


/*
Rx       eth1   eth2    eth3   eth4  eth5  eth6  eth7  eth8  10geth1 10geth2

PORT   0x8    0x9     0xa    0xb   0xc   0xd   0xe   0xf   0x10       0x11


Tx       eth1    eth2     eth3     eth4    eth5   eth6    eth7    eth8  10geth1 10geth2

PORT   0x28   0x29     0x2a    0x2b   0x2c   0x2d   0x2e   0x2f   0x30       0x31


fm --0
*/
int BspBmiRegPrint(unsigned int  port, int fm)
{
	unsigned long Fm_Offset;
	if(fm == 0)
		Fm_Offset = CCSR_FM1_OFFSET;
	if(fm == 1)
		Fm_Offset = CCSR_FM2_OFFSET;
	if((fm > 1) || (fm < 0))
		return -1;
	
	BspDpaaPrintf("BMI Common Registers:\n");
	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));

	BspDpaaPrintf("BMI Rx Port%d  Registers:\n", port);
	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));



	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));



	 
     //ETH_DESEC1
	 BspDpaaPrintf("ETH_DESEC1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RBYT))));
     //ETH_DESEC2
	 BspDpaaPrintf("ETH_DESEC2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RBYT))));
     //ETH_DESEC3
	 BspDpaaPrintf("ETH_DESEC3->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RBYT))));
     //ETH_DESEC4
	 BspDpaaPrintf("ETH_DESEC4->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RBYT))));
     //ETH_DESEC5
	 BspDpaaPrintf("ETH_DESEC5->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RBYT))));


	 
	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));

	 
	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));

	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));

     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));

     return 0;
}

int BspBmiRegSet(unsigned int  port, int fm)
{
	unsigned long Fm_Offset;
	if(fm == 0)
		Fm_Offset = CCSR_FM1_OFFSET;
	if(fm == 1)
		Fm_Offset = CCSR_FM2_OFFSET;
	if((fm > 1) || (fm < 0))
		return -1;
	
	BspDpaaPrintf("BMI Common Registers:\n");



	BspDpaaPrintf("BMI Rx Port%d  Registers:\n", port);
	
	
	(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))) = 0x440000;/* next en parser */
	BspDpaaPrintf("FMBM_RFNE:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));

	(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))) = 0x480000;
	BspDpaaPrintf("FMBM_RFPNE:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
	
	(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))) = 0xD40000;
	BspDpaaPrintf("FMBM_RFENE:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
	
	return 0;
}



/**********************************************************************
* 函数名称：BspParserRegShow
* 功能描述：
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
* 2011/05/10    V1.0           
************************************************************************/
int BspParserRegShow(unsigned int  port)
{
	unsigned long Fm_Offset;
	Fm_Offset = CCSR_FM1_OFFSET;
	
	BspDpaaPrintf("Parser Port%d  Registers:\n", port);
	BspDpaaPrintf("FMPR_PxCAC:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCAC))));
	BspDpaaPrintf("FMPR_PxCTPID:reg->0x%lx ,0x%x\n", (unsigned long)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCTPID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCTPID))));



	BspDpaaPrintf("Parser Global Configuration Registers:\n");
	
	BspDpaaPrintf("FMPR_PARSE_MEM:reg->0x%lx ,0x%x\n", (unsigned long)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PARSE_MEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PARSE_MEM))));
	
		
	BspDpaaPrintf("FMPR_PEVR:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PEVR))));
	BspDpaaPrintf("FMPR_PEVER:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET +  FMPR_PEVER))));
	BspDpaaPrintf("FMPR_PERR:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PERR))));
	BspDpaaPrintf("FMPR_PERER:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PERER))));
	BspDpaaPrintf("FMPR_PPSC:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PPSC))));
	BspDpaaPrintf("FMPR_PDS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PDS))));

    BspDpaaPrintf("FMPR_L2RRS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L2RRS))));
    BspDpaaPrintf("FMPR_L3RRS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L3RRS))));
    BspDpaaPrintf("FMPR_L4RRS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L4RRS))));
    BspDpaaPrintf("FMPR_SRRS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SRRS))));
    BspDpaaPrintf("FMPR_L2RRES:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L2RRES))));
    BspDpaaPrintf("FMPR_L3RRES:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L3RRES))));
    BspDpaaPrintf("FMPR_L4RRES:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L4RRES))));
    BspDpaaPrintf("FMPR_SRRES:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SRRES))));
    BspDpaaPrintf("FMPR_SPCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SPCS))));
    BspDpaaPrintf("FMPR_SPSCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SPSCS))));
    BspDpaaPrintf("FMPR_HXSCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_HXSCS))));
    BspDpaaPrintf("FMPR_MRCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MRCS))));
    BspDpaaPrintf("FMPR_MWCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MWCS))));
    BspDpaaPrintf("FMPR_MRSCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MRSCS))));
    BspDpaaPrintf("FMPR_MWSCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MWSCS))));
	
	return 0;
}

/*
port 0x1...0x7,0x8..0xb,0x28..0x2b,0x30,0x31

*/
int BspQmiRegPrint(unsigned int  port, int fm)
{
#if 0
	unsigned long Fm_Offset;
	if(fm == 0)
		Fm_Offset = CCSR_FM1_OFFSET;
	if(fm == 1)
		Fm_Offset = CCSR_FM2_OFFSET;
	if((fm > 1) || (fm < 0))
		return -1;
	
	BspDpaaPrintf("QMI Common Registers:\n");
	
	BspDpaaPrintf("FMQMI_GC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_GC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_GC))));
	BspDpaaPrintf("FMQMI_EIE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET  + FMQMI_EIE))));
	BspDpaaPrintf("FMQMI_EIEN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIEN))));
	BspDpaaPrintf("FMQMI_EIF->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIF),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIF))));
	BspDpaaPrintf("FMQMI_IE->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET    + FMQMI_IE))));
	BspDpaaPrintf("FMQMI_IEN->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET +  + FMQMI_IEN))));



    BspDpaaPrintf("FMQMI_IF->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQMI_IF),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_IF))));
    BspDpaaPrintf("FMQM_GS->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQM_GS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_GS))));
    BspDpaaPrintf("FMQM_ETFC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_ETFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_ETFC))));
    BspDpaaPrintf("FMQM_DTFC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTFC))));
    BspDpaaPrintf("FMQM_DC0->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC0),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC0))));
    BspDpaaPrintf("FMQM_DC1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC1))));
    BspDpaaPrintf("FMQM_DC2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC2))));
    BspDpaaPrintf("FMQM_DC3->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC3),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC3))));
    BspDpaaPrintf("FMQM_TAPC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_TAPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_TAPC))));
    BspDpaaPrintf("FMQM_DMCVC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DMCVC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DMCVC))));



    BspDpaaPrintf("FMQM_DIFDCC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + FMQM_DIFDCC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DIFDCC))));
    BspDpaaPrintf("FMQM_DA1VC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DA1VC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DA1VC))));
    BspDpaaPrintf("FMQM_DTRC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTRC))));

    BspDpaaPrintf("FMQM_EFDDD->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_EFDDD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_EFDDD))));
    BspDpaaPrintf("FMQM_DTCA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA1))));
    BspDpaaPrintf("FMQM_DTVA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA1))));
    BspDpaaPrintf("FMQM_DTMA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA1))));

    BspDpaaPrintf("FMQM_DTCA->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA))));


    BspDpaaPrintf("FMQM_DTCA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA2))));
    BspDpaaPrintf("FMQM_DTVA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA2))));
    BspDpaaPrintf("FMQM_DTMA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA2))));
#if 0
    BspDpaaPrintf("QMI Rx Port%d  Registers:\n", port);
	
	BspDpaaPrintf("FMQM_PNC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET  + FMQM_PNC))));
	BspDpaaPrintf("FMQM_PNS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET  + FMQM_PNS))));

    BspDpaaPrintf("FMQM_PNTS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNTS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNTS))));
    BspDpaaPrintf("FMQM_PNEN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNEN))));
    BspDpaaPrintf("FMQM_PNETFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNETFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNETFC))));
    BspDpaaPrintf("FMQM_PNDN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDN))));
    BspDpaaPrintf("FMQM_PNDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDC))));
    BspDpaaPrintf("FMQM_PNDTFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDTFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDTFC))));
    BspDpaaPrintf("FMQM_PNDFNOC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDFNOC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDFNOC))));
    BspDpaaPrintf("FMQM_PNDCC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDCC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDCC))));
#endif
#endif
    return 0;
}


void BspEthPortRegShow(void)
{
    int i = 0;
	unsigned long Fm_Offset;
	int port;
	Fm_Offset = CCSR_FM1_OFFSET;
	printf("TESEC5's reg\n");
	d4 ((CCSR_VIRTADDR_BASE + ETH_DESEC5),0x1000);
	
	BspDpaaPrintf("BMI Common Registers:\n");
	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));
	BspDpaaPrintf("FMBM_GDE->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_GDE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_GDE))));
	for (i=1;i<64;i++)
	{
	    BspDpaaPrintf("FMBM_PP[%d]->0x%x value:0x%x\n", i,(unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)))));
	   // BspDpaaPrintf("FMBM_PFS[%d]->0x%x value:0x%x\n", i,(unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)))));
	    BspDpaaPrintf("FMBM_SPLIODN[%d]->0x%x value:0x%x\n", i,(unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_SPLIODN(i)),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_SPLIODN(i)))));
	}
    port =0xc;
    BspDpaaPrintf("BMI Rx Port%d  Registers:\n", port);
	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));

	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c))));


	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c))));


	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c))));


	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));

	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));

	 
	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));

	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));

     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));


	
    port =0x2c;
    BspDpaaPrintf("BMI Tx Port%d  Registers:\n", port);
	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));

	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c))));


	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b))));
	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c))));


	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b))));
	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c))));


	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));

	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));

	 
	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));

	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));

     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
}

uint32_t bsp_get_recv_pkts(int port)
{
	u32 desec_offset;

	switch (port) {
	case 0:
		desec_offset = ETH_DESEC1;
		break;
	case 1:
		desec_offset = ETH_DESEC2;
		break;
	case 2:
		desec_offset = ETH_DESEC3;
		break;
	case 3:
		desec_offset = ETH_DESEC5;
		break;
	default:
		printf("%s error port %d\n", __func__, port);
		return 0;
	}

	return *(uint32_t *)(CCSR_VIRTADDR_BASE + desec_offset + ETH_MAC_RPKT);
}

uint32_t bsp_get_recv_bytes(int port)
{
	u32 desec_offset;

	switch (port) {
	case 0:
		desec_offset = ETH_DESEC1;
		break;
	case 1:
		desec_offset = ETH_DESEC2;
		break;
	case 2:
		desec_offset = ETH_DESEC3;
		break;
	case 3:
		desec_offset = ETH_DESEC5;
		break;
	default:
		printf("%s error port %d\n", __func__, port);
		return 0;
	}

	return *(uint32_t *)(CCSR_VIRTADDR_BASE + desec_offset + ETH_MAC_RBYT);
}

void BspEthInfoShow(unsigned int  port)
{
#if 1
	unsigned long Fm_Offset;
	Fm_Offset = CCSR_FM1_OFFSET;
    BspDpaaPrintf("MAC 1-5 INFO LIST!\n");
	BspDpaaPrintf("DESEC1'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RBYT))));	
	BspDpaaPrintf("DESEC1'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RPKT))));
    BspDpaaPrintf("DESEC1'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_TBYT))));
    BspDpaaPrintf("DESEC1'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_TPKT))));


	BspDpaaPrintf("DESEC2'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RBYT))));
	BspDpaaPrintf("DESEC2'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RPKT))));
    BspDpaaPrintf("DESEC2'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_TBYT))));
    BspDpaaPrintf("DESEC2'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_TPKT))));

	BspDpaaPrintf("DESEC3'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RBYT))));
	BspDpaaPrintf("DESEC3'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RPKT))));
    BspDpaaPrintf("DESEC3'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_TBYT))));
    BspDpaaPrintf("DESEC3'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_TPKT))));

	BspDpaaPrintf("DESEC4'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RBYT))));
	BspDpaaPrintf("DESEC4'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RPKT))));
    BspDpaaPrintf("DESEC4'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_TBYT))));
    BspDpaaPrintf("DESEC4'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_TPKT))));

	BspDpaaPrintf("DESEC5'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RBYT))));
	BspDpaaPrintf("DESEC5'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RPKT))));
    BspDpaaPrintf("DESEC5'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_TBYT))));
    BspDpaaPrintf("DESEC5'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_TPKT))));

   
    
	BspDpaaPrintf("BMI Common Registers:\n");
	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));

	BspDpaaPrintf("BMI Rx Port%d  Registers:\n", port);
	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));



	BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
	BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
	BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));	 
	BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RBFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
	 
	BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RLFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
	BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
	BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
	BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
	BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RODC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
	BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RBDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
	BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
	BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPCP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
	 
	BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
	BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
	BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_RRQUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC))));
	BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
	BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
	BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));

    BspDpaaPrintf("QMI Common Registers:\n");

	BspDpaaPrintf("FMQMI_GC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_GC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_GC))));
	BspDpaaPrintf("FMQMI_EIE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET  + FMQMI_EIE))));
	BspDpaaPrintf("FMQMI_EIEN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIEN))));
	BspDpaaPrintf("FMQMI_EIF->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIF),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIF))));
	BspDpaaPrintf("FMQMI_IE->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET    + FMQMI_IE))));
	BspDpaaPrintf("FMQMI_IEN->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET +  + FMQMI_IEN))));



    BspDpaaPrintf("FMQMI_IF->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQMI_IF),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_IF))));
    BspDpaaPrintf("FMQM_GS->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQM_GS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_GS))));
    BspDpaaPrintf("FMQM_ETFC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_ETFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_ETFC))));
    BspDpaaPrintf("FMQM_DTFC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTFC))));
    BspDpaaPrintf("FMQM_DC0->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC0),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC0))));
    BspDpaaPrintf("FMQM_DC1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC1))));
    BspDpaaPrintf("FMQM_DC2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC2))));
    BspDpaaPrintf("FMQM_DC3->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC3),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC3))));
    BspDpaaPrintf("FMQM_TAPC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_TAPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_TAPC))));
    BspDpaaPrintf("FMQM_DMCVC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DMCVC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DMCVC))));



    BspDpaaPrintf("FMQM_DIFDCC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + FMQM_DIFDCC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DIFDCC))));
    BspDpaaPrintf("FMQM_DA1VC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DA1VC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DA1VC))));
    BspDpaaPrintf("FMQM_DTRC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTRC))));

    BspDpaaPrintf("FMQM_EFDDD->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_EFDDD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_EFDDD))));
    BspDpaaPrintf("FMQM_DTCA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA1))));
    BspDpaaPrintf("FMQM_DTVA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA1))));
    BspDpaaPrintf("FMQM_DTMA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA1))));

    BspDpaaPrintf("FMQM_DTCA->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA))));


    BspDpaaPrintf("FMQM_DTCA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA2))));
    BspDpaaPrintf("FMQM_DTVA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA2))));
    BspDpaaPrintf("FMQM_DTMA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA2))));

    BspDpaaPrintf("QMI Rx Port%d  Registers:\n", port);

    BspDpaaPrintf("FMQM_PNC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNC))));
    BspDpaaPrintf("FMQM_PNS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNS))));

    BspDpaaPrintf("FMQM_PNTS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNTS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNTS))));
    BspDpaaPrintf("FMQM_PNEN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNEN))));
    BspDpaaPrintf("FMQM_PNETFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	 + FMQM_PNETFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNETFC))));
    BspDpaaPrintf("FMQM_PNDN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDN))));
    BspDpaaPrintf("FMQM_PNDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDC))));
    BspDpaaPrintf("FMQM_PNDTFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	 + FMQM_PNDTFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDTFC))));
    BspDpaaPrintf("FMQM_PNDFNOC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDFNOC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDFNOC))));
    BspDpaaPrintf("FMQM_PNDCC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	+ FMQM_PNDCC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDCC))));


    BspDpaaPrintf("BMAN   Registers:\n");

    //BspDpaaPrintf( (*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET + BMAN_POOL0_CONTENT_OFFSET + (bpid <<2))));
#endif
}
/**************************************************************************//**
 @Description   structure for returning revision information
*//***************************************************************************/
typedef struct ioc_fm_revision_info_t {    
unsigned char          major;               /**< Major revision */    
unsigned char         minor;               /**< Minor revision */
} ioc_fm_revision_info_t;
/**************************************************************************//**
 @Description   struct for defining Dual Tx rate limiting scale
*//***************************************************************************/
typedef enum fm_port_dual_rate_limiter_scale_down {    
e_IOC_FM_PORT_DUAL_RATE_LIMITER_NONE = 0,           /**< Use only single rate limiter  */    
e_IOC_FM_PORT_DUAL_RATE_LIMITER_SCALE_DOWN_BY_2,    /**< Divide high rate limiter by 2 */    
e_IOC_FM_PORT_DUAL_RATE_LIMITER_SCALE_DOWN_BY_4,    /**< Divide high rate limiter by 4 */    
e_IOC_FM_PORT_DUAL_RATE_LIMITER_SCALE_DOWN_BY_8     /**< Divide high rate limiter by 8 */
} fm_port_dual_rate_limiter_scale_down;

/**************************************************************************//**
 @Description   struct for defining Tx rate limiting
*//***************************************************************************/
typedef struct ioc_fm_port_rate_limit {    unsigned 
short                            max_burst_size;         /**< in KBytes for Tx ports, in frames for offline parsing ports. (note that for early chips burst size is   rounded up to a multiply of 1000 frames).*/    
unsigned int                            rate_limit;             /**< in Kb/sec for Tx ports, in frame/sec for offline parsing ports. Rate limit refers to data rate (rather than line rate). */    
fm_port_dual_rate_limiter_scale_down rate_limit_divider;    /**< For offline parsing ports only. Not-valid for some earlier chip revisions */
} ioc_fm_port_rate_limit_t;


#define NCSW_IOC_TYPE_BASE          0xe0    /**< defines the IOCTL type for all the NCSW Linux module commands */
#define FM_IOC_TYPE_BASE            (NCSW_IOC_TYPE_BASE+1)
#define FMT_IOC_TYPE_BASE           (NCSW_IOC_TYPE_BASE+3)
#define FM_IOC_NUM(n)      n
#define FM_PCD_IOC_NUM(n)   (n+20)
#define FM_PORT_IOC_NUM(n)  (n+50)
#define FM_IOC_GET_REVISION                                    _IOR_SHMEM(FM_IOC_TYPE_BASE, FM_IOC_NUM(3), ioc_fm_revision_info_t)
#define FM_PORT_IOC_SET_RATE_LIMIT                   _IOW_SHMEM(FM_IOC_TYPE_BASE, FM_PORT_IOC_NUM(3), ioc_fm_port_rate_limit_t)

#define DEVNAME_TOMCH0  "/dev/fm1-port-tx0"
#define DEVNAME_TOMCH1  "/dev/fm1-port-tx1"

