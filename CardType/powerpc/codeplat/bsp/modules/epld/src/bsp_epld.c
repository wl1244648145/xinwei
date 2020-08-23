/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bbu_config.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "fcntl.h"
#include "unistd.h"

/**************************** 私用头文件* **********************************/
#include "../../../com_inc/bsp_types.h"
#include "../inc/bsp_epld.h"
#include "../../../com_inc/bsp_spi_ext.h"
#include "../../../com_inc/bsp_epld_ext.h"
#include "../../../com_inc/fsl_p2041_ext.h"
#include "../../usdpaa/inc/compat.h"
/******************************* 局部宏定义 *********************************/


/*********************** 全局变量定义/初始化 **************************/

extern unsigned char cpld_CfgData[];
extern unsigned char cpld_UFMData[];
extern XO2FeatureRow_t cpld_FeatureRow;
extern unsigned int cpld_cfg_cnt;


/************************** 局部常数和类型定义 ************************/


/*************************** 局部函数原型声明 **************************/
// ---延迟单位为毫秒
static void bsp_real_usdelay(int dwus)
{
    unsigned long long int llstart = 0;
    unsigned long long int llend = 0;

    llstart =  BspGetCpuCycle();
    while(1)
    {
        llend =  BspGetCpuCycle();
        if ((int)(((llend-llstart)*1000)/1200)>=dwus*1000)
            break; 
    }
}
//---延迟单位为微妙
static void bsp_real_msdelay(int dwms)
{
	bsp_real_usdelay(dwms*1000);
}
/***************************函数实现 *****************************/
/*******************************************************************************
* 函数名称: bsp_read_8reg							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 正常， 其他 错误	
* 说   明: 
*******************************************************************************/
Private u8 bsp_read_8reg(u8 *u8pAddr)
{
	u8 ret = 0xFF;
    //if(is_uploading == 1)
        //return ret;
	ret = *(u8 *)u8pAddr;
	return ret;
}

/*******************************************************************************
* 函数名称: bsp_write_8reg							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 正常， 其他 错误	
* 说   明: 
*******************************************************************************/
Private void bsp_write_8reg(u8 *u8pAddr, u8 u8dwVal)
{
    //if(is_uploading == 1)
        //return;
    *u8pAddr = u8dwVal;
}

/******************************************************************************
** 函数名称:       bsp_cpld_read_reg
** 功    能:     
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* u8RegAddr     u8        input           cpld 寄存器地址
* 返回值:  0 正常，其他错误
*******************************************************************************/
u8 bsp_cpld_read_reg(u8 u8Reg_offset)
{ 
    u8 reg_data = 0xFF;
	reg_data = bsp_read_8reg(g_u8epldbase+u8Reg_offset);
	return reg_data;
}

/*****************************************************************************
*函数名称:   bsp_fpga_write_reg
*功    能:          
*函数存储类型:
*参数:
*参数名        类型        输入/输出       描述
*u8RegAddr     u8        input           cpld寄存器偏移地址
*u8Dat         u8                      设置数据
*返回值:  0 正常，其他错误
*******************************************************************************/
void bsp_cpld_write_reg(u8 u8Reg_offset,u8 u8Data)
{
	bsp_write_8reg(g_u8epldbase+u8Reg_offset,u8Data);
}

/*****************************************************************************
*函数名称:   bsp_get_resetcause
*功    能:   获取系统复位原因	        
*函数存储类型:
*参数: 无
*返回值:    
*          宏定义(值)              描述
*	RESET_CAUSE_POWER_ON(0x80)    上电复位
*	RESET_CAUSE_WATCHDOG(0x40)    看门狗复位
*	RESET_CAUSE_BUTTON(0x20) 	  按键复位
*	RESET_CAUSE_BOARD(0x10)    	  整板复位,写CPLD寄存器0x08 bit7为1
*   RESET_CAUSE_PPC(0x0)		  PPC自复位(如reboot命令)
*******************************************************************************/
u8 bsp_get_resetcause(void){
	static u8 u8IsInited = 0;
	static u8 u8ResetCause = 0;
	printf("u8Isinited=%d, u8ResetCause=0x%x\n", u8IsInited, u8ResetCause);
	if( u8IsInited == 0 ){
		u8ResetCause = bsp_cpld_read_reg(CPLD_BOARD_RESET_FLAG_REG);
		bsp_cpld_write_reg(CPLD_BOARD_RESET_FLAG_REG,0);
		bsp_cpld_write_reg(0xa,0);
		u8IsInited = 1;
	}
	printf("over, u8Isinited=%d, u8ResetCause=0x%x\n", u8IsInited, u8ResetCause);
	return u8ResetCause;
}

