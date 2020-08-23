/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_fpga.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_FPGA_H
#define BSP_FPGA_H

//#define FPGA_BASE               0xd1000000    /*LCS1*/
//#define FPGA_RMSIZE             0x00000100 
#define GLOBAL_UTILITIE_REG_BASE 0XE00E0000 
#define GLOBAL_UTILITIE_REG_SIZE 0X1000 
#define FPGA_LOAD_RMSIZE	4
#define	DSU_DELAY			1		/* 10ns - Data setup time before rising edge on DCLK */
#define	CH_DELAY			1       /* 10ns - DCLK high time */

volatile u32 *g_utilities_reg = NULL;
volatile u32 *g_fpga_gpiocr = NULL;
volatile u32 *g_fpga_gpindr = NULL;
volatile u32 *g_fpga_gpoutdr = NULL;
//volatile u8  *g_fpga = NULL;         /*fpga映射地址*/
volatile u32 *g_fpga_reset = NULL;
#if 1
const s8 fpga_bin_name[]= "/mnt/btsa/FPGA_BBU_20M.rbf";
#else
const s8 fpga_bin_name[]= "/tmp/download/BBU_20M.rbf";
#endif
#endif
/******************************* 头文件结束 ********************************/

