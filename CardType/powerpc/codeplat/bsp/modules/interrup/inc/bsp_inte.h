#ifndef __BSP_INTERRUPT_H
#define __BSP_INTERRUPT_H
#if 0
#include "bsp_types.h"
#define IA_DEVICE_NAME "/dev/IntAgent"
#define IA_MAJOR 10  /*Misc Drive Major*/
#define IA_MINOR 244
#define IA_SUCCESS       0
#define IA_ERROR         1
#define IA_TIMEOUT       2
#define IA_SIGOUT        3   /*等待状态中，被信号打断*/
#define IA_FAULT         4   /*内存访问错误*/
#define IA_OBJ_DELETE    5   /*对象被删除*/
      

#define MEM_DEVICE_NAME "/dev/mem"

typedef struct  {
    ULONG dev_id; /* Device ID */
    ULONG rs;  /* Operation Return Status */
    ULONG rv; /* Operation Return Value*/
    ULONG d0;  /* Operation specific data */
    ULONG d1;
    ULONG d2;
    ULONG d3;
    ULONG dx[17];
}ia_ioctl_t;

#define IA_CMD_MAGIC 0x44410000
#define _IA_CMD(magic,num) (IA_CMD_MAGIC + num)
enum
{  
    /*INT CMD*/
    IA_INTERRUPT_REQUEST=0,
    IA_INTERRUPT_FREE,
    IA_INTERRUPT_ENABLE,
    IA_INTERRUPT_DISABLE,
    IA_WAIT_FOR_INTERRUPT,
    IA_INTERRUPT_LOCK,
    IA_INTERRUPT_UNLOCK,
    /*MISC cmd*/
    IA_GET_SCHE_TIME,
    /*MUX CMDS*/
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

#define IA_NR_IRQS        1000
 
typedef struct __tag_Act_Irq
{
    ULONG   IrqCnt;
    ULONG   IrqStat;
    ULONG   IrqReserved[5];  /*预留*/
    ULONG  IrqNumArray[IA_NR_IRQS]; 
}ia_act_irq_t;

static ULONG ia_mult_int_mem_addr = 0;
typedef void (*ia_irq_func_t)(void*);
typedef struct 
{
    void *data;
    ia_act_irq_t      *s_tActIrq; //add
}ia_handler_parm_t;

typedef struct 
{
    ia_handler_parm_t handler_parm_t;
    ia_irq_func_t handler;
    UINT32 vector;
}ia_irq_handler_t;

ia_irq_handler_t saIaIrqHandlerArray[IA_NR_IRQS];
#endif

#endif /* BSP_H */
