/************************************************************************
*   版权所有：北京信威通信股份有限公司
*   文件名称：SysOs.h
*   文件描述：操作系统的声明文件
*                              支持32  位操作系统allign 4 for struct;
*                              包含系统头文件
*				    重新定义基本数据类型
*				    定义了任务函数、信号量函数、
*				    读写互斥信号量、消息队列函数
*   修改记录：
*   1.  2005.11.08   JinWeimin   创建
*   2.  2006.07.24   liWenyan    修改
************************************************************************/

#ifndef __SYS_SYSOS_H__
#define __SYS_SYSOS_H__

#if !(defined(UNIX) || defined(VXWORKS) || defined(WINDOWS))
#error "invalid os or undefined os!"
#endif

#ifdef  __cplusplus
extern  "C"
{
#endif

/*****************************************************************************/
/*
 *	Include the system header from different system;
 */

/*---------------------------------------------------------------------------*/
#ifdef UNIX
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread.h>
#include <pthread.h>
#include <synch.h>
#include <semaphore.h> 	
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/varargs.h>
#include <sys/filio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
	
#endif

/*---------------------------------------------------------------------------*/

#ifdef VXWORKS
	
#include <vxWorks.h>
#include <semLib.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <tickLib.h>
#include <stdlib.h>
#include <msgQLib.h>
#include <taskLib.h>
#include <socket.h>
#include <sockLib.h>
#include <inetLib.h>
#include <ioLib.h>
#include <ioctl.h>
#include <wdLib.h>
#include <a_out.h>
#include <assert.h>

	
#endif
 
/*---------------------------------------------------------------------------*/

#ifdef WINDOWS
	
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>
	
#endif

/*****************************************************************************/

#define _CHAR                         char
#define _SHORT                       short
#define _LONG                         long
#define _INT                            int
	
#define _UCHAR                       unsigned char
#define _USHORT                     unsigned short
#define _ULONG                       unsigned long
#define _UINT                          unsigned int
	
#define _VOID                          void
#define _U                                unsigned

#define _FLOAT			       float
#define _DOUBLE                      double
	
#define SUCC                            0
#define FAIL                             1
	
#define BLANK_UCHAR             ((_UCHAR)0xFF)
#define BLANK_USHORT            ((_USHORT)0xFFFF)
#define BLANK_ULONG             ((_ULONG)0xFFFFFFFF)
	
#ifndef NULL
#define NULL                             (_VOID *)0
#endif
	
#define SWITCH_ON                  1
#define SWITCH_OFF                0
	
	
#define FLAG_YES                     1
#define FLAG_NO                       0

#define WARN_HAPPEN		0
#define WARN_RECOVER		1
	
#define SYS_MALLOC(ulFId, ulLen)                     malloc(ulLen)//SYS_Malloc((_ULONG)(ulFileId), __LINE__, (_ULONG)(ulFId), (_ULONG)(ulLen))
#define SYS_FREE(pMem)                                   free(pMem)//SYS_Free((_ULONG)(ulFileId), __LINE__, (void *)(pMem))
#define SYS_STRLEN(str)                                    strlen((char*)(str))
#define SYS_STRCPY(pTo, pFrom)                      strcpy((char*)(pTo), (char*)(pFrom) )
#define SYS_MEMCPY(pDest, pSrc, ulMemLen)   memcpy((void*)(pDest), (void*)(pSrc), (size_t)(ulMemLen) )
#define SYS_MEMSET(pBuf, iC, ulLen)                 memset((char*)(pBuf), (char)(iC), (int)(ulLen))
#define SYS_MEMMOVE(dest, src, count)            memmove(dest, src, count)
#define SYS_SSCANF                          sscanf
#define SYS_SPRINTF                         sprintf

/* 主机和网络字节序转换函数宏定义*/
#define SYS_HTONS(usHostShort)              (htons(usHostShort))
#define SYS_HTONL(ulHostLong)               (htonl(ulHostLong))

#define SYS_NTOHS(usNetShort)               (ntohs(usNetShort))
#define SYS_NTOHL(ulNetLong)                (ntohl(ulNetLong))
#define	TCP_NODELAY	0x01	

#ifndef FD_SETSIZE
#define FD_SETSIZE              256     /*select使用*/
#endif

#if (defined(WINDOWS) || (UNIX))
#define LOCAL      static

#define FOREVER  for(;;)
#endif


#ifdef VXWORKS
#define PKD  __attribute__ ((packed))
#define PKDN(n)  __attribute((aligned (n)))
#else
#define PKD
#define PKDN(n)
#endif

/*---------------------------------------------------------------------------*/
/*redefine data type about task*/
#ifdef UNIX
typedef pthread_t SYS_TASK_ID;
#endif

#ifdef VXWORKS
typedef int SYS_TASK_ID;
#endif

#ifdef WINDOWS
typedef unsigned long SYS_TASK_ID;
#endif

/*---------------------------------------------------------------------------*/
/*redefine data type about mutex*/
#ifdef UNIX
typedef sem_t SYS_MUTEX_ID;
#endif

#ifdef VXWORKS
typedef SEM_ID SYS_MUTEX_ID;
#endif

#ifdef WINDOWS
typedef CRITICAL_SECTION SYS_MUTEX_ID;
#endif

/*---------------------------------------------------------------------------*/
/*define the binary semaphore*/
#ifdef UNIX
typedef enum SEM_B_STATE
{
	SEM_EMPTY=0,
	SEM_FULL
} SEM_B_STATE;

typedef sem_t SYS_SEM_B_ID;
#endif

#ifdef VXWORKS
typedef SEM_ID	SYS_SEM_B_ID;
#endif

#ifdef WINDOWS
typedef enum SEM_B_STATE
{
	SEM_EMPTY=0,
	SEM_FULL
} SEM_B_STATE;

typedef HANDLE	SYS_SEM_B_ID;
#endif

/*---------------------------------------------------------------------------*/
#ifdef UNIX
/*Need to definateion*/
#endif

#ifdef VXWORKS
typedef MSG_Q_ID SYS_MSG_Q_ID;
#endif

#ifdef WINDOWS
#define NO_WAIT                 0
#define WAIT_FOREVER            -1
#define MSG_PRI_NORMAL          0
#define MSG_PRI_URGENT          1
typedef struct tagSysMsgqPipe
{
	int             receive;    /*接收*/
	int             send;       /*发送*/
}SYS_MSGQ_PIPE;

typedef struct tagWinMessageQ
{
	SYS_MUTEX_ID    mutex;
	SYS_MSGQ_PIPE   pipe;
}WIN_MESSAGE_QUEUE;
typedef WIN_MESSAGE_QUEUE SYS_MSG_Q_ID;
#endif

/*---------------------------------------------------------------------------*/

#ifdef UNIX
typedef int SYS_SOCKET_ID;
#endif

#ifdef VXWORKS
typedef int SYS_SOCKET_ID;
#endif

#ifdef WINDOWS
typedef SOCKET SYS_SOCKET_ID;
#endif

/*****************************************************************************/
typedef _INT (*TASKFUNC)();       
typedef struct _SYSTaskCB
{
    TASKFUNC    f;  
    VOID*       p;  
}SYS_TASK_CB;
_INT SYS_TaskCreate(_CHAR* pTaskName, _INT iTaskPri, _INT iTaskStackSize, TASKFUNC fTaskFun, VOID *pPar, SYS_TASK_ID *idTask);
_INT SYS_TaskRelease(SYS_TASK_ID task);
_INT SYS_Sleep(_UINT ms);

/*****************************************************************************/
_INT SYS_MutexCreate(SYS_MUTEX_ID* mutex);
_INT SYS_MutexRelease(SYS_MUTEX_ID* mutex);
_INT SYS_MutexLock(SYS_MUTEX_ID* mutex);
_INT SYS_MutexUnlock(SYS_MUTEX_ID* mutex);

/*****************************************************************************/
_INT SYS_SemBCreate(SYS_SEM_B_ID* semID, SEM_B_STATE initialState);
_INT SYS_SemBRelease(SYS_SEM_B_ID* semID);
_INT SYS_SemBTake(SYS_SEM_B_ID* semID);
_INT SYS_SemBGive(SYS_SEM_B_ID* semID);

/*****************************************************************************/
_INT SYS_MsgQCreate(_INT maxMsgs, _INT maxMsgLength, SYS_MSG_Q_ID* msgq);
#ifdef WINDOWS
_INT SYS_MsgQDelete1(SYS_MSG_Q_ID *msgq);
_INT SYS_MsgQSend1(SYS_MSG_Q_ID *msgq, _VOID* buffer, _INT len, _INT wait, _INT prio);
_INT SYS_MsgQRecv1(SYS_MSG_Q_ID *msgq, _VOID* buffer, _INT len, _INT wait);
_INT SYS_MsgQNumMsgs1(SYS_MSG_Q_ID *msgq);
#define SYS_MsgQDelete(msgq)                                       SYS_MsgQDelete1(&msgq)
#define SYS_MsgQSend(msgq, buffer, len, wait, prio)     SYS_MsgQSend1(&msgq, buffer, len, wait, prio)
#define SYS_MsgQRecv(msgq, buffer, len, wait)              SYS_MsgQRecv1(&msgq, buffer, len, wait)
#define SYS_MsgQNumMsgs(msgq)                                  SYS_MsgQNumMsgs1(&msgq) 
#else
_INT SYS_MsgQDelete(SYS_MSG_Q_ID msgq);
_INT SYS_MsgQSend(SYS_MSG_Q_ID msgq, _VOID* buffer, _INT len, _INT wait, _INT prio);
_INT SYS_MsgQRecv(SYS_MSG_Q_ID msgq, _VOID* buffer, _INT len, _INT wait);
_INT SYS_MsgQNumMsgs(SYS_MSG_Q_ID msgq);
#endif

/*****************************************************************************/

enum
{
    SYS_IP_TYPE_NOTUSE = 0,
    SYS_IP_TYPE_TCP,
    SYS_IP_TYPE_UDP,
    SYS_IP_TYPE_RAW,
    SYS_IP_TYPE_MAX
};

typedef struct tagSysIPAddr
{
    _ULONG      ip;     /*PV4地址*/
    _USHORT     port;   /*端口号*/
}PKD  SYS_IP_ADDR;
_INT SYS_SockIni();
_INT SYS_SockDes();
_INT SYS_SockGetError();
_INT SYS_SockOpen(_INT type, _INT iProtocol, SYS_SOCKET_ID* sock);
_INT SYS_SockSetBlockMode(SYS_SOCKET_ID sock, _INT mode);
_INT SYS_SockGetV4Addr(_CHAR* cIP, _ULONG* addr);
_INT SYS_SockBind(SYS_SOCKET_ID sock, SYS_IP_ADDR* addr);
_INT SYS_SockListen(SYS_SOCKET_ID sock);
_INT SYS_SockAccept(SYS_SOCKET_ID sock, SYS_SOCKET_ID* sock1, SYS_IP_ADDR *stPeerAddr);
/*_INT SYS_SockAccept(SYS_SOCKET_ID sock, SYS_SOCKET_ID* sock1);*/
_INT SYS_SockConnV4(SYS_SOCKET_ID sock, SYS_IP_ADDR* addr);
_INT SYS_SockRecv(SYS_SOCKET_ID sock, _UCHAR* buf, _INT size);
_INT SYS_SockSend(SYS_SOCKET_ID sock, _UCHAR* buf, _INT len);
_INT SYS_SockURecv(SYS_SOCKET_ID sock, _UCHAR* buf, _INT size, SYS_IP_ADDR* addr);
_INT SYS_SockUSend(SYS_SOCKET_ID sock, _UCHAR* buf, _INT len, SYS_IP_ADDR* addr);
_INT SYS_SockClose(SYS_SOCKET_ID sock);
_INT SYS_SockIsConnected(SYS_SOCKET_ID sock);


_VOID SYS_PRINT(const _UCHAR * szDbgStr);


#ifdef  __cplusplus
}
#endif

#endif
