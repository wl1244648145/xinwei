/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_spi.c 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
/************************** �����ļ����� **********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>

/**************************** ˽��ͷ�ļ�* **********************************/
#include "bsp_types.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../inc/bsp_ethsw_bcm5389.h"
#include "../inc/bsp_ethsw_spi.h"
//#include "bsp_spi_ext.h"
#include "fsl_p2041_ext.h"
#include <pthread.h>

/******************************* �ֲ��궨�� *********************************/


/*********************** ȫ�ֱ�������/��ʼ�� **************************/
/* SPI�ź�ӳ���ϵȫ�ֱ��� */
s16 g_s16CurRegPage;
extern u32 g_u32PrintRules;
//extern s32 bsp_spi_read(u32 cs,u8 *read_data, u32 data_len);
//extern s32 bsp_spi_write(u32 cs, u8 *write_data, u32 data_len);
struct fsl_espi_reg *ethsw_spi_reg;
u8 g_u8swtest[20] = {0};
extern pthread_mutex_t  g_spi_lock;  /* ����spi���� */
/************************** �ֲ����������Ͷ��� ************************/



/*************************** �ֲ�����ԭ������ **************************/

/************************************ ����ʵ�� ************************* ****/

u32 spi_read_reg(u32 *u32preg)
{
	return *u32preg;
}
 
void spi_write_reg(u32 *u32preg, u32 u32val)
{
	*u32preg = u32val;
}

void bsp_ethsw_spi_init(void)
{
	s32 i;
	ethsw_spi_reg	 = (struct fsl_espi_reg *)(g_u8ccsbar + 0x110000);  //����Ĵ�����ַ

		/* Enable eSPI interface */
	spi_write_reg(&ethsw_spi_reg->mode, ESPI_MODE_RXTHR(3)
			| ESPI_MODE_TXTHR(4) | ESPI_MODE_EN);
	spi_write_reg(&ethsw_spi_reg->event, 0xffffffff); /* Clear all eSPI events */
	spi_write_reg(&ethsw_spi_reg->mask, 0x00000000); /* Mask  all eSPI interrupts */

	/* Init CS mode interface */
	for (i = 0; i < ESPI_MAX_CS_NUM; i++)
	{
		spi_write_reg(&ethsw_spi_reg->csmode[i], ESPI_CSMODE_INIT_VAL);
	}
}

s32 bsp_ethsw_spi_setup(u32 u32cs,u32 u32max_hz,u32 u32mode)
{
	ul32 ul32spibrg = 0;
	u8 u8pm = 0;
	u32  u32div16;
	s32 i;
	u32 u32temp;
	if (u32cs >= ESPI_MAX_CS_NUM)
		return BSP_ERROR;

	/* Set eSPI BRG clock source */
	ul32spibrg = 600000000 / 2;
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
	spi_write_reg(&ethsw_spi_reg->mode, ESPI_MODE_RXTHR(3)
			| ESPI_MODE_TXTHR(4) | ESPI_MODE_EN);
	spi_write_reg(&ethsw_spi_reg->event, 0xffffffff); /* Clear all eSPI events */
	spi_write_reg(&ethsw_spi_reg->mask, 0x00000000); /* Mask  all eSPI interrupts */

	/* Init CS mode interface */
	for (i = 0; i < ESPI_MAX_CS_NUM; i++)
	{
		spi_write_reg(&ethsw_spi_reg->csmode[i], ESPI_CSMODE_INIT_VAL);
	}
	#endif
	
	spi_write_reg(&ethsw_spi_reg->csmode[u32cs], spi_read_reg(&ethsw_spi_reg->csmode[u32cs]) &
		~(ESPI_CSMODE_PM(0xF) | ESPI_CSMODE_DIV16
		| ESPI_CSMODE_CI_INACTIVEHIGH | ESPI_CSMODE_CP_BEGIN_EDGCLK
		| ESPI_CSMODE_REV_MSB_FIRST | ESPI_CSMODE_LEN(0x8)));

	//����Ƶ��
	/* Set eSPI BRG clock source */
	spi_write_reg(&ethsw_spi_reg->csmode[u32cs], spi_read_reg(&ethsw_spi_reg->csmode[u32cs])
		| ESPI_CSMODE_PM(u8pm) | u32div16);

	//����SPIģʽ
	if (u32mode & SPI_CPHA)
	{		
		spi_write_reg(&ethsw_spi_reg->csmode[u32cs], spi_read_reg(&ethsw_spi_reg->csmode[u32cs])
			| ESPI_CSMODE_CP_BEGIN_EDGCLK);
	}
	
	if (u32mode & SPI_CPOL)
	{
		spi_write_reg(&ethsw_spi_reg->csmode[u32cs], spi_read_reg(&ethsw_spi_reg->csmode[u32cs])
			| ESPI_CSMODE_CI_INACTIVEHIGH);
	}
	/* Character bit order: msb first */
	spi_write_reg(&ethsw_spi_reg->csmode[u32cs], spi_read_reg(&ethsw_spi_reg->csmode[u32cs])
		| ESPI_CSMODE_REV_MSB_FIRST);
	
	/* Character length in bits, between 0x3~0xf, i.e. 4bits~16bits */
	spi_write_reg(&ethsw_spi_reg->csmode[u32cs], spi_read_reg(&ethsw_spi_reg->csmode[u32cs])
		| ESPI_CSMODE_LEN(0x7));
	u32temp = spi_read_reg(&ethsw_spi_reg->csmode[u32cs]);
	spi_write_reg(&ethsw_spi_reg->csmode[u32cs], 0x74171108);

	return BSP_OK;
}

