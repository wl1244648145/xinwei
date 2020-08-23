#ifndef IIC_H_
#define IIC_H_
#include "../../../com_inc/bsp_types.h"
extern unsigned char   *g_u8ccsbar;
//typedef unsigned char UCHAR;
#define CONFIG_SYS_IMMR		    g_u8ccsbar	/* PQII uses CONFIG_SYS_IMMR */
                                   
#define CONFIG_SYS_IIC_OFFSET                (0x0c)
#define CONFIG_SYS_I2C1_OFFSET		(0x118000)
#define CONFIG_SYS_I2C2_OFFSET		(0x118100)
#define CONFIG_SYS_I2C3_OFFSET          (0x119000)
#define CONFIG_SYS_I2C4_OFFSET          (0x119100)


#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0//0x31
#define	GD_FLG_RELOC	0x00001
#define min(x,y) ((x)<(y)?(x):(y))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define P2041_IIC0   (0)
#define P2041_IIC1   (1)
#define P2041_IIC2   (2)
#define P2041_IIC3   (3)

static const struct {
	unsigned short divider;
	UCHAR fdr;
} fsl_i2c_speed_map[] = {
	{20, 32}, {22, 33}, {24, 34}, {26, 35},
	{28, 0}, {28, 36}, {30, 1}, {32, 37},
	{34, 2}, {36, 38}, {40, 3}, {40, 39},
	{44, 4}, {48, 5}, {48, 40}, {56, 6},
	{56, 41}, {64, 42}, {68, 7}, {72, 43},
	{80, 8}, {80, 44}, {88, 9}, {96, 41},
	{104, 10}, {112, 42}, {128, 11}, {128, 43},
	{144, 12}, {160, 13}, {160, 48}, {192, 14},
	{192, 49}, {224, 50}, {240, 15}, {256, 51},
	{288, 16}, {320, 17}, {320, 52}, {384, 18},
	{384, 53}, {448, 54}, {480, 19}, {512, 55},
	{576, 20}, {640, 21}, {640, 56}, {768, 22},
	{768, 57}, {960, 23}, {896, 58}, {1024, 59},
	{1152, 24}, {1280, 25}, {1280, 60}, {1536, 26},
	{1536, 61}, {1792, 62}, {1920, 27}, {2048, 63},
	{2304, 28}, {2560, 29}, {3072, 30}, {3840, 31},
	{-1, 31}
};
#define __iomem

static int in_8(const volatile unsigned char  *addr)
{
	int ret;
#if 1
	__asm__ __volatile__(
		"sync; lbz%U1%X1 %0,%1;\n"
		"twi 0,%0,0;\n"
		"isync" : "=r" (ret) : "m" (*addr));
#endif
//	ret  = *(unsigned char *)(addr);
	return ret;
}

static void out_8(volatile unsigned char  *addr, int val)
{
#if 1
	__asm__ __volatile__("stb%U0%X0 %1,%0; eieio" : "=m" (*addr) : "r" (val));
#endif
//	*(unsigned char *)(addr) = (unsigned char)val;
}

#define readb(addr) in_8((volatile UCHAR *)(addr))
#define writeb(b,addr) out_8((volatile UCHAR *)(addr), (b))


#define I2C_ADR		0xFE
#define I2C_ADR_SHIFT	1
#define I2C_ADR_RES	~(I2C_ADR)

#define IC2_FDR		0x3F
#define IC2_FDR_SHIFT	0
#define IC2_FDR_RES	~(IC2_FDR)

#define I2C_CR_MEN	0x80
#define I2C_CR_MIEN	0x40
#define I2C_CR_MSTA	0x20
#define I2C_CR_MTX	0x10
#define I2C_CR_TXAK	0x08
#define I2C_CR_RSTA	0x04
#define I2C_CR_BCST	0x01

#define I2C_SR_MCF	0x80
#define I2C_SR_MAAS	0x40
#define I2C_SR_MBB	0x20
#define I2C_SR_MAL	0x10
#define I2C_SR_BCSTM	0x08
#define I2C_SR_SRW	0x04
#define I2C_SR_MIF	0x02
#define I2C_SR_RXAK	0x01
#define CONFIG_I2C_TIMEOUT	10000

#define I2C_DR		0xFF
#define I2C_DR_SHIFT	0
#define I2C_DR_RES	~(I2C_DR)

#define I2C_DFSRR	0x3F
#define I2C_DFSRR_SHIFT	0
#define I2C_DFSRR_RES	~(I2C_DR)
#define I2C_READ_BIT  1
#define I2C_WRITE_BIT 0

typedef struct fsl_i2c {

	UCHAR adr;		/* I2C slave address */
	UCHAR res0[3];
/*#define I2C_ADR		0xFE
#define I2C_ADR_SHIFT	1
#define I2C_ADR_RES	~(I2C_ADR)
*/

	UCHAR fdr;		/* I2C frequency divider register */
	UCHAR res1[3];
/*#define IC2_FDR		0x3F
#define IC2_FDR_SHIFT	0
#define IC2_FDR_RES	~(IC2_FDR)
*/
	UCHAR cr;		/* I2C control redister	*/
	UCHAR res2[3];
/*#define I2C_CR_MEN	0x80
#define I2C_CR_MIEN	0x40
#define I2C_CR_MSTA	0x20
#define I2C_CR_MTX	0x10
#define I2C_CR_TXAK	0x08
#define I2C_CR_RSTA	0x04
#define I2C_CR_BCST	0x01
*/
	UCHAR sr;		/* I2C status register */
	UCHAR res3[3];
/*#define I2C_SR_MCF	0x80
#define I2C_SR_MAAS	0x40
#define I2C_SR_MBB	0x20
#define I2C_SR_MAL	0x10
#define I2C_SR_BCSTM	0x08
#define I2C_SR_SRW	0x04
#define I2C_SR_MIF	0x02
#define I2C_SR_RXAK	0x01
#define CONFIG_I2C_TIMEOUT	10000
*/
	UCHAR dr;		/* I2C data register */
	UCHAR res4[3];
/*#define I2C_DR		0xFF
#define I2C_DR_SHIFT	0
#define I2C_DR_RES	~(I2C_DR)
*/
	UCHAR dfsrr;	/* I2C digital filter sampling rate register */
	UCHAR res5[3];
/*#define I2C_DFSRR	0x3F
#define I2C_DFSRR_SHIFT	0
#define I2C_DFSRR_RES	~(I2C_DR)
#define I2C_READ_BIT  1
#define I2C_WRITE_BIT 0
*/
	/* Fill out the reserved block */
	UCHAR res6[0xE8];
} fsl_i2c_t;

static unsigned int i2c_bus_speed[2] = {CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SPEED};


static struct fsl_i2c *i2c_dev[4] = {
	(struct fsl_i2c *) (CONFIG_SYS_I2C1_OFFSET),
	(struct fsl_i2c *) (CONFIG_SYS_I2C2_OFFSET),
    (struct fsl_i2c *) (CONFIG_SYS_I2C3_OFFSET),
    (struct fsl_i2c *) (CONFIG_SYS_I2C4_OFFSET)
};


static __inline__ int i2c_wait(int write,int port);
static int i2c_wait4bus(int port);
void BspP2041I2cWrite(UINT32 port,UINT32 uireg,UINT32 value);

#define DEVICEID  0x30
#endif /*IIC_H_*/
