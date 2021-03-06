#ifndef __BSP_CORE_H
#define __BSP_CORE_H
#include "../../../com_inc/bsp_types.h"
#include "../../interrupt/inc/bsp_interrupt.h"
 
extern u8 *g_u8ccsbar;
#define MAX_INT_EVENT_NUM           256
#define MAX_EXT_IRQ_PIN_NUM          12
#define EPLD_CC_MASTER_PIN 1
typedef struct{
    UINT32   udEventNo;          /* 事件号 */
    UINT32   udIntNo;             /* 中断号 */
    VOID     (*IsrFunc)();          /* 中断服务程序 */
    UINT32   udarg;               /* 中断回调的参数 */
    UINT32   udPrio;              /* 中断优先级 */
    UINT8    ucTrgMode;         /* 中断触发方式,1:电平,0:边沿 */
    UINT8    ucIntSecure;         /* ARM专用,1:secure,0:Non-secure */
    UINT16   uwIntMode;          /* 中断模式，表示组合还是独立 */
    UINT32   udIntMaskMode;     /* 中断屏蔽模式（self、all、bitmask）*/
	
}T_IntcAttrs;

typedef struct {

    UINT32		tma_vec[4];
    UINT32		tmb_vec[4];
}T_MPIC_TIME_VECS;

typedef struct {

    UINT32		msga_vec[4];
    UINT32		msgb_vec[4];
}T_MPIC_MSG_VECS;

typedef struct {
    UINT32 		ipi_vecs[4];
    T_MPIC_TIME_VECS	timer_vecs;
    T_MPIC_MSG_VECS     msg_vecs;
    UINT32      gmsg_vecs[32];
    UINT32		spurious_vec;
   
}T_MPIC_VECS;

/*  interrupt handler description  */
    
typedef struct intHandlerDesc     
    {
    VOIDFUNCPTR			vec;	/* interrupt vector */
    int				arg;	/* interrupt handler argument */
    struct  intHandlerDesc *	next;	/* pointer to the next handler */
    } INT_HANDLER_DESC;

typedef struct mpic_cpu_private
{
	UINT32 ipi_dpatch_reg0;
	UINT32 resvd0[3];
	UINT32 ipi_dpatch_reg1;
	UINT32 resvd1[3];
	UINT32 ipi_dpatch_reg2;
	UINT32 resvd2[3];
	UINT32 ipi_dpatch_reg3;
	UINT32 resvd3[3];
	UINT32 ctask_pri_reg;
	UINT32 resvd4[3];
	UINT32 whoami_reg;
	UINT32 resvd5[3];
	UINT32 int_ack_reg;
	UINT32 resvd6[3];
	UINT32 eoi_reg;
} MPIC_CPU_PRIVATE;

 
typedef struct  tagT_IntEventRecord
{
    ULONG     ulIntEventNo;               /* 中断事件号 */
    ULONG     ulIntVector;                /* 中断向量号 */
	ULONG     ulSrcCoreMask;              /* 中断源Core掩码 */
	ULONG     ulDstCoreMask;              /* 中断目的Core掩码 */
	ULONG     ulIntType;                   /* 中断类型 1:外部中断 2:IPI中断 3:MSG中断*/ 
	ULONG     ulReserved;                  /* 保留 */
    
}T_IntEventRecord;

/* IPI核间中断号 */
#define MPIC_IPI0_VECTOR   251
#define MPIC_IPI1_VECTOR   252
#define MPIC_IPI2_VECTOR   253
#define MPIC_IPI3_VECTOR   254
/* MSG中断号 */
#define MPIC_MSG0_VECTOR   235
#define MPIC_MSG1_VECTOR   236
#define MPIC_MSG2_VECTOR   237
#define MPIC_MSG3_VECTOR   238

#define MPIC_MSG4_VECTOR   239
#define MPIC_MSG5_VECTOR   240
#define MPIC_MSG6_VECTOR   241
#define MPIC_MSG7_VECTOR   242


/* 外部中断配置表 */
typedef struct  tagT_ExtIrqConfig
{
    ULONG     ulExtIrqEventNo;            /*中断事件号:外部中断事件号*/
    ULONG     ulDstCoreMask;              /*中断目的Core掩码*/
}T_ExtIrqConfig;

/* 内部核间中断配置表 */
typedef struct  tagT_InterCoreIntConfig
{
    ULONG     ulInterCoreIntEventNo;     /*中断事件号:核间中断事件*/
    ULONG     ulSrcCoreMask;              /*中断源Core掩码*/
    ULONG     ulDstCoreMask;              /*中断目的Core掩码*/
}T_InterCoreIntConfig;

#define EXT_IRQ_EVENT_BASE                      0  /* 外部中断事件号基址 */
#define PLATFORM_INTER_CORE_INT_EVENT_BASE   20  /* 平台核间中断事件号基址 */
#define INTER_CORE_INT_EVENT_BASE             40  /* 核间中断事件号基址 */


#define P3041_CORE_NUM 4
#define P3080_CORE_NUM 8

#define INT_TYPE_EXT_IRQ  1
#define INT_TYPE_IPI  2
#define INT_TYPE_MSG  3

typedef struct  tagT_IntCfgInit
{
    T_ExtIrqConfig * ptExtIrqCfgTbl;               /* 外部中断配置表 */
	T_InterCoreIntConfig * ptInterCoreIntCfgTbl;  /* 核间中断配置表 */
	ULONG ulExtIrqTblItemCnts;                       /* 外部中断配置表项大小 */
	ULONG ulInterIntCoreTblItemCnts;                /* 核间中断配置表项大小 */
}T_IntCfgInit;/* OSS传入的结构 */

typedef struct  tagT_InterCoreIntCfg
{
	UCHAR     aucIpiMapArray[8];
	ULONG     ulMsgUsedCnts;
}T_InterCoreIntRecord;

 typedef struct tagTSlaveCoreCfg
{
	/* 该从核黑匣子的起始地址与大小, */
	unsigned long dwBbxPhyBase;
	unsigned long dwBbxVirtBase;
	unsigned long dwBbxSize;
	

	/* 网口接收内存起始地址与大小 */
	unsigned long dwEmacPhyBase;
	unsigned long dwEmacVirtBase;
	unsigned long dwEmacSize;
	
	/* 共享设备树始地址与大小 */
	unsigned long dwFdtPhyBase;
	unsigned long dwFdtVirtBase;
	unsigned long dwFdtSize;
	
	/* 从核GDB通道内存起始地址与大小 */
	unsigned long dwGdbPhyBase;
	unsigned long dwGdbVirtBase;
	unsigned long dwGdbSize;
	
	
 
	unsigned long dwShellPhyBase;
	unsigned long dwShellVirtBase;
	unsigned long dwShellSize;

	/* 核间通信通道内存起始地址与大小 */
	unsigned long dwIpcPhyBase;
	unsigned long dwIpcVirtBase;
	unsigned long dwIpcSize;

	unsigned long dwShmManPhyBase;
	unsigned long dwShmManVirtBase;
	unsigned long dwShmManSize;
	
	unsigned long dwSfnAddr;
	
	unsigned char aucIpAddr[4];
	unsigned char aucMacAddr[6];
	
	T_IntEventRecord tIntEventReCord[256];
	
	/* 共享内存虚拟地址与物理地址的偏移 */
	unsigned long dwPageOffset;
	
}TSlaveCoreCfg;

