/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名: bsp_ms.h 
* 功能: 主备切换                 
* 版本:                                                                  
* 编制日期:2016.09.10                              
* 作者:吕后勇                                              
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "fcntl.h"
#include "unistd.h"
#include "bsp_types.h"
#include "bsp_spi_ext.h"
#include "bsp_epld_ext.h"
#include "fsl_p2041_ext.h"
//#include "bsp_conkers_ext.h"

extern s8 bsp_send_opp_slave_msg(u32 cause);
extern int bsp_get_phy_linked(void);


/**************************** 私用头文件* **********************************/
#include "../inc/bsp_ms.h"
#include "../../hmi/inc/hmi.h"

/******************************* 局部宏定义 *********************************/


/*********************** 全局变量定义/初始化 **************************/
s8 g_s8HardwareFault = 0;
u32 g_u32MasterSlaveSwitchCause = 0;	//主备状态转换原因
int g_s32DspAckMSSwitchMsg[6] = {0};
u8 g_u8bbpAckMSSwitchMsg[6]={0};


/************************** 局部常数和类型定义 ************************/
u8 ms_printf_flag =0;


/*************************** 局部函数原型声明 **************************/

/***************************函数实现 *****************************/
u8 bsp_get_self_MS_state(void)
{
	u8 tmp_u8_MS_state;
	tmp_u8_MS_state = bsp_cpld_read_reg(CPLD_MS_STATE_REG);
	//printf("MS state reg:%d",ms_state);
	return (tmp_u8_MS_state >> SELFSLOT_FLAG)&0x01;
}
u8 bsp_get_opp_MS_state(void)
{
	u8 tmp_u8_MS_state;
	tmp_u8_MS_state = bsp_cpld_read_reg(CPLD_MS_STATE_REG);
	//printf("MS state reg:%d",ms_state);
	return (tmp_u8_MS_state >> OPPSLOT_FLAG)&0x01;
}
u8 bsp_get_self_available(void)
{
	u8 tmp_u8_MS_available;
	tmp_u8_MS_available = bsp_cpld_read_reg(CPLD_MS_STATE_REG);
	//printf("MS state reg:%d",ms_state);
	return (tmp_u8_MS_available >> SELFSLOT_IND)&0x01;
}
u8 bsp_get_opp_available(void)
{
	u8 tmp_u8_MS_available;
	tmp_u8_MS_available = bsp_cpld_read_reg(CPLD_MS_STATE_REG);
	//printf("MS state reg:%d",ms_state);
	return (tmp_u8_MS_available >> OPPSLOT_IND)&0x01;
}
u8 bsp_get_oppslot_pd(void)
{	
	u8 tmp_u8_MS_pd;
	tmp_u8_MS_pd = bsp_cpld_read_reg(CPLD_MS_STATE_REG);
	//printf("MS state reg:%d",ms_state);
	return (tmp_u8_MS_pd >> OPPSLOT_PD)&0x01;
}
u8 bsp_get_oppslot_reset(void)
{
	u8 tmp_u8_opp_reset=0;
	tmp_u8_opp_reset == bsp_cpld_read_reg(CPLD_MS_STATE_REG);
	return (tmp_u8_opp_reset >> OPPSLOT_RST)&0x01;
}
u8 bsp_get_oppslot_clk(void)
{
	u8 tmp_u8_MS_pd=0;
	tmp_u8_MS_pd = bsp_cpld_read_reg(CPLD_MS_STATE_REG);
	//printf("MS state reg:%d",ms_state);
	return (tmp_u8_MS_pd >> OPPSLOT_CLK)&0x01;
}

s8 bsp_set_self_master(void)
{
	u8 tmp_u8_MS_ctr_reg;
	if(MCT_SLAVE == bsp_get_opp_MS_state()) 
	{
		tmp_u8_MS_ctr_reg = bsp_cpld_read_reg(CPLD_MS_CTR_REG);
		bsp_cpld_write_reg(CPLD_MS_CTR_REG,0x80|tmp_u8_MS_ctr_reg);
		if(1 == ms_printf_flag)
		{
			printf("switch to master!\n");
		}
		return BSP_OK;
	}
	return BSP_ERROR;
}

