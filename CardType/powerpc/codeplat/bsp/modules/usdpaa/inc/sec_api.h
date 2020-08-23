/******************************************************************************


 **************************************************************************/
#ifndef __SEC_API_H
#define __SEC_API_H
#include "../../../com_inc/bsp_types.h"
/**
 * output buffer memory structure:
 * 
 */
#define PKTWR_JD_OVERHEAD		(64 * 2)
#define PKTWR_SG_OVERHEAD		(16 * 8)
#define PKTWR_PD_OVERHEAD		(64)
#define PKTWR_OUTPUT_OVERHEAD	(PKTWR_JD_OVERHEAD + PKTWR_SG_OVERHEAD + PKTWR_PD_OVERHEAD)/* total:320 Bytes */

#define F8_KEY_LEN		16
#define F9_KEY_LEN      16           /**< Key length(in bytes) for F9 */


struct SecF8InPd
{
	TBuf *pInBuf[5];
	TBuf *pOutBuf; /* pOutBuf中的*pucBufStart和*pucData之间至少预留320个字节用于保存SEC引擎使用的Jobdescriptor */
	UCHAR ucBufNum; /* 输入buf 的数目1-5 */
	UCHAR ucDir; 
	UCHAR res1[2];
	UCHAR *pCipherKeyAddr; /* F8加解密key */
	UCHAR *pIVAddr; /* IV 地址，上层构造好传给SEC */
	UCHAR ucAlgType; /* 算法类型 0:snow 3g F8 ; 1:AES F8 ;2:空算法*/
	UCHAR ucDRBorSRB; /* 0:DRB;1:SRB */
	UCHAR ucCoreID;/* 用于区分出队队列 */
	UCHAR res2[1];
};

struct SecF9InPd
{
	TBuf *pInBuf[5];
	TBuf *pOutBuf; /* pOutBuf中的*pucBufStart和*pucData之间至少预留320个字节用于保存SEC引擎使用的Jobdescriptor */
	UCHAR ucBufNum; /* 输入buf 的数目1-5 */
	UCHAR ucDir; 
	UCHAR res1[2];
	UCHAR *pAuthKeyAddr; /* F9认证key */
	UCHAR *pIVAddr; /* IV 地址，上层构造好传给SEC */
	UCHAR ucAlgType; /* 算法类型 0:snow 3g F9 ; 1:AES F9 ;2:空算法 */
	UCHAR ucDRBorSRB; /* 0:DRB;1:SRB */
	UCHAR ucCoreID; /* 用于区分出队队列 */
	UCHAR res2[1];
};

struct SecF9F8InPd/* 结构体参数有改动 */
{
	TBuf *pInBuf[5];
	TBuf *pOutBuf; /* pOutBuf中的*pucBufStart和*pucData之间至少预留320个字节用于保存SEC引擎使用的Jobdescriptor */
	UCHAR ucBufNum; /* 输入buf 的数目1-5 */
	UCHAR ucDir; 
	UCHAR res1[2];
	UCHAR *pAuthKeyAddr; /* F9认证key */
	UCHAR *pCipherKeyAddr; /* F8加解密key */
	ULONG HFN:27;
	ULONG seq_num:5;
	ULONG bearer:5;
	ULONG dir:1;
	ULONG rsv1:26;/* 字节对齐，上层不用填写 */
	UCHAR ucAlgType; /* 算法类型 0:snow 3g F9＋F8 ; 1:AES F9＋F8 ; 2:空算法*/
	UCHAR ucDRBorSRB; /* 0:DRB;1:SRB */
	UCHAR ucCoreID; /* 用于区分出队队列 */
	UCHAR res2[1];
};
struct SecOutPd
{
	TBuf *pInBuf[5];
	TBuf *pOutBuf; /* pOutBuf中的*pucBufStart和*pucData之间至少预留320个字节用于保存SEC引擎使用的Jobdescriptor */
	UCHAR ucBufNum; /* 输入buf 的数目1-5 */
	UCHAR ucDir; 
	UCHAR res1[2];
};
typedef ULONG (*RCV_SEC_PKT_CB_FUNCPTR)(struct SecOutPd *pOutPd, ULONG status);


#endif /* __SEC_API_H */