Private void spi_cs_deactivate(void)
{
	/* clear the RXCNT and TXCNT */
	spi_write_reg(&ethsw_spi_reg->mode, spi_read_reg(&ethsw_spi_reg->mode) & (~ESPI_MODE_EN));
	spi_write_reg(&ethsw_spi_reg->mode, spi_read_reg(&ethsw_spi_reg->mode) | ESPI_MODE_EN);
}

Private void spi_cs_activate(u32 u32cs,u32 u32tran_len)
{
	u32 u32com = 0;
	u32 u32data_len = u32tran_len;

	u32com &= ~(ESPI_COM_CS(0x3) | ESPI_COM_TRANLEN(0xFFFF));
	u32com |= ESPI_COM_CS(u32cs);                //Ƭѡ
	u32com |= ESPI_COM_TRANLEN(u32data_len - 1);//�趨���ݳ���
	spi_write_reg(&ethsw_spi_reg->command, u32com);
}

int ethsw_spi_xfer(const void *pdata_out, u32 u32data_len)
{
	unsigned int u32tmpdout, u32tmpdin, u32event;
	const void *dout = NULL;
	int len = 0;
	int num_blks, num_chunks, max_tran_len, tran_len;
	unsigned char *buffer = NULL;
	size_t data_len = u32data_len;

	max_tran_len = ESPI_MAX_DATA_TRANSFER_LEN;

		len = data_len;
        const int length = len*2;
        unsigned char buf[length];
        buffer = buf;
        #if 0
		buffer = (unsigned char *)malloc(len * 2);
        #endif
		memset(buffer, 0, len * 2);
#if 0
		if (!buffer) {
			printf("SF: Failed to malloc memory.\n");
			return 1;
		}
#endif
		memcpy(buffer, pdata_out, data_len);

	num_chunks = data_len / max_tran_len +
		(data_len % max_tran_len ? 1 : 0);

	while (num_chunks--) {
		dout = buffer;
		tran_len = min(data_len , max_tran_len);
		num_blks = tran_len / 4 +
			(tran_len % 4 ? 1 : 0);

		spi_cs_activate(1,tran_len); 

		spi_write_reg(&ethsw_spi_reg->event , 0xffffffff);

		/* handle data in 32-bit chunks */
		while (num_blks--) {
			u32event = spi_read_reg(&ethsw_spi_reg->event);

			if (u32event & ESPI_EV_TNF) {
				u32tmpdout = *(u32 *)dout;

				/* Set up the next iteration */
				if (len > 4) {
					len -= 4;
					dout += 4;
				}
				spi_write_reg(&ethsw_spi_reg->tx, u32tmpdout);
				spi_write_reg(&ethsw_spi_reg->event, ESPI_EV_TNF);
			}
             ETHSW_DELAY(100);
			u32event = spi_read_reg(&ethsw_spi_reg->event);
			if (u32event & ESPI_EV_RNE) {
				u32tmpdin = spi_read_reg(&ethsw_spi_reg->rx);
				spi_write_reg(&ethsw_spi_reg->event, spi_read_reg(&ethsw_spi_reg->event)| ESPI_EV_RNE);
			}
		}
		spi_cs_deactivate();
	}
    #if 0
	free(buffer);
    #endif
	return 0;
}

