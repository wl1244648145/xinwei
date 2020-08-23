/*******************************************************************************
* *  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* �ļ�����:  cps1616.h
* ��    ��:  
* ��    ��:  V0.1
* ��д����:  2015/06/26
* ˵    ��:  ��
* �޸���ʷ:
* �޸�����           �޸���  liuganga �޸�����
*------------------------------------------------------------------------------
*------------------------------------------------------------------------------
*                                                         �����ļ�
*
*
*******************************************************************************/
/******************************** ͷ�ļ�������ͷ ******************************/

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
����IPMB������ţ�
����������:0x01~~0x1f 
���ذ����������:0x20~~~0x52
���������������:0x53~~~0x85
���������������:0x86~~~0xb8 
���Ȱ����������:0xb9~~~0xeb
����:0xec~~0xff
*/

/*���幫��������*/
#define IPMB_CMD_COMM_RESET_BOARDS                   (0x01)
#define IPMB_CMD_COMM_STOP_WATCHDOG               (0x02)
#define IPMB_CMD_COMM_FIRST_MESSAGE                  (0x03)
#define IPMB_CMD_COMM_CHECK_ALIVE                       (0x04)
#define IPMB_CMD_COMM_START_SEND_MESSAGE       (0x05)
#define IPMB_CMD_COMM_STOP_SEND_MESSAGE         (0x06)
#define IPMB_CMD_COMM_ALIVE_CMD                          (0x10)

/*�������ذ�������*/
#define IPMB_CMD_MAINBANDCTL_TEMP              (0x11)/*��ȡ�����¶�*/
/*���廷����������*/
#define IPMB_CMD_PEUACTL_GET_ARM_VER_VAL       (0x1c)/*��ȡarm�汾��*/
#define IPMB_CMD_PEUACTL_POWER_ON                    (0x1d)/*�����ϵ�*/
#define IPMB_CMD_PEUACTL_POWER_OFF                   (0x1e)/*�����µ�*/
#define IPMB_CMD_PEUACTL_SEND_PACK_OK_CNT     (0x1f)/*���ͱ��ĳɹ���ͳ��*/
#define IPMB_CMD_PEUACTL_SEND_PACK_ERR_CNT    (0x20)/*���ͱ���ʧ����ͳ��*/
#define IPMB_CMD_PEUACTL_RECV_PACK_OK_CNT      (0x21)/*���ձ��ĳɹ���ͳ��*/
#define IPMB_CMD_PEUACTL_RECV_PACK_ERR_CNT    (0x22)/*���ձ���ʧ����ͳ��*/
#define IPMB_CMD_PEUACTL_TEST                           (0x23)/* HMI��·���� */
#define IPMB_CMD_PEUACTL_REBOOT                      (0x24)/* �������� */
#define IPMB_CMD_PEUACTL_HEART_BEAT_GAP      (0x25)/* ����������� */
#define IPMB_CMD_PEUACTL_GET_BOARD_TYPE	     (0x26)// ��ȡ������
#define IPMB_CMD_PEUACTL_GET_PCB_VERSION     (0x27)// ��ȡPCB�汾��
#define IPMB_CMD_PEUACTL_GET_SLOT	             (0x28)// ��ȡ��λ��
#define IPMB_CMD_PEUACTL_SEND_EXCEPTION       (0x29)/* �쳣�ϱ� */
#define IPMB_CMD_PEUACTL_GET_TEMP                   (0x2A)/* �¶Ȼ�ȡ */
#define IPMB_CMD_PEUACTL_GET_DRYIN_STATE      (0x2B)/* �ɽ��״̬��ȡ */
#define IPMB_CMD_PEUACTL_RS485_TEST                (0x2C)/* RS485���� */

/*���彻����fsa������*/
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

/*����ES��������*/
#define IPMB_CMD_ES_REBOOT                        (0x46)
#define IPMB_CMD_ES_RESET_GESW                (0x49)
#define IPMB_CMD_ES_GET_ARM_VER_VAL      (0x4a)
#define IPMB_CMD_ES_GET_TEMP                     (0x4b)
#define IPMB_CMD_ES_HEART_BEAT_GAP         (0x4c)
#define IPMB_CMD_ES_TEST_GESWITCH           (0x4d)
#define IPMB_CMD_ES_TEST_EEPROM               (0x4e)
#define IPMB_CMD_ES_TEST                              (0x4f)

