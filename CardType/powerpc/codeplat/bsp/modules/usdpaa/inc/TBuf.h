#ifndef HEADER_TBUF_H
#define HEADER_TBUF_H

#define TBUF_MEM_TYPE0  0  /* 上行SRB */
#define TBUF_MEM_TYPE1  1  /* 上行DRB */
#define TBUF_MEM_TYPE2  2  /* 下行SRB */
#define TBUF_MEM_TYPE3  3  /* 下行DRB */


enum tbuf_mem_type {
	tbuf_mem_type0 = 0, tbuf_mem_type1, tbuf_mem_type2,
	tbuf_mem_type3
};

/* 不可以修改 */
#define UNIHEAD_SIZE_128BYTES 	(128)



/* 不可以修改 */
typedef struct tagTBuf
{
	unsigned char    *pucData;       /* 存储有效数据的位置 */
	unsigned long     dwDataSize;      /*  有效数据长度       */
	//unsigned char    *pucBufStart;      /*  buf的起始地址，防止偏移越界       */
	unsigned char    *pucEnd;  /* buf的结束地址，防止偏移越界 */
	unsigned long     dwrsv0; /* 解析GTPU头后的结果 */
	unsigned long     dwrsv1;        /* 报文时间戳信息 */
	unsigned short    wrsv2;  /* 用户面UE信息 */
	unsigned short    wrsv3;   /* 解析GTPU头后的结果 */
	unsigned short    wrsv4;  /* 解析GTPU头后的结果 */
	unsigned short    wBpid;   /* 支撑接口释放内存的依据 */
	unsigned short    wMemType; /*支撑接口释放内存的依据 */
	unsigned char     ucrsv5;
	unsigned char     ucrsv6;       /* 隧道类型 */
	//unsigned char      aucrsv[28];
	//unsigned char    aucAppHeader[64]; /* 为应用保留的头部 */
}TBuf;

TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long dwline, const char *pucFuncName);
int BspRetTbuf(TBuf  *ptBuf);


#endif /* HEADER_TBUF_H */




