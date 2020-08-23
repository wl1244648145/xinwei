 

#ifndef	__MPICTIMER_H__
#define	__MPICTIMER_H__

#include "bsp_core.h"


#define BSP_COMMON                              (0x0)
#define ERROR_BASE_BSP                          (0x1100)     /* BSP错误码基值 */

/* BSP公用错误码基值 */
#define ERROR_BASE_BSP_COMMON                   ((ERROR_BASE_BSP + BSP_COMMON)<<16)

/*****************************************
格式化的错误码-按照上面定义的格式定义
******************************************/
#define BSP_E_POINTER_NULL                      (ERROR_BASE_BSP_COMMON + 0x0001)  /* 指针参数为空    */
#define BSP_E_INPUTPARA_ERROR                   (ERROR_BASE_BSP_COMMON + 0x0002)  /* 输入参数错误    */
#define BSP_E_UNKNOW_QUERY_CODE                 (ERROR_BASE_BSP_COMMON + 0x0003)  /* 无效的查询控制码 */
#define BSP_E_INVALID_PRPMC_TYPE                (ERROR_BASE_BSP_COMMON + 0x0004)  /* 无效的PRPMC子卡类型 */
#define BSP_E_SUBCARD_NOINSERT                  (ERROR_BASE_BSP_COMMON + 0x0005)  /* 子卡未插错误 */
#define BSP_E_MEM_MALLOC                        (ERROR_BASE_BSP_COMMON + 0x0006)  /*内存分配错误*/
#define BSP_E_MESSAGE_CREATE                    (ERROR_BASE_BSP_COMMON + 0x0007)  /*消息创建错误*/
#define BSP_E_TASK_CREATE                       (ERROR_BASE_BSP_COMMON + 0x0008)  /*任务创建错误*/
#define BSP_E_EPLD_STATUS                       (ERROR_BASE_BSP_COMMON + 0x0009)  /*EPLD状态错误*/
#define BSP_E_PCI_WR_RD                         (ERROR_BASE_BSP_COMMON + 0x000a)  /*pci设备读写错误*/
#define BSP_E_BOARD_NOT_PLUGIN                  (ERROR_BASE_BSP_COMMON + 0x000b)  /*单板没有插好*/
#define BSP_E_CONFIG_INVALID                    (ERROR_BASE_BSP_COMMON + 0x000c)  /* 配置不支持 */
#define BSP_E_NOT_SUPPORT                          (ERROR_BASE_BSP_COMMON+0x0e)      /*BSP不支持该功能*/


/* 各硬件单板错误码基值 */
#define ERROR_BASE_CC                           ((ERROR_BASE_BSP + BSP_CC)<<16)
#define ERROR_BASE_FS                           ((ERROR_BASE_BSP + BSP_FS)<<16)
#define ERROR_BASE_CHV                          ((ERROR_BASE_BSP + BSP_CHV)<<16)
#define ERROR_BASE_CHD                          ((ERROR_BASE_BSP + BSP_CHD)<<16)
#define ERROR_BASE_NISA                         ((ERROR_BASE_BSP + BSP_NIS)<<16)
#define ERROR_BASE_CVI                          ((ERROR_BASE_BSP + BSP_CVI)<<16)

/* 最小系统错误码基值*/
#define ERROR_BASE_MPC8360  	                ((ERROR_BASE_BSP + BSP_MPC8360)<<16)
#define ERROR_BASE_MPC8313 	                    ((ERROR_BASE_BSP + BSP_MPC8313)<<16)
#define ERROR_BASE_MPC8541  	                ((ERROR_BASE_BSP + BSP_MPC8541)<<16)
#define ERROR_BASE_MPC8548  	                ((ERROR_BASE_BSP + BSP_MPC8548)<<16)

