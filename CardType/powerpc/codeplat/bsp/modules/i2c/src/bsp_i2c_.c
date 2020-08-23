/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_i2c_gpio.c
* 功能:                gpio I2C 模拟  
* 版本:                                                                  
* 编制日期:                              
* 作者:                  hjf                            
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>

/**************************** 私用头文件* **********************************/
#include "../../../com_inc/bsp_types.h"
#include "../../../com_inc/fsl_p2041_ext.h"
#include "../../../com_inc/bsp_fpga_ext.h"
#include "../../../com_inc/bsp_i2c_ext.h"
#include "../../../com_inc/bsp_epld_ext.h"
/******************************* 局部宏定义 *********************************/
#define GPIO_SET_OUTPUT21  (0x1<<10)
#define GPIO_SET_OUTPUT22  (0x1<<9)
#define GPIO_SET_OUTPUT23  (0x1<<8)
#define SFP_CPLD_CS_REG     0x2e    /* SFP片选寄存器 */

/*********************** 全局变量定义/初始化 **************************/
volatile u32 *g_gpiocr = NULL;
volatile u32 *g_gpiodr = NULL;

pthread_mutex_t  g_mp_gpio_i2c = PTHREAD_MUTEX_INITIALIZER;  /* 互斥 gpio I2C访问*/
pthread_mutexattr_t  g_mattr_gpio_i2c;
unsigned char g_BBU_Fiber_INfo[128]={0};
unsigned char g_BBU_Fiber_vendor_name[16]={0};
unsigned char g_print_bbu_fiber_info = 0;
unsigned short g_BBU_Fiber_Transid = 0;
const UINT16 M_EMS_BTS_BBU_Fiber_Info_RSP = 0x07D3;
int i2c_gpio_debug;


fiber_info fiber_rru_data;
FiberInfoLimint fiber_para;

/************************** 局部常数和类型定义 ************************/
//GPIO21	PPC_FPGA_SFP_IIC_SCL	P2041->FPGA	 SCL OUT
//GPIO22	PPC_FPGA_SFP_IIC_SDA	P2041->FPGA	 SDA OUT 
//GPIO23	FPGA_PPC_SFP_IIC_SDA	FPGA->P2041	 SDA INT
/*************************** 局部函数原型声明 **************************/
//extern void bsp_msdelay(ul32 dwTimeOut);// ---延迟单位为毫秒
//extern void bsp_usdelay(ul32 dwTimeOut); //---延迟单位为微妙

/************************************ 函数实现 ************************* ****/
#if 0
/********************************************************************************
* 函数名称: bsp_msdelay							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-08-15
*********************************************************************************/
void bsp_msdelay(ULONG dwTimeOut) 
{	
	int i,j;
	for(j = 0;j < dwTimeOut;j++)	
	{
		for(i = 0;i <100000;i++)	
			;
	}
}
/********************************************************************************
* 函数名称: bsp_usdelay							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-08-15
*********************************************************************************/
void bsp_usdelay(ULONG dwTimeOut) 
{	
	int i,j;
	for(j = 0;j < dwTimeOut;j++)
	{
		for(i = 0;i < 100;i++)	
			;
	}
}
#else
#include "../../usdpaa/inc/compat.h"
/********************************************************************************
* 函数名称: bsp_msdelay							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-08-15
*********************************************************************************/
void bsp_msdelay(int dwms)
{

    int x,y;
    unsigned long long int llstart = 0;
    unsigned long long int llend = 0;

    llstart =  BspGetCpuCycle();
    //for(x=dwms;x>0;x--)
    //    for(y=98100;y>0;y--);
    
    //llend =  BspGetCpuCycle();
    while(1)
    {
        llend =  BspGetCpuCycle();
        if ((int)(((llend-llstart)*1000)/1200)>=dwms*1000000)
            break; 
    }
    //printf("%d cost time %d ns\n",dwms,(int)(((llend-llstart)*1000)/1200));
}
/********************************************************************************
* 函数名称: bsp_usdelay							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-08-15
*********************************************************************************/
void bsp_usdelay(int dwus)
{

    int x,y;
    unsigned long long int llstart = 0;
    unsigned long long int llend = 0;

    llstart =  BspGetCpuCycle();
    //for(x=dwus;x>0;x--)
    //    for(y=98100;y>0;y--);
    
    //llend =  BspGetCpuCycle();
    while(1)
    {
        llend =  BspGetCpuCycle();
        if ((int)(((llend-llstart)*1000)/1200)>=dwus*1000)
            break; 
    }
    //printf("%d cost time %d ns\n",dwus,(int)(((llend-llstart)*1000)/1200));
}
#endif
/************************************ 函数实现 ************************* ****/


#if 1
s32 bsp_gpio_i2c_mutex_init(void)
{
	s32 s32ret = 0;
	/*initializea mutext oits default value*/
	s32ret |=pthread_mutex_init(&g_mp_gpio_i2c, NULL);
	return s32ret;
}
/********************************************************************************
* 函数名称: gpio_i2c_init 							
* 功    能: 初始化GPIO ,I2C接口                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
void bsp_gpio_i2c_init(void)
{	
    g_gpiocr = (volatile u32 *)((u32)g_u8ccsbar+0x130000);
	g_gpiodr = (volatile u32 *)((u32)g_u8ccsbar+0x130008);
	printf("g_gpiocr = 0x%x\n",g_gpiocr);
	printf("bsp_gpio_i2c_init!\n");
	bsp_gpio_i2c_mutex_init();
	*g_gpiocr |= GPIO_SET_OUTPUT21|GPIO_SET_OUTPUT22;  //初始化I/O口21,22输出,23输入
}

/********************************************************************************
* 函数名称: i2c_wait 							
* 功    能: 设置总线频率                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void gpio_i2c_wait(void)
{
	int i;
	bsp_usdelay(10);
}
/********************************************************************************
* 函数名称: set_SDA 							
* 功    能: sda 高低                              
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void gpio_SDA_out(u8 sda_num)
{
 	if(sda_num)
	{
    	 *g_gpiodr |= GPIO_SET_OUTPUT22;
	}
	else
	{
	 	*g_gpiodr &= ~GPIO_SET_OUTPUT22;
	}
}
/********************************************************************************
* 函数名称: get_SDA 							
* 功    能: 读I/O口                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
u8 gpio_SDA_in(void)
{
   if(*g_gpiodr& GPIO_SET_OUTPUT23)
   {
        //printf("\n0x%x\n",*g_gpiodr);
		return 1;
   }
   else
   {
		return 0;
   }
	//return (*g_gpiodr& GPIO_SET_OUTPUT23)?1:0;
}

/********************************************************************************
* 函数名称: set_SCL 							
* 功    能: scl 高低                               
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void gpio_set_SCL(u8 u8num)
{
 	if(u8num)
	{
    	 *g_gpiodr |= GPIO_SET_OUTPUT21;
	}
	else
	{
	 	 *g_gpiodr &= ~GPIO_SET_OUTPUT21;
	}    
}

/********************************************************************************
* 函数名称: gpio_i2c_start 							
* 功    能:                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void gpio_i2c_start(void)
{
	/* set SCL to 1 */
    gpio_set_SCL(1);
    gpio_i2c_wait();

	/* set SDA to 1 */
    gpio_SDA_out(1);
	gpio_i2c_wait();

	/* set SDA to 0 */	
    gpio_SDA_out(0);
    gpio_i2c_wait();
	
	/* set SCL to 0 */	
    gpio_set_SCL(0);
}
/********************************************************************************
* 函数名称: gpio_i2c_start 							
* 功    能:                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void gpio_i2c_reset(void)
{
	/* set SCL SDA to 1 */
    gpio_set_SCL(1);
    gpio_SDA_out(1);
	bsp_sys_msdelay(1);
}
/********************************************************************************
* 函数名称: gpio_i2c_stop 							
* 功    能:                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void gpio_i2c_stop(void)
{
    /* set SDA to 0 */	
    gpio_SDA_out(0);
    gpio_i2c_wait();

	/* set SCL to 1 */
    gpio_set_SCL(1);
    gpio_i2c_wait();

    /* set SDA to 1 */
    gpio_SDA_out(1);
}
/********************************************************************************
* 函数名称: gpio_send_ack 							
* 功    能:                               
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void gpio_send_ack(u8 u8ack) 
{   
    gpio_SDA_out(u8ack);
	gpio_i2c_wait();

	/* toggle clock */
    gpio_set_SCL(1);
    gpio_i2c_wait();
    gpio_set_SCL(0);
	gpio_i2c_wait();

	/* set SDA to 1 */
	gpio_SDA_out(1);
	gpio_i2c_wait();
}
/********************************************************************************
* 函数名称: gpio_read_ack 							
* 功    能:                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private u8 gpio_read_ack(void) 
{  
	u8  u8ack = 0;  
    u32 u32ackcount;
	gpio_set_SCL(1); 
	gpio_i2c_wait();

	
	/*if (gpio_SDA_in()) 
	{
		gpio_i2c_wait();
		if (gpio_SDA_in())
		{
			u8ack =1;
			printf("ack is:%x\n",u8ack);
		}
	}
	else
	{
		u8ack = 0;
	}*/	

	while(gpio_SDA_in())
	{
		gpio_i2c_wait();
		if(u32ackcount++ > 1000)
		{
			u8ack = 1;
			printf("ack is:%x\n",u8ack);
			break;
		}
	}

	gpio_set_SCL(0); 
	gpio_i2c_wait();	

	return u8ack;
}

