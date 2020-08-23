/******************************************************************************
模块名         :  bman
文件名         :  bpid.h
相关文件       :  
文件实现功能   : bman buf pool id 规划
作者           :  
版本           :  
-------------------------------------------------------------------------------
修改记录 : 
日  期          版本        修改人      修改内容 
2011/07/08				
******************************************************************************/

#ifndef HEADER_BPID_H
#define HEADER_BPID_H

#define MAX_BMAN_POOL_NUM    64


/* 内核qman使用的bpid */
#define BMAN_BPID_KERNEL_QMANMALLOC         0

/* 用户面ip报文重组使用的bpid*/
#define BMAN_BPID_IP_REASSEMBLE                   1


/* 从核网口发送使用的bpid */
#define BMAN_BPID_CORE0_MAC_TX                    2
#define BMAN_BPID_CORE7_MAC_TX                    9


/* 内核池集0使用的bpid */
#define BMAN_BPID_TBUF_MEM_TYPE0_BASE       10
#define BMAN_BPID_TBUF_MEM_TYPE0_END       19

/* 内核池集1使用的bpid */
#define BMAN_BPID_TBUF_MEM_TYPE1_BASE       20
#define BMAN_BPID_TBUF_MEM_TYPE1_END       29

/* 内核池集2使用的bpid */
#define BMAN_BPID_TBUF_MEM_TYPE2_BASE       30
#define BMAN_BPID_TBUF_MEM_TYPE2_END       39

/* 内核池集3使用的bpid */
#define BMAN_BPID_TBUF_MEM_TYPE3_BASE       40
#define BMAN_BPID_TBUF_MEM_TYPE3_END       49


/* 与CC通信的网口接收 使用的bpid */
#define BMAN_BPID_MCH_RX_BASE                  50

/* 内核网口发送和接收使用的bpid，每网口一个 */
#define BMAN_BPID_KERNEL_RX_BASE                  51
#define BMAN_BPID_KERNEL_RX_END                    63


#endif /* HEADER_BPID_H */