static s32 ethsw_spi_poll_rw_done(void)
{
    u8   u8Write[2] = {0};
    u8 u8read[4];
    u32  u32i;
    unsigned char *buffer = NULL;
    const void *dout = NULL;
    unsigned int u32tmpdout, u32tmpdin, u32event;
    unsigned char *ch;
    /* Timeout after 50 tries without MDIO_START low */
	for (u32i = 0; u32i < SPI_DEFAULT_TIMEOUT; u32i++)
	{
		spi_cs_activate(1,3); 

		spi_write_reg(&ethsw_spi_reg->event , 0xffffffff);

		u32event = spi_read_reg(&ethsw_spi_reg->event);

		if (u32event & ESPI_EV_TNF) 
		{
			u32tmpdout = 0x10fe0000;
			spi_write_reg(&ethsw_spi_reg->tx, u32tmpdout);
			spi_write_reg(&ethsw_spi_reg->event, ESPI_EV_TNF);
		}
		/* Wait for eSPI transmit to get out */
	    ETHSW_DELAY(100);
		
		u32event = spi_read_reg(&ethsw_spi_reg->event);
		if (u32event & ESPI_EV_RNE) 
		{
			u32tmpdin = spi_read_reg(&ethsw_spi_reg->rx);

			spi_write_reg(&ethsw_spi_reg->event, spi_read_reg(&ethsw_spi_reg->event) | ESPI_EV_RNE);

		}
		memcpy((unsigned char *)u8read, (unsigned char *)&u32tmpdin, 4);
	   if (0x80 == u8read[2])
	    {
	        break;
	    }
	    else
	    {
    		spi_cs_deactivate();
	        ETHSW_DELAY(SPI_POLL_DELAY_US);
	    }
    }
	if (u32i >= SPI_DEFAULT_TIMEOUT)
    {
        printf("\nethsw_spi_poll_rw_done:timeout, SPI_STATUS is <0x%02x>!\n", u8read[2]);
        return (E_ETHSW_TIMEOUT);
    }
	spi_cs_deactivate();
    return (E_ETHSW_OK);
}

static s32 ethsw_spi_select_page(u8 u8RegPage)
{
     u8   u8Temp[4] = {0};
    unsigned int event,tmpdout;

    if ((s16)u8RegPage == g_s16CurRegPage)
    {
        return (E_ETHSW_OK);
    }
    /* ������SPI���豸(����оƬ)�ĵ�ǰ����ҳ */
    g_s16CurRegPage = (s16)u8RegPage;

    u8Temp[0] = SPI_NORMAL_WRITE_CMD;
    u8Temp[1] = ETHSW_SPI_ADDR;
    u8Temp[2] = u8RegPage;
    memcpy((unsigned char *)&tmpdout, (unsigned char *)u8Temp, 4);
    spi_cs_activate(1,3); 
	spi_write_reg(&ethsw_spi_reg->event , 0xffffffff);
	event = spi_read_reg(&ethsw_spi_reg->event);
	if (event & ESPI_EV_TNF)
	{
		spi_write_reg(&ethsw_spi_reg->tx, tmpdout);
		spi_write_reg(&ethsw_spi_reg->event, ESPI_EV_TNF);
	}
    ETHSW_DELAY(50);
    spi_cs_deactivate();
    ETHSW_DELAY(100);
    return (E_ETHSW_OK);
}

