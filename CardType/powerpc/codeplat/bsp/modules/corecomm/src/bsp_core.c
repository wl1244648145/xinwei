/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_core.c
* 功能:
* 版本:
* 编制日期:
* 作者:                    liugang
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include "fcntl.h"
#include <sys/ioctl.h>
#include <sched.h>
/**************************** 私用头文件* **********************************/
#include "../inc/bsp_core.h"
#include "bsp_types.h"
#include "../../interrupt/inc/bsp_interrupt.h"
#include "../../usdpaa/inc/compat.h"
#include "../../ms/inc/bsp_ms.h"
#include "../../hmi/inc/hmi.h"
#include "../../i2c/inc/bsp_i2c.h"

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <net/if.h>
#include <string.h>
#include <pthread.h>
#include <time.h>


/******************************* 局部宏定义 *********************************/
#define IA_DEVICE_NAME   "/dev/IntAgent"
#define MEM_DEVICE_NAME  "/dev/mem"
#define IA_MAJOR           (10)
#define IA_MINOR           (244)
#define IA_SUCCESS         (0)
#define IA_ERROR           (1)
#define IA_TIMEOUT         (2)
#define IA_SIGOUT          (3)
#define IA_FAULT           (4)
#define IA_OBJ_DELETE      (5)
#define IA_CMD_MAGIC       (0x44410000)
#define _IA_CMD(magic,num) (IA_CMD_MAGIC + num)
#define IA_NR_IRQS         (1000)

/*********************** 全局变量定义/初始化 **************************/
UINT32 g_i2c_irq_print = 0;
UINT32 g_i2c_irq_count = 0;
sem_t g_gps_1s_sem;
sem_t g_gps_1s_sem1;
T_MPIC_VECS *pMpic_vecs = NULL;
INT_HANDLER_DESC * pcIrqInfoTbl [INTERRUPT_TABLESIZE];
ULONG g_ulTestIrq          = 0;
ULONG g_ulTestMsgFlag      = 0;
ULONG g_ulTestMsg[8]       = {0};
ULONG g_ulTestIpi[4]       = {0};
ULONG g_ulTestExtInt[12]   = {0};
ULONG g_ulCurCpuId         = 0;
ULONG gIntCfgTblItemCnts   = 0;
ULONG gulMsterCoreGpioMask = 0;
T_InterCoreIntRecord gtInterCoreIntRecord;
int    g_iIaDrvFd;
int    siMemDrvFd;
int    g_intflag=0;


/************************** 局部常数和类型定义 ************************/
static ULONG ulStartTime = 0;
static ULONG ulEndTime   = 0;
static ULONG ulTimeCnt   = 0;

typedef struct
{
    ULONG dev_id;
    ULONG rs;
    ULONG rv;
    ULONG d0;
    ULONG d1;
    ULONG d2;
    ULONG d3;
    ULONG dx[17];
} ia_ioctl_t;

enum
{
    IA_INTERRUPT_REQUEST=0,
    IA_INTERRUPT_FREE,
    IA_INTERRUPT_ENABLE,
    IA_INTERRUPT_DISABLE,
    IA_WAIT_FOR_INTERRUPT,
    IA_INTERRUPT_LOCK,
    IA_INTERRUPT_UNLOCK,
    IA_GET_SCHE_TIME,
    IA_DEV_MUX_INIT,
    IA_DEV_MUX_BIND,
    IA_DEV_MUX_RCV,
    IA_DEV_MUX_TX,
    IA_DEV_MUX_FREE,
    IA_DEV_MUX_SEND,
    IA_MUX_TX_ALLOC,
    IA_MUX_RX_START,
    IA_MUX_TX_FREE,
    IA_MUX_RX_MMAP,
    IA_WRITE_FLASH,
    IA_FLASH_MUTEX_DISABLE,
    IA_FLASH_MUTEX_ENABLE,
    IA_GET_INT_MEM,
    IA_FREE_INT_MEM,
    IA_SET_GPIO_MASK,
    IA_TEST_INFO,
    IA_INIT_GPIO_RESEND,
    IA_REINIT_GPIO_MASK,
};

typedef struct __tag_Act_Irq
{
    ULONG   IrqCnt;
    ULONG   IrqStat;
    ULONG   IrqReserved[5];
    ULONG  IrqNumArray[IA_NR_IRQS];

} ia_act_irq_t;

static ULONG ia_mult_int_mem_addr = 0;

typedef void (*ia_irq_func_t)(void*);

typedef struct
{
    void *data;
    ia_act_irq_t      *s_tActIrq;
} ia_handler_parm_t;

typedef struct
{
    ia_handler_parm_t handler_parm_t;
    ia_irq_func_t handler;
    UINT32 vector;
} ia_irq_handler_t;

ia_irq_handler_t saIaIrqHandlerArray[IA_NR_IRQS];

/************************* 局部函数原型声明 **************************/
int   BspMpicSelfVecsInit(void);
void  BspTestEIrqIsr(ULONG ulVector);
ULONG BspMpicDstCpuSet(ULONG ulVector,ULONG ulCpuId);
ULONG BspMpicLevSet(ULONG ulVector,ULONG ulLev);
void  BspShowIrqTestInfo(UCHAR ucFlag);
//ULONG BspTestMsgIsr(ULONG ulVector);
VOID  BspTestMsgIsr(ULONG ulVector);
ULONG BspMpicMsgStatGet(ULONG ucMsgId);
ULONG BspMsgIntSend(ULONG ulDstCoreId,UCHAR ucMsgId);
ULONG BspIpiTrig(UINT32 udDstCoreMask,UINT32 udIpiNo);
ULONG BspMpicClearMsgStat(UCHAR ucMsgId);
ULONG BspMpicMsgIntSend(unsigned char ucMsgId,unsigned char ucDstCpuId,unsigned int dwMsgVal);
VOID BspIntcDisable (const UINT32 ulIntNo);
VOID BspIntcEnable(const ULONG ulIntNo);
VOID  BspGpsIrqIsr(ULONG ulVector);
void gps_handle_task(int arg);

extern UINT32 intDisconnect(VOIDFUNCPTR * 	vector,		/* interrupt vector to attach */
                            VOIDFUNCPTR		routine,	/* routine to be called */
                            int			parameter	/* parameter to be passed to routine */
                           );
extern  int sched_setaffinity(pid_t pid, unsigned int cpusetsize, cpu_set_t *mask);
extern ULONG BspMpicTimeBaseInit(void);
extern ULONG BspGetMpicTBCurCnt(UINT32 udTimerIndex);
/*************************  函数实现    ***********************************/