#if 1
/********************************************************************************
* 函数名称: spi_cpld_write							
* 功    能: cpld 代码加载                             
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
s32 wait_busy(void)
{
	u8 write_busy[4] = {0xF0,0,0,0};
	u8 read_busy;
	u8 write_status[4] ={0x3C,0,0,0};
	u8 read_status[4];
	//spi_cpld_write(write_busy,4);
	spi_cpld_read(write_busy,4,&read_busy,1);
	//spi_cpld_write(write_status,4);
	spi_cpld_read(write_status,4,read_status,4);
	if(!(read_busy&0x80)&&!(read_status[2]&0x20))
	{
		return BSP_OK;	
	}
	else
	{
		printf("read_busy=0x%x, read_status[2]=0x%x\r\n", read_busy, read_status[2]);
		return BSP_ERROR;	
	}
}

u8 read_busy(void)
{
	u8 write_busy[4] = {0xF0,0,0,0};
	u8 read_busy;
	spi_cpld_read(write_busy,4,&read_busy,1);
	return read_busy;
	//printf("read_busy =%x \n",read_busy);
}

s32 read_status(void)
{
	u8 write_status[4] ={0x3C,0,0,0};
	u8 read_status[4];
	spi_cpld_read(write_status,4,read_status,4);
	printf("read_status=%x %x %x %x\n",read_status[0],read_status[1],read_status[2],read_status[3]);
}


s32 cpld_delay(void)
{
	bsp_real_usdelay(100);
	if(wait_busy())
	{ 
	    bsp_real_usdelay(100);
	    if(wait_busy())
    	{
			printf("wait error!\n");
			return BSP_ERROR;
    	}
	}
	return BSP_OK;
}

/********************************************************************************
* 函数名称: spi_cpld_write							
* 功    能: cpld 代码加载                             
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
s32 program_cpld(u8 *data,u8 len)
{
    char data1[20]={0};
	data1[0] = 0x70;
	data1[1] = 0;
	data1[2] = 0;
	data1[3] = 1;
	memcpy(&data1[4],data,len);
	spi_cpld_write(data1,4+len);
}

s32 write_feature(u8 *data,u8 len)
{
    char data1[12]={0};
	data1[0] = 0xE4;
	data1[1] = 0;
	data1[2] = 0;
	data1[3] = 0;
	memcpy(&data1[4],data,len);
	spi_cpld_write(data1,4+len);
}

s32 write_feabits(u8 *data,u8 len)
{
    char data1[6]={0};
	data1[0] = 0xF8;
	data1[1] = 0;
	data1[2] = 0;
	data1[3] = 0;
	memcpy(&data1[4],data,len);
	spi_cpld_write(data1,4+len);
}

s32 read_ufm_cpld(u8 *data,u8 len)
{
	u8 cmd[4] = {0x73,0,0,0x01};
	spi_cpld_read(cmd,4,data,len);
	return BSP_OK;
}


void cmd_bypass(void)
{
	u8 write[4] = {0xff,0xff,0xff,0xff};
	spi_cpld_write(write,4);
}

void enable_transparent(void)
{
	u8 write[4] = {0x74,8,0,0};
	spi_cpld_write(write,4);
}

void erase_configuration_feature_flash(void)
{
	u8 write[4] = {0x0E,0xE,0,0};



	spi_cpld_write(write,4);
}

void set_address(void)
{
	u8 write[4] = {0x46,0,0,0};
	spi_cpld_write(write,4);
}

void set_flash_done(void)
{
	u8 write[4] = {0x5E,0,0,0};
	spi_cpld_write(write,4);
}

void disable_configuration(void)
{
	u8 write[3] = {0x26,0,0};
	spi_cpld_write(write,3);
}

void issue_refresh1(void)
{
	u8 write[3] = {0x79,0,0};
	spi_cpld_write(write,3);
}
void issue_refresh(void)
{
	u8 write[3] = {0x79,0,0};	
	bsp_spi_lock(1);
	spi_cpld_write(write,3);
}

s32 verify_done(void)
{
	u8 write_status[4] ={0x3C,0,0,0};
	u8 read_status[4];
	//spi_cpld_write(write_status,4);
	//spi_cpld_read(read_status,4);
	spi_cpld_read(write_status,4,read_status,4);
	if(!(read_status[0]&0x8))
	{
		return BSP_OK;	
	}
	else
	{
		return BSP_ERROR;	
	}
}

void read_dev_id(void)
{
	u8 cmd[4] = {0xe0,0,0,0};
	u8 data[4];
	//spi_cpld_cmd_read(cmd,4,data,4);
	//enable_transparent();

	//bsp_sys_usdelay(100);

	spi_cpld_read(cmd,4,data,4);
	//if(cpld_delay())
	//return BSP_ERROR;
    printf("data=%x %x %x %x\n",data[0],data[1],data[2],data[3]);
}

void read_trace_id(void)
{
	u8 cmd[4] = {0x19,0,0,0};
	u8 data[8];
	//spi_cpld_cmd_read(cmd,4,data,4);
	spi_cpld_read(cmd,4,data,8);
	//if(cpld_delay())
		//return BSP_ERROR;
    printf("data=%x %x %x %x %x %x %x %x\n",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
}


void read_usr_id(void)
{
	u8 cmd[4] = {0xc0,0,0,0};
	u8 data[4];
	//spi_cpld_cmd_read(cmd,4,data,4);
	spi_cpld_read(cmd,4,data,4);
	//if(cpld_delay())
		//return BSP_ERROR;
    printf("data=%x %x %x %x\n",data[0],data[1],data[2],data[3]);
}
/********************************************************************************
* 函数名称: spi_cpld_write							
* 功    能: cpld 代码加载                             
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
const s8 cpld_bin_name[]= "/dev/shm/cpld.jed";
u8 verify = 1;
s32 cpld_download(void)
{  
    FILE *file_cpld;
    s32 counter,remainder;	
    u8 u8ufm[16];
    u8 u8data[16];
    u16 flag = 0;
    u32 count = 0;
    u32 count1 = 0;
    struct stat *cpld_file_stat;
    u32 cpld_size;
    s32 i;
    s32 cf_size = 0;

    //stat(cpld_bin_name,cpld_file_stat);
    //cpld_size = cpld_file_stat->st_size;
    //cpld_size = sizeof(cpld_CfgData);
    cpld_size = cpld_cfg_cnt;

    counter = cpld_size / 16; 
    remainder = cpld_size % 16;

	printf("cpld_size = 0x%x,counter = %d,remainder = %d\n",cpld_size,counter,remainder);

	if(remainder)
	{
		flag = 1;
	}

	while (counter > 0)  
	{
		program_cpld(&cpld_CfgData[cf_size],16);
		if(cpld_delay()){			
			printf("aftern program_cpld error!\r\n");
			return BSP_ERROR;
		}
		count++;
		if(!(count%640))
		{
		    count1+=10;
		    printf("%d KB download!\n",count1);
		}
		
		cf_size += 16;
		counter--;
	}

	if(cpld_size - cf_size == remainder)
	{
		 printf("download size ok!\n");
	}
 	
	if(flag)
	{
		printf("remainder = %d download!\n",remainder);
		program_cpld(&cpld_CfgData[cf_size],remainder);
		if(cpld_delay()){			
			printf("last program_cpld error!\r\n");
			return BSP_ERROR;
		}
	}

	write_feature(cpld_FeatureRow.feature,8);
	bsp_real_usdelay(200);
	write_feabits(cpld_FeatureRow.feabits,2);
	bsp_real_usdelay(200);
	
	return BSP_OK;
}

/******************************************************************************
*
* 函数名称: gps_disable_tp						
* 功    能: 关闭GPS TIMEPULSE输出                              
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_disable_tp(void) 
{    
	char data0[]={0xB5,0x62,0x06,0x31,0x20,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x57,0xCB};
	char data1[]={0xB5,0x62,0x06,0x31,0x20,0x00,0x01,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x58,0xEB};
				 
	char dataRsp[300];
	int status;
	
	status = gpsExecCommand_UBlox(data0, 40, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_disable_tp0, return failure!, try again");         
	}
	
	status = gpsExecCommand_UBlox(data1, 40, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_disable_tp1, return failure!, try again");         
	}
	return status;
}
 
/******************************************************************************
*
* 函数名称: gps_Cfg_reset						
* 功    能: Reset ColdStart                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Cfg_reset(void) 
{    
	char data[]={0xB5,0x62,0x06,0x04,0x04,0x00,0xFF,0x07,0x01,0x00,0x15,0x77};
	char dataRsp[300];    
	int status;
	
	status = gpsExecCommand_UBlox(data, 12, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_Cfg_reset, return failure!, try again");         
	}
	
	return status;
}


/********************************************************************************
* 函数名称: spi_cpld_write							
* 功    能: cpld 代码加载                             
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
s32 bsp_update_cpld(void)
{
	FILE *file_cpld;
	s32 ret = 0;
	s32 i = 0;
	s32 count = 0;

	printf("update cpld start......\n");

       printf("convert jed to array......\n");
	
	if(bsp_convert_cpld_jed()== BSP_ERROR)
	{
		printf("convert cpld jed error!\n"); 
		return BSP_ERROR;
	}

    //关闭gps 1PPS中断
    gps_disable_tp();

	enable_transparent();
	if(cpld_delay()){
		printf("aftera enable_transparent error!\r\n");
		return BSP_ERROR;
	}
	erase_configuration_feature_flash();

	bsp_real_msdelay(500);
	while(read_busy()&0x80)
	{
       bsp_real_msdelay(1);
	   count++;
	   if(count>0x10000){			
			printf("while read_busy error!\r\n");
			return BSP_ERROR;
		}
	}
	
	if(cpld_delay()){		
		printf("aftern while error!\r\n");
		return BSP_ERROR;
	}
	set_address();
	while(i < 3)
    {
	    ret = cpld_download();		
		if(ret < 0)
		{
			i++;
			printf("try to download cpld %d time !\n ",i);
		}
		else
		{
			break;
		}
	}
    set_flash_done();

	if(cpld_delay()){		
		printf("aftern set_flash_done error!\r\n");
		return BSP_ERROR;
	}
	bsp_real_msdelay(1000);

	printf("[cpld update] ==> set_flash_done!\n");
	
	disable_configuration();

	printf("[cpld update] ==> disable_configuration!\n");
	
	bsp_real_msdelay(1000);

	printf("[cpld update] ==>refresh cpld,power down......,wait for reboot!\n");

    bsp_real_msdelay(500);
    
    #if 1
	issue_refresh();

    printf("[cpld update] ==>refresh cpld done!\n");

	bsp_real_msdelay(1000);
	bsp_spi_lock(0);    	
   	if(!verify_done())
	{
		printf("[BTS boot] ==> CPLD download succeed!\n");
		return BSP_OK;
	}
	else
	{
		printf("[BTS boot] ==> CPLD download failed!\n" );  
		return BSP_ERROR;
	}
    #endif
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_enable_watchdog							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	int								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
void bsp_enable_watchdog(void)
{
	u8 u8watchdog_vale;
	u8watchdog_vale = bsp_cpld_read_reg(0xc);
	bsp_cpld_write_reg(0xc,0x40|u8watchdog_vale);
}

/********************************************************************************
* 函数名称: bsp_unenable_watchdog							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	int								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
void bsp_unenable_watchdog(void)
{
	u8 u8watchdog_vale;
	u8watchdog_vale = bsp_cpld_read_reg(0xc);
	bsp_cpld_write_reg(0xc,u8watchdog_vale&(~0x40));
}

/********************************************************************************
* 函数名称: bsp_feed_watchdog							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	int								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
void bsp_feed_watchdog(void)
{
	u8 u8watchdog_vale;
	u8watchdog_vale = bsp_cpld_read_reg(0xc);
	if(u8watchdog_vale & 0x80)
	{
		bsp_cpld_write_reg(0xc,u8watchdog_vale&(~0x80));
	}
	else
	{
		bsp_cpld_write_reg(0xc,u8watchdog_vale|0x80);
	}
}

/********************************************************************************
* 函数名称: bsp_run_watchdog							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	int								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
void bsp_run_watchdog(void)
{
	bsp_enable_watchdog();	
	bsp_print_reg_info(__func__, __FILE__, __LINE__);
	while(1)
	{
		bsp_feed_watchdog();
		sleep(30);
	}
}

/********************************************************************************
* 函数名称: bsp_init_watchdog							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	int								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
void bsp_start_watchdog(void)
{
	pthread_t watchdog_thread;
	pthread_attr_t attr;
	struct sched_param param;
	int result = 0;
	int cpld_ver = bsp_cpld_read_reg(0x4);
	//if(0x4==cpld_ver){
		printf("start watchdog!\n");	
		pthread_attr_init(&attr); 
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setstacksize(&attr, 1024*1024);
		pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
		param.sched_priority = 49; 
		pthread_attr_setschedparam(&attr, &param);
		
		result = pthread_create(&watchdog_thread, &attr,(void *)bsp_run_watchdog,NULL);
		if (result != 0)
		{
			printf("pthread_create rx_thread failed\n");
			pthread_attr_destroy(&attr);
			return (-1);
		}
		
		pthread_attr_destroy(&attr);	
	//}
	return 0;
}

#define LED_OFF		0
#define LED_ON		1
#define LED_BLINK	2

#define CPLD_LED_CTL_OFFSET	6
#define LED_SUM		(4)	/* Except gps led */
#define BASE_FREQ	100000	/* 100ms */

