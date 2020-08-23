/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_spi.c 
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
#include "../inc/bsp_spi.h"
#include "../../../com_inc/fsl_p2041_ext.h"
#include "../../../com_inc/bsp_i2c_ext.h"   //test
/******************************* 局部宏定义 *********************************/
#define ESPI_MAX_CS_NUM		4

#define ESPI_EV_RNE		(1 << 9)
#define ESPI_EV_TNF		(1 << 8)

#define ESPI_MODE_EN		(1 << 31)	/* Enable interface */
#define ESPI_MODE_TXTHR(x)	((x) << 8)	/* Tx FIFO threshold */
#define ESPI_MODE_RXTHR(x)	((x) << 0)	/* Rx FIFO threshold */

#define ESPI_COM_CS(x)		((x) << 30)
#define ESPI_COM_TRANLEN(x)	((x) << 0)

#define ESPI_CSMODE_CI_INACTIVEHIGH	(1 << 31)
#define ESPI_CSMODE_CP_BEGIN_EDGCLK	(1 << 30)
#define ESPI_CSMODE_REV_MSB_FIRST	(1 << 29)
#define ESPI_CSMODE_DIV16		(1 << 28)
#define ESPI_CSMODE_PM(x)		((x) << 24)
#define ESPI_CSMODE_POL_ASSERTED_LOW	(1 << 20)
#define ESPI_CSMODE_LEN(x)		((x) << 16)
#define ESPI_CSMODE_CSBEF(x)		((x) << 12)
#define ESPI_CSMODE_CSAFT(x)		((x) << 8)
#define ESPI_CSMODE_CSCG(x)		((x) << 3)

#define ESPI_CSMODE_INIT_VAL (ESPI_CSMODE_POL_ASSERTED_LOW | \
		ESPI_CSMODE_CSBEF(0) | ESPI_CSMODE_CSAFT(0) | \
		ESPI_CSMODE_CSCG(1))

#define ESPI_MAX_DATA_TRANSFER_LEN 0xFFF0

#define SPI_CS0  0
#define SPI_CS1  1
#define SPI_CS2  2
#define SPI_CS3  3
/*********************** 全局变量定义/初始化 **************************/

struct fsl_espi_reg {
	u32 mode;		/* 0x000 - eSPI mode register */
	u32 event;		/* 0x004 - eSPI event register */
	u32 mask;		/* 0x008 - eSPI mask register */
	u32 command;	/* 0x00c - eSPI command register */
	u32 tx;			/* 0x010 - eSPI transmit FIFO access register*/
	u32 rx;			/* 0x014 - eSPI receive FIFO access register*/
	u8 res[8];		/* 0x018 - 0x01c reserved */
	u32 csmode[4];	/* 0x020 - 0x02c eSPI cs mode register */
};
struct fsl_espi_reg *spi_reg;

ul32 ul32freqSystemBus = 600000000;//系统时钟频率，需要外部获得

pthread_mutex_t  g_spi_lock = PTHREAD_MUTEX_INITIALIZER;  /* 互斥SPI访问*/

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif /* min */

/************************** 局部常数和类型定义 ************************/


/*************************** 局部函数原型声明 **************************/

extern void bsp_sys_msdelay(ul32 dwTimeOut);// ---延迟单位为毫秒
extern void bsp_sys_usdelay(ul32 dwTimeOut); //---延迟单位为微妙