/*****************************************
格式化的错误码-按照上面定义的格式定义
******************************************/
#define BSP_E_POINTER_NULL                      (ERROR_BASE_BSP_COMMON + 0x0001)  /* 指针参数为空    */
#define BSP_E_INPUTPARA_ERROR                   (ERROR_BASE_BSP_COMMON + 0x0002)  /* 输入参数错误    */
#define BSP_E_UNKNOW_QUERY_CODE                 (ERROR_BASE_BSP_COMMON + 0x0003)  /* 无效的查询控制码 */
#define BSP_E_SUBCARD_NOINSERT                  (ERROR_BASE_BSP_COMMON + 0x0005)  /* 子卡未插错误 */
#define BSP_E_MEM_MALLOC                        (ERROR_BASE_BSP_COMMON + 0x0006)  /* 内存分配错误 */
#define BSP_E_MESSAGE_CREATE                    (ERROR_BASE_BSP_COMMON + 0x0007)  /* 消息创建错误 */
#define BSP_E_TASK_CREATE                       (ERROR_BASE_BSP_COMMON + 0x0008)  /* 任务创建错误 */
#define BSP_E_EPLD_STATUS                       (ERROR_BASE_BSP_COMMON + 0x0009)  /* EPLD状态错误 */
#define BSP_E_PCI_WR_RD                         (ERROR_BASE_BSP_COMMON + 0x000a)  /* pci设备读写错误 */
#define BSP_E_BOARD_NOT_PLUGIN                  (ERROR_BASE_BSP_COMMON + 0x000b)  /* 单板没有插好 */

/* 主备控制错误码 */
#define BSP_E_BASE_MSCTRL                       (ERROR_BASE_BSP_COMMON + (DEVICE_MSCTRL<<8) )

/* 逻辑控制接口错误码 */
#define BSP_E_BASE_FPGA_CTRL                    (ERROR_BASE_BSP_COMMON + (DEVICE_FPGA_CTRL<<8) )
#define BSP_E_FPGA_INDEX                        (BSP_E_BASE_FPGA_CTRL + 0x0001) /* 指定的逻辑芯片编号出错 */
#define BSP_E_FPGA_STATUS                       (BSP_E_BASE_FPGA_CTRL + 0x0002) /* 逻辑下载后，状态非正常 */
#define BSP_E_FPGA_DATA_LENGTH                  (BSP_E_BASE_FPGA_CTRL + 0x0003) /* 逻辑数据长度错误 */
#define BSP_E_FPGA_NORESPONSE                   (BSP_E_BASE_FPGA_CTRL + 0x0004) /* 逻辑对操作无响应 */
#define BSP_E_FPGA_NOEXIST                      (BSP_E_BASE_FPGA_CTRL + 0x0005) /* FPGA设备不存在 */

/* 控制流网口错误码 */
#define BSP_E_BASE_CTRL_NET                     (ERROR_BASE_BSP_COMMON + (DEVICE_CTRL_NET<<8))
#define BSP_E_SET_CTRL_NET_MAC                  (BSP_E_BASE_CTRL_NET + 0x0001)  /* 设置控制流网口MAC地址错误 */
#define BSP_E_GET_CTRL_NET_MAC                  (BSP_E_BASE_CTRL_NET + 0x0002)  /* 获得控制流网口MAC地址错误 */
#define BSP_E_SET_CTRL_NET_IP                   (BSP_E_BASE_CTRL_NET + 0x0003)  /* 设置控制流网口IP地址错误 */
#define BSP_E_GET_CTRL_NET_IP                   (BSP_E_BASE_CTRL_NET + 0x0004)  /* 获得控制流网口IP地址错误 */
#define BSP_E_SET_CTRL_NET_MASK                 (BSP_E_BASE_CTRL_NET + 0x0005)  /* 设置控制流网口掩码错误 */
#define BSP_E_GET_CTRL_NET_MASK                 (BSP_E_BASE_CTRL_NET + 0x0006)  /* 获得控制流网口掩码错误 */
#define BSP_E_GET_CTRL_NET_NAME                 (BSP_E_BASE_CTRL_NET + 0x0007)  /* 获得控制流网口设备名错误 */
#define BSP_E_GET_CTRL_NET_NID                  (BSP_E_BASE_CTRL_NET + 0x0008)  /*获取控制流网口END名称和ID号错误*/

/* 媒体流网口错误码 */
#define BSP_E_BASE_DATA_NET                     (ERROR_BASE_BSP_COMMON + (DEVICE_DATA_NET<<8))
#define BSP_E_SET_DATA_NET_MAC                  (BSP_E_BASE_DATA_NET + 0x0001)  /* 设置媒体流MAC地址错误 */
#define BSP_E_GET_DATA_NET_MAC                  (BSP_E_BASE_DATA_NET + 0x0002)  /* 获得媒体流MAC地址错误 */
#define BSP_E_SET_DATA_NET_IP                   (BSP_E_BASE_DATA_NET + 0x0003)  /* 设置媒体流网口IP地址错误 */
#define BSP_E_GET_DATA_NET_IP                   (BSP_E_BASE_DATA_NET + 0x0004)  /* 获得媒体流网口IP地址错误 */
#define BSP_E_SET_DATA_NET_MASK                 (BSP_E_BASE_DATA_NET + 0x0005)  /* 设置媒体流网口掩码错误 */
#define BSP_E_GET_DATA_NET_MASK                 (BSP_E_BASE_DATA_NET + 0x0006)  /* 获得媒体流网口掩码错误 */
#define BSP_E_INIT_DATA_NET                     (BSP_E_BASE_DATA_NET + 0x0007)  /* 初始化媒体流网口错误*/
#define BSP_E_GET_DATA_NET_NAME                 (BSP_E_BASE_DATA_NET + 0x0008)  /* 获得媒体流网口设备名错误 */
#define BSP_E_GET_DATA_NET_NID                  (BSP_E_BASE_DATA_NET + 0x0009)  /*获取媒体流网口END名称和ID号错误*/