/*���������������*/
#define IPMB_CMD_BASEBANDCTL_WRITE_FPGA        (0x53)/*дfpga�Ĵ���*/
#define IPMB_CMD_BASEBANDCTL_READ_FPGA         (0x54)/*��fpga�Ĵ���*/
#define IPMB_CMD_BASEBANDCTL_WRITE_EPLD        (0x55)/*дepld�Ĵ���*/
#define IPMB_CMD_BASEBANDCTL_READ_EPLD         (0x56)/*��epld�Ĵ���*/
#define IPMB_CMD_BASEBANDCTL_GET_ARM_VER_VAL   (0x57)/*��ȡarm�汾��*/
#define IPMB_CMD_BASEBANDCTL_GET_TEMP          (0x58)/*��ȡ�������¶�*/

#define IPMB_CMD_BASEBANDCTL_RESET_DSP0        (0x59)/*��λdsp0*/
#define IPMB_CMD_BASEBANDCTL_RESET_DSP1        (0x5A)/*��λdsp1*/
#define IPMB_CMD_BASEBANDCTL_RESET_DSP2        (0x5B)/*��λdsp2*/
#define IPMB_CMD_BASEBANDCTL_RESET_DSP3        (0x5C)/*��λdsp3*/
#define IPMB_CMD_BASEBANDCTL_RESET_SRIOSW    (0x5D)/*��λsrio sw*/
#define IPMB_CMD_BASEBANDCTL_RESET_GESW        (0x5E)/*��λge sw*/

#define IPMB_CMD_BASEBANDCTL_TEST                      (0x5F)
#define IPMB_CMD_BASEBANDCTL_TEST_EEPROM       (0x60)
#define IPMB_CMD_BASEBANDCTL_TEST_GESWITCH   (0x61)
#define IPMB_CMD_BASEBANDCTL_REBOOT                 (0x62)
#define IPMB_CMD_BASEBANDCTL_HEART_BEAT_GAP (0x63)

#define IPMB_CMD_BASEBANDCTL_GET_SRIOSW_PORTINFO   (0x70)/*��ȡsrio·����Ϣ*/

#define IPMB_CMD_BASEBANDCTL_POWER_ON          (0x80)/*�����ϵ�*/
#define IPMB_CMD_BASEBANDCTL_POWER_OFF         (0x81)/*�����µ�*/

#define IPMB_CMD_BASEBANDCTL_SEND_PACK_OK_CNT  (0x82)/*���ͱ��ĳɹ���ͳ��*/
#define IPMB_CMD_BASEBANDCTL_SEND_PACK_ERR_CNT (0x83)/*���ͱ���ʧ����ͳ��*/
#define IPMB_CMD_BASEBANDCTL_RECV_PACK_OK_CNT  (0x84)/*���ձ��ĳɹ���ͳ��*/
#define IPMB_CMD_BASEBANDCTL_RECV_PACK_ERR_CNT (0x85)/*���ձ���ʧ����ͳ��*/

/*���彻����������*/
#define IPMB_CMD_SWITCHCTL_RESET_GESW              (0x86)/*��λge sw*/
#define IPMB_CMD_SWITCHCTL_GET_ARM_VER_VAL    (0x87)/*��ȡarm�汾��*/
#define IPMB_CMD_SWITCHCTL_POWER_ON                 (0x88)/*�����ϵ�*/
#define IPMB_CMD_SWITCHCTL_POWER_OFF                (0x89)/*�����µ�*/
#define IPMB_CMD_SWITCHCTL_GET_TEMP                   (0x90)/*��ȡ�������¶�*/

#define IPMB_CMD_SWITCHCTL_TEST                       (0x91)
#define IPMB_CMD_SWITCHCTL_TEST_EEPROM        (0x92)
#define IPMB_CMD_SWITCHCTL_TEST_GESWITCH    (0x93)
#define IPMB_CMD_SWITCHCTL_REBOOT                  (0x94)
#define IPMB_CMD_SWITCHCTL_WORKMODE            (0x95)
#define IPMB_CMD_SWITCHCTL_HEART_BEAT_GAP      (0x96)

