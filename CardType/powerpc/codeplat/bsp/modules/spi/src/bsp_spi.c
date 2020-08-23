/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           bsp_spi.c 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
/************************** �����ļ����� **********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "fcntl.h"
#include "unistd.h"

/**************************** ˽��ͷ�ļ�* **********************************/
#include "../../../com_inc/bsp_types.h"
#include "../inc/bsp_spi.h"
#include "../../../com_inc/fsl_p2041_ext.h"
#include "../../../com_inc/bsp_i2c_ext.h"   //test
/******************************* �ֲ��궨�� *********************************/
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
/*********************** ȫ�ֱ�������/��ʼ�� **************************/

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

ul32 ul32freqSystemBus = 600000000;//ϵͳʱ��Ƶ�ʣ���Ҫ�ⲿ���

pthread_mutex_t  g_spi_lock = PTHREAD_MUTEX_INITIALIZER;  /* ����SPI����*/

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif /* min */

/************************** �ֲ����������Ͷ��� ************************/


/*************************** �ֲ�����ԭ������ **************************/

extern void bsp_sys_msdelay(ul32 dwTimeOut);// ---�ӳٵ�λΪ����
extern void bsp_sys_usdelay(ul32 dwTimeOut); //---�ӳٵ�λΪ΢��

/***************************����ʵ�� *****************************/
/********************************************************************************
* ��������: spi_write_reg						
* ��    ��: ���Ժ�����Ƭѡ��ͨ������                       
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
*********************************************************************************/
Private void spi_write_reg(u32 *u32preg, u32 u32val)
{
	*u32preg = u32val;
}

/********************************************************************************
* ��������: spi_read_reg						
* ��    ��: ���Ժ�����Ƭѡ��ͨ������                       
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
*********************************************************************************/
Private u32 spi_read_reg(u32 *u32preg)
{
	return *u32preg;
}