/******************************************************************************
* 函数名: requestIrq
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
UINT32  requestIrq(UINT32 vector, ia_irq_func_t isr_func,void *data)
{
    ia_ioctl_t devio;
    //UINT32 ret;
    //ret = OK;
    devio.dev_id = 0;
    devio.d0 = (unsigned int)vector;
    devio.d1 = ia_mult_int_mem_addr;
    ioctl(g_iIaDrvFd, IA_INTERRUPT_REQUEST, &devio);
    if (devio.rs != IA_SUCCESS)
    {
        return ERROR;
    }
    saIaIrqHandlerArray[vector].handler = isr_func;
    saIaIrqHandlerArray[vector].handler_parm_t.data = data;
    saIaIrqHandlerArray[vector].vector  = vector;
    g_intflag = 1;
    return OK;
}

/******************************************************************************
* 函数名: iaIrqThread
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void *iaIrqThread()
{
    struct sched_param param;
    UINT32    i,curr_vec,err;
    static ia_ioctl_t     devio;
    static ia_act_irq_t   s_atActIrqNumArray;
    ULONG cpumask;
#if 0
    cpumask = 1<<1;
    if (sched_setaffinity(0, sizeof(cpumask), ( cpu_set_t *)&cpumask) < 0)
    {
        printf("pthread_setaffinity_iaIrqThread failed in %s\n", __FUNCTION__);
    }
#endif
    memset(&s_atActIrqNumArray,0,sizeof(s_atActIrqNumArray));
    param.sched_priority = 99;
    if (sched_setscheduler(0, SCHED_FIFO, &param))
    {
        perror("\ninterrupt priority set: ");
    }
    err = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (err != 0)
    {
        printf ("setcanceltype failed: %s [so you can't stop me]\n", strerror (err));
    }
    bsp_print_reg_info(__func__, __FILE__, __LINE__);
    for (;;)
    {
        devio.d0 = (ULONG)&s_atActIrqNumArray;
        devio.d1 = ia_mult_int_mem_addr;

        ioctl(g_iIaDrvFd, IA_WAIT_FOR_INTERRUPT, &devio);
        for (i=0; i<s_atActIrqNumArray.IrqCnt; i++)
        {
            curr_vec = s_atActIrqNumArray.IrqNumArray[i];
            if ((curr_vec >= 0))
            {
                if (saIaIrqHandlerArray[curr_vec].handler)
                {
                    saIaIrqHandlerArray[curr_vec].handler_parm_t.s_tActIrq = &s_atActIrqNumArray;
                    saIaIrqHandlerArray[curr_vec].handler(saIaIrqHandlerArray[curr_vec].handler_parm_t.data);
                }
            }
            else
            {

            }
        }
    }
}

/******************************************************************************
* 函数名: InteruptInitExt
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
UINT32 InteruptInitExt( )
{
    pthread_t ptid1;
    ia_ioctl_t     devio;
    UINT32 intTreadId;
    memset(saIaIrqHandlerArray,0,sizeof(saIaIrqHandlerArray));
    system("mknod /dev/IntAgent c 10 244");
    if ((g_iIaDrvFd = open(IA_DEVICE_NAME,O_RDWR | O_SYNC )) < 0)
    {
        perror("open " IA_DEVICE_NAME ": ");
        return ERROR;
    }
    memset(&devio,0,sizeof(ia_ioctl_t));
    ioctl(g_iIaDrvFd, IA_GET_INT_MEM, &devio);
    if (devio.rs == 0)
    {
        printf("IA_GET_INT_MEM ###########failed\n");
        return ERROR;
    }
    ia_mult_int_mem_addr = devio.rs;
    intTreadId = pthread_create(&ptid1, NULL, (void*)iaIrqThread, NULL);
    if ( intTreadId == ERROR )
    {
        close(g_iIaDrvFd);
        return ERROR;
    }
    return OK;
}

/******************************************************************************
* 函数名: BspGetCurCpu
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
UINT32 BspGetCurCpu(void)
{
    ULONG ulRegTempVal = 0;
    ULONG ulCnt = 0;
    MPIC_REG_READ(MPIC_WHO_AM_I_PRI_CPU_REG,ulRegTempVal);
    for(ulCnt = 0; ulCnt< P3_MAX_CORE_NUM; ulCnt ++)
    {
        if(ulRegTempVal == 0)
        {
            return 0;
        }
        else if((ulRegTempVal>>ulCnt) & 0x01)
        {
            return ulCnt+1;
        }
    }
    return 0;
}
UINT32 BspMpicRegRead(UINT32 dwMpicRegOffset)
{
    UINT32 dwTemp = 0;
    UINT32 dwMpicTemp = 0;
    dwMpicTemp = (UINT32)g_u8ccsbar;
    dwTemp = *(unsigned int *) (dwMpicTemp+dwMpicRegOffset) ;
    return (dwTemp);
}
void BspMpicRegWrite(UINT32 dwRegOffset,UINT32 regVal)
{
    UINT32 dwMpicTemp = 0;
    dwMpicTemp = (UINT32)g_u8ccsbar;
    *(UINT32 *) (dwMpicTemp + dwRegOffset) = regVal;
}
/******************************************************************************
* 函数名: BspMpicSrcAddrCheck
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspMpicSrcAddrCheck(ULONG ulSourceAddr)
{
    switch (ulSourceAddr)
    {
    case MPIC_EX_INT0_VEC_REG:
    case MPIC_EX_INT1_VEC_REG:
    case MPIC_EX_INT2_VEC_REG:
    case MPIC_EX_INT3_VEC_REG:
    case MPIC_EX_INT4_VEC_REG:
    case MPIC_EX_INT5_VEC_REG:
    case MPIC_EX_INT6_VEC_REG:
    case MPIC_EX_INT7_VEC_REG:
    case MPIC_EX_INT8_VEC_REG:
    case MPIC_EX_INT9_VEC_REG:
    case MPIC_EX_INT10_VEC_REG:
    case MPIC_EX_INT11_VEC_REG:
        return (MPIC_EX_INTERRUPT);
        break;
    case MPIC_IN_INT0_VEC_REG:
    case MPIC_IN_INT_VEC_REG(1):
    case MPIC_IN_INT_VEC_REG(2):
    case MPIC_IN_INT_VEC_REG(3):
    case MPIC_IN_INT_VEC_REG(4):
    case MPIC_IN_INT_VEC_REG(5):
    case MPIC_IN_INT_VEC_REG(6):
    case MPIC_IN_INT_VEC_REG(7):
    case MPIC_IN_INT_VEC_REG(8):
    case MPIC_IN_INT_VEC_REG(9):
    case MPIC_IN_INT_VEC_REG(10):
    case MPIC_IN_INT_VEC_REG(11):
    case MPIC_IN_INT_VEC_REG(12):
    case MPIC_IN_INT_VEC_REG(13):
    case MPIC_IN_INT_VEC_REG(14):
    case MPIC_IN_INT_VEC_REG(15):
    case MPIC_IN_INT_VEC_REG(16):
    case MPIC_IN_INT_VEC_REG(17):
    case MPIC_IN_INT_VEC_REG(18):
    case MPIC_IN_INT_VEC_REG(19):
    case MPIC_IN_INT_VEC_REG(20):
    case MPIC_IN_INT_VEC_REG(21):
    case MPIC_IN_INT_VEC_REG(22):
    case MPIC_IN_INT_VEC_REG(23):
    case MPIC_IN_INT_VEC_REG(24):
    case MPIC_IN_INT_VEC_REG(25):
    case MPIC_IN_INT_VEC_REG(26):
    case MPIC_IN_INT_VEC_REG(27):
    case MPIC_IN_INT_VEC_REG(28):
    case MPIC_IN_INT_VEC_REG(29):
    case MPIC_IN_INT_VEC_REG(30):
    case MPIC_IN_INT_VEC_REG(31):
    case MPIC_IN_INT_VEC_REG(32):
    case MPIC_IN_INT_VEC_REG(33):
    case MPIC_IN_INT_VEC_REG(34):
    case MPIC_IN_INT_VEC_REG(35):
    case MPIC_IN_INT_VEC_REG(36):
    case MPIC_IN_INT_VEC_REG(37):
    case MPIC_IN_INT_VEC_REG(38):
    case MPIC_IN_INT_VEC_REG(39):
    case MPIC_IN_INT_VEC_REG(40):
    case MPIC_IN_INT_VEC_REG(41):
    case MPIC_IN_INT_VEC_REG(42):
    case MPIC_IN_INT_VEC_REG(43):
    case MPIC_IN_INT_VEC_REG(44):
    case MPIC_IN_INT_VEC_REG(45):
    case MPIC_IN_INT_VEC_REG(46):
    case MPIC_IN_INT_VEC_REG(47):
    case MPIC_IN_INT_VEC_REG(48):
    case MPIC_IN_INT_VEC_REG(49):
    case MPIC_IN_INT_VEC_REG(50):
    case MPIC_IN_INT_VEC_REG(51):
    case MPIC_IN_INT_VEC_REG(52):
    case MPIC_IN_INT_VEC_REG(53):
    case MPIC_IN_INT_VEC_REG(54):
    case MPIC_IN_INT_VEC_REG(55):
    case MPIC_IN_INT_VEC_REG(56):
    case MPIC_IN_INT_VEC_REG(57):
    case MPIC_IN_INT_VEC_REG(58):
    case MPIC_IN_INT_VEC_REG(59):
    case MPIC_IN_INT_VEC_REG(60):
    case MPIC_IN_INT_VEC_REG(61):
    case MPIC_IN_INT_VEC_REG(62):
    case MPIC_IN_INT_VEC_REG(63):
    case MPIC_IN_INT_VEC_REG(64):
    case MPIC_IN_INT_VEC_REG(65):
    case MPIC_IN_INT_VEC_REG(66):
    case MPIC_IN_INT_VEC_REG(67):
    case MPIC_IN_INT_VEC_REG(68):
    case MPIC_IN_INT_VEC_REG(69):
    case MPIC_IN_INT_VEC_REG(70):
    case MPIC_IN_INT_VEC_REG(71):
    case MPIC_IN_INT_VEC_REG(72):
    case MPIC_IN_INT_VEC_REG(73):
    case MPIC_IN_INT_VEC_REG(74):
    case MPIC_IN_INT_VEC_REG(75):
    case MPIC_IN_INT_VEC_REG(76):
    case MPIC_IN_INT_VEC_REG(77):
    case MPIC_IN_INT_VEC_REG(78):
    case MPIC_IN_INT_VEC_REG(79):
    case MPIC_IN_INT_VEC_REG(80):
    case MPIC_IN_INT_VEC_REG(81):
    case MPIC_IN_INT_VEC_REG(82):
    case MPIC_IN_INT_VEC_REG(83):
    case MPIC_IN_INT_VEC_REG(84):
    case MPIC_IN_INT_VEC_REG(85):
    case MPIC_IN_INT_VEC_REG(86):
    case MPIC_IN_INT_VEC_REG(87):
    case MPIC_IN_INT_VEC_REG(88):
    case MPIC_IN_INT_VEC_REG(89):
    case MPIC_IN_INT_VEC_REG(90):
    case MPIC_IN_INT_VEC_REG(91):
    case MPIC_IN_INT_VEC_REG(92):
    case MPIC_IN_INT_VEC_REG(93):
    case MPIC_IN_INT_VEC_REG(94):
    case MPIC_IN_INT_VEC_REG(95):
    case MPIC_IN_INT_VEC_REG(96):
    case MPIC_IN_INT_VEC_REG(97):
    case MPIC_IN_INT_VEC_REG(98):
    case MPIC_IN_INT_VEC_REG(99):
    case MPIC_IN_INT_VEC_REG(100):
    case MPIC_IN_INT_VEC_REG(101):
    case MPIC_IN_INT_VEC_REG(102):
    case MPIC_IN_INT_VEC_REG(103):
    case MPIC_IN_INT_VEC_REG(104):
    case MPIC_IN_INT_VEC_REG(105):
    case MPIC_IN_INT_VEC_REG(106):
    case MPIC_IN_INT_VEC_REG(107):
    case MPIC_IN_INT_VEC_REG(108):
    case MPIC_IN_INT_VEC_REG(109):
    case MPIC_IN_INT_VEC_REG(110):
    case MPIC_IN_INT_VEC_REG(111):
        return (MPIC_IN_INTERRUPT);
        break;
    case MPIC_TMA0_VEC_REG:
    case MPIC_TMA1_VEC_REG:
    case MPIC_TMA2_VEC_REG:
    case MPIC_TMA3_VEC_REG:
    case MPIC_TMB0_VEC_REG:
    case MPIC_TMB1_VEC_REG:
    case MPIC_TMB2_VEC_REG:
    case MPIC_TMB3_VEC_REG:
        return (MPIC_GT_INTERRUPT);
        break;
    case MPIC_MSG_INTA0_VEC_REG:
    case MPIC_MSG_INTA1_VEC_REG:
    case MPIC_MSG_INTA2_VEC_REG:
    case MPIC_MSG_INTA3_VEC_REG:
    case MPIC_MSG_INTB0_VEC_REG:
    case MPIC_MSG_INTB1_VEC_REG:
    case MPIC_MSG_INTB2_VEC_REG:
    case MPIC_MSG_INTB3_VEC_REG:
        return (MPIC_MSG_INTERRUPT);
        break;
    case MPIC_SMSG_INT0_VEC_REG:
    case MPIC_SMSG_INT1_VEC_REG:
    case MPIC_SMSG_INT2_VEC_REG:
    case MPIC_SMSG_INT3_VEC_REG:
    case MPIC_SMSG_INT4_VEC_REG:
    case MPIC_SMSG_INT5_VEC_REG:
    case MPIC_SMSG_INT6_VEC_REG:
    case MPIC_SMSG_INT7_VEC_REG:
        return (MPIC_SMSG_INTERRUPT);
        break;
    case MPIC_IPI_0_VEC_REG:
    case MPIC_IPI_1_VEC_REG:
    case MPIC_IPI_2_VEC_REG:
    case MPIC_IPI_3_VEC_REG:
        return (MPIC_IPI_INTERRUPT);
        break;
    default:
        return (MPIC_INV_INTER_SOURCE);
    }
}
/******************************************************************************
* 函数名: BspMpicGetVecRegAdrs
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspMpicGetVecRegAdrs(ULONG ulVector)
{
    if ((ulVector < MPIC_VEC_EXT_IRQ0) || (ulVector > MPIC_VEC_SPUR_IRQ))
        return (-2);

    if (ulVector < MPIC_VEC_IN_IRQ0)    /* type EXT */
    {
        return (MPIC_EX_VEC_REG (ulVector - MPIC_VEC_EXT_IRQ0));
    }

    if (ulVector < MPIC_IN_IRQS_MAX)    /* type IN */
    {
        return (MPIC_IN_VEC_REG (ulVector - MPIC_VEC_IN_IRQ0));
    }

    if (ulVector < MPIC_VEC_MSGA_IRQ0)    /* type SMSG */
    {
        return (MPIC_SMSG_VEC_REG (ulVector - MPIC_VEC_SMSG_IRQ0));
    }

    if (ulVector < MPIC_VEC_MSGB_IRQ0)    /* type MSGA */
    {
        return (MPIC_MSGA_VEC_REG (ulVector - MPIC_VEC_MSGA_IRQ0));
    }

    if (ulVector < MPIC_VEC_GTA_IRQ0)    /* type MSGB */
    {
        return (MPIC_MSGB_VEC_REG (ulVector - MPIC_VEC_MSGB_IRQ0));
    }

    if (ulVector < MPIC_VEC_GTB_IRQ0)    /* type GTA */
    {
        return (MPIC_GTA_VEC_REG (ulVector - MPIC_VEC_GTA_IRQ0));
    }

    if (ulVector < MPIC_VEC_IPI_IRQ0)    /* type GTB */
    {
        return (MPIC_GTB_VEC_REG (ulVector - MPIC_VEC_GTB_IRQ0));
    }

    if (ulVector < MPIC_VEC_SPUR_IRQ)    /* type IPI */
    {
        return (MPIC_IPI_VEC_REG (ulVector - MPIC_VEC_IPI_IRQ0));
    }

    if (ulVector == MPIC_VEC_SPUR_IRQ)   /* type SPUR */
    {
        return (MPIC_SPUR_VEC_REG);
    }
    /* should not reach here */
    return (-1);
}
/******************************************************************************
* 函数名: BspMpicGetDestRegAdrs
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspMpicGetDestRegAdrs(ULONG ulVector)
{
    ULONG ulCurCpuId = 0;
    ulCurCpuId = g_ulCurCpuId;
    if ((ulVector < MPIC_VEC_EXT_IRQ0) || (ulVector > MPIC_VEC_SPUR_IRQ))
        return (-2);
    if (ulVector < MPIC_VEC_IN_IRQ0)	/* type EXT */
    {
        return (MPIC_EX_DEST_REG (ulVector - MPIC_VEC_EXT_IRQ0));
    }
    if (ulVector < MPIC_IN_IRQS_MAX)    /* type IN */
    {
        return (MPIC_IN_DEST_REG (ulVector - MPIC_VEC_IN_IRQ0));
    }
    if (ulVector < MPIC_VEC_MSGA_IRQ0)    /* type SMSG */
    {
        return (MPIC_SMSG_DEST_REG (ulVector - MPIC_VEC_SMSG_IRQ0));
    }
    if (ulVector < MPIC_VEC_MSGB_IRQ0)    /* type MSGA */
    {
        return (MPIC_MSGA_DEST_REG (ulVector - MPIC_VEC_MSGA_IRQ0));
    }
    if (ulVector < MPIC_VEC_GTA_IRQ0)    /* type MSGB */
    {
        return (MPIC_MSGB_DEST_REG (ulVector - MPIC_VEC_MSGB_IRQ0));
    }
    if (ulVector < MPIC_VEC_GTB_IRQ0)    /* type GTA */
    {
        return (MPIC_GTA_DEST_REG (ulVector - MPIC_VEC_GTA_IRQ0));
    }
    if (ulVector < MPIC_VEC_IPI_IRQ0)    /* type GTB */
    {
        return (MPIC_GTB_DEST_REG (ulVector - MPIC_VEC_GTB_IRQ0));
    }
    if (ulVector < MPIC_VEC_SPUR_IRQ)    /* type IPI */
    {
        return (MPIC_IPI_DEST_REG (ulCurCpuId,ulVector - MPIC_VEC_IPI_IRQ0));
    }
    /* should not reach here */
    return ((ULONG) ERROR);
}

