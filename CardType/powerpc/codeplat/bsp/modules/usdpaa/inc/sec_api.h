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
	TBuf *pOutBuf; /* pOutBuf�е�*pucBufStart��*pucData֮������Ԥ��320���ֽ����ڱ���SEC����ʹ�õ�Jobdescriptor */
	UCHAR ucBufNum; /* ����buf ����Ŀ1-5 */
	UCHAR ucDir; 
	UCHAR res1[2];
	UCHAR *pCipherKeyAddr; /* F8�ӽ���key */
	UCHAR *pIVAddr; /* IV ��ַ���ϲ㹹��ô���SEC */
	UCHAR ucAlgType; /* �㷨���� 0:snow 3g F8 ; 1:AES F8 ;2:���㷨*/
	UCHAR ucDRBorSRB; /* 0:DRB;1:SRB */
	UCHAR ucCoreID;/* �������ֳ��Ӷ��� */
	UCHAR res2[1];
};

struct SecF9InPd
{
	TBuf *pInBuf[5];
	TBuf *pOutBuf; /* pOutBuf�е�*pucBufStart��*pucData֮������Ԥ��320���ֽ����ڱ���SEC����ʹ�õ�Jobdescriptor */
	UCHAR ucBufNum; /* ����buf ����Ŀ1-5 */
	UCHAR ucDir; 
	UCHAR res1[2];
	UCHAR *pAuthKeyAddr; /* F9��֤key */
	UCHAR *pIVAddr; /* IV ��ַ���ϲ㹹��ô���SEC */
	UCHAR ucAlgType; /* �㷨���� 0:snow 3g F9 ; 1:AES F9 ;2:���㷨 */
	UCHAR ucDRBorSRB; /* 0:DRB;1:SRB */
	UCHAR ucCoreID; /* �������ֳ��Ӷ��� */
	UCHAR res2[1];
};

struct SecF9F8InPd/* �ṹ������иĶ� */
{
	TBuf *pInBuf[5];
	TBuf *pOutBuf; /* pOutBuf�е�*pucBufStart��*pucData֮������Ԥ��320���ֽ����ڱ���SEC����ʹ�õ�Jobdescriptor */
	UCHAR ucBufNum; /* ����buf ����Ŀ1-5 */
	UCHAR ucDir; 
	UCHAR res1[2];
	UCHAR *pAuthKeyAddr; /* F9��֤key */
	UCHAR *pCipherKeyAddr; /* F8�ӽ���key */
	ULONG HFN:27;
	ULONG seq_num:5;
	ULONG bearer:5;
	ULONG dir:1;
	ULONG rsv1:26;/* �ֽڶ��룬�ϲ㲻����д */
	UCHAR ucAlgType; /* �㷨���� 0:snow 3g F9��F8 ; 1:AES F9��F8 ; 2:���㷨*/
	UCHAR ucDRBorSRB; /* 0:DRB;1:SRB */
	UCHAR ucCoreID; /* �������ֳ��Ӷ��� */
	UCHAR res2[1];
};
struct SecOutPd
{
	TBuf *pInBuf[5];
	TBuf *pOutBuf; /* pOutBuf�е�*pucBufStart��*pucData֮������Ԥ��320���ֽ����ڱ���SEC����ʹ�õ�Jobdescriptor */
	UCHAR ucBufNum; /* ����buf ����Ŀ1-5 */
	UCHAR ucDir; 
	UCHAR res1[2];
};
typedef ULONG (*RCV_SEC_PKT_CB_FUNCPTR)(struct SecOutPd *pOutPd, ULONG status);


#endif /* __SEC_API_H */