s8 bsp_set_self_slave(void)
{
	u8 tmp_u8_MS_ctr_reg=0;
	u8 opp_slotid=0;
	opp_slotid = !bsp_get_slot_id();
	//bsp_inform_bbp_MS_switch(opp_slotid);
	//bsp_inform_dsp_MS_switch(opp_slotid);
	memset(g_s32DspAckMSSwitchMsg,0,sizeof(g_s32DspAckMSSwitchMsg));
	memset(g_u8bbpAckMSSwitchMsg,0,sizeof(g_u8bbpAckMSSwitchMsg));
	//bsp_hmi_stop_send_message();
	bsp_close_eth3_net();
	if (system("/sbin/ifconfig eth0 down;\
		/sbin/ifconfig eth0 hw ether 00:A0:1E:01:00:01 mtu 9000;") < 0)
		perror("ERROR: Set eth0 down failed");
	tmp_u8_MS_ctr_reg = bsp_cpld_read_reg(CPLD_MS_CTR_REG);
	bsp_cpld_write_reg(CPLD_MS_CTR_REG,0x40&tmp_u8_MS_ctr_reg);
	g_u32MasterSlaveSwitchCause = MCT_MS_SWITCH_CASE_CLEAR;		//清除主备切换原因
	if(MCT_SLAVE == bsp_get_self_MS_state())
	{
		if(1 == ms_printf_flag)
		{
			printf("switch to slave!\n");
		}
		return BSP_OK;
	}
	return BSP_ERROR;
}


u8 bsp_set_self_available(void)
{
	u8 tmp_u8_MS_ctr_reg;
	tmp_u8_MS_ctr_reg = bsp_cpld_read_reg(CPLD_MS_CTR_REG);
	bsp_cpld_write_reg(CPLD_MS_CTR_REG,0x40|tmp_u8_MS_ctr_reg);
}

u8 bsp_set_self_unavailable(void)
{
	u8 tmp_u8_MS_ctr_reg;
	tmp_u8_MS_ctr_reg = bsp_cpld_read_reg(CPLD_MS_CTR_REG);
	bsp_cpld_write_reg(CPLD_MS_CTR_REG,0x80&tmp_u8_MS_ctr_reg);
}

u8 bsp_get_cpld_clock_state()
{
	u8 u8ClockStateReg;
	u8 u8PLL1,u8PLL2;
	u8ClockStateReg = bsp_cpld_read_reg(CPLD_CLOCK_STAT_REG);
	u8PLL1 = (u8ClockStateReg >> PLL1_CPLD_CLOCK)&0x01;
	u8PLL2 = (u8ClockStateReg >> PLL2_CPLD_CLOCK)&0x01;

	return u8PLL1&u8PLL2;
}
u8 bsp_get_switch_reset_status()
{
	u8 u8switch_reset_status_tmp;
	u8switch_reset_status_tmp=bsp_cpld_read_reg(CPLD_CHIP_RESET_CTRL_H_REG);
	u8switch_reset_status_tmp=(u8switch_reset_status_tmp >> SWITCH_CPLD_RESET)&0x01;
	return u8switch_reset_status_tmp;
}

s8 bsp_get_switch_if_flag()
{
	int s32i_tmp;
	for(s32i_tmp = ETH1;s32i_tmp <= ETH2;s32i_tmp++)
	{
		if(1 != bsp_get_if_flag(s32i_tmp))
			return BSP_ERROR;
	}
	return BSP_OK;
}

