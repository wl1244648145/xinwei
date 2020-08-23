/*******************************************************************************
* *  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* �ļ�����:  hmi.c
* ��    ��:
* ��    ��:  V0.1
* ��д����:  2015/06/26
* ˵    ��:  ��
* �޸���ʷ:
* �޸�����           �޸��� liuganga     �޸�����
*------------------------------------------------------------------------------
/******************************* �����ļ����� *********************************/
/**************************** ����ͷ�ļ�************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <pub.h>
/**************************** ˽��ͷ�ļ�************************************/
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

/* ������Ϣ���鶨�� */
STRU_RECV_MESSAGE g_struReceiveMsgBuf[IPMB_RECEIVE_BUF_NUM] = {0};
/*��������������鶨��*/
STRU_QUEUE_SEND       g_struQueueMsgBuf;

HMI_WRITE_FPGA_REG   g_struWriteFpgaQueueBuf[WRITE_FPGA_BUF_NUM]= {0};
HMI_READ_FPGA_REG    g_struReadFpgaQueueBuf[READ_FPGA_BUF_NUM]= {0};

HMI_WRITE_EPLD_REG   g_struWriteEpldQueueBuf[WRITE_EPLD_BUF_NUM]= {0};
HMI_READ_EPLD_REG    g_struReadEpldQueueBuf[READ_EPLD_BUF_NUM]= {0};
extern u8 *g_u8ccsbar;
/******************************* �ֲ��궨�� ***********************************/
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
/* HMI���ձ��� */
UINT8 g_u8ReceiveMsgIndex   = 0;
/* HMI���Դ�ӡ���� */
UINT32 g_hmi_heartbeat_debug = 0;
UINT32 g_hmi_response_debug = 0;
UINT32 g_u32hmi_debug = 0;
UINT32 fan_control_debug = 0;
/* HMI���Ȱ������ر��� */
uint8_t g_fan_speed_set = 0;
uint8_t g_fan_speed_get = 0;
uint8_t g_fan_eeprom_test = 0;
uint8_t g_fan_set_eeprom = 0;
uint8_t g_fan_get_eeprom = 0;
/* HMI��ذ������ر��� */
uint8_t g_peu_power_down_flag =0;
uint8_t g_peu_get_temp = 0;
uint8_t g_peu_get_dryin = 0;
uint8_t g_peu_rs485_test = 0;
uint8_t g_peu_dryin0123 = 0;
uint8_t g_peu_dryin4567 = 0;
/* HMI��·������ر��� */
uint8_t g_peu_hmi_test = 0;
uint8_t g_fan_hmi_test = 0;
uint8_t g_ges_hmi_test = 0;
uint8_t g_bbp_hmi_test = 0;

#define MAX_UPDATE_TIMES 3

/* ���Ȱ�EEPROM�������� */
EEPROM_PAR_STRU g_fan_eeprom_set_par =
{
    {0},//CRC
    "C6482",//DEVICE ID
    "C6482_FM.00.00",//�忨����
    {0},//MCA��ַ1
    {0},//MCA��ַ2
    "11-22-33-44-55",//PRODUCTSN
    "BJXINWEI",//MANUFACTURE
    {0x05,0x10,0x07,0xe0},//PRODUCTDATE
    {0},//���ջ��ͺ�
    {0x0f,0xa0},//���ȳ���ת��
    {85,-10}//�¶ȷ�Χ
};
EEPROM_PAR_STRU g_fan_eeprom_get_par = {0};

/* IPMB�ļ����ر��� */
UINT32 ipmb_filesize = 0;
int ipmb_filefd = 0;
struct stat ipmb_st;
pthread_mutex_t ipmbfile_wait;
UINT16 ipmb_pkg_id = 0;
UINT16 ipmb_pkg_len = 0;
/*********************liugang modify*****************************/
/*********************** ȫ�ֱ�������/��ʼ�� *************************/
SINT8 g_s8BbpGetTemperature[8] = {0};
SINT8 g_s8FsaGetTemperature[4] = {0};
SINT16 g_s16EsTemp = 0;
SINT16 g_s16PEUTemp = 0;