/********************************************************************************
* 函数名称: I2C_Send_Byte 							
* 功    能: 向总线上发送一个字节                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private u8 i2c_send_byte(u8 u8send_bytedata) 
{
    u8 i;

    for (i = 0; i < 8; i++)  
    {
		if (u8send_bytedata & 0x80) 
		{
		    //printf("1");
			gpio_SDA_out(1);
		}
		else
		{
		    //printf("0");
			gpio_SDA_out(0);
		}
		u8send_bytedata <<= 1;
		
	   /* toggle clock */
		gpio_i2c_wait();
		gpio_set_SCL(1);
		gpio_i2c_wait();
		gpio_set_SCL(0);
    }
	//gpio_SDA_out(1);
	//gpio_i2c_wait();		
	return gpio_read_ack();
}

/********************************************************************************
* 函数名称: I2C_Send_Byte 							
* 功    能: 接收一个字节                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private s32 i2c_receive_byte(u8 *u8preceive_data)  
{
    u8 i;
    u8 u8bytedata = 0;
    u8 u8temp;

	/* set SDA input */
    for (i = 0; i < 8; i++)
    {
        /* set SCL high */
        gpio_i2c_wait();
        gpio_set_SCL(1);
        gpio_i2c_wait();

        u8bytedata = u8bytedata<<1;
        u8temp = gpio_SDA_in(); 
        if (u8temp)
          u8bytedata |= 0x01; 

		/* set SCL low */		
		gpio_i2c_wait();
        gpio_set_SCL(0);
        gpio_i2c_wait();
    }

    /* set SDA output */	
    *u8preceive_data = u8bytedata;
	return BSP_OK;
}


/********************************************************************************
* 函数名称: bsp_gpio_i2c_Read 							
* 功    能: 读设备                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 此函数只限于I2C设备从地址小于一个字节
*********************************************************************************/
s32 bsp_gpio_i2c_read(u8 u8device_ID,u8 u8address,u8 *u8pdata,u32 u8byte_num)
{   
	u8 u8ack;
	u32 i;
	int ret = BSP_OK;

	pthread_mutex_lock(&g_mp_gpio_i2c);
	gpio_i2c_start();
	//printf("read sfp error!\n");
	u8ack = i2c_send_byte(u8device_ID);
	//printf("read sfp error2!\n");
	if (u8ack)
	{
		gpio_i2c_reset();
		ret = BSP_ERROR;
		goto out;
	}
	u8ack = i2c_send_byte(u8address);
	//printf("read sfp error44!\n");
	if (u8ack)
	{
		gpio_i2c_reset();
		ret = BSP_ERROR;
		goto out;
	}

	gpio_i2c_start();
	//printf("read sfp error2222!\n");
	u8ack = i2c_send_byte(u8device_ID+1);  //读命令
	//printf("read sfp error3!\n");
	if (u8ack)
	{
		gpio_i2c_reset();
		ret = BSP_ERROR;
		goto out;
	}
	//printf("read sfp error4!\n");
	for(i = 0;i < u8byte_num;i++)
	{
		//printf("5!\n");
		i2c_receive_byte(&u8pdata[i]);
		if(i == u8byte_num - 1) 
			gpio_send_ack(1);  
		else      
			gpio_send_ack(0);
	}
	//printf("read sfp error5!\n");
	gpio_i2c_stop();    
out:
	pthread_mutex_unlock(&g_mp_gpio_i2c);
	return ret;
}


/********************************************************************************
* 函数名称: bsp_gpio_i2c_write 							
* 功    能: 写设备                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 此函数只限于I2C设备从地址小于一个字节
*********************************************************************************/
s32 bsp_gpio_i2c_write(u8 u8device_ID,u8 u8address,u8 *u8pdata,u32 u8byte_num)
{   
	u8 u8ack;
	u32 i;
	int ret = BSP_OK;

	pthread_mutex_lock(&g_mp_gpio_i2c);
	printf("device_ID is:%x\n",u8device_ID);
	printf("address is:%x\n",u8address);
	gpio_i2c_start();
	//返回一个应答值
	u8ack = i2c_send_byte(u8device_ID); //设备地址
	if (u8ack)
	{
		gpio_i2c_stop();
		ret = BSP_ERROR;
		goto out;
	}

	u8ack = i2c_send_byte(u8address); //片内地址
	if (u8ack)
	{
		gpio_i2c_stop();
		ret = BSP_ERROR;
		goto out;
	}

	for(i=0;i<u8byte_num;i++)
	{    
		printf("0x%x\n",u8pdata[i]);
		u8ack = i2c_send_byte(u8pdata[i]); //发送一个字节
		if(u8ack)
		{
			gpio_i2c_stop();
			ret = BSP_ERROR;
			goto out;
		}
	}

	gpio_i2c_stop();
out:
	pthread_mutex_unlock(&g_mp_gpio_i2c);
	return ret;
}

