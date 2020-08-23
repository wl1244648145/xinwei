/*******************************************************************************
 *  COPYRIGHT XINWEI Communications Equipment CO.,LTD
 ********************************************************************************
 * 源文件名:           bsp_phy_app.c 
 * 功能:                  
 * 版本:                                                                  
 * 编制日期:                              
 * 作者:                                              
 *******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <time.h>
/**************************** 私用头文件* **********************************/
#include "../../../com_inc/bsp_types.h"
#include "../inc/bsp_phy_bcm54618.h"
#include "../inc/bsp_phy_app.h"
#include <linux/mii.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stddef.h>
#include <errno.h>
#include <sys/utsname.h>
#include <limits.h>
#include <ctype.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/sockios.h>
#include <linux/ethtool.h>
#include "bsp_external.h"

#define BCM_54210S
#undef BCM_54618S
#define BspDelay	sleep

#define COPPER_ENERGY_DETECT(x) 	((x & 0xa0) == 0x20)
#define COPPER_IS_LINKING(x)		((x & 0xa0) == 0xa0)
#define COPPER_IS_NOLINK(x)		((x & 0xa0) == 0x00)
#ifdef BCM_54618S
#define FIBER_SIGNAL_DETECT(x) 		((x & 0x50) == 0x10)
#define FIBER_IS_LINKING(x)		((x & 0x50) == 0x50)
#endif
#ifdef BCM_54210S
#define FIBER_SIGNAL_DETECT(x) 		((x & 0x50) == 0x10)
//#define FIBER_SIGNAL_DETECT(x) 		((x & 0x200) == 0x200)
#define FIBER_IS_LINKING(x)		((x & 0x100) == 0x100)
#define FIBER_IS_NOLINK(x)		((x & 0x100) == 0x00)
#endif
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

#define COPPER	1
#define FIBER	2

struct network_info {
	u32 dwNetWorkType;
	u32 dwElecWorkMode;
	u32 dwOptWorkMode;
	char name[12];
	int addr;	
	int eth_self_adaption;
	int status;
};

enum {
	NO_LINK,
	COPPER_LINKING,
	FIBER_LINKING,
};

/*********************** 全局变量定义/初始化 **************************/
extern struct bsp_mdio *mdio_dev[1] ;
Export  u8 *g_u8ccsbar;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* network configuration */
static struct network_info network_info[] = {

	[0] = {
		.dwNetWorkType = NET_WORK_TYPE_ELECTRICAL,
		.name = "eth0",
		.addr = 0x8,
		.eth_self_adaption = 1,
		.status = COPPER_LINKING,
	},

	[1] = {
		.dwNetWorkType = NET_WORK_TYPE_ELECTRICAL,
		.name = "eth1",
		.addr = 0x9,
		.eth_self_adaption = 1,
		.status = COPPER_LINKING,
	},

	[2] = {
		.name = "eth2",
		.addr = 0x19,
		.eth_self_adaption = -1,
		.status = NO_LINK,
	},

	[3] = {
		.name = "eth3",
		.addr = 0x19,
		.eth_self_adaption = 1,
		.status = NO_LINK,
	},
};

static int do_sset(int fd, struct ifreq *ifr, int type, int speed_wanted, int duplex_wanted, int autoneg_wanted);

static char *phy_get_name(int phy_addr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(network_info); i++)
		if (network_info[i].addr == phy_addr)
			return network_info[i].name;

	printf("No such phy address %d\n", phy_addr);

	return NULL;
}

#define phy_get_addr(x)		network_info[x].addr

static int phy_addr_check(int phy_addr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(network_info); i++)
		if (network_info[i].addr == phy_addr)
			return 0;

	return -1;
}

static int phy_get_index(int phy_addr)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(network_info); i++)
		if (network_info[i].addr == phy_addr)
			return i;
	return -1;
}

#define UNSUPPORTED	-1
#define OFF		0
#define ON		1
#define READY		2

#define is_self_adaption_on(x)		(network_info[phy_get_index(x)].eth_self_adaption == ON)
#define is_self_adaption_ready(x)	(network_info[phy_get_index(x)].eth_self_adaption == READY)
#define is_self_adaption_unsupported(x)	(network_info[phy_get_index(x)].eth_self_adaption == UNSUPPORTED)
#define set_self_adaption_on(x)		(network_info[phy_get_index(x)].eth_self_adaption = ON)
#define set_self_adaption_ready(x)	(network_info[phy_get_index(x)].eth_self_adaption = READY)
#define set_self_adaption_off(x)	(network_info[phy_get_index(x)].eth_self_adaption = OFF)

/************************** 局部常数和类型定义 ************************/

/*************************** 局部函数原型声明 **************************/

/******************************* 函数实现 *************************************/
/******************************************************************************
 * 函数名: out_be32
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
static inline void out_be32(volatile unsigned  *addr, int val)
{
	__asm__ __volatile__("stw%U0%X0 %1,%0; sync" : "=m" (*addr) : "r" (val));
}

/******************************************************************************
 * 函数名: in_be32
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
static inline unsigned in_be32(const volatile unsigned  *addr)
{
	unsigned ret;

	__asm__ __volatile__("sync; lwz%U1%X1 %0,%1;\n"
			"twi 0,%0,0;\n"
			"isync" : "=r" (ret) : "m" (*addr));
	return ret;
}

/******************************************************************************
 * 函数名: eth_bcm54618s_read
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s32  eth_bcm54618s_read1(u8 u8PhyAddr, u8 u8RegAddr, u16 *pu16Value)
{
	int value;
	int timeout = 1000000;

	bsp_mdio_t *phyregs = (bsp_mdio_t *) (void *)(g_u8ccsbar + MDIO_EMAC1);

	/* Put the address of the phy, and the register
	 * number into MIIMADD */
	pthread_mutex_lock(&mutex);
	out_be32(&phyregs->miimadd, (u8PhyAddr << 8) | (u8RegAddr & 0x1f));

	/* Clear the command register, and wait */
	out_be32(&phyregs->miimcom, 0);


	/* Initiate a read command, and wait */
	out_be32(&phyregs->miimcom, MIIMCOM_READ_CYCLE);


	/* Wait for the the indication that the read is done */
	while ((in_be32(&phyregs->miimind) & (MIIMIND_NOTVALID | MIIMIND_BUSY))
			&& timeout--)
		;

	/* Grab the value read from the PHY */
	*pu16Value = in_be32(&phyregs->miimstat);

	pthread_mutex_unlock(&mutex);

	return value;

}