/******************************************************************************
* 函数名: BspMpicGetLevlRegAdrs
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspMpicGetLevlRegAdrs(ULONG ulVector)
{
    //ULONG ulCurCpuId = 0;

    //ulCurCpuId = g_ulCurCpuId;

    if ((ulVector < MPIC_VEC_EXT_IRQ0) || (ulVector > MPIC_VEC_SPUR_IRQ))
        return (-2);

    if (ulVector < MPIC_VEC_IN_IRQ0)	/* type EXT */
    {
        return (MPIC_EX_LEVL_REG (ulVector - MPIC_VEC_EXT_IRQ0));
    }
    if (ulVector < MPIC_IN_IRQS_MAX)    /* type IN */
    {
        return (MPIC_IN_LEVL_REG (ulVector - MPIC_VEC_IN_IRQ0));
    }
    /* should not reach here */
    return ((ULONG) ERROR);
}

/******************************************************************************
* 函数名: BspMpicIntSourceSet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspMpicIntSourceSet(ULONG srcAddr,int polarity,int sense,int priority,int vector)
{
    unsigned int 	ulSrcVal;
    ULONG	errCode;
    errCode = BspMpicSrcAddrCheck(srcAddr);
    if (errCode == MPIC_INV_INTER_SOURCE)
    {
        return (errCode);
    }
    MPIC_REG_READ(srcAddr,ulSrcVal);
    switch (errCode)
    {
    case MPIC_EX_INTERRUPT:
        if (ulSrcVal & MPIC_EIVPR_INTR_ACTIVE)
        {
            return (MPIC_INTER_IN_SERVICE);
        }
        /* mask off current settings */
        ulSrcVal &= ~(MPIC_EIVPR_INTR_POLARITY |
                      MPIC_EIVPR_INTR_SENSE |
                      MPIC_EIVPR_PRIORITY_MSK  |
                      MPIC_EIVPR_VECTOR_MSK);
        /* set new values */
        ulSrcVal |= (MPIC_EIVPR_POLARITY (polarity) |
                     MPIC_EIVPR_SENS (sense) |
                     MPIC_EIVPR_PRIORITY (priority) |
                     MPIC_EIVPR_VECTOR (vector));
        break;

    case MPIC_IN_INTERRUPT:
        if (ulSrcVal & MPIC_IIVPR_INTR_ACTIVE)
        {
            return (MPIC_INTER_IN_SERVICE);
        }
        /* mask off current settings */
        ulSrcVal &= ~(MPIC_IIVPR_INTR_POLARITY |
                      MPIC_IIVPR_PRIORITY_MSK  |
                      MPIC_IIVPR_VECTOR_MSK);
        /* set new values */
        ulSrcVal |= (MPIC_IIVPR_POLARITY (polarity) |
                     MPIC_IIVPR_PRIORITY (priority) |
                     MPIC_IIVPR_VECTOR (vector));
        break;

    case MPIC_GT_INTERRUPT:
        if (ulSrcVal & MPIC_GTVPR_INTR_ACTIVE)
        {
            return (MPIC_INTER_IN_SERVICE);
        }
        /* mask off current settings */
        ulSrcVal &= ~(MPIC_GTVPR_PRIORITY_MSK  |
                      MPIC_GTVPR_VECTOR_MSK);
        /* set new values */
        ulSrcVal |= (MPIC_GTVPR_PRIORITY (priority) |
                     MPIC_GTVPR_VECTOR (vector));
        break;

    case MPIC_MSG_INTERRUPT:
        if (ulSrcVal & MPIC_MIVPR_INTR_ACTIVE)
        {
            return (MPIC_INTER_IN_SERVICE);
        }
        /* mask off current settings */
        ulSrcVal &= ~(MPIC_MIVPR_PRIORITY_MSK  |
                      MPIC_MIVPR_VECTOR_MSK);
        /* set new values */
        ulSrcVal |= (MPIC_MIVPR_PRIORITY (priority) |
                     MPIC_MIVPR_VECTOR (vector));
        break;

    case MPIC_SMSG_INTERRUPT:
        if (ulSrcVal & MPIC_MSIVPR_INTR_ACTIVE)
        {
            return (MPIC_INTER_IN_SERVICE);
        }
        /* mask off current settings */
        ulSrcVal &= ~(MPIC_MSIVPR_PRIORITY_MSK  |
                      MPIC_MSIVPR_VECTOR_MSK);
        /* set new values */
        ulSrcVal |= (MPIC_MSIVPR_PRIORITY (priority) |
                     MPIC_MSIVPR_VECTOR (vector));
        break;
    case MPIC_IPI_INTERRUPT:
        if (ulSrcVal & MPIC_IPIVPR_INTR_ACTIVE)
        {
            return (MPIC_INTER_IN_SERVICE);
        }
        /* mask off current settings */
        ulSrcVal &= ~(MPIC_IPIVPR_PRIORITY_MSK  |
                      MPIC_IPIVPR_VECTOR_MSK);
        /* set new values */
        ulSrcVal |= (MPIC_IPIVPR_PRIORITY (priority) |
                     MPIC_IPIVPR_VECTOR (vector));
        break;

    default:
        return (ERROR);
    }
    MPIC_REG_WRITE(srcAddr, ulSrcVal);
    return (OK);
}

/******************************************************************************
* 函数名: BspMpicCurTaskPrioSet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicCurTaskPrioSet(ULONG ulPriority)
{
    ULONG ulOldPrio = 0;
    ULONG ulCpuId = 0;
    if ((ulPriority < MPIC_PRIORITY_MIN) || (ulPriority > MPIC_PRIORITY_MAX))
    {
        return (MPIC_INV_PRIO_ERROR);
    }
    ulCpuId = g_ulCurCpuId;
    MPIC_REG_READ(MPIC_CTASK_PRI_REG(ulCpuId),ulOldPrio);
    MPIC_REG_WRITE(MPIC_CTASK_PRI_REG(ulCpuId), ulPriority);
    return (ulOldPrio);
}
/******************************************************************************
* 函数名: BspMpicIntEnable
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
static int BspMpicIntEnable(ULONG ulSrcAddr)
{
    ULONG 	ulSrcVal;
    int 	errCode;
    errCode = BspMpicSrcAddrCheck (ulSrcAddr);
    if (errCode == MPIC_INV_INTER_SOURCE)
    {
        return (errCode);
    }
    MPIC_REG_READ(ulSrcAddr,ulSrcVal);
    switch (errCode)
    {
    case MPIC_EX_INTERRUPT:
        ulSrcVal &= ~(MPIC_EIVPR_INTR_MSK);   /* clear the mask bit */
        break;

    case MPIC_IN_INTERRUPT:
        ulSrcVal &= ~(MPIC_IIVPR_INTR_MSK);   /* clear the mask bit */
        break;

    case MPIC_GT_INTERRUPT:
        ulSrcVal &= ~(MPIC_GTVPR_INTR_MSK);   /* clear the mask bit */
        break;

    case MPIC_MSG_INTERRUPT:
        ulSrcVal &= ~(MPIC_MIVPR_INTR_MSK);   /* clear the mask bit */
        break;

    case MPIC_SMSG_INTERRUPT:
        ulSrcVal &= ~(MPIC_MSIVPR_INTR_MSK);   /* clear the mask bit */
        break;
    case MPIC_IPI_INTERRUPT:
        ulSrcVal &= ~(MPIC_IPIVPR_INTR_MSK);  /* clear the mask bit */
        break;

    default:
        return (ERROR);
    }
    MPIC_REG_WRITE(ulSrcAddr, ulSrcVal);
    return OK;
}

