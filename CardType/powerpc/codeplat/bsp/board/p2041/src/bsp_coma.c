#include "../inc/bsp_comapi.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>


#define bcopy(src, dst, size)\
    memcpy(dst, src, size)
void Thread_Entry1(int i);
void Thread_Entry2(int i);
void Thread_Entry3(int i);
void Thread_Entry4(int i);

typedef unsigned int (*RECVGMAC_FUNCPTR)(unsigned char *u8Buf, int u32Len);
void(*EmacFromEth0ToCpu)(unsigned char *pbuf,int len);
void(*EmacFromEth1ToCpu)(unsigned char *pbuf,int len);
void(*EmacFromEth3ToCpu)(unsigned char *pbuf,int len);
void(*EmacFromEth0ToDsp)(unsigned char *pbuf,int len);
void(*EmacFromEth0ToDspTest)(unsigned char *pbuf,int len);

#define __NR_gettid    207 

/********************************************************************************
* 函数名称: system
* 功    能: 替换系统的system函数     
* 参    数: const char * cmdstring 命令字符串
* 返回值: 0 表示成功；其它值表示失败。		
*********************************************************************************/
int system(const char * cmdstring)
{
	int status = 0;
	pid_t pid;
	if ((pid = vfork())<0)
	{
		printf("[%s]:vfork process error!\n", __func__);
		status = -1;
	}
	else if (pid==0)
	{
		if (execl("/bin/sh", "sh", "-c", cmdstring, (char *)0)<0)
		{
			printf("[%s]:fail to execl %s!\n",__func__, cmdstring);
			_exit(1);
		}
		_exit(0);
	}
	else
	{   
        #if 0
	    waitpid(pid, &status, 0);
		#endif
		while(waitpid(pid, &status, 0) < 0)
		{
			printf("[%s]:waitpid error!\n",__func__);
		    status = -1;
		    break;
		}
	}
	return status;
}

pid_t gettid1(VOID)
{
    return (pid_t)syscall(__NR_gettid);
}

void bsp_print_reg_info(char *fun, char *file, int d)
{
	struct sched_param parm;

	sched_getparam(0, &parm);

	printf("########register info:%d,%s,%s,%d proiority %d\n",gettid1(), fun, file, d, parm.sched_priority);
}
unsigned int BspL3EmacRegRecvCallBack(RECVGMAC_FUNCPTR pCallBack)
{
    EmacFromEth0ToCpu = pCallBack;
    return 0;
}

unsigned int BspEmacRegRecvCallBack(RECVGMAC_FUNCPTR pCallBack)
{
    EmacFromEth3ToCpu = pCallBack;
    return 0;
}

unsigned int BspInterNetEmacRegRecvCallBack(RECVGMAC_FUNCPTR pCallBack)
{
    EmacFromEth1ToCpu = pCallBack;     
    return 0;
}

unsigned int BspCpuDspRecvCallBack(RECVGMAC_FUNCPTR pCallBack)
{
    EmacFromEth0ToDsp = pCallBack;     
    return 0;
}
unsigned int BspCpuDspTestRecvCallBack(RECVGMAC_FUNCPTR pCallBack)
{
    EmacFromEth0ToDspTest = pCallBack;     
    return 0;
}

static T_ThreadConfigInfo gaUserThreadConfigTable[] =
{
    { TRUE, &Thread_Entry1 ,99,BSP_COREID_0}
 //   { TRUE, &Thread_Entry2 ,99,BSP_COREID_1},
//    { TRUE, &Thread_Entry3 ,99,BSP_COREID_2},
 //   { TRUE, &Thread_Entry4 ,99,BSP_COREID_3}
};

/********************************************************************************
* 函数名称: netif_get_ip							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-09-15
*********************************************************************************/
int netif_get_ip(const char *ifname, unsigned char *ipaddr)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		perror("ioctl SIOCGIFADDR:");
		close(fd);
		return -1;
	}
	close(fd);
	bcopy(&ifr.ifr_addr.sa_data[2], ipaddr, sizeof(struct in_addr));
	
	return 0;
}