/******************************************************************************
 * 函数名: eth_bcm54618s_write
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s32 eth_bcm54618s_write1(u8 u8PhyAddr, u8 u8RegAddr, u16 u16Value)
{
	int timeout = 1000000;
	unsigned int tempaddr;

	pthread_mutex_lock(&mutex);
	bsp_mdio_t *phyregs = (bsp_mdio_t *) (void *)(g_u8ccsbar + MDIO_EMAC1);

	out_be32(&phyregs->miimadd, (u8PhyAddr << 8) | (u8RegAddr & 0x1f));
	out_be32(&phyregs->miimcon, u16Value);

	while ((in_be32(&phyregs->miimind) & MIIMIND_BUSY) && timeout--)
		;
	pthread_mutex_unlock(&mutex);

	return 0;
}

static int eth_bcm54618s_read(u8 phyid, u8 regaddr, u16 *value)
{
	struct ifreq ifr;
	int fd;
	int ret = 0;
	struct mii_ioctl_data *mii_val;
	u16 tmp;

	if (phy_addr_check(phyid))
		return eth_bcm54618s_read1(phyid, regaddr, value);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, phy_get_name(phyid));
	ifr.ifr_addr.sa_family = AF_INET;

	ret = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (!(ifr.ifr_flags & IFF_UP)) {
		ret = -1;
		goto err;
	}

	mii_val = (struct mii_ioctl_data *)&ifr.ifr_ifru;
	mii_val->reg_num = regaddr;

	if (ioctl(fd, SIOCGMIIPHY, &ifr) < 0) {
		ret = -1;
		goto err;
	}

	tmp = mii_val->val_out;

	if (value != NULL)
		*value = tmp;

err:
	close(fd);

	return ret;
}

static int eth_bcm54618s_write(u8 phyid, u8 regaddr, u16 value)
{
	struct ifreq ifr;
	int fd;
	int ret = 0;
	struct mii_ioctl_data *mii_val;

	if (phy_addr_check(phyid))
		return eth_bcm54618s_write1(phyid, regaddr, value);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, phy_get_name(phyid), IFNAMSIZ);
	ifr.ifr_addr.sa_family = AF_INET;

	ret = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (!(ifr.ifr_flags & IFF_UP)) {
		ret = -1;
		goto err;
	}

	mii_val = (struct mii_ioctl_data *)&ifr.ifr_ifru;
	mii_val->val_in = value;
	mii_val->reg_num = regaddr;
	mii_val->phy_id = phyid;

	if (ioctl(fd, SIOCSMIIREG, &ifr) < 0) {
		ret = -1;
		goto err;
	}
err:
	close(fd);

	return ret;
}
/******************************************************************************
 * 函数名: eth_bcm54618s_status
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s32 eth_bcm54618s_status(u8 u8PhyId)
{
	u16   u16MiiCtrl; 
	u16   u16Temp;

	eth_bcm54618s_read(u8PhyId, MII_STAT, &u16Temp);
	if(0!=(u16Temp&0x0004))
	{   
		printf("Link up!\n");
	}
	else
	{
		printf("Link down!\n");
	} 

	eth_bcm54618s_write(u8PhyId, 0x17,0x0f00|MII_ER42);
	eth_bcm54618s_read(u8PhyId, 0x15, &u16MiiCtrl);

	switch(u16MiiCtrl&0x1f)
	{
		case 0x6:
			printf("Operating Mode Status: RGMII-to-copper\n");
			break;
		case 0xd:
			printf("Operating Mode Status: RGMII-to-100BASE-FX (SerDes)\n");
			break;
		case 0xe:
			printf("Operating Mode Status: RGMII-to-SGMII (10/100/1000)\n");
			break;
		case 0xf:
			printf("Operating Mode Status: RGMII-to-SerDes\n");
			break;
		case 0x12:
			printf("Operating Mode Status: SGMII-to-SerDes\n");
			break;
		case 0x13:
			printf("Operating Mode Status: SGMII-to-100BASE-FX (SerDes)\n");
			break;
		case 0x14:
			printf("Operating Mode Status : SGMII-to-copper\n");
			break;
		case 0x16:
			printf("Operating Mode Status: SerDes-to-SerDes (media converter)\n");
			break;
		case 0x17:
			printf("Operating Mode Status: SerDes-to-copper (media converter)\n");
			break;
		default:
			printf("Operating Mode Status: Other Status\n");
			break;
	}
	return (OK); 
}

/******************************************************************************
 * 函数名: eth_bcm54618s_dump_reg
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
void eth_bcm54618s_dump_reg(u8 u8PhyId)
{
	u16 u16RegVal;

	(void)eth_bcm54618s_read(u8PhyId, MII_CTRL, &u16RegVal);
	bsp_dbg("MII_CTRL:       0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_STAT, &u16RegVal);
	bsp_dbg("MII_STAT:       0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_PHY_ID0, &u16RegVal);
	bsp_dbg("MII_PHY_ID0:    0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_PHY_ID1, &u16RegVal);
	bsp_dbg("MII_PHY_ID1:    0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_ANA, &u16RegVal);
	bsp_dbg("MII_ANA:        0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_ANP, &u16RegVal);
	bsp_dbg("MII_ANP:        0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_AN_EXP_REG, &u16RegVal);
	bsp_dbg("MII_AN_EXP_REG: 0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_GB_CTRL, &u16RegVal);
	bsp_dbg("MII_GB_CTRL:    0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_GB_STAT, &u16RegVal);
	bsp_dbg("MII_GB_STAT:    0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_ESR, &u16RegVal);
	bsp_dbg("MII_ESR:        0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_ECR, &u16RegVal);
	bsp_dbg("MII_ECR:        0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, 0x0007);
	(void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16RegVal);
	bsp_dbg("AUX_CTRL:       0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, 0x1007);
	(void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16RegVal);
	bsp_dbg("AUX_REG:        0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, 0x2007);
	(void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16RegVal);
	bsp_dbg("POWER_REG:      0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, 0x7007);
	(void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16RegVal);
	bsp_dbg("MISC_CTRL:      0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_read(u8PhyId, MII_ASSR, &u16RegVal);
	bsp_dbg("MII_ASSR:       0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_write(u8PhyId, MII_GSR, 0x7c00);
	(void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16RegVal);
	bsp_dbg("MII_GSR:      0x%04x\n", u16RegVal, 0,0,0,0,0);

	(void)eth_bcm54618s_write(u8PhyId, MII_GSR, 0x5000);
	(void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16RegVal);

	return;   
}

/******************************************************************************
 * 函数名: eth_bcm54618s_dump_all_reg
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
void eth_bcm54618s_dump_all_reg(u8 u8PhyId)
{
	int i;
	u16 u16regVlaue;
	printf("*****************normal reg***********************\n");
	for(i=0;i<=0x0a;i++)
	{
		(void)eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
		printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
	}
	for(i=0x0f;i<=0x0f;i++)
	{
		(void)eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
		printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
	}

	i = 0x10;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);


	i = 0x11;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);

	i = 0x12;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);


	i = 0x13;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);

	i = 0x14;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);


	i = 0x17;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);

	printf("*****************18 shadow reg***********************\n");
	for(i=0x0;i<=0x02;i++)
	{
		(void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, (0x07|(i<<12)));
		(void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16regVlaue);
		printf("18 Reg,shadow 0x%2x : 0x%4x\n", i, u16regVlaue);
	} 
	i = 0x04;
	(void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, (0x07|(i<<12)));
	(void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16regVlaue);
	printf("18 Reg,shadow 0x%2x : 0x%4x\n", i, u16regVlaue);
	i = 0x07;
	(void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, (0x07|(i<<12)));
	(void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16regVlaue);
	printf("18 Reg,shadow 0x%2x : 0x%4x\n", i, u16regVlaue);


	i = 0x19;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);


	i = 0x1a;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);


	i = 0x1b;
	eth_bcm54618s_read(u8PhyId, i, &u16regVlaue);
	printf("Reg 0x%2x: value =0x%4x\n", i, u16regVlaue);

	printf("*****************1C shadow reg***********************\n");
	for(i=0x02;i<=0x05;i++)
	{
		(void)eth_bcm54618s_write(u8PhyId, MII_GSR, i<<10);
		(void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16regVlaue);
		printf("1C Reg,shadow 0x%2x : 0x%4x\n", i, u16regVlaue);
	}
	for(i=0x08;i<=0x0a;i++)
	{
		(void)eth_bcm54618s_write(u8PhyId, MII_GSR, i<<10);
		(void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16regVlaue);
		printf("1C Reg,shadow 0x%2x : 0x%4x\n", i, u16regVlaue);
	}
	for(i=0x0c;i<=0x0f;i++)
	{
		(void)eth_bcm54618s_write(u8PhyId, MII_GSR, i<<10);
		(void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16regVlaue);
		printf("1C Reg,shadow 0x%2x : 0x%4x\n", i, u16regVlaue);
	}

	for(i=0x11;i<=0x18;i++)
	{
		(void)eth_bcm54618s_write(u8PhyId, MII_GSR, i<<10);
		(void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16regVlaue);
		printf("1C Reg,shadow 0x%2x : 0x%4x\n", i, u16regVlaue);
	}

	for(i=0x1a;i<=0x1f;i++)
	{
		(void)eth_bcm54618s_write(u8PhyId, MII_GSR, i<<10);
		(void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16regVlaue);
		printf("1C Reg,shadow 0x%2x : 0x%4x\n", i, u16regVlaue);
	}

	printf("*****************Expansion Registers:1111 + Expansion***********************\n");
	for(i=0x0;i<=0x2;i++)
	{
		eth_bcm54618s_write(u8PhyId, 0x17, 0x0f00|i);
		eth_bcm54618s_read(u8PhyId, 0x15, &u16regVlaue);
		printf("Expansion f Reg 0x%2x : 0x%4x\n", i, u16regVlaue);
	}

	for(i=0x4;i<=0x6;i++)
	{
		eth_bcm54618s_write(u8PhyId, 0x17, 0x0f00|i);
		eth_bcm54618s_read(u8PhyId, 0x15, &u16regVlaue);
		printf("Expansion f Reg 0x%2x : 0x%4x\n", i, u16regVlaue);
	}
	i=0x42;
	eth_bcm54618s_write(u8PhyId, 0x17, 0x0f00|i);
	eth_bcm54618s_read(u8PhyId, 0x15, &u16regVlaue);
	printf("Expansion f Reg 0x%2x : 0x%4x\n", i, u16regVlaue);

	i=0x44;
	eth_bcm54618s_write(u8PhyId, 0x17, 0x0f00|i);
	eth_bcm54618s_read(u8PhyId, 0x15, &u16regVlaue);
	printf("Expansion f Reg 0x%2x : 0x%4x\n", i, u16regVlaue);

	i=0x50;
	eth_bcm54618s_write(u8PhyId, 0x17, 0x0f00|i);
	eth_bcm54618s_read(u8PhyId, 0x15, &u16regVlaue);
	printf("Expansion f Reg 0x%2x : 0x%4x\n", i, u16regVlaue);

	i=0x52;
	eth_bcm54618s_write(u8PhyId, 0x17, 0x0f00|i);
	eth_bcm54618s_read(u8PhyId, 0x15, &u16regVlaue);
	printf("Expansion f Reg 0x%2x : 0x%4x\n", i, u16regVlaue);  
}


/******************************************************************************
 * 函数名: eth_bcm54210s_read
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s32  eth_bcm54210s_read1(u8 u8PhyAddr, u8 u8RegAddr, u16 *pu16Value)
{
	int value;
	int timeout = 1000000;

	bsp_mdio_t *phyregs = (bsp_mdio_t *) (void *)(g_u8ccsbar + MDIO_EMAC1);

	/* Put the address of the phy, and the register
	 * number into MIIMADD */
	pthread_mutex_lock(&mutex);
	out_be32(&phyregs->miimadd, (u8PhyAddr << 8) | (u8RegAddr & 0x1f));

	/* Clear the command register, and wait */
	out_be32(&phyregs->miimcom, 0);


	/* Initiate a read command, and wait */
	out_be32(&phyregs->miimcom, MIIMCOM_READ_CYCLE);


	/* Wait for the the indication that the read is done */
	while ((in_be32(&phyregs->miimind) & (MIIMIND_NOTVALID | MIIMIND_BUSY))
			&& timeout--)
		;

	/* Grab the value read from the PHY */
	*pu16Value = in_be32(&phyregs->miimstat);

	pthread_mutex_unlock(&mutex);

	return value;

}

