/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_spi.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/


/* SPI操作相关宏定义 */
#define SPI_NORMAL_WRITE_CMD           0x61     /* 以NORMAL方式写寄存器的CMD值 */
#define SPI_NORMAL_READ_CMD            0x60     /* 以NORMAL方式读寄存器的CMD值 */
#define SPI_FAST_READ_CMD              0x10     /* 以FAST方式读寄存器的CMD值 */


/* SPI轮询次数超时时限 */
#define SPI_DEFAULT_TIMEOUT            50
/* SPI轮询时等待的时间间隔 */
#define SPI_POLL_DELAY_US              200

#define SPI_CLK_WIDTH                  20    /* SPI时钟宽度(单位为us),SPI时钟的典型宽度值为500ns */

s32 ethsw_read_reg(u8 u8RegPage, u8 u8RegAddr, u8 *pu8Buf, u8 u8Len);
s32 ethsw_write_reg(u8 u8RegPage, u8 u8RegAddr, const u8 *pu8Buf, u8 u8Len);

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

/* SPI transfer flags */
#define SPI_XFER_BEGIN	0x01			/* Assert CS before transfer */
#define SPI_XFER_END	0x02			/* Deassert CS after transfer */

/* SPI mode flags */
#define	SPI_CPHA	0x01			/* clock phase */
#define	SPI_CPOL	0x02			/* clock polarity */

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif /* min */

/******************************* 头文件结束 ***********************************/
