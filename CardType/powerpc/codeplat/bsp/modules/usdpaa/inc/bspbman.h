 
#ifndef HEADER_BSPBMAN_H
#define HEADER_BSPBMAN_H


/***********************************************************
 *                    ������������ѡ��                     *
***********************************************************/
/***********************************************************
 *                   ��׼���Ǳ�׼ͷ�ļ�                    *
***********************************************************/
/***********************************************************
 *                        ��������                         *
***********************************************************/

/***********************************************************
 *                       ȫ�ֺ�                            *
***********************************************************/

//#define CONFIG_SYS_BMAN_MEM_BASE	(unsigned long)0x61400000
#define CONFIG_SYS_BMAN_MEM_BASE	0xff4000000


#define BMAN_CCSR_OFFSET  0x31A000
#define  BMAN_POOL0_SWDET                         0x000
#define  BMAN_POOL0_HWDET                        0x100
#define  BMAN_POOL0_SWDXT                        0x200
#define  BMAN_POOL0_HWDXT                        0x300
#define  BMAN_POOL0_SDCNT_OFFSET            0x400
#define  BMAN_POOL0_HDCNT_OFFSET           0x500
#define  BMAN_POOL0_CONTENT_OFFSET        0x600
#define  FBPR_FPC                                        0x800
#define  FBPR_FP_LWIT                                        0x804
#define  FBPR_HDPTR                                        0x808
#define  BMAN_CMD_PM0                        0x900
#define  BMAN_FL__PM0                        0x920




#define FBPR_BARE 0xC00   /* ��Data structure extended base address register R/W 0x0000_0000 7.2.4.22/7-38*/
#define FBPR_BAR  0xC04   /* ��Data structure base address register R/W 0x0000_0000 7.2.4.22/7-38         */
#define FBPR_AR   0xC10   /*    ��Data structure attributes register R/W 0x0000_0000 7.2.4.23/7-40        */

#define BMAN_BUF_PRERSV_SIZE   64
#define BMAN_BUF_POSTRSV_SIZE   64


#define BMAN_POOL_TRACE_ON
#define BMAN_TBUFCTL_CHECKWORD 0x5a5aa5a5
#define BMAN_TBUF_CHECKBYTE 0x5a


#define BMAN_POOL_CALC_TOTAL_SIZE(size, count)  	(((size) + BMAN_BUF_PRERSV_SIZE + BMAN_BUF_POSTRSV_SIZE  + UNIHEAD_SIZE_128BYTES) * (count))
#define BMAN_BUF_TOTAL_LEN(size)  ((size) + BMAN_BUF_PRERSV_SIZE + BMAN_BUF_POSTRSV_SIZE +UNIHEAD_SIZE_128BYTES)

#if 0
#ifndef UINT32
typedef unsigned  int   UINT32;
#endif
#endif

#ifndef NULL
#define NULL                ((void*)0)
#endif


/***********************************************************
 *                     ȫ����������                        *
***********************************************************/
typedef struct tagTMsgNode
{
    void         *pNode;                     /* ���нڵ����� */
    void         *pBuff;                 /* ָ������ */
    unsigned  int        udLen;                /* ���ݳ��� */
    void         *pParam;               /* ָ����������ָ�� */
}T_MsgNode;


#define MAX_BMAN_FUNCTION_NAME  16 

typedef struct tagTBufCtl
{
	unsigned long     dwUsedSize;
	unsigned long     checkword;         /*  У���� */
	unsigned long     bufPa;        /*  bman ������buf ��ʼ�����ַ */
	unsigned long     dwAplyTick; /*�����ڴ�ʱ��Tickֵ���Ա�������������Ϣ�� */
	unsigned long     bufSize;       /*  bman ������bufsize */
	unsigned short    wUse;         /*  �Ƿ�����ʹ�ã�ֻ�����ʹ��ʱ�� ����ֹ����ظ��ͷ�*/
	unsigned short    bpid;         /* �����ĸ�buf pool */
	unsigned short    dwLine;     /* �к� */
	unsigned short     dwPno;
	unsigned char     aucFuncName[MAX_BMAN_FUNCTION_NAME  + 1]; /* ���뺯���� */
	unsigned char     ucTid;
	unsigned char     srcid;         /*  д�����ݵ�һ������buf�����뷽 */
	unsigned char     dstid;         /*  ��ȡ���ݵ�һ����һ��Ϊbuf���ͷŷ� */
	T_MsgNode        tMsgNode;
}TBufCtl;
 
typedef struct t_BmPoolCfg {
	unsigned long bpid;              /* buf pool id */
	unsigned long count;             /* buf count */
	unsigned long curr;        
	unsigned long resv_size;          /* ����size����Ҫ�����ڽ���ʱ�� */
	unsigned long size;              /* data size */
} TBmPoolCfg;

typedef struct t_BmPoolGrpCtl
{
	unsigned int bpid_base; /*  */
	unsigned int bufsizemin; /* ��һ��buf pool��buf��size */
	unsigned int bufsizemax;/* ���һ��buf pool��buf��size */
	unsigned int  poolcount;
	unsigned int initflag;
}TBmPoolGrpCtl;

typedef struct t_BmPoolCtlInfo {
	unsigned long bpid;              /* buf pool id */
	unsigned long count;             /* buf count */
	unsigned long curr;        
	unsigned long resv_size;          /* ����size����Ҫ�����ڽ���ʱ�� */
	unsigned long size;              /* data size */
	unsigned long initflag;
} TBmPoolCtlInfo;

typedef struct t_BmMoniCount
{
	unsigned long dwGetSuccess; /* ������� */
	unsigned long dwGetfailed; /* ������� */
	unsigned int dwRetSuccess; /* �ͷŴ��� */
	unsigned int dwRetfailed; /* �ͷŴ��� */
	unsigned int dwreret; /* �ظ��ͷ� */
}TBmMoniCount;

/***********************************************************
 *                     ȫ�ֺ���ԭ��                        *
***********************************************************/
extern void BspUnCinvSetTlb0(int TlbSel,unsigned long ulVirAddrStartL, unsigned long ulPhyAddrStartL,unsigned short usPageSize);

extern TBmMoniCount g_atBmMoniCount[];

#endif /* HEADER_BSPBMAN_H */