TSlaveCoreCfg *g_ptSlaveCoreCfg;
#define  INTERRUPT_TABLESIZE   256
    
#define MPIC_CCSROFF  		0x40000			/* OFFSET of MPIC  */

#define MPIC_WHO_AM_I_PRI_CPU_REG (MPIC_CCSROFF + 0x90)
#define MPIC_IPIDR_0                             (MPIC_CCSROFF  +0x40)
#define MPIC_IPIDR_1                             (MPIC_CCSROFF  +0x50)
#define MPIC_IPIDR_2                             (MPIC_CCSROFF  +0x60)
#define MPIC_IPIDR_3                             (MPIC_CCSROFF  +0x70)

/*Private Access Registers (CPU0)*/
#define MPIC_IPI_DPATCH0_REG0	(MPIC_CCSROFF + 0x20040)/* IPI0 dispatch */
#define MPIC_IPI_DPATCH0_REG1	(MPIC_CCSROFF + 0x20050)/* IPI1 dispatch */
#define MPIC_IPI_DPATCH0_REG2	(MPIC_CCSROFF + 0x20060)/* IPI2 dispatch */
#define MPIC_IPI_DPATCH0_REG3	(MPIC_CCSROFF + 0x20070)/* IPI3 dispatch */
#define MPIC_CTASK_PRI0_REG	(MPIC_CCSROFF + 0x20080)/* Cur Task Prio */
#define MPIC_WHO_AM_I0_REG	(MPIC_CCSROFF + 0x20090)/* Who am I */
#define MPIC_INT_ACK0_REG	(MPIC_CCSROFF + 0x200a0)/* Int ack */
#define MPIC_EOI0_REG		(MPIC_CCSROFF + 0x200b0)/* End of Int */

/*Private Access Registers (CPU1)*/
#define MPIC_IPI_DPATCH1_REG0	(MPIC_CCSROFF + 0x21040)/* IPI0 dispatch */
#define MPIC_IPI_DPATCH1_REG1	(MPIC_CCSROFF + 0x21050)/* IPI1 dispatch */
#define MPIC_IPI_DPATCH1_REG2	(MPIC_CCSROFF + 0x21060)/* IPI2 dispatch */
#define MPIC_IPI_DPATCH1_REG3	(MPIC_CCSROFF + 0x21070)/* IPI3 dispatch */
#define MPIC_CTASK_PRI1_REG	(MPIC_CCSROFF + 0x21080)/* Cur Task Prio */
#define MPIC_WHO_AM_I1_REG	(MPIC_CCSROFF + 0x21090)/* Who am I */
#define MPIC_INT_ACK1_REG	(MPIC_CCSROFF + 0x210a0)/* Int ack */
#define MPIC_EOI1_REG		(MPIC_CCSROFF + 0x210b0)/* End of Int */

/*Private Access Registers (CPU2)*/
#define MPIC_IPI_DPATCH2_REG0	(MPIC_CCSROFF + 0x22040)/* IPI0 dispatch */
#define MPIC_IPI_DPATCH2_REG1	(MPIC_CCSROFF + 0x22050)/* IPI1 dispatch */
#define MPIC_IPI_DPATCH2_REG2	(MPIC_CCSROFF + 0x22060)/* IPI2 dispatch */
#define MPIC_IPI_DPATCH2_REG3	(MPIC_CCSROFF + 0x22070)/* IPI3 dispatch */
#define MPIC_CTASK_PRI2_REG	(MPIC_CCSROFF + 0x22080)/* Cur Task Prio */
#define MPIC_WHO_AM_I2_REG	(MPIC_CCSROFF + 0x22090)/* Who am I */
#define MPIC_INT_ACK2_REG	(MPIC_CCSROFF + 0x220a0)/* Int ack */
#define MPIC_EOI2_REG		(MPIC_CCSROFF + 0x220b0)/* End of Int */

/*Private Access Registers (CPU3)*/
#define MPIC_IPI_DPATCH3_REG0	(MPIC_CCSROFF + 0x23040)/* IPI0 dispatch */
#define MPIC_IPI_DPATCH3_REG1	(MPIC_CCSROFF + 0x23050)/* IPI1 dispatch */
#define MPIC_IPI_DPATCH3_REG2	(MPIC_CCSROFF + 0x23060)/* IPI2 dispatch */
#define MPIC_IPI_DPATCH3_REG3	(MPIC_CCSROFF + 0x23070)/* IPI3 dispatch */
#define MPIC_CTASK_PRI3_REG	(MPIC_CCSROFF + 0x23080)/* Cur Task Prio */
#define MPIC_WHO_AM_I3_REG	(MPIC_CCSROFF + 0x23090)/* Who am I */
#define MPIC_INT_ACK3_REG	(MPIC_CCSROFF + 0x230a0)/* Int ack */
#define MPIC_EOI3_REG		(MPIC_CCSROFF + 0x230b0)/* End of Int */

/*Private Access Registers (CPU0~7)*/
#define MPIC_IPI_DPATCH_REG0(CpuId)	(MPIC_CCSROFF + 0x20040 + (0x1000*CpuId))/* IPI0 dispatch */
#define MPIC_IPI_DPATCH_REG1(CpuId)	(MPIC_CCSROFF + 0x20050 + (0x1000*CpuId))/* IPI1 dispatch */
#define MPIC_IPI_DPATCH_REG2(CpuId)	(MPIC_CCSROFF + 0x20060 + (0x1000*CpuId))/* IPI2 dispatch */
#define MPIC_IPI_DPATCH_REG3(CpuId)	(MPIC_CCSROFF + 0x20070 + (0x1000*CpuId))/* IPI3 dispatch */
#define MPIC_CTASK_PRI_REG(CpuId)	(MPIC_CCSROFF + 0x20080 + (0x1000*CpuId))/* Cur Task Prio */
#define MPIC_WHO_AM_I_REG(CpuId)	(MPIC_CCSROFF + 0x20090 + (0x1000*CpuId))/* Who am I */
#define MPIC_INT_ACK_REG(CpuId)	    (MPIC_CCSROFF + 0x200a0 + (0x1000*CpuId))/* Int ack */
#define MPIC_EOI_REG(CpuId)		    (MPIC_CCSROFF + 0x200b0 + (0x1000*CpuId))/* End of Int */

