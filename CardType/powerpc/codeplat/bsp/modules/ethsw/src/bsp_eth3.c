/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_init.c 
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
#include "../inc/bsp_ethsw_bcm5389.h"
#include "../inc/bsp_ethsw_port.h"
#include "../inc/bsp_ethsw_init.h"
#include "../inc/bsp_ethsw_spi.h"
#include "../inc/bsp_ethsw_arl.h"

/******************************* 局部宏定义 *********************************/


/*********************** 全局变量定义/初始化 **************************/
//u16 g_pVlan[10] = {0x1ff,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100};
u32 g_pVlan[10] = {0x1ffff,0x10000,0x10000,0x10000,0x10000,0x10000,0x10000,0x10000,0x10000,0x10000};
u32 g_u32InitFlag = 0x0;
extern u32 g_u32PrintRules;


/************************** 局部常数和类型定义 ************************/



/*************************** 局部函数原型声明 **************************/

/************************************ 函数实现 ************************* ****/
/*******************************************************************************
* 函数名称: ethsw_set_switch_mode
* 函数功能: 设置端口的交换功能,Enable/Disable芯片转发功能
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Enable      u8          输入        是否设置芯片的转发功能,0:关闭,1:打开
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
s32 ethsw_set_switch_mode(u8 u8Enable)
{
    STRU_ETHSW_SWITCH_MODE  struSwitchMode; /* 定义交换模式结构 */
    /* Read-Modified-Write */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    struSwitchMode.SwFwdEn = u8Enable;
    struSwitchMode.SwFwdMode = 1 - u8Enable;
    /* 写交换模式寄存器 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}
/*******************************************************************************
* 函数名称: ethsw_set_arl_multicast
* 函数功能: 设置ARL表是否支持Multicast功能
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Enable      u8          输入        设置芯片支持组播功能,0:不支持组播,1:支持组播
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
s32 ethsw_set_arl_multicast(u8 u8Enable)
{
    STRU_ETHSW_MCAST_CTRL  struMcastCtrl;    /* 定义组播控制结构 */
    u8 u8test;
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    struMcastCtrl.MulticastEn = u8Enable; /* 修改此位 */

    /* 写组播控制寄存器 */
    //RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    u8test = 0xff;
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));


    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&u8test, ONE_BYTE_WIDTH));
    printf("\nu8test : 0x%x\n",u8test);	
    



	
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_set_dlf_forward
* 函数功能: 对单播和组播DLF(目的地址查找失败)包的转发进行设置
* 相关文档:
* 函数参数:
* 参数名称:       类型        输入/输出   描述
* u8MultiDlfEn    u8          输入        1:组播包DLF包按照Multicast Lookup Fail
                                            Forward Map register设置进行转发
                                          0:组播包DLF包广播(flood)
* u32MultiPortMap u32         输入        u8MultiDlfEn为1时组播表DLF包的转发PortMap
* u8UniDlfEn      u8          输入        1:单播包DLF包按照Multicast Lookup Fail
                                            Forward Map register设置进行转发
                                          0:单播包DLF包广播(flood)