/******************************************************************************
* 函数名: BspMpicIntDisable
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
static int BspMpicIntDisable(ULONG ulSrcAddr)
{
    ULONG 	ulSrcVal = 0;
    int 	errCode;
    errCode = BspMpicSrcAddrCheck (ulSrcAddr);
    if (errCode == MPIC_INV_INTER_SOURCE)
    {
        return (errCode);
    }

    MPIC_REG_READ(ulSrcAddr,ulSrcVal);

    switch (errCode)
    {
    case MPIC_EX_INTERRUPT:
        ulSrcVal |= MPIC_EIVPR_INTR_MSK;	/* set the mask bit */
        break;

    case MPIC_IN_INTERRUPT:
        ulSrcVal |= MPIC_IIVPR_INTR_MSK;	/* set the mask bit */
        break;

    case MPIC_GT_INTERRUPT:
        ulSrcVal |= MPIC_GTVPR_INTR_MSK;	/* set the mask bit */
        break;

    case MPIC_MSG_INTERRUPT:
        ulSrcVal |= MPIC_MIVPR_INTR_MSK;	/* set the mask bit */
        break;

    case MPIC_SMSG_INTERRUPT:
        ulSrcVal |= MPIC_MSIVPR_INTR_MSK;	/* set the mask bit */
        break;
    case MPIC_IPI_INTERRUPT:
        ulSrcVal |= MPIC_IPIVPR_INTR_MSK;	/* set the mask bit */
        break;

    default:
        return (ERROR);
    }

    MPIC_REG_WRITE(ulSrcAddr, ulSrcVal);
    return OK;
}
/******************************************************************************
* 函数名: BspMpicSelfVecsInit
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int BspMpicSelfVecsInit(void)
{
    ULONG ulCnt = 0;
    pMpic_vecs = malloc(sizeof(T_MPIC_VECS));

    if(NULL == pMpic_vecs )
    {
        printf("\n");
        return ERROR;
    }
    memset((UCHAR *)pMpic_vecs,0,sizeof(T_MPIC_VECS));

    for(ulCnt = 0; ulCnt<4; ulCnt++)
    {
        ((T_MPIC_VECS *)pMpic_vecs)->ipi_vecs[ulCnt] = MPIC_VEC_IPI_IRQ0 + ulCnt;
        ((T_MPIC_VECS *)pMpic_vecs)->msg_vecs.msga_vec[ulCnt]  = MPIC_VEC_MSGA_IRQ0 + ulCnt;
        ((T_MPIC_VECS *)pMpic_vecs)->msg_vecs.msgb_vec[ulCnt]  = MPIC_VEC_MSGB_IRQ0 + ulCnt;
        ((T_MPIC_VECS *)pMpic_vecs)->timer_vecs.tma_vec[ulCnt] = MPIC_VEC_GTA_IRQ0 + ulCnt;
        ((T_MPIC_VECS *)pMpic_vecs)->timer_vecs.tmb_vec[ulCnt] = MPIC_VEC_GTB_IRQ0 + ulCnt;
    }
    ((T_MPIC_VECS *)pMpic_vecs)->spurious_vec = MPIC_VEC_SPUR_IRQ;

    return OK;


}
/******************************************************************************
* 函数名: BspMpicDstCpuSet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicDstCpuSet(ULONG ulVector,ULONG ulCpuId)
{
    ULONG ulDstRegAddr = 0;
    if((ulVector > 250)||((ulVector>234)&&(ulVector<243)))
    {
        /* ipi,msg中断目标核在发送时设定,此处直接返回 */
        return BSP_OK;
    }
    ulDstRegAddr = BspMpicGetDestRegAdrs(ulVector);
    MPIC_REG_WRITE(ulDstRegAddr,1<<ulCpuId);
    return BSP_OK;
}

/******************************************************************************
* 函数名: BspMpicLevSet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicLevSet(ULONG ulVector,ULONG ulLev)
{
    ULONG ulLevRegAddr = 0;
    ulLevRegAddr = BspMpicGetLevlRegAdrs(ulVector);
    MPIC_REG_WRITE(ulLevRegAddr,ulLev);
    return 0;
}

/******************************************************************************
* 函数名: BspMpicSelfIntInit
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void BspMpicSelfIntInit (void)
{
    int 	irq;
    BspMpicCurTaskPrioSet(MPIC_PRIORITY_MAX);
    for (irq = 0; irq < MPIC_MAX_GTA_IRQS; irq++)
    {
        BspMpicIntDisable(MPIC_GTA_VEC_REG(irq));
        BspMpicIntSourceSet(MPIC_GTA_VEC_REG(irq),0x0, 0x0, MPIC_PRIORITY_DEFAULT, MPIC_VEC_GTA_IRQ0 + irq);
    }
    for (irq = 0; irq < MPIC_MAX_GTB_IRQS; irq++)
    {
        BspMpicIntDisable(MPIC_GTB_VEC_REG(irq));
        BspMpicIntSourceSet(MPIC_GTB_VEC_REG(irq),0x0, 0x0, MPIC_PRIORITY_DEFAULT, MPIC_VEC_GTB_IRQ0 + irq);
    }
    for (irq = 0; irq < MPIC_MAX_MSGA_IRQS; irq++)
    {
        BspMpicIntDisable(MPIC_MSGA_VEC_REG(irq));
        BspMpicIntSourceSet(MPIC_MSGA_VEC_REG(irq),0x0, 0x0, MPIC_PRIORITY_DEFAULT, MPIC_VEC_MSGA_IRQ0 + irq);
        MPIC_REG_WRITE(MPIC_MSGA_EN_REG,0xf);
    }
    for (irq = 0; irq < MPIC_MAX_MSGB_IRQS; irq++)
    {
        BspMpicIntDisable (MPIC_MSGB_VEC_REG(irq));
        BspMpicIntSourceSet (MPIC_MSGB_VEC_REG(irq),0x0, 0x0, MPIC_PRIORITY_DEFAULT, MPIC_VEC_MSGB_IRQ0 + irq);
        MPIC_REG_WRITE(MPIC_MSGB_EN_REG,0xf);
    }
    for (irq = 0; irq < MPIC_MAX_IPI_IRQS; irq++)
    {
        BspMpicIntDisable (MPIC_IPI_VEC_REG(irq));
        BspMpicIntSourceSet (MPIC_IPI_VEC_REG(irq),0x0, 0x0, MPIC_PRIORITY_DEFAULT, MPIC_VEC_IPI_IRQ0 + irq);
    }
    BspMpicCurTaskPrioSet (MPIC_PRIORITY_MIN); /* set it to highest priority */
}
/******************************************************************************
* 函数名: BspErrorCheckInit
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspErrorCheckInit(void)
{
    ULONG ulVector = 0;
    ULONG ulPolarity = 0;
    ULONG ulSense = 0;
    // ULONG ulPriority = 0;

    /*BspMpicCurTaskPrioSet(MPIC_PRIORITY_MAX);*/

    ulVector = MPIC_VEC_IN_IRQ0;
    ulPolarity = 1;
    //  ulPriority = MPIC_PRIORITY_DEFAULT;

    BspMpicIntDisable(MPIC_IN_INT0_VEC_REG);

    /* 中断源配置 */
    BspMpicIntSourceSet(MPIC_IN_INT0_VEC_REG,ulPolarity, ulSense, MPIC_PRIORITY_DEFAULT,ulVector);

    /* 中断触发核设置 */
    BspMpicDstCpuSet(ulVector,0);
    BspMpicLevSet(ulVector,MPIC_INTTGT_INT);
    return BSP_OK;
}