/********************************************************************************
* 函数名称: bsp_read_sfp 							
* 功    能:                               
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
            u8sfp_cs: sfp片选，1,2,3,4
			u8sfp_device_ID：设备从地址, 0xA0,0xA1
			u8address：设备内部子地址
			pu8read_data：接收数据指针
			u8byte_num：数据长度
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_read_sfp(u8 u8sfp_cs,u8 u8sfp_device_ID,u8 u8address,u8 *pu8read_data,u32 u8byte_num)
{
 	switch(u8sfp_cs)
 	{
	 	case 0:
			bsp_cpld_write_reg(SFP_CPLD_CS_REG,0);/*需要确认cpld寄存器号及配置方法*/
			break;
		case 1:
			bsp_cpld_write_reg(SFP_CPLD_CS_REG,1);
			break;

		default:
			printf("sfp id wrong !\n");
			return BSP_ERROR;
 	}
	bsp_msdelay(10);
	if(BSP_ERROR == bsp_gpio_i2c_read(u8sfp_device_ID,u8address,pu8read_data,u8byte_num))
	{
		if(BSP_ERROR == bsp_gpio_i2c_read(u8sfp_device_ID,u8address,pu8read_data,u8byte_num))
		{
			if(BSP_ERROR == bsp_gpio_i2c_read(u8sfp_device_ID,u8address,pu8read_data,u8byte_num))
			{
				printf("read sfp error!\n");
				return BSP_ERROR;
			}
		}
	}
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_write_sfp 							
* 功    能: 写设备                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述
			u8sfp_cs: sfp片选，1,2,3,4
			u8sfp_device_ID：设备从地址, 0xA0,0xA2,地址不需要左移一位
			u8address：设备内部子地址
			pu8read_data：发送数据指针
			u8byte_num：数据长度
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_write_sfp(u8 u8sfp_cs,u8 u8sfp_device_ID,u8 u8address,u8 *pu8write_data,u32 u8byte_num )
{
    u8 i;
 	switch(u8sfp_cs)
 	{
	 	case 1:
			bsp_cpld_write_reg(SFP_CPLD_CS_REG,0);/*需要确认fpga寄存器号及配置方法*/
			break;
		case 2:
			bsp_cpld_write_reg(SFP_CPLD_CS_REG,1);
			break;
		case 3:
			bsp_cpld_write_reg(SFP_CPLD_CS_REG,2);
			break;
		case 4:
			bsp_cpld_write_reg(SFP_CPLD_CS_REG,3);
			break;
		default:
			printf("sfp id wrong !\n");
			return BSP_ERROR;
 	}

	for(i = 0;i < u8byte_num;i++)
    {	
		if(BSP_ERROR == bsp_gpio_i2c_write(u8sfp_device_ID,u8address+i,pu8write_data+i,1))
		{
			printf("write sfp error!\n");
			return BSP_ERROR;
		}
		bsp_msdelay(10);
	}	
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_get_sfp_reg 							
* 功    能: 写设备                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述
			u8sfp_cs: sfp片选，1,2,3,4
			u8sfp_device_ID：设备从地址, 0xA0,0xA2
			u8address：设备内部子地址
			pu8read_data：发送数据指针
			u8byte_num：数据长度
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_get_sfp_reg(u8 u8sfp_cs,u8 u8address,u8 *u8read_sfp_data,u32 u8byte_num)
{
	if(BSP_OK != bsp_read_sfp(u8sfp_cs,0xa2,u8address,u8read_sfp_data,u8byte_num))
	{
		printf("bsp_get_sfp_reg error!\n");
		return BSP_ERROR;
	}
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_get_sfp_reg0 							
* 功    能: 获取光模块A0地址信息                               
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述
			u8sfp_cs: sfp片选，1,2,3,4
			u8sfp_device_ID：设备从地址, 0xA0,0xA2
			u8address：设备内部子地址
			pu8read_data：发送数据指针
			u8byte_num：数据长度
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_get_sfp_reg_a0(u8 u8sfp_cs,u8 u8address,u8 *u8read_sfp_data,u32 u8byte_num)
{
	if(BSP_OK != bsp_read_sfp(u8sfp_cs,0xa0,u8address,u8read_sfp_data,u8byte_num))
	{
		printf("bsp_get_sfp_reg error!\n");
		return BSP_ERROR;
	}
	return BSP_OK;
}

/*********************************************************
56-59 4 Rx_PWR(4) Single precision floating point calibration data - Rx optical power. Bit7 of byte 56 is MSB. Bit 0 of byte 59 is LSB. Rx_PWR(4) should be set to zero for “internally calibrated” devices.
60-63 4 Rx_PWR(3) Single precision floating point calibration data - Rx optical power.Bit 7 of byte 60 is MSB. Bit 0 of byte 63 is LSB. Rx_PWR(3) should be set to zero for “internally calibrated” devices.
64-67 4 Rx_PWR(2) Single precision floating point calibration data, Rx optical power.Bit 7 of byte 64 is MSB, bit 0 of byte 67 is LSB. Rx_PWR(2) should be set to zero for “internally calibrated” devices.
68-71 4 Rx_PWR(1) Single precision floating point calibration data, Rx optical power. Bit 7of byte 68 is MSB, bit 0 of byte 71 is LSB. Rx_PWR(1) should be set to 1 for “internally calibrated” devices.
72-75 4 Rx_PWR(0) Single precision floating point calibration data, Rx optical power. Bit 7of byte 72 is MSB, bit 0 of byte 75 is LSB. Rx_PWR(0) should be set to zero for “internally calibrated” devices.
76-77 2 Tx_I(Slope) Fixed decimal (unsigned) calibration data, laser bias current. Bit 7 of byte 76 is MSB, bit 0 of byte 77 is LSB. Tx_I(Slope) should be set to 1 for “internally calibrated” devices.
78-79 2 Tx_I(Offset) Fixed decimal (signed two’s complement) calibration data, laser bias current. Bit 7 of byte 78 is MSB, bit 0 of byte 79 is LSB. Tx_I(Offset) should be set to zero for “internally calibrated” devices.
80-81 2 Tx_PWR(Slope) Fixed decimal (unsigned) calibration data, transmitter coupled output power. Bit 7 of byte 80 is MSB, bit 0 of byte 81 is LSB. Tx_PWR(Slope) should be set to 1 for “internally calibrated” devices.
82-83 2 Tx_PWR(Offset) Fixed decimal (signed two’s complement) calibration data, transmitter coupled output power. Bit 7 of byte 82 is MSB, bit 0 of byte 83 is LSB. Tx_PWR(Offset) should be set to zero for “internally calibrated” devices.
84-85 2 T (Slope) Fixed decimal (unsigned) calibration data, internal module temperature. Bit 7 of byte 84 is MSB, bit 0 of byte 85 is LSB.T(Slope) should be set to 1 for “internally calibrated” devices.
86-87 2 T (Offset) Fixed decimal (signed two’s complement) calibration data, internal module temperature. Bit 7 of byte 86 is MSB, bit 0 of byte 87 is LSB.T(Offset) should be set to zero for “internally calibrated” devices.
88-89 2 V (Slope) Fixed decimal (unsigned) calibration data, internal module supply voltage. Bit 7 of byte 88 is MSB, bit 0 of byte 89 is LSB. V(Slope)should be set to 1 for “internally calibrated” devices.
90-91 2 V (Offset) Fixed decimal (signed two’s complement) calibration data, internal  module supply voltage. Bit 7 of byte 90 is MSB. Bit 0 of byte 91 is LSB. VLSB. V(Offset) should be set to zero for “internally calibrated”devices.

96 All Temperature MSB Internally measured module temperature.
97 All Temperature LSB
98 All Vcc MSB Internally measured supply voltage in transceiver.
99 All Vcc LSB
100 All TX Bias MSB Internally measured TX Bias Current.
101 All TX Bias LSB
102 All TX Power MSB Measured TX output power.
103 All TX Power LSB
104 All RX Power MSB Measured RX input power.
105 All RX Power LSB
106-109 All Unallocated Reserved for future diagnostic definitions
***************************************************************************/


/********************************************************************
After calibration per the equations given below for each variable, the results are consistent with
the accuracy and resolution goals for internally calibrated devices.
1) Internally measured transceiver temperature. Module temperature, T, is given by the
following equation: T(C) = Tslope * TAD (16 bit signed twos complement value) + Toffset. The result is
in units of 1/256C, yielding a total range of –128C to +128C. See Table 3.16 for locations of
TSLOPE and TOFFSET. Temperature accuracy is vendor specific but must be better than ±3 degrees
Celsius over specified operating temperature and voltage. Please see vendor specification
sheet for details on location of temperature sensor. Tables 3.13 and 3.14 above give
examples of the 16 bit signed twos complement temperature format.
2) Internally measured supply voltage. Module internal supply voltage, V, is given in microvolts
by the following equation: V(uV) = VSLOPE * VAD (16 bit unsigned integer) + VOFFSET. The result is in
units of 100uV, yielding a total range of 0 – 6.55V. See Table 3.16 for locations of VSLOPE and
VOFFSET. Accuracy is vendor specific but must be better than ±3% of the manufacturer’s nominal
value over specified operating temperature and voltage. Note that in some transceivers,
transmitter supply voltage and receiver supply voltage are isolated. In that case, only one
supply is monitored. Refer to the manufacturer’s specification for more detail.
3) Measured transmitter laser bias current. Module laser bias current, I, is given in microamps
by the following equation: I (uA) = ISLOPE * IAD (16 bit unsigned integer) + IOFFSET. This result is in
units of 2 uA, yielding a total range of 0 to 131 mA. See Table 3.16 for locations of ISLOPE and
IOFFSET. Accuracy is vendor specific but must be better than ±10% of the manufacturer’s nominal
value over specified operating temperature and voltage.
4) Measured coupled TX output power. Module transmitter coupled output power, TX_PWR,
is given in uW by the following equation: TX_PWR (uW) = TX_PWRSLOPE * TX_PWRAD (16 bit
unsigned integer) + TX_PWROFFSET. This result is in units of 0.1uW yielding a total range of 0 –
6.5mW. See Table 3.16 for locations of TX_PWRSLOPE and TX_PWROFFSET. Accuracy is vendor
specific but must be better than ±3dB over specified operating temperature and voltage. Data
is assumed to be based on measurement of a laser monitor photodiode current. It is factory
calibrated to absolute units using the most representative fiber output type. Data is not valid
when the transmitter is disabled.
Published SFF-8472 Rev 11.0
Diagnostic Monitoring Interface for Optical Transceivers Page 30
5) Measured received optical power. Received power, RX_PWR, is given in uW by the
following equation:
Rx_PWR (uW) = Rx_PWR(4) * Rx_PWRAD
4 (16 bit unsigned integer) +
Rx_PWR(3) * Rx_PWRAD
3 (16 bit unsigned integer) +
Rx_PWR(2) * Rx_PWRAD
2 (16 bit unsigned integer) +
Rx_PWR(1) * Rx_PWRAD (16 bit unsigned integer) +
Rx_PWR(0)
The result is in units of 0.1uW yielding a total range of 0 – 6.5mW. See Table 3.16 for
locations of Rx_PWR(4-0). Absolute accuracy is dependent upon the exact optical
wavelength. For the vendor specified wavelength, accuracy shall be better than ±3dB over
specified temperature and voltage. This accuracy shall be maintained for input power levels up
to the lesser of maximum transmitted or maximum received optical power per the appropriate
standard. It shall be maintained down to the minimum transmitted power minus cable plant
loss (insertion loss or passive loss) per the appropriate standard. Absolute accuracy beyond
this minimum required received input optical power range is vendor specific.

const UINT16 ALM_ID_BTS_FIBER_VOL = 0x0105;
const UINT16 ALM_ID_BTS_FIBER_Current = 0x0106;
const UINT16 ALM_ID_BTS_FIBER_TX_PWR = 0x0107;
const UINT16 ALM_ID_BTS_FIBER_RX_PWR = 0x0108;

const UINT16 ALM_ID_RRU_FIBER_VOL = 0x0d09;
const UINT16 ALM_ID_RRU_FIBER_Current = 0x0d0A;
const UINT16 ALM_ID_RRU_FIBER_TX_PWR = 0x0d0B;
const UINT16 ALM_ID_RRU_FIBER_RX_PWR = 0x0d0C;
**********************************************************************/

#if 0
/********************************************************************************
* 函数名称: read_sfp 							
* 功    能:                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
int read_sfp(void)
{
	 if(BSP_OK != bsp_get_sfp_reg(1,0,g_BBU_Fiber_INfo,128))
	{
		printf("bsp_get_sfp_reg error!\n");
		return BSP_ERROR;
	}
}
#endif
/********************************************************************************
* 函数名称: fiber_set_info_limint 							
* 功    能:                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
字段名称	          长度（Byte）	类型	定义	 描述
	光模块电压告警门限	       4	     uV	    2000000uV	
	光模块电流告警门限	       4	     uA	    8000uA	
	光模块发射功率告警门限	   4	     uW	    300uW	
	光模块接收功率告警门限	   4	     uW	    200uW	
*********************************************************************************/
s32 fiber_set_info_limint(void)
{
    if(fiber_para.initialized!=0x5555aaaa)
    {
		fiber_para.initialized = 0x5555aaaa;
		fiber_para.Voltage = 2000000;
		fiber_para.Current = 8000;
		fiber_para.Tx_Power = 150;
		fiber_para.Rx_Power = 50;
    }
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_get_fiber_Info 							
* 功    能:                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_get_fiber_Info(unsigned char flag,u8 u8sfp_cs,fiber_info *fiber_data)
{
    s32 s32i;
    float  *Rx_PWR4,*Rx_PWR3,*Rx_PWR2,*Rx_PWR1,*Rx_PWR0; /*RX_POWER*/
    short  *Tx_I_O,*TX_PWR_O,*T_O,*V_O;//offset;
    float  TX_I_S,TX_PWR_S,T_S,V_S;//scope
    short *TX_I_AD,*TX_PWR_AD,*T_AD,*V_AD,*RX_PWR_AD;//AD
    float Temper,Current ,Vol,Tx_pwer,Rx_pwer;

    if(BSP_OK != bsp_get_sfp_reg(u8sfp_cs,0,g_BBU_Fiber_INfo,128))
    {
    	printf("bsp_get_sfp_reg error!\n");
    	return BSP_ERROR;
    }

    if(flag == 1)
    {
    	g_print_bbu_fiber_info = 1;
    }
    else
    {
    	g_print_bbu_fiber_info = 0;
    }

    if(g_print_bbu_fiber_info)
    {
        for(s32i =0; s32i < 128; s32i++)
        {
        	  printf("0x%x, ",g_BBU_Fiber_INfo[s32i]);
        	  if((s32i>0)&&(s32i%10==0))
        	  {
        	     printf("\n");
        	  }
        }
    }
   
    /* ******************************** 接收功率***************************************/ 
    /*
    56-59 4 Rx_PWR(4) Single precision floating point calibration data - Rx optical power. Bit7 of byte 56 is MSB. Bit 0 of byte 59 is LSB. Rx_PWR(4) should be set to zero for “internally calibrated” devices.
    60-63 4 Rx_PWR(3) Single precision floating point calibration data - Rx optical power.Bit 7 of byte 60 is MSB. Bit 0 of byte 63 is LSB. Rx_PWR(3) should be set to zero for “internally calibrated” devices.
    64-67 4 Rx_PWR(2) Single precision floating point calibration data, Rx optical power.Bit 7 of byte 64 is MSB, bit 0 of byte 67 is LSB. Rx_PWR(2) should be set to zero for “internally calibrated” devices.
    68-71 4 Rx_PWR(1) 
    104 All RX Power MSB Measured RX input power.
    105 All RX Power LSB
    5) Measured received optical power. Received power, RX_PWR, is given in uW by the
    following equation:
    Rx_PWR (uW) = Rx_PWR(4) * Rx_PWRAD
    4 (16 bit unsigned integer) +
    Rx_PWR(3) * Rx_PWRAD
    3 (16 bit unsigned integer) +
    Rx_PWR(2) * Rx_PWRAD
    2 (16 bit unsigned integer) +
    Rx_PWR(1) * Rx_PWRAD (16 bit unsigned integer) +
    Rx_PWR(0)
    The result is in units of 0.1uW yielding a total range of 0 – 6.5mW.
    **************************************************************************************/
    Rx_PWR4 = (float*)(&g_BBU_Fiber_INfo[56]);
    Rx_PWR3 = (float*)(&g_BBU_Fiber_INfo[60]);
    Rx_PWR2 = (float*)(&g_BBU_Fiber_INfo[64]);
    Rx_PWR1 = (float*)(&g_BBU_Fiber_INfo[68]);
    Rx_PWR0 = (float*)(&g_BBU_Fiber_INfo[72]);

    RX_PWR_AD = ( short*)(&g_BBU_Fiber_INfo[104]);

    Rx_pwer = (*Rx_PWR4)*(*RX_PWR_AD^4) +  (*Rx_PWR3)*(*RX_PWR_AD^3)  +( *Rx_PWR2)*(*RX_PWR_AD^2) + (*Rx_PWR1)*(*RX_PWR_AD) + (*Rx_PWR0);
    Rx_pwer *= 0.1;
    if(g_print_bbu_fiber_info)
    {
    	printf("\nBBU Fiber optic module Rx_Powert:%6.2f(uW)\n",Rx_pwer);
    }	

    /********************************* 电流 ***************************************/ 
    /*
    76-77 2 Tx_I(Slope) Fixed decimal (unsigned) calibration data, laser bias current. Bit 7 of byte 76 is MSB, bit 0 of byte 77 is LSB. Tx_I(Slope) should be set to 1 for “internally calibrated” devices.
    78-79 2 Tx_I(Offset) Fixed decima
    100 All TX Bias MSB Internally measured TX Bias Current.
    101 All TX Bias LSB

    3) Measured transmitter laser bias current. Module laser bias current, I, is given in microamps
    by the following equation: I (uA) = ISLOPE * IAD (16 bit unsigned integer) + IOFFSET. This result is in
    units of 2 uA
    ***********************************************************************************/
    TX_I_S = g_BBU_Fiber_INfo[76] +g_BBU_Fiber_INfo[77]*0.004;
    Tx_I_O = (short*)(&g_BBU_Fiber_INfo[78]);

    TX_I_AD = (short*)(&g_BBU_Fiber_INfo[100]);

    Current = ((TX_I_S)*(*TX_I_AD)+(*Tx_I_O))*2;
    if(g_print_bbu_fiber_info)
    {
    	printf("BBU Fiber optic module Current:%6.2f(uA)\n",Current);
    }

    /********************************* 发送功率 ***************************************/ 
    /*
    80-81 2 Tx_PWR(Slope) Fixed decimal (unsigned) calibration data, transmitter coupled output power. Bit 7 of byte 80 is MSB, bit 0 of byte 81 is LSB. Tx_PWR(Slope) should be set to 1 for “internally calibrated” devices.
    82-83 2 Tx_PWR(Offset) Fixed
    102 All TX Power MSB Measured TX output power.
    103 All TX Power LSB

    4) Measured coupled TX output power. Module transmitter coupled output power, TX_PWR,
    is given in uW by the following equation: TX_PWR (uW) = TX_PWRSLOPE * TX_PWRAD (16 bit
    unsigned integer) + TX_PWROFFSET. This result is in units of 0.1uW 
    ***********************************************************************************/
    TX_PWR_S =  g_BBU_Fiber_INfo[80] +g_BBU_Fiber_INfo[81]*0.004;
    TX_PWR_O =(short*)(&g_BBU_Fiber_INfo[82]);

    TX_PWR_AD = ( short*)(&g_BBU_Fiber_INfo[102]);

    Tx_pwer = (TX_PWR_S*(*TX_PWR_AD) +(*TX_PWR_O))*0.1;
    if(g_print_bbu_fiber_info)
    {
    	printf("BBU Fiber optic module Tx_Powert:%6.2f(uW)\n",Tx_pwer);
    }	

    /********************************* 温度 *****************************************/ 
    /*  
    84-85 2 T (Slope) Fixed decimal (unsigned) calibration data, internal module temperature. Bit 7 of byte 84 is MSB, bit 0 of byte 85 is LSB.T(Slope) should be set to 1 for “internally calibrated” devices.
    86-87 2 T (Offset) Fixed decimal
    96 All Temperature MSB Internally measured module temperature.
    97 All Temperature LSB
       
    1) Internally measured transceiver temperature. Module temperature, T, is given by the
    following equation: T(C) = Tslope * TAD (16 bit signed twos complement value) + Toffset. The result is
    in units of 1/256C,
    ***********************************************************************************/
    T_S = g_BBU_Fiber_INfo[84] +g_BBU_Fiber_INfo[85]*0.004;
    T_O =(short*)(&g_BBU_Fiber_INfo[86]);

    T_AD = ( short*)(&g_BBU_Fiber_INfo[96]);

    Temper = (T_S*(*T_AD) + (*T_O))*0.004;
    if(g_print_bbu_fiber_info)
    {
    	printf("BBU Fiber optic module Temperature:%6.2f(℃)\n",Temper);
    }
  
    /********************************* 电压 ***************************************/ 
    /*
    88-89 2 V (Slope) Fixed decimal (unsigned) calibration data, internal module supply voltage. Bit 7 of byte 88 is MSB, bit 0 of byte 89 is LSB. V(Slope)should be set to 1 for “internally calibrated” devices.
    90-91 2 V (Offset) Fixed decimal
    98 All Vcc MSB Internally measured supply voltage in transceiver.
    99 All Vcc LSB

    2) Internally measured supply voltage. Module internal supply voltage, V, is given in microvolts
    by the following equation: V(uV) = VSLOPE * VAD (16 bit unsigned integer) + VOFFSET. The result is in
    units of 100uV,
    ***********************************************************************************/
    V_S =g_BBU_Fiber_INfo[88] +g_BBU_Fiber_INfo[89]*0.004;
    V_O =(short*)(&g_BBU_Fiber_INfo[90]);

    V_AD = ( short*)(&g_BBU_Fiber_INfo[98]);

    Vol = (V_S*(*V_AD) +(*V_O))*100;
    if(Vol < 0)
    {
       Vol = Vol*(-1);
    }
    if(g_print_bbu_fiber_info)
    {
    	printf("BBU Fiber optic module Voltuage:%8.2f(uV)\n",Vol);
    }

    /*获取光模块的Vendor name*/
    bsp_get_sfp_reg_a0(u8sfp_cs,20,g_BBU_Fiber_vendor_name,16);

    if(g_print_bbu_fiber_info)
    {
    	printf("BBU Fiber optic module vendor name:%s\n",g_BBU_Fiber_vendor_name);
    }

    fiber_data->rx_power= Rx_pwer;
    fiber_data->current = Current;
    fiber_data->tx_power= Tx_pwer;
    fiber_data->temper = Temper;
    fiber_data->vol = Vol;  
    memcpy(fiber_data->vendor_name,g_BBU_Fiber_vendor_name,16);

    return BSP_OK;
}

s32 bsp_get_fiber_Info_mutex(unsigned char flag,u8 u8sfp_cs,fiber_info *fiber_data)
{
	s32 ret = 0;
	ret = bsp_get_fiber_Info(flag,u8sfp_cs,fiber_data);
	return ret;
}
/********************************************************************************
* 函数名称: ShowBBUFiberInfo 							
* 功    能:                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
int ShowBBUFiberInfo(unsigned char flag,u8 cs)
{
  	u32 i=0;
	fiber_info fiber_data={0};

	if(bsp_get_fiber_Info_mutex(flag,cs,&fiber_data) == BSP_OK)
	{
	  	printf("get NO.%d fiber info ok!\n",cs);	
	}
	else
	{		
		printf("get NO.%d fiber info faild!\n",cs);
		return BSP_ERROR;
	}
	return BSP_OK;
}
#endif

#if 1
/******************	光模块i2c for test***********************/
void sfp_cs(void)
{
	bsp_fpga_write_addr(0x20,0);
	bsp_usdelay(1000);
	
	bsp_fpga_write_addr(0x20,1);
	bsp_usdelay(1000);

	bsp_fpga_write_addr(0x20,2);
	bsp_usdelay(1000);

	bsp_fpga_write_addr(0x20,3);
	bsp_usdelay(1000);
}

void test_sfp(u8 cs,u8 device_id,u8 addr)
{
    u8 sfp[10] = {0};
	u8 data = 0x55;
    int i;
    bsp_gpio_i2c_init();

	//bsp_write_sfp();
	bsp_write_sfp(cs,device_id,addr,&data,1);
	bsp_msdelay(100);
    bsp_read_sfp(cs,device_id,addr,sfp,1);
	for(i = 0;i < 10;i++)
	{
		printf("sfp = 0x%x\n",sfp[i]);
	}
}

void test_sfp_w(u8 cs,u8 device_id,u8 addr)
{
    u8 sfp[10];
	u8 data[10]={1,2,3,4,5,6,7,8,9,10};
    int i;
    bsp_gpio_i2c_init();
	//bsp_write_sfp();
	bsp_write_sfp(cs,device_id,addr,data,10);
	bsp_msdelay(100);
    //bsp_read_sfp(cs,device_id,addr,sfp,10);
	for(i=0;i<10;i++)
	{
		printf("sfp = 0x%x\n",sfp[i]);
	}
}

void test_sfp1(u8 cs,u8 device_id,u8 addr)
{
    u8 sfp[10];
    int i;
	
    bsp_gpio_i2c_init();
	//bsp_write_sfp();
    bsp_read_sfp(cs,device_id,addr,sfp,10);
	for(i=0;i<10;i++)
	{
		printf("sfp = 0x%x\n",sfp[i]);
	}
}

void test_sfp_all(u8 cs,u8 device_id)
{
   // u8 sfp[256];
    int i;
	
    //gpio_i2c_init();
    for(i=0;i<200;i+=2)
    {
		test_sfp1(cs,device_id,i);
		bsp_msdelay(100);
    }
}

void gpio_dr(u8 offset)
{
   bsp_gpio_i2c_init();

   *g_gpiocr |= 0x1<<offset;
   
}

void gpio_io1(u8 offset,u8 data)
{
  if(data)
  {
   		*g_gpiodr |= 0x1<<offset; 
  }
  else
  { 
		*g_gpiodr &= ~((u32)(0x1<<offset));
  }
}

void clk_test0(void)
{
   int i; 
   bsp_gpio_i2c_init();

   for(i= 0;i<5;i++)
   {
   		gpio_io1(10,0);
		bsp_msdelay(10);
		//gpio_i2c_wait();
		gpio_io1(10,1);
		bsp_msdelay(10);
		
   }
}


void clk_test_ms(int time)
{
   int i;
   
   bsp_gpio_i2c_init();

   //for(i= 0;i<10;i++)
   //{
   		//gpio_set_SCL(0);
		//bsp_msdelay(time);
		//gpio_i2c_wait();
		//gpio_set_SCL(1);
		//bsp_msdelay(time);
		//gpio_i2c_wait();
   		//gpio_set_SCL(0);
		//gpio_set_SCL(1);
		*g_gpiodr &= ~GPIO_SET_OUTPUT21;
        bsp_msdelay(time);
   		*g_gpiodr |= GPIO_SET_OUTPUT21;
		bsp_msdelay(time);
		*g_gpiodr &= ~GPIO_SET_OUTPUT21;
		bsp_msdelay(time);
		*g_gpiodr |= GPIO_SET_OUTPUT21;

   //}
}

void clk_test_us(int time)
{
   int i;
   
   bsp_gpio_i2c_init();

   for(i= 0;i<10;i++)
   {
   		gpio_set_SCL(0);
		bsp_usdelay(time);
		//gpio_i2c_wait();
		gpio_set_SCL(1);
		bsp_usdelay(time);
		//gpio_i2c_wait();
   }
}


void sda_test(int time)
{
   int i;
   
   bsp_gpio_i2c_init();
   for(i= 0;i<5;i++)
   {
   		gpio_SDA_out(0);
		bsp_usdelay(time);
		//gpio_i2c_wait();
		gpio_SDA_out(1);
		bsp_usdelay(time);
		//gpio_i2c_wait();
   }
}
#endif


#if 1
/******************************* 电源模拟I2C模块 *************************************/
#if 1
/********************************************************************************
* 函数名称: gpio_i2c_init 							
* 功    能: 初始化cpld I2C接口,40--clk,41--data                               
* 相关文档:   
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
void bsp_cpld_i2c_init(void)
{
	bsp_cpld_write_reg(40,1);
	bsp_cpld_write_reg(41,1);
	bsp_cpld_write_reg(46,1);
   /* g_gpiocr = (volatile u32 *)((u32)g_u8ccsbar+0x130000);
	g_gpiodr = (volatile u32 *)((u32)g_u8ccsbar+0x130008);
	printf("g_gpiocr = 0x%x\n",g_gpiocr);
	printf("bsp_gpio_i2c_init!\n");
	*g_gpiocr |= GPIO_SET_OUTPUT21|GPIO_SET_OUTPUT22;  //初始化I/O口21,22输出,23输入
	*/
}

/********************************************************************************
* 函数名称: i2c_wait 							
* 功    能: 设置总线频率                                  
* 相关文档:                 
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void cpld_i2c_wait(void)
{
	int i;
	bsp_usdelay(5);
}
/********************************************************************************
* 函数名称: set_SDA 							
* 功    能: sda 高低                              
* 相关文档: 0-低
            1-高  
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void cpld_SDA_out(u8 sda_num)
{
 	if(sda_num)
	{
    	 bsp_cpld_write_reg(41,1);
	}
	else
	{
	 	bsp_cpld_write_reg(41,0);
	}
}
/********************************************************************************
* 函数名称: get_SDA 							
* 功    能: 读I/O口                                
* 相关文档: 0-低
            1-高
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
u8 cpld_SDA_in(void)
{
   if(bsp_cpld_read_reg(46))
   {
        //printf("\n0x%x\n",*g_gpiodr);
		return 1;
   }
   else
   {
		return 0;
   }
	//return (*g_gpiodr& GPIO_SET_OUTPUT23)?1:0;
}

/********************************************************************************
* 函数名称: set_SCL 							
* 功    能: scl 高低                               
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void cpld_set_SCL(u8 u8num)
{
 	if(u8num)
	{
    	 bsp_cpld_write_reg(40,1);
	}
	else
	{
	 	 bsp_cpld_write_reg(40,0);
	}    
}

/********************************************************************************
* 函数名称: gpio_i2c_start 							
* 功    能:                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void cpld_i2c_start(void)
{
	/* set SDA to 1 */
    cpld_SDA_out(1);
	cpld_i2c_wait();

	/* set SCL to 1 */
    cpld_set_SCL(1);
    cpld_i2c_wait();

	/* set SDA to 0 */	
    cpld_SDA_out(0);
    cpld_i2c_wait();
	
	/* set SCL to 0 */	
    cpld_set_SCL(0);
}
/********************************************************************************
* 函数名称: gpio_i2c_stop 							
* 功    能:                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void cpld_i2c_stop(void)
{
    /* set SDA to 0 */	
    cpld_SDA_out(0);
    cpld_i2c_wait();

	/* set SCL to 1 */
    cpld_set_SCL(1);
    cpld_i2c_wait();

    /* set SDA to 1 */
    cpld_SDA_out(1);
}
/********************************************************************************
* 函数名称: read_ack 							
* 功    能:                               
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private void cpld_send_ack(u8 u8ack) 
{   
    cpld_SDA_out(u8ack);
	cpld_i2c_wait();

	/* toggle clock */
    cpld_set_SCL(1);
    cpld_i2c_wait();
    cpld_set_SCL(0);
	cpld_i2c_wait();

	/* set SDA to 1 */
	cpld_SDA_out(1);
	cpld_i2c_wait();
}
/********************************************************************************
* 函数名称: read_ack 							
* 功    能:                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private u8 cpld_read_ack(void) 
{  
	u8 u8ack;  

	cpld_set_SCL(1); 
	cpld_i2c_wait();

	if (cpld_SDA_in()) 
	{
		cpld_i2c_wait();
		if (cpld_SDA_in())
		{
			u8ack =1;
		}
	}
	else
	{
		u8ack = 0;
	}

	//printf("ack is:%x\n",u8ack);
	cpld_set_SCL(0); 
	cpld_i2c_wait();	

	return u8ack;
}

/********************************************************************************
* 函数名称: I2C_Send_Byte 							
* 功    能: 向总线上发送一个字节                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private u8 cpld_i2c_send_byte(u8 u8send_bytedata) 
{
    u8 i;

    for (i = 0; i < 8; i++)  
    {
		if (u8send_bytedata & 0x80) 
		{
		    //printf("1");
			cpld_SDA_out(1);
		}
		else
		{
		   //printf("0");
			cpld_SDA_out(0);
		}
		u8send_bytedata <<= 1;
		
	   /* toggle clock */
		cpld_i2c_wait();
		cpld_set_SCL(1);
		cpld_i2c_wait();
		cpld_set_SCL(0);
    }
	cpld_SDA_out(1);
	cpld_i2c_wait();		
	return cpld_read_ack();
}

