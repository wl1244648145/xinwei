/*******************************************************************************
* COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* �ļ�����:  bsp_afc_phase.c
* ��    ��:  
* ��    ��:  V0.1
* ��д����:  
* ˵    ��:
* �޸���ʷ:
* �޸�����           �޸���  BugID/CRID     �޸�����
*------------------------------------------------------------------------------
*                                                         �����ļ�
*
*
*******************************************************************************/

/******************************* �����ļ����� *********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
/**************************** ˽��ͷ�ļ�* **********************************/
#include "bsp_types.h"
#include "../inc/bsp_afc.h"
#include "../inc/bsp_afc_phase.h"
#include "bsp_epld_ext.h"

/******************************* �ֲ��궨�� ***********************************/
/******************************* ȫ�ֱ�������/��ʼ�� **************************/
extern u32 g_u32PhaseDifErrCnt;
extern u16 g_u16AheadData;
extern u16 g_u16LagData;
extern u32 g_u32Afcprint_level;

/******************************* �ֲ����������Ͷ��� ***************************/
/******************************* �ֲ�����ԭ������ *****************************/
/******************************* ����ʵ�� *************************************/
/*******************************************************************************
* ��������: afc_freq_count_rst
* ��    ��:
* ����ĵ�:
* ��������: extern
* ��    ��:
* ��������          ����        ����/���       ����
* ����ֵ:
* ��
* ˵   ��:
*******************************************************************************/
void afc_freq_count_rst(void)
{
	bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0x40);
	usleep(1000*8);
	bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0x00);
}

/*******************************************************************************
* ��������: afc_freq_count_1s
* ��    ��: 
* ����ĵ�:
* ��������: extern
* ��    ��:
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
* ��������: afc_freq_count_20s
* ��    ��: 
* ����ĵ�:
* ��������: extern
* ��    ��:
* ��������          ����        ����/���       ����
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
		//afc_dbg("INFO : 20sƵ�ʼ���������ֵ = %d\n", (int)u32data);
	}
}
/*******************************************************************************
* ��������: phasedif_get
* ��    ��: 
* ����ĵ�:
* ��������: extern
* ��    ��:
* ��������          ����        ����/���       ����
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
		printf("��λ�쳣: ��ǰ-- %d     �ͺ�-- %d \n",u16AheadData,u16LagData);
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

/******************************* Դ�ļ����� ***********************************/

