/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw.c 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>

/**************************** 私用头文件* **********************************/
#include "bsp_types.h"
#include "../inc/bsp_ethsw.h"
#include "../inc/bsp_ethsw_bcm5389.h"
#include "../inc/bsp_ethsw_init.h"

/******************************* 局部宏定义 *********************************/


/*********************** 全局变量定义/初始化 **************************/

/************************** 局部常数和类型定义 ************************/



/*************************** 局部函数原型声明 **************************/

/************************************ 函数实现 ************************* ****/
/********************************************************************************
* 函数名称: bsp_ethsw_init							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	int								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 由ioctl 控制调试输出开关
*********************************************************************************/
s32  bsp_ethsw_init(void)
{
    s32 s32Rv;

    /* 初始化SPI接口 */
printf("bsp_ethsw_init..\n"); 
#if 0    
    ethsw_spi_init();
    
    ethsw_spi_test(0x3);

   /* 初始化ARL表映射关系全局变量 */
    ethsw_arl_map_init();
#endif   
    /* 初始化BCM5389芯片 */
    s32Rv = ethsw_bcm5389_init();
    if (0 != s32Rv)
    {
        printf("SORRY, ethsw_bcm5389_init failed!\n");    
        return (E_ETHSW_INIT_FAILED);
    }
    
    /* 初始化其他全局变量,如Trunking,To BE DONE */
    
    return (s32Rv);        /*失败返回*/
}

/******************************* 源文件结束 ********************************/