#define IPMB_CMD_SWITCHCTL_SEND_PACK_OK_CNT    (0xB5)/*���ͱ��ĳɹ���ͳ��*/
#define IPMB_CMD_SWITCHCTL_SEND_PACK_ERR_CNT   (0xB6)/*���ͱ���ʧ����ͳ��*/
#define IPMB_CMD_SWITCHCTL_RECV_PACK_OK_CNT    (0xB7)/*���ձ��ĳɹ���ͳ��*/
#define IPMB_CMD_SWITCHCTL_RECV_PACK_ERR_CNT   (0xB8)/*���ձ���ʧ����ͳ��*/

/*������Ȱ�������*/
#define IPMB_CMD_FANCTL_SET_SPEED              (0xB9)/*���÷���ת��*/
#define IPMB_CMD_FANCTL_GET_SPEED              (0xBA)/*��ȡ����ת��*/
#define IPMB_CMD_FANCTL_SEND_EXCEPTION         (0xBB)/*��ȡ�쳣��Ϣ*/
#define IPMB_CMD_FANCTL_GET_ARM_VER_VAL        (0xBC)/*��ȡarm�汾��*/
#define IPMB_CMD_FANCTL_POWER_ON               (0xBD)/*�����ϵ�*/
#define IPMB_CMD_FANCTL_POWER_OFF              (0xBE)/*�����µ�*/
#define IPMB_CMD_FANCTL_SEND_PACK_OK_CNT       (0xBF)/*���ͱ��ĳɹ���ͳ��*/
#define IPMB_CMD_FANCTL_SEND_PACK_ERR_CNT      (0xC0)/*���ͱ���ʧ����ͳ��*/
#define IPMB_CMD_FANCTL_RECV_PACK_OK_CNT       (0xC1)/*���ձ��ĳɹ���ͳ��*/
#define IPMB_CMD_FANCTL_RECV_PACK_ERR_CNT      (0xC2)/*���ձ���ʧ����ͳ��*/

#define IPMB_CMD_FANCTL_TEST                   (0xC3)
#define IPMB_CMD_FANCTL_REBOOT                 (0xC4)
#define IPMB_CMD_FANCTL_HEART_BEAT_GAP         (0xC5)
#define IPMB_CMD_FANCTL_GET_BOARD_TYPE	       (0xC6)// ��ȡ������
#define IPMB_CMD_FANCTL_GET_PCB_VERSION        (0xC7)// ��ȡPCB�汾��
#define IPMB_CMD_FANCTL_GET_SLOT	           (0xC8)// ��ȡ��λ��

#define IPMB_CMD_EEPROM_SET_CRC                  (0xC9)// ����CRC
#define IPMB_CMD_EEPROM_GET_CRC                  (0xCA)// ��ȡCRC
#define IPMB_CMD_EEPROM_SET_DEVICE_ID            (0xCB)// �����豸ID
#define IPMB_CMD_EEPROM_GET_DEVICE_ID            (0xCC)// ��ȡ�豸ID
#define IPMB_CMD_EEPROM_SET_BOARD_TYPE           (0xCD)// ���ð�����
#define IPMB_CMD_EEPROM_GET_BOARD_TYPE           (0xCE)// ��ȡ������
#define IPMB_CMD_EEPROM_SET_PRODUCT_SN           (0xD1)// �����������к�
#define IPMB_CMD_EEPROM_GET_PRODUCT_SN           (0xD2)// ��ȡ�������к�
#define IPMB_CMD_EEPROM_SET_MANUFACTURER         (0xD3)// ����������
#define IPMB_CMD_EEPROM_GET_MANUFACTURER         (0xD4)// ��ȡ������
#define IPMB_CMD_EEPROM_SET_PRODUCT_DATE         (0xD5)// ������������
#define IPMB_CMD_EEPROM_GET_PRODUCT_DATE         (0xD6)// ��ȡ��������
#define IPMB_CMD_EEPROM_SET_TEMP_THRESHOLD       (0xD7)// �����¶�����ֵ
#define IPMB_CMD_EEPROM_GET_TEMP_THRESHOLD       (0xD8)// ��ȡ�¶�����ֵ
#define IPMB_CMD_EEPROM_SET_FAN_INITIALSPEED     (0xD9)// ���÷��ȳ���ת��
#define IPMB_CMD_EEPROM_GET_FAN_INITIALSPEED     (0xDA)// ��ȡ���ȳ���ת��
#define IPMB_CMD_FANCTL_TEST_EEPROM              (0xDB)// EEPROM����

/* ����&��ذ������������� */
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