#define LED_RUN		0	/* The index of RUN led */
#define LED_ALM		1	/* The index of ALM led */
#define LED_ACT		2	/* The index of FAN led */
#define LED_GPS		3	/* The index of GPS led */

struct led_state {
	int state;
	int blink_period;
	int counter;
	u8 bit;		/* control bit on cpld register 0x6 */
};

static struct led_state led[LED_SUM] = {
	[LED_RUN] = {
		.state		= LED_ON,
		.bit		= 0x80,
	},
	[LED_ACT] = {
		.state		= LED_OFF,
		.bit		= 0x20,
	},
	[LED_ALM] = {
		.state		= LED_OFF,
		.bit		= 0x40,
	},
	[LED_GPS] = {
		.state		= LED_OFF,
		.bit		= 0x10,
	}
};

static void led_ctl(struct led_state *state)
{
	u8 val = bsp_cpld_read_reg(CPLD_LED_CTL_OFFSET);

	switch (state->state) {
	case LED_ON:
		if ((val & state->bit) == 0)
			break;

		val &= ~state->bit;
		break;
	case LED_OFF:
		if (val & state->bit)
			break;

		val |= state->bit;
		break;
	case LED_BLINK:
		if (++state->counter < state->blink_period)
			return;

		state->counter = 0;
		val ^= state->bit;
		break;
	}
	bsp_cpld_write_reg(CPLD_LED_CTL_OFFSET, val);
}

