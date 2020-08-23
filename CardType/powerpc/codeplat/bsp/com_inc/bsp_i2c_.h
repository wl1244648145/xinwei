/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_i2c_ext.h 
* 功能:               i2c应用层接口   
* 版本:                                                                  
* 编制日期:                              
* 作者:                hjf                                   
*******************************************************************************/
#ifndef BSP_I2C_EXT_H
#define BSP_I2C_EXT_H

typedef struct eeprom_data
{
	u16 checkSum;	
    s8  pcb_version[32];	
	u8  product_date[4];
	u8  mac_addr1[6];
	u8  mac_addr2[6];
	u8  device_id[32];
}T_EEPROM_TABLE;

typedef struct
{
    UINT32  initialized;
    UINT32  Voltage;
    UINT32  Current;
    UINT32  Tx_Power;
    UINT32  Rx_Power;
}FiberInfoLimint;

typedef struct
{
    u8    los;
    float temper;
    float vol;
    float current;
    float tx_power;
    float rx_power;
    u32  speed;
    float vendor_name[16];
}fiber_info;

typedef enum EEPROM_ADDRESS
{
    EEPROM_ADDRESS_MIN = 0x100,
    EEPROM_ADDRESS_CRC = EEPROM_ADDRESS_MIN,
    EEPROM_ADDRESS_MAC_ADDR1 = 0x26,
    EEPROM_ADDRESS_MAC_ADDR2 = 0x2c,
    EEPROM_ADDRESS_DEVICE_ID = 0x110,
    EEPROM_ADDRESS_BOARD_TYPE = 0x120,
    EEPROM_ADDRESS_PRODUCT_SN = 0x160,
    EEPROM_ADDRESS_MANUFACTRUER = 0x180,
    EEPROM_ADDRESS_PRODUCT_DATE = 0x18c,
    EEPROM_ADDRESS_SATELLITE_RECEIVER = 0x190,
    //EEPROM_ADDRESS_FAN_INITIAL_SPEED = 0x19c,
    EEPROM_ADDRESS_TEMPERATURE_THRESHOLD = 0x19e,
    EEPROM_ADDRESS_FAN_INITIAL_SPEED = 0x200,
    EEPROM_ADDRESS_MAX = EEPROM_ADDRESS_FAN_INITIAL_SPEED
}EEPROM_ADDRESS_ENUM;

typedef struct EEPROM_PAR
{
    u16 checkSum;
    s8  device_id[16];
    s8  board_type[32];	
    u8  mac_addr1[6];
    u8  mac_addr2[6];
    s8  product_sn[32];
    s8  manufacturer[12];
    u8  product_date[4];
    s8  satellite_receiver[12];
    u16  fan_initialspeed[3];
    s8  temperature_threshold[2];
}EEPROM_PAR_STRU;

/********************************************************************************
* 函数名称: bsp_i2c_init							
* 功    能:  初始化I2C                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明:  板级外设初始化时调用
*********************************************************************************/
Export void gpio_i2c_init(void);
Export s32 bsp_i2c_init(void);

/********************************************************************************
* 函数名称: eeprom							
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
Export s32 bsp_read_eeprom(u8 *u8pread_data,u32 u32len,u16 u16addr);
Export s32 bsp_write_eeprom(u8 *u8pwrite_data,u32 u32len, u16 u16addr);

/********************************************************************************
* 函数名称:   RTC							
* 功    能:                                    
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明:  
*********************************************************************************/
Export s32 bsp_get_rtc(u8 *u8prtc_buf);
Export s32 bsp_set_rtc(u8 *u8prtc_buf);

/********************************************************************************
* 函数名称: temp						
* 功    能:                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明:  
*********************************************************************************/
Export s32 bsp_read_temp(u8 u8temp_port,u8 u8temp_measure_point,s8 *u8pread_temp_data);

/********************************************************************************
* 函数名称: bsp_spi_init							
* 功    能:  初始化SPI                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明:  板级外设初始化时调用
*********************************************************************************/
Export void bsp_current_monitor_init(void);
Export s32 bsp_read_bus_voltage(f32 *pf32voltage);
Export s32 bsp_read_current(f32 *pf32current);
Export s32 bsp_read_power(f32 *pf32power);

/********************************************************************************
* 函数名称:  FAN							
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
Export s32 bsp_get_fan_speed(u8 u8fan_id,u16 *pu16fan_speed);
Export s32 bsp_set_fan_speed(u8 u8fan_id,u8 u8fan_pwmval);

/********************************************************************************
* 函数名称: GPIO_I2C sfp						
* 功    能:                                    
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明:  
*********************************************************************************/
Export s32 bsp_get_spf_reg(u8 u8sfp_cs,u8 u8spf_reg,u8 *u8read_spf_data,u32 u8byte_num);

Export s32 bsp_read_sfp(u8 u8sfp_cs,u8 u8sfp_device_ID,u8 u8address,u8 *pu8read_data,u32 u8byte_num);
Export s32 bsp_write_sfp(u8 u8sfp_cs,u8 u8sfp_device_ID,u8 u8address,u8 *pu8write_data,u32 u8byte_num );

Export float bsp_get_temperature(void);


Export s32 bsp_set_bbu_deviceid(const char *deviceid);
Export s32 bsp_set_bbu_boardtype(const char *boardtype);
Export s32 bsp_set_bbu_macaddr1(const u8 *macaddr1);
Export s32 bsp_set_bbu_macaddr2(const u8 *macaddr2);
Export s32 bsp_set_bbu_productsn(const s8 *productsn);
Export s32 bsp_set_bbu_manufacturer(const char *manufacturer);
Export s32 bsp_set_bbu_productdate(const u8 *productdate);
Export s32 bsp_set_bbu_satellitereceiver(const char *satellitereceiver);
Export s32 bsp_set_bbu_temperaturethreshold(const s8 *temperaturethreshold);

Export s32 bsp_get_bbu_deviceid(void);
Export s32 bsp_get_bbu_boardtype(void);
Export s32 bsp_get_bbu_macaddr1(void);
Export s32 bsp_get_bbu_macaddr2(void);
Export s32 bsp_get_bbu_productsn(void);
Export s32 bsp_get_bbu_manufacturer(void);
Export s32 bsp_get_bbu_productdate(void);
Export s32 bsp_get_bbu_satellitereceiver(void);
Export s32 bsp_get_bbu_temperaturethreshold(void);

Export u16 bsp_eeprom_calc_crc(u8 *pData);
Export s32 bsp_eeprom_set_crc(u16 crc);
Export s32 bsp_eeprom_get_crc(void);

#if 0
Export s32 bsp_get_bbu_deviceid(char *buffer_deviceid, int len);
Export s32 bsp_get_bbu_pcbversion(char *buffer_version, int len);
#endif

#endif
/******************************* 头文件结束 ********************************/

