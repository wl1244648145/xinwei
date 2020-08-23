 


#ifndef __HEADER_FQID_H
#define __HEADER_FQID_H


/******** linux�ں�Э��ջʹ�õ�fqid �������պͷ���*********************/
#define FQID_LINUX_KERNEL           ((unsigned int)1)   /* linux�û��汨�Ľ���fqid */
#define FQNUM_LINUX_KERNEL        ((unsigned int)999) 
#define FQID_LINUX_KERNEL_END   ((unsigned int)FQID_LINUX_KERNEL + FQNUM_LINUX_KERNEL - 1)   /*999 */
/******** linux�ں�Э��ջʹ�õ�fqid �������պͷ���*********************/


/***************** �û��汨�Ľ���FQID��ÿ��coreһ�� ***********************/
#define FQID_GTPU_UDP           ((unsigned int)FQID_LINUX_KERNEL_END + 1)   /* linux�û��汨�Ľ���fqid = 1000*/
#define   RX_USDPAA_FQID_BASE               FQID_GTPU_UDP /* 1000 */
#define FQNUM_GTPU_UDP        ((unsigned int)100) 
#define FQID_GTPU_UDP_END   ((unsigned int)FQID_GTPU_UDP + FQNUM_GTPU_UDP - 1)    /* 1099 */
/***************** �û��汨�Ľ���FQID��ÿ��coreһ�� ***********************/


/************************* core0(�û�̬)���Ӻ˱��ķ���FQID��ÿcoreÿ����һ�� ************/
/******* ���㹫ʽ��fqid =  FQID_MACTX_BASE + (portnum * MAX_CORE_NUM) + BspGetSelfCoreId()****************/
#define FQID_MACTX_BASE   ((unsigned int)FQID_GTPU_UDP_END + 1)   /* 1100 */
#define FQNUM_MACTX   ((unsigned int)100)
#define FQID_MACTX_END     ((unsigned int)FQID_MACTX_BASE + FQNUM_MACTX - 1)  /* 1199 */
/************************* �û�̬���Ӻ˱��ķ���FQID��ÿcoreÿ����һ�� ************/

/********************************* ��������������FQID***********************************/
#define FQID_SEC_BASE   ((unsigned int)FQID_MACTX_END + 1)  /* 1200 */
#define FQNUM_SEC     ((unsigned int)100)
#define FQID_SEC_END     ((unsigned int)FQID_SEC_BASE + FQNUM_SEC - 1) /* 1299 */
/********************************* ��������������FQID***********************************/



/****************************** �Ӻ�Э��ջ���Ľ���FQID��ÿcoreÿ����һ�� ************/
#define FQID_MACRX_BASE   ((unsigned int)FQID_SEC_END + 1) /* 1300 */
#define   RX_LWOS1_FQID_BASE                (FQID_MACRX_BASE  +  1)  /* 1301 */
#define   RX_LWOS2_FQID_BASE                (FQID_MACRX_BASE  +  2)/* 1302 */
#define   RX_LWOS3_FQID_BASE                (FQID_MACRX_BASE  +  3)/* 1303 */
#define   RX_LWOS4_FQID_BASE                (FQID_MACRX_BASE  +  4)/* 1304 */
#define   RX_LWOS5_FQID_BASE                (FQID_MACRX_BASE  +  5)/* 1305 */
#define   RX_LWOS6_FQID_BASE                (FQID_MACRX_BASE  +  6)/* 1306 */
#define   RX_LWOS7_FQID_BASE                (FQID_MACRX_BASE  +  7)/* 1307 */
#define FQNUM_MACRX   ((unsigned int)100)
#define FQID_MACRx_END     ((unsigned int)FQID_MACRX_BASE + FQNUM_MACRX - 1) /* 1399 */
/****************************** �Ӻ�Э��ջ���Ľ���FQID��ÿcoreÿ����һ�� ************/


/****************************** �Ӻ�Э��ջ���Ľ���FQID��ÿcoreÿ����һ�� ************/
#define FQID_SRIO_BASE   ((unsigned int)FQID_MACRx_END + 1)   /* 1400 */

#define FQNUM_SRIO   ((unsigned int)100)
#define FQID_SRIO_END     ((unsigned int)FQID_SRIO_BASE + FQNUM_SRIO -1) /* 1499 */
/****************************** �Ӻ�Э��ջ���Ľ���FQID��ÿcoreÿ����һ�� ************/


/************************* �ں˸������ڵķ���fqid(ת��) *****************************/
#define   FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE       (FQID_SRIO_END + 1)  /* 1500 */
#define   FM0_TX0_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 1)/* 1501 */
#define   FM0_TX1_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 2)/* 1502 */
#define   FM0_TX2_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 3)/* 1503 */
#define   FM0_TX3_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 4)/* 1504 */
#define   FM0_TX4_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 5)/* 1505 */                                            
#define   FM1_TX0_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 6)/* 1506 */
#define   FM1_TX1_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 7)/* 1507 */
#define   FM1_TX2_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 8)/* 1508 */
#define   FM1_TX3_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 9)/* 1509 */
#define   FM1_TX4_NOCONFIRM_FQID_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + 10)/* 1510 */
#define   FM0_TX0_NOCONFIRM_FQID_KERNEL_NUM       100
#define   FM0_TX0_NOCONFIRM_FQID_KERNEL_END       (FM0_TX0_NOCONFIRM_FQID_KERNEL_BASE + FM0_TX0_NOCONFIRM_FQID_KERNEL_NUM - 1)  /* 1599 */
/************************* �ں˸������ڵķ���fqid(ת��) *****************************/

/************************* MGR���̿����淢��fqid *****************************/
#define   FQID_MACTX_CTRL_BASE       (FM0_TX0_NOCONFIRM_FQID_KERNEL_END + 1)  /* 1600 */

#define  FQID_MACTX_CTRL_TEST   (FQID_MACTX_CTRL_BASE + 0)

#define   FQNUM_MACTX_CTRL       100
#define   FQID_MACTX_CTRL_END       (FQID_MACTX_CTRL_BASE + FQNUM_MACTX_CTRL - 1)  /* 1699 */
/************************* MGR���̿����淢��fqid *****************************/


#endif /* __HEADER_FQID_H */
/******************************* ͷ�ļ����� ********************************/

