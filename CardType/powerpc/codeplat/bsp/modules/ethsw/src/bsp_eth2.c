/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_arl.c 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <string.h>

/**************************** 私用头文件* **********************************/
#include "bsp_types.h"
#include "../inc/bsp_ethsw_bcm5389.h"
#include "../inc/bsp_ethsw_arl.h"
#include "../inc/bsp_ethsw_spi.h"

/******************************* 局部宏定义 *********************************/

#define swap32(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | ((x)>>24))

/*********************** 全局变量定义/初始化 **************************/
extern u32 g_u32PrintRules;

/************************** 局部常数和类型定义 ************************/



/*************************** 局部函数原型声明 **************************/

/************************************ 函数实现 ************************* ****/
/*******************************************************************************
* 函数名称: save_arl_struct
* 函数功能: 根据ARL_TABLE_MAC_VID_ENTRY0/1,ARL_TABLE_DATA_ENTRY0/1,
*           ARL_TABLE_SEARCH_MAC_RESULT/ARL_TABLE_SEARCH_DATA_RESULT寄存器中读出
*           的相关信息,转化为STRU_ETHSW_ARL_TABLE结构中相关值
* 相关文档:
* 函数参数:
* 参数名称:        类型                           输入/输出   描述
* pstruMacEntry    STRU_ETHSW_ARL_MAC_VID_ENTRY*  输入        从MAC_VID/MAC_RESULT
                                                              寄存器读出的内容(8字节)
* pstruDataEntry   STRU_ETHSW_ARL_DATA_ENTRY*     输入        从DATA寄存器读出的内
                                                              容(2字节)
* pstruArlTable    STRU_ETHSW_ARL_TABLE*          输出        将输入信息转化后保存
*
* 返回值:   (各个返回值的注释)
* 函数类型: <回调、中断、可重入（阻塞、非阻塞）等函数必须说明类型及注意事项>
* 函数说明:（以下述列表的方式说明本函数对全局变量的使用和修改情况，以及本函数
*           未完成或者可能的改动）
*
* 1.被本函数改变的全局变量列表
* 2.被本函数改变的静态变量列表
* 3.引用但没有被改变的全局变量列表
* 4.引用但没有被改变的静态变量列表
*
* 修改日期    版本号   修改人  修改内容
* -----------------------------------------------------------------
*******************************************************************************/
void save_arl_struct(const STRU_ETHSW_ARL_MAC_VID *pstruMacEntry, const STRU_ETHSW_ARL_DATA *pstruDataEntry, STRU_ETHSW_ARL_TABLE *pstruArlTable)
{
	memcpy(pstruArlTable->u8MacAddr, pstruMacEntry->u8MacAddr, 6);
	pstruArlTable->valid = pstruDataEntry->valid;
	pstruArlTable->VlanId = pstruMacEntry->VlanId;
	pstruArlTable->StaticEntry = pstruDataEntry->StaticEntry;
	pstruArlTable->age = pstruDataEntry->age;
	pstruArlTable->PortId = pstruDataEntry->PortId;
	return;
	
}
/*******************************************************************************
* 函数名称: ethsw_lookup_arl_entry
* 函数功能: 查找指定MAC地址和VLAN ID的ARL表,并将查找出来的ARL表保存到输出变量中
* 相关文档:
* 函数参数:
* 参数名称:      类型                    输入/输出  描述
* pu8MacAddr     u8*                     输入       将要读取的mac地址数组的首地址
* pstruMacEntry  STRU_ETHSW_ARL_MAC_VID* 输出       将查找到的ARL表的MAC地址VID存
                                                    放在此结构中,,第一条对应于ENTRY0,
                                                    第二条对应于条对应于ENTRY1
* pstruDataEntry STRU_ETHSW_ARL_DATA*    输出       将查找到的ARL表的PortId有效静态
                                                    等信息存放在此结构中,,第一条对
                                                    应于ENTRY0,第二条对应于条对应于
                                                    ENTRY1
*
* 返回值:   (各个返回值的注释)
* 函数类型: <回调、中断、可重入（阻塞、非阻塞）等函数必须说明类型及注意事项>
* 函数说明:（以下述列表的方式说明本函数对全局变量的使用和修改情况，以及本函数
*           未完成或者可能的改动）
*
* 1.被本函数改变的全局变量列表
* 2.被本函数改变的静态变量列表
* 3.引用但没有被改变的全局变量列表
* 4.引用但没有被改变的静态变量列表
*
* 修改日期    版本号   修改人  修改内容
* -----------------------------------------------------------------
*******************************************************************************/
s32 ethsw_lookup_arl_entry(const u8 *pu8MacAddr, STRU_ETHSW_ARL_MAC_VID *pstruMacEntry, STRU_ETHSW_ARL_DATA *pstruDataEntry)
{
	u32   u32i;              /* 循环变量 */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;  /* 临时变量,查找时的控制结构 */
       u8 test[8];
	if ((NULLPTR == pu8MacAddr) || (NULLPTR == pstruMacEntry) || (NULLPTR == pstruDataEntry))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_lookup_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}
