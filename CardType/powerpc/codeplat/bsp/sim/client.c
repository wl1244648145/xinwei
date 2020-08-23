#include "xos_external.h"
#include "mysocket.h"
#include "tcp_connect.h"
#include "des.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#if __SIM_DSP__

extern unsigned char DBS_GetEnbIpAddr(unsigned char *pu8IpAddr, unsigned int u32IpAddrLen);


void GetIpCpu(unsigned int *pudIp)
{
	unsigned int au32Ip[10];

	DBS_GetEnbIpAddr((unsigned char*)au32Ip, 40); 

	pudIp[0] = au32Ip[0];
}

#define CORE_NET_IP_ADDR  "20.2.100.102"
#define LOCAL_NET_IP_ADDR "20.2.100.108"
typedef struct _T_QREF_MSG_BSP
{
    S8 *ps8Buf;
    U16 u16MsgLen;
}T_QREF_MSG_BSP;


mqd_t g_QidDspToCpuRecv0 = 0;
mqd_t g_QidDspToCpuRecv1 = 0;


extern mqd_t gudEmacDataQue;
extern mqd_t gudEmacCfgQue;
int fd;
struct sockaddr_in address;
static int encrypt_data(const int count,char *input,char *output);//加密数据函数
//static int send_data(int sockfd);//传送数据函数
#define HOSTIPADDR   "127.0.0.1"
#define LISTENPORT   "8888"

void(*EmacFromEth0ToCpu)(unsigned char *pbuf,int len)  ;
void(*EmacFromEth1ToCpu)(unsigned char *pbuf,int len)  ;
void(*EmacFromEth3ToCpu)(unsigned char *pbuf,int len)  ;


