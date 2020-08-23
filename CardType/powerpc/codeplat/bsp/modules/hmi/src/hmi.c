/*******************************************************************************
* *  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 文件名称:  hmi.c
* 功    能:
* 版    本:  V0.1
* 编写日期:  2015/06/26
* 说    明:  无
* 修改历史:
* 修改日期           修改人 liuganga     修改内容
*------------------------------------------------------------------------------
/******************************* 包含文件声明 *********************************/
/**************************** 共用头文件************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <pub.h>
/**************************** 私用头文件************************************/
#include "../inc/hmi.h"
#include "../../../com_inc/bsp_types.h"
#include "../../../com_inc/bsp_i2c_ext.h"
#include "../../../com_inc/bsp_conkers_ext.h"
#include "../../bbp_mcta/bsp_bbp_command.h"
#include "../../../modules/usdpaa/inc/compat.h"
#include "../../ms/inc/bsp_ms.h"


pthread_mutex_t g_hmi1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_hmi2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_semProtectMsg = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_semFpgaWrite = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_semEpldWrite = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ipmb_lock = PTHREAD_MUTEX_INITIALIZER;

/* 接收消息数组定义 */
STRU_RECV_MESSAGE g_struReceiveMsgBuf[IPMB_RECEIVE_BUF_NUM] = {0};
/*发送请求队列数组定义*/
STRU_QUEUE_SEND       g_struQueueMsgBuf;

HMI_WRITE_FPGA_REG   g_struWriteFpgaQueueBuf[WRITE_FPGA_BUF_NUM]= {0};
HMI_READ_FPGA_REG    g_struReadFpgaQueueBuf[READ_FPGA_BUF_NUM]= {0};

HMI_WRITE_EPLD_REG   g_struWriteEpldQueueBuf[WRITE_EPLD_BUF_NUM]= {0};
HMI_READ_EPLD_REG    g_struReadEpldQueueBuf[READ_EPLD_BUF_NUM]= {0};
extern u8 *g_u8ccsbar;
/******************************* 局部宏定义 ***********************************/
#define CONFIG_SYS_IIC_OFFSET     (0x0c)
#define CONFIG_SYS_I2C1_OFFSET	   (0x118000)
#define CONFIG_SYS_I2C2_OFFSET		(0x118100)
#define CONFIG_SYS_I2C3_OFFSET    (0x119000)
#define CONFIG_SYS_I2C4_OFFSET    (0x119100)

HMI_PACK_STATIC    g_tHmiStatic;

UINT32 g_hmi_config_table = 0;
UINT32 g_fan1_speed = 0;
UINT32 g_fan2_speed = 0;
UINT32 g_fan3_speed = 0;
static int g_fan_set_state = BSP_ERROR;
sem_t g_ipmb_sema;
sem_t g_ipmb_writefpga;
sem_t g_ipmb_writeepld;
sem_t g_ipmb_semsend;
/* HMI接收变量 */
UINT8 g_u8ReceiveMsgIndex   = 0;
/* HMI调试打印开关 */
UINT32 g_hmi_heartbeat_debug = 0;
UINT32 g_hmi_response_debug = 0;
UINT32 g_u32hmi_debug = 0;
UINT32 fan_control_debug = 0;
/* HMI风扇板测试相关变量 */
uint8_t g_fan_speed_set = 0;
uint8_t g_fan_speed_get = 0;
uint8_t g_fan_eeprom_test = 0;
uint8_t g_fan_set_eeprom = 0;
uint8_t g_fan_get_eeprom = 0;
/* HMI监控板测试相关变量 */
uint8_t g_peu_power_down_flag =0;
uint8_t g_peu_get_temp = 0;
uint8_t g_peu_get_dryin = 0;
uint8_t g_peu_rs485_test = 0;
uint8_t g_peu_dryin0123 = 0;
uint8_t g_peu_dryin4567 = 0;
/* HMI链路测试相关变量 */
uint8_t g_peu_hmi_test = 0;
uint8_t g_fan_hmi_test = 0;
uint8_t g_ges_hmi_test = 0;
uint8_t g_bbp_hmi_test = 0;

#define MAX_UPDATE_TIMES 3

/* 风扇板EEPROM参数设置 */
EEPROM_PAR_STRU g_fan_eeprom_set_par =
{
    {0},//CRC
    "C6482",//DEVICE ID
    "C6482_FM.00.00",//板卡类型
    {0},//MCA地址1
    {0},//MCA地址2
    "11-22-33-44-55",//PRODUCTSN
    "BJXINWEI",//MANUFACTURE
    {0x05,0x10,0x07,0xe0},//PRODUCTDATE
    {0},//接收机型号
    {0x0f,0xa0},//风扇出厂转速
    {85,-10}//温度范围
};
EEPROM_PAR_STRU g_fan_eeprom_get_par = {0};

/* IPMB文件下载变量 */
UINT32 ipmb_filesize = 0;
int ipmb_filefd = 0;
struct stat ipmb_st;
pthread_mutex_t ipmbfile_wait;
UINT16 ipmb_pkg_id = 0;
UINT16 ipmb_pkg_len = 0;
/*********************liugang modify*****************************/
/*********************** 全局变量定义/初始化 *************************/
SINT8 g_s8BbpGetTemperature[8] = {0};
SINT8 g_s8FsaGetTemperature[4] = {0};
SINT16 g_s16EsTemp = 0;
SINT16 g_s16PEUTemp = 0;

/***************************** 函数声明 ******************************/
UINT8 bsp_ipmb_init(void);
UINT8 bsp_ipmb_message_check(UINT8 *pu8Buf, UINT32 u32Length);
UINT8 bsp_ipmb_ioctl(UINT32 u8IpmbCmd, UINT32 u32Arg1, UINT32 u32Arg2, UINT32 u32Arg3);
UINT8 bsp_ipmb_process_request(UINT8 *pu8Buf, UINT8 u8Length);
SINT8 bsp_ipmb_send_message(UINT8 u8Channel,UINT8 u8I2cAddress, UINT8 *pu8Buf, UINT32 u32Length);
UINT8 bsp_ipmb_get_queue_index(UINT8 u8MessType,UINT8 u8Pri,UINT8 *pu8Index);
void  bsp_ipmb_create_request_header(UINT8 u8RemoteI2cAddr,UINT8 u8LocalI2cAddr,UINT8 *pu8IpmbMsg, UINT8 u8NetFn, UINT8 u8IpmbCmd);
void  bsp_ipmb_create_response_header(UINT8 *pu8RspBuf, UINT8 *pu8RecvBuf,UINT8 u8RemoteAddr);
void  bsp_ipmb_check_sum(UINT8 *pu8Buf, UINT32 u32Length);
void  bsp_ipmb_process_response(UINT8 *pu8Buf, UINT8 u8Length);
WORD32  bsp_send_hmi_data(UINT8 u8MessageType,  UINT8 u8Cmd,UINT8 u8Pri,UINT8  u8LocalI2cAddr, UINT8 u8RemoteI2cAddr, UINT8  *pu8SrcData, UINT32  dwTransLen);
UINT8 bsp_ipmb_fan_control_init(void);
UINT8 fan_control_algorithm_2(f32 temp);
void bsp_ipmb_fan_control_task(void);
SINT8 bsp_hmi_fan_speed(UINT8 u8dwSel,UINT8 u8FanChannel,UINT8 u8FanPWMVal);
s32 init_ipmb_file_download(u8 u8DstAddr);
s32 ipmb_get_file_size(char *filename);
SINT8 bsp_hmi_mcu_update(UINT8 u8boardtype,UINT8 u8slot);
/**************************************************************************************************
*  函数名称: bsp_ipmb_get_queue_index
*  描    述: 获取未被使用的发送数组索引值
*  参    数:(1)u8MessType: 消息类型
*           (2)u8Pri: 消息优先级
*           (3)pu8Index: index number
*  返    回: ERR/OK.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_get_queue_index(UINT8 u8MessType,UINT8 u8Pri,UINT8 *pu8Index)
{
    UINT8 ret;
    UINT8 u8Err;
    UINT8 u8Index;
    /* 查找可用数组元素 */
    pthread_mutex_lock(&g_queue_mutex);
#if 1
    if (u8Pri > IPMB_PRI_QUEUE)
        return FALSE;
    if (u8MessType == IPMB_SEND_MESSAGE_PROCESS)
    {
        for(u8Index = 0; u8Index < IPMB_MESSAGE_MAX_NUM; u8Index++)
        {
            if (!g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Used)
                break;
        }
        /* 检查 */
        if(u8Index < IPMB_MESSAGE_MAX_NUM)
        {
            g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Used = 1;
            *pu8Index = u8Index;
            ret = TRUE;
        }
        else
        {
            for(u8Index = 0; u8Index < IPMB_MESSAGE_MAX_NUM; u8Index++)
                g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Used = 0;
            ret = E_IPMB_SEND_FULL;
        }
    }
    else if(u8MessType == IPMB_SEND_MESSAGE_REQUEST)
    {
        for(u8Index = 0; u8Index < IPMB_MESSAGE_MAX_NUM; u8Index++)
        {
            if (!g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Used)
                break;
        }
        /* 检查 */
        if(u8Index < IPMB_MESSAGE_MAX_NUM)
        {
            g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Used = 1;
            *pu8Index = u8Index;
            ret = TRUE;
        }
        else
        {
            for(u8Index = 0; u8Index < IPMB_MESSAGE_MAX_NUM; u8Index++)
                g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Used = 0;
            ret = E_IPMB_SEND_FULL;
        }
    }
    else
    {
        return FALSE;
    }