/***************************** �������� ******************************/
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
*  ��������: bsp_ipmb_get_queue_index
*  ��    ��: ��ȡδ��ʹ�õķ�����������ֵ
*  ��    ��:(1)u8MessType: ��Ϣ����
*           (2)u8Pri: ��Ϣ���ȼ�
*           (3)pu8Index: index number
*  ��    ��: ERR/OK.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_get_queue_index(UINT8 u8MessType,UINT8 u8Pri,UINT8 *pu8Index)
{
    UINT8 ret;
    UINT8 u8Err;
    UINT8 u8Index;
    /* ���ҿ�������Ԫ�� */
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
        /* ��� */
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
        /* ��� */
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
*  ��������: bsp_get_i2caddr_byboardtype
*  ��    ��: ��ȡremote i2c addr
*  ��    ��: (1)type: board typr
* 		   (2)slot : ��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_get_slot_byi2caddr
*  ��    ��: ��ȡslot
*  ��    ��: (1)addr: i2caddr
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
u8 bsp_get_slot_byi2caddr(u8 i2caddr)
{
    u8 slot = 0;

    slot = i2caddr - IPMB_MCT_HMI1_I2C_ADDR_BASE;
    return slot;
}
/**************************************************************************************************
*  ��������: bsp_read_eeprom_buf
*  ��    ��: ��ȡeeprom���ò���
*  ��    ��: (1)type: request command
* 		   (2)buf : ����
*            (3)len : ����
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_write_eepromget_buf
*  ��    ��: ����eeprom��ȡ����
*  ��    ��: (1)type: response command
*            (2)buf : ����
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_ipmb_ioctl
*  ��    ��: ����������Ϣ
*  ��    ��: (1)u32Cmd: request command
* 		   (2)u32Arg1 : board type
*            (3)u32Arg2 : ����
*            (4)u32Arg3 : ��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_ioctl(UINT32 u8IpmbCmd, UINT32 u32Arg1, UINT32 u32Arg2,UINT32 u32Arg3)
{
    /* �ֲ��������� */
    UINT8 u8MsgBuf[IPMB_MESSAGE_MAX_LENGTH];    /* ����IPMB������Ϣ������ */
    UINT8 u8MsgLength = 0;                      /* IPMB������Ϣ��ʵ�ʳ��� */

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
*  ��������: bsp_ipmb_check_sum
*  ��    ��: ����IPMB��Ϣ���ĵ�У���
*  ��    ��: (1)pu8Buf: pointer of message buffer.
*            (2)u8Length: length of message.
*  ��    ��: ��
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_ipmb_message_check
*  ��    ��: У��IPMB��Ϣ����
*  ��    ��: (1)pu8Buf: pointer of message buffer.
*            (2)u8Length: length of message.
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_message_check(UINT8 *pu8Buf, UINT32 u32Length)
{
    UINT8 ret;
    UINT32 i;
    UINT16 sum = 0;
    /* ǰ�����ֽڵļ��� */
    for(i=0; i<3; i++)
    {
        sum+=pu8Buf[i];
    }
    if((sum & 0xff) != 0)
    {
        ret = FALSE;
        return ret;
    }
    /* ������Ϣ�ļ��� */
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
*  ��������: bsp_ipmb_create_request_header
*  ��    ��: ����������Ϣ��ͷ
*  ��    ��: (1)u8RemoteI2cAddr: choose i2c channel.
*            (2)u8LocalI2cAddr: transfer rate of i2c, Tr-WorkFreq = Fpclk/(SCLH + SCLL).
*            (3)pu8IpmbMsg: master mode/slave mode
*            (4)u8NetFn:
*            (5)u8IpmbCmd:
*  ��    ��: ��.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_create_request_header(UINT8 u8RemoteI2cAddr,UINT8 u8LocalI2cAddr,UINT8 *pu8IpmbMsg, UINT8 u8NetFn, UINT8 u8IpmbCmd)
{
    static UINT32 dwSeqNo   = 1;    /* ������Ϣ�����к� */
    pu8IpmbMsg[RSADDR_BYTE] = u8RemoteI2cAddr;
    pu8IpmbMsg[NETFN_BYTE]  =  u8NetFn;
    bsp_ipmb_check_sum(pu8IpmbMsg, 2);
    pu8IpmbMsg[RQADDR_BYTE] = u8LocalI2cAddr;
    pu8IpmbMsg[SEQNO_BYTE]  = (dwSeqNo << 2);
    pu8IpmbMsg[CMD_BYTE]    = u8IpmbCmd;
    dwSeqNo++;
}

/**************************************************************************************************
*  ��������: bsp_ipmb_create_response_header
*  ��    ��: ����Ӧ����Ϣ��ͷ
*  ��    ��: (1)pu8RspBuf: the pointer of response buffer.
*            (2)pu8RecvBuf: the pointer of receive buffer.
*  ��    ��: ��.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_ipmb_send_message
*  ��    ��: ipmb������Ϣ����
*  ��    ��: (1)u8Channel: choose i2c channel.
*            (2)u8I2cAddress: slave i2c address.
*            (3)pu8Buf: pointer of send buffer.
*            (4)u8Length: length of send data
*  ��    ��: ��.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
uint32_t test_smsg_num = 0;
SINT8 bsp_ipmb_send_message(UINT8 u8Channel,UINT8 u8I2cAddress, UINT8 *pu8Buf, UINT32 u32Length)
{
    UINT8 ret;
    UINT8 u8Err;
    UINT8 u8Cnt;
    UINT32 i=0;
    UINT8 u8Index;

    /* ����I2C������������ */
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
*  ��������: bsp_ipmb_process_request
*  ��    ��: ����������Ϣ����
*  ��    ��: (1)pu8Buf: pointer of send buffer.
*            (2)u8Length: length of send buffer.
*  ��    ��: ��.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
uint32_t ipmb_tftp_dbg_flag = 0;
UINT8 bsp_ipmb_process_request(UINT8 *pu8Buf, UINT8 u8Length)
{
    /*�ֲ�����*/
    UINT8 u8IpmbCmd;                           /* ����� */
    UINT8 u8SeqNum;
    UINT8 u8RspBuf[IPMB_MESSAGE_MAX_LENGTH];   /* Ӧ����Ϣ���� */
    UINT8 u8RspLength = 0;                     /* Ӧ����Ϣ���� */
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

    /* ȡ������� */
    u8IpmbCmd = pu8Buf[CMD_BYTE];
    /* ȡ�����к� */
    u8SeqNum = pu8Buf[SEQNO_BYTE];
    u8SrcAddr = pu8Buf[RQADDR_BYTE];
    /* ȡ����λ�� */
    u8Slot = pu8Buf[6];
    /* ȡ�������� */
    u8BoardType = pu8Buf[7];
    /* ȡ��header */
    memcpy(u8RspBuf, pu8Buf, 6);

    /* ����first message��heartbeat��Ϣ */
    if((u8Slot < 2) || (u8Slot > 10))
    {
        printf("[%s] erro slot:%d\r\n", __func__, u8Slot);
		return 0;
    }

    /* first msg��Ϣ���� */
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
            //��ȡ�汾��
            memcpy(boards[u8Slot].arm_version, (s8 *)&pu8Buf[8], 64);
        }
        u8RspBuf[6] = 0x01;
        u8RspLength = 1;
        printf("[%s]:recv slot[%d] first message.\r\n", __func__, u8Slot);
    }
    /* heartbeat��Ϣ���� */
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
            /* float�������ݴ�С��ת�� */
            u8FloatBuf[0] = pu8Buf[11];
            u8FloatBuf[1] = pu8Buf[10];
            u8FloatBuf[2] = pu8Buf[9];
            u8FloatBuf[3] = pu8Buf[8];
            /* �����¶���Ϣ */
            boards[u8Slot].temperature = *(f32*)(u8FloatBuf);
        }
        else if(BOARD_TYPE_PEU == u8BoardType)
        {
            /* �����ذ��¶� */
            boards[u8Slot].temperature = (pu8Buf[9]<<8) | pu8Buf[10];
        }
        else if(BOARD_TYPE_ES == u8BoardType)
        {
            /* ����ES���¶� */
            boards[u8Slot].temperature = (pu8Buf[8]<<8) | pu8Buf[9];
        }
    }
    /* ������Ϣ���� */
    else if(u8IpmbCmd == IPMB_CMD_ID_BOOTREQ)
    {
        if((BOARD_TYPE_FAN==u8BoardType)||(BOARD_TYPE_PEU==u8BoardType))
        {
            if(pthread_mutex_trylock(&ipmb_lock)==BSP_OK)
            {
                /* ��ȡpkg_id */
                memcpy((u8RspBuf+6), (pu8Buf+8), 2);
                /* ��ȡfilename */
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
                /* ��ȡpkg_id */
                memcpy((u8RspBuf+6),(pu8Buf+8),2);
                *(unsigned int *)(u8RspBuf+8) = 0;
                u8RspLength = 6;
                mcu_update_flag = 0;
            }
        }
    }
    /* RS485������Ϣ���� */
    else if(u8IpmbCmd == IPMB_CMD_PEUACTL_RS485_TEST)
    {
        //��RS485���Ա�־λ
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
    /* ����Ŀ��i2c��ַ */
    u8DstAddr = IPMB_MCT_HMI2_I2C_ADDR_BASE + u8Slot;

    //�ظ�Ӧ����Ϣ
    u8ret = bsp_send_hmi_data(IPMB_SEND_MESSAGE_PROCESS, \
                              u8IpmbCmd, \
                              IPMB_PRI_FIRST, \
                              IPMB_MCT_HMI2_I2C_ADDR(bsp_get_slot_id()), \
                              u8DstAddr, \
                              u8RspBuf, \
                              u8RspLength \
                             );
    /* �ж��Ƿ���Ҫ���� */
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
    /* �ж��Ƿ���Ҫ���÷���ת�� */
    if(fan_set_flag)
    {
        fan_set_flag = 0;
        fan_control_algorithm_2(100);
    }
    return u8ret;
}

