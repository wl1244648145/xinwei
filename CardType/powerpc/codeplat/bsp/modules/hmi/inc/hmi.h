/*******************************************************************************
* *  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 文件名称:  cps1616.h
* 功    能:  
* 版    本:  V0.1
* 编写日期:  2015/06/26
* 说    明:  无
* 修改历史:
* 修改日期           修改人  liuganga 修改内容
*------------------------------------------------------------------------------
*------------------------------------------------------------------------------
*                                                         创建文件
*
*
*******************************************************************************/
/******************************** 头文件保护开头 ******************************/

#ifndef __BSP_HMI_H__
#define __BSP_HMI_H__
#include "../../../com_inc/bsp_types.h"

#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed and slave address */
#define IPMB_BROADCAST_ADDR            		(0x00)
#define IPMB_MCT_HMI1_I2C_ADDR_BASE         (0x20)
#define IPMB_MCT_HMI2_I2C_ADDR_BASE         (0x30)
#define IPMB_BBP_HMI1_I2C_ADDR_BASE         (0x22)
#define IPMB_BBP_HMI2_I2C_ADDR_BASE         (0x32)
#define IPMB_FSA_HMI1_I2C_ADDR_BASE         (0x26)
#define IPMB_FSA_HMI2_I2C_ADDR_BASE         (0x36)
#define IPMB_PEU_HMI1_I2C_ADDR_BASE         (0x28)
#define IPMB_PEU_HMI2_I2C_ADDR_BASE         (0x38)
#define IPMB_FAN_HMI1_I2C_ADDR_BASE         (0x2a)
#define IPMB_FAN_HMI2_I2C_ADDR_BASE         (0x3a)

#define IPMB_SLOT0                     (0)
#define IPMB_SLOT1                     (1)
#define IPMB_SLOT2                     (2)
#define IPMB_SLOT3                     (3)
#define IPMB_SLOT4                     (4)
#define IPMB_SLOT5                     (5)
#define IPMB_SLOT6                     (6)
#define IPMB_SLOT7                     (7)
#define IPMB_SLOT8                     (8)
#define IPMB_SLOT9                     (9)
#define IPMB_SLOT10                    (10)

#define IPMB_MCT_HMI1_I2C_ADDR(n)      (IPMB_MCT_HMI1_I2C_ADDR_BASE+(n))
#define IPMB_MCT_HMI2_I2C_ADDR(n)      (IPMB_MCT_HMI2_I2C_ADDR_BASE+(n))
#define IPMB_BBP_HMI1_I2C_ADDR(n)      (IPMB_BBP_HMI1_I2C_ADDR_BASE+(n-2))
#define IPMB_BBP_HMI2_I2C_ADDR(n)      (IPMB_BBP_HMI2_I2C_ADDR_BASE+(n-2))
#define IPMB_FSA_HMI1_I2C_ADDR(n)      (IPMB_FSA_HMI1_I2C_ADDR_BASE+(n-6))
#define IPMB_FSA_HMI2_I2C_ADDR(n)      (IPMB_FSA_HMI2_I2C_ADDR_BASE+(n-6))
#define IPMB_PEU_HMI1_I2C_ADDR(n)      (IPMB_PEU_HMI1_I2C_ADDR_BASE+(n-8))
#define IPMB_PEU_HMI2_I2C_ADDR(n)      (IPMB_PEU_HMI2_I2C_ADDR_BASE+(n-8))
#define IPMB_FAN_HMI1_I2C_ADDR         (IPMB_FAN_HMI1_I2C_ADDR_BASE)
#define IPMB_FAN_HMI2_I2C_ADDR         (IPMB_FAN_HMI2_I2C_ADDR_BASE)

#define BOARD_TYPE_MCT    (0x4)
#define BOARD_TYPE_BBP    (0x2)
#define BOARD_TYPE_GES    (0x5)
#define BOARD_TYPE_FSA    (0x5)
#define BOARD_TYPE_ES      (0x7)
#define BOARD_TYPE_FAN    (0x1)
#define BOARD_TYPE_PEU    (0x3)
#define BOARD_TYPE_NULL  (0x0)
#define BOARD_TYPE_ALL	   (0xff)

/*
定义IPMB的命令号：
公共命令区:0x01~~0x1f 
主控板的命令区域:0x20~~~0x52
基带板的命令区域:0x53~~~0x85
交换板的命令区域:0x86~~~0xb8 
风扇板的命令区域:0xb9~~~0xeb
保留:0xec~~0xff
*/

