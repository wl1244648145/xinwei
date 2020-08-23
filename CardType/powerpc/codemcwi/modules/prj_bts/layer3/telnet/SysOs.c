/************************************************************************
*   版权所有：北京信威通信股份有限公司
*   文件名称：SysOs.h
*   文件描述：操作系统的实现文件
*                              支持32  位操作系统allign 4 for struct;
*                              包含系统头文件
*				    重新定义基本数据类型
*				    定义了任务函数、信号量函数、
*				    读写互斥信号量、消息队列函数
*   修改记录：
*   1.  2005.11.08   JinWeimin   创建
*   2.  2006.07.24   liWenyan    修改
************************************************************************/



#include "SysOs.h"

/*****************************************************************************/

/*--------------------------------------------------------------------------------
//      任务处理函数
//  输入:
//      p:                  参数
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//------------------------------------------------------------------------------*/
_INT SYS_TaskRoutine(VOID *p)
{
    SYS_TASK_CB cb;
    SYS_TASK_CB *cb1;
    
    if(NULL == p)
    {
        return(FAIL);
    }
    cb1 = (SYS_TASK_CB*)p;
    cb.f = cb1->f;
    cb.p = cb1->p;
    SYS_FREE(p);//这里可能以后修改为动态内存管理
    cb.f(cb.p);
    return(SUCC);
}
/*--------------------------------------------------------------------------------
//      操作系统相关任务函数
//  输入:
//      p:                  参数
//  返回:
//      各个操作系统不一
//------------------------------------------------------------------------------*/
#ifdef UNIX
void* SYS_TaskProc(void* p)
{
    SYS_TaskRoutine(p);
    return 0;
}
#endif

#ifdef VXWORKS
int SYS_TaskProc(void* p)
{
    SYS_TaskRoutine(p);
    return 0;
}
#endif

#ifdef WINDOWS
void SYS_TaskProc(void *p)
{
    SYS_TaskRoutine(p);
}
#endif

/*--------------------------------------------------------------------------------
//      创建任务
//  输入:
//      pTaskName:          任务名
//      iTaskPri:           任务优先级
//      iTaskStackSize:     堆栈大小
//      fTaskFun:           任务处理函数
//      pPar:               参数
//  返回:
//      idTask:             任务标识
//      SUCC:               成功
//      FAIL:               失败
//------------------------------------------------------------------------------*/
_INT SYS_TaskCreate(_CHAR *pTaskName, _INT iTaskPri, _INT iTaskStackSize, TASKFUNC fTaskFun, VOID *pPar, SYS_TASK_ID *idTask)
{
    SYS_TASK_CB* cb;
    
    cb = (SYS_TASK_CB*)SYS_MALLOC(FID_SYS, sizeof(SYS_TASK_CB));
    if(0 == cb)
    {
        return(FAIL);
    }
    cb->f = fTaskFun;
    cb->p = pPar;

#ifdef UNIX
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if(0 == pthread_create(idTask, &attr, SYS_TaskProc, cb))
        {
            return(SUCC);
        }
        return(FAIL);
    }
#endif

#ifdef WINDOWS
    {
        *idTask = _beginthread(SYS_TaskProc, iTaskStackSize, cb);
        return(NULL != (void *)*idTask ? SUCC : FAIL);
    }
#endif
    