/******************************************************************************
 * 函数名: eth_bcm54618s_write
 * 功  能: 
 * 函数存储类型:
 * 参数:
 * 参数名        类型        输入/输出       描述
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
s32 eth_bcm54210s_write1(u8 u8PhyAddr, u8 u8RegAddr, u16 u16Value)
{
	int timeout = 1000000;
	unsigned int tempaddr;

	pthread_mutex_lock(&mutex);
	bsp_mdio_t *phyregs = (bsp_mdio_t *) (void *)(g_u8ccsbar + MDIO_EMAC1);

	out_be32(&phyregs->miimadd, (u8PhyAddr << 8) | (u8RegAddr & 0x1f));
	out_be32(&phyregs->miimcon, u16Value);

	while ((in_be32(&phyregs->miimind) & MIIMIND_BUSY) && timeout--)
		;
	pthread_mutex_unlock(&mutex);

	return 0;
}


static int eth_bcm54210s_read(u8 phyid, u8 regaddr, u16 *value)
{
	struct ifreq ifr;
	int fd;
	int ret = 0;
	struct mii_ioctl_data *mii_val;
	u16 tmp;
#if 0
	if (phy_addr_check(phyid))
#else
		return eth_bcm54210s_read1(phyid, regaddr, value);
#endif
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, phy_get_name(phyid));
	ifr.ifr_addr.sa_family = AF_INET;

	ret = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (!(ifr.ifr_flags & IFF_UP)) {
		ret = -1;
		goto err;
	}

	mii_val = (struct mii_ioctl_data *)&ifr.ifr_ifru;
	mii_val->reg_num = regaddr;

	if (ioctl(fd, SIOCGMIIPHY, &ifr) < 0) {
		ret = -1;
		goto err;
	}

	tmp = mii_val->val_out;

	if (value != NULL)
		*value = tmp;

err:
	close(fd);

	return ret;
}

static int eth_bcm54210s_write(u8 phyid, u8 regaddr, u16 value)
{
	struct ifreq ifr;
	int fd;
	int ret = 0;
	struct mii_ioctl_data *mii_val;
#if 0
	if (phy_addr_check(phyid))
#else
		return eth_bcm54210s_write1(phyid, regaddr, value);
#endif
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, phy_get_name(phyid), IFNAMSIZ);
	ifr.ifr_addr.sa_family = AF_INET;

	ret = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (!(ifr.ifr_flags & IFF_UP)) {
		ret = -1;
		goto err;
	}

	mii_val = (struct mii_ioctl_data *)&ifr.ifr_ifru;
	mii_val->val_in = value;
	mii_val->reg_num = regaddr;
	mii_val->phy_id = phyid;

	if (ioctl(fd, SIOCSMIIREG, &ifr) < 0) {
		ret = -1;
		goto err;
	}

err:
	close(fd);

	return ret;
}

u16 bcm54210s_read_reg(u8 u8PhyId, u16 reg)
{
    u16 u16regVlaue;

    eth_bcm54210s_read(u8PhyId, reg, &u16regVlaue);
    return u16regVlaue;
}
void bcm54210s_read_reg_test(u8 u8PhyId, u16 reg)
{
    u16 u16regVlaue;

    eth_bcm54210s_read(u8PhyId, reg, &u16regVlaue);
    printf("read 54210s reg(0x%x) = 0x%x.\n",reg,u16regVlaue);
}
void bcm54210s_write_reg(u8 u8PhyId, u16 reg, u16 u16regVlaue)
{
    eth_bcm54210s_write(u8PhyId, reg, u16regVlaue);
}

u16 bcm54210s_read_clause_reg(u8 u8PhyId, u16 devad, u16 reg)
{
    u16 u16regVlaue;
    bcm54210s_write_reg(u8PhyId, 0x0d, devad|0x00<<8);
    bcm54210s_write_reg(u8PhyId, 0x0e, reg);
    bcm54210s_write_reg(u8PhyId, 0x0d, devad|0x40<<8);
    u16regVlaue = bcm54210s_read_reg(u8PhyId, 0x0e);
    return u16regVlaue;
}
u16 bcm54210s_read_clause_reg_test(u8 u8PhyId, u16 devad, u16 reg)
{
    u16 u16regVlaue;
    bcm54210s_write_reg(u8PhyId, 0x0d, devad|0x00<<8);
    bcm54210s_write_reg(u8PhyId, 0x0e, reg);
    bcm54210s_write_reg(u8PhyId, 0x0d, devad|0x40<<8);
    u16regVlaue = bcm54210s_read_reg(u8PhyId, 0x0e);
    printf("read clause reg devad(0x%x)reg(0x%x) = 0x%x.\n", devad, reg, u16regVlaue);
    return u16regVlaue;
}
void bcm54210s_write_clause_reg(u8 u8PhyId, u16 devad, u16 reg, u16 u16regVlaue)
{
    bcm54210s_write_reg(u8PhyId, 0x0d, devad|0x00<<8);
    bcm54210s_write_reg(u8PhyId, 0x0e, reg);
    bcm54210s_write_reg(u8PhyId, 0x0d, devad|0x40<<8);
    bcm54210s_write_reg(u8PhyId, 0x0e, u16regVlaue);
}
u16 bcm54210s_read_rdb_reg(u8 u8PhyId, u16 reg)
{
    u16 u16regVlaue;
    bcm54210s_write_reg(u8PhyId, 0x17, 0x0f7e);
    bcm54210s_write_reg(u8PhyId, 0x15, 0x0000);
    bcm54210s_write_reg(u8PhyId, 0x1e, reg);
    u16regVlaue = bcm54210s_read_reg(u8PhyId, 0x1f);
    return u16regVlaue;
}
u16 bcm54210s_read_rdb_reg_test(u8 u8PhyId, u16 reg)
{
    u16 u16regVlaue;
    bcm54210s_write_reg(u8PhyId, 0x17, 0x0f7e);
    bcm54210s_write_reg(u8PhyId, 0x15, 0x0000);
    bcm54210s_write_reg(u8PhyId, 0x1e, reg);
    u16regVlaue = bcm54210s_read_reg(u8PhyId, 0x1f);
    printf("read rdb reg reg(0x%x) = 0x%x.\n",  reg, u16regVlaue);
    return u16regVlaue;
}

s32 bsp_phy_bcm54210s_test(u8 u8PhyId)
{
    u16 u16regVlaue1 = 0;
    u16 u16regVlaue2 = 0;
    s32 u32retvalue = BSP_OK;

    u16regVlaue1 = bcm54210s_read_rdb_reg(u8PhyId, 0x981);
    u16regVlaue2 = bcm54210s_read_rdb_reg(u8PhyId, 0x982);

    if((u16regVlaue1==0x8100)&&(u16regVlaue2==0x88A8))
       u32retvalue = BSP_OK;
    else
       u32retvalue = BSP_ERROR; 
    return u32retvalue;
}

void bcm54210s_write_rdb_reg(u8 u8PhyId, u16 reg, u16 u16regVlaue)
{
    bcm54210s_write_reg(u8PhyId, 0x17, 0x0f7e);
    bcm54210s_write_reg(u8PhyId, 0x15, 0x0000);
    bcm54210s_write_reg(u8PhyId, 0x1e, reg);
    bcm54210s_write_reg(u8PhyId, 0x1f, u16regVlaue);
}
void bcm54210s_changereg_copper_mode(u8 u8PhyId)
{
    u16 u16regVlaue;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, 0x21);
    u16regVlaue &= 0xfffe;
    bcm54210s_write_rdb_reg(u8PhyId, 0x21, u16regVlaue);
}
void bcm54210s_changereg_sgmii_fiber_mode(u8 u8PhyId)
{
    u16 u16regVlaue;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, 0x21);
    u16regVlaue |= 0x1;
    bcm54210s_write_rdb_reg(u8PhyId, 0x21, u16regVlaue);
}
void eth_bcm54210s_dump_all_reg(u8 u8PhyId)
{
    int i;
    u16 u16regVlaue;
    
    printf("IEEE standard registers...........\n");
#if 0
    printf("copper mode........\n");
    bcm54210s_changereg_copper_mode(u8PhyId);
    for(i=0x0; i<=0xf; i++)
    {
        eth_bcm54210s_read(u8PhyId, i, &u16regVlaue);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
#endif
    //bcm54210s_changereg_sgmii_fiber_mode(u8PhyId);
    i=0;
    for(i=0x0; i<=0xf; i++)
    {
        eth_bcm54210s_read(u8PhyId, i, &u16regVlaue);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }

    printf("RDB regs..................................\n");
    i=0;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=1;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=5;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=6;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    for(i=9;i<=0xe;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    i=0x12;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x14;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x15;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    for(i=0x18;i<=0x1a;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    i=0x1d;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x1e;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x21;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x28;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x29;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x2a;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x2c;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x2f;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x30;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x34;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x35;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x36;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x37;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x70;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x87;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0xaa;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0xab;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0xac;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0xad;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0xaf;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0xf5;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    for(i=0x200;i<0x205;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    for(i=0x233;i<0x239;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    i=0x23b;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x23e;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    for(i=0x2a0;i<0x2a6;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    i=0x800;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x819;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    for(i=0x82b;i<0x82e;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    for(i=0x831;i<0x83d;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    i=0x923;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0x928;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    for(i=0x980;i<0x995;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    printf("second serdes reg.....................................\n");
    for(i=0xb00;i<0xb02;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    for(i=0xb04;i<0xb06;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    for(i=0xb10;i<0xb17;i++)
    {
        u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
        printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    }
    i=0xb1b;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0xb40;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
    i=0xb41;
    u16regVlaue = bcm54210s_read_rdb_reg(u8PhyId, i);
    printf("Reg 0x%2x: value =0x%4x\n",i,u16regVlaue);
}

/*****************************Copper/fiber self adaption code***********************/
static int bcm54618s_shadow_1c_read(int phy_addr, u8 reg_addr, u8 shadow_reg, u16 *value)
{
	int ret;

	ret = eth_bcm54618s_write(phy_addr, reg_addr, shadow_reg << 10);
	if (ret)
		return ret;

	return eth_bcm54618s_read(phy_addr, reg_addr, value);
}

