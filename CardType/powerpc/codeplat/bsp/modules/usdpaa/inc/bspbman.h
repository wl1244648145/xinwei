 
#ifndef HEADER_BSPBMAN_H
#define HEADER_BSPBMAN_H


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




#define FBPR_BARE 0xC00   /* —Data structure extended base address register R/W 0x0000_0000 7.2.4.22/7-38*/
#define FBPR_BAR  0xC04   /* —Data structure base address register R/W 0x0000_0000 7.2.4.22/7-38         */
#define FBPR_AR   0xC10   /*    —Data structure attributes register R/W 0x0000_0000 7.2.4.23/7-40        */

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
 *                     全局数据类型                        *
***********************************************************/
typedef struct tagTMsgNode
{
    void         *pNode;                     /* 队列节点描述 */
    void         *pBuff;                 /* 指向数据 */
    unsigned  int        udLen;                /* 数据长度 */
    void         *pParam;               /* 指向参数区域的指针 */
}T_MsgNode;


#define MAX_BMAN_FUNCTION_NAME  16 

typedef struct tagTBufCtl
{
	unsigned long     dwUsedSize;
	unsigned long     checkword;         /*  校验字 */
	unsigned long     bufPa;        /*  bman 看到的buf 起始物理地址 */
	unsigned long     dwAplyTick; /*申请内存时的Tick值，以便随后输出跟踪消息用 */
	unsigned long     bufSize;       /*  bman 看到的bufsize */
	unsigned short    wUse;         /*  是否正在使用，只给软件使用时用 ，防止软件重复释放*/
	unsigned short    bpid;         /* 所属哪个buf pool */
	unsigned short    dwLine;     /* 行号 */
	unsigned short     dwPno;
	unsigned char     aucFuncName[MAX_BMAN_FUNCTION_NAME  + 1]; /* 申请函数名 */
	unsigned char     ucTid;
	unsigned char     srcid;         /*  写入数据的一方，即buf的申请方 */
	unsigned char     dstid;         /*  读取数据的一方，一般为buf的释放方 */
	T_MsgNode        tMsgNode;
}TBufCtl;
 
typedef struct t_BmPoolCfg {
	unsigned long bpid;              /* buf pool id */
	unsigned long count;             /* buf count */
	unsigned long curr;        
	unsigned long resv_size;          /* 保留size，主要是网口接收时用 */
	unsigned long size;              /* data size */
} TBmPoolCfg;

typedef struct t_BmPoolGrpCtl
{
	unsigned int bpid_base; /*  */
	unsigned int bufsizemin; /* 第一个buf pool中buf的size */
	unsigned int bufsizemax;/* 最后一个buf pool中buf的size */
	unsigned int  poolcount;
	unsigned int initflag;
}TBmPoolGrpCtl;

typedef struct t_BmPoolCtlInfo {
	unsigned long bpid;              /* buf pool id */
	unsigned long count;             /* buf count */
	unsigned long curr;        
	unsigned long resv_size;          /* 保留size，主要是网口接收时用 */
	unsigned long size;              /* data size */
	unsigned long initflag;
} TBmPoolCtlInfo;

typedef struct t_BmMoniCount
{
	unsigned long dwGetSuccess; /* 申请次数 */
	unsigned long dwGetfailed; /* 申请次数 */
	unsigned int dwRetSuccess; /* 释放次数 */
	unsigned int dwRetfailed; /* 释放次数 */
	unsigned int dwreret; /* 重复释放 */
}TBmMoniCount;

/***********************************************************
 *                     全局函数原型                        *
***********************************************************/
extern void BspUnCinvSetTlb0(int TlbSel,unsigned long ulVirAddrStartL, unsigned long ulPhyAddrStartL,unsigned short usPageSize);

extern TBmMoniCount g_atBmMoniCount[];

#endif /* HEADER_BSPBMAN_H */