/**************************************************************************************************
*  ��������: bsp_ipmb_process_response
*  ��    ��: ����Ӧ����Ϣ����
*  ��    ��: (1)pu8Buf: pointer of send buffer.
*            (2)u8Length: length of send buffer.
*  ��    ��: ��.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
void bsp_ipmb_process_response(UINT8 *pu8Buf, UINT8 u8Length)
{
    UINT8 u8SeqNo;             /* ���к� */
    UINT8 u8IpmbCmd;           /* ����� */
    UINT8 u8SrcAddr,u8slot,u8boardtype;
    UINT8 i;
    UINT32 dwreg,dwval,u32packetsnum;
	
    /* ȡ�����к� */
    u8SeqNo = pu8Buf[SEQNO_BYTE];
    /* ȡ������� */
    u8IpmbCmd = pu8Buf[CMD_BYTE];
    /* ȡ��Դ��ַ */
    u8SrcAddr = pu8Buf[RQADDR_BYTE];
    /* �����λ�� */
    u8slot = bsp_get_slot_byi2caddr(u8SrcAddr);
    /* �ڷ�����Ϣ�������ҵ���Ӧ�����кţ����û���ҵ�����������Ϣ */
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
            /* ȡ�������λ�� */
            u8slot = pu8Buf[7];
            if((u8slot > 1) && (u8slot < 11))
            {
                boards[u8slot].mcu_status = MCU_STATUS_RESET_ACKED;
                printf("recv slot(%d) ipmb reset ack!\r\n", u8slot);
            }
            break;
        case IPMB_CMD_COMM_START_SEND_MESSAGE:
            /* ȡ�������λ�� */
            u8slot = pu8Buf[7];
            if((u8slot > 1) && (u8slot < 11))
            {
                boards[u8slot].mcu_alive = 1;
                printf("recv slot(%d) start message ack!\r\n", u8slot);
            }
            break;
        case IPMB_CMD_COMM_CHECK_ALIVE:
            /* ȡ�������λ�� */
            u8slot = pu8Buf[7];
            if((u8slot > 1) && (u8slot < 11))
            {
                boards[u8slot].mcu_alive = 1;
                printf("recv alive message(%d)!\r\n", u8slot);
            }
            break;

        /* дfpga�Ĵ��� */
        case IPMB_CMD_BASEBANDCTL_WRITE_FPGA:
            dwreg = (((UINT32)pu8Buf[7]<<24)|((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            dwval = (((UINT32)pu8Buf[11]<<24)|((UINT32)pu8Buf[12]<<16)|((UINT32)pu8Buf[13]<<8)|((UINT32)pu8Buf[14]<<0));
            bsp_put_to_fpga_buff(dwreg,dwval);
            if(g_hmi_response_debug == 1)
                printf("The fpga reg(%d) write value is 0x%x.\r\n",dwreg, dwval);
            break;
        /* дepld�Ĵ��� */
        case IPMB_CMD_BASEBANDCTL_WRITE_EPLD:
            dwreg = (((UINT32)pu8Buf[7]<<24)|((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            dwval = (((UINT32)pu8Buf[11]<<24)|((UINT32)pu8Buf[12]<<16)|((UINT32)pu8Buf[13]<<8)|((UINT32)pu8Buf[14]<<0));
            bsp_put_to_epld_buff(dwreg,dwval);
            if(g_hmi_response_debug == 1)
                printf("The epld reg(%d) write value is 0x%x.\r\n",dwreg, dwval);
            break;
        /* ��fpga�Ĵ��� */
        case IPMB_CMD_BASEBANDCTL_READ_FPGA:
            dwreg = (((UINT32)pu8Buf[7]<<24)|((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            dwval = (((UINT32)pu8Buf[11]<<24)|((UINT32)pu8Buf[12]<<16)|((UINT32)pu8Buf[13]<<8)|((UINT32)pu8Buf[14]<<0));
            bsp_put_to_fpga_buff(dwreg,dwval);
            if(g_hmi_response_debug == 1)
                printf("The fpga reg(%d) read value is 0x%x.\r\n",dwreg, dwval);
            break;
        /* ��epld�Ĵ��� */
        case IPMB_CMD_BASEBANDCTL_READ_EPLD:
            dwreg = (((UINT32)pu8Buf[7]<<24)|((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            dwval = (((UINT32)pu8Buf[11]<<24)|((UINT32)pu8Buf[12]<<16)|((UINT32)pu8Buf[13]<<8)|((UINT32)pu8Buf[14]<<0));
            bsp_put_to_epld_buff(dwreg,dwval);
            if(g_hmi_response_debug == 1)
                printf("The epld reg(%d) read value is 0x%x.\r\n",dwreg, dwval);
            break;

        /* ���÷���ת�� */
        case IPMB_CMD_FANCTL_SET_SPEED:
            g_fan_speed_set = 1;
			g_fan_set_state = BSP_OK;
            if(g_hmi_response_debug)
                printf("Set fan channel %d speed respond value is %d!\r\n",pu8Buf[7],pu8Buf[8]);
            break;

        /* ��ȡ����ת�� */
        case IPMB_CMD_FANCTL_GET_SPEED:
            g_fan_speed_get = 1;
            /* ��ȡͨ��0�ķ���ת�� */
            if(pu8Buf[7] == 0)
            {
                g_fan1_speed = (((UINT32)pu8Buf[8]<<24) |((UINT32)pu8Buf[9]<<16)|((UINT32)pu8Buf[10]<<8)|((UINT32)pu8Buf[11]<<0));
                if(g_hmi_response_debug)
                    printf("The channel %d Current fan speed is %d.\r\n",pu8Buf[7],g_fan1_speed);
            }
            /* ��ȡͨ��1�ķ���ת�� */
            else if(pu8Buf[7] == 1)
            {
                g_fan2_speed = (((UINT32)pu8Buf[8]<<24) |((UINT32)pu8Buf[9]<<16)|((UINT32)pu8Buf[10]<<8)|((UINT32)pu8Buf[11]<<0));
                if(g_hmi_response_debug)
                    printf("The channel %d Current fan speed is %d.\r\n",pu8Buf[7],g_fan2_speed);
            }
            /* ��ȡͨ��2�ķ���ת�� */
            else if(pu8Buf[7] == 2)
            {
                g_fan3_speed = (((UINT32)pu8Buf[8]<<24) |((UINT32)pu8Buf[9]<<16)|((UINT32)pu8Buf[10]<<8)|((UINT32)pu8Buf[11]<<0));
                if(g_hmi_response_debug)
                    printf("The channel %d Current fan speed is %d.\r\n",pu8Buf[7],g_fan3_speed);
            }
            /* ͬʱ��ȡ3��ͨ���ķ���ת�� */
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

        /* ��ȡ���ͱ��ĳɹ���ͳ�� */
        case IPMB_CMD_BASEBANDCTL_SEND_PACK_OK_CNT:
        case IPMB_CMD_SWITCHCTL_SEND_PACK_OK_CNT:
        case IPMB_CMD_FANCTL_SEND_PACK_OK_CNT:
        case IPMB_CMD_PEUACTL_SEND_PACK_OK_CNT:
            u32packetsnum = (((UINT32)pu8Buf[7]<<24) |((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            if(g_hmi_response_debug == 1)
                printf("hmi send sucessful packets is %d.\r\n",u32packetsnum);
            break;

        /* ��ȡ���ͱ���ʧ����ͳ�� */
        case IPMB_CMD_BASEBANDCTL_SEND_PACK_ERR_CNT:
        case IPMB_CMD_SWITCHCTL_SEND_PACK_ERR_CNT:
        case IPMB_CMD_FANCTL_SEND_PACK_ERR_CNT:
        case IPMB_CMD_PEUACTL_SEND_PACK_ERR_CNT:
            u32packetsnum = (((UINT32)pu8Buf[7]<<24) |((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            if(g_hmi_response_debug == 1)
                printf("hmi send failed packets is %d.\r\n",u32packetsnum);
            break;

        /* ��ȡ���ձ��ĳɹ���ͳ�� */
        case IPMB_CMD_BASEBANDCTL_RECV_PACK_OK_CNT:
        case IPMB_CMD_SWITCHCTL_RECV_PACK_OK_CNT:
        case IPMB_CMD_FANCTL_RECV_PACK_OK_CNT:
        case IPMB_CMD_PEUACTL_RECV_PACK_OK_CNT:
            u32packetsnum = (((UINT32)pu8Buf[7]<<24) |((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            if(g_hmi_response_debug == 1)
                printf("hmi received sucessful packets is %d.\r\n",u32packetsnum);
            break;

        /* ��ȡ���ձ���ʧ����ͳ�� */
        case IPMB_CMD_BASEBANDCTL_RECV_PACK_ERR_CNT:
        case IPMB_CMD_SWITCHCTL_RECV_PACK_ERR_CNT:
        case IPMB_CMD_FANCTL_RECV_PACK_ERR_CNT:
        case IPMB_CMD_PEUACTL_RECV_PACK_ERR_CNT:
            u32packetsnum = (((UINT32)pu8Buf[7]<<24) |((UINT32)pu8Buf[8]<<16)|((UINT32)pu8Buf[9]<<8)|((UINT32)pu8Buf[10]<<0));
            if(g_hmi_response_debug == 1)
                printf("hmi received failed packets is %d.\r\n",u32packetsnum);
            break;

        /* �ϵ����� */
        case IPMB_CMD_BASEBANDCTL_POWER_ON:
        case IPMB_CMD_SWITCHCTL_POWER_ON:
        case IPMB_CMD_FANCTL_POWER_ON:
        case IPMB_CMD_PEUACTL_POWER_ON:
            if(g_hmi_response_debug == 1)
                printf("Power on response value is %d!\r\n",pu8Buf[7]);
            break;
        /* �µ����� */
        case IPMB_CMD_BASEBANDCTL_POWER_OFF:
        case IPMB_CMD_SWITCHCTL_POWER_OFF:
        case IPMB_CMD_FANCTL_POWER_OFF:
        case IPMB_CMD_PEUACTL_POWER_OFF:
            if(g_hmi_response_debug == 1)
                printf("Power off response value is %d!\r\n",pu8Buf[7]);
            g_peu_power_down_flag = 1;
            break;

        /* ��ȡ�������¶�ֵ */
        case IPMB_CMD_BASEBANDCTL_GET_TEMP:
        {
            UINT8 u8cnt;
            /* UINT8��������ת��ΪSINT8 */
            memcpy(g_s8BbpGetTemperature, (SINT8 *)(pu8Buf + 7), 8);
            if(g_hmi_response_debug)
            {
                for(u8cnt = 0; u8cnt < 7; u8cnt++)
                    printf("The baseband board point%d temperature = %d.\r\n",u8cnt,g_s8BbpGetTemperature[u8cnt]);
            }
        }
        break;

        /* ��ȡFSA���¶�ֵ */
        case IPMB_CMD_FSA_GET_TEMP:
        {
            UINT8 u8cnt;
            /* UINT8��������ת��ΪSINT8 */
            memcpy(g_s8FsaGetTemperature, (SINT8 *)(pu8Buf + 7), 4);
            if(g_hmi_response_debug)
            {
                for(u8cnt = 0; u8cnt < 4; u8cnt++)
                    printf("The switch board point%d temperature = %d.\r\n",u8cnt,g_s8FsaGetTemperature[u8cnt]);
            }
        }
        break;
         /* ��ȡES���¶�ֵ */
        case IPMB_CMD_ES_GET_TEMP:
        {
            g_s16EsTemp = (pu8Buf[7]<<8) | pu8Buf[8];
            if(g_hmi_response_debug)
                printf("The es board temperature = %d.\r\n",g_s16EsTemp);
        }
        /* ��ȡ��ذ��¶�ֵ */
        case IPMB_CMD_PEUACTL_GET_TEMP:
        {
            g_peu_get_temp = 1;
            g_s16PEUTemp = (pu8Buf[7]<<8) | pu8Buf[8];
            if(g_hmi_response_debug)
                printf("The peua board temperature = %d.\r\n",g_s16PEUTemp);
        }
        break;
        /* ��ȡARM�汾��Ϣ*/
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

        /* ������HMI���� */
        case IPMB_CMD_BASEBANDCTL_TEST:
        case IPMB_CMD_FSA_TEST:
        case IPMB_CMD_ES_TEST:
            g_bbp_hmi_test = 1;
            if(g_hmi_response_debug)
                printf("The baseband board hmi test ok(0x%x).\r\n",u8SeqNo);
            break;

        /* ������HMI���� */
        case IPMB_CMD_SWITCHCTL_TEST:
            g_ges_hmi_test = 1;
            if(g_hmi_response_debug)
                printf("The switch board hmi test ok(0x%x).\r\n",u8SeqNo);
            break;

        /* ���Ȱ�HMI���� */
        case IPMB_CMD_FANCTL_TEST:
            g_fan_hmi_test = 1;
            if(g_hmi_response_debug)
                printf("The fan control board hmi test ok(0x%x).\r\n",u8SeqNo);
            break;

        /* ��ذ�HMI���� */
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

        /* ���Ȱ�EEPROM���� */
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


        /* ��λ�Ż�ȡ */
        case IPMB_CMD_FANCTL_GET_SLOT:
        case IPMB_CMD_PEUACTL_GET_SLOT:
            if(g_hmi_response_debug)
                printf("The board slot = %d.\r\n", pu8Buf[7]);
            break;
        /* �����ͻ�ȡ */
        case IPMB_CMD_FANCTL_GET_BOARD_TYPE:
        case IPMB_CMD_PEUACTL_GET_BOARD_TYPE:
            if(g_hmi_response_debug)
                printf("The board type = %d.\r\n", pu8Buf[7]);
            break;
        /* ���Ȱ�PCB�汾�Ż�ȡ */
        case IPMB_CMD_FANCTL_GET_PCB_VERSION:
        case IPMB_CMD_PEUACTL_GET_PCB_VERSION:
            if(g_hmi_response_debug)
                printf("The pcb version = %d.\r\n", pu8Buf[7]);
            break;

        /* ��ذ�ɽ��״̬��ȡ */
        case IPMB_CMD_PEUACTL_GET_DRYIN_STATE:
            g_peu_get_dryin = 1;
            g_peu_dryin0123 = pu8Buf[7];
            g_peu_dryin4567 = pu8Buf[8];
            if(g_hmi_response_debug)
                printf("The peua control dryin0123 = 0x%x,dryin4567 = 0x%x.\r\n", pu8Buf[7],pu8Buf[8]);
            break;

        /* ���Ȱ�EEPROM�������� */
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
        /* ���Ȱ�EEPROM������ȡ */
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
*  ��������: ipmb_file_download_thread
*  ��    ��: ipmb�ļ�����
*  ��    ��: ��
*  ��    ��: none.
*  ��    ��:
*  ��    ��: 2015/8/6
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
    u8  u8MsgBuf[IPMB_MESSAGE_MAX_LENGTH];   /* ��Ϣ���� */
    u16 u16MsgLength = 0;                    /* ��Ϣ���� */
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

    //����ipmb�ļ������߳�
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
*  ��������: bsp_put_to_fpga_buff
*  ��    ��: ipmbӦ����Ϣ������
*  ��    ��: (1)dwReg:�Ĵ�����ַ
*            (2)dwVal:�Ĵ�����ֵ
*  ��    ��: none.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_put_to_epld_buff
*  ��    ��: ipmbӦ����Ϣ������
*  ��    ��: dwReg:�Ĵ�����ַ
*            dwVal:�Ĵ�����ֵ
*  ��    ��: none.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_proc_recvhmidata
*  ��    ��: ipmb�������ݴ�����
*  ��    ��: none
*  ��    ��: none.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
uint32_t test_rmsg_num = 0;
void bsp_ipmb_receive_task(void)
{
    UINT8  ret;
    UINT8 u8CurrentIndex = 0;                    /* ��������ĵ�ǰ���� */
    UINT8 u8MsgBuf[IPMB_MESSAGE_MAX_LENGTH];     /* ������յ���IPMB��Ϣ */
    UINT32 u32MsgLength;                           /* ���յ���IPMB��Ϣ�ĳ��� */
    UINT8 u8NetFn;                               /* ���繦�ܺ�*/
    UINT8 u8SeqNo;                               /* ���к�*/
    UINT32 i;
    UINT8 u8Err;

    while(1)
    {
        /* �ȴ��ź��� */
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
                /* �ж���������Ϣ����Ӧ����Ϣ */
                u8NetFn = u8MsgBuf[NETFN_BYTE] >> 2;
                if(u8NetFn % 2 == 0)
                {
                    /* ������Ϣ */
                    bsp_ipmb_process_request(u8MsgBuf, u32MsgLength);
                }
                else
                {
                    /* Ӧ����Ϣ */
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
*  ��������: bsp_ipmb_sendfpga_task
*  ��    ��: дfpga�Ĵ�������
*  ��    ��: none
*  ��    ��: none.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_ipmb_sendepld_task
*  ��    ��: дcpld�Ĵ�������
*  ��    ��: none
*  ��    ��: none.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_ipmb_sendhmi_task
*  ��    ��: ����hmi��������
*  ��    ��: none
*  ��    ��: none.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_ipmb_init
*  ��    ��: ipmb��ʼ��
*  ��    ��: none
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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

    /*��ʼ����fpga���� */
    for(u8Index = 0; u8Index < READ_FPGA_BUF_NUM; u8Index++)
    {
        g_struReadFpgaQueueBuf[u8Index].u8Used = 0;
        /* ����Ԫ�س�ʼ�� */
        for(i=0; i<READ_FPGA_BUF_NUM; i++)
        {
            g_struReadFpgaQueueBuf[u8Index].dwReg = 0xdeadbeef;
            g_struReadFpgaQueueBuf[u8Index].dwVal = 0xdeadbeef;
        }
    }

    /*��ʼ����EPLD���� */
    for(u8Index = 0; u8Index < READ_EPLD_BUF_NUM; u8Index++)
    {
        g_struReadEpldQueueBuf[u8Index].u8Used = 0;
        /* ����Ԫ�س�ʼ�� */
        for(i=0; i<READ_EPLD_BUF_NUM; i++)
        {
            g_struReadEpldQueueBuf[u8Index].dwReg = 0xdeadbeef;
            g_struReadEpldQueueBuf[u8Index].dwVal = 0xdeadbeef;
        }
    }
    /* ��ʼ��������Ϣ���� */
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
    /* hmi ioctl��ʼ�� */
    bsp_hmi_ioctl_init(bsp_get_slot_id());
    /* ����ת�ٿ��� */
    bsp_ipmb_fan_control_init();

    return BSP_OK;
}

/**************************************************************************************************
*  ��������: bsp_hmi_config_table_init
*  ��    ��: hmi���ò�����ʼ��
*  ��    ��:
*  ��    ��: ��.
*  ��    ��:����
*  ��    ��: 2015/11/6
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
*  ��������: bsp_hmi_print_status
*  ��    ��: ��ӡhmi������ͳ��
*  ��    ��:
*  ��    ��: ��.
*  ��    ��:����
*  ��    ��: 2015/11/6
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
    UINT8     u8CHNo;          /*����ͨ���� */
    UINT8     u8LocalI2cAddr;  /*����i2c ��ַ */
    UINT8     u8RemoteI2cAddr; /*Զ��i2c ��ַ */
    UINT8     u8Type;
    UINT8     u8Prio;          /*�������ȼ�*/
    UINT8     u8RetryTime;     /*����ʧ�����Դ���*/
    UINT32    dwIndex;         /*����������*/
    UINT32    dwTimeout;       /*��ʱʱ��*/
    UINT32    u8IpmbCmd;
    void      *pAddr;
    UINT32    dwLen;           /* ���ݰ����� */
} T_HmiDirectIOParam;

/**************************************************************************************************
*  ��������: bsp_send_hmidata
*  ��    ��: IPMB��Ϣ���ͺ���
*  ��    ��: (1) u8Prio:��Ϣ���ȼ�
*            (2) u8Type:��Ϣ����
*            (3) index:��Ϣ����
*  ��    ��: BSP_ERROR/BSP_OK
*  ��    ��:
*  ��    ��: 2015/12/6
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
*  ��������: BspHmiDirectTransfer
*  ��    ��: IPMB��Ϣ���
*  ��    ��: (1) ptDirectIOParam:��Ϣ�ṹ��
*  ��    ��:
*  ��    ��: ����
*  ��    ��: 2015/12/6
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
*  ��������: bsp_send_hmi_data
*  ��    ��:
*  ��    ��:
*  ��    ��:
*  ��    ��: ����
*  ��    ��: 2015/12/6
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
    /*ȥ��ͷ��β���ĳ���*/
    if (dwTransLen <(IPMB_MESSAGE_DATA_LENGTH))
    {
        dwRtnVal = BspHmiDirectTransfer(&tHmiDirectIOParam);
    }
    return  dwRtnVal;

}


/**************************************************************************************************
*  ��������: bsp_fan_control_init
*  ��    ��: ���ȿ��Ƴ�ʼ��
*  ��    ��: none
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
UINT8 bsp_ipmb_fan_control_init(void)
{
    int res=0;
    pthread_t ptida;

    //��������ת�ٿ����߳�
    res = pthread_create(&ptida, NULL, (FUNCPTR)bsp_ipmb_fan_control_task,NULL);
    if (-1 == res)
    {
        perror("create bsp_ipmb_fan_control_task thread error!\n");
    }
}

/**************************************************************************************************
*  ��������: fan_control_algorithm_2
*  ��    ��: ����ת�ٿ����㷨ʵ��
*  ��    ��: none
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
  //ͨ��HMI���ͷ���ת����������
  ret = bsp_hmi_fan_speed(0, 3, pwm_val);
  if(ret == BSP_ERROR)
  {
  	temp_prev = 100;
  }  
	return ret;
}


/**************************************************************************************************
*  ��������: bsp_ipmb_fan_control_task
*  ��    ��: ����ת�ٿ���
*  ��    ��: none
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
        //��ȡ���ذ��¶�
        f32MdtTemp = bsp_get_temperature();
        //��ȡ��������¶�ֵ
        f32SubboardTemp = 0.0;
        for(slot=0; slot<MAX_BOARDS_NUMBER-1; slot++)
        {    
            if(f32SubboardTemp < boards[slot].temperature)
                f32SubboardTemp = boards[slot].temperature;
        }
        f32Temperature = (f32MdtTemp > f32SubboardTemp)? f32MdtTemp : f32SubboardTemp;
        if(fan_control_debug)
        {
            printf("���ذ��¶�=%f,��������¶�=%f,����¶�=%f\r\n",
                   f32MdtTemp,f32SubboardTemp,f32Temperature);
        }
        //�����¶ȿ��Ʒ���ת��
        if(MCT_MASTER == bsp_get_self_MS_state())
        	s32Ret = fan_control_algorithm_2(f32Temperature);
    }
}
/**************************************************************************************************
*  ��������: bsp_hmi_mcu_update
*  ��    ��: ��������
*  ��    ��: (1)u8boardtype: ������ѡ��(0������,1������,2����)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_boards_reboot
*  ��    ��: ��λ���е���
*  ��    ��: ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_check_alive
*  ��    ��: ��ѯ������λ����
*  ��    ��: ��
*  ��    ��: TRUE/FALSE.
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_start_send_message
*  ��    ��: ������Ϣ����
*  ��    ��: ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_stop_send_message
*  ��    ��: ������Ϣ����
*  ��    ��: ��
*  ��    ��: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_stop_send_message(void)
{
    u8 cmd = IPMB_CMD_COMM_STOP_SEND_MESSAGE;
    u8 u8boardtype = BOARD_TYPE_ALL;
    return bsp_ipmb_ioctl(cmd, u8boardtype, 0, 0);
}
/*************************************************************************************************
*  ��������: bsp_hmi_board_reboot
*  ��    ��: ���帴λ
*  ��    ��: (1)u8boardtype: ������ѡ��(2������,3��ذ�,1���Ȱ�)
*            (2)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_speed
*  ��    ��: ����ת������/��ȡ
*  ��    ��: (1)u8dwSel: 0����ת��,1��ȡת��
*            (2)u8FanChannel:ͨ��ѡ��(0-3)
*            (3)u8FanPWMVal:����ת��(0-100)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_get_temperature
*  ��    ��: �¶Ȼ�ȡ
*  ��    ��: (1)u8boardtype: ������ѡ��(2������,3��ذ�)
*            (2)u8slot:��λ��

*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_write_fpga_epld
*  ��    ��: дfpga/cpld�Ĵ���
*  ��    ��: (1)u8dwSel: дƬѡ(0 FPGA,1 CPLD)
*            (2)dwReg:�Ĵ�����ַ
*            (3)dwVal:�Ĵ���ֵ
*            (4)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_read_fpga_epld
*  ��    ��: ��fpga/cpld�Ĵ���
*  ��    ��: (1)u8dwSel: ��Ƭѡ(0 FPGA,1 CPLD)
*            (2)dwReg:�Ĵ�����ַ
*            (3)dwVal:�Ĵ���ֵ
*            (4)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_packets_statistics
*  ��    ��: ��ȡͳ��ֵ
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,1���Ȱ�)
*            (2)u8packstat:��ͳ������(0���ͳɹ�,1����ʧ��,2���ճɹ�,3����ʧ��)
*            (3)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
**************************************************************************************************/
SINT8 bsp_hmi_packets_statistics(UINT8 u8boardtype, UINT8 u8packstat,UINT8 u8slot)
{
    UINT8 u8Ret;
    u8 cmd;
	
    if((u8slot<2) || (u8slot>10))
    	return BSP_ERROR;
		
    /* ��ȡ����� */
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
*  ��������: bsp_hmi_packets_statistics
*  ��    ��: ���µ����
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8PowerOn:���µ�(0�µ�,1�ϵ�)
*            (3)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_get_arm_ver
*  ��    ��: ��ȡ�汾��
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_set_heart_beat_gap
*  ��    ��: �����ϱ�ʱ��������
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8TimeGap:ʱ����
*            (3)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_get_board_slot
*  ��    ��: ��ȡ�����λ��
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_get_board_type
*  ��    ��: ��ȡ��������
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_get_pcb_version
*  ��    ��: ��ȡ����PCB�汾
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_get_dryin_state
*  ��    ��: ��ȡ��ذ�ɽ��״̬
*  ��    ��: (1)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_rs485_test
*  ��    ��: ��ذ�rs485��·����
*  ��    ��: (1)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_board_test
*  ��    ��: HMI��·����
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_test_eeprom
*  ��    ��: EEPROM����
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_test_geswitch
*  ��    ��: SWITCH·�����ò���
*  ��    ��: (1)u8boardtype: ������(2������,3��ذ�,5�����壬1���Ȱ�)
*            (2)u8slot:��λ��
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_eeprom_crc
*  ��    ��: ���Ȱ�EEPROM��������/��ȡ�ӿ�
*  ��    ��: (1)u8dwSel: (u8dwSel:0���ã�1��ȡ)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_eeprom_device_id
*  ��    ��: ���Ȱ�EEPROM��������/��ȡ�ӿ�
*  ��    ��: (1)u8dwSel: (u8dwSel:0���ã�1��ȡ)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_eeprom_board_type
*  ��    ��: ���Ȱ�EEPROM��������/��ȡ�ӿ�
*  ��    ��: (1)u8dwSel: (u8dwSel:0���ã�1��ȡ)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_eeprom_product_sn
*  ��    ��: ���Ȱ�EEPROM��������/��ȡ�ӿ�
*  ��    ��: (1)u8dwSel: (u8dwSel:0���ã�1��ȡ)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_eeprom_manufacture
*  ��    ��: ���Ȱ�EEPROM��������/��ȡ�ӿ�
*  ��    ��: (1)u8dwSel: (u8dwSel:0���ã�1��ȡ)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_eeprom_product_date
*  ��    ��: ���Ȱ�EEPROM��������/��ȡ�ӿ�
*  ��    ��: (1)u8dwSel: (u8dwSel:0���ã�1��ȡ)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_eeprom_temp_threshold
*  ��    ��: ���Ȱ�EEPROM��������/��ȡ�ӿ�
*  ��    ��: (1)u8dwSel: (u8dwSel:0���ã�1��ȡ)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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
*  ��������: bsp_hmi_fan_eeprom_initial_speed
*  ��    ��: ���Ȱ�EEPROM��������/��ȡ�ӿ�
*  ��    ��: (1)u8dwSel: (u8dwSel:0���ã�1��ȡ)
*  ��    ��: TRUE/FALSE.
*  ��    ��:
*  ��    ��: 2015/8/6
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

