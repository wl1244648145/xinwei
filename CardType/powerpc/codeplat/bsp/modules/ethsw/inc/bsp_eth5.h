/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_port.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/

#define ETHSW_RRU_TEST_DISABLE	0
#define ETHSW_RRU_TEST_ENABLE	       1

#define PPC_ETHNET0_PORT    1<<0
#define PPC_ETHNET1_PORT    1<<1
#define PPC_ETHNET2_PORT    1<<3
#define PPC_ETHNET3_PORT    1<<4
#define IPRAN_PHY1_PORT     1<<5
#define IPRAN_PHY2_PORT     1<<6
#define TRACERAN_PHY_PORT   1<<7
#define PPC_SLOT2_PORT      1<<8
#define PPC_SLOT3_PORT      1<<9
#define PPC_SLOT4_PORT      1<<10
#define PPC_SLOT5_PORT      1<<11
#define PPC_SLOT6_PORT      1<<12
#define PPC_SLOT7_PORT      1<<13

#define MCT_SWITCH_PORT_ETH0	        0	//port 0
#define MCT_SWITCH_PORT_ETH1	        1	//port 1
#define MCT_SWITCH_PORT_PORT2	 2
#define MCT_SWITCH_PORT_ETH2	        3
#define MCT_SWITCH_PORT_ETH3	        4
#define MCT_SWITCH_PORT_GE0		 5
#define MCT_SWITCH_PORT_GE1		 6
#define MCT_SWITCH_PORT_LMT		 7
#define MCT_SWITCH_PORT_SLOT2	 8
#define MCT_SWITCH_PORT_SLOT3	9
#define MCT_SWITCH_PORT_SLOT4	10
#define MCT_SWITCH_PORT_SLOT5	11
#define MCT_SWITCH_PORT_SLOT6	12
#define MCT_SWITCH_PORT_SLOT7	13
#define MCT_SWITCH_PORT_OPPMCT	14

/* 端口状态控制结构,CONTROL PAGE REGISTER MAP : 8bit registers */
typedef struct tag_STRU_ETHSW_PORT_CTRL
{
    u8 StpState:3,       /* spanning tree state */
       rsvd:3,           /* reserved */
       TxDisable:1,      /* tx disable */
       RxDisable:1;      /* rx disable */
} STRU_ETHSW_PORT_CTRL;

/* 交换模式设置结构 */
typedef struct tag_STRU_ETHSW_SWITCH_MODE
{
    u8 rsvd:6,
       SwFwdEn:1,    /* Software Forwarding Enable */
       SwFwdMode:1;  /* Software Forwarding Mode,inverse of SwFwdEn */
} STRU_ETHSW_SWITCH_MODE;

typedef struct tag_STRU_ETHSW_MCAST_CTRL
{
    u8 MlfFmEn:1,     /* Multicast Lookup Fail Forward Map Enable */
       UniFmEn:1,     /* Unicast Lookup Fail Forward Map Enable */
       Rsvd:5,        /* Spare registers */
       MulticastEn:1; /* Must be set to 1 to support Multicast addresses in the ARL table */
} STRU_ETHSW_MCAST_CTRL;

/* 端口状态强制设置结构 */
typedef struct tag_STRU_ETHSW_PORT_STAT
{
    u8 PhyScanEn:1, 
       OverrideEn:1,/*CPU set software override bit to 1 to make bits [5:0]
                       affected. PHY scan register is overridden.*/
       TxFlowEn:1,   /*Software TX flow control enable */
       RxFlowEn:1,   /* Software RX flow control enable */
       Speed:2,      /*Software port speed setting:b00:10M,b01:100M,b10:1000M*/
       Duplex:1,     /*Software duplex mode setting:0: Half-duplex 1: Full-duplex*/
       LinkState:1;  /*0:Link down 1:Link up*/
} STRU_ETHSW_PORT_STAT;


#define PORT_LINK_UP                   0x1   /* 端口LINK UP */
#define PORT_LINK_DOWN                 0x0   /* 端口LINK DOWN */


s32 ethsw_set_mirror(u8 u8MirrorEnable, u8 u8MirrorRules, u8 u8MirroredPort, u8 u8MirrorPort);
s32 ethsw_set_status(u8 u8PortId, u8 u8TxEnable, u8 u8RxEnable);

/******************************* 头文件结束 ***********************************/