/*Global Registers  */
#define MPIC_FEATURES_REG	(MPIC_CCSROFF + 0x01000)/* Feature reporting */
#define MPIC_GLOBAL_CFG_REG	(MPIC_CCSROFF + 0x01020)/* Global config.  */
#define MPIC_VENDOR_ID_REG	(MPIC_CCSROFF + 0x01080)/* Vendor id */
#define MPIC_PROC_INIT_REG	(MPIC_CCSROFF + 0x01090)/* Processor init. */
#define MPIC_PROC_NMI_REG	(MPIC_CCSROFF + 0x01098)/* Processor init. */
#define MPIC_IPI_0_VEC_REG	(MPIC_CCSROFF + 0x010a0)/* IPI0 vect/prio */
#define MPIC_IPI_1_VEC_REG	(MPIC_CCSROFF + 0x010b0)/* IPI1 vect/prio */
#define MPIC_IPI_2_VEC_REG	(MPIC_CCSROFF + 0x010c0)/* IPI2 vect/prio */
#define MPIC_IPI_3_VEC_REG	(MPIC_CCSROFF + 0x010d0)/* IPI3 vect/prio */
#define MPIC_SPUR_VEC_REG	(MPIC_CCSROFF + 0x010e0)/* Spurious vector */

/* Global timer GroupA & GroupB */
#define MPIC_TMA_FREQ_REG	(MPIC_CCSROFF + 0x010f0)/* Timer Frequency */
#define MPIC_TMA0_CUR_COUNT_REG	(MPIC_CCSROFF + 0x01100)/* Gbl TM0 Cur. Count*/
#define MPIC_TMA0_BASE_COUNT_REG	(MPIC_CCSROFF + 0x01110)/* Gbl TM0 Base Count*/
#define MPIC_TMA0_VEC_REG	(MPIC_CCSROFF + 0x01120)/* Gbl TM0 Vector Pri*/
#define MPIC_TMA0_DES_REG	(MPIC_CCSROFF + 0x01130)/* Gbl TM0 Dest. */
#define MPIC_TMA1_CUR_COUNT_REG	(MPIC_CCSROFF + 0x01140)/* Gbl TM1 Cur. Count*/
#define MPIC_TMA1_BASE_COUNT_REG	(MPIC_CCSROFF + 0x01150)/* Gbl TM1 Base Count*/
#define MPIC_TMA1_VEC_REG	(MPIC_CCSROFF + 0x01160)/* Gbl TM1 Vector Pri*/
#define MPIC_TMA1_DES_REG	(MPIC_CCSROFF + 0x01170)/* Gbl TM1 Dest. */
#define MPIC_TMA2_CUR_COUNT_REG	(MPIC_CCSROFF + 0x01180)/* Gbl TM2 Cur. Count*/
#define MPIC_TMA2_BASE_COUNT_REG	(MPIC_CCSROFF + 0x01190)/* Gbl TM2 Base Count*/
#define MPIC_TMA2_VEC_REG	(MPIC_CCSROFF + 0x011a0)/* Gbl TM2 Vector Pri*/
#define MPIC_TMA2_DES_REG	(MPIC_CCSROFF + 0x011b0)/* Gbl TM2 Dest */
#define MPIC_TMA3_CUR_COUNT_REG	(MPIC_CCSROFF + 0x011c0)/* Gbl TM3 Cur. Count*/
#define MPIC_TMA3_BASE_COUNT_REG	(MPIC_CCSROFF + 0x011d0)/* Gbl TM3 Base Count*/
#define MPIC_TMA3_VEC_REG	(MPIC_CCSROFF + 0x011e0)/* Gbl TM3 Vector Pri*/
#define MPIC_TMA3_DES_REG	(MPIC_CCSROFF + 0x011f0)/* Gbl TM3 Dest. */
#define MPIC_TMA_CTRL		(MPIC_CCSROFF + 0x01300)/* Timer Control */


#define MPIC_TMB_FREQ_REG	(MPIC_CCSROFF + 0x020f0)/* Timer Frequency */
#define MPIC_TMB0_CUR_COUNT_REG	(MPIC_CCSROFF + 0x02100)/* Gbl TM0 Cur. Count*/
#define MPIC_TMB0_BASE_COUNT_REG	(MPIC_CCSROFF + 0x02110)/* Gbl TM0 Base Count*/
#define MPIC_TMB0_VEC_REG	(MPIC_CCSROFF + 0x02120)/* Gbl TM0 Vector Pri*/
#define MPIC_TMB0_DES_REG	(MPIC_CCSROFF + 0x02130)/* Gbl TM0 Dest. */
#define MPIC_TMB1_CUR_COUNT_REG	(MPIC_CCSROFF + 0x02140)/* Gbl TM1 Cur. Count*/
#define MPIC_TMB1_BASE_COUNT_REG	(MPIC_CCSROFF + 0x02150)/* Gbl TM1 Base Count*/
#define MPIC_TMB1_VEC_REG	(MPIC_CCSROFF + 0x02160)/* Gbl TM1 Vector Pri*/
#define MPIC_TMB1_DES_REG	(MPIC_CCSROFF + 0x02170)/* Gbl TM1 Dest. */
#define MPIC_TMB2_CUR_COUNT_REG	(MPIC_CCSROFF + 0x02180)/* Gbl TM2 Cur. Count*/
#define MPIC_TMB2_BASE_COUNT_REG	(MPIC_CCSROFF + 0x02190)/* Gbl TM2 Base Count*/
#define MPIC_TMB2_VEC_REG	(MPIC_CCSROFF + 0x021a0)/* Gbl TM2 Vector Pri*/
#define MPIC_TMB2_DES_REG	(MPIC_CCSROFF + 0x021b0)/* Gbl TM2 Dest */
#define MPIC_TMB3_CUR_COUNT_REG	(MPIC_CCSROFF + 0x021c0)/* Gbl TM3 Cur. Count*/
#define MPIC_TMB3_BASE_COUNT_REG	(MPIC_CCSROFF + 0x021d0)/* Gbl TM3 Base Count*/
#define MPIC_TMB3_VEC_REG	(MPIC_CCSROFF + 0x021e0)/* Gbl TM3 Vector Pri*/
#define MPIC_TMB3_DES_REG	(MPIC_CCSROFF + 0x021f0)/* Gbl TM3 Dest. */
#define MPIC_TMB_CTRL		(MPIC_CCSROFF + 0x02300)/* Timer Control */

