 

#ifndef	__MPICTIMER_H__
#define	__MPICTIMER_H__

#include "bsp_core.h"


#define BSP_COMMON                              (0x0)
#define ERROR_BASE_BSP                          (0x1100)     /* BSP�������ֵ */

/* BSP���ô������ֵ */
#define ERROR_BASE_BSP_COMMON                   ((ERROR_BASE_BSP + BSP_COMMON)<<16)

/*****************************************
��ʽ���Ĵ�����-�������涨��ĸ�ʽ����
******************************************/
#define BSP_E_POINTER_NULL                      (ERROR_BASE_BSP_COMMON + 0x0001)  /* ָ�����Ϊ��    */
#define BSP_E_INPUTPARA_ERROR                   (ERROR_BASE_BSP_COMMON + 0x0002)  /* �����������    */
#define BSP_E_UNKNOW_QUERY_CODE                 (ERROR_BASE_BSP_COMMON + 0x0003)  /* ��Ч�Ĳ�ѯ������ */
#define BSP_E_INVALID_PRPMC_TYPE                (ERROR_BASE_BSP_COMMON + 0x0004)  /* ��Ч��PRPMC�ӿ����� */
#define BSP_E_SUBCARD_NOINSERT                  (ERROR_BASE_BSP_COMMON + 0x0005)  /* �ӿ�δ����� */
#define BSP_E_MEM_MALLOC                        (ERROR_BASE_BSP_COMMON + 0x0006)  /*�ڴ�������*/
#define BSP_E_MESSAGE_CREATE                    (ERROR_BASE_BSP_COMMON + 0x0007)  /*��Ϣ��������*/
#define BSP_E_TASK_CREATE                       (ERROR_BASE_BSP_COMMON + 0x0008)  /*���񴴽�����*/
#define BSP_E_EPLD_STATUS                       (ERROR_BASE_BSP_COMMON + 0x0009)  /*EPLD״̬����*/
#define BSP_E_PCI_WR_RD                         (ERROR_BASE_BSP_COMMON + 0x000a)  /*pci�豸��д����*/
#define BSP_E_BOARD_NOT_PLUGIN                  (ERROR_BASE_BSP_COMMON + 0x000b)  /*����û�в��*/
#define BSP_E_CONFIG_INVALID                    (ERROR_BASE_BSP_COMMON + 0x000c)  /* ���ò�֧�� */
#define BSP_E_NOT_SUPPORT                          (ERROR_BASE_BSP_COMMON+0x0e)      /*BSP��֧�ָù���*/


/* ��Ӳ������������ֵ */
#define ERROR_BASE_CC                           ((ERROR_BASE_BSP + BSP_CC)<<16)
#define ERROR_BASE_FS                           ((ERROR_BASE_BSP + BSP_FS)<<16)
#define ERROR_BASE_CHV                          ((ERROR_BASE_BSP + BSP_CHV)<<16)
#define ERROR_BASE_CHD                          ((ERROR_BASE_BSP + BSP_CHD)<<16)
#define ERROR_BASE_NISA                         ((ERROR_BASE_BSP + BSP_NIS)<<16)
#define ERROR_BASE_CVI                          ((ERROR_BASE_BSP + BSP_CVI)<<16)

/* ��Сϵͳ�������ֵ*/
#define ERROR_BASE_MPC8360  	                ((ERROR_BASE_BSP + BSP_MPC8360)<<16)
#define ERROR_BASE_MPC8313 	                    ((ERROR_BASE_BSP + BSP_MPC8313)<<16)
#define ERROR_BASE_MPC8541  	                ((ERROR_BASE_BSP + BSP_MPC8541)<<16)
#define ERROR_BASE_MPC8548  	                ((ERROR_BASE_BSP + BSP_MPC8548)<<16)

