/************************************************************************
*   版权所有：北京信威通信股份有限公司
*   文件名：ipTask.h
*	文件描述: IP模块常量定义和数据结构定义
*   修改记录：
*   	1.  2006.7.5   Li Wei   创建
************************************************************************/
#ifndef _IP_TASK_H_
#define _IP_TASK_H_

//#include "../Public/mpu.h"
#include "SysOs.h"


#ifdef  __cplusplus
extern  "C"
{
#endif


/*接收到的IP包最长长度*/
#define MAX_IP_DATA_LEN                 2090

#define MAX_IP_CCB_NUM             32
#define TELNET_PORT                   32
#define TELNET_MAX_BLOCKED_TIME (240) //240 seconds
#define SIZEOF(slice)         (sizeof(slice)/sizeof((slice)[0]))

/*内部消息定义*/
enum
{
    IP_MSG_TM_CONNECT = 0,
    IP_MSG_INNER_MAX
};

/* socket  状态定义*/
enum
{
    SYS_IP_STATE_IDLE = 0,              /*空闲*/
    SYS_IP_STATE_OCCUPY,              /*占用*/
    SYS_IP_STATE_OPENED,              /*打开了*/
    SYS_IP_STATE_LISTENED,            /*正在监听*/
    SYS_IP_STATE_CONNECTTING,     /*正在连接*/
    SYS_IP_STATE_CONNECTTED,      /*已连接*/
    SYS_IP_STATE_RUNNING,            /*运行,可以传送数据*/
    SYS_IP_STATE_MAX
};

enum
{
    SYS_IP_CLASS_SERVER = 0,        /*服务器端*/
    SYS_IP_CLASS_SERVERCLIENT,    /*服务器端启动的客户连接*/
    SYS_IP_CLASS_CLIENT,               /*客户端*/
    SYS_IP_CLASS_MAX
};

/*TCP客户端连接的优先方式*/
typedef enum
{
    TCP_CLIENT_PRIO_SQUENCE = 0,    /*顺序式*/
    TCP_CLIENT_PRIO_FORCE,             /*强制式，即抢占式*/

    MAX_TCP_CLIENT_PRIO
}TCP_CLIENT_PRIO;

/*IP业务类型*/
enum
{
    IP_MAINTAIN=0,     /*维护管理类*/
    IP_CONTROL,            /*控制平面类*/
    IP_USER                  /*用户平面类*/
};

#pragma pack(1)

typedef struct _tagSysIPCcb
{
    _UCHAR                     ucState;        /*状态*/
    _UCHAR                     ucIndex;     /*CCB下标*/
    _UCHAR                     ucUser;         /*业务使用者，指使用socket  的任务*/
    _UCHAR                     ucProvider;     /*业务提供者*/
    _INT                        iType;           /*方式(1:TCP 2:UDP 3:RAW)*/
    _INT                        iProt;            /*协议号*/
    _UINT                       uiClass;        /*类型,如果是TCP方式,这里表示:0=侦听,1=服务器端启动的客户连接,2=客户*/
    _UCHAR                    ucMinCcb, ucMaxCcb;/*如果是侦听的SOCKET,这里是连接的客户CCB*/
    _UCHAR                    ucClientPrio;   /*客户端连接来时的优先级别,见枚举[TCP_CLIENT_PRIO](当是TCP连接时有效)*/
    _UINT                       uiTmConn;       /*如果是连接的客户端,这里是Connect间隔时长,单位为ms*/
    
    SYS_IP_ADDR             stMyAddr;       /*自己的IP地址*/
    SYS_IP_ADDR              stPeerAddr;     /*对端的IP地址*/
    SYS_SOCKET_ID           sock;           /*SOCKET*/
    //HTIMER                     usTimerId;
}PKD SYS_IP_CCB;

typedef struct tagSysIPData
{

    SYS_SOCKET_ID           maxFd;          //select使用的maxFd
    _INT        PipeFd;
    _INT       CliPipeFd;
    fd_set                  fdRead;         //读文件描述字
    fd_set                  fdWrite;        //写文件描述字
    fd_set                  fdException;    //异常文件描述字
    _UCHAR                  ucBuffer[sizeof(COMMON_HEADER) + MAX_IP_DATA_LEN];   //接收消息缓冲区
    SYS_IP_CCB              stCcb[MAX_IP_CCB_NUM];//控制块
}PKD SYS_IP_DATA;



typedef struct tagSysIPTCPConfig
{
    _UINT                   uiClass;        /*子类别,如果是TCP方式,这里表示:R3_SYS_IP_CLASS_SERVER=侦听,R3_SYS_IP_CLASS_CLIENT=客户*/
    _UINT                   uiTmConn;       /*如果是TCP的客户端,这里表示connect超时间隔,单位为ms*/
    _UCHAR                 ucMinCcb;       /*如果是TCP的服务器端,这里表示可以控制的最小的CCB下标*/
    _UCHAR                  ucMaxCcb;       /*如果是TCP的服务器端,这里表示可以控制的最大的CCB下标*/
    _UCHAR                  ucClientPrio;   /*客户端连接来时的优先级别,见枚举[TCP_CLIENT_PRIO]*/
}PKD SYS_IP_TCP_CONFIG;

typedef struct tagSysIPConfig
{    
    _UCHAR               ucCcbIndex;     /*CCB下标*/
    _UCHAR               ucUser;         /*业务使用者，指socket 使用任务*/
    _INT                    iType;          /*SOCKET类型,R3_SYS_IP_TYPE_TCP/R3_SYS_IP_TYPE_UDP/R3_SYS_IP_TYPE_RAW*/
    _INT                    iProt;          /*协议号*/
    
    SYS_IP_ADDR             stMyAddr;       /*自己的IP地址*/
    SYS_IP_ADDR             stPeerAddr;     /*对端IP地址*/
    SYS_IP_TCP_CONFIG       stTCPCfg;       /*如果是TCP节点,包含这个部分*/
}PKD SYS_IP_CONFIG;

#pragma pack()


#define SYS_IPFdSet(sock, set) \
{ \
    if (sock >= m_pTelnet->maxFd) \
        m_pTelnet->maxFd = sock + 1; \
    FD_SET(sock, set);\
}





_INT SYS_IPStart(_USHORT usPort);
_INT SYS_MsgSend2IP(LONG_MESSAGE *pstMsg);

//Telnet 入口
extern  _ULONG CLI_IniProc();
extern _INT SendMsgTelnet(LONG_MESSAGE *pstMsg);


#ifdef  __cplusplus
}
#endif
#endif