void bsp_led_on(int index)
{
	if (index < 0 || index > LED_SUM)
		return;

	led[index].state = LED_ON;
	led[index].blink_period = 0;
}

void bsp_led_off(int index)
{
	if (index < 0 || index > LED_SUM)
		return;

	led[index].state = LED_OFF;
	led[index].blink_period = 0;
}

void bsp_led_blink(int index, int period)
{
	if (index < 0 || index > LED_SUM)
		return;

	/* min period is 100ms */
	if (period < 100)
		period = 100;

	led[index].state = LED_BLINK;
	led[index].blink_period = period * 1000 / BASE_FREQ; 	/* get nums of BASE_FREQ (100ms) */
}

static void *led_task(void *data)
{
	int i;

	printf("led task start....\n");

	while (1) {
		for (i = 0; i < LED_SUM; i++)
			led_ctl(&led[i]);

		usleep(BASE_FREQ);
	}

	return NULL;
}

int bsp_led_ctl_init(void)
{
	pthread_t pid;

	if (pthread_create(&pid, NULL, led_task, NULL) < 0) {
		perror("bsp_led_init");
		return -1;
	}

	return 0;
}

void bsp_led_run_on(void)
{
	bsp_led_on(LED_RUN);
}

void bsp_led_run_off(void)
{
	bsp_led_off(LED_RUN);
}