static void bcm54618s_shadow_1c_write(int phy_addr, u8 reg_addr, u8 shadow_reg, u16 value)
{
	eth_bcm54618s_write(phy_addr, reg_addr, value | 0x8000 | (shadow_reg << 10));
}
s32 eth_bcm54210s_status(u8 u8PhyId)
{
	u16   u16MiiCtrl; 
	u16   u16Temp;

	eth_bcm54210s_read(u8PhyId, MII_STAT, &u16Temp);
	if(0!=(u16Temp&0x0004))
	{
		printf("Link up!\n");
	}
	else
	{
		printf("Link down!\n");
	} 

	eth_bcm54210s_write(u8PhyId, 0x17,0x0f00|MII_ER42);
	eth_bcm54210s_read(u8PhyId, 0x15, &u16MiiCtrl);

	switch(u16MiiCtrl&0x1f)
	{
		case 0x6:
			printf("Operating Mode Status: RGMII-to-copper\n");
			break;
		case 0xd:
			printf("Operating Mode Status: RGMII-to-100BASE-FX (SerDes)\n");
			break;
		case 0xe:
			printf("Operating Mode Status: RGMII-to-SGMII (10/100/1000)\n");
			break;
		case 0xf:
			printf("Operating Mode Status: RGMII-to-SerDes\n");
			break;
		case 0x12:
			printf("Operating Mode Status: SGMII-to-SerDes\n");
			break;
		case 0x13:
			printf("Operating Mode Status: SGMII-to-100BASE-FX (SerDes)\n");
			break;
		case 0x14:
			printf("Operating Mode Status : SGMII-to-copper\n");
			break;
		case 0x16:
			printf("Operating Mode Status: SerDes-to-SerDes (media converter)\n");
			break;
		case 0x17:
			printf("Operating Mode Status: SerDes-to-copper (media converter)\n");
			break;
		default:
			printf("Operating Mode Status: Other Status\n");
			break;
	}
	return (OK); 
}
static void phy_set_auto_detect(int phy_addr)
{
#ifdef BCM_54618S
	u16 value;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1e, &value))
		return;

	/* enable auto-detect medium, Copper selected when both medium active */
	value |= 1;
	value &= ~0x2;

	bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1e, value);
#endif
#ifdef BCM_54210S

	u16 value;

	value = bcm54210s_read_rdb_reg(phy_addr, 0x23e);

	/* enable auto-detect medium, Copper selected when both medium active */
	value |= 1;
	value &= ~0x2;

	bcm54210s_write_rdb_reg(phy_addr,0x23e,value);    

#endif
}

static void phy_disable_auto_detect(int phy_addr)
{
#ifdef BCM_54618S
	u16 value;
	int ret = 0;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1e, &value))
		return;

	if (value & 1) {
		value &= ~0x1;

		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1e, value);
	}