#endif
    pthread_mutex_unlock(&g_queue_mutex);
    return ret;
}
/**************************************************************************************************
*  函数名称: bsp_get_i2caddr_byboardtype
*  描    述: 获取remote i2c addr
*  参    数: (1)type: board typr
* 		   (2)slot : 槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
u8 bsp_get_i2caddr_byboardtype(u32 type, u32 slot)
{
    u8 addr;
    switch(type)
    {
    case BOARD_TYPE_BBP:
        addr = IPMB_BBP_HMI2_I2C_ADDR(slot);
        break;
    case BOARD_TYPE_FSA:
    case BOARD_TYPE_ES:
        addr = IPMB_FSA_HMI2_I2C_ADDR(slot);
        break;
    case BOARD_TYPE_FAN:
        addr = IPMB_FAN_HMI2_I2C_ADDR;
        break;
    case BOARD_TYPE_PEU:
        addr = IPMB_PEU_HMI2_I2C_ADDR(slot);
        break;
    case BOARD_TYPE_ALL:
    	addr = IPMB_BROADCAST_ADDR;
    	break;
    default:
        addr = IPMB_MCT_HMI2_I2C_ADDR(slot);
        break;
    }
    return addr;
}
/**************************************************************************************************
*  函数名称: bsp_get_slot_byi2caddr
*  描    述: 获取slot
*  参    数: (1)addr: i2caddr
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
u8 bsp_get_slot_byi2caddr(u8 i2caddr)
{
    u8 slot = 0;

    slot = i2caddr - IPMB_MCT_HMI1_I2C_ADDR_BASE;
    return slot;
}
/**************************************************************************************************
*  函数名称: bsp_read_eeprom_buf
*  描    述: 获取eeprom设置参数
*  参    数: (1)type: request command
* 		   (2)buf : 参数
*            (3)len : 长度
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_read_eepromset_buf(u32 type, u8 *buf, u8 *len)
{
    switch(type)
    {
    case IPMB_CMD_EEPROM_SET_CRC:
        memcpy(buf,(u8 *)&g_fan_eeprom_set_par.checkSum,2);
        *len = 2;
        break;
    case IPMB_CMD_EEPROM_SET_DEVICE_ID:
        memcpy(buf,g_fan_eeprom_set_par.device_id,16);
        *len = 16;
        break;
    case IPMB_CMD_EEPROM_SET_BOARD_TYPE:
        memcpy(buf,g_fan_eeprom_set_par.board_type,32);
        *len = 32;
        break;
    case IPMB_CMD_EEPROM_SET_PRODUCT_SN:
        memcpy(buf,g_fan_eeprom_set_par.product_sn,32);
        *len = 32;
        break;
    case IPMB_CMD_EEPROM_SET_MANUFACTURER:
        memcpy(buf,g_fan_eeprom_set_par.manufacturer,12);
        *len = 12;
        break;
    case IPMB_CMD_EEPROM_SET_PRODUCT_DATE:
        memcpy(buf,g_fan_eeprom_set_par.product_date,4);
        *len = 4;
        break;
    case IPMB_CMD_EEPROM_SET_TEMP_THRESHOLD:
        memcpy(buf,g_fan_eeprom_set_par.temperature_threshold,2);
        *len = 2;
        break;
    case IPMB_CMD_EEPROM_SET_FAN_INITIALSPEED:
        memcpy(buf, g_fan_eeprom_set_par.fan_initialspeed,6);
        *len = 6;
        break;
    default:
        *len = 0;
        break;
    }
}

/**************************************************************************************************
*  函数名称: bsp_write_eepromget_buf
*  描    述: 保存eeprom获取参数
*  参    数: (1)type: response command
*            (2)buf : 参数
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_write_eepromget_buf(u32 type, u8 *buf)
{
    switch(type)
    {
    case IPMB_CMD_EEPROM_GET_CRC:
        memcpy((u8 *)&g_fan_eeprom_get_par.checkSum,buf,2);
        break;
    case IPMB_CMD_EEPROM_GET_DEVICE_ID:
        memcpy(g_fan_eeprom_get_par.device_id,buf,16);
        break;
    case IPMB_CMD_EEPROM_GET_BOARD_TYPE:
        memcpy(g_fan_eeprom_get_par.board_type,buf,32);
        break;
    case IPMB_CMD_EEPROM_GET_PRODUCT_SN:
        memcpy(g_fan_eeprom_get_par.product_sn,buf,32);
        break;
    case IPMB_CMD_EEPROM_GET_MANUFACTURER:
        memcpy(g_fan_eeprom_get_par.manufacturer,buf,12);
        break;
    case IPMB_CMD_EEPROM_GET_PRODUCT_DATE:
        memcpy(g_fan_eeprom_get_par.product_date,buf,4);
        break;
    case IPMB_CMD_EEPROM_GET_TEMP_THRESHOLD:
        memcpy(g_fan_eeprom_get_par.temperature_threshold,buf,2);
        break;
    case IPMB_CMD_EEPROM_GET_FAN_INITIALSPEED:
        memcpy(g_fan_eeprom_get_par.fan_initialspeed,buf,6);
        break;
    default:
        break;
    }
}

/**************************************************************************************************
*  函数名称: bsp_ipmb_ioctl
*  描    述: 发送请求消息
*  参    数: (1)u32Cmd: request command
* 		   (2)u32Arg1 : board type
*            (3)u32Arg2 : 参数
*            (4)u32Arg3 : 槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_ioctl(UINT32 u8IpmbCmd, UINT32 u32Arg1, UINT32 u32Arg2,UINT32 u32Arg3)
{
    /* 局部变量定义 */
    UINT8 u8MsgBuf[IPMB_MESSAGE_MAX_LENGTH];    /* 保存IPMB请求消息的数组 */
    UINT8 u8MsgLength = 0;                      /* IPMB请求消息的实际长度 */

    u8 u8SlotId = bsp_get_slot_id();
    u8 u8SrcAddr;
    u8 u8DstAddr;

    u8SrcAddr = IPMB_MCT_HMI2_I2C_ADDR(u8SlotId);

    switch(u8IpmbCmd)
    {
    case IPMB_CMD_BASEBANDCTL_GET_ARM_VER_VAL:
    case IPMB_CMD_BASEBANDCTL_GET_TEMP:
    case IPMB_CMD_BASEBANDCTL_POWER_ON:
    case IPMB_CMD_BASEBANDCTL_POWER_OFF:
    case IPMB_CMD_BASEBANDCTL_SEND_PACK_OK_CNT:
    case IPMB_CMD_BASEBANDCTL_SEND_PACK_ERR_CNT:
    case IPMB_CMD_BASEBANDCTL_RECV_PACK_OK_CNT:
    case IPMB_CMD_BASEBANDCTL_RECV_PACK_ERR_CNT:
    case IPMB_CMD_BASEBANDCTL_TEST:
    case IPMB_CMD_BASEBANDCTL_TEST_EEPROM:
    case IPMB_CMD_BASEBANDCTL_TEST_GESWITCH:
    case IPMB_CMD_BASEBANDCTL_REBOOT:
    case IPMB_CMD_FSA_GET_ARM_VER_VAL:
    case IPMB_CMD_FSA_GET_TEMP:
    case IPMB_CMD_FSA_POWER_ON:
    case IPMB_CMD_FSA_POWER_OFF:
    case IPMB_CMD_FSA_REBOOT:
    case IPMB_CMD_FSA_TEST:
    case IPMB_CMD_FSA_TEST_EEPROM:
    case IPMB_CMD_FSA_TEST_GESWITCH:
    case IPMB_CMD_ES_GET_ARM_VER_VAL:
    case IPMB_CMD_ES_GET_TEMP:
    case IPMB_CMD_ES_REBOOT:
    case IPMB_CMD_ES_TEST:
    case IPMB_CMD_ES_TEST_EEPROM:
    case IPMB_CMD_ES_TEST_GESWITCH:
    case IPMB_CMD_FANCTL_GET_ARM_VER_VAL:
    case IPMB_CMD_FANCTL_SEND_PACK_OK_CNT:
    case IPMB_CMD_FANCTL_SEND_PACK_ERR_CNT:
    case IPMB_CMD_FANCTL_RECV_PACK_OK_CNT:
    case IPMB_CMD_FANCTL_RECV_PACK_ERR_CNT:
    case IPMB_CMD_FANCTL_TEST:
    case IPMB_CMD_FANCTL_TEST_EEPROM:
    case IPMB_CMD_FANCTL_REBOOT:		
    case IPMB_CMD_FANCTL_GET_SLOT:
    case IPMB_CMD_FANCTL_GET_BOARD_TYPE:
    case IPMB_CMD_FANCTL_GET_PCB_VERSION:
    case IPMB_CMD_EEPROM_GET_CRC:
    case IPMB_CMD_EEPROM_GET_DEVICE_ID:
    case IPMB_CMD_EEPROM_GET_BOARD_TYPE:
    case IPMB_CMD_EEPROM_GET_PRODUCT_SN:
    case IPMB_CMD_EEPROM_GET_MANUFACTURER:
    case IPMB_CMD_EEPROM_GET_PRODUCT_DATE:
    case IPMB_CMD_EEPROM_GET_TEMP_THRESHOLD:
    case IPMB_CMD_EEPROM_GET_FAN_INITIALSPEED:
    case IPMB_CMD_PEUACTL_GET_TEMP:
    case IPMB_CMD_PEUACTL_GET_ARM_VER_VAL:
    case IPMB_CMD_PEUACTL_SEND_PACK_OK_CNT:
    case IPMB_CMD_PEUACTL_SEND_PACK_ERR_CNT:
    case IPMB_CMD_PEUACTL_RECV_PACK_OK_CNT:
    case IPMB_CMD_PEUACTL_RECV_PACK_ERR_CNT:
    case IPMB_CMD_PEUACTL_TEST:
    case IPMB_CMD_PEUACTL_REBOOT:
    case IPMB_CMD_PEUACTL_GET_SLOT:
    case IPMB_CMD_PEUACTL_GET_BOARD_TYPE:
    case IPMB_CMD_PEUACTL_GET_PCB_VERSION:
    case IPMB_CMD_PEUACTL_GET_DRYIN_STATE:
    case IPMB_CMD_PEUACTL_POWER_ON:
    case IPMB_CMD_PEUACTL_POWER_OFF:
    case IPMB_CMD_PEUACTL_RS485_TEST:
    case IPMB_CMD_ID_UPDATE:
    case IPMB_CMD_COMM_RESET_BOARDS:
    case IPMB_CMD_COMM_CHECK_ALIVE:
    case IPMB_CMD_COMM_START_SEND_MESSAGE:
    case IPMB_CMD_COMM_STOP_SEND_MESSAGE:
        u8DstAddr = bsp_get_i2caddr_byboardtype(u32Arg1, u32Arg3);
        u8MsgLength = 0;
        break;

    case IPMB_CMD_BASEBANDCTL_WRITE_FPGA:
    case IPMB_CMD_BASEBANDCTL_WRITE_EPLD:
        u8MsgBuf[0]  = (u32Arg1 & 0xff000000)>>24;
        u8MsgBuf[1]  = (u32Arg1 & 0x00ff0000)>>16;
        u8MsgBuf[2]  = (u32Arg1 & 0x0000ff00)>>8;
        u8MsgBuf[3]  = (u32Arg1 & 0x000000ff)>>0;
        u8MsgBuf[4]  = (u32Arg2 & 0xff000000)>>24;
        u8MsgBuf[5]  = (u32Arg2 & 0x00ff0000)>>16;
        u8MsgBuf[6]  = (u32Arg2 & 0x0000ff00)>>8;
        u8MsgBuf[7]  = (u32Arg2 & 0x000000ff)>>0;
        u8MsgLength = 8;
        u8DstAddr = IPMB_BBP_HMI2_I2C_ADDR(u32Arg3);
        break;

    case IPMB_CMD_BASEBANDCTL_READ_FPGA:
    case IPMB_CMD_BASEBANDCTL_READ_EPLD:
        u8MsgBuf[0]  = (u32Arg1 & 0xff000000)>>24;
        u8MsgBuf[1]  = (u32Arg1 & 0x00ff0000)>>16;
        u8MsgBuf[2]  = (u32Arg1 & 0x0000ff00)>>8;
        u8MsgBuf[3]  = (u32Arg1 & 0x000000ff)>>0;
        u8MsgLength = 4;
        u8DstAddr = IPMB_BBP_HMI2_I2C_ADDR(u32Arg3);
        break;

    case IPMB_CMD_EEPROM_SET_CRC:
    case IPMB_CMD_EEPROM_SET_DEVICE_ID:
    case IPMB_CMD_EEPROM_SET_BOARD_TYPE:
    case IPMB_CMD_EEPROM_SET_PRODUCT_SN:
    case IPMB_CMD_EEPROM_SET_MANUFACTURER:
    case IPMB_CMD_EEPROM_SET_PRODUCT_DATE:
    case IPMB_CMD_EEPROM_SET_TEMP_THRESHOLD:
    case IPMB_CMD_EEPROM_SET_FAN_INITIALSPEED:
        u8DstAddr = IPMB_FAN_HMI2_I2C_ADDR;
        bsp_read_eepromset_buf(u8IpmbCmd, u8MsgBuf, &u8MsgLength);
        break;

    case IPMB_CMD_FANCTL_SET_SPEED:
        u8DstAddr = IPMB_FAN_HMI2_I2C_ADDR;
        u8MsgBuf[0] = (u8)u32Arg1;
        u8MsgBuf[1] = (u8)u32Arg2;
        u8MsgLength = 2;
        break;

    case IPMB_CMD_FANCTL_GET_SPEED:
        u8DstAddr = IPMB_FAN_HMI2_I2C_ADDR;
        u8MsgBuf[0] = (u8)u32Arg1;
        u8MsgLength = 1;
        break;

    case IPMB_CMD_BASEBANDCTL_HEART_BEAT_GAP:
    case IPMB_CMD_FSA_HEART_BEAT_GAP:
    case IPMB_CMD_ES_HEART_BEAT_GAP:
    case IPMB_CMD_FANCTL_HEART_BEAT_GAP:
    case IPMB_CMD_PEUACTL_HEART_BEAT_GAP:
        u8DstAddr = bsp_get_i2caddr_byboardtype(u32Arg1, u32Arg3);
        u8MsgBuf[0] = (u8)u32Arg2;
        u8MsgLength = 1;
        break;

    default:
        printf("unknown or uncorrect ipmb command, cmd:0x%x.\n", u8IpmbCmd);
        return  E_IPMB_IOCTL_CMD_NULL;
    }

    return bsp_send_hmi_data(
               IPMB_SEND_MESSAGE_REQUEST, \
               u8IpmbCmd, \
               IPMB_PRI_FIRST, \
               u8SrcAddr, \
               u8DstAddr, \
               u8MsgBuf, \
               u8MsgLength \
           );

}

/**************************************************************************************************
*  函数名称: bsp_ipmb_check_sum
*  描    述: 计算IPMB消息报文的校验和
*  参    数: (1)pu8Buf: pointer of message buffer.
*            (2)u8Length: length of message.
*  返    回: 无
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_check_sum(UINT8 *pu8Buf, UINT32 u32Length)
{
    UINT32 u32Index;
    UINT8 u8Sum = 0;
    for(u32Index = 0; u32Index < u32Length; u32Index++)
    {
        u8Sum += pu8Buf[u32Index];
    }
    pu8Buf[u32Length] = ~u8Sum + 1;
}

/**************************************************************************************************
*  函数名称: bsp_ipmb_message_check
*  描    述: 校验IPMB消息报文
*  参    数: (1)pu8Buf: pointer of message buffer.
*            (2)u8Length: length of message.
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_message_check(UINT8 *pu8Buf, UINT32 u32Length)
{
    UINT8 ret;
    UINT32 i;
    UINT16 sum = 0;
    /* 前两个字节的检验 */
    for(i=0; i<3; i++)
    {
        sum+=pu8Buf[i];
    }
    if((sum & 0xff) != 0)
    {
        ret = FALSE;
        return ret;
    }
    /* 整个消息的检验 */
    for(i=0; i<u32Length; i++)
    {
        sum+=pu8Buf[i];
    }
    if((sum & 0xff) != 0)
    {
        ret = FALSE;
        return ret;
    }
    ret = TRUE;
    return ret;
}

/**************************************************************************************************
*  函数名称: bsp_ipmb_create_request_header
*  描    述: 创建请求消息报头
*  参    数: (1)u8RemoteI2cAddr: choose i2c channel.
*            (2)u8LocalI2cAddr: transfer rate of i2c, Tr-WorkFreq = Fpclk/(SCLH + SCLL).
*            (3)pu8IpmbMsg: master mode/slave mode
*            (4)u8NetFn:
*            (5)u8IpmbCmd:
*  返    回: 无.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_create_request_header(UINT8 u8RemoteI2cAddr,UINT8 u8LocalI2cAddr,UINT8 *pu8IpmbMsg, UINT8 u8NetFn, UINT8 u8IpmbCmd)
{
    static UINT32 dwSeqNo   = 1;    /* 请求消息的序列号 */
    pu8IpmbMsg[RSADDR_BYTE] = u8RemoteI2cAddr;
    pu8IpmbMsg[NETFN_BYTE]  =  u8NetFn;
    bsp_ipmb_check_sum(pu8IpmbMsg, 2);
    pu8IpmbMsg[RQADDR_BYTE] = u8LocalI2cAddr;
    pu8IpmbMsg[SEQNO_BYTE]  = (dwSeqNo << 2);
    pu8IpmbMsg[CMD_BYTE]    = u8IpmbCmd;
    dwSeqNo++;
}

/**************************************************************************************************
*  函数名称: bsp_ipmb_create_response_header
*  描    述: 创建应答消息报头
*  参    数: (1)pu8RspBuf: the pointer of response buffer.
*            (2)pu8RecvBuf: the pointer of receive buffer.
*  返    回: 无.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_create_response_header(uint8_t *pu8RspBuf, uint8_t *pu8RecvBuf, uint8_t u8RemoteAddr)
{
    pu8RspBuf[RSADDR_BYTE] = u8RemoteAddr;//pu8RecvBuf[RQADDR_BYTE];
    pu8RspBuf[NETFN_BYTE]  = ((pu8RecvBuf[NETFN_BYTE]&0xFC)|(pu8RecvBuf[SEQNO_BYTE]&0x3))+4;
    bsp_ipmb_check_sum(pu8RspBuf, 2);
    pu8RspBuf[RQADDR_BYTE] = pu8RecvBuf[RSADDR_BYTE];
    pu8RspBuf[SEQNO_BYTE]  = pu8RecvBuf[SEQNO_BYTE];
    pu8RspBuf[CMD_BYTE]    = pu8RecvBuf[CMD_BYTE];
}

/**************************************************************************************************
*  函数名称: bsp_ipmb_send_message
*  描    述: ipmb发送消息报文
*  参    数: (1)u8Channel: choose i2c channel.
*            (2)u8I2cAddress: slave i2c address.
*            (3)pu8Buf: pointer of send buffer.
*            (4)u8Length: length of send data
*  返    回: 无.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
uint32_t test_smsg_num = 0;
SINT8 bsp_ipmb_send_message(UINT8 u8Channel,UINT8 u8I2cAddress, UINT8 *pu8Buf, UINT32 u32Length)
{
    UINT8 ret;
    UINT8 u8Err;
    UINT8 u8Cnt;
    UINT32 i=0;
    UINT8 u8Index;

    /* 调用I2C驱动发送数据 */
    if(u8Channel == IPMB_CHANNEL_IIC_2)
    {
    	  pthread_mutex_lock(&g_hmi1_mutex);
    	  i2c_init(CONFIG_SYS_I2C_SPEED,2);
          if(BSP_OK !=i2c_write(u8I2cAddress, pu8Buf[0], 1 , &pu8Buf[1],u32Length-1,IPMB_CHANNEL_IIC_2))
	      {
	              printf("write hmi data error!\n");
	              bsp_set_i2c_slave(1);
	              pthread_mutex_unlock(&g_hmi1_mutex);
	              return BSP_ERROR;
	      }

	      bsp_set_i2c_slave(1);
	      pthread_mutex_unlock(&g_hmi1_mutex);
    }
    else  if(u8Channel == IPMB_CHANNEL_IIC_3)
    {
        pthread_mutex_lock(&g_hmi2_mutex);
        if(g_u32hmi_debug == 1)
        {
            if((pu8Buf[5] == IPMB_CMD_FANCTL_TEST) ||
                    (pu8Buf[5] == IPMB_CMD_PEUACTL_TEST) ||
                    (pu8Buf[5] == IPMB_CMD_BASEBANDCTL_TEST))
            {
                printf("u8SendData(%d)=", test_smsg_num++);
            }
            else
                printf("u8SendData=");
            for(i=0; i<u32Length; i++)
                printf("%02x ",pu8Buf[i]);
            printf("\r\n");
        }
        i2c_init(CONFIG_SYS_I2C_SPEED,3);
        if(BSP_OK != i2c_write(u8I2cAddress, pu8Buf[0], 1 , &pu8Buf[1],u32Length-1,IPMB_CHANNEL_IIC_3))
        {
            if(BSP_OK != i2c_write(u8I2cAddress, pu8Buf[0], 1 , &pu8Buf[1],u32Length-1,IPMB_CHANNEL_IIC_3))
            {
                if(g_u32hmi_debug == 1)
                    printf("write hmi data error!\n");
                bsp_set_i2c_slave(2);
                pthread_mutex_unlock(&g_hmi2_mutex);
                return BSP_ERROR;
            }
        }
        else
        {
            if(g_u32hmi_debug == 1)
            {
                printf("send ipmb data ok...\r\n");
            }
        }
        bsp_set_i2c_slave(2);
        pthread_mutex_unlock(&g_hmi2_mutex);
    }
    else
    {
        return BSP_ERROR;
    }
    return BSP_OK;
}

