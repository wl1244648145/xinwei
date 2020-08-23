/********************************************************************************
* 源文件名:             bsp_i2c.c
* 功能:                 I2C 模块  
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
#include "../../../com_inc/bsp_i2c_ext.h"
#include "../inc/bsp_i2c.h"
#include "../../../modules/hmi/inc/hmi.h"

 
#define IIC_PORT     (4)
#define CONFIG_SYS_I2C_SPEED		100000

/* 设备从地址*/
#define EEPROM_I2C_ADDR 0x50
#define RTC_I2C_ADDR    0X51
#define TEMP1_I2C_ADDR  0X18
#define TEMP2_I2C_ADDR  0X4e
#define POWERSENSE_I2C_ADDR  0X40 
#define FAN_I2C_ADDR    0X2F

int g_i2c_print = 0;
int g_i2c_wait_ack_timeout = 0;
int g_i2c_write_addr_timeout = 0;
/*********************** 全局变量定义/初始化 **************************/

/************************** 局部常数和类型定义 ************************/
//GPIO21	PPC_FPGA_SFP_IIC_SCL	P2041->FPGA	 SCL OUT
//GPIO22	PPC_FPGA_SFP_IIC_SDA	P2041->FPGA	 SDA OUT 
//GPIO23	FPGA_PPC_SFP_IIC_SDA	FPGA->P2041	 SDA INT
/*************************** 局部函数原型声明 **************************/
extern void bsp_sys_msdelay(ul32 dwTimeOut);// ---延迟单位为毫秒
extern void bsp_sys_usdelay(ul32 dwTimeOut); //---延迟单位为微妙
void i2c_init(int speed,int port);
void bsp_current_monitor_init(void);
s32 bsp_fan_config_init(void);
s32 bsp_fan_control(u8 fan_pwmval);
/************************************ 函数实现 ************************* ****/
pthread_mutex_t  g_mp_i2c1 = PTHREAD_MUTEX_INITIALIZER;  /* 互斥 I2C2访问*/
pthread_mutexattr_t  g_mattr_i2c1;
pthread_mutex_t  g_mp_i2c2 = PTHREAD_MUTEX_INITIALIZER;  /* 互斥 I2C2访问*/
pthread_mutexattr_t  g_mattr_i2c2;
pthread_mutex_t  g_mp_i2c3 = PTHREAD_MUTEX_INITIALIZER;  /* 互斥I2C3访问 */
pthread_mutexattr_t  g_mattr_i2c3;
pthread_mutex_t  g_mp_i2c4 = PTHREAD_MUTEX_INITIALIZER;  /* 互斥I2C4访问 */
pthread_mutexattr_t  g_mattr_i2c4;


pthread_mutex_t  g_mp_i2cwrite = PTHREAD_MUTEX_INITIALIZER;  /* 互斥I2C4访问 */
pthread_mutex_t  g_mp_i2cread = PTHREAD_MUTEX_INITIALIZER;  /* 互斥I2C4访问 */

s32 bsp_i2c_mutex_init(void)
{
	s32 s32ret = 0;
	/*initializea mutext oits default value*/
	s32ret |=pthread_mutex_init(&g_mp_i2c1, NULL);
	/*initializea mutext oits default value*/
	s32ret |=pthread_mutex_init(&g_mp_i2c2, NULL);
	/*initializea mutext oits default value*/
	s32ret |=pthread_mutex_init(&g_mp_i2c3, NULL);
	/*initializea mutex*/
	s32ret |=pthread_mutex_init(&g_mp_i2c4, NULL);
	s32ret |=pthread_mutex_init(&g_mp_i2cwrite, NULL);
	s32ret |=pthread_mutex_init(&g_mp_i2cread, NULL);
	return s32ret;
}
/********************************************************************************
* 函数名称: bsp_i2c_init							
* 功    能: 初始化i2c及外围设备                        
* 相关文档:                    
* 函数类型:							
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 在板级初始化时调用
*********************************************************************************/
extern int g_i2c_init_delay_time_print;
s32 bsp_i2c_init(void)
{
    s32 port;
    for(port = 0;port < 2;port++)
    {
    	  if (port !=2)
		        i2c_init(CONFIG_SYS_I2C_SPEED,port);
    }
	if(1 == g_i2c_init_delay_time_print)
		printf("bsp_i2c_init-- CONFIG_SYS_I2C_SPEED \n");
	bsp_i2c_mutex_init();
	if(1 == g_i2c_init_delay_time_print)
		printf("bsp_i2c_init-- i2c_mutex_init \n");
	bsp_current_monitor_init();
	if(1 == g_i2c_init_delay_time_print)
		printf("bsp_i2c_init-- current_monitor_init \n");
	//bsp_fan_config_init();
	bsp_gpio_i2c_init();
	if(1 == g_i2c_init_delay_time_print)
		printf("bsp_i2c_init-- gpio_i2c_init!\n");
	g_i2c_init_delay_time_print=0;
	return BSP_OK;
}

/********************************************************************************
* 函数名称: i2c_wait
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
int i2c_wait(int write,int port)
{
	s32 csr;
	u32 icnt=0;
	volatile void *piicsr;
	u32 u32i = 0;
	switch(port)
	{
		case P2041_IIC0:
			piicsr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0xc;
		    break;
		case P2041_IIC1:
			piicsr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0xc;
			break;
		case P2041_IIC2:
			piicsr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0xc;
			break;
		case P2041_IIC3:
			piicsr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0xc;
			break;
		default:
			break;
	}	
	
	//while(u32i < 0x10000000)
	while(u32i < 0x10000)
	{
            csr = readb(piicsr);
            if (!(csr & I2C_SR_MIF))
            {
                u32i++;
                continue;
            }				
            csr = readb(piicsr); 
            writeb(0x0, piicsr);  //清中断
            if (csr & I2C_SR_MAL) 
            {		
                printf("i2c_wait:I2C_SR_MAL\n");
                return -1;
            }		
            if (!(csr & I2C_SR_MCF))	
            {	
                printf("i2c_wait:unfinished\n");
                return -1;
            }
            #if 0
            while(1)
            {
                icnt++;
                if (write == I2C_WRITE_BIT && (csr & I2C_SR_RXAK) && icnt>=10000000)
                {    	
                    printf("i2c_wait: No RXACK\n");
                    break;
                    //return -1;
                }
                else
                {
                    break;
                }
            }
            #endif
            if(write == I2C_WRITE_BIT)
            {
                while((csr = readb(piicsr)) & I2C_SR_RXAK)
                {
                    if(icnt++>=10000)
                    {
                        if(g_i2c_print)
                            printf("i2c_wait: NO I2C_SR_RXAK\n");
                        g_i2c_wait_ack_timeout++;
                        return -1;
                    }
                }
            }
            return 0;
	}
       if(g_i2c_print)
	    printf("u32i=0x%lx\r\n",u32i);
	return -1;
}

/********************************************************************************
* 函数名称: set_i2c_bus_speed
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
static unsigned int set_i2c_bus_speed(const struct fsl_i2c *dev,unsigned int i2c_clk, unsigned int speed)
{
	unsigned short divider = min(i2c_clk / speed, (unsigned short) -1);
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(fsl_i2c_speed_map); i++)
		if (fsl_i2c_speed_map[i].divider >= divider) 
		{
			UCHAR fdr;
			//printf("set_i2c_bus_speed\n");
			fdr = fsl_i2c_speed_map[i].fdr;
			speed = i2c_clk / fsl_i2c_speed_map[i].divider;
			writeb(fdr, &dev->fdr);		/* set bus speed */
			//writeb(0x3f, &dev->fdr);
			break;
		}
		//printf("i2c_speed = %d\n",speed);
	return speed;
}

