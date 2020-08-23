#ifdef __cplusplus
extern "C" {
#endif

#include "ui.h"


extern NET_POOL_ID _pNetDpool; 

static void *pMuxCookie;
static SEM_ID semB;
static SEM_ID semArp;
static SEM_ID semRng;
static SEM_ID semSock;
static RING_ID ring;
static socketT sockTbl[UI_MAX_HANDLE+1]={};
static int initialized=FALSE;

static BYTE  bMac[ETH_ALEN];

static DWORD dwMask;
static DWORD dwGateway;
TCTA_STAT_UIIP stStatAbnmlIp;
DWORD dwSagIp = 0;
struct sockaddr_in stSagAddr;
DWORD dwSagIp1 = 0;
struct sockaddr_in stSagAddr1;

UDP_STATS   udp_status;
LOCAL BOOL muxRecvRtn
    (
    void * pCookie,       	/* EndObj * */
    long  type,			/* network service type */
    M_BLK_ID pMblk,		/* MAC frame */
    LL_HDR_INFO * pLinkHdrInfo,	/* link level information */
    void * pSpare		/* pSpare supplied during muxBind */
    );
LOCAL STATUS muxShutdownRtn
    (
    void* cookie,
    void * pSpare
    );
LOCAL STATUS muxTxRestartRtn
    (
    void * cookie,
    void * pSpare
    );
LOCAL void muxErrorRtn
    (
    END_OBJ * cookie,
    END_ERR *endErr,
    void * pSpare
    );
static void uiTask(void);
static void uiTimer(void);
#if 1
   /* Free ARP Records List methods*/
extern  void    initARP();
 extern void   InitFreeARPList();
 extern void   InsertFreeARPList(UINT16 index);
extern  UINT16 GetFreeARPEntryIdxFromList();


   	/*ARP的操作*/
extern 	BOOL   ARPBPtreeAdd(UINT32 ip, UINT16 index);
extern 	BOOL  ARPBPtreeDel(UINT32 ip);
extern	UINT16 ARPBPtreeFind(UINT32 ip);

extern	BOOL   ARPAddEntry(UINT32 ip, newarpTableT* p);
extern	BOOL   ARPDelEntry(UINT32 ip);
extern      BOOL  ARPUpdateEntry(UINT32 ip, newarpTableT* p);
extern	newarpTableT* GetARPEntryByIdx(UINT16 index);
extern	void   ARPBPtreeExpire();
#endif

LOCAL int updateArpTblNew(newarpTableT *arp,unsigned char flag,unsigned int target_ip);
/* arp */
LOCAL int initArpTbl(void);
LOCAL int insertArpTbl(arpTableT* p);
LOCAL int removeArpTbl(arpTableT* p);
LOCAL int updateArpTbl(arpTableT *arp,unsigned char flag,unsigned int target_ip);
LOCAL int arpResolve(M_BLK_ID pMblk, UINT8*ucDstBtsMac,DWORD dwIP);
LOCAL arpTableT* allocArpBlk(void);
LOCAL int freeArpBlk(arpTableT *arp);
LOCAL void arpTimer(void);
LOCAL int sendArpPkt(DWORD ip);
u_short ui_checksum(u_short *pAddr, int len);

#include "m2Lib.h"

/*void dspRtpEventHandler ();*/

struct	ipq	my_ipq;			/* ip reass. queue */
/*u_short	my_ip_id;				 ip packet ctr, for ids */

int   m_myPort, m_peerPort;
unsigned long   m_peerIp;
unsigned char   m_txBuffer[3000];
int udp_handle;

int             m_tick;
MSG_Q_ID        m_msgQ;
WDOG_ID         m_wd;
unsigned char   m_buffer[20];

unsigned char m_rxBuffer[3000];
unsigned char m_ipTimeCount;
extern char *	bootStringToStruct (char *bootString, BOOT_PARAMS *pBootParams);

/*******************************************************************************
*
* ipReAssemble - reassemble ip fragments
*
* This function reassembles the ip fragments and returns the pointer to the
* mbuf chain if reassembly is complete orelse returns null.
*
* RETURNS: pMbuf/NULL
*
* SEE ALSO: ,
*
* NOMANUAL
*/

LOCAL struct mbuf * ipReAssemble
    (
    FAST struct mbuf *	pMbuf,		/* pointer to mbuf chain fragment */
    FAST struct ipq *	pIpFragQueue	/* pointer to ip fragment queue */
    )
{
    FAST struct mbuf ** 	pPtrMbuf; 	   /* pointer to ptr to mbuf */
    FAST struct mbuf * 		pMbPktFrag = NULL; /* pointer to the packet */
    FAST struct ip * 		pIpHdr;   	   /* pointer to ip header */
    FAST struct ipasfrag *	pIpHdrFrag = NULL; /* ipfragment header */
    FAST int		        len; 
    FAST struct mbuf *		pMbufTmp;	   /* pointer to mbuf */
    
    pIpHdr = mtod (pMbuf, struct ip *); 
    pMbuf->m_nextpkt = NULL; 
    /*
     * If first fragment to arrive, create a reassembly queue.
     */

    if (pIpFragQueue == 0)
    {
        if ((pMbufTmp = mBufClGet (M_DONTWAIT, MT_FTABLE, sizeof(struct ipq), TRUE)) == NULL)
            goto dropFrag;
        pIpFragQueue = mtod(pMbufTmp, struct ipq *);

        insque(pIpFragQueue, &my_ipq);

        pIpFragQueue->ipq_ttl = 60; /*ipfragttl configuration parameter default 60*/
        pIpFragQueue->ipq_p = pIpHdr->ip_p;
        pIpFragQueue->ipq_id = pIpHdr->ip_id;
        pIpFragQueue->ipq_src = ((struct ip *)pIpHdr)->ip_src;
        pIpFragQueue->ipq_dst = ((struct ip *)pIpHdr)->ip_dst;
        pIpFragQueue->pMbufHdr   = pMbufTmp; 	/* back pointer to mbuf */
        pIpFragQueue->pMbufPkt = pMbuf; 	/* first fragment received */
        goto ipChkReAssembly; 
    }

    for (pPtrMbuf = &(pIpFragQueue->pMbufPkt); *pPtrMbuf != NULL;
	 pPtrMbuf = &(*pPtrMbuf)->m_nextpkt)
    {
        pMbPktFrag = *pPtrMbuf; 
        pIpHdrFrag = mtod(pMbPktFrag, struct ipasfrag *); 

        if ((USHORT)pIpHdr->ip_off > (USHORT)pIpHdrFrag->ip_off)
        {
            if ((len = (signed int)((USHORT)pIpHdrFrag->ip_off +
                pIpHdrFrag->ip_len -
                (USHORT)pIpHdr->ip_off)) > 0)
            {
                if (len >= pIpHdr->ip_len)
                goto dropFrag;		/* drop the fragment */
                pIpHdrFrag->ip_len -= len; 
                m_adj(pMbPktFrag, -len); 	/* trim from the tail */
            }
            if (pMbPktFrag->m_nextpkt == NULL)
            { 	/* this is the likely case most of the time */
                pMbPktFrag->m_nextpkt = pMbuf; 	/* insert the fragment */
                pMbuf->m_nextpkt = NULL;
                break;
            }
        }
        else 		/* if pIpHdr->ip_off <= pIpHdrFrag->ip_off */
        {
            /* if complete overlap */
            while (((USHORT)pIpHdr->ip_off + pIpHdr->ip_len) >=
            ((USHORT)pIpHdrFrag->ip_off + pIpHdrFrag->ip_len))
            {
                *pPtrMbuf = (*pPtrMbuf)->m_nextpkt;
                pMbPktFrag->m_nextpkt = NULL;
                m_freem (pMbPktFrag);           /* drop the fragment */
                pMbPktFrag = *pPtrMbuf;
                if (pMbPktFrag == NULL)
                break;
                pIpHdrFrag = mtod(pMbPktFrag, struct ipasfrag *);
            }
            /* if partial overlap trim my data at the end*/
            if ((pMbPktFrag != NULL) &&
            ((len = (((USHORT) pIpHdr->ip_off + pIpHdr->ip_len) -
            (USHORT) pIpHdrFrag->ip_off)) > 0))
            {
                pIpHdr->ip_len -= len;
                m_adj (pMbuf, -len);        	/* trim from the tail */
            }
            pMbuf->m_nextpkt = pMbPktFrag; 
            *pPtrMbuf = pMbuf; 	/* insert the current fragment */	    
            break; 
        }
    }

    ipChkReAssembly:
    len = 0; 
    for (pMbPktFrag = pIpFragQueue->pMbufPkt; pMbPktFrag != NULL;
	 pMbPktFrag = pMbPktFrag->m_nextpkt)
{
    pIpHdrFrag = mtod(pMbPktFrag, struct ipasfrag *); 
    if ((USHORT)pIpHdrFrag->ip_off != len)
        return (NULL); 
    len += pIpHdrFrag->ip_len; 
}    
    if (pIpHdrFrag->ipf_mff & 1)	/* last fragment's mff bit still set */
	return (NULL); 

    /* reassemble and concatenate all fragments */
    pMbuf = pIpFragQueue->pMbufPkt; 
    pMbufTmp = pMbuf->m_nextpkt; 
    pMbuf->m_nextpkt = NULL; 

    while (pMbufTmp != NULL)
    {
        pMbPktFrag = pMbufTmp; 
        pIpHdrFrag = mtod(pMbPktFrag, struct ipasfrag *); 
        pMbPktFrag->m_data += pIpHdrFrag->ip_hl << 2;	/* remove the header */
        pMbPktFrag->m_len  -= pIpHdrFrag->ip_hl << 2;
        pMbufTmp = pMbPktFrag->m_nextpkt; 
        pMbPktFrag->m_nextpkt = NULL ; 
        m_cat (pMbuf, pMbPktFrag); 	/* concatenate the fragment */
    }

    pIpHdrFrag = mtod(pMbuf, struct ipasfrag *); 
    pIpHdrFrag->ip_len = len; 		/* length of the ip packet */
    if (len > 0xffff - (pIpHdrFrag->ip_hl << 2))  /* ping of death */
        goto dropFrag;                            /* drop entire chain */
    pIpHdrFrag->ipf_mff &= ~1;
    remque(pIpFragQueue);
    (void) m_free (pIpFragQueue->pMbufHdr); 

    /* some debugging cruft by sklower, below, will go away soon */
    if (pMbuf->m_flags & M_PKTHDR)
    { /* XXX this should be done elsewhere */
        len = 0; 
        for (pMbufTmp = pMbuf; pMbufTmp; pMbufTmp = pMbufTmp->m_next)
            len += pMbufTmp->m_len;
        pMbuf->m_pkthdr.len = len;
    }
    return (pMbuf);			/* return the assembled packet */

    dropFrag:
    m_freem (pMbuf);			/* free the fragment */
    return (NULL);
}
unsigned char switch_flag = 3;

void set_switch_flag(unsigned char flag)
{
  switch_flag = flag;
}
void ui_ip_slowtimo()
{
    register struct ipq *fp;
 	if(switch_flag&0x1==1)
 	{
      semTake(semSock, WAIT_FOREVER);
 	}
    fp = my_ipq.next;
    if (fp == 0) 
    {
       if(switch_flag&0x1==1)
       {
   		semGive(semSock);
       }
    	return;
    }

    while (fp != &my_ipq) 
    {
       if(fp->ipq_ttl>0)
       {
        --fp->ipq_ttl;
       }
        fp = fp->next;
	 if(fp!=0)
	 {
	        if (fp->prev!=NULL) 
	        {  if(fp->prev->ipq_ttl == 0)
	        	{
	        	   if(switch_flag&0x2==2)
	        	   {
	           	       ip_freef(fp->prev);
	        	   }
	        	}
	        }
	 }
	 else
	 {
	      logMsg("ui_ip_slowtimo exception",0,1,2,3,4,5);
		break;
	 }
    }
 if(switch_flag&0x1==1)
{
  semGive(semSock);
}
    return;
}

/*
 * Job entry point for net task.
 * Ip input routine.  Checksum and byte swap header.  If fragmented
 * try to reassemble.  Process options.  Pass to next level.
 */
#if 0
void
ipintr(struct mbuf *m)
{
	struct ip *ip;
	register struct ipq *fp;
	register struct in_ifaddr *ia;
	int hlen, s;

        if (m==0)
            return;
        s = splnet();

	/*
	 * If no IP addresses have been set yet but the interfaces
	 * are receiving, can't do anything with incoming packets yet.
	 */
	ip = mtod(m, struct ip *);
	if (ip->ip_v != IPVERSION) {
		goto bad;
	}
	hlen = ip->ip_hl << 2;
	if (hlen < sizeof(struct ip)) {	/* minimum header length */
		goto bad;
	}
	if (hlen > m->m_len) {
		if ((m = m_pullup(m, hlen)) == 0) {

			goto done;
		}
		ip = mtod(m, struct ip *);
	}

	/*
	 * Convert fields to host representation.
	 */
	NTOHS(ip->ip_len);
	if (ip->ip_len < hlen) {
		goto bad;
	}
	NTOHS(ip->ip_id);
	NTOHS(ip->ip_off);

	/*
	 * Check that the amount of data in the buffers
	 * is as at least much as the IP header would have us expect.
	 * Trim mbufs if longer than we expect.
	 * Drop packet if shorter than we expect.
	 */
	if (m->m_pkthdr.len < ip->ip_len) {
		goto bad;
	}
	if (m->m_pkthdr.len > ip->ip_len) {
		if (m->m_len == m->m_pkthdr.len) {
			m->m_len = ip->ip_len;
			m->m_pkthdr.len = ip->ip_len;
		} else
			m_adj(m, ip->ip_len - m->m_pkthdr.len);
	}

	/*
	 * If offset or IP_MF are set, must reassemble.
	 * Otherwise, nothing need be done.
	 * (We could look in the reassembly queue to see
	 * if the packet was previously fragmented,
	 * but it's not worth the time; just let them time out.)
	 */
	if (ip->ip_off &~ IP_DF) {
		/*
		 * Look for queue of fragments
		 * of this datagram.
		 */
		for (fp = ipq.next; fp != &ipq; fp = fp->next)
		{
			if (ip->ip_id == fp->ipq_id &&
			    ip->ip_src.s_addr == fp->ipq_src.s_addr &&
			    ip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
			    ip->ip_p == fp->ipq_p)
				goto found;
		}
		fp = 0;
found:

		/*
		 * Adjust ip_len to not reflect header,
		 * set ip_mff if more fragments are expected,
		 * convert offset of this to bytes.
		 */
		ip->ip_len -= hlen;
		((struct ipasfrag *)ip)->ipf_mff &= ~1;
		if (ip->ip_off & IP_MF)
			((struct ipasfrag *)ip)->ipf_mff |= 1;
		ip->ip_off <<= 3;

		/*
		 * If datagram marked as having more fragments
		 * or if this is not the first fragment,
		 * attempt reassembly; if it succeeds, proceed.
		 */
		if (((struct ipasfrag *)ip)->ipf_mff & 1 || ip->ip_off) {
			if ((m = ipReAssemble (m, fp)) == NULL)
				goto done;
                        ip = mtod (m, struct ip *);
			hlen = ip->ip_hl << 2;

		} else
			if (fp)
				ip_freef(fp);
	} else
		ip->ip_len -= hlen;

        splx (s);
        return;

bad:
	m_freem(m);
done:        
        splx (s);
        return;
        }
#endif


int
ui_ether_output(struct mbuf *m0, UINT8 *ucDstBtsMac,UINT32 ipAddr)
{
    register struct ether_header *eth;
    UINT32	resAddr;
    int i;
    
    M_PREPEND(m0, sizeof (struct ether_header), M_DONTWAIT);
    if (m == 0)
    {
        return ERROR;
    }

    
    eth = mtod(m0, struct ether_header *);
    
    bcopy(bMac, eth->ether_shost, ETH_ALEN);
    eth->ether_type = ETH_P_IP;	/* ip*/

    /* check default gateway */
    if((ipAddr & dwMask) == (dwIP & dwMask))
    	resAddr = ipAddr;
    else
    	resAddr = dwGateway;

	i = arpResolve(m0, ucDstBtsMac,resAddr);

	if(i == res_fail)
	{
	     udp_status.arp_fail3++;
	      m_freem(m0);//wangwenhua add 20090413
		return ERROR;
	  }
	else if(i == res_wait)
		return OK;

        if(muxSend(pMuxCookie, m0) != OK)
        {
           udp_status.arp_fail4++;
           m_freem(m0);//wangwenhua add 20090413
            return ERROR;
        }

        return OK;
}


/*
 * IP output.  The packet in mbuf chain m contains a skeletal IP
 * header (with len, off, ttl, proto, tos, src, dst).
 * The mbuf chain containing the packet will be freed.
 * The mbuf opt, if present, will not be freed.
 */
int
my_ip_output(struct mbuf *m0, UINT8 *ucDstBtsMac,UINT32 ipAddr)
{
    register struct ip *ip, *mhip;
    register struct mbuf *m = m0;
    register int hlen = sizeof (struct ip);
    int len, off, error = 0;
    int mtu = 0;

    ip = mtod(m, struct ip *);
    /*
    * Fill in IP header.
    */
    mtu = ip->ip_len;

    ip->ip_v = IPVERSION;
    ip->ip_off &= IP_DF;

    ip->ip_id = htons(ip_id++);
    ip->ip_hl = hlen >> 2;



    mtu = UDP_MTU_LEN;

    /*
    * If small enough for interface, can just send directly.
    */
    if ((u_short)ip->ip_len <= mtu) 
    {
        ip->ip_len = htons((u_short)ip->ip_len);
        ip->ip_off = htons((u_short)ip->ip_off);

	 ip->ip_sum = 0;	

        ip->ip_sum = in_cksum(m, hlen);

        if(ui_ether_output(m, ucDstBtsMac,ipAddr) != OK)
          {
         //   m_freem(m);//wangwenhua mark 20090413
           }
        goto done;
    }

    len = (mtu - hlen) &~ 7;
    if (len < 8) 
    {
        error = EMSGSIZE;
        goto bad;
    }

    {
    int mhlen, firstlen = len;
    struct mbuf **mnext = &m->m_nextpkt;


    /*
    * Loop through length of segment after first fragment,
    * make new header and copy data of each part and link onto chain.
    */
    m0 = m;
    mhlen = sizeof (struct ip);
    for (off = hlen + len; off < (u_short)ip->ip_len; off += len) 
    {
        m= mHdrClGet(M_DONTWAIT, MT_HEADER, CL_SIZE_128, TRUE);
        if (m == 0) 
        {
            error = ENOBUFS;
            goto sendorfree;
        }
        m->m_flags = m0->m_flags;
        m->m_data += 16;   /*max_linkhdr*/
        mhip = mtod(m, struct ip *);
        *mhip = *ip;
        if (hlen > sizeof (struct ip)) 
        {
            mhlen = ip_optcopy(ip, mhip) + sizeof (struct ip);
            mhip->ip_hl = mhlen >> 2;
        }
        m->m_len = mhlen;
        mhip->ip_off = ((off - hlen) >> 3) + (ip->ip_off & ~IP_MF);
        if (ip->ip_off & IP_MF)
            mhip->ip_off |= IP_MF;
        if (off + len >= (u_short)ip->ip_len)
            len = (u_short)ip->ip_len - off;
        else
            mhip->ip_off |= IP_MF;
        mhip->ip_len = htons((u_short)(len + mhlen));
        m->m_next = m_copy(m0, off, len);
        if (m->m_next == 0) 
        {
            (void) m_free(m);
            error = ENOBUFS;	/* ??? */

            goto sendorfree;
        }
        m->m_pkthdr.len = mhlen + len;
        m->m_pkthdr.rcvif = (struct ifnet *)0;
        mhip->ip_off = htons((u_short)mhip->ip_off);

        mhip->ip_sum = 0;
        mhip->ip_sum = in_cksum(m, mhlen);

        *mnext = m;
        mnext = &m->m_nextpkt;
    }
    /*
    * Update first fragment by trimming what's been copied out
    * and updating header, then send each fragment (in order).
    */
    m = m0;
    m_adj(m, hlen + firstlen - (u_short)ip->ip_len);
    m->m_pkthdr.len = hlen + firstlen;
    ip->ip_len = htons((u_short)m->m_pkthdr.len);
    ip->ip_off = htons((u_short)(ip->ip_off | IP_MF));

    ip->ip_sum = 0;
    ip->ip_sum = in_cksum(m, hlen);
sendorfree:
    for (m = m0; m; m = m0) 
    {
        m0 = m->m_nextpkt;
        m->m_nextpkt = 0;
        if (error == 0)
        {
            /*if(muxSend(pMuxCookie, m) != OK)*/
            if(ui_ether_output(m, ucDstBtsMac,ipAddr) != OK)
                {
                   //m_freem(m);//wangwenhua mark 20090413
                }
        }			
        else
        {
            m_freem(m);
         }
    }

    }
done:

    return (error);
bad:
    m_freem(m);
    goto done;
}



    
/**************************************************
	init UI (UDP interface) 

return:
	OK or RVERROR

called by initialize task
**************************************************/
int uiInit(void)
{
	BOOT_PARAMS params;
	struct ifnet 		*ifp;
	char	buf[100]; 
	M2_IPROUTETBL route;

	if(initialized)
		return TRUE;
       memset(&udp_status,0,sizeof(UDP_STATS));//wangwenhua add 20081215

	/* get mac addr */
	ifp = ifunit("mv0");
	if(!ifp)
	{
		printf("\nget mac addr failure!");
		return RVERROR;
	}	
	bcopy(((struct arpcom *)ifp)->ac_enaddr, bMac, ETH_ALEN);

	/* get  ip and subnet mask */
	if(ifAddrGet("mv0", buf) != OK)
	{
		printf("\nget ip addr failure!");		
		return RVERROR;
	}
	dwIP = inet_addr(buf);
	if(ifMaskGet("mv0", (int*)&dwMask) != OK)
	{
		printf("\nget subnetmask failure!");		
		return RVERROR;
	}


	/* get default gateway */
#if 1
	bootStringToStruct (BOOT_LINE_ADRS, &params);
	/*route.ipRouteDest = 0;
	if(m2IpRouteTblEntryGet(M2_EXACT_VALUE, &route) == OK)
		dwGateway = route.ipRouteNextHop;
	else
		dwGateway = 0;*/
	dwGateway = inet_addr(params.gad);
	
#endif

	/* event semaphore */
	semB =  semBCreate(SEM_Q_FIFO , SEM_EMPTY);
	if(!semB)
	{
		printf("\nsemBCreate(semB) error!");
		return RVERROR;
	}

	/* global semaphore */
	semArp = semMCreate(SEM_INVERSION_SAFE | SEM_Q_PRIORITY);
	if(!semArp)
	{
		printf("\nsemMCreate(semArp) error!");
		return RVERROR;
	}

	/* ring semaphore */
	semRng = semMCreate(SEM_INVERSION_SAFE | SEM_Q_PRIORITY);
	if(!semRng)
	{
		printf("\nsemMCreate(semRng) error!");
		return RVERROR;
	}

	/* sockTbl semaphore */
	semSock = semMCreate(SEM_INVERSION_SAFE | SEM_Q_PRIORITY);
	if(!semSock)
	{
		printf("\nsemMCreate(semSock) error!");
		return RVERROR;
	}

	/* ring buffer */
	ring = rngCreate(RING_BUF_LEN);
	if(!ring)
	{
		printf("\nrngCreate(ring) error!");
		return RVERROR;
	}
#if 0
	if(!initArpTbl())
	{
		printf("\ninitArpTbl() error!");
		return RVERROR;
	}
#endif
         initARP();
         InitFreeARPList();
      #if 0
	if(taskSpawn("tUiTask", 51, 0, 0x10000, (FUNCPTR)uiTask, 0,0,0,0,0,0,0,0,0,0) ==  ERROR)
	{
		printf("\ntaskSpawn(tUiTask) error!");
		return RVERROR;
	}
	

	if(taskSpawn("tUiTimer", 40, 0, 0x10000, (FUNCPTR)uiTimer, 0,0,0,0,0,0,0,0,0,0) == ERROR)
	{
		printf("\ntaskSpawn(tUiTimer) error!");
		return RVERROR;
	}

	pMuxCookie = muxBind("mv", 0, muxRecvRtn, muxShutdownRtn, 
			muxTxRestartRtn, muxErrorRtn, MUX_PROTO_SNARF, "RTP         ", NULL);
	if(pMuxCookie == NULL)
	{
		printf("\nmuxBind(motfcc0) error!");
		return RVERROR;
	}
	#endif

	my_ipq.next = my_ipq.prev = &my_ipq;
//      my_ip_id = iptime() & 0xffff;
      m_ipTimeCount = 0;

	initialized = TRUE;
	printf("uiInit.......................\n");

	memset(&stStatAbnmlIp,0,sizeof(TCTA_STAT_UIIP));
	return TRUE;
}

int startUiTask()
{
      #if 1
	if(taskSpawn("tUiTask", 51, 0, 0x10000, (FUNCPTR)uiTask, 0,0,0,0,0,0,0,0,0,0) ==  ERROR)
	{
		printf("\ntaskSpawn(tUiTask) error!");
		return RVERROR;
	}
	

	if(taskSpawn("tUiTimer", 51, 0, 0x10000, (FUNCPTR)uiTimer, 0,0,0,0,0,0,0,0,0,0) == ERROR)
	{
		printf("\ntaskSpawn(tUiTimer) error!");
		return RVERROR;
	}

	pMuxCookie = muxBind("mv", 0, muxRecvRtn, muxShutdownRtn, 
			muxTxRestartRtn, muxErrorRtn, MUX_PROTO_SNARF, "RTP         ", NULL);
	if(pMuxCookie == NULL)
	{
		printf("\nmuxBind(motfcc0) error!");
		return RVERROR;
	}
	return TRUE;
	#endif

}

/*************************************************
	stop UI, not implemented yet!! 
return:
	OK or RVERROR

called by initialize task
**************************************************/
int uiEnd(void)
{
	initialized = FALSE;
	/* free resouce */




	return OK;		
}

/************************************************* 
	create a socket and bind to UDP port 
return:
	 handle or RVERROR

called by stackTask
**************************************************/
int uiOpen (UINT32 ipAddr, UINT16 myPort, Protocol_t protocol)
{
	socketT *s;
	int handle;

	if(!initialized)
		return FALSE;

	/* check parameters */
    if(protocol != UDP)
		return FALSE;
	
	if(myPort < UI_MIN_PORT || myPort > UI_MAX_PORT)
	{
		printf("uiOpen port err :%d\n...",myPort);
		return FALSE;
	}
		
	
	/* check ipAddr ?? */
	/*if(!ipAddr)
		return RVERROR;*/

	/* assign a socket */
	handle = myPort - UI_MIN_PORT;
	s = &sockTbl[handle];
	if(s->localPort)
		return FALSE;

	/* set socket parameters */
/*	taskLock();*/
	semTake(semSock, WAIT_FOREVER);
	s->localIP = ipAddr;
	s->pMblk = NULL;
	s->callback = NULL;
	s->context = NULL;
	/*  for multitask, set port at the end*/
	s->localPort = myPort;
/*	taskUnlock();*/
	semGive(semSock);

	return handle;
}

/************************************************** 
	close a socket
return:
	OK or RVERROR

called by stackTask
**************************************************/
int uiClose(int handle)
{
	socketT *s;

	if(!initialized)
		return RVERROR;

	/* check parameters */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return RVERROR;

	s = &sockTbl[handle];
	/* for multitask, be care!!!*/
/*	taskLock();*/
	semTake(semSock, WAIT_FOREVER);
	s->localPort = 0;
	s->localIP = 0;
	s->pMblk = NULL;
	s->callback = NULL;
	s->context = NULL;
/*	taskUnlock();*/
	semGive(semSock);
	/* leave multicast group */

	return OK;
}

/*************************************************

return:
	OK or RVERROR

called by stack task
**************************************************/ 
int uiCallOn(
       int handle,
       UDP_CALLBACK_HANDLER callback,
       void* context)
{
	socketT *s;

	if(!initialized)
		return RVERROR;

	/* check parameters */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return RVERROR;

	s = &sockTbl[handle];
	/* for multitask, be care!!!*/
/*	taskLock();*/
	semTake(semSock, WAIT_FOREVER);
	s->context = context;
	s->callback = callback;
/*	taskUnlock();*/
	semGive(semSock);

	return OK;
}


/**************************************************
1. construct a ethernet packet
2. fill eth/ip/udp param
3. check default route
4. send packet

return:
	len or RVERROR

called by stackTask and dspTask
**************************************************/
int uiUdpSend (int handle, UINT8 *buf, int len, UINT8 *ucDstBtsMac,UINT32 ipAddr, UINT16 port)
{

	int i;
	M_BLK_ID	pMblk;
	socketT		*s;
	register ipUdpPktT	*pkt;
	register ipHeadT	*ip;
	register udpHeadT	*udp;
	register pseudoHeadT *phead;
	unsigned short sum;	
       udp_status.called_by_sockets++;
	if(!initialized)
		return RVERROR;

	/* check parameters */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return RVERROR;
 
	if(!buf)
		return RVERROR;

	if(len > UDP_PAYLOAD_LEN)
		return RVERROR;

	if(ipAddr == 0)
	{
/*		printf("\nuiUdpSend(0.0.0.0:%d)", port);*/
		return RVERROR;
	}


	/* count total len */
	i =len /*+ sizeof(ethHeadT)*/+ sizeof(ipHeadT)+ sizeof(udpHeadT);

	pMblk = netTupleGet(_pNetDpool, i, /*M_DONTWAIT*/M_WAIT, MT_DATA, TRUE);
	if(!pMblk)
	{
	       udp_status.err_times_no_blk++;
		return RVERROR;
	}
       udp_status.send_buf_times++;
	pMblk->m_next = NULL;
	pMblk->m_nextpkt = NULL;
	pMblk->m_len = i;
	pkt = (ipUdpPktT*)pMblk->m_data;

	s = &sockTbl[handle];
	
	/* udp head */
	udp = &pkt->udpHead;
	udp->srcPort = s->localPort;
	udp->dstPort = port;
	udp->len = sizeof(udpHeadT)+len;
	udp->checksum = 0;

	/* copy data */
	bcopy(buf, pkt->payload, len);

	/* set udp pseudo head */
	phead = &((pseudoPktT*)pkt)->phead;
	phead->srcIP.dwIP = s->localIP;
	phead->dstIP.dwIP = ipAddr;
	phead->proto = 0x11;
	phead->len = len+ sizeof(udpHeadT);

	/* count checksum (plus pseudo head) */
	sum = ui_checksum((WORD*)phead, len+ sizeof(pseudoHeadT)+ sizeof(udpHeadT));
	if(sum == 0)
		sum = 0xffff;

	udp->checksum = sum;


	/* eth head */
#if 0
	eth = &pkt->ethHead;
	bcopy(bMac, eth->h_source, ETH_ALEN);
	eth->h_proto = ETH_P_IP;	/* ip*/
#endif
	/* ip head */
	ip = &pkt->ipHead;
	ip->h_lenver	= 0x45;
	ip->tos			= 0;/*s->tos;*/
	ip->total_len	= len+ sizeof(ipHeadT)+ sizeof(udpHeadT);
	ip->ident		= 0xc2d2;
	ip->frag_and_flags	= 0;
	ip->ttl			= 64;
	ip->proto		= 0x11;	/* udp */
	ip->checksum	= 0;
	ip->srcIP.dwIP	= s->localIP;
	ip->dstIP.dwIP	= ipAddr;
	/* count checksum */
	if((ip->checksum = ui_checksum((WORD*)ip, sizeof(ipHeadT)))==0)
		ip->checksum = 0xffff;

	/* check destination: self/broadcast/multicast/route */
	/* ourself ? */
	if(ipAddr == dwIP || ipAddr == 0x7f000001)	/* 127.0.0.1*/
	{
		/* put rng buf */
		if(semTake(semRng, WAIT_FOREVER) != OK)
			goto error;
		if(rngFreeBytes(ring) > 4)
		{
			if(rngBufPut(ring, (char *)&pMblk, 4) == 4)
			{
				semGive(semRng);
				semGive(semB);
				 udp_status.send_success_times++;
				return len;
			}
		}
		semGive(semRng);
		printf("send task find ring buffer overflow");
		goto error;
	}
#if 0
	/* check default gateway */
	if((ipAddr & dwMask) == (dwIP & dwMask))
		resAddr = ipAddr;
	else
		resAddr = dwGateway;

	/* arp resolve */

	i = arpResolve(pMblk, resAddr);

	if(i == res_fail)
		goto error;
	else if(i == res_wait)
		return len;
#endif
	/*if(muxSend(pMuxCookie, pMblk) == OK)*/
        if(my_ip_output(pMblk, ucDstBtsMac,ipAddr) == OK)
        {    //send_success_times++;
             udp_status.send_success_times++;
		return len;
	}
        else
        {
           udp_status.send_fail_times++;
            return RVERROR;
        }
	
error:
	netMblkClChainFree(pMblk);
	udp_status.send_fail_times++;
	return RVERROR;
}

/**************************************************
	receive udp packet 

return:
	len

called by uiTask task
**************************************************/
int uiUdpRecv (int handle, UINT8 *buff, int len, UINT32 *ipAddr, UINT16 *port)
{
	int copyLen;
	M_BLK_ID mBlk;
	udpPktT *udp;

	if(!initialized)
		return 0;

	/* check */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return 0;

	/* read data */
/*	taskLock();*/
	semTake(semSock, WAIT_FOREVER);
	mBlk = sockTbl[handle].pMblk;
	if(mBlk)
	{
		udp = (udpPktT *)mBlk->m_data;
		copyLen = udp->udpHead.len - sizeof(udpHeadT);
		copyLen =  (copyLen > len)? len : copyLen;
		bcopy((char*)udp->payload, (char*)buff, copyLen);
		*ipAddr = udp->ipHead.srcIP.dwIP;
		*port = udp->udpHead.srcPort;
/*		taskUnlock();*/
		semGive(semSock);
		return copyLen;
	}

/*	taskUnlock();*/
	semGive(semSock);
	return 0;
}

/**************************************************

return:
	port or RVERROR

called by stack task
**************************************************/
UINT16  uiGetSockPort (int handle)
{
	UINT16 port;

	if(!initialized)
		return RVERROR;

	/* check */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return RVERROR;

	port = sockTbl[handle].localPort;

	return port;
}

/**************************************************

return:
	OK or RVERROR

called by stackTask
**************************************************/
int  uiBlock (int handle)
{
	if(!initialized)
		return RVERROR;

	/* check */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return RVERROR;

	return OK;
}

/**************************************************

return:
	OK or RVERROR

called by stack task
**************************************************/
int  uiUnblock (int handle)
{
	if(!initialized)
		return RVERROR;

	/* check */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return RVERROR;

	return OK;
}

/**************************************************

return:
	OK or RVERROR

called by stack task
**************************************************/
int  uiJoinMulticastGroup(int handle, UINT32 mcastip, UINT32 ipaddr)
{
	if(!initialized)
		return RVERROR;

	/* check */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return RVERROR;

	return OK;
}
/**************************************************

return:
	OK or RVERROR

called by stack task
**************************************************/
int  uiSetSocketBuffers(int handle, int sendSize, int recvSize)
{
	if(!initialized)
		return RVERROR;

	/* check */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		return RVERROR;

	return OK;
}

/**************************************************

return:
	OK or RVERROR

called by uiTask task
**************************************************/
int  uiBytesAvailable (int handle, int *bytesAvailable)
{
	M_BLK_ID pMblk;
	udpPktT *udp;

	if(!initialized)
		return RVERROR;

	/* check */
	if(handle < 0 || handle > UI_MAX_HANDLE)
		goto error;

/*	taskLock();*/
	semTake(semSock, WAIT_FOREVER);
	pMblk = sockTbl[handle].pMblk;
	if(pMblk)
	{
		udp = (udpPktT *)pMblk->m_data;
		*bytesAvailable = udp->udpHead.len - sizeof(udpHeadT);
/*		taskUnlock();*/
		semGive(semSock);
		return OK;
	}

error:
	*bytesAvailable= 0;
/*	taskUnlock();*/
	semGive(semSock);
	return RVERROR;
}



/**************************************************
**************************************************/
int mBlock;

int ui_Print = 0;

static void uiTask(void)
{

	M_BLK_ID	pMblk;
	ipUdpPktT *udp;
	int		handle;
	socketT		*s;


	for(;;)
	{
		if(semTake(semB, WAIT_FOREVER) != OK)
			continue;
		for(;;)
		{
			/* take ring */
			if(semTake(semRng, WAIT_FOREVER) != OK)
				break;

			if(rngIsEmpty(ring))
			{
				/* release ring */
				semGive(semRng);
				break;
			}

			if(rngBufGet(ring, (char*)&pMblk, 4) != 4)
			{
				/* release ring */
				udp_status.Rec_Ring_Buf_OverFlow++;
				semGive(semRng);
				printf("\nuiTask find ring buffer overflow!!");
				netMblkClChainFree(pMblk);/*wangwenhua add 20081210**/
				break;
			}
			/* release ring */
			semGive(semRng);
			
			if(pMblk == NULL)
			{
				printf("\nuiTask find error ring buffer content!!");
				continue;
			}


			udp = (ipUdpPktT *)pMblk->m_data;
			/* check len */
                   #if 0
			if((udp->udpHead.len /*+ sizeof(ethHeadT)*/ + sizeof(ipHeadT)) != pMblk->m_len )
			{
				netMblkClChainFree(pMblk);
/*				printf("\nUDP packet length error!");*/
				continue;
			}
                    #endif
			/* notify */
			handle = udp->udpHead.dstPort - UI_MIN_PORT;

		      if(ui_Print)
		     {
				printf("muxRecvRtn,handle:%d...\n",handle);
		      }
			udp_status.Rec_buf++;
			if(handle >= 0 && handle <= UI_MAX_HANDLE)
			{
				/*taskLock();*/
				semTake(semSock, WAIT_FOREVER);
				s = &sockTbl[handle];
				mBlock = (int)(s->callback);
				if(s->callback)
				{
					s->pMblk = pMblk;
                                s->callback(handle, 0, s->context);
					s->pMblk = NULL;
				}
				/*taskUnlock();*/
				semGive(semSock);
			}
			netMblkClChainFree(pMblk);
		}
	}
}

int gnetPoolFreeBufEndCnt = 0;
/*定时监测_pNetDpool 的使用情况，每分钟监测一次
连续5次_pNetDpool free内存耗尽，则复位基站*/
 void netPoolDetect
(
/*NET_POOL_ID	pNetPool		/* pointer the netPool */
)
{
	int nobufCt = 0;
	UCHAR 	clType; 
	CL_POOL_ID	pClPool;

	for (clType = _pNetDpool->clLg2Min; clType <= _pNetDpool->clLg2Max; clType++)
	{
	    if ((pClPool = netClPoolIdGet (_pNetDpool, CL_LOG2_TO_CL_SIZE(clType),
	                                    TRUE)) != NULL)
	    {
		if(pClPool->clNumFree==0)/*内存被耗尽*/
		{
			nobufCt ++;

		}
		
			
	    }
	}
	if(nobufCt>=3)
	{
			gnetPoolFreeBufEndCnt++;
			printf("\nNet Pool have no free buf.\n");
			mbufShow();
			return;
	}
	else
	{
	//没有内存耗尽的情况
		gnetPoolFreeBufEndCnt = 0;
	}
}

extern void ResetNetwork();


/**************************************************
**************************************************/
static void uiTimer(void)
{
	int tick = 0;
	for(;;)
	{
		taskDelay(10);
		arpTimer(); 
             taskLock();
            // ip超时处理，如果IP重组队列超时丢弃IP包(500ms)
             m_ipTimeCount = (m_ipTimeCount++)%5;
             if(m_ipTimeCount == 0)
             {
              
              
                ui_ip_slowtimo();
              
             }
	    taskUnlock();
	      tick++;
	      if(tick%100==0)//10s 清一次ICMP的相关统计
	      {
			memset(&stStatAbnmlIp,0,sizeof(TCTA_STAT_UIIP));
		}
	      if(tick%600==0)//每分钟
	      	{
	      	       if(1)
	      	       {
	      			netPoolDetect();
	      	       }
			//if(gnetPoolFreeBufEndCnt>2)/*********wangwenhua add 20081119**************/
			//{
			//   ResetNetwork();
			//}
			if(gnetPoolFreeBufEndCnt>=5)
			{
			    bspSetBtsResetReason(35/*RESET_REASON_NETMBUFFER_FULL*/);
                         taskDelay(50);
			   rebootBTS(0); 
			}
	      	}
	   
	}
}

int displayUiIcmp(void)
{
	int i = 0;
	printf("ui icmp info:\n");
	for(i = 0;i<10;i++)
	{
		printf("souse ip:%s,dest unreach count:%d,ttl count:%d,other icmp:%d\n",inet_ntoa(*(struct in_addr*)(&stStatAbnmlIp.stIp[i].ulSourceIp)),\
			stStatAbnmlIp.stIp[i].usCntOfDestUnreach,stStatAbnmlIp.stIp[i].usCntOfTtlExpire,\
			stStatAbnmlIp.stIp[i].usOtherIcmp);
	}
	

}

int dwSagIpFlag = 0;

int uiPrintFlag = 0;
void setUiPrint( int flag)
{
	uiPrintFlag = flag;
}
unsigned char ui_flag = 0;

void setUIFlag(int flag)
{
   ui_flag = flag;
}
/**************************************************
 receive packet 
**************************************************/
LOCAL BOOL muxRecvRtn
    (
    void * pCookie,       	/* EndObj * */
    long  type,			/* network service type */
    M_BLK_ID pMblk,		/* MAC frame */
    LL_HDR_INFO * pLinkHdrInfo,	/* link level information */
    void * pSpare		/* pSpare supplied during muxBind */
    )
{

    udpPktT *udp;
    ipUdpPktT *ipUdp;
    arpPktT *arpPkt;
    arpTableT	arp;
    newarpTableT arp_new;
    int i;
    int		port, handle;
    unsigned int arp_tip;
    struct ip *ip;
    register struct ipq *fp;
    int hlen, s;
    ICMPHeader* icmphdr; 
    unsigned short header_len;
    ULONG sip;
    UCHAR temp;

 
    if (pCookie == NULL || pMblk == NULL)
		return FALSE;


	switch(type)
	{
		case ETH_P_IP:	/* ip */
                     udp_status.rec_ip++;
			udp = (udpPktT *)pMblk->m_data;

			/* check protocol */
			if(udp->ipHead.proto != 0x11)	/*  udp	*/
			{
			#if 1
				if((udp->ipHead.proto ==0x01)&&(stStatAbnmlIp.ucStatAbnmlIpCnt<10))/*ICMP消息*/
				{
					/*pMblk->m_data += sizeof (struct ether_header);*/
					udp_status.rec_icmp++;
					sip = ((IPHeader *)pMblk->m_data)->source_ip;
					for(i = 0;i<10;i++)
				 	 {
						if(sip==stStatAbnmlIp.stIp[i].ulSourceIp)
							break;
							
					 }
					 if(i==10)/*新上来的用户*/
				        {	
				        	temp = stStatAbnmlIp.ucStatAbnmlIpCnt++;
						if((temp<10))
					 	stStatAbnmlIp.stIp[temp].ulSourceIp= sip;
					 }
					 else
					 {
						temp = i;
					 }
					 	
				        if(temp<10)
				        {
				        	header_len = ((IPHeader *)pMblk->m_data)->h_len * 4;
						icmphdr = (ICMPHeader*)((char*)pMblk->m_data + header_len);
					      if(icmphdr->type==ICMP_DEST_UNREACH)/*包不可达*/
						{
							stStatAbnmlIp.stIp[temp].usCntOfDestUnreach++;
						}
						else if(icmphdr->type==ICMP_TTL_EXPIRE)/*超时*/
						{
							stStatAbnmlIp.stIp[temp].usCntOfTtlExpire++;
						}
				              //其余ICMP 消息
				              else
				              {
							stStatAbnmlIp.stIp[temp].usOtherIcmp++;
						}
				        }
				
				}
			#endif
				break;
			}
				
			/* check port */
            #if 0
			port = udp->udpHead.dstPort;
			if(port < UI_MIN_PORT || port > UI_MAX_PORT)
				break;
			handle = port - UI_MIN_PORT;
			/* multitask: be care !!!!*/
			if(sockTbl[handle].localPort != port)
				break;
            #endif
                    /*
                	 * If no IP addresses have been set yet but the interfaces
                	 * are receiving, can't do anything with incoming packets yet.
                	 */
                	// printf("\n\rm_len=%d ",pMblk->m_len);
                	 if (pMblk->m_len < sizeof (struct ip) )
                	 {
                		break;
                	  }
			/*增加对MAC头的判断*/
			for(i = 0;i<ETH_ALEN;i++)
			{
				if(udp->ethHead.h_dest[i]!=bMac[i])
					return FALSE;
			}
			if(uiPrintFlag==1)
			{

				printf("\nUDP msg From:%s,data:%x,%x,%x,%x,%x,%x,%x,%x\n",inet_ntoa(*(struct in_addr*)(&udp->ipHead.srcIP.dwIP)),udp->payload[0],udp->payload[1],\
					udp->payload[2],udp->payload[3],udp->payload[4],udp->payload[5],udp->payload[6],udp->payload[7]);
			}
                     //如果是语音数据包则不进行处理
                    // if(udp->ipHead.srcIP.dwIP==dwSagIp)
                     if( ((udp->ipHead.srcIP.dwIP==stSagAddr.sin_addr.s_addr)&&(stSagAddr.sin_port==udp->udpHead.srcPort)) ||
				((udp->ipHead.srcIP.dwIP==stSagAddr1.sin_addr.s_addr)&&(stSagAddr1.sin_port==udp->udpHead.srcPort)) )
                     {
                     	dwSagIpFlag = 1;
				break;
                     }	 
					 
                     pMblk->m_data += sizeof (struct ether_header);
                     pMblk->m_len -= sizeof (struct ether_header);
                     #if 1
                	ip = mtod(pMblk, struct ip *);

                	if (ip->ip_v != IPVERSION) 
                		break;
                    
                	hlen = ip->ip_hl << 2;
                	if (hlen < sizeof(struct ip)) 	/* minimum header length */
                		break;

                    /*
                	 * If offset or IP_MF are set, must reassemble.
                	 * Otherwise, nothing need be done.
                	 * (We could look in the reassembly queue to see
                	 * if the packet was previously fragmented,
                	 * but it's not worth the time; just let them time out.)
                	 */
	              if (ip->ip_off &~ IP_DF) 
                    {
                		/*
                		 * Look for queue of fragments
                		 * of this datagram.
                		 */
                		for (fp = my_ipq.next; fp != &my_ipq; fp = fp->next)
                		{
                			if (ip->ip_id == fp->ipq_id &&
                			    ip->ip_src.s_addr == fp->ipq_src.s_addr &&
                			    ip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
                			    ip->ip_p == fp->ipq_p)
                				goto found;
                		}
		            fp = 0;
found:

                		/*
                		 * Adjust ip_len to not reflect header,
                		 * set ip_mff if more fragments are expected,
                		 * convert offset of this to bytes.
                		 */
                		// printf("\n\rsss2 %x", fp);
                		ip->ip_len -= hlen;
                		((struct ipasfrag *)ip)->ipf_mff &= ~1;
                		if (ip->ip_off & IP_MF)
                			((struct ipasfrag *)ip)->ipf_mff |= 1;
                		ip->ip_off <<= 3;

                		/*
                		 * If datagram marked as having more fragments
                		 * or if this is not the first fragment,
                		 * attempt reassembly; if it succeeds, proceed.
                		 */
                		if (((struct ipasfrag *)ip)->ipf_mff & 1 || ip->ip_off) 
                         {
                			if ((pMblk = ipReAssemble (pMblk, fp)) == NULL)
                				return TRUE;
                                        ip = mtod (pMblk, struct ip *);
                			hlen = ip->ip_hl << 2;

                		} else
                			if (fp)
                				ip_freef(fp);
                	} 
                  else
		        ip->ip_len -= hlen;
		    #endif


                   // 重组完成后判断是否为本端口数据
                   ipUdp = (ipUdpPktT *)pMblk->m_data;
			port = ipUdp->udpHead.dstPort;

			
		     if(ui_Print)
		     {
				printf("muxRecvRtn,port:%d...\n",port);
		      }
				   
			if(port < UI_MIN_PORT || port > UI_MAX_PORT)
				break;
			handle = port - UI_MIN_PORT;
			/* multitask: be care !!!!*/
			if(sockTbl[handle].localPort != port)
			{
				break;
			}
            
			/* put ring buffer */
			if(semTake(semRng, WAIT_FOREVER) != OK)
				break;
			if(rngFreeBytes(ring) > 4)
			{
				if(rngBufPut(ring, (char *)&pMblk, 4) == 4)
				{
					semGive(semRng);
					semGive(semB);
					return TRUE;
				}
			}
			semGive(semRng);
			printf("\nnetTask find ring buffer overflow!");
			netMblkClChainFree (pMblk);
			return TRUE;
			break;

		case ETH_P_ARP:	/* arp */
			udp_status.rec_arp++;
			arpPkt = (arpPktT *)pMblk->m_data;
			if(arpPkt->arpHead.ar_op == ARP_REQUEST || arpPkt->arpHead.ar_op == ARP_REPLY)
			{
#if 0
				bcopy(arpPkt->arpHead.ar_sip, (char*)&arp.dwIP, 4);
				bcopy(arpPkt->arpHead.ar_sha, arp.bMac, ETH_ALEN);
				updateArpTbl(&arp);
#endif
				bcopy(arpPkt->arpHead.ar_sip, (char*)&arp_new.dwIP, 4);
				bcopy(arpPkt->arpHead.ar_sha, arp_new.bMac, ETH_ALEN);
				arp_tip = arpPkt->arpHead.ar_tip[0]*0x1000000+arpPkt->arpHead.ar_tip[1]*0x10000 +arpPkt->arpHead.ar_tip[2]*0x100 + arpPkt->arpHead.ar_tip[3];
                            if(ui_flag==1)
                             {
                                 printf("arp type:%d,ip:%x\n",arpPkt->arpHead.ar_op,arp_tip);
                              }
				updateArpTblNew(&arp_new,arpPkt->arpHead.ar_op,arp_tip);
			}
			break;
		default:
			return FALSE;
			break;

	}


	return FALSE;
}

/*******************************************************************************
* muxAdptrEndShutdownRtn -
**************************************************/
LOCAL STATUS muxShutdownRtn
    (
    void* cookie,
    void * pSpare
    )
{
/*
    MUX_ADPTR_BIND_INFO * pBindInfo = ( MUX_ADPTR_BIND_INFO *)pSpare;
    STATUS status;

    if (pBindInfo == NULL || pBindInfo->cookie == NULL )
	return ERROR;


    status = muxUnbind (pBindInfo->cookie,pBindInfo->cookie->netSvcType,
		       muxAdptrEndRecvRtn);
    return status;
*/
	
/*	printf("\nmuxShutdown()");*/
	return TRUE;
}

/*************************************************
* muxAdptrEndTxRestartRtn -
**************************************************/
LOCAL STATUS muxTxRestartRtn
    (
    void * cookie,
    void * pSpare
    )
{
/*
    if (pSpare == NULL || cookie == NULL)
	return ERROR;

    return (muxAdptrTxRestartRtn(pSpare));
*/

/*	printf("\nmuxTxRestart()");*/
	return TRUE;
}

/**************************************************
* muxAdptrEndErrorRtn -
**************************************************/
LOCAL void muxErrorRtn
    (
    END_OBJ * cookie,
    END_ERR *endErr,
    void * pSpare
    )
{
/*
    if (pSpare == NULL || cookie == NULL)
	return ;

    muxAdptrErrorRtn(pSpare,endErr);
*/

/*	printf("\nmuxError()");*/

}
#if 0
/********************************************************************
	arp protocol and address resolve
*********************************************************************/
static arpTableT *arpTbl, *idleArpTbl;


/**************************************************

return:
	TRUE or FALSE
**************************************************/
LOCAL int initArpTbl(void)
{
	int i;

	arpTableT *p;

	/* init arpTbl */
	arpTbl = NULL;

	/* alloc memory for arp table */
	p = (arpTableT *)calloc(ARP_TABLE_LEN, sizeof(arpTableT));
	if(!p)
		goto error;
	/* construct idleArpTbl */
	for(i=0; i< ARP_TABLE_LEN; i++, p++)
		freeArpBlk(p);

	return TRUE;

error:
	return FALSE;
}


/**************************************************

return:
	TRUE/FALSE
**************************************************/
LOCAL int insertArpTbl(arpTableT* p)
{
	if(!p)
		return FALSE;

	if(!arpTbl)	/* blank */
	{
		arpTbl = p;
		arpTbl->next = NULL;
		arpTbl->prev = NULL;
	}
	else	/* insert into head */
	{
		arpTbl->prev = p;
		p->next = arpTbl;
		p->prev = NULL;
		arpTbl = p;
	}

	return TRUE;
}

/**************************************************

return:
	TRUE/FALSE
**************************************************/
LOCAL int removeArpTbl(arpTableT* p)
{
	if(!p)
		return FALSE;

	if(p->prev)	/* not head */ 
		p->prev->next = p->next;

	if(p->next) /* not tail */
		p->next->prev = p->prev;	

	/* head */
	if(arpTbl == p)
		arpTbl = p->next;	

	return TRUE;
}


/**************************************************
* updateArpTbl and send hold packet

return:
	TRUE or FALSE
**************************************************/
LOCAL int updateArpTbl(arpTableT *arp)
{
	arpTableT *t;

	if(!arp)
		return FALSE;


	/* traversal arp table */
	for(t=arpTbl; t; )
	{
		if(t->dwIP == arp->dwIP)
		{
			bcopy(arp->bMac, t->bMac, ETH_ALEN);
			t->wTimer = ARP_LIVE_TIME;
			if(t->hold)
			{
				bcopy(arp->bMac, t->hold->m_data, ETH_ALEN);
				if(muxSend(pMuxCookie, t->hold) != OK)
				{
					netMblkClChainFree(t->hold);
					t->hold = NULL;
					goto error;
				}
				t->hold = NULL;
			}
			goto ok;
		}

		t=t->next;
	}
	/* append arp table */
	t = allocArpBlk();
	if(!t)
		goto error;

	t->dwIP = arp->dwIP;
	t->wTimer = ARP_LIVE_TIME;
	t->hold = NULL;
	bcopy(arp->bMac, t->bMac, ETH_ALEN);
	if(!insertArpTbl(t))
	{
		freeArpBlk(t);
		goto error;
	}

ok:
	return TRUE;
error:
	return FALSE;
}



/**************************************************

return:
	pointer or NULL
**************************************************/
LOCAL arpTableT* allocArpBlk(void)
{
	arpTableT *p;
	
	if(!idleArpTbl)
		return NULL;

	p = idleArpTbl;
	idleArpTbl = idleArpTbl->next;

	return p;
}

/**************************************************

return:
	TRUE/FALSE
**************************************************/
LOCAL int freeArpBlk(arpTableT *arp)
{
	if(!arp)
		return FALSE;

	arp->next = idleArpTbl;
	idleArpTbl = arp;

	return TRUE;
}
#endif
extern UINT32 GetBtsIpAddr();
/**************************************************
* updateArpTbl and send hold packet

return:
	TRUE or FALSE
**************************************************/
LOCAL int updateArpTblNew(newarpTableT *arp,unsigned char flag,unsigned int target_ip)
{
	newarpTableT *t;
       unsigned int local_pubip ;
       unsigned int local_ip ;
       UINT16 index;
       local_pubip=  bspGetBtsPubIp();
       local_ip = GetBtsIpAddr();
           /* append arp table */
	/*if(flag == ARP_REQUEST)
	{*/
       index = ARPBPtreeFind(arp->dwIP);
	t = GetARPEntryByIdx(index);
       if(t==NULL)
        {
	     if((local_pubip!=target_ip)&&(local_ip!=target_ip))
	     	{
	     	   if(ui_flag==1)
	     	    {
	     	         printf("localip:%x,target_ip:%x!",local_pubip,target_ip);
	     	    }
	     	    return TRUE;
	     	}
        }
	/*}*/
       
	/*index = ARPBPtreeFind(arp->dwIP);
	t = GetARPEntryByIdx(index);*/
	if(!arp)
		return FALSE;
	if(t!=NULL)
	{
	            bcopy(arp->bMac, t->bMac, ETH_ALEN);
			t->wTimer = ARP_LIVE_TIME;
			if(t->hold)
			{
				bcopy(arp->bMac, t->hold->m_data, ETH_ALEN);
				if(muxSend(pMuxCookie, t->hold) != OK)
				{
					netMblkClChainFree(t->hold);
					t->hold = NULL;
					goto error;
				}
				t->hold = NULL;
			}
			/*ARPUpdateEntry(arp->dwIP, t);*/
			goto ok;
	}

 
     arp->wTimer = ARP_LIVE_TIME;
     arp->hold = NULL;
     if(semTake(semArp, WAIT_FOREVER) != OK)
     {              
		return  FALSE;
    }
    if(! ARPAddEntry(arp->dwIP,arp))
    	{
    	   semGive(semArp);
    	   goto error;
    	}
    semGive(semArp);
	
ok:
	return TRUE;
error:
	return FALSE;
}
/**************************************************
* arpResolve
resolve: unicast,broadcast,multicast 

return:
	res_ok=1,: resolve successful
	res_wait=2,: waiting for arp reply
	other: resolve failure
**************************************************/
LOCAL int arpResolve(M_BLK_ID pMblk, UINT8*ucDstBtsMac,DWORD dwip)
{
	UINT8*ucP;
	//register arpTableT *arp, *p;
	register newarpTableT *arp;
	newarpTableT p;
	register DWORD ip;
       UINT16  index ;
	/* check pointer */
	if(!pMblk || !dwIP)
	{
		return res_fail;
	}

	ip = dwip;

	if(semTake(semArp, WAIT_FOREVER) != OK)
		return res_fail;
	ucP = ucDstBtsMac;
       if((*ucP==0xaa)&&(*(ucP+1)==0xbb)&&(*(ucP+2)==0xcc)&&(*(ucP+3)==0xdd)&&(*(ucP+4)==0xee)&&(*(ucP+5)==0xff))
       {
		/* tranversal arp table */
		 index = ARPBPtreeFind(ip);
	        arp = GetARPEntryByIdx(index);
		 if(arp)
		 {
			 if(arp->hold)
			 {
			    udp_status.arp_fail1++;
			     goto fail;
			 }
			 bcopy(arp->bMac, ((udpPktT*)(pMblk->m_data))->ethHead.h_dest, ETH_ALEN);
			 arp->wTimer = ARP_LIVE_TIME;
			/* ARPUpdateEntry(arp->dwIP, arp);*/
			 goto ok;
		 }
       }
	else
	{	bcopy(ucDstBtsMac, ((udpPktT*)(pMblk->m_data))->ethHead.h_dest, ETH_ALEN);
		goto ok;
	}
	/* send arp packet */
	if(!sendArpPkt(ip))
	{
	      udp_status.arp_fail2++;
		goto fail;
	}
	#if 0
	/* not resolved in arp table */
	p = allocArpBlk();
	if(!p)
	{
		goto fail;
	}	
	#endif
	/* set arp table inf */
	p.dwIP = ip;
	p.wTimer = ARP_RESP_TIME;
	p.hold = pMblk;
	/* insert to arp table */
	/*insertArpTbl(p);*/
      ARPAddEntry( ip,  &p);

	semGive(semArp);
	udp_status.arp_wait++;
	return res_wait;

ok:
	semGive(semArp);
	return res_ok;

fail:
	semGive(semArp);
	udp_status.arp_fail++;
	return res_fail;

}


/*************************************************


*************************************************/
int DstBtsMacFromUiArpTbl(UINT8*ucDstBtsMac,DWORD dwip)
{
#if 0
	register arpTableT *arp, *p;
	register DWORD ip;

	/* check pointer */
	if( !dwIP)
	{
		return 1;
	}

	ip = dwip;

	if(semTake(semArp, WAIT_FOREVER) != OK)
		return 1;
	/* tranversal arp table */
	arp = arpTbl;
	for(; arp; )
	{
		if(ip != arp->dwIP)	/* mismatch */
		{
			arp = arp->next;
			continue;
		}
		/* holding packets */
		if(arp->hold)		
		{
		       semGive(semArp);
			return 1;
		}
		/* resolve successful */
		bcopy(arp->bMac, ucDstBtsMac, ETH_ALEN);
		//arp->wTimer = ARP_LIVE_TIME;
		semGive(semArp);
		return 0;
	}
       
	semGive(semArp);
#endif
	return 1;

}

/**************************************************
* arpTimer -
scan arpTable once in 100ms, process keepAlive timer 
and response timer.

**************************************************/
LOCAL void arpTimer(void)
{
	register arpTableT	*arp;	
	register arpTableT	*next;	


	if(semTake(semArp, WAIT_FOREVER) != OK)
		return;
#if 0
	for(arp = arpTbl; arp;)
	{
		arp->wTimer--;
		if(arp->wTimer ==0)
		{
			if(arp->hold)
				netMblkClChainFree(arp->hold);
			next = arp->next;
			removeArpTbl(arp);
			freeArpBlk(arp);
			arp = next;
		}
		else
			arp = arp->next;
	}
#endif
       ARPBPtreeExpire();
	semGive(semArp);

}


/**************************************************
* sendArpPkt -

return:
	TRUE or FALSE
**************************************************/
LOCAL int sendArpPkt(DWORD ip)
{
	arpPktT *arp;
	M_BLK_ID pMblk;


	if(!ip)
		return FALSE;

	pMblk = netTupleGet(_pNetDpool, 64, /*M_DONTWAIT*/M_WAIT, MT_DATA, TRUE);
	if(!pMblk)
	{
		return FALSE;
	}

	pMblk->m_next = NULL;
	pMblk->m_nextpkt = NULL;
	arp = (arpPktT*)(pMblk->m_data);
	pMblk->m_len = 60;/*sizeof(arpPktT);*/

	/*ethernet head */
	bcopy(bMac, arp->ethHead.h_source, ETH_ALEN);	/* source mac*/
	memset(arp->ethHead.h_dest, 0xff, ETH_ALEN);	/* dest mac */
	arp->ethHead.h_proto = ETH_P_ARP;				/* protocol*/
	/* arp head */
	arp->arpHead.ar_hrd = 1;						/* hardware: ethernet */
	arp->arpHead.ar_pro = ETH_P_IP;
	arp->arpHead.ar_hln = ETH_ALEN;
	arp->arpHead.ar_pln = 4;
	arp->arpHead.ar_op = ARPOP_REQUEST;
	bcopy(bMac, arp->arpHead.ar_sha, ETH_ALEN);
	bcopy((unsigned char*)&dwIP, arp->arpHead.ar_sip, 4);
	memset(arp->arpHead.ar_tha, 0, ETH_ALEN);
	bcopy((unsigned char*)&ip, arp->arpHead.ar_tip, 4);

	if(muxSend(pMuxCookie, pMblk) == OK)
		return TRUE;
	
	netMblkClChainFree(pMblk);
	return FALSE;
}


#if 0
/************************************************
Show:
	(1)arpTable
	(2)idleTable count
return:
	checksum
************************************************/
void netShow2(void)
{
	arpTableT *arp;
	int i;
	printf("\ntuiTask ARP table");
	printf("\n\n--IP-----------MAC------------Timer-----HOLD------");
	for(arp = arpTbl; arp;)
	{
		printf("\n\r%d.%d.%d.%d  %02x-%02x-%02x-%02x-%02x-%02x  %d    %08x", 
			(int)((arp->dwIP)>>24)&0xff, (int)((arp->dwIP)>>16)&0xff,(int)((arp->dwIP)>>8)&0xff,(int)(arp->dwIP)&0xff,
			arp->bMac[0], arp->bMac[1], arp->bMac[2], arp->bMac[3], arp->bMac[4], arp->bMac[5], 
			arp->wTimer, (unsigned int)arp->hold);
		arp = arp->next;
	}

	for(i=0, arp = idleArpTbl; arp;i++)
		arp = arp->next;
	printf("\n\nIdleArpTbl Count: %d\n\n", i);
}
#endif
/************************************************
return:
	checksum
************************************************/
#define _BYTE_ORDER  _BIG_ENDIAN

u_short ui_checksum
    (
    u_short *           pAddr,                  /* pointer to buffer  */
    int                 len                     /* length of buffer   */
    )
{
    register int nLeft=len;
    register int sum= 0;
    register u_short *w=pAddr;
/*    u_short     answer;*/

    while (nLeft > 1)
        {
        sum     += *w++;
        nLeft   -= 2;
        }

    if (nLeft == 1)
#if _BYTE_ORDER == _BIG_ENDIAN
        sum += 0 | ((*(u_char *) w) << 8);
#else
        sum += *(u_char *) w;
#endif

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
	
/*    answer = sum;
    return (~answer & 0xffff);
*/
    return (~sum & 0xffff);
}

void tmTimer()
{
    msgQSend(m_msgQ, m_buffer, 1, NO_WAIT, MSG_PRI_NORMAL);
    wdStart(m_wd, m_tick, (FUNCPTR)tmTimer, 0);
}

int g_uiSendLen = 1024;

/*处理任务*/
int T_Test()
{
#if  0
    int                 i, j, k;
    unsigned char       buffer[20];

    while (1)
    {
        i = msgQReceive(m_msgQ, buffer, sizeof(buffer), WAIT_FOREVER);

        if (ERROR == i)
        {
            printf("error msgq!\r\n");
            break;
        }
        
        j = uiUdpSend(udp_handle, m_txBuffer, g_uiSendLen, m_peerIp, m_peerPort);
        /*printf("j = %d\r\n", j);*/
    }
#endif
    return 0;
}


extern  void cSocket_ProcessRecvMsg(UINT32*pComMsg,int recLen, struct sockaddr_in src_ip);
extern  void cSocket_GetFreeBuf(UINT8* pRecBuf,UINT32*pComMsg );


void UDPReceive(int socketId,int error, void* context)
{
    M_BLK_ID mBlk;
    UINT32 copyLen, totalLen, i;
    UINT32 *pComMsg = NULL;
    UCHAR  *pData;
    UCHAR  *PRevData;    
    ipUdpPktT *udp;
   struct  sockaddr_in src_ip;

   PRevData = m_rxBuffer;
    
    if(!initialized)
    {
        return ;
    }
    /* check */
    if(socketId < 0 || socketId > UI_MAX_HANDLE)
    {
        return ;
    }
    /* read data */
    semTake(semSock, WAIT_FOREVER);
    mBlk = sockTbl[socketId].pMblk;


    cSocket_GetFreeBuf(&PRevData,&pComMsg);
    if(PRevData==NULL||(pComMsg==NULL))
    {
	semGive(semSock);
	printf("get free buf err...%x,%x\n",PRevData,pComMsg);
	return;
    }


	
    
   // printf("get free buf ...%x,%x\n",PRevData,pComMsg);
    totalLen=0;
   
    /*第一个MBLK需要去掉IP头和UDP头*/
    if(mBlk != NULL)
    {
	 udp = (ipUdpPktT *)mBlk->m_data;
	 src_ip.sin_addr.s_addr  = udp->ipHead.srcIP.dwIP;
	 src_ip.sin_port = udp->udpHead.srcPort;
	 
        mBlk->m_data += (sizeof (ipHeadT) + sizeof(udpHeadT));
        mBlk->m_len -= (sizeof (ipHeadT) + sizeof(udpHeadT));
    }
    while(mBlk)
    {        
        copyLen = mBlk->m_len - 6;
        if(totalLen + copyLen < M_TSOCKET_BUFFER_LENGTH)// lijinan  UDP_PAYLOAD_LEN)
        {            
            bcopy((UCHAR *)(mBlk->m_data), PRevData, copyLen);
            PRevData += copyLen;
            totalLen += copyLen;
            
        }
        else
        {
            printf("len=%d err", totalLen + copyLen);
        }
        mBlk = mBlk->m_next;
    }
    /*

    for(i = 0; i< g_uiSendLen; i++)
    {
        if(m_rxBuffer[i] != ((i+3)& 0xff))
        {
            break;
        }
    }*/
      udp_status.Rec_by_socket++;
    	cSocket_ProcessRecvMsg(pComMsg,totalLen, src_ip);
    
   //printf("\n\rlen=%d,rx:%x,%x,%x\n", totalLen ,m_rxBuffer[0], m_rxBuffer[1], m_rxBuffer[2]);
    semGive(semSock);
}

int StartTest(int count, int tick, int myport, unsigned long peerip, int peerport)
{
    int i;
    
    /*初始化UDP接口*/
    m_msgQ = msgQCreate(50, 20, MSG_Q_FIFO);
    m_tick = tick;
    m_myPort = myport;
    m_peerIp = peerip;
    m_peerPort = peerport;
    m_buffer[0] = 0;
    
    uiInit();
    
    udp_handle = uiOpen(dwIP, m_myPort, UDP);

    uiCallOn(udp_handle, UDPReceive, NULL);

    for(i = 0; i<3000; i++)
        m_txBuffer[i] = (i+3) &0xff;

    taskSpawn("T_Test", 60, 0, 200000, (FUNCPTR)T_Test, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    m_wd = wdCreate();
    wdStart(m_wd, tick, (FUNCPTR)tmTimer, 0);
    
}

void UIShow()
{

    printf( "\r\n**************************************************" );
    printf( "\r\n*      UITask Attributes             *" );
    printf( "\r\n**************************************************" );
    
 
    printf( "\r\n*      UITask Send  Attributes             *\n" );
    printf( "\r\n called_by_sockets:%d" , udp_status.called_by_sockets);
    printf( "\r\n send_buf_times:%d" , udp_status.send_buf_times);
    printf( "\r\n send_success_times:%d" , udp_status.send_success_times);
    printf( "\r\n send_fail_times:%d" ,udp_status.send_fail_times);
    printf( "\r\n err_times_no_blk:%d\n" , udp_status.err_times_no_blk);

    printf( "\r\n*      UITask Rec  Attributes             *\n" );
    printf( "\r\n rec_ip:%d" , udp_status.rec_ip);
    printf( "\r\n rec_arp:%d" , udp_status.rec_arp);
    printf( "\r\n rec_icmp:%d" , udp_status.rec_icmp);
    printf( "\r\n Rec_buf:%d" ,udp_status.Rec_buf);
    printf( "\r\n Rec_by_socket:%d" , udp_status.Rec_by_socket);
    printf( "\r\n rec_mux_Ring_Buf_OverFlow:%d" , udp_status.rec_mux_Ring_Buf_OverFlow);
    printf( "\r\n Rec_Ring_Buf_OverFlow:%d" , udp_status.Rec_Ring_Buf_OverFlow);
    printf( "\r\n arp_fail:%d" , udp_status.arp_fail);
     printf( "\r\n arp_fail1:%d" , udp_status.arp_fail1);
      printf( "\r\n arp_fail2:%d" , udp_status.arp_fail2);
       printf( "\r\n arp_fail3:%d" , udp_status.arp_fail3);
        printf( "\r\n arp_fail4:%d" , udp_status.arp_fail4);
    printf( "\r\n arp_wait:%d\n" , udp_status.arp_wait);
	

   
   
}
void UIClear()
{
    memset(&udp_status,0,sizeof(UDP_STATS));//wangwenhua add 20081215
}
#ifdef __cplusplus
}
#endif