/*******************************************************************************
* 函数名称: bsp_MCT_available_detection					
* 功    能: 硬件检查线程                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
u8 bsp_MCT_available_detection()
{
	u8 u8MCTUnableCase = BSP_OK;
	if(0 == bsp_get_cpld_clock_state())		//CPLD 锁相环
	{
		printf("CPLD PLL UNLOCK!\n");
		u8MCTUnableCase = MCT_SWITCH_CASE_CPLD_PLL;
	}
	if(bsp_get_switch_reset_status()||bsp_get_switch_if_flag())	//SWITCH配置状态
	{
		//printf("SWITCH UNINIT!\n");
		u8MCTUnableCase = MCT_SWITCH_CASE_SWITCH;
	}
	if(gps_check_clock()!=0)		//GPS接收机锁定
	{
		//printf("GPS UNLOCK!\n");
		u8MCTUnableCase = MCT_SWITCH_CASE_GPS_UNLOCK;
	}
	if(0 == bsp_get_phy_flag(GE0))	//核心网连接状态
	{
		printf("GE0 UNLINK!\n");
		u8MCTUnableCase = MCT_SWITCH_CASE_GE0_UNLINK;
	}
	return u8MCTUnableCase;
}

/******************************************************************************
 * 函数名: bsp_ms_switch_callback
 * 功  能: L3注册主备切换回调函数
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_ms_switch_callback(MSSWITCH_FUNCPTR pCallBack)
{
	funcMasterSlaveSwitch = pCallBack;
}

/******************************************************************************
 * 函数名: bsp_master_slave_competition
 * 功  能: 主备板竞争
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_master_slave_competition(void)
{
	u8 tmp_u8delay;
	u8 u8slot;
	int i;

	printf("SLOT[%d]:MASTER SLAVE COMPETITION!\n",bsp_get_slot_id());
	//sleep(20);	//等待phy完成初始化

	if(IPMB_SLOT0 == bsp_get_slot_id())
	{
		for(i = 0;i < 30;i++)
		{
			sleep(1);
			if(MCT_OPP_PD_OUT == bsp_get_oppslot_pd())
			{
				printf("SLOT 0: OPP PD OUT\n");
				break;
			}
			if(MCT_AVIABLE == bsp_get_self_available())
			{
				printf("SLOT 0: SELF is AVAILABLE\n");
				break;
			}
			if(MCT_MASTER == bsp_get_opp_MS_state())
			{
				printf("SLOT 0: OPP is MASTER !\n");
				break;
			}
		}
		
		if( (MCT_MASTER !=bsp_get_opp_MS_state()) && ( (MCT_AVIABLE == bsp_get_self_available()) || (MCT_AVIABLE != bsp_get_opp_available()) ) )
		{
			if(i>=30)
			{
				printf("SLOT 0:OPP is unavailable!\n");
			}
			g_u32MasterSlaveSwitchCause = MCT_SWITCH_CASE_COMPETITION;
			if(MCT_MASTER == bsp_get_self_MS_state())	//防止reboot复位，CPLD状态未清除
			{
				bsp_open_eth3_net();
				if (system("/sbin/ifconfig eth0 down;\
					/sbin/ifconfig eth0 hw ether 00:A0:1E:01:01:01 mtu 9000;\
							ifconfig eth0 up") < 0)
					perror("ERROR: Set eth0 failed ");
				
				bsp_send_gratuitous_arp();
				#if 1
				bsp_hmi_start_send_message();
				#else
				for(u8slot=2;u8slot<11;u8slot++)
				{
					bsp_hmi_start_send_message(u8slot);
				}
				#endif
				bsp_inform_bbp_MS_switch(bsp_get_slot_id());
				bsp_led_act_on();
				//bsp_inform_dsp_MS_switch(bsp_get_slot_id());
				if(funcMasterSlaveSwitch !=NULL)
				{
					funcMasterSlaveSwitch(MCT_MASTER,g_u32MasterSlaveSwitchCause);
				}
				g_u32MasterSlaveSwitchCause = MCT_MS_SWITCH_CASE_CLEAR;
			}
			else
			{
				bsp_set_self_master();
			}
		}
		else
		{
			if(MCT_MASTER == bsp_get_opp_MS_state())
			{
				printf("SLOT 0: OPP is MASTER !\n");
			}
			else
			{
				printf("SLOT 0: SELF is UNAVIABLE !\n");
			}
			bsp_close_eth3_net();
			bsp_set_self_slave();
			bsp_led_act_off();
		}
	}
	
	if(IPMB_SLOT1 == bsp_get_slot_id())
	{
		for(i = 0;i < 45;i++)
		{
			sleep(1);
			if(MCT_OPP_PD_OUT == bsp_get_oppslot_pd())
			{
				printf("SLOT 1: OPP PD OUT\n");
				break;
			}
			if(MCT_MASTER == bsp_get_opp_MS_state())
			{
				printf("SLOT 1: OPP is MASTER\n");
				break;
			}
			if(i > 15)
			{
				if(MCT_AVIABLE == bsp_get_self_available())
				{
					printf("SLOT 1: SELF is available\n");
					break;
				}
			}
			printf("SLOT 1: WAITING TIME %d s\n",i);
		}
		
		if(45 == i)
		{
			printf("SLOT 1: WAITING OVER TIME\n");
		}
		
		if( MCT_MASTER !=bsp_get_opp_MS_state())
		{
			g_u32MasterSlaveSwitchCause = MCT_SWITCH_CASE_COMPETITION;
			if(MCT_MASTER == bsp_get_self_MS_state())	//防止reboot复位，CPLD状态未清除
			{
				bsp_open_eth3_net();
				if (system("/sbin/ifconfig eth0 down;\
					/sbin/ifconfig eth0 hw ether 00:A0:1E:01:01:01 mtu 9000;\
							ifconfig eth0 up") < 0)
					perror("ERROR: Set eth0 failed ");
				
				bsp_send_gratuitous_arp();
				#if 1
				bsp_hmi_start_send_message();
				#else
				for(u8slot=2;u8slot<11;u8slot++)
				{
					bsp_hmi_start_send_message(u8slot);
				}
				#endif
				bsp_inform_bbp_MS_switch(bsp_get_slot_id());
				bsp_led_act_on();
				//bsp_inform_dsp_MS_switch(bsp_get_slot_id());
				if(funcMasterSlaveSwitch !=NULL)
				{
					funcMasterSlaveSwitch(MCT_MASTER,g_u32MasterSlaveSwitchCause);
				}
				g_u32MasterSlaveSwitchCause = MCT_MS_SWITCH_CASE_CLEAR;
			}
			else
			{
				bsp_set_self_master();
			}
		}
		else
		{
			bsp_close_eth3_net();
			bsp_set_self_slave();
			bsp_led_act_off();
		}
	}
	if(system("/sbin/ifconfig eth3 up;") < 0)
		perror("ERROR: Up eth3 failed ");
	
	return BSP_OK;
} 

/******************************************************************************
 * 函数名: bsp_master_slave_switch
 * 功  能: 发出主备倒换消息
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_master_slave_switch(u8 switchto,u32 cause)
{
	u32 tmp_u32_ret;
	int i;
	if(bsp_get_self_MS_state() == switchto)
	{
		printf("MCT master/slave state switch error!Master/Slave:%d switchto:%d cause:%d\n",bsp_get_self_MS_state(),switchto,cause);
		return BSP_ERROR;
	}
	
	switch(switchto)
	{
		case MCT_SLAVE:
			if(MCT_AVIABLE == bsp_get_opp_available())
			{
				for(i = 0; i < 3;i++)
				{
					if(BSP_OK == bsp_send_host_standby_switch_msg(MCT_MASTER,cause))
						break;
				}
			}
			break;
		case MCT_MASTER:
			if(MCT_SLAVE == bsp_get_opp_MS_state())
			{
				g_u32MasterSlaveSwitchCause = cause;
				bsp_set_self_master();
			}
			else
			{
				for(i = 0;i < 3;i++)
				{
					bsp_send_host_standby_switch_msg(MCT_SLAVE,cause);
				}
			}
			break; 
		default:
			printf("MCT master/slave state switch error!set state:%d\n",switchto);
			return BSP_ERROR;
	}
	return BSP_OK;
}



/*******************************************************************************
* 函数名称: bsp_hardware_check						
* 功    能: 硬件检查线程                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
void bsp_hardware_check()
{	
	u8 u8bbp_slotid;
	u8 u8SendGratuitousArp=0;
	u8 u8InformbbpMSSwitchTimes[6] = {0};
	u8 u8InformDSPMSSwitchTimes[6] = {0};

	u8 u8GE0UnlinkTimes = 0;
	u8 u8GPSUnlockTimes = 0;
	u8 u8CPLD_PLL_Times = 0;
	u8 u8SWITCH_UNINIT_Times = 0;
	while(1)
	{
		/*硬件健康检查*/
		g_s8HardwareFault = bsp_MCT_available_detection();
		
		if(BSP_OK != g_s8HardwareFault)
		{
			if(MCT_SWITCH_CASE_GE0_UNLINK == g_s8HardwareFault)
			{
				u8GE0UnlinkTimes++;
			}
			else
			{
				u8GE0UnlinkTimes=0;
			}
			if(MCT_SWITCH_CASE_GPS_UNLOCK == g_s8HardwareFault)
			{
				u8GPSUnlockTimes++;
			}
			else
			{
				u8GPSUnlockTimes=0;
			}
			if(MCT_SWITCH_CASE_CPLD_PLL == g_s8HardwareFault)
			{
				u8CPLD_PLL_Times++;
			}
			else
			{
				u8CPLD_PLL_Times=0;
			}
			if(MCT_SWITCH_CASE_SWITCH == g_s8HardwareFault)
			{
				u8SWITCH_UNINIT_Times++;
			}
			else
			{
				u8SWITCH_UNINIT_Times=0;
			}
		}
		else
		{
			u8GE0UnlinkTimes=0;
			u8GPSUnlockTimes=0;
			u8CPLD_PLL_Times=0;
			u8SWITCH_UNINIT_Times=0;
			bsp_set_self_available();
		}
		

		if(u8GE0UnlinkTimes>=1||u8GPSUnlockTimes>=3||u8CPLD_PLL_Times>=3||u8SWITCH_UNINIT_Times>=3)
		{
			bsp_set_self_unavailable();
			if(MCT_MASTER == bsp_get_self_MS_state())
				bsp_master_slave_switch(MCT_SLAVE,g_s8HardwareFault);
			u8GE0UnlinkTimes=0;
			u8GPSUnlockTimes=0;
			u8CPLD_PLL_Times=0;
			u8SWITCH_UNINIT_Times=0;
		}
		
		
		if(MCT_MASTER == bsp_get_self_MS_state())
		{
			u8SendGratuitousArp++;
			if(u8SendGratuitousArp >= 3)
			{
				if(BSP_OK == bsp_send_gratuitous_arp())
					u8SendGratuitousArp=0;
			}
			bsp_send_boards_status_info();
			
			for(u8bbp_slotid = IPMB_SLOT2;u8bbp_slotid<=IPMB_SLOT10;u8bbp_slotid++)
			{
				if(MCT_MS_SWITCH_INFORMED == g_u8bbpAckMSSwitchMsg[u8bbp_slotid-2])
				{
					bsp_hmi_start_send_message();
					break;
				}
			}
			for(u8bbp_slotid = IPMB_SLOT2;u8bbp_slotid<=IPMB_SLOT10;u8bbp_slotid++)
			{	
				if(MCT_MS_SWITCH_INFORMED == g_u8bbpAckMSSwitchMsg[u8bbp_slotid-2])
				{
					bsp_inform_bbp_master_mcta_slotid(u8bbp_slotid,bsp_get_slot_id());
					u8InformbbpMSSwitchTimes[u8bbp_slotid-2]++;
				}
				else
				{
					u8InformbbpMSSwitchTimes[u8bbp_slotid-2] = 0;
				}
				if(u8InformbbpMSSwitchTimes[u8bbp_slotid-2] >= 3)
				{
					u8InformbbpMSSwitchTimes[u8bbp_slotid-2] = 0;
					g_u8bbpAckMSSwitchMsg[u8bbp_slotid-2] = MCT_MS_SWITCH_EVENT_CLEAR;
				}
			}
		}
		sleep(1);
	}
}