/********************************************************************************
* 函数名称: i2c_init
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
int g_i2c_init_delay_time_print =1;
void i2c_init(int speed,int port)
{
	struct fsl_i2c *dev;
	unsigned int temp;
	volatile void *piiccr;
	switch(port)
	{
        case P2041_IIC0:
	        dev = (struct fsl_i2c *) (g_u8ccsbar + CONFIG_SYS_I2C1_OFFSET);
	        break;
	    case P2041_IIC1:
		    dev = (struct fsl_i2c *) (g_u8ccsbar + CONFIG_SYS_I2C2_OFFSET);
		    break;
	    case P2041_IIC2:
		    dev = (struct fsl_i2c *) (g_u8ccsbar + CONFIG_SYS_I2C3_OFFSET);
		    break;
	    case P2041_IIC3:
		    dev = (struct fsl_i2c *) (g_u8ccsbar + CONFIG_SYS_I2C4_OFFSET);
		    break;
	    default:
		    return;
	}
	if(1 == g_i2c_init_delay_time_print)
		printf("i2c_init: i2c_reg_addr = 0x%x\n",(u32)dev);
	writeb(0, &dev->cr);	/* stop I2C controller */
	if(1 == g_i2c_init_delay_time_print)
		printf("i2c_init: before sys_usdelay 5\n");
	bsp_delay_100us();
	//bsp_sys_usdelay(5);	/* let it shutdown in peace */
	if(1 == g_i2c_init_delay_time_print)
		printf("i2c_init: after sys_usdelay 5\n");
	temp = set_i2c_bus_speed(dev,100000000,speed);
	//writeb(0x3F, &dev->dfsrr);
	//writeb(slaveadd<<1 , &dev->adr);/* not write slave address */
	writeb(0x0, &dev->sr);			/* clear status register */
	//writeb(I2C_CR_MEN, &dev->cr); /* start I2C controller */
	//writeb(I2C_CR_MEN, &dev->cr); /* start I2C controller */
}
void i2c_reset_init(int speed,int port)
{
	struct fsl_i2c *dev;
	unsigned int temp;
	volatile void *piiccr;
	switch(port)
	{
        case P2041_IIC0:
	        dev = (struct fsl_i2c *) (g_u8ccsbar + CONFIG_SYS_I2C1_OFFSET);
	        break;
	    case P2041_IIC1:
		    dev = (struct fsl_i2c *) (g_u8ccsbar + CONFIG_SYS_I2C2_OFFSET);
		    break;
	    case P2041_IIC2:
		    dev = (struct fsl_i2c *) (g_u8ccsbar + CONFIG_SYS_I2C3_OFFSET);
		    break;
	    case P2041_IIC3:
		    dev = (struct fsl_i2c *) (g_u8ccsbar + CONFIG_SYS_I2C4_OFFSET);
		    break;
	    default:
		    break;
	}
	/*printf("i2c_reg_addr = 0x%x\n",(u32)dev);*/
	writeb(0, &dev->cr);	/* stop I2C controller */
	bsp_sys_usdelay(5);	/* let it shutdown in peace */	
	temp = set_i2c_bus_speed(dev,100000000,speed);
	//writeb(0x3F, &dev->dfsrr);
	//writeb(slaveadd<<1 , &dev->adr);/* not write slave address */
	writeb(0x0, &dev->sr);			/* clear status register */
	writeb(I2C_CR_MEN, &dev->cr); /* start I2C controller */
}

void reset_i2c(int port)
{
	i2c_reset_init(CONFIG_SYS_I2C_SPEED,port);
	bsp_sys_msdelay(100);
}
 
/********************************************************************************
* 函数名称: i2c_write_addr
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
int g_i2c_debug = 0;
static int i2c_write_addr (UCHAR dev, UCHAR dir, int rsta,int port)
{
    volatile void *piiccr;
    volatile void *piicdr;
    switch(port)
    {
		case P2041_IIC0:
        	     	piiccr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0x8;
        	    	piicdr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0x10;
        	     	break;
		case P2041_IIC1:
			piiccr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0x8;
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0x10;
			break;
		case P2041_IIC2:
			piiccr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0x8;
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0x10;
			break;
		case P2041_IIC3:
			piiccr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0x8;
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0x10;
			break;
		default:
			break;
	}
	//使能，主模式，发送，restart?
	writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX| (rsta ? I2C_CR_RSTA : 0),piiccr);
	//地址，读写方式
	//printf("devaddr = %x",dev);
	writeb((dev << 1) | dir, piicdr);
       if(g_i2c_debug == 1)
       {
            if(dev == 0x00)
            {
                return;
                //system("reboot");
            }
       } 
	if (i2c_wait(I2C_WRITE_BIT,port) < 0)
	{
              if(g_i2c_print)
                printf("i2c write addr wait timed out!\n");
              g_i2c_write_addr_timeout++;
		return 0;
	}	
	return 1;
}

/********************************************************************************
* 函数名称: __i2c_write
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
static int __i2c_write(UCHAR *data, int length,int port)
{
	int i=0;
	volatile void *piicdr;
	switch(port)
	{
		case P2041_IIC0:
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0x10;
			break;
		case P2041_IIC1:
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0x10;
			break;
		case P2041_IIC2:
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0x10;
			break;
		case P2041_IIC3:		
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0x10;
			break;
		default:
			break;
	}
	
	for (i = 0; i < length; i++) 
	{
		writeb(data[i], piicdr);
		if ((2==port) || (3==port))
		{
		   // printf("wrdata = 0x%x\n",data[i]);
		}
		if (i2c_wait(I2C_WRITE_BIT,port) < 0)
		{
			break;
		}
	}
	return i;
}

/********************************************************************************
* 函数名称: __i2c_read
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
static __inline__ int __i2c_read(UCHAR *data, int length,int port)
{
	int i;
	volatile void *piiccr;
    volatile void *piicdr;
	switch(port)
	{
		case P2041_IIC0:
			piiccr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0x8;
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0x10;
			break;
		case P2041_IIC1:
			piiccr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0x8;
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0x10;
			break;
		case P2041_IIC2:
			piiccr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0x8;
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0x10;
			break;
		case P2041_IIC3:
			piiccr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0x8;
			piicdr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0x10;
			break;
		default:
			break;
	}	
	writeb(I2C_CR_MEN | I2C_CR_MSTA | ((length == 1) ? I2C_CR_TXAK : 0),piiccr);
	
	readb(piicdr);
	for (i = 0; i < length; i++) 
	{
		if (i2c_wait(I2C_READ_BIT,port) < 0)
		{
		    printf("rd time out!\n");
			break;
		}
		
		if (i == length - 2)
			writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_TXAK,piiccr);
		if (i == length - 1)
			writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX,piiccr);
		data[i] = readb(piicdr);
		//printf("rddata = 0x%x\n",data[i]);
	}
	return i;
}

/********************************************************************************
* 函数名称: i2c_read
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 i2c_read(unsigned char dev, unsigned int addr, int alen, unsigned char *data, int length,int port)
{
    int i = -1; /* signal error */
    volatile void *piiccr;
	pthread_mutex_lock(&g_mp_i2cread);
    switch(port)
    {
        case P2041_IIC0:
	     piiccr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0x8;
	     break;
	 case P2041_IIC1:
	     piiccr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0x8;
	     break;
	 case P2041_IIC2:
	     piiccr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0x8;
	     break;
	 case P2041_IIC3:
	     piiccr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0x8;
	     break;
	 default:
	     break;
	}
	
	UCHAR *a = (UCHAR*)&addr;
	if (i2c_wait4bus(port) >= 0
		&& i2c_write_addr(dev, I2C_WRITE_BIT, 0,port) != 0
	    && __i2c_write(&a[4 - alen],alen,port) == alen)
	{
		i = 0; /* No error so far */
	}
	
	if (length && i2c_write_addr(dev, I2C_READ_BIT, 1,port) != 0)
	{
		i = __i2c_read(data, length,port);
	}
	
	writeb(I2C_CR_MEN, piiccr);
		
	if (i2c_wait4bus(port)) 
	{
	    printf("i2c_read: wait4bus timed out\n");
	}
	
	if (i == length)
	{
	    pthread_mutex_unlock(&g_mp_i2cread);
	    return 0;
	}
	reset_i2c(port);
	pthread_mutex_unlock(&g_mp_i2cread);
	return -1;
}