/********************************************************************************
* ��������: spi_setup_slave							
* ��    ��: ���ø���ͨ��������                                  
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
	u32max_hz :ʱ��Ƶ��
	u32mode   :ģʽ   �ɹ̶�   
	        SPI_CPHA	0x01		clock phase 
            SPI_CPOL	0x02		clock polarity 
* ˵   ��: 
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

	//����Ƶ��
	/* Set eSPI BRG clock source */
	spi_write_reg(&spi_reg->csmode[u32cs], spi_read_reg(&spi_reg->csmode[u32cs])
		| ESPI_CSMODE_PM(u8pm) | u32div16);

	//����SPIģʽ
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
* ��������: bsp_spi_init							
* ��    ��:  ��ʼ��SPI                                   
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
*********************************************************************************/
void bsp_spi_init(void)
{
	s32 i;
	printf("bsp_spi_init!\n");
	printf("g_u8ccsbar = %x\n",g_u8ccsbar);
	spi_reg	 = (struct fsl_espi_reg *)(g_u8ccsbar + 0x110000);  //����Ĵ�����ַ
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
* ��������: spi_cs_activate							
* ��    ��: ����Ƭѡ�����ݳ���                                   
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
*********************************************************************************/
Private void spi_cs_activate(u32 u32cs,u32 u32tran_len)
{
	u32 u32com = 0;
	u32 u32data_len = u32tran_len;

	u32com &= ~(ESPI_COM_CS(0x3) | ESPI_COM_TRANLEN(0xFFFF));
	u32com |= ESPI_COM_CS(u32cs);                //Ƭѡ
	u32com |= ESPI_COM_TRANLEN(u32data_len - 1);//�趨���ݳ���
	spi_write_reg(&spi_reg->command, u32com);
}

/********************************************************************************
* ��������: spi_cs_activate							
* ��    ��: ��λSPI�����������                                  
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
*********************************************************************************/
Private void spi_cs_deactivate(void)
{
	/* clear the RXCNT and TXCNT */
	spi_write_reg(&spi_reg->mode, spi_read_reg(&spi_reg->mode) & (~ESPI_MODE_EN));
	spi_write_reg(&spi_reg->mode, spi_read_reg(&spi_reg->mode) | ESPI_MODE_EN);
}

/********************************************************************************
* ��������: spi_write							
* ��    ��: SPI���ݴ���                                    
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
			u32cs:Ƭѡ
			bitlen:bit����
			pdata_out:дָ��
			pdata_in:��ָ��
			flags:���Ʊ�־λ
* ����ֵ: 0 ��ʾ�ɹ�������ֵ��ʾʧ�ܡ�								
* ˵   ��: 
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
		
		spi_cs_activate(u32cs,s32tran_len);  //Ƭѡ������

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
* ��������: spi_read							
* ��    ��: SPI���ݴ���                                    
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
			u32cs:Ƭѡ
			bitlen:bit����
			pdata_out:дָ��
			pdata_in:��ָ��
			flags:���Ʊ�־λ
* ����ֵ: 0 ��ʾ�ɹ�������ֵ��ʾʧ�ܡ�								
* ˵   ��: 
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
		
		spi_cs_activate(u32cs,cmd_len+s32tran_len);  //Ƭѡ�������޶���ʱ�ӵĸ���

		/* Clear all eSPI events */
		spi_write_reg(&spi_reg->event,0xffffffff);
		
		u32tmpdout = *(u32 *)cmd;
		//printf("%x\n",*(u32 *)cmd);
		u32event = spi_read_reg(&spi_reg->event);
		if (u32event & ESPI_EV_TNF)  //The transmitter FIFO is not full.
		{
		    //д������ݳ�ʱͬ���������룬�������������⣬���յ�����
			spi_write_reg(&spi_reg->tx, u32tmpdout); 
			spi_write_reg(&spi_reg->event, ESPI_EV_TNF);
			//printf("***spi_xfer:...%08x written\n", u32tmpdout);
		}
		
		/* Wait for eSPI transmit to get out */
		bsp_sys_usdelay(80);
	
		{
			u32event = spi_read_reg(&spi_reg->event);

			if (u32event & ESPI_EV_RNE)   //The Rx FIFO has a received character  ����һ���ֽ�
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
			    /*����ʱ��д����ʱ����ģ���Ҫ��Ϊ����������ʱ���źţ��ɴ��豸�����豸����*/
			    u32tmpdout = 0x00000000;  
				spi_write_reg(&spi_reg->tx,u32tmpdout);
				spi_write_reg(&spi_reg->event, ESPI_EV_TNF);
				//printf("***spi_xfer:...%08x written\n", u32tmpdout);
			}

			bsp_sys_usdelay(80);

			//spi_cs_activate(u32cs,s32tran_len);  //Ƭѡ������
			
			u32event = spi_read_reg(&spi_reg->event);

			if (u32event & ESPI_EV_RNE)   //The Rx FIFO has a received character  ����һ���ֽ�
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
* ��������: bsp_spi_write							
* ��    ��: SPIд�ӿ�                         
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
			u32cs��Ƭѡ
			u8pwrite_data��д����ָ��
			u32data_len�����ݳ���
* ˵   ��: 
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
* ��������: bsp_spi_write							
* ��    ��: SPIд�ӿ�                         
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
			u32cs��Ƭѡ
			u8pwrite_data��д����ָ��
			u32data_len�����ݳ���
* ˵   ��: 
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
* ��������: spi_dac_write							
* ��    ��: D/A���������24 bits                              
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
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
* ��������: spi_cpld_write							
* ��    ��: cpld SPIд                             
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
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
* ��������: spi_cpld_read							
* ��    ��:                       
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
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


/***********************************���Բ���*************************************/
/********************************************************************************
* ��������: spi_test						
* ��    ��: ���Ժ�����Ƭѡ��ͨ������                       
* ����ĵ�:                    
* ��������:									
* ��    ��: 						     			
* ��������		   ����					����/��� 		����	
* ˵   ��: 
*********************************************************************************/
s32 spi_dac_test(void)
{
	//bsp_spi_init();	
	spi_dac_write(0,0x55aa);
	spi_dac_write(0,0x5a5a);
	spi_dac_write(0,0x3344);	
	return BSP_OK;
}