/*定义公共命令区*/
#define IPMB_CMD_COMM_RESET_BOARDS                   (0x01)
#define IPMB_CMD_COMM_STOP_WATCHDOG               (0x02)
#define IPMB_CMD_COMM_FIRST_MESSAGE                  (0x03)
#define IPMB_CMD_COMM_CHECK_ALIVE                       (0x04)
#define IPMB_CMD_COMM_START_SEND_MESSAGE       (0x05)
#define IPMB_CMD_COMM_STOP_SEND_MESSAGE         (0x06)
#define IPMB_CMD_COMM_ALIVE_CMD                          (0x10)

/*定义主控板命令区*/
#define IPMB_CMD_MAINBANDCTL_TEMP              (0x11)/*获取主控温度*/
/*定义环境板命令区*/
#define IPMB_CMD_PEUACTL_GET_ARM_VER_VAL       (0x1c)/*获取arm版本号*/
#define IPMB_CMD_PEUACTL_POWER_ON                    (0x1d)/*单板上电*/
#define IPMB_CMD_PEUACTL_POWER_OFF                   (0x1e)/*单板下电*/
#define IPMB_CMD_PEUACTL_SEND_PACK_OK_CNT     (0x1f)/*发送报文成功数统计*/
#define IPMB_CMD_PEUACTL_SEND_PACK_ERR_CNT    (0x20)/*发送报文失败数统计*/
#define IPMB_CMD_PEUACTL_RECV_PACK_OK_CNT      (0x21)/*接收报文成功数统计*/
#define IPMB_CMD_PEUACTL_RECV_PACK_ERR_CNT    (0x22)/*接收报文失败数统计*/
#define IPMB_CMD_PEUACTL_TEST                           (0x23)/* HMI链路测试 */
#define IPMB_CMD_PEUACTL_REBOOT                      (0x24)/* 重启命令 */
#define IPMB_CMD_PEUACTL_HEART_BEAT_GAP      (0x25)/* 心跳间隔设置 */
#define IPMB_CMD_PEUACTL_GET_BOARD_TYPE	     (0x26)// 获取板类型
#define IPMB_CMD_PEUACTL_GET_PCB_VERSION     (0x27)// 获取PCB版本号
#define IPMB_CMD_PEUACTL_GET_SLOT	             (0x28)// 获取槽位号
#define IPMB_CMD_PEUACTL_SEND_EXCEPTION       (0x29)/* 异常上报 */
#define IPMB_CMD_PEUACTL_GET_TEMP                   (0x2A)/* 温度获取 */
#define IPMB_CMD_PEUACTL_GET_DRYIN_STATE      (0x2B)/* 干结点状态获取 */
#define IPMB_CMD_PEUACTL_RS485_TEST                (0x2C)/* RS485测试 */

/*定义交换板fsa命令区*/
#define IPMB_CMD_FSA_REBOOT                        (0x3b)
#define IPMB_CMD_FSA_POWER_ON                   (0x3c)
#define IPMB_CMD_FSA_POWER_OFF                  (0x3d)
#define IPMB_CMD_FSA_GET_ARM_VER_VAL      (0x3e)
#define IPMB_CMD_FSA_GET_TEMP                     (0x3f)
#define IPMB_CMD_FSA_HEART_BEAT_GAP         (0x40)
#define IPMB_CMD_FSA_TEST_GESWITCH           (0x41)
#define IPMB_CMD_FSA_TEST_SRIOSW               (0x42)
#define IPMB_CMD_FSA_TEST_EEPROM               (0x43)
#define IPMB_CMD_FSA_TEST                              (0x44)

/*定义ES板命令区*/
#define IPMB_CMD_ES_REBOOT                        (0x46)
#define IPMB_CMD_ES_RESET_GESW                (0x49)
#define IPMB_CMD_ES_GET_ARM_VER_VAL      (0x4a)
#define IPMB_CMD_ES_GET_TEMP                     (0x4b)
#define IPMB_CMD_ES_HEART_BEAT_GAP         (0x4c)
#define IPMB_CMD_ES_TEST_GESWITCH           (0x4d)
#define IPMB_CMD_ES_TEST_EEPROM               (0x4e)
#define IPMB_CMD_ES_TEST                              (0x4f)