/**************************************************************************************************
*  函数名称: bsp_ipmb_process_request
*  描    述: 处理请求消息报文
*  参    数: (1)pu8Buf: pointer of send buffer.
*            (2)u8Length: length of send buffer.
*  返    回: 无.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
uint32_t ipmb_tftp_dbg_flag = 0;
UINT8 bsp_ipmb_process_request(UINT8 *pu8Buf, UINT8 u8Length)
{
    /*局部变量*/
    UINT8 u8IpmbCmd;                           /* 命令号 */
    UINT8 u8SeqNum;
    UINT8 u8RspBuf[IPMB_MESSAGE_MAX_LENGTH];   /* 应答消息数组 */
    UINT8 u8RspLength = 0;                     /* 应答消息长度 */
    UINT8 u8ret;
    UINT8 u8FloatBuf[4] = {0};
    UINT8 mcu_update_flag = 0;
    UINT8 mcu_need_update_ver = 0;
    UINT8 u8SrcAddr,u8BoardType,u8DstAddr,u8Slot;
    char filename[20] = "";
    UINT16 data_len = 0;
    UINT8 fan_set_flag = 0;

    if(BSP_ERROR == bsp_subboard_is_reseted())
    	return FALSE;

    /* 取出命令号 */
    u8IpmbCmd = pu8Buf[CMD_BYTE];
    /* 取出序列号 */
    u8SeqNum = pu8Buf[SEQNO_BYTE];
    u8SrcAddr = pu8Buf[RQADDR_BYTE];
    /* 取出槽位号 */
    u8Slot = pu8Buf[6];
    /* 取出板类型 */
    u8BoardType = pu8Buf[7];
    /* 取出header */
    memcpy(u8RspBuf, pu8Buf, 6);

    /* 处理first message及heartbeat消息 */
    if((u8Slot < 2) || (u8Slot > 10))
    {
        printf("[%s] erro slot:%d\r\n", __func__, u8Slot);
		return 0;
    }

    /* first msg消息处理 */
    if(u8IpmbCmd == IPMB_CMD_COMM_FIRST_MESSAGE)
    {
        if(bsp_mcu_need_update(u8BoardType, pu8Buf+8)==BSP_OK)
        {
            //bsp_hmi_mcu_update(u8BoardType, u8Slot);
            mcu_need_update_ver = 1;
        }
        else
        {
            bsp_recv_firstmsg(u8Slot, u8BoardType);
            if(u8BoardType == BOARD_TYPE_FAN)
            {
                fan_set_flag = 1;
            }
            //获取版本号
            memcpy(boards[u8Slot].arm_version, (s8 *)&pu8Buf[8], 64);
        }
        u8RspBuf[6] = 0x01;
        u8RspLength = 1;
        printf("[%s]:recv slot[%d] first message.\r\n", __func__, u8Slot);
    }
    /* heartbeat消息处理 */
    else if(u8IpmbCmd == IPMB_CMD_COMM_ALIVE_CMD)
    {
        bsp_subboard_recv_heartBt(u8Slot, u8BoardType);
        if(g_hmi_heartbeat_debug == 1)
        {
            printf("recv slot %d channel 0 alive cmd!\r\n",u8Slot);
        }
        if((BOARD_TYPE_BBP==u8BoardType)||(BOARD_TYPE_GES==u8BoardType)||
            (BOARD_TYPE_FSA==u8BoardType))
        {
            /* float类型数据大小端转换 */
            u8FloatBuf[0] = pu8Buf[11];
            u8FloatBuf[1] = pu8Buf[10];
            u8FloatBuf[2] = pu8Buf[9];
            u8FloatBuf[3] = pu8Buf[8];
            /* 保存温度信息 */
            boards[u8Slot].temperature = *(f32*)(u8FloatBuf);
        }
        else if(BOARD_TYPE_PEU == u8BoardType)
        {
            /* 保存监控板温度 */
            boards[u8Slot].temperature = (pu8Buf[9]<<8) | pu8Buf[10];
        }
        else if(BOARD_TYPE_ES == u8BoardType)
        {
            /* 保存ES板温度 */
            boards[u8Slot].temperature = (pu8Buf[8]<<8) | pu8Buf[9];
        }
    }
    /* 升级消息处理 */
    else if(u8IpmbCmd == IPMB_CMD_ID_BOOTREQ)
    {
        if((BOARD_TYPE_FAN==u8BoardType)||(BOARD_TYPE_PEU==u8BoardType))
        {
            if(pthread_mutex_trylock(&ipmb_lock)==BSP_OK)
            {
                /* 获取pkg_id */
                memcpy((u8RspBuf+6), (pu8Buf+8), 2);
                /* 获取filename */
                memcpy(filename, (char *)(pu8Buf+10), 12);
                if(BSP_OK == ipmb_get_file_size(filename))
                {
                    *(unsigned int *)(u8RspBuf+8) = htonl(ipmb_st.st_size);
                    u8RspLength = 6;
                    mcu_update_flag = 1;
                }
                else
                {
                    pthread_mutex_unlock(&ipmb_lock);
                }
            }
            else
            {
                printf("[%s]last file download is not finish.\r\n",__func__);
                /* 获取pkg_id */
                memcpy((u8RspBuf+6),(pu8Buf+8),2);
                *(unsigned int *)(u8RspBuf+8) = 0;
                u8RspLength = 6;
                mcu_update_flag = 0;
            }
        }
    }
    /* RS485测试消息处理 */
    else if(u8IpmbCmd == IPMB_CMD_PEUACTL_RS485_TEST)
    {
        //置RS485测试标志位
        g_peu_rs485_test = 1;
        if(g_hmi_response_debug)
        {
            printf("peu rs485 tesk ok.\r\n");
        }
    }
    else
    {
        printf("unkown hmi request message.\r\n");
        return 0;
    }
    /* 计算目的i2c地址 */
    u8DstAddr = IPMB_MCT_HMI2_I2C_ADDR_BASE + u8Slot;

    //回复应答消息
    u8ret = bsp_send_hmi_data(IPMB_SEND_MESSAGE_PROCESS, \
                              u8IpmbCmd, \
                              IPMB_PRI_FIRST, \
                              IPMB_MCT_HMI2_I2C_ADDR(bsp_get_slot_id()), \
                              u8DstAddr, \
                              u8RspBuf, \
                              u8RspLength \
                             );
    /* 判断是否需要升级 */
    if(mcu_need_update_ver)
    {
        mcu_need_update_ver = 0;
        bsp_hmi_mcu_update(u8BoardType, u8Slot);
    }
    if(mcu_update_flag)
    {
        mcu_update_flag = 0;
        init_ipmb_file_download(u8DstAddr);
    }
    /* 判断是否需要设置风扇转速 */
    if(fan_set_flag)
    {
        fan_set_flag = 0;
        fan_control_algorithm_2(100);
    }
    return u8ret;
}