#endif
#ifdef BCM_54210S

	u16 value;
	int ret = 0;

	value = bcm54210s_read_rdb_reg(phy_addr, 0x23e);

	if (value & 1) {
		value &= ~0x1;

		bcm54210s_write_rdb_reg(phy_addr,0x23e,value); 
	}
#endif
}

static void phy_enable_auto_detect(int phy_addr)
{
#ifdef BCM_54618S
	u16 value;
	int ret = 0;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1e, &value))
		return;

	if (value & 1) {
		value &= ~0x1;

		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1e, value);
	}
#endif
#ifdef BCM_54210S

	u16 value;
	int ret = 0;

	value = bcm54210s_read_rdb_reg(phy_addr, 0x23e);

    value |= 0x1;

    bcm54210s_write_rdb_reg(phy_addr,0x23e,value); 
#endif
}

static void phy_reset(int phy_addr)
{
#ifdef BCM_54618S
	u16 value;
	int timeout = 1000000;

	eth_bcm54618s_write(phy_addr, 0, 0x8000);

	while (timeout--) {
		if (eth_bcm54618s_read(phy_addr, 0, &value))
			return;

		if ((value & 0x8000) == 0)
			break;
	}
#endif
#ifdef BCM_54210S

	u16 value;
	int timeout = 1000000;

	bcm54210s_write_reg(phy_addr, 0, 0x8000);

	while (timeout--) {
		value = bcm54210s_read_reg(phy_addr, 0);

		if ((value & 0x8000) == 0)
			break;
	}
#endif
}

static void phy_led_config(int phy_addr, u8 flag)
{
#ifdef BCM_54618S
	u16 value;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1e, &value))
		return;

	value = flag == COPPER ? value & ~0x40 : value | 0x40;

	bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1e, value);
#endif
#ifdef BCM_54210S

	u16 value;

	value = bcm54210s_read_rdb_reg(phy_addr, 0x23e);

	value = flag == COPPER ? value & ~0x40 : value | 0x40;

	bcm54210s_write_rdb_reg(phy_addr, 0x23e, value);
#endif
}

static int phy_get_mode_ctl(int phy_addr, u16 *value)
{
#ifdef BCM_54618S
	u16 val;

	if (eth_bcm54618s_read(phy_addr, 0, &val))
		return -1;

	if (val & 0x0800)
		eth_bcm54618s_write(phy_addr, 0, val & ~0x0800);

	return bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, value);
#endif
#ifdef BCM_54210S

	u16 val;

	//if (eth_bcm54210s_read(phy_addr, 0, &val))
	//	return -1;
	eth_bcm54210s_read(phy_addr, 0, &val);

	if (val & 0x0800)
		eth_bcm54210s_write(phy_addr, 0, val & ~0x0800);

	*value = bcm54210s_read_rdb_reg(phy_addr, 0x21);
    return 0;
#endif
}

static int phy_get_external_serdes_ctl(int phy_addr, u16 *value)
{

	u16 val;

	eth_bcm54210s_read(phy_addr, 0, &val);

	if (val & 0x0800)
		eth_bcm54210s_write(phy_addr, 0, val & ~0x0800);

	*value = bcm54210s_read_rdb_reg(phy_addr, 0x234);
    return 0;
}


void phy_read_mode_ctrl(int phy_addr)
{
    u16 value;
    phy_get_mode_ctl(phy_addr,&value);
    printf("read phy mode ctrl = 0x%x.\n",value);
}


static void phy_enable_auto_mdix(int phy_addr)
{
#ifdef BCM_54618S
	u16 value;

	if (eth_bcm54618s_write(phy_addr, 0x18, 0x7007))
		return;

	if (eth_bcm54618s_read(phy_addr, 0x18, &value))
		return;

	if (!(value & 0x200))
		eth_bcm54618s_write(phy_addr, 0x18, value | 0x200 | 0x8000);
#endif
#ifdef BCM_54210S

	u16 value;

	//if (eth_bcm54210s_write(phy_addr, 0x18, 0x7007))
	//	return;
	eth_bcm54210s_write(phy_addr, 0x18, 0x7007);

	//if (eth_bcm54210s_read(phy_addr, 0x18, &value))
	//	return;
	eth_bcm54210s_read(phy_addr, 0x18, &value);

	if (!(value & 0x200))
		eth_bcm54210s_write(phy_addr, 0x18, value | 0x200 | 0x8000);
#endif
}

static inline int fiber_online(int phy_addr)
{
#ifdef BCM_54618S
	u16 val;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return 0;

	return val & 0x10;
#endif
#ifdef BCM_54210S

	u16 val;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	return val & 0x10;
#endif
}

static inline int copper_online(int phy_addr)
{
#ifdef BCM_54618S
	u16 val;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return 0;

	return val & 0x20;
#endif
#ifdef BCM_54210S

	u16 val;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	return val & 0x20;
#endif
}

static inline int fiber_working(int phy_addr)
{
#ifdef BCM_54618S
	u16 val;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return 0;

	return (val & 0x51) == 0x51;
#endif
#ifdef BCM_54210S

	u16 val;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	return (val & 0x51) == 0x51;
#endif
}

static inline int copper_working(int phy_addr)
{
#ifdef BCM_54618S
	u16 val;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return 0;

	return (val & 0xa1) == 0xa0;
#endif
#ifdef BCM_54210S

	u16 val;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	return (val & 0xa1) == 0xa0;
#endif
}

static int phy_check_link_timout(int phy_addr)
{
#ifdef BCM_54618S
	int timeout = 10; /* 10s */
	u16 val;

	do {
		if (eth_bcm54618s_read(phy_addr, 1, &val))
			return -1;

		if (val & 0x4)
			break;
		sleep(1);
	} while (--timeout);

	printf("Establish linking cost %ds\n", 10 - timeout);
	return timeout == 0? -1 : 0;
#endif
#ifdef BCM_54210S

	int timeout = 10; /* 10s */
	u16 val;

	do {
		//if (eth_bcm54210s_read(phy_addr, 1, &val))
		//	return -1;
		eth_bcm54210s_read(phy_addr, 1, &val);

		if (val & 0x4)
			break;
		sleep(1);
	} while (--timeout);

	printf("Establish linking cost %ds\n", 10 - timeout);
	return timeout == 0? -1 : 0;
#endif
}

static void phy_fiber_poweron(int phy_addr)
{
#ifdef BCM_54618S
	u16 val, val_bak;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return;

	val_bak = val;

	if (!(val & 1))
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val | 1);

	if (eth_bcm54618s_read(phy_addr,0, &val))
		return;

	eth_bcm54618s_write(phy_addr, 0, val & ~0x0800);

	if (!(val_bak & 1))
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val_bak);
#endif
#ifdef BCM_54210S

	u16 val, val_bak;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	val_bak = val;

	if (!(val & 1))
		bcm54210s_write_rdb_reg(phy_addr, 0x21, val | 1);
    

	val = bcm54210s_read_reg(phy_addr,0);

	bcm54210s_write_reg(phy_addr, 0, val & ~0x0800);

	if (!(val_bak & 1))
		bcm54210s_write_rdb_reg(phy_addr, 0x21, val_bak);

#endif
}

static void phy_copper_poweron(int phy_addr)
{
#ifdef BCM_54618S
	u16 val, val_bak;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return;

	val_bak = val;

	if (val & 1)
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val & ~1);

	if (eth_bcm54618s_read(phy_addr,0, &val))
		return;

	eth_bcm54618s_write(phy_addr, 0, val & ~0x0800);

	if (val_bak & 1)
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val_bak);
#endif
#ifdef BCM_54210S

	u16 val, val_bak;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	val_bak = val;

	if (val & 1)
		bcm54210s_write_rdb_reg(phy_addr, 0x21, val & ~1);

	val = bcm54210s_read_reg(phy_addr,0);

	bcm54210s_write_reg(phy_addr, 0, val & ~0x0800);

	if (val_bak & 1)
		bcm54210s_write_rdb_reg(phy_addr, 0x21, val_bak);
        
#endif
}