* u32UniPortMap   u32         输入        u16UniPortMap为1时单播表DLF包的转发PortMap
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
* 
*******************************************************************************/
s32 ethsw_set_dlf_forward(u8 u8MultiDlfEn, u16 u16MultiPortMap, u8 u8UniDlfEn, u16 u16UniPortMap)
{
    u16                    u16Temp;
    STRU_ETHSW_MCAST_CTRL  struMcastCtrl; /* 定义组播控制结构 */
    /* 只需要在使能组播表DLF转发规则时才需要设置MULTICAST LOOKUP FAIL FORWARD MAP寄存器 */
    if (1 == u8MultiDlfEn)
    {
        u16Temp = u16MultiPortMap;
    }
    else
    {
        u16Temp = 0;
    }
    /* 写MULTICAST LOOKUP FAIL FORWARD MAP寄存器 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, MUL_DLF_FW_MAP, (u8 *)&u16Temp, TWO_BYTE_WIDTH));

    /* 只需要在使能单播表DLF转发规则时才需要设置UNICAST LOOKUP FAIL FORWARD MAP寄存器 */
    if (1 == u8UniDlfEn)
    {
        u16Temp = u16UniPortMap;
    }
    else
    {
        u16Temp = 0;
    }
    /* 写UNICAST LOOKUP FAIL FORWARD MAP寄存器 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, UNI_DLF_FW_MAP, (u8 *)&u16Temp, TWO_BYTE_WIDTH));

    /* Read-Modified-Write */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    /* 对组播/单播转发规则使能标记进行设置 */
    struMcastCtrl.MlfFmEn = u8MultiDlfEn;
    struMcastCtrl.UniFmEn = u8UniDlfEn;
    /* 写组播控制寄存器 */
   RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_set_port_stat
* 函数功能: 设置端口状态
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8PortId        u8          输入        端口ID
* u8PhyScanEn     u8          输入        使能标志
* u8OverrideEn    u8          输入        u8Override使能标志
* u8TxFlowEn      u8          输入        Tx流控使能标志
* u8RxFlowEn      u8          输入        Rx流控使能标志
* u8Speed         u8          输入        端口速率
* u8Duplex        u8          输入        双工状态
* u8LinkState     u8          输入        连接状态

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
* 
*******************************************************************************/
s32 ethsw_set_port_stat(u8 u8PortId, u8 u8PhyScanEn, u8 u8OverrideEn, u8 u8TxFlowEn, u8 u8RxFlowEn, u8 u8Speed, u8 u8Duplex, u8 u8LinkState)
{
    STRU_ETHSW_PORT_STAT struPortStat;
    struPortStat.PhyScanEn = u8PhyScanEn;
    struPortStat.OverrideEn = u8OverrideEn; /* 采用此寄存器中设置的速率,双工和LINK状态 */
    struPortStat.TxFlowEn = u8TxFlowEn;   /* 发送流控使能 */
    struPortStat.RxFlowEn = u8RxFlowEn;   /* 接收流控使能 */
    struPortStat.Speed = u8Speed;      /* 速率 */
    struPortStat.Duplex = u8Duplex;    /* 双工状态 */
    struPortStat.LinkState = u8LinkState; 
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, MII_PORT_STATE_OR(u8PortId), (u8 *)&struPortStat, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_set_phy_scan
* 函数功能: 对交换芯片的PHY 自动扫描功能进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
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
* 
*******************************************************************************/
s32 ethsw_set_phy_scan(void)
{
    u8   u8PortId;

    for(u8PortId = 0; u8PortId < 8; u8PortId++)
    {
        RETURN_IF_ERROR(ethsw_set_port_stat(u8PortId,1,0,0,0,0,0,0));
    }

    return (E_ETHSW_OK);
}
/*******************************************************************************
* 函数名称: ethsw_set_arl_hash
* 函数功能: 设置ARL表的配置,HASH_DISABLE,芯片查找ARL表时是先计算HASH INDEX,置0
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Val         u8          输入        设置的值
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
* 
*******************************************************************************/
s32 ethsw_set_arl_hash(u8 u8Val)
{
    u8   u8Temp;
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, GLOBAL_ARL_CFG, &u8Temp, ONE_BYTE_WIDTH));
    printf("\nu8Temp is 0x%x\n",u8Temp);	
    if (0 == u8Val)
    {
        u8Temp &= 0xfe; /* 将最低一位置为0 */
    }
    else
    {
        u8Temp |= 0x1;/* 将最低一位置为1 */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, GLOBAL_ARL_CFG, &u8Temp, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_set_qos
* 函数功能: 对芯片QoS寄存器进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Enable      u8          输入        使能或止能QoS功能
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
* 
*******************************************************************************/
s32 ethsw_set_qos(u8 u8Enable)
{
    u8   u8Temp;
    /* 设置QoS使能寄存器 */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, QOS_GB_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0xbf;  /* Bit6设为0,Port Base QOS Disable */
    }
    else
    {
        u8Temp |= 0x40;  /* Bit6设为1,Port Base QOS En  */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QOS_PAGE, QOS_GB_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    /* 设置TxQ控制寄存器 */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, TX_QUEUE_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0xf3;  /* Bit3:2设为0,QOS_MODE b00: Singe Queue (No QoS) b01: Two Queues Mode
                                                 b10: Three Queues Mode    b11: Four Queues Mode */
    }
    else
    {
        u8Temp |= 0x0c;  /* Bit3:2设为11,QOS_MODE Four Queues */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QOS_PAGE, TX_QUEUE_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}
/*******************************************************************************
* 函数名称: ethsw_set_pvlan
* 函数功能: 对PVLAN寄存器进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
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
* 
*******************************************************************************/
s32 ethsw_set_pvlan(u32 u32VlanMask)
{
    u8    u8PortId;    /* 端口ID */
    //u16   u16VlanMask; /* VLAN MASK */
    u8    u8i;
    //u16VlanMask = 0x1ff;/* 所有的端口都属于同一个PVLAN */
    //u16VlanMask = 0x83;/* 所有的端口都属于同一个PVLAN */

    g_pVlan[0] = u32VlanMask;
    for(u8i = 1; u8i < 10; u8i++)
    {
        g_pVlan[u8i] = 0x10000;
    }
    for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
    {
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortId), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
    }
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_add_port_vlan
* 函数功能: 将端口添加到指定的Vlan中
* 相关文档:
* 函数参数:
* 参数名称:     类型                    输入/输出   描述
* u8ChipId      u8                      输入        芯片ID(single should be 0)
* u8PortId      u8                      输入        端口号
* u8VlanId      u8                      输入        指定的Vlan号
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
* 
*******************************************************************************/
s32 ethsw_add_port_vlan(u8 u8PortId, u8 u8VlanId)
{
    u8    u8PortIdtemp;
    u32   u32VlanMask;
    u8    u8i;
    u32VlanMask = 0x1ffff;
    g_pVlan[0] = g_pVlan[0] & (~(u32)(1 << u8PortId));
    g_pVlan[u8VlanId] = g_pVlan[u8VlanId] | (1 << u8PortId);
    for (u8PortIdtemp = 0; u8PortIdtemp < MAX_USED_PORTS; u8PortIdtemp++)
    {
        for(u8i = 0;u8i < 10; u8i++)
        {
            if(0 != ((1 << u8PortIdtemp) & g_pVlan[u8i]))
            {
                u32VlanMask = g_pVlan[u8i];
                break;
            }
        }
        if(u8i == 10)
        {
            return (E_ETHSW_FAIL);
        }
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortIdtemp), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
    }

    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_set_port_vlan
* 函数功能: 对PVLAN寄存器进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8PortId1      u8          输入       端口号1
* u8PortId2      u8          输入       端口号2
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
* 
*******************************************************************************/
s32 ethsw_set_port_vlan(u8 u8PortId1, u8 u8PortId2)
{
	u32 u32VlanMask = 0x0001ffff;
    u8 u8Pbuf[4] = {0};
    /* 设置端口PortId1，与端口PortId2相隔离 */
	u32VlanMask &= ~(1<<u8PortId2);
    u8Pbuf[3] = (u32VlanMask & 0xff000000)>>24;
    u8Pbuf[2] = (u32VlanMask & 0x00ff0000)>>16;
    u8Pbuf[1] = (u32VlanMask & 0x0000ff00)>>8;
    u8Pbuf[0] = (u32VlanMask & 0x000000ff)>>0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)(4*u8PortId1), u8Pbuf, FOUR_BYTE_WIDTH));
    /* 设置端口PortId2，与端口PortId1相隔离 */
    u32VlanMask = 0x0001ffff;
    u32VlanMask &= ~(1<<u8PortId1);
    u8Pbuf[3] = (u32VlanMask & 0xff000000)>>24;
    u8Pbuf[2] = (u32VlanMask & 0x00ff0000)>>16;
    u8Pbuf[1] = (u32VlanMask & 0x0000ff00)>>8;
    u8Pbuf[0] = (u32VlanMask & 0x000000ff)>>0;
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)(4*u8PortId2), u8Pbuf, FOUR_BYTE_WIDTH));
	return (E_ETHSW_OK);

}
/*******************************************************************************
* 函数名称: ethsw_set_qvlan
* 函数功能: 对PVLAN寄存器进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Enable      u8          输入        0:Disable VLAN,在计算HASH时,不使用VID
*                                       1:Enable VLAN
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
* 
*******************************************************************************/
s32 ethsw_set_qvlan(u8 u8Enable)
{
    u8   u8Temp;

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL0, &u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0x1f; /* 将最高3位置为0 */
    }
    else
    {
        u8Temp |= 0xe0; /* 将最高3位置为1 */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL0, &u8Temp, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}