/******************************************************************************
* 函数名: BspIntcInit
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
UINT32 BspIntcInit(void)
{
    ULONG ulGcrVal;
    g_ulCurCpuId = 0;
    MPIC_REG_READ (MPIC_GLOBAL_CFG_REG,ulGcrVal);
    ulGcrVal |= (MPIC_GCR_MODE_EPF);
    MPIC_REG_WRITE(MPIC_GLOBAL_CFG_REG, ulGcrVal);
    BspMpicSelfIntInit();
    BspErrorCheckInit();
    if(BSP_OK != BspMpicTimeBaseInit())
    {
        bsp_dbg("BspIntcInit:Time Base Init failed!!!\n");
        return BSP_ERROR;
    }
    return OK;
}
/******************************************************************************
* 函数名: BspIntcHookRegister
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
UINT32 BspIntcHookRegister(T_IntcAttrs * const ptIntcAttrs)
{
    ULONG ulVector = 0;
    VOIDFUNCPTR pFunction;
    ULONG ulPara = 0;
    ULONG ulSrcAddr = 0;
    ULONG ulPolarity = 0;
    ULONG ulSense = 0;
    ULONG ulPriority = 0;
    if(NULL == ptIntcAttrs)
    {
        return 1;
    }
    ulVector   = ptIntcAttrs->udIntNo;
    pFunction  = ptIntcAttrs->IsrFunc;
    ulPara     = ptIntcAttrs->udarg;
    ulSense    = ptIntcAttrs->ucTrgMode & 0xf;
    ulPriority = ptIntcAttrs->udPrio;
    ulPolarity = (ptIntcAttrs->ucTrgMode & 0xf0)>>4;
    ulSrcAddr = BspMpicGetVecRegAdrs(ulVector);
    BspIntcDisable(ulVector);
    BspMpicIntSourceSet(ulSrcAddr,ulPolarity,ulSense,ulPriority,ulVector);
    BspMpicDstCpuSet(ulVector, g_ulCurCpuId);
    if(BSP_OK != requestIrq((UINT32)(VOIDFUNCPTR*)ulVector, pFunction, (void*)ulPara))
    {
        printf("Connect Vector[%d] and Function[0x%d] Failed!!!\n",(UINT32)ulVector,(UINT32)pFunction);
        return BSP_ERROR;
    }
    BspIntcEnable(ulVector);
    return BSP_OK;
}

/******************************************************************************
* 函数名: BspIntcEnable
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
VOID BspIntcEnable(const ULONG ulIntNo)
{
    ULONG ulSrcAddr = 0;
    ULONG ulRet = 0;
    ulRet = BspMpicGetVecRegAdrs(ulIntNo);
    if(ERROR == ulRet)
    {
        printf("BspIntcEnable:MPIC Get Vector Register Failed,Ret=%d\n",(UINT32)ulRet);
        return  ;
    }
    else
    {
        ulSrcAddr = ulRet;
    }
    BspMpicIntEnable(ulSrcAddr);
    return  ;
}
/******************************************************************************
* 函数名: BspIntcDisable
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
VOID BspIntcDisable (const UINT32 ulIntNo)
{
    ULONG ulSrcAddr = 0;
    ULONG ulRet = 0;
    ulRet = BspMpicGetVecRegAdrs(ulIntNo);
    if(ERROR == ulRet)
    {
        printf("BspIntcDisable:MPIC Get Vector Register Failed,Ret=%d\n",(UINT32)ulRet);
        return;
    }
    else
    {
        ulSrcAddr = ulRet;
    }
    BspMpicIntDisable(ulSrcAddr);
    return ;
}

/******************************************************************************
* 函数名: BspIpiTrig
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspIpiTrig(UINT32 udDstCoreMask,UINT32 udIpiNo)
{
    ULONG ulCurCpuId = 0;
    ULONG ulRetVal = 0;
    ulCurCpuId = g_ulCurCpuId;
    switch(udIpiNo)
    {
    case 0:
        MPIC_REG_WRITE(MPIC_IPI_DPATCH_REG0(ulCurCpuId), udDstCoreMask);
        break;
    case 1:
        MPIC_REG_WRITE(MPIC_IPI_DPATCH_REG1(ulCurCpuId), udDstCoreMask);
        break;
    case 2:
        MPIC_REG_WRITE(MPIC_IPI_DPATCH_REG2(ulCurCpuId), udDstCoreMask);
        break;
    case 3:
        MPIC_REG_WRITE(MPIC_IPI_DPATCH_REG3(ulCurCpuId), udDstCoreMask);
        break;
    default:
        ulRetVal = ERROR;
        break;
    }
    return(ulRetVal);

}



/******************************************************************************
* 函数名: BspMpicClearMsgStat
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/

ULONG BspMpicClearMsgStat(UCHAR ucMsgId)
{
    ULONG ulTemp = 0;

    if(ucMsgId < 4)
    {
        MPIC_REG_READ(MPIC_MSGA_STATE_REG,ulTemp);
        ulTemp |= 1 << ucMsgId ;
        MPIC_REG_WRITE(MPIC_MSGA_STATE_REG,ulTemp);
    }
    else if((ucMsgId > 3)&&(ucMsgId < 8))
    {
        MPIC_REG_READ(MPIC_MSGB_STATE_REG,ulTemp);
        ulTemp |= 1 << ucMsgId ;
        MPIC_REG_WRITE(MPIC_MSGB_STATE_REG,ulTemp);
    }
    else
    {
        printf("MSG ID[%d] is error!\n",ucMsgId);
        return BSP_ERROR;
    }

    return BSP_OK;

}
/******************************************************************************
* 函数名: BspMsgIntSend
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMsgIntSend(ULONG ulDstCoreId,UCHAR ucMsgId)
{
    ULONG ulMsgValue = 0;
    if(BSP_OK != BspMpicMsgIntSend(ucMsgId, ulDstCoreId, ulMsgValue))
    {
        return BSP_ERROR;
    }
    return BSP_OK;
}
/******************************************************************************
* 函数名: BspMpicMsgDstCpuSet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicMsgDstCpuSet(UCHAR ucMsgId,UCHAR ucDstCpuId )
{
    if((ucDstCpuId < 1)||(ucDstCpuId > 7))
    {
        bsp_dbg("Dst Cpu ID[%d] is error!\n",ucDstCpuId);
        return BSP_ERROR;
    }
    else
    {
        if(ucMsgId < 4)
        {
            MPIC_REG_WRITE(MPIC_MSGA_DEST_REG(ucMsgId),1<<ucDstCpuId);
        }
        else if((ucMsgId > 3)&&(ucMsgId < 8))
        {
            MPIC_REG_WRITE(MPIC_MSGB_DEST_REG(ucMsgId-4),1<<ucDstCpuId);
        }
        else
        {
            bsp_dbg("MSG ID[%d] is error!\n",ucMsgId);
            return BSP_ERROR;
        }

    }
    return 0;
}

/******************************************************************************
* 函数名: BspMpicMsgValueSet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicMsgValueSet(unsigned char ucMsgId,unsigned int dwMsgValue)
{
    if(ucMsgId < 4)
    {
        MPIC_REG_WRITE(MPIC_MSGA_REG0+ucMsgId*0x10,dwMsgValue);
    }
    else if((ucMsgId > 3)&&(ucMsgId < 8))
    {
        MPIC_REG_WRITE(MPIC_MSGB_REG0+(ucMsgId-4)*0x10,dwMsgValue);
    }
    else
    {
        bsp_dbg("MSG ID[%d] is error!\n",ucMsgId);
        return BSP_ERROR;
    }
    return BSP_OK;
}

/******************************************************************************
* 函数名: BspMpicMsgStatGet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicMsgStatGet(ULONG ucMsgId)
{
    ULONG ulMsgIntStat = 0;
    if(ucMsgId < 4)
    {
        MPIC_REG_READ(MPIC_MSGA_STATE_REG,ulMsgIntStat);
    }
    else if((ucMsgId > 3)&&(ucMsgId < 8))
    {
        MPIC_REG_READ(MPIC_MSGB_STATE_REG,ulMsgIntStat);
        ulMsgIntStat = ulMsgIntStat<<4;
    }
    else
    {
        bsp_dbg("MSG ID[%d] is error!\n",(UINT32)ucMsgId);
        return -1;
    }
    ulMsgIntStat = (ulMsgIntStat>>ucMsgId)&1;
    return ulMsgIntStat;
}

/******************************************************************************
* 函数名: BspMpicMsgEnable
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/

ULONG BspMpicMsgEnable(unsigned char ucMsgId)
{
    unsigned int dwRegTemp = 0;
    if(ucMsgId < 4)
    {
        MPIC_REG_READ(MPIC_MSGA_EN_REG,dwRegTemp);
        dwRegTemp = dwRegTemp | (1<<ucMsgId);
        MPIC_REG_WRITE(MPIC_MSGA_EN_REG,dwRegTemp);
    }
    else if((ucMsgId > 3)&&(ucMsgId < 8))
    {
        MPIC_REG_READ(MPIC_MSGA_EN_REG,dwRegTemp);
        dwRegTemp = dwRegTemp | (1<<(ucMsgId-4));
        MPIC_REG_WRITE(MPIC_MSGB_EN_REG,dwRegTemp);
    }
    else
    {
        bsp_dbg("MSG ID[%d] is error!\n",(UINT32)ucMsgId);
        return BSP_ERROR;
    }
    return BSP_OK;
}


/******************************************************************************
* 函数名: BspMpicMsgDisable
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicMsgDisable(unsigned char ucMsgId)
{
    unsigned int dwRegTemp;
    if(ucMsgId < 4)
    {
        MPIC_REG_READ(MPIC_MSGA_EN_REG,dwRegTemp);
        dwRegTemp &= (~(1<<ucMsgId));
        MPIC_REG_WRITE(MPIC_MSGA_EN_REG,dwRegTemp);
    }
    else if((ucMsgId > 3)&&(ucMsgId < 8))
    {
        MPIC_REG_READ(MPIC_MSGA_EN_REG,dwRegTemp);
        dwRegTemp = dwRegTemp &(~(1<<(ucMsgId-4)));
        MPIC_REG_WRITE(MPIC_MSGB_EN_REG,dwRegTemp);
    }
    else
    {
        bsp_dbg("MSG ID[%d] is error!\n",(UINT32)ucMsgId);
        return BSP_ERROR;
    }
    return BSP_OK;
}

/******************************************************************************
* 函数名: BspMpicMsgIntSend
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicMsgIntSend(unsigned char ucMsgId,unsigned char ucDstCpuId,unsigned int dwMsgVal)
{
    unsigned int dwRegTemp = 0;
    unsigned long ulWaiteCnt = 0;
    if((ucMsgId>7)||(ucDstCpuId>7))
    {
        bsp_dbg("BspMpicMsgIntSend Input Para ucMsgId[%d] ucDstCpuId[%d] Error!!!\n",(UINT32)ucMsgId,(UINT32)ucDstCpuId);
        return BSP_ERROR;
    }
    /* 等待中断处理结束再次发送 */
    do
    {
        dwRegTemp = BspMpicMsgStatGet(ucMsgId);
        if(!dwRegTemp)
        {
            break;
        }
        ulWaiteCnt++;
    }
    while(ulWaiteCnt<100);

    if(100 == ulWaiteCnt)
    {
        bsp_dbg("BspMpicMsgIntSend ucMsgId[%d] Send Error!!!MsgSendStat[%d]\n",(UINT32)ucMsgId,(UINT32)dwRegTemp);
        return BSP_ERROR;
    }
    BspMpicMsgEnable(ucMsgId);
    BspMpicMsgDstCpuSet(ucMsgId,ucDstCpuId);
    BspMpicMsgValueSet(ucMsgId,dwMsgVal);
    return BSP_OK;
}