/********************************************************************************
* 函数名称: i2c_write
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/ 
s32 i2c_write(unsigned char dev, unsigned int addr, int alen, unsigned char *data, int length,int port)
{
    int i = -1; /* signal error */
    UCHAR *a = (UCHAR *)&addr;
    volatile void *piiccr;	
	pthread_mutex_lock(&g_mp_i2cwrite);
    switch(port)
    {
         case P2041_IIC0:
    	     piiccr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0x8;		
    	     break;
    	 case P2041_IIC1:
    	     piiccr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0x8;		
    	     break;
    	 case P2041_IIC2:
    	     piiccr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0x8;		
    	     break;
    	 case P2041_IIC3:
    	     piiccr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0x8;	
    	     break;
    	 default:
    	     break;
	}
	if ((i2c_wait4bus(port) >= 0)
	    && (i2c_write_addr(dev, I2C_WRITE_BIT, 0,port) != 0)
	    && (__i2c_write(&a[4 - alen],alen,port) == alen)) 
    {
    	//printf("i2c_write length:0x%lx\r\n",length);
		i = __i2c_write(data,length,port);
	}
	//printf("len = %d\n",i);
	writeb(I2C_CR_MEN, piiccr);  //使能
	if (i2c_wait4bus(port)) /* Wait until STOP */
	{	       
	    printf("i2c_write: wait4bus timed out\n");
	}
	
	if (i == length)
	{
	    pthread_mutex_unlock(&g_mp_i2cwrite);
	    return 0;
	}
	reset_i2c(port);
	pthread_mutex_unlock(&g_mp_i2cwrite);
	return -1;
}