/***************************函数实现 *****************************/
/********************************************************************************
* 函数名称: spi_write_reg						
* 功    能: 测试函数，片选，通道属性                       
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
Private void spi_write_reg(u32 *u32preg, u32 u32val)
{
	*u32preg = u32val;
}

/********************************************************************************
* 函数名称: spi_read_reg						
* 功    能: 测试函数，片选，通道属性                       
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
Private u32 spi_read_reg(u32 *u32preg)
{
	return *u32preg;
}

/********************************************************************************
* 函数名称: spi_setup_slave							
* 功    能: 设置各个通道的属性                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
	u32max_hz :时钟频率
	u32mode   :模式   可固定   
	        SPI_CPHA	0x01		clock phase 
            SPI_CPOL	0x02		clock polarity 
* 说   明: 
*********************************************************************************/
s32 bsp_spi_setup_slave(u32 u32cs,u32 u32max_hz,u32 u32mode)
{
	ul32 ul32spibrg = 0;
	u8 u8pm = 0;
	u32  u32div16;
	s32 i;

	if (u32cs >= ESPI_MAX_CS_NUM)
		return BSP_ERROR;

	/* Set eSPI BRG clock source */
	ul32spibrg = ul32freqSystemBus / 2;
	u32div16 = 0;
	if ((ul32spibrg / u32max_hz) > 32) 
	{
		u32div16 = ESPI_CSMODE_DIV16;
		u8pm = ul32spibrg / (u32max_hz * 16 * 2);
		if (u8pm > 16) 
		{
			u8pm = 16;
			printf("Requested speed is too low: %d Hz, %ld Hz ""is used.\n", u32max_hz, ul32spibrg / (32 * 16));
		}
	} 
	else
	{
		u8pm = ul32spibrg / (u32max_hz * 2);
	}	
	if (u8pm)
	{
		u8pm--;
	}

	#if 1
	/* Enable eSPI interface */
	spi_write_reg(&spi_reg->mode, ESPI_MODE_RXTHR(3)
			| ESPI_MODE_TXTHR(4) | ESPI_MODE_EN);
	spi_write_reg(&spi_reg->event, 0xffffffff); /* Clear all eSPI events */
	spi_write_reg(&spi_reg->mask, 0x00000000); /* Mask  all eSPI interrupts */

	/* Init CS mode interface */
	for (i = 0; i < ESPI_MAX_CS_NUM; i++)
	{
		spi_write_reg(&spi_reg->csmode[i], ESPI_CSMODE_INIT_VAL);
	}
	#endif
	
	spi_write_reg(&spi_reg->csmode[u32cs], spi_read_reg(&spi_reg->csmode[u32cs]) &
		~(ESPI_CSMODE_PM(0xF) | ESPI_CSMODE_DIV16
		| ESPI_CSMODE_CI_INACTIVEHIGH | ESPI_CSMODE_CP_BEGIN_EDGCLK
		| ESPI_CSMODE_REV_MSB_FIRST | ESPI_CSMODE_LEN(0xF)));

	//设置频率
	/* Set eSPI BRG clock source */
	spi_write_reg(&spi_reg->csmode[u32cs], spi_read_reg(&spi_reg->csmode[u32cs])
		| ESPI_CSMODE_PM(u8pm) | u32div16);

	//设置SPI模式
	if (u32mode & SPI_CPHA)
	{		
		spi_write_reg(&spi_reg->csmode[u32cs], spi_read_reg(&spi_reg->csmode[u32cs])
			| ESPI_CSMODE_CP_BEGIN_EDGCLK);
	}
	
	if (u32mode & SPI_CPOL)
	{
		spi_write_reg(&spi_reg->csmode[u32cs], spi_read_reg(&spi_reg->csmode[u32cs])
			| ESPI_CSMODE_CI_INACTIVEHIGH);
	}
	
	/* Character bit order: msb first */
	spi_write_reg(&spi_reg->csmode[u32cs], spi_read_reg(&spi_reg->csmode[u32cs])
		| ESPI_CSMODE_REV_MSB_FIRST);
	/* Character length in bits, between 0x3~0xf, i.e. 4bits~16bits */
	spi_write_reg(&spi_reg->csmode[u32cs], spi_read_reg(&spi_reg->csmode[u32cs])
		| ESPI_CSMODE_LEN(7));
	
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_spi_init							
* 功    能:  初始化SPI                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
void bsp_spi_init(void)
{
	s32 i;
	printf("bsp_spi_init!\n");
	printf("g_u8ccsbar = %x\n",g_u8ccsbar);
	spi_reg	 = (struct fsl_espi_reg *)(g_u8ccsbar + 0x110000);  //定义寄存器地址
	printf("spi_reg = %x\n",spi_reg);
	pthread_mutex_init(&g_spi_lock, NULL);
	#if 0
	/* Enable eSPI interface */
	spi_write_reg(&spi_reg->mode, ESPI_MODE_RXTHR(3)
			| ESPI_MODE_TXTHR(4) | ESPI_MODE_EN);
	spi_write_reg(&spi_reg->event, 0xffffffff); /* Clear all eSPI events */
	spi_write_reg(&spi_reg->mask, 0x00000000); /* Mask  all eSPI interrupts */

	/* Init CS mode interface */
	for (i = 0; i < ESPI_MAX_CS_NUM; i++)
	{
		spi_write_reg(&spi_reg->csmode[i], ESPI_CSMODE_INIT_VAL);
	}
	#endif
}

/********************************************************************************
* 函数名称: spi_cs_activate							
* 功    能: 设置片选和数据长度                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
Private void spi_cs_activate(u32 u32cs,u32 u32tran_len)
{
	u32 u32com = 0;
	u32 u32data_len = u32tran_len;

	u32com &= ~(ESPI_COM_CS(0x3) | ESPI_COM_TRANLEN(0xFFFF));
	u32com |= ESPI_COM_CS(u32cs);                //片选
	u32com |= ESPI_COM_TRANLEN(u32data_len - 1);//设定数据长度
	spi_write_reg(&spi_reg->command, u32com);
}

/********************************************************************************
* 函数名称: spi_cs_activate							
* 功    能: 复位SPI，清零计数器                                  
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
Private void spi_cs_deactivate(void)
{
	/* clear the RXCNT and TXCNT */
	spi_write_reg(&spi_reg->mode, spi_read_reg(&spi_reg->mode) & (~ESPI_MODE_EN));
	spi_write_reg(&spi_reg->mode, spi_read_reg(&spi_reg->mode) | ESPI_MODE_EN);
}