/*****************************************
��ʽ���Ĵ�����-�������涨��ĸ�ʽ����
******************************************/
#define BSP_E_POINTER_NULL                      (ERROR_BASE_BSP_COMMON + 0x0001)  /* ָ�����Ϊ��    */
#define BSP_E_INPUTPARA_ERROR                   (ERROR_BASE_BSP_COMMON + 0x0002)  /* �����������    */
#define BSP_E_UNKNOW_QUERY_CODE                 (ERROR_BASE_BSP_COMMON + 0x0003)  /* ��Ч�Ĳ�ѯ������ */
#define BSP_E_SUBCARD_NOINSERT                  (ERROR_BASE_BSP_COMMON + 0x0005)  /* �ӿ�δ����� */
#define BSP_E_MEM_MALLOC                        (ERROR_BASE_BSP_COMMON + 0x0006)  /* �ڴ������� */
#define BSP_E_MESSAGE_CREATE                    (ERROR_BASE_BSP_COMMON + 0x0007)  /* ��Ϣ�������� */
#define BSP_E_TASK_CREATE                       (ERROR_BASE_BSP_COMMON + 0x0008)  /* ���񴴽����� */
#define BSP_E_EPLD_STATUS                       (ERROR_BASE_BSP_COMMON + 0x0009)  /* EPLD״̬���� */
#define BSP_E_PCI_WR_RD                         (ERROR_BASE_BSP_COMMON + 0x000a)  /* pci�豸��д���� */
#define BSP_E_BOARD_NOT_PLUGIN                  (ERROR_BASE_BSP_COMMON + 0x000b)  /* ����û�в�� */

/* �������ƴ����� */
#define BSP_E_BASE_MSCTRL                       (ERROR_BASE_BSP_COMMON + (DEVICE_MSCTRL<<8) )

/* �߼����ƽӿڴ����� */
#define BSP_E_BASE_FPGA_CTRL                    (ERROR_BASE_BSP_COMMON + (DEVICE_FPGA_CTRL<<8) )
#define BSP_E_FPGA_INDEX                        (BSP_E_BASE_FPGA_CTRL + 0x0001) /* ָ�����߼�оƬ��ų��� */
#define BSP_E_FPGA_STATUS                       (BSP_E_BASE_FPGA_CTRL + 0x0002) /* �߼����غ�״̬������ */
#define BSP_E_FPGA_DATA_LENGTH                  (BSP_E_BASE_FPGA_CTRL + 0x0003) /* �߼����ݳ��ȴ��� */
#define BSP_E_FPGA_NORESPONSE                   (BSP_E_BASE_FPGA_CTRL + 0x0004) /* �߼��Բ�������Ӧ */
#define BSP_E_FPGA_NOEXIST                      (BSP_E_BASE_FPGA_CTRL + 0x0005) /* FPGA�豸������ */

/* ���������ڴ����� */
#define BSP_E_BASE_CTRL_NET                     (ERROR_BASE_BSP_COMMON + (DEVICE_CTRL_NET<<8))
#define BSP_E_SET_CTRL_NET_MAC                  (BSP_E_BASE_CTRL_NET + 0x0001)  /* ���ÿ���������MAC��ַ���� */
#define BSP_E_GET_CTRL_NET_MAC                  (BSP_E_BASE_CTRL_NET + 0x0002)  /* ��ÿ���������MAC��ַ���� */
#define BSP_E_SET_CTRL_NET_IP                   (BSP_E_BASE_CTRL_NET + 0x0003)  /* ���ÿ���������IP��ַ���� */
#define BSP_E_GET_CTRL_NET_IP                   (BSP_E_BASE_CTRL_NET + 0x0004)  /* ��ÿ���������IP��ַ���� */
#define BSP_E_SET_CTRL_NET_MASK                 (BSP_E_BASE_CTRL_NET + 0x0005)  /* ���ÿ���������������� */
#define BSP_E_GET_CTRL_NET_MASK                 (BSP_E_BASE_CTRL_NET + 0x0006)  /* ��ÿ���������������� */
#define BSP_E_GET_CTRL_NET_NAME                 (BSP_E_BASE_CTRL_NET + 0x0007)  /* ��ÿ����������豸������ */
#define BSP_E_GET_CTRL_NET_NID                  (BSP_E_BASE_CTRL_NET + 0x0008)  /*��ȡ����������END���ƺ�ID�Ŵ���*/