/*定义基带板命令区*/
#define IPMB_CMD_BASEBANDCTL_WRITE_FPGA        (0x53)/*写fpga寄存器*/
#define IPMB_CMD_BASEBANDCTL_READ_FPGA         (0x54)/*读fpga寄存器*/
#define IPMB_CMD_BASEBANDCTL_WRITE_EPLD        (0x55)/*写epld寄存器*/
#define IPMB_CMD_BASEBANDCTL_READ_EPLD         (0x56)/*读epld寄存器*/
#define IPMB_CMD_BASEBANDCTL_GET_ARM_VER_VAL   (0x57)/*获取arm版本号*/
#define IPMB_CMD_BASEBANDCTL_GET_TEMP          (0x58)/*获取基带板温度*/

#define IPMB_CMD_BASEBANDCTL_RESET_DSP0        (0x59)/*复位dsp0*/
#define IPMB_CMD_BASEBANDCTL_RESET_DSP1        (0x5A)/*复位dsp1*/
#define IPMB_CMD_BASEBANDCTL_RESET_DSP2        (0x5B)/*复位dsp2*/
#define IPMB_CMD_BASEBANDCTL_RESET_DSP3        (0x5C)/*复位dsp3*/
#define IPMB_CMD_BASEBANDCTL_RESET_SRIOSW    (0x5D)/*复位srio sw*/
#define IPMB_CMD_BASEBANDCTL_RESET_GESW        (0x5E)/*复位ge sw*/

#define IPMB_CMD_BASEBANDCTL_TEST                      (0x5F)
#define IPMB_CMD_BASEBANDCTL_TEST_EEPROM       (0x60)
#define IPMB_CMD_BASEBANDCTL_TEST_GESWITCH   (0x61)
#define IPMB_CMD_BASEBANDCTL_REBOOT                 (0x62)
#define IPMB_CMD_BASEBANDCTL_HEART_BEAT_GAP (0x63)

#define IPMB_CMD_BASEBANDCTL_GET_SRIOSW_PORTINFO   (0x70)/*获取srio路由信息*/

#define IPMB_CMD_BASEBANDCTL_POWER_ON          (0x80)/*单板上电*/
#define IPMB_CMD_BASEBANDCTL_POWER_OFF         (0x81)/*单板下电*/

#define IPMB_CMD_BASEBANDCTL_SEND_PACK_OK_CNT  (0x82)/*发送报文成功数统计*/
#define IPMB_CMD_BASEBANDCTL_SEND_PACK_ERR_CNT (0x83)/*发送报文失败数统计*/
#define IPMB_CMD_BASEBANDCTL_RECV_PACK_OK_CNT  (0x84)/*接收报文成功数统计*/
#define IPMB_CMD_BASEBANDCTL_RECV_PACK_ERR_CNT (0x85)/*接收报文失败数统计*/

/*定义交换板命令区*/
#define IPMB_CMD_SWITCHCTL_RESET_GESW              (0x86)/*复位ge sw*/
#define IPMB_CMD_SWITCHCTL_GET_ARM_VER_VAL    (0x87)/*获取arm版本号*/
#define IPMB_CMD_SWITCHCTL_POWER_ON                 (0x88)/*单板上电*/
#define IPMB_CMD_SWITCHCTL_POWER_OFF                (0x89)/*单板下电*/
#define IPMB_CMD_SWITCHCTL_GET_TEMP                   (0x90)/*获取交换板温度*/

#define IPMB_CMD_SWITCHCTL_TEST                       (0x91)
#define IPMB_CMD_SWITCHCTL_TEST_EEPROM        (0x92)
#define IPMB_CMD_SWITCHCTL_TEST_GESWITCH    (0x93)
#define IPMB_CMD_SWITCHCTL_REBOOT                  (0x94)
#define IPMB_CMD_SWITCHCTL_WORKMODE            (0x95)
#define IPMB_CMD_SWITCHCTL_HEART_BEAT_GAP      (0x96)

#define IPMB_CMD_SWITCHCTL_SEND_PACK_OK_CNT    (0xB5)/*发送报文成功数统计*/
#define IPMB_CMD_SWITCHCTL_SEND_PACK_ERR_CNT   (0xB6)/*发送报文失败数统计*/
#define IPMB_CMD_SWITCHCTL_RECV_PACK_OK_CNT    (0xB7)/*接收报文成功数统计*/
#define IPMB_CMD_SWITCHCTL_RECV_PACK_ERR_CNT   (0xB8)/*接收报文失败数统计*/