static void phy_fiber_poweroff(int phy_addr)
{
#ifdef BCM_54618S
	u16 val, val_bak;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return;

	val_bak = val;

	if (!(val & 1))
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val | 1);

	if (eth_bcm54618s_read(phy_addr,0, &val))
		return;

	eth_bcm54618s_write(phy_addr, 0, val | 0x0800);

	if (!(val_bak & 1))
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val_bak);
#endif
#ifdef BCM_54210S

	u16 val, val_bak;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	val_bak = val;

	if (!(val & 1))
		bcm54210s_write_rdb_reg(phy_addr, 0x21,  val | 1);

	//if (eth_bcm54210s_read(phy_addr,0, &val))
	//	return;
	eth_bcm54210s_read(phy_addr,0, &val);

	eth_bcm54210s_write(phy_addr, 0, val | 0x0800);

	if (!(val_bak & 1))
		bcm54210s_write_rdb_reg(phy_addr, 0x21, val_bak);

#endif
}

static void phy_copper_poweroff(int phy_addr)
{
#ifdef BCM_54618S
	u16 val, val_bak;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return;

	val_bak = val;

	if (val & 1)
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val & ~1);

	if (eth_bcm54618s_read(phy_addr,0, &val))
		return;

	eth_bcm54618s_write(phy_addr, 0, val | 0x0800);

	if (val_bak & 1)
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val_bak);
#endif
#ifdef BCM_54210S

	u16 val, val_bak;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	val_bak = val;

	if (val & 1)
		bcm54210s_write_rdb_reg(phy_addr, 0x21, val & ~1);

	//if (eth_bcm54210s_read(phy_addr,0, &val))
	//	return;
	eth_bcm54210s_read(phy_addr,0, &val);

	eth_bcm54210s_write(phy_addr, 0, val | 0x0800);

	if (val_bak & 1)
		bcm54210s_write_rdb_reg(phy_addr, 0x21, val_bak);
#endif
}

static void phy_change_mode_to_copper(int phy_addr)
{
#ifdef BCM_54618S
	u16 val;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return;

	if (val & 1)
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val & ~1);
#endif
#ifdef BCM_54210S

	u16 val;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	if (val & 1)
		bcm54210s_write_rdb_reg(phy_addr, 0x21, val & ~1);
#endif
}

static void phy_change_mode_to_fiber(int phy_addr)
{
#ifdef BCM_54618S
	u16 val;

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &val))
		return;

	if (!(val & 1))
		bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, val | 1);
#endif
#ifdef BCM_54210S

	u16 val;

	val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

	if (!(val & 1))
		bcm54210s_write_rdb_reg(phy_addr, 0x21,  val | 1);
#endif
}
static int do_gset(int fd, struct ifreq *ifr, int *speed, int *duplex);
static int phy_set_to_fiber(int phy_addr, int speed, int duplex, int autoneg)
{
#ifdef BCM_54618S
	u16 value;
	struct ifreq ifr;
	int fd;
	int ret = 0;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, phy_get_name(phy_addr));

	phy_change_mode_to_fiber(phy_addr);

	phy_fiber_poweroff(phy_addr);

	/* delay */
	usleep(100000);

	phy_fiber_poweron(phy_addr);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return 1;
	}

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &value)) {
		ret = -1;
		goto err;
	}

	value &= ~0x6;
	value |= 0x2;

	bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, value);

	/* Force to set port to 1000M full duplex autoneg = DISABLE */ 
	if (do_sset(fd, &ifr, FIBER, speed, duplex, autoneg) < 0) {
		ret = 1;
		goto err;
	}

	if (phy_check_link_timout(phy_addr)) {
		ret = -1;
		goto err;
	}


	phy_led_config(phy_addr, FIBER);

err:
	close(fd);
	return ret;
#endif
#ifdef BCM_54210S

    /*Clear bit[7]=1'b0 to disable RGMII node*/
    bcm54210s_write_rdb_reg(phy_addr,0x02f,0x7167);
    /*Change bit[2:1] to 2'b10 to enable SGMII-to-Copper or SGMII-to-Fiber mode,and clear bit[0]=1'b0 to select copper register*/
    bcm54210s_write_rdb_reg(phy_addr,0x021,0x7c04);
    /*Set bit[11]=1'b1 to power-down the copper interface*/    
    bcm54210s_write_reg(phy_addr,0x00,0x1940);
    /*Set bit[0]=1'b1 to select SerDes register*/
    bcm54210s_write_rdb_reg(phy_addr,0x021,0x7c05);
    /*Clear bit[11]=1'b0 to power-up the primary SerDes interface*/
    bcm54210s_write_reg(phy_addr,0x00,0x1140);
    /*Set bit[0]=1'b1 to configure EXT_SERDES_SEL bit at SGMII-to-Fiber mode*/
    bcm54210s_write_rdb_reg(phy_addr,0x234,0x518f);
    /*Clear bit[11]=1'b0 to power-up the secondary SerDes interface*/
    bcm54210s_write_rdb_reg(phy_addr,0xb00,0x1140);
    /*Clear bit[9]=1'b0 to turn off the auto-medium detect function and enable SerDes LED mode if needed*/
    bcm54210s_write_rdb_reg(phy_addr,0x23e,0x78e2);

    return 0;


#endif
}

void phy_fiber_test(int phy_addr)
{
    bcm54210s_write_rdb_reg(phy_addr,0x02f,0x7167);
    bcm54210s_write_rdb_reg(phy_addr,0x021,0x7c04);
    bcm54210s_write_reg(phy_addr,0x00,0x1940);
    bcm54210s_write_rdb_reg(phy_addr,0x021,0x7c05);
    bcm54210s_write_reg(phy_addr,0x00,0x1140);
    bcm54210s_write_rdb_reg(phy_addr,0x234,0x518f);
    bcm54210s_write_rdb_reg(phy_addr,0xb00,0x1140);
    bcm54210s_write_rdb_reg(phy_addr,0x23e,0x78e2);
}
static int phy_set_to_copper(int phy_addr, int speed, int duplex, int autoneg)
{
#ifdef BCM_54618S
	u16 value;
	struct ifreq ifr;
	int fd;
	int ret = 0;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, phy_get_name(phy_addr));

	phy_change_mode_to_copper(phy_addr);

	phy_copper_poweroff(phy_addr);

	/* fore auto mdix enable when autoneg is disable */
	if (autoneg == AUTONEG_DISABLE)
		phy_enable_auto_mdix(phy_addr);

	/* delay */
	usleep(100000);

	phy_copper_poweron(phy_addr);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return 1;
	}

	if (bcm54618s_shadow_1c_read(phy_addr, 0x1c, 0x1f, &value)) {
		ret = -1;
		goto err;
	}

	value &= ~0x6;

	bcm54618s_shadow_1c_write(phy_addr, 0x1c, 0x1f, value);

	if (do_sset(fd, &ifr, COPPER, speed, duplex, autoneg) < 0) {
		ret = 1;
		goto err;
	}


	if (phy_check_link_timout(phy_addr)) {
		ret = -1;
		goto err;
	}

	phy_led_config(phy_addr, COPPER);
err:
	close(fd);
	return ret;
#endif
#ifdef BCM_54210S

	u16 value;
	struct ifreq ifr;
	int fd;
	int ret = 0;
    u16 u16CtrlVal;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, phy_get_name(phy_addr));

	phy_change_mode_to_copper(phy_addr);

	phy_copper_poweroff(phy_addr);

	/* fore auto mdix enable when autoneg is disable */
	if (autoneg == AUTONEG_DISABLE)
		phy_enable_auto_mdix(phy_addr);

	/* delay */
	usleep(100000);

	phy_copper_poweron(phy_addr);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return 1;
	}

	value = bcm54210s_read_rdb_reg(phy_addr, 0x21);
#ifdef BCM_54618S
	value &= ~0x6;
#endif
#ifdef BCM_54210S

	value &= ~0x6;
	value |= 0x4;
#endif
	bcm54210s_write_rdb_reg(phy_addr, 0x21, value);