/********************************************************************************
* 函数名称: I2C_Send_Byte 							
* 功    能: 接收一个字节                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
Private s32 cpld_i2c_receive_byte(u8 *u8preceive_data)  
{
    u8 i;
    u8 u8bytedata = 0;
    u8 u8temp;

	/* set SDA input */
    for (i = 0; i < 8; i++)
    {
        /* set SCL high */
        cpld_i2c_wait();
        cpld_set_SCL(1);
        cpld_i2c_wait();

        u8bytedata = u8bytedata<<1;
        u8temp = cpld_SDA_in(); 
        if (u8temp)
          u8bytedata |= 0x01; 

		/* set SCL low */		
		cpld_i2c_wait();
        cpld_set_SCL(0);
        cpld_i2c_wait();
    }

    /* set SDA output */	
    *u8preceive_data = u8bytedata;
	return BSP_OK;
}


/********************************************************************************
* 函数名称: bsp_gpio_i2c_Read 							
* 功    能: 读设备                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 此函数只限于I2C设备从地址小于一个字节
*********************************************************************************/
s32 bsp_cpld_i2c_read(u8 u8device_ID,u8 u8address,u8 *u8pdata,u32 u8byte_num)
{   
    u8 u8ack;
	u32 i;
	
   /* cpld_i2c_start();
	printf("read sfp error!\n");
    u8ack = cpld_i2c_send_byte(u8device_ID);
	printf("read sfp error2!\n");
	if (u8ack)
    {
        cpld_i2c_stop();
        return BSP_ERROR;
    }
    u8ack = cpld_i2c_send_byte(u8address);
	printf("read sfp error44!\n");
	if (u8ack)
    {
        cpld_i2c_stop();
        return BSP_ERROR;
    }
	*/
    cpld_i2c_start();
	//printf("a!\n");
    u8ack = cpld_i2c_send_byte(u8device_ID+1);  //读命令
    //printf("b!\n");
	if (u8ack)
    {
        cpld_i2c_stop();
        return BSP_ERROR;
    }
    //printf("c!\n");
	for(i = 0;i < u8byte_num;i++)
	{
		//printf("5!\n");
    	cpld_i2c_receive_byte(&u8pdata[i]);
        if(i == u8byte_num - 1) 
		    cpld_send_ack(1);  
		else      
		    cpld_send_ack(0);
	}
    cpld_i2c_stop();    
	return BSP_OK;
}


