#ifndef HEADER_TBUF_H
#define HEADER_TBUF_H

#define TBUF_MEM_TYPE0  0  /* ����SRB */
#define TBUF_MEM_TYPE1  1  /* ����DRB */
#define TBUF_MEM_TYPE2  2  /* ����SRB */
#define TBUF_MEM_TYPE3  3  /* ����DRB */


enum tbuf_mem_type {
	tbuf_mem_type0 = 0, tbuf_mem_type1, tbuf_mem_type2,
	tbuf_mem_type3
};

/* �������޸� */
#define UNIHEAD_SIZE_128BYTES 	(128)



/* �������޸� */
typedef struct tagTBuf
{
	unsigned char    *pucData;       /* �洢��Ч���ݵ�λ�� */
	unsigned long     dwDataSize;      /*  ��Ч���ݳ���       */
	//unsigned char    *pucBufStart;      /*  buf����ʼ��ַ����ֹƫ��Խ��       */
	unsigned char    *pucEnd;  /* buf�Ľ�����ַ����ֹƫ��Խ�� */
	unsigned long     dwrsv0; /* ����GTPUͷ��Ľ�� */
	unsigned long     dwrsv1;        /* ����ʱ�����Ϣ */
	unsigned short    wrsv2;  /* �û���UE��Ϣ */
	unsigned short    wrsv3;   /* ����GTPUͷ��Ľ�� */
	unsigned short    wrsv4;  /* ����GTPUͷ��Ľ�� */
	unsigned short    wBpid;   /* ֧�Žӿ��ͷ��ڴ������ */
	unsigned short    wMemType; /*֧�Žӿ��ͷ��ڴ������ */
	unsigned char     ucrsv5;
	unsigned char     ucrsv6;       /* ������� */
	//unsigned char      aucrsv[28];
	//unsigned char    aucAppHeader[64]; /* ΪӦ�ñ�����ͷ�� */
}TBuf;

TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long dwline, const char *pucFuncName);
int BspRetTbuf(TBuf  *ptBuf);


#endif /* HEADER_TBUF_H */