/* Global Message GroupA & GroupB */
#define MPIC_MSGA_REG0		(MPIC_CCSROFF + 0x01400)/* Message 0 */
#define MPIC_MSGA_REG1		(MPIC_CCSROFF + 0x01410)/* Message 1 */
#define MPIC_MSGA_REG2		(MPIC_CCSROFF + 0x01420)/* Message 2 */
#define MPIC_MSGA_REG3		(MPIC_CCSROFF + 0x01430)/* Message 3 */
#define MPIC_MSGA_EN_REG	(MPIC_CCSROFF + 0x01500)/* Message Enable */
#define MPIC_MSGA_STATE_REG	(MPIC_CCSROFF + 0x01510)/* Message Status */

#define MPIC_MSGB_REG0		(MPIC_CCSROFF + 0x02400)/* Message 0 */
#define MPIC_MSGB_REG1		(MPIC_CCSROFF + 0x02410)/* Message 1 */
#define MPIC_MSGB_REG2		(MPIC_CCSROFF + 0x02420)/* Message 2 */
#define MPIC_MSGB_REG3		(MPIC_CCSROFF + 0x02430)/* Message 3 */
#define MPIC_MSGB_EN_REG	(MPIC_CCSROFF + 0x02500)/* Message Enable */
#define MPIC_MSGB_STATE_REG	(MPIC_CCSROFF + 0x02510)/* Message Status */

/*  External Interrupt Source Config  */
#define MPIC_EX_INT0_VEC_REG	(MPIC_CCSROFF + 0x10000)/* Ext IRQ0 vect/prio*/
#define MPIC_EX_INT0_DES_REG	(MPIC_CCSROFF + 0x10010)/* Ext IRQ0 Dest */
#define MPIC_EX_INT0_LEV_REG    (MPIC_CCSROFF + 0x10018)/* Ext IRQ0 Level*/
#define MPIC_EX_INT1_VEC_REG	(MPIC_CCSROFF + 0x10020)/* Ext IRQ1 vect/prio*/
#define MPIC_EX_INT1_DES_REG	(MPIC_CCSROFF + 0x10030)/* Ext IRQ1 Dest */
#define MPIC_EX_INT1_LEV_REG    (MPIC_CCSROFF + 0x10038)/* Ext IRQ1 Level*/
#define MPIC_EX_INT2_VEC_REG	(MPIC_CCSROFF + 0x10040)/* Ext IRQ2 vect/prio*/
#define MPIC_EX_INT2_DES_REG	(MPIC_CCSROFF + 0x10050)/* Ext IRQ2 Dest */
#define MPIC_EX_INT2_LEV_REG    (MPIC_CCSROFF + 0x10058)/* Ext IRQ2 Level*/
#define MPIC_EX_INT3_VEC_REG	(MPIC_CCSROFF + 0x10060)/* Ext IRQ3 vect/prio*/
#define MPIC_EX_INT3_DES_REG	(MPIC_CCSROFF + 0x10070)/* Ext IRQ3 Dest */
#define MPIC_EX_INT3_LEV_REG    (MPIC_CCSROFF + 0x10078)/* Ext IRQ3 Level*/
#define MPIC_EX_INT4_VEC_REG	(MPIC_CCSROFF + 0x10080)/* Ext IRQ4 vect/prio*/
#define MPIC_EX_INT4_DES_REG	(MPIC_CCSROFF + 0x10090)/* Ext IRQ4 Dest */
#define MPIC_EX_INT4_LEV_REG    (MPIC_CCSROFF + 0x10098)/* Ext IRQ4 Level*/
#define MPIC_EX_INT5_VEC_REG	(MPIC_CCSROFF + 0x100a0)/* Ext IRQ5 vect/prio*/
#define MPIC_EX_INT5_DES_REG	(MPIC_CCSROFF + 0x100b0)/* Ext IRQ5 Dest */
#define MPIC_EX_INT5_LEV_REG    (MPIC_CCSROFF + 0x100b8)/* Ext IRQ5 Level*/
#define MPIC_EX_INT6_VEC_REG	(MPIC_CCSROFF + 0x100c0)/* Ext IRQ6 vect/prio*/
#define MPIC_EX_INT6_DES_REG	(MPIC_CCSROFF + 0x100d0)/* Ext IRQ6 Dest */
#define MPIC_EX_INT6_LEV_REG    (MPIC_CCSROFF + 0x100d8)/* Ext IRQ6 Level*/
#define MPIC_EX_INT7_VEC_REG	(MPIC_CCSROFF + 0x100e0)/* Ext IRQ7 vect/prio*/
#define MPIC_EX_INT7_DES_REG	(MPIC_CCSROFF + 0x100f0)/* Ext IRQ7 Dest */
#define MPIC_EX_INT7_LEV_REG    (MPIC_CCSROFF + 0x100f8)/* Ext IRQ7 Level*/
#define MPIC_EX_INT8_VEC_REG	(MPIC_CCSROFF + 0x10100)/* Ext IRQ8 vect/prio*/
#define MPIC_EX_INT8_DES_REG	(MPIC_CCSROFF + 0x10110)/* Ext IRQ8 Dest */
#define MPIC_EX_INT8_LEV_REG    (MPIC_CCSROFF + 0x10118)/* Ext IRQ8 Level*/
#define MPIC_EX_INT9_VEC_REG	(MPIC_CCSROFF + 0x10120)/* Ext IRQ9 vect/prio*/
#define MPIC_EX_INT9_DES_REG	(MPIC_CCSROFF + 0x10130)/* Ext IRQ9 Dest */
#define MPIC_EX_INT9_LEV_REG    (MPIC_CCSROFF + 0x10138)/* Ext IRQ9 Level*/
#define MPIC_EX_INT10_VEC_REG	(MPIC_CCSROFF + 0x10140)/* Ext IRQ10 vect/pri*/
#define MPIC_EX_INT10_DES_REG	(MPIC_CCSROFF + 0x10150)/* Ext IRQ10 Dest */
#define MPIC_EX_INT10_LEV_REG   (MPIC_CCSROFF + 0x10158)/* Ext IRQ10 Level*/
#define MPIC_EX_INT11_VEC_REG	(MPIC_CCSROFF + 0x10160)/* Ext IRQ11 vect/pri*/
#define MPIC_EX_INT11_DES_REG	(MPIC_CCSROFF + 0x10170)/* Ext IRQ11 Dest */
#define MPIC_EX_INT11_LEV_REG   (MPIC_CCSROFF + 0x10178)/* Ext IRQ11 Level*/

/* Internal Interrupt Source Config*/
#define MPIC_IN_INT0_VEC_REG	(MPIC_CCSROFF + 0x10200)/* Int IRQ0 vect/prio*/
#define MPIC_IN_INT0_DES_REG	(MPIC_CCSROFF + 0x10210)/* Int IRQ0 Dest */
#define MPIC_IN_INT0_LEV_REG    (MPIC_CCSROFF + 0x10218)/* Int IRQ0 Level*/