/********************************************************************************
* 函数名称: bsp_gpio_i2c_Read 							
* 功    能: 写设备                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 此函数只限于I2C设备从地址小于一个字节
*********************************************************************************/
s32 bsp_cpld_i2c_write(u8 u8device_ID,u8 u8address,u8 *u8pdata,u32 u8byte_num)
{   
    u8 u8ack;
	u32 i;
    //printf("device_ID is:0x%x\n",u8device_ID);
    //printf("address is:0x%x\n",u8address);
    cpld_i2c_start();
	//返回一个应答值
    u8ack = cpld_i2c_send_byte(u8device_ID); //设备地址
    if (u8ack)
    {
        cpld_i2c_stop();
        return BSP_ERROR;
    }

    u8ack = cpld_i2c_send_byte(u8address); //片内地址
    if (u8ack)
    {
       cpld_i2c_stop();
        return BSP_ERROR;
    }
	
	for(i=0;i<u8byte_num;i++)
	{    
	    //printf("0x%x\n",u8pdata[i]);
	    u8ack = cpld_i2c_send_byte(u8pdata[i]); //发送一个字节
	    if(u8ack)
	    {
			cpld_i2c_stop();
			return BSP_ERROR;
	    }
	}
	
    cpld_i2c_stop();
    return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_read_power_board 							
* 功    能:                               
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
            u8sfp_cs: sfp片选，1,2,3,4
			u8sfp_device_ID：设备从地址, 0xA0,0xA1
			u8address：设备内部子地址
			pu8read_data：接收数据指针
			u8byte_num：数据长度
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_read_power_board(u8 u8power_device_ID,u8 u8address,u8 *pu8read_data,u32 u8byte_num)
{
	if(BSP_ERROR == bsp_cpld_i2c_read(u8power_device_ID,u8address,pu8read_data,u8byte_num))
	{
		printf("read sfp error!\n");
		return BSP_ERROR;
	}
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_write_power_board 							
* 功    能: 写设备                                
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述
			u8sfp_cs: sfp片选，1,2,3,4
			u8sfp_device_ID：设备从地址, 0xA0,0xA2
			u8address：设备内部子地址
			pu8read_data：发送数据指针
			u8byte_num：数据长度
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_write_power_board(u8 u8power_device_ID,u8 u8address,u8 *pu8write_data,u32 u8byte_num )
{
    u8 i;

	for(i = 0;i < u8byte_num;i++)
    {	
		if(BSP_ERROR == bsp_cpld_i2c_write(u8power_device_ID,u8address+i,pu8write_data+i,1))
		{
			printf("bsp_write_power_board error!\n");
			return BSP_ERROR;
		}
		bsp_usdelay(10);
	}	
	return BSP_OK;
}

#endif

void test_power_i2c(u16 power_data)
{
    u8 power[3] = {0};
	u8 device_id = 0xb4;
	u8 addr = 0;
	power[0] = (u8)(power_data&0xff);
	power[1] = (u8)((power_data>>8)&0xff);
	power[2] = 1;
    bsp_cpld_i2c_init();	
	bsp_write_power_board(device_id,addr,&power,3);
}

void bsp_power_off(u16 power_off_time)
{
    u8 power[3] = {0};
	u8 device_id = 0xb4;
	u8 addr = 0;
	power[0] = (u8)(power_off_time&0xff);
	power[1] = (u8)((power_off_time>>8)&0xff);
	power[2] = 1;
    bsp_cpld_i2c_init();	
	bsp_write_power_board(device_id,addr,&power,3);
}
void test_power_i2c_read(u8 count)
{
       u8 power[256] = {0};
	u8 device_id = 0xb4;
	u8 addr = 0;
	int i=0;
       bsp_cpld_i2c_init();	
	bsp_read_power_board(device_id,addr,power,count);
	for(i=0;i<count;i++)
	{
		printf("addr: %d, data: 0x%x\n",i,power[i]);
	}
}

s32 bsp_get_psm_sn(char * sn)
{
       u8 power[256] = {0};
	u8 device_id = 0xb4;
	u8 addr = 0;
	int i=0;
	s32 ret;
       bsp_cpld_i2c_init();	
	ret = bsp_read_power_board(device_id,addr,power,80);
	memcpy(sn,power+48,32);
       return ret;
}

void bsp_get_powerinfo(void)
{
       u8 power[256] = {0};
	char SwVer[9] = {0};
	char HwVer[33] = {0};
	char SnVer[33] = {0};
	u8 device_id = 0xb4;
	u8 addr = 0;
	int i=0;
       bsp_cpld_i2c_init();	
	bsp_read_power_board(device_id,addr,power,80);
	memcpy(SwVer,power+8,8);
	memcpy(HwVer,power+16,32);
	memcpy(SnVer,power+48,32);
	printf("Sw version: %s\n",SwVer);
	printf("Hw version: %s\n",HwVer);
	printf("Sn: %s\n",SnVer);
       return;
}

void power_write(u8 addr, u8 data, u8 count)
{
    u8 test[3] = {0};
	u8 device_id = 0xb4;
	u8 i;

       bsp_cpld_i2c_init();	
	for(i = 0; i< count; i++)
	{
	    test[0] = addr+i;
		
	    test[1] = data + i;
		
	    if(BSP_ERROR == bsp_cpld_i2c_write(device_id, test[0],(u8 *)&(test[1]),1))
	    {
			printf("bsp_cpld_i2c_write error!\n");
			return BSP_ERROR;
	     }
	     bsp_usdelay(100);
	}
}
void bsp_power_write_sw(char *string)
{
       char stringtemp[32] = {0};
	u8 device_id = 0xb4;
	u8 count,i;
       u8 addr = 8;
       count = strlen((const CHAR *)string);
       if(count > 32)
       {
            printf("\n len is too long!!\n");
	     return;
	 }
	strcpy(stringtemp,(const char*)string);
       stringtemp[count]='\0';
       bsp_cpld_i2c_init();	
	for(i = 0; i< count+1; i++)
	{
	    if(BSP_ERROR == bsp_cpld_i2c_write(device_id, addr,(u8 *)(stringtemp+i),1))
	    {
			printf("bsp_cpld_i2c_write error!\n");
			return BSP_ERROR;
	     }
	     addr++;
	     bsp_usdelay(50);
	}
}


void bsp_power_write_hw(char *string)
{
       char stringtemp[32] = {0};
	u8 device_id = 0xb4;
	u8 count,i;
       u8 addr = 16;
	u8 data;
       count = strlen((const CHAR *)string);
       if(count > 32)
       {
            printf("\n len is too long!!\n");
	     return;
	 }
	strcpy(stringtemp,(const char*)string);
       stringtemp[count]='\0';
       bsp_cpld_i2c_init();	
	for(i = 0; i< count+1; i++)
	{
	    if(BSP_ERROR == bsp_cpld_i2c_write(device_id, addr,(u8 *)(stringtemp+i),1))
	    {
			printf("bsp_cpld_i2c_write error!\n");
			return BSP_ERROR;
	     }
	     addr++;
	     bsp_usdelay(50);
	}
       data = 0x5a;	 
       if(BSP_ERROR == bsp_cpld_i2c_write(device_id, 127,(u8 *)(&data),1))
       {
			printf("bsp_cpld_i2c_write error!\n");
			return BSP_ERROR;
       }
        bsp_usdelay(1000);
	 return;
}

void bsp_power_write_sn(char *string)
{
       char stringtemp[32] = {0};
	u8 device_id = 0xb4;
	u8 count,i;
       u8 addr = 48;
   	u8 data;
       count = strlen((const CHAR *)string);
       if(count > 32)
       {
            printf("\n len is too long!!\n");
	     return;
	 }
	strcpy(stringtemp,(const char*)string);
       stringtemp[count]='\0';
       bsp_cpld_i2c_init();	
	for(i = 0; i< count+1; i++)
	{
	    if(BSP_ERROR == bsp_cpld_i2c_write(device_id, addr,(u8 *)(stringtemp+i),1))
	    {
			printf("bsp_cpld_i2c_write error!\n");
			return BSP_ERROR;
	     }
	     addr++;
	     bsp_usdelay(50);
	}
       data = 0x5a;	 
       if(BSP_ERROR == bsp_cpld_i2c_write(device_id, 127,(u8 *)(&data),1))
       {
			printf("bsp_cpld_i2c_write error!\n");
			return BSP_ERROR;
        }
        bsp_usdelay(1000);
	 return;
}

#define INTERNALLY_CALIBRATED	0X20
#define EXTERNALLY_CALIBRATED	0X10

static inline short power(short x, int y)
{
	int i;
	for (i = 0; i < y; i++)	
		x *= x;
	return x;
}

static unsigned char calibrated_flag;

int bsp_get_fiber_tx_power(int sfp_id, float *tx_power)
{
	int ret, i;
	unsigned char reg_tmp[128];
	float tx_pwr_slope, tx_pwr_offset;
	short tx_pwr_ad;

	if (tx_power == NULL)
		return -1;

	if (calibrated_flag == 0) {
		/* read the type of calibrate */
		ret = bsp_read_sfp(sfp_id, 0xa0, 92, &calibrated_flag, 1);
		if (ret) {
			printf("%s %d read sfp failed\n", __func__, __LINE__);
			return -1;
		}
	}

	ret = bsp_get_sfp_reg(sfp_id, 0, reg_tmp, sizeof(reg_tmp));
	if (ret) {
		printf("%s %d read sfp failed\n", __func__, __LINE__);
		return -1;
	}

	/* according to 90th byte of 0xa0 address, bit[5] and bit[4] */
	if (calibrated_flag & EXTERNALLY_CALIBRATED) {
		/* calculate tx power */
		tx_pwr_slope = (float)reg_tmp[80] + (float)reg_tmp[81] / 256;
		tx_pwr_offset = (float)(*(short *)&reg_tmp[82]);
		tx_pwr_ad = (float)(*(short *)&reg_tmp[102]);

		*tx_power = (tx_pwr_slope * tx_pwr_ad + tx_pwr_offset) * 0.1;

		return 0;

	} else	if (calibrated_flag & INTERNALLY_CALIBRATED) {
		tx_pwr_ad = (float)(*(short *)&reg_tmp[102]);

		*tx_power = 0.1 * tx_pwr_ad;

		return 0;
	}

	return -1;
}

int bsp_get_fiber_rx_power(int sfp_id, float *rx_power)
{
	int ret, i;
	unsigned char reg_tmp[128];
	float rx_power_tmp[5];
	short rx_power_ad;

	if (rx_power == NULL)
		return -1;

	if (calibrated_flag == 0) {
		/* read the type of calibrate */
		ret = bsp_read_sfp(sfp_id, 0xa0, 92, &calibrated_flag, 1);
		if (ret) {
			printf("%s %d read sfp failed\n", __func__, __LINE__);
			return -1;
		}
	}

	ret = bsp_get_sfp_reg(sfp_id, 0, reg_tmp, sizeof(reg_tmp));
	if (ret) {
		printf("%s %d read sfp failed\n", __func__, __LINE__);
		return -1;
	}

	/* according to 90th byte of 0xa0 address, bit[5] and bit[4] */
	if (calibrated_flag & EXTERNALLY_CALIBRATED) {
		for (i = 0; i < 5; i++)
			rx_power_tmp[i] = *(float *)&reg_tmp[72 - 4 * i];

		rx_power_ad = *(short *)&reg_tmp[104];

		*rx_power = 0;

		/* calculate rx power */
		for (i = 0; i < 5; i++) {
			if (i == 0)
				*rx_power += rx_power_tmp[i];
			else if (i == 1)
				*rx_power += rx_power_tmp[i] * rx_power_ad;
			else
				*rx_power += rx_power_tmp[i] * (float)(power(rx_power_ad, i));
		}

		*rx_power *= 0.1;

		return 0;

	} else	if (calibrated_flag & INTERNALLY_CALIBRATED) {
		rx_power_ad = *(short *)&reg_tmp[104];

		*rx_power = 0.1 * (float)rx_power_ad;	

		return 0;
	}

	return -1;
}

int bsp_get_sfp_temperature(int sfp_id, float *temperature)
{
	int ret, i;
	unsigned char reg_tmp[128];
	float t_slope, t_offset, t_ad;


	if (temperature == NULL)
		return -1;

	if (calibrated_flag == 0) {
		/* read the type of calibrate */
		ret = bsp_read_sfp(sfp_id, 0xa0, 92, &calibrated_flag, 1);
		if (ret) {
			printf("%s %d read sfp failed\n", __func__, __LINE__);
			return -1;
		}
	}

	ret = bsp_get_sfp_reg(sfp_id, 0, reg_tmp, sizeof(reg_tmp));
	if (ret) {
		printf("%s %d read sfp failed\n", __func__, __LINE__);
		return -1;
	}

	/* according to 90th byte of 0xa0 address, bit[5] and bit[4] */
	if (calibrated_flag & EXTERNALLY_CALIBRATED) {
		t_slope = (float)((u16)reg_tmp[84] << 8 | (u16)reg_tmp[85]);
		t_offset = (float)((u16)reg_tmp[86] << 8 | (u16)reg_tmp[87]);
		t_ad = (float)((u16)reg_tmp[96] << 8 | (u16)reg_tmp[97]);

		*temperature = (t_slope * t_ad + t_offset) / 256;
	} else {
		t_ad = (float)reg_tmp[96]  +  (float)reg_tmp[97] / 256;

		*temperature = t_ad;
	}

	return 0;
}

int bsp_get_sfp_current(int sfp_id, float *current)
{
	int ret, i;
	unsigned char reg_tmp[128];
	float tx_i_slope, tx_i_offset, tx_i_ad;


	if (current == NULL)
		return -1;

	if (calibrated_flag == 0) {
		/* read the type of calibrate */
		ret = bsp_read_sfp(sfp_id, 0xa0, 92, &calibrated_flag, 1);
		if (ret) {
			printf("%s %d read sfp failed\n", __func__, __LINE__);
			return -1;
		}
	}

	ret = bsp_get_sfp_reg(sfp_id, 0, reg_tmp, sizeof(reg_tmp));
	if (ret) {
		printf("%s %d read sfp failed\n", __func__, __LINE__);
		return -1;
	}

	/* according to 90th byte of 0xa0 address, bit[5] and bit[4] */
	if (calibrated_flag & EXTERNALLY_CALIBRATED) {
		tx_i_slope = (float)((u16)reg_tmp[76] << 8 | (u16)reg_tmp[77]);
		tx_i_offset = (float)((u16)reg_tmp[78] << 8 | (u16)reg_tmp[79]);
		tx_i_ad = (float)((u16)reg_tmp[100] << 8 | (u16)reg_tmp[101]);

		*current = (tx_i_slope * tx_i_ad + tx_i_offset) * 2;
	} else {
		tx_i_ad = (float)((u16)reg_tmp[100] << 8 | (u16)reg_tmp[101]) ;
		*current = tx_i_ad * 2 / 1000;
	}

	return 0;
}

int bsp_get_sfp_voltuage(int sfp_id, float *volatuage)
{
	int ret, i;
	unsigned char reg_tmp[128];
	float v_slope, v_offset, v_ad;


	if (volatuage == NULL)
		return -1;

	if (calibrated_flag == 0) {
		/* read the type of calibrate */
		ret = bsp_read_sfp(sfp_id, 0xa0, 92, &calibrated_flag, 1);
		if (ret) {
			printf("%s %d read sfp failed\n", __func__, __LINE__);
			return -1;
		}
	}

	ret = bsp_get_sfp_reg(sfp_id, 0, reg_tmp, sizeof(reg_tmp));
	if (ret) {
		printf("%s %d read sfp failed\n", __func__, __LINE__);
		return -1;
	}

	/* according to 90th byte of 0xa0 address, bit[5] and bit[4] */
	if (calibrated_flag & EXTERNALLY_CALIBRATED) {
		v_slope = (float)((u16)reg_tmp[88] << 8 | (u16)reg_tmp[89]);
		v_offset = (float)((u16)reg_tmp[90] << 8 | (u16)reg_tmp[91]);
		v_ad = (float)((u16)reg_tmp[98] << 8 | (u16)reg_tmp[99]);

		*volatuage = (v_slope * v_ad + v_offset) * 100;
	} else {
		v_ad = (float)((u16)reg_tmp[98] << 8 | (u16)reg_tmp[99]);
		*volatuage = v_ad * 100 / 1000000;
	}

	return 0;
}

int bsp_get_sfp_los_state(int sfp_id)
{
	u8 val;

	if (sfp_id < 0 || sfp_id >= 3)
		return -1;

	if (bsp_get_sfp_reg(sfp_id, 110, &val, 1))
		return -1;

	return (val & 0x2) == 0x2;
}

int bsp_sfp_online(int sfp_id)
{
	u16 val;

	if (sfp_id < 0 || sfp_id >= 3)
		return -1;

	val = bsp_fpga_read_reg(10) >> sfp_id;

	return !(val & 1);
}

int bsp_get_sfp_speed(int sfp_id)
{
	unsigned char reg_tmp[128];
	int ret;

	if (sfp_id < 0 || sfp_id >= 3)
		return -1;

	ret = bsp_read_sfp(sfp_id, 0xa0, 12, reg_tmp, 1);
	if (ret) {
		printf("%s %d read sfp failed\n", __func__, __LINE__);
		return -1;
	}

	return (int)reg_tmp[0] * 100;
}

int bsp_get_sfp_vendor(int sfp_id, char *vendor)
{
	int ret;

	if (sfp_id < 0 || sfp_id >= 3 || vendor == NULL) {
		printf("%s argv error\n", __func__);
		return -1;
	}

	memset(vendor, 0, 16);

	ret = bsp_read_sfp(sfp_id, 0xa0, 20, vendor, 16);

	if (ret) {
		printf("%s %d read sfp failed\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

void bsp_get_sfp_test(int sfp_id)
{
	float current, volatuage, temperature, tx_power, rx_power;
	int ret;

	ret = bsp_get_fiber_tx_power(sfp_id, &tx_power);
	if (ret)
		printf("get fiber power failed\n");
	else 
		printf("tx power %6.1fuw\n", tx_power);

	ret = bsp_get_fiber_rx_power(sfp_id, &rx_power);
	if (ret)
		printf("get fiber power failed\n");
	else 
		printf("rx power %6.1fuw\n", rx_power);

	ret = bsp_get_sfp_current(sfp_id, &current);
	if (ret)
		printf("get current failed\n");
	else
		printf("current %6.1f mA\n", current);

	ret = bsp_get_sfp_voltuage(sfp_id, &volatuage);
	if (ret)
		printf("get volatuage failed\n");
	else
		printf("volatuage %6.1f V\n", volatuage);

	ret = bsp_get_sfp_temperature(sfp_id, &temperature);
	if (ret)
		printf("get temperature failed\n");
	else
		printf("temperature %6.1f ℃ \n", temperature);

	ret = bsp_get_sfp_los_state(sfp_id);
	if (ret < 0)
		printf("get LOS failed\n");
	else
		printf("%s\n", ret == 1? "RX LOS" : "");
}


/******************************* 源文件结束 *************************************/
#endif
/******************************* 源文件结束 *************************************/