/*定义风扇板命令区*/
#define IPMB_CMD_FANCTL_SET_SPEED              (0xB9)/*设置风扇转速*/
#define IPMB_CMD_FANCTL_GET_SPEED              (0xBA)/*获取风扇转速*/
#define IPMB_CMD_FANCTL_SEND_EXCEPTION         (0xBB)/*获取异常信息*/
#define IPMB_CMD_FANCTL_GET_ARM_VER_VAL        (0xBC)/*获取arm版本号*/
#define IPMB_CMD_FANCTL_POWER_ON               (0xBD)/*单板上电*/
#define IPMB_CMD_FANCTL_POWER_OFF              (0xBE)/*单板下电*/
#define IPMB_CMD_FANCTL_SEND_PACK_OK_CNT       (0xBF)/*发送报文成功数统计*/
#define IPMB_CMD_FANCTL_SEND_PACK_ERR_CNT      (0xC0)/*发送报文失败数统计*/
#define IPMB_CMD_FANCTL_RECV_PACK_OK_CNT       (0xC1)/*接收报文成功数统计*/
#define IPMB_CMD_FANCTL_RECV_PACK_ERR_CNT      (0xC2)/*接收报文失败数统计*/

#define IPMB_CMD_FANCTL_TEST                   (0xC3)
#define IPMB_CMD_FANCTL_REBOOT                 (0xC4)
#define IPMB_CMD_FANCTL_HEART_BEAT_GAP         (0xC5)
#define IPMB_CMD_FANCTL_GET_BOARD_TYPE	       (0xC6)// 获取板类型
#define IPMB_CMD_FANCTL_GET_PCB_VERSION        (0xC7)// 获取PCB版本号
#define IPMB_CMD_FANCTL_GET_SLOT	           (0xC8)// 获取槽位号

#define IPMB_CMD_EEPROM_SET_CRC                  (0xC9)// 设置CRC
#define IPMB_CMD_EEPROM_GET_CRC                  (0xCA)// 获取CRC
#define IPMB_CMD_EEPROM_SET_DEVICE_ID            (0xCB)// 设置设备ID
#define IPMB_CMD_EEPROM_GET_DEVICE_ID            (0xCC)// 获取设备ID
#define IPMB_CMD_EEPROM_SET_BOARD_TYPE           (0xCD)// 设置板类型
#define IPMB_CMD_EEPROM_GET_BOARD_TYPE           (0xCE)// 获取板类型
#define IPMB_CMD_EEPROM_SET_PRODUCT_SN           (0xD1)// 设置生产序列号
#define IPMB_CMD_EEPROM_GET_PRODUCT_SN           (0xD2)// 获取生产序列号
#define IPMB_CMD_EEPROM_SET_MANUFACTURER         (0xD3)// 设置制造商
#define IPMB_CMD_EEPROM_GET_MANUFACTURER         (0xD4)// 获取制造商
#define IPMB_CMD_EEPROM_SET_PRODUCT_DATE         (0xD5)// 设置生产日期
#define IPMB_CMD_EEPROM_GET_PRODUCT_DATE         (0xD6)// 获取生产日期
#define IPMB_CMD_EEPROM_SET_TEMP_THRESHOLD       (0xD7)// 设置温度门限值
#define IPMB_CMD_EEPROM_GET_TEMP_THRESHOLD       (0xD8)// 获取温度门限值
#define IPMB_CMD_EEPROM_SET_FAN_INITIALSPEED     (0xD9)// 设置风扇出厂转速
#define IPMB_CMD_EEPROM_GET_FAN_INITIALSPEED     (0xDA)// 获取风扇出厂转速
#define IPMB_CMD_FANCTL_TEST_EEPROM              (0xDB)// EEPROM测试

/* 风扇&监控板在线升级命令 */
#define IPMB_CMD_ID_BOOTREQ	    0xFF
#define IPMB_CMD_ID_DATA		0xFE
#define IPMB_CMD_ID_UPDATE		0xFD

#define IPMB_RECEIVE_TASK_LENGTH       (512)      
#define WAIT_FOREVER              0
#define E_IPMB_IOCTL_SEND_OK  0x00
#define E_IPMB_IOCTL_SEND_ERR  0x01
#define E_SPI_ERROR           0x20000000
#define E_ARG_ERROE           0x01
#define E_OS_SEM_CREATE       0x01
#define E_OS_MSGQ_CREATE      0x02
#define E_OS_TASK_CREATE      0x03
#define E_OS_SEM_PEND         0x04
#define E_OS_SEM_POST         0x05
#define E_OS_TIME_OUT         0x06
#define E_IPMB_IOCTL_ARG_NULL 0x07
#define E_IPMB_SEND_FULL      0x08
#define E_IPMB_SEND_FAIL      0x09
#define E_IPMB_IOCTL_CMD_NULL 0x0A
#define E_IPMB_SEND_TIMEOUT    0x0B
#define E_IPMB_RECEIVE_TIMEOUT 0x0C