/*******************************************************************************
*  name         : ethsw_set_port_mode
*  description  : 手动设置端口模式
*  input para   : u8PortId      - 端口号
*                     u8SerdesEn   - 使能Serdes模式标志
*                     u8AutodetectEn  - 自动检测使能标志
*  output para  : 无
*  return value : 状态
*  others       : 
*******************************************************************************/
s32 ethsw_set_port_serdes_mode(u8 u8PortId, u8 u8SerdesEn)
{
    u16 struPortMode;

    ethsw_read_reg(ETHSW_SERDES_PAGE(u8PortId), SERD_SGMII_CTRL1, (u8 *)&struPortMode, 2);
    if(u8SerdesEn)
        struPortMode |= (1 << 16);       /* SerDes模式使能 */
    ethsw_write_reg(ETHSW_SERDES_PAGE(u8PortId), SERD_SGMII_CTRL1, (u8 *)&struPortMode, 2);

    return (E_ETHSW_OK);
}
/*******************************************************************************
* 函数名称: ethsw_set_aging_time
* 函数功能: 设置芯片的老化时间(ARL表的老化时间(由于采用静态ARL表所以无配置要求),
*           也是包的老化时间)
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u32Age_Time   u32         输入        此寄存器应当设置的Aging值
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
* 
*******************************************************************************/

