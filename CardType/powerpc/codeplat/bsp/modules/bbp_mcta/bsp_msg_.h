#ifndef __BSP_MSG_PROC_H__
#define __BSP_MSG_PROC_H__

#include "bsp_types.h"
#include "bsp_bbp_command.h"

#define MSG_PROC_PORT               8060
#define NET_MSG_HEADER              0xFFBBFFBB

#define BOARD_TYPE_MCT    (0x4)
#define BOARD_TYPE_BBP    (0x2)
#define BOARD_TYPE_GES    (0x5)
#define BOARD_TYPE_FSA    (0x5)
#define BOARD_TYPE_FAN    (0x1)
#define BOARD_TYPE_PEU    (0x3)
#define BOARD_TYPE_ES      (0x7)
/******************************������Ϣ�����ֶ���******************************/
// ���������ֶ���
// ���������ֶ���
//0~0x4F : Board���
#define CMD_PRINT_MSG                      	0x00  
#define CMD_HEART_BEAT                     0x01        // BBP����
#define CMD_BOARD_START	              0x02        // �忨����
#define CMD_BOARD_RESET	              0x03        // �忨��λ
#define CMD_MCTA_START	              0x04        // ���忨����

#define CMD_GET_BOARD_TYPE	                0x10        // ��ȡ������
#define CMD_GET_PCB_VERSION          	 	0x11        // ��ȡPCB�汾��
#define CMD_GET_SW_VERSION                  0x12        // ��ȡ����汾��
#define CMD_GET_SLOT	              		0x13        // ��ȡ��λ��
#define CMD_GET_TEMPERATURE					0x14        // ��ȡ�¶�
#define CMD_GET_POWER_INFO					0x15        // ��ȡPower��������Ϣ
#define CMD_GET_EEPROM_TEST					0x16        // EEPROM����
#define CMD_GET_SFP_INFO					0x17        // ��ȡSFP��Ϣ
#define CMD_TEST_SDRAM						0x18 		// SDRAM����
#define CMD_TEST_ETHSW						0x19 		// GESwitch����
#define CMD_TEST_SRIOSWT                    0x1a        // SRIOSWITCH����  
#define CMD_TEST_GES_PHY                    0x1b        // GES_PHY�˿���������
#define CMD_GET_SRIO_PORT_STATUS            0x1c        // ��ȡsrioswitch�˿�״̬
#define CMD_GET_ETHSW_LINK_STATUS           0x1d        // ��ȡgeswitch�˿�����״̬
#define CMD_SET_DSP_LOAD_STATUS         	0x1e            //����dsp����״̬
#define CMD_TEST_PHY54210S                    0x20        // PHY54210TEST
#define CMD_TEST_PLL1_CFG                       0x21        //PLL1�Ĵ������ò���
#define CMD_GET_PLL1_STATUS                   0x22       //PLL1 LOCK״̬��ȡ
#define CMD_TEST_PLL2_CFG                       0x23        //PLL2�Ĵ������ò���
#define CMD_GET_PLL2_STATUS                   0x24       //PLL2 LOSS״̬��ȡ
#define CMD_GET_FPGA160T_SYNC_STATUS 0x25      //FPGA160T ���ٽӿ�ͬ��״̬��ȡ

#define CMD_EEPROM_SET_CRC                  0x2b
#define CMD_EEPROM_GET_CRC                  0x2c
#define CMD_EEPROM_SET_DEVICE_ID            0x2d
#define CMD_EEPROM_GET_DEVICE_ID            0x2e
#define CMD_EEPROM_SET_BOARD_TYPE           0x2f
#define CMD_EEPROM_GET_BOARD_TYPE           0x30
#define CMD_EEPROM_SET_PCB_VERSION          0x31
#define CMD_EEPROM_GET_PCB_VERSION          0x32
#define CMD_EEPROM_SET_PRODUCT_SN           0x33
#define CMD_EEPROM_GET_PRODUCT_SN           0x34
#define CMD_EEPROM_SET_MANUFACTURER         0x35
#define CMD_EEPROM_GET_MANUFACTURER         0x36
#define CMD_EEPROM_SET_PRODUCT_DATE         0x37
#define CMD_EEPROM_GET_PRODUCT_DATE         0x38
#define CMD_EEPROM_SET_TEMP_THRESHOLD       0x39
#define CMD_EEPROM_GET_TEMP_THRESHOLD       0x3a
#define CMD_SRIO_SW_STATES					0x4D		//SRIO SW״̬
#define CMD_TEST_MCU_UPLOAD                 0x4E        // MCU����
#define CMD_MCU_UPLOAD                 	    0x4F        // MCU����

//0x50~0x5F : DSP�������
#define CMD_DSP_RESET                       0x50        // Reset DSP
#define CMD_DSP_CLOSE                       0x51        // Close DSP

//0x60~0x6F : FPGA�������
#define CMD_FPGA_LOAD                       0x60        // FPGA����
#define CMD_FPGA_REG_READ			        0x61        // FPGA�Ĵ�����ȡ
#define CMD_FPGA_REG_WRITE                  0x62        // FPGA�Ĵ���д��
#define CMD_FPGA_SUCCESS_MSG                0x63
#define CMD_FPGA_FAIL_MSG                   0x64


//0x70~0x7F : CPLD�������
#define CMD_CPLD_UPLOAD                     0x70        // CPLD����
#define CMD_CPLD_REG_READ			        0x71        // CPLD�Ĵ�����ȡ
#define CMD_CPLD_REG_WRITE                  0x72        // CPLD�Ĵ���д��

#define CMD_DSP_DUMP                        0x80        // DSP DUMP

#define CMD_TFTP_GET_FILE                   0x90        // TFTP GET FILE

/****************************************************************************************/

#define PACKET_DATA_SIZE        100
typedef struct
{
    uint32_t header;
    uint32_t cmd;
    uint32_t pkgid;
    uint32_t datalen;
    uint8_t data[PACKET_DATA_SIZE]; //PACKET_TYPE_SIZE-sizeof(u32Head)-sizeof(u8Cmd)
}msg_package_t;
#define PACKET_HEADER_SIZE	 (sizeof(msg_package_t)-PACKET_DATA_SIZE)

#define MSG_SENDBUF_COUNT  1000
typedef struct{
	uint32_t pkgid;
	msg_package_t send;
	msg_package_t ack;
	pthread_mutex_t wait;
	uint32_t timeout_ms;
}msg_sendbuf_t;

msg_sendbuf_t *bsp_get_sendbuf(void);
msg_sendbuf_t *bsp_get_sendbuf_by_pkgid(int pkgid);
void bsp_release_sendbuf(msg_sendbuf_t *psend);

int msg_send(uint16_t boardid, msg_sendbuf_t *psend);

void init_msg_proc_thread(void);

#endif //__BSP_MSG_PROC_H__