/******************************************************************************
* 函数名: BspMpicMsgIntGet
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMpicMsgIntGet(unsigned char ucMsgId )
{
    ULONG ulMsgIntVal = 0;
    if(ucMsgId < 4)
    {
        MPIC_REG_READ(MPIC_MSGA_REG0+ucMsgId*0x10,ulMsgIntVal);
    }
    else if((ucMsgId > 3)&&(ucMsgId < 8))
    {
        MPIC_REG_READ(MPIC_MSGB_REG0+(ucMsgId-4)*0x10,ulMsgIntVal);
    }
    else
    {
        bsp_dbg("MSG ID[%d] is error!\n",ucMsgId);
        return -1;
    }
    return ulMsgIntVal;
}


/******************************************************************************
* 函数名: BspTestIpiIsr
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void BspTestIpiIsr(ULONG ulVector)
{
    //unsigned int uireg =0;
    switch(ulVector)
    {
    case 251:
    {
        g_ulTestIpi[0] ++;
        printf(" receice ipi 251!\n");
        break;
    }
    case 252:
    {
        printf(" receice ipi 252!\n");
        g_ulTestIpi[1] ++;
        break;
    }
    case 253:
    {
        printf(" receice ipi 253!\n");
        g_ulTestIpi[2] ++;
        break;
    }
    case 254:
    {
        printf(" receice ipi 254!\n");
        g_ulTestIpi[3] ++;
        break;
    }
    default :
        break;
    }
}



/******************************************************************************
* 函数名: BspIpiInstall
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspIpiInstall(UCHAR ucIpiVector)
{
    T_IntcAttrs tIntcAttrs;
    memset((UCHAR *)&tIntcAttrs,0,sizeof(T_IntcAttrs));
    tIntcAttrs.IsrFunc = BspTestIpiIsr;
    tIntcAttrs.udarg   = ucIpiVector;
    tIntcAttrs.udIntNo = ucIpiVector;
    tIntcAttrs.udPrio  = 10;
    BspIntcHookRegister((T_IntcAttrs *)&tIntcAttrs);
    BspIntcEnable(ucIpiVector);

    return BSP_OK;
}
/******************************************************************************
* 函数名: BspMsgInstall
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
ULONG BspMsgInstall(UCHAR ucMsgVector)
{
    T_IntcAttrs tIntcAttrs;
    memset((UCHAR *)&tIntcAttrs,0,sizeof(T_IntcAttrs));
    tIntcAttrs.IsrFunc = BspTestMsgIsr;
    tIntcAttrs.udarg   = ucMsgVector;
    tIntcAttrs.udIntNo = ucMsgVector;
    tIntcAttrs.udPrio  = 10;
    BspIntcHookRegister((T_IntcAttrs *)&tIntcAttrs);
    BspIntcEnable(ucMsgVector);
    return BSP_OK;
}

void bsp_delay_100ms(void)
{
    int delayCount=0;
    for (delayCount = 0; delayCount < 1000000; delayCount++);
}



void bsp_delay_1ms(int dwms)
{

    int x,y;
    unsigned long long int llstart = 0;
    unsigned long long int llend = 0;

    llstart =  BspGetCpuCycle();
    //for(x=dwms;x>0;x--)
    //    for(y=98100;y>0;y--);

    //llend =  BspGetCpuCycle();
    while(1)
    {
        llend =  BspGetCpuCycle();
        if ((int)(((llend-llstart)*1000)/1200)>=dwms*1000000)
            break;
    }
    //printf("%d cost time %d ns\n",dwms,(int)(((llend-llstart)*1000)/1200));
}
void bsp_delay_100us(void)
{
    unsigned long long int llstart = 0;
    unsigned long long int llend = 0;
    llstart =  BspGetCpuCycle();
    while(1)
    {
        llend =  BspGetCpuCycle();
        if ((int)(((llend-llstart)*1000)/1200)>=100000)
            break;
    }
}

void bsp_delay1ms(int dwms)
{
    struct timeval temp;
    temp.tv_sec = 0;
    temp.tv_usec = dwms*1000;
    select(0, NULL, NULL, NULL, &temp);
}


unsigned int g_intcounter = 0;
#include "bsp_gps_ext.h"

extern T_GpsAllData gUbloxGpsAllData;

#define LONG_FRAME_NUM_HIGH (515)
#define LONG_FRAME_NUM_LOW  (514)

extern unsigned char *g_u8fpgabase;

void bsp_write_long_frame_to_fpga(unsigned int dwvalue)
{

    //printf("long frame->0x%lx\n",dwvalue);
#if 0
    unsigned short ustmp0=0;
    unsigned short ustmp1=0;
    ustmp0 = (dwvalue & 0xffff);
    ustmp1 = (dwvalue & 0x0fff0000)>>16;

    bsp_delay_1ms(1);
    bsp_fpga_write_reg(LONG_FRAME_NUM_LOW,ustmp0);
    bsp_delay_100us();
    bsp_fpga_write_reg(LONG_FRAME_NUM_HIGH,ustmp1);
#else
    unsigned short ustmp0=0;
    unsigned short ustmp1=0;
    ustmp0 = (dwvalue & 0xffff);
    ustmp1 = (dwvalue & 0x0fff0000)>>16;
    bsp_cpld_write_reg(58,0);
    bsp_cpld_write_reg(54,(ustmp1 & 0x0f00)>>8);
    bsp_cpld_write_reg(55,(ustmp1 & 0xff));
    bsp_cpld_write_reg(56,(ustmp0 & 0xff00) >>8);
    bsp_cpld_write_reg(57,(ustmp0 & 0xff));
    bsp_cpld_write_reg(58,1);
    bsp_cpld_write_reg(58,0);
#endif
}
u8 bsp_get_gps_level(void)
{
    return gUbloxGpsAllData.TrackedSatellites;
    //gUbloxGpsAllData.VisibleSatellites;
}

//int g_gpscnt=0;
//int g_sync_sat=0;

int g_gps_onlock = 0;

int g_wframe = 0;

unsigned int time20s=0;
unsigned long long int costtimebefore=0;
unsigned long long int costtimeafter=0;
int timeflag=0;
extern sem_t g_semb_getphas_sendac;
extern int g_gps_lock;
#define DELAY_TO_WRITE (0)

unsigned int g_max_gps_time = 40;
unsigned int g_max_gps_time_change = 0;
unsigned int g_max_gps_time_change_stat = 0;
#define WRITE_FAME_TIME (g_max_gps_time+DELAY_TO_WRITE)
unsigned int time1s=0;
unsigned long long int costtimebefore1=0;
unsigned long long int costtimeafter1=0;
int timeflag1=0;
int dropflag=0;
unsigned int g_drop_count = 0;

unsigned int g_prepare_write = 0;
unsigned int g_real_write = 0;
unsigned int g_read_time_out = 0;
int g_gps_print_level = 0;
void print_gps_var()
{
    printf("g_intcounter=%d,g_drop_count=%d,g_prepare_write=%d,g_read_time_out=%d,g_real_write=%d,g_wframe=%d.\n",
           g_intcounter,g_drop_count,g_prepare_write,g_read_time_out,g_real_write,g_wframe);
}

#define USB_PE_MASK   (0x4)
#define USDPAA_IOC_MAGIC 'S'

extern s32 g_s32Usb1DevOnFlag;
s32 createflag = 0;
VOID BspUsbIsr(ULONG ulVector)
{
    u8 u8UsbId;
    volatile UINT32 dwusbstatus=0;
    u8UsbId = ulVector-16-28+1;
    static int count = 0;
    dwusbstatus = read_portsc(u8UsbId);

    if (dwusbstatus & USB_PE_MASK )
    {
        g_s32Usb1DevOnFlag = 0;
        count=0;
        if(createflag == 1)
        {
            bsp_remove_usb(u8UsbId);
            createflag = 0;
        }
    }
    else
    {
        count++;
        if(count >= 20)
        {
            g_s32Usb1DevOnFlag = 1;
            if(createflag == 0)
            {
                bsp_create_usb(u8UsbId);
                createflag = 1;
            }
        }
    }
}

#define MAX_HMIDATA_LEN    (500)

typedef struct t_hmidata
{
    unsigned char        isUsed;
    unsigned char        u8IsReady;
    unsigned long        dwRcvPcktCnt;
    unsigned char        ucErrorCnt;
    unsigned char        ucSucceCnt;
    unsigned int         dwBufferIndex;
    unsigned char        ucRcvData[MAX_HMIDATA_LEN];
    unsigned int         dwRcvDataLen;
} T_HmiData;

#define    USDPAA_IOC_MAGIC 'S'
#define    USDPAA_IOC_HMIINIT                  _IOR(USDPAA_IOC_MAGIC, 25, unsigned int)
#define    USDPAA_IOC_HMISETID                 _IOR(USDPAA_IOC_MAGIC, 26, unsigned int)
#define    USDPAA_IOC_HMIGETDATA               _IOR(USDPAA_IOC_MAGIC, 27, unsigned int)
#define    USDPAA_IOC_HMISETDATA               _IOR(USDPAA_IOC_MAGIC, 28, unsigned int)

extern sem_t g_ipmb_sema;
extern UINT8 g_u8ReceiveMsgIndex;
extern STRU_RECV_MESSAGE g_struReceiveMsgBuf[IPMB_RECEIVE_BUF_NUM];
VOID  BspI2cIrqIsr(ULONG ulVector)
{
    int fd_iic;
    int ret;
    int itmp;
    unsigned int dw_value;
    static T_HmiData gathmidata;

    g_i2c_irq_count++;
    fd_iic = open("/dev/usdpaa", O_RDWR);
    if (fd_iic< 0)
    {
        return;
    }
    memset(&gathmidata,0,sizeof(gathmidata));
    ret = ioctl(fd_iic, USDPAA_IOC_HMIGETDATA, &gathmidata);
    if (ret < 0)
    {
        printf("ioctl error!\r\n");
        close(fd_iic);
        return ;
    }

    if (gathmidata.u8IsReady == 1)
    {
        g_u8ReceiveMsgIndex++;
        if(g_u8ReceiveMsgIndex == IPMB_RECEIVE_BUF_NUM)
            g_u8ReceiveMsgIndex =0;
        dw_value = gathmidata.dwBufferIndex;

        //printf("[userspace] isUsed:0x%lx;u8IsReady:0x%lx;dwRcvDataLen:0x%lx;bufferindex:0x%lx\r\n",gathmidata.isUsed,gathmidata.u8IsReady,gathmidata.dwRcvDataLen,gathmidata.dwBufferIndex);
        //printf("release bufferindex:0x%lx;len:0x%lx\r\n",gathmidata.dwBufferIndex,gathmidata.dwRcvDataLen);
        ret = ioctl(fd_iic, USDPAA_IOC_HMISETDATA, &dw_value);
        if (ret < 0)
        {
            printf("USDPAA_IOC_HMISETDATA error!\r\n");
            close(fd_iic);
            return ;
        }
        if(g_i2c_irq_print==1)
        {
            printf("[userspace]:\r\n");
            for(itmp=0; itmp<gathmidata.dwRcvDataLen; itmp++)
            {
                printf("0x%02x ",gathmidata.ucRcvData[itmp]);
            }
            printf("\r\n");
        }
        g_struReceiveMsgBuf[g_u8ReceiveMsgIndex].u8Used = 1;
        g_struReceiveMsgBuf[g_u8ReceiveMsgIndex].u32Length = gathmidata.dwRcvDataLen;
        memcpy(&g_struReceiveMsgBuf[g_u8ReceiveMsgIndex].u8Message,&gathmidata.ucRcvData,gathmidata.dwRcvDataLen);
        sem_post(&g_ipmb_sema);
    }
    close(fd_iic);
}

typedef struct
{
    u8 au8Year[2];
    u8 u8Mon;
    u8 u8Day;
    u8 u8Hour;
    u8 u8Min;
    u8 u8Sec;
} T_ABS_TM;

extern  s32 gps_GMT_Offset;

s8 bsp_get_cur_time(T_ABS_TM *tTM);

void gps_get_time_test(void)
{
    T_ABS_TM tTM;

    bsp_get_cur_time(&tTM);

    printf("%d%d-%d-%d %d %d:%d:%d\n",
           tTM.au8Year[0],
           tTM.au8Year[1],
           tTM.u8Mon,
           tTM.u8Day,
           tTM.u8Hour,
           tTM.u8Min,
           tTM.u8Sec);

}

s8 bsp_get_cur_time(T_ABS_TM *tTM)
{
    if (tTM == NULL)
    {
        printf("%s tTM is NULL\n", __func__);
        return BSP_ERROR;
    }

    if (bsp_get_gps_level() < 3)
    {
        printf("GPS cable doesn't be connected\n");
        return BSP_ERROR;
    }

    gps_NAV_TimeUTC(gps_GMT_Offset);

    tTM->au8Year[0] = (gUbloxGpsAllData.Year) / 100;
    tTM->au8Year[1] = (gUbloxGpsAllData.Year) % 100;
    tTM->u8Mon = gUbloxGpsAllData.Month;
    tTM->u8Day = gUbloxGpsAllData.Day;
    tTM->u8Hour = gUbloxGpsAllData.Hour;
    tTM->u8Min = gUbloxGpsAllData.Minute;
    tTM->u8Sec = gUbloxGpsAllData.Second;

    return BSP_OK;
}

void bsp_create_gps_thread(void)
{
    int res=0;
    pthread_t ptid;
    pthread_t       a_thread;
    pthread_attr_t  attr;
    struct sched_param parm;
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 30;
    pthread_attr_setschedparam(&attr, &parm);
    sem_init(&g_gps_1s_sem,0,0);
    sem_init(&g_gps_1s_sem1,0,0);
    res = pthread_create(&ptid, &attr, (FUNCPTR)gps_handle_task,NULL);
    pthread_attr_destroy(&attr);
    if (-1 == res)
    {
        perror("create gps thread error!\n");
    }
}


ULONG BspExtIrq10Setup(UCHAR ucExtIrqVector)
{
    T_IntcAttrs tIntcAttrs;
    memset((UCHAR *)&tIntcAttrs,0,sizeof(T_IntcAttrs));
    tIntcAttrs.IsrFunc = BspGpsIrqIsr;
    //tIntcAttrs.udarg   = ucExtIrqVector+90;
    //tIntcAttrs.udIntNo = ucExtIrqVector+90;
    tIntcAttrs.udarg   = ucExtIrqVector;
    tIntcAttrs.udIntNo = ucExtIrqVector;
    tIntcAttrs.ucTrgMode = 0x0;
    tIntcAttrs.udPrio  = 10;
    printf("tIntcAttrs.udarg->%d, tIntcAttrs.udIntNo->%d\n", tIntcAttrs.udarg,tIntcAttrs.udIntNo);
    BspIntcHookRegister((T_IntcAttrs *)&tIntcAttrs);
    BspIntcEnable(ucExtIrqVector);
    return BSP_OK;
}

ULONG BspExtIrqI2cSetup(UCHAR ucExtIrqVector)
{
    T_IntcAttrs tIntcAttrs;
    memset((UCHAR *)&tIntcAttrs,0,sizeof(T_IntcAttrs));
    tIntcAttrs.IsrFunc = BspI2cIrqIsr;
    //tIntcAttrs.udarg   = ucExtIrqVector+90;
    //tIntcAttrs.udIntNo = ucExtIrqVector+90;
    tIntcAttrs.udarg   = ucExtIrqVector;
    tIntcAttrs.udIntNo = ucExtIrqVector;
    tIntcAttrs.ucTrgMode = 0x0;
    tIntcAttrs.udPrio  = 11;
    printf("tIntcAttrs.udarg->%d, tIntcAttrs.udIntNo->%d\n", tIntcAttrs.udarg,tIntcAttrs.udIntNo);
    BspIntcHookRegister((T_IntcAttrs *)&tIntcAttrs);
    BspIntcEnable(ucExtIrqVector);
    return BSP_OK;
}

ULONG BspExtIrqUsbSetup(UCHAR ucExtIrqVector)
{
    T_IntcAttrs tIntcAttrs;
    memset((UCHAR *)&tIntcAttrs,0,sizeof(T_IntcAttrs));
    tIntcAttrs.IsrFunc = BspUsbIsr;
    //tIntcAttrs.udarg   = ucExtIrqVector+90;
    //tIntcAttrs.udIntNo = ucExtIrqVector+90;
    tIntcAttrs.udarg   = ucExtIrqVector;
    tIntcAttrs.udIntNo = ucExtIrqVector;
    tIntcAttrs.ucTrgMode = 0x0;
    tIntcAttrs.udPrio  = 10;
    printf("tIntcAttrs.udarg->%d, tIntcAttrs.udIntNo->%d\n", tIntcAttrs.udarg,tIntcAttrs.udIntNo);
    BspIntcHookRegister((T_IntcAttrs *)&tIntcAttrs);
    BspIntcEnable(ucExtIrqVector);
    return BSP_OK;
}

static unsigned long long int g_llstart=0;
static unsigned long long int g_llend=0;

VOID  BspGpsIrqIsr(ULONG ulVector)
{
    static int iflag=0;
    static int icnt=0;
    unsigned int dwtmp=0;
    static int g_1pps =0;
    static unsigned long long int llstart = 0;
    static unsigned long long int llend = 0;

    if(timeflag1==0)
    {
        costtimebefore1=costtimeafter1=BspGetCpuCycle();
        timeflag1=1;
    }
    else
    {
        costtimeafter1=BspGetCpuCycle();
        time1s=(int)(((((costtimeafter1-costtimebefore1)*1000)/1200))/1000);
        if(g_gps_print_level == 1)
            printf("------pp1s 1s interval is %d us. g_intcounter->%d\n",time1s, g_intcounter);
        costtimebefore1=costtimeafter1;
    }

#if 1
    if(time1s != 0)
    {
        if((time1s >= 999000) && (time1s <= 1001000))
        {
            dropflag = 0;
        }
        else
        {
            if(g_gps_print_level == 1)
                printf("drop this time..........\n");
            dropflag = 1;
            g_drop_count++;
        }
    }
#endif


    if (dropflag == 0 && g_gps_lock == 1 && gps_check_clock() == BSP_OK && (6 != bsp_get_synsrc()))

    {
        g_intcounter++;
        if (!(g_intcounter % g_max_gps_time))

        {

            sem_post(&g_gps_1s_sem);
            g_llstart = BspGetCpuCycle();
            if(g_gps_print_level == 1)
                printf("<g_gps_1s_sem>,g_intcounter->%d g_max_gps_time->%d\n",g_intcounter, g_max_gps_time);
        }

#if 0
        if (((g_jcnt*g_max_gps_time+DELAY_TO_WRITE) == g_intcounter) && (g_intcounter>g_max_gps_time))
        {
            sem_post(&g_gps_1s_sem);
            if(g_gps_print_level == 1)
                printf("[g_gps_1s_sem],g_intcounter->%d\n",g_intcounter);

        }
#endif
    }
    else
    {
        if(g_gps_print_level == 1)
            printf("g_gps_lock=%d, bsp_get_gps_level()=%d\n", g_gps_lock, bsp_get_gps_level());
        g_gps_onlock = 0;
        g_intcounter = 0;
        //g_sync_sat = 0;
        g_max_gps_time_change = 0;
        timeflag=0;
        //g_jcnt = 0;

    }
    //sem_post(&g_gps_1s);
    sem_post(&g_semb_getphas_sendac);

}
int  g_GpsSyncTimeAlarm=0;
int  g_GpsSyncTimeErr=0;
int  g_time_out=0;
unsigned int g_GpsSvinfo_Lockinfo=0;
int get_gps_info_flag=0;
void gps_handle_task(int arg)
{
    static int iflag = 0;
    static int VisibleSatellites_bak  = 0;
    static int iretry=0;
    unsigned int dwtmp=0;

    static int dwres=0;
    unsigned long long int llstart;
    unsigned long long int llend;

    unsigned int ltime;

    u16 rb;
    time_t now;
    struct tm *timenow;
    u32 u32SyncType;

    unsigned long cpumask;
    cpumask = (1 << 1);
    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
    {
        printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
        return ;
    }

    bsp_print_reg_info(__func__, __FILE__, __LINE__);

    while(1)
    {

        if((g_intcounter == 0) || (((g_intcounter%g_max_gps_time)>0) && ((g_intcounter%g_max_gps_time) < (g_max_gps_time-5))))
        {
		get_gps_info_flag=1;
            if (g_GpsSvinfo_Lockinfo == 0){
		if (BSP_OK != gps_NAV_SVInfo())
		{
		    printf("get gps_NAV_SVInfo error!!!\n");
		}
		if(g_gps_print_level == 1)
		    printf("g_intcounter---------->%d, %d;gUbloxGpsAllData.TrackedSatellites=%d\n", g_intcounter, g_intcounter%g_max_gps_time, gUbloxGpsAllData.TrackedSatellites);			
		g_GpsSvinfo_Lockinfo = 1;
		}else{
		    if(GPS_OK != gps_Get_Lockinfo())
		    {
			bsp_dbg("get GPS Lockinfo error!!\n");
		    }
		
		    g_GpsSvinfo_Lockinfo = 0;
		}

            sleep(1);
        }
        else
        {
		get_gps_info_flag=0;
            sem_wait(&g_gps_1s_sem);
            //if (gUbloxGpsAllData.TrackedSatellites>=3)
            if (gps_check_clock() == BSP_OK)
            {
                if (!(g_intcounter % g_max_gps_time))
                {
                    g_prepare_write++;
                    llstart= BspGetCpuCycle();
                    dwtmp = gps_syn_time();
                    llend =  BspGetCpuCycle();
                    ltime = (unsigned int)(((llend-llstart)*1000)/1200);
                    if (ltime > 1000000000)
                    {
                        g_GpsSyncTimeErr++;
                        if (g_GpsSyncTimeErr>=20)
                        {
                            g_GpsSyncTimeAlarm=1;
                        }
                    }
                    else
                    {
                        g_GpsSyncTimeErr=0;
                    }

                    if(g_gps_print_level == 1)
                    {
                        printf("end to get system time,dwres->0x%lx,result->0x%lx,gUbloxGpsAllData.VisibleSatellites->%d,gUbloxGpsAllData.TrackedSatellites->%d,cost time %d ns\n",dwres,dwtmp-dwres,gUbloxGpsAllData.VisibleSatellites,gUbloxGpsAllData.TrackedSatellites,(int)(((llend-llstart)*1000)/1200));
                        printf("1GPS time: %d/%d/%d[%d:%d:%d]\n", gUbloxGpsAllData.Year, gUbloxGpsAllData.Month, gUbloxGpsAllData.Day ,
                               gUbloxGpsAllData.Hour ,gUbloxGpsAllData.Minute, gUbloxGpsAllData.Second);
                        //printf("dwtmp->%lx,dwres=%lx, (dwtmp-dwres=0x%ld), !((dwtmp-dwres) %(g_max_gps_time*100))->%lx\n",dwtmp,dwres, dwtmp-dwres, !((dwtmp-dwres)%(g_max_gps_time*100)));
                    }

                    g_llend = BspGetCpuCycle();
                    if ((((int)(((g_llend-g_llstart)*1000)/1200)) > 0) && (((int)(((g_llend-g_llstart)*1000)/1200)) < 508000000))
                    {
                        bsp_write_long_frame_to_fpga(dwtmp+DELAY_TO_WRITE*100);
                        g_real_write++;
                        if (g_gps_print_level == 1)
                        {
                            time(&now);
                            timenow = localtime(&now);
                            printf("GPS time: %d/%d/%d[%d:%d:%d]\n", gUbloxGpsAllData.Year, gUbloxGpsAllData.Month, gUbloxGpsAllData.Day ,
                                   gUbloxGpsAllData.Hour ,gUbloxGpsAllData.Minute, gUbloxGpsAllData.Second);
                            printf("Write Frame cost time is %d us;current time is %s\n", (int)(((((g_llend-g_llstart)*1000)/1200))/1000),asctime(timenow));
                            printf("readback dwtmp,dwtmp+DELAY_TO_WRITE*100->0x%lx,0x%lx\n",dwtmp,dwtmp+DELAY_TO_WRITE*100);
                            //printf("(dwtmp-dwres)->0x%lx ,readback data 192 193->0x%lx, 0x%lx\n",dwtmp-dwres,bsp_fpga_read_reg(192),bsp_fpga_read_reg(193));
                        }
                    }
                    else
                    {
                        g_time_out++;
                    }
                }
                bsp_delay_1ms(500);
            }
            else
            {
                if (BSP_OK != gps_NAV_SVInfo())
                {
                    printf("get gps_NAV_SVInfo error...\n");
                }
                if(g_gps_print_level == 1)
                    printf("gUbloxGpsAllData.TrackedSatellites=%d (<3)\n", gUbloxGpsAllData.TrackedSatellites);
            }
        }
    }
}
#define  USDPAA_IOC_REG7475  _IOR(USDPAA_IOC_MAGIC, 29, unsigned int)
extern u32 g_u32MasterSlaveSwitchCause;
extern s8 g_s8HardwareFault;
extern s8(*funcMasterSlaveSwitch)(u8 switchto,u32 cause);
extern SINT8 bsp_hmi_stop_send_message(void);
u32 g_u32MS_interrupt_print = 0;
VOID BspMasterToSlaveIrqIsr(ULONG ulVector)
{
    u8 tmp_u8_self_interrupt_incident;
    u8 tmp_u8_opp_interrupt_incident;
    int fd_tmp;
    int ret;
    int itmp;
    u8 u8slot;
    unsigned int dw_value;

    //BspIntcDisable(11);

    fd_tmp = open("/dev/usdpaa", O_RDWR);
    if (fd_tmp< 0)
    {
        return;
    }

    ret = ioctl(fd_tmp, USDPAA_IOC_REG7475, &dw_value);
    if (ret < 0)
    {
        printf("ioctl error!\r\n");
        close(fd_tmp);
        return ;
    }
    close(fd_tmp);

    tmp_u8_self_interrupt_incident = dw_value&0xff;
    tmp_u8_opp_interrupt_incident = (dw_value >> 8)&0xff;
    if(1 == g_u32MS_interrupt_print)
    {
        printf("Master Slave Interrupt!incident1---%2x,incident2---%2x\n",tmp_u8_self_interrupt_incident,tmp_u8_opp_interrupt_incident);
    }


    /*本板备升主*/
    if((tmp_u8_self_interrupt_incident >> MCT_SELF_SLAVE_to_MASTER )&0x01)
    {
        if(1 == g_u32MS_interrupt_print)
        {
            printf("self Slave to Master Interrupt!,switch cause: %d\n",g_u32MasterSlaveSwitchCause);
        }
        bsp_open_eth3_net();
        if (system("/sbin/ifconfig eth0 down;\
			/sbin/ifconfig eth0 hw ether 00:A0:1E:01:01:01 mtu 9000;\
					ifconfig eth0 up") < 0)
            perror("ERROR: Set eth0 failed ");

        bsp_send_gratuitous_arp();
        if(MCT_SWITCH_CASE_COMPETITION!=g_u32MasterSlaveSwitchCause)
        {
#if 1
            for(itmp=0; itmp < 3; itmp++)
            {
                if(BSP_OK == bsp_hmi_start_send_message())
                    break;
            }
            if(itmp==3)
                perror("ERROR:Send master slot hmi message failed\n");
#else
            for(u8slot=2; u8slot<11; u8slot++)
            {
                bsp_hmi_start_send_message(u8slot);
            }
#endif
            bsp_inform_bbp_MS_switch(bsp_get_slot_id());
        }
        bsp_led_act_on();

        if(funcMasterSlaveSwitch !=NULL)
        {
            funcMasterSlaveSwitch(MCT_MASTER,g_u32MasterSlaveSwitchCause);
        }
        g_u32MasterSlaveSwitchCause = MCT_MS_SWITCH_CASE_CLEAR;

        return;
    }

    /*************************************
    CPLD复位时有拔板事件，拔板时无复位事件
    拔板和复位事件同时存在时，当复位事件处理
    **************************************/

    /*对板复位*/
    if((tmp_u8_opp_interrupt_incident >> MCT_OPP_RESET)&0x01)
    {
        if(1 == g_u32MS_interrupt_print)
            printf("opp reboot Interrupt!\n");

        if(MCT_SLAVE == bsp_get_self_MS_state())
        {
            g_u32MasterSlaveSwitchCause = MCT_SWITCH_CASE_OPP_RESET;
            bsp_set_self_master();
        }

        return;
    }
    /*对板拔出*/
    else if((tmp_u8_opp_interrupt_incident >> MCT_OPP_PD_ON_to_OUT)&0x01)
    {
        if(1 == g_u32MS_interrupt_print)
        {
            printf("opp out Interrupt!\n");
        }
        if(MCT_SLAVE == bsp_get_self_MS_state())
        {
            g_u32MasterSlaveSwitchCause = MCT_SWITCH_CASE_OPP_OUT;
            bsp_set_self_master();
        }

        return;
    }
    /*对板主降备*/
    else if((tmp_u8_opp_interrupt_incident >> MCT_OPP_MASTER_to_SLAVE)&0x01)
    {
        if(1 == g_u32MS_interrupt_print)
        {
            printf("opp Master to Slave Interrupt!\n");
        }
        if(MCT_SLAVE == bsp_get_opp_MS_state())
        {
            bsp_set_self_master();

            return;
        }
        printf("self slave to master fail!\n");

        return;
    }

    /*本板主降备*/
    if((tmp_u8_self_interrupt_incident >> MCT_SELF_MASTER_to_SLAVE)&0x01)
    {
        bsp_led_act_off();
        if(1 == g_u32MS_interrupt_print)
        {
            printf("self Master to Slave Interrupt! switch cause: %d\n",g_u32MasterSlaveSwitchCause);
        }
        g_u32MasterSlaveSwitchCause = MCT_MS_SWITCH_CASE_CLEAR;
        return;
    }

    /*本板变不可用*/
    if((tmp_u8_self_interrupt_incident >> MCT_SELF_ENABLE_to_DISABLE)&0x01)
    {
        if(1 == g_u32MS_interrupt_print)
            printf("self aviable to disaviable Interrupt!\n");

        if(MCT_MASTER == bsp_get_self_MS_state())
            bsp_master_slave_switch(MCT_SLAVE,g_s8HardwareFault);

        return;
    }
    /*本板时钟不可用*/
    else if((tmp_u8_self_interrupt_incident >> MCT_SELF_CLOCK_to_DISABLE)&0x1)
    {
        if(1 == g_u32MS_interrupt_print)
            printf("self timer disaviable Interrupt!\n");

        if(MCT_MASTER == bsp_get_self_MS_state())
            bsp_master_slave_switch(MCT_SLAVE,MCT_SWITCH_CASE_CLK_DISABLE);

        return;
    }

    /*对板不可用*/
    if((tmp_u8_opp_interrupt_incident >> MCT_OPP_ENABLE_to_DISABLE)&0x01)
    {
        if(1 == g_u32MS_interrupt_print)
        {
            printf("opp aviable to unaviable Interrupt!\n");
        }

        return;
    }
    /*对板时钟不可用*/
    else if((tmp_u8_opp_interrupt_incident >> MCT_OPP_CLOCK_to_DISABLE)&0x01)
    {
        if(1 == g_u32MS_interrupt_print)
        {
            printf("opp timer disaviable Interrupt!\n");
        }

        return;
    }
    printf("Interrupt unknown cause !\n");

    return;
}

