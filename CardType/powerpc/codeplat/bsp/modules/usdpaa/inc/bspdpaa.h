 
#ifndef HEADER_BSPDPAA_H
#define HEADER_BSPDPAA_H



/***********************************************************
 *                    其它条件编译选项                     *
***********************************************************/
/***********************************************************
 *                   标准、非标准头文件                    *
***********************************************************/
#include "TBuf.h"
//#include "bspshmem.h"


/***********************************************************
 *                        常量定义                         *
***********************************************************/

/***********************************************************
 *                       全局宏                            *
***********************************************************/
typedef  unsigned int (*RECVGMAC_FUNCPTR)( TBuf* ptBuf);
extern unsigned int  BspEmacRegRecvCallBack (RECVGMAC_FUNCPTR pCallBack);

extern TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long dwline, const char *pucFuncName);
extern int BspRetTbuf(TBuf  *ptBuf);

#endif /* HEADER_BSPDPAA_H */




