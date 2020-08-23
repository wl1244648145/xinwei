#ifndef __BSP_COM_API_H__
#define __BSP_COM_API_H__
#include <time.h>
#include "../../../com_inc/bsp_types.h"
#define RLIMIT_MSGQUEUE		12	/* maximum bytes in POSIX mqueues */
#define RLIM_INFINITY		0x7fffffffUL
//#define SRIO_SHARE_ALIGN              (0x8000000)
struct rlimit {
	unsigned long	rlim_cur;
	unsigned long	rlim_max;
};

void bsp_sys_msdelay(ULONG dwTimeOut);
void bsp_sys_usdelay(ULONG dwTimeOut);


typedef pid_t   OSSPID;
#define MAX_FOR_ARGS        (10)
#define QUEUE_NAME_LEN      (WORD32)40
#define APP_PROCESS_NAME    "mcw_app"

#define CTRL_NET_ETH  "eth3"
#define DATA_NET_ETH  "eth3"

#define MCH1_MAC_ADRS "00:e0:0c:00:ea:00"



typedef struct tagT_ThreadConfigInfo
{
	  WORD32  dwFlag;
    LPVOID  pEntry;/* 线程入口函数*/
    WORD32  dwPrio;/* 线程优先级  */
    WORD32  dwCoreId;
}T_ThreadConfigInfo;

typedef struct tagT_ThreadParmInfo
{
	  WORD32  dwParm0;
    WORD32  dwParm1;
    WORD32  dwParm2;
}T_ThreadParmInfo;

#define MIN_CPU_NUM    (0)
#define MAX_CPU_NUM    (4)
#define BSP_COREID_0   (0)
#define BSP_COREID_1   (1)
#define BSP_COREID_2   (2)
#define BSP_COREID_3   (3)


#endif /* BSP_H */