void BspExtIntInstallInit(ULONG ulVector)
{
    T_IntcAttrs tIntcAttrs;
    tIntcAttrs.IsrFunc = BspTestEIrqIsr;
    tIntcAttrs.ucTrgMode = 0x10;
    tIntcAttrs.udarg = ulVector;
    tIntcAttrs.udIntNo = ulVector;
    tIntcAttrs.ucTrgMode = 0x0;

    tIntcAttrs.udPrio = 10;

    BspIntcHookRegister((T_IntcAttrs *)&tIntcAttrs);
}

ULONG BspExtMasterToSlaveIrqSetup(UCHAR ucExtIrqVector)
{
    T_IntcAttrs tIntcAttrs;
    memset((UCHAR *)&tIntcAttrs,0,sizeof(T_IntcAttrs));
    tIntcAttrs.IsrFunc = BspMasterToSlaveIrqIsr;
    //tIntcAttrs.udarg   = ucExtIrqVector+90;
    //tIntcAttrs.udIntNo = ucExtIrqVector+90;
    tIntcAttrs.udarg   = ucExtIrqVector;
    tIntcAttrs.udIntNo = ucExtIrqVector;
    tIntcAttrs.ucTrgMode = 0x0;
    tIntcAttrs.udPrio  = 20;
    printf("tIntcAttrs.udarg->%d, tIntcAttrs.udIntNo->%d\n", tIntcAttrs.udarg,tIntcAttrs.udIntNo);
    BspIntcHookRegister((T_IntcAttrs *)&tIntcAttrs);
    BspIntcEnable(ucExtIrqVector);
    return BSP_OK;
}