/********************************************************************************
* 函数名称: spi_write							
* 功    能: SPI数据传输                                    
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
			u32cs:片选
			bitlen:bit长度
			pdata_out:写指针
			pdata_in:读指针
			flags:控制标志位
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 spi_write(u32 u32cs,u32 s32len, const void *pdata_out)
{
	u32 u32tmpdout, u32tmpdin, u32event;
	s32 s32num_blks, s32num_chunks, s32max_tran_len, s32tran_len;
	u8 *u8pch;
	u8 *u8pbuffer = NULL;
	u8 *dout = NULL;
	s32 s32data_len = s32len;

	s32max_tran_len = ESPI_MAX_DATA_TRANSFER_LEN;

         const s32 length = s32len;
         u8 u8buffer[length];
         u8pbuffer = u8buffer;
         #if 0
	u8pbuffer = (u8 *)malloc(s32len);
	if (!u8pbuffer) 
	{
		printf("SF: Failed to malloc memory.\n");
		return BSP_ERROR;
	}
        #endif
	memset(u8pbuffer,0,s32len);
	memcpy(u8pbuffer,pdata_out,s32len);
    dout = u8pbuffer;
	s32num_chunks = s32data_len / s32max_tran_len +(s32data_len % s32max_tran_len ? 1 : 0);
	while (s32num_chunks--) 
	{
		s32tran_len = min(s32data_len,s32max_tran_len);
		if(s32tran_len == s32max_tran_len)
			s32data_len -= s32max_tran_len;
		s32num_blks = (s32tran_len)/ 4 +((s32tran_len)% 4 ? 1 : 0);
	    //printf("%x,%x,%x\n",s32tran_len,s32num_blks,s32data_len);
		
		spi_cs_activate(u32cs,s32tran_len);  //片选，长度

		/* Clear all eSPI events */
		spi_write_reg(&spi_reg->event,0xffffffff);
		
		/* handle data in 32-bit chunks */
		while (s32num_blks--) 
		{
			u32event = spi_read_reg(&spi_reg->event);
			if (u32event & ESPI_EV_TNF)  //The transmitter FIFO is not full.
			{
				u32tmpdout = *(u32 *)dout;

				/* Set up the next iteration */
				if (s32len > 4) 
				{
					s32len -= 4;
					dout += 4;
				}
				//printf("%x\n",*(u32 *)u8pbuffer);

				spi_write_reg(&spi_reg->tx, u32tmpdout);
				spi_write_reg(&spi_reg->event, ESPI_EV_TNF);
				//printf("***spi_xfer:...%08x written\n", u32tmpdout);
			}
			/* Wait for eSPI transmit to get out */
			bsp_sys_usdelay(80);			
		}
		spi_cs_deactivate();
	}
	#if 0
	free(u8pbuffer);
         #endif
	return BSP_OK;
}