/* 与BSC通讯的网口错误码 */
#define BSP_E_BASE_BSC_NET                      (ERROR_BASE_BSP_COMMON + (DEVICE_BSC_NET<<8))
#define BSP_E_SET_BSC_NET_MAC                   (BSP_E_BASE_BSC_NET + 0x0001)   /* 设置与BSC通讯的网口MAC地址错误 */
#define BSP_E_GET_BSC_NET_MAC                   (BSP_E_BASE_BSC_NET + 0x0002)   /* 获得与BSC通讯的网口MAC地址错误 */
#define BSP_E_SET_BSC_NET_IP                    (BSP_E_BASE_BSC_NET + 0x0003)   /* 设置与BSC通讯的网口IP地址错误 */
#define BSP_E_GET_BSC_NET_IP                    (BSP_E_BASE_BSC_NET + 0x0004)   /* 获得与BSC通讯的网口IP地址错误 */
#define BSP_E_SET_BSC_NET_MASK                  (BSP_E_BASE_BSC_NET + 0x0005)   /* 设置与BSC通讯的网口掩码错误 */
#define BSP_E_GET_BSC_NET_MASK                  (BSP_E_BASE_BSC_NET + 0x0006)   /* 获得与BSC通讯的网口掩码错误 */
#define BSP_E_INIT_BSC_NET                      (BSP_E_BASE_BSC_NET + 0x0007)   /* 初始化BSC网口错误*/
#define BSP_E_GET_BSC_NET_NID                   (BSP_E_BASE_BSC_NET + 0x0008)   /*获取BSC网口END名称和ID号错误*/

/*BOOTROM错误码*/
#define BSP_E_BASE_BOOTROM                      (ERROR_BASE_BSP_COMMON + (DEVICE_BOOTROM<<8))

/* switch设备 */
#define BSP_E_BASE_SWITCH                       (ERROR_BASE_BSP_COMMON + (DEVICE_SWITCH<<8))

/* FLASH芯片错误码 */
#define BSP_E_BASE_FLASH                        (ERROR_BASE_BSP_COMMON + (DEVICE_FLASH<<8))

/* HDLC芯片错误码 */
#define BSP_E_BASE_HDLC                         (ERROR_BASE_BSP_COMMON + (DEVICE_HDLC<<8))

/* UART芯片错误码 */
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

typedef struct               /* Timer 初始化结构体 */
{
    UINT32 udPrd;           /* 定时器周期，以us为单位 */  
    UINT32 udClkSrc;        /* 定时器输入时钟源选择，0表示内部时钟，1表示外部时钟 */
    UINT32 udClkFreq;       /* 定时器输入时钟源的每us的Tick数量，等于时钟频率除以10^6 */
    UINT32 udWorkMode;      /* 定时器工作模式，0 保持当前值，1 OneShot模式， 2 Continue模式*/
    VOID (*pTimerCB)(UINT32 udIntParam);/* 定时器中断回调函数，若为空表示不使能定时中断 */
    UINT32 udIntParam;       /* 回调参数 */
}T_TimerInitParam;

typedef struct
{
   VOID       (*pfTimerCB)(UINT32 udIntParam); /* 回调函数 */
    UINT32     udIntParam;                      /* 回调参数 */
}T_TimerCBParam;

typedef struct 
{
    UCHAR ucRollOver;       /* 定时器是否会roll-over 0:非roll-over模式 */
	UCHAR ucRealTimeMod;    /* 时钟模式 0:内部时钟 1:外部时钟 */
	UCHAR ucClockRatio;     /* 分频比例 */
	UCHAR ucCascadeTime;	/* 与roll-over 模式相联系 */
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

