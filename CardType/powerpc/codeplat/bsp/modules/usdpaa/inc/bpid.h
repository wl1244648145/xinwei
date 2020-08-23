/******************************************************************************
ģ����         :  bman
�ļ���         :  bpid.h
����ļ�       :  
�ļ�ʵ�ֹ���   : bman buf pool id �滮
����           :  
�汾           :  
-------------------------------------------------------------------------------
�޸ļ�¼ : 
��  ��          �汾        �޸���      �޸����� 
2011/07/08				
******************************************************************************/

#ifndef HEADER_BPID_H
#define HEADER_BPID_H

#define MAX_BMAN_POOL_NUM    64


/* �ں�qmanʹ�õ�bpid */
#define BMAN_BPID_KERNEL_QMANMALLOC         0

/* �û���ip��������ʹ�õ�bpid*/
#define BMAN_BPID_IP_REASSEMBLE                   1


/* �Ӻ����ڷ���ʹ�õ�bpid */
#define BMAN_BPID_CORE0_MAC_TX                    2
#define BMAN_BPID_CORE7_MAC_TX                    9


/* �ں˳ؼ�0ʹ�õ�bpid */
#define BMAN_BPID_TBUF_MEM_TYPE0_BASE       10
#define BMAN_BPID_TBUF_MEM_TYPE0_END       19

/* �ں˳ؼ�1ʹ�õ�bpid */
#define BMAN_BPID_TBUF_MEM_TYPE1_BASE       20
#define BMAN_BPID_TBUF_MEM_TYPE1_END       29

/* �ں˳ؼ�2ʹ�õ�bpid */
#define BMAN_BPID_TBUF_MEM_TYPE2_BASE       30
#define BMAN_BPID_TBUF_MEM_TYPE2_END       39

/* �ں˳ؼ�3ʹ�õ�bpid */
#define BMAN_BPID_TBUF_MEM_TYPE3_BASE       40
#define BMAN_BPID_TBUF_MEM_TYPE3_END       49


/* ��CCͨ�ŵ����ڽ��� ʹ�õ�bpid */
#define BMAN_BPID_MCH_RX_BASE                  50

/* �ں����ڷ��ͺͽ���ʹ�õ�bpid��ÿ����һ�� */
#define BMAN_BPID_KERNEL_RX_BASE                  51
#define BMAN_BPID_KERNEL_RX_END                    63


#endif /* HEADER_BPID_H */