/********************************************************************************
* 函数名称: netif_get_ip							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-09-15
*********************************************************************************/
ULONG BspGetCtrlNetIp(UCHAR * pucAddr) 
{
    UCHAR ip[4] = {0};
    if (BSP_OK != netif_get_ip((char*)CTRL_NET_ETH,ip))
    {
        printf("bspgetctrlnetip error!\n");
        return BSP_ERROR;
    }
    else
    {
        sprintf(pucAddr, "%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
	 //printf("pucAddr->%s\n",pucAddr);
        return BSP_OK;
    }
    return BSP_OK;
}

/********************************************************************************
* 函数名称: bsp_sys_msdelay							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-08-15
*********************************************************************************/
void bsp_sys_msdelay(ULONG dwTimeOut) 
{
    struct timespec delay;
    delay.tv_sec  = dwTimeOut/1000;
    delay.tv_nsec = (dwTimeOut%1000)*1000*1000;
    if (dwTimeOut == 0)
    {
        delay.tv_nsec = 1;
    }
    nanosleep(&delay, NULL);
}
/********************************************************************************
* 函数名称: bsp_sys_usdelay							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-08-15
*********************************************************************************/
void bsp_sys_usdelay(ULONG dwTimeOut) 
{
    struct timespec delay;
    delay.tv_sec  = dwTimeOut/1000000;
    delay.tv_nsec = (dwTimeOut%1000000)*1000;
    if (dwTimeOut == 0)
    {
        delay.tv_nsec = 1;
    }
    nanosleep(&delay, NULL);  
}

void bsp_clear_queue_limit(void)
{
    struct rlimit rlim,rlim_new;
	  if (getrlimit(RLIMIT_MSGQUEUE, &rlim)==0) 
	  {
	      rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
	      if (setrlimit(RLIMIT_MSGQUEUE, &rlim_new)!=0) 
	      {
	          rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
	          setrlimit(RLIMIT_MSGQUEUE, &rlim_new);
	      }
	  }
}

void BspConfigSysQueue(void)
{
    system("sysctl -w fs.mqueue.msg_max=50");	
    system("sysctl -w fs.mqueue.msgsize_max=8190");	
    system("sysctl -w fs.mqueue.queues_max=512");	
    system("telnetd");
}

void BspConfigMaxTransConfig(void)
{
    system("sysctl -w net.core.rmem_default=16777216");
    system("sysctl -w net.core.rmem_max=16777216");
    system("sysctl -w net.core.wmem_default=16777216");
    system("sysctl -w net.core.wmem_max=16777216");
}

void bsp_set_task_schedule(void)
{
    struct sched_param param;
    sched_getparam(0,&param);  
    param.__sched_priority = 97; 
    sched_setscheduler(0,SCHED_FIFO,&param);
    sched_getparam(0,&param);
    printf("******************__sched_priority:%d ************%d\n",param.__sched_priority,getpid());    
}

void m4(ULONG uladdr, ULONG ulval)
{
    *(ULONG*)(uladdr) = ulval;
}

void d4 (ULONG uladdr,ULONG len)
{
    int i=0;
    if ((len%16))
    {
    	printf("address not aligned\n");
    	return ;
    }
    for(i=0;i<len;i=i+16)
    {
        printf("\n 0x%08lx: %08lx  %08lx  %08lx  %08lx", 
		    uladdr+i, 
			  *((WORD32*)(uladdr+i)), 
			  *((WORD32*)(uladdr+i+4)), 
			  *((WORD32*)(uladdr+i+8)), 
			  *((WORD32*)(uladdr+i+12)));
    } 
}

int errnoGet(void)
{
    return errno;
}

pid_t bsp_create_process(CHAR *pcFileName,CHAR **ppcArgs)
{
    CHAR    *apcArgs[MAX_FOR_ARGS];
    pid_t pid;
    if(access(pcFileName,R_OK)!=0)
    {
        printf("bsp_create_process:%s access error!\n",pcFileName);
    	  return -1;
    }
    if ((pid = fork())<0)
    {
        printf("fork process error\n");
        return -1;
    }
    else if (pid==0) /*子进程分支*/
    {
        if (execl(pcFileName,pcFileName,(char *)apcArgs[0],(char *)apcArgs[1],(char *)apcArgs[2],
                  (char *)apcArgs[3],(char *)apcArgs[4],(char *)apcArgs[5],(char *)apcArgs[6],
                  (char *)apcArgs[7],(char *)apcArgs[8],(char *)apcArgs[9],(char *)0)<0)
        {
            printf("fail to execl:%s, errno is %d, %s\n", pcFileName, errnoGet(), strerror(errnoGet()));
            exit(1);
        }
        return (pid);
    }
    else if (pid > 0) /*父进程分支*/
    {
    }
    return (pid);
}

void bsp_load_product_app(void)
{
    WORD32  dwProcessId = 0;
    dwProcessId = bsp_create_process(APP_PROCESS_NAME,0);/*启动应用进程 */
    printf("dwProcessId=%d\n",dwProcessId);
    if ((WORD32)(-1) == dwProcessId)
    {
	      printf("main: detect Single Process !\n");
    }	
    else
    {
        printf("main: detect Muti Process !\n");
    }
}

int bsp_get_netif_mac_addr(const char *ifname, unsigned char *mac)
{
    struct ifreq ifr;
	  struct sockaddr *hwaddr = &(ifr.ifr_hwaddr);
	  int fd;
	  fd = socket(AF_INET, SOCK_DGRAM, 0);
	  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	  ifr.ifr_addr.sa_family = AF_INET;
	  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) 
	  {
		    perror("ioctl SIOCGIFHWADDR:");
		    close(fd);
		    return -1;
	  }
	  memcpy(mac, (unsigned char *)hwaddr->sa_data,IFHWADDRLEN);
	  close(fd);		
	  return 0;	
}

