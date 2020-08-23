/*
服务器主程序
*/
#include "mysocket.h"
#include "des.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "sys/ipc.h"


#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/socket.h>
#include "xos_external.h"
#include "ts_external.h"




#if __SIM_DSP__

char			hostipaddr[128];

typedef struct _T_TS_L3_ADDR_TMP 
{ 
    unsigned char   u8AddrLen;            
    unsigned char   au8Addr[20];          
} T_TS_L3_ADDR_TMP; 

#if 0
#define LOCAL_NET_ETH   "eth0"
unsigned long BspGetCtrlLocalIp(unsigned char * pucAddr) 
{
    unsigned char ip[4] = {0};
    if (0 != netif_get_ip((char*)LOCAL_NET_ETH,ip))
    {
        printf("bspgetctrlnetip error!\n");
        return -1;
    }
    else
    {
        sprintf((char *)pucAddr, "%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
        return 0;
    }
    return 0;
}
#endif

extern unsigned char DBS_GetEnbIpAddr(unsigned char *pu8IpAddr, unsigned int u32IpAddrLen);

#define SHMKEY 75
int shmid,i;
int *addr;


/*模块函数定义*/

static int rcv_data(int sockfd);

/*消息处理函数*/
void sig_handle(int signo)
{
	pid_t pid;
	int	stat;

	while((pid = waitpid(-1,&stat,WNOHANG)) > 0)
		printf("child %d terminated\n",pid);
	return ;	
}

void Bsp_Net_Recv_Init(int dwport)
{
	int 	lsnfd,confd;
	pid_t 	childpid;
	socklen_t	clilen;
	struct sockaddr_in cliaddr,servaddr;

	lsnfd = socket(AF_INET,SOCK_STREAM,0);//使用tcp协议
	if(lsnfd < 0)
	{
		printf("ERROR:create socket failed.\n");
		exit(1);
	}
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(dwport);//监听端口设为8888，必须和客户端相同
	if(bind(lsnfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)//邦定8888端口
	{
		printf("ERROR:bind port failed.\n");
		exit(1);
	}
	if(listen(lsnfd,LISTENQ) < 0)//监听端口
	{
		printf("ERROR:listen error.\n");
		exit(1);
	}
	signal(SIGCHLD,sig_handle);//注册消息处理函数
	printf("\tYou are Welcome!\n\tI am waiting for a connection......\n");
	while(1)
	{
		clilen = sizeof(cliaddr);
		//程序在此阻塞(停止，只有此程序返回后才执行后面的代码)，只到有客户端进行连接		
		confd = accept(lsnfd,(struct sockaddr *)&cliaddr,&clilen);
		if(confd < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}
			else	
			{
				printf("ERROR:accept failed!\n");	
				exit(1);
			}
		}
		//建立连接成功.
		childpid = fork();//如果有连接进来，则创建一个子进程来处理数据的接收
		if(childpid == 0) //childpid为零则表示当前是在子进程里
		{
			int e = 0;	
			close(lsnfd);//关闭父进程里的监听socket，因为子进程会复制父进程的所有资源
			printf("OK:I received a connection!\n");
			e = rcv_data(confd);//接收数据函数
			exit(e);
		}
		close(confd);//当前是在父进程.
	}

	close(lsnfd);//关闭连接
	exit(0);	
	
}

/*
接收数据的函数
参数sockfd是已经建立的socket连接的描述符
*/
static int rcv_data(int sockfd)
{
    size_t e = 0;
	  FILE *fd = NULL;
	  unsigned int icnt=0;
	  unsigned int count = 0;
	  char buf4receive[MAXLINE + 1] = {0};
	
	  if(sockfd < 0)//如果描述小于零则直接退出
   	{
		    printf("ERROR:bad socket!\n");
		    return (-1);
	  }
    while(1)
	  {
        count = readn(sockfd,buf4receive,MAXLINE);//从socket连接读取数据
		    if(count > 0) //如果count大于零则表示读到了数据
		    {
			      //BspEthCoreNetCallBack( buf4receive, count);
		    }
		    else if(count == 0)//如果count等于零表示数据接收完了
		    {
			      fclose(fd);
			      return (0);
		    }
		    else if(count == -1)//如果count等于负一则表示读数据出错了
		    {
			      printf("ERROR:read data failed!\n");
			      fclose(fd);
			      return (-2);
		    }
	  }
}




extern void(*EmacFromEth3ToCpu)(unsigned char *pbuf,int len);
extern void GetIpCpu(unsigned int *pudIp);
int server_main(void)
{
    int fd;
    int address_len;
    struct sockaddr_in address;
    T_TS_L3_ADDR_TMP ptL3Addr;
    int n=0;
    int icnttmp=0;
    int iResult = 0;
    fd = socket(AF_INET,SOCK_DGRAM,0);
    //fd = socket(AF_INET,SOCK_RAW,IPPROTO_UDP);
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = htonl(INADDR_ANY);
    //BspGetCtrlLocalIp(hostipaddr);
    NT_L3IpAddrGet(&ptL3Addr);
    address.sin_addr.s_addr=*(unsigned int *)ptL3Addr.au8Addr;//inet_addr("20.2.100.108");//本地ip地址
    //GetIpCpu(&address.sin_addr.s_addr);
    address.sin_port=htons(2152);
    address_len = sizeof(address);
    iResult = bind(fd,(struct sockaddr*)&address,address_len);
    while(1)
    {
        struct sockaddr_in client_address;
        socklen_t len = sizeof(client_address);
       
        unsigned char line[10240];
        fflush(stdout);
        n = recvfrom(fd,line + 42,sizeof(line) - 42,0,(struct sockaddr *)&client_address,&len);
        if (n>0)
        {
        	  //printf("recvfrom n=0x%lx\n",n);
        	  //if (client_address.sin_addr.s_addr == inet_addr("20.2.100.102") && client_address.sin_port == htons(2152))
              {
                  printf("recvfrom n=0x%lx\n",n);
                   printf("recv data ");
                  for (icnttmp=0;icnttmp<20;icnttmp++) 
                  {
                      printf("%02x ",line[icnttmp]);
                  }
                  printf("\n");
                  memcpy(line+14+12, &client_address.sin_addr.s_addr, 4); //copy the client address to Head
                  EmacFromEth3ToCpu(line,n + 42);
              }
            
        }
        //printf("server recv %d:%s",n,line);
        //sendto(fd,line,n,0,(struct sockaddr *)&client_address,len);
    }
}
#endif