/********************************************************************************
* 函数名称: spi_read							
* 功    能: SPI数据传输                                    
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
			u32cs:片选
			bitlen:bit长度
			pdata_out:写指针
			pdata_in:读指针
			flags:控制标志位
* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
*********************************************************************************/
s32 spi_read(u8 *cmd,u8 cmd_len,u32 u32cs,s32 s32len,u8 *pdata_in)
{
	u32 u32tmpdout, u32tmpdin, u32event;
	s32 s32num_blks, s32num_chunks, s32max_tran_len, s32tran_len,s32cmd_blks,s32cmd_bytes;
	s32 s32num_bytes;
	u8 *u8pch;
	void *pdin;
	u8 *u8pbuffer = NULL;
    s32 s32data_len = s32len;
		
	s32max_tran_len = ESPI_MAX_DATA_TRANSFER_LEN;

    const s32 length = s32len;
    u8 u8buffer[length];
    u8pbuffer = u8buffer;
#if 0
	u8pbuffer = (u8 *)malloc(s32len);
	if (!u8pbuffer) 
	{
		printf("SF: Failed to malloc memory.\n");
		return BSP_ERROR;
	}
#endif
	memset(u8pbuffer,0,s32len);
	pdin = u8pbuffer;

	s32num_chunks = s32data_len / s32max_tran_len +(s32data_len % s32max_tran_len ? 1 : 0);

	while (s32num_chunks--) 
	{		
		s32tran_len = min(s32data_len,s32max_tran_len);
		if(s32tran_len ==s32max_tran_len)
			s32data_len -= s32max_tran_len;
		s32num_blks = (s32tran_len) / 4 +((s32tran_len) % 4 ? 1 : 0);
		s32num_bytes = (s32tran_len) % 4;
	    //printf("%x,%x,%x\n",s32tran_len,s32num_blks,s32data_len);
		
		spi_cs_activate(u32cs,cmd_len+s32tran_len);  //片选，长度限定了时钟的个数

		/* Clear all eSPI events */
		spi_write_reg(&spi_reg->event,0xffffffff);
		
		u32tmpdout = *(u32 *)cmd;
		//printf("%x\n",*(u32 *)cmd);
		u32event = spi_read_reg(&spi_reg->event);
		if (u32event & ESPI_EV_TNF)  //The transmitter FIFO is not full.
		{
		    //写命令，数据出时同样有数据入，可能在这有问题，能收到数据
			spi_write_reg(&spi_reg->tx, u32tmpdout); 
			spi_write_reg(&spi_reg->event, ESPI_EV_TNF);
			//printf("***spi_xfer:...%08x written\n", u32tmpdout);
		}
		
		/* Wait for eSPI transmit to get out */
		bsp_sys_usdelay(80);
	
		{
			u32event = spi_read_reg(&spi_reg->event);

			if (u32event & ESPI_EV_RNE)   //The Rx FIFO has a received character  读回一个字节
			{
				u32tmpdin = spi_read_reg(&spi_reg->rx);
				spi_write_reg(&spi_reg->event, spi_read_reg(&spi_reg->event)| ESPI_EV_RNE);
				//printf("***spi_xfer1:...%x readed\n", u32tmpdin);
			}	

		}
		
		/* Clear all eSPI events */
		spi_write_reg(&spi_reg->event,0xffffffff);

		/* handle data in 32-bit chunks */  
		while (s32num_blks--) 
		{
	        u32event = spi_read_reg(&spi_reg->event);
			if (u32event & ESPI_EV_TNF)  //The transmitter FIFO is not full.
			{
			    /*读的时候写的数时任意的，主要是为了主动产生时钟信号，由从设备给主设备发数*/
			    u32tmpdout = 0x00000000;  
				spi_write_reg(&spi_reg->tx,u32tmpdout);
				spi_write_reg(&spi_reg->event, ESPI_EV_TNF);
				//printf("***spi_xfer:...%08x written\n", u32tmpdout);
			}

			bsp_sys_usdelay(80);

			//spi_cs_activate(u32cs,s32tran_len);  //片选，长度
			
			u32event = spi_read_reg(&spi_reg->event);

			if (u32event & ESPI_EV_RNE)   //The Rx FIFO has a received character  读回一个字节
			{
				u32tmpdin = spi_read_reg(&spi_reg->rx);
				if (s32num_blks == 0 && s32num_bytes != 0) 
				{
					u8pch = (u8 *)&u32tmpdin;
					while (s32num_bytes--)
						*(u8 *)pdin++ = *u8pch++;
				} 
				else 
				{
					*(u32 *) pdin = u32tmpdin;
					pdin += 4;
				}
				spi_write_reg(&spi_reg->event, spi_read_reg(&spi_reg->event)| ESPI_EV_RNE);
				//printf("***spi_xfer2:...%x readed\n", u32tmpdin);
			}	
		}
		
	   	//bsp_sys_usdelay(100);
		spi_cs_deactivate();
	}
	memcpy(pdata_in,u8pbuffer,s32len);   
    #if 0
	free(u8pbuffer);
    #endif
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_spi_write							
* 功    能: SPI写接口                         
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
			u32cs：片选
			u8pwrite_data：写数据指针
			u32data_len：数据长度
* 说   明: 
*********************************************************************************/
s32 bsp_spi_read(u32 u32cs,u8 *u8pread_data, u32 u32data_len)
{
	/*if(spi_read(u32cs,u32data_len, u8pread_data) != 0) 
	{
		printf("Error during SPI transaction\n");
		return BSP_ERROR;
	} */
	return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_spi_write							
* 功    能: SPI写接口                         
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
			u32cs：片选
			u8pwrite_data：写数据指针
			u32data_len：数据长度
* 说   明: 
*********************************************************************************/
s32 bsp_spi_write(u32 u32cs, u8 *u8pwrite_data, u32 u32data_len)
{	
	if(spi_write(u32cs,u32data_len,u8pwrite_data) != 0) 
	{
		printf("Error during SPI transaction\n");
		return BSP_ERROR;
	} 
	return BSP_OK;
}

/********************************************************************************
* 函数名称: spi_dac_write							
* 功    能: D/A输出操作，24 bits                              
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
s32 spi_dac_write(u8 u8_mod,u16 u16da_data)
{
	u8 u8da_data[3]={0};

	pthread_mutex_lock(&g_spi_lock);
	if (BSP_OK !=bsp_spi_setup_slave(SPI_CS0,1000000,SPI_MODE_1)) 
	{
		printf("Invalid device.\n");
		pthread_mutex_unlock(&g_spi_lock);
		return BSP_ERROR;
	}
    u8da_data[0] =  u8_mod;
	u8da_data[1] =  (u16da_data >> 8)&0xff;
	u8da_data[2] =  u16da_data&0xff;
	
	if(bsp_spi_write(SPI_CS0,(u8 *)u8da_data,3))
	{
		printf("dac write wrong!\n");
		pthread_mutex_unlock(&g_spi_lock);
		return BSP_ERROR;
	}
		pthread_mutex_unlock(&g_spi_lock);
	return BSP_OK;
}


/********************************************************************************
* 函数名称: spi_cpld_write							
* 功    能: cpld SPI写                             
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
static int lock_spi_bus = 0;
int bsp_spi_lock(int lock){
    lock_spi_bus = lock;
    if(lock_spi_bus==0){
        pthread_mutex_unlock(&g_spi_lock);
    }
    return 0;
}

s32 spi_cpld_write(u8 *u8cpld_data,u32 len)
{
	pthread_mutex_lock(&g_spi_lock);
	if (BSP_OK !=bsp_spi_setup_slave(SPI_CS2,1000000,SPI_MODE_0)) 
	{
		printf("Invalid device.\n");
		pthread_mutex_unlock(&g_spi_lock);
		return BSP_ERROR;
	}

	if(bsp_spi_write(SPI_CS2,u8cpld_data,len))
	{
		printf("cpld write wrong!\n");
		pthread_mutex_unlock(&g_spi_lock);
		return BSP_ERROR;
	}
    if(lock_spi_bus==0){
		pthread_mutex_unlock(&g_spi_lock);
	}
	return BSP_OK;
}

/********************************************************************************
* 函数名称: spi_cpld_read							
* 功    能:                       
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
s32 spi_cpld_read(u8 *cmd,u8 cmd_len,u8 *u8cpld_data,u32 len)
{
	pthread_mutex_lock(&g_spi_lock);
	if (BSP_OK !=bsp_spi_setup_slave(SPI_CS2,1000000,SPI_MODE_0)) 
	{
		printf("Invalid device.\n");
		pthread_mutex_unlock(&g_spi_lock);
		return BSP_ERROR;
	}
	if(BSP_OK != spi_read(cmd,cmd_len,SPI_CS2,len,u8cpld_data))
	{
		printf("cpld write wrong!\n");
		pthread_mutex_unlock(&g_spi_lock);
		return BSP_ERROR;
	}
	pthread_mutex_unlock(&g_spi_lock);
	return BSP_OK;
}


/***********************************测试部分*************************************/
/********************************************************************************
* 函数名称: spi_test						
* 功    能: 测试函数，片选，通道属性                       
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/
s32 spi_dac_test(void)
{
	//bsp_spi_init();	
	spi_dac_write(0,0x55aa);
	spi_dac_write(0,0x5a5a);
	spi_dac_write(0,0x3344);	
	return BSP_OK;
}