#if 0
	printf("\n0x%x\n",*pu8MacAddr);
	printf("\n0x%x\n",*(pu8MacAddr+1));
	printf("\n0x%x\n",*(pu8MacAddr+2));
	printf("\n0x%x\n",*(pu8MacAddr+3));
	printf("\n0x%x\n",*(pu8MacAddr+4));
	printf("\n0x%x\n",*(pu8MacAddr+5));
#endif
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, MAC_ADDRESS_INDEX, pu8MacAddr, SIX_BYTE_WIDTH));

	//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, MAC_ADDRESS_INDEX,(u8 *)test, SIX_BYTE_WIDTH));
	//printf("\ntest is 0x%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5]);


	struArlRWCtrl.ArlStart = 1;/* 开始读写标记 */
	struArlRWCtrl.ArlRW = 1;   /* 读标记 */
	struArlRWCtrl.rsvd = 0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	
	if (u32i >= ETHSW_TIMEOUT_VAL) /* 超时 */
	{
		//if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_lookup_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	else  
	{
		/* 读取MAC_VID_ENRTY0的内容,读取8字节内容 */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)pstruMacEntry, EIGHT_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));
		//	printf("\ntest is 0x%x-%x-%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5],test[6],test[7]);


		/* 读取DATA_ENRTY0的内容,读取2字节内容 */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)pstruDataEntry, FOUR_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)test, FOUR_BYTE_WIDTH));

		/* 读取MAC_VID_ENRTY1的内容,读取8字节内容 */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)(pstruMacEntry + 1), EIGHT_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1,(u8 *)test, EIGHT_BYTE_WIDTH));
		//	printf("\ntest is 0x%x-%x-%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5],test[6],test[7]);

		/* 读取DATA_ENRTY1的内容,读取2字节内容 */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1,(u8 *)pstruDataEntry, FOUR_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1,(u8 *)test, FOUR_BYTE_WIDTH));
		//	printf("\ntest is 0x%x-%x\n",test[0],test[1]);

	}
	return (E_ETHSW_OK);
	
}