static int ethsw_spi_poll_spif()
{
    u8   u8Write[4] = {0};
    u8 u8read[4];
    u32  u32i;
    unsigned char *buffer = NULL;
    const void *dout = NULL;
    unsigned int u32tmpdout, u32tmpdin, u32event;
    unsigned char *ch;

    for (u32i = 0; u32i < SPI_DEFAULT_TIMEOUT; u32i++)
    {
        u8Write[0] = SPI_NORMAL_READ_CMD;
        u8Write[1] = 0xfe;
        memcpy((unsigned char *)&u32tmpdout, (unsigned char *)u8Write, 4);
        spi_cs_activate(1,3); 
    	spi_write_reg(&ethsw_spi_reg->event , 0xffffffff);
	u32event = spi_read_reg(&ethsw_spi_reg->event);
	if (u32event & ESPI_EV_TNF)
	{
		spi_write_reg(&ethsw_spi_reg->tx, u32tmpdout);
		spi_write_reg(&ethsw_spi_reg->event, ESPI_EV_TNF);
	}
    /* Wait for eSPI transmit to get out */
	    ETHSW_DELAY(100);
		
		u32event = spi_read_reg(&ethsw_spi_reg->event);
		if (u32event & ESPI_EV_RNE) 
		{
			u32tmpdin = spi_read_reg(&ethsw_spi_reg->rx);

			spi_write_reg(&ethsw_spi_reg->event, spi_read_reg(&ethsw_spi_reg->event) | ESPI_EV_RNE);

		}
		memcpy((unsigned char *)u8read, (unsigned char *)&u32tmpdin, 4);
	   if (!(u8read[2] & 0x80))
	    {
	        break;
	    }
	    else
	    {
    		spi_cs_deactivate();
	        ETHSW_DELAY(SPI_POLL_DELAY_US);
	    }

        
    }

    if (u32i >= SPI_DEFAULT_TIMEOUT)
    {
        printf("\npoll spif time out,reg=0x%x\n", u8read[2]);
        return (E_ETHSW_TIMEOUT);
    }
	spi_cs_deactivate();
    return (E_ETHSW_OK);    

}
static int ethsw_spi_poll_rack()
{
    u8   u8Write[4] = {0};
    u8 u8read[4];
    u32  u32i;
    unsigned char *buffer = NULL;
    const void *dout = NULL;
    unsigned int u32tmpdout, u32tmpdin, u32event;
    unsigned char *ch;

    for (u32i = 0; u32i < SPI_DEFAULT_TIMEOUT; u32i++)
    {
        u8Write[0] = SPI_NORMAL_READ_CMD;
        u8Write[1] = 0xfe;
        memcpy((unsigned char *)&u32tmpdout, (unsigned char *)u8Write, 4);
        spi_cs_activate(1,3); 
    	spi_write_reg(&ethsw_spi_reg->event , 0xffffffff);
	u32event = spi_read_reg(&ethsw_spi_reg->event);
	if (u32event & ESPI_EV_TNF)
	{
		spi_write_reg(&ethsw_spi_reg->tx, u32tmpdout);
		spi_write_reg(&ethsw_spi_reg->event, ESPI_EV_TNF);
	}
    /* Wait for eSPI transmit to get out */
	    ETHSW_DELAY(100);
		
		u32event = spi_read_reg(&ethsw_spi_reg->event);
		if (u32event & ESPI_EV_RNE) 
		{
			u32tmpdin = spi_read_reg(&ethsw_spi_reg->rx);

			spi_write_reg(&ethsw_spi_reg->event, spi_read_reg(&ethsw_spi_reg->event) | ESPI_EV_RNE);

		}
		memcpy((unsigned char *)u8read, (unsigned char *)&u32tmpdin, 4);
	   if ((u8read[2] & 0x20))
	    {
	        break;
	    }
	    else
	    {
    		spi_cs_deactivate();
	        ETHSW_DELAY(SPI_POLL_DELAY_US);
	    }

        
    }

    if (u32i >= SPI_DEFAULT_TIMEOUT)
    {
        printf("\npoll spif time out,reg=0x%x\n", u8read[2]);
        return (E_ETHSW_TIMEOUT);
    }
	spi_cs_deactivate();
    return (E_ETHSW_OK);    

}