#define MPIC_IN_INT_VEC_REG(IntNum)  (MPIC_CCSROFF + 0x10200 +(0x20*IntNum))  /* Int IRQ vect/prio*/
#define MPIC_IN_INT_DES_REG(IntNum)	 (MPIC_CCSROFF + 0x10210 +(0x20*IntNum))  /* Int IRQ Dest */
#define MPIC_IN_INT_LEV_REG(IntNum)  (MPIC_CCSROFF + 0x10218 +(0x20*IntNum))  /* Int IRQ Level*/

/*   ...   */
/*  Message Interrupt Source Config  */
#define MPIC_MSG_INTA0_VEC_REG	(MPIC_CCSROFF + 0x11600)/* MSG INT0 vect/prio*/
#define MPIC_MSG_INTA0_DES_REG	(MPIC_CCSROFF + 0x11610)/* MSG INT0 Dest  */
#define MPIC_MSG_INTA1_VEC_REG	(MPIC_CCSROFF + 0x11620)/* MSG INT1 vect/prio*/
#define MPIC_MSG_INTA1_DES_REG	(MPIC_CCSROFF + 0x11630)/* MSG INT1 Dest  */
#define MPIC_MSG_INTA2_VEC_REG	(MPIC_CCSROFF + 0x11640)/* MSG INT2 vect/prio*/
#define MPIC_MSG_INTA2_DES_REG	(MPIC_CCSROFF + 0x11650)/* MSG INT2 Dest  */
#define MPIC_MSG_INTA3_VEC_REG	(MPIC_CCSROFF + 0x11660)/* MSG INT3 vect/prio*/
#define MPIC_MSG_INTA3_DES_REG	(MPIC_CCSROFF + 0x11670)/* MSG INT3 Dest  */

#define MPIC_MSG_INTB0_VEC_REG	(MPIC_CCSROFF + 0x11680)/* MSG INT0 vect/prio*/
#define MPIC_MSG_INTB0_DES_REG	(MPIC_CCSROFF + 0x11690)/* MSG INT0 Dest  */
#define MPIC_MSG_INTB1_VEC_REG	(MPIC_CCSROFF + 0x116A0)/* MSG INT1 vect/prio*/
#define MPIC_MSG_INTB1_DES_REG	(MPIC_CCSROFF + 0x116B0)/* MSG INT1 Dest  */
#define MPIC_MSG_INTB2_VEC_REG	(MPIC_CCSROFF + 0x116C0)/* MSG INT2 vect/prio*/
#define MPIC_MSG_INTB2_DES_REG	(MPIC_CCSROFF + 0x116D0)/* MSG INT2 Dest  */
#define MPIC_MSG_INTB3_VEC_REG	(MPIC_CCSROFF + 0x116E0)/* MSG INT3 vect/prio*/
#define MPIC_MSG_INTB3_DES_REG	(MPIC_CCSROFF + 0x116F0)/* MSG INT3 Dest  */

/*  Share Message Interrupt Source Config  */
#define MPIC_SMSG_INT0_VEC_REG	(MPIC_CCSROFF + 0x11c00)/* MSG INT0 vect/prio*/
#define MPIC_SMSG_INT0_DES_REG	(MPIC_CCSROFF + 0x11c10)/* MSG INT0 Dest  */
#define MPIC_SMSG_INT1_VEC_REG	(MPIC_CCSROFF + 0x11c20)/* MSG INT1 vect/prio*/
#define MPIC_SMSG_INT1_DES_REG	(MPIC_CCSROFF + 0x11c30)/* MSG INT1 Dest  */
#define MPIC_SMSG_INT2_VEC_REG	(MPIC_CCSROFF + 0x11c40)/* MSG INT2 vect/prio*/
#define MPIC_SMSG_INT2_DES_REG	(MPIC_CCSROFF + 0x11c50)/* MSG INT2 Dest  */
#define MPIC_SMSG_INT3_VEC_REG	(MPIC_CCSROFF + 0x11c60)/* MSG INT3 vect/prio*/
#define MPIC_SMSG_INT3_DES_REG	(MPIC_CCSROFF + 0x11c70)/* MSG INT3 Dest  */
#define MPIC_SMSG_INT4_VEC_REG	(MPIC_CCSROFF + 0x11c80)/* MSG INT4 vect/prio*/
#define MPIC_SMSG_INT4_DES_REG	(MPIC_CCSROFF + 0x11c90)/* MSG INT4 Dest  */
#define MPIC_SMSG_INT5_VEC_REG	(MPIC_CCSROFF + 0x11ca0)/* MSG INT5 vect/prio*/
#define MPIC_SMSG_INT5_DES_REG	(MPIC_CCSROFF + 0x11cb0)/* MSG INT5 Dest  */
#define MPIC_SMSG_INT6_VEC_REG	(MPIC_CCSROFF + 0x11cc0)/* MSG INT6 vect/prio*/
#define MPIC_SMSG_INT6_DES_REG	(MPIC_CCSROFF + 0x11cd0)/* MSG INT6 Dest  */
#define MPIC_SMSG_INT7_VEC_REG	(MPIC_CCSROFF + 0x11ce0)/* MSG INT7 vect/prio*/
#define MPIC_SMSG_INT7_DES_REG	(MPIC_CCSROFF + 0x11cf0)/* MSG INT7 Dest  */


/* other */
#define MPIC_IRQ_SUMM_REG0	(MPIC_CCSROFF + 0x01310)/* IRQ_OUT Summary 0 */
#define MPIC_IRQ_SUMM_REG1	(MPIC_CCSROFF + 0x01320)/* IRQ_OUT Summary 1 */
#define MPIC_CRIT_SUMM_REG0	(MPIC_CCSROFF + 0x01330)/* Crit Int Summary 0 */
#define MPIC_CRIT_SUMM_REG1	(MPIC_CCSROFF + 0x01340)/* Crit Int Summary 1 */
#define MPIC_PERFMON_0_MSK_REG0	(MPIC_CCSROFF + 0x01350)/* PerfMon 0 Mask 0 */
#define MPIC_PERFMON_0_MSK_REG1	(MPIC_CCSROFF + 0x01360)/* PerfMon 0 Mask 1 */
#define MPIC_PERFMON_1_MSK_REG0	(MPIC_CCSROFF + 0x01370)/* PerfMon 1 Mask 0 */
#define MPIC_PERFMON_1_MSK_REG1	(MPIC_CCSROFF + 0x01380)/* PerfMon 1 Mask 1 */
#define MPIC_PERFMON_2_MSK_REG0	(MPIC_CCSROFF + 0x01390)/* PerfMon 2 Mask 0 */
#define MPIC_PERFMON_2_MSK_REG1	(MPIC_CCSROFF + 0x013a0)/* PerfMon 2 Mask 1 */
#define MPIC_PERFMON_3_MSK_REG0	(MPIC_CCSROFF + 0x013b0)/* PerfMon 3 Mask 0 */
#define MPIC_PERFMON_3_MSK_REG1	(MPIC_CCSROFF + 0x013c0)/* PerfMon 3 Mask 1 */