s32 ethsw_set_aging_time(u32 u32AgeTime)
{
    STRU_ETHSW_AGE_TIME struAgeTime;   /* 直接设置老化时间 */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, AGING_TIME_CTRL, (u8 *)&struAgeTime, FOUR_BYTE_WIDTH));
    struAgeTime.AgeTime = u32AgeTime;
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, AGING_TIME_CTRL, (u8 *)&struAgeTime, FOUR_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_bcm5389_init
* 函数功能: 初始化BCM5389芯片
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
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
* 
*******************************************************************************/

s32 ethsw_bcm5389_init(void)
{
    u8   u8PortId;
    
    if (0 != (g_u32InitFlag & ETHSW_CHIP_INIT_FLAG))
    {
        (void)printf("CHIP has been initialized!\n");
        return (E_ETHSW_CHIP_REINIT);
    }
    /* 1.打开端口的收发功能 */
    for (u8PortId = 0; u8PortId < 1; u8PortId++)
    {
        RETURN_IF_ERROR(ethsw_set_status(u8PortId, 1, 1));
    }

    /* 2.打开芯片转发功能 */
    RETURN_IF_ERROR(ethsw_set_switch_mode(1));

    /* 3.打开ARL表支持Multicast功能 */
    RETURN_IF_ERROR(ethsw_set_arl_multicast(1));

    /* 4.对单播和组播DLF包的转发进行配置:单播组播DLF包都转发到所有端口 */
    RETURN_IF_ERROR(ethsw_set_dlf_forward(0, 0x00, 0, 0x00));

    /* 5.强制设置各端口的状态,DSP各端口需要强制设置 */
    for (u8PortId = 0; u8PortId < 8; u8PortId++)
    {
        //RETURN_IF_ERROR(ethsw_force_port_up(u8PortId));
        RETURN_IF_ERROR(ethsw_set_port_stat(u8PortId,0,1,1,1,2,1,1));
    }

    /* 6.对交换芯片的PHY自动扫描功能进行配置 */
    RETURN_IF_ERROR(ethsw_set_phy_scan());

    /* 7.配置ARL配置寄存器,HASH_ENABLE */
    RETURN_IF_ERROR(ethsw_set_arl_hash(0));

    /* 8.配置QoS控制寄存器,Disable QoS功能 */
    RETURN_IF_ERROR(ethsw_set_qos(0));

    /* 9.配置PVLAN */
    //RETURN_IF_ERROR(ethsw_set_pvlan());

    /* 10.配置VLAN控制寄存器*/
    RETURN_IF_ERROR(ethsw_set_qvlan(0));

    /* 11.配置AGING TIME控制寄存器,设置老化时间,设置为不老化*/
    RETURN_IF_ERROR(ethsw_set_aging_time(0));

    /* 芯片初始化完毕,设置标记 */
    g_u32InitFlag |= ETHSW_CHIP_INIT_FLAG;
    return (E_ETHSW_OK);
}

