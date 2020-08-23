/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bbu_config.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_EPLD_EXT_H
#define BSP_EPLD_EXT_H


#define CPLD_VAR    0x0
#define CPLD_PCB_VAR     0x4

#define CPLD_CLOCK_STAT_REG     0x5
#define CPLD_LED_CTRL_H_REG     0x6
#define CPLD_LED_CTRL_L_REG     0x7

#define CPLD_BOARD_RESET_CTRL_REG    0x8
#define CPLD_BOARD_RESET_FLAG_REG     0x9
#define CPLD_CHIP_RESET_CTRL_H_REG     0xA
#define CPLD_CHIP_RESET_CTRL_L_REG     0xB

#define RESET_CAUSE_POWER_ON	0x80 /*power on reset标志*/
#define RESET_CAUSE_WATCHDOG	0x40 /*watch dog reset标志*/
#define RESET_CAUSE_BUTTON		0x20 /*全局按钮 reset标志*/
#define RESET_CAUSE_BOARD		0x10 /*整板reset标志*/
#define RESET_CAUSE_PPC			0x0  /*PPC自复位 reset标志*/


#define CPLD_EXT_WD_CTRL_REG     0xC
#define CPLD_FPGA_CFG_REG     0xD
#if 1
#define CPLD_AFC_CTRL_REG               0xF
/*bit5：用于AFC初始相位对齐，1对齐，0 freerun。其他bit暂保留*/
/*bit6：用于AFC初始相位对齐，1对齐，0 freerun。其他bit暂保留*/

#define CPLD_AFC_FD_1S_CNT_3_REG     0x10
#define CPLD_AFC_FD_1S_CNT_2_REG    0x11
#define CPLD_AFC_FD_1S_CNT_1_REG   0x12
#define CPLD_AFC_FD_1S_CNT_0_REG  0x13

#define CPLD_AFC_FD_20S_CNT_3_REG     0x14
#define CPLD_AFC_FD_20S_CNT_2_REG    0x15
#define CPLD_AFC_FD_20S_CNT_1_REG   0x16
#define CPLD_AFC_FD_20S_CNT_0_REG  0x17

#define CPLD_AFC_PD_AHEAD_CNT_1_REG  0x18
#define CPLD_AFC_PD_AHEAD_CNT_0_REG   0x19
#define CPLD_AFC_PD_LAG_CNT_1_REG    0x1A
#define CPLD_AFC_PD_LAG_CNT_0_REG     0x1B


#define CPLD_CHIP_STATUS_REG     0x1C
#define CPLD_DRVIF_STATUS_REG     0x1D
#define CPLD_POWER_STATUS_2_REG     0x1E
#define CPLD_POWER_STATUS_1_REG     0x1F
#define CPLD_POWER_STATUS_0_REG     0x20


#define CPLD_GPS_MSG_3        0x36
#define CPLD_GPS_MSG_2        0x37
#define CPLD_GPS_MSG_1        0x38
#define CPLD_GPS_MSG_0        0x39
#define CPLD_GPS_MSG_FLAG        0x3a
#endif

Export u8 bsp_cpld_read_reg(u8 u8Reg_offset);
Export void bsp_cpld_write_reg(u8 u8Reg_offset,u8 u8Data);
Export s32 bsp_update_cpld(void);
Export void bsp_start_watchdog(void);

Export void bsp_led_run_on(void);
Export void bsp_led_run_off(void);
Export void bsp_led_alarm_on(void);
Export void bsp_led_alarm_off(void);
Export void bsp_led_fan_on(void);
Export void bsp_led_fan_off(void);
Export void bsp_led_gps_on(void);
Export void bsp_led_gps_off(void);
Export void bsp_led_init(void);
Export u8 bsp_get_resetcause(void);

#endif
/******************************* 头文件结束 ********************************/