/* ý�������ڴ����� */
#define BSP_E_BASE_DATA_NET                     (ERROR_BASE_BSP_COMMON + (DEVICE_DATA_NET<<8))
#define BSP_E_SET_DATA_NET_MAC                  (BSP_E_BASE_DATA_NET + 0x0001)  /* ����ý����MAC��ַ���� */
#define BSP_E_GET_DATA_NET_MAC                  (BSP_E_BASE_DATA_NET + 0x0002)  /* ���ý����MAC��ַ���� */
#define BSP_E_SET_DATA_NET_IP                   (BSP_E_BASE_DATA_NET + 0x0003)  /* ����ý��������IP��ַ���� */
#define BSP_E_GET_DATA_NET_IP                   (BSP_E_BASE_DATA_NET + 0x0004)  /* ���ý��������IP��ַ���� */
#define BSP_E_SET_DATA_NET_MASK                 (BSP_E_BASE_DATA_NET + 0x0005)  /* ����ý��������������� */
#define BSP_E_GET_DATA_NET_MASK                 (BSP_E_BASE_DATA_NET + 0x0006)  /* ���ý��������������� */
#define BSP_E_INIT_DATA_NET                     (BSP_E_BASE_DATA_NET + 0x0007)  /* ��ʼ��ý�������ڴ���*/
#define BSP_E_GET_DATA_NET_NAME                 (BSP_E_BASE_DATA_NET + 0x0008)  /* ���ý���������豸������ */
#define BSP_E_GET_DATA_NET_NID                  (BSP_E_BASE_DATA_NET + 0x0009)  /*��ȡý��������END���ƺ�ID�Ŵ���*/

/* ��BSCͨѶ�����ڴ����� */
#define BSP_E_BASE_BSC_NET                      (ERROR_BASE_BSP_COMMON + (DEVICE_BSC_NET<<8))
#define BSP_E_SET_BSC_NET_MAC                   (BSP_E_BASE_BSC_NET + 0x0001)   /* ������BSCͨѶ������MAC��ַ���� */
#define BSP_E_GET_BSC_NET_MAC                   (BSP_E_BASE_BSC_NET + 0x0002)   /* �����BSCͨѶ������MAC��ַ���� */
#define BSP_E_SET_BSC_NET_IP                    (BSP_E_BASE_BSC_NET + 0x0003)   /* ������BSCͨѶ������IP��ַ���� */
#define BSP_E_GET_BSC_NET_IP                    (BSP_E_BASE_BSC_NET + 0x0004)   /* �����BSCͨѶ������IP��ַ���� */
#define BSP_E_SET_BSC_NET_MASK                  (BSP_E_BASE_BSC_NET + 0x0005)   /* ������BSCͨѶ������������� */
#define BSP_E_GET_BSC_NET_MASK                  (BSP_E_BASE_BSC_NET + 0x0006)   /* �����BSCͨѶ������������� */
#define BSP_E_INIT_BSC_NET                      (BSP_E_BASE_BSC_NET + 0x0007)   /* ��ʼ��BSC���ڴ���*/
#define BSP_E_GET_BSC_NET_NID                   (BSP_E_BASE_BSC_NET + 0x0008)   /*��ȡBSC����END���ƺ�ID�Ŵ���*/

/*BOOTROM������*/
#define BSP_E_BASE_BOOTROM                      (ERROR_BASE_BSP_COMMON + (DEVICE_BOOTROM<<8))

/* switch�豸 */
#define BSP_E_BASE_SWITCH                       (ERROR_BASE_BSP_COMMON + (DEVICE_SWITCH<<8))

