/*******************************************************************************
* COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 文件名称:  bsp_afc_phase.c
* 功    能:  
* 版    本:  V0.1
* 编写日期:  
* 说    明:
* 修改历史:
* 修改日期           修改人  BugID/CRID     修改内容
*------------------------------------------------------------------------------
*                                                         创建文件
*
*
*******************************************************************************/

/******************************* 包含文件声明 *********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
/**************************** 私用头文件* **********************************/
#include "bsp_types.h"
#include "../inc/bsp_afc.h"
#include "../inc/bsp_afc_phase.h"
#include "bsp_epld_ext.h"

/******************************* 局部宏定义 ***********************************/
/******************************* 全局变量定义/初始化 **************************/
extern u32 g_u32PhaseDifErrCnt;
extern u16 g_u16AheadData;
extern u16 g_u16LagData;
extern u32 g_u32Afcprint_level;

/******************************* 局部常数和类型定义 ***************************/
/******************************* 局部函数原型声明 *****************************/
/******************************* 函数实现 *************************************/
/*******************************************************************************
* 函数名称: afc_freq_count_rst
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
* 返回值:
* 无
* 说   明:
*******************************************************************************/
void afc_freq_count_rst(void)
{
	bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0x40);
	usleep(1000*8);
	bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0x00);
}

/*******************************************************************************
* 函数名称: afc_freq_count_1s
* 功    能: 
* 相关文档:
* 函数类型: extern
* 参    数:
*******************************************************************************/
void afc_freq_count_1s(vu32 *pu32data1,vs32 *ps32data2)
{
	u32 u32data1 = 0;
	s32 s32data2 = 0;

	*pu32data1 = 0;
	*ps32data2 = 0;
	u32data1 = (bsp_cpld_read_reg(CPLD_AFC_FD_1S_CNT_3_REG) << 24) |
		(bsp_cpld_read_reg(CPLD_AFC_FD_1S_CNT_2_REG) << 16) |
		(bsp_cpld_read_reg(CPLD_AFC_FD_1S_CNT_1_REG) << 8)   |
		(bsp_cpld_read_reg(CPLD_AFC_FD_1S_CNT_0_REG));

	u32data1 &= 0x3ffffff;
	*pu32data1 = u32data1;
	s32data2 = (s32)u32data1 -COUNTER_61M44;
	
	if (abs(s32data2) > 400)
	{
		s32data2 = 0;
	}	
	*ps32data2 = s32data2;
	
}
/*******************************************************************************
* 函数名称: afc_freq_count_20s
* 功    能: 
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
void afc_freq_count_20s(vu32 *pu32data)
{
	u32 u32data = 0;

	*pu32data = 0;
	u32data = (bsp_cpld_read_reg(CPLD_AFC_FD_20S_CNT_3_REG) << 24) |
			(bsp_cpld_read_reg(CPLD_AFC_FD_20S_CNT_2_REG) << 16) |
			(bsp_cpld_read_reg(CPLD_AFC_FD_20S_CNT_1_REG) << 8)   |
			(bsp_cpld_read_reg(CPLD_AFC_FD_20S_CNT_0_REG));
	
	u32data &= 0x7fffffff;
	*pu32data = u32data;

	if(AFC_PRINT_LEVEL_DEBUG < g_u32Afcprint_level)
	{
		//afc_dbg("INFO : 20s频率计数器计数值 = %d\n", (int)u32data);
	}
}
/*******************************************************************************
* 函数名称: phasedif_get
* 功    能: 
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
void phasedif_get(vs32 *ps32Data)
{
	u8 u8AheadHigh = 0;
	u8 u8AheadLow = 0;
	u16 u16AheadData;

	u8 u8LagHigh = 0;
	u8 u8LagLow = 0;
	u16 u16LagData;
	u16 u16TemHigh = 0;
	u16 u16TemLow = 0;

	*ps32Data = 0;
	u8AheadHigh = bsp_cpld_read_reg(CPLD_AFC_PD_AHEAD_CNT_1_REG);
	u8AheadLow = bsp_cpld_read_reg(CPLD_AFC_PD_AHEAD_CNT_0_REG);
	u8LagHigh = bsp_cpld_read_reg(CPLD_AFC_PD_LAG_CNT_1_REG);
	u8LagLow = bsp_cpld_read_reg(CPLD_AFC_PD_LAG_CNT_0_REG);

	u16TemHigh = (((u16)u8AheadHigh)<<8) & ((u16)(0xFF00));
	u16TemLow = (u16)u8AheadLow;
	u16AheadData = (u16)(u16TemHigh | u16TemLow);
	
	if(AFC_PRINT_LEVEL_ALL < g_u32Afcprint_level)
	{
		afc_dbg( "INFO : u16AheadData = %d\n", u16AheadData);
	}

	u16TemHigh = (((u16)u8LagHigh)<<8) & ((u16)(0xFF00));
	u16TemLow = (u16)u8LagLow;
	u16LagData = (u16)(u16TemHigh | u16TemLow);
	
	if(AFC_PRINT_LEVEL_ALL < g_u32Afcprint_level)
	{
		afc_dbg( "INFO : u16LagData = %d\n", u16LagData);
	}

	g_u16AheadData = u16AheadData;
	g_u16LagData = u16LagData;

	if (0 != u16AheadData && 0 != u16LagData)
	{
		g_u32PhaseDifErrCnt++;
		printf("相位异常: 超前-- %d     滞后-- %d \n",u16AheadData,u16LagData);
		printf("35--%d  36--%d  37--%d 38--%d\n",bsp_cpld_read_reg(CPLD_AFC_PD_AHEAD_CNT_1_REG),bsp_cpld_read_reg(CPLD_AFC_PD_AHEAD_CNT_0_REG),bsp_cpld_read_reg(CPLD_AFC_PD_LAG_CNT_1_REG),bsp_cpld_read_reg(CPLD_AFC_PD_LAG_CNT_0_REG));
	}

	if (u16AheadData >= u16LagData)
	{
		*ps32Data =  (s32)((u32)u16AheadData);
		if(AFC_PRINT_LEVEL_ALL < g_u32Afcprint_level)
		{
			afc_dbg( "INFO : (s32)((u32)u16AheadData) = %d\n", (s32)((u32)u16AheadData));
		}
	}
	else
	{
		*ps32Data = (s32)((s32)((u32)(u16LagData))*(-1)); 
	}
}

/******************************* 源文件结束 ***********************************/