#ifdef VXWORKS
    if (iTaskStackSize <= 0)
    {
        iTaskStackSize = 40960;//40k
    }
    *idTask = taskSpawn(pTaskName, iTaskPri, 0, iTaskStackSize, (FUNCPTR)SYS_TaskProc,
        (int)cb, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    return(ERROR != *idTask ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//      删除任务
//  输入:
//      task:               任务标识
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//------------------------------------------------------------------------------*/
_INT SYS_TaskRelease(SYS_TASK_ID task)
{
#ifdef UNIX
    return(0 == pthread_cancel(task) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    TerminateThread((HANDLE)task, 0);
    return(0 != CloseHandle((HANDLE)task) ? SUCC : FAIL);
#endif

#ifdef VXWORKS
    return(OK != taskDelete(task) ? SUCC : FAIL);
#endif
}
/*--------------------------------------------------------------------------------
//      任务睡眠一段时间
//  输入:
//      ms:                 时间长度(million second)
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//------------------------------------------------------------------------------*/
_INT SYS_Sleep(_UINT ms)
{
#ifdef UNIX
    struct timespec req;
    struct timespec rem;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&req, &rem);
    return(SUCC);
#endif

#ifdef WINDOWS
    Sleep(ms);
    return(SUCC);
#endif

#ifdef VXWORKS
    taskDelay(ms * sysClkRateGet() / 1000);
    return(SUCC);
#endif
}

/*--------------------------------------------------------------------------------
//      互斥量初始化
//  输入:
//      mutex:              回传的互斥量
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//------------------------------------------------------------------------------*/
_INT SYS_MutexCreate(SYS_MUTEX_ID* mutex)
{
#ifdef UNIX
    return(0 == mutex_init(mutex, USYNC_THREAD, NULL) ? SUCC : FAIL);/*在单个进程内*/
#endif

#ifdef WINDOWS
    InitializeCriticalSection(mutex);
    return(SUCC);
#endif

#ifdef VXWORKS
    *mutex = semMCreate(SEM_Q_PRIORITY |SEM_INVERSION_SAFE|SEM_DELETE_SAFE);/*首先给一个信号量*/
    return((NULL != *mutex) ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//      互斥量释放
//  输入:
//      mutex:              互斥量
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//------------------------------------------------------------------------------*/
_INT SYS_MutexRelease(SYS_MUTEX_ID* mutex)
{
#ifdef UNIX
    return(0 == mutex_destroy(mutex) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    DeleteCriticalSection(mutex);
    return(SUCC);
#endif

#ifdef VXWORKS
    return(OK == semDelete(*mutex) ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//      互斥量加锁
//  输入:
//      mutex:              锁
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//-------------------------------------------------------------------------------*/
_INT SYS_MutexLock(SYS_MUTEX_ID* mutex)
{
#ifdef UNIX
    return(0 == mutex_lock(mutex) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    EnterCriticalSection(mutex);
    return(SUCC);
#endif

#ifdef VXWORKS
    return(ERROR != semTake(*mutex, WAIT_FOREVER) ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//      互斥量解锁
//  输入:
//      mutex:              锁
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//-------------------------------------------------------------------------------*/
_INT SYS_MutexUnlock(SYS_MUTEX_ID* mutex)
{
#ifdef UNIX
    return(0 == mutex_unlock(mutex) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    LeaveCriticalSection(mutex);
    return(SUCC);
#endif

#ifdef VXWORKS
    return(ERROR != semGive(*mutex) ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//  SYS_SemBCreate     
//  输入:
//      SYS_SEM_B_ID* semID;二进制信号量；
//		SEM_B_STATE initialState;二进制信号量状态；
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//-------------------------------------------------------------------------------*/
_INT SYS_SemBCreate(SYS_SEM_B_ID* semID, SEM_B_STATE initialState)
{
#ifdef UNIX
	return(0 == sem_init(semID, USYNC_THREAD, initialState==SEM_FULL?1:0) ? SUCC : FAIL);/*在单个进程内*/
#endif

#ifdef VXWORKS
    *semID = semBCreate(SEM_Q_FIFO, initialState);/*首先给一个信号量*/
    return((NULL != *semID) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
	*semID=CreateSemaphore(NULL, initialState==SEM_FULL?1:0, 1, NULL);
	return ((*semID==INVALID_HANDLE_VALUE)? FAIL : SUCC);
#endif
}

/*--------------------------------------------------------------------------------
//  SYS_SemBRelease     
//  输入:
//      SYS_SEM_B_ID* semID;二进制信号量；
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//-------------------------------------------------------------------------------*/
_INT SYS_SemBRelease(SYS_SEM_B_ID* semID)
{
#ifdef UNIX
	return(0 == sem_destroy(mutex) ? SUCC : FAIL);
#endif

#ifdef VXWORKS
    return(OK == semDelete(*semID) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
	return (TRUE==CloseHandle(*semID)?SUCC:FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//  SYS_SemBTake     
//  输入:
//      SYS_SEM_B_ID* semID;二进制信号量；
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//-------------------------------------------------------------------------------*/
_INT SYS_SemBTake(SYS_SEM_B_ID* semID)
{
#ifdef UNIX
	return(0 == sem_wait(mutex) ? SUCC : FAIL);
#endif

#ifdef VXWORKS
    return(ERROR != semTake(*semID, WAIT_FOREVER) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
	return (WAIT_FAILED==WaitForSingleObject(*semID,INFINITE)?FAIL:SUCC);
#endif
}

/*--------------------------------------------------------------------------------
//  SYS_SemBGive     
//  输入:
//      SYS_SEM_B_ID* semID;二进制信号量；
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//-------------------------------------------------------------------------------*/
_INT SYS_SemBGive(SYS_SEM_B_ID* semID)
{
#ifdef UNIX
	return(0 == sem_post(mutex) ? SUCC : FAIL);
#endif

#ifdef VXWORKS
    return(ERROR != semGive(*semID) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
	return (TRUE==ReleaseSemaphore(*semID, 1, NULL)?SUCC:FAIL);
#endif
}


/*---------------------------------------------------------------------
 SYS_MsgQCreate - 消息队列创建
 参数说明：
 maxMsgs            － 输入，最大消息数目
 maxMsgLength       － 输入，最大消息长度
 msgq               － 输出，消息队列号
 返回值: SUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
_INT SYS_MsgQCreate(_INT maxMsgs, _INT maxMsgLength, SYS_MSG_Q_ID *msgq)
{
    if (NULL == msgq)
    {
        return(FAIL);
    }

#ifdef UNIX
#endif

#ifdef VXWORKS
    *msgq = msgQCreate(maxMsgs, maxMsgLength, MSG_Q_FIFO);
    return((NULL != *msgq) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    if (SUCC != SYS_MutexCreate(&msgq->mutex))
        return FAIL;
    return((0 == _pipe((int*)&msgq->pipe, (maxMsgLength + sizeof(int)) * maxMsgs, _O_BINARY))
        ? SUCC : FAIL);
#endif
}

/*---------------------------------------------------------------------
 SYS_MsgQDelete - 删除一个消息队列
 参数说明：
 msgq           － 输入，消息队列号
 返回值: SUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
#ifdef WINDOWS
_INT SYS_MsgQDelete1(SYS_MSG_Q_ID *msgq)
#else
_INT SYS_MsgQDelete(SYS_MSG_Q_ID msgq)
#endif
{
#ifdef UNIX
#endif

#ifdef VXWORKS
    return((OK == msgQDelete(msgq)) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    close(msgq->pipe.send);
    close(msgq->pipe.receive);
    SYS_MutexRelease(&msgq->mutex);
    return(SUCC);
#endif
}

/*---------------------------------------------------------------------
 SYS_MsgQSend - 发送一条消息到指定消息队列
 参数说明：
 msgq           - 输入，消息队列号
 buffer         - 输入，消息内容
 len            - 输入，消息长度
 wait           - 输入，等待标志
 prio           - 输入，优先级
 返回值: SUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
#ifdef WINDOWS
_INT SYS_MsgQSend1(SYS_MSG_Q_ID *msgq, _VOID* buffer, _INT len, _INT wait, _INT prio)
#else
_INT SYS_MsgQSend(SYS_MSG_Q_ID msgq, _VOID* buffer, _INT len, _INT wait, _INT prio)
#endif
{
#ifdef UNIX
#endif

#ifdef VXWORKS
    return((OK == msgQSend(msgq, (char*)buffer, (UINT)len, wait, prio)) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    if (SUCC != SYS_MutexLock(&msgq->mutex))
        return FAIL;
    if (-1 == write(msgq->pipe.send, &len, sizeof(len)))
    {
        SYS_MutexUnlock(&msgq->mutex);
        return(FAIL);
    }
    if (-1 == write(msgq->pipe.send, buffer, len))
    {
        SYS_MutexUnlock(&msgq->mutex);
        return(FAIL);
    }
    SYS_MutexUnlock(&msgq->mutex);
    return(SUCC);
#endif
}

/*---------------------------------------------------------------------
 SYS_MsgQRecv - 从指定消息队列接收一条消息
 参数说明：
 msgq           - 输入，消息队列号
 buffer         - 输入，消息内容存放缓冲区
 len            - 输入，消息长度
 wait           - 输入，等待标志
 返回值: 接收到的消息实际长度
 -1:失败
------------------------------------------------------------------------*/
#ifdef WINDOWS
_INT SYS_MsgQRecv1(SYS_MSG_Q_ID *msgq, _VOID* buffer, _INT len, _INT wait)
#else
_INT SYS_MsgQRecv(SYS_MSG_Q_ID msgq, _VOID* buffer, _INT len, _INT wait)
#endif
{
#ifdef UNIX
#endif

#ifdef VXWORKS
    return (msgQReceive(msgq, (char*)buffer, (UINT)len, wait));
#endif

#ifdef WINDOWS
    _INT    iTmpLen, iBufLen, i;
    _UCHAR  *pBuf;

    if (sizeof(int) != read(msgq->pipe.receive, &iTmpLen, sizeof(int)))
    {
        return -1;
    }
    if (iTmpLen > len || 0 == iTmpLen)
    {
        return -1;
    }
/*    if (iTmpLen != read(msgq.receive, buffer, iTmpLen))
    {
        return(FAIL);
    }
    return(iTmpLen);*/
    iBufLen = 0;
    pBuf = (_UCHAR*)buffer;
    while(1)
    {
        i = read(msgq->pipe.receive, &pBuf[iBufLen], iTmpLen - iBufLen);//注意这里以iTmpLen - iBufLen计算，不读多的数据
        if (i <= 0)//0:结束,-1:错误句柄
        {
            return -1;//出错，直接返回0
        }
        iBufLen += i;
        if (iBufLen >= iTmpLen)//接收完成
            break;
    }
    return iBufLen;
#endif
}

/*---------------------------------------------------------------------
 SYS_MsgQNumMsgs - 获取指定消息队列的现有消息数目
 参数说明：
 msgq           - 输入，消息队列号
 返回值: 消息队列的现有消息数目
------------------------------------------------------------------------*/
#ifdef WINDOWS
_INT SYS_MsgQNumMsgs1(SYS_MSG_Q_ID *msgq)
#else
_INT SYS_MsgQNumMsgs1(SYS_MSG_Q_ID msgq)
#endif
{
#ifdef UNIX
#endif

#ifdef VXWORKS
    return(msgQNumMsgs(msgq));
#endif

#ifdef WINDOWS
    return 0;
#endif
}

//SOCKET
//--------------------------------------------------------------------------------
//      SOCKET系统初始化
//  输入:
//      无
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockIni()
{
#ifdef UNIX
    return(SUCC);
#endif
    
#ifdef WINDOWS
    WSADATA ws;
    return(0 == WSAStartup(MAKEWORD(2, 1), &ws) ? SUCC : FAIL);
#endif
    
#ifdef VXWORKS
    return(SUCC);
#endif
}
//--------------------------------------------------------------------------------
//      SOCKET系统销毁
//  输入:
//      无
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockDes()
{
#ifdef UNIX
    return(SUCC);
#endif
    
#ifdef WINDOWS
    return (0 == WSACleanup() ? SUCC : FAIL);
#endif
    
#ifdef VXWORKS
    return(SUCC);
#endif
}
//--------------------------------------------------------------------------------
//      获取错误码
//  输入:
//      无
//  返回:
//      1：             没有数据可读
//      2：             SOCKET已经关闭或其他错误
//--------------------------------------------------------------------------------
_INT SYS_SockGetError()
{
#ifdef UNIX
    return 1;
#endif
    
#ifdef WINDOWS
    return ((WSAGetLastError() == WSAEWOULDBLOCK) ? 1 : 2);
#endif

#ifdef VXWORKS
    return 1; /* errnoGet();*/
#endif
}
//--------------------------------------------------------------------------------
//      打开一个SOCKET
//  输入:
//      type:               方式(1:TCP 2:UDP 3:RAW)
//      protocol:           协议号
//  返回:
//      sock:               SOCKET句柄
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockOpen(_INT type, _INT iProtocol, SYS_SOCKET_ID *sock)
{
    int optval;

    if (SYS_IP_TYPE_RAW == type)//RAW SOCKET
    {
        sock[0] = socket(AF_INET, SOCK_RAW, iProtocol);
#ifdef WINDOWS
        //在WINDOWS下使用RAW SOCKET,必须先触发一下,否则将不能收到数据包
        if (-1 != sock[0])
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = 0;
            addr.sin_addr.s_addr = 0;
            bind(sock[0], (struct sockaddr*)&addr, sizeof(addr));
        }
#endif
    }
    else
    {
        sock[0] = socket(AF_INET, (SYS_IP_TYPE_TCP == type) ? SOCK_STREAM : SOCK_DGRAM, 0);
    }
    if (-1 == sock[0])
        return(FAIL);
    optval = 1;
    if (1 == type)//如果是TCP
    {
        //setsockopt(sock[0], SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof (optval));
        setsockopt (sock[0], IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof (optval));
    
    }
    //设置为可重用的,64K缓冲,非阻塞方式
    setsockopt(sock[0], SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
    optval = 65536;//64k
    setsockopt(sock[0], SOL_SOCKET, SO_RCVBUF, (char*)&optval, sizeof(optval));
    setsockopt(sock[0], SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval));
    
    optval = 200;//128k
    SYS_SockSetBlockMode(sock[0], 1);//设置为非阻塞方式
    return(SUCC);
}
//--------------------------------------------------------------------------------
//      设置一个SOCKET非阻塞方式
//  输入:
//          sock:           SOCKET句柄
//          mode:           0:阻塞,1:非阻塞
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockSetBlockMode(SYS_SOCKET_ID sock, _INT mode)
{
#ifdef UNIX
    int flag = mode;
    return (-1 != ioctl(sock, FIONBIO, (int)&flag) ? SUCC : FAIL);
#endif
    
#ifdef WINDOWS
    long flag = mode;
    return (0 == ioctlsocket(sock, FIONBIO, &flag) ? SUCC : FAIL);
#endif
    
#ifdef VXWORKS
    int flag = mode;
    return (ERROR != ioctl(sock, FIONBIO, (int)&flag) ? SUCC : FAIL);
#endif
}
//--------------------------------------------------------------------------------
//      将IPV4地址转换成为内部地址
//  输入:
//      ip：                IP地址串 "10.0.10.28"
//  返回:
//      addr                IPV4地址
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockGetV4Addr(_CHAR *cIP, _ULONG *addr)
{
    *addr = inet_addr(cIP);
    return(SUCC);
}
//--------------------------------------------------------------------------------
//      绑定本地端口
//  输入:
//      sock：              SOCKET句柄
//      addr:               地址
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockBind(SYS_SOCKET_ID sock, SYS_IP_ADDR *addr)
{
    struct sockaddr_in s;

    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr->port);
    s.sin_addr.s_addr = addr->ip;
    return (0 == bind(sock, (struct sockaddr*)&s, sizeof(s)) ? SUCC : FAIL);
}
//--------------------------------------------------------------------------------
//      侦听一个SOCKET
//  输入:
//      sock：              SOCKET句柄
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockListen(SYS_SOCKET_ID sock)
{
#ifdef UNIX
    return (0 == listen(sock, 8) ? SUCC : FAIL);//LISTENQ
#endif
    
#ifdef WINDOWS
    return (0 == listen(sock, 8) ? SUCC : FAIL);
#endif
    
#ifdef VXWORKS
    return (OK == listen(sock, 8) ? SUCC : FAIL);
#endif
}
//--------------------------------------------------------------------------------
//      接受一个SOCKET
//  输入:
//      sock：              SOCKET句柄
//  返回:
//      sock1:              连接的SOCKET句柄
//      SUCC:               成功
//      FAIL:               失败

// modify by liwei 2006.7.31 增加对端ip地址的返回,TCP连接需要根据对端Ip分配IP FSM
//--------------------------------------------------------------------------------
_INT SYS_SockAccept(SYS_SOCKET_ID sock, SYS_SOCKET_ID* sock1, SYS_IP_ADDR *stPeerAddr)
/*_INT SYS_SockAccept(SYS_SOCKET_ID sock, SYS_SOCKET_ID* sock1)*/
{   
    struct sockaddr_in s;
    int len;
  
    if(stPeerAddr == NULL)
        return FAIL;    
    
    memset(&s, 0, sizeof(s));
    len = sizeof(s);
  

    //这里不将对方IP地址传递到用户，即安全控制完全由上层协议控制，与传输无关
    *sock1 = accept(sock, (struct sockaddr*)&s, &len);
    if (sock1[0] != -1)
    {
        SYS_SockSetBlockMode(sock1[0], 1);//设置为非阻塞方式
        stPeerAddr->ip = s.sin_addr.s_addr;
        stPeerAddr->port = SYS_NTOHS(s.sin_port);
    }
    return ((*sock1 != -1) ? SUCC : FAIL);
}
//--------------------------------------------------------------------------------
//      连接到一个IP+PORT
//  输入:
//      sock：              SOCKET句柄
//      addr:               对端地址
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockConnV4(SYS_SOCKET_ID sock, SYS_IP_ADDR* addr)
{
#ifdef UNIX
    struct sockaddr_in s;

    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr->port);
    s.sin_addr.s_addr = addr->ip;
    connect(sock, (struct sockaddr*)&s, sizeof(s));
    return(SUCC);
    //发connect调用,这时返回-1,但是errno被设为EINPROGRESS,意即connect仍旧  
    //在进行还没有完成
#endif
    
#ifdef WINDOWS
    struct sockaddr_in s;
    int lasterror;

    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr->port);
    s.sin_addr.s_addr = addr->ip;
    if (0 == connect(sock, (struct sockaddr*)&s, sizeof(s)))
        return(SUCC);
    lasterror = WSAGetLastError();
    if (WSAEWOULDBLOCK == lasterror || WSAEINPROGRESS == lasterror)
        return(SUCC);
    return(FAIL);
#endif
    
#ifdef VXWORKS
    struct sockaddr_in s;

    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr->port);
    s.sin_addr.s_addr = addr->ip;
    connect(sock, (struct sockaddr*)&s, sizeof(s));
    return(SUCC);
    //发connect调用,这时返回-1,但是errno被设为EINPROGRESS,意即connect仍旧  
    //在进行还没有完成
#endif
}
//--------------------------------------------------------------------------------
//      接收TCP数据
//  输入:
//      sock：              SOCKET句柄
//      buf：               缓冲区
//      size：              缓冲区大小
//  返回:
//      _INT：              接收数据长度
//--------------------------------------------------------------------------------
_INT SYS_SockRecv(SYS_SOCKET_ID sock, _UCHAR* buf, _INT size)
{
    _INT i;
    i = recv(sock, (char*)buf, size, 0);

#ifdef UNIX
    if (0 == i || -1 == i)
        return 0;
#endif

#ifdef WINDOWS
    if (0 == i || SOCKET_ERROR == i)
        return 0;
#endif

#ifdef VXWORKS
    if (0 == i || ERROR == i)
        return 0;
#endif

    return i;
}
//--------------------------------------------------------------------------------
//      发送TCP数据
//  输入:
//      sock：              SOCKET句柄
//      buf：               缓冲区
//      size：              缓冲区大小
//  返回:
//      _INT：              接收数据长度
//--------------------------------------------------------------------------------
_INT SYS_SockSend(SYS_SOCKET_ID sock, _UCHAR* buf, _INT len)
{
    return (send(sock, (char*)buf, len, 0));
}
//--------------------------------------------------------------------------------
//      接收UDP数据包
//  输入:
//      sock：              SOCKET句柄
//      buf：               缓冲区
//      len：               数据大小
//      addr：              地址
//  返回:
//      _INT：              接收数据长度
//--------------------------------------------------------------------------------
_INT SYS_SockURecv(SYS_SOCKET_ID sock, _UCHAR* buf, _INT size, SYS_IP_ADDR* addr)
{
#ifdef UNIX
    struct sockaddr_in s;
    int i;
    _INT j;

    i = sizeof(s);
    j = recvfrom(sock, (char*)buf, size, 0, (struct sockaddr*)&s, &i);
    if (0 == j || -1 == j)//????????
        return 0;
    if(0 != addr)
    {
        addr[0].ip = s.sin_addr.s_addr;
        addr[0].port = s.sin_port;
    }
    return j;
#endif
    
#ifdef WINDOWS
    struct sockaddr_in s;
    int i, j;

    i = sizeof(s);
    j = recvfrom(sock, buf, size, 0, (struct sockaddr*)&s, &i);
    if (0 == j || SOCKET_ERROR == j)
        return 0;
    if(NULL != addr)
    {
        addr[0].ip = s.sin_addr.S_un.S_addr;
        addr[0].port = s.sin_port;
    }
    return j;
#endif
    
#ifdef VXWORKS
    struct sockaddr_in s;
    int i, j;

    i = sizeof(s);
    j = recvfrom(sock, (_CHAR *)buf, size, 0, (struct sockaddr*)&s, &i);
    if (0 == j || ERROR == j)
        return 0;
    if(NULL != addr)
    {
        addr[0].ip = s.sin_addr.s_addr;
        addr[0].port = s.sin_port;
    }
    return j;
#endif
}
//--------------------------------------------------------------------------------
//      发送UDP数据包
//  输入:
//      sock：              SOCKET句柄
//      buf：               缓冲区
//      len：               数据大小
//      addr：              地址
//  返回:
//      _INT：              发送数据长度
//--------------------------------------------------------------------------------
_INT SYS_SockUSend(SYS_SOCKET_ID sock, _UCHAR* buf, _INT len, SYS_IP_ADDR* addr)
{
#ifdef UNIX
    struct sockaddr_in s;
    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr[0].port);
    s.sin_addr.s_addr = addr[0].ip;
    return (sendto(sock, (char*)buf, len, 0, (struct sockaddr*)&s, sizeof(s)));
#endif
    
#ifdef WINDOWS
    struct sockaddr_in s;
    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr[0].port);
    s.sin_addr.s_addr = addr[0].ip;
    return (sendto(sock, buf, len, 0, (struct sockaddr*)&s, sizeof(s)));
#endif

#ifdef VXWORKS
    struct sockaddr_in s;
    memset(&s,0,sizeof(s));
    s.sin_family=AF_INET;
    s.sin_port=htons(addr[0].port);
    s.sin_addr.s_addr = addr[0].ip;
    return (sendto(sock, (_CHAR *)buf, len, 0, (struct sockaddr*)&s, sizeof(s)));
#endif
}
//--------------------------------------------------------------------------------
//      关闭一个SOCKET句柄
//  输入:
//      sock：              SOCKET句柄
//          
//  返回:
//      SUCC:               成功
//      FAIL:               失败
//--------------------------------------------------------------------------------
_INT SYS_SockClose(SYS_SOCKET_ID sock)
{
#ifdef UNIX
//    if(-1 == unlink((char*)&sock)) return -1;
//    shutdown(sock, 2);//#define SD_BOTH         0x02
    return (0 == close(sock) ? 0 : -1);
#endif
    
#ifdef WINDOWS
    shutdown(sock, 2);//#define SD_BOTH         0x02
    return (0 == closesocket(sock) ? 0 : -1);
#endif
    
#ifdef VXWORKS
    shutdown(sock, 2);//#define SD_BOTH         0x02
    return (ERROR != close(sock) ? 0 : -1);
#endif
}
//--------------------------------------------------------------------------------
//      判断一个SOCKET是否处于连接状态
//  输入:
//      sock：              SOCKET句柄
//  返回:
//      0:                  成功
//      -1:                 失败
//--------------------------------------------------------------------------------
_INT SYS_SockIsConnected(SYS_SOCKET_ID sock)
{
    struct sockaddr_in s;
    int i;

    i = sizeof(s);
#ifdef UNIX
    return (0 == getpeername(sock, (struct sockaddr*)&s, &i) ? 0 : -1);
#endif

#ifdef WINDOWS
    return (0 == getpeername(sock, (struct sockaddr*)&s, &i) ? 0 : -1);
#endif

#ifdef VXWORKS
    return (OK == getpeername(sock, (struct sockaddr*)&s, &i) ? 0 : -1);
#endif

}

_VOID SYS_PRINT(const _UCHAR * szDbgStr)
{
#ifdef VXWORKS
	/*logMsg("%s", (_INT)szDbgStr, 0, 0, 0, 0, 0);*/
	printf("%s", szDbgStr);
#endif
	
#ifdef WINDOWS
	printf("%s", szDbgStr);
#endif
	
#ifdef UNIX
	printf("%s", szDbgStr);
#endif
}
