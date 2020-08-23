#include "../inc/bsp_interrupt.h"
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#if 0
int    g_iIaDrvFd;
int    siMemDrvFd;
UINT32  requestIrq(UINT32 vector, ia_irq_func_t isr_func,void *data)
{
    ia_ioctl_t devio;
    UINT32 ret;
    ret = OK;
    devio.dev_id = 0;
    devio.d0 = (unsigned int)vector;
    devio.d1 = ia_mult_int_mem_addr;
    //printf("request irq %d, addr = 0x%lx\n",vector,ia_mult_int_mem_addr);
    ioctl(g_iIaDrvFd, IA_INTERRUPT_REQUEST, &devio);
    //printf("setup requestirq!\n");
    if (devio.rs != IA_SUCCESS)
    {
        return ERROR;
    }
    saIaIrqHandlerArray[vector].handler = isr_func;
    saIaIrqHandlerArray[vector].handler_parm_t.data = data;
    saIaIrqHandlerArray[vector].vector  = vector;
    return OK;
}

void * iaIrqThread()
{
    struct sched_param param;
    UINT32    i,curr_vec,err;
    static ia_ioctl_t     devio;
    static ia_act_irq_t   s_atActIrqNumArray;
    ULONG cpumask;
    cpumask = 1<<0;	
    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
    {
        printf("pthread_setaffinity_iaIrqThread failed in %s\n", __FUNCTION__);
    }
    memset(&s_atActIrqNumArray,0,sizeof(s_atActIrqNumArray));
    /*The realtime scheduling priority (1 - 99), this task need the higherst*/
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
    for (;;)
    {
        devio.d0 = &s_atActIrqNumArray;
        devio.d1 = ia_mult_int_mem_addr;
        ioctl(g_iIaDrvFd, IA_WAIT_FOR_INTERRUPT, &devio);
        #if 1
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
                printf("!!! IA curr_vec err: %d ,IrqCnt:%d \n",curr_vec,s_atActIrqNumArray.IrqCnt);
            }
        }
        #endif
    }
}



 
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
#endif