/*******************************************************************************
* ��������: ethsw_read_reg
* ��������: ͨ��SPI�ӿڶ�BCM53xxоƬ�Ĵ���
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8RegPage     u8          ����        page number
* u8RegAddr     u8          ����        ��ַ
* pu8Buf        u8*         ���        ��������ŵ�ַ
* u8Len         u8          ����        Ҫ�����ĳ���
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_write_reg(u8 u8RegPage, u8 u8RegAddr, const u8 *pu8Buf, u8 u8Len)
{
    s32   s32Rv;
    u8 u8Temp[8] = {0};
    u8 u8i;
    
    pthread_mutex_lock(&g_spi_lock);
    if (BSP_OK !=bsp_ethsw_spi_setup(1,1875000,1)) 
    {
        printf("Invalid device.\n");
        pthread_mutex_unlock(&g_spi_lock);
        return BSP_ERROR;
    }
    s32Rv = ethsw_spi_poll_rw_done();
    if (0 != s32Rv)
    {
        pthread_mutex_unlock(&g_spi_lock);
        return (s32Rv);
    }
    ETHSW_DELAY(SPI_CLK_WIDTH);
    s32Rv = ethsw_spi_select_page(u8RegPage);
    if (0 != s32Rv)
    {
        pthread_mutex_unlock(&g_spi_lock);
        return (s32Rv);
    }

    /* delay so uP have sufficient time to prepare data */
    ETHSW_DELAY(SPI_CLK_WIDTH);

    u8Temp[0] = SPI_NORMAL_WRITE_CMD;
    u8Temp[1] = u8RegAddr;
    for(u8i = 0; u8i < u8Len; u8i++)
    {
        u8Temp[2 + u8i] = *(pu8Buf + u8i);
    }
    if(ethsw_spi_xfer(u8Temp, u8Len+2))
    {
        pthread_mutex_unlock(&g_spi_lock);
        return -1;
    }

    ETHSW_DELAY(SPI_CLK_WIDTH);
    pthread_mutex_unlock(&g_spi_lock);
    return (E_ETHSW_OK);
}

