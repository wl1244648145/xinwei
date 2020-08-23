#ifndef __BSP_BBP_COMMAND_H__
#define __BSP_BBP_COMMAND_H__

#include "bsp_types.h"

//srio switch端口配置
#define SRIO_PORT_OF_DSP1_PORT                    	(0)
#define SRIO_PORT_OF_DSP2_PORT                    	(1)
#define SRIO_PORT_OF_DSP3_PORT                    	(2)
#define SRIO_PORT_OF_DSP4_PORT                   	(3)
#define SRIO_PORT_OF_FPGA_PORT                  	       (4)
#define SRIO_PORT_OF_BP1_PORT                            (5)
#define SRIO_PORT_OF_BP2_PORT                            (6)

#define  SRIO_PORT_OF_FS_FPGA0_PORT                  (0)
#define  SRIO_PORT_OF_FS_FPGA1_PORT                  (1)
#define  SRIO_PORT_OF_FS_SLOT0_PORT                  (2)
#define  SRIO_PORT_OF_FS_SLOT1_PORT                  (3)
#define  SRIO_PORT_OF_FS_SLOT2_PORT                  (4)
#define  SRIO_PORT_OF_FS_SLOT3_PORT                  (5)
#define  SRIO_PORT_OF_FS_SLOT4_PORT                  (6)
#define  SRIO_PORT_OF_FS_SLOT5_PORT                  (7)
#define  SRIO_PORT_OF_FS_SLOT6_PORT                  (8)


/* 设备参数结构体 */
typedef struct tagT_DevParm 
{           
	char *Name;          /* 设备名 */
	u8 SrioPort;          /* 设备对应的srio端口 */
}T_DEV_Param; 

static T_DEV_Param strBbpSrioDevParam[]=
{
    {"DSP1 PORT", SRIO_PORT_OF_DSP1_PORT},
    {"DSP2 PORT", SRIO_PORT_OF_DSP2_PORT},
    {"DSP3 PORT", SRIO_PORT_OF_DSP3_PORT},
    {"DSP4 PORT", SRIO_PORT_OF_DSP4_PORT},
    {"FPGA PORT", SRIO_PORT_OF_FPGA_PORT},
    {"BP1 PORT", SRIO_PORT_OF_BP1_PORT},
    {"BP2 PORT", SRIO_PORT_OF_BP2_PORT},
};
static const int g_SrioBbpDevParamNum = sizeof(strBbpSrioDevParam)/sizeof(T_DEV_Param);

static T_DEV_Param strFsaSrioDevParam[]=
{
    {"FPGA0 PORT", SRIO_PORT_OF_FS_FPGA0_PORT},
    {"FPGA1 PORT", SRIO_PORT_OF_FS_FPGA1_PORT},
    {"SLOT0 PORT", SRIO_PORT_OF_FS_SLOT0_PORT},
    {"SLOT1 PORT", SRIO_PORT_OF_FS_SLOT1_PORT},
    {"SLOT2 PORT", SRIO_PORT_OF_FS_SLOT2_PORT},
    {"SLOT3 PORT", SRIO_PORT_OF_FS_SLOT3_PORT},
    {"SLOT4 PORT", SRIO_PORT_OF_FS_SLOT4_PORT},
    {"SLOT5 PORT", SRIO_PORT_OF_FS_SLOT5_PORT},
    {"SLOT6 PORT", SRIO_PORT_OF_FS_SLOT6_PORT},    
};
static const int g_SrioFsaDevParamNum = sizeof(strFsaSrioDevParam)/sizeof(T_DEV_Param);

//GeSwitch端口配置
#define BBP_DSP0_PORT      (1<<0)
#define BBP_DSP1_PORT      (1<<1)
#define BBP_DSP2_PORT      (1<<2)
#define BBP_DSP3_PORT      (1<<3)
#define FSA_PHY_PORT        (1<<3)  
#define FPGA_PORT            (1<<4)
#define PORT_PPC_SLOT0    (1<<5)
#define PORT_PPC_SLOT1    (1<<6)
#define PORT_IMP                (1<<8)

int8_t bsp_mcta_start(uint32_t boardid);
int8_t bsp_get_boardtype(uint32_t boardid, uint8_t *boardtype);
int8_t bsp_get_pcb_version(uint32_t boardid, uint8_t * pcb_version);
int8_t bsp_get_slot(uint32_t boardid, uint8_t *slot);
void bsp_test_board_type(uint32_t boardid);
void bsp_test_slot(uint32_t boardid);
void bsp_test_pcb_version(uint32_t boardid);
int8_t bsp_bbp_cpld_read(uint32_t boardid, uint8_t reg, uint8_t *val);
int8_t bsp_bbp_cpld_write(uint32_t boardid, uint8_t reg, uint8_t val);
int8_t bsp_bbp_fpga_read(uint32_t boardid, uint8_t reg, uint16_t *val);
int8_t bsp_bbp_fpga_write(uint32_t boardid, uint8_t reg, uint16_t val);
int8_t bsp_bbp_fpga_load(uint32_t boardid);
int8_t bsp_bbp_mcu_update(uint32_t boardid);
int8_t bsp_bbp_cpld_update(uint32_t boardid);
int8_t bsp_bbp_dsp_reset(uint32_t boardid, uint8_t dsp_set);
int8_t bsp_bbp_dsp_close(uint32_t boardid, uint8_t dsp_set);

int8_t bsp_bbp_read_temp(uint32_t boardid, uint8_t *temp);
int8_t bsp_ges_read_temp(int8_t *temp);
int8_t bsp_bbp_test_eeprom(uint32_t boardid);
int8_t bsp_ges_test_eeprom(void);
int8_t bsp_bbp_test_ethsw(uint32_t boardid);
int8_t bsp_ges_test_ethsw(void);
int8_t bsp_bbp_srioport_status(uint32_t boardid);
int8_t bsp_get_geswitch_port_status(uint8_t boardid, uint16_t *u16status);
int8_t bsp_ges_set_phy_workmode(uint8_t portid,uint8_t workmode);

int8_t board_eeprom_set_crc(uint32_t boardid, u16 crc);
int8_t board_eeprom_get_crc(uint32_t boardid);
int8_t board_eeprom_set_deviceid(uint32_t boardid, char *deviceid);
int8_t board_eeprom_get_deviceid(uint32_t boardid);
int8_t board_eeprom_set_boardtype(uint32_t boardid, char *boardtype);
int8_t board_eeprom_get_boardtype(uint32_t boardid);
int8_t board_eeprom_set_productsn(uint32_t boardid, u8 *productsn);
int8_t board_eeprom_get_productsn(uint32_t boardid);
int8_t board_eeprom_set_manufacturer(uint32_t boardid, char *manufacturer);
int8_t board_eeprom_get_manufacturer(uint32_t boardid);
int8_t board_eeprom_set_productdate(uint32_t boardid, u8 *productdate);
int8_t board_eeprom_get_productdate(uint32_t boardid);
int8_t board_eeprom_set_tempthreshold(uint32_t boardid, s8 *tempthreshold);
int8_t board_eeprom_get_tempthreshold(uint32_t boardid);


#endif //__BPS_BBP_COMMAND_H__