#define OSP_OK                0
#define OSP_ERR               1

/* 请求消息 */
#define RSADDR_BYTE         (0x00)
#define NETFN_BYTE          (0x01)
#define CHECK_BYTE          (0x02)
#define RQADDR_BYTE         (0x03)
#define SEQNO_BYTE          (0x04)
#define CMD_BYTE            (0x05)
#define COMPLETE_BYTE       (0x06)
/* 响应消息 */
#define CCODE_BYTE          (6)
#define DATASTART_BYTE      (7)
#define OS_NO_ERR                      0
#define IPMB_CHANNEL_IIC_0             0
#define IPMB_CHANNEL_IIC_1             1
#define IPMB_CHANNEL_IIC_2             2
#define IPMB_CHANNEL_IIC_3             3

/* 状态定义 */
#define STATUS_WAIT_RESPOND    0x01
#define STATUS_RECEIVE_RESPOND 0x02
#define IPMB_RECEIVE_BUF_NUM         (32)  /* 接收消息数组的最大长度                   */
#define IPMB_SEND_BUF_NUM            (3)  /* 发送消息数组的最大长度                   */
#define IPMB_QUEUE_BUF_NUM           (100)  /* 接收消息数组的最大长度                   */
#define NETFN_IPMB_REQUEST           (0x30 << 2)
#define NETFN_IPMB_RESPONSE          (NETFN_IPMB_REQUEST + 4)
#define WRITE_FPGA_BUF_NUM                 (20)
#define READ_FPGA_BUF_NUM                 (200)

#define WRITE_EPLD_BUF_NUM                 (20)
#define READ_EPLD_BUF_NUM                 (200)

#define IPMB_PRI_QUEUE             3
#define IPMB_PRI_FIRST             0
#define IPMB_PRI_SECOND            1
#define IPMB_PRI_THIRD             2
#define IPMB_MESSAGE_MAX_NUM       200

#define IPMB_MESSAGE_MAX_LENGTH    1<<10   /* IPMB消息的最大长度(字节)                 */
#define IPMB_MESSAGE_DATA_LENGTH   300   /* IPMB应答消息中的数据部分的最大长度(字节) */

#define HMI_SND_PACK_OK        (0)
#define HMI_SND_PACK_ERR       (1)
#define HMI_RCV_PACK_OK        (2)
#define HMI_RCV_PACK_ERR       (3)

#define IPMB_SEND_MESSAGE_PROCESS  (0)
#define IPMB_SEND_MESSAGE_REQUEST  (1)

/* 接收消息数组定义 */
typedef struct tag_STRU_RESERVE_MESSAGE
{
    UINT8            u8Used;                                 /* 是否使用             */
    UINT32           u32Length;                              /* 长度                 */
    UINT8            u8Message[IPMB_MESSAGE_MAX_LENGTH];     /* 保存接收到的消息     */
} STRU_RECV_MESSAGE;


/* 发送消息数组定义 */
typedef struct tag_STRU_SEND_MESSAGE
{
    UINT8            u8Used;                                /* 是否使用                     */
    UINT8            u8Seq;		                            /* 请求消息的序列号             */
    UINT8            u8Status;                              /* 发送请求消息的任务的状态     */
    UINT32           u32Len;
    UINT8            u8Data[IPMB_MESSAGE_DATA_LENGTH];      /* 保存应答消息中的数据部分     */
	//sem_t            semID;                                 /* 接收到应答消息后激活的信号量 */
} STRU_SEND_MESSAGE;


/*发送消息队列结构体*/
typedef struct tag_STRU_QUEUE_SEND
{    
    STRU_SEND_MESSAGE   StruReplyMessage[IPMB_PRI_QUEUE][IPMB_MESSAGE_MAX_NUM];
    STRU_SEND_MESSAGE   StruRequestMessage[IPMB_PRI_QUEUE][IPMB_MESSAGE_MAX_NUM];
}STRU_QUEUE_SEND;