void bsp_led_alarm_on(void)
{
	bsp_led_on(LED_ALM);
}

void bsp_led_alarm_off(void)
{
	bsp_led_off(LED_ALM);
}

void bsp_led_act_on(void)
{
	bsp_led_on(LED_ACT);
}

void bsp_led_act_off(void)
{
	bsp_led_off(LED_ACT);
}

void bsp_led_gps_on(void)
{
	bsp_led_on(LED_GPS);
}

void bsp_led_gps_off(void)
{
	bsp_led_off(LED_GPS);
}

void bsp_led_run_blink(int period)
{
	bsp_led_blink(LED_RUN, period);
}

void bsp_led_act_blink(int period_ms)
{
	bsp_led_blink(LED_ACT, period_ms);
}

void bsp_led_alarm_blink(int period)
{
	bsp_led_blink(LED_ALM, period);
}

void bsp_led_gps_blink(int period)
{
	bsp_led_blink(LED_GPS, period);
}

void bsp_led_control(u8 on_off,u8 led_mask)
{
   	u8 reg_led;
	reg_led = bsp_cpld_read_reg(6);
	if(on_off)
	{
		bsp_cpld_write_reg(6,reg_led|led_mask);
	}
	else
	{
		bsp_cpld_write_reg(6,reg_led&led_mask);
	}
}
#if 0
void bsp_led_run_on(void)
{
	bsp_led_control(1,0x80);
}