/**************************************************************************************************
*  函数名称: bsp_ipmb_process_response
*  描    述: 处理应答消息报文
*  参    数: (1)pu8Buf: pointer of send buffer.
*            (2)u8Length: length of send buffer.
*  返    回: 无.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_process_response(UINT8 *pu8Buf, UINT8 u8Length)
{
    UINT8 u8SeqNo;             /* 序列号 */
    UINT8 u8IpmbCmd;           /* 命令号 */
    UINT8 u8SrcAddr,u8slot,u8boardtype;
    UINT8 i;
    UINT32 dwreg,dwval,u32packetsnum;
	
    /* 取出序列号 */
    u8SeqNo = pu8Buf[SEQNO_BYTE];
    /* 取出命令号 */
    u8IpmbCmd = pu8Buf[CMD_BYTE];
    /* 取出源地址 */
    u8SrcAddr = pu8Buf[RQADDR_BYTE];
    /* 计算槽位号 */
    u8slot = bsp_get_slot_byi2caddr(u8SrcAddr);
    /* 在发送消息数组中找到对应的序列号，如果没有找到则丢弃该条消息 */
    for(i = 0; i < IPMB_MESSAGE_MAX_NUM; i++)
    {
        if((g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][i].u8Used == 1) && (g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][i].u8Seq == u8SeqNo))
        {
            g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][i].u8Status = STATUS_RECEIVE_RESPOND;
            if((u8IpmbCmd != IPMB_CMD_COMM_CHECK_ALIVE)&&(u8IpmbCmd != IPMB_CMD_COMM_START_SEND_MESSAGE))
            	g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][i].u8Used = 0;
        }
        else if((g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_SECOND][i].u8Used == 1) && (g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_SECOND][i].u8Seq == u8SeqNo))
        {
            g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_SECOND][i].u8Status = STATUS_RECEIVE_RESPOND;
            g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_SECOND][i].u8Used = 0;
        }
        else if	((g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_THIRD][i].u8Used == 1) &&(g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_THIRD][i].u8Seq == u8SeqNo))
        {
            g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_THIRD][i].u8Status = STATUS_RECEIVE_RESPOND;
            g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_THIRD][i].u8Used = 0;
        }
        else
        {
            continue;
        }
        switch(u8IpmbCmd)
        {
        case IPMB_CMD_COMM_RESET_BOARDS:
        case IPMB_CMD_BASEBANDCTL_REBOOT:
        case IPMB_CMD_SWITCHCTL_REBOOT:
        case IPMB_CMD_FSA_REBOOT:            
        case IPMB_CMD_ES_REBOOT:
        case IPMB_CMD_FANCTL_REBOOT:
        case IPMB_CMD_PEUACTL_REBOOT:
            /* 取出单板槽位号 */
            u8slot = pu8Buf[7];
            if((u8slot > 1) && (u8slot < 11))
            {
                boards[u8slot].mcu_status = MCU_STATUS_RESET_ACKED;
                printf("recv slot(%d) ipmb reset ack!\r\n", u8slot);
            }
            break;
        case IPMB_CMD_COMM_START_SEND_MESSAGE:
            /* 取出单板槽位号 */
            u8slot = pu8Buf[7];
            if((u8slot > 1) && (u8slot < 11))
            {
                boards[u8slot].mcu_alive = 1;
                printf("recv slot(%d) start message ack!\r\n", u8slot);
            }
            break;
        case IPMB_CMD_COMM_CHECK_ALIVE:
            /* 取出单板槽位号 */
            u8slot = pu8Buf[7];
            if((u8slot > 1) && (u8slot < 11))
            {
                boards[u8slot].mcu_alive = 1;
                printf("recv alive message(%d)!\r\n", u8slot);
            }
            break;

        /* 写fpga寄存器 */
        case IPMB_CMD_BASEBANDCTL_WRITE_FPGA:
            dwreg = (((UINT32)pu8Buf[7]<<24)|((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            dwval = (((UINT32)pu8Buf[11]<<24)|((UINT32)pu8Buf[12]<<16)|((UINT32)pu8Buf[13]<<8)|((UINT32)pu8Buf[14]<<0));
            bsp_put_to_fpga_buff(dwreg,dwval);
            if(g_hmi_response_debug == 1)
                printf("The fpga reg(%d) write value is 0x%x.\r\n",dwreg, dwval);
            break;
        /* 写epld寄存器 */
        case IPMB_CMD_BASEBANDCTL_WRITE_EPLD:
            dwreg = (((UINT32)pu8Buf[7]<<24)|((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            dwval = (((UINT32)pu8Buf[11]<<24)|((UINT32)pu8Buf[12]<<16)|((UINT32)pu8Buf[13]<<8)|((UINT32)pu8Buf[14]<<0));
            bsp_put_to_epld_buff(dwreg,dwval);
            if(g_hmi_response_debug == 1)
                printf("The epld reg(%d) write value is 0x%x.\r\n",dwreg, dwval);
            break;
        /* 读fpga寄存器 */
        case IPMB_CMD_BASEBANDCTL_READ_FPGA:
            dwreg = (((UINT32)pu8Buf[7]<<24)|((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            dwval = (((UINT32)pu8Buf[11]<<24)|((UINT32)pu8Buf[12]<<16)|((UINT32)pu8Buf[13]<<8)|((UINT32)pu8Buf[14]<<0));
            bsp_put_to_fpga_buff(dwreg,dwval);
            if(g_hmi_response_debug == 1)
                printf("The fpga reg(%d) read value is 0x%x.\r\n",dwreg, dwval);
            break;
        /* 读epld寄存器 */
        case IPMB_CMD_BASEBANDCTL_READ_EPLD:
            dwreg = (((UINT32)pu8Buf[7]<<24)|((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            dwval = (((UINT32)pu8Buf[11]<<24)|((UINT32)pu8Buf[12]<<16)|((UINT32)pu8Buf[13]<<8)|((UINT32)pu8Buf[14]<<0));
            bsp_put_to_epld_buff(dwreg,dwval);
            if(g_hmi_response_debug == 1)
                printf("The epld reg(%d) read value is 0x%x.\r\n",dwreg, dwval);
            break;

        /* 设置风扇转速 */
        case IPMB_CMD_FANCTL_SET_SPEED:
            g_fan_speed_set = 1;
			g_fan_set_state = BSP_OK;
            if(g_hmi_response_debug)
                printf("Set fan channel %d speed respond value is %d!\r\n",pu8Buf[7],pu8Buf[8]);
            break;

        /* 获取风扇转速 */
        case IPMB_CMD_FANCTL_GET_SPEED:
            g_fan_speed_get = 1;
            /* 获取通道0的风扇转速 */
            if(pu8Buf[7] == 0)
            {
                g_fan1_speed = (((UINT32)pu8Buf[8]<<24) |((UINT32)pu8Buf[9]<<16)|((UINT32)pu8Buf[10]<<8)|((UINT32)pu8Buf[11]<<0));
                if(g_hmi_response_debug)
                    printf("The channel %d Current fan speed is %d.\r\n",pu8Buf[7],g_fan1_speed);
            }
            /* 获取通道1的风扇转速 */
            else if(pu8Buf[7] == 1)
            {
                g_fan2_speed = (((UINT32)pu8Buf[8]<<24) |((UINT32)pu8Buf[9]<<16)|((UINT32)pu8Buf[10]<<8)|((UINT32)pu8Buf[11]<<0));
                if(g_hmi_response_debug)
                    printf("The channel %d Current fan speed is %d.\r\n",pu8Buf[7],g_fan2_speed);
            }
            /* 获取通道2的风扇转速 */
            else if(pu8Buf[7] == 2)
            {
                g_fan3_speed = (((UINT32)pu8Buf[8]<<24) |((UINT32)pu8Buf[9]<<16)|((UINT32)pu8Buf[10]<<8)|((UINT32)pu8Buf[11]<<0));
                if(g_hmi_response_debug)
                    printf("The channel %d Current fan speed is %d.\r\n",pu8Buf[7],g_fan3_speed);
            }
            /* 同时获取3个通道的风扇转速 */
            if(pu8Buf[7] == 3)
            {
                g_fan1_speed = (((UINT32)pu8Buf[8]<<24) |((UINT32)pu8Buf[9]<<16)|((UINT32)pu8Buf[10]<<8)|((UINT32)pu8Buf[11]<<0));
                g_fan2_speed = (((UINT32)pu8Buf[12]<<24) |((UINT32)pu8Buf[13]<<16)|((UINT32)pu8Buf[14]<<8)|((UINT32)pu8Buf[15]<<0));
                g_fan3_speed = (((UINT32)pu8Buf[16]<<24) |((UINT32)pu8Buf[17]<<16)|((UINT32)pu8Buf[18]<<8)|((UINT32)pu8Buf[19]<<0));
                if(g_hmi_response_debug)
                {
                    printf("The channel 0 Current fan speed is %d.\r\n",g_fan1_speed);
                    printf("The channel 1 Current fan speed is %d.\r\n",g_fan2_speed);
                    printf("The channel 2 Current fan speed is %d.\r\n",g_fan3_speed);
                }
            }
            break;

        /* 获取发送报文成功数统计 */
        case IPMB_CMD_BASEBANDCTL_SEND_PACK_OK_CNT:
        case IPMB_CMD_SWITCHCTL_SEND_PACK_OK_CNT:
        case IPMB_CMD_FANCTL_SEND_PACK_OK_CNT:
        case IPMB_CMD_PEUACTL_SEND_PACK_OK_CNT:
            u32packetsnum = (((UINT32)pu8Buf[7]<<24) |((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            if(g_hmi_response_debug == 1)
                printf("hmi send sucessful packets is %d.\r\n",u32packetsnum);
            break;

        /* 获取发送报文失败数统计 */
        case IPMB_CMD_BASEBANDCTL_SEND_PACK_ERR_CNT:
        case IPMB_CMD_SWITCHCTL_SEND_PACK_ERR_CNT:
        case IPMB_CMD_FANCTL_SEND_PACK_ERR_CNT:
        case IPMB_CMD_PEUACTL_SEND_PACK_ERR_CNT:
            u32packetsnum = (((UINT32)pu8Buf[7]<<24) |((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            if(g_hmi_response_debug == 1)
                printf("hmi send failed packets is %d.\r\n",u32packetsnum);
            break;

        /* 获取接收报文成功数统计 */
        case IPMB_CMD_BASEBANDCTL_RECV_PACK_OK_CNT:
        case IPMB_CMD_SWITCHCTL_RECV_PACK_OK_CNT:
        case IPMB_CMD_FANCTL_RECV_PACK_OK_CNT:
        case IPMB_CMD_PEUACTL_RECV_PACK_OK_CNT:
            u32packetsnum = (((UINT32)pu8Buf[7]<<24) |((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            if(g_hmi_response_debug == 1)
                printf("hmi received sucessful packets is %d.\r\n",u32packetsnum);
            break;

        /* 获取接收报文失败数统计 */
        case IPMB_CMD_BASEBANDCTL_RECV_PACK_ERR_CNT:
        case IPMB_CMD_SWITCHCTL_RECV_PACK_ERR_CNT:
        case IPMB_CMD_FANCTL_RECV_PACK_ERR_CNT:
        case IPMB_CMD_PEUACTL_RECV_PACK_ERR_CNT:
            u32packetsnum = (((UINT32)pu8Buf[7]<<24) |((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            if(g_hmi_response_debug == 1)
                printf("hmi received failed packets is %d.\r\n",u32packetsnum);
            break;

        /* 上电命令 */
        case IPMB_CMD_BASEBANDCTL_POWER_ON:
        case IPMB_CMD_SWITCHCTL_POWER_ON:
        case IPMB_CMD_FANCTL_POWER_ON:
        case IPMB_CMD_PEUACTL_POWER_ON:
            if(g_hmi_response_debug == 1)
                printf("Power on response value is %d!\r\n",pu8Buf[7]);
            break;
        /* 下电命令 */
        case IPMB_CMD_BASEBANDCTL_POWER_OFF:
        case IPMB_CMD_SWITCHCTL_POWER_OFF:
        case IPMB_CMD_FANCTL_POWER_OFF:
        case IPMB_CMD_PEUACTL_POWER_OFF:
            if(g_hmi_response_debug == 1)
                printf("Power off response value is %d!\r\n",pu8Buf[7]);
            g_peu_power_down_flag = 1;
            break;

        /* 获取基带板温度值 */
        case IPMB_CMD_BASEBANDCTL_GET_TEMP:
        {
            UINT8 u8cnt;
            /* UINT8类型数据转换为SINT8 */
            memcpy(g_s8BbpGetTemperature, (SINT8 *)(pu8Buf + 7), 8);
            if(g_hmi_response_debug)
            {
                for(u8cnt = 0; u8cnt < 7; u8cnt++)
                    printf("The baseband board point%d temperature = %d.\r\n",u8cnt,g_s8BbpGetTemperature[u8cnt]);
            }
        }
        break;

        /* 获取FSA板温度值 */
        case IPMB_CMD_FSA_GET_TEMP:
        {
            UINT8 u8cnt;
            /* UINT8类型数据转换为SINT8 */
            memcpy(g_s8FsaGetTemperature, (SINT8 *)(pu8Buf + 7), 4);
            if(g_hmi_response_debug)
            {
                for(u8cnt = 0; u8cnt < 4; u8cnt++)
                    printf("The switch board point%d temperature = %d.\r\n",u8cnt,g_s8FsaGetTemperature[u8cnt]);
            }
        }
        break;
         /* 获取ES板温度值 */
        case IPMB_CMD_ES_GET_TEMP:
        {
            g_s16EsTemp = (pu8Buf[7]<<8) | pu8Buf[8];
            if(g_hmi_response_debug)
                printf("The es board temperature = %d.\r\n",g_s16EsTemp);
        }
        /* 获取监控板温度值 */
        case IPMB_CMD_PEUACTL_GET_TEMP:
        {
            g_peu_get_temp = 1;
            g_s16PEUTemp = (pu8Buf[7]<<8) | pu8Buf[8];
            if(g_hmi_response_debug)
                printf("The peua board temperature = %d.\r\n",g_s16PEUTemp);
        }
        break;
        /* 获取ARM版本信息*/
        case IPMB_CMD_BASEBANDCTL_GET_ARM_VER_VAL:
        case IPMB_CMD_SWITCHCTL_GET_ARM_VER_VAL:
        case IPMB_CMD_FSA_GET_ARM_VER_VAL:
        case IPMB_CMD_ES_GET_ARM_VER_VAL:
        case IPMB_CMD_FANCTL_GET_ARM_VER_VAL:
        case IPMB_CMD_PEUACTL_GET_ARM_VER_VAL:   
        {
            memcpy(boards[u8slot].arm_version, (s8*)&pu8Buf[7], u8Length - 8);
            if(g_hmi_response_debug)
                printf("The slot(%d) board ARM Version is %s\r\n", u8slot, boards[u8slot].arm_version);
        }
        break;

        /* 基带板HMI测试 */
        case IPMB_CMD_BASEBANDCTL_TEST:
        case IPMB_CMD_FSA_TEST:
        case IPMB_CMD_ES_TEST:
            g_bbp_hmi_test = 1;
            if(g_hmi_response_debug)
                printf("The baseband board hmi test ok(0x%x).\r\n",u8SeqNo);
            break;

        /* 交换板HMI测试 */
        case IPMB_CMD_SWITCHCTL_TEST:
            g_ges_hmi_test = 1;
            if(g_hmi_response_debug)
                printf("The switch board hmi test ok(0x%x).\r\n",u8SeqNo);
            break;

        /* 风扇板HMI测试 */
        case IPMB_CMD_FANCTL_TEST:
            g_fan_hmi_test = 1;
            if(g_hmi_response_debug)
                printf("The fan control board hmi test ok(0x%x).\r\n",u8SeqNo);
            break;

        /* 监控板HMI测试 */
        case IPMB_CMD_PEUACTL_TEST:
            g_peu_hmi_test = 1;
            if(g_hmi_response_debug)
                printf("The peua control board hmi test ok(0x%x).\r\n",u8SeqNo);
            break;

        case IPMB_CMD_BASEBANDCTL_TEST_EEPROM:
        case IPMB_CMD_SWITCHCTL_TEST_EEPROM:
        case IPMB_CMD_FSA_TEST_EEPROM:
        case IPMB_CMD_ES_TEST_EEPROM:
        case IPMB_CMD_BASEBANDCTL_TEST_GESWITCH:
        case IPMB_CMD_SWITCHCTL_TEST_GESWITCH:
        case IPMB_CMD_FSA_TEST_GESWITCH:
        case IPMB_CMD_ES_TEST_GESWITCH:
        case IPMB_CMD_SWITCHCTL_WORKMODE:
            if(pu8Buf[7] == 1)
                printf("The test result ok.\r\n");
            else
                printf("The test result failed.\r\n");
            break;

        /* 风扇板EEPROM测试 */
        case IPMB_CMD_FANCTL_TEST_EEPROM:
            if(pu8Buf[7] == 1)
            {
                g_fan_eeprom_test = 1;
                if(g_hmi_response_debug)
                    printf("The fan control board hmi test eeprom ok.\r\n");
            }
            else
            {
                g_fan_eeprom_test = 0;
                if(g_hmi_response_debug)
                    printf("The fan control board hmi test eeprom failed.\r\n");
            }
            break;


        /* 槽位号获取 */
        case IPMB_CMD_FANCTL_GET_SLOT:
        case IPMB_CMD_PEUACTL_GET_SLOT:
            if(g_hmi_response_debug)
                printf("The board slot = %d.\r\n", pu8Buf[7]);
            break;
        /* 板类型获取 */
        case IPMB_CMD_FANCTL_GET_BOARD_TYPE:
        case IPMB_CMD_PEUACTL_GET_BOARD_TYPE:
            if(g_hmi_response_debug)
                printf("The board type = %d.\r\n", pu8Buf[7]);
            break;
        /* 风扇板PCB版本号获取 */
        case IPMB_CMD_FANCTL_GET_PCB_VERSION:
        case IPMB_CMD_PEUACTL_GET_PCB_VERSION:
            if(g_hmi_response_debug)
                printf("The pcb version = %d.\r\n", pu8Buf[7]);
            break;

        /* 监控板干结点状态获取 */
        case IPMB_CMD_PEUACTL_GET_DRYIN_STATE:
            g_peu_get_dryin = 1;
            g_peu_dryin0123 = pu8Buf[7];
            g_peu_dryin4567 = pu8Buf[8];
            if(g_hmi_response_debug)
                printf("The peua control dryin0123 = 0x%x,dryin4567 = 0x%x.\r\n", pu8Buf[7],pu8Buf[8]);
            break;

        /* 风扇板EEPROM参数设置 */
        case IPMB_CMD_EEPROM_SET_CRC:
        case IPMB_CMD_EEPROM_SET_DEVICE_ID:
        case IPMB_CMD_EEPROM_SET_BOARD_TYPE:
        case IPMB_CMD_EEPROM_SET_PRODUCT_SN:
        case IPMB_CMD_EEPROM_SET_MANUFACTURER:
        case IPMB_CMD_EEPROM_SET_PRODUCT_DATE:
        case IPMB_CMD_EEPROM_SET_TEMP_THRESHOLD:
        case IPMB_CMD_EEPROM_SET_FAN_INITIALSPEED:
            if(pu8Buf[7] == 1)
            {
                g_fan_set_eeprom = 1;
                if(g_hmi_response_debug)
                    printf("The fan control board set eeprom ok.\r\n");
            }
            else
            {
                g_fan_set_eeprom = 0;
                if(g_hmi_response_debug)
                    printf("The fan control board set eeprom failed.\r\n");
            }
            break;
        /* 风扇板EEPROM参数获取 */
        case IPMB_CMD_EEPROM_GET_CRC:
        case IPMB_CMD_EEPROM_GET_DEVICE_ID:
        case IPMB_CMD_EEPROM_GET_BOARD_TYPE:
        case IPMB_CMD_EEPROM_GET_PRODUCT_SN:
        case IPMB_CMD_EEPROM_GET_MANUFACTURER:
        case IPMB_CMD_EEPROM_GET_PRODUCT_DATE:
        case IPMB_CMD_EEPROM_GET_TEMP_THRESHOLD:
        case IPMB_CMD_EEPROM_GET_FAN_INITIALSPEED:
            g_fan_get_eeprom = 1;
            bsp_write_eepromget_buf(u8IpmbCmd,(u8 *)(pu8Buf+7));
            break;

        case IPMB_CMD_ID_DATA:
            ipmb_pkg_id = (pu8Buf[7]<<8) | pu8Buf[8];
            ipmb_pkg_len = (pu8Buf[9]<<8) | pu8Buf[10];
            //sem_post(&sem_ipmb_recved);
            pthread_mutex_unlock(&ipmbfile_wait);
            break;

        default:
            break;
        }
    }
}
/**************************************************************************************************
*  函数名称: ipmb_file_download_thread
*  描    述: ipmb文件加载
*  参    数: 无
*  返    回: none.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
s32 ipmb_get_file_size(char *filename)
{
    if(filename == NULL)
        return BSP_ERROR;
    ipmb_filefd = open(filename, O_RDONLY);
    if(ipmb_filefd < 0)
    {
        perror("open ipmb transfile");
    }
    else
    {
        fstat(ipmb_filefd, &ipmb_st);
        if(ipmb_tftp_dbg_flag)
        {
            printf("filesize=%d\r\n", ipmb_st.st_size);
        }

        ipmb_filesize = ipmb_st.st_size;
        printf("start send file %s, filesize = %d.\r\n", filename, ipmb_filesize);
    }

    return BSP_OK;
}

s32 ipmb_file_download_thread(void *arg)
{
    u8  u8MsgBuf[IPMB_MESSAGE_MAX_LENGTH];   /* 消息数组 */
    u16 u16MsgLength = 0;                    /* 消息长度 */
    u16 data_len = 0;
    u16 need_pkg_id = 0;
    u16 need_pkg_len = 0;
    s32 ret = 0;
    u8 failtimes = 0;
    struct timespec ts;
    int count = 0;
    u8 u8DstAddr = *(u8*)arg;

    while(ipmb_filesize)
    {
        if(ipmb_filesize > 256)
            need_pkg_len = 256;
        else
            need_pkg_len = ipmb_filesize;
        if(failtimes == 0)
        {
            data_len = read(ipmb_filefd, u8MsgBuf+6, need_pkg_len);
        }
        if(data_len==0)
        {
            perror("data_len = 0!\n");
            return BSP_ERROR;
        }
        memcpy(u8MsgBuf, (u8 *)&need_pkg_id, 2);
        memcpy((u8MsgBuf+2), (u8 *)&need_pkg_len, 2);
        memset((u8MsgBuf+4), 0, 2);
        u16MsgLength = 6 + data_len;
        ret = bsp_send_hmi_data(
                  IPMB_SEND_MESSAGE_REQUEST, \
                  IPMB_CMD_ID_DATA, \
                  IPMB_PRI_FIRST, \
                  IPMB_MCT_HMI2_I2C_ADDR(bsp_get_slot_id()), \
                  u8DstAddr, \
                  u8MsgBuf, \
                  u16MsgLength \
              );
        count = 0;
        while((pthread_mutex_trylock(&ipmbfile_wait)==0));
        while(pthread_mutex_trylock(&ipmbfile_wait))
        {
            if(count == 50)
                break;
            usleep(100000);
            count++;
        }

        if((need_pkg_id == ipmb_pkg_id) && (need_pkg_len = ipmb_pkg_len))
        {
            need_pkg_id++;
            failtimes = 0;
            ipmb_filesize -= ipmb_pkg_len;
            if(ipmb_filesize == 0)
            {
                printf("\nipmb send file succeed.\n");
                pthread_mutex_unlock(&ipmb_lock);
            }
        }
        else
        {
            failtimes++;
            printf("\nerror pkg[need_pkg_id=%d, need_pkg_len=%d, pkg_id=%d, pkg_len=%d], last_size=%d.\n",
                   need_pkg_id, need_pkg_len, ipmb_pkg_id, ipmb_pkg_len, ipmb_filesize);
            if(failtimes == 3)
            {
                printf("send fail 3 times, stop send...\n");
                pthread_mutex_unlock(&ipmb_lock);
                break;
            }
        }
    }
}
s32 init_ipmb_file_download(u8 u8DstAddr)
{
    int res=0;
    pthread_t ptida;

    //创建ipmb文件下载线程
    res = pthread_create(&ptida, NULL, (FUNCPTR)ipmb_file_download_thread, (void*)&u8DstAddr);
    if (BSP_ERROR == res)
    {
        perror("create bsp_ipmb_file_download_thread error!\n");
        pthread_mutex_unlock(&ipmb_lock);
    }
    pthread_detach(ptida);
    sleep(1);
}
/**************************************************************************************************
*  函数名称: bsp_put_to_fpga_buff
*  描    述: ipmb应答消息处理函数
*  参    数: (1)dwReg:寄存器地址
*            (2)dwVal:寄存器数值
*  返    回: none.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_put_to_fpga_buff(UINT32 dwReg,UINT32 dwVal)
{
    UINT8 u8Index = 0;
    UINT8 u8flag  = 0;
    printf("loading bsp_put_to_fpga_buff!dwReg:0x%lx,dwVal:0x%lx\r\n",dwReg,dwVal);
    for(u8Index = 0; u8Index < READ_FPGA_BUF_NUM; u8Index++)
    {
        if( dwReg == g_struReadFpgaQueueBuf[u8Index].dwReg)
        {
            g_struReadFpgaQueueBuf[u8Index].dwVal = dwVal;
            g_struReadFpgaQueueBuf[u8Index].u8Used = 1;
            u8flag = 1;
            break;
        }
    }
    if (u8flag !=1)
    {
        for(u8Index = 0; u8Index < READ_FPGA_BUF_NUM; u8Index++)
        {
            if (g_struReadFpgaQueueBuf[u8Index].u8Used == 0)
            {
                g_struReadFpgaQueueBuf[u8Index].dwReg = dwReg;
                g_struReadFpgaQueueBuf[u8Index].dwVal = dwVal;
                g_struReadFpgaQueueBuf[u8Index].u8Used = 1;
                break;
            }
        }
    }

}
/**************************************************************************************************
*  函数名称: bsp_put_to_epld_buff
*  描    述: ipmb应答消息处理函数
*  参    数: dwReg:寄存器地址
*            dwVal:寄存器数值
*  返    回: none.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_put_to_epld_buff(UINT32 dwReg,UINT32 dwVal)
{
    UINT8 u8Index = 0;
    UINT8 u8flag  = 0;
    printf("loading bsp_put_to_epld_buff!dwReg:0x%lx,dwVal:0x%lx\r\n",dwReg,dwVal);
    for(u8Index = 0; u8Index < READ_EPLD_BUF_NUM; u8Index++)
    {
        if( dwReg == g_struReadEpldQueueBuf[u8Index].dwReg)
        {
            g_struReadEpldQueueBuf[u8Index].dwVal = dwVal;
            g_struReadEpldQueueBuf[u8Index].u8Used = 1;
            u8flag = 1;
            break;
        }
    }
    if (u8flag !=1)
    {
        for(u8Index = 0; u8Index < READ_EPLD_BUF_NUM; u8Index++)
        {
            if (g_struReadEpldQueueBuf[u8Index].u8Used == 0)
            {
                g_struReadEpldQueueBuf[u8Index].dwReg = dwReg;
                g_struReadEpldQueueBuf[u8Index].dwVal = dwVal;
                g_struReadEpldQueueBuf[u8Index].u8Used = 1;
                break;
            }
        }
    }

}

/**************************************************************************************************
*  函数名称: bsp_proc_recvhmidata
*  描    述: ipmb接收数据处理函数
*  参    数: none
*  返    回: none.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
uint32_t test_rmsg_num = 0;
void bsp_ipmb_receive_task(void)
{
    UINT8  ret;
    UINT8 u8CurrentIndex = 0;                    /* 接收数组的当前索引 */
    UINT8 u8MsgBuf[IPMB_MESSAGE_MAX_LENGTH];     /* 保存接收到的IPMB消息 */
    UINT32 u32MsgLength;                           /* 接收到的IPMB消息的长度 */
    UINT8 u8NetFn;                               /* 网络功能号*/
    UINT8 u8SeqNo;                               /* 序列号*/
    UINT32 i;
    UINT8 u8Err;

    while(1)
    {
        /* 等待信号量 */
        sem_wait(&g_ipmb_sema);
        for(u8CurrentIndex=0; u8CurrentIndex<IPMB_RECEIVE_BUF_NUM; u8CurrentIndex++)
        {
            memset(u8MsgBuf,0,sizeof(u8MsgBuf));
            if(g_struReceiveMsgBuf[u8CurrentIndex].u8Used != 1)
                continue;
            u32MsgLength = g_struReceiveMsgBuf[u8CurrentIndex].u32Length;
            if(u32MsgLength == 0)
                continue;

            for(i=0; i<u32MsgLength; i++)
            {
                u8MsgBuf[i] = g_struReceiveMsgBuf[u8CurrentIndex].u8Message[i];
            }
            ret = bsp_ipmb_message_check(u8MsgBuf, u32MsgLength);
            if(g_u32hmi_debug==1)
            {
                if((u8MsgBuf[5] == IPMB_CMD_FANCTL_TEST) ||
                        (u8MsgBuf[5] == IPMB_CMD_PEUACTL_TEST) ||
                        (u8MsgBuf[5] == IPMB_CMD_BASEBANDCTL_TEST))
                {
                    printf("u8RecvData(%d)=", test_rmsg_num++);
                }
                else
                    printf("u8RecvData=");
                for(i=0; i<u32MsgLength; i++)
                    printf("%2x ", u8MsgBuf[i]);
                printf("\r\n");
            }
            if(ret == TRUE)
            {
                /* 判断是请求消息还是应答消息 */
                u8NetFn = u8MsgBuf[NETFN_BYTE] >> 2;
                if(u8NetFn % 2 == 0)
                {
                    /* 请求消息 */
                    bsp_ipmb_process_request(u8MsgBuf, u32MsgLength);
                }
                else
                {
                    /* 应答消息 */
                    bsp_ipmb_process_response(u8MsgBuf, u32MsgLength);
                }
            }
            else
            {
                printf("recv data check error.\r\n");
            }
            g_struReceiveMsgBuf[u8CurrentIndex].u8Used = 0;
        }
    }
}
#if 0
/**************************************************************************************************
*  函数名称: bsp_ipmb_sendfpga_task
*  描    述: 写fpga寄存器任务
*  参    数: none
*  返    回: none.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_sendfpga_task(void)
{
    UINT8 u8Err;
    UINT32 u32Index=0;
    while(1)
    {
        sem_wait(&g_ipmb_writefpga);
        for (u32Index=0; u32Index<WRITE_FPGA_BUF_NUM; u32Index++)
        {
            printf("g_struWriteFpgaQueueBuf[%d].u8Used=0x%lx\r\n",u32Index,g_struWriteFpgaQueueBuf[u32Index].u8Used);
            if (g_struWriteFpgaQueueBuf[u32Index].u8Used == 1)
            {
                printf("oam write fpga reg:0x%lx,value=0x%lx\r\n",g_struWriteFpgaQueueBuf[u32Index].dwReg,g_struWriteFpgaQueueBuf[u32Index].dwVal);
                bsp_ipmb_ioctl(IPMB_CMD_BASEBANDCTL_WRITE_FPGA, g_struWriteFpgaQueueBuf[u32Index].dwReg, g_struWriteFpgaQueueBuf[u32Index].dwVal);
                g_struWriteFpgaQueueBuf[u32Index].u8Used = 0;
                usleep(100);
            }
        }
    }

}

/**************************************************************************************************
*  函数名称: bsp_ipmb_sendepld_task
*  描    述: 写cpld寄存器任务
*  参    数: none
*  返    回: none.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_sendepld_task(void)
{
    UINT8 u8Err;
    UINT32 u32Index=0;
    while(1)
    {
        sem_wait(&g_ipmb_writeepld);
        for (u32Index=0; u32Index<WRITE_EPLD_BUF_NUM; u32Index++)
        {
            printf("g_struWriteEpldQueueBuf[%d].u8Used=0x%lx\r\n",u32Index,g_struWriteEpldQueueBuf[u32Index].u8Used);
            if (g_struWriteEpldQueueBuf[u32Index].u8Used == 1)
            {
                printf("oam write epld reg:0x%lx,value=0x%lx\r\n",g_struWriteEpldQueueBuf[u32Index].dwReg,g_struWriteEpldQueueBuf[u32Index].dwVal);
                bsp_ipmb_ioctl(IPMB_CMD_BASEBANDCTL_WRITE_EPLD, g_struWriteEpldQueueBuf[u32Index].dwReg, g_struWriteEpldQueueBuf[u32Index].dwVal);
                g_struWriteEpldQueueBuf[u32Index].u8Used = 0;
                usleep(100);
            }
        }
    }
}
/**************************************************************************************************
*  函数名称: bsp_ipmb_sendhmi_task
*  描    述: 发送hmi数据任务
*  参    数: none
*  返    回: none.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_sendhmi_task(void)
{
    UINT8 u8Err;
    UINT32 u32Index=0;
    UINT32 u32Pri=0;
    while(1)
    {
        sem_wait(&g_ipmb_semsend);
        for (u32Index=0; u32Index<IPMB_MESSAGE_MAX_NUM; u32Index++)
        {
            if (g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_FIRST][u32Index].u8Used == 1)
            {
                bsp_ipmb_send_message(IPMB_CHANNEL_IIC_3, g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_FIRST][u32Index].u8Data[RSADDR_BYTE], g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_FIRST][u32Index].u8Data, g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_FIRST][u32Index].u32Len);
                g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_FIRST][u32Index].u8Used = 0;
                //bsp_delay_1ms(100);
                //usleep(100000);
            }
        }

        for (u32Index=0; u32Index<IPMB_MESSAGE_MAX_NUM; u32Index++)
        {
            if (g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_SECOND][u32Index].u8Used == 1)
            {
                bsp_ipmb_send_message(IPMB_CHANNEL_IIC_3, g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_FIRST][u32Index].u8Data[RSADDR_BYTE], g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_SECOND][u32Index].u8Data, g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_SECOND][u32Index].u32Len);

                g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_SECOND][u32Index].u8Used = 0;
                //bsp_delay_1ms(100);
                //usleep(100000);
            }
        }

        for (u32Index=0; u32Index<IPMB_MESSAGE_MAX_NUM; u32Index++)
        {
            if (g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_THIRD][u32Index].u8Used == 1)
            {
                bsp_ipmb_send_message(IPMB_CHANNEL_IIC_3, g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_FIRST][u32Index].u8Data[RSADDR_BYTE], g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_THIRD][u32Index].u8Data, g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_THIRD][u32Index].u32Len);

                g_struQueueMsgBuf.StruReplyMessage[IPMB_PRI_THIRD][u32Index].u8Used = 0;
                //bsp_delay_1ms(100);
                //usleep(100000);
            }
        }

        for (u32Index=0; u32Index<IPMB_MESSAGE_MAX_NUM; u32Index++)
        {
            if (g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][u32Index].u8Used == 1)
            {
                bsp_ipmb_send_message(IPMB_CHANNEL_IIC_3,g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][u32Index].u8Data[RSADDR_BYTE], g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][u32Index].u8Data, g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][u32Index].u32Len);
                //usleep(100000);
                //bsp_delay_1ms(100);
                g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][u32Index].u8Used = 0;
            }
        }

        for (u32Index=0; u32Index<IPMB_MESSAGE_MAX_NUM; u32Index++)
        {
            if (g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_SECOND][u32Index].u8Used == 1)
            {
                bsp_ipmb_send_message(IPMB_CHANNEL_IIC_3,g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][u32Index].u8Data[RSADDR_BYTE], g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_SECOND][u32Index].u8Data, g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_SECOND][u32Index].u32Len);
                //usleep(100000);
                //bsp_delay_1ms(100);
                g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_SECOND][u32Index].u8Used = 0;
            }
        }

        for (u32Index=0; u32Index<IPMB_MESSAGE_MAX_NUM; u32Index++)
        {
            if (g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_THIRD][u32Index].u8Used == 1)
            {
                bsp_ipmb_send_message(IPMB_CHANNEL_IIC_3,g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_FIRST][u32Index].u8Data[RSADDR_BYTE], g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_THIRD][u32Index].u8Data, g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_THIRD][u32Index].u32Len);
                //usleep(100000);
                //bsp_delay_1ms(100);
                g_struQueueMsgBuf.StruRequestMessage[IPMB_PRI_THIRD][u32Index].u8Used = 0;
            }
        }
    }
}
#endif

/**************************************************************************************************
*  函数名称: bsp_ipmb_init
*  描    述: ipmb初始化
*  参    数: none
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_init(void)
{
    UINT8 u8Index,i;
    INT8 s8Err;
    int res=0;

    pthread_t ptida,ptidwritefpga,ptidwriteepld;
    pthread_attr_t  attra,attrsnd,attrepld;
    struct sched_param parma,parmsnd,parmepld;

    pthread_attr_init(&attra);
    pthread_attr_setinheritsched(&attra, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attra, 1024*1024);
    pthread_attr_setschedpolicy(&attra, SCHED_FIFO);
    parma.sched_priority = 30;
    pthread_attr_setschedparam(&attra, &parma);
    pthread_attr_init(&attrsnd);
    pthread_attr_setinheritsched(&attrsnd, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attrsnd, 1024*1024);
    pthread_attr_setschedpolicy(&attrsnd, SCHED_FIFO);
    parmsnd.sched_priority = 60;
    pthread_attr_setschedparam(&attrsnd, &parmsnd);

    sem_init(&g_ipmb_sema,0,0);
    //sem_init(&g_ipmb_writefpga,0,0);
    //sem_init(&g_ipmb_semsend,0,0);

    pthread_mutex_init(&g_hmi1_mutex, NULL);
    pthread_mutex_init(&g_hmi2_mutex, NULL);
    pthread_mutex_init(&g_queue_mutex, NULL);
    pthread_mutex_init(&g_semFpgaWrite, NULL);
    pthread_mutex_init(&g_semEpldWrite, NULL);
    pthread_mutex_init(&ipmb_lock, NULL);
    memset(g_struQueueMsgBuf.StruReplyMessage,0,sizeof(g_struQueueMsgBuf.StruReplyMessage));
    memset(g_struQueueMsgBuf.StruRequestMessage,0,sizeof(g_struQueueMsgBuf.StruRequestMessage));

    /*初始化读fpga数组 */
    for(u8Index = 0; u8Index < READ_FPGA_BUF_NUM; u8Index++)
    {
        g_struReadFpgaQueueBuf[u8Index].u8Used = 0;
        /* 其他元素初始化 */
        for(i=0; i<READ_FPGA_BUF_NUM; i++)
        {
            g_struReadFpgaQueueBuf[u8Index].dwReg = 0xdeadbeef;
            g_struReadFpgaQueueBuf[u8Index].dwVal = 0xdeadbeef;
        }
    }

    /*初始化读EPLD数组 */
    for(u8Index = 0; u8Index < READ_EPLD_BUF_NUM; u8Index++)
    {
        g_struReadEpldQueueBuf[u8Index].u8Used = 0;
        /* 其他元素初始化 */
        for(i=0; i<READ_EPLD_BUF_NUM; i++)
        {
            g_struReadEpldQueueBuf[u8Index].dwReg = 0xdeadbeef;
            g_struReadEpldQueueBuf[u8Index].dwVal = 0xdeadbeef;
        }
    }
    /* 初始化接收消息数组 */
    for(u8Index = 0; u8Index < IPMB_RECEIVE_BUF_NUM; u8Index++)
    {
        g_struReceiveMsgBuf[u8Index].u8Used = 0;
    }

    res = pthread_create(&ptida, &attra, (FUNCPTR)bsp_ipmb_receive_task,NULL);
    pthread_attr_destroy(&attra);
    if (-1 == res)
    {
        perror("create bsp_ipmb_receive_task thread error!\n");
    }
#if 0
    res = pthread_create(&ptidwritefpga, &attrsnd, (FUNCPTR)bsp_ipmb_sendhmi_task,NULL);
    pthread_attr_destroy(&attrsnd);
    if (-1 == res)
    {
        perror("create bsp_ipmb_sendhmi_task thread error!\n");
    }
#endif
    /* hmi ioctl初始化 */
    bsp_hmi_ioctl_init(bsp_get_slot_id());
    /* 风扇转速控制 */
    bsp_ipmb_fan_control_init();

    return BSP_OK;
}

/**************************************************************************************************
*  函数名称: bsp_hmi_config_table_init
*  描    述: hmi配置参数初始化
*  参    数:
*  返    回: 无.
*  作    者:刘刚
*  日    期: 2015/11/6
**************************************************************************************************/
void bsp_hmi_config_table_init(void)
{
    WORD16  dwThreadTypeIndex = 0;
    static  UINT32 dwindex=0;
    WORD16  dwTableItemCounts = (WORD16)(sizeof(gaUserHmiConfigTable)/sizeof(T_HmiRegInfo));
    for (dwThreadTypeIndex = 0; dwThreadTypeIndex < dwTableItemCounts; dwThreadTypeIndex++)
    {
        //if (TRUE == (gaUserHmiConfigTable + dwThreadTypeIndex)->isFlag)
        {
            (gaUserHmiConfigTable + dwThreadTypeIndex)->index = dwindex++;
        }
    }
    g_hmi_config_table = dwindex;
}


/**************************************************************************************************
*  函数名称: bsp_hmi_print_status
*  描    述: 打印hmi的数据统计
*  参    数:
*  返    回: 无.
*  作    者:刘刚
*  日    期: 2015/11/6
**************************************************************************************************/
void bsp_print_hmi_status(void)
{
    HMI_PACK_STATIC *ptHmiStatic;
    ptHmiStatic = &g_tHmiStatic;
    UINT32  dwThreadTypeIndex = 0;
    WORD16  dwTableItemCounts = (WORD16)(sizeof(gaUserHmiConfigTable)/sizeof(T_HmiRegInfo));
    UINT32  i=0;

    printf("\n **********IPMI Statistic Info Begin**********\n");
    printf("\n Send Frame                     : %lu\n", ptHmiStatic->dwSndAllPacketCnt);
    printf("\n Send Frame Ok                  : %lu\n", ptHmiStatic->dwSndAllPacketOk);
    printf("\n Send Frame Err                 : %lu\n", ptHmiStatic->dwSndAllPacketErr);
    printf("\n Recv Frame                     : %lu\n", ptHmiStatic->dwRcvAllPacketCnt);
    printf("\n Recv Frame Ok                  : %lu\n", ptHmiStatic->dwRcvAllPacketOk);
    printf("\n Recv Frame Err                 : %lu\n", ptHmiStatic->dwRcvAllPacketErr);
    printf("\n Bbp  Alive Cnt                 : %lu\n", ptHmiStatic->dwAliveCnt[HMI_DEVICE_BBP]);
    printf("\n Ges  Alive Cnt                 : %lu\n", ptHmiStatic->dwAliveCnt[HMI_DEVICE_GES]);
    printf("\n Fan  Alive Cnt                 : %lu\n", ptHmiStatic->dwAliveCnt[HMI_DEVICE_FAN]);

    for (i=0; i<dwTableItemCounts; i++)
    {
        if ((TRUE == (gaUserHmiConfigTable + i)->isFlag) )
        {
            //(gaUserHmiConfigTable + dwThreadTypeIndex)->index = dwindex++;
            printf("\n NAME:%s               CMD:0x%lx\n",(gaUserHmiConfigTable + i)->pName,(gaUserHmiConfigTable + i)->dwCmd);
            printf("\n SEND        OK        ERR        TIMEOUT\n");
            printf("\n             %lu     %lu      %lu\n", ptHmiStatic->dwSndPacketOk[(gaUserHmiConfigTable + i)->index],ptHmiStatic->dwSndPacketErr[(gaUserHmiConfigTable + i)->index],ptHmiStatic->dwSndPacketTimeout[(gaUserHmiConfigTable + i)->index]);
            printf("\n RECV        OK        ERR        TIMEOUT\n");
            printf("\n             %lu 	 %lu      %lu\n", ptHmiStatic->dwRcvPacketOk[(gaUserHmiConfigTable + i)->index],ptHmiStatic->dwRcvPacketErr[(gaUserHmiConfigTable + i)->index],ptHmiStatic->dwRcvPacketTimeout[(gaUserHmiConfigTable + i)->index]);
        }
    }
    printf("\n **********IPMI Statistic Info End**********\n");

}


typedef struct tagT_HmiDirectIOParam
{
    UINT8     u8CHNo;          /*传输通道号 */
    UINT8     u8LocalI2cAddr;  /*本地i2c 地址 */
    UINT8     u8RemoteI2cAddr; /*远端i2c 地址 */
    UINT8     u8Type;
    UINT8     u8Prio;          /*发送优先级*/
    UINT8     u8RetryTime;     /*发送失败重试次数*/
    UINT32    dwIndex;         /*发送索引号*/
    UINT32    dwTimeout;       /*超时时间*/
    UINT32    u8IpmbCmd;
    void      *pAddr;
    UINT32    dwLen;           /* 数据包长度 */
} T_HmiDirectIOParam;

/**************************************************************************************************
*  函数名称: bsp_send_hmidata
*  描    述: IPMB消息发送函数
*  参    数: (1) u8Prio:消息优先级
*            (2) u8Type:消息类型
*            (3) index:消息索引
*  返    回: BSP_ERROR/BSP_OK
*  作    者:
*  日    期: 2015/12/6
**************************************************************************************************/
s32 bsp_send_hmidata(u8 u8Prio, u8 u8Type, u8 index)
{
    int len;
    STRU_SEND_MESSAGE *msg;
    u8 u8Addr;
	
    if(MCT_SLAVE == bsp_get_self_MS_state())
    {
    	return BSP_ERROR;
    }
	
    if(u8Type == IPMB_SEND_MESSAGE_REQUEST)
    {
        msg = &g_struQueueMsgBuf.StruRequestMessage[u8Prio][index];
    }
    if(u8Type == IPMB_SEND_MESSAGE_PROCESS)
    {
        msg = &g_struQueueMsgBuf.StruReplyMessage[u8Prio][index];
    }
    u8Addr = msg->u8Data[RSADDR_BYTE];
    if (msg->u8Used == 1)
    {
        if (BSP_OK != bsp_ipmb_send_message(IPMB_CHANNEL_IIC_3, u8Addr, msg->u8Data, msg->u32Len))
        {
            if(g_u32hmi_debug == 1)
                printf("send error reply or request  message\r\n");
            return BSP_ERROR;
        }
        if(u8Type == IPMB_SEND_MESSAGE_PROCESS)
            msg->u8Used = 0;
    }
    else
    {
        printf("msg.u8Used != 1, error...\n");
    }

    return BSP_OK;
}
/**************************************************************************************************
*  函数名称: BspHmiDirectTransfer
*  描    述: IPMB消息组包
*  参    数: (1) ptDirectIOParam:消息结构体
*  返    回:
*  作    者: 刘刚
*  日    期: 2015/12/6
**************************************************************************************************/
UINT32 BspHmiDirectTransfer(T_HmiDirectIOParam *ptDirectIOParam)
{
    UINT32        udCHNo,udDmaNo,udChnNo,udCount;
    UINT32        udCCSRDmaBaseAddr;
    static UINT32 dwSeqIndex   = 1;
    UINT8 ret=0;
    UINT8 u8Pri;
    UINT8 u8Type;
    UINT8 u8Index;
    UINT8 u8NetFn;
    UINT32 u8IpmbCmd;
    UINT32 dwLen = 0;
    UINT32 i=0;
    u8Pri     = ptDirectIOParam->u8Prio;
    u8Type    = ptDirectIOParam->u8Type;
    u8IpmbCmd = ptDirectIOParam->u8IpmbCmd;
    ret       = bsp_ipmb_get_queue_index(u8Type,u8Pri,&u8Index);
    if(ret != TRUE)
    {
        if(u8Type == IPMB_SEND_MESSAGE_PROCESS)
            printf("aaaaaaaaaaaaaaaabbb\r\n");
        if(u8Type == IPMB_SEND_MESSAGE_REQUEST)
            printf("bbbbbbbbbbbbbbbbaaa\r\n");
        return E_IPMB_SEND_FULL;
    }
    if (u8Type == IPMB_SEND_MESSAGE_REQUEST)
    {
        u8NetFn   = NETFN_IPMB_REQUEST;
        bsp_ipmb_create_request_header(ptDirectIOParam->u8RemoteI2cAddr,ptDirectIOParam->u8LocalI2cAddr,g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Data, u8NetFn, u8IpmbCmd);
        if (ptDirectIOParam->dwLen > 0)
        {
            memcpy(&g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Data[6],ptDirectIOParam->pAddr,ptDirectIOParam->dwLen);
            dwLen = 6 + ptDirectIOParam->dwLen;
        }
        else
        {
            dwLen = 6 ;
        }

        bsp_ipmb_check_sum(g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Data, dwLen);
        g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u32Len = dwLen+1;
        g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Seq = g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Data[SEQNO_BYTE];
        g_struQueueMsgBuf.StruRequestMessage[u8Pri][u8Index].u8Status = STATUS_WAIT_RESPOND;

    }
    else if (u8Type == IPMB_SEND_MESSAGE_PROCESS )
    {
        bsp_ipmb_create_response_header(g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Data, ptDirectIOParam->pAddr,ptDirectIOParam->u8RemoteI2cAddr);
        g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Data[COMPLETE_BYTE] = 0;
        //bsp_ipmb_check_sum(g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Data, 7);
        //g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Len = 8;
        if (ptDirectIOParam->dwLen > 0)
        {
            memcpy(&g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Data[7],(uint8_t *)(ptDirectIOParam->pAddr) + 6,ptDirectIOParam->dwLen);
            dwLen = 7 + ptDirectIOParam->dwLen;
        }
        else
        {
            dwLen = 7 ;
        }
        bsp_ipmb_check_sum(g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u8Data, dwLen);
        g_struQueueMsgBuf.StruReplyMessage[u8Pri][u8Index].u32Len = dwLen+1;
    }
    else
    {
        printf("send data type is error.\n");
        return E_IPMB_IOCTL_SEND_ERR;
    }
    //sem_post(&g_ipmb_semsend);
    bsp_send_hmidata(u8Pri, u8Type, u8Index);
    return E_IPMB_IOCTL_SEND_OK;
}


/**************************************************************************************************
*  函数名称: bsp_send_hmi_data
*  描    述:
*  参    数:
*  返    回:
*  作    者: 刘刚
*  日    期: 2015/12/6
**************************************************************************************************/
WORD32  bsp_send_hmi_data(UINT8 u8MessageType,  UINT8 u8Cmd,UINT8 u8Pri,UINT8  u8LocalI2cAddr,
                          UINT8 u8RemoteI2cAddr, UINT8  *pu8SrcData, UINT32  dwTransLen)
{
    UINT32                      dwRtnVal = 0;
    T_HmiDirectIOParam          tHmiDirectIOParam;
    tHmiDirectIOParam.u8CHNo          = 0;
    tHmiDirectIOParam.u8LocalI2cAddr  = u8LocalI2cAddr;
    tHmiDirectIOParam.u8RemoteI2cAddr = u8RemoteI2cAddr;
    tHmiDirectIOParam.u8Prio          = u8Pri;
    tHmiDirectIOParam.u8Type          = u8MessageType;
    tHmiDirectIOParam.dwTimeout       = 3;
    tHmiDirectIOParam.u8RetryTime     = 3;
    tHmiDirectIOParam.u8IpmbCmd       = u8Cmd;
    tHmiDirectIOParam.pAddr           = pu8SrcData;
    tHmiDirectIOParam.dwLen           = dwTransLen;
    /*去除头和尾部的长度*/
    if (dwTransLen <(IPMB_MESSAGE_DATA_LENGTH))
    {
        dwRtnVal = BspHmiDirectTransfer(&tHmiDirectIOParam);
    }
    return  dwRtnVal;

}


/**************************************************************************************************
*  函数名称: bsp_fan_control_init
*  描    述: 风扇控制初始化
*  参    数: none
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_fan_control_init(void)
{
    int res=0;
    pthread_t ptida;

    //创建风扇转速控制线程
    res = pthread_create(&ptida, NULL, (FUNCPTR)bsp_ipmb_fan_control_task,NULL);
    if (-1 == res)
    {
        perror("create bsp_ipmb_fan_control_task thread error!\n");
    }
}

/**************************************************************************************************
*  函数名称: fan_control_algorithm_2
*  描    述: 风扇转速控制算法实现
*  参    数: none
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
UINT8 fan_control_algorithm_2(f32 temp)
{
	static INT8 temp_prev = 100;
	static UINT8 pwm_val = 0;
  int ret;
    
	if (temp > (temp_prev+5)) 
	{
		if (temp < 30)
			pwm_val = 0;
		else if (temp >= 30 && temp <= 70)
			pwm_val = (UINT8)(100 * (0.025 * (float)temp - 0.75));
		else
			pwm_val = 100;
	} 
	else if(temp < (temp_prev-5)) 
	{
		if (temp < 25)
			pwm_val = 0;
		else if (temp >= 25 && temp <= 65)
			pwm_val = (UINT8)(100 * (0.025 * (float)temp - 0.625));
		else
			pwm_val = 100;
	}
	else 
	{ 
		return BSP_OK;
	}
	temp_prev = temp;
	printf("set fan speed...temp=%f, pwm=%d\n", temp, pwm_val);
  //通过HMI发送风扇转速设置命令
  ret = bsp_hmi_fan_speed(0, 3, pwm_val);
  if(ret == BSP_ERROR)
  {
  	temp_prev = 100;
  }  
	return ret;
}


/**************************************************************************************************
*  函数名称: bsp_ipmb_fan_control_task
*  描    述: 风扇转速控制
*  参    数: none
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_fan_control_task(void)
{
    float f32MdtTemp;
    float f32SubboardTemp;
    float f32Temperature;
    s32   s32Ret;
    int slot = 0;
    while(1)
    {
        sleep(30);
        //获取主控板温度
        f32MdtTemp = bsp_get_temperature();
        //获取单板最大温度值
        f32SubboardTemp = 0.0;
        for(slot=0; slot<MAX_BOARDS_NUMBER-1; slot++)
        {    
            if(f32SubboardTemp < boards[slot].temperature)
                f32SubboardTemp = boards[slot].temperature;
        }
        f32Temperature = (f32MdtTemp > f32SubboardTemp)? f32MdtTemp : f32SubboardTemp;
        if(fan_control_debug)
        {
            printf("主控板温度=%f,单板最大温度=%f,最大温度=%f\r\n",
                   f32MdtTemp,f32SubboardTemp,f32Temperature);
        }
        //根据温度控制风扇转速
        if(MCT_MASTER == bsp_get_self_MS_state())
        	s32Ret = fan_control_algorithm_2(f32Temperature);
    }
}
/**************************************************************************************************
*  函数名称: bsp_hmi_mcu_update
*  描    述: 单板升级
*  参    数: (1)u8boardtype: 板类型选择(0基带板,1交换板,2风扇)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_mcu_update(UINT8 u8boardtype,UINT8 u8slot)
{
    WORD32 u8Ret;
    u8 cmd = 0;

    if((u8slot < 2) || (u8slot > 10))
    {
        printf("[%s]:error slot.\n", __func__);
        return BSP_ERROR;
    }

   cmd = IPMB_CMD_ID_UPDATE;

   return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_boards_reboot
*  描    述: 复位所有单板
*  参    数: 无
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_reset_subboard(UINT8 u8slot)
{
    WORD32 u8Ret;
    u8 cmd = 0;
    u8 u8boardtype = BOARD_TYPE_NULL;
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;
    cmd = IPMB_CMD_COMM_RESET_BOARDS;

    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_check_alive
*  描    述: 查询所有在位单板
*  参    数: 无
*  返    回: TRUE/FALSE.
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_check_alive(void)
{
    u8 slotid;
    u8 cmd = IPMB_CMD_COMM_CHECK_ALIVE;
    u8 u8boardtype = BOARD_TYPE_ALL;
    for(slotid=0; slotid<MAX_BOARDS_NUMBER; slotid++)
        boards[slotid].mcu_alive = 0;
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, 0);
}

/**************************************************************************************************
*  函数名称: bsp_hmi_start_send_message
*  描    述: 启动消息发送
*  参    数: 无
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_start_send_message(void)
{
    u8 cmd = IPMB_CMD_COMM_START_SEND_MESSAGE;
    u8 u8boardtype = BOARD_TYPE_ALL;
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, 0);
}

SINT8 bsp_hmi_start_send_message_byslot(UINT8 u8slot)
{
    u8 cmd = 0;
    u8 u8boardtype = BOARD_TYPE_NULL;
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;
    cmd = IPMB_CMD_COMM_START_SEND_MESSAGE;
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_stop_send_message
*  描    述: 启动消息发送
*  参    数: 无
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_stop_send_message(void)
{
    u8 cmd = IPMB_CMD_COMM_STOP_SEND_MESSAGE;
    u8 u8boardtype = BOARD_TYPE_ALL;
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, 0);
}
/*************************************************************************************************
*  函数名称: bsp_hmi_board_reboot
*  描    述: 单板复位
*  参    数: (1)u8boardtype: 板类型选择(2基带板,3监控板,1风扇板)
*            (2)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_board_reboot(UINT8 u8boardtype,UINT8 u8slot)
{
    WORD32 u8Ret;
    u8 cmd = 0;
	
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;
    memset(&boards[u8slot], 0, sizeof(board_info));
    if (BOARD_TYPE_BBP == u8boardtype)
    {
        cmd = IPMB_CMD_BASEBANDCTL_REBOOT;
    }
    else if(BOARD_TYPE_FSA == u8boardtype)
    {
        cmd = IPMB_CMD_FSA_REBOOT;
    }
    else if(BOARD_TYPE_ES == u8boardtype)
    {
        cmd = IPMB_CMD_ES_REBOOT;
    }
    else if(BOARD_TYPE_FAN == u8boardtype)
    {
        cmd = IPMB_CMD_FANCTL_REBOOT;
    }
    else if(BOARD_TYPE_PEU == u8boardtype)
    {
        cmd = IPMB_CMD_PEUACTL_REBOOT;
    }
    else
    {
        return BSP_ERROR;
    }

    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}

/**************************************************************************************************
*  函数名称: bsp_hmi_fan_speed
*  描    述: 风扇转速设置/获取
*  参    数: (1)u8dwSel: 0设置转速,1获取转速
*            (2)u8FanChannel:通道选择(0-3)
*            (3)u8FanPWMVal:风扇转速(0-100)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_speed(UINT8 u8dwSel,UINT8 u8FanChannel,UINT8 u8FanPWMVal)
{
    UINT8 u8Ret;
    u8 cmd = 0;
    if(u8FanChannel > 3)
    {
        printf("Fan channel is overstep the boundary...\r\n");
        return BSP_ERROR;
    }

    if(u8FanPWMVal > 100)
    {
        printf("Fan PWM value is overstep the boundary...\r\n");
        return BSP_ERROR;
    }
    
    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_FANCTL_SET_SPEED;
		g_fan_set_state = BSP_ERROR;
    }
    else if(1 == u8dwSel)
    {
        u8FanPWMVal = 0;
        g_fan1_speed = 0;
        g_fan2_speed = 0;
        g_fan3_speed = 0;
        cmd = IPMB_CMD_FANCTL_GET_SPEED;
    }
    else
    {
        return BSP_ERROR;	
    }

	u8Ret = bsp_ipmb_ioctl(cmd, u8FanChannel, u8FanPWMVal,IPMB_SLOT10);
	
	if (0 == u8dwSel) 
    {
        sleep(1);
        return g_fan_set_state;
    }
    return u8Ret;
}
/**************************************************************************************************
*  函数名称: bsp_hmi_get_temperature
*  描    述: 温度获取
*  参    数: (1)u8boardtype: 板类型选择(2基带板,3监控板)
*            (2)u8slot:槽位号

*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_get_temperature(UINT8 u8boardtype,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
    if((u8slot<2) || (u8slot>9))
    	return BSP_ERROR;
		
    if (BOARD_TYPE_BBP == u8boardtype)
    {
        memset(g_s8BbpGetTemperature, 0, 8);
        cmd = IPMB_CMD_BASEBANDCTL_GET_TEMP;
    }
    else if(BOARD_TYPE_FSA == u8boardtype)
    {
        memset(g_s8FsaGetTemperature, 0, 4);
        cmd = IPMB_CMD_FSA_GET_TEMP;
    }
    else if(BOARD_TYPE_ES == u8boardtype)
    {
        g_s16EsTemp = 0;
        cmd = IPMB_CMD_ES_GET_TEMP;
    }
    else if(BOARD_TYPE_PEU == u8boardtype)
    {
        g_s16PEUTemp = 0;
        cmd = IPMB_CMD_PEUACTL_GET_TEMP;
    }
    else
    {
        return BSP_ERROR;
    }

    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}

/**************************************************************************************************
*  函数名称: bsp_hmi_write_fpga_epld
*  描    述: 写fpga/cpld寄存器
*  参    数: (1)u8dwSel: 写片选(0 FPGA,1 CPLD)
*            (2)dwReg:寄存器地址
*            (3)dwVal:寄存器值
*            (4)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
UINT32 bsp_hmi_write_fpga_epld(UINT32 u8dwSel,UINT32 dwReg,UINT32 dwVal,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
    if((u8slot<2) || (u8slot>7))
    	return BSP_ERROR;

    if (0 == u8dwSel)
        cmd = IPMB_CMD_BASEBANDCTL_WRITE_FPGA;
    else if(1 == u8dwSel)
        cmd = IPMB_CMD_BASEBANDCTL_WRITE_EPLD;
    else
        return BSP_ERROR;
    return bsp_ipmb_ioctl(cmd, dwReg, dwVal,u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_read_fpga_epld
*  描    述: 读fpga/cpld寄存器
*  参    数: (1)u8dwSel: 读片选(0 FPGA,1 CPLD)
*            (2)dwReg:寄存器地址
*            (3)dwVal:寄存器值
*            (4)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
UINT32 bsp_hmi_read_fpga_epld(UINT32 u8dwSel,UINT32 dwReg,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
	if((u8slot<2) || (u8slot>7))
		return BSP_ERROR;
		
    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_BASEBANDCTL_READ_FPGA;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_BASEBANDCTL_READ_EPLD;
    }
    else
    {
        return BSP_ERROR;
    }

    return bsp_ipmb_ioctl(cmd, dwReg, 0,u8slot);
}

/**************************************************************************************************
*  函数名称: bsp_hmi_packets_statistics
*  描    述: 获取统计值
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,1风扇板)
*            (2)u8packstat:包统计类型(0发送成功,1发送失败,2接收成功,3接收失败)
*            (3)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_packets_statistics(UINT8 u8boardtype, UINT8 u8packstat,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;
		
    /* 获取命令号 */
    if (BOARD_TYPE_BBP == u8boardtype)
    {
        if(HMI_SND_PACK_OK == u8packstat)
        {
            cmd = IPMB_CMD_BASEBANDCTL_SEND_PACK_OK_CNT;
        }
        else if(HMI_SND_PACK_ERR == u8packstat)
        {
            cmd = IPMB_CMD_BASEBANDCTL_SEND_PACK_ERR_CNT;
        }
        else if(HMI_RCV_PACK_OK == u8packstat)
        {
            cmd = IPMB_CMD_BASEBANDCTL_RECV_PACK_OK_CNT;
        }
        else if(HMI_RCV_PACK_ERR == u8packstat)
        {
            cmd = IPMB_CMD_BASEBANDCTL_RECV_PACK_ERR_CNT;
        }
        else
            return BSP_ERROR;
    }
    else if (BOARD_TYPE_GES == u8boardtype)
    {
        if(HMI_SND_PACK_OK == u8packstat)
        {
            cmd = IPMB_CMD_SWITCHCTL_SEND_PACK_OK_CNT;
        }
        else if(HMI_SND_PACK_ERR == u8packstat)
        {
            cmd = IPMB_CMD_SWITCHCTL_SEND_PACK_ERR_CNT;
        }
        else if(HMI_RCV_PACK_OK == u8packstat)
        {
            cmd = IPMB_CMD_SWITCHCTL_RECV_PACK_OK_CNT;
        }
        else if(HMI_RCV_PACK_ERR == u8packstat)
        {
            cmd = IPMB_CMD_SWITCHCTL_RECV_PACK_ERR_CNT;
        }
        else
            return BSP_ERROR;
    }

    else if (BOARD_TYPE_FAN == u8boardtype)
    {
        if(HMI_SND_PACK_OK == u8packstat)
        {
            cmd = IPMB_CMD_FANCTL_SEND_PACK_OK_CNT;
        }
        else if(HMI_SND_PACK_ERR == u8packstat)
        {
            cmd = IPMB_CMD_FANCTL_SEND_PACK_ERR_CNT;
        }
        else if(HMI_RCV_PACK_OK == u8packstat)
        {
            cmd = IPMB_CMD_FANCTL_RECV_PACK_OK_CNT;
        }
        else if(HMI_RCV_PACK_ERR == u8packstat)
        {
            cmd = IPMB_CMD_FANCTL_RECV_PACK_ERR_CNT;
        }
        else
            return BSP_ERROR;
    }

    else if (BOARD_TYPE_PEU == u8boardtype)
    {

        if(HMI_SND_PACK_OK == u8packstat)
        {
            cmd = IPMB_CMD_PEUACTL_SEND_PACK_OK_CNT;
        }
        else if(HMI_SND_PACK_ERR == u8packstat)
        {
            cmd = IPMB_CMD_PEUACTL_SEND_PACK_ERR_CNT;
        }
        else if(HMI_RCV_PACK_OK == u8packstat)
        {
            cmd = IPMB_CMD_PEUACTL_RECV_PACK_OK_CNT;
        }
        else if(HMI_RCV_PACK_ERR == u8packstat)
        {
            cmd = IPMB_CMD_PEUACTL_RECV_PACK_ERR_CNT;
        }
        else
            return BSP_ERROR;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0,u8slot);;
}
/**************************************************************************************************
*  函数名称: bsp_hmi_packets_statistics
*  描    述: 上下电控制
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8PowerOn:上下电(0下电,1上电)
*            (3)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_board_power_on_off(UINT8 u8boardtype, UINT8 u8PowerOn,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;

    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;
		
    if (BOARD_TYPE_BBP == u8boardtype)
    {
        if(1 == u8PowerOn)
        {
            cmd = IPMB_CMD_BASEBANDCTL_POWER_ON;
        }
        else if(0 == u8PowerOn)
        {
            cmd = IPMB_CMD_BASEBANDCTL_POWER_OFF;
        }
        else
            return BSP_ERROR;
    }
    else if (BOARD_TYPE_FSA == u8boardtype)
    {
        if(1 == u8PowerOn)
        {
            cmd = IPMB_CMD_FSA_POWER_ON;
        }
        else if(0 == u8PowerOn)
        {
            cmd = IPMB_CMD_FSA_POWER_OFF;
        }
        else
            return BSP_ERROR;
    }
    else if (BOARD_TYPE_FAN == u8boardtype)
    {
        if(1 == u8PowerOn)
        {
            cmd = IPMB_CMD_FANCTL_POWER_ON;
        }
        else if(0 == u8PowerOn)
        {
            cmd = IPMB_CMD_FANCTL_POWER_OFF;
        }
        else
            return BSP_ERROR;
    }
    else if (BOARD_TYPE_PEU == u8boardtype)
    {
        if(1 == u8PowerOn)
        {
            cmd = IPMB_CMD_PEUACTL_POWER_ON;
        }
        else if(0 == u8PowerOn)
        {
            cmd = IPMB_CMD_PEUACTL_POWER_OFF;
        }
        else
            return BSP_ERROR;
    }
    else
    {
        return BSP_ERROR;
    }

    return bsp_ipmb_ioctl(cmd, u8boardtype, 0,u8slot);;
}

/**************************************************************************************************
*  函数名称: bsp_hmi_get_arm_ver
*  描    述: 获取版本号
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_get_arm_ver(UINT8 u8boardtype,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;

    if (BOARD_TYPE_BBP == u8boardtype)
    {
        cmd = IPMB_CMD_BASEBANDCTL_GET_ARM_VER_VAL;
    }
    else if (BOARD_TYPE_FSA == u8boardtype)
    {
        cmd = IPMB_CMD_FSA_GET_ARM_VER_VAL;
    }
    else if (BOARD_TYPE_ES == u8boardtype)
    {
        cmd = IPMB_CMD_ES_GET_ARM_VER_VAL;
    }
    else if (BOARD_TYPE_FAN == u8boardtype)
    {
        cmd = IPMB_CMD_FANCTL_GET_ARM_VER_VAL;
    }
    else if (BOARD_TYPE_PEU == u8boardtype)
    {
        cmd = IPMB_CMD_PEUACTL_GET_ARM_VER_VAL;
    }
    else
    {
        return BSP_ERROR;
    }

    return bsp_ipmb_ioctl(cmd, u8boardtype, 0,u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_set_heart_beat_gap
*  描    述: 心跳上报时间间隔配置
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8TimeGap:时间间隔
*            (3)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_set_heart_beat_gap(UINT8 u8boardtype, UINT8 u8TimeGap,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;

    if (BOARD_TYPE_BBP == u8boardtype)
    {
        cmd = IPMB_CMD_BASEBANDCTL_HEART_BEAT_GAP;
    }
    else if (BOARD_TYPE_FSA == u8boardtype)
    {
        cmd = IPMB_CMD_FSA_HEART_BEAT_GAP;
    }
    else if (BOARD_TYPE_ES == u8boardtype)
    {
        cmd = IPMB_CMD_ES_HEART_BEAT_GAP;
    }
    else if (BOARD_TYPE_FAN == u8boardtype)
    {
        cmd = IPMB_CMD_FANCTL_HEART_BEAT_GAP;
    }
    else if (BOARD_TYPE_PEU == u8boardtype)
    {
        cmd = IPMB_CMD_PEUACTL_HEART_BEAT_GAP;
    }
    else
    {
        return BSP_ERROR;
    }

    return bsp_ipmb_ioctl(cmd, u8boardtype, u8TimeGap, u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_get_board_slot
*  描    述: 获取单板槽位号
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_get_board_slot(UINT8 u8boardtype,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
    if((u8slot<7) || (u8slot>10))
    	return BSP_ERROR;

    if (BOARD_TYPE_FAN == u8boardtype)
    {
        cmd = IPMB_CMD_FANCTL_GET_SLOT;
    }
    else if (BOARD_TYPE_PEU == u8boardtype)
    {
        cmd = IPMB_CMD_PEUACTL_GET_SLOT;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}

/**************************************************************************************************
*  函数名称: bsp_hmi_get_board_type
*  描    述: 获取单板类型
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_get_board_type(UINT8 u8boardtype,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
	if((u8slot<7) || (u8slot>10))
		return BSP_ERROR;

    if (BOARD_TYPE_FAN == u8boardtype)
    {
        cmd = IPMB_CMD_FANCTL_GET_BOARD_TYPE;
    }
    else if (BOARD_TYPE_PEU == u8boardtype)
    {
        cmd = IPMB_CMD_PEUACTL_GET_BOARD_TYPE;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_get_pcb_version
*  描    述: 获取单板PCB版本
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_get_pcb_version(UINT8 u8boardtype,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
	if((u8slot<8) || (u8slot>10))
		return BSP_ERROR;
		
    if (BOARD_TYPE_FAN == u8boardtype)
    {
        cmd = IPMB_CMD_FANCTL_GET_PCB_VERSION;
    }
    else if (BOARD_TYPE_PEU == u8boardtype)
    {
        cmd = IPMB_CMD_PEUACTL_GET_PCB_VERSION;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_get_dryin_state
*  描    述: 获取监控板干结点状态
*  参    数: (1)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_get_dryin_state(UINT8 u8slot)
{
    UINT8 u8Ret;
	
    if((u8slot<8) || (u8slot>9))
    	return BSP_ERROR;
	
    u8Ret = bsp_ipmb_ioctl(IPMB_CMD_PEUACTL_GET_DRYIN_STATE, BOARD_TYPE_PEU, 0,u8slot);
    if(u8Ret != E_IPMB_IOCTL_SEND_OK)
        return u8Ret;
}
/**************************************************************************************************
*  函数名称: bsp_hmi_rs485_test
*  描    述: 监控板rs485链路测试
*  参    数: (1)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_rs485_test(UINT8 u8slot)
{
    UINT8 u8Ret;
	
    if((u8slot<8) || (u8slot>9))
    	return BSP_ERROR;
	
    g_peu_rs485_test = 0;
    u8Ret = bsp_ipmb_ioctl(IPMB_CMD_PEUACTL_RS485_TEST, BOARD_TYPE_PEU, 0,u8slot);
    if(u8Ret != E_IPMB_IOCTL_SEND_OK)
        return u8Ret;
}

/**************************************************************************************************
*  函数名称: bsp_hmi_board_test
*  描    述: HMI链路测试
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_board_test(UINT8 u8boardtype,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;

    if (BOARD_TYPE_BBP == u8boardtype)
    {
        cmd = IPMB_CMD_BASEBANDCTL_TEST;
    }
    else if (BOARD_TYPE_FSA == u8boardtype)
    {
        cmd = IPMB_CMD_FSA_TEST;
    }
    else if (BOARD_TYPE_ES == u8boardtype)
    {
        cmd = IPMB_CMD_ES_TEST;
    }
    else if (BOARD_TYPE_FAN == u8boardtype)
    {
        cmd = IPMB_CMD_FANCTL_TEST;
    }
    else if (BOARD_TYPE_PEU == u8boardtype)
    {
        cmd = IPMB_CMD_PEUACTL_TEST;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, u8slot);
}

/**************************************************************************************************
*  函数名称: bsp_hmi_test_eeprom
*  描    述: EEPROM测试
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_test_eeprom(UINT8 u8boardtype,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
    
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;
		
    if (BOARD_TYPE_BBP == u8boardtype)
    {
        cmd = IPMB_CMD_BASEBANDCTL_TEST_EEPROM;
    }
    else if(BOARD_TYPE_FSA == u8boardtype)
    {
        cmd = IPMB_CMD_FSA_TEST_EEPROM;
    }
    else if(BOARD_TYPE_ES == u8boardtype)
    {
        cmd = IPMB_CMD_ES_TEST_EEPROM;
    }
    else if (BOARD_TYPE_FAN == u8boardtype)
    {
        cmd = IPMB_CMD_FANCTL_TEST_EEPROM;
    }
    else
    {
        return BSP_ERROR;
    }

    return bsp_ipmb_ioctl(cmd, u8boardtype, 0,u8slot);
}

/**************************************************************************************************
*  函数名称: bsp_hmi_test_geswitch
*  描    述: SWITCH路由配置测试
*  参    数: (1)u8boardtype: 板类型(2基带板,3监控板,5交换板，1风扇板)
*            (2)u8slot:槽位号
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_test_geswitch(UINT8 u8boardtype,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
   
    if((u8slot<2) || (u8slot>7))
    	return BSP_ERROR;

    if (BOARD_TYPE_BBP == u8boardtype)
    {
        cmd = IPMB_CMD_BASEBANDCTL_TEST_GESWITCH;
    }
    else if(BOARD_TYPE_FSA == u8boardtype)
    {
        cmd = IPMB_CMD_FSA_TEST_GESWITCH;
    }
    else if(BOARD_TYPE_ES == u8boardtype)
    {
        cmd = IPMB_CMD_ES_TEST_GESWITCH;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0,u8slot);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_fan_eeprom_crc
*  描    述: 风扇板EEPROM参数配置/读取接口
*  参    数: (1)u8dwSel: (u8dwSel:0设置，1读取)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_eeprom_crc(UINT8 u8dwSel)
{
    UINT8 u8Ret;
    u8 cmd;

    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_SET_CRC;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_GET_CRC;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, BOARD_TYPE_FAN, 0,IPMB_SLOT10);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_fan_eeprom_device_id
*  描    述: 风扇板EEPROM参数配置/读取接口
*  参    数: (1)u8dwSel: (u8dwSel:0设置，1读取)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_eeprom_device_id(UINT8 u8dwSel)
{
    UINT8 u8Ret;
    u8 cmd;

    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_SET_DEVICE_ID;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_GET_DEVICE_ID;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, BOARD_TYPE_FAN, 0,IPMB_SLOT10);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_fan_eeprom_board_type
*  描    述: 风扇板EEPROM参数配置/读取接口
*  参    数: (1)u8dwSel: (u8dwSel:0设置，1读取)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_eeprom_board_type(UINT8 u8dwSel)
{
    UINT8 u8Ret;
    u8 cmd;

    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_SET_BOARD_TYPE;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_GET_BOARD_TYPE;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, BOARD_TYPE_FAN, 0,IPMB_SLOT10);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_fan_eeprom_product_sn
*  描    述: 风扇板EEPROM参数配置/读取接口
*  参    数: (1)u8dwSel: (u8dwSel:0设置，1读取)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_eeprom_product_sn(UINT8 u8dwSel)
{
    UINT8 u8Ret;
    u8 cmd;

    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_SET_PRODUCT_SN;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_GET_PRODUCT_SN;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, BOARD_TYPE_FAN, 0,IPMB_SLOT10);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_fan_eeprom_manufacture
*  描    述: 风扇板EEPROM参数配置/读取接口
*  参    数: (1)u8dwSel: (u8dwSel:0设置，1读取)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_eeprom_manufacture(UINT8 u8dwSel)
{
    UINT8 u8Ret;
    u8 cmd;

    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_SET_MANUFACTURER;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_GET_MANUFACTURER;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, BOARD_TYPE_FAN, 0,IPMB_SLOT10);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_fan_eeprom_product_date
*  描    述: 风扇板EEPROM参数配置/读取接口
*  参    数: (1)u8dwSel: (u8dwSel:0设置，1读取)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_eeprom_product_date(UINT8 u8dwSel)
{
    UINT8 u8Ret;
    u8 cmd;

    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_SET_PRODUCT_DATE;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_GET_PRODUCT_DATE;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, BOARD_TYPE_FAN, 0,IPMB_SLOT10);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_fan_eeprom_temp_threshold
*  描    述: 风扇板EEPROM参数配置/读取接口
*  参    数: (1)u8dwSel: (u8dwSel:0设置，1读取)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_eeprom_temp_threshold(UINT8 u8dwSel)
{
    UINT8 u8Ret;
    u8 cmd;

    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_SET_TEMP_THRESHOLD;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_GET_TEMP_THRESHOLD;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, BOARD_TYPE_FAN, 0,IPMB_SLOT10);
}
/**************************************************************************************************
*  函数名称: bsp_hmi_fan_eeprom_initial_speed
*  描    述: 风扇板EEPROM参数配置/读取接口
*  参    数: (1)u8dwSel: (u8dwSel:0设置，1读取)
*  返    回: TRUE/FALSE.
*  作    者:
*  日    期: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_fan_eeprom_initial_speed(UINT8 u8dwSel)
{
    UINT8 u8Ret;
    u8 cmd;

    if (0 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_SET_FAN_INITIALSPEED;
    }
    else if(1 == u8dwSel)
    {
        cmd = IPMB_CMD_EEPROM_GET_FAN_INITIALSPEED;
    }
    else
    {
        return BSP_ERROR;
    }
    return bsp_ipmb_ioctl(cmd, BOARD_TYPE_FAN, 0,IPMB_SLOT10);
}

