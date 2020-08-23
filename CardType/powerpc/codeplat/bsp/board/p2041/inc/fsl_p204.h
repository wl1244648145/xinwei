/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bbu_config.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_P2041_H
#define BSP_P2041_H
#include "../../../com_inc/bsp_types.h"
#ifdef CPU_FSL_SYS_BIT32_WIDTH
#define P2041_LEN  (16<<20)
#define P2041_BASE (0xfe000000)
#define FPGA_LEN  (8192)
#define FPGA_BASE (0xf2000000)

#define EPLD_LEN  (8192)
#define EPLD_BASE (0xf1000000)
#else
#define P2041_LEN  (16<<20)
#define P2041_BASE (0xffe000000)
#define FPGA_LEN  (8192)
#define FPGA_BASE (0xff2000000)

#define EPLD_LEN  (8192)
#define EPLD_BASE (0xff1000000)
#endif


#define P2041_I2C1               (0)
#define P2041_I2C2               (1)
#define P2041_I2C3               (2)
#define P2041_I2C4               (3)

#define CONFIG_SYS_I2C_SLAVE		  (0x7F)


#endif
/******************************* 头文件结束 ********************************/
