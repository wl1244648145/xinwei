#ifndef __BSP_MS_H
#define __BSP_MS_H



/*主备状态*/
#define MCT_MASTER 			1	//为主
#define MCT_SLAVE 			0	//为备
#define MCT_AVIABLE 		1	//可用
#define MCT_UNAVIABLE		0	//不可用
#define MCT_OPP_PD_ON		1	//对板在位
#define MCT_OPP_PD_OUT		0	//对板不在位
#define MCT_OPP_INRESET		1	//对板在复位
#define MCT_OPP_UNRESET 	0	//对板没有复位
#define MCT_SLAVE_ONLINE	1	//备板在位
#define MCT_SLAVE_UNONLINE	0	//备板不在位

/*CPLD寄存器*/
#define CPLD_MS_CTR_REG 			76	//控制寄存器
#define CPLD_MS_STATE_REG 			73	//状态寄存器
#define CPLD_MS_SELF_INCIDENT_REG 	74	//事件寄存器1
#define CPLD_MS_OPP_INCIDENT_REG 	75	//事件寄存器2

/*状态寄存器*/
#define SELFSLOT_FLAG 	7	//本板主备状态
#define SELFSLOT_IND  	6	//本板可用状态
#define	OPPSLOT_FLAG	5	//对板主备状态
#define	OPPSLOT_IND		4	//对板可用状态
#define OPPSLOT_PD		3	//对板在位状态
#define OPPSLOT_RST		2	//对板复位状态
#define OPPSLOT_CLK		1	//对板时钟状态
#define OPPSLOT_OTH		0	//本板时钟在用状态

/*事件寄存器1*/
#define MCT_SELF_MASTER_to_SLAVE	7	//本板主降备
#define MCT_SELF_SLAVE_to_MASTER	6	//本板备升主
#define MCT_SELF_ENABLE_to_DISABLE	5	//本板可用变不可用
#define MCT_SELF_CLOCK_to_DISABLE	4	//本板时钟可用变不可用

/*事件寄存器2*/
#define MCT_OPP_MASTER_to_SLAVE		7	//对板主降备
#define MCT_OPP_ENABLE_to_DISABLE	6	//对板可用变不可用
#define MCT_OPP_PD_ON_to_OUT		5	//对板拔出
#define MCT_OPP_RESET				4	//对板复位
#define MCT_OPP_CLOCK_to_DISABLE	3	//对板时钟可用变不可用

/*标示通知BBP/DSP主备切换过程*/
#define MCT_MS_SWITCH_EVENT_CLEAR	0	//没有主备切换
#define	MCT_MS_SWITCH_INFORMED		1	//已通知BBP/DSP
#define	MCT_MS_SWITCH_RCV_ACK		2	//收到BBP/DSP回应消息

/*外网口*/
#define GE0	0
#define GE1	1
#define LMT	2

/*内网卡*/
#define ETH0	0
#define ETH1	1
#define ETH2	2
#define ETH3	3

/*CPLD PLL寄存器*/
#define PLL1_CPLD_CLOCK	3
#define PLL2_CPLD_CLOCK	7

/*CPLD RESET寄存器*/
#define SWITCH_CPLD_RESET	2	

typedef s8 (*MSSWITCH_FUNCPTR)(u8 switchto,u32 cause);
s8(*funcMasterSlaveSwitch)(u8 switchto,u32 cause);		//函数指针，指向L3注册函数
	
/*主备切换原因及不能切换原因*/
#define MCT_MS_SWITCH_CASE_CLEAR 	0x00	//清除主备切换原因
#define MCT_SWITCH_CASE_OPP_OUT		0x01	//对板拔出
#define	MCT_SWITCH_CASE_OPP_RESET 	0x02	//对板复位
#define MCT_SWITCH_CASE_OPP_DISABLE	0x03	//对板不可用
#define MCT_SWITCH_CASE_COMPETITION	0x04	//主备竞争
#define MCT_SWITCH_CASE_GE0_UNLINK  0x05	//核心网网线没有连接
#define MCT_SWITCH_CASE_GPS_UNLOCK	0x06	//GPS锁定星数不足
#define MCT_SWITCH_CASE_CPLD_PLL	0x07	//CPLD PLL失锁
#define MCT_SWITCH_CASE_SWITCH		0x08	//SWITCH配置失败
#define MCT_SWITCH_CASE_CLK_DISABLE	0x09	//本板时钟不可用

#endif