void BspClientSendData(unsigned char *pbuf,int len)
{
    struct ip *header_ip;

    len = len - 14;
    pbuf += 14;
    
    if (len <=64)
    {
        header_ip = (struct ip *)pbuf;
        sendto(fd,pbuf,ntohs(header_ip->ip_len),0,(struct sockaddr *)&address,sizeof(address));
    }    
    else
    {
        sendto(fd,pbuf,len,0,(struct sockaddr *)&address,sizeof(address));
    }
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

/*
传送数据，这里是用从一个文件读取数据传送给服务器来模拟流媒体数据。
*/
#if 0
static int send_data(int sockfd)
{
    unsigned int count = 0;
	  char buf4send[MAXLINE + 1] = {0};
	  writen(sockfd,buf4send,count);//穿送给服务器
	  return (0);
}
#endif

unsigned int BspSendIpData(int dwport,unsigned char *pbuf,int len)
{
    unsigned char *udTemp; 
    T_QREF_MSG_BSP tRMsg;
    switch(dwport)
    {
    case 1:
    #if __SIM_DSP__
            if ( 0!=gudEmacDataQue)
            {
                udTemp = (unsigned char *)XOS_MALLOC(len + 16); 
                memset(udTemp,0xaa,16);
                memcpy((void *)(udTemp+16), (void *)pbuf, len);
                tRMsg.ps8Buf = (S8 *)udTemp;
                tRMsg.u16MsgLen = len+16;
                printf("gudEmacDataQue send!,udtmp=0x%lx....................\n",udTemp);	
                VOS_QSend(gudEmacDataQue, &tRMsg, sizeof(T_QREF_MSG_BSP));
	        }	
            else
            {
             printf("gudEmacDataQue error!.\n");	
            }
          //BspShmemWrite(pbuf,len);
#endif
	      	break;
	      case 0:
#if __SIM_DSP__
            if ( 0!=gudEmacCfgQue)
            {

                udTemp = (unsigned char *)XOS_MALLOC(len + 16); 
                memset(udTemp,0xaa,16);
                memcpy((void *)(udTemp+16), (void *)pbuf, len);
                tRMsg.ps8Buf = (S8 *)udTemp;
                tRMsg.u16MsgLen = len+16;
                printf("gudEmacCfgQue send!,udtmp=0x%lx........\n",udTemp);
                VOS_QSend(gudEmacCfgQue, &tRMsg, sizeof(T_QREF_MSG_BSP));

	      	}
            else
            {
             printf("gudEmacCfgQue error!\n");	
            }
               //BspShmemWrite(pbuf,len);
#endif
	      	break;
	      case 2:
	      	//debug口不用
	      	break;
	      case 3:
	      	BspClientSendData(pbuf,len);
	      	break;
	      default:
	      	break;
	  }
    return 0;	
}

#if 1
int g_DspToCpuRecv0Flag=0;
int g_DspToCpuRecv1Flag=0;

unsigned char BspCreateQueue(void)
{
     g_QidDspToCpuRecv0 = VOS_CreateQueue("DspToCpuRecv0",1024,sizeof(T_QREF_MSG_BSP));
     g_QidDspToCpuRecv1 = VOS_CreateQueue("DspToCpuRecv1",1024,sizeof(T_QREF_MSG_BSP));
     
	 g_DspToCpuRecv0Flag=1;
	 g_DspToCpuRecv1Flag=1;
	 
     return 1;
}
//extern void *CoreNetDataRecv(VOID *pVoid);
//extern void *DspToCpuRecv0(VOID *pVoid);
//extern void *DspToCpuRecv1(VOID *pVoid);

//void BspEthSwPort1CallBack(unsigned char *pbyReceiveData, unsigned long dwDataLen)
//void BspEthSwPort2CallBack(unsigned char *pbyReceiveData, unsigned long dwDataLen)
void *DspToCpuRecv0(void *pVoid)
{
    T_QREF_MSG_BSP tRMsg;    
    void *pMsgBuf = NULL;
#if 1
    //g_QidDspToCpuRecv0 = VOS_CreateQueue("DspToCpuRecv0",1024,sizeof(T_QREF_MSG_BSP));
    if (1 != g_DspToCpuRecv0Flag)
    {
        sleep(1);
    }
	else
	{
    while(!VOS_QRecv(g_QidDspToCpuRecv0, (S8*)&tRMsg, sizeof(T_QREF_MSG_BSP)))
    {
        printf("recv g_QidDspToCpuRecv0!..........................................................................\n");	
        pMsgBuf = XOS_MALLOC(tRMsg.u16MsgLen); 
        memcpy(pMsgBuf, (void *)tRMsg.ps8Buf, tRMsg.u16MsgLen);
        //VOS_QSend(QID qID, S8 *ps8Buf, U32 u32Len);
        EmacFromEth0ToCpu(pMsgBuf,tRMsg.u16MsgLen);
        if (NULL != tRMsg.ps8Buf)
        {
            XOS_FREE(tRMsg.ps8Buf); 
        }
        if (NULL != pMsgBuf)
        {
            XOS_FREE(pMsgBuf); 
        }
    }
	}
#endif
}

void *DspToCpuRecv1(void *pVoid)
{
    T_QREF_MSG_BSP tRMsg;
    void *pMsgBuf = NULL;
#if 1
   // g_QidDspToCpuRecv1 = VOS_CreateQueue("DspToCpuRecv1",1024,sizeof(T_QREF_MSG_BSP));
    if(1 != g_DspToCpuRecv1Flag)
    {
        sleep(1);
    }
	else
	{
    while(!VOS_QRecv(g_QidDspToCpuRecv1, (S8*)&tRMsg, sizeof(T_QREF_MSG_BSP)))
    {
        //printf("DspToCpuRecv0asl;dfjasdklfjasl;kfjl;sjkfl;asdjfl;jkasl;jfasdkl;fjl;asdkg;hsdl;")
         printf("recv g_QidDspToCpuRecv1!..........................................................................\n");	
         pMsgBuf = XOS_MALLOC(tRMsg.u16MsgLen); 
        memcpy(pMsgBuf, (void *)tRMsg.ps8Buf, tRMsg.u16MsgLen);
        //VOS_QSend(QID qID, S8 *ps8Buf, U32 u32Len);
        EmacFromEth1ToCpu(pMsgBuf,tRMsg.u16MsgLen);
        if (NULL != tRMsg.ps8Buf)
        {
            XOS_FREE(tRMsg.ps8Buf); 
        }
        if (NULL != pMsgBuf)
        {
            XOS_FREE(pMsgBuf); 
        }
        //free(tRMsg.ps8Buf);
        //free(udTemp);
    }
	}
#endif
}

#endif

int fd;
struct sockaddr_in address;
int Init_Connect(void)
{
    
    int address_len;
    char line[80]="client to server!\n";
    int n;
    //fd = socket(AF_INET,SOCK_DGRAM,0);
    fd = socket(AF_INET,SOCK_RAW,IPPROTO_UDP);
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr=inet_addr(CORE_NET_IP_ADDR);
    GetIpCpu(&address.sin_addr.s_addr);
    address.sin_port=htons(2152);
    
    const int on =1;
    if (setsockopt(fd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on))<0)
    {
        printf("setsockopt() error!\n");
    }
    
    address_len=sizeof(address);

    return 0;
}

extern int server_main(void);
void bsp_init(void)
{
  int i,j;
    int ret =0;
    int ret1 = 0;
    pthread_t  pid,pid1;
    //while(1)
    {
        if (Init_Connect() == 0)
        {
            printf("init connect ok!\n");   
            //break;
        }
        usleep(100);  
    }

    ret = pthread_create(&pid, NULL, (void*)server_main, &i);
    if (ret != 0)
    {
        printf("pthread_create  failed\n");
        return (-1);
    }
    //pthread_join(pid,NULL); 
    //pthread_join(pid1,NULL);  
}

int client_main(void)
{
    int fd;
    struct sockaddr_in address;
    int address_len;
    char	host[128];
    char line[80]="client to server!\n";
    int n;
    unsigned char pu8ipaddr[4];
    DBS_GetEnbIpAddr(pu8ipaddr,4);
   sprintf(host, "%d.%d.%d.%d",pu8ipaddr[0],pu8ipaddr[1],pu8ipaddr[2],pu8ipaddr[3]);
    fd = socket(AF_INET,SOCK_DGRAM,0);
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr=inet_addr(host);//inet_addr("20.2.100.102");
    address.sin_port=htons(2152);
    address_len=sizeof(address);
    sendto(fd,line,strlen(line)+1,0,(struct sockaddr *)&address,sizeof(address));
    n = recvfrom(fd,line,80,0,NULL,NULL);
    printf("recv%d %s\n",n,line);
}

int BspSendToUser(unsigned char *pbuf, int len)
{
	return BspSendIpData(3, pbuf, len);
}
#endif