/********************************************************************************
* 函数名称: BspGetDataNetMac							
* 功    能:                                     
* 相关文档:                    
* 函数类型:								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 表示成功；其它值表示失败。								
* 说   明: 
* 作者:刘刚
*日期:2013-08-15
*********************************************************************************/
ULONG BspGetDataNetMac(UCHAR * pucAddr)
{
    if (BSP_OK != bsp_get_netif_mac_addr((CHAR*)DATA_NET_ETH,pucAddr))
    {
        printf("MAC Address Get Error\n");
        return BSP_ERROR;
    }
    else
    {
        printf("MAC ADDR: %x:%x:%x:%x:%x:%x\n",pucAddr[0],pucAddr[1],pucAddr[2],pucAddr[3],pucAddr[4],pucAddr[5]);
        return BSP_OK;
    }
    return BSP_OK;
}


extern int web_app();
void bsp_webserver_init(void)
{
    web_app();
}

void Thread_Entry1(int i)
{
    unsigned long cpumask;
    if (i<MIN_CPU_NUM || i>MAX_CPU_NUM)
        return ; 
    cpumask = (1 << i);
    printf("now init webserver!\n");
    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
    {
        printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
        return ;
    }
    while(1)
    {
        bsp_webserver_init();	
    } 
}

void Thread_Entry2(int i)
{
    unsigned long cpumask;
    if (i<MIN_CPU_NUM || i>MAX_CPU_NUM)
        return ; 
    cpumask = (1 << i);
    printf("loding Thread_Entry2!!\n");
    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
    {
        printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
        return ;
    }
    while(1)
    {
    	
    }
}

void Thread_Entry3(int i)
{
    unsigned long cpumask;
    if (i<MIN_CPU_NUM || i>MAX_CPU_NUM)
        return ; 
    cpumask = (1 << i);
    printf("loding Thread_Entry3!!\n");
    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
    {
        printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
        return ;
    }
    while(1)
    {
    	
    } 
}

void Thread_Entry4(int i)
{
    unsigned long cpumask;
    if (i<MIN_CPU_NUM || i>MAX_CPU_NUM)
        return ; 
    cpumask = (1 << i);
    printf("loding Thread_Entry4!!\n");
    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
    {
        printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
        return ;
    }
    while(1)
    {
    	
    } 
}

int bsp_attach_core(T_ThreadConfigInfo *pThreadTypeTable)
{
    pthread_t rx_thread;
    pthread_attr_t attr;
    struct sched_param param;
    int result = 0;
    pthread_attr_init(&attr); 
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = (pThreadTypeTable)->dwPrio; 
    pthread_attr_setschedparam(&attr, &param);
    result = pthread_create(&rx_thread, &attr, (void *)((pThreadTypeTable)->pEntry),(void *)(pThreadTypeTable)->dwCoreId);
    if (result != 0)
    {
        printf("pthread_create rx_thread failed\n");
        pthread_attr_destroy(&attr);
        return (-1);
    }
    pthread_attr_destroy(&attr);
    return 0;
}

UINT32 ThreadConfigInit(T_ThreadConfigInfo *pThreadTypeTable,WORD16 dwTableItemCounts)
{
    WORD16  dwThreadTypeIndex = 0;
    if ((NULL == pThreadTypeTable) || (0 == dwTableItemCounts))
    {
        return BSP_ERROR;
    }
    for (dwThreadTypeIndex = 0; dwThreadTypeIndex < dwTableItemCounts; dwThreadTypeIndex++)
    {
        if (TRUE == (pThreadTypeTable + dwThreadTypeIndex)->dwFlag)
        {
            bsp_attach_core(pThreadTypeTable + dwThreadTypeIndex);
        }
    }
    return BSP_OK;
}

void BspAttachCoreTest(void)
{
   // ThreadConfigInit(gaUserThreadConfigTable,(WORD16)sizeof(gaUserThreadConfigTable)/sizeof(T_ThreadConfigInfo));	
}


void BspTimeDelay(unsigned long usec)
{


}

#define MAX_BUF_LEN 128
extern void bsp_get_next_hop_mac(char *ethname,char *srcip,char *destip);
extern char strDesMAC[MAX_BUF_LEN];
unsigned char gaucCoreNetPacketHead[14];
unsigned char g_nexthopaddr[128];

int iremotemacflag=0;
unsigned int BspGetRemoteMacAddr(char *pdestip)
{
    char host[128];
    memcpy(g_nexthopaddr,pdestip,128);
    iremotemacflag = 1;
    BspGetCtrlNetIp((unsigned char *)host);
    bsp_get_next_hop_mac(CTRL_NET_ETH,host,pdestip);
    memcpy(gaucCoreNetPacketHead,strDesMAC,6);
    return BSP_OK;
	
}
pid_t bsp_get_tid(void)
{
    return syscall(SYS_gettid);
}


void bsp_reset_cpu(void)
{
	bsp_set_self_slave();
    system("reboot");
}