/********************************************************************************
* 函数名称: i2c_get_bus_speed
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
unsigned int i2c_get_bus_speed(void)
{
	return i2c_bus_speed[0];
}

/********************************************************************************
* 函数名称: i2c_wait4bus
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
static int i2c_wait4bus(int port)  //busy?
{
	volatile void *piicsr;
	int i = 0;
	//unsigned char itmp=0;
	switch(port)
	{
		case P2041_IIC0:
			piicsr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET+0xc;
			break;
		case P2041_IIC1:
			piicsr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET+0xc;
			break;
		case P2041_IIC2:
			piicsr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0xc;
			break;
		case P2041_IIC3:
			piicsr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0xc;
			break;
		default:
		    break;
	}		
	while (readb(piicsr) & I2C_SR_MBB) 
	{
		//if (i > 10000000)
		if (i > 10000)
		{		
            printf("i2c_wait4bus: i2c bus busy!\n");
			return -1;
		}
		//itmp = readb(piicsr) & I2C_SR_MBB;
		i++;
	}
	return 0;
}

/********************************************************************************
* 函数名称: Bspi2cprobe
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
u32 Bspi2cprobe(u8 chip,u32 port)
{
    volatile void *piicadr;
    switch(port)
    {
        case P2041_IIC0:
	        piicadr = g_u8ccsbar+CONFIG_SYS_I2C1_OFFSET;	
	        break;
        case P2041_IIC1:
            piicadr = g_u8ccsbar+CONFIG_SYS_I2C2_OFFSET;	
            break;
        case P2041_IIC2:
            piicadr = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET;	
            break;
        case P2041_IIC3:
            piicadr = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET;
            break;
        default:
            break;
	}	
	if (chip == (readb(piicadr) >> 1))
	{
	    return -1;
	}
	return i2c_read(chip, 0, 0, 0, 0,port);
}

/********************************************************************************
* 函数名称: BspDetectI2cDeviceId
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
void BspDetectI2cDeviceId(UINT32 port)
{
    int i=0;
    for (i = 0; i < 128; i++) 
    {
	    if (Bspi2cprobe(i,port) == 0)
	    {
		    printf("IIC port %d device id =0x%02x\n", port,i);
		    break;
	    }
	}
}

/********************************************************************************
* 函数名称:   bsp_read_eeprom							
* 功    能:   AT24C32AN  addr:1010 000                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 
* 参数名称		   类型					输入/输出 		描述		
   pdata           u8 *                 写入的数据缓冲区
   len:            int                  缓冲区长度,
   addr:           u16                  设备内部偏移地址
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_read_eeprom(u8 *u8pread_data,u32 u32len,u16 u16addr)
{  
   u32 u32eepromDevId = 0;
   
   pthread_mutex_lock(&g_mp_i2c1);
   if(BSP_OK != i2c_read(EEPROM_I2C_ADDR,u16addr,sizeof(u16),u8pread_data,u32len,u32eepromDevId))
   {
		if(BSP_OK != i2c_read(EEPROM_I2C_ADDR,u16addr,sizeof(u16),u8pread_data,u32len,u32eepromDevId))
   		{
    		printf("eeprom read error!\n");
    		pthread_mutex_unlock(&g_mp_i2c1);
    		return BSP_ERROR;
        }
   }
   pthread_mutex_unlock(&g_mp_i2c1);
   
   /*if(BSP_OK != i2c_read(EEPROM_I2C_ADDR,u16addr,sizeof(u16),u8pread_data,u32len,u32eepromDevId))
   {
		printf("eeprom read error!\n");
		return BSP_ERROR;
   }*/

   return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_i2c_read							
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
   pdata           u8 *                 写入的数据缓冲区
   len:            int                  缓冲区长度,
   addr:           u16                  设备内部偏移地址
   
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_write_eeprom(u8 *u8pwrite_data,u32 u32len, u16 u16addr)
{ 
	u32 u32eepromDevId = 0;
	u8 eerom_data;
	s32 i;
	pthread_mutex_lock(&g_mp_i2c1);
	for(i = 0;i < u32len;i++)
    {	
        //eeprom_data = u8pwrite_data[i];
	    if(BSP_OK != i2c_write(EEPROM_I2C_ADDR,u16addr+i,sizeof(u16),u8pwrite_data+i,1,u32eepromDevId))
	    {
	    	if(BSP_OK != i2c_write(EEPROM_I2C_ADDR,u16addr+i,sizeof(u16),u8pwrite_data+i,1,u32eepromDevId))
	        {
				printf("eeprom write error!\n");
				pthread_mutex_unlock(&g_mp_i2c1);
				return BSP_ERROR;
	        }
		}
		bsp_sys_msdelay(8);
	}
	pthread_mutex_unlock(&g_mp_i2c1);

	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_get_rtc
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_get_rtc(u8 *u8prtc_buf)
{
    u32 u32rtcDevId = 0;
	u32 u32i;
	u8 u8a,u8b;
	u8 u8addr  = 2;
	u8 u8len =7;

	pthread_mutex_lock(&g_mp_i2c1);
	if(BSP_OK != i2c_read(RTC_I2C_ADDR,u8addr,sizeof(u8),u8prtc_buf,u8len,u32rtcDevId))
	{
	 	if(BSP_OK != i2c_read(RTC_I2C_ADDR,u8addr,sizeof(u8),u8prtc_buf,u8len,u32rtcDevId))
		{
		printf("get rtc error!\n");
		pthread_mutex_unlock(&g_mp_i2c1);
		return BSP_ERROR;
	}
	}
	pthread_mutex_unlock(&g_mp_i2c1);

	u8prtc_buf[0] = u8prtc_buf[0]&0x7f;/*****s********/
	u8prtc_buf[1] = u8prtc_buf[1]&0x7f;/*****min******/
	u8prtc_buf[2] = u8prtc_buf[2]&0x3f;/*****hour*****/
	u8prtc_buf[3] = u8prtc_buf[3]&0x3f;/*****day******/
	u8prtc_buf[4] = u8prtc_buf[4]&0x7; /*****week*****/
	u8prtc_buf[5] = u8prtc_buf[5]&0x1f;/*****month****/
	u8prtc_buf[6] = u8prtc_buf[6]&0xff;/*****year*****/
	
	//printf("time1:%2x,%2x,%2x,%2x,%2x,%2x\n",u8prtc_buf[0]&0x7f,u8prtc_buf[1]&0x7f,u8prtc_buf[2]&0x3f,u8prtc_buf[3]&0x3f,u8prtc_buf[5]&0x1f,u8prtc_buf[6]&0xff);
    //BCD ma
	for(u32i = 0;u32i< 7;u32i++)
	{
		u8a = u8prtc_buf[u32i]>>4;
		u8b = u8prtc_buf[u32i]&0xf;
		u8prtc_buf[u32i] = u8a*10+u8b;
	}
	
	printf("time:%2d:%2d:%2d:%2d:%2d:%2d,week:%2d\n",u8prtc_buf[6],u8prtc_buf[5],u8prtc_buf[3],u8prtc_buf[2],u8prtc_buf[1],u8prtc_buf[0],u8prtc_buf[4]);
    return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_set_rtc
* 功    能:                                     
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_set_rtc(u8 *u8prtc_buf)
{
	u32 u32i;
	u8 u8a,u8b;
    u32 u32rtcDevId = 0;
	u8 u8addr  = 2;
	u8 u8len =7;
	
	for(u32i = 0;u32i < 7;u32i++)
	{
		u8a = u8prtc_buf[u32i]/10;
		u8b = u8prtc_buf[u32i]%10;
		u8prtc_buf[u32i] = u8a*0x10+u8b;
	}
	
	u8prtc_buf[0] = u8prtc_buf[0]&0x7f;/*****s********/
	u8prtc_buf[1] = u8prtc_buf[1]&0x7f;/*****min******/
	u8prtc_buf[2] = u8prtc_buf[2]&0x3f;/*****hour*****/
	u8prtc_buf[3] = u8prtc_buf[3]&0x3f;/*****day******/
	u8prtc_buf[4] = u8prtc_buf[4]&0x7; /*****week*****/
	u8prtc_buf[5] = u8prtc_buf[5]&0x1f;/*****month****/
	u8prtc_buf[6] = u8prtc_buf[6]&0xff;/*****year*****/

	// printf("timeSet:%2x,%2x,%2x,%2x,%2x,%2x\n",u8prtc_buf[0],u8prtc_buf[1],u8prtc_buf[2],u8prtc_buf[3],u8prtc_buf[5],u8prtc_buf[6]);
	pthread_mutex_lock(&g_mp_i2c1);
	if(BSP_OK != i2c_write(RTC_I2C_ADDR,u8addr,sizeof(u8),u8prtc_buf,u8len,u32rtcDevId))
	{
		if(BSP_OK != i2c_write(RTC_I2C_ADDR,u8addr,sizeof(u8),u8prtc_buf,u8len,u32rtcDevId))
		{
		printf("set rtc error!\n");
		pthread_mutex_unlock(&g_mp_i2c1);
		return BSP_ERROR;
	}
	}
	pthread_mutex_unlock(&g_mp_i2c1);

    return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_read_temp_reg							
* 功    能: 温度传感器                                  
* 相关文档:                    
* 函数类型:									
* 参    数:
* 参数名称		   类型					输入/输出 		描述		
				  u8temp_port: temp0,temp1
				  u8temp_measure_point:测量点，1,2,3,4
				  u8pread_temp_data:读数据指针	
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_read_temp(u8 u8temp_port,u8 u8temp_measure_point,s8 *u8pread_temp_data)
{
	u32 u32tempDevId = 1;
	u8 u8temp_addr;
	u8 u8read_temp_data1;
	u8 u8read_temp_data2;

	if(0 == u8temp_port)
	{
		u8temp_addr = TEMP1_I2C_ADDR;
	}
	else if(1 == u8temp_port)
	{
		u8temp_addr = TEMP2_I2C_ADDR;
	}
	else
	{
		printf("the temp port is wrong!\n");
		return BSP_ERROR;
	}

	pthread_mutex_lock(&g_mp_i2c2);
	if(BSP_OK != i2c_read(u8temp_addr,u8temp_measure_point+0x10, sizeof(u8), &u8read_temp_data1,1,u32tempDevId))
	{
		if(BSP_OK != i2c_read(u8temp_addr,u8temp_measure_point+0x10, sizeof(u8), &u8read_temp_data1,1,u32tempDevId))
	    {
		printf("read temp reg error!\n");
		pthread_mutex_unlock(&g_mp_i2c2);
		return BSP_ERROR;
	}
	}
	if(BSP_OK != i2c_read(u8temp_addr,u8temp_measure_point+0x20, sizeof(u8), &u8read_temp_data2,1,u32tempDevId))
	{
	if(BSP_OK != i2c_read(u8temp_addr,u8temp_measure_point+0x20, sizeof(u8), &u8read_temp_data2,1,u32tempDevId))
	{
		printf("read temp reg error!\n");
		pthread_mutex_unlock(&g_mp_i2c2);
		return BSP_ERROR;
	}
	}
	pthread_mutex_unlock(&g_mp_i2c2);
	if(u8read_temp_data1&0x80)
	{
       //printf("the Temperature is -%d\n",u8read_temp_data1);
	}
	else
	{
      // printf("the Temperature is %d\n",u8read_temp_data1);
	}
	*u8pread_temp_data = (s8)(u8read_temp_data1);

    return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_write_temp_reg							
* 功    能: 温度传感器                                  
* 相关文档:                    
* 函数类型:									
* 参    数:
* 参数名称		   类型					输入/输出 		描述		
				  u8temp_port: temp0,temp1
				  u8temp_reg_offset:寄存器
				  read_temp_data:写入的值
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_write_temp(u8 u8temp_port,u8 u8temp_reg_offset,u8 u8write_temp_data)
{
	u32 u32tempDevId = 1;
	u8 u8temp_addr;
	
	if(0 == u8temp_port)
	{
		u8temp_addr = TEMP1_I2C_ADDR;
	}
	else if(1 == u8temp_port)
	{
		u8temp_addr = TEMP2_I2C_ADDR;
	}
	else
	{
		printf("the temp port is wrong!\n");
		return BSP_ERROR;
	}
	pthread_mutex_lock(&g_mp_i2c2);
	if(BSP_OK != i2c_write(u8temp_addr, u8temp_reg_offset, sizeof(u8),&u8write_temp_data,1,u32tempDevId))
	{
		if(BSP_OK != i2c_write(u8temp_addr, u8temp_reg_offset, sizeof(u8),&u8write_temp_data,1,u32tempDevId))
		{
		printf("write temp reg error!\n");
		pthread_mutex_unlock(&g_mp_i2c2);
		return BSP_ERROR;
		}
	}
	pthread_mutex_unlock(&g_mp_i2c2);
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_config_power_sense							
* 功    能: 电源监测传感器                                 
* 相关文档:                    
* 函数类型:									
* 参    数:	 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: u8reg_offset
           0    =  Configuration 寄存器
           5    =  Calibration   寄存器
*********************************************************************************/
s32 bsp_config_power_sense(u16 u16power_val,u8 u8reg_offset)
{
	u32 u32powerDevId = 1;
	u8  u8sense_config[2]={0};
	
    u8sense_config[0] = (u16power_val>>8)&0xff;
	u8sense_config[1] = u16power_val&0xff;
	pthread_mutex_lock(&g_mp_i2c2);
	if(BSP_OK !=i2c_write(POWERSENSE_I2C_ADDR, u8reg_offset, sizeof(u8), u8sense_config,2,u32powerDevId))
	{
		if(BSP_OK !=i2c_write(POWERSENSE_I2C_ADDR, u8reg_offset, sizeof(u8), u8sense_config,2,u32powerDevId))
		{
		printf("write temp reg error!\n");
		pthread_mutex_unlock(&g_mp_i2c2);
		return BSP_ERROR;
		}
	}
	pthread_mutex_unlock(&g_mp_i2c2);
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_current_monitor_init							
* 功    能: 电源监测传感器初始化                                
* 相关文档:                    
* 函数类型:									
* 参    数:						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 在板级初始化时调用
*********************************************************************************/
void  bsp_current_monitor_init(void)
{
   //Configuration
   bsp_config_power_sense(0x19ff,0);
   //Calibration
   bsp_config_power_sense(0X27FF,5);
}

/******************************************************************************
* 函数名称: bsp_read_power_sense							
* 功    能: 读power 传感器寄存器值                             
* 相关文档:                    
* 函数类型:									
* 参    数:						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
			Configuration Register:0
			Shunt Voltage:1
			Bus Voltage:2
			Power:      3
			Current:    4
			Calibration:5
*********************************************************************************/
s32 bsp_read_power_sense(u8 u8reg_offset,u16 *u16p_power)
{
	u32 u32powerDevId = 1;
	u8 u8read_power_val[2]= {0};

	pthread_mutex_lock(&g_mp_i2c2);
	if(BSP_OK != i2c_read(POWERSENSE_I2C_ADDR, u8reg_offset,sizeof(u8),u8read_power_val,2,u32powerDevId))
	{
		if(BSP_OK != i2c_read(POWERSENSE_I2C_ADDR, u8reg_offset,sizeof(u8),u8read_power_val,2,u32powerDevId))
		{
		printf("write power sense reg error!\n");
		pthread_mutex_unlock(&g_mp_i2c2);	
		return BSP_ERROR;
	}
	}
	pthread_mutex_unlock(&g_mp_i2c2);	
	
	*u16p_power = (u16)u8read_power_val[0];	
	//printf("0x%x\n",(*u16p_power)<<8);
	//printf("0x%x\n",u8read_power_val[1]);
	*u16p_power = (*u16p_power)*256 + u8read_power_val[1];   //先读高字节，后低字节
	//printf("u16p_power = 0x%x\n",(*u16p_power));

	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_read_bus_voltage							
* 功    能: 读取线电压寄存器值                   
* 相关文档:                    
* 函数类型:							
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: float  Vbus = *Voltage*0.004  (V)
*********************************************************************************/
s32 bsp_read_bus_voltage(f32 *f32pvoltage)
{
    //u8 voltage_reg[2];
	u16 u16voltage_reg;
    if(bsp_read_power_sense(2,&u16voltage_reg)== BSP_ERROR)
    {
		printf("read Bus Voltage error!\n");
		return BSP_ERROR;
	}

   /* if(u16voltage_reg&0x01)
    {
		printf("the Power or Current calculations are out of range!\n");
        return BSP_ERROR;
	}
	
	if(!(u16voltage_reg&0x02))
    {
		printf("Conversion is not Ready!\n");
        return BSP_ERROR;
	}
	*/
	
	//printf("voltage = 0x%x\n",u16voltage_reg>>3);
	
	*f32pvoltage = (u16voltage_reg>>3)*0.004;  //根据定义右移三位

	printf("voltage = %f\n",*f32pvoltage);
	
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_read_current							
* 功    能: 读电流传感器                        
* 相关文档:                    
* 函数类型:							
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: LSB = 0.001A
*********************************************************************************/
s32 bsp_read_current(f32 *f32pcurrent)
{
    u16 u16current_reg;
    if(bsp_read_power_sense(4,&u16current_reg) == BSP_ERROR)
    {
        if(bsp_read_power_sense(4,&u16current_reg) == BSP_ERROR)
		{
			printf("read Bus Voltage error!\n");
			return BSP_ERROR;
		}	
		printf("read Bus Voltage error!\n");
		return BSP_ERROR;
	}
	if(u16current_reg == 0)
	{
	    bsp_sys_msdelay(50);
		if(bsp_read_power_sense(4,&u16current_reg) == BSP_ERROR)
		{
			printf("read Bus Voltage error!\n");
			return BSP_ERROR;
		}	
	}
	*f32pcurrent = u16current_reg*0.001-0.6;//校正电流值0.6A
	printf("current = %f\n",*f32pcurrent);
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_read_power							
* 功    能: 读取功率                           
* 相关文档:                    
* 函数类型:							
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_read_power(f32 *f32p_power)
{
    //u16 u16power_reg;
	f32 f32pcurrent;
	f32 f32pvoltage;
   /* if(bsp_read_power_sense(3,&u16power_reg)== BSP_ERROR)
    {
		printf("read Bus Voltage error!\n");
		return BSP_ERROR;
	}
	//Shunt voltage 60mv，current 1mA
    //printf("power = %d\n",u16power_reg*6/100/1000);
    *f32p_power = u16power_reg*0.06*0.001;
	*/
	bsp_read_current(&f32pcurrent);
    bsp_read_bus_voltage(&f32pvoltage);
    
    f32p_power[0] = f32pvoltage;
    f32p_power[1] = f32pcurrent;
    f32p_power[2] = f32pcurrent*f32pvoltage;
    
	printf("power = %f\n",f32p_power[2]);
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_fan_config							
* 功    能: 风扇控制器操作接口                               
* 相关文档:                    
* 函数类型:		Fan Speed (RPM) = (90,000 × 60)/Fan Tach Reading 							
* 参    数:pdata:写入的数据缓冲区,len:缓冲区长度,addr:设备内部偏移地址	 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_fan_config_init(void)
{
 	u32 u32fanDevId =3;
	u8 u8freq = 0x10;

    pthread_mutex_lock(&g_mp_i2c4);
    if(BSP_OK != i2c_write(FAN_I2C_ADDR,0x74, sizeof(u8), &u8freq,1,u32fanDevId))
	{
	    if(BSP_OK != i2c_write(FAN_I2C_ADDR,0x74, sizeof(u8), &u8freq,1,u32fanDevId))
		{
		printf("the fan init config error!\n");
		pthread_mutex_unlock(&g_mp_i2c4);
		return BSP_ERROR;
	}
	}
	pthread_mutex_unlock(&g_mp_i2c4);
	bsp_fan_control(255);
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_get_fan_speed							
* 功    能: 风扇控制器操作接口                               
* 相关文档:                    
* 函数类型:		Fan Speed (RPM) = (90,000 × 60)/Fan Tach Reading 							
* 参    数:pdata:写入的数据缓冲区,len:缓冲区长度,addr:设备内部偏移地址	 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_get_fan_speed(u8 u8fan_id,u16 *u16pfan_speed)
{
    u32 u32fanDevId =3;
	u8 u8speed_high,u8speed_low;
	u8 u8reg_high,u8reg_low;
	u16 u16speed_reg;
	switch(u8fan_id)
	{
		case 1:
			u8reg_low  = 0x2a;
			u8reg_high = 0x2b;		
			break;
		case 2:
			u8reg_low = 0x2c;
			u8reg_high = 0x2d;
			break;
		case 3:
			u8reg_low = 0x2e;
			u8reg_high = 0x2f;
			break;
		case 4:
			u8reg_low  = 0x30;
			u8reg_high = 0x31;
			break;
		default:
			printf("the fan num is wrong!\n");
			return BSP_ERROR;
	}

    pthread_mutex_lock(&g_mp_i2c4);
	if(BSP_ERROR == i2c_read(FAN_I2C_ADDR, u8reg_low, sizeof(u8), &u8speed_low, 1,u32fanDevId))
	{
	   	if(BSP_ERROR == i2c_read(FAN_I2C_ADDR, u8reg_low, sizeof(u8), &u8speed_low, 1,u32fanDevId))
		{
		printf("get fan speed low reg error!\n");
		pthread_mutex_unlock(&g_mp_i2c4);	
		return BSP_ERROR;
	}
	}
	
    if(BSP_ERROR == i2c_read(FAN_I2C_ADDR, u8reg_high, sizeof(u8), &u8speed_high, 1,u32fanDevId))
	{
    if(BSP_ERROR == i2c_read(FAN_I2C_ADDR, u8reg_high, sizeof(u8), &u8speed_high, 1,u32fanDevId))
	{
		printf("get fan speed high reg error!\n");
		pthread_mutex_unlock(&g_mp_i2c4);	
		return BSP_ERROR;
	}
	}
	pthread_mutex_unlock(&g_mp_i2c4);	

	u16speed_reg = u8speed_high*256 + u8speed_low;
	//printf("u16speed_reg = 0x%x",u16speed_reg);
	*u16pfan_speed = 90000*60/u16speed_reg;
	/*弥补计算误差，小于100转/min表明实际转速已经为0*/
    if(*u16pfan_speed<100 || *u16pfan_speed>18000)
    {
		*u16pfan_speed = 0;	
		return BSP_ERROR;
	}
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_set_fan_speed							
* 功    能: 设置风扇转速                               
* 相关文档:                    
* 函数类型:					
* 参    数:						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 bsp_set_fan_speed(u8 u8fan_id,u8 u8fan_pwmval)
{
    u32 u32fanDevId =3;
	u8 u8reg;
	switch(u8fan_id)
	{
		case 1:
			u8reg = 0x32;
			break;
		case 2:
			u8reg = 0x33;
			break;
		case 3:
			u8reg = 0x34;
			break;
		case 4:
			u8reg = 0x35;
			break;
		default:
			printf("the fan num is wrong!\n");
			return BSP_ERROR;
	}
	
	pthread_mutex_lock(&g_mp_i2c4);
    if(BSP_ERROR == i2c_write(FAN_I2C_ADDR,u8reg, sizeof(u8), &u8fan_pwmval,1,u32fanDevId))
	{
	    if(BSP_ERROR == i2c_write(FAN_I2C_ADDR,u8reg, sizeof(u8), &u8fan_pwmval,1,u32fanDevId))
		{
		printf("set fan speed error!\n");
		pthread_mutex_unlock(&g_mp_i2c4);
		return BSP_ERROR;
	}
	}
	pthread_mutex_unlock(&g_mp_i2c4);
	return BSP_OK;
}

float bsp_get_temperature(void)
{ 
	s32 i;
	s8 temp0[4];
	float temp_avg;

	for (i = 1; i < 5; i++) {
		bsp_read_temp(0, i, &temp0[i - 1]);
	}

	temp_avg = (float)((float)temp0[0] * 0.25 + (float)temp0[1] * 0.25 + (float)temp0[2] * 0.25 + (float)temp0[3] * 0.25);

    return temp_avg;
}

s32 bsp_fan_control(u8 fan_pwmval)
{
	int i;

	for (i = 1; i < 5; i++) {
		if (bsp_set_fan_speed(i, fan_pwmval) == BSP_ERROR)
			return BSP_ERROR; 
	}
	return BSP_OK;
}

/******************************************************************************
 * * 函数名: fan_control_algorithm1
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
#if 0
s32 fan_control_algorithm_1(s8 temp)
{
	static u8 pwm_val = 255;

	if (temp <= 27)
		return 0;
	else if (temp > 27 && temp <= 38)
		pwm_val = 60;	
	else if (temp > 40 && temp <= 48)
		pwm_val = 90;
	else if (temp > 50 && temp <= 58)
		pwm_val = 120;	
	else if (temp > 60 && temp <= 68)
		pwm_val = 200;
	else if (temp > 70)
		pwm_val = 255;

	return bsp_fan_control(pwm_val);
}
#else 
s32 fan_control_algorithm_1(s8 temp)
{
	static s8 temp_prev = 0;
	static u8 pwm_val = 0;
	
	if (temp > (temp_prev+5)) {
		if (temp < 30)
			pwm_val = 0;
		else if (temp >= 30 && temp <= 70)
			pwm_val = (u8)(255 * (0.025 * (float)temp - 0.75));
		else
			pwm_val = 255;
	} else if(temp < (temp_prev-5)) {
		if (temp < 25)
			pwm_val = 0;
		else if (temp >= 25 && temp <= 65)
			pwm_val = (u8)(255 * (0.025 * (float)temp - 0.625));
		else
			pwm_val = 255;
	}else { //if(temp == temp_prev)
		//temp_prev = temp;
		return BSP_OK;
	}
	
	temp_prev = temp;
	//printf("set fan speed...temp=%d, pwm=%d\n", temp, pwm_val);
	return bsp_fan_control(pwm_val);
}
#endif 
s32 bsp_get_i2c_data(T_EEPROM_TABLE *eeprom_data)
{
	if(eeprom_data==NULL)
	{
		return BSP_ERROR;
	}
    if ( BSP_OK != bsp_read_eeprom((u8 *)eeprom_data, sizeof(T_EEPROM_TABLE),0))
    {
        printf("I2C read failed\r\n");
        return BSP_ERROR;
    }
	return BSP_OK;
}

#define MPC_I2C_SR    0x0c
#define CSR_MIF       0x02
#define MPC_I2C_CR    0x08


#define CONFIG_SYS_I2C_SPEED		100000
void bsp_set_i2c_slave(int i)
{
    volatile void *piiccr_iic3;
    volatile void *piiccr_iic4;
    int i1=0;
    //piiccr_iic3 = g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+MPC_I2C_CR;
    //piiccr_iic4 = g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+MPC_I2C_CR;
    //i2c_init(CONFIG_SYS_I2C_SPEED,2);
    //i2c_init(CONFIG_SYS_I2C_SPEED,3);
    if (1 == i)
    {
        writeb(IPMB_MCT_HMI1_I2C_ADDR(bsp_get_slot_id())<<1, g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0);	
    }
    else
    {
        writeb(IPMB_MCT_HMI2_I2C_ADDR(bsp_get_slot_id())<<1, g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0);
    }
    if (1 == i)
    {
        writeb(0x29, g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+0x4);	
    }
    else
    {
        writeb(0x29, g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+0x4);	
    }
    //writeb(0xc1, g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+MPC_I2C_CR);	
    //writeb(0xc1, g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+MPC_I2C_CR);	
    
    //writeb(0xe1, g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+MPC_I2C_SR);	
    //writeb(0xe1, g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+MPC_I2C_SR);	
    if (1 == i)
    {
        writeb(0xc0, g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+MPC_I2C_SR);	
    }
    else
    {
        writeb(0xc0, g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+MPC_I2C_SR);	
    }
    
    if (1 == i)
    {
        writeb(0xe0, g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+MPC_I2C_CR);	
    }
    else
    {
        writeb(0xe0, g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+MPC_I2C_CR);	
    }
    for(i1=0;i1<100;i1++);
    if (1==i)
    {
        writeb(0xc0, g_u8ccsbar+CONFIG_SYS_I2C3_OFFSET+MPC_I2C_CR);	
    }
    else
    {
        writeb(0xc0, g_u8ccsbar+CONFIG_SYS_I2C4_OFFSET+MPC_I2C_CR);
    }	    
}


/*********************************************************************************************

*********************************** EEPROM配置/获取接口***************************************

**********************************************************************************************/
EEPROM_PAR_STRU g_mca_eeprom_par = {0};
/******************************************************************************
 * * 函数名: bsp_eeprom_calc_crc
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
u16 bsp_eeprom_calc_crc(u8 *pData)
{
    u8 i = 0;
    u16 checksum = 0;
    /* 减去CRC两个字节 */
    u16 data_len = (sizeof(EEPROM_PAR_STRU) - 2)/2;
    u16 *data = (u16 *)pData;

    for(i = 0; i < data_len; i++)
    {       
        checksum ^= *(data+ i);
    }

    printf("bsp_eeprom_calc_crc:crc is 0x%x\n",checksum);
    
    return checksum;
}
/******************************************************************************
 * * 函数名: bsp_eeprom_set_crc
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_eeprom_set_crc(u16 crc)
{
    u16 data = crc;
    s32 return_value = BSP_OK;
    
    if(ERROR == bsp_write_eeprom((u8 *)&data,sizeof(data),EEPROM_ADDRESS_CRC))
    {
        printf("write i2c eeprom data error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        g_mca_eeprom_par.checkSum= crc;
    }
    
    return return_value;
}
/******************************************************************************
 * * 函数名: bsp_eeprom_get_crc
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_eeprom_get_crc(void)
{
    u16 data = 0;
    s32 return_value = BSP_OK;
    
    if ( BSP_OK != bsp_read_eeprom((u8 *)&data, sizeof(data),EEPROM_ADDRESS_CRC))
    {
        printf("I2C read failed\n");
        return_value = BSP_ERROR;
    }
    else
    {
        g_mca_eeprom_par.checkSum = data;
    }

    return return_value;
}
/******************************************************************************
 * * 函数名: bsp_set_bbu_deviceid
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_deviceid(const char *deviceid)
{
	s8 data[16] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, deviceid, strlen(deviceid));
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_DEVICE_ID) )
	{
		printf("bsp_set_bbu_deviceid error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.device_id, data, len);
    }    
	
	return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_deviceid
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_deviceid(void)
{
	s8 data[16] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;
	
	if ( BSP_OK != bsp_read_eeprom(data, sizeof(data),EEPROM_ADDRESS_DEVICE_ID))
    {
        printf("bsp_get_bbu_deviceid error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.device_id, data, len);
    }    
    
    return return_value;

}
/******************************************************************************
 * * 函数名: bsp_set_bbu_boardtype
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_boardtype(const char *board_type)
{
	s8 data[32] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, board_type, strlen(board_type));
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_BOARD_TYPE) )
	{
		printf("bsp_set_bbu_boardtype error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.board_type, data, len);
    }    
	
	return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_boardtype
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_boardtype(void)
{
	s8 data[32] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;
	
	if ( BSP_OK != bsp_read_eeprom(data, sizeof(data),EEPROM_ADDRESS_BOARD_TYPE))
    {
        printf("bsp_get_bbu_boardtype error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.board_type, data, len);
    }    
    
    return return_value;
}

/******************************************************************************
 * * 函数名: bsp_set_bbu_macaddr1
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_macaddr1(const u8 *macaddr1)
{
	u8 data[6] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, macaddr1, len);
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_MAC_ADDR1) )
	{
		printf("bsp_set_bbu_macaddr1 error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.mac_addr1, data, len);
    }    
	
	return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_macaddr1
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_macaddr1(void)
{
	u8 data[6] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;
	
	if ( BSP_OK != bsp_read_eeprom(data, sizeof(data),EEPROM_ADDRESS_MAC_ADDR1))
    {
        printf("bsp_get_bbu_macaddr1 error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.mac_addr1, data, len);
    }    
    
    return return_value;
}
/******************************************************************************
 * * 函数名: bsp_set_bbu_macaddr2
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_macaddr2(const u8 *macaddr2)
{
	u8 data[6] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, macaddr2, len);
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_MAC_ADDR2) )
	{
		printf("bsp_set_bbu_macaddr2 error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.mac_addr2, data, len);
    }    
	
	return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_macaddr2
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_macaddr2(void)
{
	u8 data[6] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;
	
	if ( BSP_OK != bsp_read_eeprom(data, sizeof(data),EEPROM_ADDRESS_MAC_ADDR2))
    {
        printf("bsp_get_bbu_macaddr2 error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.mac_addr2, data, len);
    }    
    
    return return_value;
}
/******************************************************************************
 * * 函数名: bsp_set_bbu_productsn
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_productsn(const s8 *productsn)
{
	s8 data[32] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, productsn, len);
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_PRODUCT_SN) )
	{
		printf("bsp_set_bbu_productsn error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.product_sn, data, len);
    }    
	
	return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_productsn
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_productsn(void)
{
	s8 data[32] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;
	
	if ( BSP_OK != bsp_read_eeprom(data, sizeof(data),EEPROM_ADDRESS_PRODUCT_SN))
    {
        printf("bsp_get_bbu_productsn error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.product_sn, data, len);
    }    
    
    return return_value;
}
/******************************************************************************
 * * 函数名: bsp_set_bbu_manufacturer
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_manufacturer(const char *manufacturer)
{
	s8 data[12] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, manufacturer, strlen(manufacturer));
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_MANUFACTRUER) )
	{
		printf("bsp_set_bbu_manufacturer error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.manufacturer, data, len);
    }    
	
	return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_manufacturer
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_manufacturer(void)
{
	s8 data[12] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;
	
	if ( BSP_OK != bsp_read_eeprom(data, sizeof(data),EEPROM_ADDRESS_MANUFACTRUER))
    {
        printf("bsp_get_bbu_manufacturer error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.manufacturer, data, len);
    }    
    
    return return_value;
}
/******************************************************************************
 * * 函数名: bsp_set_bbu_productdate
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_productdate(const u8 *productdate)
{
	u8 data[4] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, productdate, strlen(productdate));
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_PRODUCT_DATE) )
	{
		printf("bsp_set_bbu_productdate error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.product_date, data, len);
    }    
	
	return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_productdate
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_productdate(void)
{
	u8 data[4] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;
	
	if ( BSP_OK != bsp_read_eeprom(data, sizeof(data),EEPROM_ADDRESS_PRODUCT_DATE))
    {
        printf("bsp_get_bbu_productdate error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.product_date, data, len);
    }    
    
    return return_value;
}

/******************************************************************************
 * * 函数名: bsp_set_bbu_satellitereceiver
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_satellitereceiver(const char *satellitereceiver)
{
	s8 data[12] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, satellitereceiver, strlen(satellitereceiver));
	if ( BSP_OK != bsp_write_eeprom(data, sizeof(data),EEPROM_ADDRESS_SATELLITE_RECEIVER))
    {
        printf("bsp_set_bbu_satellitereceiver error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.satellite_receiver, data, len);
    }    
    
    return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_satellitereceiver
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_satellitereceiver(void)
{
	s8 data[12] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;
	
	if ( BSP_OK != bsp_read_eeprom(data, sizeof(data),EEPROM_ADDRESS_SATELLITE_RECEIVER))
    {
        printf("bsp_get_bbu_satellitereceiver error!\n");
        return_value = BSP_ERROR;
    }
    else
    {
        memcpy(g_mca_eeprom_par.satellite_receiver, data, len);
    }    
    
    return return_value;
}
/******************************************************************************
 * * 函数名: bsp_set_bbu_temperaturethreshold
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_set_bbu_temperaturethreshold(const s8 *temperaturethreshold)
{
	s8 data[2] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, temperaturethreshold, len);
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_TEMPERATURE_THRESHOLD) )
	{
		printf("bsp_set_bbu_temperaturethreshold error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.temperature_threshold, data, len);
    }    
	
	return return_value;
}

/******************************************************************************
 * * 函数名: bsp_get_bbu_temperaturethreshold
 * * 功  能: 
 * * 函数存储类型:
 * * 参数:
 * * 参数名        类型        输入/输出       描述
 * * 返回值: 0 正常，其他错误
 * * 说明:
 * ******************************************************************************/
s32 bsp_get_bbu_temperaturethreshold(void)
{
	s8 data[2] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

	if ( ERROR == bsp_read_eeprom(data, len, EEPROM_ADDRESS_TEMPERATURE_THRESHOLD) )
	{
		printf("bsp_get_bbu_temperaturethreshold error!\n");
        return_value = BSP_ERROR;
	}
    else
    {
        memcpy(g_mca_eeprom_par.temperature_threshold, data, len);
    }    
	
	return return_value;
}

#if 0
s32 bsp_get_bbu_pcbversion(char *buffer_version, int len)
{
	T_EEPROM_TABLE eepromData ={0};	
	
	if(BSP_OK==bsp_get_i2c_data(&eepromData))
	{
		if(len > 32)
		{
			len = 32;
		}
		memset(buffer_version, 0, len);
		memcpy(buffer_version, eepromData.pcb_version, len-1);
		return BSP_OK;
	}
	return BSP_ERROR;
}

s32 bsp_get_bbu_devicdid(char *buffer_devicdid, int len)
{
	T_EEPROM_TABLE eepromData ={0};	
	
	if(BSP_OK==bsp_get_i2c_data(&eepromData))
	{
		if(len > 32)
		{
			len = 32;
		}
		memset(buffer_devicdid, 0, len);
		memcpy(buffer_devicdid, eepromData.device_id, len-1);
		return BSP_OK;
	}
	return BSP_ERROR;
}

int bsp_get_bbu_production_date(int *year, int *month, int *day)
{
	T_EEPROM_TABLE eepromData;
	int ret;

	memset(&eepromData, 0, sizeof(eepromData));

	ret = bsp_get_i2c_data(&eepromData);

	if (ret == BSP_ERROR) {
		printf("bsp_get_bbu_production_date error\n");
		goto out;
	}

	*year = (int)eepromData.product_date[0] * 100 + (int)eepromData.product_date[1];
	*month = (int)eepromData.product_date[2];
	*day = (int)eepromData.product_date[3];

	printf("%d-%02d-%02d\n", *year, *month, *day);
out:
	return ret;
}
#endif

/*********************************************************************************/
/*                                      for test                                 */
/*********************************************************************************/
#if 1
s32 eeprom_read_test(u16 addr, u32 len)
{
    u8 eeprom[32] = {0};
    s32 return_value = BSP_OK;
    u8 i = 0;
    
    if(len<32)
    {
        if(BSP_OK != bsp_read_eeprom(eeprom,len,addr))
            return_value = BSP_ERROR;
        else
        {
            for(i=0;i<len;i++)
                printf("0x%x ", eeprom[i]);
            printf("\r\n");
        }
    }
    return return_value;
}
void eeprom_write_test(void)
{
	u8  init_data[100] ; 
	u8  eeprom[100];
	int i;

	//i2c_init(CONFIG_SYS_I2C_SPEED,0);
	for(i=0;i<100;i++)
	{
		init_data[i] = i;
	}

	bsp_write_eeprom(init_data,50,0);
	bsp_read_eeprom(eeprom,50,0);

	for(i = 0;i < 50; i++)
	{
		printf("%d",eeprom[i]);
	}
}

void rtc_test(void)
{
    u8 rtc[7];
	u8 rtc1[7] = {0,10,10,23,1,9,13};
    //i2c_init(CONFIG_SYS_I2C_SPEED,0);	
	bsp_get_rtc(rtc);
	bsp_set_rtc(rtc1);	
	bsp_get_rtc(rtc);
}

void rtc_get(void)
{
	u8 rtc[7];	
	bsp_get_rtc(rtc);	
}

void temp_test(void)
{ 
	u8 i;
	s8 temp[4];

	//i2c_init(CONFIG_SYS_I2C_SPEED,1);

	for(i=0;i<4;i++)
	{
		bsp_read_temp(0,i+1,&temp[i]); 
	}
	
	printf("temp0(1~4) = %d,%d,%d,%d\n",temp[0],temp[1],temp[2],temp[3]);
#if 0
	for(i=0;i < 4;i++)
	{
		bsp_read_temp(1,i+1,&temp[i]);
	}
	printf("temp1(1~4) = %d,%d,%d,%d\n",temp[0],temp[1],temp[2],temp[3]);
#endif
 }

void power_test(void)
{
	f32 vol;
	f32 current;
	f32 power;
	//i2c_init(CONFIG_SYS_I2C_SPEED,2);
	//bsp_current_monitor_init();
	bsp_read_bus_voltage(&vol);
	bsp_read_current(&current);
	bsp_read_power(&power);
}

void fan_test(u8 fan_pwmval)//0x80 半速
{
	u16 fan_speed;
	u8  fan_pwmval1;

	//i2c_init(CONFIG_SYS_I2C_SPEED,3);
	//bsp_fan_config_init();
	bsp_get_fan_speed(3,&fan_speed);
	printf("fan_spped = %d\n",fan_speed);
	bsp_set_fan_speed(3,fan_pwmval); 
	
	bsp_get_fan_speed(3,&fan_speed);
	if(BSP_ERROR == i2c_read(FAN_I2C_ADDR,0x34,sizeof(u8),&fan_pwmval1,1,3))
	{
		printf("get fan speed error!\n");
		return ;
	}
	
	printf("fan_pwmval1 = 0x%x\n",fan_pwmval1);
}

s32 bsp_set_bbu_deviceid_test(void)
{
    char *deviceid = "C6448";

    s8 data[16] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, deviceid, strlen(deviceid));
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_DEVICE_ID) )
	{
		printf("bsp_set_bbu_deviceid error!\n");
        return_value = BSP_ERROR;
	}
    	
	return return_value;
}

s32 bsp_set_bbu_boardtype_test(void)
{
    char *board_type = "C6448_MCa.00.01";

	s8 data[32] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, board_type, strlen(board_type));
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_BOARD_TYPE) )
	{
		printf("bsp_set_bbu_boardtype error!\n");
        return_value = BSP_ERROR;
	}  
	
	return return_value;

}

s32 bsp_set_bbu_macaddr1_test(void)
{
    u8 macaddr1[6] = {0x00,0x0a,0x1e,0x04,0x04,0x04};

    u8 data[6] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, macaddr1, len);
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_MAC_ADDR1) )
	{
		printf("bsp_set_bbu_macaddr1 error!\n");
        return_value = BSP_ERROR;
	}
  	
	return return_value;
}

s32 bsp_set_bbu_macaddr2_test(void)
{
    u8 macaddr2[6] = {0x00,0x0a,0x1e,0x03,0x03,0x03};

    u8 data[6] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, macaddr2, len);
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_MAC_ADDR2) )
	{
		printf("bsp_set_bbu_macaddr2 error!\n");
        return_value = BSP_ERROR;
	}
  	
	return return_value;
}

