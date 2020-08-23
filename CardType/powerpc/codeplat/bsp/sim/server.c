/*
������������
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


/*ģ�麯������*/

static int rcv_data(int sockfd);

/*��Ϣ������*/
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

	lsnfd = socket(AF_INET,SOCK_STREAM,0);//ʹ��tcpЭ��
	if(lsnfd < 0)
	{
		printf("ERROR:create socket failed.\n");
		exit(1);
	}
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(dwport);//�����˿���Ϊ8888������Ϳͻ�����ͬ
	if(bind(lsnfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)//�8888�˿�
	{
		printf("ERROR:bind port failed.\n");
		exit(1);
	}
	if(listen(lsnfd,LISTENQ) < 0)//�����˿�
	{
		printf("ERROR:listen error.\n");
		exit(1);
	}
	signal(SIGCHLD,sig_handle);//ע����Ϣ������
	printf("\tYou are Welcome!\n\tI am waiting for a connection......\n");
	while(1)
	{
		clilen = sizeof(cliaddr);
		//�����ڴ�����(ֹͣ��ֻ�д˳��򷵻غ��ִ�к���Ĵ���)��ֻ���пͻ��˽�������		
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
		//�������ӳɹ�.
		childpid = fork();//��������ӽ������򴴽�һ���ӽ������������ݵĽ���
		if(childpid == 0) //childpidΪ�����ʾ��ǰ�����ӽ�����
		{
			int e = 0;	
			close(lsnfd);//�رո�������ļ���socket����Ϊ�ӽ��̻Ḵ�Ƹ����̵�������Դ
			printf("OK:I received a connection!\n");
			e = rcv_data(confd);//�������ݺ���
			exit(e);
		}
		close(confd);//��ǰ���ڸ�����.
	}

	close(lsnfd);//�ر�����
	exit(0);	
	
}

/*
�������ݵĺ���
����sockfd���Ѿ�������socket���ӵ�������
*/
static int rcv_data(int sockfd)
{
    size_t e = 0;
	  FILE *fd = NULL;
	  unsigned int icnt=0;
	  unsigned int count = 0;
	  char buf4receive[MAXLINE + 1] = {0};
	
	  if(sockfd < 0)//�������С������ֱ���˳�
   	{
		    printf("ERROR:bad socket!\n");
		    return (-1);
	  }
    while(1)
	  {
        count = readn(sockfd,buf4receive,MAXLINE);//��socket���Ӷ�ȡ����
		    if(count > 0) //���count���������ʾ����������
		    {
			      //BspEthCoreNetCallBack( buf4receive, count);
		    }
		    else if(count == 0)//���count�������ʾ���ݽ�������
		    {
			      fclose(fd);
			      return (0);
		    }
		    else if(count == -1)//���count���ڸ�һ���ʾ�����ݳ�����
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
    address.sin_addr.s_addr=*(unsigned int *)ptL3Addr.au8Addr;//inet_addr("20.2.100.108");//����ip��ַ
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
