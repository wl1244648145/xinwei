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

#ifndef __BSP_HMI_EXT_H__
#define __BSP_HMI_EXT_H__

extern SINT8 g_s8BbpGetTemperature[8];
extern SINT8 g_s8GesGetTemperature[4];
extern SINT16 g_s16PEUTemp;

extern UINT32 g_fan1_speed;
extern UINT32 g_fan2_speed;
extern UINT32 g_fan3_speed;
extern uint8_t g_fan_speed_set;
extern uint8_t g_fan_speed_get;
extern uint8_t g_fan_eeprom_test;
extern uint8_t g_fan_set_eeprom;
extern uint8_t g_fan_get_eeprom;

extern uint8_t g_peu_power_down_flag;
extern uint8_t g_peu_get_temp;
extern uint8_t g_peu_get_dryin;
extern uint8_t g_peu_rs485_test;
extern uint8_t g_peu_dryin0123;
extern uint8_t g_peu_dryin4567;

extern uint8_t g_fan_hmi_test;
extern uint8_t g_ges_hmi_test;
extern uint8_t g_bbp_hmi_test;
extern uint8_t g_peu_hmi_test;



/* 函数声明 */
SINT8 bsp_hmi_board_test(UINT8 u8boardtype, UINT8 u8slot);
SINT8 bsp_hmi_test_eeprom(UINT8 u8boardtype,UINT8 u8slot);
SINT8 bsp_hmi_test_geswitch(UINT8 u8boardtype,UINT8 u8slot);
SINT8 bsp_hmi_get_temperature(UINT8 u8boardtype,UINT8 u8slot);
SINT8 bsp_hmi_fan_speed(UINT8 u8dwSel,UINT8 u8FanChannel,UINT8 u8FanPWMVal);
SINT8 bsp_hmi_board_reboot(UINT8 u8boardtype, UINT8 u8slot);
SINT8 bsp_hmi_get_arm_ver(UINT8 u8boardtype,UINT8 u8slot);
SINT8 bsp_hmi_get_dryin_state(UINT8 u8slot);
SINT8 bsp_hmi_board_power_on_off(UINT8 u8boardtype, UINT8 u8PowerOn,UINT8 u8slot);

SINT8 bsp_hmi_fan_eeprom_crc(UINT8 u8dwSel);
SINT8 bsp_hmi_fan_eeprom_device_id(UINT8 u8dwSel);
SINT8 bsp_hmi_fan_eeprom_board_type(UINT8 u8dwSel);
SINT8 bsp_hmi_fan_eeprom_product_sn(UINT8 u8dwSel);
SINT8 bsp_hmi_fan_eeprom_manufacture(UINT8 u8dwSel);
SINT8 bsp_hmi_fan_eeprom_product_date(UINT8 u8dwSel);
SINT8 bsp_hmi_fan_eeprom_temp_threshold(UINT8 u8dwSel);
SINT8 bsp_hmi_fan_eeprom_temp_threshold(UINT8 u8dwSel);

/******************************** 头文件保护结尾 ******************************/
#endif /* __BSP_HMI_H__ */
/******************************** 头文件结束 **********************************/