/*******************************************************************************
* 函数名称: bsp_hardware_check_init						
* 功    能: 初始化硬件检测线程                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 bsp_hardware_check_init(void)
{
	s32 s32ret;
	u32 u32i;
    int res=0;
	int fd;
    pthread_t ptid;
    pthread_t       a_thread;
    pthread_attr_t  attr;
    struct sched_param parm;
	//unsigned char touchFileCmd[100] = "";
    pthread_attr_init(&attr); 
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 30; 
    pthread_attr_setschedparam(&attr, &parm);
    res = pthread_create(&ptid, &attr, (FUNCPTR)bsp_hardware_check,NULL);
    pthread_attr_destroy(&attr);
    if (BSP_ERROR == res)
    {
        perror("create hardware check thread error!\n");
    }

	bsp_dbg("bsp_hardware_check success!\n");

	return BSP_OK;

}


/******************************************************************************
 * 函数名: bsp_ready_MCT_state_switch
 * 功  能: 是否具备主备状态切换条件
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_ready_MCT_state_switch(void)
{
	u8 tmp_u8MS_state = 0;
	tmp_u8MS_state = bsp_get_self_MS_state();
	switch(tmp_u8MS_state)
	{
		case MCT_MASTER:
			if( MCT_AVIABLE != bsp_get_opp_available() )
			{
				if(MCT_OPP_PD_OUT == bsp_get_oppslot_pd())
					return MCT_SWITCH_CASE_OPP_OUT;

				if(MCT_OPP_INRESET == bsp_get_oppslot_reset())
					return MCT_SWITCH_CASE_OPP_RESET;

				return MCT_SWITCH_CASE_OPP_DISABLE;
			}
			return BSP_OK;
		case MCT_SLAVE:
			if( MCT_AVIABLE != bsp_get_self_available() )
			{
				return g_s8HardwareFault;
			}
			break;
		default:
			printf("MCT MS state error! MS state = %d\n",tmp_u8MS_state);
			return BSP_ERROR;
	}
	return BSP_OK;
}

/******************************************************************************
 * 函数名: bsp_get_host_ip
 * 功  能: 获取 主板ip
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
u32 g_u32HostSlaveIPdebug = 0;
s8 bsp_get_host_ip(u8 *ip)
{
	u8 tmp_hostip[4] = {192,168,1,13};
	if(MCT_MASTER == bsp_get_self_MS_state())
	{
		tmp_hostip[2] = bsp_get_slot_id() + 1;
		memcpy(ip,tmp_hostip,4);
		if(1 == g_u32HostSlaveIPdebug)
		{
			printf("MASTER IP:%d,%d,%d,%d\n",ip[0],ip[1],ip[2],ip[3]);
		}
		return BSP_OK;
	}
	if(MCT_MASTER == bsp_get_opp_MS_state())
	{
		tmp_hostip[2] = (!bsp_get_slot_id()) + 1;
		memcpy(ip,tmp_hostip,4);
		if(1 == g_u32HostSlaveIPdebug)
		{
			printf("MASTER IP:%d,%d,%d,%d\n",ip[0],ip[1],ip[2],ip[3]);
		}
		return BSP_OK;
	}
	return BSP_ERROR;
}
/******************************************************************************
 * 函数名: bsp_get_host_mac
 * 功  能: 获取 主板MAC 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_get_host_mac(u8 * mac)
{
	u8 tmp_hostmac[6]={0x00,0xA0,0x1E,0x01,0x01,0x02};
	if(MCT_MASTER == bsp_get_self_MS_state())
	{
		tmp_hostmac[4] = bsp_get_slot_id() + 1;
		memcpy(mac,tmp_hostmac,6);
		if(1 == g_u32HostSlaveIPdebug)
		{
			printf("MASTER MAC:%x,%x,%x,%x,%x,%x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		}
		return BSP_OK;
	}
	if(MCT_MASTER == bsp_get_opp_MS_state())
	{
		tmp_hostmac[4] = (!bsp_get_slot_id()) + 1;
		memcpy(mac,tmp_hostmac,6);
		if(1 == g_u32HostSlaveIPdebug)
		{
			printf("MASTER MAC:%x,%x,%x,%x,%x,%x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		}
		return BSP_OK;
	}
	return BSP_ERROR;
}
/******************************************************************************
 * 函数名: bsp_get_standby_ip
 * 功  能: 获取备板ip 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_get_standby_ip(u8 *ip)
{
	u8 tmp_standby_ip[4]={192,168,1,13};
	if(MCT_SLAVE == bsp_get_self_MS_state())
	{
		tmp_standby_ip[2] = bsp_get_slot_id() + 1;
		memcpy(ip,tmp_standby_ip,4);
		if(1 == g_u32HostSlaveIPdebug)
		{
			printf("SLAVE IP:%x,%x,%x,%x,%x,%x\n",ip[0],ip[1],ip[2],ip[3]);
		}
		return BSP_OK;
	}
	
	if(MCT_OPP_PD_ON == bsp_get_oppslot_pd())
	{
		if(MCT_SLAVE == bsp_get_opp_MS_state())
		{
			tmp_standby_ip[2] = (!bsp_get_slot_id()) + 1;
			memcpy(ip,tmp_standby_ip,4);
			if(1 == g_u32HostSlaveIPdebug)
			{
				printf("SLAVE IP:%d,%d,%d,%d\n",ip[0],ip[1],ip[2],ip[3]);
			}
			return BSP_OK;
		}
	}
	return BSP_ERROR;
}
/******************************************************************************
 * 函数名: bsp_get_standby_mac
 * 功  能: 获取备板mac
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_get_standby_mac(u8 *mac)
{
	u8 tmp_standbymac[6]={0x00,0xA0,0x1E,0x01,0x01,0x02};
	if(MCT_SLAVE == bsp_get_self_MS_state())
	{
		tmp_standbymac[4] = bsp_get_slot_id()+1;
		memcpy(mac,tmp_standbymac,6);
		if(1 == g_u32HostSlaveIPdebug)
		{
			printf("SLAVE MAC:%x,%x,%x,%x,%x,%x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		}
		return BSP_OK;
	}
	if(MCT_OPP_PD_ON == bsp_get_oppslot_pd())
	{
		if(MCT_SLAVE == bsp_get_opp_MS_state())
		{
			tmp_standbymac[4] = (!bsp_get_slot_id())+1;
			memcpy(mac,tmp_standbymac,6);
			if(1 == g_u32HostSlaveIPdebug)
			{
				printf("SLAVE MAC:%x,%x,%x,%x,%x,%x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			}
			return BSP_OK;
		}
	}
	return BSP_ERROR;
}
/******************************************************************************
 * 函数名: bsp_get_standby_online()
 * 功  能: 备板是否在位
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 不在位，1在位
 * 说明:
 ******************************************************************************/