/*偏移计算 */
#define MPIC_EX_VEC_REG_INTERVAL	0x20	/* ex vector regs distance */
#define MPIC_IN_VEC_REG_INTERVAL	0x20	/* in vector regs distance */
#define MPIC_GT_VEC_REG_INTERVAL	0x40	/* tm vector regs distance */
#define MPIC_MSG_VEC_REG_INTERVAL	0x20	/* msg vector regs distance */
#define MPIC_SMSG_VEC_REG_INTERVAL	0x20	/* shared msg vector regs distance */
#define MPIC_IPI_VEC_REG_INTERVAL	0x10	/* ipi vector regs distance */

#define MPIC_EX_DEST_REG_VECREGOFF	0x10	/* EIDR offset from vec reg */
#define MPIC_IN_DEST_REG_VECREGOFF	0x10	/* IIDR offset from vec reg */
#define MPIC_MSG_DEST_REG_VECREGOFF	0x10	/* MIDR offset from vec reg */
#define MPIC_GT_DEST_REG_VECREGOFF	0x10	/* GTIDR offset from vec reg */
#define MPIC_SMSG_DEST_REG_VECREGOFF	0x10	/* MSIDR offset from vec reg */

#define MPIC_EX_LEVL_REG_VECREGOFF	0x18	/* EIDR offset from vec reg */
#define MPIC_IN_LEVL_REG_VECREGOFF	0x18	/* IIDR offset from vec reg */

/*读取中断源的向量\优先级配置寄存器*/
#define MPIC_EX_VEC_REG(irq)     (MPIC_EX_INT0_VEC_REG + \
                                 ((irq) * MPIC_EX_VEC_REG_INTERVAL))
#define MPIC_IN_VEC_REG(irq)     (MPIC_IN_INT0_VEC_REG + \
                                 ((irq) * MPIC_IN_VEC_REG_INTERVAL))
#define MPIC_GTA_VEC_REG(irq)     (MPIC_TMA0_VEC_REG + \
                                 ((irq) * MPIC_GT_VEC_REG_INTERVAL))
#define MPIC_GTB_VEC_REG(irq)     (MPIC_TMB0_VEC_REG + \
                                 ((irq) * MPIC_GT_VEC_REG_INTERVAL))
#define MPIC_MSGA_VEC_REG(irq)    (MPIC_MSG_INTA0_VEC_REG + \
                                 ((irq) * MPIC_MSG_VEC_REG_INTERVAL))
#define MPIC_MSGB_VEC_REG(irq)    (MPIC_MSG_INTB0_VEC_REG + \
                                 ((irq) * MPIC_MSG_VEC_REG_INTERVAL))
#define MPIC_SMSG_VEC_REG(irq)    (MPIC_SMSG_INT0_VEC_REG + \
                                 ((irq) * MPIC_SMSG_VEC_REG_INTERVAL))
#define MPIC_IPI_VEC_REG(irq)    (MPIC_IPI_0_VEC_REG + \
                                 ((irq) * MPIC_IPI_VEC_REG_INTERVAL))
/*读取中断源的目的配置寄存器*/
#define MPIC_EX_DEST_REG(irq)    (MPIC_EX_VEC_REG(irq) + \
                                  MPIC_EX_DEST_REG_VECREGOFF)
#define MPIC_IN_DEST_REG(irq)    (MPIC_IN_VEC_REG(irq) + \
                                  MPIC_IN_DEST_REG_VECREGOFF)
#define MPIC_MSGA_DEST_REG(irq)   (MPIC_MSGA_VEC_REG(irq) + \
                                  MPIC_MSG_DEST_REG_VECREGOFF)
#define MPIC_MSGB_DEST_REG(irq)   (MPIC_MSGB_VEC_REG(irq) + \
                                  MPIC_MSG_DEST_REG_VECREGOFF)
#define MPIC_SMSG_DEST_REG(irq)   (MPIC_SMSG_VEC_REG(irq) + \
                                  MPIC_SMSG_DEST_REG_VECREGOFF)
#define MPIC_GTA_DEST_REG(irq)    (MPIC_GTA_VEC_REG(irq)+ \
	                              MPIC_GT_DEST_REG_VECREGOFF)
#define MPIC_GTB_DEST_REG(irq)    (MPIC_GTB_VEC_REG(irq)+ \
	                              MPIC_GT_DEST_REG_VECREGOFF)
#define MPIC_IPI_DEST_REG(cpuid,irq) (MPIC_IPI_DPATCH_REG0(cpuid)+irq*0x10)
/*读取中断源的中断连接方式寄存器*/
#define MPIC_EX_LEVL_REG(irq)    (MPIC_EX_VEC_REG(irq) + \
                                  MPIC_EX_LEVL_REG_VECREGOFF)
#define MPIC_IN_LEVL_REG(irq)    (MPIC_IN_VEC_REG(irq) + \
                                  MPIC_IN_LEVL_REG_VECREGOFF)        

/* 中断源类型码*/
#define MPIC_IN_INTERRUPT 	20	/* internal type */
#define MPIC_EX_INTERRUPT 	21	/* external type */
#define MPIC_INV_INTER_SOURCE 	22	/* invalid interrupt source */
#define MPIC_GT_INTERRUPT 	23	/* global timer type */
#define MPIC_MSG_INTERRUPT 	24	/* message type */
#define MPIC_IPI_INTERRUPT 	25	/* inter-processor type */
#define MPIC_VEC_HAS_NO_IDR     26	/* vector has no IDR reg */
#define MPIC_VEC_OPTION_NA      27	/* option(s) not avail for this vec */
#define MPIC_VEC_OPTION_INV     28	/* option mask is invalid */
#define MPIC_SMSG_INTERRUPT 	29	/* shared message type */


#define MPIC_MAX_EXT_IRQS	12
#define MPIC_MAX_IN_IRQS	108
#define MPIC_MAX_GTA_IRQS	4
#define MPIC_MAX_GTB_IRQS	4

#define MPIC_MAX_MSGA_IRQS	4
#define MPIC_MAX_MSGB_IRQS	4

#define MPIC_MAX_IPI_IRQS	4
#define MPIC_MAX_SMSG_IRQS	32

#define MPIC_IN_IRQS_OFFSET  16 /*LINUX下内部中断源相对硬件中断号偏移16*/