#ifdef BCM_54618S
	if (do_sset(fd, &ifr, COPPER, speed, duplex, autoneg) < 0) {
		ret = 1;
		goto err;
	}
#endif
#ifdef BCM_54210S

    u16CtrlVal = bcm54210s_read_reg(phy_addr, 0);
    u16CtrlVal |= 0x40;
    u16CtrlVal |= 0x100;
    u16CtrlVal |= 0x1000;
    bcm54210s_write_reg(phy_addr, 0, u16CtrlVal);

    u16CtrlVal = bcm54210s_read_rdb_reg(phy_addr,0x234);
    bcm54210s_write_rdb_reg(phy_addr,0x234,u16CtrlVal & (~0x1));
#endif


	if (phy_check_link_timout(phy_addr)) {
		ret = -1;
		goto err;
	}

	phy_led_config(phy_addr, COPPER);
err:
	close(fd);
	return ret;
#endif
}

void set_phy_fiber(int phy_addr, int speed, int duplex, int autoneg)
{
    phy_set_to_fiber(phy_addr,speed,duplex,autoneg);
}
void set_phy_copper(int phy_addr, int speed, int duplex, int autoneg)
{
    phy_set_to_copper(phy_addr,speed,duplex,autoneg);
}

void phy_register_reset(int phy_addr)
{
    u16 val, val_bak;

    val = bcm54210s_read_rdb_reg(phy_addr, 0x21);

    val_bak = val;

    if (val & 1)
        bcm54210s_write_rdb_reg(phy_addr, 0x21, val & ~1);

    val = bcm54210s_read_rdb_reg(phy_addr, 0x70);

    bcm54210s_write_rdb_reg(phy_addr, 0x70, val | 0x1);

    val = bcm54210s_read_rdb_reg(phy_addr, 0x82b);

    bcm54210s_write_rdb_reg(phy_addr, 0x82b, val | 0x8000);

    val = bcm54210s_read_clause_reg(phy_addr, 0x1, 0x0);

    bcm54210s_write_clause_reg(phy_addr, 0x1, 0x0, val | 0x8000);

    if (val_bak & 1)
        bcm54210s_write_rdb_reg(phy_addr, 0x21, val_bak);
    
}

static void phy_config_init(int phy_addr)
{

	phy_reset(phy_addr);

	phy_copper_poweron(phy_addr);
	phy_fiber_poweron(phy_addr);

	//phy_disable_auto_detect(phy_addr);

	/* delay 1s to wait for linking */
	sleep(1);

	set_self_adaption_on(phy_addr);
}


static void phy_reset_link(int phy_addr)
{
	network_info[phy_get_index(phy_addr)].status = NO_LINK;
}

static int netif_config(int phy_addr, int work_type, struct network_info *info)
{
	int speed, duplex, autoneg;
	int work_mode;

	if (work_type == COPPER)
		work_mode = info->dwElecWorkMode;
	else{
		work_mode = info->dwOptWorkMode;
    work_mode = NET_WORK_MODE_FULL_DUPLEX_1000M;
}
	switch (work_mode) {
	case NET_WORK_MODE_FULL_DUPLEX_10M:
		speed = SPEED_10;
		duplex = DUPLEX_FULL;
		autoneg = AUTONEG_DISABLE;
		break;
	case NET_WORK_MODE_FULL_DUPLEX_100M:
		speed = SPEED_100;
		duplex = DUPLEX_FULL;
		autoneg = AUTONEG_DISABLE;
		break;
	case NET_WORK_MODE_FULL_DUPLEX_1000M:
		speed = SPEED_1000;
		duplex = DUPLEX_FULL;
		autoneg = AUTONEG_DISABLE;
		break;
	case NET_WORK_MODE_AUTONEG:
		speed = -1;
		duplex = -1;
		autoneg = AUTONEG_ENABLE;
	}

	if (work_type == COPPER) {
		if (phy_set_to_copper(phy_addr, speed, duplex, autoneg)){
			return -1;
		    }
	} else {
		if (phy_set_to_fiber(phy_addr, speed, duplex, autoneg)){
			return -1;
		    }
	}

	return 0;
}

#if 1
static void *phy_thread(void *data)
{
	u16 value;
    u16 extvalue;
	int phy_addr = (int)data;
	int speed = 0, duplex = 0, autoneg = 0;
	int counter = 0;
	int index = phy_get_index(phy_addr);
	struct network_info *info = &network_info[index];

	printf("%s id:%d start....\n", __func__, phy_addr);

	phy_config_init(phy_addr);

	while(1) {
		if (phy_get_mode_ctl(phy_addr, &value) || phy_get_external_serdes_ctl(phy_addr, &extvalue))
			goto sleep;

		switch (network_info[index].status) {
		case NO_LINK:
			if (info->dwNetWorkType == NET_WORK_TYPE_SELF_ADAPTIVE) {
				if (COPPER_ENERGY_DETECT(value) || COPPER_IS_LINKING(value)) {
					if (netif_config(phy_addr, COPPER, info))
						break;

					info->status = COPPER_LINKING;
					printf("[self auto adaption] Copper link up\n");
				} else if (FIBER_SIGNAL_DETECT(value) || FIBER_IS_LINKING(extvalue)) {
					if (netif_config(phy_addr, FIBER, info))
						break;

					info->status = FIBER_LINKING;
					printf("[self auto adaption] Fiber link up\n");
				}
			} else if (info->dwNetWorkType == NET_WORK_TYPE_ELECTRICAL) {
				if (COPPER_ENERGY_DETECT(value) || COPPER_IS_LINKING(value)) {
					if (netif_config(phy_addr, COPPER, info))
						break;

					info->status = COPPER_LINKING;
					printf("[force setting] Copper link up\n");
				}
			} else {
				if (FIBER_SIGNAL_DETECT(extvalue) || FIBER_IS_LINKING(extvalue)) {
					if (netif_config(phy_addr, FIBER, info))
						break;

					info->status = FIBER_LINKING;
					printf("[force setting] Fiber link up\n");
				}
			}

			break;
		case COPPER_LINKING:
			/* Copper energy is slow than fiber signal */
			if (COPPER_ENERGY_DETECT(value) )
				counter++;
			else if (COPPER_IS_NOLINK(value) || counter == 10) {
				printf("COPPER_LINKING reg[rdb]:[0x21] = %04x\n", value);
				printf("Copper link down\n");
				info->status = NO_LINK;
				counter = 0;
			}

			break;
		case FIBER_LINKING:
			#if 1
			if (!FIBER_IS_LINKING(extvalue)) {
				printf("FIBER_LINKING reg[rdb]:[0x234] = %04x\n", extvalue);
				info->status = NO_LINK;
				printf("Fiber link down\n");
			}			
			#else
			if (FIBER_SIGNAL_DETECT(value))
			       counter++;
			else if (counter == 10 || FIBER_IS_NOLINK(value)) {
				printf("FIBER_LINKING reg[1c]:[11111] = %04x\n", value);
				info->status = NO_LINK;
				printf("Fiber link down\n");
				counter = 0;
			}
			#endif

			break;
		default:
			printf("error %d\n", network_info[phy_get_index(phy_addr)].status);
		}
sleep:
		sleep(1);
	}

	return NULL;
}
#endif

static int send_ioctl(int fd, struct ifreq *ifr)
{
	return ioctl(fd, SIOCETHTOOL, ifr);
}

static int do_gset(int fd, struct ifreq *ifr, int *speed, int *duplex)
{
	int err;
	struct ethtool_cmd ecmd;

	ecmd.cmd = ETHTOOL_GSET;
	ifr->ifr_data = (caddr_t)&ecmd;
	err = send_ioctl(fd, ifr);
	if (err)
		return -1;

	*speed = ethtool_cmd_speed(&ecmd);
	*duplex = ecmd.duplex;

	return 0;
}