s8 bsp_get_standby_online(void)
{
	if(MCT_SLAVE == bsp_get_self_MS_state())
	{
		return MCT_SLAVE_ONLINE;
	}
	if(MCT_OPP_PD_ON == bsp_get_oppslot_pd())
	{
		if(MCT_SLAVE == bsp_get_opp_MS_state())
		{
			return MCT_SLAVE_ONLINE;
		}
	}
	return MCT_SLAVE_UNONLINE;
}
#if 0
/******************************************************************************
 * 函数名: bsp_get_self_ip
 * 功  能:获取对板ip
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_get_self_ip(u8 *ip)
{
	u8 tmp_selfip[4]={192,168,0,13};
	tmp_selfip[2]=bsp_get_slot_id();
	memcpy(tmp_selfip,ip,4);
	if(1 == g_u32HostSlaveIPdebug)
	{
		printf("SELF IP:%d,%d,%d,%d\n",ip[0],ip[1],ip[2],ip[3]);
	}
	return BSP_OK;
}
#endif

/******************************************************************************
 * 函数名: bsp_get_another_board_ip
 * 功  能:获取对板ip
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_get_another_board_ip(u8 *ip)
{
	u8 tmp_oppip[4]={192,168,1,13};
	if(MCT_OPP_PD_ON == bsp_get_oppslot_pd())
	{
		tmp_oppip[2] = (!bsp_get_slot_id()) + 1;
		memcpy(ip,tmp_oppip,4);
		if(1 == g_u32HostSlaveIPdebug)
		{
			printf("OPP IP:%d,%d,%d,%d\n",ip[0],ip[1],ip[2],ip[3]);
		}
		return BSP_OK;
	}
	return BSP_ERROR;
}
/******************************************************************************
 * 函数名: bsp_MS_IP_MAC_test
 * 功  能:测试获取ip和mac是否正确
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s8 bsp_MS_IP_MAC_test(void)
{
	u8 tmp_ip[4]={0};
	u8 tmp_mac[6]={0};
	
	bsp_get_host_ip(tmp_ip);
	printf("MASTER IP:%d,%d,%d,%d\n",tmp_ip[0],tmp_ip[1],tmp_ip[2],tmp_ip[3]);
	
	memset(tmp_ip,0,4);
	bsp_get_standby_ip(tmp_ip);
	printf("SLAVE IP:%d,%d,%d,%d\n",tmp_ip[0],tmp_ip[1],tmp_ip[2],tmp_ip[3]);
	
	memset(tmp_ip,0,4);
	bsp_get_another_board_ip(tmp_ip);
	printf("OPP IP:%d,%d,%d,%d\n",tmp_ip[0],tmp_ip[1],tmp_ip[2],tmp_ip[3]);

	bsp_get_host_mac(tmp_mac);
	printf("MASTER MAC:%x,%x,%x,%x,%x,%x\n",tmp_mac[0],tmp_mac[1],tmp_mac[2],tmp_mac[3],tmp_mac[4],tmp_mac[5]);

	memset(tmp_mac,0,6);
	bsp_get_standby_mac(tmp_mac);
	printf("SLAVE MAC:%x,%x,%x,%x,%x,%x\n",tmp_mac[0],tmp_mac[1],tmp_mac[2],tmp_mac[3],tmp_mac[4],tmp_mac[5]);
	if(MCT_SLAVE_ONLINE == bsp_get_standby_online())
	{
		printf("SLAVE ONLINE!\n");
	}
	else
	{
		printf("SLAVE NOT ONLINE!\n");
	}
}


/******************************* 源文件结束 ********************************/