/******************************************************************************
* 函数名: BspTestMsgIsr
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/

VOID  BspTestMsgIsr(ULONG ulVector)
{
    switch(ulVector)
    {
    case 235:
    {
        g_ulTestMsg[0] ++;
        BspMpicMsgIntGet(0);
    }
    break;
    case 236:
    {
        g_ulTestMsg[1] ++;
        BspMpicMsgIntGet(1);
    }
    break;
    case 237:
    {
        g_ulTestMsg[2] ++;
        BspMpicMsgIntGet(2);
    }
    break;
    case 238:
    {
        g_ulTestMsg[3] ++;
        BspMpicMsgIntGet(3);
    }
    break;
    case 239:
    {
        g_ulTestMsg[4] ++;
        BspMpicMsgIntGet(4);
    }
    break;
    case 240:
    {
        g_ulTestMsg[5] ++;
        BspMpicMsgIntGet(5);
    }
    break;
    case 241:
    {
        g_ulTestMsg[6] ++;
        BspMpicMsgIntGet(6);
    }
    break;
    case 242:
    {
        g_ulTestMsg[7] ++;
        BspMpicMsgIntGet(7);
    }
    break;
    default :
        break;
    }
}

/******************************************************************************
* 函数名: BspExtIrqInstall
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void BspExtIrqInstall(void)
{
    ULONG ulCnt = 0;
#if 0
    for(ulCnt = 0; ulCnt < 4; ulCnt ++)
    {
        BspIpiInstall(MPIC_VEC_IPI_IRQ0+ulCnt);
    }
    for(ulCnt = 0; ulCnt < 4; ulCnt ++)
    {
        BspMsgInstall(MPIC_VEC_MSGA_IRQ0+ulCnt);
    }
    for(ulCnt = 0; ulCnt < 4; ulCnt ++)
    {
        BspMsgInstall(MPIC_VEC_MSGB_IRQ0+ulCnt);
    }
#endif
    BspExtIrq10Setup(10);

    BspExtMasterToSlaveIrqSetup(11);/*主备倒换irq*/

    BspExtIrqI2cSetup(39);
    BspExtIrqUsbSetup(44);
}

/******************************************************************************
* 函数名: BspEnableInterrupt
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void BspEnableInterrupt(void)
{
    ULONG i=0;
#if 0
    for(i = 0; i < 4; i++)
    {
        BspIntcEnable(MPIC_IPI0_VECTOR  + i);
        BspIntcEnable(MPIC_MSG0_VECTOR  + i);
        BspIntcEnable(MPIC_MSG4_VECTOR  + i);
        BspIntcEnable(MPIC_VEC_GTA_IRQ0 + i);
    }
#endif
    BspIntcEnable(10);
}


/******************************************************************************
* 函数名: BspTestEIrqIsr
* 功  能:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void BspTestEIrqIsr(ULONG ulVector)
{

    switch(ulVector)
    {
    case 0:
    {
        g_ulTestExtInt[0] ++;
    }
    break;
    case 1:
    {
        g_ulTestExtInt[1] ++;
    }
    break;
    case 2:
    {
        g_ulTestExtInt[2] ++;
    }
    break;
    case 3:
    {
        g_ulTestExtInt[3] ++;
    }
    break;
    case 4:
    {
        g_ulTestExtInt[4] ++;
    }
    break;
    case 5:
    {
        g_ulTestExtInt[5] ++;
    }
    break;
    case 6:
    {
        g_ulTestExtInt[6] ++;
    }
    break;
    case 7:
    {
        g_ulTestExtInt[7] ++;
    }
    break;
    case 8:
    {
        g_ulTestExtInt[8] ++;
    }
    break;
    case 9:
    {
        g_ulTestExtInt[9] ++;
    }
    break;
    case 10:
    {
        g_ulTestExtInt[10] ++;
    }
    break;
    case 11:
    {
        g_ulTestExtInt[11] ++;
    }
    break;
    default :
        break;
    }
}

int bsp_get_if_flag(int index)
{
    int sock_fd, ret;
    struct ifreq ifreq;

    if (index < 0 || index > 3)
    {
        printf("error index %d\n", index);
        return -1;
    }

    sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (!sock_fd)
    {
        printf("Couldn't create socket\n");
        return -1;
    }

    sprintf(ifreq.ifr_name, "eth%d", index);

    ret = ioctl(sock_fd, SIOCGIFFLAGS, &ifreq);
    if (ret)
    {
        printf("Couldn't get info for if\n");
        perror("SIOCGIFFLAGS:");
        close(sock_fd);
        return -1;
    }

    close(sock_fd);

    if (ifreq.ifr_flags & IFF_RUNNING)
        return 1;
    else
        return 0;
}
/******************************* 源文件结束 ********************************/