static int do_sset(int fd, struct ifreq *ifr, int type, int speed_wanted,
		   int duplex_wanted, int autoneg_wanted)
{
	int err;

	struct ethtool_cmd ecmd;

	ecmd.cmd = ETHTOOL_GSET;
	ifr->ifr_data = (caddr_t)&ecmd;
	err = send_ioctl(fd, ifr);
	if (err < 0) {
		perror("Cannot get current device settings");
		return -1;
	} else {
		if (speed_wanted != -1)
			ethtool_cmd_speed_set(&ecmd, speed_wanted);
		if (duplex_wanted != -1)
			ecmd.duplex = duplex_wanted;
		if (autoneg_wanted != -1)
			ecmd.autoneg = autoneg_wanted;

		ecmd.advertising = ecmd.supported &
			(ADVERTISED_10baseT_Half |
			 ADVERTISED_10baseT_Full |
			 ADVERTISED_100baseT_Half |
			 ADVERTISED_100baseT_Full |
			 ADVERTISED_1000baseT_Half |
			 ADVERTISED_1000baseT_Full |
			 ADVERTISED_2500baseX_Full |
			 ADVERTISED_10000baseT_Full |
			 ADVERTISED_20000baseMLD2_Full |
			 ADVERTISED_20000baseKR2_Full);

		if (speed_wanted == SPEED_1000 &&
		    autoneg_wanted == AUTONEG_DISABLE &&
		    type == COPPER) {
			ecmd.autoneg = AUTONEG_ENABLE;
			ecmd.advertising = ecmd.supported & ADVERTISED_1000baseT_Full;
		}

		if (speed_wanted == SPEED_100 &&
		    autoneg_wanted == AUTONEG_ENABLE &&
		    type == COPPER) {
			ecmd.autoneg = AUTONEG_ENABLE;
			ecmd.advertising = ecmd.supported & ADVERTISED_100baseT_Full;
		}

		if (speed_wanted == SPEED_10 &&
		    autoneg_wanted == AUTONEG_ENABLE &&
		    type == COPPER) {
			ecmd.autoneg = AUTONEG_ENABLE;
			ecmd.advertising = ecmd.supported & ADVERTISED_10baseT_Full;
		}
		/* Try to perform the update. */
		ecmd.cmd = ETHTOOL_SSET;
		ifr->ifr_data = (caddr_t)&ecmd;
		err = send_ioctl(fd, ifr);
		if (err < 0) {
			perror("Cannot set new settings");
			return -1;
		}
	}

	return 0;
}

UINT32 BspSetNetWorkInfo(UINT32 dwPort, UINT32 dwNetWorkType, UINT32 dwElecWorkMode, UINT32 dwOptWorkMode)
{
	struct ifreq ifr;
	int ret = 0;

	if (dwPort > 3)
		return ERR_UNSUPPORT;

	if (dwNetWorkType == NET_WORK_TYPE_OPTICAL) {
		/* Fiber only supper 1000M */
		if (dwOptWorkMode != NET_WORK_MODE_FULL_DUPLEX_1000M) {
			printf("Fiber only supper 1000M\n");
			ret = ERR_UNSUPPORT;
			goto err2;
		}

		if (!fiber_online(phy_get_addr(dwPort))) {
			printf("Fiber isn't online, configuration failed\n");
			ret = ERR_NO_LINK;
			goto err1;
		}

	} else if (dwNetWorkType == NET_WORK_TYPE_ELECTRICAL) {

		switch (dwElecWorkMode) {
		case NET_WORK_MODE_FULL_DUPLEX_10M:
		case NET_WORK_MODE_FULL_DUPLEX_100M:
		case NET_WORK_MODE_FULL_DUPLEX_1000M:
		case NET_WORK_MODE_AUTONEG:
			break;
		default:
			printf("The mode isn't supported\n");
			ret = ERR_UNSUPPORT;
			goto err2;
		} 

		if (!copper_online(phy_get_addr(dwPort))) {
			printf("Copper isn't online, configuration failed\n");
			ret = ERR_NO_LINK;
			goto err1;
		}

	}

err1:
	network_info[dwPort].dwNetWorkType = dwNetWorkType;
	network_info[dwPort].dwElecWorkMode = dwElecWorkMode;
	network_info[dwPort].dwOptWorkMode = dwOptWorkMode;

	phy_reset_link(phy_get_addr(dwPort));
err2:
	return ret;
}

UINT32 BspGetNetPortState(UINT32 dwPort, NetPortInfo_t *info)
{
	struct ifreq ifr;
	int fd, ret = 0;
	int speed, duplex;

	if (dwPort > 3 || info == NULL)
		return ERR_UNSUPPORT;

	if (copper_working(phy_get_addr(dwPort)))
		info->type = NET_WORK_TYPE_ELECTRICAL;
	else if (fiber_working(phy_get_addr(dwPort)))
		info->type = NET_WORK_TYPE_OPTICAL;
	else {
		/* Both offline */
		info->type = -1;
		info->speed = -1;
		info->duplex = -1;
		return ERR_NO_LINK;
	}

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, network_info[dwPort].name);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return 0;
	}

	if (do_gset(fd, &ifr, &speed, &duplex) < 0) {
		goto out;
	}

	info->speed = speed;
	info->duplex = duplex;
out:
	close(fd);
	return ret;
}

static void BspGetNetPortState_test(int port)
{
	int ret;
	NetPortInfo_t info;

	ret = BspGetNetPortState(port, &info);
	printf("device eth%d type %s speed %d duplex %s\n", port,
			info.type == -1 ? "NO LINK" : info.type == NET_WORK_TYPE_ELECTRICAL ? "Copper" : "Fiber", info.speed,
			info.duplex == DUPLEX_HALF ? "half" : "full");
}

void port_info_print()
{
    int i;
    printf("***************port status***************\n");
    
    for(i=0;i<sizeof(network_info)/sizeof(network_info[0]);i++)
    {
        printf("phy addr: %d.\n",network_info[i].addr);
        printf("net name: %s.\n",network_info[i].name);
        printf("net work type: %d.\n",network_info[i].dwNetWorkType);
        printf("elc work mode: %d.\n",network_info[i].dwElecWorkMode);
        printf("opt work mode: %d.\n",network_info[i].dwOptWorkMode);
        printf("self adaption: %d.\n",network_info[i].eth_self_adaption);
        printf("status: %d (0:NO_LINK/1:COPPER_LINKING/2:FIBER_LINKING).\n",network_info[i].status);

    }
}

void get_port_state(int port)
{
    BspGetNetPortState_test(port);
}

void phy_connect_adapt(void)
{
	pthread_t pid;
	pthread_attr_t attr;
	int i;
	int dwPort;

	for (i = 0; i < ARRAY_SIZE(network_info); i++) {
		char cmd[100];
		dwPort = i;

		if (is_self_adaption_unsupported(phy_get_addr(dwPort)))
			continue;

		memset(cmd, 0, sizeof(cmd));

		sprintf(cmd, "ifconfig eth%d up", dwPort);

		system(cmd);

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		pthread_create(&pid, &attr, phy_thread, phy_get_addr(dwPort));
		pthread_attr_destroy(&attr);

		//network_info[dwPort].eth_self_adaption = 0;
	}
}

int bsp_get_phy_flag(int dwPort)
{
	int phy_addr;
	int index;
	
	if(dwPort < 0 || dwPort > 3)
		return BSP_ERROR;
	
	phy_addr = phy_get_addr(dwPort);
	index = phy_get_index(phy_addr);
	if(NO_LINK == network_info[index].status)
		return 0;
	else
		return 1;
}


int bsp_get_phy_linked(void)
{
	int dwPort;
	int phy_addr;
	int index;

	phy_addr = phy_get_addr(2);
	index = phy_get_index(phy_addr);
	if(NO_LINK == network_info[index].status)
	{
		return BSP_ERROR;
	}

	for(dwPort = 0;dwPort < 2;dwPort++)
	{
		phy_addr = phy_get_addr(0);
		index = phy_get_index(phy_addr);
		if(NO_LINK != network_info[index].status)
		{
			break;
		}
	}
	if(2 == dwPort)
	{
		return BSP_ERROR;
	}
	return BSP_OK;
}


/******************************* 源文件结束 ********************************/