typedef struct Tag_HMI_REG_INFO
{
	UINT8     *pName;
    WORD32    dwCmd;
	WORD32    index;
	UINT8     isFlag;
}T_HmiRegInfo;



static T_HmiRegInfo gaUserHmiConfigTable[] =
{
	{"resetboard",IPMB_CMD_COMM_RESET_BOARDS,0, TRUE},
	{"stopwatchdog",IPMB_CMD_COMM_STOP_WATCHDOG,0, TRUE},
	{"alive",IPMB_CMD_COMM_ALIVE_CMD,0, TRUE},
	{"maintemp",IPMB_CMD_MAINBANDCTL_TEMP,0, TRUE},
	{"writefpga",IPMB_CMD_BASEBANDCTL_WRITE_FPGA,0, TRUE},
	{"readfpga",IPMB_CMD_BASEBANDCTL_READ_FPGA,0, TRUE},
	{"writeepld",IPMB_CMD_BASEBANDCTL_WRITE_EPLD,0, TRUE},
	{"readepld",IPMB_CMD_BASEBANDCTL_READ_EPLD,0, TRUE},
};

#define HMI_STATIC_MAX_CONFIG      (sizeof(gaUserHmiConfigTable)/sizeof(T_HmiRegInfo))
#define MAX_HMI_DEVICE             (3)
#define HMI_DEVICE_BBP             (0)
#define HMI_DEVICE_GES             (1)
#define HMI_DEVICE_FAN             (2)


typedef struct Tag_STRU_HMI_PACK_STATIC
{
    UINT32    dwSndAllPacketCnt;/*发送hmi数据的总包数*/
	UINT32    dwSndAllPacketOk; /*发送hmi数据的成功计数*/
	UINT32    dwSndAllPacketErr;/*发送hmi数据的失败计数*/
    UINT32    dwRcvAllPacketCnt;/*接收hmi数据的总包数*/
	UINT32    dwRcvAllPacketOk; /*接收hmi数据的成功计数*/
    UINT32    dwRcvAllPacketErr;/*接收hmi数据的失败计数*/
    UINT32    dwAliveCnt[MAX_HMI_DEVICE];	
    UINT32    dwSndPacketOk[HMI_STATIC_MAX_CONFIG];
	UINT32    dwSndPacketErr[HMI_STATIC_MAX_CONFIG];
	UINT32    dwSndPacketTimeout[HMI_STATIC_MAX_CONFIG];

    UINT32    dwRcvPacketOk[HMI_STATIC_MAX_CONFIG];
    UINT32    dwRcvPacketErr[HMI_STATIC_MAX_CONFIG];
	UINT32    dwRcvPacketTimeout[HMI_STATIC_MAX_CONFIG];
	
}HMI_PACK_STATIC;

typedef struct Tag_STRU_HMI_WRITE_FPGA_REG
{
    UINT8  u8Used; 
    UINT32 dwReg;
	UINT32 dwVal;
}HMI_WRITE_FPGA_REG;


typedef struct Tag_STRU_HMI_READ_FPGA_REG
{
    UINT8  u8Used; 
    UINT32 dwReg;
	UINT32 dwVal;
}HMI_READ_FPGA_REG;

typedef struct Tag_STRU_HMI_WRITE_EPLD_REG
{
    UINT8  u8Used; 
    UINT32 dwReg;
	UINT32 dwVal;
}HMI_WRITE_EPLD_REG;
typedef struct Tag_STRU_HMI_READ_EPLD_REG
{
    UINT8  u8Used; 
    UINT32 dwReg;
	UINT32 dwVal;
}HMI_READ_EPLD_REG;


/*
   sRIO Switch SerDes端口互联分配表
   Quad	            端口编号	          对端设备	对端设备SerDes编号	备注
   Port0~1	        DSP2	                    RIO0~1	             2X    5g
   Port2~3	        DSP1	                    RIO0~1	             2X    5g
   Port4~5	        DSP3	                    RIO0~1	             2X    5g
   Port6	               DSP4	                    RIO0	                    1X    5g
   Port7	               FPGA	                    Bank117 Port2	      1X    5g
   Port8~9	        BP                             Connector	-	      2X    6.25g
   Port10~11	        BP                             Connector	-	      2X    6.25g
   Port12~15	        BP                             Connector	-	      4X    6.25g
*/
/******************************** 头文件保护结尾 ******************************/
#endif /* __BSP_HMI_H__ */
/******************************** 头文件结束 **********************************/