void bsp_led_run_off(void)
{
	bsp_led_control(0,0x7f);
}

void bsp_led_alarm_on(void)
{
	bsp_led_control(1,0x40);
}

void bsp_led_alarm_off(void)
{
	bsp_led_control(0,0xbf);
}

void bsp_led_fan_on(void)
{
	bsp_led_control(1,0x20);
}

void bsp_led_fan_off(void)
{
	bsp_led_control(0,0xdf);
}
#endif
/*
void bsp_led_gps_on(void)
{
	bsp_led_control(1,0x10);
}

void bsp_led_gps_off(void)
{
	bsp_led_control(0,0xef);
}
*/
void bsp_led_init(void)
{
	bsp_led_run_blink(100);
	bsp_led_alarm_off();
	bsp_led_act_off();
	bsp_led_gps_off();
	bsp_led_ctl_init();
}
#endif

void bsp_write_long_frame_to_cpld(u32 u32Value)
{
    bsp_cpld_write_reg(CPLD_GPS_MSG_0,u32Value&0xff);
    bsp_cpld_write_reg(CPLD_GPS_MSG_1,(u32Value&0xff00)>>8);
    bsp_cpld_write_reg(CPLD_GPS_MSG_2,(u32Value&0xff0000)>>16);
    bsp_cpld_write_reg(CPLD_GPS_MSG_3,(u32Value&0xf000000)>>24);
}
void bsp_write_long_frame_flag_to_cpld(u8 u8Value)
{
    bsp_cpld_write_reg(CPLD_GPS_MSG_FLAG, u8Value);
}
u32 bsp_read_long_frame_to_cpld()
{
    return bsp_cpld_read_reg(CPLD_GPS_MSG_0)&0xff | \
        (bsp_cpld_read_reg(CPLD_GPS_MSG_1) << 8)&0xff00 | \
        (bsp_cpld_read_reg(CPLD_GPS_MSG_2) << 16)&0xff0000 | \
        (bsp_cpld_read_reg(CPLD_GPS_MSG_3) << 24)&0xf000000;
}

/******************************* 源文件结束 ********************************/