/*******************************************************************************
* 函数名称: ethsw_add_arl_entry
* 函数功能: 添加一条ARL表:将指定MAC地址(VLAN ID)的ARL表写入到芯片内部集成Memory中
* 相关文档:
* 函数参数:
* 参数名称:      类型                    输入/输出  描述
* u8CastType     u8                      输入        单播组播标记,0:单播,1:组播
* pstruMacPort   STRU_ETHSW_MAC_PORT*    输入        将此结构对应MAC地址与端口插
                                                     入到ARL表中
*
* 返回值:  s32
*          0:成功
*          其它错误
* 函数类型: <回调、中断、可重入（阻塞、非阻塞）等函数必须说明类型及注意事项>
* 函数说明:（以下述列表的方式说明本函数对全局变量的使用和修改情况，以及本函数
*           未完成或者可能的改动）
*
* 1.被本函数改变的全局变量列表
* 2.被本函数改变的静态变量列表
* 3.引用但没有被改变的全局变量列表
* 4.引用但没有被改变的静态变量列表
*
* 修改日期    版本号   修改人  修改内容
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_add_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort)
{
	s32                     s32Rv1;
	s32                     s32Rv2;           /* 返回值 */
	u32                     u32i;             /* 循环变量 */
	u32                     u32Entry0 = 0;
	u32                     u32Entry1 = 0;    /* 标记,当为1时表示写相应Entry寄存器 */
	STRU_ETHSW_ARL_MAC_VID  struMacEntry[2];  /* 临时变量,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA     struDataEntry[2]; /* 临时变量,DATA Entry */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;    /* 临时变量,ARL表读写操作结构 */
    u8 test[8];
	u32 u32regval = 0;

	if (NULLPTR == pstruMacPort)
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_add_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	/* 清0临时变量 */
	memset((void *)struMacEntry, 0x00, sizeof(struMacEntry));
	memset((void *)struDataEntry, 0x00, sizeof(struDataEntry));

	RETURN_IF_ERROR(ethsw_lookup_arl_entry(pstruMacPort->u8MacAddr, struMacEntry, struDataEntry));

	s32Rv1 = memcmp(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);
	s32Rv2 = memcmp(struMacEntry[1].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (0 == s32Rv1)       /* 已经存在一条MAC地址相同的条目,直接覆盖 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry0!\n");
			if (0 == s32Rv2)   /* 2条相同的MAC地址 */
			{
				printf("ethsw_add_arl_entry:Two Same MAC ARLs have EXISTED!\n");
				return (E_ETHSW_FAIL);
			}
		}
		u32Entry0 = 1;
	}
	else if (0 == s32Rv2)   /* 已经存在一条MAC地址相同的条目,直接覆盖 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry1!\n");
		}
		u32Entry1 = 1;
	}
	else                    /* 没有MAC地址相同的表项 */
	{
		if (0 == struDataEntry[0].valid)
		{
			u32Entry0 = 1;  /* Entry0无效,选择写入Entry0 */
		}
		else if (0 == struDataEntry[1].valid)
		{
			u32Entry1 = 1;  /* Entry0有效,Entry1无效,选择写入Entry1 */
		}
		else  /* Entry0,Entry1均有效 */
		{
			if (0 != (g_u32PrintRules & ETHSW_PRINT_EXCEPTION))
			{
				printf("ethsw_add_arl_entry:ARL Bucket have no Space!\n");
			}
			return (E_ETHSW_BUCKET_FULL);
		} /* end of else Entry0,1均有效 */
	} /* end of else,没有MAC地址相同的表项 */

	memcpy(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (1 == u32Entry0)      /* 选择的是ENTRY0 */
	{
		/* 写入ARL TABLE MAC/VID Entry0 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
        RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));
		//printf("\ntest is 0x%x-%x-%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5],test[6],test[7]);
		u32regval = (pstruMacPort->u16PortId<<6)|0x38;
        u32regval = swap32(u32regval);
		//printf("u32regval = 0x%x\r\n", u32regval);
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0, (u8 *)&u32regval, FOUR_BYTE_WIDTH));
		//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)&struDataEntry[0], FOUR_BYTE_WIDTH));
		//printf("\ntest is 0x%x-%x-%x-%x\n",struDataEntry[0].PortId,struDataEntry[0].StaticEntry,struDataEntry[0].age,struDataEntry[0].valid);
		
	}
	else if (1 == u32Entry1) /* 选择的是ENTRY1 */
	{
		/* 写入ARL TABLE MAC/VID Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
        RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));
		u32regval = (pstruMacPort->u16PortId<<6)|0x38;
		printf("u32regval = 0x%x\r\n", u32regval);
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)&u32regval, FOUR_BYTE_WIDTH));
		//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)&struDataEntry[1], FOUR_BYTE_WIDTH));
		//printf("\ntest is 0x%x-%x-%x-%x\n",struDataEntry[1].PortId,struDataEntry[1].StaticEntry,struDataEntry[1].age,struDataEntry[1].valid);
	}

	struArlRWCtrl.ArlStart = 1; /* 读写开始标记 */
	struArlRWCtrl.ArlRW = 0;    /* 写标记 */
	struArlRWCtrl.rsvd = 0;
	
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	if (u32i >= ETHSW_TIMEOUT_VAL) /* 超时 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_add_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	
	return (E_ETHSW_OK);
	
}


s32 ethsw_remove_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort)
{
	s32                     s32Rv1;
	s32                     s32Rv2;           /* 返回值 */
	u32                     u32i;             /* 循环变量 */
	u32                     u32Entry0 = 0;
	u32                     u32Entry1 = 0;    /* 标记,当为1时表示写相应Entry寄存器 */
	STRU_ETHSW_ARL_MAC_VID  struMacEntry[2];  /* 临时变量,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA     struDataEntry[2]; /* 临时变量,DATA Entry */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;    /* 临时变量,ARL表读写操作结构 */
       u8 test[8];

	if (NULLPTR == pstruMacPort)
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_add_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	/* 清0临时变量 */
	memset((void *)struMacEntry, 0x00, sizeof(struMacEntry));
	memset((void *)struDataEntry, 0x00, sizeof(struDataEntry));

	RETURN_IF_ERROR(ethsw_lookup_arl_entry(pstruMacPort->u8MacAddr, struMacEntry, struDataEntry));

	s32Rv1 = memcmp(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);
	s32Rv2 = memcmp(struMacEntry[1].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (0 == s32Rv1)       /* 已经存在一条MAC地址相同的条目,直接覆盖 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry0!\n");
			if (0 == s32Rv2)   /* 2条相同的MAC地址 */
			{
				printf("ethsw_add_arl_entry:Two Same MAC ARLs have EXISTED!\n");
				return (E_ETHSW_FAIL);
			}
		}
		
		u32Entry0 = 1;
	}
	else if (0 == s32Rv2)   /* 已经存在一条MAC地址相同的条目,直接覆盖 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry1!\n");
		}
		u32Entry1 = 1;
	}
	else                    /* 没有MAC地址相同的表项 */
	{
		if (0 == struDataEntry[0].valid)
		{
			u32Entry0 = 1;  /* Entry0无效,选择写入Entry0 */
		}
		else if (0 == struDataEntry[1].valid)
		{
			u32Entry1 = 1;  /* Entry0有效,Entry1无效,选择写入Entry1 */
		}
		else  /* Entry0,Entry1均有效 */
		{
			if (0 != (g_u32PrintRules & ETHSW_PRINT_EXCEPTION))
			{
				printf("ethsw_add_arl_entry:ARL Bucket have no Space!\n");
			}
			return (E_ETHSW_BUCKET_FULL);
		} /* end of else Entry0,1均有效 */
	} /* end of else,没有MAC地址相同的表项 */

	memcpy(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (1 == u32Entry0)      /* 选择的是ENTRY0 */
	{
		/* 写入ARL TABLE MAC/VID Entry0 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));

RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));
//printf("\ntest is 0x%x-%x-%x-%x-%x-%x-%x-%x\n",test[0],test[1],test[2],test[3],test[4],test[5],test[6],test[7]);
		
              test[1] = 0x60;


		//RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0, (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0, (u8 *)test, TWO_BYTE_WIDTH));

//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)test, TWO_BYTE_WIDTH));
//printf("\ntest is 0x%x-%x\n",test[0],test[1]);
		
	}
	else if (1 == u32Entry1) /* 选择的是ENTRY1 */
	{
		/* 写入ARL TABLE MAC/VID Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
              RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,(u8 *)test, EIGHT_BYTE_WIDTH));

              test[1] = 0x60;
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)test, TWO_BYTE_WIDTH));
	}

	struArlRWCtrl.ArlStart = 1; /* 读写开始标记 */
	struArlRWCtrl.ArlRW = 0;    /* 写标记 */
	struArlRWCtrl.rsvd = 0;
	
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	if (u32i >= ETHSW_TIMEOUT_VAL) /* 超时 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_add_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	
	return (E_ETHSW_OK);
	
}


s32 ethsw_dump_arl_entry(u16 u16ArlNum, STRU_ETHSW_ARL_TABLE *pstruArlTable)
{
	u32                         u32i = 0;         /* 循环变量,用来标记指针是否越界 */
	u32                         u32Count = 0;  /* 计数器,用来标记读寄存器是否超时 */

	STRU_ETHSW_ARL_SEARCH_CTRL  struSearchCtrl; /* 临时变量,查找时的控制结构 */
	STRU_ETHSW_ARL_MAC_VID      struMacEntry;   /* 临时变量,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA         struDataEntry;  /* 临时变量,DATA Entry */

	struSearchCtrl.ArlStart = 1;
	struSearchCtrl.rsvd = 0;
       struSearchCtrl.valid = 0;
	u8 test;
	u8 readdata[8];
    u32 u32regval = 0;
	/* 开始搜索有效的ARL表 */
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&struSearchCtrl, ONE_BYTE_WIDTH));

	//test = 0x80;
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&test, ONE_BYTE_WIDTH));
	//RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&test, ONE_BYTE_WIDTH));

	//printf("\(u8 *)&struSearchCtrl is %d\n",*(u8 *)&struSearchCtrl);
	/* 轮询查找控制寄存器,判断搜索到的ARL表是否有效,搜索过程是否结束 */
	while (1) /* 无循环判断条件,靠内部条件跳出 */
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&struSearchCtrl, ONE_BYTE_WIDTH));
		//RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&test, ONE_BYTE_WIDTH));
		//printf("\ntest is %d\n",test);

		//printf("\(u8 *)&struSearchCtrl is %d\n",*(u8 *)&struSearchCtrl);

		/* 判断搜索到的ARL是否有效,如有效则保存,无效则计数以防超时 */
		if (0x1 == (struSearchCtrl.valid))       /* 搜索到一条有效ARL表,保存起来 */
		{
			u32Count = 0;  /* 计数器清0 */
			RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_MAC_RESULT, (u8 *)readdata, EIGHT_BYTE_WIDTH));
			printf("\nMAC : 0x%x-%x-%x-%x-%x-%x    ",readdata[5],readdata[4],readdata[3],readdata[2],readdata[1],readdata[0]);
			if(0 == (readdata[5] & 0x1))
	     	{
                RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_DATA_RESULT, (u8 *)readdata, FOUR_BYTE_WIDTH));
                u32regval = (readdata[3]<<24)|(readdata[2]<<16)|(readdata[1]<<8)|(readdata[0]);
                //printf("u32regval = 0x%x\r\n", u32regval);
                if(0 == (u32regval & 0x20))
                 {
                    printf("LEARNING!  ");
                 }
                 else
                 {
                    printf("    STATIC!    ");        /* Static */
                 }
                 printf("port : %d \n",(u32regval>>6)&0x1f);
            }
            else
            {
                RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_DATA_RESULT1, (u8 *)readdata, FOUR_BYTE_WIDTH));
                u32regval = (readdata[3]<<24)|(readdata[2]<<16)|(readdata[1]<<8)|(readdata[0]);
                printf("portmap : 0x%x \n",(u32regval>>6)&0xffff);
            }   			
		}
		else /* 0x00 == struSearchCtrl.valid,当前搜索到的ARL表无效 */
		{
			/* 继续搜索,计数器自增,并检验是否超时 */
			u32Count++;
			if (u32Count >= ETHSW_TIMEOUT_VAL)
			{
				/* 在进行查找的时候超时 */
				printf("\ntime out!!!\n");
				return (E_ETHSW_TIMEOUT);
			}
			
		}
		/* 检查搜索过程是否结束 */
		if (0x0 == (struSearchCtrl.ArlStart)) /* 搜索过程结束,停止循环 */
		{
			break;
		}
		/*else, 0x01 == struSearchCtrl.ArlStart,搜索过程未结束,do nothing */
	}
	
	return (E_ETHSW_OK);
}

/******************************* 源文件结束 ***********************************/

