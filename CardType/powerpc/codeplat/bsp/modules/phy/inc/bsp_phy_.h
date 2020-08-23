/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_phy_app.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/

#define cpu_relax()	asm volatile ("" ::: "memory");

#define E_PHY_OK              0
#define E_PHY_INIT_FAILED    -1
#define E_PHY_WRONG_PARAM    -2  /* 参数错误 */
#define E_PHY_ERR             -3 


#define MIIMIND_BUSY            0x00000001
#define MIIMIND_NOTVALID        0x00000004

#define MII_READ_COMMAND       0x00000001
#define MIIMCOM_READ_CYCLE			0x00000001 /* Read cycle */


#define MII_CTRL_SS_10          0
#define MII_CTRL_SS_100         (MII_CTRL_SS_LSB)
#define MII_CTRL_SS_1000        (MII_CTRL_SS_MSB)


typedef struct bsp_mdio 
{
	u32 miimcfg;		/* MII management configuration reg */
	u32 miimcom;		/* MII management command reg */
	u32 miimadd;		/* MII management address reg */
	u32 miimcon;		/* MII management control reg */
	u32 miimstat;		/* MII management status reg */
	u32 miimind;		/* MII management indication reg */
} bsp_mdio_t;

#define MDIO_EMAC1    (0x4E1120)
#define MDIO_EMAC2    (0x4E3120)
#define MDIO_EMAC3    (0x4E5120)
#define MDIO_EMAC4    (0x4E7120)
#define MDIO_EMAC5    (0x4E9120)


//s32 phy_bcm54618_init(u32 u32PhyId);

/******************************* 头文件结束 ********************************/

