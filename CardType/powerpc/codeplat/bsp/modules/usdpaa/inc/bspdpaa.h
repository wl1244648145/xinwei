 
#ifndef HEADER_BSPDPAA_H
#define HEADER_BSPDPAA_H



/***********************************************************
 *                    ������������ѡ��                     *
***********************************************************/
/***********************************************************
 *                   ��׼���Ǳ�׼ͷ�ļ�                    *
***********************************************************/
#include "TBuf.h"
//#include "bspshmem.h"


/***********************************************************
 *                        ��������                         *
***********************************************************/

/***********************************************************
 *                       ȫ�ֺ�                            *
***********************************************************/
typedef  unsigned int (*RECVGMAC_FUNCPTR)( TBuf* ptBuf);
extern unsigned int  BspEmacRegRecvCallBack (RECVGMAC_FUNCPTR pCallBack);

extern TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long dwline, const char *pucFuncName);
extern int BspRetTbuf(TBuf  *ptBuf);

#endif /* HEADER_BSPDPAA_H */