#define MPIC_IN_IRQS_MAX   (MPIC_MAX_IN_IRQS + MPIC_IN_IRQS_OFFSET) /*124*/



#define MPIC_VEC_EXT_IRQ0       0
#define MPIC_VEC_IN_IRQ0        (MPIC_VEC_EXT_IRQ0 + MPIC_IN_IRQS_OFFSET)/*16*/

#define MPIC_VEC_SMSG_IRQ0      (MPIC_VEC_MSGA_IRQ0 - MPIC_MAX_SMSG_IRQS)/*203*/
#define MPIC_VEC_MSGA_IRQ0      (MPIC_VEC_MSGB_IRQ0 - MPIC_MAX_GTA_IRQS)/*235*/
#define MPIC_VEC_MSGB_IRQ0      (MPIC_VEC_GTA_IRQ0 - MPIC_MAX_MSGB_IRQS)/*239*/
#define MPIC_VEC_GTA_IRQ0       (MPIC_VEC_GTB_IRQ0 - MPIC_MAX_GTA_IRQS)/*243*/
#define MPIC_VEC_GTB_IRQ0       (MPIC_VEC_IPI_IRQ0 - MPIC_MAX_GTB_IRQS)/*247*/
#define MPIC_VEC_IPI_IRQ0       (MPIC_VEC_SPUR_IRQ - MPIC_MAX_IPI_IRQS)/*251*/
#define MPIC_VEC_SPUR_IRQ       255

#define BIT(x)          (1 << (x))




/* GCR register */

#define MPIC_GCR_RESET		BIT(31)
#define MPIC_GCR_MODE_MIXED	BIT(29)
#define MPIC_GCR_MODE_EPF   (BIT(29) | BIT(30))

/* IPI Vector/Priority registers */

#define MPIC_IPIVPR_INTR_MSK        BIT(31)
#define MPIC_IPIVPR_INTR_ACTIVE     BIT(30)
#define MPIC_IPIVPR_PRIORITY_MSK    (BIT(19) | BIT(18) | BIT(17) | BIT(16))
#define MPIC_IPIVPR_PRIORITY(p)     (((p) << 16) & MPIC_IPIVPR_PRIORITY_MSK)
#define MPIC_IPIVPR_PRIORITY_GET(p) (((p) & MPIC_IPIVPR_PRIORITY_MSK) >> 16)
#define MPIC_IPIVPR_VECTOR_MSK      (0xffff)
#define MPIC_IPIVPR_VECTOR(vec)     ((vec) & MPIC_IPIVPR_VECTOR_MSK)

/* Global Timer Vector/Priority registers */

#define MPIC_GTVPR_INTR_MSK        BIT(31)
#define MPIC_GTVPR_INTR_ACTIVE     BIT(30)
#define MPIC_GTVPR_PRIORITY_MSK    (BIT(19) | BIT(18) | BIT(17) | BIT(16))
#define MPIC_GTVPR_PRIORITY(p)     (((p) << 16) & MPIC_GTVPR_PRIORITY_MSK)
#define MPIC_GTVPR_VECTOR_MSK      (0xffff)
#define MPIC_GTVPR_VECTOR(vec)     ((vec) & MPIC_GTVPR_VECTOR_MSK)

/* Summary registers */

#define MPIC_IRQSR0_MSG_INT_MSK 0xf000
#define MPIC_IRQSR0_MSG_INT(n)  (BIT(15-(n)) & MPIC_IRQSR0_MSG_INT_MSK)
#define MPIC_IRQSR0_EX_INT_MSK  0xfff
#define MPIC_IRQSR0_EX_INT(n)   (BIT(11-(n)) & MPIC_IRQSR0_EX_INT_MSK)
#define MPIC_IRQSR1_IN_INT(n)   BIT(31-(n))
#define MPIC_CISR0_MSG_INT_MSK  0xf000
#define MPIC_CISR0_MSG_INT(n)   (BIT(15-(n)) & MPIC_CISR0_MSG_INT_MSK)
#define MPIC_CISR0_EX_INT_MSK   0xfff
#define MPIC_CISR0_EX_INT(n)    (BIT(11-(n)) & MPIC_CISR0_EX_INT_MSK)
#define MPIC_CISR1_IN_INT(n)    BIT(31-(n))

/* Message registers */

#define MPIC_MER_EN_MSK         0xf
#define MPIC_MER_EN(n)          (BIT(n) & MPIC_MER_EN_MSK)
#define MPIC_MSR_ST_MSK         0xf
#define MPIC_MSR_ST(n)          (BIT(n) & MPIC_MER_ST_MSK)

/* EIVPR registers */

#define MPIC_EIVPR_INTR_MSK         BIT(31)
#define MPIC_EIVPR_INTR_ACTIVE      BIT(30)
#define MPIC_EIVPR_INTR_POLARITY    BIT(23)
#define MPIC_EIVPR_INTR_SENSE       BIT(22)
#define MPIC_EIVPR_POLARITY(p)      ((p) << 23)
#define MPIC_EIVPR_SENS(s)          ((s) << 22)
#define MPIC_EIVPR_PRIORITY_MSK	    (BIT(19) | BIT(18) | BIT(17) | BIT(16))
#define MPIC_EIVPR_PRIORITY(p) 	    (((p) << 16) & MPIC_EIVPR_PRIORITY_MSK)
#define MPIC_EIVPR_VECTOR_MSK       (0xffff)
#define MPIC_EIVPR_VECTOR(vec) 	    ((vec) & MPIC_EIVPR_VECTOR_MSK)


#define MPIC_INT_ACT_LOW            0
#define MPIC_INT_ACT_HIGH           1
#define MPIC_INT_EDG_NEG            0
#define MPIC_INT_EDG_POS            1
#define MPIC_SENSE_LVL              1
#define MPIC_SENSE_EDG              0

/* EIDR registers */

#define MPIC_EIDR_EX_PIN        BIT(31)
#define MPIC_EIDR_CRIT_INT      BIT(30)
#define MPIC_EIDR_CRIT0_INT      BIT(30)
#define MPIC_EIDR_CRIT1_INT      BIT(29)
#define MPIC_EIDR_P1_INT     BIT(1)
#define MPIC_EIDR_P0_INT     BIT(0)

/* Options for *VPR and *IDR registers */