/* FLASHоƬ������ */
#define BSP_E_BASE_FLASH                        (ERROR_BASE_BSP_COMMON + (DEVICE_FLASH<<8))

/* HDLCоƬ������ */
#define BSP_E_BASE_HDLC                         (ERROR_BASE_BSP_COMMON + (DEVICE_HDLC<<8))

/* UARTоƬ������ */
#define BSP_E_BASE_UART                         (ERROR_BASE_BSP_COMMON + (DEVICE_UART<<8))

/*PEF22558*/
#define BSP_E_BASE_PEF22558                     (ERROR_BASE_BSP_COMMON + (DEVICE_PEF22558<<8))

/*SW*/
#define BSP_E_BASE_SW                           (ERROR_BASE_BSP_COMMON + (DEVICE_SW<<8))

/*IPMB*/
#define BSP_E_BASE_IPMB                         (ERROR_BASE_BSP_COMMON + (DEVICE_IPMB<<8))

/*I2C*/
#define BSP_E_BASE_I2C                         (ERROR_BASE_BSP_COMMON + (DEVICE_I2C<<8))
#define I2C_ERR_DEV_NO                         (BSP_E_BASE_I2C + 0x1)
#define I2C_ERR_LINEBUZY                       (BSP_E_BASE_I2C + 0x2)
#define I2C_ERR_ACK                            (BSP_E_BASE_I2C + 0x3)
#define I2C_ERR_SEMTAKE                        (BSP_E_BASE_I2C + 0x5)

typedef struct               /* Timer ��ʼ���ṹ�� */
{
    UINT32 udPrd;           /* ��ʱ�����ڣ���usΪ��λ */  
    UINT32 udClkSrc;        /* ��ʱ������ʱ��Դѡ��0��ʾ�ڲ�ʱ�ӣ�1��ʾ�ⲿʱ�� */
    UINT32 udClkFreq;       /* ��ʱ������ʱ��Դ��ÿus��Tick����������ʱ��Ƶ�ʳ���10^6 */
    UINT32 udWorkMode;      /* ��ʱ������ģʽ��0 ���ֵ�ǰֵ��1 OneShotģʽ�� 2 Continueģʽ*/
    VOID (*pTimerCB)(UINT32 udIntParam);/* ��ʱ���жϻص���������Ϊ�ձ�ʾ��ʹ�ܶ�ʱ�ж� */
    UINT32 udIntParam;       /* �ص����� */
}T_TimerInitParam;

typedef struct
{
   VOID       (*pfTimerCB)(UINT32 udIntParam); /* �ص����� */
    UINT32     udIntParam;                      /* �ص����� */
}T_TimerCBParam;

typedef struct 
{
    UCHAR ucRollOver;       /* ��ʱ���Ƿ��roll-over 0:��roll-overģʽ */
	UCHAR ucRealTimeMod;    /* ʱ��ģʽ 0:�ڲ�ʱ�� 1:�ⲿʱ�� */
	UCHAR ucClockRatio;     /* ��Ƶ���� */
	UCHAR ucCascadeTime;	/* ��roll-over ģʽ����ϵ */
}T_MPIC_TIMER_CONFIG;



#define MPIC_GROUPA_TIME0         0
#define MPIC_GROUPA_TIME1         1
#define MPIC_GROUPA_TIME2         2
#define MPIC_GROUPA_TIME3         3
#define MPIC_GROUPB_TIME0         4
#define MPIC_GROUPB_TIME1         5
#define MPIC_GROUPB_TIME2         6
#define MPIC_GROUPB_TIME3         7

#define P3_MPIC_GT_DISABLE                     (0x80000000)
#define P3_MPIC_GT_BASECNT_MAX_VAL             (0x7fffffff)
#define P3_MPIC_GT_CURCNT_VAL_MASK            (0x7fffffff)

#define TIME_BASE_TIMER_INDEX 4



ULONG BspGetMpicTBCurCnt(UINT32 udTimerIndex);



#endif