s32 bsp_set_bbu_productsn_test(void)
{
    char *productsn = "aa-bb-cc-dd-ee-ff";

    s8 data[32] = {0};
    u32 len = strlen(productsn);
    s32 return_value = BSP_OK;

    memcpy(data, productsn, len);
    if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_PRODUCT_SN) )
    {
    	printf("bsp_set_bbu_productsn error!\n");
        return_value = BSP_ERROR;
    }
   	
    return return_value;
}

s32 bsp_set_bbu_manufacturer_test(void)
{
    char *manufacturer = "XINWEI";

    s8 data[12] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, manufacturer, strlen(manufacturer));
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_MANUFACTRUER) )
	{
		printf("bsp_set_bbu_manufacturer error!\n");
        return_value = BSP_ERROR;
	}
    
	return return_value;
}

s32 bsp_set_bbu_productdate_test(void)
{
    u8 productdate[4] = {0x02,0x25,0x20,0x16};

    u8 data[4] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, productdate, len);
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_PRODUCT_DATE) )
	{
		printf("bsp_set_bbu_productdate error!\n");
        return_value = BSP_ERROR;
	}
 	
	return return_value;
}

s32 bsp_set_bbu_satellitereceiver_test(void)
{
    char *satellitereceiver = "LEA-8T";

    s8 data[12] = {0};
	u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, satellitereceiver, strlen(satellitereceiver));
	if ( BSP_OK != bsp_write_eeprom(data, sizeof(data),EEPROM_ADDRESS_SATELLITE_RECEIVER))
    {
        printf("bsp_set_bbu_satellitereceiver error!\n");
        return_value = BSP_ERROR;
    }   
    
    return return_value;
}

s32 bsp_set_bbu_temperaturethreshold_test(void)
{
    s8 temperaturethreshold[2] = {85, -10};

    s8 data[2] = {0};
    u32 len = sizeof(data);
	s32 return_value = BSP_OK;

    memcpy(data, temperaturethreshold, len);
	if ( ERROR == bsp_write_eeprom(data, len, EEPROM_ADDRESS_TEMPERATURE_THRESHOLD) )
	{
		printf("bsp_set_bbu_temperaturethreshold error!\n");
        return_value = BSP_ERROR;
	}
    
	return return_value;
}

#endif