#define MPIC_OPT_EN_MSK             MPIC_EIVPR_INTR_MSK
#define MPIC_OPT_EN_Y               0x00000000
#define MPIC_OPT_EN_N               0x10000000
#define MPIC_OPT_POLAR_MSK          MPIC_EIVPR_INTR_POLARITY
#define MPIC_OPT_POLAR_ACT_LOW      0x00000000
#define MPIC_OPT_POLAR_ACT_HIGH     0x00800000
#define MPIC_OPT_POLAR_EDG_NEG      0x00000000
#define MPIC_OPT_POLAR_EDG_POS      0x00800000
#define MPIC_OPT_SENSE_MSK          MPIC_EIVPR_INTR_SENSE
#define MPIC_OPT_SENSE_EDG          0x00000000
#define MPIC_OPT_SENSE_LVL          0x00400000
#define MPIC_OPT_PRI_MSK            MPIC_EIVPR_PRIORITY_MSK
#define MPIC_OPT_PRI_VALUE(p)       MPIC_EIVPR_PRIORITY(p)
#define MPIC_OPT_EXPIN_MSK          (MPIC_EIDR_EX_PIN >> 16)
#define MPIC_OPT_EXPIN_OFF          (0x00000000 >> 16)
#define MPIC_OPT_EXPIN_ON           (0x80000000 >> 16)
#define MPIC_OPT_CRIT_MSK           (MPIC_EIDR_CRIT_INT >> 16)
#define MPIC_OPT_CRIT_OFF           (0x00000000 >> 16)
#define MPIC_OPT_CRIT_ON            (0x40000000 >> 16)

/* IIVPR registers */

#define MPIC_IIVPR_INTR_MSK         BIT(31)
#define MPIC_IIVPR_INTR_ACTIVE      BIT(30)
#define MPIC_IIVPR_INTR_POLARITY    BIT(23)
#define MPIC_IIVPR_POLARITY(p)      ((p) << 23)
#define MPIC_IIVPR_PRIORITY_MSK     (BIT(19) | BIT(18) | BIT(17) | BIT(16))
#define MPIC_IIVPR_PRIORITY(p)      (((p) << 16) & MPIC_IIVPR_PRIORITY_MSK)
#define MPIC_IIVPR_VECTOR_MSK       (0xffff)
#define MPIC_IIVPR_VECTOR(vec)      ((vec) & MPIC_IIVPR_VECTOR_MSK)

/* IIDR registers */

#define MPIC_IIDR_EX_PIN        BIT(31)
#define MPIC_IIDR_CRIT_INT      BIT(30)
#define MPIC_IIDR_CRIT0_INT      BIT(30)
#define MPIC_IIDR_CRIT1_INT      BIT(29)
#define MPIC_IIDR_P1_INT		BIT(1)
#define MPIC_IIDR_P0_INT		BIT(0)

/* MIVPR registers */

#define MPIC_MIVPR_INTR_MSK         BIT(31)
#define MPIC_MIVPR_INTR_ACTIVE      BIT(30)
#define MPIC_MIVPR_PRIORITY_MSK     (BIT(19) | BIT(18) | BIT(17) | BIT(16))
#define MPIC_MIVPR_PRIORITY(p)      (((p) << 16) & MPIC_MIVPR_PRIORITY_MSK)
#define MPIC_MIVPR_VECTOR_MSK       (0xffff)
#define MPIC_MIVPR_VECTOR(vec)      ((vec) & MPIC_MIVPR_VECTOR_MSK)

/* MSIVPR registers */

#define MPIC_MSIVPR_INTR_MSK         BIT(31)
#define MPIC_MSIVPR_INTR_ACTIVE      BIT(30)
#define MPIC_MSIVPR_PRIORITY_MSK     (BIT(19) | BIT(18) | BIT(17) | BIT(16))
#define MPIC_MSIVPR_PRIORITY(p)      (((p) << 16) & MPIC_MSIVPR_PRIORITY_MSK)
#define MPIC_MSIVPR_VECTOR_MSK       (0xffff)
#define MPIC_MSIVPR_VECTOR(vec)      ((vec) & MPIC_MSIVPR_VECTOR_MSK)

/* MIDR registers */

#define MPIC_MIDR_EX_PIN        BIT(31)
#define MPIC_MIDR_CRIT_INT      BIT(30)
#define MPIC_MIDR_CRIT0_INT      BIT(30)
#define MPIC_MIDR_CRIT1_INT      BIT(29)
#define MPIC_MIDR_P1_INT      BIT(1)
#define MPIC_MIDR_P0_INT      BIT(0)


/* MSIDR registers */

#define MPIC_MSIDR_EX_PIN        BIT(31)
#define MPIC_MSIDR_CRIT_INT      BIT(30)
#define MPIC_MSIDR_CRIT0_INT      BIT(30)
#define MPIC_MSIDR_CRIT1_INT      BIT(29)
#define MPIC_MSIDR_P1_INT      BIT(1)
#define MPIC_MSIDR_P0_INT      BIT(0)


/* IPIDR registers */

#define MPIC_IPIDR_P0           BIT(0)
#define MPIC_IPIDR_P1           BIT(1)

/* CTPR register */

#define MPIC_CTPR_TASKPRI_MSK   (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define MPIC_CTPR_TASKPRI(p)    ((p) & MPIC_CTPR_TASKPRI_MSK)

/* WHOAMI register */

#define MPIC_WHOAMI_ID_MSK      (BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define MPIC_WHOAMI_ID(n)       ((n) & MPIC_WHOAMI_ID_MSK)

/* FRR (feature reporting register) */
#define MPIC_FRR_NCPU_MASK      0x00001f00
#define MPIC_FRR_NCPU_SHIFT	8



#define MPIC_PRIORITY_MIN	0    /* minimum level of priority */
#define MPIC_PRIORITY_MAX	15   /* maximum level of priority */
#define MPIC_PRIORITY_DEFAULT	10
#define MPIC_INV_PRIO_ERROR	((ULONG)(-1))
#define MPIC_INTER_IN_SERVICE 4

/*#define BSP_LINUX_CORE*/

/*#define P3_MAX_CORE_NUM  (IsBstqrBoard()? 4:8)*/
#define MASTER_CORE   0x1
 

 

#define MPIC_INTTGT_INT   0x00
#define MPIC_INTTGT_CINT   0x01
#define MPIC_INTTGT_MCP   0x02

#define MPIC_INTTGT_SIE0   0xF0
#define MPIC_INTTGT_SIE1   0xF1
#define MPIC_INTTGT_SIE2   0xF2
#define MPIC_INTTGT_IRQ_OUT   0xFF

#define MPIC_RISING_EDGE  0x10
#define MPIC_FALLING_EDGE 0x00
#define MPIC_HIGH_LEVEL 0x11
#define MPIC_LOW_LEVEL 0x10
#define     MPIC_REG_READ(reg,value) \
             (value) = (*(volatile int*)(reg+g_u8ccsbar))

#define     MPIC_REG_WRITE(reg,value) \
             (*(volatile int*)(reg+g_u8ccsbar)) = ((unsigned int)value)
             



#endif /* BSP_H */
/******************************* 头文件结束 ********************************/