#if 0
s32 ethsw_write_test(void)
{
    u8   u8Temp[8];
    u8   u8Temp1[8];
    u8   u8Temp2[8];
    int i;

	
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR1, u8Temp, SIX_BYTE_WIDTH));
    #if 0
    for(i =0;i<6;i++)
    {
          printf("\ndata is 0x%x\n",u8Temp[i]);
    }
    #endif

    for(i =0;i<6;i++)
    {
        u8Temp1[i] = 1+i;
    }
	#if 0
    for(i =0;i<6;i++)
    {
          printf("\ndata is 0x%x\n",u8Temp1[i]);
    }
    #endif
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR1, u8Temp1, SIX_BYTE_WIDTH));

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR1, u8Temp2, SIX_BYTE_WIDTH));
    #if 0
    for(i =0;i<6;i++)
    {
          printf("\ndata is 0x%x\n",u8Temp2[i]);
    }
    #endif
    if(memcmp(u8Temp1, u8Temp2, 6) != 0)
        return (E_ETHSW_FAIL);

    return (E_ETHSW_OK);
}
#endif

s32 ethsw_write_test(void)
{
    u16 tmpIn = 0;
    u16 tmpOut = 0;

    // 读取寄存器值    
    ethsw_read_reg(ETHSW_CTRL_PAGE, LED_CONTROL_A, (u8 *)(&tmpIn), 2);
    // 设置寄存器
    ethsw_write_reg(ETHSW_CTRL_PAGE, LED_CONTROL_A, (u8 *)(&tmpIn), 2); 
    // 重新读取寄存器值
    ethsw_read_reg(ETHSW_CTRL_PAGE, LED_CONTROL_A, (u8 *)(&tmpOut), 2);
    if(tmpOut == tmpIn)
    {
        return E_ETHSW_OK;
    }
    else
    {
        return E_ETHSW_FAIL;
    }
    
}

int ethsw_set_portvlan(u8 u8PortIdtemp,u32 u32VlanMask)
{
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortIdtemp), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
}

int ethsw_read_portvlan(u8 u8PortIdtemp)
{
    u32 u32VlanMask;
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortIdtemp), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
    printf("read u32VlanMask = 0x%x.\n",u32VlanMask);
}