/* ������Ϣ */
#define RSADDR_BYTE         (0x00)
#define NETFN_BYTE          (0x01)
#define CHECK_BYTE          (0x02)
#define RQADDR_BYTE         (0x03)
#define SEQNO_BYTE          (0x04)
#define CMD_BYTE            (0x05)
#define COMPLETE_BYTE       (0x06)
/* ��Ӧ��Ϣ */
#define CCODE_BYTE          (6)
#define DATASTART_BYTE      (7)
#define OS_NO_ERR                      0
#define IPMB_CHANNEL_IIC_0             0
#define IPMB_CHANNEL_IIC_1             1
#define IPMB_CHANNEL_IIC_2             2
#define IPMB_CHANNEL_IIC_3             3

/* ״̬���� */
#define STATUS_WAIT_RESPOND    0x01
#define STATUS_RECEIVE_RESPOND 0x02
#define IPMB_RECEIVE_BUF_NUM         (32)  /* ������Ϣ�������󳤶�                   */
#define IPMB_SEND_BUF_NUM            (3)  /* ������Ϣ�������󳤶�                   */
#define IPMB_QUEUE_BUF_NUM           (100)  /* ������Ϣ�������󳤶�                   */
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

#define IPMB_MESSAGE_MAX_LENGTH    1<<10   /* IPMB��Ϣ����󳤶�(�ֽ�)                 */
#define IPMB_MESSAGE_DATA_LENGTH   300   /* IPMBӦ����Ϣ�е����ݲ��ֵ���󳤶�(�ֽ�) */

#define HMI_SND_PACK_OK        (0)
#define HMI_SND_PACK_ERR       (1)
#define HMI_RCV_PACK_OK        (2)
#define HMI_RCV_PACK_ERR       (3)

#define IPMB_SEND_MESSAGE_PROCESS  (0)
#define IPMB_SEND_MESSAGE_REQUEST  (1)

/* ������Ϣ���鶨�� */
typedef struct tag_STRU_RESERVE_MESSAGE
{
    UINT8            u8Used;                                 /* �Ƿ�ʹ��             */
    UINT32           u32Length;                              /* ����                 */
    UINT8            u8Message[IPMB_MESSAGE_MAX_LENGTH];     /* ������յ�����Ϣ     */
} STRU_RECV_MESSAGE;


/* ������Ϣ���鶨�� */
typedef struct tag_STRU_SEND_MESSAGE
{
    UINT8            u8Used;                                /* �Ƿ�ʹ��                     */
    UINT8            u8Seq;		                            /* ������Ϣ�����к�             */
    UINT8            u8Status;                              /* ����������Ϣ�������״̬     */
    UINT32           u32Len;
    UINT8            u8Data[IPMB_MESSAGE_DATA_LENGTH];      /* ����Ӧ����Ϣ�е����ݲ���     */
	//sem_t            semID;                                 /* ���յ�Ӧ����Ϣ�󼤻���ź��� */
} STRU_SEND_MESSAGE;


/*������Ϣ���нṹ��*/
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
    UINT32    dwSndAllPacketCnt;/*����hmi���ݵ��ܰ���*/
	UINT32    dwSndAllPacketOk; /*����hmi���ݵĳɹ�����*/
	UINT32    dwSndAllPacketErr;/*����hmi���ݵ�ʧ�ܼ���*/
    UINT32    dwRcvAllPacketCnt;/*����hmi���ݵ��ܰ���*/
	UINT32    dwRcvAllPacketOk; /*����hmi���ݵĳɹ�����*/
    UINT32    dwRcvAllPacketErr;/*����hmi���ݵ�ʧ�ܼ���*/
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
   sRIO Switch SerDes�˿ڻ��������
   Quad	            �˿ڱ��	          �Զ��豸	�Զ��豸SerDes���	��ע
   Port0~1	        DSP2	                    RIO0~1	             2X    5g
   Port2~3	        DSP1	                    RIO0~1	             2X    5g
   Port4~5	        DSP3	                    RIO0~1	             2X    5g
   Port6	               DSP4	                    RIO0	                    1X    5g
   Port7	               FPGA	                    Bank117 Port2	      1X    5g
   Port8~9	        BP                             Connector	-	      2X    6.25g
   Port10~11	        BP                             Connector	-	      2X    6.25g
   Port12~15	        BP                             Connector	-	      4X    6.25g
*/
/******************************** ͷ�ļ�������β ******************************/
#endif /* __BSP_HMI_H__ */
/******************************** ͷ�ļ����� **********************************/