s32 ethsw_read_reg(u8 u8RegPage, u8 u8RegAddr, u8 *pu8Buf, u8 u8Len)
{
    unsigned int tmpdout, event;
    unsigned int tmpdin = 0;
    u8 u8Temp[4] = {0};
    u8 u8TempData[4] = {0};
    u8 u8i;
    const void *dout = NULL;
    unsigned char *ch;
    unsigned int num_blks = u8Len / 4 + (u8Len % 4 ? 1 : 0);
    unsigned int num_bytes = u8Len;
    s32 s32Rv;
    //printf("\n num_blks is %d,num_bytes is %d\n",num_blks,num_bytes);
    dout = pu8Buf;
    pthread_mutex_lock(&g_spi_lock);
    bsp_ethsw_spi_init();
    if (BSP_OK !=bsp_ethsw_spi_setup(1,1875000,1)) 
    {
        printf("Invalid device.\n");
        pthread_mutex_unlock(&g_spi_lock);
        return BSP_ERROR;
    }
    //if(ethsw_spi_poll_spif() != E_ETHSW_OK)
    //    return BSP_ERROR;
    (void)ethsw_spi_select_page(u8RegPage);

    for (u8i = 0; u8i < 5; u8i++)
    {
            spi_cs_activate(1,u8Len+3); 

            u8Temp[0] = SPI_FAST_READ_CMD;
            u8Temp[1] = u8RegAddr;
            memcpy((unsigned char *)&tmpdout, (unsigned char *)u8Temp, 4);

            event = spi_read_reg(&ethsw_spi_reg->event);
            if (event & ESPI_EV_TNF) {
            spi_write_reg(&ethsw_spi_reg->tx, tmpdout);
            spi_write_reg(&ethsw_spi_reg->event, ESPI_EV_TNF);
            }
            ETHSW_DELAY(100);
            //if(ethsw_spi_poll_rack()!=E_ETHSW_OK)
            //    return BSP_ERROR; 

		event = spi_read_reg(&ethsw_spi_reg->event);
		if (event & ESPI_EV_RNE) {
			tmpdin = spi_read_reg(&ethsw_spi_reg->rx);
			spi_write_reg(&ethsw_spi_reg->event, spi_read_reg(&ethsw_spi_reg->event)| ESPI_EV_RNE);
		}
	       memcpy((unsigned char *)u8TempData, (unsigned char *)&tmpdin, 4);
		if(0x01 == u8TempData[2])
		{
                    pu8Buf[0] = u8TempData[3];
                    num_bytes --;
                    //		printf("\naaadout is 0x%x\n",*(unsigned char *)dout );        
                    dout ++;
                    break;
		}
		else
		{
    	    spi_cs_deactivate();

		    ETHSW_DELAY(100);
		}
    }
    if(u8i >= 5)
    {
        printf("\nNot received RACK!!\n");
        pthread_mutex_unlock(&g_spi_lock);
        return (E_ETHSW_TIMEOUT);

    }
#if 1
	if(1 == u8Len)
	{
           spi_write_reg(&ethsw_spi_reg->event, 0xffffffff); /* Clear all eSPI events */
	     spi_cs_deactivate();

	    ETHSW_DELAY(100);
//printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");	
        pthread_mutex_unlock(&g_spi_lock);
        return 0;
	}
#endif	
	//else if(u8Len > 1)
	else	
	{
               spi_write_reg(&ethsw_spi_reg->event, 0xffffffff); /* Clear all eSPI events */

	        while(num_blks --)
	        {
			event = spi_read_reg(&ethsw_spi_reg->event);
			if (event & ESPI_EV_TNF) {
				tmpdout = 0x00000000;

				spi_write_reg(&ethsw_spi_reg->tx, tmpdout);
				spi_write_reg(&ethsw_spi_reg->event, ESPI_EV_TNF);
			}
                    ETHSW_DELAY(100);
       		event = spi_read_reg(&ethsw_spi_reg->event);
			if (event & ESPI_EV_RNE) {
				tmpdin = spi_read_reg(&ethsw_spi_reg->rx);
				spi_write_reg(&ethsw_spi_reg->event, spi_read_reg(&ethsw_spi_reg->event)| ESPI_EV_RNE);
			}
		
            if(num_blks == 0 && num_bytes != 0)
            {
                ch = (unsigned char *)&tmpdin;
//		printf("\naaatmpdinis 0x%x\n",tmpdin );        

		        while(num_bytes --)
		        {
		//printf("\ndout is 0x%x, ch 0x%x\n",*(unsigned char *)dout,*ch );        
		        
                    *(unsigned char *)dout ++ = *(ch ++);
		        }
		    }
		    else
		    {
                memcpy((unsigned char *)dout, (unsigned char *)&tmpdin, 4);
//	printf("\ntmpdinis 0x%x\n",tmpdin );        
			
			    dout += 4;
			    num_bytes -= 4;
//		printf("\nnum_bytes is %d\n",num_bytes);		
		    }
	    }
	}
    spi_cs_deactivate();
    ETHSW_DELAY(100);
//printf("\n#####################################\n");	
    pthread_mutex_unlock(&g_spi_lock);
    return (E_ETHSW_OK);
}

void ethsw_read_reg_test(u8 u8RegPage, u8 u8RegAddr, u8 len)
{
    u8 i;
	u8 pbuf[8]; 
	ethsw_read_reg(u8RegPage,u8RegAddr,pbuf,len);
	for(i = 0; i< len; i++)
	{
	    printf("\nreg[%d] = 0x%x\n", i, pbuf[i]);	
	}
}

void ethsw_write_reg_test(u8 u8RegPage, u8 u8RegAddr,u32 regvalue, u8 len)
{
	u8 pbuf[4] = {0}; 
	pbuf[0] = 0xff&(regvalue>>24);
	pbuf[1] = 0xff&(regvalue>>16);	
	pbuf[2] = 0xff&(regvalue>>8);
	pbuf[3] = 0xff&(regvalue);
	ethsw_write_reg(u8RegPage,u8RegAddr,(pbuf+4-len),len);
}
/******************************* Դ�ļ����� ***********************************/
